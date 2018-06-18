# ECSpp -  C++ 17 Entity Component System library
ECSpp is a library for managing entities in games. It is implemented using "data-oriented design" approach and Entity Comonent System pattern. Library optimize iteration over objects with specyfic components set by arranging them contiguously in memory, maximizing cache usage while keeping entities addition and removal fast.

You can see an example of use in my [Game Engine](https://github.com/Exepp/GameEngine).

Current TODO
------------
Internal static stack and pool allocators (pool mainly for handling Entity instances)

Benchmarks

Multithreading support

# Classes description
1. Component - data. A unique set of different components, describes what entity really is (what is its archetype)
2. Pool<Component> - a pool of components. Each component is "owned" by a different entity
3. Archetype - a unique set of different components (in fact it owns pools of these components, which are used by ASpawner)
4. ASpawner (archetype spawner) - Provides an interface for creating entities with components specified in Archetype.
5. CGroup (ASpawnersPack) - container of all the ASpawners that meets the requirements of CGroup's filter (The filter contains set of wanted and unwanted components) with an iterator, that iterates over every entity of contained ASpawners and provides fast and safe access to their components.
6. EntityManager - container of ASpawners that provides interface for: spawning new entities of given Archetype (with given components), adding or removing components from an entity and requesting CGroups. 
7. System - Nothing fancy, just a class that can be used to implement some logic by iterating over entities form CGroup. CGroups can be requested everywhere, so its not like this class is special in any case, just use it, if it and World class are good enough for you.
7. World - just a container of Systems + EntityManager instance...

# Memory Layout

![](Documentation/Memory%20Layout/Pool.PNG)

![](Documentation/Memory%20Layout/Entity.PNG)

![](Documentation/Memory%20Layout/Archetype4.PNG)    

![](Documentation/Memory%20Layout/ASpawner2.PNG)

![](Documentation/Memory%20Layout/EntityManager.PNG)

![](Documentation/Memory%20Layout/CGroup.PNG)           


# Examples

## Spawning an entity:
Every spawned entity must be accepted before CGroup iterators can iterate over them. That happens automatically in World::update before each System::update but you can accept them manually by calling EntityManager::acceptSpawningEntities. If you call it inside a CGroup iteration loop, it is undefined which one of the added entities will iterator iterate over (more about it in [Adding/Removing components](#addingremoving-components))
```c++
EntityManager entityManager;

entityManager.spawn<Component1, Component2>();
//or
entityManager.spawn(makeArchetype<Component1, Component2>());
// or
Archetype archetype = makeArchetype<Component1, Component2>();
// or make archetype by yourself: 
// Archetype archetype;
// archetype.addComponent<Component1, Component2>();

entityManager.spawn(archetype);
    
```

## Getting and using EntityRef
```c++
EntityRef eRef = entityManager.spawn<Component1, Component2>();
eRef.isValid(); // checks if the reference is valid (if it points to any entity, and if it does, checks if that entity is valid)
eRef.isAlive(); // checks if the entity is alive (if CGroup iterators will iterate over this entity, also calls isValid())
eRef.hasComponent<Component1>();
eRef.hasComponent(getCTypeId<Component2>());
Component1* c1Ptr = eRef.getComponent<Component1>(); // returns a pointer to the component. Returns nullptr if the entity does not have the component, or the reference is invalid
Component* c2Ptr = eRef.getComponent(getCTypeId<Component2>()); // note, that here it returns a Component*, not Component2*

eRef.getComponent_noCheck<Component1>(); // for no internal isValid() checks - when you want to get many components, and you already know that the reference is valid.
eRef.hasComponent_noCheck<Component1>(); // same as above
eRef.die(); // destroys the owned components, invalidates all the references to this entity
```
EntityRef::die requires a special treatment while called in a CGroup iteration loop, more about it in: [Using CGroup](#using-cgroup))

## Adding/Removing components
This is kind of an expensive operation, especially when resulting archetype is new to the EntityManager
To add a new archetype, EntityManager must let all the requested CGroups know about it (and these will add that archetype if it meets the requirements of their filters), plus of course there will be some memory allocation, but all that being said, all of this happens only for the first time a new Archetype is added. To prevent this, you can call: EntityManager::registerArchetype, but you would have to register every possible archetype that will be used in your game (of course order doesn't matter).

Second thing is that changing the archetype of an entity makes the EntityManager move entity's components to tje new ASpawner and that causes the same problems as spawning a new entity in that ASpawner. For this reason entity with added/removed components will be treated as spawned ones, so as mentioned before in "Spawning an entity", EntityManager::acceptSpawningEntities must be called before any CGroup iterator can iterate over this entity.

Component can be added/removed only through EntityManager.

```c++
EntityRef eRef = entityManager.spawn<Component1, Component2>();
entityManager.removeComponent<Component1>(eRef);
entityManager.addComponent<Component3>(eRef);
```

## Using CGroup:
First of all, you must always request a group in an EntityModule, or your it will be empty. This can be done in System::init, but acually requesting an existing (requested before) CGroup is quite cheap (unordered_map average O(1) get), so you could just request it each time inside of your System::update.

```c++
//to request a group:
CGroup<Component1, Component2> group = entityManager.requestCGroup<Component1, Component2>(Bitmask({ getCTypeId<Component3>() })); // requests a group, that wants entities with Component1, Component2 and without Component3
// or without repeating <Component1, Component2>:
entityManager.requestCGroup(group, { getCTypeId<Component1>() }); // when there are the same components wanted and unwanted, unwanted specifier will be ignored for these components

// iterating over a group using range-based for loop (don't use it if you kill entities or add/remove components from entities): 
for (auto entity : group) // note, that the variable "entity" is not a reference
    // getting a component:
    entity.get<Component1&>(); // note that the requested type must be a reference (won't compile otherwise) (const reference if group is const)
    
// iterating using an iterator (this is the only way for correct iteration with in-loop components adding/removal or entities killing (you can also access an entity reference through the iterator))
for (auto iterator = group.begin(); iterator != group.end(); )
{
    iterator.getComponent<Component1>(); // no reference type this time
        
    // getting reference to the entity
    if (EntityRef ref = iterator.getERef(); ref.hasComponent_noCheck<Component3>()) // just some reference use example
    {
        ref.die();
        // after killing an entity, iterate only if iterator is invalid
        // the same must be done when adding/removing components
        if (!iterator.isValid())
            ++iterator;
    }
    // if there is no entity killing, increment as usually
    else
        ++iterator;
}
```
So what happend here:
Because an entity is a set of components that are located in pools, removing the entity means removing components of that entity from these pools. Pools to keep all the data arranged contiguously will move their last elements to fill the gap, so a new entity is in the place of the removed one. In this case the iterator shouldn't be incremented, to prevent omitting that entity. But when the removed entity is the last one (of current ASpawner iterator points to), then no entity will be moved, so after removal iterator will point to the invalid entity and that's when you should increment it.
