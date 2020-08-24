#include "ComponentsT.h"
#include <ECSpp/internal/Archetype.h>
#include <gtest/gtest.h>

using namespace epp;


template <typename... CompTs>
static void TestArchetype(Archetype const& arch)
{
    ASSERT_TRUE(arch.hasAllOf(IdOfL<CompTs...>()));
    ASSERT_TRUE(arch.hasAnyOf(IdOfL<CompTs...>()) == (sizeof...(CompTs) != 0));
    ASSERT_TRUE((arch.hasAnyOf({ ComponentId(55), IdOf<CompTs>(), ComponentId(8) }) && ...));
    ASSERT_FALSE(arch.hasAnyOf({ ComponentId(5), ComponentId(45), ComponentId(8) }));
    ASSERT_TRUE((arch.has(IdOf<CompTs>()) && ...));
    ASSERT_FALSE(arch.has(ComponentId(8)));

    ASSERT_EQ(arch.getCIds().size(), sizeof...(CompTs));
    ASSERT_TRUE(((std::find(arch.getCIds().begin(), arch.getCIds().end(), IdOf<CompTs>()) !=
                  arch.getCIds().end()) &&
                 ...));
    ASSERT_EQ(arch.getMask(), CMask(IdOfL<CompTs...>()));
}

TEST(Archetype, DefaultConstr)
{
    Archetype arch;
    TestArchetype<>(arch);
}

TEST(Archetype, ListConstr)
{
    {
        Archetype arch(IdOfL<TComp1>());
        TestArchetype<TComp1>(arch);
    }
    {
        Archetype arch(IdOfL<TComp2, TComp4, TComp1>());
        TestArchetype<TComp2, TComp4, TComp1>(arch);
    }
}

TEST(Archetype, AddComponentCId)
{
    Archetype arch;
    arch.addComponent(IdOf<TComp1>());
    TestArchetype<TComp1>(arch);
    arch.addComponent(IdOf<TComp1>());
    TestArchetype<TComp1>(arch);

    arch.addComponent(IdOf<TComp2>());
    TestArchetype<TComp1, TComp2>(arch);
    arch.addComponent(IdOf<TComp2>());
    arch.addComponent(IdOf<TComp1>());
    TestArchetype<TComp1, TComp2>(arch);
}

TEST(Archetype, AddComponentList)
{
    Archetype arch;
    arch.addComponent(IdOfL<TComp1>());
    TestArchetype<TComp1>(arch);
    arch.addComponent(IdOfL<TComp1>());
    TestArchetype<TComp1>(arch);

    arch.addComponent(IdOfL<TComp2>());
    TestArchetype<TComp1, TComp2>(arch);
    arch.addComponent(IdOfL<TComp2>());
    arch.addComponent(IdOfL<TComp1>());
    TestArchetype<TComp1, TComp2>(arch);

    arch.addComponent(IdOfL<TComp1, TComp3, TComp3>());
    TestArchetype<TComp1, TComp2, TComp3>(arch);
    arch.addComponent(IdOfL<TComp1, TComp3, TComp3>());
    arch.addComponent(IdOfL<TComp2>());
    arch.addComponent(IdOfL<TComp1>());
    TestArchetype<TComp1, TComp2, TComp3>(arch);

    arch.addComponent(IdOfL<TComp1, TComp2, TComp3, TComp4>());
    TestArchetype<TComp1, TComp2, TComp3, TComp4>(arch);
    arch.addComponent(IdOfL<TComp1, TComp2, TComp3, TComp4>());
    arch.addComponent(IdOfL<TComp3, TComp3>());
    arch.addComponent(IdOfL<TComp2>());
    arch.addComponent(IdOfL<TComp1>());
    TestArchetype<TComp1, TComp2, TComp3, TComp4>(arch);

    Archetype arch2;
    arch2.addComponent(IdOfL<TComp1, TComp4>());
    TestArchetype<TComp1, TComp4>(arch2);
    arch2.addComponent(IdOfL<TComp1, TComp2, TComp4, TComp3>());
    TestArchetype<TComp1, TComp2, TComp3, TComp4>(arch2);
}

