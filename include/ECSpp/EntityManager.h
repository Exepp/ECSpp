#ifndef EPP_ENTITYMANAGER_H
#define EPP_ENTITYMANAGER_H

#include <ECSpp/internal/EntityList.h>
#include <ECSpp/internal/EntitySpawner.h>
#include <ECSpp/internal/Selection.h>
#include <deque>
#include <unordered_map>

namespace epp {

// TODO: redo the tests
class EntityManager {
    using Spawners_t = std::deque<EntitySpawner>; // deque, to keep selections' references valid
    using EntityPool_t = EntitySpawner::EntityPool_t;
    using EPoolCIter_t = EntitySpawner::EntityPool_t::Container_t::const_iterator;
    using IdList_t = decltype(IdOfL<>());

    inline static auto DefCreationFn = [](EntityCreator&&) {};
    using DefCreationFn_t = decltype(DefCreationFn);

public:
    template <typename FnType = DefCreationFn_t>
    std::enable_if_t<std::is_invocable_v<FnType, EntityCreator&&>, Entity> // disable for spawn(arche, n)
    spawn(Archetype const& arch, FnType fn = DefCreationFn);


    // first - iterator to the first of spawned entities
    // second - end iterator
    template <typename FnType = DefCreationFn_t>
    std::pair<EPoolCIter_t, EPoolCIter_t>
    spawn(Archetype const& arch, std::size_t n, FnType fn = DefCreationFn);


    // ent - valid entity
    // newArchetype - any archetype
    template <typename FnType = DefCreationFn_t>
    void
    changeArchetype(Entity ent, Archetype const& newArchetype, FnType fn = DefCreationFn);


    /// Changes the archetype of ent, removing components from toRemove list and adding the ones from toAdd list
    /**
     * There are no overloaded versions of this function (for iterators) on purpose - it is more optimal to create a separate archetype for those situations
     * @param ent A valid entity
     * @param toAdd Components that will be added to ent (or kept it these are already there). toAdd has a priority over toRemove
     * @param toRemove Components that will be removed from ent (except for the ones specified in toAdd)
     * @param fn A callable object that can use the Creator instance to construct the added components
     */
    template <typename FnType = DefCreationFn_t>
    void
    changeArchetype(Entity ent, IdList_t toAdd, IdList_t toRemove = IdOfL<>(), FnType fn = DefCreationFn);


    /// Changes the archetype of the entity associated with the iterator and returns the next valid one
    /**
     * There are no overloaded versions of this function (for iterators) on purpose - it is more optimal to create a separate archetype for those situations
     * @param it A valid and non-end iterator
     * @param newArchetype Any archetype - a new archetype of the entity associated with the iterator  
     * @param fn A callable object that can use the Creator instance to construct the added components
     */
    template <typename Iterator, typename FnType = DefCreationFn_t>
    Iterator
    changeArchetype(Iterator it, Archetype const& newArchetype, FnType fn = DefCreationFn);


    /** @copydoc EntityManager::changeArchetype(Iterator, Archetype const&, EntitySpawner::UserCreationFn_t const&) */
    template <typename FnType = DefCreationFn_t>
    EPoolCIter_t
    changeArchetype(EPoolCIter_t const& it, Archetype const& newArchetype, FnType fn = DefCreationFn);


    /// Changes the archetype of entities of oldArchetype to newArchetype
    /** Memory reserved in the spawner of oldArchetype will be reset (only if that spawner is not empty)
     * @param oldArchetype One of archetypes that were already used to spawn entities during this application (throws for other archetypes)
     * @param newArchetype Any archetype
     * @param fn A callable object that can use the Creator instance to construct the added components
     */
    template <typename FnType = DefCreationFn_t>
    std::pair<EPoolCIter_t, EPoolCIter_t>
    changeArchetype(Archetype const& oldArchetype, Archetype const& newArchetype, FnType fn = DefCreationFn);


    // ent - valid entity
    void destroy(Entity ent);


    // destroys the entity associated with the iterator and returns the next valid one
    // iterator - valid and non-end iterator
    template <typename Iterator>
    Iterator
    destroy(Iterator it);


    EPoolCIter_t destroy(EPoolCIter_t const& it);


    // destroys every entity
    void clear();


    // destroys every entity of the given archetype
    // arch - any archetype
    void clear(Archetype const& arch);


    // the next n spawn(arche, ...) calls will not require reallocation in any of the internal structures
    void prepareToSpawn(Archetype const& arch, std::size_t n);


