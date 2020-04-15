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
    bool fReo = 0; // None False
    int nReoScheme = 0; // None 12
    int nMaxGrowth = 20; // None 1 100
    // end
    
    CuddParam( std::string fname = "_CuddMan.hpp_setting.txt" )
    {
      std::ifstream f( fname );
      if ( !f )
	{
	  return;
	}
      std::string str;
      std::getline( f, str );
      nUnique = std::stoi( str );
      std::getline( f, str );
      nCache = std::stoi( str );
      std::getline( f, str );
      nMaxMem = std::stoi( str );
      std::getline( f, str );
      nMinHit = std::stoi( str );
      std::getline( f, str );
      fGC = ( str == "True" );
      std::getline( f, str );
      fReo = ( str == "True" );
      std::getline( f, str );
      nReoScheme = std::stoi( str );
      std::getline( f, str );
      nMaxGrowth = std::stoi( str );
    }
  };
    
  class CuddMan : public BddMan<DdNode *>
  {
  private:
    DdManager * man;
    CuddParam param;
    
  public:
    CuddMan( int nVars, CuddParam param ) : param( param )
    {
      man = Cudd_Init( nVars, 0, param.nUnique, param.nCache, param.nMaxMem );
      Cudd_SetMinHit( man, param.nMinHit );
      if ( !param.fGC )
	{
	  Cudd_DisableGarbageCollection( man );
	}
      if ( param.fReo )
	{
	  Cudd_AutodynEnable( man, (Cudd_ReorderingType)( CUDD_REORDER_SIFT + param.nReoScheme ) );
	}
      Cudd_SetMaxGrowth( man, 1.0 + param.nMaxGrowth * 0.01 );
    }
    CuddMan( int nVars ) : CuddMan( nVars, CuddParam() ) {}
    ~CuddMan() { Cudd_Quit( man ); }

    int GetNumVar() override { return Cudd_ReadSize( man ); }
    uint64_t Id( DdNode * const & x ) { return (uint64_t)x; }

    DdNode * Const0() override { return Cudd_Not( Cudd_ReadOne( man ) ); }
    DdNode * Const1() override { return Cudd_ReadOne( man ); }
    DdNode * IthVar( int i ) override { return Cudd_ReadVars( man, i ); }
    DdNode * Regular( DdNode * const & x ) override { return Cudd_Regular( x ); }
    bool IsCompl( DdNode * const & x ) override { return Cudd_IsComplement( x ); }
    int Var( DdNode * const & x ) override { return Cudd_NodeReadIndex( x ); }
    DdNode * Then( DdNode * const & x ) override { return Cudd_NotCond( Cudd_T( x ), IsCompl( x ) ); }
    DdNode * Else( DdNode * const & x ) override { return Cudd_NotCond( Cudd_E( x ), IsCompl( x ) ); }
    DdNode * Not( DdNode * const & x ) override { return Cudd_Not( x ); }
    
    void Ref( DdNode * const & x ) override { Cudd_Ref( x ); }
    void Deref( DdNode * const & x ) override { Cudd_RecursiveDeref( man, x ); }

    int Perm( int i ) override { return Cudd_ReadPerm( man, i ); }
    void Reorder() override { Cudd_ReduceHeap( man, (Cudd_ReorderingType)( CUDD_REORDER_SIFT + param.nReoScheme ), 0 ); }
    
    DdNode * And( DdNode * const & x, DdNode * const & y ) override { return Cudd_bddAnd( man, x, y ); }
    DdNode * Or( DdNode * const & x, DdNode * const & y ) override { return Cudd_bddOr( man, x, y ); }
    DdNode * Xor( DdNode * const & x, DdNode * const & y ) override { return Cudd_bddXor( man, x, y ); }
    DdNode * Ite( DdNode * const & c, DdNode * const & x, DdNode * const & y ) override { return Cudd_bddIte( man, c, x, y ); }
    DdNode * Exist( DdNode * const & x, DdNode * const & cube ) override { return Cudd_bddExistAbstract( man, x, cube ); }
    DdNode * Univ( DdNode * const & x, DdNode * const & cube ) override { return Cudd_bddUnivAbstract( man, x, cube ); }
    DdNode * AndExist( DdNode * const & x, DdNode * const & y, DdNode * const & cube ) override { return Cudd_bddAndAbstract( man, x, y, cube ); }
    DdNode * Restrict( DdNode * const & x, DdNode * const & c ) override { return Cudd_bddRestrict( man, x, c ); }
    DdNode * Compose( DdNode * const & x, int i, DdNode * const & c ) override { return Cudd_bddCompose( man, x, c, i ); }
    DdNode * VecCompose( DdNode * const & x, std::vector<DdNode *> & cs ) override { return Cudd_bddVectorCompose( man, x, cs.data() ); }
    
    void Support( DdNode * const & x, std::vector<int> & vVars ) override
    {
      DdNode * y = Cudd_Support( man, x );
      while ( y != Const1() )
	{
	  vVars.push_back( Var( y ) );
	  y = Then( y );
	}
    }
    
    void PrintStats( std::vector<DdNode *> & vNodes ) override
    {
      uint64_t count = 0;
      for ( uint32_t i = 0; i < vNodes.size(); i++ )
	{
	  count += Cudd_DagSize( vNodes[i] );
	}
      std::cout << "Shared BDD nodes = " << Cudd_SharingSize( vNodes.data(), vNodes.size() ) << std::endl;
      std::cout << "Sum of BDD nodes = " << count << std::endl;
    }
  };
}


#endif
