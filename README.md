# ECSpp - C++ 17 Header-only ECS library

# Table of contents
* [Introduction](#introduction)
    * [Motivation](#motivation)
* [Code example](#code-example)
* [Benchmarks](#benchmarks)
    * [Building](#building-benchmarks)
    * [Results](#results)
* [Tests](#tests)
    * [Building](#building-tests)
    * [Coverage](#coverage)

Current TODO
------------
Documentation's home page
Github's actions
Better out of the box building

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

# Benchmarks
To test the performance of this library, I am using google's benchmark library: [benchmark](https://github.com/google/benchmark). The "external" folder contains header files, lib files and executables needed to create the solution and build benchmarks for x64 version on Linux and Windows. If included files do not work on your platform, unfortunately for now you will have to provide the correct ones yourself.

Most of these benchmarks are based on the benchmarks from the [entt](https://github.com/skypjack/entt) library to see how my library compares to arguably the best ECS library. See below for the results.

## Building benchmarks
On windows:
* Run makeSolution_Win.bat 
* Open the generated solution
* Build Benchmarks project

On linux:
* `$ ./makeSolution_Linux.sh` 
* `$ ./runBenchmarks.sh` 

## Results
See the results in [BenchmarksResults](https://github.com/Exepp/ECSpp/blob/master/BenchmarksResults/makeCharts.ipynb) folder.
More to come soon.

# Tests
To test this library, I am using google's test library: [googletest](https://github.com/google/googletest). 
As in the case of the benchmarks, files to build tests for x64 version on Linux and Windows also resides in the "external" folder.

## Building tests
On windows:
* Run makeSolution_Win.bat 
* Open the generated solution
* Build Tests project

On linux:
* `$ ./makeSolution_Linux.sh` 
* `$ ./runTests.sh` 

## Coverage
For now I am using gcc's gcov to generate the code coverage data of the tests and gcovr to generate the html representation. To generate the coverage:
* Make sure that you have gcovr package
    * `$ apt install gcovr`
* `$ ./makeCoverage.sh`

Coverage will be available at [docs/coverage.html](https://exepp.github.io/ECSpp/coverage.html)