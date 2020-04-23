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
    bool fGC = 1; // Bool
    uint32_t nGC = 1 << 25; // Log 100 1000000000
    bool fReo = 0; // None False
    uint32_t nReo = 4000; // None 100 1000000000
    int nMaxGrowth = 20; // None 1 100
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
      fReo = ( str == "True" );
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
    SimpleBddMan( int nVars, SimpleBddParam param ) : param( param )
    {
      man = new SimpleBdd::BddMan( nVars, param.nNodes, NULL, 0 );
      man->RefreshConfig( param.fRealloc, param.fGC, param.nGC, param.fReo, param.nReo, param.nMaxGrowth );
    };
    SimpleBddMan( int nVars ) : SimpleBddMan( nVars, SimpleBddParam() ) {}
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
    void Reorder() override { man->Reorder(); }
    
    SimpleBddNode Not( SimpleBddNode const & x ) override { return SimpleBddNode( man, man->LitNot( x.val ) ); }
    SimpleBddNode And( SimpleBddNode const & x, SimpleBddNode const & y ) override { return SimpleBddNode( man, man->And( x.val, y.val ) ); }
    SimpleBddNode Or( SimpleBddNode const & x, SimpleBddNode const & y ) override { return SimpleBddNode( man, man->Or( x.val, y.val ) ); }
    SimpleBddNode Xor( SimpleBddNode const & x, SimpleBddNode const & y ) override { return SimpleBddNode( man, man->Xor( x.val, y.val ) ); }

    void SupportRef() override { man->SupportRef(); }
    void UnsupportRef() override { man->UnsupportRef(); }
    
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
