
template<class... CTypes>
template<bool IsConst>
inline EntityCollection<CTypes...>::Iterator<IsConst>::Iterator(Collection_t& col, std::size_t sIdx)
    : collection(col)
    , spawnerIdx(sIdx)
{
    findValidIndices();
}

template<class... CTypes>
template<bool IsConst>
template<class T>
inline T& EntityCollection<CTypes...>::Iterator<IsConst>::getComponent() const
{
    assert(spawnerIdx < collection.spawners.size());
    return collection.poolsCollection.template get<PoolPtrs_t<T>>()[spawnerIdx][poolIdx];
}

template<class... CTypes>
template<bool IsConst>
inline Archetype const& EntityCollection<CTypes...>::Iterator<IsConst>::getArchetype() const
{
    assert(spawnerIdx < collection.spawners.size());
    return collection.spawners[spawnerIdx]->getArchetype();
}

template<class... CTypes>
template<bool IsConst>
inline typename EntityCollection<CTypes...>::template Iterator<IsConst>& EntityCollection<CTypes...>::Iterator<IsConst>::operator++()
{
    ++poolIdx;
    if (!isValid())
    {
        ++spawnerIdx;
        findValidIndices();
    }
    return *this;
}

template<class... CTypes>
template<bool IsConst>
inline typename EntityCollection<CTypes...>::template Iterator<IsConst> EntityCollection<CTypes...>::Iterator<IsConst>::operator++(int)
{
    ThisIterator_t result = *this;
    ++(*this);
    return result;
}

template<class... CTypes>
template<bool IsConst>
inline void EntityCollection<CTypes...>::Iterator<IsConst>::findValidIndices()
{
    poolIdx = 0;
    while (spawnerIdx < collection.spawners.size())
        if (collection.spawners[spawnerIdx]->getEntities().content.size())
            return;
        else
            ++spawnerIdx;
}

template<class... CTypes>
template<bool IsConst>
inline bool EntityCollection<CTypes...>::Iterator<IsConst>::isValid() const
{
    assert(spawnerIdx < collection.spawners.size());
    return poolIdx < collection.spawners[spawnerIdx]->getEntities().content.size();
}

template<class... CTypes>
template<bool IsConst>
inline Entity EntityCollection<CTypes...>::Iterator<IsConst>::operator*()
{
    assert(spawnerIdx < collection.spawners.size());
    return collection.spawners[spawnerIdx]->getEntities()[poolIdx];
}

template<class... CTypes>
template<bool IsConst>
inline bool EntityCollection<CTypes...>::Iterator<IsConst>::operator==(ThisIterator_t const& other) const
{
    return &collection == &other.collection && spawnerIdx == other.spawnerIdx && poolIdx == other.poolIdx;
}

template<class... CTypes>
template<bool IsConst>
inline bool EntityCollection<CTypes...>::Iterator<IsConst>::operator!=(ThisIterator_t const& other) const
{
    return !(*this == other);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class... CTypes>
inline typename EntityCollection<CTypes...>::Iterator_t EntityCollection<CTypes...>::begin()
{
    return Iterator_t(poolsCollection);
}

template<class... CTypes>
inline typename EntityCollection<CTypes...>::Iterator_t EntityCollection<CTypes...>::end()
{
    return Iterator_t(poolsCollection, spawners.size());
}

template<class... CTypes>
inline typename EntityCollection<CTypes...>::ConstIterator_t EntityCollection<CTypes...>::begin() const
{
    return ConstIterator_t(poolsCollection);
}

template<class... CTypes>
inline typename EntityCollection<CTypes...>::ConstIterator_t EntityCollection<CTypes...>::end() const
{
    return ConstIterator_t(poolsCollection, spawners.size());
}