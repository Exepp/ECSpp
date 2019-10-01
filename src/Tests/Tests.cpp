// #include <chrono>
// #include <gtest/gtest.h>


// //#include "vld.h"

// using namespace epp;


// //////////////////////////////////////////////////////////////////////////////////////////////////////////


// TEST(ArchetypeTest, Add_Remove)
// {
//     Archetype archeT;
//     ASSERT_TRUE((archeT.addComponent<TComp1>()));
//     ASSERT_TRUE((archeT.addComponent<TComp2, TComp3>()));
//     ASSERT_FALSE((archeT.addComponent<TComp1>()));
//     ASSERT_FALSE((archeT.addComponent<TComp1, TComp4>()));
//     ASSERT_TRUE((archeT.hasComponent<TComp1, TComp2, TComp3, TComp4>()));

//     ASSERT_TRUE((archeT.removeComponent<TComp1>()));
//     ASSERT_TRUE((archeT.removeComponent<TComp2, TComp3>()));
//     ASSERT_FALSE((archeT.removeComponent<TComp1>()));
//     ASSERT_FALSE((archeT.removeComponent(getCTypeId<TComp1>(), getCTypeId<TComp4>())));
//     ASSERT_FALSE((archeT.hasComponent<TComp1, TComp2, TComp3, TComp4>()));

//     archeT.addComponent<TComp1, TComp2>();
//     archeT.reset();
//     ASSERT_FALSE((archeT.hasComponent<TComp1, TComp2>()));
// }

// TEST(ArchetypeTest, Copy_Mask)
// {
//     Archetype archeT;
//     Bitmask   cMask;

//     archeT.addComponent<TComp1, TComp2, TComp3>();
//     cMask.set({ getCTypeId<TComp1>(), getCTypeId<TComp2>(), getCTypeId<TComp3>() });

//     ASSERT_EQ(archeT.getMask(), cMask);
//     Archetype copy = archeT;
//     ASSERT_EQ(copy.getMask(), cMask);

//     archeT.reset();
//     cMask.clear();
//     ASSERT_EQ(archeT.getMask(), cMask);
// }

// TEST(ArchetypeTest, Filter_Interactions)
// {
//     Archetype archetype;
//     BitFilter filter;

//     archetype.addComponent<TComp3, TComp1>();
//     filter.addWanted<TComp1, TComp3>();
//     ASSERT_TRUE(archetype.meetsRequirementsOf(filter));

//     filter.addUnwanted<TComp1>();
//     ASSERT_FALSE(archetype.meetsRequirementsOf(filter));
// }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////

// TEST(ASpawnerTest, ArchetypeValidation)
// {
//     {
//         Archetype archeT;
//         archeT.addComponent<TComp1, TComp3>();
//         ASpawner aSpawner(archeT);
//         ASSERT_EQ(archeT.getMask(), aSpawner.getArchetype().getMask());
//     }
//     {
//         // try with an empty Archetype
//         ASpawner aSpawner(std::move(Archetype()));
//         ASSERT_EQ(Archetype().getMask(), aSpawner.getArchetype().getMask());
//     }
// }

// TEST(ASpawnerTest, Spawn_Kill_Count_SpawningOnly)
// {
//     ASpawner aSpawner(MakeArchetype<TComp1, TComp3>());

//     auto ref  = aSpawner.spawn();
//     auto ref2 = aSpawner.spawn();
//     auto ref3 = aSpawner.spawn();
//     ASSERT_EQ(aSpawner.getSpawningEntitiesCount(), 3);

//     aSpawner.kill(ref);
//     ASSERT_EQ(aSpawner.getSpawningEntitiesCount(), 2);

//     // because spawner is pools based, removing entity from the beginning moves last entity to the front so index 0 is always valid as long as aSpawner.get*EntitiesCount() != 0
//     aSpawner.kill(ref2);
//     aSpawner.kill(ref3);
//     ASSERT_EQ(aSpawner.getSpawningEntitiesCount(), 0);

//     aSpawner.spawn();
//     aSpawner.spawn();
//     aSpawner.clear();
//     ASSERT_EQ(aSpawner.getSpawningEntitiesCount(), 0);

