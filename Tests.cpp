#include <ECSpp/ECSWorld.h>
#include <chrono>
#include <gtest/gtest.h>



// TODO:
// Notifier.h
// Time.h

//#include "vld.h"

using namespace epp;


using Clock_t = std::chrono::high_resolution_clock;

struct TComp1 : public Component
{
    int test[5];
};
struct TComp2 : public Component
{
    int test[5];
};
struct TComp3 : public Component
{
    int test[5];
};

struct TestSystem : public System
{
    virtual void init(EntityManager& entityManager) override
    {
        entityManager.requestCGroup(group);
    }

    virtual void update(EntityManager& entityManager, float dt, bool catchUpTick) override
    {
        for (auto pack : group)
        {
            entityManager.spawn(makeArchetype<TComp1, TComp3>(), 10);
            std::cout << "entity" << dt << std::endl;
        }
    }

    CGroup<TComp1, TComp3> group;
};

struct TestClass
{

    void cb(EntityEvent& ev)
    {
    }
};


TEST(BitmaskTest, Set_Unset_Get)
{
    Bitmask bitmask;

    EXPECT_FALSE((TuplePP<int, float>::containsType<>()));

    bitmask.set(1).set(123);
    EXPECT_TRUE(bitmask.get(1) && bitmask.get(123));

    bitmask.unset(1);
    EXPECT_FALSE(bitmask.get(1) && bitmask.get(123));

    bitmask = { 1, 2, 3, 4, 5 };

    EXPECT_TRUE(bitmask.get(1) && bitmask.get(2) && bitmask.get(3) && bitmask.get(4) && bitmask.get(5));
}

TEST(BitmaskTest, Clear_Count)
{
    Bitmask bitmask;

    // check if setting the same bit more than once changes anything
    bitmask.set(1).set(123).set(1).set(123);
    EXPECT_TRUE(bitmask.getSetCount() == 2);

    bitmask.clear();
    bitmask.set(120).set(123).set(0).set(1024);
    EXPECT_TRUE(bitmask.getSetCount() == 4);

    // check if unsetting the same bit more than once changes anything
    bitmask.unset(120).unset(120).set(123).unset(0).unset(0).set(1024);
    EXPECT_TRUE(bitmask.getSetCount() == 2);

    bitmask.clear();
    EXPECT_TRUE(bitmask.getSetCount() == 0);
}

TEST(BitmaskTest, Operators_Common)
{
    Bitmask bitmask1;
    Bitmask bitmask2;

    bitmask1.set(0).set(5).set(7);
    bitmask2.set(0).set(1).set(4).set(7);

    Bitmask noCommonTest({ 1, 2, 3, 4, 6 });
    ASSERT_FALSE(bitmask1.hasCommon(noCommonTest));
    ASSERT_TRUE(bitmask1.hasCommon(bitmask2));
    ASSERT_EQ(bitmask1.numberOfCommon(bitmask2), 2);

    // operators: =, ==, !=
    Bitmask bitmask1Copy = bitmask1;
    ASSERT_EQ(bitmask1Copy, bitmask1);
    ASSERT_NE(bitmask1, bitmask2);

    // operator &
    Bitmask andResult;
    andResult.set(0).set(7);
    ASSERT_EQ(bitmask1 & bitmask2, andResult);

    // operator |
    Bitmask orResult;
    orResult.set(0).set(1).set(4).set(5).set(7);
    ASSERT_EQ(bitmask1 | bitmask2, orResult);
    ASSERT_NE(bitmask1 & bitmask2, orResult);

    // operator &=
    ASSERT_EQ(bitmask1Copy &= bitmask2, andResult);

    // operator |=
    ASSERT_EQ(bitmask1 |= bitmask2, orResult);
}

