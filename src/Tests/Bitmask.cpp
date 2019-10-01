#include <ECSpp/Utility/Bitmask.h>
#include <gtest/gtest.h>

using namespace epp;

TEST(BitmaskTest, Set_Unset_Get_InitializerList)
{
    Bitmask bitmask;

    bitmask.set(1);
    EXPECT_TRUE(bitmask.get(1) && !bitmask.get(0) && !bitmask.get(2));

    bitmask.set(123);
    EXPECT_TRUE(bitmask.get(123) && bitmask.get(1));

    bitmask.unset(1);
    EXPECT_FALSE(bitmask.get(1));
    EXPECT_TRUE(bitmask.get(123));

    bitmask.unset(1); // unset for 0
    EXPECT_FALSE(bitmask.get(1));

    // get/unset for previously not-set index
    EXPECT_FALSE(bitmask.get(1024));
    bitmask.unset(1024);
    EXPECT_FALSE(bitmask.get(1024));

    bitmask.unset(123);
    bitmask.set({ 0, 1, 2 });
    EXPECT_TRUE(bitmask.get(0) && bitmask.get(1) && bitmask.get(2));

    // check if setting the same bit more than once changes anything
    bitmask.set({ 63, 64, 63, 64 });
    EXPECT_TRUE(bitmask.get(63) && bitmask.get(64));

    bitmask.unset({ 0, 2 });
    EXPECT_TRUE(!bitmask.get(0) && bitmask.get(1) && !bitmask.get(2));

    // check if unsetting the same bit more than once changes anything
    bitmask.unset({ 63, 64, 63, 64 });
    EXPECT_FALSE(bitmask.get(63) || bitmask.get(64));

    bitmask = Bitmask({ 0, 1, 64, 128 });
    EXPECT_TRUE(bitmask.get(0) && bitmask.get(1) && bitmask.get(64) && bitmask.get(128));
}

TEST(BitmaskTest, Operators)
{
    Bitmask bitmask1{ 0, 5, 7 };
    Bitmask bitmask2{ 0, 1, 4, 7 };

    {
        EXPECT_EQ(bitmask1 & bitmask2, bitmask1 &= bitmask2);
        EXPECT_TRUE(bitmask1.get(0) && !bitmask1.get(5) && bitmask1.get(7));
        EXPECT_FALSE(bitmask1.get(1) || bitmask1.get(4));

        bitmask1.set({ 1, 4 });
        EXPECT_EQ(bitmask1, bitmask2); // operator ==

        bitmask1.unset({ 0, 1, 4, 7 });
        EXPECT_EQ(bitmask1, Bitmask());
        EXPECT_NE(bitmask1, bitmask2); // operator !=
    }

    {
        bitmask1.set(2);
        EXPECT_EQ(bitmask1 | bitmask2, bitmask1 |= bitmask2);
        EXPECT_TRUE(bitmask1.get(0) && bitmask1.get(1) && bitmask1.get(2) && bitmask1.get(4) && bitmask1.get(7));
        EXPECT_NE(bitmask1, bitmask2);

        bitmask1.unset(2);
        EXPECT_EQ(bitmask1, bitmask2);
    }

    bitmask1 = bitmask2;
    EXPECT_EQ(bitmask1, bitmask2);

    bitmask1.unset(0);
    EXPECT_NE(bitmask1, bitmask2);
    EXPECT_LT(bitmask1, bitmask2); // operator <
    EXPECT_GT(bitmask2, bitmask1); // operator >

    bitmask1.set({ 127, 128 });
    EXPECT_GT(bitmask1, bitmask2); // operator >
    EXPECT_LT(bitmask2, bitmask1); // operator <

    bitmask1.unset({ 127, 128 });
    EXPECT_LT(bitmask1, bitmask2); // operator <
    EXPECT_GT(bitmask2, bitmask1); // operator >
}

TEST(BitmaskTest, Clear_Count)
{
    Bitmask bitmask;

    EXPECT_EQ(bitmask.getSetCount(), 0);

    bitmask.set({ 1, 123, 1, 123 });
    EXPECT_EQ(bitmask.getSetCount(), 2);

    bitmask.unset({ 1 });
    EXPECT_EQ(bitmask.getSetCount(), 1);

    bitmask.clear();
    EXPECT_EQ(bitmask.getSetCount(), 0);

    bitmask.set({ 120, 0, 1024 });
    EXPECT_EQ(bitmask.getSetCount(), 3);

    bitmask.unset({ 120, 120, 0, 0 });
    bitmask.set({ 123, 1024 });
    EXPECT_EQ(bitmask.getSetCount(), 2);

    bitmask.unset({ 123, 1024 });
    EXPECT_EQ(bitmask.getSetCount(), 0);
}

TEST(BitmaskTest, Common)
{
    Bitmask bitmask1{ 64, 5, 77, 128 };
    Bitmask bitmask2{ 64, 1, 4, 128 };

    Bitmask zeroCommonMask({ 1, 2, 512 });
    ASSERT_FALSE(bitmask1.hasCommon(zeroCommonMask));
    ASSERT_TRUE(bitmask1.hasCommon(bitmask2));
    ASSERT_EQ(bitmask1.numberOfCommon(bitmask2), 2);

    ASSERT_FALSE(bitmask1.contains(bitmask2));
    bitmask2.unset({ 1, 4 });
    ASSERT_TRUE(bitmask1.contains(bitmask2));

    bitmask1.removeCommon(zeroCommonMask);
    ASSERT_EQ(bitmask1, Bitmask({ 64, 5, 77, 128 }));

    bitmask2 |= zeroCommonMask;
    bitmask2.removeCommon(bitmask1);
    ASSERT_EQ(bitmask1.numberOfCommon(bitmask2), 0);
    ASSERT_EQ(bitmask2, zeroCommonMask);
}