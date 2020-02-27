#include "ComponentsT.h"
#include <ECSpp/EntityManager.h>
#include <algorithm>
#include <gtest/gtest.h>

using namespace epp;

template <typename ValidComp, typename InvalidComp>
static void TestEntityManager(EntityManager& mgr, std::size_t const sizeAll,
                              Archetype const& arch,
                              std::vector<Entity> const& entsOfArch, Entity entToCheck = Entity(),
                              typename TCompBase<-1>::Arr_t const& correct = {})
{
    ASSERT_EQ(mgr.size(arch), entsOfArch.size());
    ASSERT_EQ(mgr.size(), sizeAll);
    ASSERT_EQ(mgr.entitiesOf(arch).data, entsOfArch);
    ASSERT_THROW(mgr.entitiesOf(Archetype(IdOfL<TComp4>())), AssertFailed); // assuming archetype with only TComp4 is never used
    ASSERT_THROW(mgr.archetypeOf(Entity()), AssertFailed);
    ASSERT_THROW(mgr.maskOf(Entity()), AssertFailed);
    if constexpr (!std::is_same_v<ValidComp, void>) {
        ASSERT_THROW(mgr.componentOf<ValidComp>(Entity()), AssertFailed);
        ASSERT_GE(ValidComp::AliveCounter, entsOfArch.size());
        ASSERT_LE(ValidComp::AliveCounter, sizeAll);
    }
    if (entsOfArch.size()) {
        ASSERT_THROW(mgr.componentOf<InvalidComp>(entsOfArch.front()), AssertFailed);
        if constexpr (!std::is_same_v<ValidComp, void>)
            if (entToCheck != Entity()) {
                ASSERT_TRUE(mgr.isValid(entToCheck));
                ASSERT_EQ(entsOfArch[mgr.cellOf(entToCheck).poolIdx.value], entToCheck);
                ASSERT_EQ(mgr.componentOf<ValidComp>(entToCheck), ValidComp(correct));
            }
    }

    for (auto ent : entsOfArch) {
        ASSERT_TRUE(mgr.isValid(ent));
        ASSERT_EQ(mgr.archetypeOf(ent).getMask(), arch.getMask());
        ASSERT_EQ(mgr.maskOf(ent), arch.getMask());
        if constexpr (!std::is_same_v<ValidComp, void>)
            ASSERT_NO_THROW(mgr.componentOf<ValidComp>(ent));
    }
}

TEST(EntityManager, Constructor)
{
    EntityManager mgr;
    Archetype arch(IdOf<TComp3, TComp4>());
    ASSERT_THROW((TestEntityManager<TComp3, TComp1>(mgr, 0, arch, {})), AssertFailed);
}

TEST(EntityManager, Spawn_Creator)
{
    EntityManager mgr;
    Archetype arch(IdOf<TComp1, TComp2>());
    std::vector<Entity> ents;
    ents.push_back(mgr.spawn(arch));
    TestEntityManager<TComp1, TComp3>(mgr, 1, arch, ents, {});
    TestEntityManager<TComp2, TComp3>(mgr, 1, arch, ents, {});

    ents.push_back(mgr.spawn(arch,
                             [](EntityCreator&& creator) {
                                 creator.constructed<TComp1>(TComp1::Arr_t({ 5, 3, 2 }));
                                 creator.constructed<TComp2>();
                             }));
    ents.push_back(mgr.spawn(arch,
                             [](EntityCreator&& creator) {
                                 creator.constructed<TComp1>(TComp1::Arr_t({ 6, 3, 2 }));
                                 creator.constructed<TComp2>();
                             }));
    TestEntityManager<TComp1, TComp3>(mgr, 3, arch, ents, ents[2], { 6, 3, 2 });
    TestEntityManager<TComp1, TComp3>(mgr, 3, arch, ents, ents[1], { 5, 3, 2 });
    TestEntityManager<TComp2, TComp3>(mgr, 3, arch, ents, ents[0], {});

    ents.clear();
    arch = Archetype();
    arch.addComponent<TComp3>();
    for (int i = 0; i < 1e4; ++i)
        ents.push_back(mgr.spawn(arch));
    ents.push_back(mgr.spawn(arch,
                             [](EntityCreator&& creator) {
                                 creator.constructed<TComp3>(TComp3::Arr_t({ 1, 10, 2 }));
                             }));
    for (int i = 0; i < 1e4; ++i)
        ents.push_back(mgr.spawn(arch));

    TestEntityManager<TComp3, TComp1>(mgr, 4 + 2e4, arch, ents, ents[1e4], { 1, 10, 2 });

    decltype(ents) emptyArchEnts;
    for (int i = 0; i < 1e4; ++i) // empty archetype
        emptyArchEnts.push_back(mgr.spawn(Archetype()));
    TestEntityManager<void, TComp1>(mgr, 4 + 3e4, Archetype(), emptyArchEnts);
    TestEntityManager<TComp3, TComp1>(mgr, 4 + 3e4, arch, ents, ents[1e4], { 1, 10, 2 }); // no changes
}

