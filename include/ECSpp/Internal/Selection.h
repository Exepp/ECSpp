#ifndef SELECTION_H
#define SELECTION_H

#include <ECSpp/Internal/CMask.h>
#include <ECSpp/Internal/EntitySpawner.h>
#include <ECSpp/Utility/TuplePP.h>

namespace epp {

template <typename... CTypes>
class Selection;

template <bool IsConst, typename T>
using CondConstType = std::conditional_t<IsConst, T const, T>;

template <bool _HasTypes, typename... CTypes> // false version
struct SelectionBase {

    template <bool IsConst>
    struct IteratorBase {
        using Selection_t = CondConstType<IsConst, Selection<>>;
        IteratorBase(Selection_t&) {}
    };
};

template <typename... CTypes> // true version
struct SelectionBase<true, CTypes...> {
    template <typename T> // discard T
    struct PoolsPtrs_t : public std::vector<CPool*> {};

    using PoolsPtrsPack_t = TuplePP<PoolsPtrs_t<CTypes>...>;

    template <bool IsConst>
    struct IteratorBase {
        using Selection_t = CondConstType<IsConst, Selection<CTypes...>>;
        IteratorBase(Selection_t& sel) : poolsPack(&sel.poolsPack) {}
        CondConstType<IsConst, PoolsPtrsPack_t>* poolsPack;
    };

    PoolsPtrsPack_t poolsPack; // for each component type, a vector of pools of that component,
                               // one pool for every accepted spawner
};


template <typename... CTypes>
class Selection : SelectionBase<(sizeof...(CTypes) > 0), CTypes...> {
    using Base_t = SelectionBase<(sizeof...(CTypes) > 0), CTypes...>;
    using EntityPools_t = std::vector<Pool<Entity> const*>;

    //////////////////////// <iterator>

    template <bool IsConst>
    class Iterator : Base_t::template IteratorBase<IsConst> {
        using ItBase_t = typename Base_t::template IteratorBase<IsConst>;
        using Selection_t = typename ItBase_t::Selection_t;
        using SIdx_t = std::uint64_t;
        using PIdx_t = SIdx_t; // compiles better (at least on my machine) with 64 bits

        constexpr static SIdx_t const EndValue = std::numeric_limits<SIdx_t>::max();

    public:
        Iterator(Selection_t& sel, bool end = false);

        template <typename T>
        std::enable_if_t<(sizeof...(CTypes) > 0), CondConstType<IsConst, T>>&
        getComponent() const;

        Iterator& operator++();                   // incrementing an end iterator throws
        Iterator operator++(int);                 // incrementing an end iterator throws
        Iterator& operator+=(std::size_t offset); /**  Returns an iterator to the entity that would
                                                       be reached after "offset" increments, or end */
        Iterator operator+(std::size_t offset) const;
        Entity operator*() const;
        bool operator==(Iterator const& other) const;
        bool operator!=(Iterator const& other) const;

    private:
        bool isValid() const;
        void findNextSpawner();
        auto sizeOfCurrentSpawner() const;
        auto numOfSpawners() const;

    private:
        EntityPools_t const* entityPools;
        PIdx_t poolIdx;
        SIdx_t spawnerIdx;

        friend class EntityManager;
    };

    //////////////////////// </iterator>

public:
    using Iterator_t = Iterator<false>;

    using ConstIterator_t = Iterator<true>;

public:
    Selection() : wantedMask(IdOfL<CTypes...>()) {}

    explicit Selection(CMask unwanted) : wantedMask(IdOfL<CTypes...>()),
                                         unwantedMask(unwanted.removeCommon(wantedMask)) {}

    template <typename Func>
    void forEach(Func func)
    {
        for (Iterator_t it = begin(), itEnd = end(); it != itEnd; ++it)
            func(it.template getComponent<CTypes>()...);
    }

    Iterator_t begin();
    Iterator_t end();
    ConstIterator_t begin() const;
    ConstIterator_t end() const;

    CMask const& getWanted() const { return wantedMask; }
    CMask const& getUnwanted() const { return unwantedMask; }

private:
    void addSpawnerIfMeetsRequirements(EntitySpawner& spawner);

private:
    CMask const wantedMask;    // must be declared before unwanted
    CMask const unwantedMask;  // if wanted & unwated (common part) != 0, then unwanted = unwanted \ (unwanted & wanted)
    EntityPools_t entityPools; // one pool for every accepted spawner
    std::size_t checkedSpawnersNum = 0;


    friend SelectionBase<true, CTypes...>;

