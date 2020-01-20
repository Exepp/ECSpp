#include "ComponentsT.h"
#include <ECSpp/Component.h>
#include <gtest/gtest.h>

using namespace epp;

struct ExcTestComp {};

// THIS TEST REGISTERS COMPONENTS, SO IT HAS TO BE INVOKED BEFORE ANY OTHER TEST THAT USES COMPONENTS,
// OTHERWISE PROGRAM WILL TERMINATE ON THIS TEST
TEST(Component, Register_Id)
{
    CMetadata::Register<TComp1, TComp2, TComp3, TComp4>();
    ASSERT_THROW((CMetadata::Register<TComp1, TComp2, TComp3, TComp4>()), AssertFailed);
    ASSERT_THROW(auto x = IdOf<ExcTestComp>();, AssertFailed); // usage of unregistered component

    // reverse order
    ASSERT_EQ(IdOf<TComp4>().value, 3);
    ASSERT_EQ(IdOf<TComp3>().value, 2);
    ASSERT_EQ(IdOf<TComp2>().value, 1);
    ASSERT_EQ(IdOf<TComp1>().value, 0);
}

TEST(Component, IdOf)
{
    auto a1 = IdOf<TComp1, TComp2, TComp3, TComp4>();
    auto a2 = { IdOf<TComp1>(), IdOf<TComp2>(), IdOf<TComp3>(), IdOf<TComp4>() };
    for (int i = 0; i < a1.size(); ++i)
        ASSERT_EQ(*(a1.begin() + i), *(a2.begin() + i));
}

TEST(Component, Metadata)
{
    ASSERT_EQ(CMetadata::GetData(IdOf<TComp1>()).size, sizeof(TComp1));
    ASSERT_EQ(CMetadata::GetData(IdOf<TComp4>()).size, sizeof(TComp4));
    ASSERT_EQ(CMetadata::GetData(IdOf<TComp1>()).alignment, alignof(TComp1));
    ASSERT_EQ(CMetadata::GetData(IdOf<TComp4>()).alignment, alignof(TComp4));

    {
        auto obj = std::make_unique<TComp1>();
        obj->~TComp1();
        ASSERT_EQ(TComp1::AliveCounter, 0);

        CMetadata::GetData(IdOf<TComp1>()).defaultConstructor(obj.get());
        ASSERT_EQ(TComp1::AliveCounter, 1);

        CMetadata::GetData(IdOf<TComp1>()).destructor(obj.get());
        ASSERT_EQ(TComp1::AliveCounter, 0);

        {
            TComp1 temp;
            CMetadata::GetData(IdOf<TComp1>()).moveConstructor(obj.get(), &temp);
        }
        ASSERT_EQ(TComp1::AliveCounter, 1);

        CMetadata::GetData(IdOf<TComp1>()).destructor(obj.get());
        ASSERT_EQ(TComp1::AliveCounter, 0);

        // for unique ptr to cleanup
        CMetadata::GetData(IdOf<TComp1>()).defaultConstructor(obj.get());
    }
    {
        auto obj = std::make_unique<TComp4>();
        obj->~TComp4();
        ASSERT_EQ(TComp4::AliveCounter, 0);

        CMetadata::GetData(IdOf<TComp4>()).defaultConstructor(obj.get());
        ASSERT_EQ(TComp4::AliveCounter, 1);

        CMetadata::GetData(IdOf<TComp4>()).destructor(obj.get());
        ASSERT_EQ(TComp4::AliveCounter, 0);

        {
            TComp4 temp;
            CMetadata::GetData(IdOf<TComp4>()).moveConstructor(obj.get(), &temp);
        }
        ASSERT_EQ(TComp4::AliveCounter, 1);

        CMetadata::GetData(IdOf<TComp4>()).destructor(obj.get());
        ASSERT_EQ(TComp4::AliveCounter, 0);

        // for unique ptr to cleanup
        CMetadata::GetData(IdOf<TComp4>()).defaultConstructor(obj.get());
    }
}