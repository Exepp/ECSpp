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
    Entity spawn(Archetype const& arch, EntitySpawner::UserCreationFn_t const& fn = ([](EntitySpawner::Creator&&) {}));

    // first - iterator to the first of spawned entities
    // second - end iterator
    std::pair<EPoolCIter_t, EPoolCIter_t> spawn(Archetype const& arch, std::size_t n, EntitySpawner::UserCreationFn_t const& fn = ([](EntitySpawner::Creator&&) {}));


    // ent - valid entity
    // newArchetype - any archetype
    void changeArchetype(Entity ent, Archetype const& newArchetype, EntitySpawner::UserCreationFn_t const& fn = ([](EntitySpawner::Creator&&) {}));


    // changes the archetype of the entity associated with the iterator and returns the next valid one
    // iterator - valid and non-end iterator
    template <typename Iterator>
    Iterator changeArchetype(Iterator it, Archetype const& newArchetype, EntitySpawner::UserCreationFn_t const& fn = ([](EntitySpawner::Creator&&) {}))
    {
        changeArchetype(*it, newArchetype, fn);
        if (!it.isValid())
            return ++it;
        return it;
    }


    EPoolCIter_t changeArchetype(EPoolCIter_t const& it, Archetype const& newArchetype, EntitySpawner::UserCreationFn_t const& fn = ([](EntitySpawner::Creator&&) {}))
    {
        changeArchetype(*it, newArchetype, fn);
        return it; // same iterator (valid for vector)
    }


    // the next n spawn(arche, ...) calls will not require reallocation in any of the internal structures
    void prepareToSpawn(Archetype const& arch, std::size_t n);


    // ent - valid entity
    void destroy(Entity ent);


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
    CMask const& maskOf(Entity ent) const;


    // ent - valid entity
    // returns the archetype of ent
    Archetype archetypeOf(Entity ent) const;


    // arch - one of archetypes that were used to spawn entities during this application (throws for other archetypes)
    EntityPool_t const& entitiesOf(Archetype const& arch) const
    {
        EPP_ASSERT(findSpawner(arch) != spawners.end());
        return findSpawner(arch)->getEntities();
    }


    bool isValid(Entity ent) const { return entList.isValid(ent); }


    std::size_t size() const { return entList.size(); }


    // arch - any archetype
    std::size_t size(Archetype const& arch) const;

private:
    EntitySpawner& _prepareToSpawn(Archetype const& arch, std::size_t n);

    EntitySpawner& getSpawner(Archetype const& arch); // makes spawner, if missing

    EntitySpawner& getSpawner(Entity ent); // assumes ent is valid

    EntitySpawner const& getSpawner(Entity ent) const; // assumes ent is valid

    Spawners_t::iterator findSpawner(Archetype const& arch);

    Spawners_t::const_iterator findSpawner(Archetype const& arch) const;

private:
    Spawners_t spawners;

    EntityList entList;
};

} // namespace epp

#endif // ENTITYMANAGER_H