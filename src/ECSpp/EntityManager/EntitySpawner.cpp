#include <ECSpp/EntityManager/EntitySpawner.h>
#include <assert.h>

using namespace epp;


EntitySpawner::EntitySpawner(Archetype arche, SpawnerID id)
    : spawnerID(id)
    , archetype(std::move(arche))
{
    for (auto const& creator : archetype.creators)
        cPools.push_back({ creator.id, creator.create() });
    std::sort(cPools.begin(), cPools.end(),
              [](auto const& lhs, auto const& rhs) { return lhs.id < rhs.id; });
}

Entity EntitySpawner::create(EntityList& entList)
{
    Entity ent = entList.allocEntity(PoolIdx(uint32_t(entityPool.content.size())), spawnerID);
    entityPool.alloc(ent);
    for (auto& pool : cPools)
        pool.ptr->alloc();
    notify({ EntityEvent::Type::Creation, ent });
    return ent;
}

void EntitySpawner::destroy(Entity ent, EntityList& entList)
{
    assert(entList.isValid(ent));

    notify({ EntityEvent::Type::Destruction, ent });

    PoolIdx entPoolIdx = entList.get(ent).poolIdx;
    for (auto& pool : cPools)
        pool.ptr->free(entPoolIdx.value);
    removeEntityFromEntityPoolOnly(entPoolIdx, entList);
    entList.freeEntity(ent);
}

void EntitySpawner::removeEntityFromEntityPoolOnly(PoolIdx idx, EntityList& entList)
{
    if (entityPool.free(idx.value)) // if data was relocated in pools, change poolIdx in entList
        entList.changeEntity(entityPool.content[idx.value], idx, spawnerID);
}

void EntitySpawner::clear()
{
    entityPool.content.clear();
    for (auto& pool : cPools)
        pool.ptr->clear();
}

void EntitySpawner::moveEntityHere(Entity ent, EntityList& entList, EntitySpawner& originSpawner)
{
    assert(entList.isValid(ent) && &originSpawner != this);

    PoolIdx entPoolIdx  = entList.get(ent).poolIdx;
    auto    orgPoolsPtr = originSpawner.cPools.begin();
    auto    orgPoolsEnd = originSpawner.cPools.end();

    // moving/creating components
    for (auto& pool : cPools)
    {
        while (orgPoolsPtr != orgPoolsEnd && orgPoolsPtr->id < pool.id)
        {
            orgPoolsPtr->ptr->free(entPoolIdx.value);
            ++orgPoolsPtr;
        }
        if (orgPoolsPtr == orgPoolsEnd || orgPoolsPtr->id != pool.id)
            pool.ptr->alloc();
        else
        {
            pool.ptr->alloc(std::move((*orgPoolsPtr->ptr)[entPoolIdx.value]));
            orgPoolsPtr->ptr->free(entPoolIdx.value);
        }
    }
    originSpawner.removeEntityFromEntityPoolOnly(entPoolIdx, entList);
    entList.changeEntity(ent, PoolIdx(uint32_t(entityPool.content.size())), spawnerID);
    entityPool.alloc(ent);
}

const Archetype& EntitySpawner::getArchetype() const
{
    return archetype;
}

EntitySpawner::EntityPool_t const& EntitySpawner::getEntities() const
{
    return entityPool;
}