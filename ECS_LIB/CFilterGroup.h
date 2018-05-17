#pragma once
#include "Component.h"
#include "TuplePlus.h"

#include "Bitmask.h"

class CFilterGroup
{
	friend struct FilterHasher;
	friend struct FilterEqual;

public:

	CFilterGroup() = default;


	template<class ...CTypes>
	void addWanted();

	template<class ...CTypes>
	void addUnwanted();


	size_t getFilterId() const;

	bool operator==(const CFilterGroup& rhs);

	bool operator!=(const CFilterGroup& rhs);

private:

	bool sameMasks(const CFilterGroup& rhs) const;


	template<class T>
	static size_t getCTypeId();


private:

	static size_t cTypeCounter;

	static const size_t unidentifiedFilterId = size_t(-1);

private:

	Bitmask wantedMask;

	Bitmask unwantedMask;

	mutable size_t id = unidentifiedFilterId;

};




template<class ...CTypes>
inline void CFilterGroup::addWanted()
{
	(wantedMask.set(getCTypeId<CTypes>()), ...);
}


template<class ...CTypes>
inline void CFilterGroup::addUnwanted()
{
	(unwantedMask.set(getCTypeId<CTypes>()), ...);
}


template<class T>
inline size_t CFilterGroup::getCTypeId()
{
	static_assert(std::is_base_of_v<Component, T>, "Filters supports only Component based types");

	static const size_t cTypeId = cTypeCounter++;

	return cTypeId;
}

