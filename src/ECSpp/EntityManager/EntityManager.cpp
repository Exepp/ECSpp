#include <ECSpp/EntityManager.h>

using namespace epp;

Entity EntityManager::spawn(Archetype const& arch, EntitySpawner::UserCreationFn_t const& fn)
{
    return getSpawner(arch).spawn(entList, fn);
}

std::pair<EntityManager::EPoolCIter_t, EntityManager::EPoolCIter_t>
EntityManager::spawn(Archetype const& arch, std::size_t n, EntitySpawner::UserCreationFn_t const& fn)
{
    EntitySpawner& spawner = _prepareToSpawn(arch, n);
    for (std::size_t i = 0; i < n; ++i)
        spawner.spawn(entList, fn);
    return { spawner.getEntities().data.end() - n, spawner.getEntities().data.end() };
}

void EntityManager::changeArchetype(Entity ent, Archetype const& newArchetype, EntitySpawner::UserCreationFn_t const& fn)
{
    EPP_ASSERT(entList.isValid(ent));
    EntitySpawner& spawner = getSpawner(ent);
    if (spawner.mask != newArchetype.getMask())
        getSpawner(newArchetype).moveEntityHere(ent, entList, spawner, fn);
}

void EntityManager::changeArchetype(Entity ent, IdList_t toAdd, IdList_t toRemove, EntitySpawner::UserCreationFn_t const& fn)
{
    EPP_ASSERT(entList.isValid(ent));
    EntitySpawner& spawner = getSpawner(ent);
    Archetype newArchetype = spawner.makeArchetype().removeComponent(toRemove).addComponent(toAdd);
    if (spawner.mask != newArchetype.getMask())
        getSpawner(newArchetype).moveEntityHere(ent, entList, spawner, fn);
}

std::pair<EntityManager::EPoolCIter_t, EntityManager::EPoolCIter_t>
EntityManager::changeArchetype(Archetype const& oldArchetype, Archetype const& newArchetype, EntitySpawner::UserCreationFn_t const& fn)
{
    EPP_ASSERT(findSpawner(oldArchetype) != spawners.end());
    EntitySpawner& oldSp = getSpawner(oldArchetype);
    EntitySpawner& newSp = getSpawner(newArchetype);
    if (oldSp.mask == newSp.mask)
        return { EntityManager::EPoolCIter_t(), EntityManager::EPoolCIter_t() };
    std::size_t newSpSize = newSp.getEntities().data.size();
    newSp.moveEntitiesHere(oldSp, entList, fn);
    return { newSp.getEntities().data.begin() + newSpSize, newSp.getEntities().data.end() };
}

void EntityManager::clear()
{
    for (auto& spawner : spawners)
        spawner.clear();
    entList.freeAll();
}

void EntityManager::clear(Archetype const& arch)
{
    if (auto found = findSpawner(arch); found != spawners.end())
        found->clear(entList);
}


void EntityManager::prepareToSpawn(Archetype const& arch, std::size_t n)
{
    _prepareToSpawn(arch, n);
}

inline EntitySpawner& EntityManager::_prepareToSpawn(Archetype const& arch, std::size_t n)
{
    EntitySpawner& spawner = getSpawner(arch);
    entList.fitNextN(n);
    spawner.fitNextN(n);
    return spawner;
}