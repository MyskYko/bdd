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
    uint32_t nNodes = 1 << 20; // Pow 7 30
    bool fRealloc = 1; // None True
    bool fGC = 0; // Bool
    uint32_t nGC = 1 << 25; // Log 100 1000000000
    uint32_t nReo = 4000; // Log 100 1000000000
    int nMaxGrowth = 20; // Int 1 100
    // end
    
    SimpleBddParam( std::string fname = "_SimpleBddMan.hpp_setting.txt" )
    {
      std::ifstream f( fname );
      if ( !f )
	{
	  return;
	}
      std::string str;
      std::getline( f, str );
      nNodes = std::stoul( str );
      std::getline( f, str );
      fRealloc = ( str == "True" );
      std::getline( f, str );
      fGC = ( str == "True" );
      std::getline( f, str );
      nGC = std::stoul( str );
      std::getline( f, str );
      nReo = std::stoul( str );
      std::getline( f, str );
      nMaxGrowth = std::stoi( str );
    }
  };

  class SimpleBddNode
  {
  private:
    friend class SimpleBddMan;
    SimpleBdd::BddMan * man;
    SimpleBdd::lit val;
    
  public:
    SimpleBddNode( SimpleBdd::BddMan * man, SimpleBdd::lit val ) : man( man ), val( val )
    {
      man->Ref( val );
    }
    SimpleBddNode()
    {
      man = NULL;
    }
    SimpleBddNode( const SimpleBddNode & right )
    {
      man = right.man;
      val = right.val;
      man->Ref( val );
    }
    ~SimpleBddNode()
    {
      if ( man )
	{
	  man->Deref( val );
	}
    }
    SimpleBddNode & operator=( const SimpleBddNode & right )
    {
      if (this == &right)
	{
	  return *this;
	}
      if ( man )
	{
	  man->Deref( val );
	}
      val = right.val;
      man = right.man;
      man->Ref( val );
      return *this;
    }
    bool operator==( const SimpleBddNode & other ) const
    {
      return val == other.val;
    }
    bool operator!=( const SimpleBddNode & other ) const
    {
      return val != other.val;
    }
  };
  
  class SimpleBddMan : public BddMan<SimpleBddNode>
  {
  private:
    SimpleBdd::BddMan * man;
    SimpleBddParam param;
    
  public:
    SimpleBddMan( int nVars, SimpleBddParam param, int nVerbose ) : param( param )
    {
      man = new SimpleBdd::BddMan( nVars, param.nNodes, NULL, nVerbose );
      man->RefreshConfig( param.fRealloc, param.fGC, param.nGC, 0, param.nReo, param.nMaxGrowth );
    };
    SimpleBddMan( int nVars, int nVerbose = 0 ) : SimpleBddMan( nVars, SimpleBddParam(), nVerbose ) {}
    ~SimpleBddMan() { delete man; }
    
    int GetNumVar() override { return man->get_nVars(); }
    uint64_t Id( SimpleBddNode const & x ) { return (uint64_t)x.val; }
    
    SimpleBddNode Const0() override { return SimpleBddNode( man, man->LitConst0() ); }
    SimpleBddNode Const1() override { return SimpleBddNode( man, man->LitConst1() ); }
    SimpleBddNode IthVar( int i ) override { return SimpleBddNode( man, man->LitIthVar( i ) ); }
    
    int Var( SimpleBddNode const & x ) override { return man->get_order( man->Var( x.val ) ); }
    SimpleBddNode Then( SimpleBddNode const & x ) override { return SimpleBddNode( man, man->Then( x.val ) ); }
    SimpleBddNode Else( SimpleBddNode const & x ) override { return SimpleBddNode( man, man->Else( x.val ) ); }
    
    SimpleBddNode Regular( SimpleBddNode const & x ) override { return SimpleBddNode( man, man->LitRegular( x.val ) ); }
    bool IsCompl( SimpleBddNode const & x ) override { return man->LitIsCompl( x.val ); }

    int Level( int i ) override { return man->Var( man->LitIthVar( i ) ); }
    void Reorder() override { if ( man->get_pvNodesExists() ) man->Reorder(); }
    void Dvr() override
    {
      if ( man->get_nObjs() != 1 + man->get_nVars() )
	{
	  std::cerr << "dvr is not turned on because there are nodes already built" << std::endl;
	  return;
	}
      man->Dvr();
      man->SupportRef();
    }
    void DvrOff() override { man->DvrOff(); man->UnsupportRef(); }
    
    SimpleBddNode Not( SimpleBddNode const & x ) override { return SimpleBddNode( man, man->LitNot( x.val ) ); }
    SimpleBddNode And( SimpleBddNode const & x, SimpleBddNode const & y ) override { return SimpleBddNode( man, man->And( x.val, y.val ) ); }

    void PrintStats( std::vector<SimpleBddNode> & vNodes ) override
    {
      uint64_t count = 0;
      std::vector<SimpleBdd::lit> v;
      for ( uint32_t i = 0; i < vNodes.size(); i++ )
	{
	  count += man->CountNodes( vNodes[i].val );
	  v.push_back( vNodes[i].val );
	}
      std::cout << "Shared BDD nodes = " << man->CountNodesArrayShared( v ) << std::endl;
      std::cout << "Sum of BDD nodes = " << count << std::endl;
    }
  };
}

#endif
