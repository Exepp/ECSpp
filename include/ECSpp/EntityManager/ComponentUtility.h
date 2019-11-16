#ifndef COMPONENTUTILITY_H
#define COMPONENTUTILITY_H

#include <ECSpp/EntityManager/CPool.h>
#include <cstdint>
#include <memory>
#include <vector>

namespace epp
{

using ComponentID = std::uint16_t;


class ComponentUtility
{
private:
    using CPoolBasePtr_t  = std::unique_ptr<CPoolBase>;
    using CPoolCreator_t  = CPoolBasePtr_t (*)();
    using CPoolsFactory_t = std::vector<CPoolCreator_t>;

    template<class T>
    static CPoolBasePtr_t Make_Unique_Pool()
    {
        return std::make_unique<CPool<T>>();
    }

    template<class ComponentT>
    inline static ComponentID RegisterComponent()
    {
        static_assert(std::is_base_of_v<Component, ComponentT>);
        CPoolCreator_t ptr = Make_Unique_Pool<ComponentT>;
        GetCPoolsFactory().push_back(ptr);
        return NextID();
    }

    static CPoolsFactory_t& GetCPoolsFactory()
    {
        static CPoolsFactory_t cPoolsfactory;
        return cPoolsfactory;
    }

    static ComponentID NextID()
    {
        static ComponentID typeCounter = 0;
        return typeCounter++;
    }

public:
    template<class T>
    inline static ComponentID const ID = RegisterComponent<T>();

    friend class Archetype;
};

template<class... Types>
inline std::initializer_list<ComponentID> IDOfComp()
{
    static std::initializer_list<ComponentID> list = { ComponentUtility::ID<Types>... };
    return list;
}

} // namespace epp

#endif // COMPONENTUTILITY_H