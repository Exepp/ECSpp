#pragma once
#include <vector>
#include "Utility/Pool.h"
#include "Component.h"

namespace epp
{

template<class T, bool IsConst>
class PArrayIterator
{
	using ThisIterator_t = PArrayIterator<T, IsConst>;

	using PoolPtrsHolder_t = std::vector<Pool<T>*>;

	using PoolsHolderIterator_t = std::conditional_t<IsConst, typename PoolPtrsHolder_t::const_iterator, typename PoolPtrsHolder_t::iterator>;

	using PoolIterator_t = std::conditional_t<IsConst, typename Pool<T>::ConstIterator_t, typename Pool<T>::Iterator_t>;

	using Pool_t = std::conditional_t<IsConst, const Pool<T>, Pool<T>>;

	using ValueRef_t = std::conditional_t<IsConst, const T&, T&>;

public:

	PArrayIterator(PoolsHolderIterator_t poolsHolderBegin, PoolsHolderIterator_t poolsHolderEnd);

	ValueRef_t& operator*() const;

	ThisIterator_t& operator++();

	ThisIterator_t operator++(int);

	bool operator==(const ThisIterator_t& other) const;

	bool operator!=(const ThisIterator_t& other) const;

private:

	void findValidPoolIt();

private:

	PoolsHolderIterator_t holderIt;

	const PoolsHolderIterator_t holderEndIt;

	PoolIterator_t poolIt;
};




template<class T>
class PArray
{
	using PoolPtr_t = Pool<T>*;

	using PoolPtrsHolder_t = std::vector<PoolPtr_t>;

	using Iterator_t = PArrayIterator<T, false>;

	using ConstIterator_t = PArrayIterator<T, true>;

public:

	PArray() = default;


	void addPool(Pool<T>& pool);


	Iterator_t begin();

	ConstIterator_t begin() const;

	Iterator_t end();

	ConstIterator_t end() const;


	size_t size() const;


	Pool<T>& operator[](size_t i);

	const Pool<T>& operator[](size_t i) const;

	
	const PoolPtrsHolder_t& getPools() const;


private:

	PoolPtrsHolder_t pools;
};

#include "PArray.inl"

}