TEST(EntityManager, SpawnN)
{
    EntityManager mgr;
    Archetype arch(IdOf<TComp1, TComp2>());
    {
        auto [begin, end] = mgr.spawn(arch, 1, // spawn n = 1
                                      [](EntityCreator&& creator) {
                                          creator.constructed<TComp2>(TComp2::Arr_t{ 3, 3, 3 });
                                      });
        TestEntityManager<TComp2, TComp3>(mgr, 1, arch, { *begin }, *begin, { 3, 3, 3 });
    }
    {
        int cnt = 0;
        auto [begin, end] = mgr.spawn(arch, 1e5,
                                      [&cnt](EntityCreator&& creator) {
                                          if (cnt++ == 1e3)
                                              creator.constructed<TComp1>(TComp1::Arr_t{ 1, 2, 0 });
                                          else
                                              creator.constructed<TComp1>(TComp1::Arr_t{ 9, 9, 9 });
                                      });
        TestEntityManager<TComp1, TComp3>(mgr, 1 + 1e5, arch, { begin - 1, end }, *(begin + 1e3), { 1, 2, 0 });
        TestEntityManager<TComp1, TComp3>(mgr, 1 + 1e5, arch, { begin - 1, end }, *(begin + 1e4), { 9, 9, 9 });
    }
    {
        arch = Archetype();
        arch.addComponent<TComp3>();

        auto [begin, end] = mgr.spawn(arch, 1e4);
        TestEntityManager<TComp3, TComp1>(mgr, 1 + 1e5 + 1e4, arch, { begin, end });
    }
    {
        auto [begin, end] = mgr.spawn(Archetype(), 1e4);
        TestEntityManager<void, TComp1>(mgr, 1 + 1e5 + 1e4 + 1e4, Archetype(), { begin, end });
    }
}

