#include <ECSpp/Internal/CPool.h>
#include <ECSpp/Utility/Pool.h>
#include <cmath>

using namespace epp;


CPool::CPool(CId_t cId) : cId(cId), metadata(CMetadata::GetData(cId)) {}

CPool::CPool(CPool&& rval)
    : data(rval.data), dataSize(rval.dataSize), dataUsed(rval.dataUsed), cId(rval.cId), metadata(rval.metadata)
{
    rval.data = nullptr;
    rval.dataSize = 0;
    rval.dataUsed = 0;
}

CPool& CPool::operator=(CPool&& rval)
{
    this->~CPool();
    return *(new (this) CPool(std::move(rval)));
}

CPool::~CPool()
{
    clear();
    if (data)
        operator delete[](data, _getComponentAlign());
}

void* CPool::alloc()
{
    if (dataUsed >= dataSize)
        reserve(2 * dataSize ?: 4);  // 4 as first size
    return addressAtIdx(dataUsed++); // post-inc here
}

void* CPool::create()
{
    return _construct(alloc());
}

void* CPool::create(void* rVal)
{
    return _moveConstruct(alloc(), rVal);
}

void* CPool::construct(Idx_t idx)
{
    EPP_ASSERT(idx < dataUsed);
    return _construct(addressAtIdx(idx));
}

bool CPool::destroy(Idx_t i)
{
    EPP_ASSERT(i < dataUsed);

    bool notLast = (i + 1) < dataUsed;
    if (notLast) {
        _destroy(addressAtIdx(i));
        _moveConstruct(addressAtIdx(i), addressAtIdx(dataUsed - 1));
    }
    _destroy(addressAtIdx(--dataUsed)); // pre-dec here
    return notLast;
}

void CPool::fitNextN(Idx_t n)
{
    reserve(SizeToFitNextN(n, dataSize, dataSize - dataUsed));
}

void CPool::reserve(Idx_t newSize)
{
    EPP_ASSERT(newSize >= dataSize);
    if (newSize == dataSize)
        return;
    void* newData = operator new[](_getComponentSize() * std::uint64_t(newSize), _getComponentAlign());

    if (data) {
        for (Idx_t i = 0; i < dataUsed; ++i) {
            _moveConstruct(addressAtIdx(newData, i), addressAtIdx(i));
            _destroy(addressAtIdx(i));
        }
        operator delete[](data, _getComponentAlign());
    }
    data = newData;
    dataSize = newSize;
}

void CPool::clear()
{
    for (Idx_t i = 0; i < dataUsed; ++i)
        _destroy(addressAtIdx(i));
    dataUsed = 0;
}

void* CPool::operator[](Idx_t i)
{
    EPP_ASSERT(i < dataUsed)
    return addressAtIdx(i);
}

inline void* CPool::_construct(void* address)
{
    return metadata.defaultConstructor(address);
}

inline void* CPool::_moveConstruct(void* dest, void* src)
{
    return metadata.moveConstructor(dest, src);
}

inline void CPool::_destroy(void* address)
{
    metadata.destructor(address);
}

inline std::uint32_t CPool::_getComponentSize() const
{
    return metadata.size;
}

inline std::align_val_t CPool::_getComponentAlign() const
{
    return std::align_val_t(metadata.alignment);
}

inline void* CPool::addressAtIdx(Idx_t i) const
{
    return addressAtIdx(data, i);
}

inline void* CPool::addressAtIdx(void* base, Idx_t i) const
{
    return reinterpret_cast<void*>(static_cast<std::uint8_t*>(base) + metadata.size * std::uint64_t(i));
}