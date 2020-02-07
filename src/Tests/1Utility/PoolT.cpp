#include <ECSpp/utility/Pool.h>
#include <gtest/gtest.h>

using namespace epp;

TEST(Pool, Create)
{
    {
        Pool<int> pool;
        int x = 2;
        int a1 = pool.create();
        int a2 = pool.create(-1);
        int a3 = pool.create(x);
        EXPECT_EQ(a1, 0);
        EXPECT_EQ(a2, -1);
        EXPECT_EQ(a3, x);
        EXPECT_EQ(a1, pool.data[0]);
        EXPECT_EQ(a2, pool.data[1]);
        EXPECT_EQ(a3, pool.data[2]);

        // force expansion
        for (int i = 0; i < 1024; ++i)
            pool.create(i);
        for (int i = 0; i < 1024; ++i)
            EXPECT_EQ(i, pool.data[3 + i]);
        EXPECT_EQ(a1, pool.data[0]);
        EXPECT_EQ(a2, pool.data[1]);
        EXPECT_EQ(a3, pool.data[2]);
    }
    {
        Pool<std::unique_ptr<int>> pool;

        auto p1 = pool.create(std::make_unique<int>(1)).get();
        auto p2 = pool.create(std::make_unique<int>(10)).get();
        auto p3 = pool.create(std::move(pool.data[0])).get();
        EXPECT_EQ(pool.data[0].get(), nullptr);
        EXPECT_EQ(pool.data[1].get(), p2);
        EXPECT_EQ(pool.data[2].get(), p1);
        EXPECT_EQ(p1, p3);
    }
}

TEST(Pool, Destroy)
{
    {
        Pool<int> pool;

        pool.create(1);
        int b = pool.create(10);
        int c = pool.create(100);

        EXPECT_TRUE(pool.destroy(0));
        EXPECT_EQ(c, pool.data[0]);
        EXPECT_EQ(b, pool.data[1]);
        EXPECT_FALSE(pool.destroy(1)); // 1 - last index (2 elements in pool - no swap)
    }
    {
        Pool<int> pool;

        for (int i = 0; i < 1024; ++i)
            pool.create(321);
        for (int i = 0; i < 1024; ++i)
            pool.create(123);
        for (int i = 0; i < 1024; ++i)
            pool.create(4321);

        for (int i = 0; i < 512; ++i)
            EXPECT_TRUE(pool.destroy(1024 + i)); // delete half of 123 (none of them is last, so there is an internal swap of elements)

        for (int i = 0; i < 1024; ++i)
            EXPECT_EQ(321, pool.data[i]);
        for (int i = 0; i < 512; ++i)
            EXPECT_EQ(4321, pool.data[1024 + i]); // 123 replaced by 4321
        for (int i = 0; i < 512; ++i)
            EXPECT_EQ(123, pool.data[1024 + 512 + i]);
        for (int i = 0; i < 512; ++i)
            EXPECT_EQ(4321, pool.data[2 * 1024 + i]);
    }
    {
        Pool<std::unique_ptr<int>> pool;

        pool.create(std::make_unique<int>(1)).get();
        auto p2 = pool.create(std::make_unique<int>(10)).get();
        auto p3 = pool.create(std::make_unique<int>(100)).get();
        pool.destroy(0);
        EXPECT_EQ(pool.data[0].get(), p3);
        EXPECT_EQ(pool.data[1].get(), p2);
    }
}

TEST(Pool, FitNextN)
{
    Pool<int> pool;
    int* ptr = &pool.create(123);
    pool.fitNextN(512);
    ASSERT_NE(ptr, &pool.data[0]);
    ASSERT_EQ(123, pool.data[0]);

    std::vector<int*> ptrs(512);
    for (std::size_t i = 0; i < 512; ++i)
        ptrs[i] = &pool.create(i);

    for (std::size_t i = 0; i < 512; ++i)
        ASSERT_EQ(ptrs[i], &pool.data[1 + i]);

    pool.fitNextN(5096);
    for (std::size_t i = 0; i < 512; ++i) {
        ASSERT_NE(ptrs[i], &pool.data[1 + i]);
        ASSERT_EQ(i, pool.data[1 + i]);
    }
}