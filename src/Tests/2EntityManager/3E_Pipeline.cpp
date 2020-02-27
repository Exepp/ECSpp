#include "ComponentsT.h"
#include <ECSpp/internal/Pipeline.h>
#include <gtest/gtest.h>

using namespace epp;

TEST(Pipeline, Constructor)
{
    EntityManager mgr;
    Pipeline pipline;
    ASSERT_NO_THROW(pipline.run(mgr));
}

TEST(Pipeline, AddSubtask_Run)
{
    EntityManager mgr;
    Archetype arch[] = { Archetype(IdOf<TComp1, TComp3>()),
                         Archetype(IdOf<TComp1, TComp3, TComp2>()),
                         Archetype(IdOf<TComp1, TComp3, TComp4>()) };
    Pipeline pipeline;

    int n = 0;
    auto fn = [& n = std::as_const(n)](EntityCreator&& creator) {
        creator.constructed<TComp1>(TComp1::Arr_t{n, 0, 0});
        creator.constructed<TComp3>(TComp3::Arr_t{ -n, -n, -n }); };

    for (; n < 1024; ++n)
        mgr.spawn(arch[0], fn);
    for (; n < 2 * 1024; ++n)
        mgr.spawn(arch[1], fn);
    for (; n < 3 * 1024; ++n)
        mgr.spawn(arch[2], fn);

    struct Functor {
        int operator()(int x) const { return 120; };
    };

    auto mainTaskFn = [](TaskIterator<TComp1, TComp3> const& it) { it.getComponent<TComp1>().data[0] += 10; ++it.getComponent<TComp1>().data[1]; };
    auto subTaskFn1 = [](TaskIterator<TComp1, TComp3> const& it) { it.getComponent<TComp1>().data[0] *= 7;  ++it.getComponent<TComp1>().data[1]; };
    auto subTaskFn2 = [](TaskIterator<TComp1, TComp3> const& it) { it.getComponent<TComp1>().data[0] -= 3;  ++it.getComponent<TComp1>().data[1]; };
    auto subTaskFn3 = [](TaskIterator<TComp1, TComp3> const& it) { it.getComponent<TComp1>().data[0] *= 13;  ++it.getComponent<TComp1>().data[1]; };
    pipeline.setTask<TComp1, TComp3>(mainTaskFn)
        .setSubtask<TComp1, TComp3>(subTaskFn1)
        .setSubtask<TComp1, TComp3>(subTaskFn2)
        .setSubtask<TComp1, TComp3>(subTaskFn3);
    pipeline.run(mgr);

    Selection<TComp1, TComp3> sel;
    mgr.updateSelection(sel);

    int i = 0;
    for (auto it = sel.begin(), end = sel.end(); it != end; ++it, ++i) {
        ASSERT_EQ(it.getComponent<TComp1>().data[0], (((i + 10) * 7) - 3) * 13);
        ASSERT_EQ(it.getComponent<TComp1>().data[1], 4);
        ASSERT_EQ(it.getComponent<TComp3>().data, TComp3::Arr_t({ -i, -i, -i }));
    }
}