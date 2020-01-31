#include <ECSpp/Internal/EntityList.h>
#include <ECSpp/Utility/Pool.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <utility>

using namespace epp;

constexpr static EntityList::Size_t const InitReserveSize = 32;

////////////////////////////////////////////////////////////

EntVersion EntVersion::nextVersion() const
{
    return EntVersion(value + 1); // no need for a % (MaxValue + 1). The return value of this function will be always shifted to the left, removing the excess
}

////////////////////////////////////////////////////////////

EntityCell::EntityCell(Occupied cell)
    : value(uint64_t(cell.poolIdx.value) |
            (uint64_t(cell.spawnerId.value) << PoolIdx::BitLength) |
            (uint64_t(cell.version.value) << (PoolIdx::BitLength + SpawnerId::BitLength)))
{}

EntityCell::EntityCell(Free cell) // version has the same offset in free and occupied
    : value(uint64_t(cell.listIdx.value) |
            (uint64_t(cell.version.value) << (PoolIdx::BitLength + SpawnerId::BitLength)))
{}

ListIdx EntityCell::nextFreeListIdx() const
{
    return ListIdx(value & ListIdx::Mask);
}

PoolIdx EntityCell::poolIdx() const
{
    return PoolIdx(value & PoolIdx::Mask);
}

SpawnerId EntityCell::spawnerId() const
{
    return SpawnerId(uint32_t((value & SpawnerId::Mask) >> PoolIdx::BitLength));
}

EntVersion EntityCell::entVersion() const
{
    return EntVersion((value & EntVersion::Mask) >> (PoolIdx::BitLength + SpawnerId::BitLength));
}

EntityCell::Occupied EntityCell::asOccupied() const
{
    return { poolIdx(), spawnerId(), entVersion() };
}

EntityCell::Free EntityCell::asFree() const
{
    return { nextFreeListIdx(), entVersion() };
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EntityList::~EntityList()
{
    // no need to destroy
    if (data)
        operator delete[](data);
}

Entity EntityList::allocEntity(PoolIdx poolIdx, SpawnerId spawnerId)
{
    EPP_ASSERT(poolIdx.value != PoolIdx::BadValue && spawnerId.value != SpawnerId::BadValue);
    if (freeLeft == 0)
        if (reserved)
            reserve(2 * reserved);
        else
            reserve(InitReserveSize); // first alloc

    ListIdx idx = freeIndex;
    EntVersion version = data[idx.value].entVersion();
    freeIndex = data[idx.value].nextFreeListIdx();
    data[idx.value] = EntityCell({ poolIdx, spawnerId, version });
    --freeLeft;

    return Entity{ idx, version };
}

void EntityList::changeEntity(Entity ent, PoolIdx index, SpawnerId spawnerId)
{
    EPP_ASSERT(index.value != PoolIdx::BadValue && spawnerId.value != SpawnerId::BadValue);
    EPP_ASSERT(isValid(ent));

    data[ent.listIdx.value] = EntityCell({ index, spawnerId, ent.version });
}

void EntityList::freeEntity(Entity ent)
{
    EPP_ASSERT(isValid(ent));
    // increment version now, so old references wont be valid for deleted cells
    data[ent.listIdx.value] = EntityCell({ freeIndex, ent.version.nextVersion() });
    freeIndex = ent.listIdx;
    ++freeLeft;
}

void EntityList::freeAll()
{
    if (reserved == 0 || reserved == freeLeft)
        return;
    freeLeft = reserved;
    freeIndex = ListIdx(0);
    for (auto i = freeIndex; i.value < reserved - 1; ++i.value)
        data[i.value] = EntityCell({ ListIdx(i.value + 1), data[i.value].entVersion().nextVersion() });
    data[reserved - 1] = EntityCell({ ListIdx(ListIdx::BadValue), data[reserved - 1].entVersion().nextVersion() });
}

void EntityList::fitNextN(Size_t n)
{
    n = SizeToFitNextN(n, reserved, freeLeft);
    if (n > reserved)
        reserve(n);
}

void EntityList::reserve(Size_t newReserved)
{
    EPP_ASSERT(newReserved > reserved);
    EntityCell* newMemory = reinterpret_cast<EntityCell*>(operator new[](sizeof(EntityCell) * std::uint64_t(newReserved)));
    if (data) {
        std::memcpy(newMemory, data, reserved * sizeof(EntityCell));
        operator delete[](data);
    }
    freeLeft += newReserved - reserved;
    data = newMemory;

    // init new elements
    for (ListIdx i(reserved); i.value < newReserved - 1; ++i.value)
        new (data + i.value) EntityCell({ ListIdx(i.value + 1), EntVersion(0) });
    new (data + (newReserved - 1)) EntityCell({ ListIdx(freeIndex), EntVersion(0) });

    freeIndex = ListIdx(reserved);
    reserved = newReserved;
}