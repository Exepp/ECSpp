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
    // registry.reserve<>(state.range(0));
    // reserveComponents<cNum>(registry, state.range(0));
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
static void BM_AddComponents(benchmark::State& state)
{
    static NewLine nl;

    entt::registry registry;
    std::vector<entt::entity> entities(state.range(0));
    registry.create(entities.begin(), entities.end());
    registry.assign<comp<cNum + 1>>(entities.begin(), entities.end());
    registry.assign<comp<cNum + 2>>(entities.begin(), entities.end());
    for (auto _ : state)
        assignComponents<cNum>(registry, entities);
}

#define MYBENCHMARK_TEMPLATE(name, iters, reps, shortReport, ...) BENCHMARK_TEMPLATE(name, __VA_ARGS__) \
                                                                      ->Range(1e6, 1e6)                 \
                                                                      ->Iterations(iters)               \
                                                                      ->Repetitions(reps)               \
                                                                      ->ReportAggregatesOnly(shortReport);

#define MYBENCHMARK_TEMPLATE_N(name, iters, reps, shortReport) MYBENCHMARK_TEMPLATE(name, iters, reps, shortReport, 1)

MYBENCHMARK_TEMPLATE_N(BM_EntitiesSequentialCreation, 1, 100, true)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesSequentialCreationReserved, 1, 100, true)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesAtOnceCreation, 1, 100, true)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesSequentialDestroy, 1, 100, true)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesAtOnceDestroy, 1, 100, true)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesIteration, 100, 10, true)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesIterationHalf, 100, 10, true)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesIterationOneOfMany, 100, 10, true)
MYBENCHMARK_TEMPLATE_N(BM_EntitiesIterationReal, 100, 10, true)
MYBENCHMARK_TEMPLATE_N(BM_AddComponents, 1, 100, true)
// MYBENCHMARK_TEMPLATE_N(BM_RemoveComponents, 1, 100, false)


// static void BM_1M_EntitiesIterationRuntime(benchmark::State& state)
// {
//     static NewLine nl;
// }
// BENCHMARK(BM_1M_EntitiesIterationRuntime)->Iterations(1)->Repetitions(Repetitions);

// TEST(Benchmark, IterateSingleComponentRuntime1M) {
//   entt::registry registry;

//   std::cout << "Iterating over 1000000 entities, one component, runtime view
//   "
//             << std::endl;

//   for (std::uint64_t i = 0; i < 1000000L; i++) {
//     const auto entity = registry.create();
//     registry.assign<position>(entity);
//   }

//   auto test = [&registry](auto func) {
//     ENTT_ID_TYPE types[] = {entt::type_info<position>::id()};

//     timer timer;
//     registry.runtime_view(std::begin(types), std::end(types)).each(func);
//     timer.elapsed();
//   };

//   test([&registry](auto entity) { registry.get<position>(entity).x = {}; });
// }

// TEST(Benchmark, IterateTwoComponents1MOne) {
//     entt::registry registry;

//     std::cout << "Iterating over 1000000 entities, two components, only one
//     entity has all the components" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<velocity>(entity);

//         if(i == 500000L) {
//             registry.assign<position>(entity);
//         }
//     }

//     auto test = [&registry](auto func) {
//         timer timer;
//         registry.view<position, velocity>().each(func);
//         timer.elapsed();
//     };

//     test([](auto &... comp) {
//         ((comp.x = {}), ...);
//     });
// }

// TEST(Benchmark, IterateTwoComponentsNonOwningGroup1M) {
//     entt::registry registry;
//     registry.group<>(entt::get<position, velocity>);

//     std::cout << "Iterating over 1000000 entities, two components, non owning
//     group" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<position>(entity);
//         registry.assign<velocity>(entity);
//     }

//     auto test = [&registry](auto func) {
//         timer timer;
//         registry.group<>(entt::get<position, velocity>).each(func);
//         timer.elapsed();
//     };

