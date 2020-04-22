project(bdd CXX)
cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-g -O3)

add_library(bdd INTERFACE)
target_include_directories(bdd INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

option(BDD_METHOD "enable bdd method" OFF)
option(BDD_TEST "build bdd test" OFF)

if(BDD_TEST)
set(BDD_METHOD ON CACHE BOOL "enable bdd method" FORCE)
endif()

add_subdirectory(lib)

if(BDD_METHOD)

add_library(bddmethod INTERFACE)
target_include_directories(bddmethod INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/method)
target_link_libraries(bddmethod INTERFACE bdd mockturtle)
  
endif()

if(BDD_TEST)

add_executable(test_simplebdd ${CMAKE_CURRENT_SOURCE_DIR}/test_all.cpp)
target_link_libraries(test_simplebdd mockturtle bddmethod simplebdd)
target_compile_definitions(test_simplebdd PUBLIC SIMPLEBDD)

add_executable(test_cudd ${CMAKE_CURRENT_SOURCE_DIR}/test_all.cpp)
target_link_libraries(test_cudd mockturtle bddmethod cudd)
target_compile_definitions(test_cudd PUBLIC CUDD)

add_executable(test_buddy ${CMAKE_CURRENT_SOURCE_DIR}/test_all.cpp)
target_link_libraries(test_buddy mockturtle bddmethod buddy)
target_compile_definitions(test_buddy PUBLIC BUDDY)

add_executable(test_cacbdd ${CMAKE_CURRENT_SOURCE_DIR}/test_all.cpp)
target_link_libraries(test_cacbdd mockturtle bddmethod cacbdd)
target_compile_definitions(test_cacbdd PUBLIC CACBDD)

add_executable(test_atbdd ${CMAKE_CURRENT_SOURCE_DIR}/test_all.cpp)
target_link_libraries(test_atbdd mockturtle bddmethod atbdd)
target_compile_definitions(test_atbdd PUBLIC ATBDD)

endif()
