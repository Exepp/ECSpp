#ifndef ENTITYCOLLECTION_H
#define ENTITYCOLLECTION_H

#include <ECSpp/Internal/CFilter.h>
#include <ECSpp/Internal/EntitySpawner.h>
#include <ECSpp/Utility/TuplePP.h>

namespace epp {


template <typename... CTypes>
class EntityCollection {
    using EntityPools_t = std::vector<Pool<Entity> const*>;

    template <typename T> // discard T
    struct PoolsPtrs_t : public std::vector<CPool*> {};

    using PoolsPtrsPack_t = TuplePP<PoolsPtrs_t<CTypes>...>;


    //////////////////////// <iterator>

    template <bool IsConst>
    class Iterator {
        using Collection_t = std::conditional_t<IsConst, EntityCollection const, EntityCollection>;

        using SIdx_t = std::size_t;

    public:
        Iterator(Collection_t& collection, bool end = false);


        template <typename T>
        T& getComponent() const;


        // incrementing an end iterator throws
        Iterator& operator++();

        // incrementing an end iterator throws
        Iterator operator++(int);

        Entity operator*() const;

        bool operator==(Iterator const& other) const;

        bool operator!=(Iterator const& other) const;

    private:
        bool isValid() const;

        void findNextSpawner();

    private:
        EntityPools_t* entityPools;

        PoolsPtrsPack_t* poolsPack;

        SIdx_t spawnerIdx;

        PoolIdx poolIdx;

        friend class EntityManager;
    };

    //////////////////////// </iterator>

public:
    using Iterator_t = Iterator<false>;

    using ConstIterator_t = Iterator<true>;

public:
    EntityCollection() : filter(IdOfL<CTypes...>(), {}) {}

    explicit EntityCollection(CMask const& unwanted) : filter(IdOfL<CTypes...>(), unwanted) {}

    EntityCollection(EntityCollection const&) = default;

    EntityCollection(EntityCollection&&) = default;

    EntityCollection& operator=(EntityCollection const&) = default;

    EntityCollection& operator=(EntityCollection&&) = default;


    Iterator_t begin();

    Iterator_t end();

    ConstIterator_t begin() const;

    ConstIterator_t end() const;


    const CFilter& getFilter() const { return filter; }

private:
    void addSpawnerIfMeetsRequirements(EntitySpawner& spawner)
    {
        if (filter & spawner.mask) {
            entityPools.push_back(&spawner.getEntities());
            (poolsPack.template get<PoolsPtrs_t<CTypes>>().push_back(&spawner.getPool(IdOf<CTypes>())), ...);
        }
    }

private:
    EntityPools_t entityPools; // one pool for every accepted spawner

    PoolsPtrsPack_t poolsPack; // for each component type, a vector of pools of that component,
                               // one pool for every accepted spawner

    CFilter filter;

    std::size_t checkedSpawnersCount = 0;


    friend class EntityManager;
};


#include <ECSpp/Internal/EntityCollection.inl>

} // namespace epp

#endif // ENTITYCOLLECTION_H