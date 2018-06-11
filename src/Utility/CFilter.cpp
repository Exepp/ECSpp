#include "Utility/CFilter.h"

using namespace epp;

CFilter::CFilter(Bitmask wanted, Bitmask unwated) : unwantedMask(std::move(unwated))
{
	setWanted(std::move(wanted));
}

void CFilter::clear()
{
	unwantedMask.clear();
	wantedMask.clear();
	rehash = true;
}

void CFilter::setWanted(Bitmask wantedMask)
{
	this->wantedMask = std::move(wantedMask);
	if (this->wantedMask.hasCommon(unwantedMask))
		unwantedMask &= this->wantedMask.flipped();
	rehash = true;
}

void CFilter::setUnwanted(Bitmask unwantedMask)
{
	this->unwantedMask = std::move(unwantedMask);
	if (this->unwantedMask.hasCommon(wantedMask))
		wantedMask &= this->unwantedMask.flipped();
	rehash = true;
}

Bitmask & CFilter::getWantedCMask()
{
	return wantedMask;
}

const Bitmask & CFilter::getWantedCMask() const
{
	return wantedMask;
}

Bitmask & CFilter::getUnwantedCMask()
{
	return unwantedMask;
}

const Bitmask & CFilter::getUnwantedCMask() const
{
	return unwantedMask;
}

size_t CFilter::hash() const
{
	if (rehash)
	{
		rehash = false;
		hashValue = std::hash<CFilter>()(*this);
	}
	return hashValue;
}

bool CFilter::operator==(const CFilter & rhs) const
{
	return  (wantedMask == rhs.wantedMask && unwantedMask == rhs.unwantedMask);
}

bool CFilter::operator!=(const CFilter & rhs) const
{
	return !(*this == rhs);
}