TEST(Archetype, AddComponentTemplate)
{
    Archetype arch;
    arch.addComponent<TComp1>();
    TestArchetype<TComp1>(arch);
    arch.addComponent<TComp1>();
    TestArchetype<TComp1>(arch);

    arch.addComponent<TComp2>();
    TestArchetype<TComp1, TComp2>(arch);
    arch.addComponent<TComp2>();
    arch.addComponent<TComp1>();
    TestArchetype<TComp1, TComp2>(arch);

    arch.addComponent<TComp1, TComp3, TComp3>();
    TestArchetype<TComp1, TComp2, TComp3>(arch);
    arch.addComponent<TComp2, TComp3, TComp3>();
    arch.addComponent<TComp2>();
    arch.addComponent<TComp1>();
    TestArchetype<TComp1, TComp2, TComp3>(arch);

    arch.addComponent<TComp1, TComp2, TComp3, TComp4>();
    TestArchetype<TComp1, TComp2, TComp3, TComp4>(arch);
    arch.addComponent<TComp1, TComp2, TComp3, TComp4>();
    arch.addComponent<TComp3, TComp3>();
    arch.addComponent<TComp2>();
    arch.addComponent<TComp1>();
    TestArchetype<TComp1, TComp2, TComp3, TComp4>(arch);

    Archetype arch2;
    arch2.addComponent<TComp1, TComp4>();
    TestArchetype<TComp1, TComp4>(arch2);
    arch2.addComponent<TComp1, TComp2, TComp4, TComp3>();
    TestArchetype<TComp1, TComp2, TComp3, TComp4>(arch2);
}

TEST(Archetype, RemoveComponentCId)
{
    Archetype arch;
    arch.removeComponent(IdOf<TComp1>());
    TestArchetype<>(arch);

    arch.addComponent<TComp4, TComp1, TComp3>();
    arch.removeComponent(IdOf<TComp1>());
    TestArchetype<TComp4, TComp3>(arch);
    arch.removeComponent(IdOf<TComp1>());
    arch.removeComponent(IdOf<TComp2>());
    TestArchetype<TComp4, TComp3>(arch);

    arch.addComponent<TComp1, TComp2>();
    TestArchetype<TComp1, TComp2, TComp4, TComp3>(arch);

    arch.removeComponent(IdOf<TComp1>());
    arch.removeComponent(IdOf<TComp2>());
    arch.removeComponent(IdOf<TComp3>());
    arch.removeComponent(IdOf<TComp4>());
    TestArchetype<>(arch);
}

TEST(Archetype, RemoveComponentList)
{
    Archetype arch;
    arch.removeComponent(IdOfL<TComp1>());
    TestArchetype<>(arch);

    arch.addComponent<TComp4, TComp1, TComp3>();
    arch.removeComponent(IdOfL<TComp1>());
    TestArchetype<TComp4, TComp3>(arch);
    arch.removeComponent(IdOfL<TComp1, TComp2>());
    arch.removeComponent(IdOfL<TComp1, TComp2>());
    TestArchetype<TComp4, TComp3>(arch);

    arch.addComponent<TComp1, TComp2>();
    TestArchetype<TComp1, TComp2, TComp4, TComp3>(arch);

    arch.removeComponent(IdOfL<TComp1, TComp2, TComp3, TComp4>());
    TestArchetype<>(arch);

    arch.removeComponent(IdOfL<TComp1, TComp2, TComp3, TComp4>());
    TestArchetype<>(arch);
}

TEST(Archetype, RemoveComponentTemplate)
{
    Archetype arch;
    arch.removeComponent<TComp1>();
    TestArchetype<>(arch);

    arch.addComponent<TComp4, TComp1, TComp3>();
    arch.removeComponent<TComp1>();
    TestArchetype<TComp4, TComp3>(arch);
    arch.removeComponent<TComp1, TComp2>();
    arch.removeComponent<TComp1, TComp2>();
    TestArchetype<TComp4, TComp3>(arch);

    arch.addComponent<TComp1, TComp2>();
    TestArchetype<TComp1, TComp2, TComp4, TComp3>(arch);

    arch.removeComponent<TComp1, TComp2, TComp3, TComp4>();
    TestArchetype<>(arch);
    arch.removeComponent<TComp1, TComp2, TComp3, TComp4>();
    TestArchetype<>(arch);
}