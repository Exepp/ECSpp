cmake_minimum_required(VERSION 3.8)

add_subdirectory("${PROJECT_SOURCE_DIR}/extern/googletest" "extern/googletest") # "extern/googletest" evaluated with respect to the current output directory (pwd)
mark_as_advanced(
    BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
    gmock_build_tests gtest_build_samples gtest_build_tests
    gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

set_target_properties(gtest PROPERTIES FOLDER extern)
set_target_properties(gtest_main PROPERTIES FOLDER extern)
set_target_properties(gmock PROPERTIES FOLDER extern)
set_target_properties(gmock_main PROPERTIES FOLDER extern)

macro(package_add_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})
    target_compile_features(${TESTNAME} PRIVATE cxx_std_17)
    target_include_directories(${TESTNAME} PRIVATE ${PROJECT_SOURCE_DIR}/include/)
    target_link_libraries(${TESTNAME} gtest gmock gtest_main)
    target_compile_definitions(${TESTNAME} PRIVATE $<$<CONFIG:Debug>:EPP_DEBUG>)
    
    gtest_discover_tests(${TESTNAME}
        WORKING_DIRECTORY ${PROJECT_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
    )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
    
endmacro()

package_add_test(tests 
    Utility/PoolT.cpp
    EntityManager/ComponentT.cpp
    EntityManager/CMaskT.cpp
    EntityManager/ArchetypeT.cpp
    EntityManager/SelectionT.cpp
    EntityManager/CPoolT.cpp
    EntityManager/EntityManagerT.cpp
    EntityManager/EntitySpawnerT.cpp
    EntityManager/EntityListT.cpp
)

