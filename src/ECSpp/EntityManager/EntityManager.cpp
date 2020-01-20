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

void EntityManager::destroy(Entity ent)
{
    EPP_ASSERT(entList.isValid(ent));
    getSpawner(ent).destroy(ent, entList);
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

CMask const& EntityManager::maskOf(Entity ent) const
{
    EPP_ASSERT(entList.isValid(ent));
    return getSpawner(ent).mask;
}

Archetype EntityManager::archetypeOf(Entity ent) const
{
    EPP_ASSERT(entList.isValid(ent));
    return getSpawner(ent).makeArchetype();
}

std::size_t EntityManager::size(Archetype const& arch) const
{
    if (auto found = findSpawner(arch); found != spawners.end())
        return found->getEntities().data.size();
    return 0;
}

EntitySpawner& EntityManager::getSpawner(Archetype const& arch)
{
    if (auto found = findSpawner(arch); found != spawners.end())
        return *found;
    return spawners.emplace_back(SpawnerId(std::uint32_t(spawners.size())), arch); // if not found, make one
}

EntitySpawner& EntityManager::getSpawner(Entity ent)
{
    return spawners[entList.get(ent).spawnerId.value];
}

EntitySpawner const& EntityManager::getSpawner(Entity ent) const
{
    return spawners[entList.get(ent).spawnerId.value];
}

EntityManager::Spawners_t::iterator EntityManager::findSpawner(Archetype const& arch)
{
    return std::find_if(spawners.begin(), spawners.end(), [mask = arch.getMask()](EntitySpawner const& spawner) { return spawner.mask == mask; });
}

EntityManager::Spawners_t::const_iterator EntityManager::findSpawner(Archetype const& arch) const
{
    return std::find_if(spawners.begin(), spawners.end(), [mask = arch.getMask()](EntitySpawner const& spawner) { return spawner.mask == mask; });
}