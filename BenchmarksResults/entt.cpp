#include <benchmark/benchmark.h>
#include <entt/core/type_info.hpp>
#include <entt/entity/registry.hpp>


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
inline void assignComponents(entt::registry& reg, entt::entity ent)
{
    if constexpr (id != 0)
        assignComponents<id - 1, id, ids...>(reg, ent);
    else
        ((reg.template assign<comp<ids>>(ent)), ...);
}

template <int id, int... ids>
inline void assignComponents(entt::registry& reg, std::vector<entt::entity>& ents)
{
    if constexpr (id != 0)
        assignComponents<id - 1, id, ids...>(reg, ents);
    else
        ((reg.template assign<comp<ids>>(ents.begin(), ents.end())), ...);
}

template <int id, int... ids>
inline void reserveComponents(entt::registry& reg, std::size_t n)
{
    if constexpr (id != 0)
        reserveComponents<id - 1, id, ids...>(reg, n);
    else
        reg.template reserve<comp<ids>...>(n);
}

template <int id, int... ids>
inline decltype(auto) makeView(entt::registry& reg)
{
    if constexpr (id != 0)
        return makeView<id - 1, id, ids...>(reg);
    else
        return reg.template view<comp<ids>...>();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

template <int cNum>
static void BM_EntitiesSequentialCreation(benchmark::State& state)
{
    static NewLine nl;

    entt::registry registry;
    for (auto _ : state)
        for (std::uint64_t i = 0; i < state.range(0); ++i)
            assignComponents<cNum>(registry, registry.create());
}

template <int cNum>
static void BM_EntitiesSequentialCreationReserved(benchmark::State& state)
{
    static NewLine nl;

    entt::registry registry;
    registry.reserve<>(state.range(0));
    reserveComponents<cNum>(registry, state.range(0));

    for (auto _ : state)
        for (std::uint64_t i = 0; i < state.range(0); ++i)
            assignComponents<cNum>(registry, registry.create());
}

template <int cNum>
static void BM_EntitiesAtOnceCreation(benchmark::State& state)
{
    static NewLine nl;

    entt::registry registry;
    std::vector<entt::entity> entities(state.range(0));
    registry.reserve<>(state.range(0));
    reserveComponents<cNum>(registry, state.range(0));
    for (auto _ : state) {
        registry.create(entities.begin(), entities.end());
        assignComponents<cNum>(registry, entities);
    }
}

template <int cNum>
static void BM_EntitiesSequentialDestroy(benchmark::State& state)
{
    static NewLine nl;

    entt::registry registry;
    for (std::uint64_t i = 0; i < state.range(0); ++i)
        assignComponents<cNum>(registry, registry.create());

    for (auto _ : state)
        registry.each([&registry](auto entity) { registry.destroy(entity); });
}

template <int cNum>
static void BM_EntitiesAtOnceDestroy(benchmark::State& state)
{
    static NewLine nl;

    entt::registry registry;
    std::vector<entt::entity> entities(state.range(0));
    registry.create(entities.begin(), entities.end());
    assignComponents<cNum>(registry, entities);

    for (auto _ : state)
        registry.destroy(entities.begin(), entities.end());
}

template <int cNum>
static void BM_EntitiesIteration(benchmark::State& state)
{
    static NewLine nl;

    entt::registry registry;
    for (std::uint64_t i = 0; i < state.range(0); ++i)
        assignComponents<cNum>(registry, registry.create());

    // auto test = [&](auto func) {
    //     ENTT_ID_TYPE types[] = { entt::type_info<comp<1>>::id() };
    //     for (auto _ : state)
    //         registry.runtime_view(std::begin(types), std::end(types)).each(func);
    // };

    // test([&registry](auto entity) { registry.get<comp<1>>(entity).x = {}; });

    auto test = [&](auto func) {
        for (auto _ : state)
            makeView<cNum>(registry).each(func);
    };
    test([](auto&... comp) { ((comp.x = {}), ...); });
}

template <int cNum>
static void BM_EntitiesIterationHalf(benchmark::State& state)
{
    static NewLine nl;

    entt::registry registry;
    for (std::uint64_t i = 0; i < state.range(0); ++i)
        if (i % 2)
            assignComponents<cNum>(registry, registry.create());
        else
            assignComponents<cNum - 1>(registry, registry.create());

    auto test = [&](auto func) {
        for (auto _ : state)
            makeView<cNum>(registry).each(func);
    };
    test([](auto&... comp) { ((comp.x = {}), ...); });
}

template <int cNum>
static void BM_EntitiesIterationOneOfMany(benchmark::State& state)
{
    static NewLine nl;

    entt::registry registry;
    for (std::uint64_t i = 0; i < state.range(0); ++i)
        if (i == state.range(0) / 2)
            assignComponents<cNum>(registry, registry.create());
        else
            assignComponents<cNum - 1>(registry, registry.create());

    auto test = [&](auto func) {
        for (auto _ : state)
            makeView<cNum>(registry).each(func);
    };
    test([](auto&... comp) { ((comp.x = {}), ...); });
}

template <int cNum>
static void BM_EntitiesIterationReal(benchmark::State& state)
{
    // static_assert(cNum >= 2);
    static NewLine nl;

    entt::registry registry;

    // simulate some usage
    std::vector<entt::entity> ents(state.range(0));
    registry.create(ents.begin(), ents.end());
    assignComponents<cNum>(registry, ents); // dynamic bodies
    for (int i = 0; i < state.range(0) / 2; ++i) {
        auto idx = rand() % ents.size();
        registry.destroy(ents[idx]);
        std::swap(ents.back(), ents[idx]);
        ents.pop_back();
    }

    for (int i = 0; i < state.range(0) / 200; ++i) {
        for (int j = 0; j < 4; ++j)
            assignComponents<cNum + 1>(registry, registry.create()); // interactive bodies
        for (int j = 0; j < 100; ++j)
            assignComponents<cNum>(registry, registry.create()); // dynamic bodies
        for (int j = 0; j < 5; ++j)
            assignComponents<cNum - 1>(registry, registry.create()); // static bodies
        if (i % 10 == 0) {
            auto ent = registry.create(); // ai
            registry.assign<comp<cNum + 2>>(ent);
            registry.assign<comp<cNum + 3>>(ent);
            registry.assign<comp<cNum + 4>>(ent);
            registry.assign<comp<cNum + 5>>(ent);
            registry.assign<comp<cNum + 6>>(ent);
        }
        if (i == state.range(0) / 100 / 2)
            for (int j = 0; j < 10; ++j) {
                auto ent = registry.create(); // players
                registry.assign<comp<cNum + 2>>(ent);
                registry.assign<comp<cNum + 3>>(ent);
                registry.assign<comp<cNum + 4>>(ent);
                registry.assign<comp<cNum + 7>>(ent);
                registry.assign<comp<cNum + 8>>(ent);
            }
    }
    int i = 0;
    auto test = [&](auto func) {
        for (auto _ : state)
            makeView<cNum>(registry).each(func);
    };
    test([](auto&... comp) { ((comp.x = {}), ...); });
}

template <int cNum>
static void BM_Add2Components(benchmark::State& state)
{
    static NewLine nl;

    entt::registry registry;
    std::vector<entt::entity> entities(state.range(0));
    registry.create(entities.begin(), entities.end());
    assignComponents<cNum>(registry, entities);
    for (auto _ : state) {
        registry.assign<comp<cNum + 1>>(entities.begin(), entities.end());
        registry.assign<comp<cNum + 2>>(entities.begin(), entities.end());
    }
}

#define MYBENCHMARK_TEMPLATE(name, iters, reps, shortReport, ...)     \
    BENCHMARK_TEMPLATE(name, __VA_ARGS__)                             \
        ->DenseRange(1024 * 1024 / 16, 1024 * 1024, 1024 * 1024 / 16) \
        ->Iterations(iters)                                           \
        ->Repetitions(reps)                                           \
        ->ReportAggregatesOnly(shortReport);

#define MYBENCHMARK_TEMPLATE_N(name, iters, reps)    \
    MYBENCHMARK_TEMPLATE(name, iters, reps, true, 1) \
    MYBENCHMARK_TEMPLATE(name, iters, reps, true, 2) \
    MYBENCHMARK_TEMPLATE(name, iters, reps, true, 3) \
    MYBENCHMARK_TEMPLATE(name, iters, reps, true, 6)

constexpr static std::size_t const ITERS = 100;
constexpr static std::size_t const REPS = 10;

MYBENCHMARK_TEMPLATE_N(BM_EntitiesSequentialCreation, 1, ITERS)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesSequentialCreationReserved, 1, ITERS)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesAtOnceCreation, 1, ITERS)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesSequentialDestroy, 1, ITERS)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesAtOnceDestroy, 1, ITERS)
MYBENCHMARK_TEMPLATE_N(BM_Add2Components, 1, ITERS)
// MYBENCHMARK_TEMPLATE_N(BM_Remove2Components, 1, ITERS)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesIteration, ITERS, REPS)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesIterationHalf, ITERS, REPS)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesIterationOneOfMany, ITERS, REPS)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesIterationReal, ITERS, REPS)