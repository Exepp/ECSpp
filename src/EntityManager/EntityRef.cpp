#include "EntityManager/EntityRef.h"
#include "EntityManager/ASpawner.h"

using namespace epp;

EntityRef::EntityRef(ASpawner * spawnerPtr, EntityId_t id, bool living) : originSpawner(spawnerPtr), id(id), alive(living) {}

void EntityRef::die()
{
	if (isValid())
		originSpawner->kill(*this);
}

bool EntityRef::isValid() const
{
	return originSpawner;
}

bool EntityRef::isAlive() const
{
	return alive;
}

bool EntityRef::hasComponent(CTypeId_t id) const
{
	return isValid() && hasComponent_noCheck(id);
}

bool EntityRef::hasComponent_noCheck(CTypeId_t id) const
{
	return originSpawner->getArchetype().hasComponent(id);;
}

Component* EntityRef::getComponent(size_t cTypeId)
{
	if (hasComponent(cTypeId))
		return getComponent_NoCheck(cTypeId);
	return nullptr;
}

const Component * EntityRef::getComponent(size_t cTypeId) const
{
	if (hasComponent(cTypeId))
		return getComponent_NoCheck(cTypeId);
	return nullptr;
}

Component * EntityRef::getComponent_NoCheck(size_t cTypeId)
{
	return &(*originSpawner).getComponent(cTypeId, id, alive);
}

const Component * EntityRef::getComponent_NoCheck(size_t cTypeId) const
{
	return &(*originSpawner).getComponent(cTypeId, id, alive);
}

ASpawner const* EntityRef::getOriginSpawner() const
{
	return originSpawner;
}

void EntityRef::invalidate()
{
	originSpawner = nullptr;
	id = unidentifiedId;
	alive = false;
}

bool EntityRef::operator==(const EntityRef& rhs) const
{
	return originSpawner == rhs.originSpawner && id == rhs.id;
}

bool EntityRef::operator!=(const EntityRef& rhs) const
{
	return !(*this == rhs);
}