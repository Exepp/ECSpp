#include "EntityManager/EntityManager.h"

using namespace epp;

const ERefPtr_t& EntityManager::spawn(const Archetype& arche)
{
	return registerArchetypeIfNew(arche).spawn();
}

ASpawner::ERefPtrPoolIteratorPair_t EntityManager::spawn(const Archetype & arche, size_t n)
{
	return registerArchetypeIfNew(arche).spawn(n);
}

void EntityManager::acceptSpawnedEntities()
{
	for (auto& aSpawnerNode : aSpawners)
		aSpawnerNode.second->acceptSpawningEntities();
}

void EntityManager::clear()
{
	aSpawners.clear();

	for (auto& pArrayPackNode : pArraysPacks)
		pArrayPackNode.second->clear();
	pArraysPacks.clear();
}

void EntityManager::registerArchetype(Archetype arche)
{
	registerArchetypeIfNew(std::move(arche));
}

bool EntityManager::isEntityRefValid(const EntityRef & eRef)
{
	if (eRef.isValid())
	{
		auto found = aSpawners.find(eRef.getOriginSpawner()->getArchetype().hash());
		if (found != aSpawners.end())
			return found->second.get() == eRef.getOriginSpawner();
	}
	return false;
}

const EntityManager::ASpawnersHolder_t & EntityManager::getArchetypesSpawners() const
{
	return aSpawners;
}