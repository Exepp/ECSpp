#ifndef EPP_CPOOL_H
#define EPP_CPOOL_H

#include <ECSpp/Component.h>
#include <ECSpp/utility/Pool.h>

namespace epp {

// TODO: redo the tests
class CPool final {
    using Idx_t = std::size_t;

public:
    explicit CPool(ComponentId cId);
    CPool(CPool&& rval);
    CPool& operator=(CPool&& rval);
    CPool(CPool const&) = delete;
    CPool& operator=(CPool const&) = delete;
    ~CPool() { reserve(0); }


    // raw (no constructor call)
    // !! EVERY ALLOCATED COMPONENT MUST BE CONSTRUCTED BEFORE NEXT ALLOC CALL !!
    // if there are 2 consecutive calls, the second one may lead to reallocation of the internal storage
    // which will move and destroy all components (even the uninitialized ones)
    // so move constructor and destructor will be called on the uninitialized component from the first call
    void* alloc();

    // TODO: tests
    // same as above
    // allocates at once n components
    // returns nullptr for n == 0
    void* alloc(Idx_t n);

    // calls constructor on the object located at given index
    // it is up to the caller to make sure this function is called only on components
    // that are not yet constructed (that is, created with alloc)
    void* construct(Idx_t idx);

    // TODO: tests
    // calls move constructor on the object located at given index
    // it is up to the caller to make sure this function is called only on components
    // that are not yet constructed (that is, created with alloc) and that rValComp is of the same type as components of this pool
    void* construct(Idx_t idx, void* rValComp);

    // expects an index to a constructed component (calls destructor)
    // returns true if deleted object was replaced with the last element (false only for the last element)
    bool destroy(Idx_t i);

    // makes sure, to fit n more elements without realloc
    void fitNextN(std::size_t n);
    void shrinkToFit() { reserve(dataUsed); }
    void clear();
    void reserve(std::size_t newReserved);

    void* operator[](Idx_t i);
    void const* operator[](Idx_t i) const;

    ComponentId getCId() const { return metadata.cId; }
    std::size_t size() const { return dataUsed; }
    std::size_t capacity() const { return reserved; }

private:
    void* addressAtIdx(Idx_t i) const { return addressAtIdx(data, i); }

    void* addressAtIdx(void* base, Idx_t i) const { return reinterpret_cast<void*>(static_cast<std::uint8_t*>(base) + metadata.size * std::uint64_t(i)); }

private:
    void* data = nullptr;
    std::size_t reserved = 0;
    std::size_t dataUsed = 0;
    CMetadata const metadata;
};


inline CPool::CPool(ComponentId cid) : metadata(CMetadata::GetData(cid)) {}

inline CPool::CPool(CPool&& rval)
    : data(rval.data),
      reserved(rval.reserved),
      dataUsed(rval.dataUsed),
      metadata(rval.metadata)
{
    rval.data = nullptr;
    rval.reserved = 0;
    rval.dataUsed = 0;
}

inline CPool& CPool::operator=(CPool&& rval)
{
    this->~CPool();
    return *(new (this) CPool(std::move(rval)));
}

inline void* CPool::alloc()
{
    if (dataUsed >= reserved)
        reserve(reserved ? 2 * reserved : 4); // 4 as first size
    return addressAtIdx(dataUsed++);          // post-inc here
}

inline void* CPool::alloc(Idx_t n)
{
    if (n == 0)
        return nullptr;
    fitNextN(n);
    void* ptr = addressAtIdx(dataUsed);
    dataUsed += n;
    return ptr;
}

inline void* CPool::construct(Idx_t idx)
{
    EPP_ASSERT(idx < dataUsed);
    return metadata.defaultConstructor(addressAtIdx(idx));
}

inline void* CPool::construct(Idx_t idx, void* rValComp)
{
    EPP_ASSERT(idx < dataUsed);
    return metadata.moveConstructor(addressAtIdx(idx), rValComp);
}

inline bool CPool::destroy(Idx_t i)
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

inline void CPool::fitNextN(std::size_t n)
{
    reserve(SizeToFitNextN(n, reserved, reserved - dataUsed));
}

inline void CPool::reserve(std::size_t newReserved)
{
    if (newReserved == reserved)
        return;
    void* newData = newReserved ? operator new[](metadata.size* newReserved, std::align_val_t(metadata.alignment))
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

inline void CPool::clear()
{
    for (Idx_t i = 0; i < dataUsed; ++i)
        metadata.destructor(addressAtIdx(i));
    dataUsed = 0;
}

inline void* CPool::operator[](Idx_t i)
{
    EPP_ASSERT(i < dataUsed)
    return addressAtIdx(i);
}

inline void const* CPool::operator[](Idx_t i) const
{
    EPP_ASSERT(i < dataUsed)
    return addressAtIdx(i);
}

} // namespace epp

#endif // CPool_H