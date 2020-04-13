#ifndef CUDD_MAN_HPP_
#define CUDD_MAN_HPP_

#include <iostream>
#include <fstream>
#include <string>
#include "BddMan.hpp"
#include <cudd.h>

namespace Bdd
{
  struct CuddParam
  {
    // Param
    int nUnique = CUDD_UNIQUE_SLOTS; // Log 1000 1000000000
    int nCache = CUDD_CACHE_SLOTS; // Log 1000 1000000000
    int nMaxMem = 0; // Log 1000 1000000000
    int nMinHit = 30; // Int 1 99
    bool fGC = 1; // Bool
    int nReoScheme = 1; // Switch 13
    int nMaxGrowth = 20; // Int 1 100
    // end
    
    CuddParam( std::string fname = "_CuddMan.hpp_setting.txt" )
    {
      std::ifstream f( fname );
      if ( !f )
	return;
      std::string str;
      if ( std::getline( f, str ) )
	nUnique = std::stoi( str );
      if ( std::getline( f, str ) )
	nCache = std::stoi( str );
      if ( std::getline( f, str ) )
	nMaxMem = std::stoi( str );
      if ( std::getline( f, str ) )
	nMinHit = std::stoi( str );
      if ( std::getline( f, str ) )
	fGC = ( str == "True" );
      if ( std::getline( f, str ) )
	nReoScheme = std::stoi( str );
      if ( std::getline( f, str ) )
	nMaxGrowth = std::stoi( str );
    }
  };
    
  class CuddMan : public BddMan<DdNode *>
  {
  private:
    DdManager * man;
    
  public:
    CuddMan( int nVars )
    {
      CuddParam p;
      man = Cudd_Init( nVars, 0, p.nUnique, p.nCache, p.nMaxMem );
      Cudd_SetMinHit( man, p.nMinHit );
      if ( !p.fGC )
	Cudd_DisableGarbageCollection( man );
      if ( p.nReoScheme )
	{
	  Cudd_AutodynEnable( man, (Cudd_ReorderingType)(CUDD_REORDER_SIFT + p.nReoScheme - 1) );
	  Cudd_SetMaxGrowth( man, 1.0 + p.nMaxGrowth * 0.01 );
	}
    }
    CuddMan( int nVars, CuddParam p )
    {
      man = Cudd_Init( nVars, 0, p.nUnique, p.nCache, p.nMaxMem );
      Cudd_SetMinHit( man, p.nMinHit );
      if ( !p.fGC )
	Cudd_DisableGarbageCollection( man );
      if ( p.nReoScheme )
	{
	  Cudd_AutodynEnable( man, (Cudd_ReorderingType)(CUDD_REORDER_SIFT + p.nReoScheme - 1) );
	  Cudd_SetMaxGrowth( man, 1.0 + p.nMaxGrowth * 0.01 );
	}
    }
    ~CuddMan() { Cudd_Quit( man ); }
    DdNode * Const0() override { return Cudd_Not( Cudd_ReadOne( man ) ); }
    DdNode * Const1() override { return Cudd_ReadOne( man ); }
    DdNode * IthVar( int i ) override { return Cudd_ReadVars( man, i ); }
    DdNode * Regular( DdNode * const & x ) override { return Cudd_Regular( x ); }
    bool     IsCompl( DdNode * const & x ) override { return Cudd_IsComplement( x ); }
    DdNode * Not( DdNode * const & x ) override { return Cudd_Not( x ); }
    int      Var( DdNode * const & x ) override { return Cudd_NodeReadIndex( x ); }
    DdNode * Then( DdNode * const & x ) override { return Cudd_NotCond( Cudd_T( x ), IsCompl( x ) ); }
    DdNode * Else( DdNode * const & x ) override { return Cudd_NotCond( Cudd_E( x ), IsCompl( x ) ); }
    
    void     Ref( DdNode * const & x ) override { Cudd_Ref( x ); }
    void     Deref( DdNode * const & x ) override { Cudd_RecursiveDeref( man, x ); }
    
    DdNode * And( DdNode * const & x, DdNode * const & y ) override { return Cudd_bddAnd( man, x, y ); }
    DdNode * Or( DdNode * const & x, DdNode * const & y ) override { return Cudd_bddOr( man, x, y ); }
    DdNode * Xor( DdNode * const & x, DdNode * const & y ) override { return Cudd_bddXor( man, x, y ); }

    void     Reorder() override { Cudd_ReduceHeap( man, CUDD_REORDER_SIFT, 0 ); }
    
    int      GetNumVar() override { return Cudd_ReadSize( man ); }
    void     PrintStats() override
    {
      uint64_t count = 0;
      for ( uint32_t i = 0; i < vNodes.size(); i++ )
	{
	  count += Cudd_DagSize( vNodes[i] );
	}
      std::cout << "Shared BDD nodes = " << Cudd_SharingSize( vNodes.data(), vNodes.size() ) << std::endl;
      std::cout << "Sum of BDD nodes = " << count << std::endl;
    }

    uint64_t Id( DdNode * const & x ) { return (uint64_t)x; }
  };
}


#endif
