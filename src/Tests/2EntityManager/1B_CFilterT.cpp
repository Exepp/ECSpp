#include "ComponentsT.h"
#include <ECSpp/Internal/CFilter.h>
#include <gtest/gtest.h>

using namespace epp;

void testFilter(CFilter const& filter, CMask wanted, CMask unwanted)
{
    ASSERT_EQ(filter.getWanted(), wanted);
    ASSERT_EQ(filter.getUnwanted(), unwanted);
    ASSERT_EQ(filter, CFilter(wanted, unwanted));
    ASSERT_TRUE(filter & wanted);

    if (filter.getWanted().getSetCount() || filter.getUnwanted().getSetCount())
        ASSERT_FALSE(filter & unwanted);
    else
        ASSERT_TRUE(filter & unwanted); // true for empty unwanted masks

    if (wanted != unwanted) {
        ASSERT_NE(filter, CFilter(unwanted, wanted));
        ASSERT_NE(filter, CFilter({}, wanted));
        ASSERT_NE(filter, CFilter(unwanted, {}));
        ASSERT_NE(filter, CFilter({}, {}));

        if (wanted.getSetCount() && unwanted.getSetCount()) {
            wanted.getBitset() |= unwanted.getBitset();
            unwanted = wanted;
            ASSERT_NE(filter.getWanted(), wanted);
            ASSERT_NE(filter.getUnwanted(), unwanted);
            ASSERT_NE(filter, CFilter(wanted, unwanted));
            ASSERT_FALSE(filter & wanted);
        }
    }
}

TEST(CFilter, Constructor)
{
    {
        CFilter filter;
        testFilter(filter, {}, {});
    }
    {
        auto wanted = CMask(IdOf<TComp1, TComp2>());
        auto unwanted = CMask();
        CFilter filter(wanted, unwanted);
        testFilter(filter, wanted, unwanted);
    }
    {
        auto wanted = CMask();
        auto unwanted = CMask(IdOf<TComp1, TComp2>());
        CFilter filter(wanted, unwanted);
        testFilter(filter, wanted, unwanted);
    }
    {
        auto wanted = CMask(IdOf<TComp1, TComp2>());
        auto unwanted = CMask(IdOf<TComp3, TComp4>());
        CFilter filter(wanted, unwanted);
        testFilter(filter, wanted, unwanted);
    }
    { // overlapping
        auto wanted = CMask(IdOf<TComp1, TComp2>());
        auto unwanted = CMask(IdOf<TComp2, TComp3, TComp4>());
        CFilter filter(wanted, unwanted);
        testFilter(filter, wanted, unwanted.removeCommon(wanted));
    }
}

TEST(CFilter, SetWanted)
{
    {
        auto wanted = CMask();
        auto unwanted = CMask(IdOf<TComp1, TComp2>());
        CFilter filter(wanted, unwanted);
        wanted.set(IdOf<TComp3>());
        filter.setWanted(wanted);
        filter.setWanted(wanted); // twice
        testFilter(filter, wanted, unwanted);
    }
    {
        auto wanted = CMask(IdOf<TComp1, TComp2>());
        auto unwanted = CMask(IdOf<TComp3, TComp4>());
        CFilter filter(wanted, unwanted);
        wanted.set(IdOf<TComp3>());
        filter.setWanted(wanted);
        testFilter(filter, wanted, unwanted.removeCommon(wanted));
    }
}

TEST(CFilter, AddWanted)
{
    {
        auto wanted = CMask();
        auto unwanted = CMask(IdOf<TComp1, TComp2>());
        CFilter filter(wanted, unwanted);
        wanted.set(IdOf<TComp3>());
        filter.addWanted(IdOf<TComp3>());
        filter.addWanted(IdOf<TComp3>()); // twice
        testFilter(filter, wanted, unwanted);
    }
    {
        auto wanted = CMask(IdOf<TComp1, TComp2>());
        auto unwanted = CMask(IdOf<TComp3, TComp4>());
        CFilter filter(wanted, unwanted);
        wanted.set(IdOf<TComp3>());
        filter.addWanted(IdOf<TComp3>());
        testFilter(filter, wanted, unwanted.removeCommon(wanted));
    }
}

