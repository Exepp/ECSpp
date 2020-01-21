#ifndef CPOOL_H
#define CPOOL_H

#include <ECSpp/Component.h>

namespace epp {

// TODO: redo the tests
class CPool final {
    using CId_t = decltype(IdOfL<>())::value_type;

    using Idx_t = std::uint32_t;

public:
    explicit CPool(CId_t cId);

    CPool(CPool&& rval);

    CPool& operator=(CPool&& rval);

    CPool(CPool const&) = delete;

    CPool& operator=(CPool const&) = delete;

    ~CPool() { reset(); }


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
    void* construct(Idx_t idx)
    {
        EPP_ASSERT(idx < dataUsed);
        return metadata.defaultConstructor(addressAtIdx(idx));
    }

    // TODO: tests
    // calls move constructor on the object located at given index
    // it is up to the caller to make sure this function is called only on components
    // that are not yet constructed (that is, created with alloc) and that rValComp is of the same type as components of this pool
    void* construct(Idx_t idx, void* rValComp)
    {
        EPP_ASSERT(idx < dataUsed);
        return metadata.moveConstructor(addressAtIdx(idx), rValComp);
    }

    // expects an index to a constructed component (calls destructor)
    // returns true if deleted object was replaced with the last element (false only for the last element)
    bool destroy(Idx_t i);

    // makes sure, to fit n more elements without realloc
    void fitNextN(Idx_t n);

    void clear();

    // TODO: tests
    // clear + deallocation
    void reset();


    void* operator[](Idx_t i)
    {
        EPP_ASSERT(i < dataUsed)
        return addressAtIdx(i);
    }

    CId_t getCId() const { return cId; }

    std::size_t size() const { return dataUsed; }

    std::size_t capacity() const { return reserved; }

private:
    void reserve(Idx_t n);

    void* addressAtIdx(Idx_t i) const { return addressAtIdx(data, i); }

    void* addressAtIdx(void* base, Idx_t i) const { return reinterpret_cast<void*>(static_cast<std::uint8_t*>(base) + metadata.size * std::uint64_t(i)); }

private:
    void* data = nullptr;

    Idx_t reserved = 0;

    Idx_t dataUsed = 0;

    CMetadata metadata;

    CId_t const cId; // may change on assignment
};

} // namespace epp

#endif // CPool_H