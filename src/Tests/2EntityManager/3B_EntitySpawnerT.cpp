#include "ComponentsT.h"
#include <ECSpp/internal/EntitySpawner.h>
#include <gtest/gtest.h>

using namespace epp;

template <typename... CompTs>
static void TestSpawner(EntitySpawner const& spawner, SpawnerId const sId, EntityList const& list,
                        std::size_t const sizeOffset = 0, TCompBase<0>::Arr_t const& correctArr = TCompBase<-1>().data)
{
    ASSERT_THROW(spawner.getPool(ComponentId(44)), AssertFailed);
    ASSERT_EQ(spawner.spawnerId, sId);
    ASSERT_EQ(spawner.mask, CMask(IdOfL<CompTs...>()));
    auto const& ents = spawner.getEntities().data;
    for (std::size_t i = 0; i < ents.size(); ++i) {
        ASSERT_EQ(list.get(ents[i]).poolIdx, PoolIdx(i));
        ASSERT_EQ(list.get(ents[i]).spawnerId, SpawnerId(sId));
        ASSERT_TRUE(((*(static_cast<CompTs const*>(spawner.getPool(IdOf<CompTs>())[i])) == CompTs(correctArr)) && ...));
    }
    ASSERT_TRUE(((CompTs::AliveCounter == ents.size() + sizeOffset) && ...));
    ASSERT_EQ(spawner.makeArchetype().getMask(), CMask(IdOfL<CompTs...>()));
}


TEST(EntitySpawner, Constructor)
{
    EntityList list;
    {
        EntitySpawner spawner(SpawnerId(0), Archetype());
        TestSpawner<>(spawner, SpawnerId(0), list);

        EntitySpawner spawner2(SpawnerId(1), Archetype(IdOfL<TComp1>()));
        TestSpawner<TComp1>(spawner2, SpawnerId(1), list);

        EntitySpawner spawner3(SpawnerId(0), Archetype(IdOfL<TComp2, TComp4>()));
        TestSpawner<TComp2, TComp4>(spawner3, SpawnerId(0), list);
    }
}

TEST(EntitySpawner, Spawn_Creator)
{
    {
        EntityList list;
        EntitySpawner spawner(SpawnerId(0), Archetype());
        spawner.spawn(list, [](EntityCreator&& creator) {
            ASSERT_THROW(creator.constructed<TComp1>(), AssertFailed);
        });
        TestSpawner<>(spawner, SpawnerId(0), list);

        for (int i = 0; i < 1024; ++i)
            spawner.spawn(list, [](auto&&) {});
        TestSpawner<>(spawner, SpawnerId(0), list);
    }
    {
        EntityList list;
        EntitySpawner spawner(SpawnerId(4), Archetype(IdOfL<TComp3>()));
        spawner.spawn(list, [](EntityCreator&& creator) { creator.constructed<TComp3>().data = { 3, 4, 3 }; });
        TestSpawner<TComp3>(spawner, SpawnerId(4), list, 0, { 3, 4, 3 });

        for (int i = 0; i < 1024; ++i)
            spawner.spawn(list, [](EntityCreator&& creator) {
                creator.constructed<TComp3>().data = { 3, 4, 3 };
                ASSERT_THROW(creator.constructed<TComp1>(), AssertFailed);
            });
        TestSpawner<TComp3>(spawner, SpawnerId(4), list, 0, { 3, 4, 3 });
    }
    {
        EntityList list;
        EntitySpawner spawner(SpawnerId(9), Archetype(IdOfL<TComp3, TComp4>()));
        spawner.spawn(list, [](EntityCreator&& creator) {
            creator.constructed<TComp3>().data = { 9, 1, 7 };
            creator.constructed<TComp4>().data = { 9, 1, 7 };
            ASSERT_THROW(creator.constructed<TComp1>(), AssertFailed);
        });
        TestSpawner<TComp3, TComp4>(spawner, SpawnerId(9), list, 0, { 9, 1, 7 });

        for (int i = 0; i < 1024; ++i)
            spawner.spawn(list, [](EntityCreator&& creator) {
                creator.constructed<TComp3>().data = { 9, 1, 7 };
                creator.constructed<TComp4>().data = { 9, 1, 7 };
            });
        TestSpawner<TComp3, TComp4>(spawner, SpawnerId(9), list, 0, { 9, 1, 7 });
    }
}

TEST(EntitySpawner, MoveEntityHere)
{
    EntityList list;
    EntitySpawner spawner(SpawnerId(0), Archetype(IdOf<TComp1, TComp2>()));
    ASSERT_THROW(spawner.moveEntityHere(Entity(), list, spawner, [](auto&&) {}), AssertFailed);
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);

    ASSERT_THROW(spawner.moveEntityHere(Entity(), list, spawner, [](auto&&) {}), AssertFailed);
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);

    for (int i = 0; i < 1024; ++i)
        spawner.spawn(list, [](auto&&) {});
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);

    EntitySpawner spawner2(SpawnerId(0), Archetype(IdOf<TComp1, TComp2>()));
    ASSERT_THROW(spawner2.moveEntityHere(Entity(), list, spawner, [](auto&&) {}), AssertFailed);

    auto ents = spawner.getEntities().data; // copy
    for (int i = 0; i < 1024; ++i)
        spawner2.spawn(list, [](auto&&) {});
    TestSpawner<TComp1, TComp2>(spawner2, SpawnerId(0), list, 1024);

    ents = spawner.getEntities().data; // copy, not spawner2
    for (auto ent : ents)
        spawner.destroy(ent, list);
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list, 1024);
    TestSpawner<TComp1, TComp2>(spawner2, SpawnerId(0), list);
}

