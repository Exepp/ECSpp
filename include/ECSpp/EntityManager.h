#ifndef EPP_ENTITYMANAGER_H
#define EPP_ENTITYMANAGER_H

#include <ECSpp/internal/EntityList.h>
#include <ECSpp/internal/EntitySpawner.h>
#include <ECSpp/internal/Selection.h>
#include <deque>

namespace epp {


class EntityManager {
    using Spawners_t = std::deque<EntitySpawner>; // deque, to keep selections' references valid
    using EntityPool_t = EntitySpawner::EntityPool_t;
    using EPoolIter_t = EntitySpawner::EntityPool_t::Container_t::iterator;
    using EPoolCIter_t = EntitySpawner::EntityPool_t::Container_t::const_iterator;
    static_assert(std::is_same_v<EntityPool_t::Container_t, std::vector<Entity>>, "changeEntity works only with vectors");

    using IdList_t = decltype(IdOfL<>());

    inline static auto DefCreationFn = [](EntityCreator&&) {};
    using DefCreationFn_t = decltype(DefCreationFn);

public:
    /// Spawns a new entity with a given archetype
    /**
     * @tparam FnType Callable type that takes r-value reference to the EntityCreator
     * @param arch Any archetype. The archetype of the spawned entity
     * @param fn A Callable type that can use the Creator instance to construct components of the spawned entity
     * @returns An entity
     */
    template <typename FnType = DefCreationFn_t>
    std::enable_if_t<std::is_invocable_v<FnType, EntityCreator&&>, Entity> // disable for spawn(arche, n)
    spawn(Archetype const& arch, FnType fn = DefCreationFn);


    /// Spawns n new entities with a given archetype
    /**
     * @tparam FnType Callable type that takes r-value reference to the EntityCreator
     * @param arch Any archetype. The archetype of spawned entities
     * @param fn A Callable type that can use the Creator instance to construct components of the spawned entities
     * @returns A (begin, end) iterators pair to the spawned entities
     */
    template <typename FnType = DefCreationFn_t>
    std::pair<EPoolCIter_t, EPoolCIter_t>
    spawn(Archetype const& arch, std::size_t n, FnType fn = DefCreationFn);


    /// Changes the archetype of a given entity
    /**
     * @tparam FnType Callable type that takes r-value reference to the EntityCreator
     * @param ent A valid entity
     * @param newArchetype Any archetype. A new archetype of ent
     * @param fn A Callable type that can use the Creator instance to construct new components
     * @returns True when changed the archetype, false otherwise (newArchetype was the same as ent's current archetype) 
     */
    template <typename FnType = DefCreationFn_t>
    bool changeArchetype(Entity ent, Archetype const& newArchetype, FnType fn = DefCreationFn);


    /// Changes the archetype of ent, removing components from toRemove list and adding the ones from toAdd list
    /**
     * There are intentionally no overloaded versions of this function (for iterators) - it is more optimal to create a separate archetype for those situations
     * @tparam FnType Callable type that takes r-value reference to the EntityCreator
     * @param ent A valid entity
     * @param toAdd Components that will be added to ent (or kept it these are already there). toAdd has a priority over toRemove
     * @param toRemove Components that will be removed from ent (except for the ones specified in toAdd)
     * @param fn A Callable type that can use the Creator instance to construct the added components
     * @returns True when changed the archetype, false otherwise (resulting archetype was the same as ent's current archetype) 
     */
    template <typename FnType = DefCreationFn_t>
    bool changeArchetype(Entity ent, IdList_t toRemove, IdList_t toAdd, FnType fn = DefCreationFn);


