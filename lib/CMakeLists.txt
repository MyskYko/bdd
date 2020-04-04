add_subdirectory(mockturtle/lib)
add_library(mockturtle INTERFACE)
target_include_directories(mockturtle INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/mockturtle/include)
target_link_libraries(mockturtle INTERFACE ez kitty lorina sparsepp percy)

add_library(simplebdd INTERFACE)
set_target_properties(simplebdd PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/simplebdd)

add_library(cudd STATIC IMPORTED GLOBAL)
set_target_properties(cudd PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/cudd/cudd/.libs/libcudd.a)
set_target_properties(cudd PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/cudd/cudd)

add_library(buddy STATIC IMPORTED GLOBAL)
set_target_properties(buddy PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/buddy/src/.libs/libbdd.a)
set_target_properties(buddy PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/buddy/src)

add_library(cacbdd STATIC IMPORTED GLOBAL)
set_target_properties(cacbdd PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/cacbdd/libcacBDD.a)
set_target_properties(cacbdd PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/cacbdd/includes)