//     test([](auto &... comp) {
//         ((comp.x = {}), ...);
//     });
// }

// TEST(Benchmark, IterateTwoComponentsFullOwningGroup1M) {
//     entt::registry registry;
//     registry.group<position, velocity>();

//     std::cout << "Iterating over 1000000 entities, two components, full
//     owning group" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<position>(entity);
//         registry.assign<velocity>(entity);
//     }

//     auto test = [&registry](auto func) {
//         timer timer;
//         registry.group<position, velocity>().each(func);
//         timer.elapsed();
//     };

//     test([](auto &... comp) {
//         ((comp.x = {}), ...);
//     });
// }

// TEST(Benchmark, IterateTwoComponentsPartialOwningGroup1M) {
//     entt::registry registry;
//     registry.group<position>(entt::get<velocity>);

//     std::cout << "Iterating over 1000000 entities, two components, partial
//     owning group" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<position>(entity);
//         registry.assign<velocity>(entity);
//     }

//     auto test = [&registry](auto func) {
//         timer timer;
//         registry.group<position>(entt::get<velocity>).each(func);
//         timer.elapsed();
//     };

//     test([](auto &... comp) {
//         ((comp.x = {}), ...);
//     });
// }

// TEST(Benchmark, IterateTwoComponentsRuntime1M) {
//     entt::registry registry;

//     std::cout << "Iterating over 1000000 entities, two components, runtime
//     view" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<position>(entity);
//         registry.assign<velocity>(entity);
//     }

//     auto test = [&registry](auto func) {
//         ENTT_ID_TYPE types[] = {
//             entt::type_info<position>::id(),
//             entt::type_info<velocity>::id()
//         };

//         timer timer;
//         registry.runtime_view(std::begin(types), std::end(types)).each(func);
//         timer.elapsed();
//     };

//     test([&registry](auto entity) {
//         registry.get<position>(entity).x = {};
//         registry.get<velocity>(entity).x = {};
//     });
// }

// TEST(Benchmark, IterateTwoComponentsRuntime1MHalf) {
//     entt::registry registry;

//     std::cout << "Iterating over 1000000 entities, two components, half of
//     the entities have all the components, runtime view" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<velocity>(entity);

//         if(i % 2) {
//             registry.assign<position>(entity);
//         }
//     }

//     auto test = [&registry](auto func) {
//         ENTT_ID_TYPE types[] = {
//             entt::type_info<position>::id(),
//             entt::type_info<velocity>::id()
//         };

//         timer timer;
//         registry.runtime_view(std::begin(types), std::end(types)).each(func);
//         timer.elapsed();
//     };

//     test([&registry](auto entity) {
//         registry.get<position>(entity).x = {};
//         registry.get<velocity>(entity).x = {};
//     });
// }

// TEST(Benchmark, IterateTwoComponentsRuntime1MOne) {
//     entt::registry registry;

//     std::cout << "Iterating over 1000000 entities, two components, only one
//     entity has all the components, runtime view" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<velocity>(entity);

//         if(i == 500000L) {
//             registry.assign<position>(entity);
//         }
//     }

//     auto test = [&registry](auto func) {
//         ENTT_ID_TYPE types[] = {
//             entt::type_info<position>::id(),
//             entt::type_info<velocity>::id()
//         };

//         timer timer;
//         registry.runtime_view(std::begin(types), std::end(types)).each(func);
//         timer.elapsed();
//     };

//     test([&registry](auto entity) {
//         registry.get<position>(entity).x = {};
//         registry.get<velocity>(entity).x = {};
//     });
// }

// TEST(Benchmark, IterateThreeComponents1M) {
//     entt::registry registry;

//     std::cout << "Iterating over 1000000 entities, three components" <<
//     std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<position>(entity);
//         registry.assign<velocity>(entity);
//         registry.assign<comp<0>>(entity);
//     }

