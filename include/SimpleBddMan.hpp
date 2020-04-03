#ifndef SIMPLE_BDD_MAN_HPP_
#define SIMPLE_BDD_MAN_HPP_

#include <fstream>
#include "BddMan.hpp"
#include <SimpleBdd.hpp>

namespace Bdd
{
  struct SimpleBddParam
  {
    // Param
    int  nNodes = 1; // Pow 0 30
    int  nVerbose = 0; // None 0
    bool fGC = 1; // None 1
    bool fRealloc = 1; // None 1
    int  nMaxGrowth = 0; // None 0
    // end
    
    SimpleBddParam( std::string fname = "_SimpleBddMan.hpp_setting.txt" )
    {
      std::ifstream f( fname );
      if ( !f )
	return;
      std::string str;
      if ( std::getline( f, str ) )
	nNodes = std::stoi( str );
      if ( std::getline( f, str ) )
	nVerbose = std::stoi( str );
      if ( std::getline( f, str ) )
	fGC = std::stoi( str );
      if ( std::getline( f, str ) )
	fRealloc = std::stoi( str );
      if ( std::getline( f, str ) )
	nMaxGrowth = std::stoi( str );
    }
  };
  
  template<typename var = uint8_t>
  class SimpleBddMan : public BddMan<uint32_t>
  {
  private:
    SimpleBdd::BddMan<var> * man;
    
  public:
    SimpleBddMan( int nVars )
    {
      assert( nVars < (int)std::numeric_limits<var>::max() );
      SimpleBddParam p;
      man = new SimpleBdd::BddMan<var>( nVars, p.nNodes, NULL, p.nVerbose );
      man->RefreshConfig( p.fGC, p.fRealloc, p.nMaxGrowth );
    };

    SimpleBddMan( int nVars, SimpleBddParam p )
    {
      assert( nVars < (int)std::numeric_limits<var>::max() );
      man = new SimpleBdd::BddMan<var>( nVars, p.nNodes, NULL, p.nVerbose );
      man->RefreshConfig( p.fGC, p.fRealloc, p.nMaxGrowth );
    };
    ~SimpleBddMan() { delete man; }
    uint32_t Const0() override { return man->LitConst0(); }
    uint32_t Const1() override { return man->LitConst1(); }
    uint32_t IthVar( int i ) override { return man->LitIthVar( i ); }
    uint32_t Regular( uint32_t const & x ) override { return man->LitRegular( x ); }
    bool IsCompl( uint32_t const & x ) override { return man->LitIsCompl( x ); }
    int Var( uint32_t const & x ) override { return man->get_order( man->Var( x ) ); }
    uint32_t Then( uint32_t const & x ) override { return man->Then( x ); }
    uint32_t Else( uint32_t const & x ) override { return man->Else( x ); }
    void Ref( uint32_t const & x ) override { man->Ref( x ); }
    void Deref( uint32_t const & x ) override { man->Deref( x ); }
    uint32_t NotCond( uint32_t const & x, bool c ) override { return man->LitNotCond( x, c ); }
    uint32_t And( uint32_t const & x, uint32_t const & y ) override { return man->And( x, y ); }
    int GetNumVar() override { return man->get_nVars(); }
    void PrintStats() override
    {
      std::cout << "Shared BDD nodes = " << man->CountNodesArrayShared( vNodes ) << std::endl;
      std::cout << "Sum of BDD nodes = " << man->CountNodesArrayIndependent( vNodes ) << std::endl;
    }
  };
}

#endif
