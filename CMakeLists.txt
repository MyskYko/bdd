project(bdd CXX)
cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-g -O3)	

add_subdirectory(lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/method)

add_executable(aig2bdd ${CMAKE_CURRENT_SOURCE_DIR}/test_all.cpp)
target_link_libraries(aig2bdd mockturtle atbdd)

add_executable(aig2bdd_gc ${CMAKE_CURRENT_SOURCE_DIR}/test_all.cpp)
target_link_libraries(aig2bdd_gc mockturtle atbdd)
target_compile_definitions(aig2bdd_gc PUBLIC GARBAGE_COLLECT)

add_executable(aig2bdd_reo ${CMAKE_CURRENT_SOURCE_DIR}/test_all.cpp)
target_link_libraries(aig2bdd_reo mockturtle atbdd)
target_compile_definitions(aig2bdd_reo PUBLIC REORDER)
