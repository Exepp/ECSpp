#include <ECSpp/EntityManager/EntityList.h>
#include <gtest/gtest.h>

using namespace epp;

TEST(EntityList, allocEntity)
{
    EntityList list;
    auto       entity = list.allocEntity(PoolIdx(0), SpawnerID(0));

    ASSERT_EQ(entity.listIdx.value, 0);
    ASSERT_EQ(entity.version.value, 0);

    list.freeEntity(entity);
    entity = list.allocEntity(PoolIdx(0), SpawnerID(0));
    ASSERT_EQ(entity.listIdx.value, 0);
    ASSERT_EQ(entity.version.value, 1);

    entity = list.allocEntity(PoolIdx(1), SpawnerID(1));
    ASSERT_EQ(entity.listIdx.value, 1);
    ASSERT_EQ(entity.version.value, 0);

    for (int i = 0; i < 1023; ++i)
        list.allocEntity(PoolIdx(0), SpawnerID(0));
    auto entity2 = list.allocEntity(PoolIdx(1), SpawnerID(1));
    ASSERT_EQ(entity2.listIdx.value, 1 + 1024);
    ASSERT_EQ(entity2.version.value, 0);
    ASSERT_EQ(entity.listIdx.value, 1);
    ASSERT_EQ(entity.version.value, 0);
}

TEST(EntityList, allocEntity)
{
    EntityList list;
    auto       entity = list.allocEntity(PoolIdx(0), SpawnerID(0));

    ASSERT_EQ(entity.listIdx.value, 0);
    ASSERT_EQ(entity.version.value, 0);

    list.freeEntity(entity);
    entity = list.allocEntity(PoolIdx(0), SpawnerID(0));
    ASSERT_EQ(entity.listIdx.value, 0);
    ASSERT_EQ(entity.version.value, 1);

    entity = list.allocEntity(PoolIdx(1), SpawnerID(1));
    ASSERT_EQ(entity.listIdx.value, 1);
    ASSERT_EQ(entity.version.value, 0);

    for (int i = 0; i < 1023; ++i)
        list.allocEntity(PoolIdx(0), SpawnerID(0));
    auto entity2 = list.allocEntity(PoolIdx(1), SpawnerID(1));
    ASSERT_EQ(entity2.listIdx.value, 1 + 1024);
    ASSERT_EQ(entity2.version.value, 0);
    ASSERT_EQ(entity.listIdx.value, 1);
    ASSERT_EQ(entity.version.value, 0);
}