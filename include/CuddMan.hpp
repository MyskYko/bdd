#ifndef CUDD_MAN_HPP_
#define CUDD_MAN_HPP_

#include <fstream>
#include <string>
#include "BddMan.hpp"
#include <cuddObj.hh>

namespace Bdd
{
  struct CuddParam
  {
    // Param
    int nUnique = CUDD_UNIQUE_SLOTS; // Log 100 1000000000
    int nCache = CUDD_CACHE_SLOTS; // Log 100 1000000000
    int nMaxMem = 0; // Log 100 1000000000
    int nMinHit = 30; // Int 1 100
    bool fGC = 0; // Bool
    int nReoScheme = 0; // Switch 12
    int nMaxGrowth = 20; // Int 1 100
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
      nReoScheme = std::stoi( str );
      std::getline( f, str );
      nMaxGrowth = std::stoi( str );
    }
  };
    
  class CuddMan : public BddMan<CUDD::BDD>
  {
  private:
    CUDD::Cudd * man;
    CuddParam param;
    
  public:
    CuddMan( int nVars, CuddParam param, bool fVerbose ) : param( param )
    {
      man = new CUDD::Cudd( nVars, 0, param.nUnique, param.nCache, param.nMaxMem );
      man->SetMinHit( param.nMinHit );
      if ( !param.fGC )
	{
	  man->DisableGarbageCollection();
	}
      man->SetMaxGrowth( 1.0 + param.nMaxGrowth * 0.01 );
      if ( fVerbose )
	{
	  man->makeVerbose();
	}
    }
    CuddMan( int nVars, bool fVerbose = 0 ) : CuddMan( nVars, CuddParam(), fVerbose ) {}
    ~CuddMan() { delete man; }

    int GetNumVar() override { return man->ReadSize(); }
    uint64_t Id( CUDD::BDD const & x ) { return (uint64_t)x.getNode(); }

    CUDD::BDD Const0() override { return !Const1(); }
    CUDD::BDD Const1() override { return man->bddOne(); }
    CUDD::BDD IthVar( int i ) override { return man->ReadVars( i ); }

    int Var( CUDD::BDD const & x ) override { return x.NodeReadIndex(); }
    CUDD::BDD Then( CUDD::BDD const & x ) override
    {
      auto y = CUDD::BDD( *man, Cudd_T( x.getNode() ) );
      return IsCompl( x ) ? !y : y;
    }
    CUDD::BDD Else( CUDD::BDD const & x ) override
    {
      auto y = CUDD::BDD( *man, Cudd_E( x.getNode() ) );
      return IsCompl( x ) ? !y : y;
    }

    CUDD::BDD Regular( CUDD::BDD const & x ) override { return CUDD::BDD( *man, Cudd_Regular( x.getNode() ) ); }
    bool IsCompl( CUDD::BDD const & x ) override { return Cudd_IsComplement( x.getNode() ); }
    
    int Level( int i ) override { return man->ReadPerm( i ); }
    void Reorder() override { man->ReduceHeap( (Cudd_ReorderingType)( CUDD_REORDER_SIFT + param.nReoScheme ) ); }
    void Dvr() override { man->AutodynEnable( (Cudd_ReorderingType)( CUDD_REORDER_SIFT + param.nReoScheme ) ); }
    void DvrOff() override { man->AutodynDisable(); }
    
    CUDD::BDD Not( CUDD::BDD const & x ) override { return !x; }
    CUDD::BDD And( CUDD::BDD const & x, CUDD::BDD const & y ) override { return x & y; }
    CUDD::BDD Or( CUDD::BDD const & x, CUDD::BDD const & y ) override { return x | y ; }
    CUDD::BDD Xor( CUDD::BDD const & x, CUDD::BDD const & y ) override { return x ^ y; }
    CUDD::BDD Ite( CUDD::BDD const & c, CUDD::BDD const & x, CUDD::BDD const & y ) override { return c.Ite( x, y ); }
    CUDD::BDD Exist( CUDD::BDD const & x, CUDD::BDD const & cube ) override { return x.ExistAbstract( cube ); }
    CUDD::BDD Univ( CUDD::BDD const & x, CUDD::BDD const & cube ) override { return x.UnivAbstract( cube ); }
    CUDD::BDD AndExist( CUDD::BDD const & x, CUDD::BDD const & y, CUDD::BDD const & cube ) override { return x.AndAbstract( y, cube ); }
    CUDD::BDD Restrict( CUDD::BDD const & x, CUDD::BDD const & c ) override { return x.Restrict( c ); }
    CUDD::BDD Compose( CUDD::BDD const & x, int i, CUDD::BDD const & c ) override { return x.Compose( c, i ); }
    CUDD::BDD VecCompose( CUDD::BDD const & x, std::vector<CUDD::BDD> & cs ) override { return x.VectorCompose( cs ); }
    
    void Support( CUDD::BDD const & x, std::vector<int> & vVars ) override
    {
      CUDD::BDD y = x.Support();
      while ( y != Const1() )
	{
	  vVars.push_back( Var( y ) );
	  y = Then( y );
	}
    }
    
    void PrintStats( std::vector<CUDD::BDD> & vNodes ) override
    {
      uint64_t count = 0;
      for ( uint32_t i = 0; i < vNodes.size(); i++ )
	{
	  count += Cudd_DagSize( vNodes[i].getNode() );
	}
      std::cout << "Shared BDD nodes = " << man->SharingSize( vNodes ) << std::endl;
      std::cout << "Sum of BDD nodes = " << count << std::endl;
    }
  };
}


#endif
