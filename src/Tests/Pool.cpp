// TEST(PoolTest, Alloc_Free_Capacity)
// {
//     Pool<int> pool;
//     int       x  = 2;
//     int       a1 = pool.alloc();
//     int       a2 = pool.alloc(-1);
//     int       a3 = pool.alloc(x);

//     EXPECT_EQ(a1, pool[0]);
//     EXPECT_EQ(a2, pool[1]);
//     EXPECT_EQ(a3, pool[2]);
//     EXPECT_EQ(pool.getSize(), 3);

//     pool.free(0); // move last object to freed index (2 -> 0)
//     pool.free(2); // free again, now invalid index
//     EXPECT_NE(a1, pool[0]);
//     EXPECT_EQ(a3, pool[0]);
//     EXPECT_EQ(a2, pool[1]);
//     EXPECT_EQ(pool.getSize(), 2);

//     pool.reserve(10);
//     EXPECT_EQ(pool.getReserved(), 10);
//     EXPECT_EQ(pool.getSize(), 2);

//     pool.clear();
//     EXPECT_EQ(pool.getSize(), 0);
//     EXPECT_EQ(pool.getReserved(), 10);

//     for (size_t i = 0; i < 8; i++)
//         pool.alloc();

//     pool.reserve(2);

//     EXPECT_EQ(pool.getSize(), 2);
// }

// TEST(PoolTest, AllocN)
// {
//     Pool<int> pool;
//     auto      it = pool.allocN(10, 123);

//     ASSERT_GT(pool.getReserved(), 10);
//     ASSERT_EQ(pool.getSize(), 10);

//     int i = 0;
//     for (; it != pool.end(); ++it)
//     {
//         ASSERT_EQ(*it, 123);
//         ++i;
//     }
//     ASSERT_EQ(i, 10);
// }

// TEST(PoolTest, Access_Iterators)
// {
//     Pool<int>    pool;
//     const size_t allocCount = 6;
//     for (size_t i = 0; i < allocCount; i++)
//         pool.alloc();

//     Pool<int>::Iterator_t iterator(&pool[0]);
//     ASSERT_EQ(iterator, pool.begin());
//     ASSERT_EQ(&*iterator, &pool.front());
//     for (size_t i = 0; i < allocCount - 1; i++)
//         ++iterator;
//     ASSERT_EQ(&*iterator, &pool.back());
//     ++iterator;
//     ASSERT_EQ(iterator, pool.end());

// #ifdef _DEBUG
//     // test the exception throwing on out of range index
//     bool catched = false;
//     try
//     {
//         pool[allocCount];
//     }
//     catch (std::out_of_range)
//     {
//         catched = true;
//     }
//     // did not catch out of range exception
//     ASSERT_TRUE(catched);
// #endif
// }

// TEST(PoolTest, Operator)
// {
//     Pool<int>    pool;
//     const size_t allocCount = 6;
//     for (size_t i = 0; i < allocCount; i++)
//         pool.alloc();
//     auto poolBegin = &pool.front();

//     Pool<int> moveConstructed = std::move(pool);

//     ASSERT_GT(moveConstructed.getReserved(), allocCount);
//     ASSERT_EQ(moveConstructed.getSize(), allocCount);
//     ASSERT_EQ(&moveConstructed[0], poolBegin);
// }

// TEST(PoolIteratorTest, Operators)
// {
//     Pool<int> pool;
//     size_t    n = 20;
//     pool.allocN(n);

//     auto it = pool.begin();

//     ASSERT_EQ(it + n, pool.end());
//     ASSERT_EQ(it, pool.end() - n);
//     ASSERT_EQ(it += n, pool.end());
//     ASSERT_EQ(it -= n, pool.begin());
//     ASSERT_EQ(pool.end() - it, ptrdiff_t(n));

//     ASSERT_TRUE(it < pool.end());
//     ASSERT_TRUE(pool.end() > it);
//     ASSERT_TRUE(it >= pool.begin());
//     ASSERT_TRUE(it <= pool.begin());
//     ASSERT_TRUE((it += n) == pool.end());
//     ASSERT_TRUE(it != pool.begin());

//     it = pool.begin();
//     ASSERT_TRUE(it++ == pool.begin());
//     ASSERT_TRUE(++it == pool.begin() + 2);
//     ASSERT_TRUE(it-- == pool.begin() + 2);
//     ASSERT_TRUE(--it == pool.begin());

//     ASSERT_EQ(&*it, &it[0]);
//     ASSERT_EQ(&*(it + n - 1), &it[n - 1]);
// }