//     auto test = [&registry](auto func) {
//         timer timer;
//         registry.view<position, velocity, comp<0>>().each(func);
//         timer.elapsed();
//     };

//     test([](auto &... comp) {
//         ((comp.x = {}), ...);
//     });
// }

// TEST(Benchmark, IterateThreeComponents1MHalf) {
//     entt::registry registry;

//     std::cout << "Iterating over 1000000 entities, three components, half of
//     the entities have all the components" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<velocity>(entity);
//         registry.assign<comp<0>>(entity);

//         if(i % 2) {
//             registry.assign<position>(entity);
//         }
//     }

//     auto test = [&registry](auto func) {
//         timer timer;
//         registry.view<position, velocity, comp<0>>().each(func);
//         timer.elapsed();
//     };

//     test([](auto &... comp) {
//         ((comp.x = {}), ...);
//     });
// }

// TEST(Benchmark, IterateThreeComponents1MOne) {
//     entt::registry registry;

//     std::cout << "Iterating over 1000000 entities, three components, only one
//     entity has all the components" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<velocity>(entity);
//         registry.assign<comp<0>>(entity);

//         if(i == 500000L) {
//             registry.assign<position>(entity);
//         }
//     }

//     auto test = [&registry](auto func) {
//         timer timer;
//         registry.view<position, velocity, comp<0>>().each(func);
//         timer.elapsed();
//     };

//     test([](auto &... comp) {
//         ((comp.x = {}), ...);
//     });
// }

// TEST(Benchmark, IterateThreeComponentsNonOwningGroup1M) {
//     entt::registry registry;
//     registry.group<>(entt::get<position, velocity, comp<0>>);

//     std::cout << "Iterating over 1000000 entities, three components, non
//     owning group" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<position>(entity);
//         registry.assign<velocity>(entity);
//         registry.assign<comp<0>>(entity);
//     }

//     auto test = [&registry](auto func) {
//         timer timer;
//         registry.group<>(entt::get<position, velocity, comp<0>>).each(func);
//         timer.elapsed();
//     };

//     test([](auto &... comp) {
//         ((comp.x = {}), ...);
//     });
// }

// TEST(Benchmark, IterateThreeComponentsFullOwningGroup1M) {
//     entt::registry registry;
//     registry.group<position, velocity, comp<0>>();

//     std::cout << "Iterating over 1000000 entities, three components, full
//     owning group" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<position>(entity);
//         registry.assign<velocity>(entity);
//         registry.assign<comp<0>>(entity);
//     }

//     auto test = [&registry](auto func) {
//         timer timer;
//         registry.group<position, velocity, comp<0>>().each(func);
//         timer.elapsed();
//     };

//     test([](auto &... comp) {
//         ((comp.x = {}), ...);
//     });
// }

// TEST(Benchmark, IterateThreeComponentsPartialOwningGroup1M) {
//     entt::registry registry;
//     registry.group<position, velocity>(entt::get<comp<0>>);

//     std::cout << "Iterating over 1000000 entities, three components, partial
//     owning group" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<position>(entity);
//         registry.assign<velocity>(entity);
//         registry.assign<comp<0>>(entity);
//     }

//     auto test = [&registry](auto func) {
//         timer timer;
//         registry.group<position, velocity>(entt::get<comp<0>>).each(func);
//         timer.elapsed();
//     };

//     test([](auto &... comp) {
//         ((comp.x = {}), ...);
//     });
// }

// TEST(Benchmark, IterateThreeComponentsRuntime1M) {
//     entt::registry registry;

//     std::cout << "Iterating over 1000000 entities, three components, runtime
//     view" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<position>(entity);
//         registry.assign<velocity>(entity);
//         registry.assign<comp<0>>(entity);
//     }

//     auto test = [&registry](auto func) {
//         ENTT_ID_TYPE types[] = {
//             entt::type_info<position>::id(),
//             entt::type_info<velocity>::id(),
//             entt::type_info<comp<0>>::id()
//         };

