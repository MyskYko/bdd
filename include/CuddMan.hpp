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
    
  class CuddMan : public BddMan<Cudd::DdNode *>
  {
  private:
    Cudd::DdManager * man;
    
  public:
    CuddMan( int nVars )
    {
      using namespace Cudd;
      CuddParam p;
      man = Cudd_Init( nVars, 0, p.nNodes, p.nCache, p.nMaxMem );
    }
    CuddMan( int nVars, CuddParam p )
    {
      using namespace Cudd;
      man = Cudd_Init( nVars, 0, p.nNodes, p.nCache, p.nMaxMem );
    }
    ~CuddMan() { using namespace Cudd; Cudd_Quit( man ); }
    Cudd::DdNode * Const0() override { using namespace Cudd; return Cudd_Not( Cudd_ReadOne( man ) ); }
    Cudd::DdNode * Const1() override { using namespace Cudd; return Cudd_ReadOne( man ); }
    Cudd::DdNode * IthVar( int i ) override { using namespace Cudd; return Cudd_ReadVars( man, i ); }
    Cudd::DdNode * Regular( Cudd::DdNode * const & x ) override { using namespace Cudd; return Cudd_Regular( x ); }
    bool     IsCompl( Cudd::DdNode * const & x ) override { using namespace Cudd; return Cudd_IsComplement( x ); }
    int      Var( Cudd::DdNode * const & x ) override { using namespace Cudd; return Cudd_NodeReadIndex( x ); }
    Cudd::DdNode * Then( Cudd::DdNode * const & x ) override { using namespace Cudd; return Cudd_NotCond( Cudd_T( x ), IsCompl( x ) ); }
    Cudd::DdNode * Else( Cudd::DdNode * const & x ) override { using namespace Cudd; return Cudd_NotCond( Cudd_E( x ), IsCompl( x ) ); }
    void     Ref( Cudd::DdNode * const & x ) override { using namespace Cudd; Cudd_Ref( x ); }
    void     Deref( Cudd::DdNode * const & x ) override { using namespace Cudd; Cudd_RecursiveDeref( man, x ); }
    Cudd::DdNode * NotCond( Cudd::DdNode * const & x, bool c ) override { using namespace Cudd; return Cudd_NotCond( x, c ); }
    Cudd::DdNode * And( Cudd::DdNode * const & x, Cudd::DdNode * const & y ) override { using namespace Cudd; return Cudd_bddAnd( man, x, y ); }
    int      GetNumVar() override { using namespace Cudd; return Cudd_ReadSize( man ); }
    void     PrintStats() override
    {
      using namespace Cudd; 
      uint64_t count = 0;
      for ( uint32_t i = 0; i < vNodes.size(); i++ )
	count += Cudd_DagSize( vNodes[i] );
      std::cout << "Shared BDD nodes = " << Cudd_SharingSize( vNodes.data(), vNodes.size() ) << std::endl;
      std::cout << "Sum of BDD nodes = " << count << std::endl;
    }

    uint64_t Id( Cudd::DdNode * const & x ) { return (uint64_t)x; }
  };
}


#endif
