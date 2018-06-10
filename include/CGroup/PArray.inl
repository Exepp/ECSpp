
template<class T, bool IsConst>
inline PArrayIterator<T, IsConst>::PArrayIterator(PoolsHolderIterator_t poolsHolderBegin, PoolsHolderIterator_t poolsHolderEnd) :
	holderIt(poolsHolderBegin), holderEndIt(poolsHolderEnd)
{
	findValidPoolIt();
}

template<class T, bool IsConst>
inline typename PArrayIterator<T, IsConst>::ValueRef_t & PArrayIterator<T, IsConst>::operator*() const
{
	return *poolIt;
}

template<class T, bool IsConst>
inline PArrayIterator<T, IsConst>& PArrayIterator<T, IsConst>::operator++()
{
	++poolIt;
	if (poolIt == ((Pool_t*)*holderIt)->end()) // Pool_t* cast for a proper return value
	{
		++holderIt;
		findValidPoolIt();
	}
	return *this;
}

template<class T, bool IsConst>
inline PArrayIterator<T, IsConst> PArrayIterator<T, IsConst>::operator++(int)
{
	ThisIterator_t result = *this;
	++(*this);
	return result;

}

template<class T, bool IsConst>
inline bool PArrayIterator<T, IsConst>::operator==(const ThisIterator_t & other) const
{
	return holderIt == other.holderIt && (holderIt == holderEndIt || poolIt == other.poolIt);
}

template<class T, bool IsConst>
inline bool PArrayIterator<T, IsConst>::operator!=(const ThisIterator_t & other) const
{
	return !(*this == other);
}

template<class T, bool IsConst>
inline void PArrayIterator<T, IsConst>::findValidPoolIt()
{
	while (holderIt != holderEndIt)
		if ((*holderIt)->getSize())
		{
			poolIt = ((Pool_t*)*holderIt)->begin(); // Pool_t* cast for a proper return value
			return;
		}
		else
			++holderIt;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////


template<class T>
inline void PArray<T>::addPool(Pool<T>& pool)
{
	pools.push_back(&pool);
}

template<class T>
inline typename PArray<T>::Iterator_t PArray<T>::begin()
{
	return Iterator_t(pools.begin(), pools.end());
}

template<class T>
inline typename PArray<T>::ConstIterator_t PArray<T>::begin() const
{
	return ConstIterator_t(pools.cbegin(), pools.cend());
}

template<class T>
inline typename PArray<T>::Iterator_t PArray<T>::end()
{
	return Iterator_t(pools.end(), pools.end());
}

template<class T>
inline typename PArray<T>::ConstIterator_t PArray<T>::end() const
{
	return ConstIterator_t(pools.cend(), pools.cend());
}

template<class T>
inline size_t PArray<T>::size() const
{
	return pools.size();
}

template<class T>
inline Pool<T>& PArray<T>::operator[](size_t i)
{
	EXC_ASSERT((i < pools.size()), std::out_of_range, "Pool index out of range")
	return *pools[i];
}

template<class T>
inline const Pool<T>& PArray<T>::operator[](size_t i) const
{
	EXC_ASSERT((i < pools.size()), std::out_of_range, "Pool index out of range")
	return *pools[i];
}

template<class T>
inline const typename PArray<T>::PoolPtrsHolder_t& PArray<T>::getPools() const
{
	return pools;
}

