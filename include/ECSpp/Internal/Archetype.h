#ifndef ARCHETYPE_H
#define ARCHETYPE_H

#include <ECSpp/Internal/CFilter.h>
#include <algorithm>
#include <memory>

namespace epp {

class Archetype {
    using IdList_t = decltype(IdOfL<>());

    using CId_t = IdList_t::value_type;

    using IdVec_t = std::vector<CId_t>;

public:
    Archetype() = default;

    explicit Archetype(IdList_t initList);

    // resets to an empty archetype (with no set components)
    void reset();


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


    IdVec_t const& getCIds() const { return cIds; }

    CMask const& getMask() const { return cMask; }

private:
    CMask cMask;

    IdVec_t cIds;
};


template <typename... CTypes>
inline Archetype& Archetype::addComponent()
{
    return addComponent(IdOfL<CTypes...>());
}

template <typename... CTypes>
inline Archetype& Archetype::removeComponent()
{
    return removeComponent(IdOfL<CTypes...>());
}

} // namespace epp

#endif // ARCHETYPE_H