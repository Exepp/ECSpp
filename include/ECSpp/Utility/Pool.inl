
template<class T>
inline T* allocNObjects(size_t n)
{
	return (T*)::operator new(n * sizeof(T), std::align_val_t(alignof(T)));
}



template<class T>
inline void destroyMObjects(T* memory, size_t m)
{
	for (size_t i = 0; i < m; i++)
		(memory + i)->~T();
}

template<class T>
inline void destroyMFreeNObjects(T* memory, size_t m, size_t n)
{
	destroyMObjects(memory, m);
	::operator delete((void *)memory, n * sizeof(T), std::align_val_t(alignof(T)));
}

template<class T>
Pool<T>::Pool(ThisPool_t&& rhs)
{
	*this = std::move(rhs);
}

template<class T>
Pool<T>::~Pool()
{
	reserve(0);
}

template<class T>
Pool<T>& Pool<T>::operator=(ThisPool_t&& rhs)
{
	reserve(0);
	memory = rhs.memory;
	reserved = rhs.reserved;
	inUseCount = rhs.inUseCount;
	rhs.memory = nullptr;
	rhs.reserved = 0;
	rhs.inUseCount = 0;
	return *this;
}

template<class T>
template<class ...U>
inline T& Pool<T>::alloc(U&&... arg)
{
	 if (inUseCount >= reserved)
		reserve((reserved + DefaultReserveSize )* SizeMultiplier);

	T* obj = (memory + inUseCount++);
	new(obj) T(std::forward<U>(arg)...);
	return *obj;
}

template<class T>
template<class ...U>
inline typename Pool<T>::Iterator_t Pool<T>::allocN(size_t n, const U&... arg)
{
	if (inUseCount + n > reserved)
		reserve((reserved + n) * SizeMultiplier);
	
	for (size_t i = 0; i < n; i++)
	{
		T* obj = (memory + inUseCount++);
		new(obj) T(arg...);
	}
	return end() - n;
}


template<class T>
void Pool<T>::free(size_t i)
{
	if (i < inUseCount)
	{
		T* object = (memory + i);
		object->~T();
		--inUseCount;
		if (inUseCount && inUseCount != i)
		{
			new(object) T(std::move(*(memory + inUseCount)));
			(memory + inUseCount)->~T();
		}
	}
}

template<class T>
inline void Pool<T>::reserve(size_t n)
{
	if (reserved == n)
		return;

	T* newMemory = nullptr;
	size_t objToMoveNum = n < inUseCount ? n : inUseCount;
	if (n)
	{
		newMemory = allocNObjects<T>(n);
		for (size_t i = 0; i < objToMoveNum; i++)
			new(newMemory + i) T(std::move(*(memory + i)));
	}
	if (memory)
		destroyMFreeNObjects<T>(memory, inUseCount, reserved);

	inUseCount = objToMoveNum;
	reserved = n;
	memory = newMemory;
}

template<class T>
void Pool<T>::clear()
{
	if (inUseCount)
	{
		destroyMObjects(memory, inUseCount);
		inUseCount = 0;
	}
}

template<class T>
inline std::unique_ptr<Pool<T>> Pool<T>::makeEmptyCopy() const
{
	return std::make_unique<Pool<T>>();
}

template<class T>
inline T & Pool<T>::front()
{
	return *memory;
}

template<class T>
inline const T & Pool<T>::front() const
{
	return *memory;
}

template<class T>
inline T & Pool<T>::back()
{
	return *(memory + inUseCount - 1);
}

template<class T>
inline const T & Pool<T>::back() const
{
	return *(memory + inUseCount - 1);
}

template<class T>
typename Pool<T>::Iterator_t Pool<T>::begin()
{
	return Iterator_t(memory);
}

template<class T>
typename Pool<T>::ConstIterator_t Pool<T>::begin() const
{
	return ConstIterator_t(memory);
}

template<class T>
typename Pool<T>::Iterator_t Pool<T>::end()
{
	return Iterator_t(memory + inUseCount);
}

template<class T>
typename Pool<T>::ConstIterator_t Pool<T>::end() const
{
	return ConstIterator_t(memory + inUseCount);
}

