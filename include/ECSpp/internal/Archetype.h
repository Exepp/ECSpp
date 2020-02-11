#ifndef EPP_ARCHETYPE_H
#define EPP_ARCHETYPE_H

#include <ECSpp/external/llvm/SmallVector.h>
#include <ECSpp/internal/CMask.h>

namespace epp {

class Archetype {
    using IdList_t = decltype(IdOfL<>());
    using CId_t = IdList_t::value_type;
    using IdVec_t = llvm::SmallVector<CId_t, 8>;

public:
    Archetype() = default;
    explicit Archetype(IdList_t initList);

    template <typename... CTypes>
    Archetype& addComponent();
    Archetype& addComponent(IdList_t ids);
    Archetype& addComponent(CId_t id);

    template <typename... CTypes>
    Archetype& removeComponent();
    Archetype& removeComponent(IdList_t ids);
    Archetype& removeComponent(CId_t id);

    bool hasAllOf(IdList_t ids) const;
    bool hasAnyOf(IdList_t ids) const;
    bool has(CId_t id) const;

    IdVec_t const& getCIds() const;
    CMask const& getMask() const;

private:
    CMask cMask;
    IdVec_t cIds;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


inline Archetype::Archetype(IdList_t initList) { addComponent(initList); }

template <typename... CTypes>
inline Archetype& Archetype::addComponent() { return addComponent(IdOfL<CTypes...>()); }

inline Archetype& Archetype::addComponent(IdList_t ids)
{
    for (auto id : ids)
        addComponent(id);
    return *this;
}

inline Archetype& Archetype::addComponent(CId_t id)
{
    if (!cMask.get(id)) {
        cMask.set(id);
        cIds.push_back(id);
    }
    return *this;
}

template <typename... CTypes>
inline Archetype& Archetype::removeComponent() { return removeComponent(IdOfL<CTypes...>()); }

inline Archetype& Archetype::removeComponent(IdList_t ids)
{
    for (auto id : ids)
        removeComponent(id);
    return *this;
}

inline Archetype& Archetype::removeComponent(CId_t id)
{
    if (has(id)) {
        cMask.unset(id);
        cIds.erase(std::find_if(cIds.begin(), cIds.end(), [id](auto const& cId) { return cId == id; }));
    }
    return *this;
}

inline bool Archetype::hasAllOf(IdList_t ids) const { return cMask.contains(ids); }

inline bool Archetype::hasAnyOf(IdList_t ids) const { return cMask.hasCommon(ids); }

inline bool Archetype::has(CId_t id) const { return cMask.get(id); }

inline Archetype::IdVec_t const& Archetype::getCIds() const { return cIds; }

inline CMask const& Archetype::getMask() const { return cMask; }

} // namespace epp

#endif // EPP_ARCHETYPE_H