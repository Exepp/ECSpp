#ifndef EPP_PIPELINE_H
#define EPP_PIPELINE_H

#include <ECSpp/EntityManager.h>
#include <ECSpp/internal/utility/TuplePP.h>
#include <condition_variable>
#include <future>

namespace epp {

class TaskBase {

protected:
    struct Fence {
        bool operator==(Fence const& rhs) const { return sId == rhs.sId && pIdx == rhs.pIdx; }
        bool operator!=(Fence const& rhs) const { return !(*this == rhs); }
        bool operator<(Fence const& rhs) const { return sId < rhs.sId || (sId == rhs.sId && pIdx < rhs.pIdx); }

        SpawnerId sId = SpawnerId(0);
        PoolIdx pIdx = PoolIdx(0);
    };

public:
    TaskBase(EntityManager& mgrRef, std::launch lPolicy, TaskBase* upt)
        : mgr(mgrRef), launchPolicy(lPolicy), uptask(upt) {}
    virtual ~TaskBase() = default;
    virtual void run() = 0;
    virtual bool updateAndCheckIfHasEntities() = 0;

    EntityManager& mgr;

    Fence fence;
    std::mutex mutex;
    std::condition_variable condVar;

    std::launch launchPolicy;
    TaskBase* uptask = nullptr;
    std::unique_ptr<TaskBase> subtask;
};


/// A simple tool for creating basic concurrent code
/**
 * This class eliminates race conditions by ensuring a single order of iteration for every task
 * and keeping subtask always behind the uptasks. The execution of tasks is divided into batches. 
 * The task will notify the subtask (if there is one) after completing every batch, so the subtask 
 * can reach entities of that batch aswell. This way there are no concurrent reads or writes
 * as long as EntityManager or any other external objects (pointers) are not used.
 * @tparam FnType Callable type that takes Selection<CTypes...>::Iterator_t const& as the argument
 * @tparam BatchSizeFnType A Callable type that takes the number of entities as the argument 
 *         and returns the size of a batch.
 * @tparam CTypes Types of the components used to select wanted entites
 */
template <typename FnType, typename BatchSizeFnType, typename... CTypes>
class Task : public TaskBase {
public:
    using Select_t = Selection<CTypes...>;
    using Iterator_t = typename Select_t::Iterator_t;
    static_assert(std::is_invocable_v<FnType, Iterator_t const&>, "Function of the task must take TaskIterator<CTypes...> const& as the argument");

public:
    /**
     * @param mgr EntityManager that will be used to select entities from
     * @param fn A callable object that takes Selection<CTypes...>::Iterator_t const& as the argument and will be invoked 
     *           for each entity that owns at least all of the components of CTypes... types
     * @param bFn A Callable object that takes the number of entities as the argument and returns the size of a batch.
     *            This object will be used for this task and its subtasks
     * @param lPolicy How will this task launch (applies only to subtasks)
     * @param upt A parent task, that will notify this task when new b
     * atch of entities will be ready to  
     */
    Task(EntityManager& mgr, FnType fn, BatchSizeFnType bFn,
         std::launch lPolicy = std::launch::async | std::launch::deferred, TaskBase* upt = nullptr)
        : TaskBase(mgr, lPolicy, upt),
          runFn(std::move(fn)),
          batchSizeFn(std::move(bFn)) {}


    /// Sets the subtask that will (always) run on a new thread next to this task
    /**
     * @note This function compiles only if the subtask will cover a subset of entities that this task covers
     * @tparam OtherCTypes Types of the components used to select wanted entites for this subtask. For this function to compile, 
     *         OtherCTypes must contain at least all of the CTypes of this task. If OtherCTypes is empty, CTypes of this task are used.
     * @tparam OtherFnType A callable type that takes Selection<OtherCTypes...>::Iterator_t const& as the argument
     * @param fn A callable object that takes Selection<OtherCTypes...>::Iterator_t const& as the argument and that will be invoked 
     *           for each entity that owns at least all of the components of CTypes... types
     * @returns Reference to the created subtask, which can be used to create even more subtasks
     */
    template <typename... OtherCTypes, typename OtherFnType>
    decltype(auto)
    setSubtask(OtherFnType fn, std::launch lPolicy = std::launch::async | std::launch::deferred)
    {
        if constexpr (sizeof...(OtherCTypes)) {
            static_assert(TuplePP<OtherCTypes...>::template containsType<CTypes...>());
            return static_cast<Task<OtherFnType, BatchSizeFnType, OtherCTypes...>&>(*(
                subtask = std::make_unique<Task<OtherFnType, BatchSizeFnType, OtherCTypes...>>(mgr, std::move(fn), batchSizeFn, lPolicy, this)));
        }
        else
            return static_cast<Task<OtherFnType, BatchSizeFnType, CTypes...>&>(*(
                subtask = std::make_unique<Task<OtherFnType, BatchSizeFnType, CTypes...>>(mgr, std::move(fn), batchSizeFn, lPolicy, this)));
    }

