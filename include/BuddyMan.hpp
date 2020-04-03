#ifndef BUDDY_MAN_HPP_
#define BUDDY_MAN_HPP_

#include <iostream>
#include <fstream>
#include "BddMan.hpp"
namespace Buddy
{
  extern "C"
  {
    #include <bdd.h>
  }
}

using namespace Buddy;

namespace Bdd
{
  struct BuddyParam
  {
    // Param
    int nNodes = 100000; // Int 10000 1000000
    int nCache = 10000; // Int 10000 100000
    // end

    BuddyParam( std::string fname = "_BuddyMan.hpp_setting.txt" )
    {
      std::ifstream f( fname );
      if ( !f )
	return;
      std::string str;
      if ( std::getline( f, str ) )
	nNodes = std::stoi( str );
      if ( std::getline( f, str ) )
	nCache = std::stoi( str );
    }
  };
    
  class BuddyMan : public BddMan<BDD>
  {
  public:
    BuddyMan( int nVars )
    {
      BuddyParam p;
      bdd_init( p.nNodes, p.nCache );
      bdd_setvarnum( nVars );
    };
    BuddyMan( int nVars, BuddyParam p )
    {
      bdd_init( p.nNodes, p.nCache );
      bdd_setvarnum( nVars );
    };
    ~BuddyMan() { bdd_done(); }
    BDD  Const0() override { return bdd_false(); }
    BDD  Const1() override { return bdd_true(); }
    BDD  IthVar( int i ) override { return bdd_ithvar( i ); }
    BDD  Regular( BDD const & x ) override { return x; }
    bool IsCompl( BDD const & x ) override { (void)x; return 0; }
    int  Var( BDD const & x ) override { return bdd_var( x ); }
    BDD  Then( BDD const & x ) override { return bdd_high( x ); }
    BDD  Else( BDD const & x ) override { return bdd_low( x ); }
    void Ref( BDD const & x ) override { bdd_addref( x ); }
    void Deref( BDD const & x ) override { bdd_delref( x ); }
    BDD  NotCond( BDD const & x, bool c ) override { return c? bdd_not( x ): x; }
    BDD  And( BDD const & x, BDD const & y ) override { return bdd_and( x, y ); }
    int  GetNumVar() override { return bdd_varnum(); }
    void PrintStats() override
    {
      uint64_t count = 0;
      for ( uint32_t i = 0; i < vNodes.size(); i++ )
	{
	  count += bdd_nodecount( vNodes[i] );
	}
      std::cout << "Shared BDD nodes = " << bdd_anodecount( vNodes.data(), vNodes.size() ) << std::endl;
      std::cout << "Sum of BDD nodes = " << count << std::endl;
    }

    uint64_t Id( BDD const & x ) { return (uint64_t)x; }
  };
}

#endif
