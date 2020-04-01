#ifndef BUDDY_MAN_HPP_
#define BUDDY_MAN_HPP_

#include <iostream>
#include "BddMan.hpp"
namespace Buddy
{
  extern "C"
  {
    #include <bdd.h>
    // Notice : BDD, bdd, BDD_(somehting), bdd_(something) are defined in buddy
  }
}

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
    
  class BuddyMan : public BddMan<BDD>
  {
  public:
    BuddyMan( BuddyParam p )
    {
      bdd_init( p.nNodes, p.nCache );
      bdd_setvarnum( p.nVars );
    };
    ~BuddyMan() { bdd_done(); }
    BDD  Const0() override { return bdd_false(); }
    BDD  Const1() override { return bdd_true(); }
    BDD  IthVar( int i ) override { return bdd_ithvar( i ); }
    BDD  Regular( BDD x ) override { return x; }
    bool IsCompl( BDD x ) override { (void)x; return 0; }
    int  Var( BDD x ) override { return bdd_var( x ); }
    BDD  Then( BDD x ) override { return bdd_high( x ); }
    BDD  Else( BDD x ) override { return bdd_low( x ); }
    void Ref( BDD x ) override { bdd_addref( x ); }
    void Deref( BDD x ) override { bdd_delref( x ); }
    BDD  NotCond( BDD x, bool c ) override { return c? bdd_not( x ): x; }
    BDD  And( BDD x, BDD y ) override { return bdd_and( x, y ); }
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
