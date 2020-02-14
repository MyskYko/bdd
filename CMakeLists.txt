project(test CXX)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-g -O3)	

if(UNIX)
  # show quite some warnings (but remove some intentionally)
  include(CheckCXXCompilerFlag)
  add_compile_options(-W -Wall -Wextra)
  foreach (WARNING unknown-pragmas gnu-anonymous-struct nested-anon-types)
    check_cxx_compiler_flag("-Wno-${WARNING}" HAS_WNO_${WARNING})
    if (HAS_WNO_${WARNING})
      add_compile_options(-Wno-${WARNING})
    endif()
  endforeach()
endif()

add_subdirectory(lib)

add_executable(a.out test.cpp)
target_link_libraries(a.out mockturtle simplebdd cudd buddy)
target_include_directories(a.out PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