//         timer timer;
//         registry.runtime_view(std::begin(types), std::end(types)).each(func);
//         timer.elapsed();
//     };

//     test([&registry](auto entity) {
//         registry.get<position>(entity).x = {};
//         registry.get<velocity>(entity).x = {};
//         registry.get<comp<0>>(entity).x = {};
//     });
// }

// TEST(Benchmark, IterateThreeComponentsRuntime1MHalf) {
//     entt::registry registry;

//     std::cout << "Iterating over 1000000 entities, three components, half of
//     the entities have all the components, runtime view" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<velocity>(entity);
//         registry.assign<comp<0>>(entity);

//         if(i % 2) {
//             registry.assign<position>(entity);
//         }
//     }

//     auto test = [&registry](auto func) {
//         ENTT_ID_TYPE types[] = {
//             entt::type_info<position>::id(),
//             entt::type_info<velocity>::id(),
//             entt::type_info<comp<0>>::id()
//         };

//         timer timer;
//         registry.runtime_view(std::begin(types), std::end(types)).each(func);
//         timer.elapsed();
//     };

//     test([&registry](auto entity) {
//         registry.get<position>(entity).x = {};
//         registry.get<velocity>(entity).x = {};
//         registry.get<comp<0>>(entity).x = {};
//     });
// }

// TEST(Benchmark, IterateThreeComponentsRuntime1MOne) {
//     entt::registry registry;

//     std::cout << "Iterating over 1000000 entities, three components, only one
//     entity has all the components, runtime view" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<velocity>(entity);
//         registry.assign<comp<0>>(entity);

//         if(i == 500000L) {
//             registry.assign<position>(entity);
//         }
//     }

//     auto test = [&registry](auto func) {
//         ENTT_ID_TYPE types[] = {
//             entt::type_info<position>::id(),
//             entt::type_info<velocity>::id(),
//             entt::type_info<comp<0>>::id()
//         };

//         timer timer;
//         registry.runtime_view(std::begin(types), std::end(types)).each(func);
//         timer.elapsed();
//     };

//     test([&registry](auto entity) {
//         registry.get<position>(entity).x = {};
//         registry.get<velocity>(entity).x = {};
//         registry.get<comp<0>>(entity).x = {};
//     });
// }

// TEST(Benchmark, IterateFiveComponents1M) {
//     entt::registry registry;

//     std::cout << "Iterating over 1000000 entities, five components" <<
//     std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<position>(entity);
//         registry.assign<velocity>(entity);
//         registry.assign<comp<0>>(entity);
//         registry.assign<comp<1>>(entity);
//         registry.assign<comp<2>>(entity);
//     }

//     auto test = [&registry](auto func) {
//         timer timer;
//         registry.view<position, velocity, comp<0>, comp<1>,
//         comp<2>>().each(func); timer.elapsed();
//     };

//     test([](auto &... comp) {
//         ((comp.x = {}), ...);
//     });
// }

// TEST(Benchmark, IterateFiveComponents1MHalf) {
//     entt::registry registry;

//     std::cout << "Iterating over 1000000 entities, five components, half of
//     the entities have all the components" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<velocity>(entity);
//         registry.assign<comp<0>>(entity);
//         registry.assign<comp<1>>(entity);
//         registry.assign<comp<2>>(entity);

//         if(i % 2) {
//             registry.assign<position>(entity);
//         }
//     }

//     auto test = [&registry](auto func) {
//         timer timer;
//         registry.view<position, velocity, comp<0>, comp<1>,
//         comp<2>>().each(func); timer.elapsed();
//     };

//     test([](auto &... comp) {
//         ((comp.x = {}), ...);
//     });
// }

// TEST(Benchmark, IterateFiveComponents1MOne) {
//     entt::registry registry;

//     std::cout << "Iterating over 1000000 entities, five components, only one
//     entity has all the components" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<velocity>(entity);
//         registry.assign<comp<0>>(entity);
//         registry.assign<comp<1>>(entity);
//         registry.assign<comp<2>>(entity);

