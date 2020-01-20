#ifndef COMPONENT_H
#define COMPONENT_H

#include <ECSpp/Utility/Assert.h>
#include <cstdint>
#include <memory>
#include <vector>

namespace epp {

struct ComponentId {
    using Val_t = std::uint16_t;

    ComponentId() = default;
    explicit ComponentId(Val_t val) : value(val) {}

    bool operator==(ComponentId const& rhs) const { return value == rhs.value; }
    bool operator!=(ComponentId const& rhs) const { return value != rhs.value; }
    bool operator<(ComponentId const& rhs) const { return value < rhs.value; }
    bool operator>(ComponentId const& rhs) const { return value > rhs.value; }
    bool operator<=(ComponentId const& rhs) const { return value <= rhs.value; }
    bool operator>=(ComponentId const& rhs) const { return value >= rhs.value; }

    constexpr static Val_t const BadValue = std::numeric_limits<Val_t>::max();

    Val_t value = BadValue;
};


struct CMetadata {
private:
    using DefCstrFnPtr_t = void* (*)(void*);
    using MoveCstrFnPtr_t = void* (*)(void* dest, void* src);
    using DestrFnPtr_t = void (*)(void*);
    using MetadataVec_t = std::vector<CMetadata>;

public:
    DefCstrFnPtr_t defaultConstructor;
    MoveCstrFnPtr_t moveConstructor;
    DestrFnPtr_t destructor;

    std::uint32_t size;
    std::uint16_t alignment;

public:
    template <class CType>
    static ComponentId Id()
    {
        static ComponentId id = RegisterComponent<CType>();
        EPP_ASSERTA(id.value < MaxRegisteredComponents);
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

    inline static CMetadata const& GetData(ComponentId id)
    {
        return MetadataVec[id.value];
    }

private:
    template <class CType>
    static ComponentId RegisterComponent()
    {
        static_assert(std::is_default_constructible_v<CType>);
        static_assert(std::is_move_constructible_v<CType>);

        EPP_ASSERTA(!CMetadata::Registered); // if CMetadata::Register was used
                                             // further registration is not allowed

        CMetadata data;
        data.defaultConstructor = [](void* mem) -> void* { return new (mem) CType(); };
        data.moveConstructor = [](void* dest, void* src) -> void* { return new (dest) CType(std::move(*static_cast<CType*>(src))); };
        data.destructor = [](void* mem) { static_cast<CType*>(mem)->~CType(); };
        data.size = sizeof(CType);
        data.alignment = alignof(CType);
        MetadataVec.push_back(data);
        return ComponentId(MetadataVec.size() - 1);
    }

public:
    static constexpr std::size_t const MaxRegisteredComponents = 64; // 8 * sizeof(std::vector<uint64_t>);
    static_assert(MaxRegisteredComponents > 0);

private:
    inline static bool Registered = false;
    inline static MetadataVec_t MetadataVec;
};


template <typename T>
inline decltype(auto) IdOf()
{
    return CMetadata::Id<T>();
}

template <typename... Comps>
inline decltype(auto) IdOfL()
{
    static std::initializer_list<ComponentId> list = { IdOf<Comps>()... };
    return list;
}

template <typename C1, typename C2, typename... CRest>
inline decltype(auto) IdOf()
{
    return IdOfL<C1, C2, CRest...>();
}

} // namespace epp

#endif // COMPONENT_H