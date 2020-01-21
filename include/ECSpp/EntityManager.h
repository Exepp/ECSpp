#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include <ECSpp/Internal/EntityCollection.h>
#include <ECSpp/Internal/EntityList.h>
#include <ECSpp/Internal/EntitySpawner.h>
#include <deque>
#include <unordered_map>

namespace epp {

// TODO: redo the tests
class EntityManager {
    using Spawners_t = std::deque<EntitySpawner>; // deque, to keep collections' references valid

    using EntityPool_t = EntitySpawner::EntityPool_t;

    using EPoolCIter_t = EntitySpawner::EntityPool_t::Container_t::const_iterator;

    using IdList_t = decltype(IdOfL<>());

public:
    Entity spawn(Archetype const& arch, EntitySpawner::UserCreationFn_t const& fn = ([](EntityCreator&&) {}));

    // first - iterator to the first of spawned entities
    // second - end iterator
    std::pair<EPoolCIter_t, EPoolCIter_t> spawn(Archetype const& arch, std::size_t n, EntitySpawner::UserCreationFn_t const& fn = ([](EntityCreator&&) {}));


    // ent - valid entity
    // newArchetype - any archetype
    void changeArchetype(Entity ent, Archetype const& newArchetype, EntitySpawner::UserCreationFn_t const& fn = ([](EntityCreator&&) {}));


    /// Changes the archetype of ent, removing components from toRemove list and adding the ones from toAdd list
    /**
     * There are no overloaded versions of this function (for iterators) on purpose - it is more optimal to create a separate archetype for those situations
     * @param ent A valid entity
     * @param toAdd Components that will be added to ent (or kept it these are already there). toAdd has a priority over toRemove
     * @param toRemove Components that will be removed from ent (except for the ones specified in toAdd)
     * @param fn A callable object that can use the Creator instance to construct the added components
     */
    void changeArchetype(Entity ent, IdList_t toAdd, IdList_t toRemove = IdOfL<>(), EntitySpawner::UserCreationFn_t const& fn = ([](EntityCreator&&) {}));


    /// Changes the archetype of the entity associated with the iterator and returns the next valid one
    /**
     * There are no overloaded versions of this function (for iterators) on purpose - it is more optimal to create a separate archetype for those situations
     * @param it A valid and non-end iterator
     * @param newArchetype Any archetype - a new archetype of the entity associated with the iterator  
     * @param fn A callable object that can use the Creator instance to construct the added components
     */
    template <typename Iterator>
    Iterator changeArchetype(Iterator it, Archetype const& newArchetype, EntitySpawner::UserCreationFn_t const& fn = ([](EntityCreator&&) {}))
    {
        changeArchetype(*it, newArchetype, fn);
        if (!it.isValid())
            return ++it;
        return it;
    }

    /** @copydoc EntityManager::changeArchetype(Iterator, Archetype const&, EntitySpawner::UserCreationFn_t const&) */
    EPoolCIter_t changeArchetype(EPoolCIter_t const& it, Archetype const& newArchetype, EntitySpawner::UserCreationFn_t const& fn = ([](EntityCreator&&) {}))
    {
        changeArchetype(*it, newArchetype, fn);
        return it; // same iterator (valid for vector)
    }

    /// Changes the archetype of entities of oldArchetype to newArchetype
    /** Memory reserved in the spawner of oldArchetype will be reset (only if that spawner is not empty)
     * @param oldArchetype One of archetypes that were already used to spawn entities during this application (throws for other archetypes)
     * @param newArchetype Any archetype
     * @param fn A callable object that can use the Creator instance to construct the added components
     */
    std::pair<EPoolCIter_t, EPoolCIter_t> changeArchetype(Archetype const& oldArchetype, Archetype const& newArchetype, EntitySpawner::UserCreationFn_t const& fn = ([](EntityCreator&&) {}));


