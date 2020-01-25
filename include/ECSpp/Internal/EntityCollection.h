#ifndef ENTITYCOLLECTION_H
#define ENTITYCOLLECTION_H

#include <ECSpp/Internal/CFilter.h>
#include <ECSpp/Internal/EntitySpawner.h>
#include <ECSpp/Utility/TuplePP.h>

namespace epp {


template <typename... CTypes>
class EntityCollection {
    using EntityPools_t = std::vector<Pool<Entity> const*>;

    template <typename T> // discard T
    struct PoolsPtrs_t : public std::vector<CPool*> {};

    using PoolsPtrsPack_t = TuplePP<PoolsPtrs_t<CTypes>...>;


    //////////////////////// <iterator>

    template <bool IsConst>
    class Iterator {
        using Collection_t = std::conditional_t<IsConst, EntityCollection const, EntityCollection>;

        using SIdx_t = std::size_t;

    public:
        Iterator(Collection_t& collection, bool end = false);


        template <typename T>
        T& getComponent() const;


        // incrementing an end iterator throws
        Iterator& operator++();

        // incrementing an end iterator throws
        Iterator operator++(int);

        Entity operator*() const;

        bool operator==(Iterator const& other) const;

        bool operator!=(Iterator const& other) const;

    private:
        bool isValid() const;

        void findNextSpawner();

    private:
        EntityPools_t* entityPools;

        PoolsPtrsPack_t* poolsPack;

        SIdx_t spawnerIdx;

        PoolIdx poolIdx;

        friend class EntityManager;
    };

    //////////////////////// </iterator>

public:
    using Iterator_t = Iterator<false>;

    using ConstIterator_t = Iterator<true>;

public:
    EntityCollection() : filter(IdOfL<CTypes...>(), {}) {}

    explicit EntityCollection(CMask const& unwanted) : filter(IdOfL<CTypes...>(), unwanted) {}

    EntityCollection(EntityCollection const&) = default;

    EntityCollection(EntityCollection&&) = default;

    EntityCollection& operator=(EntityCollection const&) = default;

    EntityCollection& operator=(EntityCollection&&) = default;


    Iterator_t begin();

    Iterator_t end();

    ConstIterator_t begin() const;

    ConstIterator_t end() const;


    const CFilter& getFilter() const { return filter; }

private:
    void addSpawnerIfMeetsRequirements(EntitySpawner& spawner)
    {
        if (filter & spawner.mask) {
            entityPools.push_back(&spawner.getEntities());
            (poolsPack.template get<PoolsPtrs_t<CTypes>>().push_back(&spawner.getPool(IdOf<CTypes>())), ...);
        }
    }

private:
    EntityPools_t entityPools; // one pool for every accepted spawner

    PoolsPtrsPack_t poolsPack; // for each component type, a vector of pools of that component,
                               // one pool for every accepted spawner

    CFilter filter;

    std::size_t checkedSpawnersCount = 0;


    friend class EntityManager;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template <typename... CTypes>
template <bool IsConst>
inline EntityCollection<CTypes...>::Iterator<IsConst>::Iterator(Collection_t& col, bool end)
    : entityPools(&col.entityPools),
      poolsPack(&col.poolsPack),
      spawnerIdx(end ? std::numeric_limits<SIdx_t>::max() : 0)
{
    findNextSpawner();
}

template <typename... CTypes>
template <bool IsConst>
template <typename T>
inline T& EntityCollection<CTypes...>::Iterator<IsConst>::getComponent() const
{
    // redundant (with tuple's assert), but a more verbose message
    static_assert(isTypeInPack<T, CTypes...>(), "This type is not specified in this collection's declaration");
    EPP_ASSERT(isValid());
    return *static_cast<T*>((*poolsPack->template get<PoolsPtrs_t<T>>()[spawnerIdx])[poolIdx.value]);
}

template <typename... CTypes>
template <bool IsConst>
inline typename EntityCollection<CTypes...>::template Iterator<IsConst>&
EntityCollection<CTypes...>::Iterator<IsConst>::operator++()
{
    EPP_ASSERT(spawnerIdx != std::numeric_limits<SIdx_t>::max());
    ++poolIdx.value;
    if (!isValid()) {
        ++spawnerIdx;
        findNextSpawner();
    }
    return *this;
}

template <typename... CTypes>
template <bool IsConst>
inline typename EntityCollection<CTypes...>::template Iterator<IsConst>
EntityCollection<CTypes...>::Iterator<IsConst>::operator++(int)
{
    Iterator result = *this;
    ++(*this);
    return result;
}

template <typename... CTypes>
template <bool IsConst>
inline void
EntityCollection<CTypes...>::Iterator<IsConst>::findNextSpawner()
{
    poolIdx.value = 0;
    for (; spawnerIdx < poolsPack->template get<0>().size(); ++spawnerIdx)
        if (poolsPack->template get<0>()[spawnerIdx]->size())
            return;
    spawnerIdx = std::numeric_limits<SIdx_t>::max(); // end
}

template <typename... CTypes>
template <bool IsConst>
inline bool
EntityCollection<CTypes...>::Iterator<IsConst>::isValid() const
{
    EPP_ASSERT(spawnerIdx < poolsPack->template get<0>().size());
    return poolIdx.value < poolsPack->template get<0>()[spawnerIdx]->size();
}

template <typename... CTypes>
template <bool IsConst>
inline Entity
    EntityCollection<CTypes...>::Iterator<IsConst>::operator*() const
{
    EPP_ASSERT(isValid());
    return (*(*entityPools)[spawnerIdx]).data[poolIdx.value];
}

template <typename... CTypes>
template <bool IsConst>
inline bool
EntityCollection<CTypes...>::Iterator<IsConst>::operator==(Iterator const& other) const
{
    return spawnerIdx == other.spawnerIdx && poolIdx.value == other.poolIdx.value;
}

template <typename... CTypes>
template <bool IsConst>
inline bool
EntityCollection<CTypes...>::Iterator<IsConst>::operator!=(Iterator const& other) const
{
    return !(*this == other);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename... CTypes>
inline typename EntityCollection<CTypes...>::Iterator_t
EntityCollection<CTypes...>::begin()
{
    return Iterator_t(*this);
}

template <typename... CTypes>
inline typename EntityCollection<CTypes...>::Iterator_t
EntityCollection<CTypes...>::end()
{
    return Iterator_t(*this, true);
}

template <typename... CTypes>
inline typename EntityCollection<CTypes...>::ConstIterator_t
EntityCollection<CTypes...>::begin() const
{
    return ConstIterator_t(*this);
}

template <typename... CTypes>
inline typename EntityCollection<CTypes...>::ConstIterator_t
EntityCollection<CTypes...>::end() const
{
    return ConstIterator_t(*this, true);
}

} // namespace epp

#endif // ENTITYCOLLECTION_H