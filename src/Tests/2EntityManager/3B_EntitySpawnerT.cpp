#include "ComponentsT.h"
#include <ECSpp/Internal/EntitySpawner.h>
#include <gtest/gtest.h>

using namespace epp;

TEST(EntitySpawner, Create)
{
    EntityList elist;
    EntitySpawner spawner(SpawnerId(0), Archetype(IdOf<TComp1, TComp2>()));
    Entity ent = spawner.spawn(elist, [](auto&&) {});
    ASSERT_EQ(TComp1::AliveCounter, 1);
    ASSERT_EQ(TComp2::AliveCounter, 1);

    EntitySpawner spawner2(SpawnerId(1), Archetype(IdOf<TComp1, TComp3>()));
    spawner2.moveEntityHere(ent, elist, spawner, [](auto&&) {});
    ASSERT_EQ(TComp1::AliveCounter, 1);
    ASSERT_EQ(TComp2::AliveCounter, 0);
    ASSERT_EQ(TComp3::AliveCounter, 1);

    spawner.spawn(elist, [](auto&&) {});
    spawner.fitNextN(1024);
    ASSERT_EQ(TComp1::AliveCounter, 2);
    ASSERT_EQ(TComp2::AliveCounter, 1);
    ASSERT_EQ(TComp3::AliveCounter, 1);

    for (int i = 0; i < 1e4; ++i)
        spawner.spawn(elist, [](auto&&) {});
    ASSERT_EQ(TComp1::AliveCounter, 2 + 1e4);
    ASSERT_EQ(TComp2::AliveCounter, 1 + 1e4);
    ASSERT_EQ(TComp3::AliveCounter, 1);
}

TEST(EntitySpawner, Destroy)
{
    EntityList elist;
    EntitySpawner spawner(SpawnerId(0), Archetype(IdOf<TComp1, TComp2>()));
    spawner.destroy(spawner.spawn(elist, [](auto&&) {}), elist);
    ASSERT_EQ(TComp1::AliveCounter, 0);
    ASSERT_EQ(TComp2::AliveCounter, 0);

    Entity ent = spawner.spawn(elist, [](auto&&) {});
    EntitySpawner spawner2(SpawnerId(1), Archetype(IdOf<TComp1, TComp3>()));
    spawner2.moveEntityHere(ent, elist, spawner, [](auto&&) {});
    spawner2.destroy(ent, elist);
    ASSERT_EQ(TComp1::AliveCounter, 0);
    ASSERT_EQ(TComp2::AliveCounter, 0);
    ASSERT_EQ(TComp3::AliveCounter, 0);
    ASSERT_THROW(spawner2.destroy(ent, elist), AssertFailed);

    ent = spawner.spawn(elist, [](auto&&) {});
    spawner.fitNextN(1024);
    spawner.destroy(ent, elist);
    ASSERT_EQ(TComp1::AliveCounter, 0);
    ASSERT_EQ(TComp2::AliveCounter, 0);
    ASSERT_EQ(TComp3::AliveCounter, 0);

    for (int i = 0; i < 1e4; ++i)
        spawner.spawn(elist, [](auto&&) {});
    for (int i = 0; i < 1e4; ++i)
        spawner.destroy(spawner.getEntities().data.front(), elist);
    ASSERT_EQ(TComp1::AliveCounter, 0);
    ASSERT_EQ(TComp2::AliveCounter, 0);
    ASSERT_EQ(TComp3::AliveCounter, 0);
}

TEST(EntitySpawner, Clear_EntList)
{
    EntityList elist;
    EntitySpawner spawner(SpawnerId(0), Archetype(IdOf<TComp1, TComp2>()));
    spawner.spawn(elist, [](auto&&) {});
    spawner.clear(elist);
    spawner.clear(elist); // twice
    ASSERT_EQ(spawner.getEntities().data.size(), 0);
    ASSERT_EQ(spawner.spawn(elist, [](auto&&) {}).version.value, 1);

    Entity ent = spawner.spawn(elist, [](auto&&) {});
    ASSERT_EQ(ent.version.value, 0);

    EntitySpawner spawner2(SpawnerId(1), Archetype(IdOf<TComp1, TComp3>()));
    spawner2.moveEntityHere(ent, elist, spawner, [](auto&&) {});
    spawner2.clear(elist); // 1 moved
    spawner.clear(elist);  // 1
    ASSERT_EQ(spawner2.getEntities().data.size(), 0);
    ASSERT_EQ(spawner.getEntities().data.size(), 0);

    ent = spawner.spawn(elist, [](auto&&) {});
    spawner.fitNextN(1024);
    spawner.clear(elist);
    ASSERT_EQ(spawner.getEntities().data.size(), 0);

    for (int i = 0; i < 1e4; ++i)
        spawner.spawn(elist, [](auto&&) {});
    spawner.clear(elist);
    ASSERT_EQ(spawner.getEntities().data.size(), 0);
}

