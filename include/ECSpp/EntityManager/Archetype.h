#ifndef ARCHETYPE_H
#define ARCHETYPE_H

#include <ECSpp/EntityManager/CPool.h>
#include <ECSpp/EntityManager/ComponentUtility.h>
#include <ECSpp/Utility/BitFilter.h>
#include <algorithm>
#include <memory>

namespace epp
{

class Archetype
{
    struct IdentifiedCPoolCreator
    {
        ComponentID                      id;
        ComponentUtility::CPoolCreator_t create;
    };


    using CPoolsCreators_t = std::vector<IdentifiedCPoolCreator>;

    using IDList_t = std::initializer_list<ComponentID>;

public:
    Archetype() = default;

    explicit Archetype(IDList_t initList);

    // resets to an empty archetype (with no set components)
    void reset();


    template<class... CTypes>
    void addComponent();

    void addComponent(IDList_t ids);

    void addComponent(ComponentID id);

    template<class... CTypes>
    void removeComponent();

    void removeComponent(IDList_t ids);

    void removeComponent(ComponentID id);

    template<class... CTypes>
    bool hasAllOf() const;

    bool hasAllOf(IDList_t ids) const;

    template<class... CTypes>
    bool hasAnyOf() const;

    bool hasAnyOf(IDList_t ids) const;

    bool has(ComponentID id) const;


    Bitmask const& getMask() const;

private:
    Bitmask cMask;

    CPoolsCreators_t creators;


    friend class EntitySpawner;
};


template<class... CTypes>
inline void Archetype::addComponent()
{
    addComponent({ ComponentUtility::ID<CTypes>... });
}

template<class... CTypes>
inline void Archetype::removeComponent()
{
    removeComponent({ ComponentUtility::ID<CTypes>... });
}

template<class... CTypes>
inline bool Archetype::hasAllOf() const
{
    return hasAllOf({ ComponentUtility::ID<CTypes>... });
}

template<class... CTypes>
inline bool Archetype::hasAnyOf() const
{
    return hasAnyOf({ ComponentUtility::ID<CTypes>... });
}


} // namespace epp

#endif // ARCHETYPE_H