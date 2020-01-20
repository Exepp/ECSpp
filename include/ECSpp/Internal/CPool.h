#ifndef CPOOL_H
#define CPOOL_H

#include <ECSpp/Component.h>

namespace epp {

class CPool final {
    using Idx_t = std::uint32_t;

public:
    explicit CPool(ComponentId cId);

    CPool(CPool&& rval);

    CPool& operator=(CPool&& rval);

    CPool(CPool const&) = delete;

    CPool& operator=(CPool const&) = delete;

    ~CPool();


    // raw (no constructor call)
    // !! EVERY ALLOCATED COMPONENT MUST BE CONSTRUCTED BEFORE NEXT ALLOC CALL !!
    // if there are 2 consecutive calls, the second one may lead to reallocation of the internal storage
    // which will move and destroy all components (even the uninitialized ones)
    // so move constructor and destructor will be called on the uninitialized component from the first call
    void* alloc();

    // alloc + default constructor
    void* create();

    // alloc + move constructor
    void* create(void* rVal);

    // calls constructor on an object located at given index
    // it is up to the caller to make sure this function is called only on components
    // that are not yet constructed (that is, created with alloc)
    void* construct(Idx_t idx);

    // expects an index to a constructed component (calls destructor)
    // returns true if deleted object was replaced with the last element (false only for the last element)
    bool destroy(Idx_t i);

    // makes sure, to fit n more elements without realloc
    void fitNextN(Idx_t n);

    void clear();


    void* operator[](Idx_t i);

    ComponentId getCId() const { return cId; }

    std::size_t size() const { return dataUsed; }

    std::size_t reserved() const { return dataSize; }

private:
    void reserve(Idx_t n);

    // constructs the object at the given address with default constructor
    void* _construct(void* address);

    // constructs the object at the given address with move constructor
    void* _moveConstruct(void* dest, void* src);

    // calls destructor on the component at the given address
    void _destroy(void* address);

    std::uint32_t _getComponentSize() const;

    std::align_val_t _getComponentAlign() const;

    void* addressAtIdx(Idx_t i) const;

    void* addressAtIdx(void* base, Idx_t i) const;

private:
    void* data = nullptr;

    Idx_t dataSize = 0;

    Idx_t dataUsed = 0;

    CMetadata metadata;

    ComponentId const cId; // may change on assignment
};

} // namespace epp

#endif // CPool_H