    /// Changes the archetype of the entity associated with the iterator and returns the next valid one
    /**
     * There are no overloaded versions of this function (for iterators) on purpose - it is more optimal to create a separate archetype for those situations
     * @tparam Iter EntitySpawner::EntityPool_t::Container_t::iterator or Selection<...>::Iterator_t
     * @tparam FnType Callable type that takes r-value reference to the EntityCreator
     * @param it A valid and non-end iterator
     * @param newArchetype Any archetype - a new archetype of the entity associated with the iterator  
     * @param fn A Callable type that can use the Creator instance to construct the added components
     * @returns A valid iterator to the next entity or end
     */
    template <typename Iter, typename FnType = DefCreationFn_t>
    Iter changeArchetype(Iter const& it, Archetype const& newArchetype, FnType fn = DefCreationFn);


    /// Destroys a valid entity and makes it invalid
    /**
     * @param ent A valid entity
     * @throws (Debug only) Throws the AssertionFailed exception if ent is invalid
     */
    void destroy(Entity ent);


    /// Destroys the entity associated with the iterator and returns the next valid one
    /**
     * @tparam Iter EntitySpawner::EntityPool_t::Container_t::iterator or Selection<...>::Iterator_t
     * @param it A valid and non-end iterator
     * @returns A valid iterator to the next entity or end
     * @throws (Debug only) Throws the AssertionFailed exception if "it" is invalid
     */
    template <typename Iter>
    Iter destroy(Iter const& it);


    /// Destroys every entity, keeps resereved memory
    /** 
     * Faster than calling destroy on each entity individually
     */
    void clear();


    /// Destroys every entity with a given archetype
    /** 
     * Faster than calling destroy on each entity individually
     * @param arch Any archetype
     */
    void clear(Archetype const& arch);


    /// The next n spawn(arch, ...) calls will not require reallocation in any of the internal structures
    /** 
     * This function can be used to "register" an archetype
     * @param arch Any archetype
     * @param n Number of entities to reserve the additional memory for
     */
    void prepareToSpawn(Archetype const& arch, std::size_t n);


    /// Removes the excess of the reserved memory in the internal structures for a given archetype
    /** 
     * @param arch Any archetype
     */
    void shrinkToFit(Archetype const& arch);


    /// Removes the excess of the reserved memory in the internal structures for every used archetype
    void shrinkToFit();


    /// Updates selection so that its iterators can reach entities of the archetypes that were not present in its last update
    /** 
     * Complexity: O(n), where n is a number of new (for that selection) archetypes used
     * @warning Once selection has been updated in one EntityManager it musn't be updated in any other EntityManager
     * @tparam CTypes Types of components specified in the selection's type
     * @param selection Any Selection to be updated
     */
    template <typename... CTypes>
    void updateSelection(Selection<CTypes...>& selection);


    /// Returns an internal data that describes the location of a given entity
    /** 
     * This data can be used with selections' iterators
     * @param ent Valid entity
     * @returns Default-constructed cell for invalid
     * @throws (Debug only) Throws the AssertionFailed exception if ent is invalid
     */
    EntityList::Cell::Occupied cellOf(Entity ent) const;


    /// Returns a reference to the component of type TComp owned by a given entity
    /** 
     * @tparam TComp A type of the component to return
     * @param ent A valid entity that owns the component (to be sure that an entity owns a component, use maskOf(ent).get(Component::Id().value))
     * @returns A reference to the component of type TComp associated with ent
     * @throws (Debug only) Throws the AssertionFailed exception if ent is invalid
     */
    template <typename TComp>
    TComp& componentOf(Entity ent);


    /// Returns a mask that describes which of the components are owned by a given entity
    /** 
     * @param ent A valid entity
     * @returns The CMask of the spawner that ent is currently in
     * @throws (Debug only) Throws the AssertionFailed exception if ent is invalid
     */
    CMask maskOf(Entity ent) const;


    /// Returns a mask that describes which of the components are owned by a given entity
    /** 
     * @param ent A valid entity
     * @returns The archetype of ent
     * @throws (Debug only) Throws the AssertionFailed exception if ent is invalid
     */
    Archetype archetypeOf(Entity ent) const;


