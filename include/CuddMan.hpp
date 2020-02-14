#ifndef CUDD_MAN_HPP_
#define CUDD_MAN_HPP_

#include <iostream>
#include "BddMan.hpp"
namespace Cudd
{
  #include <cudd.h>
}

using namespace Cudd;

namespace Bdd
{
  class CuddMan : public BddMan
  {
  private:
    DdManager * man;
    
  public:
    CuddMan( int nVars )
    {
      man = Cudd_Init( nVars, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0 );
    }
    ~CuddMan() { Cudd_Quit( man ); }
    uint64_t Const0() override { return (uint64_t)Cudd_Not( Cudd_ReadOne( man ) ); }
    uint64_t Const1() override { return (uint64_t)Cudd_ReadOne( man ); }
    uint64_t IthVar( int i ) override { return (uint64_t)Cudd_ReadVars( man, i ); }
    uint64_t Regular( uint64_t x ) override { return (uint64_t)Cudd_Regular( (DdNode *)x ); }
    int IsCompl( uint64_t x ) override { return Cudd_IsComplement( (DdNode *)x ); }
    int Var( uint64_t x ) override { return Cudd_NodeReadIndex( (DdNode *)x ); }
    uint64_t Then( uint64_t x ) override { return (uint64_t)Cudd_NotCond( Cudd_T( (DdNode *)x ), IsCompl( x ) ); }
    uint64_t Else( uint64_t x ) override { return (uint64_t)Cudd_NotCond( Cudd_E( (DdNode *)x ), IsCompl( x ) ); }
    void Ref( uint64_t x ) override { Cudd_Ref( (DdNode *)x ); }
    void Deref( uint64_t x ) override { Cudd_RecursiveDeref( man, (DdNode *)x ); }
    uint64_t NotCond( uint64_t x, int c ) override { return (uint64_t)Cudd_NotCond( (DdNode *)x, c ); }
    uint64_t And( uint64_t x, uint64_t y ) override { return (uint64_t)Cudd_bddAnd( man, (DdNode *)x, (DdNode *)y ); }
    int GetNumVar() override { return Cudd_ReadSize( man ); }
    void PrintStats() override
    {
      DdNode ** vNodes_ = (DdNode **)malloc( sizeof(DdNode*) * vNodes.size() );
      uint64_t count = 0;
      for ( uint32_t i = 0; i < vNodes.size(); i++ )
	{
	  vNodes_[i] = (DdNode *)vNodes[i];
	  count += Cudd_DagSize( vNodes_[i] );
	}
      std::cout << "Shared BDD nodes = " << Cudd_SharingSize( vNodes_, vNodes.size() ) << std::endl;
      std::cout << "Sum of BDD nodes = " << count << std::endl;
      free( vNodes_ );
    }
  };
}

#endif
