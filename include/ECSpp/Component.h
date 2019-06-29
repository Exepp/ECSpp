#ifndef COMPONENT_H
#define COMPONENT_H

#include <cstdlib>
#include <type_traits>

namespace epp
{

using CTypeId_t = size_t;

inline const size_t unidentifiedId = size_t(-1);

struct Component
{
    virtual ~Component() = default;

private:
    static CTypeId_t cTypeCounter;


    template<class T>
    friend CTypeId_t getCTypeId();
};

template<class T>
CTypeId_t getCTypeId()
{
    static_assert(std::is_base_of_v<Component, T>, "Only Component based types can request Id here");
    static const CTypeId_t cTypeId = Component::cTypeCounter++;
    return cTypeId;
}

template<class... CTypes>
void registerCTypes()
{
    static_assert((std::is_base_of_v<Component, CTypes> && ...), "Only Component based types can request Id here");
    (getCTypeId<CTypes>(), ...);
}

} // namespace epp

#endif // COMPONENT_H