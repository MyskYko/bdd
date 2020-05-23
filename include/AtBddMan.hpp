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
    uint32_t nNodes = 1 << 20; // Pow 7 30
    uint32_t nUnique = 1 << 18; // Pow 7 30
    uint32_t nCache = 1 << 18; // Pow 7 30
    int nUniqueMinRate = 25; // Int 1 100
    int nCallThold = 200000; // Log 100 1000000000
    bool fRealloc = 1; // None True
    bool fGC = 0; // Bool
    uint32_t nGC = 1 << 25; // Log 100 1000000000
    uint32_t nReo = 4000; // Log 100 1000000000
    int nMaxGrowth = 20; // Int 1 100
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
      nReo = std::stoul( str );
      std::getline( f, str );
      nMaxGrowth = std::stoi( str );
    }
  };

  class AtBddNode
  {
  private:
    friend class AtBddMan;
    AtBdd::BddMan * man;
    AtBdd::lit val;
    
  public:
    AtBddNode( AtBdd::BddMan * man, AtBdd::lit val ) : man( man ), val( val )
    {
      man->IncRef( val );
    }
    AtBddNode()
    {
      man = NULL;
    }
    AtBddNode( const AtBddNode & right )
    {
      man = right.man;
      val = right.val;
      man->IncRef( val );
    }
    ~AtBddNode()
    {
      if ( man )
	{
	  man->DecRef( val );
	}
    }
    AtBddNode & operator=( const AtBddNode & right )
    {
      if (this == &right)
	{
	  return *this;
	}
      if ( man )
	{
	  man->DecRef( val );
	}
      val = right.val;
      man = right.man;
      man->IncRef( val );
      return *this;
    }
    bool operator==( const AtBddNode & other ) const
    {
      return val == other.val;
    }
    bool operator!=( const AtBddNode & other ) const
    {
      return val != other.val;
    }
  };
  
  class AtBddMan : public BddMan<AtBddNode>
  {
  private:
    AtBdd::BddMan * man;
    AtBddParam param;
    
  public:
    AtBddMan( int nVars, AtBddParam param, int nVerbose, bool fRef ) : param( param )
    {
      man = new AtBdd::BddMan( nVars, param.nNodes, param.nUnique, param.nCache, param.nUniqueMinRate, param.nCallThold, NULL, nVerbose );
      if ( param.fGC )
	{
	  fRef = 1;
	}
      man->RefreshConfig( param.fRealloc, param.fGC, param.nGC, 0, param.nReo, param.nMaxGrowth, fRef );
    };
    AtBddMan( int nVars, int nVerbose = 0, bool fRef = 0 ) : AtBddMan( nVars, AtBddParam(), nVerbose, fRef) {}
    ~AtBddMan() { delete man; }
    
    int GetNumVar() override { return man->get_nVars(); }
    uint64_t Id( AtBddNode const & x ) { return (uint64_t)x.val; }
    
    AtBddNode Const0() override { return AtBddNode( man, man->LitConst0() ); }
    AtBddNode Const1() override { return AtBddNode( man, man->LitConst1() ); }
    AtBddNode IthVar( int i ) override { return AtBddNode( man, man->LitIthVar( i ) ); }
    
    int Var( AtBddNode const & x ) override { return man->get_order( man->Var( x.val ) ); }
    AtBddNode Then( AtBddNode const & x ) override { return AtBddNode( man, man->Then( x.val ) ); }
    AtBddNode Else( AtBddNode const & x ) override { return AtBddNode( man, man->Else( x.val ) ); }
    
    AtBddNode Regular( AtBddNode const & x ) override { return AtBddNode( man, man->LitRegular( x.val ) ); }
    bool IsCompl( AtBddNode const & x ) override { return man->LitIsCompl( x.val ); }
    
    int Level( int i ) override { return man->Var( man->LitIthVar( i ) ); }
    void Reorder() override { if ( man->get_pRefsExists() ) man->Reorder(); }
    void Dvr() override
    {
      if ( !man->get_pRefsExists() && man->get_nObjs() != 1 + man->get_nVars() )
	{
	  std::cerr << "dvr is not turned on because there are nodes already built" << std::endl;
	  return;
	}
      man->Dvr();
    }
    void DvrOff() override { man->DvrOff(); }
    
    AtBddNode Not( AtBddNode const & x ) override { return AtBddNode( man, man->LitNot( x.val ) ); }
    AtBddNode And( AtBddNode const & x, AtBddNode const & y ) override { return AtBddNode( man, man->And( x.val, y.val ) ); }

    void PrintStats( std::vector<AtBddNode> & vNodes ) override
    {
      uint64_t count = 0;
      std::vector<AtBdd::lit> v;
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
