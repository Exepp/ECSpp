#ifndef EPP_ENTITYSPAWNER_H
#define EPP_ENTITYSPAWNER_H

#include <ECSpp/internal/Archetype.h>
#include <ECSpp/internal/CPool.h>
#include <ECSpp/internal/EntityList.h>

namespace epp {

class EntitySpawner {
public:
    /// Creator is responsible for easy components initialization owned by some entity
    /**
     * Instances of this class can only be created and destroyed in EntitySpawner functions, 
     * because they construct unconstructed components in their destructor. So to ensure
     * that every component will be constructed after leaving the scope of EntitySpanwer::spawn function,
     * Creator has no public constructors or assignment operators (to prevent moving the instance out of the function's scope)
     */
    class Creator {
    public:
        /// Ensures that the returned component is constructed
        /** 
         * Consecutive calls to "constructed" on already constructed components will only return the reference
         * @tparam CType Component type owned by the entity that is being constructed
         * @tparam Args Types of arguments that will be forwarded
         * @param args Arguments forwarded to construct an unconstructed component
         * @returns A reference to the constructed component of type CType
         * @throws (Debug only) Throws the AssertionFailed exception if the entity that 
         * the creator is pointing to does not own component of CType type
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
    /// Constructs the spawner to spawn entities of a given archetype
    /** 
     * @param id A unique id to identify this spawner
     * @param arch Archetype of entities that will be spawned in this spawner
    */
    EntitySpawner(SpawnerId id, Archetype const& arch);

    EntitySpawner(EntitySpawner&&) = delete;
    EntitySpawner& operator=(EntitySpawner&&) = delete;
    EntitySpawner& operator=(EntitySpawner const&) = delete;
    EntitySpawner(EntitySpawner const&) = delete;


    /// Spawns a new entity with the archetype specified in the Spawner's constructor
    /**
     * @tparam FnType Callable type that takes r-value reference to the EntityCreator
     * @param entList List of entities to get a unique Entity instance from
     * @param fn A Callable type that can use the Creator instance to construct the components of the spawned entity
     * @returns An entity
     */
    template <typename FnType>
    Entity spawn(EntityList& entList, FnType fn);


    /// Destroys a valid entity and makes it invalid
    /**
     * @param ent A valid entity
     * @param entList List of entities to free ent for future use
     * @throws (Debug only) Throws the AssertionFailed exception if ent is invalid
     */
    void destroy(Entity ent, EntityList& entList);


    /// Changes the archetype of a given entity
    /**
     * @details Components that are not present in this spawner's archetype are destroyed
     * @details Components that are not present in originSpawner's archetype but are present in this spawner's archetype 
     *          are allocated and can be constructed for the first time with Creator  
     * @tparam FnType Callable type that takes r-value reference to the EntityCreator
     * @param ent A valid entity
     * @param entList List of entities to change the location data of ent
     * @param originSpawner any OTHER spawner that owns entity
     * @param fn A Callable type that can use the Creator instance to construct new components
     * @returns True when changed the archetype, false otherwise (newArchetype was the same as ent's current archetype) 
     * @throws (Debug only) Throws the AssertionFailed exception if ent is invalid or &originSpawner == this or if originSpawner
     *          is not the origin spawner of ent
     */
    template <typename FnType>
    void moveEntityHere(Entity ent, EntityList& entList, EntitySpawner& originSpawner, FnType fn);


    /// Destroys every entity in this spawner, keeps resereved memory
    /** 
     * Frees each entity individually in entList, to ensure that no entity cell will be lost 
     * (that every destroyed entity will be reused)
     */
    void clear(EntityList& entList);

    /// Destroys every entity, keeps resereved memory
    /** 
     * Faster than clear(entList) but if used without calling entList's clear before/after this function,
     * then every cell of destroyed here entity will be lost until next entList's clear 
     */
    void clear();


    /// The next n spawn(...) calls will not require reallocation in any of the internal structures
    /** 
     * @param n Number of entities to reserve the additional memory for
     */
    void fitNextN(std::size_t n);


    /// Removes the excess of the reserved memory in the internal structures
    void shrinkToFit();


    /// Returns CPool of components with cId id
    /** 
     * @param cId ComponentId that is present in the archetype of this spawner
     * @returns A reference to the pool
     * @throws (Debug only) Throws the AssertionFailed exception if cId is not present in this spawner's archetype
     */
    CPool& getPool(ComponentId cId);


    /** @copydoc EntitySpawner::getPool(ComponentId cId) */
    CPool const& getPool(ComponentId cId) const;


    /// Returns a pool of entities used by this spawner
    /** 
     * @returns A reference to the pool of entities
     */
    EntityPool_t const& getEntities() const { return entityPool; }


    /// Returns archetype of this spawner
    /** 
     * Creartes the same archetype that this spawner was constructed with
     * @returns Archetype of this spawner
     */
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

inline EntityCreator::Creator(EntitySpawner& sp, PoolIdx index, CMask const& cstred)
    : spawner(sp), constrMask(cstred), idx(index) {}


template <typename CType, typename... Args>
inline CType& EntityCreator::constructed(Args&&... args)
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
    EPP_ASSERT(entList.isValid(ent) && &originSpawner != this && entList.get(ent).spawnerId == originSpawner.spawnerId);

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