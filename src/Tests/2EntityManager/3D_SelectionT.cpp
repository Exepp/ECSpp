#include "ComponentsT.h"
#include <ECSpp/EntityManager.h>
#include <gtest/gtest.h>

using namespace epp;

TEST(Selection, GetMasks)
{
    EntityManager mgr;
    CMask wanted;
    CMask unwanted;

    wanted = CMask(IdOf<TComp1, TComp4>());
    Selection<TComp1, TComp4> selection1;
    ASSERT_EQ(selection1.getWanted(), wanted);
    ASSERT_EQ(selection1.getUnwanted(), unwanted);

    wanted = CMask(IdOf<TComp2, TComp3>());
    unwanted = CMask(IdOf<TComp1, TComp4>());
    Selection<TComp2, TComp3> selection2(IdOf<TComp1, TComp4>());
    ASSERT_EQ(selection2.getWanted(), wanted);
    ASSERT_EQ(selection2.getUnwanted(), unwanted);

    wanted = CMask(IdOf<TComp2, TComp3>());
    unwanted = CMask(IdOf<TComp1, TComp2, TComp3, TComp4>());
    Selection<TComp2, TComp3> selection3(IdOf<TComp1, TComp2, TComp3, TComp4>());
    ASSERT_EQ(selection3.getWanted(), wanted);
    ASSERT_NE(selection3.getUnwanted(), unwanted);
    ASSERT_EQ(selection3.getUnwanted(), unwanted.removeCommon(wanted));
}

TEST(Selection, UpdateSelection_Begin_End)
{
    EntityManager mgr;
    Selection<TComp1> selection;
    Archetype arch1(IdOf<TComp1, TComp3>());
    Archetype arch2(IdOf<TComp1, TComp3, TComp2>());
    Archetype arch3(IdOf<TComp1, TComp4>());
    int i = 0;

    ASSERT_THROW(selection.end()++, AssertFailed);
    ASSERT_THROW(++selection.end(), AssertFailed);

    for (auto it = selection.begin(); it != selection.end(); ++it)
        ++i;
    ASSERT_EQ(i, 0);

    mgr.spawn(arch1);
    for (auto it = selection.begin(); it != selection.end(); ++it)
        ++i;
    ASSERT_EQ(i, 0);

    mgr.updateSelection(selection);
    for (auto it = selection.begin(); it != selection.end(); ++it)
        ++i;
    ASSERT_EQ(i, 1);

    i = 0;
    for (auto it = selection.begin(); it != selection.end(); ++it)
        if (++i == 1) {
            mgr.spawn(arch1);
            mgr.spawn(arch2);
            mgr.spawn(arch3);
        }
    ASSERT_EQ(i, 2); // no updateSelection was called, only iterated over arch1 entities

    i = 0;
    for (auto it = selection.begin(); it != selection.end(); ++it)
        if (++i == 1) {
            mgr.spawn(arch1);
            mgr.spawn(arch2);
            mgr.spawn(arch3);
            mgr.updateSelection(selection);
        }
    ASSERT_EQ(i, 7); // 4 older ones + 3 new

    i = 0;
    for (auto it = selection.begin(); it != selection.end(); ++it)
        if (++i == 4) { // omit first 3 entities (all of arch1)
            mgr.spawn(arch1);
            mgr.updateSelection(selection);
        }
    ASSERT_EQ(i, 7); // 7 not 8, iterator already passed the spawner of arch1

    i = 0;
    ASSERT_NO_THROW(
        for (auto ent
             : selection) // range based
        if (++i == 1) {
            mgr.spawn(Archetype(IdOfL<TComp1>()));                // new archetype
            mgr.clear(Archetype(IdOfL<TComp1>()));                // remove before iteration (check if end will update)
            mgr.spawn(Archetype(IdOf<TComp1, TComp3, TComp4>())); // but also check if it will iterate over
                                                                  // entities spawned inside the loop
            mgr.updateSelection(selection);
        });
    ASSERT_EQ(i, 9);
    mgr.spawn(Archetype(IdOfL<TComp1>())); // spawn without update (updated in previous test)
    i = 0;
    for (auto ent : selection)
        ++i;
    ASSERT_EQ(i, 10);

    i = 0;
    mgr.clear();
    for (auto pack : selection)
        ++i;
    ASSERT_EQ(i, 0);

    mgr.spawn(arch1);
    mgr.spawn(arch2);
    mgr.spawn(arch3);
    mgr.spawn(Archetype(IdOfL<TComp1>()));
    mgr.spawn(Archetype(IdOf<TComp1, TComp3, TComp4>()));
    i = 0;
    for (auto ent : selection) // no updates needed after clear
        ++i;
    ASSERT_EQ(i, 5);

    ASSERT_THROW(selection.end()++, AssertFailed);
    ASSERT_THROW(++selection.end(), AssertFailed);
}


