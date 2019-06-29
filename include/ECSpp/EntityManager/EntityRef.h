#ifndef ENTITYREF_H
#define ENTITYREF_H

#include <ECSpp/Component.h>
#include <ECSpp/EntityManager/Archetype.h>
#include <memory>

namespace epp
{
class ASpawner;


using EntityId_t = size_t;


struct Entity : public std::enable_shared_from_this<Entity>
{
    Entity(ASpawner* originSpawner, EntityId_t id);

    void invalidate();


    ASpawner* originSpawner = nullptr;

    EntityId_t id = unidentifiedId;

    bool alive = false;
};


using EPtr_t = std::shared_ptr<Entity>;


class EntityRef
{
private:
    EntityRef(const EPtr_t& entity);

public:
    EntityRef() = default;


    void die();


    bool isValid() const;

    bool isAlive() const;


    template<class... CTypes>
    bool hasComponent() const;

    bool hasComponent(CTypeId_t id) const;

    // performs no validation check (may call functions on a nullptr), call isValid by yourself before
    template<class... CTypes>
    bool hasComponent_noCheck() const;

    // performs no validation check (may call functions on a nullptr), call isValid by yourself before
    bool hasComponent_noCheck(CTypeId_t id) const;


    Component* getComponent(size_t cTypeId);

    const Component* getComponent(size_t cTypeId) const;

    Component* getComponent_noCheck(size_t cTypeId);

    const Component* getComponent_noCheck(size_t cTypeId) const;


    // returns nullptr when entity does not own given component, or reference is invalid
    template<class T>
    T* getComponent();

    template<class T>
    const T* getComponent() const;

    template<class T>
    T* getComponent_noCheck();

    template<class T>
    const T* getComponent_noCheck() const;


    ASpawner const* getOriginSpawner() const;


    bool operator==(const EntityRef& rhs) const;

    bool operator!=(const EntityRef& rhs) const;

private:
    EPtr_t entity;

    friend ASpawner;
};


template<class... CTypes>
inline bool EntityRef::hasComponent() const
{
    return (hasComponent(getCTypeId<CTypes>()) && ...);
}

template<class... CTypes>
inline bool EntityRef::hasComponent_noCheck() const
{
    return (hasComponent_noCheck(getCTypeId<CTypes>()) && ...);
}

template<class T>
inline T* EntityRef::getComponent()
{
    return (T*)getComponent(getCTypeId<T>());
}

template<class T>
inline const T* EntityRef::getComponent() const
{
    return (T*)getComponent(getCTypeId<T>());
}

template<class T>
inline T* EntityRef::getComponent_noCheck()
{
    return (T*)getComponent_noCheck(getCTypeId<T>());
}

template<class T>
inline const T* EntityRef::getComponent_noCheck() const
{
    return (T*)getComponent_noCheck(getCTypeId<T>());
}

} // namespace epp

#endif // ENTITYREF_H