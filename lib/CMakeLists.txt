if(BDD_METHOD)
add_subdirectory(mockturtle)
endif()

add_library(simplebdd INTERFACE)
set_target_properties(simplebdd PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/simplebdd)

add_library(cudd STATIC IMPORTED GLOBAL)
set_target_properties(cudd PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/cudd/cudd/.libs/libcudd.a)
set_target_properties(cudd PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/cudd/cudd;${CMAKE_CURRENT_SOURCE_DIR}/cudd/cplusplus")


add_subdirectory(buddy)

add_subdirectory(cacbdd)

add_library(atbdd INTERFACE)
set_target_properties(atbdd PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/atbdd)
