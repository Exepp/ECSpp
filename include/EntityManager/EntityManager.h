#pragma once
#include "ASpawner.h"
#include "CGroup/CGroup.h"

namespace epp
{

class EntityManager
{
	using ASpawnerPtr_t = std::unique_ptr<ASpawner>;

	using ASpawnersHolder_t = spp::sparse_hash_map<size_t, ASpawnerPtr_t>;


	using ASpawnersPackPtr_t = std::shared_ptr<ASpawnersPackInterace>;

	using ASpawnersPacksHolder_t = spp::sparse_hash_map<size_t, ASpawnersPackPtr_t>;

public:

	template<class ...CTypes>
	const ERefPtr_t& spawn();

	template<class ...CTypes>
	const ERefPtr_t& spawn(size_t n);

	const ERefPtr_t& spawn(const Archetype& arche);

	ASpawner::ERefPtrPoolIteratorPair_t spawn(const Archetype& arche, size_t n);

	
	// makes it possible for a groups to iterate over recently spawned entities (should be called outside of any group iteration loop)
	void acceptSpawnedEntities();


	// kills all entities
	void clear();


	// adding (new)component will always make entity unreachable for any group's iterator, by the time EntityManager::acceptSpawnedEntities is called
	template<class ...CTypes>
	void addComponent(EntityRef& eRef);

	// removing (present)component will always make entity unreachable for any group's iterator, by the time EntityManager::acceptSpawnedEntities is called
	template<class ...CTypes>
	void removeComponent(EntityRef& eRef);

	template<class ...IdTypes>
	void removeComponent(EntityRef& eRef, IdTypes ...ids);


	// registers archetype, so it doesn't need to be done on spawn/(add/remove component) calls
	void registerArchetype(Archetype arche);

	const ASpawnersHolder_t& getArchetypesSpawners() const;


	// returns a group of entities, that meets the given requirements
	template<class ...CTypes>
	CGroup<CTypes...> requestCGroup(Bitmask unwantedComponents = Bitmask());

private:

	bool isEntityRefValid(const EntityRef& eRef);

	template<class AType>
	ASpawner& registerArchetypeIfNew(AType&& arche);

private:

	ASpawnersHolder_t aSpawners;

	ASpawnersPacksHolder_t pArraysPacks;
};




template<class ...CTypes>
inline const ERefPtr_t & EntityManager::spawn()
{
	spawn(makeArchetype<CTypes...>());
}

template<class ...CTypes>
inline const ERefPtr_t & EntityManager::spawn(size_t n)
{
	spawn(makeArchetype<CTypes...>(), n);
}

template<class ...CTypes>
inline void EntityManager::addComponent(EntityRef& eRef)
{
	if (!isEntityRefValid(eRef) || (eRef.hasComponent_noCheck<CTypes>() && ...))
		return;

	Archetype newArchetype = eRef.getOriginSpawner()->getArchetype();
	newArchetype.addComponent<CTypes...>();
	registerArchetypeIfNew(std::move(newArchetype)).moveExistingEntityHere(eRef);
}

template<class ...CTypes>
inline void EntityManager::removeComponent(EntityRef & eRef)
{
	removeComponent(eRef, getCTypeId<CTypes>()...);
}

template<class ...IdTypes>
inline void EntityManager::removeComponent(EntityRef & eRef, IdTypes ...ids)
{
	if (!isEntityRefValid(eRef) || !(eRef.hasComponent_noCheck(ids) || ...))
		return;

	Archetype newArchetype = eRef.getOriginSpawner()->getArchetype();
	newArchetype.removeComponent(ids...);
	registerArchetypeIfNew(std::move(newArchetype)).moveExistingEntityHere(eRef);
}

template<class ...CTypes>
inline CGroup<CTypes...> EntityManager::requestCGroup(Bitmask unwantedComponents)
{
	CFilter filter;
	filter.setUnwanted(std::move(unwantedComponents));
	filter.addWanted<CTypes...>();

	ASpawnersPackPtr_t& cArraysPackPtr = pArraysPacks[filter.hash()];
	if (!cArraysPackPtr)
	{
		cArraysPackPtr = std::make_shared<ASpawnersPack<CTypes...>>(std::move(filter));

		for (auto& aSpawner : aSpawners)
			cArraysPackPtr->addASpawnerIfMeetsRequirements(*aSpawner.second);
	}
	return CGroup<CTypes...>(cArraysPackPtr);
}

template<class AType>
inline ASpawner & EntityManager::registerArchetypeIfNew(AType && arche)
{
	ASpawnerPtr_t& aSpawnerPtr = aSpawners[arche.hash()];
	if (aSpawnerPtr)
		return *aSpawnerPtr;

	aSpawnerPtr = std::make_unique<ASpawner>(std::forward<AType>(arche));

	for (auto& pArrayPack : pArraysPacks)
		pArrayPack.second->addASpawnerIfMeetsRequirements(*aSpawnerPtr);
	return *aSpawnerPtr;
}

}