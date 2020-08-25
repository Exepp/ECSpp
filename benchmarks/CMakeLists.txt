cmake_minimum_required(VERSION 3.8)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Enable testing of the benchmark library.")
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/benchmark" "extern/benchmark") # "extern/benchmark" evaluated with respect to the current output directory (pwd)

mark_as_advanced(FORCE
    BENCHMARK_DOWNLOAD_DEPENDENCIE BENCHMARK_BUILD_32_BITS
    FORCE BENCHMARK_ENABLE_ASSEMBLY_TEST BENCHMARK_ENABLE_EXCEPTIONS BENCHMARK_ENABLE_GTEST_TESTS
    BENCHMARK_ENABLE_INSTALL BENCHMARK_ENABLE_LTO BENCHMARK_ENABLE_TESTING BENCHMARK_USE_LIBCXX)

add_executable(benchmarks ecspp.cpp)
target_compile_features(benchmarks PRIVATE cxx_std_17)
target_include_directories(benchmarks PRIVATE ${PROJECT_SOURCE_DIR}/include/)
target_link_libraries(benchmarks benchmark benchmark_main)
set_target_properties(benchmarks PROPERTIES FOLDER benchmarks)

