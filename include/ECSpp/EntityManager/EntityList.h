#ifndef ENTITYLIST_H
#define ENTITYLIST_H

#include <cstddef>
#include <cstdint>

namespace epp
{

struct PoolIdx
{
    PoolIdx() = default;
    explicit PoolIdx(uint32_t value)
        : value(value)
    {}
    constexpr static std::size_t const BitLength = 26; // max 32
    constexpr static uint32_t const    BadValue  = (1ull << BitLength) - 1;
    constexpr static uint64_t const    Mask      = uint64_t(BadValue);

    uint32_t value = BadValue;
};

struct SpawnerID
{
    SpawnerID() = default;
    explicit SpawnerID(uint32_t value)
        : value(value)
    {}
    constexpr static std::size_t const BitLength = 12; // max 32
    constexpr static uint32_t const    BadValue  = (1ull << BitLength) - 1;
    constexpr static uint64_t const    Mask      = uint64_t(BadValue) << PoolIdx::BitLength;

    uint32_t value = BadValue;
};

struct EntVersion
{
    EntVersion() = default;
    explicit EntVersion(uint32_t value)
        : value(value)
    {}
    constexpr static std::size_t const BitLength = 64 - (PoolIdx::BitLength + SpawnerID::BitLength);
    constexpr static uint32_t const    BadValue  = (1ull << BitLength) - 1;
    constexpr static uint64_t const    Mask      = uint64_t(BadValue) << (PoolIdx::BitLength + SpawnerID::BitLength);

    EntVersion nextVersion() const;

    uint32_t value = BadValue;
};

static_assert(EntVersion::BitLength != 0 && PoolIdx::BitLength + SpawnerID::BitLength + EntVersion::BitLength == 64);


struct ListIdx
{
    ListIdx() = default;
    explicit ListIdx(uint32_t value)
        : value(value)
    {}
    constexpr static std::size_t const BitLength = 32;
    constexpr static uint32_t const    BadValue  = (1ull << BitLength) - 1;
    constexpr static uint64_t const    Mask      = uint64_t(BadValue);

    uint32_t value = BadValue;
};


struct Entity
{
    ListIdx    listIdx;
    EntVersion version;
};


class OccupiedCell
{
public:
    struct Broken
    {
        PoolIdx    poolIdx;
        SpawnerID  spawnerID;
        EntVersion version;
    };

public:
    explicit OccupiedCell(Broken cell);

    PoolIdx    poolIdx() const;
    SpawnerID  spawnerID() const;
    EntVersion entVersion() const;

    Broken asBroken() const;

private:
    uint64_t value;
};

static_assert(sizeof(Entity) == sizeof(OccupiedCell) && sizeof(Entity) == sizeof(uint64_t));


class FreeCell
{
public:
    using Broken = Entity;

public:
    explicit FreeCell(Broken cell);

    ListIdx    nextFreeListIdx() const;
    EntVersion entVersion() const;

    Broken asBroken() const;

private:
    uint64_t value;
};


class EntityList
{
public:
    using Size_t = uint32_t;

public:
    EntityList() = default;

    EntityList(EntityList&& rhs);

    EntityList& operator=(EntityList&& rhs);

    EntityList(const EntityList&) = delete;

    EntityList& operator=(const EntityList&) = delete;

    ~EntityList();


    Entity allocEntity(PoolIdx privIndex, SpawnerID spawnerID);

    void changeEntity(Entity ent, PoolIdx privIndex, SpawnerID spawnerID);

    void freeEntity(Entity ent);


    void prepareToFitNMore(Size_t n);


    bool isValid(Entity ent) const;


    OccupiedCell const& get(Entity ent) const;

private:
    OccupiedCell& ocAtIdx(ListIdx idx);

    FreeCell& fcAtIdx(ListIdx idx);

    OccupiedCell const& ocAtIdx(ListIdx idx) const;

    FreeCell const& fcAtIdx(ListIdx idx) const;


    void resize(Size_t n);

    void reserve(Size_t n);

protected:
    FreeCell* memory   = nullptr;
    Size_t    freeLeft = 0;
    Size_t    reserved = 0;
    ListIdx   freeIndex;
};

} // namespace epp

#endif // ENTITYLIST_H