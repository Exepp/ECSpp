#ifndef INDEXTYPE_H
#define INDEXTYPE_H
#include <cstdint>

namespace epp {
template <int n>
struct IndexType {
    using Val_t = std::uint32_t;
    using This_t = IndexType<n>;

    constexpr static Val_t BadValue = Val_t(-1);

    IndexType() = default;
    explicit IndexType(Val_t val) : value(val) {}
    template <typename UIntType>
    explicit IndexType(UIntType val) : value(Val_t(val)) { EPP_ASSERT(val <= BadValue); }
    bool operator==(This_t const& rhs) const { return value == rhs.value; }
    bool operator!=(This_t const& rhs) const { return value != rhs.value; }
    bool operator<(This_t const& rhs) const { return value < rhs.value; }
    bool operator>(This_t const& rhs) const { return value > rhs.value; }
    bool operator<=(This_t const& rhs) const { return value <= rhs.value; }
    bool operator>=(This_t const& rhs) const { return value >= rhs.value; }

    Val_t value = BadValue;
};
} // namespace epp

#endif // INDEXTYPE_H