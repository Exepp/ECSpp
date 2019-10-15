#include <ECSpp/EntityManager/EntityList.h>
#include <algorithm>
#include <assert.h>
#include <cstring>
#include <utility>

using namespace epp;

constexpr static EntityList::Size_t const InitReserveSize = 32;
constexpr static double const             GrowthFactor    = 1.61803398875;

////////////////////////////////////////////////////////////

EntVersion EntVersion::nextVersion() const
{
    return EntVersion((value + 1) % BadValue);
}

////////////////////////////////////////////////////////////

EntityCell::EntityCell(Occupied cell)
    : value(uint64_t(cell.poolIdx.value) |
            (uint64_t(cell.spawnerID.value) << PoolIdx::BitLength) |
            (uint64_t(cell.version.value) << (PoolIdx::BitLength + SpawnerID::BitLength)))
{}

EntityCell::EntityCell(Free cell)
    : value(uint64_t(cell.listIdx.value) | // version has the same offset as in EntityCell
            (uint64_t(cell.version.value) << (PoolIdx::BitLength + SpawnerID::BitLength)))
{}

ListIdx EntityCell::nextFreeListIdx() const
{
    return ListIdx(value & ListIdx::Mask);
}

PoolIdx EntityCell::poolIdx() const
{
    return PoolIdx(value & PoolIdx::Mask);
}

SpawnerID EntityCell::spawnerID() const
{
    return SpawnerID(uint32_t((value & SpawnerID::Mask) >> PoolIdx::BitLength));
}

EntVersion EntityCell::entVersion() const
{
    return EntVersion((value & EntVersion::Mask) >> (PoolIdx::BitLength + SpawnerID::BitLength));
}

EntityCell::Occupied EntityCell::asOccupied() const
{
    return { poolIdx(), spawnerID(), entVersion() };
}

EntityCell::Free EntityCell::asFree() const
{
    return { nextFreeListIdx(), entVersion() };
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EntityList::~EntityList()
{
    if (memory)
        operator delete[](memory);
}

Entity EntityList::allocEntity(PoolIdx poolIdx, SpawnerID spawnerID)
{
    assert(poolIdx.value != PoolIdx::BadValue && spawnerID.value != SpawnerID::BadValue);
    if (!freeLeft)
        resize(Size_t(std::max(reserved, InitReserveSize) * GrowthFactor));

    ListIdx    idx     = freeIndex;
    EntVersion version = memory[idx.value].entVersion();
    freeIndex          = memory[idx.value].nextFreeListIdx();
    memory[idx.value]  = EntityCell({ poolIdx, spawnerID, version });
    --freeLeft;

    return Entity{ idx, version };
}

void EntityList::changeEntity(Entity ent, PoolIdx index, SpawnerID spawnerID)
{
    assert(index.value != PoolIdx::BadValue && spawnerID.value != SpawnerID::BadValue);
    assert(isValid(ent));

    memory[ent.listIdx.value] = EntityCell({ index, spawnerID, ent.version });
}

void EntityList::freeEntity(Entity ent)
{
    assert(isValid(ent));
    // increment version now, so old references wont be valid for deleted cells
    memory[ent.listIdx.value] = EntityCell({ freeIndex, ent.version.nextVersion() });
    freeIndex                 = ent.listIdx;
    ++freeLeft;
}

EntityCell::Occupied EntityList::get(Entity ent) const
{
    assert(isValid(ent));
    return memory[ent.listIdx.value].asOccupied();
}

bool EntityList::isValid(Entity ent) const
{
    return ent.listIdx.value < reserved && ent.version.value == memory[ent.listIdx.value].entVersion().value; // UB here, casting  to EntityCell
}

void EntityList::prepareToFitNMore(Size_t n)
{
    if (freeLeft >= n)
        return;
    resize(Size_t((reserved + (n - freeLeft)) * GrowthFactor));
}

void EntityList::resize(Size_t n)
{
    ListIdx first{ reserved };
    reserve(n);
    for (ListIdx i = first; i.value < reserved - 1; ++i.value)
        memory[i.value] = EntityCell({ ListIdx{ i.value + 1 }, EntVersion{ 0 } });
    memory[reserved - 1] = EntityCell({ ListIdx(freeIndex), EntVersion{ 0 } });
    freeIndex            = first;
}

void EntityList::reserve(Size_t n)
{
    assert(n > reserved);
    EntityCell* newMemory = reinterpret_cast<EntityCell*>(operator new[](n * sizeof(EntityCell)));
    if (memory)
    {
        std::memcpy(newMemory, memory, reserved * sizeof(EntityCell));
        operator delete[](memory);
    }
    freeLeft += n - reserved;
    reserved = n;
    memory   = newMemory;
}