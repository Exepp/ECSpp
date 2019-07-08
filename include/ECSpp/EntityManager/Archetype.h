#ifndef ARCHETYPE_H
#define ARCHETYPE_H

#include "CPool.h"

#include <ECSpp/Utility/CFilter.h>
#include <memory>
#include <unordered_map>

namespace epp
{

class Archetype
{
    using CPoolBase_t = CPoolInterface;

    using CPoolBasePtr_t = std::unique_ptr<CPoolBase_t>;

    using CPoolsHolder_t = std::unordered_map<CTypeId_t, CPoolBasePtr_t>;

public:
    Archetype() = default;

    Archetype(const Archetype& rhs);

    Archetype(Archetype&& rhs) = default;

    Archetype& operator=(const Archetype&);

    Archetype& operator=(Archetype&&) = default;


    // resets to an empty archetype (with no set components)
    void reset();


    bool meetsRequirementsOf(const CFilter& filter) const;


    // returns true when successfully added every given components
    template<class T1, class T2, class... CTypes>
    bool addComponent();

    template<class T>
    bool addComponent();


    // returns true when successfully removed given components
    template<class CType1, class... CTypes>
    bool removeComponent();

    template<class... IdTypes>
    bool removeComponent(CTypeId_t id1, CTypeId_t id2, IdTypes... ids);

    bool removeComponent(CTypeId_t id);


    template<class... CTypes>
    bool hasComponent() const;

    template<class... IdTypes>
    bool hasComponent(CTypeId_t id1, IdTypes... ids) const;

    const Bitmask& getCMask() const;

    size_t hash() const;

private:
    // clears pools from components
    void clear();

private:
    Bitmask cMask;

    CPoolsHolder_t cPools;


    mutable bool rehash = true;

    mutable size_t hashValue = 0;


    friend class ASpawner;

    friend struct std::hash<Archetype>;
};


template<class T1, class T2, class... CTypes>
inline bool Archetype::addComponent()
{
    return addComponent<T1>() & addComponent<T2>() & (1 & ... & addComponent<CTypes>());
}

template<class T>
inline bool Archetype::addComponent()
{
    size_t cTId = getCTypeId<T>();
    if (!cMask.get(cTId))
    {
        cMask.set(cTId);
        cPools[cTId] = std::make_unique<CPool<T>>();
        rehash       = true;
        return true;
    }
    return false;
}

template<class CType1, class... CTypes>
inline bool Archetype::removeComponent()
{
    return removeComponent(getCTypeId<CType1>(), getCTypeId<CTypes>()...);
}

template<class... IdTypes>
inline bool Archetype::removeComponent(CTypeId_t id1, CTypeId_t id2, IdTypes... ids)
{
    return removeComponent(id1) & removeComponent(id2) & (1 & ... & removeComponent(ids));
}

template<class... CTypes>
inline bool Archetype::hasComponent() const
{
    return hasComponent(getCTypeId<CTypes>()...);
}

template<class... IdTypes>
inline bool Archetype::hasComponent(CTypeId_t id1, IdTypes... ids) const
{
    return cMask.get(id1) && (cMask.get(ids) && ...);
}

template<class FirstType, class... CTypes>
inline const Archetype& makeArchetype()
{
    static Archetype arche;
    if (!arche.hasComponent<FirstType>())
    {
        arche.addComponent<FirstType, CTypes...>();
        arche.hash();
    }
    return arche;
}

} // namespace epp


namespace std
{
template<>
struct hash<epp::Archetype>
{
    size_t operator()(const epp::Archetype& arche) const
    {
        return hasher(arche.cMask);
    }
    hash<epp::Bitmask> hasher;
};
} // namespace std

#endif // ARCHETYPE_H