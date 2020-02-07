#include "ComponentsT.h"

#include <ECSpp/internal/CMask.h>
#include <gtest/gtest.h>

using namespace epp;

template <typename SetT1, typename SetT2, typename UnsetT1, typename UnsetT2>
static void testComps(CMask const& cmask)
{
    EXPECT_TRUE(cmask.get(IdOf<SetT1>()) && cmask.get(IdOf<SetT2>()));
    EXPECT_FALSE(cmask.get(IdOf<UnsetT1>()) | cmask.get(IdOf<UnsetT2>()));
    EXPECT_EQ(cmask.getSetCount(), 2);
    EXPECT_EQ(cmask.numberOfCommon(IdOf<SetT1, SetT2>()), 2);
    EXPECT_EQ(cmask.numberOfCommon(IdOf<UnsetT1, UnsetT2>()), 0);
    EXPECT_EQ(cmask.numberOfCommon(IdOf<SetT1, UnsetT2>()), 1);
    EXPECT_TRUE(cmask.hasCommon(IdOf<SetT1, UnsetT2>()));
    EXPECT_FALSE(cmask.hasCommon(IdOf<UnsetT1, UnsetT2>()));
    EXPECT_TRUE(cmask.contains(IdOfL<SetT1>()) & cmask.contains(IdOfL<SetT2>()) & cmask.contains(IdOf<SetT1, SetT2>()));
    EXPECT_FALSE(cmask.contains(IdOf<SetT1, UnsetT1>()) | cmask.contains(IdOf<SetT2, UnsetT2>()));
    EXPECT_FALSE(cmask.contains(IdOf<SetT1, UnsetT1, SetT2, UnsetT2>()));
    EXPECT_EQ(cmask, CMask(IdOf<SetT1, SetT2>()));
    EXPECT_NE(cmask, CMask(IdOf<SetT1, SetT2, UnsetT1, UnsetT2>()));
    EXPECT_NE(cmask, CMask(IdOf<SetT1, UnsetT2>()));
    EXPECT_NE(cmask, CMask());
}

TEST(CMask, DefaultConstr)
{
    CMask cmask;
    EXPECT_FALSE(cmask.get(IdOf<TComp1>()) | cmask.get(IdOf<TComp2>()) | cmask.get(IdOf<TComp3>()) | cmask.get(IdOf<TComp4>()));
    EXPECT_EQ(cmask.getSetCount(), 0);
    EXPECT_EQ(cmask.numberOfCommon(IdOf<TComp1, TComp2, TComp3, TComp4>()), 0);
    EXPECT_FALSE(cmask.hasCommon(IdOf<TComp1, TComp2, TComp3, TComp4>()));
    EXPECT_FALSE(cmask.contains(IdOfL<TComp1>()) | cmask.contains(IdOfL<TComp2>()) | cmask.contains(IdOfL<TComp3>()) | cmask.contains(IdOfL<TComp4>()));
    EXPECT_FALSE(cmask.contains(IdOf<TComp1, TComp2, TComp3, TComp4>()));
}


TEST(CMask, ListConstr)
{
    CMask cmask(IdOf<TComp1, TComp3>());
    testComps<TComp1, TComp3, TComp2, TComp4>(cmask);

    EXPECT_NO_THROW(CMask({ ComponentId(CMetadata::MaxRegisteredComponents - 1) }));
    EXPECT_THROW(CMask({ ComponentId(CMetadata::MaxRegisteredComponents) }), std::exception);
}

TEST(CMask, TempConstr)
{
    CMask cmask = CMask(IdOf<TComp2, TComp4>());
    testComps<TComp2, TComp4, TComp1, TComp3>(cmask);
}

TEST(CMask, MoveConstr)
{
    CMask test(IdOf<TComp1, TComp4>());
    CMask cmask = std::move(test);
    testComps<TComp1, TComp4, TComp2, TComp3>(cmask);

    EXPECT_FALSE(test.hasCommon(IdOf<TComp1, TComp2, TComp3, TComp4>()));
}

