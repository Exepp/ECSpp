#ifndef Selection_H
#define Selection_H

#include <ECSpp/Internal/CFilter.h>
#include <ECSpp/Internal/EntitySpawner.h>
#include <ECSpp/Utility/TuplePP.h>

namespace epp {

template <typename... CTypes>
class Selection;

template <>
class Selection<> {
protected:
    using EntityPools_t = std::vector<Pool<Entity> const*>;

    //////////////////////// <iterator>

    template <class T>
    class IteratorBase {
    protected:
        using SIdx_t = std::uint32_t;

        constexpr static SIdx_t const EndValue = std::numeric_limits<SIdx_t>::max();

    public:
        IteratorBase(Selection& selection, bool end = false);
        T& operator++();                   // incrementing an end iterator throws
        T operator++(int);                 // incrementing an end iterator throws
        T& operator+=(std::size_t offset); /**  Returns an iterator to the entity that would
                                                be reached after "offset" increments, or end */
        T operator+(std::size_t offset) const;
        Entity operator*() const;
        bool operator==(T const& other) const;
        bool operator!=(T const& other) const;

    protected:
        bool isValid() const;
        void findNextSpawner();

    private:
        T& thisAsT();
        T const& thisAsT() const;

    protected:
        EntityPools_t const* entityPools;
        SIdx_t spawnerIdx;
        PoolIdx poolIdx;
    };


private:
    class EntIterator : public IteratorBase<EntIterator> {
    public:
        EntIterator(Selection& selection, bool end = false)
            : IteratorBase<EntIterator>(selection, end) { findNextSpawner(); };

    private:
        auto sizeOfCurrentSpawner() const { return (*entityPools)[spawnerIdx]->data.size(); }
        auto numOfSpawners() const { return entityPools->size(); }

        friend IteratorBase<EntIterator>;
    };

public:
    using Iterator_t = EntIterator;

    //////////////////////// </iterator>

public:
    explicit Selection(CFilter const& flr) : filter(flr) {}

    Selection() = default;

    Iterator_t begin() { return Iterator_t(*this); }
    Iterator_t end() { return Iterator_t(*this, true); }

    const CFilter& getFilter() const { return filter; }

    std::size_t size() const;

private:
    void addSpawnerIfMeetsRequirements(EntitySpawner& spawner);

protected:
    EntityPools_t entityPools; // one pool for each accepted spawner

    CFilter filter;

    std::size_t checkedSpawnersCount = 0;


    friend class EntityManager;
};


template <typename... CTypes>
class Selection : public Selection<> {
    using Base_t = Selection<>;

    template <typename T> // discard T
    struct PoolsPtrs_t : public std::vector<CPool*> {};

    using PoolsPtrsPack_t = TuplePP<PoolsPtrs_t<CTypes>...>;


    //////////////////////// <iterator>

    class CompIterator : public IteratorBase<CompIterator> {
        using ItBase_t = IteratorBase<CompIterator>;

        using ItBase_t::findNextSpawner;
        using ItBase_t::isValid;
        using ItBase_t::poolIdx;
        using ItBase_t::spawnerIdx;


    public:
        CompIterator(Selection& sel, bool end = false)
            : ItBase_t(sel, end), poolsPack(&sel.poolsPack) { findNextSpawner(); }

        template <typename T>
        T& getComponent() const;

    private:
        auto sizeOfCurrentSpawner() const { return poolsPack->template get<0>()[spawnerIdx]->size(); }
        auto numOfSpawners() const { return poolsPack->template get<0>().size(); }

    private:
        PoolsPtrsPack_t* poolsPack;

        friend ItBase_t;
        friend class EntityManager;
    };

    //////////////////////// </iterator>

public:
    using Iterator_t = CompIterator;

public:
    Selection() : Base_t({ IdOfL<CTypes...>(), {} }) {}

    explicit Selection(CMask const& unwanted) : Base_t({ IdOfL<CTypes...>(), unwanted }) {}

