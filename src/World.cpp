#include "World.h"
#include <algorithm>

bool World::removeSystem(STypeId_t id)
{
	auto found = std::find_if(systems.begin(), systems.end(), SystemUnaryPredicate(id));
	ASSERT((found != systems.end()), "Tried to remove unidentified system")
	if (found != systems.end())
	{
		systems.erase(found);
		return true;
	}
	return false;
}

bool World::hasSystem(STypeId_t id)
{
	auto found = std::find_if(systems.begin(), systems.end(), SystemUnaryPredicate(id));
	return found != systems.end();
}

void World::update(float dt)
{
	entityManager.acceptSpawnedEntities();
	for (auto& systemNode : systems)
	{
		systemNode.second->update(entityManager, dt);
		entityManager.acceptSpawnedEntities();
	}
}
