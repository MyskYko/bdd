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
  mockturtle::NameMap<mockturtle::aig_network> namemap;
  lorina::read_aiger( filename, mockturtle::aiger_reader( aig , &namemap ) );
  std::vector<std::string> pi_names( aig.num_pis() );
  std::vector<std::string> po_names( aig.num_pos() );
  for ( int i = 0; i < aig.num_pis(); i++ )
    {
      pi_names[i] = namemap.get_name( aig.make_signal( aig.pi_at( i ) ) )[0];
    }
  for ( int i = 0; i < aig.num_pos(); i++ )
    {
      po_names[i] = namemap.get_name( aig.po_at( i ) )[0];
    }
  
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
      
#if defined(REORDER)
      std::cout << "Ordering :" << std::endl;
      std::vector<int> v( aig.num_pis() );
      for ( int i = 0; i < aig.num_pis(); i++ )
	{
	  v[bdd.Perm( i )] = i;
	}
      for ( int i : v )
	{
	  std::cout << pi_names[i] << " ";
	}
      std::cout << std::endl;
#endif
      
      mockturtle::klut_network lut;
      Bdd2Lut( lut, bdd, vNodes );
      mockturtle::names_view lut_{lut};
      for ( int i = 0; i < aig.num_pis(); i++ )
	{
	  lut_.set_name( lut_.make_signal( lut_.pi_at( i ) ), pi_names[i] );
	}
      for ( int i = 0; i < aig.num_pos(); i++ )
	{
	  lut_.set_output_name( i, po_names[i] );
	}
      mockturtle::write_blif( lut_, filename2 );
    }
  catch ( char const * error )
    {
      std::cout << error << std::endl;
      return 1;
    }
  return 0;
}
