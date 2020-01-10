#include <ECSpp/Utility/Bitmask.h>
#include <algorithm>
#include <numeric>

using namespace epp;


Bitmask::Bitmask(IdxList_t list)
{
    set(list);
}

void Bitmask::set(Idx_t bitIndex)
{
    Idx_t index = bitIndex / MaskBits;
    if (index >= masks.size())
        masks.resize(index + 1, Mask_t(0));
    masks[index] |= 1ull << (bitIndex % MaskBits);
}

void Bitmask::set(IdxList_t list)
{
    for (auto index : list)
        set(index);
}

void Bitmask::unset(Idx_t bitIndex)
{
    unsetNoShrink(bitIndex);
    shrinkToFit();
}

void Bitmask::unset(IdxList_t list)
{
    if (masks.empty())
        return;
    for (auto index : list)
        unsetNoShrink(index);
    shrinkToFit();
}

void Bitmask::unsetNoShrink(Idx_t bitIndex)
{
    Idx_t index = bitIndex / MaskBits;
    if (index < masks.size())
        masks[index] &= ~(1ull << (bitIndex % MaskBits));
}

void Bitmask::shrinkToFit()
{
    std::intptr_t idx = std::intptr_t(masks.size());
    while (--idx >= 0 && masks[idx] == 0x0)
        ;
    masks.resize(std::size_t(idx + 1));
}

void Bitmask::clear()
{
    masks.clear();
}

Bitmask& Bitmask::removeCommon(Bitmask const& other)
{
    Idx_t smallest = std::min(masks.size(), other.masks.size());
    for (Idx_t i = 0; i < smallest; ++i)
        masks[i] &= ~other.masks[i];
    shrinkToFit();
    return *this;
}

bool Bitmask::get(Idx_t bitIndex) const
{
    Idx_t index = bitIndex / MaskBits;
    if (index >= masks.size())
        return false;
    return (masks[index] & (1ull << (bitIndex % MaskBits)));
}

Bitmask::MaskContainer_t const& Bitmask::getMasks() const
{
    return masks;
}

std::size_t Bitmask::getSetCount() const
{
    std::size_t sum = 0;
    Bitset_t counter;

    for (auto mask : masks)
        sum += (counter = mask).count();
    return sum;
}

bool Bitmask::contains(Bitmask const& other) const
{
    if (other.masks.size() > masks.size())
        return false;
    auto maskIt = masks.begin();
    return std::all_of(other.masks.begin(), other.masks.end(),
                       [&maskIt](Mask_t mask) { return (*maskIt++ & mask) == mask; });
}

bool Bitmask::hasCommon(Bitmask const& other) const
{
    auto maskIt = masks.begin();
    return std::any_of(other.masks.begin(), other.masks.begin() + std::min(masks.size(), other.masks.size()),
                       [&maskIt](Mask_t mask) { return *maskIt++ & mask; });
}

std::size_t Bitmask::numberOfCommon(Bitmask const& other) const
{
    Idx_t smallest = std::min(masks.size(), other.masks.size());
    std::size_t sum = 0;
    Bitset_t counter;

    for (Idx_t i = 0; i < smallest; ++i)
        sum += (counter = (masks[i] & other.masks[i])).count();
    return sum;
}

Bitmask Bitmask::operator&(Bitmask const& rhs) const
{
    return Bitmask(*this) &= rhs;
}

Bitmask Bitmask::operator|(Bitmask const& rhs) const
{
    return Bitmask(*this) |= rhs;
}

Bitmask& Bitmask::operator&=(Bitmask const& rhs)
{
    std::size_t smallest = std::min(masks.size(), rhs.masks.size());
    masks.resize(smallest);

    for (Idx_t i = 0; i < smallest; ++i)
        masks[i] &= rhs.masks[i];
    shrinkToFit();
    return *this;
}

Bitmask& Bitmask::operator|=(Bitmask const& rhs)
{
    if (masks.size() < rhs.masks.size())
        masks.resize(rhs.masks.size(), Mask_t(0));

    for (Idx_t i = 0; i < rhs.masks.size(); ++i)
        masks[i] |= rhs.masks[i];
    return *this;
}

bool Bitmask::operator==(Bitmask const& rhs) const
{
    return masks == rhs.masks;
}

bool Bitmask::operator!=(Bitmask const& rhs) const
{
    return !(*this == rhs);
}

bool Bitmask::operator<(Bitmask const& rhs) const
{
    if (masks.size() == rhs.masks.size()) {
        std::intptr_t i = std::intptr_t(masks.size());
        while (--i >= 0)
            if (masks[i] != rhs.masks[i])
                return masks[i] < rhs.masks[i];
        return false; // equal
    }
    return masks.size() < rhs.masks.size();
}

bool Bitmask::operator>(Bitmask const& rhs) const
{
    if (masks.size() == rhs.masks.size()) {
        std::intptr_t i = std::intptr_t(masks.size());
        while (--i >= 0)
            if (masks[i] != rhs.masks[i])
                return masks[i] > rhs.masks[i];
        return false; // equal
    }
    return masks.size() > rhs.masks.size();
}