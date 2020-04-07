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
    uint32_t nNodes = 1 << 20; // Pow 0 31
    uint32_t nCache = 1 << 18; // Pow 10 31 
    int  nVerbose = 0; // None 0
    bool fGC = 1; // None 1
    bool fRealloc = 1; // None 1
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
	nCache = std::stoul( str );
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
  
  class AtBddMan : public BddMan<AtBdd::lit>
  {
  private:
    AtBdd::BddManWrap * man;
    
  public:
    AtBddMan( int nVars )
    {
      AtBddParam p;
      if( nVars < (int)std::numeric_limits<uint8_t>::max() )
	{
	  man = new AtBdd::BddMan<uint8_t>( nVars, p.nNodes, p.nCache, NULL, p.nVerbose );
	}
      else if( nVars < (int)std::numeric_limits<uint16_t>::max() )
	{
	  man = new AtBdd::BddMan<uint16_t>( nVars, p.nNodes, p.nCache, NULL, p.nVerbose );
	}
      else
	{
	  assert(0);
	}
      man->RefreshConfig( p.fRealloc, p.fGC, p.nMaxGrowth );
    };

    AtBddMan( int nVars, AtBddParam p )
    {
      if( nVars < (int)std::numeric_limits<uint8_t>::max() )
	{
	  man = new AtBdd::BddMan<uint8_t>( nVars, p.nNodes, p.nCache, NULL, p.nVerbose );
	}
      else if( nVars < (int)std::numeric_limits<uint16_t>::max() )
	{
	  man = new AtBdd::BddMan<uint16_t>( nVars, p.nNodes, p.nCache, NULL, p.nVerbose );
	}
      else
	{
	  assert(0);
	}
      man->RefreshConfig( p.fRealloc, p.fGC, p.nMaxGrowth );
    };
    ~AtBddMan() { delete man; }
    AtBdd::lit Const0() override { return man->LitConst0(); }
    AtBdd::lit Const1() override { return man->LitConst1(); }
    AtBdd::lit IthVar( int i ) override { return man->LitIthVar_( i ); }
    AtBdd::lit Regular( AtBdd::lit const & x ) override { return man->LitRegular( x ); }
    bool IsCompl( AtBdd::lit const & x ) override { return man->LitIsCompl( x ); }
    AtBdd::lit Not( AtBdd::lit const & x ) override { return man->LitNot( x ); }
    int Var( AtBdd::lit const & x ) override { return man->get_order( man->Var_( x ) ); }
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
