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
    int nUniqueMinRate = 25; // Int 1 100
    int nCallThold = 200000; // Log 1000 1000000000
    bool fRealloc = 1; // None True
    bool fGC = 1; // Bool
    uint32_t nGC = 1 << 25; // Log 1000 1000000000
    bool fReo = 0; // None False
    uint32_t nReo = 4000; // None 1000 1000000000
    int nMaxGrowth = 20; // None 1 100
    // end
    
    AtBddParam( std::string fname = "_AtBddMan.hpp_setting.txt" )
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
      nUnique = std::stoul( str );
      std::getline( f, str );
      nCache = std::stoul( str );
      std::getline( f, str );
      nUniqueMinRate = std::stoi( str );
      std::getline( f, str );
      nCallThold = std::stoi( str );
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
  
  class AtBddMan : public BddMan<AtBdd::lit>
  {
  private:
    AtBdd::BddMan * man;
    AtBddParam param;
    
  public:
    AtBddMan( int nVars, AtBddParam param ) : param( param )
    {
      man = new AtBdd::BddMan( nVars, param.nNodes, param.nUnique, param.nCache, param.nUniqueMinRate, param.nCallThold, NULL, 0 );
      man->RefreshConfig( param.fRealloc, param.fGC, param.nGC, param.fReo, param.nReo, param.nMaxGrowth );
    };
    AtBddMan( int nVars ) : AtBddMan( nVars, AtBddParam() ) {}
    ~AtBddMan() { delete man; }
    
    int GetNumVar() override { return man->get_nVars(); }
    uint64_t Id( AtBdd::lit const & x ) { return (uint64_t)x; }
    
    AtBdd::lit Const0() override { return man->LitConst0(); }
    AtBdd::lit Const1() override { return man->LitConst1(); }
    AtBdd::lit IthVar( int i ) override { return man->LitIthVar( i ); }
    AtBdd::lit Regular( AtBdd::lit const & x ) override { return man->LitRegular( x ); }
    bool IsCompl( AtBdd::lit const & x ) override { return man->LitIsCompl( x ); }
    int Var( AtBdd::lit const & x ) override { return man->get_order( man->Var( x ) ); }
    AtBdd::lit Then( AtBdd::lit const & x ) override { return man->Then( x ); }
    AtBdd::lit Else( AtBdd::lit const & x ) override { return man->Else( x ); }
    AtBdd::lit Not( AtBdd::lit const & x ) override { return man->LitNot( x ); }
    
    void Ref( AtBdd::lit const & x ) override { man->Ref( x ); }
    void Deref( AtBdd::lit const & x ) override { man->Deref( x ); }
    void Pop( AtBdd::lit const & x ) override { (void)x; man->Pop(); }
    void SupportRef() { man->SupportRef(); }
    void UnsupportRef() { man->UnsupportRef(); }
    
    AtBdd::lit And( AtBdd::lit const & x, AtBdd::lit const & y ) override { return man->And( x, y ); }
    AtBdd::lit Or( AtBdd::lit const & x, AtBdd::lit const & y ) override { return man->Or( x, y ); }
    AtBdd::lit Xor( AtBdd::lit const & x, AtBdd::lit const & y ) override { return man->Xor( x, y ); }

    void PrintStats( std::vector<AtBdd::lit> & vNodes ) override
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
