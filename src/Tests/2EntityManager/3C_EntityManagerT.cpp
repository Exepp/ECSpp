#include "ComponentsT.h"
#include <ECSpp/EntityManager.h>
#include <gtest/gtest.h>

using namespace epp;

TEST(EntityManager, Spawn_Creator)
{
    {
        EntityManager mgr;
        Archetype arch(IdOf<TComp1, TComp2>());
        mgr.spawn(arch,
                  [](EntityCreator&& creator) {
                      creator.constructed<TComp2>();
                  });
        ASSERT_EQ(TComp2::AliveCounter, 1); // Creator did not call default constructor on its own
        ASSERT_EQ(TComp1::AliveCounter, 1); // Creator called default constructor on its own

        mgr.spawn(arch,
                  [](EntityCreator&& creator) {
                      creator.constructed<TComp1>();
                      creator.constructed<TComp2>();
                  });
        ASSERT_EQ(TComp2::AliveCounter, 2); // Creator did not call default constructor on its own
        ASSERT_EQ(TComp1::AliveCounter, 2);

        arch = Archetype();
        arch.addComponent<TComp3>();
        for (int i = 0; i < 1e4; ++i)
            mgr.spawn(arch);
        ASSERT_EQ(TComp3::AliveCounter, 1e4);

        for (int i = 0; i < 1e4; ++i)
            mgr.spawn(Archetype()); // empty archetype
        ASSERT_EQ(TComp3::AliveCounter, 1e4);
    }
    { // spawn N
        EntityManager mgr;
        Archetype arch(IdOf<TComp1, TComp2>());
        {
            auto [begin, end] = mgr.spawn(arch, 1, // spawn n = 1
                                          [](EntityCreator&& creator) {
                                              creator.constructed<TComp2>();
                                          });
            ASSERT_EQ(TComp2::AliveCounter, 1); // Creator did not call default constructor on its own
            ASSERT_EQ(TComp1::AliveCounter, 1); // Creator called default constructor on its own
            for (int i = 0; begin != end; ++begin, ++i)
                ASSERT_EQ(*begin, mgr.entitiesOf(arch).data[i]);
        }
        ASSERT_THROW(mgr.entitiesOf(Archetype()), epp::AssertFailed);

        mgr.spawn(arch, 1e5,
                  [](EntityCreator&& creator) {
                      creator.constructed<TComp1>();
                      creator.constructed<TComp2>();
                  });
        ASSERT_EQ(TComp2::AliveCounter, 1 + 1e5); // Creator did not call default constructor on its own
        ASSERT_EQ(TComp1::AliveCounter, 1 + 1e5);

        arch = Archetype();
        arch.addComponent<TComp3>();
        {
            auto [begin, end] = mgr.spawn(arch, 1e4);
            ASSERT_EQ(TComp3::AliveCounter, 1e4);
            ASSERT_EQ(TComp2::AliveCounter, 1 + 1e5); // still same
            ASSERT_EQ(TComp1::AliveCounter, 1 + 1e5);
            for (int i = 0; begin != end; ++begin, ++i)
                ASSERT_EQ(*begin, mgr.entitiesOf(arch).data[i]);
        }

        mgr.spawn(Archetype(), 1e4);
        ASSERT_EQ(TComp3::AliveCounter, 1e4); // still same
        ASSERT_EQ(TComp2::AliveCounter, 1 + 1e5);
        ASSERT_EQ(TComp1::AliveCounter, 1 + 1e5);
    }
}

