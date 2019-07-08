#ifndef CFILTER_H
#define CFILTER_H

#include "Bitmask.h"

#include <ECSpp/Component.h>

namespace epp
{

class CFilter
{
public:
    CFilter() = default;

    // if wanted n unwated != 0, then unwanted = unwanted & ~(unwanted & wanted)
    CFilter(Bitmask wanted, Bitmask unwated);


    void clear();


    void setWanted(Bitmask wantedMask);

    template<class... CTypes>
    CFilter& addWanted();

    template<class... IdTypes>
    CFilter& addWanted(IdTypes... ids);

    template<class... CTypes>
    CFilter& removeWanted();

    template<class... IdTypes>
    CFilter& removeWanted(IdTypes... ids);


    void setUnwanted(Bitmask unwantedMask);

    template<class... CTypes>
    CFilter& addUnwanted();

    template<class... IdTypes>
    CFilter& addUnwanted(IdTypes... ids);

    template<class... CTypes>
    CFilter& removeUnwanted();

    template<class... IdTypes>
    CFilter& removeUnwanted(IdTypes... ids);


    Bitmask& getWantedCMask();

    const Bitmask& getWantedCMask() const;

    Bitmask& getUnwantedCMask();

    const Bitmask& getUnwantedCMask() const;


    size_t hash() const;


    bool operator==(const CFilter& rhs) const;

    bool operator!=(const CFilter& rhs) const;


private:
    Bitmask wantedMask;

    Bitmask unwantedMask;

    mutable bool rehash = true;

    mutable size_t hashValue = 0;


    friend struct std::hash<CFilter>;
};


template<class... CTypes>
inline CFilter& CFilter::addWanted()
{
    return addWanted(getCTypeId<CTypes>()...);
}

template<class... IdTypes>
inline CFilter& CFilter::addWanted(IdTypes... ids)
{
    (wantedMask.set(ids), ...);
    (unwantedMask.unset(ids), ...);
    rehash = true;
    return *this;
}

template<class... CTypes>
inline CFilter& CFilter::removeWanted()
{
    return removeWanted(getCTypeId<CTypes>()...);
}

template<class... IdTypes>
inline CFilter& CFilter::removeWanted(IdTypes... ids)
{
    (wantedMask.unset(ids), ...);
    rehash = true;
    return *this;
}

template<class... CTypes>
inline CFilter& CFilter::addUnwanted()
{
    return addUnwanted(getCTypeId<CTypes>()...);
}

template<class... IdTypes>
inline CFilter& CFilter::addUnwanted(IdTypes... ids)
{
    (unwantedMask.set(ids), ...);
    (wantedMask.unset(ids), ...);
    rehash = true;
    return *this;
}

template<class... CTypes>
inline CFilter& CFilter::removeUnwanted()
{
    return removeUnwanted(getCTypeId<CTypes>()...);
}

template<class... IdTypes>
inline CFilter& CFilter::removeUnwanted(IdTypes... ids)
{
    (unwantedMask.unset(ids), ...);
    rehash = true;
    return *this;
}

} // namespace epp


namespace std
{
template<>
struct hash<epp::CFilter>
{
    size_t operator()(const epp::CFilter& filter) const
    {
        return hasher(filter.wantedMask) - hasher(filter.unwantedMask) % (0x7FFFFFFF);
    }
    hash<epp::Bitmask> hasher;
};
} // namespace std

#endif // CFILTER_H