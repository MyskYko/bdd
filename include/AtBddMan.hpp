#ifndef AT_BDD_MAN_HPP_
#define AT_BDD_MAN_HPP_

#include <fstream>
#include "BddMan.hpp"
#include <AtBdd.hpp>

namespace Bdd
{
  struct AtBddParam
  {
    // Param
    uint32_t nNodes = 1 << 20; // Pow 10 30
    uint32_t nUnique = 1 << 18; // Pow 10 30
    uint32_t nCache = 1 << 18; // Pow 10 30
    bool fGC = 1; // Bool
    bool fRealloc = 1; // None True
    int  nMaxGrowth = 0; // None 0
    // end
    
    AtBddParam( std::string fname = "_AtBddMan.hpp_setting.txt" )
    {
      std::ifstream f( fname );
      if ( !f )
	return;
      std::string str;
      if ( std::getline( f, str ) )
	nNodes = std::stoul( str );
      if ( std::getline( f, str ) )
	nUnique = std::stoul( str );
      if ( std::getline( f, str ) )
	nCache = std::stoul( str );
      if ( std::getline( f, str ) )
	fGC = ( str == "True" );
      if ( std::getline( f, str ) )
	fRealloc = ( str == "True" );
      if ( std::getline( f, str ) )
	nMaxGrowth = std::stoi( str );
    }
  };
  
  class AtBddMan : public BddMan<AtBdd::lit>
  {
  private:
    AtBdd::BddMan * man;
    
  public:
    AtBddMan( int nVars )
    {
      AtBddParam p;
      man = new AtBdd::BddMan( nVars, p.nNodes, p.nUnique, p.nCache, NULL, 0 );
      man->RefreshConfig( p.fRealloc, p.fGC, p.nMaxGrowth );
    };

    AtBddMan( int nVars, AtBddParam p )
    {
      man = new AtBdd::BddMan( nVars, p.nNodes, p.nUnique, p.nCache, NULL, 0 );
      man->RefreshConfig( p.fRealloc, p.fGC, p.nMaxGrowth );
    };
    ~AtBddMan() { delete man; }
    AtBdd::lit Const0() override { return man->LitConst0(); }
    AtBdd::lit Const1() override { return man->LitConst1(); }
    AtBdd::lit IthVar( int i ) override { return man->LitIthVar( i ); }
    AtBdd::lit Regular( AtBdd::lit const & x ) override { return man->LitRegular( x ); }
    bool IsCompl( AtBdd::lit const & x ) override { return man->LitIsCompl( x ); }
    AtBdd::lit Not( AtBdd::lit const & x ) override { return man->LitNot( x ); }
    int Var( AtBdd::lit const & x ) override { return man->get_order( man->Var( x ) ); }
    AtBdd::lit Then( AtBdd::lit const & x ) override { return man->Then( x ); }
    AtBdd::lit Else( AtBdd::lit const & x ) override { return man->Else( x ); }
    void Ref( AtBdd::lit const & x ) override { man->Ref( x ); }
    void Deref( AtBdd::lit const & x ) override { man->Deref( x ); }
    AtBdd::lit And( AtBdd::lit const & x, AtBdd::lit const & y ) override { return man->And( x, y ); }
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

    uint64_t Id( AtBdd::lit const & x ) { return (uint64_t)x; }
  };
}

#endif