TEST(EntityManager, PrepareToSpawn)
{
    Archetype arch(IdOfL<TComp1>());
    {
        EntityManager mgr;
        auto ent = mgr.spawn(arch);
        void* ptr = &mgr.componentOf<TComp1>(ent);
        for (int i = 0; i < 16; ++i)
            mgr.spawn(arch);
        ASSERT_NE(ptr, &mgr.componentOf<TComp1>(ent));
    }
    { // now with prepare
        EntityManager mgr;
        mgr.prepareToSpawn(arch, 17);
        auto ent = mgr.spawn(arch);
        void* ptr = &mgr.componentOf<TComp1>(ent);
        for (int i = 0; i < 16; ++i)
            mgr.spawn(arch);
        ASSERT_EQ(ptr, &mgr.componentOf<TComp1>(ent));
    }
    { // on more complex archetype
        arch.addComponent<TComp2, TComp4>();
        EntityManager mgr;
        mgr.prepareToSpawn(arch, 17);
        auto ent = mgr.spawn(arch);
        void* ptr1 = &mgr.componentOf<TComp1>(ent);
        void* ptr2 = &mgr.componentOf<TComp2>(ent);
        void* ptr4 = &mgr.componentOf<TComp4>(ent);
        for (int i = 0; i < 16; ++i)
            mgr.spawn(arch);
        ASSERT_EQ(ptr1, &mgr.componentOf<TComp1>(ent));
        ASSERT_EQ(ptr2, &mgr.componentOf<TComp2>(ent));
        ASSERT_EQ(ptr4, &mgr.componentOf<TComp4>(ent));
    }
}

TEST(EntityManager, Destroy)
{
    EntityManager mgr;
    Archetype arch(IdOf<TComp1, TComp2>());
    Entity ent = mgr.spawn(arch);
    ASSERT_EQ(TComp1::AliveCounter, 1);

    mgr.destroy(ent);
    ASSERT_EQ(TComp1::AliveCounter, 0);
    ASSERT_THROW(mgr.destroy(ent), AssertFailed);      // unvalid (destroyed) entity
    ASSERT_THROW(mgr.destroy(Entity()), AssertFailed); // default entity object (always unvalid)
    ASSERT_EQ(TComp1::AliveCounter, 0);

    // bigger scale & new archetype
    arch.addComponent<TComp4>();
    std::vector<Entity> ents;
    for (int i = 0; i < 1e4; ++i)
        ents.push_back(mgr.spawn(arch));
    ASSERT_EQ(TComp4::AliveCounter, 1e4);

    for (int i = 0; i < 1e4; ++i)
        ASSERT_EQ(ents[i], mgr.entitiesOf(arch).data[i]);
    ASSERT_EQ(ents.size(), mgr.entitiesOf(arch).data.size());

    for (auto ent : ents)
        mgr.destroy(ent);
    ASSERT_EQ(TComp4::AliveCounter, 0);
    ASSERT_EQ(mgr.entitiesOf(arch).data.size(), 0);

    arch.addComponent<TComp3>();
    for (int i = 0; i < 1e4; ++i)
        mgr.destroy(mgr.spawn(arch)); // immediately destroyed
    ASSERT_EQ(TComp4::AliveCounter, 0);
}