TEST(EntityManager, ChangeArchetypeEntity)
{
    EntityManager mgr;
    Archetype archFrom(IdOf<TComp3, TComp4>());
    Archetype archTo(IdOf<TComp3, TComp2>());

    auto ent = mgr.spawn(archFrom, [](EntityCreator&& cr) { cr.constructed<TComp3>(TComp3::Arr_t{ 444, 44, 4 }); });
    mgr.changeArchetype(ent, archFrom);
    TestEntityManager<TComp3, TComp1>(mgr, 1, archFrom, { ent }, ent, { 444, 44, 4 });

    mgr.changeArchetype(ent, archTo);
    TestEntityManager<TComp3, TComp1>(mgr, 1, archFrom, {});
    TestEntityManager<TComp3, TComp1>(mgr, 1, archTo, { ent }, ent, { 444, 44, 4 });

    int cnt = 0;
    auto [begin, end] = mgr.spawn(archTo, 1e4, [&cnt](EntityCreator&& cr) { if(cnt++ == 1e3) cr.constructed<TComp3>(TComp3::Arr_t{ 222, 22, 2 }); });
    std::vector<Entity> ents(begin - 1, end);
    TestEntityManager<TComp3, TComp1>(mgr, 1 + 1e4, archFrom, {});
    TestEntityManager<TComp3, TComp1>(mgr, 1 + 1e4, archTo, ents, ent, { 444, 44, 4 });
    TestEntityManager<TComp3, TComp1>(mgr, 1 + 1e4, archTo, ents, *(begin + 1e3), { 222, 22, 2 });
    TestEntityManager<TComp3, TComp1>(mgr, 1 + 1e4, archTo, ents, *(begin + 1e3 + 1), {});

    for (auto entity : ents) {
        mgr.changeArchetype(entity, archFrom);
    }
    TestEntityManager<TComp3, TComp1>(mgr, 1 + 1e4, archFrom, ents, ent, { 444, 44, 4 });
    TestEntityManager<TComp3, TComp1>(mgr, 1 + 1e4, archFrom, ents, ents[1 + 1e3], { 222, 22, 2 });
    TestEntityManager<TComp3, TComp1>(mgr, 1 + 1e4, archFrom, ents, ents[1 + 1e3 + 1], {});
    TestEntityManager<TComp3, TComp1>(mgr, 1 + 1e4, archTo, {});
}

TEST(EntityManager, ChangeArchetypeEntityDifference)
{
    EntityManager mgr;
    Archetype archFrom(IdOf<TComp3, TComp4>());
    auto compsRemoved = IdOfL<TComp4>();
    auto compsAdded = IdOfL<TComp2>();
    Archetype archTo = archFrom;
    archTo.removeComponent(compsRemoved).addComponent(compsAdded);

    auto ent = mgr.spawn(archFrom, [](EntityCreator&& cr) { cr.constructed<TComp3>(TComp3::Arr_t{ 444, 44, 4 }); });
    mgr.changeArchetype(ent, compsRemoved, compsRemoved); // remove and add the same one
    TestEntityManager<TComp3, TComp1>(mgr, 1, archFrom, { ent }, ent, { 444, 44, 4 });

    mgr.changeArchetype(ent, compsRemoved, compsAdded);
    TestEntityManager<TComp3, TComp1>(mgr, 1, archFrom, {});
    TestEntityManager<TComp3, TComp1>(mgr, 1, archTo, { ent }, ent, { 444, 44, 4 });

    int cnt = 0;
    auto [begin, end] = mgr.spawn(archTo, 1e4, [&cnt](EntityCreator&& cr) { if(cnt++ == 1e3) cr.constructed<TComp3>(TComp3::Arr_t{ 222, 22, 2 }); });
    std::vector<Entity> ents(begin - 1, end);
    TestEntityManager<TComp3, TComp1>(mgr, 1 + 1e4, archFrom, {});
    TestEntityManager<TComp3, TComp1>(mgr, 1 + 1e4, archTo, ents, ent, { 444, 44, 4 });
    TestEntityManager<TComp3, TComp1>(mgr, 1 + 1e4, archTo, ents, *(begin + 1e3), { 222, 22, 2 });
    TestEntityManager<TComp3, TComp1>(mgr, 1 + 1e4, archTo, ents, *(begin + 1e3 + 1), {});

    for (auto entity : ents) {
        mgr.changeArchetype(entity, compsAdded, compsRemoved);
    }
    TestEntityManager<TComp3, TComp1>(mgr, 1 + 1e4, archFrom, ents, ent, { 444, 44, 4 });
    TestEntityManager<TComp3, TComp1>(mgr, 1 + 1e4, archFrom, ents, ents[1 + 1e3], { 222, 22, 2 });
    TestEntityManager<TComp3, TComp1>(mgr, 1 + 1e4, archFrom, ents, ents[1 + 1e3 + 1], {});
    TestEntityManager<TComp3, TComp1>(mgr, 1 + 1e4, archTo, {});
}