//     aSpawner.spawn();
//     aSpawner.spawn();
//     aSpawner.acceptSpawningEntities();
//     ASSERT_EQ(aSpawner.getSpawningEntitiesCount(), 0);
// }

// TEST(ASpawnerTest, Spawn_Kill_Count_AliveOnly)
// {
//     ASpawner aSpawner(MakeArchetype<TComp1, TComp3>());

//     auto ref = aSpawner.spawn();

//     auto ref2 = aSpawner.spawn();
//     auto ref3 = aSpawner.spawn();

//     aSpawner.acceptSpawningEntities();

//     ASSERT_EQ(aSpawner.getAliveEntitiesCount(), 3);

//     aSpawner.kill(ref);

//     ASSERT_EQ(aSpawner.getAliveEntitiesCount(), 2);

//     aSpawner.kill(ref2);
//     aSpawner.kill(ref3);
//     ASSERT_EQ(aSpawner.getAliveEntitiesCount(), 0);

//     aSpawner.spawn();
//     aSpawner.spawn();
//     aSpawner.acceptSpawningEntities();
//     aSpawner.clear();
//     ASSERT_EQ(aSpawner.getAliveEntitiesCount(), 0);
// }

// TEST(ASpawnerTest, Spawn_Kill_Count_Both)
// {
//     ASpawner aSpawner(MakeArchetype<TComp1, TComp3>());

//     auto ref1 = aSpawner.spawn();
//     auto ref2 = aSpawner.spawn();
//     aSpawner.acceptSpawningEntities();

//     ASSERT_EQ(aSpawner.getAliveEntitiesCount(), 2);

//     auto ref3 = aSpawner.spawn();
//     aSpawner.kill(ref3); // not alive yet, shouldnt change alive entities count

//     ASSERT_EQ(aSpawner.getAliveEntitiesCount(), 2);

//     aSpawner.spawn(); // killing alive entities shouldnt affect spawning entities
//     aSpawner.kill(ref1);
//     aSpawner.kill(ref2);
//     ASSERT_EQ(aSpawner.getAliveEntitiesCount(), 0);
//     ASSERT_EQ(aSpawner.getSpawningEntitiesCount(), 1);
// }

// TEST(ASpawnerTest, SpawnN)
// {
//     ASpawner aSpawner(MakeArchetype<TComp1, TComp3>());
//     size_t   n = 20;
//     aSpawner.spawn(n);
//     aSpawner.acceptSpawningEntities();
//     ASSERT_EQ(aSpawner.getAliveEntitiesCount(), n);

//     size_t i = aSpawner.getAliveEntitiesCount() - n;
//     for (; i < aSpawner.getAliveEntitiesCount(); ++i)
//     {
//         ASSERT_EQ(aSpawner[i].getComponent<TComp1>(), &aSpawner.getPool<TComp1>(true)[i]);
//         ++i;
//     }
//     ASSERT_EQ(i, n);
// }

// TEST(ASpawnerTest, Acces)
// {
//     Archetype archeT;
//     archeT.addComponent<TComp1, TComp3>();
//     ASpawner aSpawner(MakeArchetype<TComp1, TComp3>());

//     auto ref = aSpawner.spawn();

//     aSpawner.acceptSpawningEntities();

//     aSpawner.getPool<TComp1>(true)[0];
//     aSpawner.getPool(getCTypeId<TComp3>(), true)[0];
//     aSpawner.kill(ref);
// #ifdef _DEBUG
//     // test the exception throwing on out of range entity reference index
//     bool refAccessExcCatched = false;
//     try
//     {
//         aSpawner[123];
//     }
//     catch (std::out_of_range)
//     {
//         refAccessExcCatched = true;
//     }
//     ASSERT_TRUE(refAccessExcCatched);

//     bool poolAccessExcCatched = false;
//     try
//     {
//         aSpawner.getPool<TComp2>(true)[0];
//     }
//     catch (std::out_of_range)
//     {
//         poolAccessExcCatched = true;
//     }
//     ASSERT_TRUE(poolAccessExcCatched);
// #endif
// }