//         if(i == 500000L) {
//             registry.assign<position>(entity);
//         }
//     }

//     auto test = [&registry](auto func) {
//         timer timer;
//         registry.view<position, velocity, comp<0>, comp<1>,
//         comp<2>>().each(func); timer.elapsed();
//     };

//     test([](auto &... comp) {
//         ((comp.x = {}), ...);
//     });
// }

// TEST(Benchmark, IterateFiveComponentsNonOwningGroup1M) {
//     entt::registry registry;
//     registry.group<>(entt::get<position, velocity, comp<0>, comp<1>,
//     comp<2>>);

//     std::cout << "Iterating over 1000000 entities, five components, non
//     owning group" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<position>(entity);
//         registry.assign<velocity>(entity);
//         registry.assign<comp<0>>(entity);
//         registry.assign<comp<1>>(entity);
//         registry.assign<comp<2>>(entity);
//     }

//     auto test = [&registry](auto func) {
//         timer timer;
//         registry.group<>(entt::get<position, velocity, comp<0>, comp<1>,
//         comp<2>>).each(func); timer.elapsed();
//     };

//     test([](auto &... comp) {
//         ((comp.x = {}), ...);
//     });
// }

// TEST(Benchmark, IterateFiveComponentsFullOwningGroup1M) {
//     entt::registry registry;
//     registry.group<position, velocity, comp<0>, comp<1>, comp<2>>();

//     std::cout << "Iterating over 1000000 entities, five components, full
//     owning group" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<position>(entity);
//         registry.assign<velocity>(entity);
//         registry.assign<comp<0>>(entity);
//         registry.assign<comp<1>>(entity);
//         registry.assign<comp<2>>(entity);
//     }

//     auto test = [&registry](auto func) {
//         timer timer;
//         registry.group<position, velocity, comp<0>, comp<1>,
//         comp<2>>().each(func); timer.elapsed();
//     };

//     test([](auto &... comp) {
//         ((comp.x = {}), ...);
//     });
// }

// TEST(Benchmark, IterateFiveComponentsPartialFourOfFiveOwningGroup1M) {
//     entt::registry registry;
//     registry.group<position, velocity, comp<0>, comp<1>>(entt::get<comp<2>>);

//     std::cout << "Iterating over 1000000 entities, five components, partial
//     (4 of 5) owning group" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<position>(entity);
//         registry.assign<velocity>(entity);
//         registry.assign<comp<0>>(entity);
//         registry.assign<comp<1>>(entity);
//         registry.assign<comp<2>>(entity);
//     }

//     auto test = [&registry](auto func) {
//         timer timer;
//         registry.group<position, velocity, comp<0>,
//         comp<1>>(entt::get<comp<2>>).each(func); timer.elapsed();
//     };

//     test([](auto &... comp) {
//         ((comp.x = {}), ...);
//     });
// }

// TEST(Benchmark, IterateFiveComponentsPartialThreeOfFiveOwningGroup1M) {
//     entt::registry registry;
//     registry.group<position, velocity, comp<0>>(entt::get<comp<1>, comp<2>>);

//     std::cout << "Iterating over 1000000 entities, five components, partial
//     (3 of 5) owning group" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<position>(entity);
//         registry.assign<velocity>(entity);
//         registry.assign<comp<0>>(entity);
//         registry.assign<comp<1>>(entity);
//         registry.assign<comp<2>>(entity);
//     }

//     auto test = [&registry](auto func) {
//         timer timer;
//         registry.group<position, velocity, comp<0>>(entt::get<comp<1>,
//         comp<2>>).each(func); timer.elapsed();
//     };

//     test([](auto &... comp) {
//         ((comp.x = {}), ...);
//     });
// }

// TEST(Benchmark, IterateFiveComponentsRuntime1M) {
//     entt::registry registry;

