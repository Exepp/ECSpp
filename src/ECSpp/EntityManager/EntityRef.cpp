#include <ECSpp/EntityManager/EntityRef.h>
#include <ECSpp/EntityManager/ASpawner.h>

using namespace epp;

Entity::Entity(ASpawner * originSpawner, EntityId_t id) : originSpawner(originSpawner), id(id)
{}

void epp::Entity::invalidate()
{
	originSpawner = nullptr;
	id = unidentifiedId;
	alive = false;
}



epp::EntityRef::EntityRef(const EPtr_t & entity) : entity(entity)
{}

void EntityRef::die()
{
	if (isValid())
		entity->originSpawner->kill(*this);
}

bool EntityRef::isValid() const
{
	return entity && entity->originSpawner;
}

bool EntityRef::isAlive() const
{
	return isValid() && entity->alive;
}

bool EntityRef::hasComponent(CTypeId_t id) const
{
	return isValid() && hasComponent_noCheck(id);
}

bool EntityRef::hasComponent_noCheck(CTypeId_t id) const
{
	return entity->originSpawner->getArchetype().hasComponent(id);;
}

Component* EntityRef::getComponent(size_t cTypeId)
{
	if (hasComponent(cTypeId))
		return getComponent_noCheck(cTypeId);
	return nullptr;
}

const Component * EntityRef::getComponent(size_t cTypeId) const
{
	if (hasComponent(cTypeId))
		return getComponent_noCheck(cTypeId);
	return nullptr;
}

Component * EntityRef::getComponent_noCheck(size_t cTypeId)
{
	return &(*entity->originSpawner).getPool(cTypeId, entity->alive)[entity->id];
}

const Component * EntityRef::getComponent_noCheck(size_t cTypeId) const
{
	return &(*entity->originSpawner).getPool(cTypeId, entity->alive)[entity->id];
}

ASpawner const* EntityRef::getOriginSpawner() const
{
	if(entity)
		return entity->originSpawner;
	return nullptr;
}

bool EntityRef::operator==(const EntityRef& rhs) const
{
	return entity == rhs.entity;
}

bool EntityRef::operator!=(const EntityRef& rhs) const
{
	return !(*this == rhs);
}
