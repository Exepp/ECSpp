#include <ECSpp/internal/EntityList.h>
#include <ECSpp/utility/Pool.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <utility>

using namespace epp;

constexpr static std::size_t const InitReserveSize = 32;

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
        reserve(2 * reserved);

    ListIdx idx = freeIndex;
    EntVersion version = data[idx.value].entVersion();
    freeIndex = data[idx.value].nextFreeListIdx();
    data[idx.value] = Cell(Cell::Occupied{ poolIdx, version, spawnerId });
    --freeLeft;

    return Entity{ idx, version };
}

void EntityList::changeEntity(Entity ent, PoolIdx index, SpawnerId spawnerId)
{
    EPP_ASSERT(index.value != PoolIdx::BadValue && spawnerId.value != SpawnerId::BadValue);
    EPP_ASSERT(isValid(ent));

    data[ent.listIdx.value] = Cell(Cell::Occupied{ index, ent.version, spawnerId });
}

void EntityList::freeEntity(Entity ent)
{
    EPP_ASSERT(isValid(ent));
    // increment version now, so old references wont be valid for freed cells
    data[ent.listIdx.value] = Cell(Cell::Free{ freeIndex, ent.version.nextVersion() });
    freeIndex = ent.listIdx;
    ++freeLeft;
}

void EntityList::freeAll()
{
    if (freeLeft == reserved)
        return;
    freeLeft = reserved;
    freeIndex = ListIdx(0);
    for (auto i = freeIndex; i.value < reserved - 1; ++i.value)
        data[i.value] = Cell(Cell::Free{ ListIdx(i.value + 1), data[i.value].entVersion().nextVersion() });
    data[reserved - 1] = Cell(Cell::Free{ ListIdx(ListIdx::BadValue), data[reserved - 1].entVersion().nextVersion() });
}

void EntityList::fitNextN(std::size_t n)
{
    n = SizeToFitNextN(n, reserved, freeLeft);
    if (n > reserved)
        reserve(n);
}

void EntityList::reserve(std::size_t newReserved)
{
    EPP_ASSERT(newReserved > reserved);
    Cell* newMemory = reinterpret_cast<Cell*>(operator new[](sizeof(Cell) * std::uint64_t(newReserved)));
    if (data) {
        std::memcpy(newMemory, data, reserved * sizeof(Cell));
        operator delete[](data);
    }
    freeLeft += newReserved - reserved;
    data = newMemory;

    // init new elements
    for (ListIdx i(reserved); i.value < newReserved - 1; ++i.value)
        new (data + i.value) Cell(Cell::Free{ ListIdx(i.value + 1), EntVersion(0) });
    new (data + (newReserved - 1)) Cell(Cell::Free{ ListIdx(freeIndex), EntVersion(0) });

    freeIndex = ListIdx(reserved);
    reserved = newReserved;
}