#include <ECSpp/Utility/BitFilter.h>
#include <gtest/gtest.h>

using namespace epp;

TEST(BitFilter, addWanted_setWanted_getWanted)
{
    {
        BitFilter filter;
        Bitmask   wantedMask;
        filter.addWanted(2048);
        filter.addWanted(1);
        filter.addWanted(512);
        wantedMask.set({ 1, 512, 2048 });
        ASSERT_EQ(wantedMask, filter.getWanted());
    }
    {
        BitFilter filter;
        Bitmask   wantedMask;
        filter.addWanted({ 0, 512, 5000 });
        wantedMask.set({ 5000, 512, 0 });
        ASSERT_EQ(wantedMask, filter.getWanted());
    }
    {
        BitFilter filter(Bitmask{ 0, 1, 2 }, Bitmask{});
        Bitmask   wantedMask({ 0, 1, 2 });
        ASSERT_EQ(wantedMask, filter.getWanted());
    }
}

TEST(BitFilter, setWanted)
{
    {
        BitFilter filter;
        Bitmask   wantedMask({ 0, 1, 2 });
        ASSERT_EQ(Bitmask(), filter.getWanted());
        filter.setWanted(wantedMask);
        ASSERT_EQ(wantedMask, filter.getWanted());
    }
    {
        BitFilter filter;
        Bitmask   wantedMask({ 0, 1, 2 });
        filter.setWanted(Bitmask{ 1000, 200, 3000 });
        filter.setWanted(wantedMask);  // set after already set
        filter.addWanted({ 3, 4, 5 }); // add after set
        wantedMask.set({ 3, 4, 5 });
        ASSERT_EQ(wantedMask, filter.getWanted());
    }
}

TEST(BitFilter, addUnwanted_getUnwanted)
{
    {
        BitFilter filter;
        Bitmask   unwantedMask;
        filter.addUnwanted(2048);
        filter.addUnwanted(1);
        filter.addUnwanted(512);
        unwantedMask.set({ 1, 512, 2048 });
        ASSERT_EQ(unwantedMask, filter.getUnwanted());
    }
    {
        BitFilter filter;
        Bitmask   unwantedMask;
        filter.addUnwanted({ 0, 512, 5000 });
        unwantedMask.set({ 5000, 512, 0 });
        ASSERT_EQ(unwantedMask, filter.getUnwanted());
    }
    {
        BitFilter filter(Bitmask{}, Bitmask{ 0, 1, 2 });
        Bitmask   unwantedMask({ 0, 1, 2 });
        ASSERT_EQ(unwantedMask, filter.getUnwanted());
    }
}

TEST(BitFilter, setUnwanted)
{
    {
        BitFilter filter;
        Bitmask   unwantedMask({ 0, 1, 2 });
        ASSERT_EQ(Bitmask(), filter.getUnwanted());
        filter.setUnwanted(unwantedMask);
        ASSERT_EQ(unwantedMask, filter.getUnwanted());
    }
    {
        BitFilter filter;
        Bitmask   unwantedMask({ 0, 1, 2 });
        filter.setUnwanted(Bitmask{ 1000, 200, 3000 });
        filter.setUnwanted(unwantedMask);
        filter.addUnwanted({ 3, 4, 5 });
        unwantedMask.set({ 3, 4, 5 });
        ASSERT_EQ(unwantedMask, filter.getUnwanted());
    }
}

