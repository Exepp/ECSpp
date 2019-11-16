#include <ECSpp/EntityManager/EntityList.h>
#include <gtest/gtest.h>
using namespace epp;

TEST(EntityList, alloc_free_Entity)
{
    EntityList list;
    auto       entity = list.allocEntity(PoolIdx(0), SpawnerID(0));

    ASSERT_EQ(entity.listIdx.value, 0u);
    ASSERT_EQ(entity.version.value, 0u);

    list.freeEntity(entity);
    entity = list.allocEntity(PoolIdx(0), SpawnerID(0));
    ASSERT_EQ(entity.listIdx.value, 0u);
    ASSERT_EQ(entity.version.value, 1u);

    entity = list.allocEntity(PoolIdx(1), SpawnerID(1));
    ASSERT_EQ(entity.listIdx.value, 1u);
    ASSERT_EQ(entity.version.value, 0u);

    for (int i = 0; i < 1024; ++i)
        list.allocEntity(PoolIdx(0), SpawnerID(0));
    auto entity2 = list.allocEntity(PoolIdx(1), SpawnerID(1));
    ASSERT_EQ(entity2.listIdx.value, 2u + 1024u); // 2 already existing + 1024 from a loop (no + 1, indexing from 0)
    ASSERT_EQ(entity2.version.value, 0u);
    ASSERT_EQ(entity.listIdx.value, 1u);
    ASSERT_EQ(entity.version.value, 0u);
}

TEST(EntityList, size)
{
    EntityList list;

    auto entity = list.allocEntity(PoolIdx(0), SpawnerID(0));
    ASSERT_EQ(list.size(), 1u);

    list.freeEntity(entity);
    ASSERT_EQ(list.size(), 0u);

    for (int i = 0; i < 2048; ++i)
        list.allocEntity(PoolIdx(0), SpawnerID(0));
    ASSERT_EQ(list.size(), 2048);
}

TEST(EntityList, get)
{
    EntityList list;
    auto       entity = list.allocEntity(PoolIdx(123), SpawnerID(1024));
    auto       broken = list.get(entity);

    ASSERT_EQ(broken.poolIdx.value, 123u);
    ASSERT_EQ(broken.spawnerID.value, 1024u);
    ASSERT_EQ(broken.version.value, 0u);

    list.freeEntity(entity);
    entity = list.allocEntity(PoolIdx(12345), SpawnerID(321));
    broken = list.get(entity);
    ASSERT_EQ(broken.poolIdx.value, 12345u);
    ASSERT_EQ(broken.spawnerID.value, 321u);
    ASSERT_EQ(broken.version.value, 1u);

    entity = list.allocEntity(PoolIdx(1), SpawnerID(2));
    broken = list.get(entity);
    ASSERT_EQ(broken.poolIdx.value, 1u);
    ASSERT_EQ(broken.spawnerID.value, 2u);
    ASSERT_EQ(broken.version.value, 0u);

    for (int i = 0; i < 512; ++i)
        list.allocEntity(PoolIdx(0), SpawnerID(0));
    auto entity2 = list.allocEntity(PoolIdx(32), SpawnerID(16));
    broken       = list.get(entity2);

    for (int i = 0; i < 512; ++i)
        list.allocEntity(PoolIdx(0), SpawnerID(0));
    ASSERT_EQ(broken.poolIdx.value, 32u);
    ASSERT_EQ(broken.spawnerID.value, 16u);
    ASSERT_EQ(broken.version.value, 0u);

    broken = list.get(entity); // check if its still the same
    ASSERT_EQ(broken.poolIdx.value, 1u);
    ASSERT_EQ(broken.spawnerID.value, 2u);
    ASSERT_EQ(broken.version.value, 0u);
}

TEST(EntityList, isValid)
{
    EntityList list;
    auto       entity = list.allocEntity(PoolIdx(0), SpawnerID(0));

    ASSERT_TRUE(list.isValid(entity));

    list.freeEntity(entity);
    ASSERT_FALSE(list.isValid(entity));

    list.allocEntity(PoolIdx(0), SpawnerID(0));
    ASSERT_FALSE(list.isValid(entity));

    entity = list.allocEntity(PoolIdx(0), SpawnerID(0));
    ASSERT_TRUE(list.isValid(entity));

    auto entity2 = list.allocEntity(PoolIdx(1), SpawnerID(1));
    list.freeEntity(entity2);
    for (int i = 0; i < 1024; ++i)
        list.freeEntity(list.allocEntity(PoolIdx(1), SpawnerID(1)));
    ASSERT_FALSE(list.isValid(entity2));
    ASSERT_TRUE(list.isValid(entity));
}

TEST(EntityList, changeEntity)
{
    EntityList list;
    auto       entity = list.allocEntity(PoolIdx(123), SpawnerID(1024));
    auto       broken = list.get(entity);

    list.changeEntity(entity, PoolIdx(321), SpawnerID(2048));
    broken = list.get(entity);
    ASSERT_EQ(broken.poolIdx.value, 321u);
    ASSERT_EQ(broken.spawnerID.value, 2048u);

    for (int i = 0; i < 512; ++i)
        list.allocEntity(PoolIdx(0), SpawnerID(0));

    entity = list.allocEntity(PoolIdx(12345), SpawnerID(321));

    for (int i = 0; i < 512; ++i)
        list.allocEntity(PoolIdx(0), SpawnerID(0));
    list.changeEntity(entity, PoolIdx(54321), SpawnerID(123));
    broken = list.get(entity);

    ASSERT_EQ(broken.poolIdx.value, 54321u);
    ASSERT_EQ(broken.spawnerID.value, 123u);
}

TEST(EntityList, prepareToFitNMore)
{
    EntityList list;

    auto entity = list.allocEntity(PoolIdx(123), SpawnerID(1024));
    list.prepareToFitNMore(1024);

    // after internal expansion:
    auto cell = list.get(entity);
    ASSERT_EQ(cell.poolIdx.value, 123u);
    ASSERT_EQ(cell.spawnerID.value, 1024u);

    for (int i = 0; i < 5000; ++i) // alloc over prepared space
        list.allocEntity(PoolIdx(0), SpawnerID(0));

    cell = list.get(entity);
    ASSERT_EQ(cell.poolIdx.value, 123u);
    ASSERT_EQ(cell.spawnerID.value, 1024u);
}

TEST(EntityList, freeAll)
{
    EntityList list;
    list.freeAll(); // on empty
    for (int i = 0; i < 1024; ++i)
        list.allocEntity(PoolIdx(0), SpawnerID(0));
    list.freeAll();
    for (int i = 0; i < 1024; ++i)
    {
        auto entity = list.allocEntity(PoolIdx(0), SpawnerID(0));
        ASSERT_EQ(entity.listIdx.value, i);
        ASSERT_EQ(entity.version.value, 1);
    }
    // freeAll increments version of all reserved cells, even unused
    ASSERT_EQ(list.allocEntity(PoolIdx(0), SpawnerID(0)).version.value, 1);
}