cmake_minimum_required(VERSION 3.8)
project(ECSpp
    VERSION 0.5.0
    LANGUAGES CXX)

    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug)
    endif()


    if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
        option(ECSPP_TESTS "Build the tests" ON)
        if(ECSPP_TESTS)
            enable_testing()
            include(GoogleTest) # standard module
            add_subdirectory(tests)
        endif()
        
        option(ECSPP_BENCHMARKS "Build the benchmarks" ON)
        if(ECSPP_BENCHMARKS)
            add_subdirectory(benchmarks)
        endif()
    endif()