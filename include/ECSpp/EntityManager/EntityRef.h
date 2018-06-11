#pragma once
#include <memory>
#include <ECSpp/Component.h>
#include <ECSpp/EntityManager/Archetype.h>

namespace epp
{

class ASpawner;

using EntityId_t = size_t;


class EntityRef : public std::enable_shared_from_this<EntityRef>
{
public:

	EntityRef() = default;

	EntityRef(ASpawner* spawnerPtr, EntityId_t id, bool living);


	void die();


	bool isValid() const;

	bool isAlive() const;


	template<class ...CTypes>
	bool hasComponent() const;

	bool hasComponent(CTypeId_t id) const;

	// performs no validation check (may call functions on a nullptr), call isValid by yourself before
	template<class ...CTypes>
	bool hasComponent_noCheck() const;

	// performs no validation check (may call functions on a nullptr), call isValid by yourself before
	bool hasComponent_noCheck(CTypeId_t id) const;


	Component* getComponent(size_t cTypeId);

	const Component* getComponent(size_t cTypeId) const;

	Component* getComponent_NoCheck(size_t cTypeId);

	const Component* getComponent_NoCheck(size_t cTypeId) const;


	// returns nullptr when entity does not own given component, or reference is invalid
	template<class T>
	T* getComponent();

	template<class T>
	const T* getComponent() const;

	template<class T>
	T* getComponent_NoCheck();

	template<class T>
	const T* getComponent_NoCheck() const;


	ASpawner const * getOriginSpawner() const;


	bool operator==(const EntityRef& rhs) const;

	bool operator!=(const EntityRef& rhs) const;

private:

	void invalidate();

private:

	ASpawner* originSpawner = nullptr;

	EntityId_t id = epp::unidentifiedId;

	bool alive = false;


	friend ASpawner;
};


using ERefPtr_t = std::shared_ptr<EntityRef>;



template<class ...CTypes>
inline bool EntityRef::hasComponent() const
{
	return (hasComponent(getCTypeId<CTypes>()) && ...);
}

template<class ...CTypes>
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
inline T * EntityRef::getComponent_NoCheck()
{
	return (T*)getComponent_NoCheck(getCTypeId<T>());
}

template<class T>
inline const T * EntityRef::getComponent_NoCheck() const
{
	return (T*)getComponent_NoCheck(getCTypeId<T>());
}

}