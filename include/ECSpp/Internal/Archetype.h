#ifndef ARCHETYPE_H
#define ARCHETYPE_H

#include <ECSpp/Internal/CFilter.h>
#include <algorithm>
#include <memory>

namespace epp {

class Archetype {
    using CIdVec_t = std::vector<ComponentId>;

    using IdList_t = std::initializer_list<ComponentId>;

public:
    Archetype() = default;

    explicit Archetype(IdList_t initList);

    // resets to an empty archetype (with no set components)
    void reset();


    template <typename... CTypes>
    Archetype& addComponent();

    Archetype& addComponent(IdList_t ids);

    Archetype& addComponent(ComponentId id);

    template <typename... CTypes>
    Archetype& removeComponent();

    Archetype& removeComponent(IdList_t ids);

    Archetype& removeComponent(ComponentId id);


    bool hasAllOf(IdList_t ids) const;

    bool hasAnyOf(IdList_t ids) const;

    bool has(ComponentId id) const;


    CIdVec_t const& getCIds() const { return cIds; }

    CMask const& getMask() const { return cMask; }

private:
    CMask cMask;

    CIdVec_t cIds;
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