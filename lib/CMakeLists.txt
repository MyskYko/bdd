add_subdirectory(mockturtle/lib)
add_library(mockturtle INTERFACE)
target_include_directories(mockturtle INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/mockturtle/include)
target_link_libraries(mockturtle INTERFACE ez kitty lorina sparsepp percy)

add_library(simplebdd INTERFACE)
target_include_directories(simplebdd INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(cudd STATIC IMPORTED GLOBAL)
set_target_properties(cudd PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/cudd/cudd/.libs/libcudd.a)
set_target_properties(cudd PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/cudd/cudd)

add_library(buddy STATIC IMPORTED GLOBAL)
set_target_properties(buddy PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/buddy/src/.libs/libbdd.a)
target_include_directories(buddy INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/buddy/src)
