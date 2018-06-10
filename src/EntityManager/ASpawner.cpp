#include "EntityManager/ASpawner.h"

ASpawner::ASpawner(Archetype archetype) : aliveEntities(archetype), spawningEntities(std::move(archetype)) {}

ASpawner::~ASpawner()
{
	clear();
}

const ERefPtr_t& ASpawner::spawn()
{
	for (auto & pool : spawningEntities.archetype.cPools)
		pool.second->alloc();

	return spawningEntities.eRefs.alloc(std::make_shared<EntityRef>(this, spawningEntities.eRefs.getSize(), false));
}

ASpawner::ERefPtrPoolIteratorPair_t ASpawner::spawn(size_t n)
{
	for (auto & pool : spawningEntities.archetype.cPools)
		pool.second->allocN(n);

	spawningEntities.eRefs.allocN(n);

	for (size_t i = spawningEntities.eRefs.getSize() - n; i < spawningEntities.eRefs.getSize(); ++i)
		spawningEntities.eRefs[i] = std::make_shared<EntityRef>(this, i, false);

	return std::make_pair(spawningEntities.eRefs.end() - n, spawningEntities.eRefs.end());
}

void ASpawner::moveExistingEntityHere(EntityRef & eRef)
{
	if (!eRef.isValid() || eRef.originSpawner == this)
		return;

	ASpawner& originSpawner = *eRef.originSpawner;
	EntitiesData& originEData = eRef.alive ? originSpawner.aliveEntities : originSpawner.spawningEntities;

	// moving components
	for (auto& pool : spawningEntities.archetype.cPools)
	{
		auto found = originEData.archetype.cPools.find(pool.first);
		if (found != originEData.archetype.cPools.end())
		{
			Component& componentToMove = (*found->second)[eRef.id];
			pool.second->alloc(std::move(componentToMove));
			found->second->free(eRef.id);
		}
		else
			pool.second->alloc();
	}

	// moving reference
	spawningEntities.eRefs.alloc(eRef.shared_from_this());
	originSpawner.referenceKill(originEData, eRef.id);
	eRef.originSpawner = this;
	eRef.id = spawningEntities.eRefs.getSize() - 1;
}

void ASpawner::acceptSpawningEntities()
{

	for (auto& pool : aliveEntities.archetype.cPools)
	{
		auto& spawnedPool = *spawningEntities.archetype.cPools[pool.first];
		for (size_t i = 0; i < spawnedPool.getSize(); i++)
			pool.second->alloc(std::move(spawnedPool[i]));
	}
	spawningEntities.archetype.clear();

	for (auto& eRef : spawningEntities.eRefs)
	{
		eRef->id = aliveEntities.eRefs.getSize();
		eRef->alive = true;
		aliveEntities.eRefs.alloc(std::move(eRef));
	}
	spawningEntities.eRefs.clear();
}

void ASpawner::kill(EntityRef & ref)
{
	if (ref.originSpawner != this)
		return;

	EntitiesData& eData = ref.alive ? aliveEntities : spawningEntities;
	EntityId_t id = ref.id;

	for (auto & pool : eData.archetype.cPools)
		pool.second->free(id);
	referenceKill(eData, id);
}

void ASpawner::referenceKill(EntitiesData& eData, size_t id)
{
	eData.eRefs[id]->invalidate();
	eData.eRefs.free(id);

	// eRefs already moved the last object to the freed one, now we need to update its index to the current one
	// check if any entity was moved (if "id" wasn't the last index)
	if (eData.eRefs.getSize() && eData.eRefs.getSize() != id)
		eData.eRefs[id]->id = id;
}

void ASpawner::clear()
{
	aliveEntities.archetype.clear();
	spawningEntities.archetype.clear();

	for (auto& eRef : aliveEntities.eRefs)
		eRef->invalidate();
	for (auto& eRef : spawningEntities.eRefs)
		eRef->invalidate();

	aliveEntities.eRefs.clear();
	spawningEntities.eRefs.clear();
}

Component & ASpawner::getComponent(CTypeId_t cId, EntityId_t entityId, bool alive)
{
	return getPool(cId, alive)[entityId];
}

const Component & ASpawner::getComponent(CTypeId_t cId, EntityId_t entityId, bool alive) const
{
	return getPool(cId, alive)[entityId];
}

CPoolInterface & ASpawner::getPool(CTypeId_t cId, bool alive)
{
	return const_cast<CPoolInterface &>(((const ASpawner*)(this))->getPool(cId, alive));
}

const CPoolInterface & ASpawner::getPool(CTypeId_t cId, bool alive) const
{
	const EntitiesData& eData = alive ? aliveEntities : spawningEntities;

	auto found = eData.archetype.cPools.find(cId);
	EXC_ASSERT((found != eData.archetype.cPools.end()), std::out_of_range, "Wrong component type")
	return *found->second;
}

const Archetype & ASpawner::getArchetype() const
{
	return aliveEntities.archetype;
}

size_t ASpawner::getAliveEntitiesCount() const
{
	return aliveEntities.eRefs.getSize();
}


size_t ASpawner::getSpawningEntitiesCount() const
{
	return spawningEntities.eRefs.getSize();
}

const ERefPtr_t& ASpawner::operator[](EntityId_t i) const
{
	EXC_ASSERT((i < aliveEntities.eRefs.getSize()), std::out_of_range, "Wrong entity id")
	return aliveEntities.eRefs[i];
}