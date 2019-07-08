#include <ECSpp/EntityManager/EntityManager.h>

using namespace epp;

EntityRef EntityManager::spawn(const Archetype& arche)
{
    return registerArchetypeIfNew(arche).spawn();
}

const ASpawner& EntityManager::spawn(const Archetype& arche, size_t n)
{
    auto& aSpawner = registerArchetypeIfNew(arche);
    aSpawner.spawn(n);
    return aSpawner;
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

bool EntityManager::isEntityRefValid(const EntityRef& eRef)
{
    if (eRef.isValid())
    {
        auto found = aSpawners.find(eRef.getOriginSpawner()->getArchetype().getCMask());
        if (found != aSpawners.end())
            return found->second.get() == eRef.getOriginSpawner();
    }
    return false;
}

const EntityManager::ASpawnersHolder_t& EntityManager::getArchetypesSpawners() const
{
    return aSpawners;
}