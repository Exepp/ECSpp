#include <ECSpp/ECSWorld.h>
#include <benchmark/benchmark.h>

using namespace epp;


struct PositionComponent : public Component
{
    float x, y;
};

struct DirectionComponent : public Component
{
    float x, y;
};

struct ComflabulationComponent : public Component
{
    float       thingy;
    int         dingy;
    std::string stringy;
    bool        mingy;
};


static void BM_EntitiesCreateDestroy(benchmark::State& state)
{
    EntityManager manager;
    manager.registerArchetype(MakeArchetype<PositionComponent, DirectionComponent>());
    for (auto _ : state)
    {
        manager.spawn(MakeArchetype<PositionComponent, DirectionComponent>(), state.range(0));
        manager.clear();
    }
}
BENCHMARK(BM_EntitiesCreateDestroy)->Range(8, 1e6);

static void BM_EntitiesIteration_2M(benchmark::State& state)
{
    EntityManager manager;
    manager.spawn(MakeArchetype<PositionComponent, DirectionComponent, ComflabulationComponent>(), size_t(2e6));
    manager.acceptSpawnedEntities();
    EntityCollection<PositionComponent, DirectionComponent, ComflabulationComponent> group;
    manager.requestCGroup(group);

    for (auto _ : state)
    {
        for (auto entity : group)
        {
            entity.get<PositionComponent&>().x += entity.get<DirectionComponent&>().x * 0.016f;
            entity.get<PositionComponent&>().y += entity.get<DirectionComponent&>().y * 0.016f;
        }

        for (auto entity : group)
        {
            entity.get<ComflabulationComponent&>().thingy *= 1.000001f;
            entity.get<ComflabulationComponent&>().mingy = !entity.get<ComflabulationComponent&>().mingy;
            entity.get<ComflabulationComponent&>().dingy++;
        }
    }
}

BENCHMARK(BM_EntitiesIteration_2M);

static void BM_AddingRemovingComponent(benchmark::State& state)
{
    // removing and adding operations have the same performance (also almost indentical code)
    EntityManager manager;
    auto          entity = manager.spawn(MakeArchetype<PositionComponent>());
    manager.acceptSpawnedEntities();
    for (auto _ : state)
        manager.addComponent<DirectionComponent, ComflabulationComponent>(entity);
}
BENCHMARK(BM_AddingRemovingComponent);

static void BM_UsingReference_GettingComponent(benchmark::State& state)
{
    // removing and adding operations have the same performance (also almost indentical code)
    EntityManager manager;
    auto          entity = manager.spawn(MakeArchetype<PositionComponent>());
    manager.acceptSpawnedEntities();
    for (auto _ : state)
    {
        entity.getComponent<PositionComponent>();
    }
}
BENCHMARK(BM_UsingReference_GettingComponent);

BENCHMARK_MAIN();