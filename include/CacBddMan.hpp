#ifndef CACBDD_MAN_HPP_
#define CACBDD_MAN_HPP_

#include <iostream>
#include <fstream>
#include "BddMan.hpp"
#include <BDDNode.h>

namespace Bdd
{
  struct CacBddParam
  {
    // Param
    int slotSize = 1000000; // Log 1000 1000000000
    int uSize = 1 << 18; // Pow 10 30
    int cSize = 1 << 18; // Pow 10 30
    // end

    CacBddParam( std::string fname = "_CacBddMan.hpp_setting.txt" )
    {
      std::ifstream f( fname );
      if ( !f )
	return;
      std::string str;
      if ( std::getline( f, str ) )
	slotSize = std::stoi( str );
      if ( std::getline( f, str ) )
	uSize = std::stoi( str );
      if ( std::getline( f, str ) )
	cSize = std::stoi( str );
    }
  };
    
  class CacBddMan : public BddMan<cacBDD::BDD>
  {
  private:
    cacBDD::XBDDManager * man;
    
  public:
    CacBddMan( int nVars )
    {
      CacBddParam p;
      man = new cacBDD::XBDDManager( nVars, p.slotSize, p.uSize, p.cSize );
    };
    CacBddMan( int nVars, CacBddParam p )
    {
      man = new cacBDD::XBDDManager( nVars, p.slotSize, p.uSize, p.cSize );
    };
    ~CacBddMan() { delete man; }
    cacBDD::BDD Const0() override { return man->BddZero(); }
    cacBDD::BDD Const1() override { return man->BddOne(); }
    cacBDD::BDD IthVar( int i ) override { return man->BddVar( i+1 ); }
    cacBDD::BDD Regular( cacBDD::BDD const & x ) override { cacBDD::BDD y = x; return y.IsComp()? !x: x; }
    bool IsCompl( cacBDD::BDD const & x ) override { cacBDD::BDD y = x; return y.IsComp(); }
    cacBDD::BDD Not( cacBDD::BDD const & x ) override { return !x; }
    int Var( cacBDD::BDD const & x ) override { cacBDD::BDD y = x; return y.Variable()-1; }
    cacBDD::BDD Then( cacBDD::BDD const & x ) override { return x.Then(); }
    cacBDD::BDD Else( cacBDD::BDD const & x ) override { return x.Else(); }
    
    void Ref( cacBDD::BDD const & x ) override { (void)x; }
    void Deref( cacBDD::BDD const & x ) override { (void)x; }
    
    cacBDD::BDD And( cacBDD::BDD const & x, cacBDD::BDD const & y ) override { return x * y; }
    cacBDD::BDD Or( cacBDD::BDD const & x, cacBDD::BDD const & y ) override { return x + y; }
    cacBDD::BDD Xor( cacBDD::BDD const & x, cacBDD::BDD const & y ) override { return x ^ y; }

    void Reorder() override { throw "undefined"; }
    
    int GetNumVar() override { return man->manager()->GetVariableCount(); }
    void PrintStats( std::vector<cacBDD::BDD> & vNodes ) override
    {
      (void)vNodes;
      std::cout << "UTable = " << man->GetUTableCount() << std::endl;
      man->ShowInfo();
    }

    uint64_t Id( cacBDD::BDD const & x ) { return (uint64_t)x.Node(); }
  };
}

#endif