TEST(BitmaskTest, Flip)
{
    Bitmask flipTest{ 0, 1, 127 };
    Bitmask flipped = flipTest.flipped();

    ASSERT_EQ(flipped.getSetCount(), 128 - 3);
    ASSERT_FALSE(flipped.get(0));
    ASSERT_FALSE(flipped.get(1));
    ASSERT_FALSE(flipped.get(127));

    for (int i = 2; i < 127; i++)
        ASSERT_TRUE(flipped.get(i));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(CFilterTest, Wanted_Unwanted)
{
    CFilter cFilter;
    Bitmask wantedMask;
    Bitmask unwantedMask;

    cFilter.addWanted<TComp1, TComp2, TComp3>().addUnwanted<Component>();
    wantedMask.set(getCTypeId<TComp1>()).set(getCTypeId<TComp2>()).set(getCTypeId<TComp3>());
    unwantedMask.set(getCTypeId<Component>());
    ASSERT_EQ(cFilter.getWantedCMask(), wantedMask);
    ASSERT_EQ(cFilter.getUnwantedCMask(), unwantedMask);

    cFilter.removeWanted<TComp1>().addUnwanted<TComp3>();
    wantedMask.unset(getCTypeId<TComp1>()).unset(getCTypeId<TComp3>());
    unwantedMask.set(getCTypeId<TComp3>());
    ASSERT_EQ(cFilter.getWantedCMask(), wantedMask);
    ASSERT_EQ(cFilter.getUnwantedCMask(), unwantedMask);

    cFilter.addUnwanted<TComp2>();
    wantedMask.clear();

    ASSERT_EQ(cFilter.getWantedCMask(), wantedMask);
    ASSERT_NE(cFilter.getUnwantedCMask(), unwantedMask);

    unwantedMask.set(getCTypeId<TComp2>());
    ASSERT_EQ(cFilter.getUnwantedCMask(), unwantedMask);
}

TEST(CFilterTest, Set_Wanted_Unwanted)
{
    CFilter filter;
    filter.setWanted({ 1, 2, 5, 10 });
    filter.setUnwanted({ 1, 10, 15 }); // common 1 & 10

    ASSERT_FALSE(filter.getWantedCMask().hasCommon(filter.getUnwantedCMask()));
    ASSERT_TRUE(filter.getUnwantedCMask().get(1) && filter.getUnwantedCMask().get(10));

    filter.setWanted({ 1, 15 }); // common 1 & 15
    ASSERT_FALSE(filter.getWantedCMask().hasCommon(filter.getUnwantedCMask()));
    ASSERT_TRUE(filter.getWantedCMask().get(1) && filter.getWantedCMask().get(15));

    CFilter constructCopy(filter.getWantedCMask(), filter.getUnwantedCMask());
    ASSERT_EQ(constructCopy, filter);
}

TEST(CFilterTest, Clear_Hashing)
{
    CFilter cFilter1;
    CFilter cFilter2;

    cFilter1.addWanted<TComp1, TComp2>().addUnwanted<TComp3>();
    cFilter2.addWanted<TComp1>().addUnwanted<TComp3>();
    ASSERT_NE(cFilter1.hash(), cFilter2.hash());

    cFilter1.removeWanted<TComp2>();
    ASSERT_EQ(cFilter1.hash(), cFilter2.hash());

    cFilter1.removeUnwanted<TComp3>();
    ASSERT_NE(cFilter1.hash(), cFilter2.hash());

    cFilter1.addUnwanted<Component>();
    ASSERT_NE(cFilter1.hash(), cFilter2.hash());

    cFilter1.addWanted<Component>();
    cFilter2.addWanted<Component>();
    cFilter2.removeUnwanted<TComp3>();
    ASSERT_EQ(cFilter1.hash(), cFilter2.hash());

    cFilter1.clear();
    ASSERT_NE(cFilter1.hash(), cFilter2.hash());
    ASSERT_EQ(cFilter1, CFilter());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(PoolTest, Alloc_Free_Capacity)
{
    Pool<int> pool;
    int       x  = 2;
    int       a1 = pool.alloc();
    int       a2 = pool.alloc(-1);
    int       a3 = pool.alloc(x);

    EXPECT_EQ(a1, pool[0]);
    EXPECT_EQ(a2, pool[1]);
    EXPECT_EQ(a3, pool[2]);
    EXPECT_EQ(pool.getSize(), 3);

    pool.free(0); // move last object to freed index (2 -> 0)
    pool.free(2); // free again, now invalid index
    EXPECT_NE(a1, pool[0]);
    EXPECT_EQ(a3, pool[0]);
    EXPECT_EQ(a2, pool[1]);
    EXPECT_EQ(pool.getSize(), 2);

    pool.reserve(10);
    EXPECT_EQ(pool.getReserved(), 10);
    EXPECT_EQ(pool.getSize(), 2);

    pool.clear();
    EXPECT_EQ(pool.getSize(), 0);
    EXPECT_EQ(pool.getReserved(), 10);

    for (size_t i = 0; i < 8; i++)
        pool.alloc();

    pool.reserve(2);

    EXPECT_EQ(pool.getSize(), 2);
}

TEST(PoolTest, AllocN)
{
    Pool<int> pool;
    auto      it = pool.allocN(10, 123);

    ASSERT_GT(pool.getReserved(), 10);
    ASSERT_EQ(pool.getSize(), 10);

    int i = 0;
    for (; it != pool.end(); ++it)
    {
        ASSERT_EQ(*it, 123);
        ++i;
    }
    ASSERT_EQ(i, 10);
}

TEST(PoolTest, Access_Iterators)
{
    Pool<int>    pool;
    const size_t allocCount = 6;
    for (size_t i = 0; i < allocCount; i++)
        pool.alloc();

    Pool<int>::Iterator_t iterator(&pool[0]);
    ASSERT_EQ(iterator, pool.begin());
    ASSERT_EQ(&*iterator, &pool.front());
    for (size_t i = 0; i < allocCount - 1; i++)
        ++iterator;
    ASSERT_EQ(&*iterator, &pool.back());
    ++iterator;
    ASSERT_EQ(iterator, pool.end());

#ifdef _DEBUG
    // test the exception throwing on out of range index
    bool catched = false;
    try
    {
        pool[allocCount];
    }
    catch (std::out_of_range)
    {
        catched = true;
    }
    // did not catch out of range exception
    ASSERT_TRUE(catched);
#endif
}

TEST(PoolTest, Operator)
{
    Pool<int>    pool;
    const size_t allocCount = 6;
    for (size_t i = 0; i < allocCount; i++)
        pool.alloc();
    auto poolBegin = &pool.front();

    Pool<int> moveConstructed = std::move(pool);

    ASSERT_GT(moveConstructed.getReserved(), allocCount);
    ASSERT_EQ(moveConstructed.getSize(), allocCount);
    ASSERT_EQ(&moveConstructed[0], poolBegin);
}

TEST(PoolIteratorTest, Operators)
{
    Pool<int> pool;
    size_t    n = 20;
    pool.allocN(n);

    auto it = pool.begin();

    ASSERT_EQ(it + n, pool.end());
    ASSERT_EQ(it, pool.end() - n);
    ASSERT_EQ(it += n, pool.end());
    ASSERT_EQ(it -= n, pool.begin());
    ASSERT_EQ(pool.end() - it, ptrdiff_t(n));

    ASSERT_TRUE(it < pool.end());
    ASSERT_TRUE(pool.end() > it);
    ASSERT_TRUE(it >= pool.begin());
    ASSERT_TRUE(it <= pool.begin());
    ASSERT_TRUE((it += n) == pool.end());
    ASSERT_TRUE(it != pool.begin());

    it = pool.begin();
    ASSERT_TRUE(it++ == pool.begin());
    ASSERT_TRUE(++it == pool.begin() + 2);
    ASSERT_TRUE(it-- == pool.begin() + 2);
    ASSERT_TRUE(--it == pool.begin());

    ASSERT_EQ(&*it, &it[0]);
    ASSERT_EQ(&*(it + n - 1), &it[n - 1]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(ArchetypeTest, Add_Remove)
{
    Archetype archeT;
    ASSERT_TRUE((archeT.addComponent<TComp1>()));
    ASSERT_TRUE((archeT.addComponent<TComp2, TComp3>()));
    ASSERT_FALSE((archeT.addComponent<TComp1>()));
    ASSERT_FALSE((archeT.addComponent<TComp1, Component>()));
    ASSERT_TRUE((archeT.hasComponent<TComp1, TComp2, TComp3, Component>()));

    ASSERT_TRUE((archeT.removeComponent<TComp1>()));
    ASSERT_TRUE((archeT.removeComponent<TComp2, TComp3>()));
    ASSERT_FALSE((archeT.removeComponent<TComp1>()));
    ASSERT_FALSE((archeT.removeComponent(getCTypeId<TComp1>(), getCTypeId<Component>())));
    ASSERT_FALSE((archeT.hasComponent<TComp1, TComp2, TComp3, Component>()));

    archeT.addComponent<TComp1, TComp2>();
    archeT.reset();
    ASSERT_FALSE((archeT.hasComponent<TComp1, TComp2>()));
}

TEST(ArchetypeTest, Copy_Mask)
{
    Archetype archeT;
    Bitmask   cMask;

    archeT.addComponent<TComp1, TComp2, TComp3>();
    cMask.set(getCTypeId<TComp1>()).set(getCTypeId<TComp2>()).set(getCTypeId<TComp3>());

    ASSERT_EQ(archeT.getCMask(), cMask);
    Archetype copy = archeT;
    ASSERT_EQ(copy.getCMask(), cMask);

    archeT.reset();
    cMask.clear();
    ASSERT_EQ(archeT.getCMask(), cMask);
}

TEST(ArchetypeTest, Filter_Interactions)
{
    Archetype archetype;
    CFilter   filter;

    archetype.addComponent<TComp3, TComp1>();
    filter.addWanted<TComp1, TComp3>();
    ASSERT_TRUE(archetype.meetsRequirementsOf(filter));

    filter.addUnwanted<TComp1>();
    ASSERT_FALSE(archetype.meetsRequirementsOf(filter));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(ASpawnerTest, ArchetypeValidation)
{
    {
        Archetype archeT;
        archeT.addComponent<TComp1, TComp3>();
        ASpawner aSpawner(archeT);
        ASSERT_EQ(archeT.getCMask(), aSpawner.getArchetype().getCMask());
    }
    {
        // try with an empty Archetype
        ASpawner aSpawner(std::move(Archetype()));
        ASSERT_EQ(Archetype().getCMask(), aSpawner.getArchetype().getCMask());
    }
}

TEST(ASpawnerTest, Spawn_Kill_Count_SpawningOnly)
{
    ASpawner aSpawner(makeArchetype<TComp1, TComp3>());

    auto ref  = aSpawner.spawn();
    auto ref2 = aSpawner.spawn();
    auto ref3 = aSpawner.spawn();
    ASSERT_EQ(aSpawner.getSpawningEntitiesCount(), 3);

    aSpawner.kill(ref);
    ASSERT_EQ(aSpawner.getSpawningEntitiesCount(), 2);

    // because spawner is pools based, removing entity from the beginning moves last entity to the front so index 0 is always valid as long as aSpawner.get*EntitiesCount() != 0
    aSpawner.kill(ref2);
    aSpawner.kill(ref3);
    ASSERT_EQ(aSpawner.getSpawningEntitiesCount(), 0);

    aSpawner.spawn();
    aSpawner.spawn();
    aSpawner.clear();
    ASSERT_EQ(aSpawner.getSpawningEntitiesCount(), 0);

    aSpawner.spawn();
    aSpawner.spawn();
    aSpawner.acceptSpawningEntities();
    ASSERT_EQ(aSpawner.getSpawningEntitiesCount(), 0);
}

TEST(ASpawnerTest, Spawn_Kill_Count_AliveOnly)
{
    ASpawner aSpawner(makeArchetype<TComp1, TComp3>());

    auto ref = aSpawner.spawn();

    auto ref2 = aSpawner.spawn();
    auto ref3 = aSpawner.spawn();

    aSpawner.acceptSpawningEntities();

    ASSERT_EQ(aSpawner.getAliveEntitiesCount(), 3);

    aSpawner.kill(ref);

    ASSERT_EQ(aSpawner.getAliveEntitiesCount(), 2);

    aSpawner.kill(ref2);
    aSpawner.kill(ref3);
    ASSERT_EQ(aSpawner.getAliveEntitiesCount(), 0);

    aSpawner.spawn();
    aSpawner.spawn();
    aSpawner.acceptSpawningEntities();
    aSpawner.clear();
    ASSERT_EQ(aSpawner.getAliveEntitiesCount(), 0);
}

TEST(ASpawnerTest, Spawn_Kill_Count_Both)
{
    ASpawner aSpawner(makeArchetype<TComp1, TComp3>());

    auto ref1 = aSpawner.spawn();
    auto ref2 = aSpawner.spawn();
    aSpawner.acceptSpawningEntities();

    ASSERT_EQ(aSpawner.getAliveEntitiesCount(), 2);

    auto ref3 = aSpawner.spawn();
    aSpawner.kill(ref3); // not alive yet, shouldnt change alive entities count

    ASSERT_EQ(aSpawner.getAliveEntitiesCount(), 2);

    aSpawner.spawn(); // killing alive entities shouldnt affect spawning entities
    aSpawner.kill(ref1);
    aSpawner.kill(ref2);
    ASSERT_EQ(aSpawner.getAliveEntitiesCount(), 0);
    ASSERT_EQ(aSpawner.getSpawningEntitiesCount(), 1);
}

TEST(ASpawnerTest, SpawnN)
{
    ASpawner aSpawner(makeArchetype<TComp1, TComp3>());
    size_t   n = 20;
    aSpawner.spawn(n);
    aSpawner.acceptSpawningEntities();
    ASSERT_EQ(aSpawner.getAliveEntitiesCount(), n);

    size_t i = aSpawner.getAliveEntitiesCount() - n;
    for (; i < aSpawner.getAliveEntitiesCount(); ++i)
    {
        ASSERT_EQ(aSpawner[i].getComponent<TComp1>(), &aSpawner.getPool<TComp1>(true)[i]);
        ++i;
    }
    ASSERT_EQ(i, n);
}

TEST(ASpawnerTest, Acces)
{
    Archetype archeT;
    archeT.addComponent<TComp1, TComp3>();
    ASpawner aSpawner(makeArchetype<TComp1, TComp3>());

    auto ref = aSpawner.spawn();

    aSpawner.acceptSpawningEntities();

    aSpawner.getPool<TComp1>(true)[0];
    aSpawner.getPool(getCTypeId<TComp3>(), true)[0];
    aSpawner.kill(ref);
#ifdef _DEBUG
    // test the exception throwing on out of range entity reference index
    bool refAccessExcCatched = false;
    try
    {
        aSpawner[123];
    }
    catch (std::out_of_range)
    {
        refAccessExcCatched = true;
    }
    ASSERT_TRUE(refAccessExcCatched);

    bool poolAccessExcCatched = false;
    try
    {
        aSpawner.getPool<TComp2>(true)[0];
    }
    catch (std::out_of_range)
    {
        poolAccessExcCatched = true;
    }
    ASSERT_TRUE(poolAccessExcCatched);
#endif
}

TEST(ASpawnerTest, EntityMove)
{
    Archetype archeT;
    archeT.addComponent<TComp1, TComp3>();
    ASpawner aSpawner(archeT);
    archeT.addComponent<TComp2>();
    ASpawner aSpawner2(archeT);

    auto      ref       = aSpawner.spawn();
    EntityRef refBefore = ref;

    ASSERT_NE(aSpawner.getSpawningEntitiesCount(), 0);
    ASSERT_NE(aSpawner2.getSpawningEntitiesCount(), 1);

    aSpawner2.moveExistingEntityHere(ref);
    ASSERT_EQ(refBefore, ref);
    ASSERT_EQ(aSpawner.getSpawningEntitiesCount(), 0);
    ASSERT_EQ(aSpawner2.getSpawningEntitiesCount(), 1);

    ref = aSpawner.spawn();
    aSpawner.acceptSpawningEntities();
    ASSERT_NE(aSpawner.getAliveEntitiesCount(), 0);
    ASSERT_NE(aSpawner2.getAliveEntitiesCount(), 1);

    aSpawner2.moveExistingEntityHere(ref);
    ASSERT_EQ(aSpawner.getAliveEntitiesCount(), 0);

    ASSERT_NE(aSpawner2.getAliveEntitiesCount(), 1); // NOT equal - thats because moving an entity, ie. changing its archetype,
                                                     // is treated same as killing that entity and spawning in the other ASpawner
    ASSERT_EQ(aSpawner2.getSpawningEntitiesCount(), 2);

    aSpawner2.acceptSpawningEntities();
    ASSERT_EQ(aSpawner2.getAliveEntitiesCount(), 2);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(EntityRefTest, Validation)
{
    ASpawner aSpawner(makeArchetype<Component, TComp1, TComp3>());

    auto ref0 = aSpawner.spawn();
    auto ref1 = aSpawner.spawn();
    auto ref2 = aSpawner.spawn();

    ASSERT_TRUE((ref0.isValid() && ref1.isValid() && ref2.isValid()));
    ASSERT_TRUE(ref0.isAlive() == ref1.isAlive() == ref2.isAlive() == false);

    aSpawner.kill(ref0);
    ASSERT_FALSE(ref0.isValid());

    aSpawner.acceptSpawningEntities();
    ASSERT_TRUE(ref1.isAlive() == ref2.isAlive() == true);

    ASSERT_TRUE(ref1.isValid());
    ref1.die();
    ASSERT_FALSE(ref1.isValid());

    ASSERT_TRUE(ref2.isValid());

    ASpawner aSpawner2(makeArchetype<Component, TComp1, TComp2, TComp3>());
    aSpawner2.moveExistingEntityHere(ref2);

    ASSERT_TRUE(ref2.isValid());
    ASSERT_FALSE(ref2.isAlive());
    ASSERT_TRUE((ref2.hasComponent<Component, TComp1, TComp2, TComp3>()));
    ASSERT_TRUE((ref2.getComponent<Component>() && ref2.getComponent<TComp1>() && ref2.getComponent<TComp2>() && ref2.getComponent<TComp3>()));
}

TEST(EntityRefTest, ComponentAccess)
{
    ASpawner aSpawner1(makeArchetype<TComp1, TComp3>());
    ASpawner aSpawner2(makeArchetype<TComp1, TComp3>());

    auto ref0 = aSpawner1.spawn();
    auto ref1 = aSpawner1.spawn();
    auto ref2 = aSpawner1.spawn();

    ASSERT_TRUE(ref0.getComponent<TComp1>());
    ASSERT_TRUE(ref0.getComponent<TComp3>());
    ASSERT_FALSE(ref0.getComponent<TComp2>());

    ASSERT_TRUE(ref1.getComponent<TComp1>());
    ASSERT_TRUE(ref1.getComponent<TComp3>());

    ASSERT_TRUE(ref2.getComponent<TComp1>());
    ASSERT_TRUE(ref2.getComponent<TComp3>());

    ref1.die();
    ASSERT_TRUE(ref0.getComponent<TComp1>());
    ASSERT_TRUE(ref0.getComponent<TComp3>());

    ASSERT_FALSE(ref1.getComponent<TComp1>());
    ASSERT_FALSE(ref1.getComponent<TComp3>());

    ASSERT_TRUE(ref2.getComponent<TComp1>());
    ASSERT_TRUE(ref2.getComponent<TComp3>());

    aSpawner2.moveExistingEntityHere(ref2);
    ASSERT_TRUE(ref0.getComponent<TComp1>());
    ASSERT_TRUE(ref0.getComponent<TComp3>());

    ASSERT_TRUE(ref2.getComponent<TComp1>());
    ASSERT_TRUE(ref2.getComponent<TComp3>());

    aSpawner2.clear();
    aSpawner1.clear();
    ASSERT_FALSE(ref0.getComponent<TComp1>());
    ASSERT_FALSE(ref0.getComponent<TComp3>());
    ASSERT_FALSE(ref2.getComponent<TComp1>());
    ASSERT_FALSE(ref2.getComponent<TComp3>());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(SpawnersPackTest, addASpawner_clear)
{
    ASpawnersPack<TComp1, TComp3> aSpawnersPack({ getCTypeId<TComp2>() });
    ASpawner                      spawner1(makeArchetype<TComp3, TComp1>());
    ASpawner                      spawner2(makeArchetype<TComp2, TComp3, TComp1>()); // not matching
    ASpawner                      spawner3(makeArchetype<TComp3, TComp1>());

    aSpawnersPack.addASpawnerIfMeetsRequirements(spawner1);
    aSpawnersPack.addASpawnerIfMeetsRequirements(spawner2);
    aSpawnersPack.addASpawnerIfMeetsRequirements(spawner3);

    ASSERT_EQ(aSpawnersPack.getSpawners().size(), 2);
    ASSERT_EQ(aSpawnersPack.getSpawners()[0], &spawner1);
    ASSERT_NE(aSpawnersPack.getSpawners()[1], &spawner2);
    ASSERT_EQ(aSpawnersPack.getSpawners()[1], &spawner3);

    aSpawnersPack.clear();
    ASSERT_EQ(aSpawnersPack.getSpawners().size(), 0);
}

TEST(SpawnersPackTest, Filter)
{
    ASpawnersPack<TComp1, TComp3> aSpawnersPack({ getCTypeId<TComp2>() });
    CFilter                       filter({ { getCTypeId<TComp1>(), getCTypeId<TComp3>() }, { getCTypeId<TComp2>() } });

    ASSERT_EQ(aSpawnersPack.getFilter(), filter);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(CGroupTest, Filter)
{
    EntityManager          mgr;
    CFilter                filter({ getCTypeId<TComp1>(), getCTypeId<TComp3>() }, {});
    CGroup<TComp1, TComp3> group;
    mgr.requestCGroup(group, filter.getUnwantedCMask());

    ASSERT_EQ(filter, group.getFilter());
}

TEST(CGroupTest, SpawnersValidation)
{
    EntityManager          mgr;
    CGroup<TComp1, TComp3> group;
    mgr.requestCGroup(group, Bitmask({ getCTypeId<TComp2>() }));

    ASpawner matchingSpawner(makeArchetype<TComp1, TComp3>());
    ASpawner notMatchingSpawner(makeArchetype<TComp1, TComp3, TComp2>());

    auto ref1   = matchingSpawner.spawn();
    auto ref2   = matchingSpawner.spawn();
    auto nmRef4 = notMatchingSpawner.spawn();

    mgr.registerArchetype(matchingSpawner.getArchetype());
    mgr.registerArchetype(notMatchingSpawner.getArchetype());

    int i = 0;
    for (auto pack : group)
    {
        if (i == 0)
        {
            ASSERT_EQ(&pack.get<TComp1&>(), ref1.getComponent<TComp1>());
            ASSERT_EQ(&pack.get<TComp3&>(), ref1.getComponent<TComp3>());
        }
        else if (i == 1)
        {
            ASSERT_EQ(&pack.get<TComp1&>(), ref2.getComponent<TComp1>());
            ASSERT_EQ(&pack.get<TComp3&>(), ref2.getComponent<TComp3>());
        }
        else
            FAIL();
        ++i;
    }
}

TEST(CGroupTest, Iteration)
{
    EntityManager  mgr;
    CGroup<TComp1> group;
    mgr.requestCGroup(group);

    mgr.spawn<TComp1, TComp3>();
    mgr.spawn<TComp1, TComp3>();
    mgr.spawn<TComp1, TComp3, TComp2>();
    mgr.spawn<TComp1, TComp3, TComp2>();

    mgr.acceptSpawnedEntities();

    int i = 0;
    // adding a spawner while iterating - range based for loop will not iterate over entities of added spawner
    // begin -> end based for loop will
    for (auto it = group.begin(); it.isValid() && it != group.end(); ++it)
    {
        it.getComponent<TComp1>();
        if (i++ == 1)
        {
            mgr.spawn<Component, TComp1, TComp3, TComp2>();
            mgr.spawn<Component, TComp1, TComp3, TComp2>();
            mgr.acceptSpawnedEntities();
        }
    }
    ASSERT_EQ(i, 6);

    i = 0;
    for (auto pack : group)
        if (i++ == 1)
        {
            mgr.spawn<TComp1>();
            mgr.spawn<TComp1>();
            mgr.acceptSpawnedEntities();
        }
    ASSERT_EQ(i, 6); // still 6, not 8

    i = 0;

    // now it will also iterate over added spawner's entities
    for (auto pack : group)
    {
        pack.get<TComp1&>();
        i++;
    }
    ASSERT_EQ(i, 8);
    ASSERT_EQ(mgr.getArchetypesSpawners().size(), 4);
    mgr.clear();

    i = 0;
    for (auto pack : group)
        ++i;
    ASSERT_EQ(i, 0);

    // unregistered CGroup:
#if _DEBUG
    std::cout << "Wanted Assertion fails incomming (one for CGroup::begin, one for CGroup::end): " << std::endl;
#endif
    CGroup<TComp1, TComp2> unregistered1;
    for (auto s : unregistered1)
        FAIL();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(EntityManagerTest, registerArchetype)
{
    EntityManager aSpawnerSet;

    aSpawnerSet.registerArchetype(makeArchetype<TComp2>());
    aSpawnerSet.registerArchetype(makeArchetype<TComp2>()); // again TComp2 (shouldn't register anything)
    aSpawnerSet.registerArchetype(makeArchetype<TComp1, TComp3>());
    aSpawnerSet.registerArchetype(makeArchetype<TComp3, TComp1>()); // same as above, but different order (shouldn't register anything)

    ASSERT_EQ(aSpawnerSet.getArchetypesSpawners().size(), 2);
}

TEST(EntityManagerTest, spawn_add_removeComponent)
{
    EntityManager em;
    auto          eRef                   = em.spawn(makeArchetype<TComp1, TComp3>());
    eRef.getComponent<TComp1>()->test[0] = 123;
    eRef.getComponent<TComp3>()->test[0] = 321;

    ASSERT_TRUE(eRef.isValid());
    ASSERT_FALSE(eRef.isAlive());

    em.acceptSpawnedEntities();
    ASSERT_TRUE(eRef.isAlive());
    ASSERT_EQ(eRef.getComponent<TComp1>()->test[0], 123);
    ASSERT_EQ(eRef.getComponent<TComp3>()->test[0], 321);

    ASSERT_TRUE(eRef.getComponent<TComp1>());
    ASSERT_TRUE(eRef.getComponent<TComp3>());
    ASSERT_FALSE(eRef.getComponent<TComp2>());

    em.addComponent<TComp2>(eRef); // changes archetype -> registers new archetype

    ASSERT_TRUE(eRef.getComponent<TComp2>());
    ASSERT_EQ(eRef.getComponent<TComp1>()->test[0], 123);
    ASSERT_EQ(eRef.getComponent<TComp3>()->test[0], 321);

    em.removeComponent<TComp2>(eRef);

    ASSERT_FALSE(eRef.getComponent<TComp2>());
    ASSERT_EQ(eRef.getComponent<TComp1>()->test[0], 123);
    ASSERT_EQ(eRef.getComponent<TComp3>()->test[0], 321);

    ASSERT_EQ(em.getArchetypesSpawners().size(), 2);
}

TEST(EntityManagerTest, spawnN)
{
    EntityManager em;
    Archetype     arche = makeArchetype<TComp1, TComp3>();
    size_t        n     = 1000;

    em.spawn(arche, n);
    em.acceptSpawnedEntities();

    for (int i = 0; i < n; ++i)
    {
        ASSERT_TRUE((*em.getArchetypesSpawners().at(arche.getCMask()))[i].isValid());
        ASSERT_TRUE((*em.getArchetypesSpawners().at(arche.getCMask()))[i].isAlive());
    }

    em.spawn<TComp1, TComp3>();
    em.spawn<TComp1, TComp3>(5);
}

TEST(EntityManagerTest, Group)
{
    EntityManager em;
    size_t        n = 1000;

    Archetype arche[4];
    arche[0] = makeArchetype<TComp1, TComp2>();
    arche[1] = makeArchetype<TComp1, TComp2, TComp3>();
    arche[2] = makeArchetype<TComp1, TComp2, Component>();
    arche[3] = makeArchetype<TComp1, TComp3>();


    for (size_t i = 0; i < 4; i++)
        em.spawn(arche[i], n);

    em.acceptSpawnedEntities();

    CGroup<TComp1, TComp2> group;
    em.requestCGroup(group, { getCTypeId<Component>() }); // unwanted component type: Component
    // in this filter setup, group will contain entities from archetypes of indices: 0, 1

    size_t counter = 0;
    for (auto pack : group)
    {
        ++counter;
        pack.get<TComp1&>();
        pack.get<TComp2&>();
    }
    ASSERT_EQ(counter, 2 * n);

    counter = 0;
    for (auto it = group.begin(); it != group.end(); ++it)
    {
        counter++;
        // adding/removing components, or killing entities in a loop couses iterators to skip entities that are moved in a place of removed one
        // (in this case it will skip exacly half of all entities in the group: n*2 / 2 = n (n*2 couse there are 2 archetypes matching group's filter)
        em.addComponent<Component>(it.getERef());
    }
    ASSERT_EQ(counter, n);

    // solution to above problem
    counter = 0;
    for (auto it = group.begin(); it != group.end();)
    {
        counter++;
        em.addComponent<Component>(it.getERef());

        // increment only if iterator is invalid (only after adding a component, if there would be no component adding, then that would make an endless loop)
        if (!it.isValid())
            ++it;
    }

    // now its only n, couse earlier we already removed half of the initial number of entities
    ASSERT_EQ(counter, n);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(WorldTest, MakeRemoveSystem)
{
    ECSWorld world;

    auto& system = world.makeSystem<TestSystem>();
    ASSERT_EQ(&system, &world.getSystem<TestSystem>());
    ASSERT_TRUE(world.hasSystem<TestSystem>());
    world.removeSystem<TestSystem>();
    ASSERT_FALSE(world.hasSystem<TestSystem>());

#ifdef _DEBUG
    bool catched = false;
    try
    {
        world.getSystem<TestSystem>();
    }
    catch (std::out_of_range)
    {
        catched = true;
    }
    ASSERT_TRUE(catched);
#endif // _DEBUG
}