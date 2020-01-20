#ifndef CFilter_H
#define CFilter_H

#include <ECSpp/Component.h>
#include <ECSpp/Internal/CMask.h>

namespace epp {

class CFilter {
    using IdxList_t = CMask::IdxList_t;

    using Idx_t = CMask::Idx_t;

public:
    // if wanted & unwated (common part) != 0, then unwanted = unwanted & ~wanted
    CFilter(CMask const& wanted = {}, CMask unwanted = {}) : wantedMask(wanted), unwantedMask(unwanted.removeCommon(wantedMask)) {}


    void setWanted(CMask const& wanted)
    {
        wantedMask = wanted;
        unwantedMask.removeCommon(wantedMask);
    }

    CFilter& addWanted(Idx_t idx)
    {
        wantedMask.set(idx);
        unwantedMask.unset(idx);
        return *this;
    }

    CFilter& addWanted(IdxList_t idxList)
    {
        wantedMask.set(idxList);
        unwantedMask.removeCommon(wantedMask);
        return *this;
    }

    CFilter& removeWanted(Idx_t idx)
    {
        wantedMask.unset(idx);
        return *this;
    }

    CFilter& removeWanted(IdxList_t idxList)
    {
        wantedMask.unset(idxList);
        return *this;
    }

    void setUnwanted(CMask const& unwanted)
    {
        unwantedMask = unwanted;
        wantedMask.removeCommon(unwantedMask);
    }

    CFilter& addUnwanted(Idx_t idx)
    {
        unwantedMask.set(idx);
        wantedMask.unset(idx);
        return *this;
    }

    CFilter& addUnwanted(IdxList_t idxList)
    {
        unwantedMask.set(idxList);
        wantedMask.removeCommon(unwantedMask);
        return *this;
    }

    CFilter& removeUnwanted(Idx_t idx)
    {
        unwantedMask.unset(idx);
        return *this;
    }

    CFilter& removeUnwanted(IdxList_t idxList)
    {
        unwantedMask.unset(idxList);
        return *this;
    }

    void clear()
    {
        wantedMask.clear();
        unwantedMask.clear();
    }

    const CMask& getWanted() const { return wantedMask; }

    const CMask& getUnwanted() const { return unwantedMask; }

    bool operator==(const CFilter& rhs) const { return (wantedMask == rhs.wantedMask && unwantedMask == rhs.unwantedMask); }

    bool operator!=(const CFilter& rhs) const { return !(*this == rhs); }

    bool operator&(CMask const& rhs) const { return rhs.contains(wantedMask) && !rhs.hasCommon(unwantedMask); }

private:
    CMask wantedMask; // must be declared first

    CMask unwantedMask;
};

} // namespace epp


#endif // CFilter_H