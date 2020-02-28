# ECSpp -  C++ 17 x64 Entity Component System library
ECSpp is a library that manages entities in games. It is implemented using "data-oriented design" approach and Entity Comonent System pattern. Library optimize iteration over objects with specyfic components by arranging them contiguously in memory, maximizing cache usage.
Its design is based on Unity's Entity Component System (archetype-based ECS).

Documentation: https://exepp.github.io/ECSpp/

Main goal: Fast iteration time. 
In the BenchmarksResults folder you can find result of my benchmarks for my library and Entt. Number in the name of the file indicates the number of components tested in these tests.
I managed to get a stable and in general better performance at the expense of adding/removing new components, which is rather a rare operation.

Current TODO
------------
More better Benchmarks \
Documentation's home page


# Core classes description
This library is basically a specialized relational database.
1. Entity - an entity (primiary key)
2. Component - an entity's attribute
3. Archetype - a relation schema
4. CPool - a relation's column (components)
5. EntitySpawner - a relation
6. Selection - a query
7. EntityManager - DBMS   