// TEST(ASpawnerTest, EntityMove)
// {
//     Archetype archeT;
//     archeT.addComponent<TComp1, TComp3>();
//     ASpawner aSpawner(archeT);
//     archeT.addComponent<TComp2>();
//     ASpawner aSpawner2(archeT);

//     auto      ref       = aSpawner.spawn();
//     EntityRef refBefore = ref;

//     ASSERT_NE(aSpawner.getSpawningEntitiesCount(), 0);
//     ASSERT_NE(aSpawner2.getSpawningEntitiesCount(), 1);

//     aSpawner2.moveExistingEntityHere(ref);
//     ASSERT_EQ(refBefore, ref);
//     ASSERT_EQ(aSpawner.getSpawningEntitiesCount(), 0);
//     ASSERT_EQ(aSpawner2.getSpawningEntitiesCount(), 1);

//     ref = aSpawner.spawn();
//     aSpawner.acceptSpawningEntities();
//     ASSERT_NE(aSpawner.getAliveEntitiesCount(), 0);
//     ASSERT_NE(aSpawner2.getAliveEntitiesCount(), 1);

//     aSpawner2.moveExistingEntityHere(ref);
//     ASSERT_EQ(aSpawner.getAliveEntitiesCount(), 0);

//     ASSERT_NE(aSpawner2.getAliveEntitiesCount(), 1); // NOT equal - thats because moving an entity, ie. changing its archetype,
//                                                      // is treated same as killing that entity and spawning in the other ASpawner
//     ASSERT_EQ(aSpawner2.getSpawningEntitiesCount(), 2);

//     aSpawner2.acceptSpawningEntities();
//     ASSERT_EQ(aSpawner2.getAliveEntitiesCount(), 2);
// }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////

// TEST(EntityRefTest, Validation)
// {
//     ASpawner aSpawner(MakeArchetype<TComp4, TComp1, TComp3>());

//     auto ref0 = aSpawner.spawn();
//     auto ref1 = aSpawner.spawn();
//     auto ref2 = aSpawner.spawn();

//     ASSERT_TRUE((ref0.isValid() && ref1.isValid() && ref2.isValid()));
//     ASSERT_TRUE(ref0.isAlive() == ref1.isAlive() == ref2.isAlive() == false);

//     aSpawner.kill(ref0);
//     ASSERT_FALSE(ref0.isValid());

//     aSpawner.acceptSpawningEntities();
//     ASSERT_TRUE(ref1.isAlive() == ref2.isAlive() == true);

//     ASSERT_TRUE(ref1.isValid());
//     ref1.die();
//     ASSERT_FALSE(ref1.isValid());

//     ASSERT_TRUE(ref2.isValid());

//     ASpawner aSpawner2(MakeArchetype<TComp4, TComp1, TComp2, TComp3>());
//     aSpawner2.moveExistingEntityHere(ref2);

//     ASSERT_TRUE(ref2.isValid());
//     ASSERT_FALSE(ref2.isAlive());
//     ASSERT_TRUE((ref2.hasComponent<TComp4, TComp1, TComp2, TComp3>()));
//     ASSERT_TRUE((ref2.getComponent<TComp4>() && ref2.getComponent<TComp1>() && ref2.getComponent<TComp2>() && ref2.getComponent<TComp3>()));
// }

// TEST(EntityRefTest, ComponentAccess)
// {
//     ASpawner aSpawner1(MakeArchetype<TComp1, TComp3>());
//     ASpawner aSpawner2(MakeArchetype<TComp1, TComp3>());

//     auto ref0 = aSpawner1.spawn();
//     auto ref1 = aSpawner1.spawn();
//     auto ref2 = aSpawner1.spawn();

//     ASSERT_TRUE(ref0.getComponent<TComp1>());
//     ASSERT_TRUE(ref0.getComponent<TComp3>());
//     ASSERT_FALSE(ref0.getComponent<TComp2>());

//     ASSERT_TRUE(ref1.getComponent<TComp1>());
//     ASSERT_TRUE(ref1.getComponent<TComp3>());

//     ASSERT_TRUE(ref2.getComponent<TComp1>());
//     ASSERT_TRUE(ref2.getComponent<TComp3>());