TEST(BitFilter, add_set_sideEffects)
{
    { // state of unwanted has no effect on wanted mask (when set after unwanted or when there are no common)
        BitFilter filter;
        Bitmask   wantedMask({ 0, 1, 2 });
        filter.addUnwanted({ 3, 4, 5 });
        filter.setWanted(wantedMask);
        filter.addUnwanted({ 128, 256, 1024 });
        ASSERT_EQ(wantedMask, filter.getWanted());
    }
    { // state of wanted mask has no effect on unwanted mask (when set after wanted or before and there are no common bits)
        BitFilter filter;
        Bitmask   unwantedMask({ 0, 1, 2 });
        filter.addWanted({ 3, 4, 5 });
        filter.setUnwanted(unwantedMask);
        filter.addWanted({ 128, 256, 1024 });
        ASSERT_EQ(unwantedMask, filter.getUnwanted());
    }
    // setting the same indices to wanted or unwanted masks results in removal of the common part in the other mask
    {
        BitFilter filter(Bitmask{ 1, 2, 3 }, Bitmask{ 3, 4, 5 }); // common: 3, in the constructor case, wanted mask has priority over unwanted
        ASSERT_EQ(filter.getWanted(), Bitmask({ 1, 2, 3 }));
        ASSERT_EQ(filter.getUnwanted(), (Bitmask{ 4, 5 }));

        filter.addWanted({ 4, 5, 6 });
        ASSERT_EQ(filter.getWanted(), Bitmask({ 1, 2, 3, 4, 5, 6 }));
        ASSERT_EQ(filter.getUnwanted(), Bitmask());

        filter.addUnwanted({ 2, 4, 6, 1024 });
        ASSERT_EQ(filter.getWanted(), Bitmask({ 1, 3, 5 }));
        ASSERT_EQ(filter.getUnwanted(), Bitmask({ 2, 4, 6, 1024 }));

        filter.setWanted(Bitmask{ 2, 4 });
        ASSERT_EQ(filter.getWanted(), Bitmask({ 2, 4 }));
        ASSERT_EQ(filter.getUnwanted(), Bitmask({ 6, 1024 }));

        filter.setUnwanted(Bitmask{ 4 });
        ASSERT_EQ(filter.getWanted(), Bitmask({ 2 }));
        ASSERT_EQ(filter.getUnwanted(), Bitmask({ 4 }));
    }
}

TEST(BitFilter, removeWanted)
{
    {
        BitFilter filter;
        filter.removeWanted(64);

        filter.addWanted(128);
        filter.removeWanted(128);
        ASSERT_EQ(filter.getWanted(), Bitmask());

        filter.addWanted({ 1, 2, 1024, 3 });
        filter.removeWanted(2);
        ASSERT_EQ(filter.getWanted(), Bitmask({ 1, 1024, 3 }));
    }
    {
        BitFilter filter;
        filter.removeWanted({ 1, 128, 2048 }); // on uninitialized

        filter.addWanted({ 2048, 1, 512 });
        filter.removeWanted({ 0, 2, 4, 5, 1 }); // remove invalid bits
        ASSERT_EQ(Bitmask({ 2048, 512 }), filter.getWanted());

        filter.addWanted({ 64, 65, 67, 68 });
        filter.removeWanted({ 65, 67, 2048 });
        ASSERT_EQ(Bitmask({ 64, 68, 512 }), filter.getWanted());

        filter.removeWanted({ 64, 68, 512 });
        ASSERT_EQ(Bitmask(), filter.getWanted()); // to empty
    }
    { // removal of bits from one masks has no effect on the other mask
        BitFilter filter(Bitmask{ 0, 1, 2 }, Bitmask{ 64, 65, 66 });
        filter.removeWanted({ 0, 2 });
        ASSERT_EQ(filter.getUnwanted(), Bitmask({ 64, 65, 66 }));

        filter.removeWanted(1);
        ASSERT_EQ(filter.getUnwanted(), Bitmask({ 64, 65, 66 }));
    }
}

