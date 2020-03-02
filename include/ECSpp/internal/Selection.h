#ifndef EPP_SELECTION_H
#define EPP_SELECTION_H

#include <ECSpp/internal/EntityList.h>
#include <ECSpp/internal/EntitySpawner.h>
#include <ECSpp/internal/utility/TuplePP.h>

namespace epp {

template <typename... CTypes>
class Selection;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <bool IsConst, typename T>
using CondConstType = std::conditional_t<IsConst, std::add_const_t<T>, T>;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/// A helper class for conditional member variables
/** No-types version - no additional member variables */
template <bool _HasTypes, typename... CTypes> // false version
struct SelectionBase {
    template <bool IsConst>
    struct IteratorBase {
        using Selection_t = CondConstType<IsConst, Selection<>>;
        IteratorBase(Selection_t&) {}
    };
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// A specialization of helper class for conditional member variables
/** Version with types - adding member variables */
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/// A query to the EntityManager for entites that own a certain set of components
/**
 * Using iterators of this class you can iterate over every entity that ows at least all of the components of CTypes... types.
 * This class stores pointers to CPools and EntityPools of the EntitySpawners with
 * archetypes that matches the specified requirements (CTypes and unwanted mask)
 * @tparam CTypes A pack of component types used to select wanted entities
 */
template <typename... CTypes>
class Selection : SelectionBase<(sizeof...(CTypes) > 0), CTypes...> {
    using Base_t = SelectionBase<(sizeof...(CTypes) > 0), CTypes...>;
    using EntityPools_t = std::vector<Pool<Entity> const*>;
    using SpawnerIds_t = std::vector<SpawnerId>;

    //////////////////////// <iterator>

    /// Iterator of the Selection
    /**
     * Selection's iterator is invalid if it points to a location that would be impossible to get to with the
     * current state of an EntityManager and only incrementation, i.e. changing the state of an EntityManager may
     * invalidate iterators. 
     * @details Here are the operations that may invalidates the iterators:
     * - EntityManager::changeArchetype (has the overload for iterators that returns a next valid one)
     * - EntityManager::destroy (has the overload for iterators that returns a next valid one)
     * - EntityManager::clear
     * 
     * @tparam IsConst A boolean that specifies whether this iterator operates on const Selection, or non-const
     */
    template <bool IsConst>
    class Iterator : Base_t::template IteratorBase<IsConst> {
        using ItBase_t = typename Base_t::template IteratorBase<IsConst>;
        using Selection_t = typename ItBase_t::Selection_t;
        using SIdx_t = std::size_t;
        using PIdx_t = SIdx_t; // gcc optimizes better (at least on my machine) with 64 bits

        // so user can call getComponent<CType> for const and non-const types
        template <class T>
        using CondConstComp_t = CondConstType<IsConst || isTypeInPack<std::add_const_t<T>, CTypes...>(), T>;

        constexpr static SIdx_t const EndValue = std::numeric_limits<SIdx_t>::max();

    public:
        /**
         * @param sel Selection of which needed data about spawners will be extracted
         * @param end Should this iterator point to the end
         */
        Iterator(Selection_t& sel, bool end);


        /// Returns a reference to the component of type T that the current entity owns
        /**
         * @tparam T Type of the component
         * @returns A reference to the component
         * @throws (Debug only) Throws the AssertionFailed exception if the iterator is invalid or points to the end 
         */
        template <typename T>
        std::enable_if_t<(sizeof...(CTypes) > 0), CondConstComp_t<T>>&
        getComponent() const;


        /**
         * @returns A reference to this iterator
         * @throws (Debug only) Throws the AssertionFailed exception if the iterator points to the end 
         */
        Iterator& operator++();


        /**
         * @returns A copy of pre-increment version of this iterator
         * @throws (Debug only) Throws the AssertionFailed exception if the iterator points to the end 
         */
        Iterator operator++(int);


        /**
         * Applies "offset" increments reaching an entity or end
         * @returns A reference to this iterator
         */
        Iterator& operator+=(std::size_t offset);


        /**
         * @returns An iterator pointing to the entity that would be reached after "offset" increments or to the end
         */
        Iterator operator+(std::size_t offset) const;


        /**
         * @returns Entity that this iterator points to
         * @throws (Debug only) Throws the AssertionFailed exception if the iterator is invalid or points to the end 
         */
        Entity operator*() const;


