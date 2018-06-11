#pragma once
#include "Archetype.h"
#include "EntityRef.h"

namespace epp
{

class ASpawner
{
public:
	
	using ERefPtrPool_t = Pool<ERefPtr_t>;


	struct EntitiesData
	{
		EntitiesData() = default;

		EntitiesData(Archetype archetype) : archetype(std::move(archetype)) {}

		Archetype archetype;

		ERefPtrPool_t eRefs;
	};


	using ERefPtrPoolIterator_t = Pool<ERefPtr_t>::Iterator_t;

	using ERefPtrPoolIteratorPair_t = std::pair<ERefPtrPoolIterator_t, ERefPtrPoolIterator_t>;

public:

	explicit ASpawner(Archetype archetype);

	~ASpawner();

	ASpawner(const ASpawner&) = default;

	ASpawner(ASpawner&&) = default;

	ASpawner& operator=(const ASpawner&) = delete;

	ASpawner& operator=(ASpawner&&) = default;


	const ERefPtr_t& spawn();

	ERefPtrPoolIteratorPair_t spawn(size_t n);


	void kill(EntityRef & ref);


	// kills alive and spawning entities
	void clear();

	
	void moveExistingEntityHere(EntityRef& eRef);


	// accepts recently added (since last registerNewEntities call) entities, ie. enables CGroup iterators to reach these entities
	void acceptSpawningEntities();

	
	Component& getComponent(CTypeId_t cId, EntityId_t entityId, bool alive);

	const Component& getComponent(CTypeId_t cId, EntityId_t entityId, bool alive) const;

	template<class T>
	T& getComponent(EntityId_t entityId, bool alive);

	template<class T>
	const T& getComponent(EntityId_t entityId, bool alive) const;


	CPoolInterface & getPool(CTypeId_t cId, bool alive);

	const CPoolInterface & getPool(CTypeId_t cId, bool alive) const;

	template<class T>
	Pool<T>& getPool(bool alive);

	template<class T>
	const Pool<T>& getPool(bool alive) const;



	size_t getSpawningEntitiesCount() const;

	size_t getAliveEntitiesCount() const;

	const Archetype& getArchetype() const;


	// returns ptr to ALIVE entity reference 
	const ERefPtr_t& operator[](EntityId_t id) const;

private:

	void referenceKill(EntitiesData& eData, size_t id);

private:

	EntitiesData aliveEntities;

	EntitiesData spawningEntities;
};




template<class T>
inline T & ASpawner::getComponent(EntityId_t entityId, bool alive)
{
	return static_cast<T&>(getComponent(getCTypeId<T>(), entityId, alive));
}

template<class T>
inline const T & ASpawner::getComponent(EntityId_t entityId, bool alive) const
{
	return static_cast<const T&>(getComponent(getCTypeId<T>(), entityId, alive));
}

template<class T>
inline Pool<T> & ASpawner::getPool(bool alive)
{
	return static_cast<CPool<T>&>(getPool(getCTypeId<T>(), alive)).pool;
}

template<class T>
inline const Pool<T> & ASpawner::getPool(bool alive) const
{
	return static_cast<CPool<T>&>(getPool(getCTypeId<T>(), alive)).pool;
}

}