TEST(EntityManager, Size)
{
    EntityManager mgr;
    Archetype arch(IdOf<TComp1, TComp2>());

    std::vector<Entity> ents;
    for (int i = 0; i < 1e4; ++i)
        ents.push_back(mgr.spawn(arch));
    ASSERT_EQ(mgr.size(), 1e4);
    ASSERT_EQ(mgr.size(), mgr.size(arch));

    arch.removeComponent<TComp1>();
    for (auto ent : ents)
        mgr.changeArchetype(ent, arch);
    ASSERT_EQ(mgr.size(), 1e4);
    ASSERT_EQ(mgr.size(), mgr.size(arch));

    for (auto ent : ents)
        mgr.destroy(ent);
    ASSERT_EQ(mgr.size(), 0);
    ASSERT_EQ(mgr.size(), mgr.size(arch));

    for (int i = 0; i < 1e4; ++i)
        mgr.spawn(Archetype());
    ASSERT_EQ(mgr.size(), 1e4);
    ASSERT_EQ(mgr.size(), mgr.size(Archetype()));

    mgr.clear();
    ASSERT_EQ(mgr.size(), 0);
    ASSERT_EQ(mgr.size(arch), 0);
    ASSERT_EQ(mgr.size(Archetype()), 0);

    ents.clear();
    Archetype arch2(IdOf<TComp3, TComp4>());
    for (int i = 0; i < 1e4; ++i) {
        ents.push_back(mgr.spawn(Archetype()));
        ents.push_back(mgr.spawn(arch));
        ents.push_back(mgr.spawn(arch2));
    }
    ASSERT_EQ(mgr.size(), 3 * 1e4);
    ASSERT_EQ(mgr.size(Archetype()), 1e4);
    ASSERT_EQ(mgr.size(arch), 1e4);
    ASSERT_EQ(mgr.size(arch2), 1e4);

    for (int i = 0, j = 0; i < 3 * 1e4; i += 3, ++j) {
        ASSERT_EQ(ents[i], mgr.entitiesOf(Archetype()).data[j]);
        ASSERT_EQ(ents[i + 1], mgr.entitiesOf(arch).data[j]);
        ASSERT_EQ(ents[i + 2], mgr.entitiesOf(arch2).data[j]);
    }
    ASSERT_EQ(mgr.size(Archetype()), mgr.entitiesOf(Archetype()).data.size());
    ASSERT_EQ(mgr.size(arch), mgr.entitiesOf(arch).data.size());
    ASSERT_EQ(mgr.size(arch2), mgr.entitiesOf(arch2).data.size());


    for (int i = 0; i < 3 * 1e4 / 2; i += 3) {
        mgr.destroy(ents[i]);
        mgr.destroy(ents[i + 1]);
        mgr.destroy(ents[i + 2]);
    }
    ASSERT_EQ(mgr.size(), 3 * 1e4 / 2);
    ASSERT_EQ(mgr.size(Archetype()), 1e4 / 2);
    ASSERT_EQ(mgr.size(arch), 1e4 / 2);
    ASSERT_EQ(mgr.size(arch2), 1e4 / 2);

    mgr.clear(arch);
    ASSERT_EQ(mgr.size(), 2 * 1e4 / 2);
    ASSERT_EQ(mgr.size(Archetype()), 1e4 / 2);
    ASSERT_EQ(mgr.size(arch), 0);
    ASSERT_EQ(mgr.size(arch2), 1e4 / 2);

    mgr.clear();
    ASSERT_EQ(mgr.size(), 0);
    ASSERT_EQ(mgr.size(Archetype()), 0);
    ASSERT_EQ(mgr.size(arch), 0);
    ASSERT_EQ(mgr.size(arch2), 0);
}

