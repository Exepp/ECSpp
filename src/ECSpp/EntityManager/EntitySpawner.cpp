#include <ECSpp/Internal/EntitySpawner.h>
#include <ECSpp/Utility/Assert.h>

using namespace epp;


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
    // notify({ EntityEvent::Type::Creation, ent });
    return ent;
}

void EntitySpawner::destroy(Entity ent, EntityList& entList)
{
    EPP_ASSERT(entList.isValid(ent));

    // notify({ EntityEvent::Type::Destruction, ent });

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

void EntitySpawner::shrinkToFit()
{
    entityPool.data.shrink_to_fit();
    for (auto& pool : cPools)
        pool.shrinkToFit();
}

void EntitySpawner::moveEntityHere(Entity ent, EntityList& entList, EntitySpawner& originSpawner, UserCreationFn_t const& fn)
{
    EPP_ASSERT(entList.isValid(ent) && &originSpawner != this && fn);

    PoolIdx oldIdx = entList.get(ent).poolIdx;
    PoolIdx newIdx(uint32_t(entityPool.data.size()));
    auto oriPoolsPtr = originSpawner.cPools.begin();
    auto oriPoolsEnd = originSpawner.cPools.end();

    // moving/creating/destroying components
    // this loop uses the fact, that the pools are sorted by their cIds
    for (auto& pool : cPools) {
        while (oriPoolsPtr != oriPoolsEnd && oriPoolsPtr->getCId() < pool.getCId()) // omit (destroy) components that were not specified (in archetype) to be in this spawner
            (oriPoolsPtr++)->destroy(oldIdx.value);
        pool.alloc();                                                             // only allocates memory (constructor is not called yet)
        if (oriPoolsPtr != oriPoolsEnd && oriPoolsPtr->getCId() == pool.getCId()) // this component is in the original spawner, move it
            pool.construct(newIdx.value, (*oriPoolsPtr)[oldIdx.value]);           // component from the original will be destroyed either on the next loop or, for the last one, after the whole loop
    }
    while (oriPoolsPtr != oriPoolsEnd) // destroy the rest (if there is any)
        (oriPoolsPtr++)->destroy(oldIdx.value);
    originSpawner.removeFromEntityPool(oldIdx, entList); // remove entity
    entityPool.create(ent);                              // add entity
    entList.changeEntity(ent, newIdx, spawnerId);
    fn(Creator(*this, newIdx, originSpawner.mask)); // originSpawner.mask contains all the components that were moved, no need to delete possible excess
}

void EntitySpawner::moveEntitiesHere(EntitySpawner& originSpawner, EntityList& entList, UserCreationFn_t fn)
{
    if (originSpawner.entityPool.data.empty())
        return;
    auto oriSize = std::uint32_t(originSpawner.entityPool.data.size());
    auto thisSize = std::uint32_t(entityPool.data.size());
    auto oriPoolsPtr = originSpawner.cPools.begin();
    auto oriPoolsEnd = originSpawner.cPools.end();
    for (auto& pool : cPools) {
        for (; oriPoolsPtr != oriPoolsEnd && oriPoolsPtr->getCId() < pool.getCId(); ++oriPoolsPtr)
            oriPoolsPtr->clear();
        pool.alloc(oriSize);
        if (oriPoolsPtr != oriPoolsEnd && oriPoolsPtr->getCId() == pool.getCId())
            for (std::uint32_t i = 0; i < oriSize; ++i)
                pool.construct(thisSize + i, (*oriPoolsPtr)[i]);
    }
    for (; oriPoolsPtr != oriPoolsEnd; ++oriPoolsPtr)
        oriPoolsPtr->clear();

    entityPool.fitNextN(oriSize);
    for (PoolIdx i(thisSize), end(thisSize + oriSize); i.value < end.value; ++i.value) {
        entityPool.create(originSpawner.entityPool.data.front());
        entList.changeEntity(originSpawner.entityPool.data.front(), i, spawnerId);
        originSpawner.entityPool.destroy(0);
        fn(Creator(*this, i, originSpawner.mask));
    }
}

Archetype EntitySpawner::makeArchetype() const
{
    Archetype arch;
    for (auto const& pool : cPools)
        arch.addComponent(pool.getCId());
    return arch;
}