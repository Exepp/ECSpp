#include <ECSpp/EntityManager/ASpawner.h>

using namespace epp;

ASpawner::ASpawner(Archetype archetype)
    : aliveEntities(archetype)
    , spawningEntities(std::move(archetype))
{}

ASpawner::~ASpawner()
{
    clear();
}

EntityRef ASpawner::spawn()
{
    for (auto& pool : spawningEntities.archetype.cPools)
        pool.second->alloc();

    EntityRef eRef = EntityRef(spawningEntities.ePtrs.alloc(std::make_shared<Entity>(this, spawningEntities.ePtrs.getSize())));

    notify({ EntityEvent::Type::Creation, eRef });

    return eRef;
}

void ASpawner::spawn(size_t n)
{
    for (auto& pool : spawningEntities.archetype.cPools)
        pool.second->allocN(n);

    spawningEntities.ePtrs.allocN(n);

    for (size_t i = spawningEntities.ePtrs.getSize() - n; i < spawningEntities.ePtrs.getSize(); ++i)
        spawningEntities.ePtrs[i] = std::make_shared<Entity>(this, i);
}

void ASpawner::moveExistingEntityHere(const EntityRef& eRef)
{
    if (!eRef.isValid() || eRef.entity->originSpawner == this)
        return;

    auto& entity = *eRef.entity;

    ASpawner&     originSpawner = *entity.originSpawner;
    EntitiesData& originEData   = entity.alive ? originSpawner.aliveEntities : originSpawner.spawningEntities;

    // moving components
    for (auto& pool : spawningEntities.archetype.cPools)
    {
        auto found = originEData.archetype.cPools.find(pool.first);
        if (found != originEData.archetype.cPools.end())
        {
            Component& componentToMove = (*found->second)[entity.id];
            pool.second->alloc(std::move(componentToMove));
            found->second->free(entity.id);
        }
        else
            pool.second->alloc();
    }

    // moving reference
    spawningEntities.ePtrs.alloc(entity.shared_from_this());
    originSpawner.entityKill(originEData, entity.id);
    entity.originSpawner = this;
    entity.id            = spawningEntities.ePtrs.getSize() - 1;
}

void ASpawner::acceptSpawningEntities()
{
    for (auto& pool : aliveEntities.archetype.cPools)
    {
        auto& spawnedPool = *spawningEntities.archetype.cPools[pool.first];
        for (size_t i = 0; i < spawnedPool.getSize(); i++)
            pool.second->alloc(std::move(spawnedPool[i]));
    }
    spawningEntities.archetype.clear();

    for (auto& ePtr : spawningEntities.ePtrs)
    {
        ePtr->id    = aliveEntities.ePtrs.getSize();
        ePtr->alive = true;
        aliveEntities.ePtrs.alloc(std::move(ePtr));
    }
    spawningEntities.ePtrs.clear();
}

void ASpawner::kill(const EntityRef& ref)
{
    if (ref.entity->originSpawner != this)
        return;

    notify({ EntityEvent::Type::Destruction, ref });

    EntitiesData& eData = ref.entity->alive ? aliveEntities : spawningEntities;
    EntityId_t    id    = ref.entity->id;

    for (auto& pool : eData.archetype.cPools)
        pool.second->free(id);
    entityKill(eData, id);
}

void ASpawner::entityKill(EntitiesData& eData, size_t id)
{
    eData.ePtrs[id]->invalidate();
    eData.ePtrs.free(id);

    // ePtrs already moved the last object to the freed one, now we need to update its index to the current one
    // check if any entity was moved (if "id" wasn't the last index)
    if (eData.ePtrs.getSize() && eData.ePtrs.getSize() != id)
        eData.ePtrs[id]->id = id;
}

void ASpawner::clear()
{
    aliveEntities.archetype.clear();
    spawningEntities.archetype.clear();

    for (auto& ePtr : aliveEntities.ePtrs)
        ePtr->invalidate();
    for (auto& ePtr : spawningEntities.ePtrs)
        ePtr->invalidate();

    aliveEntities.ePtrs.clear();
    spawningEntities.ePtrs.clear();
}

CPoolInterface& ASpawner::getPool(CTypeId_t cId, bool alive)
{
    return const_cast<CPoolInterface&>(((const ASpawner*)(this))->getPool(cId, alive));
}

const CPoolInterface& ASpawner::getPool(CTypeId_t cId, bool alive) const
{
    const EntitiesData& eData = alive ? aliveEntities : spawningEntities;

    auto found = eData.archetype.cPools.find(cId);
    EXC_ASSERT((found != eData.archetype.cPools.end()), std::out_of_range, "Wrong component type")
    return *found->second;
}

const Archetype& ASpawner::getArchetype() const
{
    return aliveEntities.archetype;
}

size_t ASpawner::getAliveEntitiesCount() const
{
    return aliveEntities.ePtrs.getSize();
}


size_t ASpawner::getSpawningEntitiesCount() const
{
    return spawningEntities.ePtrs.getSize();
}

EntityRef ASpawner::operator[](EntityId_t i) const
{
    EXC_ASSERT((i < aliveEntities.ePtrs.getSize()), std::out_of_range, "Wrong entity id")
    return EntityRef(aliveEntities.ePtrs[i]);
}