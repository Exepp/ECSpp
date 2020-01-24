#include "ComponentsT.h"
#include <ECSpp/EntityManager.h>
#include <gtest/gtest.h>

using namespace epp;

TEST(EntityCollection, GetFilter)
{
    EntityManager mgr;
    CFilter filter;

    filter = CFilter(IdOf<TComp1, TComp4>(), {});
    EntityCollection<TComp1, TComp4> collection1;
    ASSERT_EQ(collection1.getFilter(), filter);

    filter = CFilter(IdOf<TComp2, TComp3>(), IdOf<TComp1, TComp4>());
    EntityCollection<TComp2, TComp3> collection3(IdOf<TComp1, TComp4>());
    ASSERT_EQ(collection3.getFilter(), filter);

    filter = CFilter(IdOf<TComp2, TComp3>(), IdOf<TComp1, TComp2, TComp3, TComp4>());
    EntityCollection<TComp2, TComp3> collection4(IdOf<TComp1, TComp2, TComp3, TComp4>());
    ASSERT_EQ(collection4.getFilter(), filter);
}

TEST(EntityCollection, UpdateCollection_Begin_End)
{
    EntityManager mgr;
    EntityCollection<TComp1> collection;
    Archetype arch1(IdOf<TComp1, TComp3>());
    Archetype arch2(IdOf<TComp1, TComp3, TComp2>());
    Archetype arch3(IdOf<TComp1, TComp4>());
    int i = 0;

    ASSERT_THROW(collection.end()++, AssertFailed);
    ASSERT_THROW(++collection.end(), AssertFailed);

    for (auto it = collection.begin(); it != collection.end(); ++it)
        ++i;
    ASSERT_EQ(i, 0);

    mgr.spawn(arch1);
    for (auto it = collection.begin(); it != collection.end(); ++it)
        ++i;
    ASSERT_EQ(i, 0);

    mgr.updateCollection(collection);
    for (auto it = collection.begin(); it != collection.end(); ++it)
        ++i;
    ASSERT_EQ(i, 1);

    i = 0;
    for (auto it = collection.begin(); it != collection.end(); ++it)
        if (++i == 1) {
            mgr.spawn(arch1);
            mgr.spawn(arch2);
            mgr.spawn(arch3);
        }
    ASSERT_EQ(i, 2); // no updateCollection was called, only iterated over arch1 entities

    i = 0;
    for (auto it = collection.begin(); it != collection.end(); ++it)
        if (++i == 1) {
            mgr.spawn(arch1);
            mgr.spawn(arch2);
            mgr.spawn(arch3);
            mgr.updateCollection(collection);
        }
    ASSERT_EQ(i, 7); // 4 older ones + 3 new

    i = 0;
    for (auto it = collection.begin(); it != collection.end(); ++it)
        if (++i == 4) { // omit first 3 entities (all of arch1)
            mgr.spawn(arch1);
            mgr.updateCollection(collection);
        }
    ASSERT_EQ(i, 7); // 7 not 8, iterator already passed the spawner of arch1

    i = 0;
    ASSERT_NO_THROW(
        for (auto ent
             : collection) // range based
        if (++i == 1) {
            mgr.spawn(Archetype(IdOfL<TComp1>()));                // new archetype
            mgr.clear(Archetype(IdOfL<TComp1>()));                // remove before iteration (check if end will update)
            mgr.spawn(Archetype(IdOf<TComp1, TComp3, TComp4>())); // but also check if it will iterate over
                                                                  // entities spawned inside the loop
            mgr.updateCollection(collection);
        });
    ASSERT_EQ(i, 9);
    mgr.spawn(Archetype(IdOfL<TComp1>())); // spawn without update (updated in previous test)
    i = 0;
    for (auto ent : collection)
        ++i;
    ASSERT_EQ(i, 10);

    i = 0;
    mgr.clear();
    for (auto pack : collection)
        ++i;
    ASSERT_EQ(i, 0);

    mgr.spawn(arch1);
    mgr.spawn(arch2);
    mgr.spawn(arch3);
    mgr.spawn(Archetype(IdOfL<TComp1>()));
    mgr.spawn(Archetype(IdOf<TComp1, TComp3, TComp4>()));
    i = 0;
    for (auto ent : collection) // no updates needed after clear
        ++i;
    ASSERT_EQ(i, 5);

    ASSERT_THROW(collection.end()++, AssertFailed);
    ASSERT_THROW(++collection.end(), AssertFailed);
}


TEST(EntityCollection, IterationValidation)
{
    EntityManager mgr;
    EntityCollection<TComp3, TComp1> collection;
    Archetype arch[] = { Archetype(IdOf<TComp1, TComp3>()),
                         Archetype(IdOf<TComp1, TComp3, TComp2>()),
                         Archetype(IdOf<TComp1, TComp3, TComp4>()) };
    int n = 0;
    auto fn = [& n = std::as_const(n)](EntityCreator&& creator) {
        creator.constructed<TComp1>(n, n, n);
        creator.constructed<TComp3>(std::array<int, 3>{ -n, -n, -n }); };

    for (; n < 1024; ++n)
        mgr.spawn(arch[0], fn);
    for (; n < 2 * 1024; ++n)
        mgr.spawn(arch[1], fn);
    for (; n < 3 * 1024; ++n)
        mgr.spawn(arch[2], fn);
    mgr.updateCollection(collection);

    int i = 0;
    for (auto it = collection.begin(); it != collection.end(); ++it, ++i) {
        ASSERT_EQ(it.getComponent<TComp1>(), TComp1(i, i, i));
        ASSERT_EQ(it.getComponent<TComp3>(), TComp3({ -i, -i, -i }));
    }

    int k = 0;
    i = mgr.size(arch[k]);
    for (auto it = collection.begin(); it != collection.end();) { // no auto-increment
        if (mgr.size(arch[k]) == 1) {
            ++k;
            if (k == 3)
                break;
            i = k * 1024 + mgr.size(arch[k]);
            it = mgr.destroy(it);
        }
        it = mgr.destroy(it);
        --i;
        ASSERT_EQ(it.getComponent<TComp1>(), TComp1(i, i, i));
        ASSERT_EQ(it.getComponent<TComp3>(), TComp3({ -i, -i, -i }));
    }
}