        /**
         * @warning Both iterators must come from the same selection!
         * @returns True if both iterators points to the same place, false otherwise
         */
        bool operator==(Iterator const& other) const;


        /**
         * @warning Both iterators must come from the same selection!
         * @returns True if the iterators points to different places, false otherwise
         */
        bool operator!=(Iterator const& other) const;


        /// Jumps to an entity located at {sId, pIdx} or beyond
        /** 
         * Makes this iterator point to an entity located at {sId, pIdx} or if that location is not
         * covered by the selection that this iterator comes from, to a first entity that lies beyond 
         * and IS covered by the selection or to the end if there is no such an entity.
         * @note Iterator can only jump forward. Passing a location lies before this iterator will make it jump to the end
         * @param sId Id of a spawner
         * @param pIdx Index of the entity in a spawner with "sId" Id 
         */
        Iterator& jumpToOrBeyond(SpawnerId sId, PoolIdx pIdx);


        /// Jumps to an entity located at {entCell.spawnerId, entCell.poolIdx} or beyond
        /** 
         * Makes this iterator point to an entity located at {entCell.spawnerId, entCell.poolIdx} or if that
         * location is not covered by the selection that this iterator comes from, to a first entity that 
         * lies beyond and IS covered by the selection or to the end if there is no such an entity.
         * @note Iterator can only jump forward. Passing a data of an entity that lies before this iterator will make it jump to the end
         * @param entCell Data of an entity to jump to (or beyond)
        */
        Iterator& jumpToOrBeyond(EntityList::Cell::Occupied entCell);


        /**
         * @returns Id of the spawner that the iterator points to 
         */
        SpawnerId getSpawnerId() const;


        /**
         * @returns Index of the entity iterator points to
         */
        PoolIdx getPoolIdx() const;

    private:
        bool isValid() const;
        void findNextSpawner();
        auto sizeOfCurrentSpawner() const;
        auto numOfSpawners() const;

    private:
        EntityPools_t const* entityPools;
        PIdx_t poolIdx;
        SIdx_t spawnerIdx;
        SpawnerIds_t const* spawnerIds;

        friend class EntityManager;
    };

    //////////////////////// </iterator>

public:
    using Iterator_t = Iterator<false>;

    using ConstIterator_t = Iterator<true>;

public:
    /// Constructs a selection with a specified requirements
    /**
     * @param unwanted Mask of components the entities mustn't have.
     */
    explicit Selection(CMask unwanted = CMask());


    /// Calls func on each entity that this selection covers
    /**
     * @tparam Func A callable type that accepts Iterator_t const&
     * @param func A callable object that accepts Iterator_t const&
     */
    template <typename Func>
    void forEach(Func func);


    /** @returns An iterator to the first entity covered by this selection */
    Iterator_t begin();


    /** @returns An iterator pointing to the end (always valid)  */
    Iterator_t end();


    /** @copydoc Selection::begin()  */
    ConstIterator_t begin() const;


    /** @copydoc Selection::end()  */
    ConstIterator_t end() const;


    /** @returns Mask with wanted types of components (CTypes...) */
    CMask const& getWanted() const;


    /** @returns Mask with unwanted types of components passed in the selection's constructor*/
    CMask const& getUnwanted() const;


    /** @returns Number of entites that this selection covers */
    std::size_t countEntities() const;

private:
    void addSpawnerIfMeetsRequirements(EntitySpawner& spawner);

private:
    CMask const wantedMask;    // must be declared before unwanted
    CMask const unwantedMask;  // if wanted & unwated (common part) != 0, then unwanted = unwanted \ (unwanted & wanted)
    EntityPools_t entityPools; // one pool for every accepted spawner
    SpawnerIds_t spawnerIds;
    std::size_t checkedSpawnersNum = 0;


