#pragma once
#include "CGroup/CGroup.h"
#include "EntityManager/EntityManager.h"

namespace epp
{

using STypeId_t = size_t;

class System
{
public:

	virtual ~System() = default;

	virtual void init(EntityManager& entityManager) = 0;

	virtual void update(EntityManager& entityManager, float dt) = 0;

private:

	static STypeId_t typeCounter;

	template<class T>
	friend STypeId_t getSTypeId();
};


template<class T>
inline STypeId_t getSTypeId()
{
	static_assert(std::is_base_of_v<System, T>, "Only System based types can request Id here");
	static STypeId_t id = System::typeCounter++;
	return id;
}

}