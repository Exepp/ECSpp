#ifndef ARCHETYPE_H
#define ARCHETYPE_H

#include <ECSpp/Internal/CMask.h>
#include <ECSpp/external/llvm/SmallVector.h>
#include <algorithm>
#include <memory>

namespace epp {

class Archetype {
    using IdList_t = decltype(IdOfL<>());
    using CId_t = IdList_t::value_type;
    using IdVec_t = llvm::SmallVector<CId_t, 8>;

public:
    Archetype() = default;

    explicit Archetype(IdList_t initList) { addComponent(initList); }

    template <typename... CTypes>
    Archetype& addComponent() { return addComponent(IdOfL<CTypes...>()); }
    Archetype& addComponent(IdList_t ids)
    {
        for (auto id : ids)
            addComponent(id);
        return *this;
    }

    Archetype& addComponent(CId_t id)
    {
        if (!cMask.get(id)) {
            cMask.set(id);
            cIds.push_back(id);
        }
        return *this;
    }


    template <typename... CTypes>
    Archetype& removeComponent() { return removeComponent(IdOfL<CTypes...>()); }
    Archetype& removeComponent(IdList_t ids)
    {
        for (auto id : ids)
            removeComponent(id);
        return *this;
    }

    Archetype& removeComponent(CId_t id)
    {
        if (has(id)) {
            cMask.unset(id);
            cIds.erase(std::find_if(cIds.begin(), cIds.end(), [id](auto const& cId) { return cId == id; }));
        }
        return *this;
    }

    bool hasAllOf(IdList_t ids) const { return cMask.contains(ids); }
    bool hasAnyOf(IdList_t ids) const { return cMask.hasCommon(ids); }
    bool has(CId_t id) const { return cMask.get(id); }

    IdVec_t const& getCIds() const { return cIds; }
    CMask const& getMask() const { return cMask; }

private:
    CMask cMask;
    IdVec_t cIds;
};

} // namespace epp

#endif // ARCHETYPE_H