#include "ComponentsT.h"
#include <ECSpp/internal/CPool.h>
#include <gtest/gtest.h>

using namespace epp;

template <typename CompT>
static void TestCPool(CPool& pool, Pool<CompT> const& correct)
{
    ASSERT_EQ(pool.getCId(), IdOf<CompT>());
    ASSERT_EQ(pool.size(), correct.data.size());
    ASSERT_GE(pool.capacity(), pool.size());
    ASSERT_EQ(CompT::AliveCounter, 2 * correct.data.size()); // constructed correctly (there are many tests, so it also tests destruction)
                                                             // * 2, to accout also for the "correct" vector
    for (std::size_t i = 0; i < correct.data.size(); ++i) {
        ASSERT_EQ(std::uintptr_t(pool[i]) & (alignof(CompT) - 1), 0);
        ASSERT_EQ(*static_cast<CompT*>(pool[i]), correct.data[i]);
    }
}


TEST(CPool, ComponentIdConstr)
{
    CPool pool(IdOf<TComp1>());
    TestCPool(pool, Pool<TComp1>());
}

TEST(CPool, Alloc_Construct)
{
    CPool pool(IdOf<TComp1>());
    Pool<TComp1> correct;
    correct.data.resize(100);
    for (int i = 0; i < 100; ++i) {
        pool.alloc();
        pool.construct(i);
    }
    TestCPool(pool, correct);
}

TEST(CPool, AllocN)
{
    CPool pool(IdOf<TComp2>());
    Pool<TComp2> correct;
    ASSERT_EQ(pool.alloc(0), nullptr);

    for (int n = 0; n < 10; ++n) {
        correct.data.resize(correct.data.size() + 100);
        pool.alloc(100);
        for (int i = 0; i < 100; ++i)
            pool.construct(n * 100 + i);
        TestCPool(pool, correct);
        ASSERT_EQ(pool.alloc(0), nullptr);
    }
}

TEST(CPool, MoveConstruct)
{
    CPool pool(IdOf<TComp3>());
    Pool<TComp3> correct;
    for (int i = 0; i < 100; ++i) {
        TComp3 temp({ 1, 2, 3 });
        correct.create(temp.data);
        pool.alloc();
        pool.construct(i, &temp);
    }
    TestCPool(pool, correct);
}

template <typename CompT>
static void CreateNDistinct(CPool& pool, Pool<CompT>& correct, std::size_t const N)
{
    for (int i = 0; i < N; ++i) {
        CompT temp({ (int)correct.data.size() + 1, (int)correct.data.size() + 2, (int)correct.data.size() + 3 });
        correct.create(temp.data);
        pool.alloc();
        pool.construct(pool.size() - 1, &temp);
    }
}

TEST(CPool, MoveConstr)
{
    {
        CPool pool((CPool(IdOf<TComp2>())));
        TestCPool(pool, Pool<TComp2>());
    }
    {
        CPool poolToMove(IdOf<TComp3>());
        Pool<TComp3> correct;
        CreateNDistinct(poolToMove, correct, 100);
        {
            CPool pool(std::move(poolToMove));
            TestCPool(pool, correct);
        }
        correct = Pool<TComp3>();
        TestCPool(poolToMove, correct);
    }
}

TEST(CPool, MoveAssign)
{
    CPool poolToMove(IdOf<TComp4>());
    Pool<TComp4> correct;
    CreateNDistinct(poolToMove, correct, 100);
    {
        CPool pool(IdOf<TComp4>());
        pool = std::move(poolToMove);
        TestCPool(pool, correct);
    }
    correct = Pool<TComp4>();
    TestCPool(poolToMove, correct);
}

TEST(CPool, Destroy)
{
    CPool pool(IdOf<TComp4>());
    Pool<TComp4> correct;
    CreateNDistinct(pool, correct, 100);

    for (int i = 0; i < correct.data.size() / 2; ++i) {
        std::size_t idxToErase = rand() % correct.data.size();
        correct.destroy(idxToErase);
        pool.destroy(idxToErase);
    }
    TestCPool(pool, correct);

    for (int i = 0; i < correct.data.size(); ++i) { // remove rest
        std::size_t idxToErase = rand() % correct.data.size();
        correct.destroy(idxToErase);
        pool.destroy(idxToErase);
    }
    TestCPool(pool, correct);
}

template <typename CompT>
static void TestFitNextN(CPool& pool, Pool<CompT>& correct) // increases capacity 2 times, sets size to oldCapacity + 1
{
    std::size_t befSize = pool.size();
    std::size_t befCap = pool.capacity();
    ASSERT_GE(befCap, 16);
    ASSERT_GE(befCap - befSize, 2);

    TestCPool(pool, correct);

    correct.create();
    pool.alloc();
    pool.construct(pool.size() - 1);
    void* first = pool[0];
    CreateNDistinct(pool, correct, befCap - befSize - 1);
    ASSERT_EQ(first, pool[0]);

    TestCPool(pool, correct);

    // one more than reserved
    correct.create();
    pool.alloc();
    pool.construct(pool.size() - 1);
    ASSERT_NE(first, pool[0]);
    ASSERT_GT(pool.capacity(), befCap);

    TestCPool(pool, correct);
}

TEST(CPool, FitNextN)
{
    CPool pool(IdOf<TComp1>());
    Pool<TComp1> correct;
    pool.fitNextN(100); // capacity = 128
    pool.fitNextN(50);  // still capacity = 128
    TestFitNextN(pool, correct);

    CreateNDistinct(pool, correct, 50);

    pool.fitNextN(1024); // capacity = 2048
    TestFitNextN(pool, correct);
    ASSERT_EQ(pool.size(), 2049);
    ASSERT_EQ(pool.capacity(), 4096);
}


template <typename CompT>
static void TestShrink(CPool& pool, Pool<CompT>& correct) // increases capacity 4 times, changes size to 2*capacity + 1 and shrinks to fit
{
    pool.fitNextN(llvm::NextPowerOf2(pool.capacity() + 16));
    TestFitNextN(pool, correct); // size = always 2^x + 1
    pool.shrinkToFit();
    TestCPool(pool, correct);
}

TEST(CPool, shrinkToFit)
{
    CPool pool(IdOf<TComp2>());
    Pool<TComp2> correct;
    pool.shrinkToFit();
    TestCPool(pool, correct);
    TestShrink(pool, correct); // 0 + 16 -> 16 --*2--> 32

    pool.fitNextN(256);
    TestShrink(pool, correct); // 32 + 256 (capacity = 512) --*2--> 1024 --TestFitNextN--> 2049
    ASSERT_EQ(pool.capacity(), 2049);
    ASSERT_EQ(pool.size(), 2049);
}

TEST(CPool, clear)
{
    CPool pool(IdOf<TComp3>());
    Pool<TComp3> correct;
    pool.clear();
    TestCPool(pool, correct);
    TestShrink(pool, correct);
    correct.data.clear();
    pool.clear();
    ASSERT_GT(pool.capacity(), 0);

    TestCPool(pool, correct);
    TestShrink(pool, correct);
}

TEST(CPool, reserve)
{
    CPool pool(IdOf<TComp1>());
    Pool<TComp1> correct;
    pool.reserve(100); // capacity = 128
    pool.reserve(50);  // still capacity = 128
    TestFitNextN(pool, correct);

    pool.reserve(10); // destroy 90
    correct.data.resize(10);
    correct.data.shrink_to_fit();
    TestCPool(pool, correct);

    pool.reserve(0);
    correct.data.resize(0);
    correct.data.shrink_to_fit();
    TestCPool(pool, correct);
}