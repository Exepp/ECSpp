#include <ECSpp/internal/EntityList.h>
#include <gtest/gtest.h>

using namespace epp;

static void TestEntity(Entity const& entity, Entity const& correct, EntityList::Cell::Occupied const& correctCell, EntityList const& eList, bool valid)
{
    ASSERT_FALSE(eList.isValid(Entity()));

    ASSERT_EQ(entity.listIdx, correct.listIdx);
    ASSERT_EQ(entity.version, correct.version);
    ASSERT_EQ(entity, entity);
    ASSERT_EQ(entity, correct);
    ASSERT_EQ(eList.isValid(entity), valid);
    if (!valid)
        return;

    auto cell = eList.get(entity);
    ASSERT_EQ(cell.poolIdx, correctCell.poolIdx);
    ASSERT_EQ(cell.spawnerId, correctCell.spawnerId);
    ASSERT_EQ(cell.version, correctCell.version);
}

TEST(EntityList, DefaultConstr)
{
    EntityList list;
    ASSERT_EQ(list.size(), 0);
}

TEST(EntityList, AllocEntity)
{
    EntityList list;
    auto entity = list.allocEntity(PoolIdx(0), SpawnerId(0));
    TestEntity(entity, { ListIdx(0), EntVersion(0) }, { PoolIdx(0), EntVersion(0), SpawnerId(0) }, list, true);
    ASSERT_EQ(list.size(), 1);

    entity = list.allocEntity(PoolIdx(0), SpawnerId(0));
    TestEntity(entity, { ListIdx(1), EntVersion(0) }, { PoolIdx(0), EntVersion(0), SpawnerId(0) }, list, true);

    entity = list.allocEntity(PoolIdx(1), SpawnerId(1));
    TestEntity(entity, { ListIdx(2), EntVersion(0) }, { PoolIdx(1), EntVersion(0), SpawnerId(1) }, list, true);
    ASSERT_EQ(list.size(), 3);

    for (int i = 0; i < 1024; ++i)
        list.allocEntity(PoolIdx(0), SpawnerId(0));
    auto entity2 = list.allocEntity(PoolIdx(1), SpawnerId(1));
    TestEntity(entity2, { ListIdx(3 + 1024), EntVersion(0) }, { PoolIdx(1), EntVersion(0), SpawnerId(1) }, list, true);
    ASSERT_NE(entity, entity2);
    ASSERT_EQ(list.size(), 3 + 1024 + 1);
}

TEST(EntityList, ChangeEntity)
{
    EntityList list;
    auto entity = list.allocEntity(PoolIdx(123), SpawnerId(1024));
    list.changeEntity(entity, PoolIdx(321), SpawnerId(2048));
    TestEntity(entity, { ListIdx(0), EntVersion(0) }, { PoolIdx(321), EntVersion(0), SpawnerId(2048) }, list, true);
    ASSERT_EQ(list.size(), 1);

    std::vector<Entity> ents;
    for (int i = 0; i < 512; ++i)
        ents.push_back(list.allocEntity(PoolIdx(0), SpawnerId(0)));

    entity = list.allocEntity(PoolIdx(12345), SpawnerId(321));

    for (int i = 0; i < 512; ++i)
        list.allocEntity(PoolIdx(0), SpawnerId(0));

    list.changeEntity(entity, PoolIdx(54321), SpawnerId(123));
    TestEntity(entity, { ListIdx(1 + 512), EntVersion(0) }, { PoolIdx(54321), EntVersion(0), SpawnerId(123) }, list, true);

    for (int i = 0; i < ents.size(); ++i)
        list.changeEntity(ents[i], PoolIdx(54321), SpawnerId(123));
    for (int i = 0; i < ents.size(); ++i)
        TestEntity(ents[i], { ListIdx(1 + i), EntVersion(0) }, { PoolIdx(54321), EntVersion(0), SpawnerId(123) }, list, true);
    ASSERT_EQ(list.size(), 1 + 512 + 1 + 512);
}