TEST(EntitySpawner, Clear)
{
    EntityList elist;
    EntitySpawner spawner(SpawnerId(0), Archetype(IdOf<TComp1, TComp2>()));
    spawner.spawn(elist, [](auto&&) {});
    spawner.clear(); // does not release used ids
    spawner.clear(); // twice
    ASSERT_EQ(spawner.getEntities().data.size(), 0);
    ASSERT_EQ(spawner.spawn(elist, [](auto&&) {}).listIdx.value, 1); // 0 is now lost until freeAll

    Entity ent = spawner.spawn(elist, [](auto&&) {});
    ASSERT_EQ(ent.listIdx.value, 2);

    EntitySpawner spawner2(SpawnerId(1), Archetype(IdOf<TComp1, TComp3>()));
    spawner2.moveEntityHere(ent, elist, spawner, [](auto&&) {});
    spawner2.clear(); // 1 moved destroyed ;    index 2 is lost
    spawner.clear();  // 1 destroyed;           indices 0 & 1 are lost
    ASSERT_EQ(spawner.getEntities().data.size(), 0);
    ASSERT_EQ(spawner2.getEntities().data.size(), 0);
    ASSERT_EQ(spawner2.spawn(elist, [](auto&&) {}).listIdx.value, 3);

    ent = spawner.spawn(elist, [](auto&&) {});
    spawner.fitNextN(1024);
    spawner.clear(); // idx: 2 is lost
    ASSERT_EQ(spawner.getEntities().data.size(), 0);
    ASSERT_EQ(spawner.spawn(elist, [](auto&&) {}).listIdx.value, 5);

    elist.freeAll(); // restores all, but also increments versions of every present and reserved element
    for (int i = 0; i < 1e4; ++i)
        spawner.spawn(elist, [](auto&&) {});
    spawner.clear(); // indices <0, 1e4) are lost
    ASSERT_EQ(spawner.getEntities().data.size(), 0);
    ASSERT_EQ(spawner.spawn(elist, [](auto&&) {}).listIdx.value, 1e4);
}

TEST(EntitySpawner, FitNextN)
{
    EntityList elist;
    EntitySpawner spawner(SpawnerId(0), Archetype(IdOf<TComp1, TComp2>()));
    spawner.fitNextN(1024);
    spawner.fitNextN(512); // 512 elements will fit in 1024 reserved ones
    Entity ent = spawner.spawn(elist, [](auto&&) {});
    auto ptr = spawner.getPool(IdOf<TComp1>())[elist.get(ent).poolIdx.value];
    auto eptr = &spawner.getEntities().data[elist.get(ent).poolIdx.value];
    for (int i = 1; i < 1024; ++i)
        spawner.spawn(elist, [](auto&&) {});
    ASSERT_EQ(ptr, spawner.getPool(IdOf<TComp1>())[elist.get(ent).poolIdx.value]);
    ASSERT_EQ(eptr, &spawner.getEntities().data[elist.get(ent).poolIdx.value]);
    spawner.spawn(elist, [](auto&&) {}); // 1025. does not fit
    ASSERT_NE(ptr, spawner.getPool(IdOf<TComp1>())[elist.get(ent).poolIdx.value]);
    ASSERT_NE(eptr, &spawner.getEntities().data[elist.get(ent).poolIdx.value]);
}

// TEST(EntitySpawner, MoveEntityHere) - already tested in other tests

