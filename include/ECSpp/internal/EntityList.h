#ifndef EPP_ENTITYLIST_H
#define EPP_ENTITYLIST_H

#include <ECSpp/internal/utility/Assert.h>
#include <ECSpp/internal/utility/IndexType.h>
#include <ECSpp/internal/utility/Pool.h>
#include <cstring>


namespace epp {

using UniIdx = IndexType<1>;
using PoolIdx = UniIdx;
using ListIdx = UniIdx;
using SpawnerId = IndexType<2>;

struct EntVersion : public IndexType<3> {
    EntVersion() = default;
    explicit EntVersion(Val_t val) : IndexType(val) {}

    /// Returns the next version
    /**
     * This value can overflow
     * @returns The next version based on the current one
     */
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


/// A vector-like freelist for entities data
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
        /// Assumes that Cell will be used as an occupied one
        /**
         * @param cell Values that the cell will take as occupied
         */
        explicit Cell(Occupied cell) : cellData(cell) {}


        /// Assumes that Cell will be used as a free one
        /**
         * @param cell Values that the cell will take as free
         */
        explicit Cell(Free cell) : cellData(cell) {}

        Cell() = delete;
        Cell(Cell&&) = default;
        Cell(Cell const&) = default;
        Cell& operator=(Cell&&) = default;
        Cell& operator=(Cell const&) = default;

        /// Assumes that Cell is now used as Free and returns an index to the next free cell
        /**
         * @returns Index to the next free cell
         */
        ListIdx nextFreeListIdx() const { return cellData.idx; }


        /// Assumes that Cell is now used as Occupied and returns the PoolIndex of the entity
        /**
         * @returns PoolIdx of the entity
         */
        PoolIdx poolIdx() const { return cellData.idx; }

        /// Assumes that Cell is now used as Occupied and returns the SpawnerId of the EntitySpawner that entity is currently in
        /**
         * @returns PoolIdx of the entity
         */
        SpawnerId spawnerId() const { return cellData.spawnerId; }


        /// Returns the version of the entity
        /**
         * Used both in free and occupied cell
         * @returns Version of the entity
         */
        EntVersion entVersion() const { return cellData.version; }


        /// Represent the current data as an Occupied cell
        /**
         * Universal cellData.idx becomes Occupied::poolIdx
         * @returns Occupied representation of this cell
         */
        Occupied asOccupied() const { return { cellData.idx, cellData.version, cellData.spawnerId }; }


        /// Represent the current data as a free cell
        /**
         * Universal cellData.idx becomes Free::listIdx, cellData.spawnerId is discarded
         * @returns Free representation of this cell
         */
        Free asFree() const { return { cellData.idx, cellData.version }; }

    private:
        Data cellData;
    };

public:
    /// Default constructor
    /** By default EntityList reserves memory for 32 elements */
    EntityList() { reserve(32); }; // init size

    EntityList(EntityList&& rhs) = delete;
    EntityList& operator=(EntityList&& rhs) = delete;
    EntityList(const EntityList&) = delete;
    EntityList& operator=(const EntityList&) = delete;


    /// Destructor
    /** Releases resources */
    ~EntityList();


    /// Allocates a unique entity
    /**
     * Entitylist may use a new entity, or a used one with changed version.
     * @param poolIdx Index of the entity in a spawner with "spawnerId" Id 
     * @param spawnerId Spawner's id
     * @returns A unique entity 
     */
    Entity allocEntity(PoolIdx poolIdx, SpawnerId spawnerId);


    /// Changes the values that describe the location of a given entity
    /**
     * Ent stays valid
     * @param ent A valid entity which data will be changed
     * @param poolIdx Index of the entity in a spawner with "spawnerId" Id 
     * @param spawnerId A new Spawner's id
     * @returns A unique entity 
     * @throws (Debug only) Throws the AssertionFailed exception if ent is invalid
     */
    void changeEntity(Entity ent, PoolIdx poolIdx, SpawnerId spawnerId);


    /// Frees a given entity
    /**
     * Increases the version of the entity and sets its cell's universal idx to the freeIndex 
     * and freeIndex is set to the value of ent.listIdx
     * @param ent A valid entity whose data will be changed
     * @throws (Debug only) Throws the AssertionFailed exception if ent is invalid
     */
    void freeEntity(Entity ent);

    /// Frees every entity
    /**
     * Increments version of every reserved cell (even the unused ones)
     */
    void freeAll();

    /// The next n allocEntity(...) calls will not require reallocation
    /** 
     * @param n Number of entities to reserve the additional memory for
     */
    void fitNextN(std::size_t n);


    /// Check whether a given entity is valid
    /**
     * @param ent Any entity
     * @returns True if the entity is valid, false otherwise
     */
    bool isValid(Entity ent) const { return ent.listIdx.value < reserved && ent.version.value == data[ent.listIdx.value].entVersion().value; }


    /// Returns the values that describe the location of a given entity
    /**
     * @param ent A valid entity
     * @returns Occupied version of the entity's cell
     * @throws (Debug only) Throws the AssertionFailed exception if ent is invalid
     */
    Cell::Occupied get(Entity ent) const;


    /// Returns the number of valid entities
    /**
     * @returns Number of valid entities
     */
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

inline void EntityList::changeEntity(Entity ent, PoolIdx poolIdx, SpawnerId spawnerId)
{
    EPP_ASSERT(poolIdx.value != PoolIdx::BadValue && spawnerId.value != SpawnerId::BadValue);
    EPP_ASSERT(isValid(ent));

    data[ent.listIdx.value] = Cell(Cell::Occupied{ poolIdx, ent.version, spawnerId });
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
    Cell* newMemory = reinterpret_cast<Cell*>(operator new[](sizeof(Cell) * std::uintptr_t(newReserved)));
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