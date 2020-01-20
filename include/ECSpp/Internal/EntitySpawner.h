#ifndef ENTITYSPAWNER_H
#define ENTITYSPAWNER_H

#include <ECSpp/Internal/Archetype.h>
#include <ECSpp/Internal/CPool.h>
#include <ECSpp/Internal/EntityList.h>
#include <ECSpp/Utility/Notifier.h>
#include <ECSpp/Utility/Pool.h>
#include <functional>

namespace epp {

struct EntityEvent {
    enum class Type : uint8_t {
        Creation,
        Destruction,
        JoinedCollection, // when existing entity's archetype changes to the ones, that are accepted by this collection
        LeftCollection,   // when entity gets/loses component and no longer passes through the collection's filter
        _Every            // must be the last one
    };

    Type const type;

    Entity entity;
};

using SpawnerNotifier = Notifier<EntityEvent>;


class EntitySpawner : public SpawnerNotifier {
public:
    class Creator {
    public:
        /** 
         * Consecutive calls to construct already constructed components will result in their reconstruction (destruction -> construction)
        */
        template <typename CType, typename... Args>
        CType& construct(Args&&... args)
        {
            ComponentId cId = IdOf<CType>();
            EPP_ASSERT(spawner.mask.get(cId));
            CType* component = static_cast<CType*>(spawner.getPool(cId)[idx.value]);
            if (constructed.get(cId))
                component->~CType();
            new (component) CType(std::forward<Args>(args)...);
            constructed.set(cId);

            return *component;
        }

    private:
        Creator(EntitySpawner& spawner, PoolIdx idx, CMask const& constructed = CMask());
        Creator(Creator&& rVal) = delete;
        Creator(Creator const&) = delete;
        Creator& operator=(Creator const&) = delete;
        Creator& operator=(Creator&&) = delete;
        ~Creator(); // constructs components that the user didnt construct himself

    private:
        EntitySpawner& spawner;
        PoolIdx const idx;
        CMask constructed;

        friend class EntitySpawner;
    };

    using UserCreationFn_t = std::function<void(Creator&&)>;

    using EntityPool_t = Pool<Entity>;

private:
    using CPools_t = std::vector<CPool>;

public:
    EntitySpawner(SpawnerId id, Archetype const& arch);

    EntitySpawner(EntitySpawner&&) = delete;

    EntitySpawner& operator=(EntitySpawner&&) = delete;

    EntitySpawner& operator=(EntitySpawner const&) = delete;

    EntitySpawner(EntitySpawner const&) = delete;


    Entity spawn(EntityList& entList, UserCreationFn_t const& fn);

    void destroy(Entity ent, EntityList& entList);

    // destroys all entities, keeps allocated memory
    void clear(EntityList& entList);

    void clear(); // for clear-all (no need to free indices, all will be freed)

    // makes sure, to fit n more elements without realloc
    void fitNextN(EntityList::Size_t n);


    void moveEntityHere(Entity ent, EntityList& entList, EntitySpawner& originSpawner, UserCreationFn_t const& fn);


    CPool& getPool(ComponentId cId)
    {
        EPP_ASSERT(mask.get(cId));
        return *std::find_if(cPools.begin(), cPools.end(), [cId](CPool const& pool) { return pool.getCId() == cId; });
    }

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

} // namespace epp

#endif // ENTITYSPAWNER_H