TEST(EntityManager, ComponentOf)
{
    EntityManager mgr;
    Archetype arch(IdOf<TComp1, TComp2>());

    TComp1 tester1(0, 1.f, 2.0);
    TComp2 tester2(TComp2::Arr_t{ 1, 2, 3 });
    TComp4 tester4(TComp2::Arr_t{ 4, 5, 6 });
    ASSERT_NE(tester2, TComp2());
    std::vector<Entity> ents;

    for (int i = 0; i < 1e3; ++i)
        ents.push_back(mgr.spawn(arch, [&](EntityCreator&& creator) {
            creator.constructed<TComp1>(tester1.a + i, tester1.b + i, tester1.c + i);
            creator.constructed<TComp2>(tester2.data);
        }));
    for (int i = 0; i < ents.size(); ++i) {
        ASSERT_EQ(mgr.componentOf<TComp1>(ents[i]), tester1.copy(i));
        ASSERT_EQ(mgr.componentOf<TComp2>(ents[i]), tester2);
    }

    arch.removeComponent<TComp2>();
    for (auto ent : ents)
        mgr.changeArchetype(ent, arch);
    for (int i = 0; i < ents.size(); ++i) {
        ASSERT_EQ(mgr.componentOf<TComp1>(ents[i]), tester1.copy(i));
        ASSERT_THROW(mgr.componentOf<TComp2>(ents[i]), AssertFailed); // removed component
    }

    for (int i = ents.size() - 1; i >= ents.size() / 2; --i) // destroy only half
        mgr.destroy(ents[i]);
    for (int i = 0; i < ents.size() / 2; ++i) {
        ASSERT_EQ(mgr.componentOf<TComp1>(ents[i]), tester1.copy(i));
        ASSERT_THROW(mgr.componentOf<TComp2>(ents[i]), AssertFailed); // removed component
    }
    for (int i = ents.size() / 2; i < ents.size(); ++i) {
        ASSERT_THROW(mgr.componentOf<TComp1>(ents[i]), AssertFailed); // invalid entity
        ASSERT_THROW(mgr.componentOf<TComp2>(ents[i]), AssertFailed);
    }

    ents.clear();
    for (int i = 0; i < 1e3; ++i) {
        mgr.spawn(Archetype());
        ents.push_back(mgr.spawn(arch,
                                 [&](EntityCreator&& creator) {
                                     creator.constructed<TComp1>(tester1.a + i, tester1.b + i, tester1.c + i);
                                 }));
        ents.push_back(mgr.spawn(Archetype(IdOf<TComp1, TComp4>()),
                                 [&](EntityCreator&& creator) {
                                     creator.constructed<TComp1>(tester1.a + 2 * i, tester1.b + 2 * i, tester1.c + 2 * i);
                                     creator.constructed<TComp4>(tester4.data);
                                 }));
    }
    for (int i = 0; i < ents.size(); i += 2) {
        ASSERT_EQ(mgr.componentOf<TComp1>(ents[i]), tester1.copy(i / 2));
        ASSERT_EQ(mgr.componentOf<TComp1>(ents[i + 1]), tester1.copy(i / 2 * 2));
        ASSERT_EQ(mgr.componentOf<TComp4>(ents[i + 1]), tester4);
    }
    mgr.clear(arch);
    for (int i = 0; i < ents.size(); i += 2) {
        ASSERT_THROW(mgr.componentOf<TComp1>(ents[i]), AssertFailed); // invalid entity
        ASSERT_EQ(mgr.componentOf<TComp1>(ents[i + 1]), tester1.copy(i / 2 * 2));
        ASSERT_EQ(mgr.componentOf<TComp4>(ents[i + 1]), tester4);
    }

    mgr.clear();
    for (int i = 0; i < ents.size(); i += 2) {
        ASSERT_THROW(mgr.componentOf<TComp1>(ents[i]), AssertFailed); // invalid entity
        ASSERT_THROW(mgr.componentOf<TComp1>(ents[i + 1]), AssertFailed);
        ASSERT_THROW(mgr.componentOf<TComp4>(ents[i + 1]), AssertFailed);
    }
}

TEST(EntityManager, MaskOf)
{
    EntityManager mgr;
    Archetype arch(IdOf<TComp1, TComp2>());
    Entity ent = mgr.spawn(arch);

    ASSERT_EQ(mgr.maskOf(ent), arch.getMask());

    mgr.changeArchetype(ent, arch); // same archetype
    ASSERT_EQ(mgr.maskOf(ent), arch.getMask());

    mgr.changeArchetype(ent, arch.addComponent<TComp3>());
    ASSERT_EQ(mgr.maskOf(ent), arch.getMask());

    mgr.changeArchetype(ent, arch.addComponent<TComp4>().removeComponent<TComp1, TComp3>());
    ASSERT_EQ(mgr.maskOf(ent), arch.getMask());

    mgr.destroy(ent);
    ASSERT_THROW(mgr.maskOf(ent), AssertFailed);

    ent = mgr.spawn(arch);
    mgr.clear();
    ASSERT_THROW(mgr.maskOf(ent), AssertFailed);
}