    /// Returns an internal pool used by the spawner of entities with a given archetype
    /** 
     * @param arch One of archetypes that were already used to spawn entities during this application
     * @returns Pool of entities with the arch archetype
     * @throws (Debug only) Throws the AssertionFailed exception if arch was never used before in this EntityManager
     */
    EntityPool_t const& entitiesOf(Archetype const& arch) const;


    /// Checks whether a given entity is valid or not
    /** 
     * @param ent Any entity
     * @returns True for valid entity, false for invalid one
     */
    bool isValid(Entity ent) const { return entList.isValid(ent); }


    /// Returns the number of alive entities
    /** 
     * @returns The number of alive entities
     */
    std::size_t size() const { return entList.size(); }


    /// Returns the number of alive entities with a given archetype
    /** 
     * @param arch Any archetype
     * @returns The number of alive entities with a given archetype
     */
    std::size_t size(Archetype const& arch) const;

private:
    EntitySpawner& _prepareToSpawn(Archetype const& arch, std::size_t n);
    EntitySpawner& getSpawner(Archetype const& arch);
    EntitySpawner& getSpawner(Entity ent) { return spawners[entList.get(ent).spawnerId.value]; }
    EntitySpawner const& getSpawner(Entity ent) const { return spawners[entList.get(ent).spawnerId.value]; }
    Spawners_t::iterator findSpawner(Archetype const& arch);
    Spawners_t::const_iterator findSpawner(Archetype const& arch) const;

private:
    Spawners_t spawners;

