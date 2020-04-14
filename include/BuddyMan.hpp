#ifndef BUDDY_MAN_HPP_
#define BUDDY_MAN_HPP_

#include <iostream>
#include <fstream>
#include "BddMan.hpp"
namespace Buddy
{
  extern "C"
  {
    #include <bdd.h>
  }
}

namespace Bdd
{
  struct BuddyParam
  {
    // Param
    int nNodes = 1000000; // Log 1000 1000000000
    int nMaxInc = 1000000; // Log 1000 1000000000
    int nCache = 10000; // Log 1000 1000000000
    bool fDynCache = 1; // Bool
    int nDynCache = 4; // Log 1 1000
    int nMinFree = 20; // Int 0 100
    bool fReo = 0; // None False
    int nReoScheme = 3; // Switch 6
    // end

    BuddyParam( std::string fname = "_BuddyMan.hpp_setting.txt" )
    {
      std::ifstream f( fname );
      if ( !f )
	return;
      std::string str;
      if ( std::getline( f, str ) )
	nNodes = std::stoi( str );
      if ( std::getline( f, str ) )
	nMaxInc = std::stoi( str );
      if ( std::getline( f, str ) )
	nCache = std::stoi( str );
      if ( std::getline( f, str ) )
	fDynCache = ( str == "True" );
      if ( std::getline( f, str ) )
	nDynCache = std::stoi( str );
      if ( std::getline( f, str ) )
	nMinFree = std::stoi( str );
      if ( std::getline( f, str ) )
	fReo = ( str == "True" );
      if ( std::getline( f, str ) )
	nReoScheme = std::stoi( str );
    }
  };
    
  class BuddyMan : public BddMan<Buddy::BDD>
  {
  private:
    BuddyParam param;
    
  public:
    BuddyMan( int nVars, BuddyParam param ) : param( param )
    {
      Buddy::bdd_init( param.nNodes, param.nCache );
      Buddy::bdd_setmaxincrease( param.nMaxInc );
      if ( param.fDynCache )
	{
	  Buddy::bdd_setcacheratio( param.nDynCache );
	}
      Buddy::bdd_setminfreenodes( param.nMinFree );
      Buddy::bdd_setvarnum( nVars );
      Buddy::bdd_varblockall();
      if ( param.fReo )
	{
	  Buddy::bdd_autoreorder( param.nReoScheme + 1 );
	}
    };
    BuddyMan( int nVars ) : BuddyMan( nVars, BuddyParam() ) {}
    ~BuddyMan() { Buddy::bdd_done(); }
    
    int  GetNumVar() override { return Buddy::bdd_varnum(); }
    uint64_t Id( Buddy::BDD const & x ) { return (uint64_t)x; }
    
    Buddy::BDD Const0() override { return Buddy::bdd_false(); }
    Buddy::BDD Const1() override { return Buddy::bdd_true(); }
    Buddy::BDD IthVar( int i ) override { return Buddy::bdd_ithvar( i ); }
    Buddy::BDD Regular( Buddy::BDD const & x ) override { return x; }
    bool IsCompl( Buddy::BDD const & x ) override { (void)x; return 0; }
    int Var( Buddy::BDD const & x ) override { return Buddy::bdd_var( x ); }
    Buddy::BDD Then( Buddy::BDD const & x ) override { return Buddy::bdd_high( x ); }
    Buddy::BDD Else( Buddy::BDD const & x ) override { return Buddy::bdd_low( x ); }
    Buddy::BDD Not( Buddy::BDD const & x ) override { return Buddy::bdd_not( x ); }
    
    void Ref( Buddy::BDD const & x ) override { Buddy::bdd_addref( x ); }
    void Deref( Buddy::BDD const & x ) override { Buddy::bdd_delref( x ); }
    void RefNot( Buddy::BDD const & x ) override { Buddy::bdd_addref( x ); }
    void DerefNot( Buddy::BDD const & x ) override { Buddy::bdd_delref( x ); }

    int Perm( int i ) override { return Buddy::bdd_var2level( i ); }
    void Reorder() override { Buddy::bdd_reorder( param.nReoScheme + 1 ); }

    Buddy::BDD And( Buddy::BDD const & x, Buddy::BDD const & y ) override { return Buddy::bdd_and( x, y ); }
    Buddy::BDD Or( Buddy::BDD const & x, Buddy::BDD const & y ) override { return Buddy::bdd_or( x, y ); }
    Buddy::BDD Xor( Buddy::BDD const & x, Buddy::BDD const & y ) override { return Buddy::bdd_xor( x, y ); }
    Buddy::BDD Ite( Buddy::BDD const & c, Buddy::BDD const & x, Buddy::BDD const & y ) override { return Buddy::bdd_ite( c, x, y ); }
    Buddy::BDD Exist( Buddy::BDD const & x, Buddy::BDD const & cube ) override { return Buddy::bdd_exist( x, cube ); }
    Buddy::BDD Univ( Buddy::BDD const & x, Buddy::BDD const & cube ) override { return Buddy::bdd_forall( x, cube ); }
    Buddy::BDD AndExist( Buddy::BDD const & x, Buddy::BDD const & y, Buddy::BDD const & cube ) override { return Buddy::bdd_appex( x, y, bddop_and, cube ); }
    Buddy::BDD Restrict( Buddy::BDD const & x, Buddy::BDD const & c ) override { return Buddy::bdd_simplify( x, c ); }
    Buddy::BDD Compose( Buddy::BDD const & x, int i, Buddy::BDD const & c ) override { return Buddy::bdd_compose( x, c, IthVar( i ) ); }
    Buddy::BDD VecCompose( Buddy::BDD const & x, std::vector<Buddy::BDD> & cs ) override
    {
      auto ps = Buddy::bdd_newpair();
      for ( int i = 0; i < GetNumVar(); i++ )
	{
	  Buddy::bdd_setbddpair( ps, i, cs[i] );
	}
      Buddy::BDD y = Buddy::bdd_veccompose( x, ps );
      Buddy::bdd_freepair( ps );
      return y;
    }
    
    void Support( Buddy::BDD const & x, std::vector<int> & vVars ) override
    {
      Buddy::BDD y = Buddy::bdd_support( x );
      int * pVars;
      int nVars;
      Buddy::bdd_scanset( y, &pVars, &nVars );
      for( int i = 0; i < nVars; i++ )
	{
	  vVars.push_back( pVars[i] );
	}
      free( pVars );
    }

    void PrintStats( std::vector<Buddy::BDD> & vNodes ) override
    {
      uint64_t count = 0;
      for ( uint32_t i = 0; i < vNodes.size(); i++ )
	{
	  count += Buddy::bdd_nodecount( vNodes[i] );
	}
      std::cout << "Shared BDD nodes = " << Buddy::bdd_anodecount( vNodes.data(), vNodes.size() ) << std::endl;
      std::cout << "Sum of BDD nodes = " << count << std::endl;
    }
  };
}

#endif
