#include <ECSpp/EntityManager/EntityManager.h>

using namespace epp;

Entity EntityManager::spawn(Archetype const& arche)
{
    return registerArchetypeIfNew(arche).create(entList);
}

void EntityManager::destroy(Entity ent)
{
    if (entList.isValid(ent))
        spawnersByIDs[entList.get(ent).spawnerID().value].destroy(ent, entList);
}

void EntityManager::addComponent(Entity ent, CompIDList_t cIDs)
{
    if (!entList.isValid(ent))
        return;
    auto           entity  = entList.get(ent);
    EntitySpawner& spawner = spawnersByIDs[entity.spawnerID().value];
    if (spawner.getArchetype().hasAllOf(cIDs))
        return;
    Archetype newArchetype = spawner.getArchetype();
    newArchetype.addComponent(cIDs);
    registerArchetypeIfNew(std::move(newArchetype)).moveEntityHere(ent, entList, spawner);
}

void EntityManager::removeComponent(Entity ent, CompIDList_t cIDs)
{
    if (!entList.isValid(ent))
        return;
    auto           entity  = entList.get(ent);
    EntitySpawner& spawner = spawnersByIDs[entity.spawnerID().value];
    if (spawner.getArchetype().hasAnyOf(cIDs))
        return;
    Archetype newArchetype = spawner.getArchetype();
    newArchetype.removeComponent(cIDs);
    registerArchetypeIfNew(std::move(newArchetype)).moveEntityHere(ent, entList, spawner);
}

void EntityManager::clear()
{
    for (auto& spawner : spawnersByIDs)
        spawner.clear();
}

void EntityManager::registerArchetype(Archetype arche)
{
    registerArchetypeIfNew(std::move(arche));
}