TEST(EntityManager, ChangeArchetypeSelectionIterator)
{
    EntityManager mgr;
    Archetype archFrom(IdOf<TComp3, TComp4>());
    Archetype archTo(IdOf<TComp3, TComp2>());
    std::vector<Entity> ents;
    Selection<TComp3, TComp4> sel1;
    Selection<TComp3, TComp2> sel2;

    auto [beg1, end1] = mgr.spawn(archFrom, 123, [](EntityCreator&& cr) { cr.constructed<TComp3>(TComp3::Arr_t{ 444, 44, 4 }); });
    mgr.updateSelection(sel1);
    ents = { beg1, end1 };

    for (auto it = sel1.begin(), end = sel1.end(); it != end;)
        it = mgr.changeArchetype(it, archFrom); // does nothing
    TestEntityManager<TComp3, TComp1>(mgr, 123, archFrom, ents, ents.front(), { 444, 44, 4 });

    for (auto it = sel1.begin(), end = sel1.end(); it != end;)
        it = mgr.changeArchetype(it, archTo);
    std::reverse(ents.begin(), ents.end());
    ents.insert(ents.begin(), ents.back());
    ents.pop_back();
    TestEntityManager<TComp3, TComp1>(mgr, 123, archFrom, {});
    TestEntityManager<TComp3, TComp1>(mgr, 123, archTo, ents, ents.front(), { 444, 44, 4 });

    int cnt = 0;
    auto [beg2, end2] = mgr.spawn(archTo, 1e4, [&cnt](EntityCreator&& cr) { if(cnt++ == 1e3) cr.constructed<TComp3>(TComp3::Arr_t{ 222, 22, 2 }); });
    ents = { beg2 - 123, end2 };
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archFrom, {});
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archTo, ents, ents.front(), { 444, 44, 4 });
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archTo, ents, *(beg2 + 1e3), { 222, 22, 2 });
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archTo, ents, *(beg2 + 1e3 + 1), {});

    mgr.updateSelection(sel2);
    for (auto it = sel2.begin(), end = sel2.end(); it != end;)
        it = mgr.changeArchetype(it, archFrom);
    std::reverse(ents.begin(), ents.end());
    ents.insert(ents.begin(), ents.back());
    ents.pop_back();
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archFrom, ents, ents.front(), { 444, 44, 4 });
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archFrom, ents, ents[1e4 - 1e3], { 222, 22, 2 });
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archFrom, ents, ents[1e4], {});
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archTo, {});
}

TEST(EntityManager, ChangeArchetypeEntityPoolIterator)
{
    EntityManager mgr;
    Archetype archFrom(IdOf<TComp3, TComp4>());
    Archetype archTo(IdOf<TComp3, TComp2>());
    std::vector<Entity> ents;

    auto [beg1, end1] = mgr.spawn(archFrom, 123, [](EntityCreator&& cr) { cr.constructed<TComp3>(TComp3::Arr_t{ 444, 44, 4 }); });
    ents = { beg1, end1 };

    for (auto it = ents.begin(), end = ents.end(); it != end; ++it)
        mgr.changeArchetype(it, archFrom); // does nothing
    TestEntityManager<TComp3, TComp1>(mgr, 123, archFrom, ents, ents.front(), { 444, 44, 4 });

    for (auto it = ents.begin(), end = ents.end(); it != end; ++it)
        mgr.changeArchetype(it, archTo);
    TestEntityManager<TComp3, TComp1>(mgr, 123, archFrom, {});
    TestEntityManager<TComp3, TComp1>(mgr, 123, archTo, ents, ents.front(), { 444, 44, 4 });

    int cnt = 0;
    auto [beg2, end2] = mgr.spawn(archTo, 1e4, [&cnt](EntityCreator&& cr) { if(cnt++ == 1e3) cr.constructed<TComp3>(TComp3::Arr_t{ 222, 22, 2 }); });
    ents = { beg2 - 123, end2 };
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archFrom, {});
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archTo, ents, ents.front(), { 444, 44, 4 });
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archTo, ents, *(beg2 + 1e3), { 222, 22, 2 });
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archTo, ents, *(beg2 + 1e3 + 1), {});

    for (auto it = ents.begin(), end = ents.end(); it != end; ++it)
        mgr.changeArchetype(it, archFrom);
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archFrom, ents, ents.front(), { 444, 44, 4 });
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archFrom, ents, *(ents.begin() + 123 + 1e3), { 222, 22, 2 });
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archFrom, ents, *(ents.begin() + 123 + 1e3 + 1), {});
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archTo, {});

    for (auto it = mgr.entitiesOf(archFrom).data.begin(), end = mgr.entitiesOf(archFrom).data.end(); it != end;)
        it = mgr.changeArchetype(it, archTo);
    std::reverse(ents.begin(), ents.end());
    ents.insert(ents.begin(), ents.back());
    ents.pop_back();
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archFrom, {});
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archTo, ents, ents.front(), { 444, 44, 4 });
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archTo, ents, ents[1e4 - 1e3], { 222, 22, 2 });
    TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e4, archTo, ents, ents[1e4], {});
}

