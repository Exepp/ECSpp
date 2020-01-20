#include <ECSpp/Internal/EntitySpawner.h>
#include <ECSpp/Utility/Assert.h>

using namespace epp;


EntitySpawner::Creator::Creator(EntitySpawner& spr, PoolIdx const index, CMask const& cstred)
    : spawner(spr), idx(index), constructed(cstred)
{}

EntitySpawner::Creator::~Creator()
{
    for (auto& pool : spawner.cPools)
        if (constructed.get(pool.getCId()) == false)
            pool.construct(idx.value);
}

//////////////// EntitySpawner

EntitySpawner::EntitySpawner(SpawnerId id, Archetype const& arch)
    : spawnerId(id), mask(arch.getMask())
{
    cPools.reserve(arch.getCIds().size());
    for (auto cId : arch.getCIds())
        cPools.emplace_back(cId);
    std::sort(cPools.begin(), cPools.end(), [](auto const& lhs, auto const& rhs) { return lhs.getCId() < rhs.getCId(); });
}

Entity EntitySpawner::spawn(EntityList& entList, UserCreationFn_t const& fn)
{
    EPP_ASSERT(fn);
    PoolIdx idx(uint32_t(entityPool.data.size()));
    Entity ent = entList.allocEntity(idx, spawnerId);
    entityPool.create(ent);
    for (auto& pool : cPools)
        pool.alloc(); // only allocates memory (constructor is not called yet)
    fn(Creator(*this, idx));
    // constructors of the components are now already called
    notify({ EntityEvent::Type::Creation, ent });
    return ent;
}

void EntitySpawner::destroy(Entity ent, EntityList& entList)
{
    EPP_ASSERT(entList.isValid(ent));

    notify({ EntityEvent::Type::Destruction, ent });

    PoolIdx entPoolIdx = entList.get(ent).poolIdx;
    for (auto& pool : cPools)
        pool.destroy(entPoolIdx.value);
    removeFromEntityPool(entPoolIdx, entList);
    entList.freeEntity(ent);
}

void EntitySpawner::removeFromEntityPool(PoolIdx idx, EntityList& entList)
{
    if (entityPool.destroy(idx.value)) // if data was relocated in pools, change poolIdx in entList
        entList.changeEntity(entityPool.data[idx.value], idx, spawnerId);
}

void EntitySpawner::clear(EntityList& entList)
{
    for (auto ent : entityPool.data)
        entList.freeEntity(ent);
    clear();
}

void EntitySpawner::clear()
{
    entityPool.data.clear();
    for (auto& pool : cPools)
        pool.clear();
}

void EntitySpawner::fitNextN(EntityList::Size_t n)
{
    entityPool.fitNextN(n);
    for (auto& pool : cPools)
        pool.fitNextN(n);
}

void EntitySpawner::moveEntityHere(Entity ent, EntityList& entList, EntitySpawner& originSpawner, UserCreationFn_t const& fn)
{
    EPP_ASSERT(entList.isValid(ent) && &originSpawner != this && fn);

    PoolIdx entPoolIdx = entList.get(ent).poolIdx;
    auto oriPoolsPtr = originSpawner.cPools.begin();
    auto oriPoolsEnd = originSpawner.cPools.end();

    // moving/creating/destroying components
    // this loop uses the fact, that the pools are sorted by their cIds
    for (auto& pool : cPools) {
        while (oriPoolsPtr != oriPoolsEnd && oriPoolsPtr->getCId() < pool.getCId())
            (oriPoolsPtr++)->destroy(entPoolIdx.value);
        if (oriPoolsPtr != oriPoolsEnd && oriPoolsPtr->getCId() == pool.getCId()) // this component is in the original spawner, move it
            // component from the original will be destroyed either on the next loop, or
            // for the last one, after the whole loop
            pool.create((*oriPoolsPtr)[entPoolIdx.value]);
        else              // oriPoolsPtr->cId > pool.cId || oriPoolsPtr == oriPoolsEnd - this component in not in the original spawner
            pool.alloc(); // only allocates memory (constructor is not called yet)
    }
    while (oriPoolsPtr != oriPoolsEnd) // destroy the rest (if there is any)
        (oriPoolsPtr++)->destroy(entPoolIdx.value);
    originSpawner.removeFromEntityPool(entPoolIdx, entList);

    PoolIdx newIdx(uint32_t(entityPool.data.size()));
    entList.changeEntity(ent, newIdx, spawnerId);
    entityPool.create(ent);
    fn(Creator(*this, newIdx, originSpawner.mask)); // originSpawner.mask contains all the components that were moved, no need to delete possible excess
}

Archetype EntitySpawner::makeArchetype() const
{
    Archetype arch;
    for (auto const& pool : cPools)
        arch.addComponent(pool.getCId());
    return arch;
}