TEST(BitFilter, removeUnwanted)
{
    {
        BitFilter filter;
        filter.removeUnwanted(64);

        filter.addUnwanted(128);
        filter.removeUnwanted(128);
        ASSERT_EQ(filter.getUnwanted(), Bitmask());

        filter.addUnwanted({ 1, 2, 1024, 3 });
        filter.removeUnwanted(2);
        ASSERT_EQ(filter.getUnwanted(), Bitmask({ 1, 1024, 3 }));
    }
    {
        BitFilter filter;
        filter.removeUnwanted({ 1, 128, 2048 }); // on uninitialized

        filter.addUnwanted({ 2048, 1, 512 });
        filter.removeUnwanted({ 0, 2, 4, 5, 1 });
        ASSERT_EQ(Bitmask({ 2048, 512 }), filter.getUnwanted());

        filter.addUnwanted({ 64, 65, 67, 68 });
        filter.removeUnwanted({ 65, 67, 2048 });
        ASSERT_EQ(Bitmask({ 64, 68, 512 }), filter.getUnwanted());

        filter.removeUnwanted({ 64, 68, 512 });
        ASSERT_EQ(Bitmask(), filter.getUnwanted()); // to empty
    }
    { // removal of bits from one masks has no effect on the other mask
        BitFilter filter(Bitmask{ 0, 1, 2 }, Bitmask{ 64, 65, 66 });
        filter.removeUnwanted({ 64, 66 });
        ASSERT_EQ(filter.getWanted(), Bitmask({ 0, 1, 2 }));

        filter.removeUnwanted(65);
        ASSERT_EQ(filter.getWanted(), Bitmask({ 0, 1, 2 }));
    }
}

TEST(BitFilter, Clear)
{
    BitFilter filter(Bitmask{ 0, 1, 2 }, Bitmask{ 64, 65, 66 });
    filter.clear();
    ASSERT_EQ(filter.getWanted(), Bitmask());
    ASSERT_EQ(filter.getUnwanted(), Bitmask());
}

TEST(BitFilter, Operators_EQ_NE)
{
    BitFilter filter1(Bitmask{ 0, 1, 2 }, Bitmask{ 3, 4, 5 });
    BitFilter filter2 = filter1; // copy constructor

    ASSERT_EQ(filter1, filter2);     // ==
    ASSERT_NE(filter1, BitFilter()); // !=

    filter2.removeWanted(1);
    ASSERT_NE(filter1, filter2);
    filter2.setWanted(Bitmask());
    ASSERT_NE(filter1, filter2);

    filter2 = filter1; // copy assign
    ASSERT_EQ(filter1, filter2);

    filter2.removeUnwanted(4);
    ASSERT_NE(filter1, filter2);
    filter2.setUnwanted(Bitmask());
    ASSERT_NE(filter1, filter2);

    filter1.clear();
    filter2.clear();
    ASSERT_EQ(filter1, filter2);
    ASSERT_EQ(filter1, BitFilter());
}

TEST(BitFilter, Operators_AND)
{
    BitFilter filter(Bitmask{ 0, 1, 2 }, Bitmask{ 3, 4, 5 });
    Bitmask   mask{ 0, 1, 2 };

    ASSERT_TRUE(filter & mask);

    mask.set({ 6, 64, 128 });
    ASSERT_TRUE(filter & mask);

    mask.unset(2);
    ASSERT_FALSE(filter & mask);

    mask.set({ 2, 4 });
    ASSERT_FALSE(filter & mask);
    filter.addWanted(4);
    ASSERT_TRUE(filter & mask);
}

TEST(BitFilter, Hash)
{
    std::hash<BitFilter> hasher;

    auto h1 = hasher(BitFilter());
    auto h2 = hasher(BitFilter(Bitmask{ 0 }, Bitmask{}));
    auto h3 = hasher(BitFilter(Bitmask{ 0, 1 }, Bitmask{}));
    auto h4 = hasher(BitFilter(Bitmask{ 0, 1 }, Bitmask{ 3 }));
    ASSERT_NE(h1, h2);
    ASSERT_NE(h1, h3);
    ASSERT_NE(h1, h4);
    ASSERT_NE(h2, h3);
    ASSERT_NE(h2, h4);
    ASSERT_NE(h3, h4);
}