    // TODO: tests
    void shrinkToFit(Archetype const& arch);


    // TODO: tests
    void shrinkToFit();


    template <typename... CTypes>
    void updateSelection(Selection<CTypes...>& selection);


    // ent - valid entity that owns given component (to be sure that an entity owns a component, use maskOf(ent).get(Component::Id().value))
    // returns a reference to the component associated with ent
    template <typename TComp>
    TComp& componentOf(Entity ent);


    // ent - valid entity
    // returns the CMask of the spawner that ent is currently in
    CMask const& maskOf(Entity ent) const;


    // ent - valid entity
    // returns the archetype of ent
    Archetype archetypeOf(Entity ent) const;


    // arch - one of archetypes that were already used to spawn entities during this application (throws for other archetypes)
    EntityPool_t const& entitiesOf(Archetype const& arch) const;


    bool isValid(Entity ent) const { return entList.isValid(ent); }


    std::size_t size() const { return entList.size(); }


    // arch - any archetype
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
        spawner.spawn(entList, std::move(fn));
    return { spawner.getEntities().data.end() - std::ptrdiff_t(n), spawner.getEntities().data.end() };
}

template <typename FnType>
inline void EntityManager::changeArchetype(Entity ent, Archetype const& newArchetype, FnType fn)
{
    EPP_ASSERT(entList.isValid(ent));
    EntitySpawner& spawner = getSpawner(ent);
    if (spawner.mask != newArchetype.getMask())
        getSpawner(newArchetype).moveEntityHere(ent, entList, spawner, std::move(fn));
}

template <typename FnType>
inline void EntityManager::changeArchetype(Entity ent, IdList_t toAdd, IdList_t toRemove, FnType fn)
{
    EPP_ASSERT(entList.isValid(ent));
    EntitySpawner& spawner = getSpawner(ent);
    Archetype newArchetype = spawner.makeArchetype().removeComponent(toRemove).addComponent(toAdd);
    if (spawner.mask != newArchetype.getMask())
        getSpawner(newArchetype).moveEntityHere(ent, entList, spawner, std::move(fn));
}

template <typename FnType>
inline std::pair<EntityManager::EPoolCIter_t, EntityManager::EPoolCIter_t>
EntityManager::changeArchetype(Archetype const& oldArchetype, Archetype const& newArchetype, FnType fn)
{
    EPP_ASSERT(findSpawner(oldArchetype) != spawners.end());
    EntitySpawner& oldSp = getSpawner(oldArchetype);
    EntitySpawner& newSp = getSpawner(newArchetype);
    if (oldSp.mask == newSp.mask)
        return { EntityManager::EPoolCIter_t(), EntityManager::EPoolCIter_t() };
    auto newSpSize = std::ptrdiff_t(newSp.getEntities().data.size());
    newSp.moveEntitiesHere(oldSp, entList, std::move(fn));
    return { newSp.getEntities().data.begin() + newSpSize, newSp.getEntities().data.end() };
}

template <typename Iterator, typename FnType>
inline Iterator EntityManager::changeArchetype(Iterator it, Archetype const& newArchetype, FnType fn)
{
    changeArchetype(*it, newArchetype, std::move(fn));
    if (!it.isValid())
        return ++it;
    return it;
}

template <typename FnType>
inline EntityManager::EPoolCIter_t
EntityManager::changeArchetype(EPoolCIter_t const& it, Archetype const& newArchetype, FnType fn)
{
    changeArchetype(*it, newArchetype, std::move(fn));
    return it; // same iterator (valid for vector)
}

inline void EntityManager::destroy(Entity ent)
{
    EPP_ASSERT(entList.isValid(ent));
    getSpawner(ent).destroy(ent, entList);
}

template <typename Iterator>
inline Iterator EntityManager::destroy(Iterator it)
{
    destroy(*it);
    if (!it.isValid())
        return ++it;
    return it;
}

inline EntityManager::EPoolCIter_t EntityManager::destroy(EPoolCIter_t const& it)
{
    destroy(*it);
    return it; // same iterator (valid for vector - last element was removed)
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

template <typename TComp>
inline TComp& EntityManager::componentOf(Entity ent)
{
    EPP_ASSERT(entList.isValid(ent) && getSpawner(ent).mask.get(IdOf<TComp>()));
    return *static_cast<TComp*>(getSpawner(ent).getPool(IdOf<TComp>())[entList.get(ent).poolIdx.value]);
}

inline CMask const& EntityManager::maskOf(Entity ent) const
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