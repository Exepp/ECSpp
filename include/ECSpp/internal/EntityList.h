#ifndef ENTITYLIST_H
#define ENTITYLIST_H

#include <ECSpp/utility/Assert.h>
#include <ECSpp/utility/IndexType.h>
#include <cstddef>
#include <cstdint>

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


struct Entity {
    ListIdx listIdx;
    EntVersion version;

    bool operator==(Entity const& rhs) const { return listIdx == rhs.listIdx && version == rhs.version; }
    bool operator!=(Entity const& rhs) const { return !(*this == rhs); }
};


class EntityList {

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
    EntityList() { reserve(32); };
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

    Cell::Occupied get(Entity ent) const
    {
        EPP_ASSERT(isValid(ent));
        return data[ent.listIdx.value].asOccupied();
    }

    std::size_t size() const { return reserved - freeLeft; }

private:
    void reserve(std::size_t newReserved);

private:
    Cell* data = nullptr;
    std::size_t freeLeft = 0;
    std::size_t reserved = 0;

    ListIdx freeIndex;
};

} // namespace epp

#endif // ENTITYLIST_H