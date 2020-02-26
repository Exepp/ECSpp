#ifndef EPP_ENTITYSPAWNER_H
#define EPP_ENTITYSPAWNER_H

#include <ECSpp/internal/Archetype.h>
#include <ECSpp/internal/CPool.h>
#include <ECSpp/internal/EntityList.h>

namespace epp {

class EntitySpawner {
public:
    class Creator {
    public:
        /** 
         * Consecutive calls to "constructed" on already constructed components will only return the reference
         * TODO: Tests
        */
        template <typename CType, typename... Args>
        CType& constructed(Args&&... args);

    private:
        Creator(EntitySpawner& sp, PoolIdx index, CMask const& cstred = CMask());
        Creator(Creator&& rVal) = delete;
        Creator(Creator const&) = delete;
        Creator& operator=(Creator const&) = delete;
        Creator& operator=(Creator&&) = delete;
        ~Creator(); /** constructs components that the user didnt construct himself */


    private:
        EntitySpawner& spawner;
        CMask constrMask;
        PoolIdx const idx;

        friend class EntitySpawner;
    };

    using EntityPool_t = Pool<Entity>;

private:
    using CPools_t = std::vector<CPool>;

public:
    EntitySpawner(SpawnerId id, Archetype const& arch);
    EntitySpawner(EntitySpawner&&) = delete;
    EntitySpawner& operator=(EntitySpawner&&) = delete;
    EntitySpawner& operator=(EntitySpawner const&) = delete;
    EntitySpawner(EntitySpawner const&) = delete;

    template <typename FnType>
    Entity spawn(EntityList& entList, FnType fn);

    void destroy(Entity ent, EntityList& entList);

    template <typename FnType>
    void moveEntityHere(Entity ent, EntityList& entList, EntitySpawner& originSpawner, FnType fn);

    template <typename FnType>
    void moveEntitiesHere(EntitySpawner& originSpawner, EntityList& entList, FnType fn);

    // destroys all entities, keeps allocated memory
    void clear(EntityList& entList);
    void clear(); // for clear-all (no need to free indices, all will be freed)

    // makes sure, to fit n more elements without realloc
    void fitNextN(std::size_t n);

    // TODO: tests
    void shrinkToFit();


    CPool& getPool(ComponentId cId);
    CPool const& getPool(ComponentId cId) const;
    EntityPool_t const& getEntities() const { return entityPool; }
    Archetype makeArchetype() const;

private:
    void removeFromEntityPool(PoolIdx idx, EntityList& entList);

public:
    SpawnerId const spawnerId;
    CMask const mask;

private:
    EntityPool_t entityPool;

