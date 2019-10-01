#ifndef CFILTER_H
#define CFILTER_H

#include <ECSpp/EntityManager/ComponentUtility.h>
#include <ECSpp/Utility/Bitmask.h>

namespace epp
{

class BitFilter
{
    using IdxList_t = Bitmask::IdxList_t;

public:
    BitFilter() = default;
    // if wanted /\ unwated != 0, then unwanted = unwanted & ~(unwanted & wanted)
    BitFilter(Bitmask wanted, Bitmask unwanted);

    void clear();


    void setWanted(Bitmask wantedMask);

    BitFilter& addWanted(IdxList_t idxList);

    BitFilter& removeWanted(IdxList_t idxList);


    void setUnwanted(Bitmask unwantedMask);

    BitFilter& addUnwanted(IdxList_t idxList);

    BitFilter& removeUnwanted(IdxList_t idxList);


    Bitmask& getWantedMask();

    const Bitmask& getWantedMask() const;

    Bitmask& getUnwantedMask();

    const Bitmask& getUnwantedMask() const;


    bool operator==(BitFilter const& rhs) const;

    bool operator!=(BitFilter const& rhs) const;

    // returns true if rhs meets requirements of this filter, false otherwise
    bool operator&(Bitmask const& rhs) const;

private:
    Bitmask wantedMask;

    Bitmask unwantedMask;


    friend struct std::hash<BitFilter>;
};

} // namespace epp


namespace std
{
template<>
struct hash<epp::BitFilter>
{
    std::size_t operator()(const epp::BitFilter& filter) const
    {
        hash<epp::Bitmask> hasher;
        auto               unwanted = hasher(filter.unwantedMask);
        return hasher(filter.wantedMask) + 0x9e3779b97f4a7c15 + (unwanted << 6) + (unwanted >> 2);
    }
};
} // namespace std

#endif // CFILTER_H