TEST(CMask, MoveAssign)
{
    CMask test(IdOf<TComp1, TComp2>());
    CMask cmask(IdOfL<TComp3>());
    cmask = std::move(test);
    testComps<TComp1, TComp2, TComp3, TComp4>(cmask);

    EXPECT_FALSE(test.hasCommon(IdOf<TComp1, TComp2, TComp3, TComp4>()));
}

TEST(CMask, CopyConstr)
{
    CMask test(IdOf<TComp1, TComp3>());
    CMask cmask = test;
    testComps<TComp1, TComp3, TComp2, TComp4>(cmask);

    EXPECT_TRUE(test.contains(IdOf<TComp1, TComp3>()));
}

TEST(CMask, CopyAssign)
{
    CMask test(IdOf<TComp3, TComp4>());
    CMask cmask(IdOfL<TComp1>());
    cmask = test;
    testComps<TComp3, TComp4, TComp1, TComp2>(cmask);

    EXPECT_TRUE(test.contains(IdOf<TComp3, TComp4>()));
}

TEST(CMask, Set)
{
    CMask cmask;
    cmask.set(IdOf<TComp1>());
    cmask.set(IdOf<TComp3>());
    testComps<TComp1, TComp3, TComp2, TComp4>(cmask);

    EXPECT_NO_THROW(cmask.set(ComponentId(CMetadata::MaxRegisteredComponents - 1)));
    EXPECT_THROW(cmask.set(ComponentId(CMetadata::MaxRegisteredComponents)), std::exception);
}

TEST(CMask, SetList)
{
    CMask cmask;
    cmask.set({ IdOf<TComp1>(), IdOf<TComp3>() });
    testComps<TComp1, TComp3, TComp2, TComp4>(cmask);

    EXPECT_NO_THROW(cmask.set({ ComponentId(CMetadata::MaxRegisteredComponents - 1) }));
    EXPECT_THROW(cmask.set({ ComponentId(CMetadata::MaxRegisteredComponents) }), std::exception);
}

TEST(CMask, Unset)
{
    CMask cmask(IdOf<TComp1, TComp2, TComp3, TComp4>());
    cmask.unset(IdOf<TComp2>());
    cmask.unset(IdOf<TComp4>());
    testComps<TComp1, TComp3, TComp2, TComp4>(cmask);

    EXPECT_NO_THROW(cmask.unset(ComponentId(CMetadata::MaxRegisteredComponents - 1)));
    EXPECT_THROW(cmask.unset(ComponentId(CMetadata::MaxRegisteredComponents)), std::exception);
}

TEST(CMask, UnsetList)
{
    CMask cmask(IdOf<TComp1, TComp2, TComp3, TComp4>());
    cmask.unset(IdOf<TComp2, TComp4>());
    testComps<TComp1, TComp3, TComp2, TComp4>(cmask);

    EXPECT_NO_THROW(cmask.unset({ ComponentId(CMetadata::MaxRegisteredComponents - 1) }));
    EXPECT_THROW(cmask.unset({ ComponentId(CMetadata::MaxRegisteredComponents) }), std::exception);
}

TEST(CMask, RemoveCommon)
{
    CMask cmask;
    cmask.removeCommon(IdOf<TComp2, TComp4>());
    EXPECT_FALSE(cmask.hasCommon(IdOf<TComp1, TComp2, TComp3, TComp4>()));

    cmask.set(IdOf<TComp1, TComp2, TComp3, TComp4>());
    cmask.removeCommon(IdOf<TComp2, TComp4>());
    testComps<TComp1, TComp3, TComp2, TComp4>(cmask);
}

TEST(CMask, Clear)
{
    CMask cmask;
    cmask.clear();
    EXPECT_FALSE(cmask.hasCommon(IdOf<TComp1, TComp2, TComp3, TComp4>()));

    cmask.set(IdOf<TComp1, TComp2, TComp3, TComp4>());
    cmask.clear();
    EXPECT_FALSE(cmask.hasCommon(IdOf<TComp1, TComp2, TComp3, TComp4>()));
}