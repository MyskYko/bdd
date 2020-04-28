#include <SimpleBddMan.hpp>
#include <CuddMan.hpp>
#include <BuddyMan.hpp>
#include <CacBddMan.hpp>
#include <AtBddMan.hpp>

#include <NetBdd.hpp>
#include <mockturtle/mockturtle.hpp>
#include <lorina/lorina.hpp>
#include <string>

int main( int argc, char ** argv )
{
  if ( argc == 1 )
    return 1;
  std::string filename = argv[1];
  std::string filename2;
  if ( argc > 2 )
    filename2 = argv[2];
  
  mockturtle::aig_network aig;
  lorina::read_aiger( filename, mockturtle::aiger_reader( aig ) );

  Bdd::SimpleBddMan bdd( aig.num_pis() );
  //Bdd::CuddMan bdd( aig.num_pis() );
  //Bdd::BuddyMan bdd( aig.num_pis() );
  //Bdd::CacBddMan bdd( aig.num_pis() );
  //Bdd::AtBddMan bdd( aig.num_pis() );
  
  return 0;
}
