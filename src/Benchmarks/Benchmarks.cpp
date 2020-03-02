#include <ECSpp/EntityManager.h>
#include <ECSpp/internal/Pipeline.h>
#include <benchmark/benchmark.h>


template <std::size_t n>
struct comp {
    std::size_t x;
    std::size_t y;
};


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
decltype(auto) makeSelection()
{
    if constexpr (id != 0)
        return makeSelection<id - 1, id, ids...>();
    else
        return epp::Selection<comp<ids>...>();
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
    mgr.prepareToSpawn(arch, state.range(0));
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
    for (auto _ : state)
        mgr.clear(arch);
}

template <int cNum>
static void BM_EntitiesIteration(benchmark::State& state)
{
    static NewLine nl;

    epp::EntityManager mgr;
    epp::Archetype arch = makeArchetype<cNum>();
    auto sel = makeSelection<cNum>();
    // epp::Selection<> sel;
    mgr.spawn(arch, state.range(0));
    mgr.updateSelection(sel);
    for (auto _ : state) {
        // sel.forEach([](auto const& it) { assignComponents<cNum>(it); });
        for (auto it = sel.begin(), end = sel.end(); it != end; ++it)
            // assignComponents<cNum>(mgr, *it);
            assignComponents<cNum>(it);
    }
}

template <int cNum>
static void BM_EntitiesIterationHalf(benchmark::State& state)
{
    static NewLine nl;

    epp::EntityManager mgr;
    epp::Archetype archFull = makeArchetype<cNum>();
    epp::Archetype archMissing = makeArchetype<cNum - 1>();
    auto sel = makeSelection<cNum>();

    for (int i = 0; i < state.range(0); ++i)
        if (i % 2)
            mgr.spawn(archFull);
        else
            mgr.spawn(archMissing);
    mgr.updateSelection(sel);
    for (auto _ : state)
        for (auto it = sel.begin(), end = sel.end(); it != end; ++it)
            assignComponents<cNum>(it);
}

template <int cNum>
static void BM_EntitiesIterationOneOfMany(benchmark::State& state)
{
    static NewLine nl;

    epp::EntityManager mgr;
    epp::Archetype archFull = makeArchetype<cNum>();
    epp::Archetype archMissing = makeArchetype<cNum - 1>();
    auto sel = makeSelection<cNum>();

    mgr.spawn(archMissing, state.range(0) / 2);
    mgr.spawn(archFull);
    mgr.spawn(archMissing, state.range(0) / 2 - 1);
    mgr.updateSelection(sel);
    for (auto _ : state)
        for (auto it = sel.begin(), end = sel.end(); it != end; ++it)
            assignComponents<cNum>(it);
}

template <int cNum>
static void BM_EntitiesIterationReal(benchmark::State& state)
{
    // static_assert(cNum >= 2);
    static NewLine nl;

    epp::EntityManager mgr;
    epp::Archetype archs[] = { makeArchetype<cNum + 1>(),                                                                                     // interactive bodies
                               makeArchetype<cNum>(),                                                                                         // dynamic bodies
                               makeArchetype<cNum - 1>(),                                                                                     // static bodies
                               epp::Archetype(epp::IdOf<comp<cNum + 2>, comp<cNum + 3>, comp<cNum + 4>, comp<cNum + 5>, comp<cNum + 6>>()),   // ai
                               epp::Archetype(epp::IdOf<comp<cNum + 2>, comp<cNum + 3>, comp<cNum + 4>, comp<cNum + 7>, comp<cNum + 8>>()) }; // players


    mgr.spawn(archs[1], state.range(0));
    auto& ents = mgr.entitiesOf(archs[1]).data;
    for (int i = 0; i < state.range(0) / 2; ++i) // remove half
        mgr.destroy(ents[rand() % ents.size()]);

    for (int i = 0; i < state.range(0) / 200; ++i) {
        mgr.spawn(archs[0], 4);
        mgr.spawn(archs[1], 100);
        mgr.spawn(archs[2], 5);
        if (i % 10 == 0)
            mgr.spawn(archs[3]);
        if (i == state.range(0) / 100 / 2)
            mgr.spawn(archs[4], 10);
    }

    auto sel = makeSelection<cNum>();
    mgr.updateSelection(sel);
    for (auto _ : state)
        for (auto it = sel.begin(), end = sel.end(); it != end; ++it)
            assignComponents<cNum>(it);
}

template <int cNum>
static void BM_AddComponents(benchmark::State& state)
{
    static NewLine nl;

    epp::EntityManager mgr;
    epp::Archetype archMissing = makeArchetype<2>();
    epp::Archetype archFull = makeArchetype<2 + cNum>();
    auto sel = makeSelection<2>();

    mgr.spawn(archMissing, state.range(0));
    mgr.updateSelection(sel);
    for (auto _ : state)
        for (auto it = sel.begin(), end = sel.end(); it != end;)
            it = mgr.changeArchetype(it, archFull);
    // mgr.changeArchetype(archMissing, archFull, [](epp::EntityCreator&& creator) { creator.constructed<comp<4>>().y = 2 * creator.idx.value; });
}

template <int cNum>
static void BM_RemoveComponents(benchmark::State& state)
{
    static NewLine nl;

    epp::EntityManager mgr;
    epp::Archetype archFull = makeArchetype<2 + cNum>();
    epp::Archetype archMissing = makeArchetype<2>();
    auto sel = makeSelection<2 + cNum>();

    mgr.spawn(archFull, state.range(0));
    mgr.updateSelection(sel);
    for (auto _ : state)
        for (auto it = sel.begin(), end = sel.end(); it != end;)
            it = mgr.changeArchetype(it, archMissing);
}


#define MYBENCHMARK_TEMPLATE(name, iters, reps, shortReport, ...) BENCHMARK_TEMPLATE(name, __VA_ARGS__) \
                                                                      ->Range(1e6, 1e6)                 \
                                                                      ->Iterations(iters)               \
                                                                      ->Repetitions(reps)               \
                                                                      ->ReportAggregatesOnly(shortReport);

#define MYBENCHMARK_TEMPLATE_N(name, iters, reps) MYBENCHMARK_TEMPLATE(name, iters, reps, true, 1)

constexpr static std::size_t const ITERS = 100;
constexpr static std::size_t const REPS = 10;

MYBENCHMARK_TEMPLATE_N(BM_EntitiesSequentialCreation, 1, ITERS)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesSequentialCreationReserved, 1, ITERS)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesAtOnceCreation, 1, ITERS)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesSequentialDestroy, 1, ITERS)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesAtOnceDestroy, 1, ITERS)
MYBENCHMARK_TEMPLATE_N(BM_AddComponents, 1, ITERS)
MYBENCHMARK_TEMPLATE_N(BM_RemoveComponents, 1, ITERS)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesIteration, ITERS, REPS)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesIterationHalf, ITERS, REPS)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesIterationOneOfMany, ITERS, REPS)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesIterationReal, ITERS, REPS)

BENCHMARK_MAIN();