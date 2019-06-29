#ifndef BITMASK_H
#define BITMASK_H

#include <cstdint>
#include <vector>

namespace epp
{

class Bitmask
{
    using Mask_t = uint64_t;

    using MaskContainer_t = std::vector<Mask_t>;

public:
    Bitmask() = default;

    Bitmask(std::initializer_list<size_t> list);


    Bitmask& set(size_t bitIndex);

    Bitmask& unset(size_t bitIndex);

    bool get(size_t bitIndex) const;


    void clear();

    Bitmask& flip();

    Bitmask flipped() const;


    size_t getSetCount() const;

    bool hasCommon(const Bitmask& other) const;

    size_t numberOfCommon(const Bitmask& other) const;


    size_t hash() const;


    Bitmask operator&(const Bitmask& rhs) const;

    Bitmask operator|(const Bitmask& rhs) const;


    Bitmask& operator&=(const Bitmask& rhs);

    Bitmask& operator|=(const Bitmask& rhs);


    bool operator==(const Bitmask& rhs) const;

    bool operator!=(const Bitmask& rhs) const;


private:
    static const uint8_t maskBits = sizeof(Mask_t) * 8u;

private:
    MaskContainer_t masks;

    size_t setCount = 0;

    mutable bool rehash = true;

    mutable size_t hashValue = 0;


    friend struct std::hash<Bitmask>;
};

} // namespace epp

namespace std
{
template<>
struct hash<epp::Bitmask>
{
    size_t operator()(const epp::Bitmask& bitmask) const
    {
        size_t val = 0;
        for (size_t i : bitmask.masks)
            // equation from boost library: hash_combine function
            val ^= std::hash<size_t>()(i) + 0x9e3779b9 + (val << 6) + (val >> 2);
        return val;
    }
};
} // namespace std

#endif // BITMASK_H;