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

    template<class ComponentT>
    inline static ComponentID RegisterComponent()
    {
        static_assert(std::is_base_of_v<Component, ComponentT>);
        GetCPoolsFactory().push_back(std::make_unique<CPool<ComponentT>>);
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
    inline static ComponentID ID = RegisterComponent<T>();

    friend class Archetype;
};

} // namespace epp

#endif // COMPONENTUTILITY_H