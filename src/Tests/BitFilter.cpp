// #include "Components.h"

// #include <ECSpp/Utility/BitFilter.h>
// #include <gtest/gtest.h>

// TEST(BitFilterTest, Wanted_Unwanted)
// {
//     BitFilter cFilter;
//     Bitmask   wantedMask;
//     Bitmask   unwantedMask;

//     cFilter.addWanted({ ComponentUtility::ID<TComp1>, ComponentUtility::ID<TComp2>, ComponentUtility::ID<TComp3> }).addUnwanted({ ComponentUtility::ID<TComp4> });
//     wantedMask.set({ ComponentUtility::ID<TComp1>, ComponentUtility::ID<TComp2>, ComponentUtility::ID<TComp3> });
//     unwantedMask.set({ ComponentUtility::ID<TComp4> });
//     ASSERT_EQ(cFilter.getWantedMask(), wantedMask);
//     ASSERT_EQ(cFilter.getUnwantedMask(), unwantedMask);

//     cFilter.removeWanted<TComp1>().addUnwanted<TComp3>();
//     wantedMask.unset({ getCTypeId<TComp1>(), getCTypeId<TComp3>() });
//     unwantedMask.set(getCTypeId<TComp3>());
//     ASSERT_EQ(cFilter.getWantedMask(), wantedMask);
//     ASSERT_EQ(cFilter.getUnwantedMask(), unwantedMask);

//     cFilter.addUnwanted<TComp2>();
//     wantedMask.clear();

//     ASSERT_EQ(cFilter.getWantedMask(), wantedMask);
//     ASSERT_NE(cFilter.getUnwantedMask(), unwantedMask);

//     unwantedMask.set(getCTypeId<TComp2>());
//     ASSERT_EQ(cFilter.getUnwantedMask(), unwantedMask);
// }

// TEST(BitFilterTest, Set_Wanted_Unwanted)
// {
//     BitFilter filter;
//     filter.setWanted({ 1, 2, 5, 10 });
//     filter.setUnwanted({ 1, 10, 15 }); // common 1 & 10

//     ASSERT_FALSE(filter.getWantedMask().hasCommon(filter.getUnwantedMask()));
//     ASSERT_TRUE(filter.getUnwantedMask().get(1) && filter.getUnwantedMask().get(10));

//     filter.setWanted({ 1, 15 }); // common 1 & 15
//     ASSERT_FALSE(filter.getWantedMask().hasCommon(filter.getUnwantedMask()));
//     ASSERT_TRUE(filter.getWantedMask().get(1) && filter.getWantedMask().get(15));

//     BitFilter constructCopy(filter.getWantedMask(), filter.getUnwantedMask());
//     ASSERT_EQ(constructCopy, filter);
// }


// TEST(BitFilterTest, Clear_Hashing)
// {
//     BitFilter cFilter1;
//     BitFilter cFilter2;

//     cFilter1.addWanted<TComp1, TComp2>().addUnwanted<TComp3>();
//     cFilter2.addWanted<TComp1>().addUnwanted<TComp3>();
//     std::hash<BitFilter> hasher;
//     ASSERT_NE(hasher(cFilter1), hasher(cFilter2));

//     cFilter1.removeWanted<TComp2>();
//     ASSERT_EQ(hasher(cFilter1), hasher(cFilter2));

//     cFilter1.removeUnwanted<TComp3>();
//     ASSERT_NE(hasher(cFilter1), hasher(cFilter2));

//     cFilter1.addUnwanted<TComp2>();
//     ASSERT_NE(hasher(cFilter1), hasher(cFilter2));

//     cFilter1.addWanted<TComp2>();
//     cFilter2.addWanted<TComp2>();
//     cFilter2.removeUnwanted<TComp3>();
//     ASSERT_EQ(hasher(cFilter1), hasher(cFilter2));

//     cFilter1.clear();
//     ASSERT_NE(hasher(cFilter1), hasher(cFilter2));
//     ASSERT_EQ(cFilter1, BitFilter());
// }