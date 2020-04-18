#if defined(SIMPLEBDD)
#include <SimpleBddMan.hpp>
#elif defined(CUDD)
#include <CuddMan.hpp>
#elif defined(BUDDY)
#include <BuddyMan.hpp>
#elif defined(CACBDD)
#include <CacBddMan.hpp>
#elif defined(ATBDD)
#include <AtBddMan.hpp>
#else
#error
#endif


#include <NetBdd.hpp>
#include <Transduction.hpp>
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

  try
    {
#if defined(SIMPLEBDD)
      Bdd::SimpleBddMan bdd( aig.num_pis() );
#elif defined(CUDD)
      Bdd::CuddMan bdd( aig.num_pis() );
#elif defined(BUDDY)
      Bdd::BuddyMan bdd( aig.num_pis() );
#elif defined(CACBDD)
      Bdd::CacBddMan bdd( aig.num_pis() );
#elif defined(ATBDD)
      Bdd::AtBddMan bdd( aig.num_pis() );
#endif
      
      if ( filename2.empty() )
	{
	  Transduction( aig, bdd );
	}
      else
	{
	  Transduction( aig, bdd, 0, 1, 1 );
	}
    }
  catch ( char const * error )
    {
      std::cout << error << std::endl;
      return 1;
    }
  return 0;
}
