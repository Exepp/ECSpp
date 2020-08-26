# ECSpp - C++ 17 Header-only ECS library
[![Build](https://github.com/Exepp/ECSpp/workflows/Build/badge.svg)](https://github.com/Exepp/ECSpp/actions)
[![codecov](https://codecov.io/gh/Exepp/ECSpp/branch/master/graph/badge.svg)](https://codecov.io/gh/Exepp/ECSpp)

# Table of contents
* [Introduction](#introduction)
* [Code example](#code-example)
* [Building tests and benchmarks](#building-tests-and-benchmarks)
    * [Coverage](#coverage)
* [Performance](#performance)

Current TODO
------------
Documentation's home page
Github's actions

# Introduction
ECSpp is a header only library that manages objects in games or simulations. It was developed with "[data-oriented design](https://www.youtube.com/watch?v=rX0ItVEVjHc)" approach to create a fast [ECS](https://en.wikipedia.org/wiki/Entity_component_system) model. Library optimizes the iteration over components by arranging them contiguously in memory, maximizing the spatial locality of the data.
The memory model is based on Unity's Entity Component System (Archetype-based ECS). 

Documentation: https://exepp.github.io/ECSpp/

## Motivation
I wanted to create a library that would help me with development of my game/simulation projects. Then I came across ECS pattern that addressed many of the problems I had with inheritance and isolating data from logic. After some more research I came across Unity's ECS. Impressed by their great design, I decided to make my own C++ implementation.

# Code Example
```cpp

#include <ECSpp/EntityManager.h>
#include <cstdio>

struct Component1 {
    Component1() { printf("Component1 default constr\n"); }
    Component1(float a) : a(a)
    {
        printf("Component1 constr %d %f\n", b, a);
    }
    ~Component1() { printf("Component1 default destr\n"); }

    int b = 123;
    float a;
};

struct Component2 {
    Component2() { printf("Component2 default constr\n"); }
    ~Component2() { printf("Component2 default destr\n"); }

    std::string str = "hello";
};


int main()
{
    epp::EntityManager mgr;
    epp::Archetype arch(epp::IdOf<Component1, Component2, std::string>());
    epp::Selection<Component1, std::string> sel;

    for (int k = 0; k < 3; ++k)
        mgr.spawn(arch);

    int i = 0;
    mgr.spawn(arch, [&i](epp::EntityCreator&& cr) {
        cr.constructed<std::string>(cr.constructed<Component2>().str); // Component2 first, std::string second
        cr.constructed<Component1>(float(++i));                        // Component1 third
    });

    mgr.updateSelection(sel);
    sel.forEach([](epp::Selection<Component1, std::string>::Iterator_t const& entityIt) {
        printf("%d %f %s\n",
               entityIt.getComponent<Component1>().b,
               entityIt.getComponent<Component1>().a,
               entityIt.getComponent<std::string>().c_str());
    });

    return 0;
}

```
# Building tests and benchmarks
To test this library, I am using google's test library: [googletest](https://github.com/google/googletest).
To test the performance of this library, I am using google's benchmark library: [benchmark](https://github.com/google/benchmark).

* Clone the library
    * `$ git clone https://github.com/Exepp/ECSpp.git --recurse-submodules`
    * or 
    * `$ git clone https://github.com/Exepp/ECSpp.git`
    * `$ cd ECSpp`
    * `$ git submodule update --init`
* Generate build files using cmake
(in ECSpp's root directory)
    * `$ mkdir build`
    * `$ cd build`
    * `$ cmake ..`
    * `$ cmake --build . --target tests`
    * Using single-config generators:
        * `$ cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . --target benchmarks`
    * Using multi-config generators:
        * `$ cmake --build . --config Release --target benchmarks`

# Coverage
Coverage available at [docs/coverage.html](https://exepp.github.io/ECSpp/coverage.html)

# Performance
Most of my benchmarks are based on the benchmarks from the [entt](https://github.com/skypjack/entt) library, to see how my library compares to arguably the best ECS library. See my results in [BenchmarksResults](https://github.com/Exepp/ECSpp/blob/master/BenchmarksResults/makeCharts.ipynb) folder.
