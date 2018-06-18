#pragma once
#include <ECSpp/EntityManager/ASpawner.h>
#include <ECSpp/Utility/CFilter.h>
#include <ECSpp/Utility/TuplePP.h>

namespace epp
{
	template<class T>
	using PArray = std::vector<Pool<T>*>;


template<bool IsConst, class FirstType, class ...CTypes>
class ASpawnersPackIterator
{
	using ThisIterator_t = ASpawnersPackIterator<IsConst, FirstType, CTypes...>;

	using CProxyPack_t = std::conditional_t<IsConst, TuplePP<const FirstType&, const CTypes&...>, TuplePP<FirstType&, CTypes&...>>;

	using ASpawnersHolder_t = std::vector<ASpawner*>;

	using PoolArraysHolder_t = std::conditional_t<IsConst, const TuplePP<PArray<FirstType>, PArray<CTypes>...>, TuplePP<PArray<FirstType>, PArray<CTypes>...>>;

	using FirstTypePools_t = PArray<FirstType>;

public:

	ASpawnersPackIterator() = default;

	ASpawnersPackIterator(const ASpawnersHolder_t& spawners, PoolArraysHolder_t& poolArrays, size_t archetypeIndex = 0);


	template<class T>
	T& getComponent() const;

	EntityRef getERef() const;


	bool isValid() const;


	ThisIterator_t& operator++();

	ThisIterator_t operator++(int);

	CProxyPack_t operator*();

	bool operator==(const ThisIterator_t& other) const;

	bool operator!=(const ThisIterator_t& other) const;

private:

	void findValidIndices();

private:

	const ASpawnersHolder_t*  spawners;

	PoolArraysHolder_t* poolArrays;

	const FirstTypePools_t* firstTypePools;

	size_t archetypeIndex = 0;

	size_t entityIndex = 0;
};



struct ASpawnersPackInterace
{

	using ASpawnersHolder_t = std::vector<ASpawner*>;

public:


	ASpawnersPackInterace(CFilter filter) : filter(std::move(filter)) {}

	virtual ~ASpawnersPackInterace() = default;


	virtual void addASpawnerIfMeetsRequirements(ASpawner& aSpawner) = 0;

	void clear()
	{
		spawners.clear();
		filter.clear();
	}


	const CFilter& getFilter() const { return filter; }

protected:

	ASpawnersHolder_t spawners;

	CFilter filter;
};



template<class ...CTypes>
struct ASpawnersPack : public ASpawnersPackInterace
{
	static_assert(sizeof ...(CTypes), "Cannot create an empty ASpawnersPack");

	using ArraysHolder_t = TuplePP<PArray<CTypes>...>;


	using Iterator_t = ASpawnersPackIterator<false, CTypes...>;

	using ConstIterator_t = ASpawnersPackIterator<true, CTypes...>;

public:

	// it is up to caller to make sure, that filter matches the filter of CTypes
	ASpawnersPack(CFilter filter) : ASpawnersPackInterace(std::move(filter)) {}


	// adds a ASpawner if its archetype meets the requirements of the filer of this pack
	// its up to caller to make sure, that ASpawnersPack contains only unique spawners
	virtual void addASpawnerIfMeetsRequirements(ASpawner& aSpawner) override
	{
		if (aSpawner.getArchetype().meetsRequirementsOf(filter))
		{
			spawners.push_back(&aSpawner);
			(poolArrays.get<PArray<CTypes>>().push_back(&aSpawner.getPool<CTypes>(true)), ...);
		}
	}


	Iterator_t begin();

	ConstIterator_t begin() const;

	Iterator_t end();

	ConstIterator_t end() const;


	const ASpawnersHolder_t& getSpawners() const;

private:

	ArraysHolder_t poolArrays;
};


#include "ASpawnersPack.inl"

}