#include "ComponentsT.h"
#include <ECSpp/Internal/CPool.h>
#include <gtest/gtest.h>

using namespace epp;


TEST(CPool, GetCId)
{
    CPool pool1(IdOf<TComp1>());
    ASSERT_EQ(pool1.getCId(), IdOf<TComp1>());

    // the only way to change the CId of a pool
    pool1 = CPool(IdOf<TComp2>());
    ASSERT_NE(pool1.getCId(), IdOf<TComp1>());
    ASSERT_EQ(pool1.getCId(), IdOf<TComp2>());

    CPool pool2 = std::move(pool1);
    ASSERT_EQ(pool1.getCId(), IdOf<TComp2>()); // still same
    ASSERT_EQ(pool2.getCId(), IdOf<TComp2>());
}

TEST(CPool, Size_Reserved)
{
    CPool pool(IdOf<TComp2>());
    for (std::size_t i = 0; i < 1e4; ++i) {
        pool.create();
        ASSERT_LE(pool.size(), pool.reserved());
    }
    ASSERT_EQ(pool.size(), 1e4);
    std::size_t reserved = pool.reserved();

    for (std::size_t i = 0; i < 1e4; ++i) {
        pool.destroy(0);
        ASSERT_LE(pool.size(), pool.reserved());
    }
    ASSERT_EQ(pool.size(), 0);
    ASSERT_EQ(pool.reserved(), reserved);

    for (std::size_t i = 0; i < 1e4; ++i) {
        TComp2 tmp;
        pool.create(&tmp);
    }
    ASSERT_EQ(pool.size(), 1e4);
    ASSERT_EQ(pool.reserved(), reserved);

    pool.clear();
    ASSERT_EQ(pool.size(), 0);
    ASSERT_EQ(pool.reserved(), reserved);

    for (std::size_t i = 0; i < 10; ++i)
        pool.alloc(); // no reallocation will happen (10 < 1e4)
    ASSERT_EQ(pool.size(), 10);
    ASSERT_EQ(pool.reserved(), reserved);
    pool.fitNextN(123); // size == 10 & reserved > 10 + 123 -> does nothing
    ASSERT_EQ(pool.size(), 10);
    ASSERT_EQ(pool.reserved(), reserved);
    for (std::size_t i = 0; i < 10; ++i)
        pool.construct(i);
    ASSERT_EQ(pool.size(), 10);

    pool.fitNextN(reserved + 123);
    ASSERT_EQ(pool.size(), 10);
    ASSERT_NE(pool.reserved(), reserved);
    ASSERT_GE(pool.reserved(), 10 + reserved + 123);

    CPool pool2(std::move(pool));
    ASSERT_EQ(pool2.size(), 10);
    ASSERT_GE(pool2.reserved(), 10 + reserved + 123);
    ASSERT_EQ(pool.size(), 0);
    ASSERT_EQ(pool.reserved(), 0);

    pool = std::move(pool2);
    ASSERT_EQ(pool.size(), 10);
    ASSERT_GE(pool.reserved(), 10 + reserved + 123);
    ASSERT_EQ(pool2.size(), 0);
    ASSERT_EQ(pool2.reserved(), 0);
}

TEST(CPool, Alloc_GetOperator_Construct)
{
    {
        CPool pool(IdOf<TComp1>());
        pool.fitNextN(1e5); // for no reallocations (crucial for no memory leaks for this test)

        void* ptr1 = pool.alloc();
        void* ptr2 = pool.alloc();
        ASSERT_TRUE((std::uintptr_t(ptr1) & (alignof(TComp1) - 1)) == 0);       // aligned properly
        ASSERT_EQ(std::uintptr_t(ptr2) - std::uintptr_t(ptr1), sizeof(TComp1)); // one is placed next to the other

        // operator[] tests
        ASSERT_EQ(pool[0], ptr1);
        ASSERT_EQ(pool[1], ptr2);

        for (int i = 0; i < 1e4; ++i)
            ASSERT_EQ(pool.alloc(), pool[pool.size() - 1]); // more operator[] tests
        ASSERT_EQ(TComp1::AliveCounter, 0);                 // alloc should not construct anything

        ASSERT_TRUE((std::uintptr_t(pool[0]) & (alignof(TComp1) - 1)) == 0); // still aligned
        for (std::size_t i = 0; i < pool.size() - 1; ++i)                    // check if all are placed contiguously
            ASSERT_EQ(std::uintptr_t(pool[i + 1]) - std::uintptr_t(pool[i]), sizeof(TComp1));

        // pool expects every allocated object to be constructed before it itself is destroyed
        for (std::size_t i = 0; i < pool.size(); ++i)
            pool.construct(i);
        ASSERT_EQ(TComp1::AliveCounter, pool.size()); // called default constructor properly
    }
    ASSERT_EQ(TComp1::AliveCounter, 0); // destructor test when using alloc only
}