    friend class EntityManager;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template <typename... CTypes>
template <bool IsConst>
inline Selection<CTypes...>::Iterator<IsConst>::Iterator(Selection_t& sel, bool end)
    : ItBase_t(sel),
      entityPools(&sel.entityPools),
      spawnerIdx(end ? EndValue : 0)
{
    findNextSpawner();
}

template <typename... CTypes>
template <bool IsConst>
template <typename T>
inline std::enable_if_t<(sizeof...(CTypes) > 0), CondConstType<IsConst, T>>&
Selection<CTypes...>::Iterator<IsConst>::getComponent() const
{
    // redundant (with tuple's assert), but a more verbose message
    static_assert(isTypeInPack<T, CTypes...>(), "This type is not specified in this sel's declaration");
    EPP_ASSERT(isValid());
    return *static_cast<T*>((*this->poolsPack->template get<typename Base_t::template PoolsPtrs_t<T>>()[spawnerIdx])[poolIdx]);
}

template <typename... CTypes>
template <bool IsConst>
inline typename Selection<CTypes...>::template Iterator<IsConst>&
Selection<CTypes...>::Iterator<IsConst>::operator++()
{
    EPP_ASSERT(spawnerIdx != EndValue);
    ++poolIdx;
    if (!isValid()) {
        ++spawnerIdx;
        findNextSpawner();
    }
    return *this;
}

template <typename... CTypes>
template <bool IsConst>
inline typename Selection<CTypes...>::template Iterator<IsConst>
Selection<CTypes...>::Iterator<IsConst>::operator++(int)
{
    Iterator result = *this;
    ++(*this);
    return result;
}

template <typename... CTypes>
template <bool IsConst>
inline typename Selection<CTypes...>::template Iterator<IsConst>&
Selection<CTypes...>::Iterator<IsConst>::operator+=(std::size_t offset)
{
    poolIdx += offset;
    auto spawnersNum = numOfSpawners();
    for (; spawnerIdx < spawnersNum; ++spawnerIdx)
        if (auto size = sizeOfCurrentSpawner(); size <= poolIdx)
            poolIdx -= size;
        else
            return *this;
    spawnerIdx = EndValue;
    return *this;
}

template <typename... CTypes>
template <bool IsConst>
inline typename Selection<CTypes...>::template Iterator<IsConst>
Selection<CTypes...>::Iterator<IsConst>::operator+(std::size_t offset) const
{
    auto it = *this;
    return it += offset;
}

template <typename... CTypes>
template <bool IsConst>
inline Entity
    Selection<CTypes...>::Iterator<IsConst>::operator*() const
{
    EPP_ASSERT(isValid());
    return (*(*entityPools)[spawnerIdx]).data[poolIdx];
}

template <typename... CTypes>
template <bool IsConst>
inline bool
Selection<CTypes...>::Iterator<IsConst>::operator==(Iterator const& other) const
{
    return spawnerIdx == other.spawnerIdx && poolIdx == other.poolIdx;
}

template <typename... CTypes>
template <bool IsConst>
inline bool
Selection<CTypes...>::Iterator<IsConst>::operator!=(Iterator const& other) const
{
    return !(*this == other);
}

template <typename... CTypes>
template <bool IsConst>
inline void
Selection<CTypes...>::Iterator<IsConst>::findNextSpawner()
{
    poolIdx = 0;
    for (; spawnerIdx < numOfSpawners(); ++spawnerIdx)
        if (sizeOfCurrentSpawner())
            return;
    spawnerIdx = EndValue; // end
}

template <typename... CTypes>
template <bool IsConst>
inline bool
Selection<CTypes...>::Iterator<IsConst>::isValid() const
{
    EPP_ASSERT(spawnerIdx < numOfSpawners());
    return poolIdx < sizeOfCurrentSpawner();
}

template <typename... CTypes>
template <bool IsConst>
inline auto
Selection<CTypes...>::Iterator<IsConst>::sizeOfCurrentSpawner() const
{
    if constexpr (sizeof...(CTypes) > 0)
        return this->poolsPack->template get<0>()[spawnerIdx]->size();
    else
        return (*entityPools)[spawnerIdx]->data.size();
}

template <typename... CTypes>
template <bool IsConst>
inline auto
Selection<CTypes...>::Iterator<IsConst>::numOfSpawners() const
{
    if constexpr (sizeof...(CTypes) > 0)
        return this->poolsPack->template get<0>().size();
    else
        return entityPools->size();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename... CTypes>
inline typename Selection<CTypes...>::Iterator_t
Selection<CTypes...>::begin()
{
    return Iterator_t(*this);
}

template <typename... CTypes>
inline typename Selection<CTypes...>::Iterator_t
Selection<CTypes...>::end()
{
    return Iterator_t(*this, true);
}

template <typename... CTypes>
inline typename Selection<CTypes...>::ConstIterator_t
Selection<CTypes...>::begin() const
{
    return ConstIterator_t(*this);
}

template <typename... CTypes>
inline typename Selection<CTypes...>::ConstIterator_t
Selection<CTypes...>::end() const
{
    return ConstIterator_t(*this, true);
}

template <typename... CTypes>
inline void Selection<CTypes...>::addSpawnerIfMeetsRequirements(EntitySpawner& spawner)
{
    if (spawner.mask.contains(wantedMask) && !spawner.mask.hasCommon(unwantedMask)) {
        entityPools.push_back(&spawner.getEntities());
        if constexpr (sizeof...(CTypes) > 0)
            (this->poolsPack.template get<typename Base_t::template PoolsPtrs_t<CTypes>>().push_back(&spawner.getPool(IdOf<CTypes>())), ...);
    }
}


} // namespace epp

#endif // SELECTION_H