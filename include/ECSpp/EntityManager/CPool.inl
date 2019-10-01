template<class T>
inline Component& CPool<T>::alloc()
{
    return static_cast<Component&>(pool.alloc());
}

template<class T>
Component& CPool<T>::alloc(Component&& arg)
{
    return static_cast<Component&>(pool.alloc(static_cast<T&&>(arg)));
}

template<class T>
inline void CPool<T>::free(std::size_t i)
{
    pool.free(i);
}

template<class T>
inline void CPool<T>::prepareToFitNMore(std::size_t n)
{
    pool.prepareToFitNMore(n);
}

template<class T>
inline void CPool<T>::clear()
{
    pool.clear();
}

template<class T>
inline Component& CPool<T>::operator[](std::size_t i)
{
    return pool.content[i];
}