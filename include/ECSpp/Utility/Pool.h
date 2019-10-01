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

    // returns true if deleted object was relocated by last entity
    bool free(std::size_t i);

    void prepareToFitNMore(std::size_t n);


    std::vector<T> content;
};

#include <ECSpp/Utility/Pool.inl>

} // namespace epp

#endif // POOL_H