TEST(Selection, IterationValidation)
{
    EntityManager mgr;
    Selection<TComp3, TComp1> selection;
    Archetype arch[] = { Archetype(IdOf<TComp1, TComp3>()),
                         Archetype(IdOf<TComp1, TComp3, TComp2>()),
                         Archetype(IdOf<TComp1, TComp3, TComp4>()) };
    int n = 0;
    auto fn = [& n = std::as_const(n)](EntityCreator&& creator) {
        creator.constructed<TComp1>(TComp1::Arr_t{n, n, n});
        creator.constructed<TComp3>(std::array<int, 3>{ -n, -n, -n }); };

    for (; n < 1024; ++n)
        mgr.spawn(arch[0], fn);
    for (; n < 2 * 1024; ++n)
        mgr.spawn(arch[1], fn);
    for (; n < 3 * 1024; ++n)
        mgr.spawn(arch[2], fn);
    mgr.updateSelection(selection);

    int i = 0;
    for (auto it = selection.begin(); it != selection.end(); ++it, ++i) {
        ASSERT_EQ(it.getComponent<TComp1>(), TComp1({ i, i, i }));
        ASSERT_EQ(it.getComponent<TComp3>(), TComp3({ -i, -i, -i }));
    }

    i = 1.5 * 1024;
    for (auto it = selection.begin() + i; it != selection.end(); ++it, ++i) {
        ASSERT_EQ(it.getComponent<TComp1>(), TComp1({ i, i, i }));
        ASSERT_EQ(it.getComponent<TComp3>(), TComp3({ -i, -i, -i }));
    }


    int k = 0;
    i = mgr.size(arch[k]);
    for (auto it = selection.begin(); it != selection.end();) { // no auto-increment
        if (mgr.size(arch[k]) == 1) {
            ++k;
            if (k == 3)
                break;
            i = k * 1024 + mgr.size(arch[k]);
            it = mgr.destroy(it);
        }
        it = mgr.destroy(it);
        --i;
        ASSERT_EQ(it.getComponent<TComp1>(), TComp1({ i, i, i }));
        ASSERT_EQ(it.getComponent<TComp3>(), TComp3({ -i, -i, -i }));
    }
}

TEST(Selection, Empty)
{
    EntityManager mgr;
    Selection<> sel;
    Archetype arch[] = { Archetype(IdOf<TComp1, TComp3>()),
                         Archetype(IdOf<TComp1, TComp3, TComp2>()),
                         Archetype(IdOf<TComp1, TComp3, TComp4>()) };
    int n = 0;
    auto fn = [& n = std::as_const(n)](EntityCreator&& creator) {
        creator.constructed<TComp1>(TComp1::Arr_t{n, n, n});
        creator.constructed<TComp3>(TComp3::Arr_t{ -n, -n, -n }); };

    for (; n < 1024; ++n)
        mgr.spawn(arch[0], fn);
    for (; n < 2 * 1024; ++n)
        mgr.spawn(arch[1], fn);
    for (; n < 3 * 1024; ++n)
        mgr.spawn(arch[2], fn);
    mgr.updateSelection(sel);

    int i = 0;
    for (auto it = sel.begin(); it != sel.end(); ++it, ++i) {
        if (n < 1024)
            ASSERT_EQ(*it, mgr.entitiesOf(arch[0]).data[i]);
        else if (n < 2 * 1024) {
            ASSERT_EQ(*it, mgr.entitiesOf(arch[1]).data[i - 1024]);
            ASSERT_TRUE(mgr.maskOf(*it).get(IdOf<TComp2>()));
        }
        else if (n < 3 * 1024) {
            ASSERT_EQ(*it, mgr.entitiesOf(arch[2]).data[i - 2 * 1024]);
            ASSERT_TRUE(mgr.maskOf(*it).get(IdOf<TComp4>()));
        }
        ASSERT_TRUE(mgr.maskOf(*it).contains(CMask(IdOf<TComp1, TComp3>())));
    }
}

