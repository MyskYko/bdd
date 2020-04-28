add_executable(aig2bdd ${CMAKE_CURRENT_SOURCE_DIR}/aig2bdd.cpp)
target_link_libraries(aig2bdd mockturtle bddmethod simplebdd cudd buddy cacbdd atbdd)