    EntityList entList;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template <typename FnType>
inline std::enable_if_t<std::is_invocable_v<FnType, EntityCreator&&>, Entity>
EntityManager::spawn(Archetype const& arch, FnType fn)
{
    return getSpawner(arch).spawn(entList, std::move(fn));
}

template <typename FnType>
inline std::pair<EntityManager::EPoolCIter_t, EntityManager::EPoolCIter_t>
EntityManager::spawn(Archetype const& arch, std::size_t n, FnType fn)
{
    EntitySpawner& spawner = _prepareToSpawn(arch, n);
    for (std::size_t i = 0; i < n; ++i)
        spawner.spawn(entList, fn);
    return { spawner.getEntities().data.end() - std::ptrdiff_t(n), spawner.getEntities().data.end() };
}

template <typename FnType>
inline bool EntityManager::changeArchetype(Entity ent, Archetype const& newArchetype, FnType fn)
{
    EPP_ASSERT(entList.isValid(ent));
    EntitySpawner& spawner = getSpawner(ent);
    if (spawner.mask != newArchetype.getMask()) {
        getSpawner(newArchetype).moveEntityHere(ent, entList, spawner, std::move(fn));
        return true;
    }
    return false;
}

template <typename FnType>
inline bool EntityManager::changeArchetype(Entity ent, IdList_t toRemove, IdList_t toAdd, FnType fn)
{
    EPP_ASSERT(entList.isValid(ent));
    EntitySpawner& spawner = getSpawner(ent);
    Archetype newArchetype = spawner.makeArchetype().removeComponent(toRemove).addComponent(toAdd);
    if (spawner.mask != newArchetype.getMask()) {
        getSpawner(newArchetype).moveEntityHere(ent, entList, spawner, std::move(fn));
        return true;
    }
    return false;
}

template <typename Iter, typename FnType>
inline Iter
EntityManager::changeArchetype(Iter const& it, Archetype const& newArchetype, FnType fn)
{
    if (changeArchetype(*it, newArchetype, std::move(fn)))
        if constexpr (std::is_same_v<Iter, EPoolIter_t> || std::is_same_v<Iter, EPoolCIter_t>)
            return it;
        else if (it.isValid())
            return it;
    return ++Iter(it); // if didn't change - entity stays in its place -> need to increment
                       // or changed and iterator is now invalid (iterator can be valid after a change)
}

inline void EntityManager::destroy(Entity ent)
{
    EPP_ASSERT(entList.isValid(ent));
    getSpawner(ent).destroy(ent, entList);
}

template <typename Iter>
inline Iter EntityManager::destroy(Iter const& it)
{
    destroy(*it);
    if constexpr (!std::is_same_v<Iter, EPoolIter_t> && !std::is_same_v<Iter, EPoolCIter_t>)
        if (!it.isValid())
            return ++Iter(it);
    return it;
}

inline void EntityManager::clear()
{
    for (auto& spawner : spawners)
        spawner.clear();
    entList.freeAll();
}

inline void EntityManager::clear(Archetype const& arch)
{
    if (auto spawner = findSpawner(arch); spawner != spawners.end())
        spawner->clear(entList);
}

inline void EntityManager::prepareToSpawn(Archetype const& arch, std::size_t n)
{
    _prepareToSpawn(arch, n);
}

inline void EntityManager::shrinkToFit(Archetype const& arch)
{
    if (auto spawner = findSpawner(arch); spawner != spawners.end())
        spawner->shrinkToFit();
}

inline void EntityManager::shrinkToFit()
{
    for (auto& spawner : spawners)
        spawner.shrinkToFit();
}

template <typename... CTypes>
inline void EntityManager::updateSelection(Selection<CTypes...>& selection)
{
    for (; selection.checkedSpawnersNum < spawners.size(); ++selection.checkedSpawnersNum)
        selection.addSpawnerIfMeetsRequirements(spawners[selection.checkedSpawnersNum]);
}

inline EntityList::Cell::Occupied EntityManager::cellOf(Entity ent) const
{
    EPP_ASSERT(entList.isValid(ent));
    return entList.get(ent);
}

template <typename TComp>
inline TComp& EntityManager::componentOf(Entity ent)
{
    EPP_ASSERT(entList.isValid(ent) && getSpawner(ent).mask.get(IdOf<TComp>()));
    return *static_cast<TComp*>(getSpawner(ent).getPool(IdOf<TComp>())[entList.get(ent).poolIdx.value]);
}

inline CMask EntityManager::maskOf(Entity ent) const
{
    EPP_ASSERT(entList.isValid(ent));
    return getSpawner(ent).mask;
}

inline Archetype EntityManager::archetypeOf(Entity ent) const
{
    EPP_ASSERT(entList.isValid(ent));
    return getSpawner(ent).makeArchetype();
}

inline EntityManager::EntityPool_t const&
EntityManager::entitiesOf(Archetype const& arch) const
{
    EPP_ASSERT(findSpawner(arch) != spawners.end());
    return findSpawner(arch)->getEntities();
}

inline std::size_t EntityManager::size(Archetype const& arch) const
{
    if (auto found = findSpawner(arch); found != spawners.end())
        return found->getEntities().data.size();
    return 0;
}

inline EntitySpawner& EntityManager::_prepareToSpawn(Archetype const& arch, std::size_t n)
{
    EntitySpawner& spawner = getSpawner(arch);
    entList.fitNextN(n);
    spawner.fitNextN(n);
    return spawner;
}

inline EntitySpawner& EntityManager::getSpawner(Archetype const& arch)
{
    if (auto found = findSpawner(arch); found != spawners.end())
        return *found;
    return spawners.emplace_back(SpawnerId(spawners.size()), arch); // if not found, make one
}

inline EntityManager::Spawners_t::iterator
EntityManager::findSpawner(Archetype const& arch)
{
    return std::find_if(spawners.begin(), spawners.end(), [mask = arch.getMask()](EntitySpawner const& spawner) { return spawner.mask == mask; });
}

inline EntityManager::Spawners_t::const_iterator
EntityManager::findSpawner(Archetype const& arch) const
{
    return std::find_if(spawners.begin(), spawners.end(), [mask = arch.getMask()](EntitySpawner const& spawner) { return spawner.mask == mask; });
}


} // namespace epp

#endif // EPP_ENTITYMANAGER_H