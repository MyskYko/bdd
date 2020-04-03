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

add_executable(test_simplebdd ${CMAKE_CURRENT_SOURCE_DIR}/test_simplebdd.cpp)
target_link_libraries(test_simplebdd mockturtle simplebdd)
target_include_directories(test_simplebdd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(test_cudd ${CMAKE_CURRENT_SOURCE_DIR}/test_cudd.cpp)
target_link_libraries(test_cudd mockturtle cudd)
target_include_directories(test_cudd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(test_buddy_cpp ${CMAKE_CURRENT_SOURCE_DIR}/test_buddy_cpp.cpp)
target_link_libraries(test_buddy_cpp mockturtle buddy_cpp)
target_include_directories(test_buddy_cpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(test_buddy ${CMAKE_CURRENT_SOURCE_DIR}/test_buddy.cpp)
target_link_libraries(test_buddy mockturtle buddy)
target_include_directories(test_buddy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(test_cacbdd ${CMAKE_CURRENT_SOURCE_DIR}/test_cacbdd.cpp)
target_link_libraries(test_cacbdd mockturtle cacbdd)
target_include_directories(test_cacbdd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