    Iterator_t begin() { return Iterator_t(*this); }
    Iterator_t end() { return Iterator_t(*this, true); }

private:
    void addSpawnerIfMeetsRequirements(EntitySpawner& spawner);

private:
    PoolsPtrsPack_t poolsPack; // for each component type, a vector of pools of that component,
                               // one pool for every accepted spawner
    friend class EntityManager;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
inline Selection<>::IteratorBase<T>::IteratorBase(Selection& sel, bool end)
    : entityPools(&sel.entityPools), spawnerIdx(end ? EndValue : 0) {}

template <class T>
inline T& Selection<>::IteratorBase<T>::operator++()
{
    EPP_ASSERT(spawnerIdx < EndValue);
    ++poolIdx.value;
    if (!isValid()) {
        ++spawnerIdx;
        findNextSpawner();
    }
    return thisAsT();
}

template <class T>
inline T Selection<>::IteratorBase<T>::operator++(int)
{
    T result = thisAsT();
    ++thisAsT();
    return result;
}

template <class T>
inline T& Selection<>::IteratorBase<T>::operator+=(std::size_t offset)
{
    poolIdx.value += offset;
    auto spawnersNum = thisAsT().numOfSpawners();
    for (; spawnerIdx < spawnersNum; ++spawnerIdx)
        if (auto size = thisAsT().sizeOfCurrentSpawner(); size <= poolIdx.value)
            poolIdx.value -= size;
        else
            return thisAsT();
    spawnerIdx = EndValue;
    return thisAsT();
}

template <class T>
inline T Selection<>::IteratorBase<T>::operator+(std::size_t offset) const
{
    auto it = thisAsT();
    return it += offset;
}

template <class T>
inline void Selection<>::IteratorBase<T>::findNextSpawner()
{
    poolIdx.value = 0;
    auto spawnersNum = thisAsT().numOfSpawners();
    for (; spawnerIdx < spawnersNum; ++spawnerIdx)
        if (thisAsT().sizeOfCurrentSpawner())
            return;
    spawnerIdx = EndValue;
}

template <class T>
inline bool Selection<>::IteratorBase<T>::isValid() const
{
    EPP_ASSERT(spawnerIdx < thisAsT().numOfSpawners());
    return poolIdx.value < thisAsT().sizeOfCurrentSpawner();
}

template <class T>
inline Entity Selection<>::IteratorBase<T>::operator*() const
{
    EPP_ASSERT(isValid());
    return (*(*entityPools)[spawnerIdx]).data[poolIdx.value];
}

template <class T>
inline bool Selection<>::IteratorBase<T>::operator==(T const& other) const
{
    return spawnerIdx == other.spawnerIdx && poolIdx.value == other.poolIdx.value;
}

template <class T>
inline bool Selection<>::IteratorBase<T>::operator!=(T const& other) const
{
    return !(thisAsT() == other);
}

template <class T>
inline T& Selection<>::IteratorBase<T>::thisAsT()
{
    return *static_cast<T*>(this);
}

template <class T>
inline T const& Selection<>::IteratorBase<T>::thisAsT() const
{
    return *static_cast<T const*>(this);
}


///////////////////////////////////////////////////////////

inline std::size_t Selection<>::size() const
{
    std::size_t result = 0;
    for (auto const& entpool : entityPools)
        result += entpool->data.size();
    return result;
}

inline void Selection<>::addSpawnerIfMeetsRequirements(EntitySpawner& spawner)
{
    if (filter & spawner.mask) entityPools.push_back(&spawner.getEntities());
}

template <typename... CTypes>
template <typename T>
inline T& Selection<CTypes...>::CompIterator::getComponent() const
{
    // redundant (with tuple's assert), but a more verbose message
    static_assert(isTypeInPack<T, CTypes...>(), "This type is not specified in this declaration of this Selection");
    EPP_ASSERT(isValid());
    return *static_cast<T*>((*poolsPack->template get<PoolsPtrs_t<T>>()[spawnerIdx])[poolIdx.value]);
}

template <typename... CTypes>
inline void Selection<CTypes...>::addSpawnerIfMeetsRequirements(EntitySpawner& spawner)
{
    if (filter & spawner.mask) {
        entityPools.push_back(&spawner.getEntities());
        (poolsPack.template get<PoolsPtrs_t<CTypes>>().push_back(&spawner.getPool(IdOf<CTypes>())), ...);
    }
}


} // namespace epp

#endif // Selection_H