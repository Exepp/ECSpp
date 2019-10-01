
template<class T>
template<class... U>
inline T& Pool<T>::alloc(U&&... arg)
{
    return content.emplace_back(std::forward<U>(arg)...);
}

template<class T>
bool Pool<T>::free(std::size_t i)
{
    assert(!content.empty());

    bool replaced = i + 1 < content.size();
    if (replaced)
        std::swap(content.back(), content[i]);
    content.pop_back();
    return replaced;
}

template<class T>
void Pool<T>::prepareToFitNMore(std::size_t n)
{
    std::size_t freeLeft = content.capacity() - content.size();
    if (freeLeft < n)
        content.reserve(std::size_t((content.capacity() + (n - freeLeft)) * 1.61803398875));
}