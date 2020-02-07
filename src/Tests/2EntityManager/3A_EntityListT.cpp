#include <ECSpp/internal/EntityList.h>
#include <gtest/gtest.h>

using namespace epp;

TEST(EntityList, alloc_free_Entity)
{
    EntityList list;
    auto entity = list.allocEntity(PoolIdx(0), SpawnerId(0));

    ASSERT_EQ(entity.listIdx.value, 0u);
    ASSERT_EQ(entity.version.value, 0u);
    ASSERT_EQ(entity, entity);
    ASSERT_NE(entity, Entity());

    list.freeEntity(entity);
    entity = list.allocEntity(PoolIdx(0), SpawnerId(0));
    ASSERT_EQ(entity.listIdx.value, 0u);
    ASSERT_EQ(entity.version.value, 1u);

    entity = list.allocEntity(PoolIdx(1), SpawnerId(1));
    ASSERT_EQ(entity.listIdx.value, 1u);
    ASSERT_EQ(entity.version.value, 0u);

    for (int i = 0; i < 1024; ++i)
        list.allocEntity(PoolIdx(0), SpawnerId(0));
    auto entity2 = list.allocEntity(PoolIdx(1), SpawnerId(1));
    ASSERT_EQ(entity2.listIdx.value, 2u + 1024u); // 2 already existing + 1024 from a loop (no + 1, indexing from 0)
    ASSERT_EQ(entity2.version.value, 0u);
    ASSERT_EQ(entity.listIdx.value, 1u);
    ASSERT_EQ(entity.version.value, 0u);
    ASSERT_NE(entity, entity2);
}

TEST(EntityList, size)
{
    EntityList list;

    auto entity = list.allocEntity(PoolIdx(0), SpawnerId(0));
    ASSERT_EQ(list.size(), 1u);

    list.freeEntity(entity);
    ASSERT_EQ(list.size(), 0u);

    for (int i = 0; i < 2048; ++i)
        list.allocEntity(PoolIdx(0), SpawnerId(0));
    ASSERT_EQ(list.size(), 2048);
}

TEST(EntityList, get)
{
    EntityList list;
    auto entity = list.allocEntity(PoolIdx(123), SpawnerId(1024));
    auto broken = list.get(entity);

    ASSERT_EQ(broken.poolIdx.value, 123u);
    ASSERT_EQ(broken.spawnerId.value, 1024u);
    ASSERT_EQ(broken.version.value, 0u);

    list.freeEntity(entity);
    entity = list.allocEntity(PoolIdx(12345), SpawnerId(321));
    broken = list.get(entity);
    ASSERT_EQ(broken.poolIdx.value, 12345u);
    ASSERT_EQ(broken.spawnerId.value, 321u);
    ASSERT_EQ(broken.version.value, 1u);

    entity = list.allocEntity(PoolIdx(1), SpawnerId(2));
    broken = list.get(entity);
    ASSERT_EQ(broken.poolIdx.value, 1u);
    ASSERT_EQ(broken.spawnerId.value, 2u);
    ASSERT_EQ(broken.version.value, 0u);

    for (int i = 0; i < 512; ++i)
        list.allocEntity(PoolIdx(0), SpawnerId(0));
    auto entity2 = list.allocEntity(PoolIdx(32), SpawnerId(16));
    broken = list.get(entity2);

    for (int i = 0; i < 512; ++i)
        list.allocEntity(PoolIdx(0), SpawnerId(0));
    ASSERT_EQ(broken.poolIdx.value, 32u);
    ASSERT_EQ(broken.spawnerId.value, 16u);
    ASSERT_EQ(broken.version.value, 0u);

    broken = list.get(entity); // check if its still the same
    ASSERT_EQ(broken.poolIdx.value, 1u);
    ASSERT_EQ(broken.spawnerId.value, 2u);
    ASSERT_EQ(broken.version.value, 0u);
}

TEST(EntityList, isValid)
{
    EntityList list;
    auto entity = list.allocEntity(PoolIdx(0), SpawnerId(0));

    ASSERT_TRUE(list.isValid(entity));

    list.freeEntity(entity);
    ASSERT_FALSE(list.isValid(entity));

    list.allocEntity(PoolIdx(0), SpawnerId(0));
    ASSERT_FALSE(list.isValid(entity));

    entity = list.allocEntity(PoolIdx(0), SpawnerId(0));
    ASSERT_TRUE(list.isValid(entity));

    auto entity2 = list.allocEntity(PoolIdx(1), SpawnerId(1));
    list.freeEntity(entity2);
    for (int i = 0; i < 1024; ++i)
        list.freeEntity(list.allocEntity(PoolIdx(1), SpawnerId(1)));
    ASSERT_FALSE(list.isValid(entity2));
    ASSERT_TRUE(list.isValid(entity));
}

TEST(EntityList, changeEntity)
{
    EntityList list;
    auto entity = list.allocEntity(PoolIdx(123), SpawnerId(1024));
    auto broken = list.get(entity);

    list.changeEntity(entity, PoolIdx(321), SpawnerId(2048));
    broken = list.get(entity);
    ASSERT_EQ(broken.poolIdx.value, 321u);
    ASSERT_EQ(broken.spawnerId.value, 2048u);

    for (int i = 0; i < 512; ++i)
        list.allocEntity(PoolIdx(0), SpawnerId(0));

    entity = list.allocEntity(PoolIdx(12345), SpawnerId(321));

    for (int i = 0; i < 512; ++i)
        list.allocEntity(PoolIdx(0), SpawnerId(0));
    list.changeEntity(entity, PoolIdx(54321), SpawnerId(123));
    broken = list.get(entity);

    ASSERT_EQ(broken.poolIdx.value, 54321u);
    ASSERT_EQ(broken.spawnerId.value, 123u);
}

TEST(EntityList, fitNextN)
{
    EntityList list;

    auto entity = list.allocEntity(PoolIdx(123), SpawnerId(1024));
    list.fitNextN(1024);

    // after internal expansion:
    auto cell = list.get(entity);
    ASSERT_EQ(cell.poolIdx.value, 123u);
    ASSERT_EQ(cell.spawnerId.value, 1024u);

    for (int i = 0; i < 5000; ++i) // alloc over prepared space
        list.allocEntity(PoolIdx(0), SpawnerId(0));

    cell = list.get(entity);
    ASSERT_EQ(cell.poolIdx.value, 123u);
    ASSERT_EQ(cell.spawnerId.value, 1024u);
}

TEST(EntityList, freeAll)
{
    EntityList list;
    list.freeAll(); // on empty
    for (int i = 0; i < 1024; ++i)
        list.allocEntity(PoolIdx(0), SpawnerId(0));
    list.freeAll();
    for (int i = 0; i < 1024; ++i) {
        auto entity = list.allocEntity(PoolIdx(0), SpawnerId(0));
        ASSERT_EQ(entity.listIdx.value, i);
        ASSERT_EQ(entity.version.value, 1);
    }
    // freeAll increments version of all reserved cells, even unused
    list.freeAll();
    list.freeAll(); // but only if there is anything to free
    ASSERT_EQ(list.allocEntity(PoolIdx(0), SpawnerId(0)).version.value, 2);
}