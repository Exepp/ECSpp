#ifndef EPP_CMASK_H
#define EPP_CMASK_H

#include <ECSpp/Component.h>
#include <bitset>

namespace epp {

/// A Bitset wrapper
/**
 * In a CMask, every bit represents a unique registered type.
 * Each type has a unique id, counting sequentially from 0. 
 * Each id corresponds to exactly one bit in a CMask.
 */
class CMask {
public:
    using IdxList_t = decltype(IdOfL<>());
    using Idx_t = IdxList_t::value_type;
    using Bitset_t = std::bitset<CMetadata::MaxRegisteredComponents>;

public:
    /// Sets the bits corresponding to the ComponentIds from a given list
    /**
     * By default list is empty
     * @param list A List of ComponentIds returned from CMetadata::Id (or IdOf) function
     */
    CMask(IdxList_t list = {});


    /// Move Constructor
    /**
     * Takes the state of rval
     * rval is cleared
     */
    CMask(CMask&& rval);


    /// Default Copy Constructor
    CMask(CMask const&) = default;


    /// Move Assignment
    /**
     * Takes the state of rval
     * rval is cleared
     */
    CMask& operator=(CMask&& rval);


    /// Default Copy Assignment
    CMask& operator=(CMask const&) = default;


    /// Sets the bit corresponding to a given ComponentId
    /**
     * @param bitIndex A ComponentId returned from CMetadata::Id (or IdOf) function
     */
    void set(Idx_t bitIndex);


    /// Sets the bits corresponding to the ComponentIds from a given list
    /**
     * @param list A List of ComponentIds returned from CMetadata::Id (or IdOf) function
     */
    void set(IdxList_t list);


    /// Unsets the bit corresponding to a given ComponentId
    /**
     * @param bitIndex A ComponentId returned from CMetadata::Id (or IdOf) function
     */
    void unset(Idx_t bitIndex);


    /// Unsets the bits corresponding to the ComponentIds from a given list
    /**
     * @param list A List of ComponentIds returned from CMetadata::Id (or IdOf) function
     */
    void unset(IdxList_t list);


    /// Unsets the bits that are set both in this CMask and in the other one
    /**
     * @param other Any CMask
     * @returns A reference to this object
     */
    CMask& removeCommon(CMask const& other);


    /// Unsets all bits
    void clear();


    /// Returns the value of the bit corresponding to a given ComponentId
    /**
     * @param bitIndex A ComponentId returned from CMetadata::Id (or IdOf) function
     * @returns The value of the bit
     */
    bool get(Idx_t bitIndex) const;


    /// Returns the number of set bits
    /**
     * @returns The number of set bits
     */
    std::size_t getSetCount() const;


    /// Returns the number of bits that are set both in this CMask and in the other one
    /**
     * @param other Any CMask
     * @returns The number of set bits
     */
    std::size_t numberOfCommon(CMask const& other) const;


    /// Returns whether there is at least one bit that is set both in this CMask and in the other one
    /**
     * @param other Any CMask
     * @returns True when there is at least one common bit, false otherwise
     */
    bool hasCommon(CMask const& other) const;


    /// Returns whether every set bit from a given CMask is also set in this one
    /**
     * @param other Any CMask
     * @returns True when other is a subset of this CMask, false otherewise
     */
    bool contains(CMask const& other) const;


    /// Returns the underlying bitset
    /**
     * @returns Bitset used by this class
     */
    Bitset_t& getBitset();


    /// Returns the underlying bitset
    /**
     * @returns Bitset used by this class
     */
    Bitset_t const& getBitset() const;


    /// Returns whether all the set bits and the unset ones are the same in both CMasks
    /**
     * @param other Any CMask
     * @returns True if both are equal, false otherwise 
     */
    bool operator==(CMask const& rhs) const;


    /// Returns whether at least one bit is different in both CMasks
    /**
     * @param other Any CMask
     * @returns True if both are different, false otherwise
     */
    bool operator!=(CMask const& rhs) const;

private:
    Bitset_t bitset;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


inline CMask::CMask(IdxList_t list) { set(list); }

inline CMask::CMask(CMask&& rval) { *this = std::move(rval); }

inline CMask& CMask::operator=(CMask&& rval)
{
    bitset = std::move(rval.bitset);
    rval.bitset.reset();
    return *this;
}

inline void CMask::set(Idx_t bitIndex) { bitset.set(bitIndex.value); }

inline void CMask::set(IdxList_t list)
{
    for (auto idx : list)
        set(idx);
}

inline void CMask::unset(Idx_t bitIndex) { bitset.set(bitIndex.value, false); }

inline void CMask::unset(IdxList_t list)
{
    for (auto idx : list)
        unset(idx);
}

inline CMask& CMask::removeCommon(CMask const& other)
{
    bitset &= ~other.bitset;
    return *this;
}

inline void CMask::clear() { bitset.reset(); }

inline bool CMask::get(Idx_t bitIndex) const { return bitset.test(bitIndex.value); }

inline std::size_t CMask::getSetCount() const { return bitset.count(); }

inline std::size_t CMask::numberOfCommon(CMask const& other) const { return (bitset & other.bitset).count(); }

inline bool CMask::hasCommon(CMask const& other) const { return (bitset & other.bitset).any(); }

inline bool CMask::contains(CMask const& other) const { return other.bitset == (other.bitset & bitset); }

inline CMask::Bitset_t& CMask::getBitset() { return bitset; }

inline CMask::Bitset_t const& CMask::getBitset() const { return bitset; }

inline bool CMask::operator==(CMask const& rhs) const { return bitset == rhs.bitset; }

inline bool CMask::operator!=(CMask const& rhs) const { return !(*this == rhs); }

} // namespace epp

#endif // EPP_CMASK_H;