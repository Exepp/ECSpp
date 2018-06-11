
template<bool IsConst, class FirstType, class ...CTypes>
inline ASpawnersPackIterator<IsConst, FirstType, CTypes...>::
ASpawnersPackIterator(const ASpawnersHolder_t& spawners, PoolArraysHolder_t & poolArrays, size_t archetypeIndex) :
	spawners(spawners), poolArrays(poolArrays), archetypeIndex(archetypeIndex), firstTypePools(poolArrays.get<PArray<FirstType>>().getPools()), startingSize(firstTypePools.size())
{
	findValidIndices();
}

template<bool IsConst, class FirstType, class ...CTypes>
inline const ERefPtr_t & ASpawnersPackIterator<IsConst, FirstType, CTypes...>::getERefPtr() const
{
	return (*spawners[archetypeIndex])[entityIndex];
}

template<bool IsConst, class FirstType, class ...CTypes>
inline ASpawnersPackIterator<IsConst, FirstType, CTypes...> & ASpawnersPackIterator<IsConst, FirstType, CTypes...>::operator++()
{
	++entityIndex;
	if (!isValid())
	{
		++archetypeIndex;
		findValidIndices();
	}
	return *this;
}

template<bool IsConst, class FirstType, class ...CTypes>
inline void ASpawnersPackIterator<IsConst, FirstType, CTypes...>::findValidIndices()
{
	entityIndex = 0;
	while (archetypeIndex < startingSize)
		if (firstTypePools[archetypeIndex]->getSize())
		{
			return;
		}
		else
			++archetypeIndex;
}

template<bool IsConst, class FirstType, class ...CTypes>
inline bool ASpawnersPackIterator<IsConst, FirstType, CTypes...>::isValid() const
{
	return archetypeIndex < startingSize && entityIndex < firstTypePools[archetypeIndex]->getSize();
}

template<bool IsConst, class FirstType, class ...CTypes>
inline ASpawnersPackIterator<IsConst, FirstType, CTypes...> ASpawnersPackIterator<IsConst, FirstType, CTypes...>::operator++(int)
{
	ThisIterator_t result = *this;
	++(*this);
	return result;
}

template<bool IsConst, class FirstType, class ...CTypes>
inline typename ASpawnersPackIterator<IsConst, FirstType, CTypes...>::CProxyPack_t ASpawnersPackIterator<IsConst, FirstType, CTypes...>::operator*()
{
	return  CProxyPack_t::Base_t({ (*firstTypePools[archetypeIndex])[entityIndex], poolArrays.get<PArray<CTypes>>()[archetypeIndex][entityIndex]... });
}

template<bool IsConst, class FirstType, class ...CTypes>
inline bool ASpawnersPackIterator<IsConst, FirstType, CTypes...>::operator==(const ThisIterator_t & other) const
{
	return &poolArrays == &poolArrays && archetypeIndex == other.archetypeIndex &&
		(archetypeIndex == startingSize || entityIndex == other.entityIndex);
}

template<bool IsConst, class FirstType, class ...CTypes>
inline bool ASpawnersPackIterator<IsConst, FirstType, CTypes...>::operator!=(const ThisIterator_t & other) const
{
	return !(*this == other);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class ...CTypes>
inline typename ASpawnersPack<CTypes...>::Iterator_t ASpawnersPack<CTypes...>::begin()
{
	return Iterator_t(spawners, poolArrays);
}

template<class ...CTypes>
inline typename ASpawnersPack<CTypes...>::ConstIterator_t ASpawnersPack<CTypes...>::begin() const
{
	return ConstIterator_t(spawners, poolArrays);
}

template<class ...CTypes>
inline typename ASpawnersPack<CTypes...>::Iterator_t ASpawnersPack<CTypes...>::end()
{
	return Iterator_t(spawners, poolArrays, spawners.size());
}

template<class ...CTypes>
inline typename ASpawnersPack<CTypes...>::ConstIterator_t ASpawnersPack<CTypes...>::end() const
{
	return ConstIterator_t(spawners, poolArrays, spawners.size());
}

template<class ...CTypes>
inline const typename ASpawnersPack<CTypes...>::ASpawnersHolder_t & ASpawnersPack<CTypes...>::getSpawners() const
{
	return spawners;
}