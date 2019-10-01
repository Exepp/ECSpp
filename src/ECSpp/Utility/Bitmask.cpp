#include <ECSpp/Utility/Bitmask.h>
#include <algorithm>

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
    std::intptr_t idx = masks.size() - 1;
    while (masks[idx] == 0x0 && --idx >= 0)
        ;
    masks.resize(idx + 1);
}

void Bitmask::clear()
{
    masks.clear();
}

Bitmask& Bitmask::removeCommon(Bitmask const& other)
{
    std::size_t smallest = std::min(masks.size(), other.masks.size());

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
    Bitset_t    counter;

    for (auto mask : masks)
        sum += (counter = mask).count();
    return sum;
}

bool Bitmask::contains(Bitmask const& other) const
{
    if (other.masks.size() > masks.size())
        return false;
    for (Idx_t i = 0; i < other.masks.size(); ++i)
        if ((masks[i] & other.masks[i]) != other.masks[i])
            return false;
    return true;
}

bool Bitmask::hasCommon(Bitmask const& other) const
{
    std::size_t smallest = std::min(masks.size(), other.masks.size());
    for (Idx_t i = 0; i < smallest; ++i)
        if (masks[i] & other.masks[i])
            return true;
    return false;
}

std::size_t Bitmask::numberOfCommon(Bitmask const& other) const
{
    std::size_t smallest = std::min(masks.size(), other.masks.size());
    std::size_t sum      = 0;
    Bitset_t    counter;

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
    if (masks.size() == rhs.masks.size())
    {
        for (std::intptr_t i = masks.size() - 1; i >= 0; --i)
            if (masks[i] != rhs.masks[i])
                return masks[i] < rhs.masks[i];
        return false; // equal
    }
    return masks.size() < rhs.masks.size();
}

bool Bitmask::operator>(Bitmask const& rhs) const
{
    if (masks.size() == rhs.masks.size())
    {
        for (std::intptr_t i = masks.size() - 1; i >= 0; --i)
            if (masks[i] != rhs.masks[i])
                return masks[i] > rhs.masks[i];
        return false; // equal
    }
    return masks.size() > rhs.masks.size();
}