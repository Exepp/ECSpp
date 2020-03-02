#ifndef EPP_COMPONENT_H
#define EPP_COMPONENT_H

#include <ECSpp/internal/utility/Assert.h>
#include <ECSpp/internal/utility/IndexType.h>
#include <cstdint>
#include <vector>

namespace epp {

using ComponentId = IndexType<0, std::uint8_t>;

/// A class responsible for gathering components' metadata used in CPools to construct,
/// move and destroy components without the type information
class CMetadata {
    using DefCstrFnPtr_t = void (*)(void*);
    using MoveCstrFnPtr_t = void (*)(void* dest, void* src);
    using DestrFnPtr_t = void (*)(void*);
    using MetadataVec_t = std::vector<CMetadata>;

public:
    DefCstrFnPtr_t defaultConstructor;
    MoveCstrFnPtr_t moveConstructor;
    DestrFnPtr_t destructor;
    std::uint32_t size;
    std::uint32_t alignment;
    ComponentId cId;

public:
    /// Registers a component type on first call and returns a unique id for that type
    /**
     * @tparam CType Type of component
     * @returns A unique id for that type
    */
    template <typename CType>
    static ComponentId Id()
    {
        static ComponentId id = RegisterComponent<CType>();
        return id;
    }

    /// Registers components in a consistent and specific order and locks registering
    /// (following attempts to register a component will throw in debug and release)
    /**
     * @tparam Comps A pack of types to register
     * @throws An AssertFailed exception if CMetadata::Register was already called once before or if any of the types was already registered with wrong id 
    */
    template <typename... Comps>
    static void Register()
    {
        int i = 0;
        // this code must be present also for a release version, so assert version A (always)
        EPP_ASSERTA(!Registered);
        EPP_ASSERTA(((Id<Comps>().value == i++) && ...));
        Registered = i;
    }

    /// Returns a copy of metadata associated with a given id
    /**
     * @param id Any id returned from the Metadata::Id function
    */
    static CMetadata GetData(ComponentId id)
    {
        return MetadataVec[id.value];
    }

private:
    template <typename CType>
    static ComponentId RegisterComponent()
    {
        static_assert(std::is_same_v<std::remove_pointer_t<std::decay_t<CType>>, CType>);
        static_assert(std::is_default_constructible_v<CType>);
        static_assert(std::is_move_constructible_v<CType>);

        EPP_ASSERTA(MetadataVec.size() < MaxRegisteredComponents);
        EPP_ASSERTA(!CMetadata::Registered); // if CMetadata::Register was used
                                             // further registration is not allowed
        CMetadata data;
        data.defaultConstructor = [](void* mem) { new (mem) CType(); };
        data.moveConstructor = [](void* dest, void* src) { new (dest) CType(std::move(*static_cast<CType*>(src))); };
        data.destructor = [](void* mem) { static_cast<CType*>(mem)->~CType(); };
        data.size = sizeof(CType);
        data.alignment = alignof(CType);
        data.cId = ComponentId(MetadataVec.size());
        MetadataVec.push_back(data);
        return data.cId;
    }

public:
    /// The maximum number of registered components
    /**
     * This value should be a multiple of 64 as it describes the number of bits in a CMask's bitset 
    */
    static constexpr std::size_t const MaxRegisteredComponents = 64;
    static_assert(MaxRegisteredComponents > 0 && (MaxRegisteredComponents & (64 - 1)) == 0);

private:
    inline static bool Registered = false;
    inline static MetadataVec_t MetadataVec;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/// A convenient function returning the ComponentId of a given type
/**
 * @tparam T Any type
 * @returns A unique Id of the type T
*/
template <typename T>
inline ComponentId IdOf()
{
    return CMetadata::Id<T>();
}

/// A convenient function returning the ComponentId list of a given types
/**
 * Use this function if you always need a list as a return value (for example passing a single ComponentId to the constructor of the Archetype class)
 * @tparam Comps A pack of any types
 * @returns A list of unique Ids for each of the types
*/
template <typename... Comps>
inline std::initializer_list<ComponentId> IdOfL()
{
    static std::initializer_list<ComponentId> list = { IdOf<Comps>()... };
    return list;
}

/// A convenient function returning the ComponentId list of a given types
/**
 * @tparam Comps A pack of any types
 * @returns A list of unique Ids for each of the types
*/
template <typename C1, typename C2, typename... CRest>
inline std::initializer_list<ComponentId> IdOf()
{
    return IdOfL<C1, C2, CRest...>();
}

} // namespace epp

#endif // EPP_COMPONENT_H