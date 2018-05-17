#include "CFilterGroup.h"
#include <unordered_set>

#include <iostream>
struct FilterHasher
{
	size_t operator()(const CFilterGroup& obj) const
	{
		std::cout << "hash" << std::endl;
		return obj.wantedMask.hash() - obj.unwantedMask.hash() % 0xFFFF;
	}
};

struct FilterEqual
{
	bool operator()(const CFilterGroup& lhs, const CFilterGroup& rhs) const
	{
		return lhs.sameMasks(rhs);
	}
};


size_t CFilterGroup::cTypeCounter = 0;


bool CFilterGroup::sameMasks(const CFilterGroup & rhs) const
{
	return (wantedMask == rhs.wantedMask && unwantedMask == rhs.unwantedMask);
}

size_t CFilterGroup::getFilterId() const
{
	static std::unordered_multiset<CFilterGroup, FilterHasher, FilterEqual> uniqueFiltersSet;

	if (id == unidentifiedFilterId)
	{
		std::cout << "start for" << std::endl;
		auto range = uniqueFiltersSet.equal_range(*this);
		for (auto i = range.first; i != range.second; ++i)
			if (sameMasks(*i))
				return (id = i->id);
		std::cout << "end for" << std::endl;
		id = uniqueFiltersSet.size();
		uniqueFiltersSet.emplace(*this);
	}
	return id;
}

bool CFilterGroup::operator==(const CFilterGroup & rhs)
{
	return getFilterId() == rhs.getFilterId();
}

bool CFilterGroup::operator!=(const CFilterGroup & rhs)
{
	return !(*this == rhs);
}