//     ref1.die();
//     ASSERT_TRUE(ref0.getComponent<TComp1>());
//     ASSERT_TRUE(ref0.getComponent<TComp3>());

//     ASSERT_FALSE(ref1.getComponent<TComp1>());
//     ASSERT_FALSE(ref1.getComponent<TComp3>());

//     ASSERT_TRUE(ref2.getComponent<TComp1>());
//     ASSERT_TRUE(ref2.getComponent<TComp3>());

//     aSpawner2.moveExistingEntityHere(ref2);
//     ASSERT_TRUE(ref0.getComponent<TComp1>());
//     ASSERT_TRUE(ref0.getComponent<TComp3>());

//     ASSERT_TRUE(ref2.getComponent<TComp1>());
//     ASSERT_TRUE(ref2.getComponent<TComp3>());

//     aSpawner2.clear();
//     aSpawner1.clear();
//     ASSERT_FALSE(ref0.getComponent<TComp1>());
//     ASSERT_FALSE(ref0.getComponent<TComp3>());
//     ASSERT_FALSE(ref2.getComponent<TComp1>());
//     ASSERT_FALSE(ref2.getComponent<TComp3>());
// }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////

// TEST(SpawnersPackTest, addASpawner_clear)
// {
//     ASpawnersPack<TComp1, TComp3> aSpawnersPack({ getCTypeId<TComp2>() });
//     ASpawner                      spawner1(MakeArchetype<TComp3, TComp1>());
//     ASpawner                      spawner2(MakeArchetype<TComp2, TComp3, TComp1>()); // not matching
//     ASpawner                      spawner3(MakeArchetype<TComp3, TComp1>());

//     aSpawnersPack.addASpawnerIfMeetsRequirements(spawner1);
//     aSpawnersPack.addASpawnerIfMeetsRequirements(spawner2);
//     aSpawnersPack.addASpawnerIfMeetsRequirements(spawner3);

//     ASSERT_EQ(aSpawnersPack.getSpawners().size(), 2);
//     ASSERT_EQ(aSpawnersPack.getSpawners()[0], &spawner1);
//     ASSERT_NE(aSpawnersPack.getSpawners()[1], &spawner2);
//     ASSERT_EQ(aSpawnersPack.getSpawners()[1], &spawner3);

//     aSpawnersPack.clear();
//     ASSERT_EQ(aSpawnersPack.getSpawners().size(), 0);
// }

// TEST(SpawnersPackTest, Filter)
// {
//     ASpawnersPack<TComp1, TComp3> aSpawnersPack({ getCTypeId<TComp2>() });
//     BitFilter                     filter({ { getCTypeId<TComp1>(), getCTypeId<TComp3>() }, { getCTypeId<TComp2>() } });

//     ASSERT_EQ(aSpawnersPack.getFilter(), filter);
// }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////

// TEST(CGroupTest, Filter)
// {
//     EntityManager                    mgr;
//     BitFilter                        filter({ getCTypeId<TComp1>(), getCTypeId<TComp3>() }, {});
//     EntityCollection<TComp1, TComp3> group;
//     mgr.requestCGroup(group, filter.getUnwantedMask());

//     ASSERT_EQ(filter, group.getFilter());
// }

// TEST(CGroupTest, SpawnersValidation)
// {
//     EntityManager                    mgr;
//     EntityCollection<TComp1, TComp3> group;
//     mgr.requestCGroup(group, Bitmask({ getCTypeId<TComp2>() }));

//     ASpawner matchingSpawner(MakeArchetype<TComp1, TComp3>());
//     ASpawner notMatchingSpawner(MakeArchetype<TComp1, TComp3, TComp2>());

//     auto ref1   = matchingSpawner.spawn();
//     auto ref2   = matchingSpawner.spawn();
//     auto nmRef4 = notMatchingSpawner.spawn();

//     mgr.registerArchetype(matchingSpawner.getArchetype());
//     mgr.registerArchetype(notMatchingSpawner.getArchetype());

