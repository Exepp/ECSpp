#ifndef EPP_PIPELINE_H
#define EPP_PIPELINE_H

#include <ECSpp/EntityManager.h>
#include <ECSpp/utility/TuplePP.h>
#include <condition_variable>
#include <thread>

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
    TaskBase(TaskBase* upt = nullptr) : fence(Fence()), uptask(upt) {}
    virtual ~TaskBase() = default;
    virtual void run(EntityManager& mgr) = 0;
    virtual bool updateAndCheckIfHasEntities(EntityManager& mgr) = 0;

    Fence fence;
    std::mutex mutex;
    std::condition_variable condVar;
    TaskBase* uptask = nullptr;
    std::unique_ptr<TaskBase> subtask;
};

template <typename FnType, typename StepFnType, typename... CTypes>
class Task : public TaskBase {
    struct JoinGuard {
        JoinGuard(std::thread& thread) : thr(thread){};
        ~JoinGuard() { thr.joinable() ? thr.join() : void(); }
        std::thread& thr;
    };

public:
    using Select_t = Selection<CTypes...>;
    using Iterator_t = typename Select_t::Iterator_t;
    static_assert(std::is_invocable_v<FnType, Iterator_t const&>);

public:
    Task(FnType fn, StepFnType sFn, TaskBase* upt = nullptr)
        : TaskBase(upt), runFn(std::move(fn)), stepFn(std::move(sFn)) {}

    template <typename... OtherCTypes, typename OtherFnType>
    Task<OtherFnType, StepFnType, OtherCTypes...>& setSubtask(OtherFnType fn)
    {
        static_assert(TuplePP<OtherCTypes...>::template containsType<CTypes...>());
        return static_cast<Task<OtherFnType, StepFnType, OtherCTypes...>&>(*(
            subtask = std::make_unique<Task<OtherFnType, StepFnType, OtherCTypes...>>(std::move(fn), stepFn, this)));
    }

    virtual void run(EntityManager& mgr) override
    {
        std::thread subThread;
        JoinGuard jGuard(subThread);
        fence = Fence();
        mgr.updateSelection(select);
        if (subtask && subtask->updateAndCheckIfHasEntities(mgr))
            subThread = std::thread(&TaskBase::run, subtask.get(), std::ref(mgr));

        if (uptask) // is subtask
            if (subThread.joinable())
                _run<true, true>();
            else
                _run<true, false>();
        else // is main task
            if (subThread.joinable())
            _run<false, true>();
        else
            _run<false, false>();
    }

private:
    template <bool isSub, bool hasSub>
    void _run()
    {
        Iterator_t it = select.begin(), end = select.end(), tempEnd = it;
        std::size_t const step = isSub ? 0 : stepFn(select.countEntities());
        while (it != end) {
            if constexpr (isSub) {
                std::unique_lock guard(uptask->mutex); // no need to lock this->mutex - only this thread can modify its fence
                uptask->condVar.wait(guard, [&] { return fence < uptask->fence; });
                tempEnd.jumpToOrBeyond(uptask->fence.sId, uptask->fence.pIdx);
            }
            else if constexpr (hasSub) // is main
                tempEnd += step;
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

    virtual bool updateAndCheckIfHasEntities(EntityManager& mgr) override
    {
        mgr.updateSelection(select);
        return select.countEntities() != 0;
    }

private:
    FnType runFn;
    StepFnType stepFn;
    Select_t select;
};


class Pipeline {
    inline static auto const DefRunFn = [](auto&) {};
    using DefaultRunFn_t = decltype(DefRunFn);

    inline static auto const DefStepFn = [](std::size_t n) -> std::size_t {
        return std::max(std::size_t(32), std::min(std::size_t(512), n / 10));
    };
    using DefaultStepFn_t = decltype(DefStepFn);

public:
    template <typename... CTypes>
    using TaskIterator_t = typename Task<DefaultRunFn_t, DefaultStepFn_t, CTypes...>::Iterator_t;

public:
    void run(EntityManager& mgr)
    {
        if (mainTask)
            mainTask->run(mgr);
    }

    template <typename... CTypes, typename FnType, typename StepFnType = DefaultStepFn_t>
    Task<FnType, StepFnType, CTypes...>& setTask(FnType fn, StepFnType stepFn = DefStepFn)
    {
        return static_cast<Task<FnType, StepFnType, CTypes...>&>(*(
            mainTask = std::make_unique<Task<FnType, StepFnType, CTypes...>>(std::move(fn), std::move(stepFn))));
    }

private:
    std::unique_ptr<TaskBase> mainTask;
};

template <typename... CTypes>
using TaskIterator = Pipeline::TaskIterator_t<CTypes...>;

} // namespace epp

#endif