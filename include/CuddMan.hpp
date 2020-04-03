#ifndef CUDD_MAN_HPP_
#define CUDD_MAN_HPP_

#include <iostream>
#include <fstream>
#include <string>
#include "BddMan.hpp"
namespace Cudd
{
  #include <cudd.h>
}

using namespace Cudd;

namespace Bdd
{
  struct CuddParam
  {
    // Param
    int nNodes = CUDD_UNIQUE_SLOTS; // Int 100 10000
    int nCache = CUDD_CACHE_SLOTS; // Int 10000 1000000
    int nMaxMem = 0; // Int 10000000 1000000000
    // end
    
    CuddParam( std::string fname = "_CuddMan.hpp_setting.txt" )
    {
      std::ifstream f( fname );
      if ( !f )
	return;
      std::string str;
      if ( std::getline( f, str ) )
	nNodes = std::stoi( str );
      if ( std::getline( f, str ) )
	nCache = std::stoi( str );
      if ( std::getline( f, str ) )
	nMaxMem = std::stoi( str );
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
      man = Cudd_Init( nVars, 0, p.nNodes, p.nCache, p.nMaxMem );
    }
    CuddMan( int nVars, CuddParam p )
    {
      man = Cudd_Init( nVars, 0, p.nNodes, p.nCache, p.nMaxMem );
    }
    ~CuddMan() { Cudd_Quit( man ); }
    DdNode * Const0() override { return Cudd_Not( Cudd_ReadOne( man ) ); }
    DdNode * Const1() override { return Cudd_ReadOne( man ); }
    DdNode * IthVar( int i ) override { return Cudd_ReadVars( man, i ); }
    DdNode * Regular( DdNode * const & x ) override { return Cudd_Regular( x ); }
    bool     IsCompl( DdNode * const & x ) override { return Cudd_IsComplement( x ); }
    int      Var( DdNode * const & x ) override { return Cudd_NodeReadIndex( x ); }
    DdNode * Then( DdNode * const & x ) override { return Cudd_NotCond( Cudd_T( x ), IsCompl( x ) ); }
    DdNode * Else( DdNode * const & x ) override { return Cudd_NotCond( Cudd_E( x ), IsCompl( x ) ); }
    void     Ref( DdNode * const & x ) override { Cudd_Ref( x ); }
    void     Deref( DdNode * const & x ) override { Cudd_RecursiveDeref( man, x ); }
    DdNode * NotCond( DdNode * const & x, bool c ) override { return Cudd_NotCond( x, c ); }
    DdNode * And( DdNode * const & x, DdNode * const & y ) override { return Cudd_bddAnd( man, x, y ); }
    int      GetNumVar() override { return Cudd_ReadSize( man ); }
    void     PrintStats() override
    {
      uint64_t count = 0;
      for ( uint32_t i = 0; i < vNodes.size(); i++ )
	count += Cudd_DagSize( vNodes[i] );
      std::cout << "Shared BDD nodes = " << Cudd_SharingSize( vNodes.data(), vNodes.size() ) << std::endl;
      std::cout << "Sum of BDD nodes = " << count << std::endl;
    }
  };
}

#endif