//     std::cout << "Iterating over 1000000 entities, five components, runtime
//     view" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<position>(entity);
//         registry.assign<velocity>(entity);
//         registry.assign<comp<0>>(entity);
//         registry.assign<comp<1>>(entity);
//         registry.assign<comp<2>>(entity);
//     }

//     auto test = [&registry](auto func) {
//         ENTT_ID_TYPE types[] = {
//             entt::type_info<position>::id(),
//             entt::type_info<velocity>::id(),
//             entt::type_info<comp<0>>::id(),
//             entt::type_info<comp<1>>::id(),
//             entt::type_info<comp<2>>::id()
//         };

//         timer timer;
//         registry.runtime_view(std::begin(types), std::end(types)).each(func);
//         timer.elapsed();
//     };

//     test([&registry](auto entity) {
//         registry.get<position>(entity).x = {};
//         registry.get<velocity>(entity).x = {};
//         registry.get<comp<0>>(entity).x = {};
//         registry.get<comp<1>>(entity).x = {};
//         registry.get<comp<2>>(entity).x = {};
//     });
// }

// TEST(Benchmark, IterateFiveComponentsRuntime1MHalf) {
//     entt::registry registry;

//     std::cout << "Iterating over 1000000 entities, five components, half of
//     the entities have all the components, runtime view" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<velocity>(entity);
//         registry.assign<comp<0>>(entity);
//         registry.assign<comp<1>>(entity);
//         registry.assign<comp<2>>(entity);

//         if(i % 2) {
//             registry.assign<position>(entity);
//         }
//     }

//     auto test = [&registry](auto func) {
//         ENTT_ID_TYPE types[] = {
//             entt::type_info<position>::id(),
//             entt::type_info<velocity>::id(),
//             entt::type_info<comp<0>>::id(),
//             entt::type_info<comp<1>>::id(),
//             entt::type_info<comp<2>>::id()
//         };

//         timer timer;
//         registry.runtime_view(std::begin(types), std::end(types)).each(func);
//         timer.elapsed();
//     };

//     test([&registry](auto entity) {
//         registry.get<position>(entity).x = {};
//         registry.get<velocity>(entity).x = {};
//         registry.get<comp<0>>(entity).x = {};
//         registry.get<comp<1>>(entity).x = {};
//         registry.get<comp<2>>(entity).x = {};
//     });
// }

// TEST(Benchmark, IterateFiveComponentsRuntime1MOne) {
//     entt::registry registry;

//     std::cout << "Iterating over 1000000 entities, five components, only one
//     entity has all the components, runtime view" << std::endl;

//     for(std::uint64_t i = 0; i < 1000000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<velocity>(entity);
//         registry.assign<comp<0>>(entity);
//         registry.assign<comp<1>>(entity);
//         registry.assign<comp<2>>(entity);

//         if(i == 500000L) {
//             registry.assign<position>(entity);
//         }
//     }

//     auto test = [&registry](auto func) {
//         ENTT_ID_TYPE types[] = {
//             entt::type_info<position>::id(),
//             entt::type_info<velocity>::id(),
//             entt::type_info<comp<0>>::id(),
//             entt::type_info<comp<1>>::id(),
//             entt::type_info<comp<2>>::id()
//         };

//         timer timer;
//         registry.runtime_view(std::begin(types), std::end(types)).each(func);
//         timer.elapsed();
//     };

//     test([&registry](auto entity) {
//         registry.get<position>(entity).x = {};
//         registry.get<velocity>(entity).x = {};
//         registry.get<comp<0>>(entity).x = {};
//         registry.get<comp<1>>(entity).x = {};
//         registry.get<comp<2>>(entity).x = {};
//     });
// }

// TEST(Benchmark, IteratePathological) {
//     std::cout << "Pathological case" << std::endl;

//     pathological([](auto &registry, auto func) {
//         timer timer;
//         registry.template view<position, velocity, comp<0>>().each(func);
//         timer.elapsed();
//     });
// }

