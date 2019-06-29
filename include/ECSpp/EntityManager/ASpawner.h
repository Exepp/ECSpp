#ifndef ASPAWNER_H
#define ASPAWNER_H

#include <ECSpp/EntityManager/Archetype.h>
#include <ECSpp/EntityManager/EntityRef.h>
#include <ECSpp/Utility/Notifier.h>

namespace epp
{

struct EntityEvent
{
    enum class Type : uint8_t
    {
        Creation,
        Destruction,
        Count
    };

    Type type;

    EntityRef entity;
};

using SpawnerNotifier_t = Notifier<EntityEvent>;


class ASpawner : public SpawnerNotifier_t
{
public:
    using EPtrPool_t = Pool<EPtr_t>;

    struct EntitiesData
    {
        EntitiesData() = default;

        EntitiesData(Archetype archetype)
            : archetype(std::move(archetype)) {}


        Archetype archetype;

        EPtrPool_t ePtrs;
    };


public:
    explicit ASpawner(Archetype archetype);

    ~ASpawner();

    ASpawner(const ASpawner&) = default;

    ASpawner(ASpawner&&) = default;

    ASpawner& operator=(const ASpawner&) = delete;

    ASpawner& operator=(ASpawner&&) = default;


    EntityRef spawn();

    void spawn(size_t n);

    void kill(const EntityRef& ref);

    // kills alive and spawning entities
    void clear();


    void moveExistingEntityHere(const EntityRef& eRef);


    // accepts recently added (since last registerNewEntities call) entities, ie. enables CGroup iterators to reach these entities
    void acceptSpawningEntities();


    CPoolInterface& getPool(CTypeId_t cId, bool alive);

    const CPoolInterface& getPool(CTypeId_t cId, bool alive) const;

    template<class T>
    Pool<T>& getPool(bool alive);

    template<class T>
    const Pool<T>& getPool(bool alive) const;


    size_t getSpawningEntitiesCount() const;

    size_t getAliveEntitiesCount() const;


    const Archetype& getArchetype() const;


    //returns ref to ALIVE entity
    EntityRef operator[](EntityId_t id) const;

private:
    void entityKill(EntitiesData& eData, size_t id);

private:
    EntitiesData aliveEntities;

    EntitiesData spawningEntities;
};


template<class T>
inline Pool<T>& ASpawner::getPool(bool alive)
{
    return static_cast<CPool<T>&>(getPool(getCTypeId<T>(), alive)).pool;
}

template<class T>
inline const Pool<T>& ASpawner::getPool(bool alive) const
{
    return static_cast<CPool<T>&>(getPool(getCTypeId<T>(), alive)).pool;
}

} // namespace epp

#endif // ASPAWNER_H