TEST(EntityManager, DestroyEntity)
{
    EntityManager mgr;
    Archetype arch(IdOf<TComp1, TComp2>());
    ASSERT_THROW(mgr.destroy(Entity()), AssertFailed);

    Entity ent = mgr.spawn(arch);
    mgr.destroy(ent);
    TestEntityManager<TComp2, TComp3>(mgr, 0, arch, {});
    ASSERT_THROW(mgr.destroy(ent), AssertFailed);      // invalid (destroyed) entity
    ASSERT_THROW(mgr.destroy(Entity()), AssertFailed); // still throws

    // bigger scale & new archetype
    arch.addComponent<TComp4>();
    std::vector<Entity> ents;
    for (int i = 0; i < 1e4; ++i)
        ents.push_back(mgr.spawn(arch));
    TestEntityManager<TComp4, TComp3>(mgr, 1e4, arch, ents);

    for (auto ent : ents)
        mgr.destroy(ent);
    TestEntityManager<TComp4, TComp3>(mgr, 0, arch, {});

    arch.addComponent<TComp3>().removeComponent<TComp1>();
    for (int i = 0; i < 1e4; ++i)
        mgr.destroy(mgr.spawn(arch)); // immediately destroyed
    TestEntityManager<TComp4, TComp3>(mgr, 0, arch, {});
}

TEST(EntityManager, DestroySelectionIterator)
{
    EntityManager mgr;
    Archetype arch(IdOf<TComp1, TComp2>());
    Selection<TComp1, TComp2> sel(IdOfL<TComp4>());
    ASSERT_THROW(mgr.destroy(sel.begin()), AssertFailed); // without update & invalid

    Entity ent = mgr.spawn(arch);
    mgr.updateSelection(sel);

    mgr.destroy(sel.begin());
    TestEntityManager<TComp2, TComp3>(mgr, 0, arch, {});
    ASSERT_THROW(mgr.destroy(sel.begin()), AssertFailed); // invalid iterator

    // bigger scale & new archetype
    arch.addComponent<TComp4>();
    std::vector<Entity> ents;
    for (int i = 0; i < 1e4; ++i)
        ents.push_back(mgr.spawn(arch));
    TestEntityManager<TComp4, TComp3>(mgr, 1e4, arch, ents);

    Selection<TComp1, TComp2, TComp4> sel2;
    mgr.updateSelection(sel2);
    for (auto it = sel2.begin(), end = sel2.end(); it != end;)
        it = mgr.destroy(it);
    TestEntityManager<TComp4, TComp3>(mgr, 0, arch, {});
}

