#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include <ECSpp/EntityManager/EntityCollection.h>
#include <ECSpp/EntityManager/EntityList.h>
#include <ECSpp/EntityManager/EntitySpawner.h>
#include <deque>

namespace epp
{

class EntityManager
{
    struct BitmaskPtrHash
    {
        std::size_t operator()(Bitmask const* bmask) const { return std::hash<Bitmask>()(*bmask); }
    };

    struct BitmaskPtrCompare
    {
        bool operator()(Bitmask const* lhs, Bitmask const* rhs) const { return *lhs == *rhs; }
    };

    using SpawnerIdsByBitmask_t = std::unordered_map<Bitmask const*, SpawnerID, BitmaskPtrHash, BitmaskPtrCompare>;

    using SpawnersByID_t = std::deque<EntitySpawner>;

    using SpawnerCollections_t = std::unordered_map<BitFilter, std::unique_ptr<EntityCollectionBase>>;

    using CompIDList_t = std::initializer_list<ComponentID>;

public:
    template<class... CTypes>
    Entity spawn();

    Entity spawn(Archetype const& arche);

    void destroy(Entity ent);


    template<class... CTypes>
    void addComponent(Entity ent);

    void addComponent(Entity ent, CompIDList_t cIDs);


    template<class... CTypes>
    void removeComponent(Entity ent);

    void removeComponent(Entity ent, CompIDList_t cIDs);


    // destroys all entities
    void clear();


    // registers archetype manually, so it doesn't need to be done on spawn/(add/remove component) calls
    void registerArchetype(Archetype arche);


    template<class... CTypes>
    EntityCollection<CTypes...>& requestCollection(Bitmask unwantedComponents = Bitmask());

private:
    template<class AType>
    EntitySpawner& registerArchetypeIfNew(AType&& arche);

private:
    SpawnersByID_t spawnersByIDs;

    SpawnerIdsByBitmask_t idsByBitmasks;

    EntityList entList;

    SpawnerCollections_t collections;
};


template<class... CTypes>
inline Entity EntityManager::spawn()
{
    return spawn(Archetype({ ComponentUtility::ID<CTypes>... }));
}

template<class... CTypes>
inline void EntityManager::addComponent(Entity ent)
{
    addComponent(ent, { ComponentUtility::ID<CTypes>... });
}

template<class... CTypes>
inline void EntityManager::removeComponent(Entity ent)
{
    removeComponent(ent, { ComponentUtility::ID<CTypes>... });
}

template<class... CTypes>
inline EntityCollection<CTypes...>& EntityManager::requestCollection(Bitmask unwantedComponents)
{
    BitFilter filter(Bitmask({ ComponentUtility::ID<CTypes>... }), unwantedComponents);
    auto&     collectionPtr = collections[filter];

    if (!collectionPtr)
        collectionPtr = std::unique<EntityCollection<CTypes...>>(std::move(filter));
    collectionPtr->update(spawnersByIDs);

    return static_cast<EntityCollection<CTypes...>&>(*collectionPtr);
}

template<class AType>
inline EntitySpawner& EntityManager::registerArchetypeIfNew(AType&& arche)
{
    auto found = idsByBitmasks.find(&arche.getMask());
    if (found != idsByBitmasks.end())
        return spawnersByIDs[found->second.value];

    spawnersByIDs.emplace_back(std::forward<AType>(arche), SpawnerID(uint32_t(spawnersByIDs.size())));
    idsByBitmasks.emplace(&spawnersByIDs.back().getArchetype().getMask(), spawnersByIDs.back().spawnerID);
    return spawnersByIDs.back();
}

} // namespace epp

#endif // ENTITYMANAGER_H