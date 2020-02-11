#ifndef EPP_POOL_H
#define EPP_POOL_H

#include <ECSpp/external/llvm/SmallVector.h>
#include <ECSpp/utility/Assert.h>
#include <cmath>
#include <memory>
#include <vector>

namespace epp {


inline std::size_t SizeToFitNextN(std::size_t n, std::size_t reserved, std::size_t freeLeft)
{
    return freeLeft >= n ? reserved : llvm::NextPowerOf2(reserved + (n - freeLeft) - 1);
}


template <typename T>
struct Pool {
    using Container_t = std::vector<T>;

    template <typename... U>
    inline T& create(U&&... arg)
    {
        return data.emplace_back(std::forward<U>(arg)...);
    }

    inline bool destroy(std::size_t i)
    {
        EPP_ASSERT(i < data.size());

        bool notLast = (i + 1) < data.size();
        if (notLast) {
            data[i].~T();
            new (&data[i]) T(std::move(data.back()));
        }
        data.pop_back();
        return notLast;
    }

    void fitNextN(std::size_t n)
    {
        data.reserve(SizeToFitNextN(n, data.capacity(), data.capacity() - data.size()));
    }


    Container_t data;
};


} // namespace epp

#endif // EPP_POOL_H