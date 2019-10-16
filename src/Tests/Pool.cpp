#include <ECSpp/Utility/Pool.h>
#include <gtest/gtest.h>

using namespace epp;

TEST(Pool, Alloc)
{
    {
        Pool<int> pool;
        int       x  = 2;
        int       a1 = pool.alloc();
        int       a2 = pool.alloc(-1);
        int       a3 = pool.alloc(x);

        EXPECT_EQ(a1, 0);
        EXPECT_EQ(a2, -1);
        EXPECT_EQ(a3, x);
        EXPECT_EQ(a1, pool.content[0]);
        EXPECT_EQ(a2, pool.content[1]);
        EXPECT_EQ(a3, pool.content[2]);

        // force expansion
        for (int i = 0; i < 1024; ++i)
            pool.alloc(i);
        for (int i = 0; i < 1024; ++i)
            EXPECT_EQ(i, pool.content[3 + i]);
        EXPECT_EQ(a1, pool.content[0]);
        EXPECT_EQ(a2, pool.content[1]);
        EXPECT_EQ(a3, pool.content[2]);
    }
    {
        Pool<std::unique_ptr<int>> pool;

        auto p1 = pool.alloc(std::make_unique<int>(1)).get();
        auto p2 = pool.alloc(std::make_unique<int>(10)).get();
        auto p3 = pool.alloc(std::move(pool.content[0])).get();
        EXPECT_EQ(pool.content[0].get(), nullptr);
        EXPECT_EQ(pool.content[1].get(), p2);
        EXPECT_EQ(pool.content[2].get(), p1);
    }
}

TEST(Pool, free)
{
    {
        Pool<int> pool;

        int a = pool.alloc(1);
        int b = pool.alloc(10);
        int c = pool.alloc(100);

        pool.free(0);
        EXPECT_EQ(c, pool.content[0]);
        EXPECT_EQ(b, pool.content[1]);
    }
    {
        Pool<int> pool;

        for (int i = 0; i < 1024; ++i)
            pool.alloc(321);
        for (int i = 0; i < 1024; ++i)
            pool.alloc(123);
        for (int i = 0; i < 1024; ++i)
            pool.alloc(4321);

        for (int i = 0; i < 512; ++i)
            pool.free(1024 + i); // delete half of 123

        for (int i = 0; i < 1024; ++i)
            EXPECT_EQ(321, pool.content[i]);
        for (int i = 0; i < 512; ++i)
            EXPECT_EQ(4321, pool.content[1024 + i]); // 123 replaced by 4321
        for (int i = 0; i < 512; ++i)
            EXPECT_EQ(123, pool.content[1024 + 512 + i]);
        for (int i = 0; i < 512; ++i)
            EXPECT_EQ(4321, pool.content[2 * 1024 + i]);
    }
    {
        Pool<std::unique_ptr<int>> pool;

        auto p1 = pool.alloc(std::make_unique<int>(1)).get();
        auto p2 = pool.alloc(std::make_unique<int>(10)).get();
        auto p3 = pool.alloc(std::make_unique<int>(100)).get();
        pool.free(0);
        EXPECT_EQ(pool.content[0].get(), p3);
        EXPECT_EQ(pool.content[1].get(), p2);
    }
}

TEST(Pool, prepareToFitNMore)
{
    Pool<int> pool;
    int*      ptr = &pool.alloc(123);
    pool.prepareToFitNMore(512);

    ASSERT_NE(ptr, &pool.content[0]);
    ASSERT_EQ(123, pool.content[0]);

    std::vector<int*> ptrs(512);
    for (int i = 0; i < 512; ++i)
        ptrs[i] = &pool.alloc(i);

    for (int i = 0; i < 512; ++i)
        ASSERT_EQ(ptrs[i], &pool.content[1 + i]);

    pool.prepareToFitNMore(5096);
    for (int i = 0; i < 512; ++i)
    {
        ASSERT_NE(ptrs[i], &pool.content[1 + i]);
        ASSERT_EQ(i, pool.content[1 + i]);
    }
}