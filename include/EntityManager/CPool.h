#pragma once
#include "Component.h"
#include "Utility/Pool.h"

namespace epp
{

// interface of a component pool 
struct CPoolInterface
{
	~CPoolInterface() = default;

	virtual Component& alloc() = 0;

	virtual Component& alloc(const Component& arg) = 0;

	virtual Component& alloc(Component&& arg) = 0;

	virtual void allocN(size_t n) = 0;


	virtual void free(size_t i) = 0;


	virtual void reserve(size_t n) = 0;

	virtual void clear() = 0;


	virtual std::unique_ptr<CPoolInterface> makeEmptyCopy() const = 0;


	virtual size_t getSize() const = 0;

	virtual size_t getReserved() const = 0;


	virtual Component& operator[](size_t i) = 0;

	virtual const Component& operator[](size_t i) const = 0;
};



// component pool
template<class T>
struct CPool : public CPoolInterface
{
	static_assert(std::is_base_of_v<Component, T>, "Use only component types in CPools");
	
	using ThisPool_t = CPool<T>;

	using Pool_t = Pool<T>;


	using Iterator_t = typename Pool_t::Iterator_t;

	using ConstIterator_t = typename Pool_t::ConstIterator_t;


	virtual Component& alloc() override;

	virtual Component& alloc(const Component& arg) override;

	virtual Component& alloc(Component&& arg) override;

	virtual void allocN(size_t n) override;


	virtual void free(size_t i) override;


	virtual void reserve(size_t n) override;

	virtual void clear() override;


	virtual std::unique_ptr<CPoolInterface> makeEmptyCopy() const override;


	virtual size_t getSize() const override;

	virtual size_t getReserved() const override;


	virtual Component& operator[](size_t i) override;

	virtual const Component& operator[](size_t i) const override;


	Pool_t pool;
};

#include "CPool.inl"

}