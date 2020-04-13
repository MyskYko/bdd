project(test CXX)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-g -O3)	

if(UNIX)
  # show quite some warnings (but remove some intentionally)
  include(CheckCXXCompilerFlag)
  add_compile_options(-W -Wall -Wextra)
  foreach (WARNING unknown-pragmas gnu-anonymous-struct nested-anon-types deprecated-copy parentheses)
    check_cxx_compiler_flag("-Wno-${WARNING}" HAS_WNO_${WARNING})
    if (HAS_WNO_${WARNING})
      add_compile_options(-Wno-${WARNING})
    endif()
  endforeach()
endif()

add_subdirectory(lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/method)

add_executable(test_simplebdd ${CMAKE_CURRENT_SOURCE_DIR}/test_all.cpp)
target_link_libraries(test_simplebdd mockturtle simplebdd)
target_compile_definitions(test_simplebdd PUBLIC SIMPLEBDD)

add_executable(test_cudd ${CMAKE_CURRENT_SOURCE_DIR}/test_all.cpp)
target_link_libraries(test_cudd mockturtle cudd)
target_compile_definitions(test_cudd PUBLIC CUDD)

add_executable(test_buddy ${CMAKE_CURRENT_SOURCE_DIR}/test_all.cpp)
target_link_libraries(test_buddy mockturtle buddy)
target_compile_definitions(test_buddy PUBLIC BUDDY)

add_executable(test_cacbdd ${CMAKE_CURRENT_SOURCE_DIR}/test_all.cpp)
target_link_libraries(test_cacbdd mockturtle cacbdd)
target_compile_definitions(test_cacbdd PUBLIC CACBDD)

add_executable(test_atbdd ${CMAKE_CURRENT_SOURCE_DIR}/test_all.cpp)
target_link_libraries(test_atbdd mockturtle atbdd)
target_compile_definitions(test_atbdd PUBLIC ATBDD)
