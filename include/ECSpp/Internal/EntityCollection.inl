
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