#ifndef POOL_H
#define POOL_H

#include <ECSpp/Utility/Assert.h>
#include <cmath>
#include <memory>
#include <vector>

namespace epp {


inline std::uint32_t SizeToFitNextN(std::uint32_t n, std::uint32_t reserved, std::uint32_t freeLeft)
{
    return freeLeft >= n ? reserved : (std::uint32_t(0x1) << std::uint8_t(std::ceil(std::log2(reserved + (n - freeLeft)))));
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

    void fitNextN(std::uint32_t n)
    {
        data.reserve(SizeToFitNextN(n, std::uint32_t(data.capacity()), std::uint32_t(data.capacity() - data.size())));
    }


    Container_t data;
};


} // namespace epp

#endif // POOL_H