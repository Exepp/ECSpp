#ifndef EPP_POOL_H
#define EPP_POOL_H

#include <ECSpp/external/llvm/SmallVector.h>
#include <ECSpp/internal/utility/Assert.h>
#include <cmath>
#include <memory>
#include <vector>

namespace epp {


inline std::size_t SizeToFitNextN(std::size_t n, std::size_t reserved, std::size_t freeLeft)
{
    return freeLeft >= n ? reserved : llvm::NextPowerOf2(reserved + (n - freeLeft) - 1);
}


/**
 * A Vector that does not maintain order - the last component is always moved in the place of a removed one
 */
template <typename T>
struct Pool {
    using Container_t = std::vector<T>;


    /// Creates a new element with emplace_back
    /**
     * @param arg Arguments forwarded to the emplace_back
     * @returns A reference to the created object
     */
    template <typename... U>
    inline T& create(U&&... arg)
    {
        return data.emplace_back(std::forward<U>(arg)...);
    }


    /// Removes the object located at a given index
    /**
     * Moves the last element in place of the removed one
     * @param idx index of the element to be deleted
     * @returns False if idx was the last element, True otherwise
     */
    inline bool destroy(std::size_t idx)
    {
        EPP_ASSERT(idx < data.size());

        bool notLast = (idx + 1) < data.size();
        if (notLast) {
            data[idx].~T();
            new (&data[idx]) T(std::move(data.back()));
        }
        data.pop_back();
        return notLast;
    }


    /// The next n create(...) calls will not require reallocation
    /** 
     * @param n Number of elements to reserve the additional memory for
     */
    void fitNextN(std::size_t n)
    {
        data.reserve(SizeToFitNextN(n, data.capacity(), data.capacity() - data.size()));
    }


    Container_t data;
};


} // namespace epp

#endif // EPP_POOL_H