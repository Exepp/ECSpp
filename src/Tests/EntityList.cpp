#include <ECSpp/EntityManager/EntityList.h>
#include <gtest/gtest.h>

using namespace epp;

TEST(EntityList, allocEntity)
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

    for (int i = 0; i < 1023; ++i)
        list.allocEntity(PoolIdx(0), SpawnerID(0));
    auto entity2 = list.allocEntity(PoolIdx(1), SpawnerID(1));
    ASSERT_EQ(entity2.listIdx.value, 1u + 1024u);
    ASSERT_EQ(entity2.version.value, 0u);
    ASSERT_EQ(entity.listIdx.value, 1u);
    ASSERT_EQ(entity.version.value, 0u);
}

TEST(EntityList, t)
{
}