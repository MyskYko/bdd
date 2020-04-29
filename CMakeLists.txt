cmake_minimum_required(VERSION 3.8)
project(bdd CXX)
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
add_subdirectory(example)
endif()
