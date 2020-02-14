#ifndef SIMPLE_BDD_MAN_HPP_
#define SIMPLE_BDD_MAN_HPP_

#include "BddMan.hpp"
#include <SimpleBdd.hpp>

namespace Bdd
{
  struct SimpleBddParam
  {
    int  nVars = 254;
    int  nNodes = 1;
    int  nVerbose = 0;
    bool fGC = 1;
    bool fRealloc = 1;
    int  nMaxGrowth = 0;
  };
  
  template<typename var = uint8_t>
  class SimpleBddMan : public BddMan
  {
  private:
    SimpleBdd::BddMan<var> * man;
    
  public:
    SimpleBddMan( SimpleBddParam p )
    {
      assert( p.nVars < (int)std::numeric_limits<var>::max() );
      man = new SimpleBdd::BddMan<var>( p.nVars, p.nNodes, NULL, p.nVerbose );
      man->RefreshConfig( p.fGC, p.fRealloc, p.nMaxGrowth );
    };
    ~SimpleBddMan() { delete man; }
    uint64_t Const0() override { return man->LitConst0(); }
    uint64_t Const1() override { return man->LitConst1(); }
    uint64_t IthVar( int i ) override { return man->LitIthVar( i ); }
    uint64_t Regular( uint64_t x ) override { return man->LitRegular( x ); }
    int IsCompl( uint64_t x ) override { return man->LitIsCompl( x ); }
    int Var( uint64_t x ) override { return man->get_order( man->Var( x ) ); }
    uint64_t Then( uint64_t x ) override { return man->Then( x ); }
    uint64_t Else( uint64_t x ) override { return man->Else( x ); }
    void Ref( uint64_t x ) override { man->Ref( x ); }
    void Deref( uint64_t x ) override { man->Deref( x ); }
    uint64_t NotCond( uint64_t x, int c ) override { return man->LitNotCond( x, c ); }
    uint64_t And( uint64_t x, uint64_t y ) override { return man->And( x, y ); }
    int GetNumVar() override { return man->get_nVars(); }
    void PrintStats() override
    {
      std::vector<uint32_t> vNodes_( vNodes_.size() );
      for ( uint32_t i = 0; i < vNodes.size(); i++ )
	vNodes_.push_back( vNodes[i] );
      std::cout << "Shared BDD nodes = " << man->CountNodesArrayShared( vNodes_ ) << std::endl;
      std::cout << "Sum of BDD nodes = " << man->CountNodesArrayIndependent( vNodes_ ) << std::endl;
    }
  };
}

#endif