    // ent - valid entity
    void destroy(Entity ent)
    {
        EPP_ASSERT(entList.isValid(ent));
        getSpawner(ent).destroy(ent, entList);
    }

    // destroys the entity associated with the iterator and returns the next valid one
    // iterator - valid and non-end iterator
    template <typename Iterator>
    Iterator destroy(Iterator it)
    {
        destroy(*it);
        if (!it.isValid())
            return ++it;
        return it;
    }


    EPoolCIter_t destroy(EPoolCIter_t const& it)
    {
        destroy(*it);
        return it; // same iterator (valid for vector)
    }


    // destroys every entity
    void clear();


    // destroys every entity of the given archetype
    // arch - any archetype
    void clear(Archetype const& arch);

    // the next n spawn(arche, ...) calls will not require reallocation in any of the internal structures
    void prepareToSpawn(Archetype const& arch, std::size_t n);


    template <typename... CTypes>
    void updateCollection(EntityCollection<CTypes...>& collection)
    {
        for (; collection.checkedSpawnersCount < spawners.size(); ++collection.checkedSpawnersCount)
            collection.addSpawnerIfMeetsRequirements(spawners[collection.checkedSpawnersCount]);
    }


    // ent - valid entity that owns given component (to be sure that an entity owns a component, use maskOf(ent).get(Component::Id().value))
    // returns a reference to the component associated with ent
    template <typename TComp>
    TComp& componentOf(Entity ent)
    {
        EPP_ASSERT(entList.isValid(ent) && getSpawner(ent).mask.get(IdOf<TComp>()));
        return *static_cast<TComp*>(getSpawner(ent).getPool(IdOf<TComp>())[entList.get(ent).poolIdx.value]);
    }


    // ent - valid entity
    // returns the CMask of the spawner that ent is currently in
    CMask const& maskOf(Entity ent) const
    {
        EPP_ASSERT(entList.isValid(ent));
        return getSpawner(ent).mask;
    }

    // ent - valid entity
    // returns the archetype of ent
    Archetype archetypeOf(Entity ent) const
    {
        EPP_ASSERT(entList.isValid(ent));
        return getSpawner(ent).makeArchetype();
    }

    // arch - one of archetypes that were already used to spawn entities during this application (throws for other archetypes)
    EntityPool_t const& entitiesOf(Archetype const& arch) const
    {
        EPP_ASSERT(findSpawner(arch) != spawners.end());
        return findSpawner(arch)->getEntities();
    }

    bool isValid(Entity ent) const { return entList.isValid(ent); }

    std::size_t size() const { return entList.size(); }

    // arch - any archetype
    std::size_t size(Archetype const& arch) const
    {
        if (auto found = findSpawner(arch); found != spawners.end())
            return found->getEntities().data.size();
        return 0;
    }

private:
    EntitySpawner& _prepareToSpawn(Archetype const& arch, std::size_t n);

    EntitySpawner& getSpawner(Archetype const& arch)
    {
        if (auto found = findSpawner(arch); found != spawners.end())
            return *found;
        return spawners.emplace_back(SpawnerId(std::uint32_t(spawners.size())), arch); // if not found, make one
    }

    EntitySpawner& getSpawner(Entity ent) { return spawners[entList.get(ent).spawnerId.value]; }

    EntitySpawner const& getSpawner(Entity ent) const { return spawners[entList.get(ent).spawnerId.value]; }

    Spawners_t::iterator findSpawner(Archetype const& arch)
    {
        return std::find_if(spawners.begin(), spawners.end(), [mask = arch.getMask()](EntitySpawner const& spawner) { return spawner.mask == mask; });
    }

    Spawners_t::const_iterator findSpawner(Archetype const& arch) const
    {
        return std::find_if(spawners.begin(), spawners.end(), [mask = arch.getMask()](EntitySpawner const& spawner) { return spawner.mask == mask; });
    }

private:
    Spawners_t spawners;

    EntityList entList;
};

} // namespace epp

#endif // ENTITYMANAGER_H