    CPools_t cPools;
};


using EntityCreator = EntitySpawner::Creator;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EntityCreator::Creator(EntitySpawner& sp, PoolIdx index, CMask const& cstred)
    : spawner(sp), constrMask(cstred), idx(index) {}


template <typename CType, typename... Args>
CType& EntityCreator::constructed(Args&&... args)
{
    EPP_ASSERT(spawner.mask.get(IdOf<CType>()));
    auto cId = IdOf<CType>();
    CType* component = static_cast<CType*>(spawner.getPool(cId)[idx.value]);
    if (constrMask.get(cId))
        return *component;
    constrMask.set(cId);
    return *(new (component) CType(std::forward<Args>(args)...));
}

inline EntityCreator::~Creator()
{
    for (auto& pool : spawner.cPools)
        if (constrMask.get(pool.getCId()) == false)
            pool.construct(idx.value);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


inline EntitySpawner::EntitySpawner(SpawnerId id, Archetype const& arch)
    : spawnerId(id), mask(arch.getMask())
{
    cPools.reserve(arch.getCIds().size());
    for (auto cId : arch.getCIds())
        cPools.emplace_back(cId);
    std::sort(cPools.begin(), cPools.end(), [](auto const& lhs, auto const& rhs) { return lhs.getCId() < rhs.getCId(); });
}

template <typename FnType>
inline Entity EntitySpawner::spawn(EntityList& entList, FnType fn)
{
    static_assert(std::is_invocable_v<FnType, Creator&&>);

    PoolIdx idx(entityPool.data.size());
    Entity ent = entList.allocEntity(idx, spawnerId);
    entityPool.create(ent);
    for (auto& pool : cPools)
        pool.alloc(); // only allocates memory (constructor is not called yet)
    fn(Creator(*this, idx));
    // constructors of the components are now already called
    // notify({ EntityEvent::Type::Creation, ent });
    return ent;
}

inline void EntitySpawner::destroy(Entity ent, EntityList& entList)
{
    EPP_ASSERT(entList.isValid(ent));

    // notify({ EntityEvent::Type::Destruction, ent });

    PoolIdx entPoolIdx = entList.get(ent).poolIdx;
    for (auto& pool : cPools)
        pool.destroy(entPoolIdx.value);
    removeFromEntityPool(entPoolIdx, entList);
    entList.freeEntity(ent);
}

inline void EntitySpawner::removeFromEntityPool(PoolIdx idx, EntityList& entList)
{
    if (entityPool.destroy(idx.value)) // if data was relocated in pools, change poolIdx in entList
        entList.changeEntity(entityPool.data[idx.value], idx, spawnerId);
}

template <typename FnType>
inline void EntitySpawner::moveEntityHere(Entity ent, EntityList& entList, EntitySpawner& originSpawner, FnType fn)
{
    static_assert(std::is_invocable_v<FnType, Creator&&>);
    EPP_ASSERT(entList.isValid(ent) && &originSpawner != this);

    PoolIdx oldIdx = entList.get(ent).poolIdx;
    PoolIdx newIdx(entityPool.data.size());
    auto oriPoolsPtr = originSpawner.cPools.begin();
    auto oriPoolsEnd = originSpawner.cPools.end();

    // moving/creating/destroying components
    // this loop takes advantage of the fact, that the pools are sorted by their cIds
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

template <typename FnType>
inline void EntitySpawner::moveEntitiesHere(EntitySpawner& originSpawner, EntityList& entList, FnType fn)
{
    static_assert(std::is_invocable_v<FnType, Creator&&>);

    if (originSpawner.entityPool.data.empty())
        return;
    std::size_t oriSize = originSpawner.entityPool.data.size();
    std::size_t thisSize = entityPool.data.size();
    auto oriPoolsPtr = originSpawner.cPools.begin();
    auto oriPoolsEnd = originSpawner.cPools.end();
    for (auto& pool : cPools) {
        for (; oriPoolsPtr != oriPoolsEnd && oriPoolsPtr->getCId() < pool.getCId(); ++oriPoolsPtr)
            oriPoolsPtr->clear();
        pool.alloc(oriSize);
        if (oriPoolsPtr != oriPoolsEnd && oriPoolsPtr->getCId() == pool.getCId())
            for (std::size_t i = 0; i < oriSize; ++i)
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

inline void EntitySpawner::clear(EntityList& entList)
{
    for (auto ent : entityPool.data)
        entList.freeEntity(ent);
    clear();
}

inline void EntitySpawner::clear()
{
    entityPool.data.clear();
    for (auto& pool : cPools)
        pool.clear();
}

inline void EntitySpawner::fitNextN(std::size_t n)
{
    entityPool.fitNextN(n);
    for (auto& pool : cPools)
        pool.fitNextN(n);
}

inline void EntitySpawner::shrinkToFit()
{
    entityPool.data.shrink_to_fit();
    for (auto& pool : cPools)
        pool.shrinkToFit();
}

inline Archetype EntitySpawner::makeArchetype() const
{
    Archetype arch;
    for (auto const& pool : cPools)
        arch.addComponent(pool.getCId());
    return arch;
}

inline CPool& EntitySpawner::getPool(ComponentId cId)
{
    EPP_ASSERT(mask.get(cId));
    return *std::find_if(cPools.begin(), cPools.end(), [cId](CPool const& pool) { return pool.getCId() == cId; });
}

inline CPool const& EntitySpawner::getPool(ComponentId cId) const
{
    EPP_ASSERT(mask.get(cId));
    return *std::find_if(cPools.begin(), cPools.end(), [cId](CPool const& pool) { return pool.getCId() == cId; });
}

} // namespace epp

#endif // EPP_ENTITYSPAWNER_H