#ifndef EPP_CPOOL_H
#define EPP_CPOOL_H

#include <ECSpp/Component.h>
#include <ECSpp/internal/utility/Pool.h>

namespace epp {

/**
 * A Vector that uses CMetadata to manage the data without a type information
 * Does not maintain order - the last component is always moved in the place of a removed one
 */
class CPool final {
    using Idx_t = std::size_t;

public:
    /// Constructs a pool of components with ComponentIds equal to cId
    /**
     * CPool uses the cId to get metadata from the CMetadata::GetData static function
     * @param cId A ComponentId returned from CMetadata::Id (or IdOf) function
     */
    explicit CPool(ComponentId cId);


    /// Move constructor
    /**
     * Moves the ovnership of the rval's data to this CPool
     * Clears rval, leaves metadata intact
     * @param CPool Any CPool to be moved
     */
    CPool(CPool&& rval);


    /// Move assignment
    /**
     * Clears this CPool
     * Moves the ovnership of the rval's data to this CPool
     * Clears rval, leaves metadata intact
     * @param CPool Any CPool to be moved
     */
    CPool& operator=(CPool&& rval);


    /// Deleted copy constructor
    CPool(CPool const&) = delete;


    /// Deleted copy assignment
    CPool& operator=(CPool const&) = delete;


    /// Destructor
    /**
     * Frees owned resources
     */
    ~CPool() { reserve(0); }


    /// Allocates memory for one component
    /**
     * @details Allocated component is always located at size() index (after allocation size() - 1) - just as vector's push_back
     * @warning EVERY ALLOCATED COMPONENT MUST BE CONSTRUCTED BEFORE NEXT ALLOC CALL.
     * If there are 2 consecutive calls, the second one may lead to reallocation of the internal storage
     * which will move and destroy all the components (even the uninitialized ones).
     * This way the destructor and move constructor will be called on the uninitialized component from the first call
     * @returns The address of the allocated component. This addres is not permanent, every component may be moved freely by the CPool
     */
    void* alloc();


    /// Allocates memory for n components
    /**
     * The same warning as above 
     * @returns The address to the first of the allocated components. The components are stored contiguously
     */
    void* alloc(Idx_t n);


    /// Calls the constructor on the component located at a given index
    /**
     * @note It is up to the caller to make sure this function is called only on components
     * that are not yet constructed
     * 
     * @param idx Index of the component in this pool to call the constructor on
     * @returns The address to the first of the allocated components. The components are stored contiguously
     * @throws (Debug only) Throws the AssertionFailed exception if idx is greater or equal to the size()
     */
    void construct(Idx_t idx);


    /// Calls the move constructor on the component located at a given index
    /**
     * The same warning as above 
     * @param idx Index of the component in this pool to call the move constructor on
     * @returns The address of the component located at a given location
     * @throws (Debug only) Throws the AssertionFailed exception if idx is greater or equal to the size()
     */
    void construct(Idx_t idx, void* rValComp);


    /// Calls the destructor on the component located at a given index
    /**
     * The last components is moved in place of the removed one
     * The same warning as above 
     * @param idx Index of the component in this pool to call the destructor on
     * @returns True if deleted object was replaced with the last element (false only for the last element)
     * @throws (Debug only) Throws the AssertionFailed exception if idx is greater or equal to the size()
     */
    bool destroy(Idx_t idx);


    /// The next alloc(n) call or n alloc() calls will not require reallocation
    /** 
     * CPool will grow its capacity to the next power of 2 that will fit size() + n components 
     * @param n Number of components to reserve the additional space for
     */
    void fitNextN(std::size_t n);


    /// Removes the excess of the reserved memory
    void shrinkToFit() { reserve(dataUsed); }


    /// Destroys every component
    void clear();


    /// Changes the capacity to exactly newReserved
    /**
     * @details If newReserved < size() then components that don't fit get destroyed
     * @details If newReserved == capacity() nothing happens
     * @details If newReserved != capacity() reallocates to set capacity to newReserved
     * @param newReserved New capacity
     */
    void reserve(std::size_t newReserved);


    /// Returns pointer to the component located at a given index
    /**
     * @param idx Index of the component in this pool
     * @returns A Pointer to the component located at a given index
     * @throws (Debug only) Throws the AssertionFailed exception if idx is greater or equal to the size()
     */
    void* operator[](Idx_t idx);


    /** @copydoc CPool::operator[](Idx_t idx) */
    void const* operator[](Idx_t idx) const;


    /// Returns the ComponentId of this pool
    ComponentId getCId() const { return metadata.cId; }


    /// Returns the number of "used" components
    std::size_t size() const { return dataUsed; }


    /// Returns the capacity of the pool (how many components can it fit without reallocation)
    std::size_t capacity() const { return reserved; }

private:
    void* addressAtIdx(Idx_t idx) const { return addressAtIdx(data, idx); }

    void* addressAtIdx(void* base, Idx_t idx) const { return reinterpret_cast<void*>(static_cast<std::uint8_t*>(base) + metadata.size * std::uintptr_t(idx)); }

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
        fitNextN(reserved ? reserved : 4); // 4 as first size
    return addressAtIdx(dataUsed++);       // post-inc here
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

inline void CPool::construct(Idx_t idx)
{
    EPP_ASSERT(idx < dataUsed);
    metadata.defaultConstructor(addressAtIdx(idx));
}

inline void CPool::construct(Idx_t idx, void* rValComp)
{
    EPP_ASSERT(idx < dataUsed);
    metadata.moveConstructor(addressAtIdx(idx), rValComp);
}

inline bool CPool::destroy(Idx_t idx)
{
    EPP_ASSERT(idx < dataUsed);

    bool notLast = (idx + 1) < dataUsed;
    if (notLast) {
        metadata.destructor(addressAtIdx(idx));
        construct(idx, addressAtIdx(dataUsed - 1));
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

inline void* CPool::operator[](Idx_t idx)
{
    EPP_ASSERT(idx < dataUsed)
    return addressAtIdx(idx);
}

inline void const* CPool::operator[](Idx_t idx) const
{
    EPP_ASSERT(idx < dataUsed)
    return addressAtIdx(idx);
}

} // namespace epp

#endif // CPool_H