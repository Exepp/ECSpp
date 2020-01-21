#include "ComponentsT.h"
#include <ECSpp/Internal/Archetype.h>
#include <gtest/gtest.h>

using namespace epp;


TEST(Archetype, addComponent_hasAllOf_hasAnyOf_Types)
{
    Archetype archeT;
    ASSERT_FALSE(archeT.hasAnyOf(IdOf<TComp1, TComp2, TComp3, TComp4>()));
    ASSERT_FALSE(archeT.hasAnyOf(IdOf<TComp2, TComp3, TComp4>()));

    archeT.addComponent(IdOf<TComp1, TComp1, TComp1>());
    ASSERT_TRUE(archeT.hasAnyOf(IdOf<TComp1, TComp2, TComp3, TComp4>()));
    ASSERT_FALSE(archeT.hasAnyOf(IdOf<TComp2, TComp3, TComp4>()));

    archeT.addComponent(IdOf<TComp2, TComp3>());
    archeT.addComponent(IdOf<TComp1>());
    ASSERT_FALSE(archeT.hasAllOf(IdOf<TComp1, TComp2, TComp3, TComp4>()));
    ASSERT_TRUE(archeT.hasAllOf(IdOf<TComp1, TComp3>()));

    archeT.addComponent(IdOf<TComp2, TComp4>());
    ASSERT_TRUE(archeT.hasAllOf(IdOf<TComp1, TComp2, TComp3, TComp4>()));
    ASSERT_TRUE(archeT.hasAnyOf(IdOf<TComp1, TComp2, TComp3, TComp4>()));

    Archetype copy = archeT;
    ASSERT_TRUE(copy.hasAllOf(IdOf<TComp1, TComp2, TComp3, TComp4>()));
    ASSERT_TRUE(copy.hasAnyOf(IdOf<TComp1, TComp2, TComp3, TComp4>()));

    Archetype moved = std::move(copy);
    ASSERT_TRUE(moved.hasAllOf(IdOf<TComp1, TComp2, TComp3, TComp4>()));
    ASSERT_TRUE(moved.hasAnyOf(IdOf<TComp1, TComp2, TComp3, TComp4>()));
    ASSERT_FALSE(copy.hasAllOf(IdOf<TComp1, TComp2, TComp3, TComp4>()));
    ASSERT_FALSE(copy.hasAnyOf(IdOf<TComp1, TComp2, TComp3, TComp4>()));
}

TEST(Archetype, Remove)
{
    Archetype archeT;
    archeT.removeComponent<TComp1>();
    archeT.removeComponent<TComp2, TComp3>();
    ASSERT_FALSE(archeT.hasAnyOf(IdOf<TComp1, TComp2, TComp3, TComp4>()));

    archeT.addComponent<TComp4, TComp1, TComp3>();
    archeT.removeComponent<TComp1>();
    ASSERT_TRUE(archeT.hasAllOf(IdOf<TComp4, TComp3>()));
    ASSERT_FALSE(archeT.hasAnyOf(IdOf<TComp1, TComp2>()));

    archeT.removeComponent(IdOf<TComp4, TComp3, TComp4>());
    ASSERT_FALSE(archeT.hasAnyOf(IdOf<TComp1, TComp2, TComp3, TComp4>()));

    archeT.addComponent<TComp4, TComp1, TComp3, TComp2>();
    archeT.removeComponent<TComp1, TComp2, TComp3, TComp3>();
    ASSERT_TRUE(archeT.has(IdOf<TComp4>()));
    ASSERT_FALSE(archeT.hasAnyOf(IdOf<TComp1, TComp2, TComp3>()));
}

TEST(Archetype, GetMask)
{
    Archetype archeT;
    CMask cMask;

    archeT.addComponent<TComp1, TComp2, TComp4>();
    cMask.set(IdOf<TComp1, TComp2, TComp4>());
    ASSERT_EQ(archeT.getMask(), cMask);

    archeT.removeComponent<TComp2>();
    ASSERT_NE(archeT.getMask(), cMask);

    cMask.unset(IdOf<TComp2>());
    ASSERT_EQ(archeT.getMask(), cMask);


    Archetype copy = archeT;
    ASSERT_EQ(copy.getMask(), cMask);
    ASSERT_EQ(archeT.getMask(), cMask);

    Archetype moved = std::move(copy);
    ASSERT_EQ(moved.getMask(), cMask);
    ASSERT_EQ(archeT.getMask(), cMask);
    ASSERT_EQ(copy.getMask(), CMask());
}

TEST(Archetype, GetCIds)
{
    Archetype archeT;
    auto cIds = archeT.getCIds();
    ASSERT_TRUE(cIds.empty());

    archeT.addComponent<TComp1, TComp1, TComp1, TComp2, TComp3>(); // many times the same component
    cIds.push_back(IdOf<TComp1>());
    cIds.push_back(IdOf<TComp2>());
    cIds.push_back(IdOf<TComp3>());
    ASSERT_EQ(archeT.getCIds(), cIds);

    archeT.addComponent<TComp2>(); // again the same, in separate call
    ASSERT_EQ(archeT.getCIds(), cIds);

    Archetype copy = archeT;
    ASSERT_EQ(copy.getCIds(), cIds);
    ASSERT_EQ(archeT.getCIds(), cIds);

    Archetype moved = std::move(copy);
    ASSERT_EQ(moved.getCIds(), cIds);
    ASSERT_EQ(archeT.getCIds(), cIds);
    ASSERT_TRUE(copy.getCIds().empty());

    archeT.removeComponent<TComp2, TComp2>();
    ASSERT_NE(archeT.getCIds(), cIds);
    cIds.erase(cIds.begin() + 1);
    ASSERT_EQ(archeT.getCIds(), cIds);

    archeT.removeComponent<TComp2>(); // again the same, in separate call
    ASSERT_EQ(archeT.getCIds(), cIds);
}

TEST(Archetype, Constructors)
{
    Archetype archeT(IdOf<TComp4, TComp3>());
    ASSERT_TRUE(archeT.hasAllOf(IdOf<TComp3, TComp4>()));

    Archetype archeT2(archeT);
    ASSERT_TRUE(archeT2.hasAllOf(IdOf<TComp3, TComp4>()));
    ASSERT_TRUE(archeT.hasAllOf(IdOf<TComp3, TComp4>()));

    Archetype archeT3(std::move(archeT));
    ASSERT_TRUE(archeT3.hasAllOf(IdOf<TComp3, TComp4>()));
    ASSERT_TRUE(archeT2.hasAllOf(IdOf<TComp3, TComp4>())); // second one is still ok
    ASSERT_FALSE(archeT.hasAnyOf(IdOf<TComp3, TComp4>())); // first one is now empty
}