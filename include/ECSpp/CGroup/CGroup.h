#pragma once
#include <ECSpp/Utility/CFilter.h>
#include <ECSpp/CGroup/ASpawnersPack.h>

namespace epp
{

template<class ...CTypes>
class CGroup
{
	using SpawnersPack_t = ASpawnersPack<CTypes...>;

	using SpawnersPackPtr_t = std::shared_ptr<SpawnersPack_t>;

public:

	using Iterator_t = typename SpawnersPack_t::Iterator_t;

	using ConstIterator_t = typename SpawnersPack_t::ConstIterator_t;

public:

	CGroup() = default;

	// it is up to caller to provide ptr to ASpawnersPack of a correct type (SpawnersPack_t)
	CGroup(const std::shared_ptr<ASpawnersPackInterace>& arraysPtr);


	const CFilter& getFilter() const;


	Iterator_t begin();

	ConstIterator_t begin() const;

	Iterator_t end();

	ConstIterator_t end() const;

private:

	SpawnersPackPtr_t spawnersPtr;
};




template<class ...CTypes>
inline CGroup<CTypes...>::CGroup(const std::shared_ptr<ASpawnersPackInterace>& arraysPtr) : spawnersPtr(std::static_pointer_cast<SpawnersPack_t>(arraysPtr)) 
{}

template<class ...CTypes>
inline const CFilter & CGroup<CTypes...>::getFilter() const
{
	return spawnersPtr->getFilter();
}

template<class ...CTypes>
inline typename CGroup<CTypes...>::Iterator_t CGroup<CTypes...>::begin()
{
	return spawnersPtr->begin();
}

template<class ...CTypes>
inline typename CGroup<CTypes...>::ConstIterator_t CGroup<CTypes...>::begin() const
{
	return static_cast<const SpawnersPack_t&>(*spawnersPtr).begin();
}

template<class ...CTypes>
inline typename CGroup<CTypes...>::Iterator_t CGroup<CTypes...>::end()
{
	return spawnersPtr->end();
}

template<class ...CTypes>
inline typename CGroup<CTypes...>::ConstIterator_t CGroup<CTypes...>::end() const
{
	return static_cast<const SpawnersPack_t&>(*spawnersPtr).end();
}

}