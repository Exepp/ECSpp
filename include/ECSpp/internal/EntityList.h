#ifndef EPP_ENTITYLIST_H
#define EPP_ENTITYLIST_H

#include <ECSpp/utility/Assert.h>
#include <ECSpp/utility/IndexType.h>
#include <ECSpp/utility/Pool.h>
#include <cstring>


namespace epp {

using UniIdx = IndexType<1>;
using PoolIdx = UniIdx;
using ListIdx = UniIdx;
using SpawnerId = IndexType<2>;

struct EntVersion : public IndexType<3> {
    EntVersion() = default;
    explicit EntVersion(Val_t val) : IndexType(val) {}
    EntVersion nextVersion() const { return EntVersion(Val_t(value + 1)); }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Entity {
    ListIdx listIdx;
    EntVersion version;

    bool operator==(Entity const& rhs) const { return listIdx == rhs.listIdx && version == rhs.version; }
    bool operator!=(Entity const& rhs) const { return !(*this == rhs); }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class EntityList {
public:
    class Cell {
    public:
        struct Occupied {
            PoolIdx poolIdx;
            EntVersion version;
            SpawnerId spawnerId;
        };

        using Free = Entity;

    private:
        struct Data {
            explicit Data(Free fCell) : idx(fCell.listIdx), version(fCell.version) {}
            explicit Data(Occupied oCell) : idx(oCell.poolIdx), version(oCell.version), spawnerId(oCell.spawnerId) {}
            UniIdx idx; // when free, used as next free ListIdx; when occupied, used as PoolIdx
            EntVersion version;
            SpawnerId spawnerId;
        };

    public:
        explicit Cell(Occupied cell) : cellData(cell) {}
        explicit Cell(Free cell) : cellData(cell) {}

        Cell() = delete;
        Cell(Cell&&) = default;
        Cell(Cell const&) = default;
        Cell& operator=(Cell&&) = default;
        Cell& operator=(Cell const&) = default;

        ListIdx nextFreeListIdx() const { return cellData.idx; } // free

        PoolIdx poolIdx() const { return cellData.idx; }           // occupied
        SpawnerId spawnerId() const { return cellData.spawnerId; } // occupied
        EntVersion entVersion() const { return cellData.version; } // occupied & free

        Occupied asOccupied() const { return { cellData.idx, cellData.version, cellData.spawnerId }; }
        Free asFree() const { return { cellData.idx, cellData.version }; }

    private:
        Data cellData;
    };

public:
    EntityList() { reserve(32); }; // init size
    EntityList(EntityList&& rhs) = delete;
    EntityList& operator=(EntityList&& rhs) = delete;
    EntityList(const EntityList&) = delete;
    EntityList& operator=(const EntityList&) = delete;
    ~EntityList();

    Entity allocEntity(PoolIdx privIndex, SpawnerId spawnerId);
    void changeEntity(Entity ent, PoolIdx privIndex, SpawnerId spawnerId);
    void freeEntity(Entity ent);

    // increments versions of all currently reserved cells
    void freeAll();

    // makes sure, to fit n more elements without realloc
    void fitNextN(std::size_t n);

    bool isValid(Entity ent) const { return ent.listIdx.value < reserved && ent.version.value == data[ent.listIdx.value].entVersion().value; }

    Cell::Occupied get(Entity ent) const;

    std::size_t size() const { return reserved - freeLeft; }

private:
    void reserve(std::size_t newReserved);

private:
    Cell* data = nullptr;
    std::size_t freeLeft = 0;
    std::size_t reserved = 0;

    ListIdx freeIndex;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


inline EntityList::~EntityList()
{
    // no need to destroy
    if (data)
        operator delete[](data);
}

inline Entity EntityList::allocEntity(PoolIdx poolIdx, SpawnerId spawnerId)
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

inline void EntityList::changeEntity(Entity ent, PoolIdx index, SpawnerId spawnerId)
{
    EPP_ASSERT(index.value != PoolIdx::BadValue && spawnerId.value != SpawnerId::BadValue);
    EPP_ASSERT(isValid(ent));

    data[ent.listIdx.value] = Cell(Cell::Occupied{ index, ent.version, spawnerId });
}

inline void EntityList::freeEntity(Entity ent)
{
    EPP_ASSERT(isValid(ent));
    // increment version now, so old references wont be valid for freed cells
    data[ent.listIdx.value] = Cell(Cell::Free{ freeIndex, ent.version.nextVersion() });
    freeIndex = ent.listIdx;
    ++freeLeft;
}

inline void EntityList::freeAll()
{
    if (freeLeft == reserved)
        return;
    freeLeft = reserved;
    freeIndex = ListIdx(0);
    for (auto i = freeIndex; i.value < reserved - 1; ++i.value)
        data[i.value] = Cell(Cell::Free{ ListIdx(i.value + 1), data[i.value].entVersion().nextVersion() });
    data[reserved - 1] = Cell(Cell::Free{ ListIdx(ListIdx::BadValue), data[reserved - 1].entVersion().nextVersion() });
}

inline void EntityList::fitNextN(std::size_t n)
{
    n = SizeToFitNextN(n, reserved, freeLeft);
    if (n > reserved)
        reserve(n);
}

inline void EntityList::reserve(std::size_t newReserved)
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

inline EntityList::Cell::Occupied EntityList::get(Entity ent) const
{
    EPP_ASSERT(isValid(ent));
    return data[ent.listIdx.value].asOccupied();
}

} // namespace epp

#endif // EPP_ENTITYLIST_H