TEST(EntityManager, DestroyEntityPoolIterator)
{
    EntityManager mgr;
    Archetype arch(IdOf<TComp1, TComp2>());
    ASSERT_THROW(mgr.destroy(mgr.entitiesOf(arch).data.begin()), AssertFailed); // no entities of that archetype yet

    Entity ent = mgr.spawn(arch);
    mgr.destroy(mgr.entitiesOf(arch).data.begin());
    TestEntityManager<TComp2, TComp3>(mgr, 0, arch, {});

    // bigger scale & new archetype
    arch.addComponent<TComp4>();
    std::vector<Entity> ents;
    for (int i = 0; i < 1e4; ++i)
        ents.push_back(mgr.spawn(arch));
    TestEntityManager<TComp4, TComp3>(mgr, 1e4, arch, ents);

    for (auto it = mgr.entitiesOf(arch).data.begin(); it != mgr.entitiesOf(arch).data.end();)
        it = mgr.destroy(it);
    TestEntityManager<TComp4, TComp3>(mgr, 0, arch, {});
}

TEST(EntityManager, PrepareToSpawn)
{
    Archetype arch(IdOfL<TComp1>());
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

// TEST(EntityManager, ChangeArchetypeOfWholeSpawner)
// {
//     EntityManager mgr;
//     Archetype archFrom(IdOf<TComp3, TComp4>());
//     Archetype archTo(IdOf<TComp3, TComp2>());
//     std::vector<Entity> ents;

//     // works only for already used archetypes
//     ASSERT_THROW(mgr.changeArchetype(archFrom, archFrom), AssertFailed);
//     ASSERT_THROW(mgr.changeArchetype(archTo, archTo), AssertFailed);
//     ASSERT_THROW(mgr.changeArchetype(archFrom, archTo), AssertFailed);
//     ASSERT_THROW(mgr.changeArchetype(archTo, archFrom), AssertFailed);

//     auto [beg1, end1] = mgr.spawn(archFrom, 123, [](EntityCreator&& cr) { cr.constructed<TComp3>(TComp3::Arr_t{ 444, 44, 4 }); });
//     ents = { beg1, end1 };

//     mgr.changeArchetype(archFrom, archFrom); // does nothing
//     TestEntityManager<TComp3, TComp1>(mgr, 123, archFrom, ents, ents.front(), { 444, 44, 4 });

//     mgr.changeArchetype(archFrom, archTo);
//     std::reverse(ents.begin(), ents.end());
//     ents.insert(ents.begin(), ents.back());
//     ents.pop_back();
//     TestEntityManager<TComp3, TComp1>(mgr, 123, archFrom, {});
//     TestEntityManager<TComp3, TComp1>(mgr, 123, archTo, ents, ents.front(), { 444, 44, 4 });

//     int cnt = 0;
//     auto [beg2, end2] = mgr.spawn(archTo, 1e3, [&cnt](EntityCreator&& cr) { if(cnt++ == 2e2) cr.constructed<TComp3>(TComp3::Arr_t{ 222, 22, 2 }); });
//     ents = { beg2 - 123, end2 };
//     TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e3, archFrom, {});
//     TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e3, archTo, ents, ents.front(), { 444, 44, 4 });
//     TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e3, archTo, ents, *(beg2 + 2e2), { 222, 22, 2 });
//     TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e3, archTo, ents, *(beg2 + 1e3 + 1), {});

//     mgr.changeArchetype(archTo, archFrom);
//     std::reverse(ents.begin(), ents.end());
//     ents.insert(ents.begin(), ents.back());
//     ents.pop_back();

//     TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e3, archFrom, ents, ents.front(), { 444, 44, 4 });
//     TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e3, archFrom, ents, ents[1e3 - 2e2], { 222, 22, 2 });
//     TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e3, archFrom, ents, ents[1e3], {});
//     TestEntityManager<TComp3, TComp1>(mgr, 123 + 1e3, archTo, {});
// }