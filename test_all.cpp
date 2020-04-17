#include <AtBddMan.hpp>
#include <AigBdd.hpp>
#include <LutBdd.hpp>
#include <mockturtle/mockturtle.hpp>
#include <lorina/lorina.hpp>
#include <string>

int main( int argc, char ** argv )
{
  if ( argc <= 2 )
    {
      std::cout << "usage : aig2bdd <input aig> <output blif>" << std::endl;
      return 1;
    }
  std::string filename = argv[1];
  std::string filename2 = argv[2];
  
  mockturtle::aig_network aig;
  lorina::read_aiger( filename, mockturtle::aiger_reader( aig ) );

  try
    {
      Bdd::AtBddParam p;
      p.nNodes = 16777216;
      p.nUnique = 8388608;
      p.nCache = 65536;
      p.nUniqueMinRate = 35;
      p.nCallThold = 100476;
      p.fRealloc = 1;
      p.fGC = 0;
      p.nGC;
      p.fReo = 0;
      p.nReo;
      p.nMaxGrowth;
#if defined(GARBAGE_COLLECT)
      p.fGC = 1;
      p.nGC = 1 << 30;
#elif defined(REORDER)
      p.nNodes = 8192;
      p.nUnique = 2097152;
      p.nCache = 16384;
      p.nUniqueMinRate = 11;
      p.nCallThold = 33382;
      p.fRealloc = 1;
      p.fGC = 1;
      p.nGC = 744068;
      p.fReo = 1;
      p.nReo = 1000;
      p.nMaxGrowth = 59;
#endif
      
      Bdd::AtBddMan bdd( aig.num_pis(), p );
      auto vNodes = Aig2Bdd( aig, bdd );
      bdd.PrintStats( vNodes );
      mockturtle::klut_network lut;
      Bdd2Lut( lut, bdd, vNodes );
      mockturtle::write_blif( lut, filename2 );
    }
  catch ( char const * error )
    {
      std::cout << error << std::endl;
      return 1;
    }
  return 0;
}
