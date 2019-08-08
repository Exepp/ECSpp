#ifndef ECSWORLD_H
#define ECSWORLD_H

#include "System.h"

#include <algorithm>

namespace epp
{
struct ECSWorld
{

    using SystemPtr_t = std::unique_ptr<System>;

    using SystemPair_t = std::pair<STypeId_t, SystemPtr_t>;

    using SystemsHolder_t = std::vector<SystemPair_t>;

    struct SystemUnaryPredicate
    {
        SystemUnaryPredicate(STypeId_t id)
            : id(id)
        {}
        bool operator()(const SystemPair_t& system)
        {
            return system.first == id;
        }
        STypeId_t id = unidentifiedId;
    };

public:
    void update(float dt, bool catchUpTick = false);

    // order of making system defines the order of update
    template<class T, class... Args>
    T& makeSystem(Args&&... args);


    template<class T, class... Args>
    bool removeSystem();

    bool removeSystem(STypeId_t id);


    template<class T>
    bool hasSystem();

    bool hasSystem(STypeId_t id);


    template<class T>
    T& getSystem();

    template<class T>
    const T& getSystem() const;

public:
    EntityManager entityManager;

protected:
    SystemsHolder_t systems;
};


template<class T, class... Args>
inline T& ECSWorld::makeSystem(Args&&... args)
{
    auto found = std::find_if(systems.begin(), systems.end(), SystemUnaryPredicate(getSTypeId<T>()));
    ASSERT((found == systems.end()), "Making the same System twice. Returning the existing one")
    if (found == systems.end())
    {
        systems.emplace_back(std::make_pair(getSTypeId<T>(), std::make_unique<T>(std::forward<Args>(args)...)));
        systems.back().second->init(entityManager);
        return static_cast<T&>(*systems.back().second);
    }
    return static_cast<T&>(*found->second);
}

template<class T, class... Args>
bool ECSWorld::removeSystem()
{
    return removeSystem(getSTypeId<T>());
}

template<class T>
inline bool ECSWorld::hasSystem()
{
    return hasSystem(getSTypeId<T>());
}

template<class T>
inline T& ECSWorld::getSystem()
{
    auto found = std::find_if(systems.begin(), systems.end(), SystemUnaryPredicate(getSTypeId<T>()));
    EXC_ASSERT((found != systems.end()), std::out_of_range, "Getting unidentified system")
    return static_cast<T&>(*found->second);
}

template<class T>
inline const T& ECSWorld::getSystem() const
{
    return static_cast<const T&>(*systems[getSTypeId<T>()]);
}

} // namespace epp

#endif // ECSWORLD_H