TEST(EntitySpawner, GetPool)
{
    EntityList elist;
    EntitySpawner spawner(SpawnerId(0), Archetype(IdOf<TComp1, TComp2>()));
    ASSERT_THROW(spawner.getPool(IdOf<TComp3>()).size(), AssertFailed);
    ASSERT_THROW(spawner.getPool(IdOf<TComp4>()).size(), AssertFailed);
    ASSERT_EQ(spawner.getPool(IdOf<TComp1>()).size(), 0);
    ASSERT_EQ(spawner.getPool(IdOf<TComp2>()).size(), 0);

    Entity ent = spawner.spawn(elist, [](auto&&) {});
    ASSERT_EQ(spawner.getPool(IdOf<TComp1>()).size(), 1);
    ASSERT_EQ(spawner.getPool(IdOf<TComp2>()).size(), 1);

    spawner.destroy(ent, elist);
    ASSERT_EQ(spawner.getPool(IdOf<TComp1>()).size(), 0);
    ASSERT_EQ(spawner.getPool(IdOf<TComp2>()).size(), 0);

    EntitySpawner spawner2(SpawnerId(1), Archetype(IdOf<TComp1, TComp3>()));
    ent = spawner.spawn(elist, [](auto&&) {});
    spawner2.moveEntityHere(ent, elist, spawner, [](auto&&) {});
    ASSERT_EQ(spawner2.getPool(IdOf<TComp1>()).size(), 1);
    ASSERT_EQ(spawner2.getPool(IdOf<TComp3>()).size(), 1);
    ASSERT_THROW(spawner2.getPool(IdOf<TComp2>()).size(), AssertFailed);
    ASSERT_EQ(spawner.getPool(IdOf<TComp1>()).size(), 0);
    ASSERT_EQ(spawner.getPool(IdOf<TComp2>()).size(), 0);

    spawner2.clear();
    ASSERT_EQ(spawner2.getPool(IdOf<TComp1>()).size(), 0);
    ASSERT_EQ(spawner2.getPool(IdOf<TComp3>()).size(), 0);

    spawner2.spawn(elist, [](auto&&) {});
    spawner2.clear(elist);
    ASSERT_EQ(spawner2.getPool(IdOf<TComp1>()).size(), 0);
    ASSERT_EQ(spawner2.getPool(IdOf<TComp3>()).size(), 0);

    spawner.spawn(elist, [](auto&&) {});
    spawner.fitNextN(1024);
    ASSERT_EQ(spawner.getPool(IdOf<TComp1>()).size(), 1);
    ASSERT_EQ(spawner.getPool(IdOf<TComp2>()).size(), 1);

    for (int i = 1; i < 1e4; ++i)
        spawner.spawn(elist, [](auto&&) {});
    ASSERT_EQ(spawner.getPool(IdOf<TComp1>()).size(), 1e4);
    ASSERT_EQ(spawner.getPool(IdOf<TComp2>()).size(), 1e4);
}

TEST(EntitySpawner, GetEntities)
{
    EntityList elist;
    EntitySpawner spawner(SpawnerId(0), Archetype(IdOf<TComp1, TComp2>()));
    ASSERT_EQ(spawner.getEntities().data.size(), 0);

    Entity ent = spawner.spawn(elist, [](auto&&) {});
    ASSERT_EQ(spawner.getEntities().data.size(), 1);

    spawner.destroy(ent, elist);
    ASSERT_EQ(spawner.getEntities().data.size(), 0);

    EntitySpawner spawner2(SpawnerId(1), Archetype(IdOf<TComp1, TComp3>()));
    ent = spawner.spawn(elist, [](auto&&) {});
    spawner2.moveEntityHere(ent, elist, spawner, [](auto&&) {});
    ASSERT_EQ(spawner2.getEntities().data.size(), 1);
    ASSERT_EQ(spawner.getEntities().data.size(), 0);

    spawner2.clear();
    ASSERT_EQ(spawner2.getEntities().data.size(), 0);

    spawner2.spawn(elist, [](auto&&) {});
    spawner2.clear(elist);
    ASSERT_EQ(spawner2.getEntities().data.size(), 0);

    spawner.spawn(elist, [](auto&&) {});
    spawner.fitNextN(1024);
    ASSERT_EQ(spawner.getEntities().data.size(), 1);

    for (int i = 1; i < 1e4; ++i)
        spawner.spawn(elist, [](auto&&) {});
    ASSERT_EQ(spawner.getEntities().data.size(), 1e4);
}

TEST(EntitySpawner, MakeArchetype)
{
    EntityList elist;
    Archetype arch(IdOf<TComp1, TComp2>());
    EntitySpawner spawner(SpawnerId(0), Archetype(IdOf<TComp1, TComp2>()));
    ASSERT_EQ(spawner.makeArchetype().getCIds(), arch.getCIds());

    Archetype arche2 = spawner.makeArchetype().removeComponent<TComp2>().addComponent<TComp3>();
    EntitySpawner spawner2(SpawnerId(1), arche2);
    ASSERT_EQ(spawner2.makeArchetype().getCIds(), arche2.getCIds());
}