TEST(EntityManager, ArchetypeOf)
{
    EntityManager mgr;
    Archetype arch(IdOf<TComp1, TComp2>());
    Entity ent = mgr.spawn(arch);

    ASSERT_EQ(mgr.archetypeOf(ent).getMask(), arch.getMask());

    mgr.changeArchetype(ent, arch); // same archetype
    ASSERT_EQ(mgr.archetypeOf(ent).getMask(), arch.getMask());

    mgr.changeArchetype(ent, arch.addComponent<TComp3>());
    ASSERT_EQ(mgr.archetypeOf(ent).getMask(), arch.getMask());

    mgr.changeArchetype(ent, arch.addComponent<TComp4>().removeComponent<TComp1, TComp3>());
    ASSERT_EQ(mgr.archetypeOf(ent).getMask(), arch.getMask());

    mgr.destroy(ent);
    ASSERT_THROW(mgr.archetypeOf(ent), AssertFailed);

    ent = mgr.spawn(arch);
    mgr.clear();
    ASSERT_THROW(mgr.archetypeOf(ent), AssertFailed);
}

TEST(EntityManager, ChangeArchetype_IsValid)
{
    {
        EntityManager mgr;
        Archetype arch(IdOf<TComp1, TComp2>());
        Entity ent = mgr.spawn(arch,
                               [](EntityCreator&& creator) {
                                   creator.constructed<TComp1>(4321, 43.21f, 4.321);
                               });
        ASSERT_TRUE(mgr.isValid(ent));

        mgr.changeArchetype(ent, arch); // to the same archetype
        ASSERT_EQ(TComp1::AliveCounter, 1);
        ASSERT_EQ(TComp2::AliveCounter, 1);
        ASSERT_EQ(TComp2::AliveCounter, 1);
        ASSERT_TRUE(mgr.isValid(ent));
        ASSERT_EQ(mgr.componentOf<TComp1>(ent), TComp1(4321, 43.21f, 4.321));

        mgr.changeArchetype(ent, arch.addComponent<TComp3>());
        ASSERT_EQ(TComp1::AliveCounter, 1);
        ASSERT_EQ(TComp2::AliveCounter, 1);
        ASSERT_EQ(TComp3::AliveCounter, 1);
        ASSERT_TRUE(mgr.isValid(ent));
        ASSERT_EQ(mgr.componentOf<TComp1>(ent), TComp1(4321, 43.21f, 4.321));

        mgr.changeArchetype(ent, arch.addComponent<TComp4>().removeComponent<TComp1, TComp3>());
        ASSERT_EQ(TComp1::AliveCounter, 0);
        ASSERT_EQ(TComp2::AliveCounter, 1);
        ASSERT_EQ(TComp3::AliveCounter, 0);
        ASSERT_EQ(TComp4::AliveCounter, 1);
        ASSERT_TRUE(mgr.isValid(ent));

        mgr.destroy(ent);
        ASSERT_FALSE(mgr.isValid(ent));
        ASSERT_THROW(mgr.changeArchetype(ent, arch.addComponent<TComp1>().removeComponent<TComp4>()), AssertFailed);

        ent = mgr.spawn(arch);
        mgr.clear();
        ASSERT_FALSE(mgr.isValid(ent));
        ASSERT_THROW(mgr.changeArchetype(ent, arch.addComponent<TComp1>().removeComponent<TComp4>()), AssertFailed);
    }
    {
        epp::EntityManager mgr;
        epp::Archetype archMissing = epp::Archetype(epp::IdOf<TComp1, TComp2>());
        epp::Archetype archFull = epp::Archetype(epp::IdOf<TComp1, TComp2, TComp3, TComp4>());
        mgr.spawn(archMissing, 1e4, [](epp::EntityCreator&& creator) { creator.constructed<TComp1>().a = creator.idx.value; });
        mgr.changeArchetype(archMissing, archFull, [](epp::EntityCreator&& creator) { creator.constructed<TComp4>().data[0] = 2 * creator.idx.value; });
        epp::EntityCollection<TComp1, TComp2, TComp3, TComp4> coll;
        mgr.updateCollection(coll);
        int i = 0;
        for (auto it = coll.begin(), end = coll.end(); it != end; ++it, ++i) {
            ASSERT_EQ(it.getComponent<TComp1>().a, i);
            ASSERT_EQ(2 * it.getComponent<TComp1>().a, it.getComponent<TComp4>().data[0]);
        }
    }
}
// no need for clear tests
// updateCollection test with EntityCollection tests