#include <ECSpp/Utility/BitFilter.h>

using namespace epp;

BitFilter::BitFilter(Bitmask wanted, Bitmask unwanted)
    : wantedMask(std::move(wanted))
    , unwantedMask(std::move(unwanted.removeCommon(wantedMask)))
{}

void BitFilter::clear()
{
    wantedMask.clear();
    unwantedMask.clear();
}

void BitFilter::setWanted(Bitmask wanted)
{
    wantedMask = std::move(wanted);
    unwantedMask.removeCommon(wantedMask);
}

BitFilter& epp::BitFilter::addWanted(Idx_t idx)
{
    wantedMask.set(idx);
    unwantedMask.unset(idx);
    return *this;
}

BitFilter& BitFilter::addWanted(IdxList_t idxList)
{
    wantedMask.set(idxList);
    unwantedMask.removeCommon(wantedMask);
    return *this;
}

BitFilter& epp::BitFilter::removeWanted(Idx_t idx)
{
    wantedMask.unset(idx);
    return *this;
}

BitFilter& BitFilter::removeWanted(IdxList_t idxList)
{
    wantedMask.unset(idxList);
    return *this;
}

void BitFilter::setUnwanted(Bitmask unwanted)
{
    unwantedMask = std::move(unwanted);
    wantedMask.removeCommon(unwantedMask);
}

BitFilter& epp::BitFilter::addUnwanted(Idx_t idx)
{
    unwantedMask.set(idx);
    wantedMask.unset(idx);
    return *this;
}

BitFilter& BitFilter::addUnwanted(IdxList_t idxList)
{
    unwantedMask.set(idxList);
    wantedMask.removeCommon(unwantedMask);
    return *this;
}

BitFilter& epp::BitFilter::removeUnwanted(Idx_t idx)
{
    unwantedMask.unset(idx);
    return *this;
}

BitFilter& BitFilter::removeUnwanted(IdxList_t idxList)
{
    unwantedMask.unset(idxList);
    return *this;
}

Bitmask& BitFilter::getWanted()
{
    return wantedMask;
}

const Bitmask& BitFilter::getWanted() const
{
    return wantedMask;
}

Bitmask& BitFilter::getUnwanted()
{
    return unwantedMask;
}

const Bitmask& BitFilter::getUnwanted() const
{
    return unwantedMask;
}

bool BitFilter::operator==(const BitFilter& rhs) const
{
    return (wantedMask == rhs.wantedMask && unwantedMask == rhs.unwantedMask);
}

bool BitFilter::operator!=(const BitFilter& rhs) const
{
    return !(*this == rhs);
}

bool BitFilter::operator&(Bitmask const& rhs) const
{
    return rhs.contains(wantedMask) && !rhs.hasCommon(unwantedMask);
}