#ifndef CGROUP_H
#define CGROUP_H

#include <ECSpp/EntityManager/EntitySpawner.h>
#include <ECSpp/Utility/BitFilter.h>
#include <ECSpp/Utility/TuplePP.h>
#include <deque>

namespace epp
{

class EntityCollectionBase
{
protected:
    using Spawners_t = std::vector<EntitySpawner const*>;

public:
    EntityCollectionBase(BitFilter filter)
        : filter(std::move(filter))
    {}

    virtual ~EntityCollectionBase() = default;

    const BitFilter& getFilter() const { return filter; }

private:
    void update(std::deque<EntitySpawner>& spawners)
    {
        for (; checkedSpawnersCount < spawners.size(); ++checkedSpawnersCount)
            addSpawnerIfMeetsRequirements(spawners[checkedSpawnersCount]);
    }

    // adds a spawner if its archetype meets the requirements of this Collection's filter
    virtual void addSpawnerIfMeetsRequirements(EntitySpawner const& spawner) = 0;

protected:
    Spawners_t spawners;

    BitFilter filter;

    std::size_t checkedSpawnersCount = 0;

    friend class EntityManager;
};


template<class... CTypes>
class EntityCollection : public EntityCollectionBase
{
    using ThisCollection_t = EntityCollection<CTypes...>;

    template<class T>
    using PoolPtrs_t = std::vector<Pool<T>*>;

    using PoolPtrsCollection_t = TuplePP<PoolPtrs_t<CTypes>...>;


    template<bool IsConst>
    class Iterator
    {
        using ThisIterator_t = Iterator<IsConst>;

        using Collection_t = std::conditional_t<IsConst, ThisCollection_t const, ThisCollection_t>;

        using Spawns_t = std::conditional_t<IsConst, Spawners_t const, Spawners_t>;

    public:
        Iterator(Collection_t& collection, std::size_t spawnerIdx = 0);


        template<class T>
        T& getComponent() const;

        Archetype const& getArchetype() const;


        bool isValid() const;


        ThisIterator_t& operator++();

        ThisIterator_t operator++(int);

        Entity operator*();

        bool operator==(ThisIterator_t const& other) const;

        bool operator!=(ThisIterator_t const& other) const;

    private:
        void findValidIndices();

    private:
        Collection_t& collection;

        std::size_t spawnerIdx;

        PoolIdx poolIdx;
    };


    using Iterator_t = Iterator<false>;

    using ConstIterator_t = Iterator<true>;

public:
    EntityCollection(BitFilter filter)
        : EntityCollectionBase(std::move(filter))
    {}


    Iterator_t begin();

    Iterator_t end();

    ConstIterator_t begin() const;

    ConstIterator_t end() const;

private:
    virtual void addSpawnerIfMeetsRequirements(EntitySpawner const& spawner) override
    {
        if (filter & spawner.getArchetype().getMask())
        {
            spawners.push_back(&spawner);
            (poolsCollection.template get<PoolPtrs_t<CTypes>>().push_back(&spawner.getPool<CTypes>()), ...);
        }
    }

private:
    PoolPtrsCollection_t poolsCollection;
};


#include <ECSpp/EntityManager/EntityCollection.inl>

} // namespace epp

#endif // CGROUP_H