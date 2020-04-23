#ifndef CUDD_MAN_HPP_
#define CUDD_MAN_HPP_

#include <iostream>
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
    
  class CuddMan : public BddMan<BDD>
  {
  private:
    Cudd * man;
    CuddParam param;
    
  public:
    CuddMan( int nVars, CuddParam param ) : param( param )
    {
      man = new Cudd( nVars, 0, param.nUnique, param.nCache, param.nMaxMem );
      man->SetMinHit( param.nMinHit );
      if ( !param.fGC )
	{
	  man->DisableGarbageCollection();
	}
      if ( param.fReo )
	{
	  man->AutodynEnable( (Cudd_ReorderingType)( CUDD_REORDER_SIFT + param.nReoScheme ) );
	}
      man->SetMaxGrowth( 1.0 + param.nMaxGrowth * 0.01 );
    }
    CuddMan( int nVars ) : CuddMan( nVars, CuddParam() ) {}
    ~CuddMan() { delete man; }

    int GetNumVar() override { return man->ReadSize(); }
    uint64_t Id( BDD const & x ) { return (uint64_t)x.getNode(); }

    BDD Const0() override { return !Const1(); }
    BDD Const1() override { return man->bddOne(); }
    BDD IthVar( int i ) override { return man->ReadVars( i ); }

    int Var( BDD const & x ) override { return x.NodeReadIndex(); }
    BDD Then( BDD const & x ) override
    {
      auto y = BDD( *man, Cudd_T( x.getNode() ) );
      return IsCompl( x ) ? !y : y;
    }
    BDD Else( BDD const & x ) override
    {
      auto y = BDD( *man, Cudd_E( x.getNode() ) );
      return IsCompl( x ) ? !y : y;
    }

    BDD Regular( BDD const & x ) override { return BDD( *man, Cudd_Regular( x.getNode() ) ); }
    bool IsCompl( BDD const & x ) override { return Cudd_IsComplement( x.getNode() ); }
    
    int Level( int i ) override { return man->ReadPerm( i ); }
    void Reorder() override { man->ReduceHeap( (Cudd_ReorderingType)( CUDD_REORDER_SIFT + param.nReoScheme ) ); }
    
    BDD Not( BDD const & x ) override { return !x; }
    BDD And( BDD const & x, BDD const & y ) override { return x & y; }
    BDD Or( BDD const & x, BDD const & y ) override { return x | y ; }
    BDD Xor( BDD const & x, BDD const & y ) override { return x ^ y; }
    BDD Ite( BDD const & c, BDD const & x, BDD const & y ) override { return c.Ite( x, y ); }
    BDD Exist( BDD const & x, BDD const & cube ) override { return x.ExistAbstract( cube ); }
    BDD Univ( BDD const & x, BDD const & cube ) override { return x.UnivAbstract( cube ); }
    BDD AndExist( BDD const & x, BDD const & y, BDD const & cube ) override { return x.AndAbstract( y, cube ); }
    BDD Restrict( BDD const & x, BDD const & c ) override { return x.Restrict( c ); }
    BDD Compose( BDD const & x, int i, BDD const & c ) override { return x.Compose( c, i ); }
    BDD VecCompose( BDD const & x, std::vector<BDD> & cs ) override { return x.VectorCompose( cs ); }
    
    void Support( BDD const & x, std::vector<int> & vVars ) override
    {
      BDD y = x.Support();
      while ( y != Const1() )
	{
	  vVars.push_back( Var( y ) );
	  y = Then( y );
	}
    }
    
    void PrintStats( std::vector<BDD> & vNodes ) override
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
