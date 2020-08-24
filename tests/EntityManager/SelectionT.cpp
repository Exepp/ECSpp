#include "ComponentsT.h"
#include <ECSpp/EntityManager.h>
#include <gtest/gtest.h>

using namespace epp;

TEST(Selection, GetMasks)
{
#ifndef EPP_DEBUG
    FAIL();
#endif
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

TEST(Selection, ForEach)
{
    EntityManager mgr;
    Selection<TComp2, TComp3> sel;
    Archetype arch[] = { Archetype(IdOf<TComp1, TComp3>()),
                         Archetype(IdOf<TComp1, TComp3, TComp2>()),
                         Archetype(IdOf<TComp2, TComp3, TComp4>()) };
    int n = 0;
    auto good1 = [&]() { return TComp1(TComp1::Arr_t{ n, n, n }); };
    auto good2 = [&]() { return TComp2(TComp2::Arr_t{ n % 10, n % 2, n % 5 }); };
    auto good3 = [&]() { return TComp3(TComp3::Arr_t{ -n, -n, -n }); };
    auto good4 = [&]() { return TComp4(TComp4::Arr_t{ 100 - n, 500 - n, 1000 - n }); };
    auto fn = [&](EntityCreator&& creator) {
            if (creator.getCMask().get(IdOf<TComp1>()))
                creator.constructed<TComp1>(good1());
            if (creator.getCMask().get(IdOf<TComp2>()))
                creator.constructed<TComp2>(good2());
            if (creator.getCMask().get(IdOf<TComp3>()))
                creator.constructed<TComp3>(good3());
            if (creator.getCMask().get(IdOf<TComp4>()))
                creator.constructed<TComp4>(good4()); };

    for (; n < 1024; ++n)
        mgr.spawn(arch[0], fn);
    for (; n < 2 * 1024; ++n)
        mgr.spawn(arch[1], fn);
    for (; n < 3 * 1024; ++n)
        mgr.spawn(arch[2], fn);
    mgr.updateSelection(sel);

    n = 1024; // skip entities of arch[0] archetype
    sel.forEach([&](Entity ent, TComp2& c2, TComp3& c3) {
        ASSERT_NE(mgr.archetypeOf(ent).getMask(), arch[0].getMask());
        if (n < 2 * 1024) {
            ASSERT_EQ(ent, mgr.entitiesOf(arch[1]).data[n - 1024]);
            ASSERT_TRUE(mgr.maskOf(ent).get(IdOf<TComp2>()));
        }
        else if (n < 3 * 1024) {
            ASSERT_EQ(ent, mgr.entitiesOf(arch[2]).data[n - 2 * 1024]);
            ASSERT_TRUE(mgr.maskOf(ent).get(IdOf<TComp3>()));
        }
        ASSERT_TRUE(mgr.maskOf(ent).contains(CMask(IdOf<TComp2, TComp3>())));
        ASSERT_EQ(mgr.componentOf<TComp2>(ent), good2());
        ASSERT_EQ(mgr.componentOf<TComp3>(ent), good3());
        ++n;
    });
}

TEST(Selection, Empty)
{
    EntityManager mgr;
    Selection<> sel;
    Archetype arch[] = { Archetype(IdOf<TComp1, TComp3>()),
                         Archetype(IdOf<TComp1, TComp3, TComp2>()),
                         Archetype(IdOf<TComp1, TComp3, TComp4>()) };
    int n = 0;
    auto fn = [&n = std::as_const(n)](EntityCreator&& creator) {
        creator.constructed<TComp1>(TComp1::Arr_t{n, n, n});
        creator.constructed<TComp3>(TComp3::Arr_t{ -n, -n, -n }); };

    for (; n < 1024; ++n)
        mgr.spawn(arch[0], fn);
    for (; n < 2 * 1024; ++n)
        mgr.spawn(arch[1], fn);
    for (; n < 3 * 1024; ++n)
        mgr.spawn(arch[2], fn);
    mgr.updateSelection(sel);

    n = 0;
    sel.forEach([&](Entity ent) {
        if (n < 1024)
            ASSERT_EQ(ent, mgr.entitiesOf(arch[0]).data[n]);
        else if (n < 2 * 1024) {
            ASSERT_EQ(ent, mgr.entitiesOf(arch[1]).data[n - 1024]);
            ASSERT_TRUE(mgr.maskOf(ent).get(IdOf<TComp2>()));
        }
        else if (n < 3 * 1024) {
            ASSERT_EQ(ent, mgr.entitiesOf(arch[2]).data[n - 2 * 1024]);
            ASSERT_TRUE(mgr.maskOf(ent).get(IdOf<TComp4>()));
        }
        ASSERT_TRUE(mgr.maskOf(ent).contains(CMask(IdOf<TComp1, TComp3>())));
        ASSERT_EQ(mgr.componentOf<TComp1>(ent), TComp1(TComp1::Arr_t{ n, n, n }));
        ASSERT_EQ(mgr.componentOf<TComp3>(ent), TComp3(TComp1::Arr_t{ -n, -n, -n }));
        ++n;
    });
}

// TODO: forEach