//     int i = 0;
//     for (auto pack : group)
//     {
//         if (i == 0)
//         {
//             ASSERT_EQ(&pack.get<TComp1&>(), ref1.getComponent<TComp1>());
//             ASSERT_EQ(&pack.get<TComp3&>(), ref1.getComponent<TComp3>());
//         }
//         else if (i == 1)
//         {
//             ASSERT_EQ(&pack.get<TComp1&>(), ref2.getComponent<TComp1>());
//             ASSERT_EQ(&pack.get<TComp3&>(), ref2.getComponent<TComp3>());
//         }
//         else
//             FAIL();
//         ++i;
//     }
// }

// TEST(CGroupTest, Iteration)
// {
//     EntityManager            mgr;
//     EntityCollection<TComp1> group;
//     mgr.requestCGroup(group);

//     mgr.spawn<TComp1, TComp3>();
//     mgr.spawn<TComp1, TComp3>();
//     mgr.spawn<TComp1, TComp3, TComp2>();
//     mgr.spawn<TComp1, TComp3, TComp2>();

//     mgr.acceptSpawnedEntities();

//     int i = 0;
//     // adding a spawner while iterating - range based for loop will not iterate over entities of added spawner
//     // begin -> end based for loop will
//     for (auto it = group.begin(); it.isValid() && it != group.end(); ++it)
//     {
//         it.getComponent<TComp1>();
//         if (i++ == 1)
//         {
//             mgr.spawn<TComp4, TComp1, TComp3, TComp2>();
//             mgr.spawn<TComp4, TComp1, TComp3, TComp2>();
//             mgr.acceptSpawnedEntities();
//         }
//     }
//     ASSERT_EQ(i, 6);

//     i = 0;
//     for (auto pack : group)
//         if (i++ == 1)
//         {
//             mgr.spawn<TComp1>();
//             mgr.spawn<TComp1>();
//             mgr.acceptSpawnedEntities();
//         }
//     ASSERT_EQ(i, 6); // still 6, not 8

//     i = 0;

//     // now it will also iterate over added spawner's entities
//     for (auto pack : group)
//     {
//         pack.get<TComp1&>();
//         i++;
//     }
//     ASSERT_EQ(i, 8);
//     ASSERT_EQ(mgr.getArchetypesSpawners().size(), 4);
//     mgr.clear();

//     i = 0;
//     for (auto pack : group)
//         ++i;
//     ASSERT_EQ(i, 0);

//     // unregistered EntityCollection:
// #if _DEBUG
//     std::cout << "Wanted Assertion fails incomming (one for EntityCollection::begin, one for EntityCollection::end): " << std::endl;
// #endif
//     EntityCollection<TComp1, TComp2> unregistered1;
//     for (auto s : unregistered1)
//         FAIL();
// }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////

// TEST(EntityManagerTest, registerArchetype)
// {
//     EntityManager aSpawnerSet;

//     aSpawnerSet.registerArchetype(MakeArchetype<TComp2>());
//     aSpawnerSet.registerArchetype(MakeArchetype<TComp2>()); // again TComp2 (shouldn't register anything)
//     aSpawnerSet.registerArchetype(MakeArchetype<TComp1, TComp3>());
//     aSpawnerSet.registerArchetype(MakeArchetype<TComp3, TComp1>()); // same as above, but different order (shouldn't register anything)

//     ASSERT_EQ(aSpawnerSet.getArchetypesSpawners().size(), 2);
// }

// TEST(EntityManagerTest, spawn_add_removeComponent)
// {
//     EntityManager em;
//     auto          eRef                   = em.spawn(MakeArchetype<TComp1, TComp3>());
//     eRef.getComponent<TComp1>()->test[0] = 123;
//     eRef.getComponent<TComp3>()->test[0] = 321;

//     ASSERT_TRUE(eRef.isValid());
//     ASSERT_FALSE(eRef.isAlive());

//     em.acceptSpawnedEntities();
//     ASSERT_TRUE(eRef.isAlive());
//     ASSERT_EQ(eRef.getComponent<TComp1>()->test[0], 123);
//     ASSERT_EQ(eRef.getComponent<TComp3>()->test[0], 321);

//     ASSERT_TRUE(eRef.getComponent<TComp1>());
//     ASSERT_TRUE(eRef.getComponent<TComp3>());
//     ASSERT_FALSE(eRef.getComponent<TComp2>());

