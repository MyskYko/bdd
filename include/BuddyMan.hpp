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
    int nVars = 0;
    int nNodes = 100000;
    int nCache = 10000;
  };
    
  class BuddyMan : public BddMan
  {
  public:
    BuddyMan( BuddyParam p )
    {
      bdd_init( p.nNodes, p.nCache );
      bdd_setvarnum( p.nVars );
    };
    ~BuddyMan() { bdd_done(); }
    uint64_t Const0() override { return bdd_false(); }
    uint64_t Const1() override { return bdd_true(); }
    uint64_t IthVar( int i ) override { return bdd_ithvar( i ); }
    uint64_t Regular( uint64_t x ) override { return x; }
    int IsCompl( uint64_t x ) override { (void)x; return 0; }
    int Var( uint64_t x ) override { return bdd_var( x ); }
    uint64_t Then( uint64_t x ) override { return bdd_high( x ); }
    uint64_t Else( uint64_t x ) override { return bdd_low( x ); }
    void Ref( uint64_t x ) override { bdd_addref( x ); }
    void Deref( uint64_t x ) override { bdd_delref( x ); }
    uint64_t NotCond( uint64_t x, int c ) override { return c? bdd_not( x ): x; }
    uint64_t And( uint64_t x, uint64_t y ) override { return bdd_and( x, y ); }
    int GetNumVar() override { return bdd_varnum(); }
    void PrintStats() override
    {
      BDD * vNodes_ = (BDD *)malloc( sizeof(BDD) * vNodes.size() );
      uint64_t count = 0;
      for ( uint32_t i = 0; i < vNodes.size(); i++ )
	{
	  vNodes_[i] = (BDD)vNodes[i];
	  count += bdd_nodecount( vNodes_[i] );
	}
      std::cout << "Shared BDD nodes = " << bdd_anodecount( vNodes_, vNodes.size() ) << std::endl;
      std::cout << "Sum of BDD nodes = " << count << std::endl;
    }
  };
}

#endif
