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
    /// Creates a default (empty) archetype
    Archetype() = default;


    /// Creates an archetype with the components from the list
    /**
     * @param initList A List of ComponentIds returned from CMetadata::Id (or IdOf) function
     */
    explicit Archetype(IdList_t initList);


    /// Adds a given components to the archetype
    /**
     * @tparam CTypes A pack of any types
     * @returns A reference to this object
     */
    template <typename... CTypes>
    Archetype& addComponent();


    /// Adds a given components to the archetype
    /**
     * @param ids A List of ComponentIds returned from CMetadata::Id (or IdOf) function
     * @returns A reference to this object
     */
    Archetype& addComponent(IdList_t ids);


    /// Adds a given component to the archetype
    /**
     * @param id A ComponentId returned from CMetadata::Id (or IdOf) function
     * @returns A reference to this object
     */
    Archetype& addComponent(CId_t id);


    /// Removes a given components from the archetype
    /**
     * @tparam CTypes A pack of any types
     * @returns A reference to this object
     */
    template <typename... CTypes>
    Archetype& removeComponent();


    /// Removes a given components from the archetype
    /**
     * @param ids A List of ComponentIds returned from CMetadata::Id (or IdOf) function
     * @returns A reference to this object
     */
    Archetype& removeComponent(IdList_t ids);


    /// Removes a given component from the archetype
    /**
     * @param id A ComponentId returned from CMetadata::Id (or IdOf) function
     * @returns A reference to this object
     */
    Archetype& removeComponent(CId_t id);


    /// Returns whether the archetype contains all of a given components
    /**
     * @param ids A List of ComponentIds returned from CMetadata::Id (or IdOf) function
     * @returns True if the archetype contains every component from the list, false otherwise
     */
    bool hasAllOf(IdList_t ids) const;


    /// Returns whether the archetype contains any of a given components
    /**
     * @param ids A List of ComponentIds returned from CMetadata::Id (or IdOf) function
     * @returns True if the archetype contains at least one of the components from the list, false otherwise
     */
    bool hasAnyOf(IdList_t ids) const;


    /// Returns whether the archetype contains a given component
    /**
     * @param id A ComponentId returned from CMetadata::Id (or IdOf) function
     * @returns True if the archetype contains the component, false otherwise
     */
    bool has(CId_t id) const;


    /// Returns the CMask representation of the archetype
    /**
     * @returns The CMask representing a set of components contained in the archetype
     */
    CMask const& getMask() const;


    /// Returns the vector of components that the archetype containts
    /**
     * @returns The vector of the archetype's components 
     */
    IdVec_t const& getCIds() const;

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