    friend SelectionBase<true, CTypes...>; // in used iterator
    friend class EntityManager;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template <typename... CTypes>
template <bool IsConst>
inline Selection<CTypes...>::Iterator<IsConst>::Iterator(Selection_t& sel, bool end)
    : ItBase_t(sel),
      entityPools(&sel.entityPools),
      spawnerIds(&sel.spawnerIds),
      spawnerIdx(end ? EndValue : 0)
{
    if (end)
        poolIdx = 0;
    else
        findNextSpawner(); // always sets poolIdx to 0
}

template <typename... CTypes>
template <bool IsConst>
template <typename T>
inline std::enable_if_t<(sizeof...(CTypes) > 0),
                        typename Selection<CTypes...>::template Iterator<IsConst>::template CondConstComp_t<T>>&
Selection<CTypes...>::Iterator<IsConst>::getComponent() const
{
    // redundant (with tuple's assert), but a more specific message
    static_assert(isTypeInPack<std::remove_const_t<T>, std::remove_const_t<CTypes>...>(),
                  "This type is not specified in the declaration of this selection");
    EPP_ASSERT(isValid());
    return *static_cast<CondConstComp_t<T>*>((*this->poolsPack->template get<typename Base_t::template PoolsPtrs_t<CondConstComp_t<T>>>()[spawnerIdx])[poolIdx]);
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
    poolIdx = 0;
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
inline typename Selection<CTypes...>::template Iterator<IsConst>&
Selection<CTypes...>::Iterator<IsConst>::jumpToOrBeyond(EntityList::Cell::Occupied entCell)
{
    return jumpToOrBeyond(entCell.spawnerId, entCell.poolIdx);
}

template <typename... CTypes>
template <bool IsConst>
inline typename Selection<CTypes...>::template Iterator<IsConst>&
Selection<CTypes...>::Iterator<IsConst>::jumpToOrBeyond(SpawnerId sId, PoolIdx pIdx)
{
    if (sId != SpawnerId(SpawnerId::BadValue)) {
        auto spawnersNum = numOfSpawners();
        for (; spawnerIdx < spawnersNum; ++spawnerIdx)
            if ((*spawnerIds)[spawnerIdx] == sId) { // dont use getSpawnerId to avoid additional check
                poolIdx = pIdx.value;
                return *this;
            }
    }
    spawnerIdx = EndValue;
    poolIdx = 0;
    return *this;
}

template <typename... CTypes>
template <bool IsConst>
inline SpawnerId
Selection<CTypes...>::Iterator<IsConst>::getSpawnerId() const
{
    return spawnerIdx != EndValue ? (*spawnerIds)[spawnerIdx] : SpawnerId();
}

template <typename... CTypes>
template <bool IsConst>
inline PoolIdx
Selection<CTypes...>::Iterator<IsConst>::getPoolIdx() const
{
    return PoolIdx(poolIdx);
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
Selection<CTypes...>::Selection(CMask unwanted) : wantedMask(IdOfL<std::remove_const_t<CTypes>...>()),
                                                  unwantedMask(unwanted.removeCommon(wantedMask)) {}
template <typename... CTypes>
template <typename Func>
void Selection<CTypes...>::forEach(Func func)
{
    static_assert(std::is_invocable_v<Func, Iterator_t const&>);
    for (Iterator_t it = begin(), itEnd = end(); it != itEnd; ++it)
        func(it);
}

template <typename... CTypes>
inline typename Selection<CTypes...>::Iterator_t
Selection<CTypes...>::begin()
{
    return Iterator_t(*this, false);
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
    return ConstIterator_t(*this, false);
}

template <typename... CTypes>
inline typename Selection<CTypes...>::ConstIterator_t
Selection<CTypes...>::end() const
{
    return ConstIterator_t(*this, true);
}

template <typename... CTypes>
CMask const& Selection<CTypes...>::getWanted() const { return wantedMask; }

template <typename... CTypes>
CMask const& Selection<CTypes...>::getUnwanted() const { return unwantedMask; }


template <typename... CTypes>
inline std::size_t
Selection<CTypes...>::countEntities() const
{
    std::size_t sum = 0;
    for (auto const& pool : entityPools)
        sum += pool->data.size();
    return sum;
}

template <typename... CTypes>
inline void Selection<CTypes...>::addSpawnerIfMeetsRequirements(EntitySpawner& spawner)
{
    if (spawner.mask.contains(wantedMask) && !spawner.mask.hasCommon(unwantedMask)) {
        entityPools.push_back(&spawner.getEntities());
        spawnerIds.push_back(spawner.spawnerId);
        if constexpr (sizeof...(CTypes) > 0)
            (this->poolsPack.template get<typename Base_t::template PoolsPtrs_t<CTypes>>().push_back(&spawner.getPool(IdOf<std::remove_const_t<CTypes>>())), ...);
    }
}


} // namespace epp

#endif // EPP_SELECTION_H