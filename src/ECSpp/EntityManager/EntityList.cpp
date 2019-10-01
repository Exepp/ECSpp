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


OccupiedCell::OccupiedCell(Broken cell)
    : value(uint64_t(cell.poolIdx.value) | (uint64_t(cell.spawnerID.value) << PoolIdx::BitLength) | (uint64_t(cell.version.value) << (PoolIdx::BitLength + SpawnerID::BitLength)))
{}

PoolIdx OccupiedCell::poolIdx() const
{
    return PoolIdx(value & PoolIdx::BadValue);
}

SpawnerID OccupiedCell::spawnerID() const
{
    return SpawnerID((value & SpawnerID::Mask) >> PoolIdx::BitLength);
}

EntVersion OccupiedCell::entVersion() const
{
    return EntVersion((value & EntVersion::Mask) >> (PoolIdx::BitLength + SpawnerID::BitLength));
}

OccupiedCell::Broken OccupiedCell::asBroken() const
{
    return { poolIdx(), spawnerID(), entVersion() };
}


////////////////////////////////////////////////////////////

FreeCell::FreeCell(Broken cell) // version has the same offset as in OccupiedCell
    : value(uint64_t(cell.listIdx.value) | (uint64_t(cell.version.value) << (PoolIdx::BitLength + SpawnerID::BitLength)))
{}

ListIdx FreeCell::nextFreeListIdx() const
{
    return ListIdx(value & ListIdx::Mask);
}

EntVersion FreeCell::entVersion() const
{
    return EntVersion((value & EntVersion::Mask) >> (PoolIdx::BitLength + SpawnerID::BitLength)); // same offset as in OccupiedCell
}

FreeCell::Broken FreeCell::asBroken() const
{
    return { nextFreeListIdx(), entVersion() };
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


EntityList::EntityList(EntityList&& rhs)
{
    *this = std::move(rhs);
}

EntityList& EntityList::operator=(EntityList&& rhs)
{
    if (memory)
        free(memory);
    memory        = rhs.memory;
    reserved      = rhs.reserved;
    freeLeft      = rhs.freeLeft;
    freeIndex     = rhs.freeIndex;
    rhs.memory    = nullptr;
    rhs.reserved  = 0;
    rhs.freeLeft  = 0;
    rhs.freeIndex = ListIdx();
    return *this;
}

EntityList::~EntityList()
{
    if (memory)
        free(memory);
}

Entity EntityList::allocEntity(PoolIdx poolIdx, SpawnerID spawnerID)
{
    assert(poolIdx.value != PoolIdx::BadValue && spawnerID.value != SpawnerID::BadValue);

    if (!freeLeft)
        resize(Size_t(std::max(reserved, InitReserveSize) * GrowthFactor));
    ListIdx    idx     = freeIndex;
    EntVersion version = fcAtIdx(freeIndex).entVersion();
    freeIndex          = fcAtIdx(freeIndex).nextFreeListIdx(); // free cell's index points to the next free cell
    ocAtIdx(idx)       = OccupiedCell({ poolIdx, spawnerID, version });
    --freeLeft;

    return Entity{ idx, version };
}

void EntityList::changeEntity(Entity ent, PoolIdx index, SpawnerID spawnerID)
{
    assert(index.value != PoolIdx::BadValue && spawnerID.value != SpawnerID::BadValue);

    assert(isValid(ent));
    ocAtIdx(ent.listIdx) = OccupiedCell({ index, spawnerID, ent.version });
}

void EntityList::freeEntity(Entity ent)
{
    assert(isValid(ent));
    // increment version now, so old references wont be valid for deleted cells
    fcAtIdx(ent.listIdx) = FreeCell({ freeIndex, ent.version.nextVersion() });
    freeIndex            = ent.listIdx;
    ++freeLeft;
}

OccupiedCell const& EntityList::get(Entity ent) const
{
    assert(isValid(ent));
    return ocAtIdx(ent.listIdx);
}

bool EntityList::isValid(Entity ent) const
{
    return ent.listIdx.value < reserved && ent.version.value == ocAtIdx(ent.listIdx).entVersion().value;
}

inline OccupiedCell& EntityList::ocAtIdx(ListIdx idx)
{
    return *(OccupiedCell*)(&memory[idx.value]);
}

inline FreeCell& EntityList::fcAtIdx(ListIdx idx)
{
    return memory[idx.value];
}

inline OccupiedCell const& EntityList::ocAtIdx(ListIdx idx) const
{
    return *(OccupiedCell*)(&memory[idx.value]);
}

inline FreeCell const& EntityList::fcAtIdx(ListIdx idx) const
{
    return memory[idx.value];
}

void EntityList::prepareToFitNMore(Size_t n)
{
    if (freeLeft >= n)
        return;
    resize((reserved + (n - freeLeft)) * GrowthFactor);
}

void EntityList::resize(Size_t n)
{
    ListIdx first{ reserved };
    reserve(n);
    for (ListIdx i = first; i.value < reserved - 1; ++i.value)
        memory[i.value] = FreeCell({ ListIdx{ i.value + 1 }, EntVersion{ 0 } });
    memory[reserved - 1] = FreeCell({ ListIdx(freeIndex), EntVersion{ 0 } });
    freeIndex            = first;
}

void EntityList::reserve(Size_t n)
{
    assert(n > reserved);
    FreeCell* newMemory = (FreeCell*)std::aligned_alloc(alignof(FreeCell), n * sizeof(FreeCell));
    if (memory)
    {
        std::memcpy(newMemory, memory, reserved * sizeof(FreeCell));
        free(memory);
    }
    freeLeft += n - reserved;
    reserved = n;
    memory   = newMemory;
}