TEST(EntityList, FreeEntity)
{
    EntityList list;
    auto entity = list.allocEntity(PoolIdx(0), SpawnerId(0));
    list.freeEntity(entity);
    TestEntity(entity, { ListIdx(0), EntVersion(0) }, {}, list, false);
    ASSERT_EQ(list.size(), 0);

    entity = list.allocEntity(PoolIdx(0), SpawnerId(0));
    TestEntity(entity, { ListIdx(0), EntVersion(1) }, { PoolIdx(0), EntVersion(1), SpawnerId(0) }, list, true);
    ASSERT_EQ(list.size(), 1);

    list.changeEntity(entity, PoolIdx(321), SpawnerId(123));
    list.freeEntity(entity);
    list.allocEntity(PoolIdx(0), SpawnerId(0));                         // a new one in place of "entity"
    TestEntity(entity, { ListIdx(0), EntVersion(1) }, {}, list, false); // still invalid
    ASSERT_EQ(list.size(), 1);

    entity = list.allocEntity(PoolIdx(1), SpawnerId(1));
    TestEntity(entity, { ListIdx(1), EntVersion(0) }, { PoolIdx(1), EntVersion(0), SpawnerId(1) }, list, true);
    ASSERT_EQ(list.size(), 2);

    std::vector<Entity> ents;
    for (int i = 0; i < 1024; ++i)
        ents.push_back(list.allocEntity(PoolIdx(0), SpawnerId(0)));
    for (int i = 0; i < ents.size(); ++i) {
        TestEntity(ents[i], { ListIdx(2 + i), EntVersion(0) }, { PoolIdx(0), EntVersion(0), SpawnerId(0) }, list, true);
        list.freeEntity(ents[i]);
        TestEntity(ents[i], { ListIdx(2 + i), EntVersion(0) }, {}, list, false);
    }
    ASSERT_EQ(list.size(), 2);
    for (int i = 0; i < ents.size(); ++i) {
        TestEntity(list.allocEntity(PoolIdx(23), SpawnerId(24)),
                   { ListIdx(2 + 1024 - 1 - i), EntVersion(1) },
                   { PoolIdx(23), EntVersion(1), SpawnerId(24) }, list, true);
    }
    ASSERT_EQ(list.size(), 2 + 1024);
}

TEST(EntityList, freeAll)
{
    EntityList list;
    list.freeAll();
    ASSERT_EQ(list.size(), 0);

    for (int i = 0; i < 520; ++i)
        list.allocEntity(PoolIdx(0), SpawnerId(0));
    ASSERT_EQ(list.size(), 520); // capacity = 1024

    list.freeAll();
    ASSERT_EQ(list.size(), 0); // capacity = 1024

    for (int i = 0; i < 1024; ++i) // over 520 (use new cells)
        TestEntity(list.allocEntity(PoolIdx(i), SpawnerId(i)),
                   { ListIdx(i), EntVersion(1) }, // even previously unused ones have version = 1
                   { PoolIdx(i), EntVersion(1), SpawnerId(i) }, list, true);

    // freeAll increments version of all reserved cells (even unused) but only if list is not empty
    list.freeAll(); // versions = 1 + 1
    list.freeAll(); // still versions = 2
    for (int i = 0; i < 1024; ++i)
        TestEntity(list.allocEntity(PoolIdx(i), SpawnerId(i)),
                   { ListIdx(i), EntVersion(2) },
                   { PoolIdx(i), EntVersion(2), SpawnerId(i) }, list, true);
}

TEST(EntityList, fitNextN)
{
    EntityList list; // fits 32 by default

    list.fitNextN(64);
    list.allocEntity(PoolIdx(123), SpawnerId(1024));
    list.freeAll();
    for (int i = 0; i < 32; ++i)
        TestEntity(list.allocEntity(PoolIdx(i), SpawnerId(i)),
                   { ListIdx(i), EntVersion(1) },
                   { PoolIdx(i), EntVersion(1), SpawnerId(i) }, list, true);
    TestEntity(list.allocEntity(PoolIdx(32), SpawnerId(32)),
               { ListIdx(32), EntVersion(1) }, // 33rd entity was not in the default storage, but fitNextN replaced it with a bigger one
               { PoolIdx(32), EntVersion(1), SpawnerId(32) }, list, true);

    list.freeAll();
    auto entity = list.allocEntity(PoolIdx(123), SpawnerId(1024));
    list.fitNextN(1024);
    TestEntity(entity, { ListIdx(0), EntVersion(2) }, { PoolIdx(123), EntVersion(2), SpawnerId(1024) }, list, true);

    list.freeEntity(entity);
    for (int i = 0; i < 5000; ++i)
        list.allocEntity(PoolIdx(0), SpawnerId(0));
    TestEntity(entity, { ListIdx(0), EntVersion(2) }, {}, list, false);
}