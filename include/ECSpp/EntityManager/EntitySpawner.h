#ifndef ENTITYSPAWNER_H
#define ENTITYSPAWNER_H

#include <ECSpp/EntityManager/Archetype.h>
#include <ECSpp/EntityManager/EntityList.h>
#include <ECSpp/Utility/Notifier.h>

namespace epp
{

struct EntityEvent
{
    enum class Type : uint8_t {
        Creation,
        Destruction,
        _Every // must be the last one
    };

    Type const type;

    Entity entity;
};

using SpawnerNotifier_t = Notifier<EntityEvent>;


class EntitySpawner : public SpawnerNotifier_t
{
public:
    struct IdentifiedCPool
    {
        using BasePtr_t = std::unique_ptr<CPoolBase>;

        ComponentID id;
        BasePtr_t   ptr;
    };

    using CPools_t = std::vector<IdentifiedCPool>;

    using EntityPool_t = Pool<Entity>;

public:
    EntitySpawner(Archetype arche, SpawnerID id);

    EntitySpawner(EntitySpawner&&) = delete;

    EntitySpawner& operator=(EntitySpawner&&) = delete;

    EntitySpawner& operator=(EntitySpawner const&) = delete;

    EntitySpawner(EntitySpawner const&) = delete;


    Entity create(EntityList& entList);

    void destroy(Entity ent, EntityList& entList);

    // destroys all entities, keeps allocated memory
    void clear();

    void reserveForNMore(EntityList::Size_t n);


    void moveEntityHere(Entity ent, EntityList& entList, EntitySpawner& originSpawner);


    template<class T>
    Pool<T>* getPool();

    template<class T>
    Pool<T> const* getPool() const;

    EntityPool_t const& getEntities() const;


    Archetype const& getArchetype() const;

private:
    void removeEntityFromEntityPoolOnly(PoolIdx idx, EntityList& entList);

public:
    SpawnerID const spawnerID;

private:
    Archetype archetype;

    EntityPool_t entityPool;

    CPools_t cPools;
};


template<class T>
inline Pool<T>* EntitySpawner::getPool()
{
    return const_cast<Pool<T>*>(const_cast<EntitySpawner const&>(*this).getPool<T>());
}

template<class T>
inline Pool<T> const* EntitySpawner::getPool() const
{
    ComponentID const cID = ComponentUtility::ID<T>;
    if (archetype.has(cID))
        for (auto const& pool : cPools)
            if (pool.id == cID)
                return &static_cast<CPool<T>&>(*pool.ptr).pool;
    return nullptr;
}

} // namespace epp

#endif // ENTITYSPAWNER_H