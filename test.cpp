#include <SimpleBddMan.hpp>
#include <CuddMan.hpp>
#include <BuddyMan.hpp>
#include <AigBdd.hpp>
#include <mockturtle/mockturtle.hpp>
#include <lorina/lorina.hpp>
#include <string>

int main( int argc, char ** argv )
{
  if ( argc == 1 )
    return 1;
  std::string filename = argv[1];
  mockturtle::aig_network aig;
  lorina::read_aiger( filename, mockturtle::aiger_reader( aig ) );

  try
    {
      Bdd::SimpleBddParam p;
      //Bdd::CuddParam p;
      //Bdd::BuddyParam p;
      p.nVars = aig.num_pis();
      Bdd::SimpleBddMan<> bdd( p );
      //Bdd::CuddMan bdd( p );
      //Bdd::BuddyMan bdd( p );
      Bdd::Aig2Bdd( aig, bdd );
      /*
      bdd.PrintStats();
      mockturtle::aig_network aig2;
      Bdd::Bdd2Aig( aig2, bdd );
      mockturtle::write_bench( aig2, "file_simple.bench" );
      */
    }
  catch ( char const * error )
    {
      std::cout << error << std::endl;
      return 1;
    }
  return 0;
}