TEST(EntitySpawner, Destroy)
{
    EntityList list;
    EntitySpawner spawner(SpawnerId(0), Archetype(IdOf<TComp1, TComp2>()));
    ASSERT_THROW(spawner.destroy(Entity(), list), AssertFailed);

    spawner.destroy(spawner.spawn(list, [](auto&&) {}), list);
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);

    for (int i = 0; i < 1024; ++i)
        spawner.spawn(list, [](auto&&) {});
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);

    auto ents = spawner.getEntities().data; // copy
    for (auto ent : ents)
        spawner.destroy(ent, list);
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);

    for (int i = 0; i < 1024; ++i)
        spawner.spawn(list, [](auto&&) {});
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);

    EntitySpawner spawner2(SpawnerId(0), Archetype(IdOf<TComp1, TComp2>()));
    for (int i = 0; i < 1024; ++i)
        spawner2.spawn(list, [](auto&&) {});
    TestSpawner<TComp1, TComp2>(spawner2, SpawnerId(0), list, 1024);

    ents = spawner.getEntities().data; // copy, not spawner2
    for (auto ent : ents)
        spawner.destroy(ent, list);
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list, 1024);
    TestSpawner<TComp1, TComp2>(spawner2, SpawnerId(0), list);
}

TEST(EntitySpawner, Clear_EntList)
{
    EntityList list;
    EntitySpawner spawner(SpawnerId(0), Archetype(IdOf<TComp1, TComp2>()));
    spawner.clear(list);
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);

    Entity ent = spawner.spawn(list, [](auto&&) {});
    spawner.clear(list);
    spawner.clear(list); // twice
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);
    ASSERT_EQ(list.size(), 0);
    ASSERT_FALSE(list.isValid(ent));

    spawner.spawn(list, [](auto&&) {});

    EntitySpawner spawner2(SpawnerId(1), Archetype(IdOf<TComp1, TComp2>()));
    spawner2.clear(list);
    TestSpawner<TComp1, TComp2>(spawner2, SpawnerId(1), list, 1);
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);

    spawner2.spawn(list, [](auto&&) {});
    TestSpawner<TComp1, TComp2>(spawner2, SpawnerId(1), list, 1);
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list, 1);

    spawner2.clear(list);
    TestSpawner<TComp1, TComp2>(spawner2, SpawnerId(1), list, 1);
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);
}

TEST(EntitySpawner, Clear)
{
    EntityList list;
    EntitySpawner spawner(SpawnerId(0), Archetype(IdOf<TComp1, TComp2>()));
    spawner.clear();
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);

    Entity ent = spawner.spawn(list, [](auto&&) {});
    spawner.clear(); // twice
    spawner.clear();
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);
    ASSERT_EQ(list.size(), 1);      // idx = 0 is lost
    ASSERT_TRUE(list.isValid(ent)); // wasn't destroyed -> it is still valid

    spawner.spawn(list, [](auto&&) {});

    EntitySpawner spawner2(SpawnerId(1), Archetype(IdOf<TComp1, TComp2>()));
    spawner2.clear();
    TestSpawner<TComp1, TComp2>(spawner2, SpawnerId(1), list, 1);
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);

    spawner2.spawn(list, [](auto&&) {});
    TestSpawner<TComp1, TComp2>(spawner2, SpawnerId(1), list, 1);
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list, 1);

    spawner2.clear();
    TestSpawner<TComp1, TComp2>(spawner2, SpawnerId(1), list, 1);
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);
}

TEST(EntitySpawner, FitNextN)
{
    EntityList list;
    EntitySpawner spawner(SpawnerId(0), Archetype(IdOf<TComp1, TComp2>()));
    spawner.fitNextN(1024);
    spawner.fitNextN(512); // 512 elements will fit in 1024 reserved ones
    spawner.fitNextN(0);
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);

    Entity ent = spawner.spawn(list, [](auto&&) {});
    auto ptr = spawner.getPool(IdOf<TComp1>())[list.get(ent).poolIdx.value];
    auto eptr = &spawner.getEntities().data[list.get(ent).poolIdx.value];
    for (int i = 1; i < 1024; ++i)
        spawner.spawn(list, [](auto&&) {});
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);
    ASSERT_EQ(ptr, spawner.getPool(IdOf<TComp1>())[list.get(ent).poolIdx.value]);
    ASSERT_EQ(eptr, &spawner.getEntities().data[list.get(ent).poolIdx.value]);
    spawner.spawn(list, [](auto&&) {}); // 1025. does not fit
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);
    ASSERT_NE(ptr, spawner.getPool(IdOf<TComp1>())[list.get(ent).poolIdx.value]);
    ASSERT_NE(eptr, &spawner.getEntities().data[list.get(ent).poolIdx.value]);
}

TEST(EntitySpawner, ShrinkToFit)
{
    EntityList list;
    EntitySpawner spawner(SpawnerId(0), Archetype(IdOf<TComp1, TComp2>()));

    spawner.fitNextN(1024);
    Entity ent = spawner.spawn(list, [](auto&&) {});
    void* ptr = spawner.getPool(IdOf<TComp1>())[0];
    for (int i = 0; i < 512; ++i)
        spawner.spawn(list, [](auto&&) {});
    spawner.shrinkToFit();
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);

    spawner.spawn(list, [](auto&&) {}); // new one does not fit
    TestSpawner<TComp1, TComp2>(spawner, SpawnerId(0), list);
    ASSERT_NE(ptr, spawner.getPool(IdOf<TComp1>())[list.get(ent).poolIdx.value]);
    ASSERT_NE(ptr, spawner.getPool(IdOf<TComp1>())[0]);
}