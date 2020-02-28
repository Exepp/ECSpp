#ifndef EPP_INDEXTYPE_H
#define EPP_INDEXTYPE_H

#include <ECSpp/internal/utility/Assert.h>
#include <cstdint>

namespace epp {
template <int n, typename ValueT = std::uint32_t>
struct IndexType {
    using Val_t = ValueT;

    constexpr static Val_t BadValue = Val_t(-1);

    IndexType() = default;
    explicit IndexType(Val_t val) : value(val) {}
    template <typename UIntType>
    explicit IndexType(UIntType val) : value(Val_t(val)) { EPP_ASSERT(val <= BadValue); }
    bool operator==(IndexType const& rhs) const { return value == rhs.value; }
    bool operator!=(IndexType const& rhs) const { return value != rhs.value; }
    bool operator<(IndexType const& rhs) const { return value < rhs.value; }
    bool operator>(IndexType const& rhs) const { return value > rhs.value; }
    bool operator<=(IndexType const& rhs) const { return value <= rhs.value; }
    bool operator>=(IndexType const& rhs) const { return value >= rhs.value; }

    Val_t value = BadValue;
};
} // namespace epp

#endif // EPP_INDEXTYPE_H