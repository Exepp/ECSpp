#ifndef POOL_H
#define POOL_H

#include <assert.h>
#include <memory>
#include <vector>

namespace epp
{

template<class T>
struct Pool
{
    template<class... U>
    T& alloc(U&&... arg);

    // returns true if deleted object was replaced with last element (false only for last element)
    bool free(std::size_t i);

    void prepareToFitNMore(std::size_t n);


    std::vector<T> content;
};


template<class T>
template<class... U>
inline T& Pool<T>::alloc(U&&... arg)
{
    return content.emplace_back(std::forward<U>(arg)...);
}

template<class T>
bool Pool<T>::free(std::size_t i)
{
    assert(i < content.size());

    bool isLast = (i + 1) == content.size();
    if (!isLast)
        std::swap(content.back(), content[i]);
    content.pop_back();
    return isLast;
}

template<class T>
void Pool<T>::prepareToFitNMore(std::size_t n)
{
    std::size_t freeLeft = content.capacity() - content.size();
    if (freeLeft < n)
        content.reserve(std::size_t((content.capacity() + (n - freeLeft)) * 1.61803398875));
}

} // namespace epp

#endif // POOL_H