#ifndef BUDDY_CPP_MAN_HPP_
#define BUDDY_CPP_MAN_HPP_

#include <iostream>
#include <fstream>
#include "BddMan.hpp"
#include <bdd.h>

namespace Bdd
{
  struct BuddyCppParam
  {
    // Param
    int nNodes = 100000; // Int 10000 1000000
    int nCache = 10000; // Int 10000 100000
    // end

    BuddyCppParam( std::string fname = "_BuddyCppMan.hpp_setting.txt" )
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
    
  class BuddyCppMan : public BddMan<Buddy::bdd>
  {
  public:
    BuddyCppMan( int nVars )
    {
      BuddyCppParam p;
      Buddy::bdd_init( p.nNodes, p.nCache );
      Buddy::bdd_setvarnum( nVars );
    };
    BuddyCppMan( int nVars, BuddyCppParam p )
    {
      Buddy::bdd_init( p.nNodes, p.nCache );
      Buddy::bdd_setvarnum( nVars );
    };
    ~BuddyCppMan() { Buddy::bdd_done(); }
    Buddy::bdd  Const0() override { return Buddy::bdd_false(); }
    Buddy::bdd  Const1() override { return Buddy::bdd_true(); }
    Buddy::bdd  IthVar( int i ) override { return Buddy::bdd_ithvar( i ); }
    Buddy::bdd  Regular( Buddy::bdd const & x ) override { return x; }
    bool IsCompl( Buddy::bdd const & x ) override { (void)x; return 0; }
    int  Var( Buddy::bdd const & x ) override { return Buddy::bdd_var( x ); }
    Buddy::bdd  Then( Buddy::bdd const & x ) override { return Buddy::bdd_high( x ); }
    Buddy::bdd  Else( Buddy::bdd const & x ) override { return Buddy::bdd_low( x ); }
    void Ref( Buddy::bdd const & x ) override { (void)x; }
    void Deref( Buddy::bdd const & x ) override { (void)x; }
    Buddy::bdd  NotCond( Buddy::bdd const & x, bool c ) override { return c? Buddy::bdd_not( x ): x; }
    Buddy::bdd  And( Buddy::bdd const & x, Buddy::bdd const & y ) override { return Buddy::bdd_and( x, y ); }
    int  GetNumVar() override { return Buddy::bdd_varnum(); }
    void PrintStats() override
    {
      uint64_t count = 0;
      for ( uint32_t i = 0; i < vNodes.size(); i++ )
	{
	  count += Buddy::bdd_nodecount( vNodes[i] );
	}
      std::cout << "Shared BDD nodes = " << bdd_anodecountpp( vNodes.data(), vNodes.size() ) << std::endl;
      std::cout << "Sum of BDD nodes = " << count << std::endl;
    }

    uint64_t Id( Buddy::bdd const & x ) { return (uint64_t)x.id(); }
  };
}

#endif
