#include <ECSpp/Utility/BitFilter.h>

using namespace epp;

BitFilter::BitFilter(Bitmask wanted, Bitmask unwanted)
    : wantedMask(wanted)
    , unwantedMask(std::move(unwanted.removeCommon(wantedMask)))
{}

void BitFilter::clear()
{
    unwantedMask.clear();
    wantedMask.clear();
}

void BitFilter::setWanted(Bitmask wanted)
{
    wantedMask = std::move(wanted);
    unwantedMask.removeCommon(wantedMask);
}

BitFilter& BitFilter::addWanted(IdxList_t idxList)
{
    wantedMask.set(idxList);
    unwantedMask.removeCommon(wantedMask);
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

BitFilter& BitFilter::addUnwanted(IdxList_t idxList)
{
    unwantedMask.set(idxList);
    wantedMask.removeCommon(wantedMask);
    return *this;
}

BitFilter& BitFilter::removeUnwanted(IdxList_t idxList)
{
    unwantedMask.unset(idxList);
    return *this;
}

Bitmask& BitFilter::getWantedMask()
{
    return wantedMask;
}

const Bitmask& BitFilter::getWantedMask() const
{
    return wantedMask;
}

Bitmask& BitFilter::getUnwantedMask()
{
    return unwantedMask;
}

const Bitmask& BitFilter::getUnwantedMask() const
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