TEST(CFilter, AddWanted_List)
{
    {
        auto wanted = CMask();
        auto unwanted = CMask(IdOf<TComp1, TComp2>());
        CFilter filter(wanted, unwanted);
        wanted.set(IdOf<TComp3, TComp4>());
        filter.addWanted(IdOf<TComp3, TComp3, TComp4>());
        filter.addWanted(IdOf<TComp3, TComp3, TComp4>()); // twice
        testFilter(filter, wanted, unwanted);
    }
    {
        auto wanted = CMask(IdOf<TComp1, TComp2>());
        auto unwanted = CMask(IdOf<TComp3, TComp4>());
        CFilter filter(wanted, unwanted);
        wanted.set(IdOf<TComp3, TComp4>());
        filter.addWanted(IdOf<TComp3, TComp3, TComp4>()); // overlapping
        testFilter(filter, wanted, unwanted.removeCommon(wanted));
    }
}

TEST(CFilter, RemoveWanted)
{
    {
        auto wanted = CMask(IdOf<TComp1, TComp2>());
        auto unwanted = CMask(IdOf<TComp3, TComp4>());
        CFilter filter(wanted, unwanted);
        wanted.unset(IdOf<TComp1>());
        filter.removeWanted(IdOf<TComp1>());
        filter.removeWanted(IdOf<TComp1>()); // twice
        filter.removeWanted(IdOf<TComp3>()); // from unwanted (does nothing)
        testFilter(filter, wanted, unwanted);
    }
    {
        auto wanted = CMask(IdOf<TComp1, TComp2>());
        auto unwanted = CMask(IdOf<TComp3, TComp4>());
        CFilter filter(wanted, unwanted);
        wanted.unset(IdOf<TComp1>());
        wanted.unset(IdOf<TComp2>());
        filter.removeWanted(IdOf<TComp1>());
        filter.removeWanted(IdOf<TComp2>()); // remove every wanted
        testFilter(filter, wanted, unwanted);
    }
}

TEST(CFilter, RemoveWanted_List)
{
    {
        auto wanted = CMask(IdOf<TComp1, TComp2>());
        auto unwanted = CMask(IdOf<TComp3, TComp4>());
        CFilter filter(wanted, unwanted);
        wanted.unset(IdOf<TComp1>());
        filter.removeWanted(IdOf<TComp1, TComp1, TComp3>()); // twice the same + one from unwanted
        filter.removeWanted(IdOf<TComp1, TComp1, TComp3>()); // twice
        testFilter(filter, wanted, unwanted);
    }
    {
        auto wanted = CMask(IdOf<TComp1, TComp2>());
        auto unwanted = CMask(IdOf<TComp3, TComp4>());
        CFilter filter(wanted, unwanted);
        wanted.unset(IdOf<TComp1, TComp2>());
        filter.removeWanted(IdOf<TComp1, TComp2>()); // remove every wanted
        testFilter(filter, wanted, unwanted);
    }
}

TEST(CFilter, SetUnwanted)
{
    {
        auto wanted = CMask(IdOf<TComp1, TComp2>());
        auto unwanted = CMask();
        CFilter filter(wanted, unwanted);
        unwanted.set(IdOf<TComp3>());
        filter.setUnwanted(unwanted);
        filter.setUnwanted(unwanted); // twice
        testFilter(filter, wanted, unwanted);
    }
    {
        auto wanted = CMask(IdOf<TComp1, TComp2>());
        auto unwanted = CMask(IdOf<TComp3, TComp4>());
        CFilter filter(wanted, unwanted);
        unwanted.set(IdOf<TComp3>()); // overlapping
        filter.setUnwanted(unwanted);
        testFilter(filter, wanted.removeCommon(unwanted), unwanted);
    }
}

TEST(CFilter, AddUnwanted)
{
    {
        auto wanted = CMask(IdOf<TComp1, TComp2>());
        auto unwanted = CMask();
        CFilter filter(wanted, unwanted);
        unwanted.set(IdOf<TComp3>());
        filter.addUnwanted(IdOf<TComp3>());
        filter.addUnwanted(IdOf<TComp3>()); // twice
        testFilter(filter, wanted, unwanted);
    }
    {
        auto wanted = CMask(IdOf<TComp1, TComp2>());
        auto unwanted = CMask(IdOf<TComp3, TComp4>());
        CFilter filter(wanted, unwanted);
        unwanted.set(IdOf<TComp3>());
        filter.addUnwanted(IdOf<TComp3>()); // overlapping
        testFilter(filter, wanted.removeCommon(unwanted), unwanted);
    }
}

