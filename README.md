# ECSpp -  C++ 17 Entity Component System library
ECSpp is a library for managing entities in games. It is implemented using "data-oriented design" approach using Entity Comonent System pattern, that optimize iteration over object with specyfic components set by arranging them contiguously in memory, maximizing cache usage while keeping entities addition and removal fast.

# Classes description
1. Component - data. A unique set of different components describes what entity really is (what is its archetype)
2. Pool<Component> - a container of one type of component, each component is "owned" by different entity
3. Archetype - a unique set of different components (in fact it owns pools of these components, which are used by ASpawner)
4. ASpawner (archetype spawner) - Archetype wrapper. Provides an interface for creating "instances of Archetype" (just creates instances of components that the Archetype contains and returns EntityRef, that contains a valid index to these pools)
5. CGroup (ASpawnersPack) - container of archetypes that meets the requirements of CGroup's filter (The filter contains set of wanted and unwanted components) with an iterator, that iterates over all entities of contained ASpawners and provides fast access to their components
6. EntityManager - container of ASpawners, that provides interface for spawning new entities of given Archetype (with given components), adding or removing components from an entity, requesting CGroups. 
  
# Memory Layout
![](Documentation/Memory%20Layout/Pool.PNG)

![](Documentation/Memory%20Layout/Archetype2.PNG)        ![](Documentation/Memory%20Layout/ASpawner.PNG)

![](Documentation/Memory%20Layout/CGroup.PNG)           
![](Documentation/Memory%20Layout/Entity.PNG)

# Examples
