#include "EntityManager/Archetype.h"

using namespace epp;

Archetype::Archetype(const Archetype & rhs)
{
	*this = rhs;
}

Archetype & Archetype::operator=(const Archetype & rhs)
{
	cMask = rhs.cMask;
	cPools.clear();
	for (auto& pool : rhs.cPools)
		cPools.emplace(std::make_pair(pool.first, pool.second->makeEmptyCopy()));
	rehash = rhs.rehash;
	hashValue = rhs.hashValue;

	return *this;
}

bool Archetype::removeComponent(CTypeId_t id)
{
	if (hasComponent(id))
	{
		cMask.unset(id);
		cPools.erase(id);
		rehash = true;
		return true;
	}
	return false;
}

void Archetype::reset()
{
	cPools.clear();
	cMask.clear();
	rehash = true;
}

bool Archetype::meetsRequirementsOf(const CFilter & filter) const
{
	return filter.getWantedCMask().numberOfCommon(cMask) == filter.getWantedCMask().getSetCount() && !filter.getUnwantedCMask().hasCommon(cMask);
}

const Bitmask& Archetype::getCMask() const
{
	return cMask;
}

size_t Archetype::hash() const
{
	if (rehash)
	{
		rehash = false;
		hashValue = std::hash<Archetype>()(*this);
	}
	return hashValue;
}

void Archetype::clear()
{
	for (auto& pool : cPools)
		pool.second->clear();
}
