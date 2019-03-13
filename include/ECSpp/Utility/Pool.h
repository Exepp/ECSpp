#pragma once
#include <memory>
#include <new>
#include "Bitmask.h"
#include "DebugAssert.h"

namespace epp
{

template<class T>
class Pool;

template<class T, bool IsConst>
class PoolIterator
{
	using ThisIterator_t = PoolIterator<T, IsConst>;

public:

	using iterator_category = std::forward_iterator_tag;
	using value_type = T;
	using difference_type = ptrdiff_t;
	using pointer = std::conditional_t<IsConst, const T*, T*>;
	using reference = std::conditional_t<IsConst, const T&, T&>;

public:

	PoolIterator() = default;

	PoolIterator(pointer ptr) : ptr(ptr) {}


	reference operator*() const;

	pointer operator->() const;


	ThisIterator_t& operator++();

	ThisIterator_t operator++(int);

	ThisIterator_t& operator--();

	ThisIterator_t operator--(int);


	ThisIterator_t& operator+=(difference_type n);

	ThisIterator_t& operator-=(difference_type n);


	ThisIterator_t operator+(difference_type n) const;

	ThisIterator_t operator-(difference_type n) const;

	difference_type operator-(const ThisIterator_t& rhs) const;

	
	reference operator[](size_t i);


	bool operator<(const ThisIterator_t& rhs) const;

	bool operator<=(const ThisIterator_t& rhs) const;

	bool operator>(const ThisIterator_t& rhs) const;

	bool operator>=(const ThisIterator_t& rhs) const;

	bool operator==(const ThisIterator_t& rhs) const;

	bool operator!=(const ThisIterator_t& rhs) const;

private:

	pointer ptr;
};





template<class T>
class Pool
{
	using ThisPool_t = Pool<T>;

public:

	using Iterator_t = PoolIterator<T, false>;

	using ConstIterator_t = PoolIterator<T, true>;


public:

	Pool() = default;

	Pool(const ThisPool_t&) = delete;

	Pool(ThisPool_t&& rhs);

	~Pool();

	ThisPool_t& operator=(const ThisPool_t&) = delete;

	ThisPool_t& operator=(ThisPool_t&& rhs);


	template<class ...U>
	T& alloc(U&&... arg);

	template<class ...U>
	Pool<T>::Iterator_t allocN(size_t n, const U&... arg);


	void free(size_t i);


	void reserve(size_t n);

	void clear();


	std::unique_ptr<ThisPool_t> makeEmptyCopy() const;


	T& front();

	const T& front() const;

	T& back();

	const T& back() const;


	Iterator_t begin();

	ConstIterator_t begin() const;

	Iterator_t end();

	ConstIterator_t end() const;


	size_t getSize() const;

	size_t getReserved() const;


	T& operator[](size_t i);

	const T& operator[](size_t i) const;


public:

	static const size_t DefaultReserveSize = 8;

	static const size_t SizeMultiplier = 2;

private:

	T * memory = nullptr;

	size_t inUseCount = 0;

	size_t reserved = 0;
};

#include "Pool.inl"

}