#include "ComponentsT.h"

#include <ECSpp/EntityManager/Archetype.h>
#include <gtest/gtest.h>

using namespace epp;

TEST(ArchetypeTest, addComponent_hasAllOf_hasAnyOf)
{
    Archetype archeT;
    archeT.addComponent<TComp1>();
    archeT.addComponent<TComp2, TComp3>();
    archeT.addComponent<TComp1>();
    archeT.addComponent<TComp1, TComp4>();
    archeT.hasAllOf<TComp1, TComp2, TComp3, TComp4>();
}

TEST(ArchetypeTest, Remove)
{
    Archetype archeT;
    archeT.removeComponent<TComp1>();
    archeT.removeComponent<TComp2, TComp3>();
    archeT.removeComponent<TComp1>();
    archeT.removeComponent(IDOfComp<TComp1, TComp4>());
    archeT.hasAllOf<TComp1, TComp2, TComp3, TComp4>();
}

TEST(ArchetypeTest, Reset)
{
    Archetype archeT;
    archeT.addComponent<TComp1, TComp2>();
    archeT.reset();
    ASSERT_FALSE((archeT.hasAllOf<TComp1, TComp2>()));
}

TEST(ArchetypeTest, Copy_Mask)
{
    Archetype archeT;
    Bitmask   cMask;

    archeT.addComponent<TComp1, TComp2, TComp3>();
    cMask.set({ ComponentUtility::ID<TComp1>, ComponentUtility::ID<TComp2>, ComponentUtility::ID<TComp3> });

    ASSERT_EQ(archeT.getMask(), cMask);
    Archetype copy = archeT;
    ASSERT_EQ(copy.getMask(), cMask);

    archeT.reset();
    cMask.clear();
    ASSERT_EQ(archeT.getMask(), cMask);
}