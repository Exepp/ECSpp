#include <ECSpp/Internal/CPool.h>
#include <ECSpp/Utility/Pool.h>
#include <cmath>
#include <algorithm>

using namespace epp;


CPool::CPool(CId_t cId) : cId(cId), metadata(CMetadata::GetData(cId)) {}

CPool::CPool(CPool&& rval)
    : data(rval.data), reserved(rval.reserved), dataUsed(rval.dataUsed), cId(rval.cId), metadata(rval.metadata)
{
    rval.data = nullptr;
    rval.reserved = 0;
    rval.dataUsed = 0;
}

CPool& CPool::operator=(CPool&& rval)
{
    this->~CPool();
    return *(new (this) CPool(std::move(rval)));
}

void* CPool::alloc()
{
    if (dataUsed >= reserved)
        reserve(reserved ? 2 * reserved : 4); // 4 as first size
    return addressAtIdx(dataUsed++);          // post-inc here
}

void* CPool::alloc(Idx_t n)
{
    if (n == 0)
        return nullptr;
    fitNextN(n);
    void* ptr = addressAtIdx(dataUsed);
    dataUsed += n;
    return ptr;
}

bool CPool::destroy(Idx_t i)
{
    EPP_ASSERT(i < dataUsed);

    bool notLast = (i + 1) < dataUsed;
    if (notLast) {
        metadata.destructor(addressAtIdx(i));
        construct(i, addressAtIdx(dataUsed - 1));
    }
    metadata.destructor(addressAtIdx(--dataUsed)); // pre-dec here
    return notLast;
}

void CPool::fitNextN(Idx_t n)
{
    n = SizeToFitNextN(n, reserved, reserved - dataUsed);
    if (n > reserved)
        reserve(n);
}

void CPool::reserve(Idx_t newReserved)
{
    if (newReserved == reserved)
        return;
    void* newData = newReserved ? operator new[](metadata.size* std::uint64_t(newReserved), std::align_val_t(metadata.alignment))
                                : nullptr;
    if (data) {
        auto toMove = std::min(newReserved, dataUsed);
        for (Idx_t i = 0; i < toMove; ++i)
            metadata.moveConstructor(addressAtIdx(newData, i), addressAtIdx(i));
        for (Idx_t i = 0; i < dataUsed; ++i)
            metadata.destructor(addressAtIdx(i));
        dataUsed = toMove;
        operator delete[](data, std::align_val_t(metadata.alignment));
    }
    data = newData;
    reserved = newReserved;
}

void CPool::clear()
{
    for (Idx_t i = 0; i < dataUsed; ++i)
        metadata.destructor(addressAtIdx(i));
    dataUsed = 0;
}