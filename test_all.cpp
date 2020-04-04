#include <SimpleBddMan.hpp>
#include <CuddMan.hpp>
#include <BuddyMan.hpp>
#include <CacBddMan.hpp>
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
      Bdd::SimpleBddMan<> sbdd( aig.num_pis() );
      Bdd::Aig2Bdd( aig, sbdd );
      sbdd.PrintStats();
      
      Bdd::CuddMan cbdd( aig.num_pis() );
      Bdd::Aig2Bdd( aig, cbdd );
      cbdd.PrintStats();
      
      Bdd::BuddyMan bbdd( aig.num_pis() );
      Bdd::Aig2Bdd( aig, bbdd );
      bbdd.PrintStats();

      Bdd::CacBddMan cacbdd( aig.num_pis() );
      Bdd::Aig2Bdd( aig, cacbdd );
      cacbdd.PrintStats();
      
      //      mockturtle::aig_network aig2;
      //      Bdd::Bdd2Aig( aig2, bdd );
      //      mockturtle::write_bench( aig2, "file_simple.bench" );

    }
  catch ( char const * error )
    {
      std::cout << error << std::endl;
      return 1;
    }
  return 0;
}
