#ifndef BUDDY_MAN_HPP_
#define BUDDY_MAN_HPP_

#include <fstream>
#include "BddMan.hpp"
#include <bdd.h>

namespace Bdd
{
  struct BuddyParam
  {
    // Param
    int nNodes = 1000000; // Log 100 1000000000
    int nMaxInc = 1000000; // Log 100 1000000000
    int nCache = 10000; // Log 100 1000000000
    bool fDynCache = 1; // Bool
    int nDynCache = 4; // Int 1 100
    int nMinFree = 20; // Int 1 100
    bool fReo = 0; // None False
    int nReoScheme = 3; // None 6
    // end

    BuddyParam( std::string fname = "_BuddyMan.hpp_setting.txt" )
    {
      std::ifstream f( fname );
      if ( !f )
	{
	  return;
	}
      std::string str;
      std::getline( f, str );
      nNodes = std::stoi( str );
      std::getline( f, str );
      nMaxInc = std::stoi( str );
      std::getline( f, str );
      nCache = std::stoi( str );
      std::getline( f, str );
      fDynCache = ( str == "True" );
      std::getline( f, str );
      nDynCache = std::stoi( str );
      std::getline( f, str );
      nMinFree = std::stoi( str );
      std::getline( f, str );
      fReo = ( str == "True" );
      std::getline( f, str );
      nReoScheme = std::stoi( str );
    }
  };
    
  class BuddyMan : public BddMan<Buddy::bdd>
  {
  private:
    BuddyParam param;
    
  public:
    BuddyMan( int nVars, BuddyParam param ) : param( param )
    {
      Buddy::bdd_init( param.nNodes, param.nCache );
      Buddy::bdd_gbc_hook( NULL );
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
    uint64_t Id( Buddy::bdd const & x ) { return (uint64_t)x.id(); }
    
    Buddy::bdd Const0() override { return Buddy::bdd_false(); }
    Buddy::bdd Const1() override { return Buddy::bdd_true(); }
    Buddy::bdd IthVar( int i ) override { return Buddy::bdd_ithvar( i ); }

    int Var( Buddy::bdd const & x ) override { return Buddy::bdd_var( x ); }
    Buddy::bdd Then( Buddy::bdd const & x ) override { return Buddy::bdd_high( x ); }
    Buddy::bdd Else( Buddy::bdd const & x ) override { return Buddy::bdd_low( x ); }
    
    int Level( int i ) override { return Buddy::bdd_var2level( i ); }
    void Reorder() override { Buddy::bdd_reorder( param.nReoScheme + 1 ); }
    void Dvr() override { Buddy::bdd_autoreorder( param.nReoScheme + 1 ); }
    void DvrOff() override { Buddy::bdd_autoreorder( 0 ); }

    Buddy::bdd Not( Buddy::bdd const & x ) override { return Buddy::bdd_not( x ); }
    Buddy::bdd And( Buddy::bdd const & x, Buddy::bdd const & y ) override { return Buddy::bdd_and( x, y ); }
    Buddy::bdd Or( Buddy::bdd const & x, Buddy::bdd const & y ) override { return Buddy::bdd_or( x, y ); }
    Buddy::bdd Xor( Buddy::bdd const & x, Buddy::bdd const & y ) override { return Buddy::bdd_xor( x, y ); }
    Buddy::bdd Ite( Buddy::bdd const & c, Buddy::bdd const & x, Buddy::bdd const & y ) override { return Buddy::bdd_ite( c, x, y ); }
    Buddy::bdd Exist( Buddy::bdd const & x, Buddy::bdd const & cube ) override { return Buddy::bdd_exist( x, cube ); }
    Buddy::bdd Univ( Buddy::bdd const & x, Buddy::bdd const & cube ) override { return Buddy::bdd_forall( x, cube ); }
    Buddy::bdd AndExist( Buddy::bdd const & x, Buddy::bdd const & y, Buddy::bdd const & cube ) override { return Buddy::bdd_appex( x, y, bddop_and, cube ); }
    Buddy::bdd Restrict( Buddy::bdd const & x, Buddy::bdd const & c ) override { return Buddy::bdd_simplify( x, c ); }
    Buddy::bdd Compose( Buddy::bdd const & x, int i, Buddy::bdd const & c ) override { return Buddy::bdd_compose( x, c, i ); }
    Buddy::bdd VecCompose( Buddy::bdd const & x, std::vector<Buddy::bdd> & cs ) override
    {
      auto ps = Buddy::bdd_newpair();
      for ( int i = 0; i < GetNumVar(); i++ )
	{
	  Buddy::bdd_setbddpair( ps, i, cs[i] );
	}
      auto y = Buddy::bdd_veccompose( x, ps );
      Buddy::bdd_freepair( ps );
      return y;
    }
    
    void Support( Buddy::bdd const & x, std::vector<int> & vVars ) override
    {
      auto y = Buddy::bdd_support( x );
      int * pVars;
      int nVars;
      Buddy::bdd_scanset( y, pVars, nVars );
      for( int i = 0; i < nVars; i++ )
	{
	  vVars.push_back( pVars[i] );
	}
      free( pVars );
    }

    void PrintStats( std::vector<Buddy::bdd> & vNodes ) override
    {
      uint64_t count = 0;
      for ( uint32_t i = 0; i < vNodes.size(); i++ )
	{
	  count += Buddy::bdd_nodecount( vNodes[i] );
	}
      std::cout << "Shared BDD nodes = " << bdd_anodecount( vNodes.data(), vNodes.size() ) << std::endl;
      std::cout << "Sum of BDD nodes = " << count << std::endl;
    }
  };
}

#endif
