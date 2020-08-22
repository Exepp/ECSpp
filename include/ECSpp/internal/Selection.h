#ifndef EPP_SELECTION_H
#define EPP_SELECTION_H

#include <ECSpp/internal/EntityList.h>
#include <ECSpp/internal/EntitySpawner.h>
#include <ECSpp/internal/utility/TuplePP.h>
#include <type_traits>


namespace epp {

/// A set of valid iteration-time operations on EntityManager
/** "Current" refers to the entity provided by Selection in forEach method. \
 * This means, that performing these operations on other entities is an undefined behaviour. */
enum class IterTimeChange : std::size_t { ArchetypeCurrent = 0,
                                          DestroyedCurrent = 0,
                                          SpawnedNew = 1,
                                          AnyClear = 1,
                                          ChangeFailed = 1 };


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
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// A specialization of helper class for conditional member variables
/** Version with types - adding member variables */
template <typename... CTypes> // true version
struct SelectionBase<true, CTypes...> {
    template <typename T> // discard T
    struct PoolsPtrs_t : public std::vector<CPool*> {};

    using PoolsPtrsPack_t = TuplePP<PoolsPtrs_t<CTypes>...>;


    PoolsPtrsPack_t poolsPack; // for each component type, a vector of pools of that component,
                               // one pool for each accepted spawner (archetype)
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/// A query to the EntityManager for entites that own a certain set of components
/**
 * Using forEach member function you can iterate over each entity that ows at least all of the components of CTypes... types.
 * This class stores pointers to CPools and EntityPools of the EntitySpawners with
 * archetypes that matches the specified requirements (CTypes and unwanted mask)
 * @tparam CTypes A pack of component types used to select wanted entities
 */
template <typename... CTypes>
class Selection : public SelectionBase<(sizeof...(CTypes) > 0), CTypes...> {
    using Base_t = SelectionBase<(sizeof...(CTypes) > 0), CTypes...>;
    using EntityPools_t = std::vector<Pool<Entity> const*>;
    using SpawnerIds_t = std::vector<SpawnerId>;


public:
    /// Constructs a selection with a specified requirements
    /**
     * @param unwanted Mask of components the entities mustn't have.
     */
    explicit Selection(CMask unwanted = CMask());


    /// Calls func on each entity that this selection covers
    /**
     * @tparam Func A callable type that accepts (Entity, CTypes&...) as arguments
     * @param func A callable object that accepts (Entity, CTypes&...) as arguments
     */
    template <typename Func>
    void forEach(Func func);


    /** @returns Mask with wanted types of components (CTypes...) */
    CMask const& getWanted() const;


    /** @returns Mask with unwanted types of components passed in the selection's constructor*/
    CMask const& getUnwanted() const;


    /** @returns Number of entites that this selection covers */
    std::size_t countEntities() const;

private:
    void addSpawnerIfMeetsRequirements(EntitySpawner& spawner);

    template <typename T>
    T& getComponent(std::size_t sIdx, std::size_t eIdx) { return *static_cast<T*>((
        *this->poolsPack.template get<typename Base_t::template PoolsPtrs_t<T>>()[sIdx])[eIdx]); }

    Entity getEntity(std::size_t sIdx, std::size_t eIdx) { return entityPools[sIdx]->data[eIdx]; }

private:
    CMask const wantedMask;    // must be declared before unwanted
    CMask const unwantedMask;  // if wanted & unwated (common part) != 0, then unwanted = unwanted \ (unwanted & wanted)
    EntityPools_t entityPools; // one pool for each accepted spawner
    SpawnerIds_t spawnerIds;
    std::size_t checkedSpawnersNum = 0;


    friend class EntityManager;
};


template <typename... CTypes>
Selection<CTypes...>::Selection(CMask unwanted) : wantedMask(IdOfL<std::remove_const_t<CTypes>...>()),
                                                  unwantedMask(unwanted.removeCommon(wantedMask))
{}
template <typename... CTypes>
template <typename Func>
void Selection<CTypes...>::forEach(Func func)
{
    static_assert(std::is_invocable_v<Func, Entity, CTypes&...>);
    constexpr static bool ReturnsIterTimeChange = std::is_same_v<std::invoke_result_t<Func, Entity, CTypes&...>, IterTimeChange>;
    constexpr static bool ReturnsVoid = std::is_same_v<std::invoke_result_t<Func, Entity, CTypes&...>, void>;
    static_assert(ReturnsIterTimeChange || ReturnsVoid, "Wrong return type of func");

    for (std::size_t sIdx = 0; sIdx < entityPools.size(); ++sIdx)
        for (std::size_t eIdx = 0; eIdx < entityPools[sIdx]->data.size();) {
            if constexpr (ReturnsIterTimeChange)
                eIdx += static_cast<std::size_t>(func(getEntity(sIdx, eIdx), getComponent<CTypes>(sIdx, eIdx)...));
            else {
                func(getEntity(sIdx, eIdx), getComponent<CTypes>(sIdx, eIdx)...);
                ++eIdx;
            }
        }
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