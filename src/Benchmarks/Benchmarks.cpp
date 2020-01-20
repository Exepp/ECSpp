#include <ECSpp/EntityManager.h>
#include <benchmark/benchmark.h>


template <std::size_t n>
struct comp {
    std::uint64_t x;
    std::uint64_t y;
};

static constexpr std::size_t const Repetitions = 1;


struct NewLine {
    NewLine() { printf("\n"); }
};

template <int id, int... ids>
epp::Archetype makeArchetype()
{
    if constexpr (id != 0)
        return makeArchetype<id - 1, id, ids...>();
    else
        return epp::Archetype(epp::IdOfL<comp<ids>...>());
}

template <int id, int... ids>
decltype(auto) makeCollection()
{
    if constexpr (id != 0)
        return makeCollection<id - 1, id, ids...>();
    else
        return epp::EntityCollection<comp<ids>...>();
}

template <int id, int... ids, typename It>
inline void assignComponents(It& it)
{
    if constexpr (id != 0)
        assignComponents<id - 1, id, ids...>(it);
    else
        ((it.template getComponent<comp<ids>>().x = {}), ...);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

template <int cNum>
static void BM_EntitiesSequentialCreation(benchmark::State& state)
{
    static NewLine nl;

    epp::EntityManager mgr;
    epp::Archetype arch = makeArchetype<cNum>();
    for (auto _ : state)
        for (int i = 0; i < state.range(0); ++i)
            mgr.spawn(arch);
}

template <int cNum>
static void BM_EntitiesSequentialCreationReserved(benchmark::State& state)
{
    static NewLine nl;

    epp::EntityManager mgr;
    epp::Archetype arch = makeArchetype<cNum>();
    mgr.prepareToSpawn(arch, state.range(0));
    for (auto _ : state)
        for (int i = 0; i < state.range(0); ++i)
            mgr.spawn(arch);
}

template <int cNum>
static void BM_EntitiesAtOnceCreation(benchmark::State& state)
{
    static NewLine nl;

    epp::EntityManager mgr;
    epp::Archetype arch = makeArchetype<cNum>();
    for (auto _ : state)
        mgr.spawn(arch, state.range(0));
}

template <int cNum>
static void BM_EntitiesSequentialDestroy(benchmark::State& state)
{
    static NewLine nl;

    epp::EntityManager mgr;
    epp::Archetype arch = makeArchetype<cNum>();

    mgr.spawn(arch, state.range(0));
    auto& entities = mgr.entitiesOf(arch);
    for (auto _ : state)
        for (auto it = entities.data.begin(); it != entities.data.end();)
            it = mgr.destroy(it);
}

template <int cNum>
static void BM_EntitiesAtOnceDestroy(benchmark::State& state)
{
    static NewLine nl;

    epp::EntityManager mgr;
    epp::Archetype arch = makeArchetype<cNum>();

    mgr.spawn(arch, state.range(0));
    auto& entities = mgr.entitiesOf(arch);
    for (auto _ : state)
        mgr.clear(arch);
}

template <int cNum>
static void BM_EntitiesIteration(benchmark::State& state)
{
    static NewLine nl;

    epp::EntityManager mgr;
    epp::Archetype arch = makeArchetype<cNum>();
    auto coll = makeCollection<cNum>();

    mgr.spawn(arch, state.range(0));
    mgr.updateCollection(coll);
    for (auto _ : state)
        for (auto it = coll.begin(), end = coll.end(); it != end; ++it)
            assignComponents<cNum>(it);
}

template <int cNum>
static void BM_EntitiesIterationHalf(benchmark::State& state)
{
    static NewLine nl;

    epp::EntityManager mgr;
    epp::Archetype archFull = makeArchetype<cNum>();
    epp::Archetype archMissing = makeArchetype<cNum - 1>();
    auto coll = makeCollection<cNum>();

    for (int i = 0; i < state.range(0); ++i)
        if (i % 2)
            mgr.spawn(archFull);
        else
            mgr.spawn(archMissing);
    mgr.updateCollection(coll);

    for (auto _ : state)
        for (auto it = coll.begin(), end = coll.end(); it != end; ++it)
            assignComponents<cNum>(it);
}

template <int cNum>
static void BM_EntitiesIterationOneOfMany(benchmark::State& state)
{
    static NewLine nl;

    epp::EntityManager mgr;
    epp::Archetype archFull = makeArchetype<cNum>();
    epp::Archetype archMissing = makeArchetype<cNum - 1>();
    auto coll = makeCollection<cNum>();

    mgr.spawn(archMissing, state.range(0) / 2);
    mgr.spawn(archFull);
    mgr.spawn(archMissing, state.range(0) / 2 - 1);

    for (auto _ : state)
        for (auto it = coll.begin(), end = coll.end(); it != end; ++it)
            assignComponents<cNum>(it);
}

template <int cNum>
static void BM_EntitiesIterationReal(benchmark::State& state)
{
    static_assert(cNum >= 2);
    static NewLine nl;

    epp::EntityManager mgr;
    epp::Archetype archs[] = { makeArchetype<cNum + 1>(),                                                                                     // interactive bodies
                               makeArchetype<cNum>(),                                                                                         // dynamic bodies
                               makeArchetype<cNum - 1>(),                                                                                     // static bodies
                               epp::Archetype(epp::IdOf<comp<cNum + 2>, comp<cNum + 3>, comp<cNum + 4>, comp<cNum + 5>, comp<cNum + 6>>()),   // ai
                               epp::Archetype(epp::IdOf<comp<cNum + 2>, comp<cNum + 3>, comp<cNum + 4>, comp<cNum + 7>, comp<cNum + 8>>()) }; // players


    mgr.spawn(epp::Archetype(), 2 * cNum * state.range(0));
    auto& ents = mgr.entitiesOf(epp::Archetype()).data;
    for (int i = 0; i < cNum * state.range(0); ++i)
        mgr.destroy(ents[rand() % ents.size()]);

    for (int i = 0; i < state.range(0) / 100; ++i) {
        mgr.spawn(archs[0], 4);
        mgr.spawn(archs[1], 100);
        mgr.spawn(archs[2], 5);
        if (i % 10 == 0)
            mgr.spawn(archs[3]);
        if (i == state.range(0) / 100 / 2)
            mgr.spawn(archs[4], 10);
    }

    // static std::vector<comp<0>> vec;
    // vec = std::vector<comp<0>>(1e7);
    // vec.resize(0);

    auto coll = makeCollection<cNum>();
    mgr.updateCollection(coll);

    for (auto _ : state)
        for (auto it = coll.begin(), end = coll.end(); it != end; ++it)
            assignComponents<cNum>(it);
}

template <int cNum>
static void BM_AddComponents(benchmark::State& state)
{
    static NewLine nl;

    epp::EntityManager mgr;
    epp::Archetype archMissing = makeArchetype<2>();
    epp::Archetype archFull = makeArchetype<2 + cNum>();
    auto coll = makeCollection<2>();

    mgr.spawn(archMissing, state.range(0));
    mgr.updateCollection(coll);
    auto const& ents = mgr.entitiesOf(archMissing);
    for (auto _ : state)
        for (auto it = coll.begin(), end = coll.end(); it != end;)
            it = mgr.changeArchetype(it, archFull);
}


#define MYBENCHMARK_TEMPLATE(name, ...) BENCHMARK_TEMPLATE(name, __VA_ARGS__)->Range(1e6, 5e6)->Iterations(1)->Repetitions(Repetitions * 10)->ReportAggregatesOnly(true);
#define MYBENCHMARK_TEMPLATE_2_3_5(name) \
    MYBENCHMARK_TEMPLATE(name, 2)        \
    MYBENCHMARK_TEMPLATE(name, 3)        \
    MYBENCHMARK_TEMPLATE(name, 5)

MYBENCHMARK_TEMPLATE_2_3_5(BM_EntitiesSequentialCreation)
MYBENCHMARK_TEMPLATE_2_3_5(BM_EntitiesSequentialCreationReserved)
MYBENCHMARK_TEMPLATE_2_3_5(BM_EntitiesAtOnceCreation)
MYBENCHMARK_TEMPLATE_2_3_5(BM_EntitiesSequentialDestroy)
MYBENCHMARK_TEMPLATE_2_3_5(BM_EntitiesAtOnceDestroy)
MYBENCHMARK_TEMPLATE_2_3_5(BM_EntitiesIteration)
MYBENCHMARK_TEMPLATE_2_3_5(BM_EntitiesIterationHalf)
MYBENCHMARK_TEMPLATE_2_3_5(BM_EntitiesIterationOneOfMany)
MYBENCHMARK_TEMPLATE_2_3_5(BM_EntitiesIterationReal)
MYBENCHMARK_TEMPLATE_2_3_5(BM_AddComponents)

BENCHMARK_MAIN();