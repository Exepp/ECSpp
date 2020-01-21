#ifndef ENTITYLIST_H
#define ENTITYLIST_H

#include <ECSpp/Utility/Assert.h>
#include <cstddef>
#include <cstdint>

namespace epp {

struct PoolIdx {
    PoolIdx() = default;
    explicit PoolIdx(uint32_t val) : value(val) {}
    constexpr static std::size_t const BitLength = 26; // max 32
    constexpr static uint32_t const BadValue = (1ull << BitLength) - 1;
    constexpr static uint64_t const Mask = uint64_t(BadValue);

    uint32_t value = BadValue;
};

struct SpawnerId {
    SpawnerId() = default;
    explicit SpawnerId(uint32_t val) : value(val) {}
    constexpr static std::size_t const BitLength = 12; // max 32
    constexpr static uint32_t const BadValue = (1ull << BitLength) - 1;
    constexpr static uint64_t const Mask = uint64_t(BadValue) << PoolIdx::BitLength;

    uint32_t value = BadValue;
};

struct EntVersion {
    EntVersion() = default;
    explicit EntVersion(uint32_t val) : value(val) {}
    constexpr static std::size_t const BitLength = 64 - (PoolIdx::BitLength + SpawnerId::BitLength);
    constexpr static uint32_t const MaxValue = (1ull << BitLength) - 1;
    constexpr static uint64_t const Mask = uint64_t(MaxValue) << (PoolIdx::BitLength + SpawnerId::BitLength);

    EntVersion nextVersion() const;

    uint32_t value = MaxValue;
};

static_assert(EntVersion::BitLength != 0 && PoolIdx::BitLength + SpawnerId::BitLength + EntVersion::BitLength == 64);


struct ListIdx {
    ListIdx() = default;
    explicit ListIdx(uint32_t val) : value(val) {}
    constexpr static std::size_t const BitLength = 32;
    constexpr static uint32_t const BadValue = (1ull << BitLength) - 1;
    constexpr static uint64_t const Mask = uint64_t(BadValue);

    uint32_t value = BadValue;
};


struct Entity {
    ListIdx listIdx;
    EntVersion version;

    bool operator==(Entity const& rhs) const { return listIdx.value == rhs.listIdx.value && version.value == rhs.version.value; }
    bool operator!=(Entity const& rhs) const { return !(*this == rhs); }
};


class EntityCell {
public:
    struct Occupied {
        PoolIdx poolIdx;
        SpawnerId spawnerId;
        EntVersion version;
    };

    using Free = Entity;

public:
    explicit EntityCell(Occupied cell);
    explicit EntityCell(Free cell);

    ListIdx nextFreeListIdx() const; // free

    PoolIdx poolIdx() const;       // occupied
    SpawnerId spawnerId() const;   // occupied
    EntVersion entVersion() const; // occupied & free

    Occupied asOccupied() const;
    Free asFree() const;

private:
    uint64_t value;
};

static_assert(sizeof(Entity) == sizeof(EntityCell) && sizeof(Entity) == sizeof(uint64_t));


class EntityList {
public:
    using Size_t = uint32_t;

public:
    EntityList() = default;

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
    void fitNextN(Size_t n);

    bool isValid(Entity ent) const { return ent.listIdx.value < reserved && ent.version.value == data[ent.listIdx.value].entVersion().value; }

    EntityCell::Occupied get(Entity ent) const
    {
        EPP_ASSERT(isValid(ent));
        return data[ent.listIdx.value].asOccupied();
    }

    Size_t size() const { return reserved - freeLeft; }

private:
    void reserve(Size_t newReserved);

private:
    EntityCell* data = nullptr;
    Size_t freeLeft = 0;
    Size_t reserved = 0;
    ListIdx freeIndex;
};

} // namespace epp

#endif // ENTITYLIST_H