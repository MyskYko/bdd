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
    
  class BuddyMan : public BddMan<Buddy::BDD>
  {
  public:
    BuddyMan( int nVars )
    {
      BuddyParam p;
      Buddy::bdd_init( p.nNodes, p.nCache );
      Buddy::bdd_setvarnum( nVars );
    };
    BuddyMan( int nVars, BuddyParam p )
    {
      Buddy::bdd_init( p.nNodes, p.nCache );
      Buddy::bdd_setvarnum( nVars );
    };
    ~BuddyMan() { Buddy::bdd_done(); }
    Buddy::BDD  Const0() override { return Buddy::bdd_false(); }
    Buddy::BDD  Const1() override { return Buddy::bdd_true(); }
    Buddy::BDD  IthVar( int i ) override { return Buddy::bdd_ithvar( i ); }
    Buddy::BDD  Regular( Buddy::BDD const & x ) override { return x; }
    bool IsCompl( Buddy::BDD const & x ) override { (void)x; return 0; }
    int  Var( Buddy::BDD const & x ) override { return Buddy::bdd_var( x ); }
    Buddy::BDD  Then( Buddy::BDD const & x ) override { return Buddy::bdd_high( x ); }
    Buddy::BDD  Else( Buddy::BDD const & x ) override { return Buddy::bdd_low( x ); }
    void Ref( Buddy::BDD const & x ) override { Buddy::bdd_addref( x ); }
    void Deref( Buddy::BDD const & x ) override { Buddy::bdd_delref( x ); }
    Buddy::BDD  NotCond( Buddy::BDD const & x, bool c ) override { return c? Buddy::bdd_not( x ): x; }
    Buddy::BDD  And( Buddy::BDD const & x, Buddy::BDD const & y ) override { return Buddy::bdd_and( x, y ); }
    int  GetNumVar() override { return Buddy::bdd_varnum(); }
    void PrintStats() override
    {
      uint64_t count = 0;
      for ( uint32_t i = 0; i < vNodes.size(); i++ )
	{
	  count += Buddy::bdd_nodecount( vNodes[i] );
	}
      std::cout << "Shared BDD nodes = " << Buddy::bdd_anodecount( vNodes.data(), vNodes.size() ) << std::endl;
      std::cout << "Sum of BDD nodes = " << count << std::endl;
    }

    uint64_t Id( Buddy::BDD const & x ) { return (uint64_t)x; }
  };
}

#endif