TEST(CFilter, AddUnwanted_List)
{
    {
        auto wanted = CMask(IdOf<TComp1, TComp2>());
        auto unwanted = CMask();
        CFilter filter(wanted, unwanted);
        unwanted.set(IdOf<TComp3, TComp4>());
        filter.addUnwanted(IdOf<TComp3, TComp3, TComp4>());
        filter.addUnwanted(IdOf<TComp3, TComp3, TComp4>()); // twice
        testFilter(filter, wanted, unwanted);
    }
    {
        auto wanted = CMask(IdOf<TComp1, TComp2>());
        auto unwanted = CMask(IdOf<TComp3, TComp4>());
        CFilter filter(wanted, unwanted);
        unwanted.set(IdOf<TComp3, TComp4>());
        filter.addUnwanted(IdOf<TComp3, TComp3, TComp4>()); // overlapping
        testFilter(filter, wanted.removeCommon(unwanted), unwanted);
    }
}

TEST(CFilter, RemoveUnwanted)
{
    {
        auto wanted = CMask(IdOf<TComp3, TComp4>());
        auto unwanted = CMask(IdOf<TComp1, TComp2>());
        CFilter filter(wanted, unwanted);
        unwanted.unset(IdOf<TComp1>());
        filter.removeUnwanted(IdOf<TComp1>());
        filter.removeUnwanted(IdOf<TComp1>()); // twice
        filter.removeUnwanted(IdOf<TComp3>()); // from wanted (does nothing)
        testFilter(filter, wanted, unwanted);
    }
    {
        auto wanted = CMask(IdOf<TComp3, TComp4>());
        auto unwanted = CMask(IdOf<TComp1, TComp2>());
        CFilter filter(wanted, unwanted);
        unwanted.unset(IdOf<TComp1>());
        unwanted.unset(IdOf<TComp2>());
        filter.removeUnwanted(IdOf<TComp1>());
        filter.removeUnwanted(IdOf<TComp2>()); // remove every unwanted
        testFilter(filter, wanted, unwanted);
    }
}

TEST(CFilter, RemoveUnwanted_List)
{
    {
        auto wanted = CMask(IdOf<TComp3, TComp4>());
        auto unwanted = CMask(IdOf<TComp1, TComp2>());
        CFilter filter(wanted, unwanted);
        unwanted.unset(IdOf<TComp1>());
        filter.removeUnwanted(IdOf<TComp1, TComp1, TComp3>()); // twice the same + one from wanted
        filter.removeUnwanted(IdOf<TComp1, TComp1, TComp3>()); // twice
        testFilter(filter, wanted, unwanted);
    }
    {
        auto wanted = CMask(IdOf<TComp3, TComp4>());
        auto unwanted = CMask(IdOf<TComp1, TComp2>());
        CFilter filter(wanted, unwanted);
        unwanted.unset(IdOf<TComp1, TComp2>());
        filter.removeUnwanted(IdOf<TComp1, TComp2>()); // remove every unwanted
        testFilter(filter, wanted, unwanted);
    }
}

TEST(CFilter, Clear)
{
    {
        CFilter filter;
        filter.clear();
        testFilter(filter, {}, {});
    }
    {
        auto wanted = CMask(IdOf<TComp1, TComp2>());
        auto unwanted = CMask();
        CFilter filter(wanted, unwanted);
        filter.clear();
        wanted.clear();
        unwanted.clear();
        testFilter(filter, wanted, unwanted);
    }
    {
        auto wanted = CMask();
        auto unwanted = CMask(IdOf<TComp1, TComp2>());
        CFilter filter(wanted, unwanted);
        filter.clear();
        wanted.clear();
        unwanted.clear();
        testFilter(filter, wanted, unwanted);
    }
    {
        auto wanted = CMask(IdOf<TComp1, TComp2>());
        auto unwanted = CMask(IdOf<TComp3, TComp4>());
        CFilter filter(wanted, unwanted);
        filter.clear();
        wanted.clear();
        unwanted.clear();
        testFilter(filter, wanted, unwanted);
    }
    { // overlapping
        auto wanted = CMask(IdOf<TComp1, TComp2>());
        auto unwanted = CMask(IdOf<TComp2, TComp3, TComp4>());
        CFilter filter(wanted, unwanted);
        filter.clear();
        wanted.clear();
        unwanted.clear();
        testFilter(filter, wanted, unwanted);
    }
}