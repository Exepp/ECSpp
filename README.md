# ECSpp -  C++ 17 Entity Component System library
ECSpp is a library for managing entities in games. It is implemented using "data-oriented design" approach and Entity Comonent System pattern. Library optimize iteration over objects with specyfic components set by arranging them contiguously in memory, maximizing cache usage while keeping entities addition and removal fast.

You can see an example of use in my [Game Engine](https://github.com/Exepp/GameEngine).

Current TODO
------------
Add internal static stack and pool allocators (pool mainly for handling Entity instances)

Add multithreading support

# Classes description
1. Component - data. A unique set of different components describes what entity really is (what is its archetype)
2. Pool<Component> - a pool of components. Each component is "owned" by a different entity
3. Archetype - a unique set of different components (in fact it owns pools of these components, which are used by ASpawner)
4. ASpawner (archetype spawner) - Provides an interface for creating entities with components specified in Archetype.
5. CGroup (ASpawnersPack) - container of all the archetypes that meets the requirements of CGroup's filter (The filter contains set of wanted and unwanted components) with an iterator, that iterates over every entity of contained ASpawners and provides fast access to their components.
6. EntityManager - container of ASpawners, that provides interface for spawning new entities of given Archetype (with given components), adding or removing components from an entity and requesting CGroups. 
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
Every spawned entity must be accepted before CGroup iterators can iterate over them
that happens before each System::Update, or you can accept them manually by calling EntityManager::acceptSpawningEntities
if you call it inside CGroup iteration loop, it is undefined which one of the added entities will iterator iterate over (more about it in [Adding/Removing components](#addingremoving-components))
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
eRef.isValid(); // checks if reference is valid (if it points to an Entity and if is, then checks if that Entity is valid)
eRef.isAlive(); // checks if an entity is alive (if CGroup iterators will iterate over this entity)
eRef.hasComponent<Component1>();
eRef.hasComponent(getCTypeId<Component2>());
Component1* c1Ptr = eRef.getComponent<Component1>(); // returns a pointer to the component, returns nullptr if the entity does not have the component
Component* c2Ptr = eRef.getComponent(getCTypeId<Component2>()); // note, that here it returns a Component*, not Component2*

eRef.getComponent_noCheck<Component1>(); // for no isValid() checks - call it if you checked it by yourself (it will throw if invalid) and you want to get many components
eRef.hasComponent_noCheck<Component1>(); // same as above
eRef.die(); // destroys owned components, invalidates all the references to this entity)
```
EntityRef::die requires a special treatment while called in a CGroup iteration loop, more info in: [Using CGroup](#using-cgroup))

## Adding/Removing components
This is kind of an expensive operation, especially when resulting archetype is new to the EntityManager
To add a new archetype, EntityManager must go through every requested CGroup and add that archetype to the ones that accept it (when it has a proper components set), plus of course there will be some memory allocation, but that being said, all of this happens only for the first time the new Archetype is added.To prevent this, you can call: EntityManager::registerArchetype, but you would have to register every possible archetype that will be used in your game (of course order doesn't matter).

Second thing is that changing the archetype of the entity makes the EntityManager move entity's components to a new ASpawner and that causes the same problems as spawning a new entity in that ASpawner. For this reason entity with added/removed components will be treated as spawned one, so as mentioned before in "Spawning an entity", EntityManager::acceptSpawningEntities must be called before any CGroup iterator can iterate over this entity.

Component can be added/removed only through EntityManager.

```c++
EntityRef eRef = entityManager.spawn<Component1, Component2>();
entityManager.removeComponent<Component1>(eRef);
entityManager.addComponent<Component3>(eRef);
```

## Using CGroup:
First, you must always request the group in an EntityModule. If you will use it before that, it will throw. This can be done in System::init.

```c++
//to request a group:
CGroup<Component1, Component2> group = entityManager.requestCGroup<Component1, Component2>(Bitmask({ getCTypeId<Component3>() })); // requests a group, that wants entities with Component1, Component2 and without Component3
// or without repeating <Component1, Component2>:
entityManager.requestCGroup(group, { getCTypeId<Component1>() }); // when there are the same components wanted and unwanted, unwanted specifier will be ignored

// iterating over a group using range-based for loop (don't use it if you kill entities or add/remove components from entities): 
for (auto entity : group) // note, that the variable entity is not a reference
    // getting a component:
    entity.get<Component1&>(); // note that the requested type must be a reference (won't compile otherwise) (const reference if group is const)
    
// iterating using an iterator (this is the only way for correct iteration with in-loop components adding/removal or entities killing (also you can access entity reference through an iterator))
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
Because entity is a set of components, that are located in pools that keeps all the data arranged contiguously, 
removing the component from the pool makes it move the last element to fill the gap, so the iterator now points to a new valid entity, so there should be no incrementation. But when the removed components are the last elements of the pools then removing them will invalidate the iterator and that's when you should increment it
