#ifndef EPP_COMPONENT_H
#define EPP_COMPONENT_H

#include <ECSpp/utility/Assert.h>
#include <ECSpp/utility/IndexType.h>
#include <cstdint>
#include <vector>

namespace epp {

using ComponentId = IndexType<0, std::uint16_t>;


class CMetadata {
    using DefCstrFnPtr_t = void* (*)(void*);
    using MoveCstrFnPtr_t = void* (*)(void* dest, void* src);
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
    template <typename CType>
    static ComponentId Id()
    {
        static ComponentId id = RegisterComponent<CType>();
        return id;
    }

    template <typename... Comps>
    static void Register()
    {
        int i = 0;
        // this code must be present also for a release version, so assert version A (always)
        EPP_ASSERTA(!Registered);
        EPP_ASSERTA(((Id<Comps>().value == i++) && ...));
        Registered = i;
    }

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
        data.defaultConstructor = [](void* mem) -> void* { return new (mem) CType(); };
        data.moveConstructor = [](void* dest, void* src) -> void* { return new (dest) CType(std::move(*static_cast<CType*>(src))); };
        data.destructor = [](void* mem) { static_cast<CType*>(mem)->~CType(); };
        data.size = sizeof(CType);
        data.alignment = alignof(CType);
        data.cId = ComponentId(MetadataVec.size());
        MetadataVec.push_back(data);
        return data.cId;
    }

public:
    static constexpr std::size_t const MaxRegisteredComponents = 64; // 8 * sizeof(std::vector<uint64_t>);
    static_assert(MaxRegisteredComponents > 0);

private:
    inline static bool Registered = false;
    inline static MetadataVec_t MetadataVec;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template <typename T>
inline ComponentId IdOf()
{
    return CMetadata::Id<T>();
}

template <typename... Comps>
inline std::initializer_list<ComponentId> IdOfL()
{
    static std::initializer_list<ComponentId> list = { IdOf<Comps>()... };
    return list;
}

template <typename C1, typename C2, typename... CRest>
inline std::initializer_list<ComponentId> IdOf()
{
    return IdOfL<C1, C2, CRest...>();
}

} // namespace epp

#endif // EPP_COMPONENT_H