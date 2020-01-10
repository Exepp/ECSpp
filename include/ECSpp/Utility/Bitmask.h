#ifndef BITMASK_H
#define BITMASK_H

#include <bitset>
#include <cstdint>
#include <vector>

namespace epp {

class Bitmask {
public:
    using Idx_t = std::size_t;

    using IdxList_t = std::initializer_list<Idx_t>;

    using Mask_t = uint64_t;

private:
    using MaskContainer_t = std::vector<Mask_t>;

    using Bitset_t = std::bitset<sizeof(Mask_t) * 8u>;

public:
    Bitmask() = default;

    explicit Bitmask(IdxList_t list);

    // returns true if changed
    void set(Idx_t bitIndex);

    // returns true if changed every index from list
    void set(IdxList_t list);

    // returns true if changed
    void unset(Idx_t bitIndex);

    // returns true if changed every index from list
    void unset(IdxList_t list);

    void clear();


    Bitmask& removeCommon(Bitmask const& other);

    bool hasCommon(Bitmask const& other) const;

    std::size_t numberOfCommon(Bitmask const& other) const;

    bool contains(Bitmask const& other) const;


    bool get(Idx_t bitIndex) const;

    std::size_t getSetCount() const;

    MaskContainer_t const& getMasks() const;


    Bitmask& operator&=(Bitmask const& rhs);

    Bitmask& operator|=(Bitmask const& rhs);

    Bitmask operator&(Bitmask const& rhs) const;

    Bitmask operator|(Bitmask const& rhs) const;

    bool operator==(Bitmask const& rhs) const;

    bool operator!=(Bitmask const& rhs) const;

    // treating Bitmask as a number
    bool operator<(Bitmask const& rhs) const;

    // treating Bitmask as a number
    bool operator>(Bitmask const& rhs) const;

private:
    void unsetNoShrink(Idx_t bitIndex);

    void shrinkToFit();

private:
    static std::size_t const MaskBits = sizeof(Mask_t) * 8u;

private:
    MaskContainer_t masks;


    friend struct std::hash<epp::Bitmask>;
};


} // namespace epp

namespace std {
template <>
struct hash<epp::Bitmask> {
    std::size_t operator()(epp::Bitmask const& bitmask) const
    {
        std::size_t val = 0;
        for (auto mask : bitmask.masks)
            // equation from boost library: hash_combine function
            val ^= std::hash<epp::Bitmask::Mask_t>()(mask) + 0x9e3779b97f4a7c15 + (val << 6) + (val >> 2);
        return val;
    }
};
} // namespace std

#endif // BITMASK_H;