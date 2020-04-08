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
    uint32_t  nNodes = 1; // Pow 0 31
    int  nVerbose = 0; // None 0
    bool fGC = 1; // None 0
    bool fRealloc = 1; // None 1
    int  nMaxGrowth = 10; // None 0
    // end
    
    SimpleBddParam( std::string fname = "_SimpleBddMan.hpp_setting.txt" )
    {
      std::ifstream f( fname );
      if ( !f )
	return;
      std::string str;
      if ( std::getline( f, str ) )
	nNodes = std::stoul( str );
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
  
  class SimpleBddMan : public BddMan<SimpleBdd::lit>
  {
  private:
    SimpleBdd::BddMan * man;
    
  public:
    SimpleBddMan( int nVars )
    {
      SimpleBddParam p;
      man = new SimpleBdd::BddMan( nVars, p.nNodes, NULL, p.nVerbose );
      man->RefreshConfig( p.fRealloc, p.fGC, p.nMaxGrowth );
    };

    SimpleBddMan( int nVars, SimpleBddParam p )
    {
      man = new SimpleBdd::BddMan( nVars, p.nNodes, NULL, p.nVerbose );
      man->RefreshConfig( p.fRealloc, p.fGC, p.nMaxGrowth );
    };
    ~SimpleBddMan() { delete man; }
    SimpleBdd::lit Const0() override { return man->LitConst0(); }
    SimpleBdd::lit Const1() override { return man->LitConst1(); }
    SimpleBdd::lit IthVar( int i ) override { return man->LitIthVar( i ); }
    SimpleBdd::lit Regular( SimpleBdd::lit const & x ) override { return man->LitRegular( x ); }
    bool IsCompl( SimpleBdd::lit const & x ) override { return man->LitIsCompl( x ); }
    SimpleBdd::lit Not( SimpleBdd::lit const & x ) override { return man->LitNot( x ); }
    int Var( SimpleBdd::lit const & x ) override { return man->get_order( man->Var( x ) ); }
    SimpleBdd::lit Then( SimpleBdd::lit const & x ) override { return man->Then( x ); }
    SimpleBdd::lit Else( SimpleBdd::lit const & x ) override { return man->Else( x ); }
    void Ref( SimpleBdd::lit const & x ) override { man->Ref( x ); }
    void Deref( SimpleBdd::lit const & x ) override { man->Deref( x ); }
    SimpleBdd::lit And( SimpleBdd::lit const & x, SimpleBdd::lit const & y ) override { return man->And( x, y ); }
    int GetNumVar() override { return man->get_nVars(); }
    void PrintStats() override
    {
      uint64_t count = 0;
      for ( uint32_t i = 0; i < vNodes.size(); i++ )
	{
	  count += man->CountNodes( vNodes[i] );
	}
      std::cout << "Shared BDD nodes = " << man->CountNodesArrayShared( vNodes ) << std::endl;
      std::cout << "Sum of BDD nodes = " << count << std::endl;
    }

    uint64_t Id( SimpleBdd::lit const & x ) { return (uint64_t)x; }
  };
}

#endif