//     em.addComponent<TComp2>(eRef); // changes archetype -> registers new archetype

//     ASSERT_TRUE(eRef.getComponent<TComp2>());
//     ASSERT_EQ(eRef.getComponent<TComp1>()->test[0], 123);
//     ASSERT_EQ(eRef.getComponent<TComp3>()->test[0], 321);

//     em.removeComponent<TComp2>(eRef);

//     ASSERT_FALSE(eRef.getComponent<TComp2>());
//     ASSERT_EQ(eRef.getComponent<TComp1>()->test[0], 123);
//     ASSERT_EQ(eRef.getComponent<TComp3>()->test[0], 321);

//     ASSERT_EQ(em.getArchetypesSpawners().size(), 2);
// }

// TEST(EntityManagerTest, spawnN)
// {
//     EntityManager em;
//     Archetype     arche = MakeArchetype<TComp1, TComp3>();
//     size_t        n     = 1000;

//     em.spawn(arche, n);
//     em.acceptSpawnedEntities();

//     for (int i = 0; i < n; ++i)
//     {
//         ASSERT_TRUE((*em.getArchetypesSpawners().at(arche.getMask()))[i].isValid());
//         ASSERT_TRUE((*em.getArchetypesSpawners().at(arche.getMask()))[i].isAlive());
//     }

//     em.spawn<TComp1, TComp3>();
//     em.spawn<TComp1, TComp3>(5);
// }

// TEST(EntityManagerTest, Group)
// {
//     EntityManager em;
//     size_t        n = 1000;

//     Archetype arche[4];
//     arche[0] = MakeArchetype<TComp1, TComp2>();
//     arche[1] = MakeArchetype<TComp1, TComp2, TComp3>();
//     arche[2] = MakeArchetype<TComp1, TComp2, TComp4>();
//     arche[3] = MakeArchetype<TComp1, TComp3>();


//     for (size_t i = 0; i < 4; i++)
//         em.spawn(arche[i], n);

//     em.acceptSpawnedEntities();

//     EntityCollection<TComp1, TComp2> group;
//     em.requestCGroup(group, { getCTypeId<TComp4>() }); // unwanted component type: Component
//     // in this filter setup, group will contain entities from archetypes of indices: 0, 1

//     size_t counter = 0;
//     for (auto pack : group)
//     {
//         ++counter;
//         pack.get<TComp1&>();
//         pack.get<TComp2&>();
//     }
//     ASSERT_EQ(counter, 2 * n);

//     counter = 0;
//     for (auto it = group.begin(); it != group.end(); ++it)
//     {
//         counter++;
//         // adding/removing components, or killing entities in a loop couses iterators to skip entities that are moved in a place of removed one
//         // (in this case it will skip exacly half of all entities in the group: n*2 / 2 = n (n*2 couse there are 2 archetypes matching group's filter)
//         em.addComponent<TComp4>(it.getERef());
//     }
//     ASSERT_EQ(counter, n);

//     // solution to above problem
//     counter = 0;
//     for (auto it = group.begin(); it != group.end();)
//     {
//         counter++;
//         em.addComponent<TComp4>(it.getERef());

//         // increment only if iterator is invalid (only after adding a component, if there would be no component adding, then that would make an endless loop)
//         if (!it.isValid())
//             ++it;
//     }

//     // now its only n, couse earlier we already removed half of the initial number of entities
//     ASSERT_EQ(counter, n);
// }

// //////////////////////////////////////////////////////////////////////////////////////////////////////////

// TEST(WorldTest, MakeRemoveSystem)
// {
//     ECSWorld world;

//     auto& system = world.makeSystem<TestSystem>();
//     ASSERT_EQ(&system, &world.getSystem<TestSystem>());
//     ASSERT_TRUE(world.hasSystem<TestSystem>());
//     world.removeSystem<TestSystem>();
//     ASSERT_FALSE(world.hasSystem<TestSystem>());

// #ifdef _DEBUG
//     bool catched = false;
//     try
//     {
//         world.getSystem<TestSystem>();
//     }
//     catch (std::out_of_range)
//     {
//         catched = true;
//     }
//     ASSERT_TRUE(catched);
// #endif // _DEBUG
// }