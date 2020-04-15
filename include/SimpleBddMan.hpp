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
    uint32_t nNodes = 1 << 20; // Pow 10 30
    bool fRealloc = 1; // None True
    bool fGC = 1; // Bool
    uint32_t nGC = 1 << 25; // Log 1000 1000000000
    bool fReo = 0; // None False
    uint32_t nReo = 4000; // Lob 1000 1000000000
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
      fReo = ( str == "True" );
      std::getline( f, str );
      nReo = std::stoul( str );
      std::getline( f, str );
      nMaxGrowth = std::stoi( str );
    }
  };
  
  class SimpleBddMan : public BddMan<SimpleBdd::lit>
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
    uint64_t Id( SimpleBdd::lit const & x ) { return (uint64_t)x; }
    
    SimpleBdd::lit Const0() override { return man->LitConst0(); }
    SimpleBdd::lit Const1() override { return man->LitConst1(); }
    SimpleBdd::lit IthVar( int i ) override { return man->LitIthVar( i ); }
    SimpleBdd::lit Regular( SimpleBdd::lit const & x ) override { return man->LitRegular( x ); }
    bool IsCompl( SimpleBdd::lit const & x ) override { return man->LitIsCompl( x ); }
    int Var( SimpleBdd::lit const & x ) override { return man->get_order( man->Var( x ) ); }
    SimpleBdd::lit Then( SimpleBdd::lit const & x ) override { return man->Then( x ); }
    SimpleBdd::lit Else( SimpleBdd::lit const & x ) override { return man->Else( x ); }
    SimpleBdd::lit Not( SimpleBdd::lit const & x ) override { return man->LitNot( x ); }
    
    void Ref( SimpleBdd::lit const & x ) override { man->Ref( x ); }
    void Deref( SimpleBdd::lit const & x ) override { man->Deref( x ); }
    void Pop( SimpleBdd::lit const & x ) override { (void)x; man->Pop(); }
    void SupportRef() override { man->SupportRef(); }
    void UnsupportRef() override { man->UnsupportRef(); }

    int Perm( int i ) override { return man->Var( IthVar( i ) ); }
    void Reorder() override { man->Reorder(); }
    
    SimpleBdd::lit And( SimpleBdd::lit const & x, SimpleBdd::lit const & y ) override { return man->And( x, y ); }
    SimpleBdd::lit Or( SimpleBdd::lit const & x, SimpleBdd::lit const & y ) override { return man->Or( x, y ); }
    SimpleBdd::lit Xor( SimpleBdd::lit const & x, SimpleBdd::lit const & y ) override { return man->Xor( x, y ); }
    
    void PrintStats( std::vector<SimpleBdd::lit> & vNodes ) override
    {
      uint64_t count = 0;
      for ( uint32_t i = 0; i < vNodes.size(); i++ )
	{
	  count += man->CountNodes( vNodes[i] );
	}
      std::cout << "Shared BDD nodes = " << man->CountNodesArrayShared( vNodes ) << std::endl;
      std::cout << "Sum of BDD nodes = " << count << std::endl;
    }
  };
}

#endif