TEST(CPool, Create)
{
    {
        CPool pool(IdOf<TComp1>());

        void* ptr1 = pool.create();
        ASSERT_TRUE((std::uintptr_t(ptr1) & (alignof(TComp1) - 1)) == 0); // aligned properly

        void* ptr2 = pool.create();
        ASSERT_TRUE((std::uintptr_t(ptr2) & (alignof(TComp1) - 1)) == 0);       // second one is also aligned
        ASSERT_EQ(std::uintptr_t(ptr2) - std::uintptr_t(ptr1), sizeof(TComp1)); // and one is placed next to the other

        for (int i = 0; i < 1e4; ++i)
            ASSERT_EQ(pool.create(), pool[pool.size() - 1]);

        ASSERT_TRUE((std::uintptr_t(pool[0]) & (alignof(TComp1) - 1)) == 0); // still aligned
        for (int i = 0; i < pool.size() - 1; ++i)                            // check if all are placed next to the other
            ASSERT_EQ(std::uintptr_t(pool[i + 1]) - std::uintptr_t(pool[i]), sizeof(TComp1));

        TComp1 tester;
        for (std::size_t i = 0; i < pool.size(); ++i) // check if all were constructed correctly
            ASSERT_EQ(*static_cast<TComp1*>(pool[i]), tester);
        ASSERT_EQ(TComp1::AliveCounter, pool.size() + 1); // + 1 - tester
    }
    ASSERT_EQ(TComp1::AliveCounter, 0); // destructor test when using create only
}

TEST(CPool, Create_Move)
{
    {
        CPool pool(IdOf<TComp1>());
        TComp1 tester(2222, 22.22f, 222.222);
        for (int n = 0; n < 1e4; ++n) {
            TComp1 temp = tester.copy(n);
            ASSERT_EQ(pool.create(&temp), pool[pool.size() - 1]);
            // was the move constructor invoked correctly
            ASSERT_EQ(temp, TComp1(0, 0, 0));
        }

        ASSERT_TRUE((std::uintptr_t(pool[0]) & (alignof(TComp1) - 1)) == 0); // check alignment
        for (int i = 0; i < pool.size() - 1; ++i)                            // check if all are placed next to the other
            ASSERT_EQ(std::uintptr_t(pool[i + 1]) - std::uintptr_t(pool[i]), sizeof(TComp1));

        for (int i = 0; i < pool.size(); ++i) // check if all were constructed correctly
            ASSERT_EQ(*static_cast<TComp1*>(pool[i]), tester.copy(i));
        ASSERT_EQ(TComp1::AliveCounter, pool.size() + 1); // + 1 - tester
    }
    ASSERT_EQ(TComp1::AliveCounter, 0); // destructor test when using create only, move version
}

TEST(CPool, Clear)
{
    CPool pool(IdOf<TComp1>());
    for (int i = 0; i < 1e4; ++i)
        pool.create();
    pool.clear();
    ASSERT_EQ(TComp1::AliveCounter, 0);
    ASSERT_EQ(pool.create(), pool[0]); // check if objects are added at the beginning
    // tests of how CPool::clear influences the CPool::size and CPool:;reserved values are already in size_reserved test
}

TEST(CPool, Destroy)
{
    CPool pool(IdOf<TComp1>());
    // pool.destroy(0); - program will terminate
    pool.create();
    pool.create();
    TComp1 tester(12345, 123.45f, 1.2345);
    {
        TComp1 temp = tester.copy();
        pool.create(&temp);
    }
    ASSERT_NE(*static_cast<TComp1*>(pool[0]), tester);
    ASSERT_TRUE(pool.destroy(0)); // there was a swap: first <- last
    ASSERT_EQ(*static_cast<TComp1*>(pool[0]), tester);
    ASSERT_EQ(TComp1::AliveCounter, pool.size() + 1); // + 1 to account for the local variable (tester)

    pool.clear();

    // bigger scale test:
    for (int i = 0; i < 1e4; ++i) {
        TComp1 temp = tester.copy(i);
        pool.create(&temp);
    }
    for (int i = pool.size() - 1, limit = pool.size() / 2; i >= limit; --i) {
        ASSERT_TRUE(pool.destroy(0)); // there was a swap: first <- last
        ASSERT_EQ(*static_cast<TComp1*>(pool[0]), tester.copy(i));
    }
    // test if there are any side effects
    for (int i = 1; i < pool.size(); ++i) // skipping the first one
        ASSERT_EQ(*static_cast<TComp1*>(pool[i]), tester.copy(i));

    // check if destroying the last element has any side effects
    for (int i = pool.size() / 2; i > 0; --i)
        pool.destroy(pool.size() - 1);
    for (int i = 1; i < pool.size(); ++i) // skipping the first one
        ASSERT_EQ(*static_cast<TComp1*>(pool[i]), tester.copy(i));
}

TEST(CPool, FitNextN)
{
    CPool pool(IdOf<TComp1>());
    TComp1 tester(2222, 22.22f, 2.222);
    TComp1 tmp = tester.copy();
    void* ptr = pool.create(&tmp);
    pool.fitNextN(512);
    ASSERT_NE(ptr, pool[0]); // data was reallocated
    ASSERT_EQ(tester, *static_cast<TComp1*>(pool[0]));

    pool.clear();
    std::vector<void*> ptrs(512);
    for (int i = 0; i < 512; ++i) {
        TComp1 tmp2 = tester.copy(i);
        ptrs[i] = pool.create(&tmp2);
    }
    for (int i = 0; i < 512; ++i)
        ASSERT_EQ(ptrs[i], pool[i]);

    pool.fitNextN(5096);
    for (int i = 0; i < 512; ++i) {
        ASSERT_NE(ptrs[i], pool[i]);
        ASSERT_EQ(*static_cast<TComp1*>(pool[i]), tester.copy(i));
    }
}

TEST(CPool, MoveConstructorAssignment)
{
    CPool pool(IdOf<TComp1>());

    for (int i = 0; i < 1e3; ++i)
        pool.create();

    CPool pool2(std::move(pool));
    ASSERT_EQ(TComp1::AliveCounter, pool2.size());

    pool = std::move(pool2);
    ASSERT_EQ(TComp1::AliveCounter, pool.size());
}