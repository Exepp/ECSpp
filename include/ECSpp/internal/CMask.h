#ifndef CMASK_H
#define CMASK_H

#include <ECSpp/Component.h>
#include <bitset>
#include <cstdint>

namespace epp {

class CMask {
public:
    using IdxList_t = decltype(IdOfL<>());
    using Idx_t = IdxList_t::value_type;
    using Bitset_t = std::bitset<CMetadata::MaxRegisteredComponents>;

public:
    CMask(IdxList_t list = {}) { set(list); }
    CMask(CMask&& rval) { *this = std::move(rval); }
    CMask& operator=(CMask&& rval)
    {
        bitset = std::move(rval.bitset);
        rval.bitset.reset();
        return *this;
    }

    CMask(CMask const& rval) = default;
    CMask& operator=(CMask const&) = default;

    void set(Idx_t bitIndex) { bitset.set(bitIndex.value); }
    void set(IdxList_t list)
    {
        for (auto idx : list)
            set(idx);
    }

    void unset(Idx_t bitIndex) { bitset.set(bitIndex.value, false); }
    void unset(IdxList_t list)
    {
        for (auto idx : list)
            unset(idx);
    }

    CMask& removeCommon(CMask const& other)
    {
        bitset &= ~other.bitset;
        return *this;
    }

    void clear() { bitset.reset(); }

    bool get(Idx_t bitIndex) const { return bitset.test(bitIndex.value); }
    std::size_t getSetCount() const { return bitset.count(); }

    std::size_t numberOfCommon(CMask const& other) const { return (bitset & other.bitset).count(); }
    bool hasCommon(CMask const& other) const { return (bitset & other.bitset).any(); }
    bool contains(CMask const& other) const { return other.bitset == (other.bitset & bitset); }

    Bitset_t& getBitset() { return bitset; }
    Bitset_t const& getBitset() const { return bitset; }

    bool operator==(CMask const& rhs) const { return bitset == rhs.bitset; }
    bool operator!=(CMask const& rhs) const { return !(*this == rhs); }

private:
    Bitset_t bitset;
};


} // namespace epp

#endif // CMASK_H;