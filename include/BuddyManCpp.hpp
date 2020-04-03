#ifndef BUDDY_MAN_HPP_
#define BUDDY_MAN_HPP_

#include <iostream>
#include "BddMan.hpp"
#include <bdd.h>

using namespace Buddy;

namespace Bdd
{
  struct BuddyParam
  {
    // Param
    int nVars = 0; // None 0
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
	nVars = std::stoi( str );
      if ( std::getline( f, str ) )
	nNodes = std::stoi( str );
      if ( std::getline( f, str ) )
	nCache = std::stoi( str );
    }
  };
    
  class BuddyMan : public BddMan<bdd>
  {
  public:
    BuddyMan( BuddyParam p )
    {
      bdd_init( p.nNodes, p.nCache );
      bdd_setvarnum( p.nVars );
    };
    ~BuddyMan() { bdd_done(); }
    bdd  Const0() override { return bdd_false(); }
    bdd  Const1() override { return bdd_true(); }
    bdd  IthVar( int i ) override { return bdd_ithvar( i ); }
    bdd  Regular( bdd const & x ) override { return x; }
    bool IsCompl( bdd const & x ) override { (void)x; return 0; }
    int  Var( bdd const & x ) override { return bdd_var( x ); }
    bdd  Then( bdd const & x ) override { return bdd_high( x ); }
    bdd  Else( bdd const & x ) override { return bdd_low( x ); }
    void Ref( bdd const & x ) override { (void)x; }
    void Deref( bdd const & x ) override { (void)x; }
    bdd  NotCond( bdd const & x, bool c ) override { return c? bdd_not( x ): x; }
    bdd  And( bdd const & x, bdd const & y ) override { return bdd_and( x, y ); }
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
  };
}

#endif