template<class T>
inline size_t Pool<T>::getSize() const
{
	return inUseCount;
}

template<class T>
inline size_t Pool<T>::getReserved() const
{
	return reserved;
}

template<class T>
inline T & Pool<T>::operator[](size_t i)
{
	EXC_ASSERT((i < inUseCount), std::out_of_range, "Wrong entity id")
	return *(memory + i);
}

template<class T>
inline const T & Pool<T>::operator[](size_t i) const
{
	EXC_ASSERT((i < inUseCount), std::out_of_range, "Wrong entity id")
	return *(memory + i);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T, bool IsConst>
inline typename PoolIterator<T, IsConst>::reference PoolIterator<T, IsConst>::operator*() const
{
	return *ptr;
}

template<class T, bool IsConst>
inline typename PoolIterator<T, IsConst>::pointer PoolIterator<T, IsConst>::operator->() const
{
	return ptr;
}


template<class T, bool IsConst>
inline typename PoolIterator<T, IsConst>::ThisIterator_t & PoolIterator<T, IsConst>::operator++()
{
	++ptr;
	return *this;
}

template<class T, bool IsConst>
inline typename PoolIterator<T, IsConst>::ThisIterator_t PoolIterator<T, IsConst>::operator++(int)
{
	ThisIterator_t result = *this;
	++ptr;
	return std::move(result);
}

template<class T, bool IsConst>
inline typename PoolIterator<T, IsConst>::ThisIterator_t & PoolIterator<T, IsConst>::operator--()
{
	--ptr;
	return *this;
}

template<class T, bool IsConst>
inline typename PoolIterator<T, IsConst>::ThisIterator_t PoolIterator<T, IsConst>::operator--(int)
{
	ThisIterator_t result = *this;
	--ptr;
	return std::move(result);
}


template<class T, bool IsConst>
inline PoolIterator<T, IsConst> & PoolIterator<T, IsConst>::operator+=(difference_type n)
{
	ptr += n;
	return *this;
}

template<class T, bool IsConst>
inline PoolIterator<T, IsConst> & PoolIterator<T, IsConst>::operator-=(difference_type n)
{
	ptr -= n;
	return *this;
}

template<class T, bool IsConst>
inline PoolIterator<T, IsConst> PoolIterator<T, IsConst>::operator+(difference_type n) const
{
	return ThisIterator_t(ptr + n);
}

template<class T, bool IsConst>
inline PoolIterator<T, IsConst> PoolIterator<T, IsConst>::operator-(difference_type n) const
{
	return ThisIterator_t(ptr - n);
}

template<class T, bool IsConst>
inline typename PoolIterator<T, IsConst>::difference_type PoolIterator<T, IsConst>::operator-(const ThisIterator_t & rhs) const
{
	return ptr - rhs.ptr;
}

template<class T, bool IsConst>
inline typename PoolIterator<T, IsConst>::reference PoolIterator<T, IsConst>::operator[](size_t i)
{
	return *(ptr + i);
}

template<class T, bool IsConst>
inline bool PoolIterator<T, IsConst>::operator<(const ThisIterator_t & rhs) const
{
	return ptr < rhs.ptr;
}

template<class T, bool IsConst>
inline bool PoolIterator<T, IsConst>::operator<=(const ThisIterator_t & rhs) const
{
	return ptr <= rhs.ptr;
}

template<class T, bool IsConst>
inline bool PoolIterator<T, IsConst>::operator>(const ThisIterator_t & rhs) const
{
	return ptr > rhs.ptr;
}

template<class T, bool IsConst>
inline bool PoolIterator<T, IsConst>::operator>=(const ThisIterator_t & rhs) const
{
	return  ptr >= rhs.ptr;;
}

template<class T, bool IsConst>
inline bool PoolIterator<T, IsConst>::operator==(const ThisIterator_t & rhs) const
{
	return (ptr == rhs.ptr);
}

template<class T, bool IsConst>
inline bool PoolIterator<T, IsConst>::operator!=(const ThisIterator_t & rhs) const
{
	return !(*this == rhs);
}