    /// Runs the main task and the subtask (if set)
    /**
     * This function returns only if every task completed its execution
     */
    virtual void run() override
    {
        std::future<void> subFt; // waits in the destructor for task to finish
        fence = Fence();
        mgr.updateSelection(select);
        if (subtask && subtask->updateAndCheckIfHasEntities())
            subFt = std::async(subtask->launchPolicy, &TaskBase::run, subtask.get());

        if (uptask) // is subtask
            if (subFt.valid())
                _run<true, true>();
            else
                _run<true, false>();
        else // is main task
            if (subFt.valid())
            _run<false, true>();
        else
            _run<false, false>();
    }

private:
    template <bool isSub, bool hasSub>
    void _run()
    {
        Iterator_t it = select.begin(), end = select.end(), tempEnd = it;
        std::size_t const batchSize = isSub ? 0 : batchSizeFn(select.countEntities());
        while (it != end) {
            if constexpr (isSub) {
                std::unique_lock guard(uptask->mutex); // no need to lock this->mutex - only this thread can modify its fence
                uptask->condVar.wait(guard, [&] { return fence < uptask->fence; });
                tempEnd.jumpToOrBeyond(uptask->fence.sId, uptask->fence.pIdx);
            }
            else if constexpr (hasSub) // is main
                tempEnd += batchSize;
            else
                tempEnd = end;

            for (; it != tempEnd; ++it)
                runFn(it);
            if constexpr (hasSub) {
                std::lock_guard guard(mutex);
                fence = { it.getSpawnerId(), it.getPoolIdx() };
                condVar.notify_one();
            }
        }
    }

    virtual bool updateAndCheckIfHasEntities() override
    {
        mgr.updateSelection(select);
        return select.countEntities() != 0;
    }

private:
    FnType runFn;
    BatchSizeFnType batchSizeFn;
    Select_t select;
};


/// A simple tool for creating basic concurrent code
class Pipeline {
    inline static auto const DefRunFn = [](auto&) {};
    using DefaultRunFn_t = decltype(DefRunFn);

    inline static auto const DefBatchSizeFn = [](std::size_t n) -> std::size_t {
        return std::max(std::size_t(32), std::min(std::size_t(512), n / 10));
    };
    using DefaultBatchSizeFn_t = decltype(DefBatchSizeFn);

public:
    template <typename... CTypes>
    using TaskIterator_t = typename Task<DefaultRunFn_t, DefaultBatchSizeFn_t, CTypes...>::Iterator_t;

public:
    /// Runs the main task with its subtasks
    /**
     * This function returns only if every task completed its execution
     * @param mgr Manager with entities that sh to update internal selection
     */
    void run()
    {
        if (mainTask)
            mainTask->run();
    }

    /// Sets the main task
    /**
     * @tparam CTypes Types of the components used to select wanted entites for this subtask. For this function to compile, 
     *         CTypes must contain at least all of the CTypes of this task.
     * @tparam FnType A callable type that takes Selection<CTypes...>::Iterator_t const& as the argument
     * @tparam BatchSizeFnType A Callable type that takes the number of entities as the argument and returns the size of a batch.
     * @param mgr EntityManager that will be used to select the entities from
     * @param fn A callable object that takes Selection<CTypes...>::Iterator_t const& as the argument and that will be invoked 
     *           for each entity that owns at least all of the components of CTypes... types
     * @param bFn A Callable object that takes the number of entities as the argument and returns the size of a batch.
     *            Copies of this object will be used for this task and its subtasks
     * @returns Reference to the created task, which can be used to create subtasks
     */
    template <typename... CTypes, typename FnType, typename BatchSizeFnType = DefaultBatchSizeFn_t>
    Task<FnType, BatchSizeFnType, CTypes...>& setTask(EntityManager& mgr, FnType fn,
                                                      BatchSizeFnType batchSizeFn = DefBatchSizeFn)
    {
        return static_cast<Task<FnType, BatchSizeFnType, CTypes...>&>(*(
            mainTask = std::make_unique<Task<FnType, BatchSizeFnType, CTypes...>>(mgr, std::move(fn), std::move(batchSizeFn))));
    }

private:
    std::unique_ptr<TaskBase> mainTask;
};


/// A convenient typedef usefull to figure out what is the type that the task expects
/**
 * @tparam CTypes A pack of types specified in the Task 
 */
template <typename... CTypes>
using TaskIterator = Pipeline::TaskIterator_t<CTypes...>;

} // namespace epp

#endif