TEST(Selection, Iterator)
{
    EntityManager mgr;
    Selection<TComp3 const, TComp1 const> sel;
    Archetype arch[] = { Archetype(IdOf<TComp1, TComp3>()),
                         Archetype(IdOf<TComp1, TComp3, TComp2>()),
                         Archetype(IdOf<TComp1, TComp3, TComp4>()) };
    int n = 0;
    auto fn = [& n = std::as_const(n)](EntityCreator&& creator) {
        creator.constructed<TComp1>(TComp1::Arr_t{n, n, n});
        creator.constructed<TComp3>(TComp3::Arr_t{ -n, -n, -n }); };

    for (; n < 1024; ++n)
        mgr.spawn(arch[0], fn);
    for (; n < 2 * 1024; ++n)
        mgr.spawn(arch[1], fn);
    for (; n < 3 * 1024; ++n)
        mgr.spawn(arch[2], fn);
    mgr.updateSelection(sel);

    auto it1 = sel.begin();
    auto it2 = it1 + sel.countEntities() / 4;
    auto it3 = it2 + sel.countEntities() / 4;
    auto it4 = it3 + sel.countEntities() / 4;
    auto it5 = sel.end();

    int cn = 0;
    for (; it1 != it2; ++it1, ++cn) {
        ASSERT_EQ(it1.getComponent<TComp1>(), TComp1({ cn, cn, cn }));
        ASSERT_EQ(it1.getComponent<TComp3>(), TComp3({ -cn, -cn, -cn }));
    }
    for (; it2 != it3; ++it2, ++cn) {
        ASSERT_EQ(it2.getComponent<TComp1>(), TComp1({ cn, cn, cn }));
        ASSERT_EQ(it2.getComponent<TComp3>(), TComp3({ -cn, -cn, -cn }));
    }
    for (; it3 != it4; ++it3, ++cn) {
        ASSERT_EQ(it3.getComponent<TComp1>(), TComp1({ cn, cn, cn }));
        ASSERT_EQ(it3.getComponent<TComp3>(), TComp3({ -cn, -cn, -cn }));
    }
    for (; it4 != it5; ++it4, ++cn) {
        ASSERT_EQ(it4.getComponent<TComp1>(), TComp1({ cn, cn, cn }));
        ASSERT_EQ(it4.getComponent<TComp3>(), TComp3({ -cn, -cn, -cn }));
    }
    ASSERT_EQ(cn, n);

    it1 = sel.begin();
    it2 = it1 + sel.countEntities() / 4;
    it3 = it2 + sel.countEntities() / 4;
    it4 = it3 + sel.countEntities() / 4;
    it5 = sel.end();

    ASSERT_EQ(it1.jumpToOrBeyond(it2.getSpawnerId(), it2.getPoolIdx()), it2);
    ASSERT_EQ(it1.jumpToOrBeyond(it2.getSpawnerId(), it2.getPoolIdx()), it2);
    ASSERT_EQ(it1.jumpToOrBeyond(it3.getSpawnerId(), it3.getPoolIdx()), it3);
    ASSERT_EQ(it1.jumpToOrBeyond(it4.getSpawnerId(), it4.getPoolIdx()), it4);
    ASSERT_EQ(it1.jumpToOrBeyond(SpawnerId(132), PoolIdx(0)), it5);

    it1 = sel.begin();
    auto ent = mgr.spawn(arch[1]);
    ASSERT_EQ(*it1.jumpToOrBeyond(mgr.cellOf(ent)), ent);
    ASSERT_EQ(*it1.jumpToOrBeyond(mgr.cellOf(ent)), ent);
    ASSERT_EQ(it1.jumpToOrBeyond(it4.getSpawnerId(), it4.getPoolIdx()), it4);
    ASSERT_EQ(it1.jumpToOrBeyond(it2.getSpawnerId(), it2.getPoolIdx()), it5); // cannot jump back, points to the end
    ASSERT_EQ(sel.begin().jumpToOrBeyond(it4.getSpawnerId(), it4.getPoolIdx()), it4);
}