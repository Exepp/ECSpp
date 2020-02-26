#include <ECSpp/EntityManager.h>
#include <benchmark/benchmark.h>


template <std::size_t n>
struct comp {
    std::uint64_t x;
    std::uint64_t y;
};


struct NewLine {
    NewLine() { printf("\n"); }
};


template <std::size_t... ids>
inline epp::Archetype makeArchetype(std::index_sequence<ids...>)
{
    return epp::Archetype(epp::IdOfL<comp<ids>...>());
}
template <int id>
inline epp::Archetype makeArchetype()
{
    return makeArchetype(std::make_index_sequence<id>());
}

template <std::size_t... ids>
inline decltype(auto) makeSelection(std::index_sequence<ids...>)
{
    return epp::Selection<comp<ids>...>();
}
template <int id>
inline decltype(auto) makeSelection()
{
    return makeSelection(std::make_index_sequence<id>());
}

template <typename It, std::size_t... ids>
inline void assignComponents(It& it, std::index_sequence<ids...>)
{
    static float j = 0;
    ((it.template getComponent<comp<ids>>() = { (std::size_t)(123e5 + j), (std::size_t)(431e6 + j) }), ...);
    j += 123;
}
template <int id, typename It>
inline void assignComponents(It& it)
{
    assignComponents(it, std::make_index_sequence<id>());
}

template <std::size_t... ids>
inline void assignComponents(epp::EntityManager& mgr, epp::Entity ent, std::index_sequence<ids...>)
{
    ((mgr.componentOf<comp<ids>>(ent) = { (int)123e5, (int)431e6 }), ...);
}
template <int id>
inline void assignComponents(epp::EntityManager& mgr, epp::Entity ent)
{
    assignComponents(mgr, ent, std::make_index_sequence<id>());
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
    for (auto _ : state)
        mgr.clear(arch);
}
#include <ECSpp/internal/Pipeline.h>
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
    epp::Pipeline tasks;
    auto fn = [](auto const& it) {
        static float j = 0;
        it.template getComponent<comp<0>>() = { (std::size_t)(123e5 + j), (std::size_t)(431e6 + j) };
        j += 123;
    };
    tasks.setTask<comp<0>>(fn, [](std::size_t n) { return 500; })
        .template setSubtask<comp<0>>(fn)
        .template setSubtask<comp<0>>(fn)
        .template setSubtask<comp<0>>(fn);
    ;
    int i = 0;

    auto thFn = [&] { 
        for (auto it = sel.begin(), end = sel.end(); it != end; ++it)
            assignComponents<cNum>(it); };

    for (auto _ : state) {
        // sel.forEach([](auto const& it) { assignComponents<cNum>(it); });
        // for (auto it = sel.begin(), end = sel.end(); it != end; ++it)
        // assignComponents<cNum>(mgr, *it);
        // assignComponents<cNum>(it);
        // for (auto it = sel.begin(), end = sel.end(); it != end; ++it)
        //     fn(it);
        // for (auto it = sel.begin(), end = sel.end(); it != end; ++it)
        //     fn(it);
        // for (auto it = sel.begin(), end = sel.end(); it != end; ++it)
        //     fn(it);
        // for (auto it = sel.begin(), end = sel.end(); it != end; ++it)
        //     fn(it);
        tasks.run(mgr);
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


#define MYBENCHMARK_TEMPLATE(name, shortReport, ...) BENCHMARK_TEMPLATE(name, __VA_ARGS__) \
                                                         ->Range(1e6, 1e6)                 \
                                                         ->Iterations(100)                 \
                                                         ->Repetitions(10)                 \
                                                         ->ReportAggregatesOnly(shortReport);

#define MYBENCHMARK_TEMPLATE_N(name, shortReport) MYBENCHMARK_TEMPLATE(name, shortReport, 1)

MYBENCHMARK_TEMPLATE_N(BM_EntitiesSequentialCreation, true)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesSequentialCreationReserved, true)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesAtOnceCreation, true)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesSequentialDestroy, true)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesAtOnceDestroy, true)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesIteration, true)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesIterationHalf, true)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesIterationOneOfMany, true)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesIterationReal, true)
MYBENCHMARK_TEMPLATE_N(BM_AddComponents, false)
MYBENCHMARK_TEMPLATE_N(BM_RemoveComponents, false)

BENCHMARK_MAIN();