// TEST(Benchmark, IteratePathologicalNonOwningGroup) {
//     std::cout << "Pathological case (non-owning group)" << std::endl;

//     pathological([](auto &registry, auto func) {
//         auto group = registry.template group<>(entt::get<position, velocity,
//         comp<0>>);

//         timer timer;
//         group.each(func);
//         timer.elapsed();
//     });
// }

// TEST(Benchmark, IteratePathologicalFullOwningGroup) {
//     std::cout << "Pathological case (full-owning group)" << std::endl;

//     pathological([](auto &registry, auto func) {
//         auto group = registry.template group<position, velocity, comp<0>>();

//         timer timer;
//         group.each(func);
//         timer.elapsed();
//     });
// }

// TEST(Benchmark, IteratePathologicalPartialOwningGroup) {
//     std::cout << "Pathological case (partial-owning group)" << std::endl;

//     pathological([](auto &registry, auto func) {
//         auto group = registry.template group<position,
//         velocity>(entt::get<comp<0>>);

//         timer timer;
//         group.each(func);
//         timer.elapsed();
//     });
// }

// TEST(Benchmark, SortSingle) {
//     entt::registry registry;

//     std::cout << "Sort 150000 entities, one component" << std::endl;

//     for(std::uint64_t i = 0; i < 150000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<position>(entity, i, i);
//     }

//     timer timer;

//     registry.sort<position>([](const auto &lhs, const auto &rhs) {
//         return lhs.x < rhs.x && lhs.y < rhs.y;
//     });

//     timer.elapsed();
// }

// TEST(Benchmark, SortMulti) {
//     entt::registry registry;

//     std::cout << "Sort 150000 entities, two components" << std::endl;

//     for(std::uint64_t i = 0; i < 150000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<position>(entity, i, i);
//         registry.assign<velocity>(entity, i, i);
//     }

//     registry.sort<position>([](const auto &lhs, const auto &rhs) {
//         return lhs.x < rhs.x && lhs.y < rhs.y;
//     });

//     timer timer;

//     registry.sort<velocity, position>();

//     timer.elapsed();
// }

// TEST(Benchmark, AlmostSortedStdSort) {
//     entt::registry registry;
//     entt::entity entities[3];

//     std::cout << "Sort 150000 entities, almost sorted, std::sort" <<
//     std::endl;

//     for(std::uint64_t i = 0; i < 150000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<position>(entity, i, i);

//         if(!(i % 50000)) {
//             entities[i / 50000] = entity;
//         }
//     }

//     for(std::uint64_t i = 0; i < 3; ++i) {
//         registry.destroy(entities[i]);
//         const auto entity = registry.create();
//         registry.assign<position>(entity, 50000 * i, 50000 * i);
//     }

//     timer timer;

//     registry.sort<position>([](const auto &lhs, const auto &rhs) {
//         return lhs.x > rhs.x && lhs.y > rhs.y;
//     });

//     timer.elapsed();
// }

// TEST(Benchmark, AlmostSortedInsertionSort) {
//     entt::registry registry;
//     entt::entity entities[3];

//     std::cout << "Sort 150000 entities, almost sorted, insertion sort" <<
//     std::endl;

//     for(std::uint64_t i = 0; i < 150000L; i++) {
//         const auto entity = registry.create();
//         registry.assign<position>(entity, i, i);

//         if(!(i % 50000)) {
//             entities[i / 50000] = entity;
//         }
//     }

//     for(std::uint64_t i = 0; i < 3; ++i) {
//         registry.destroy(entities[i]);
//         const auto entity = registry.create();
//         registry.assign<position>(entity, 50000 * i, 50000 * i);
//     }

//     timer timer;

//     registry.sort<position>([](const auto &lhs, const auto &rhs) {
//         return lhs.x > rhs.x && lhs.y > rhs.y;
//     }, entt::insertion_sort{});

//     timer.elapsed();
// }
