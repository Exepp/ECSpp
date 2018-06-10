
template<class T>
inline Component& CPool<T>::alloc()
{
	return static_cast<Component&>(pool.alloc());
}

template<class T>
inline Component& CPool<T>::alloc(const Component& arg)
{
	return static_cast<Component&>(pool.alloc(static_cast<const T&>(arg)));
}

template<class T>
inline Component& CPool<T>::alloc(Component&& arg)
{
	return static_cast<Component&>(pool.alloc(std::move(static_cast<T&&>(arg))));
}

template<class T>
inline void CPool<T>::allocN(size_t n)
{
	pool.allocN(n);
}

template<class T>
inline void CPool<T>::free(size_t i)
{
	pool.free(i);
}

template<class T>
inline void CPool<T>::reserve(size_t n)
{
	pool.reserve(n);
}

template<class T>
inline void CPool<T>::clear()
{
	pool.clear();
}

template<class T>
inline std::unique_ptr<CPoolInterface> CPool<T>::makeEmptyCopy() const
{
	return std::make_unique<ThisPool_t>();
}

template<class T>
inline size_t CPool<T>::getSize() const
{
	return pool.getSize();
}

template<class T>
inline size_t CPool<T>::getReserved() const
{
	return pool.getReserved();
}

template<class T>
inline Component& CPool<T>::operator[](size_t i)
{
	return pool[i];
}

template<class T>
inline const Component& CPool<T>::operator[](size_t i) const
{
	return pool[i];
}
