#ifndef EPP_CMASK_H
#define EPP_CMASK_H

#include <ECSpp/Component.h>
#include <bitset>

namespace epp {

class CMask {
public:
    using IdxList_t = decltype(IdOfL<>());
    using Idx_t = IdxList_t::value_type;
    using Bitset_t = std::bitset<CMetadata::MaxRegisteredComponents>;

public:
    CMask(IdxList_t list = {});
    CMask(CMask&& rval);
    CMask(CMask const& rval) = default;
    CMask& operator=(CMask&& rval);
    CMask& operator=(CMask const&) = default;

    void set(Idx_t bitIndex);
    void set(IdxList_t list);
    void unset(Idx_t bitIndex);
    void unset(IdxList_t list);

    CMask& removeCommon(CMask const& other);

    void clear();

    bool get(Idx_t bitIndex) const;
    std::size_t getSetCount() const;
    std::size_t numberOfCommon(CMask const& other) const;
    bool hasCommon(CMask const& other) const;
    bool contains(CMask const& other) const;

    Bitset_t& getBitset();
    Bitset_t const& getBitset() const;

    bool operator==(CMask const& rhs) const;
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