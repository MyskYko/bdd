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
    // end

    CacBddParam( std::string fname = "_CacBddMan.hpp_setting.txt" )
    {
      std::ifstream f( fname );
      if ( !f )
	return;
    }
  };
    
  class CacBddMan : public BddMan<cacBDD::BDD>
  {
  private:
    cacBDD::XBDDManager * man;
    
  public:
    CacBddMan( int nVars ) {
      CacBddParam p;
      (void)p;
      man = new cacBDD::XBDDManager( nVars );
    };
    CacBddMan( int nVars, CacBddParam p )
    {
      (void)p;
      man = new cacBDD::XBDDManager( nVars );
    };
    ~CacBddMan()
    {
      vNodes.clear();
      delete man;
    }
    cacBDD::BDD  Const0() override { return man->BddZero(); }
    cacBDD::BDD  Const1() override { return man->BddOne(); }
    cacBDD::BDD  IthVar( int i ) override { return man->BddVar( i+1 ); }
    cacBDD::BDD  Regular( cacBDD::BDD const & x ) override { cacBDD::BDD y = x; return y.IsComp()? !x: x; }
    bool IsCompl( cacBDD::BDD const & x ) override { cacBDD::BDD y = x; return y.IsComp(); }
    int  Var( cacBDD::BDD const & x ) override { cacBDD::BDD y = x; return y.Variable()-1; }
    cacBDD::BDD  Then( cacBDD::BDD const & x ) override { return x.Then(); }
    cacBDD::BDD  Else( cacBDD::BDD const & x ) override { return x.Else(); }
    void Ref( cacBDD::BDD const & x ) override { (void)x; }
    void Deref( cacBDD::BDD const & x ) override { (void)x; }
    cacBDD::BDD  NotCond( cacBDD::BDD const & x, bool c ) override { return c? !x: x; }
    cacBDD::BDD  And( cacBDD::BDD const & x, cacBDD::BDD const & y ) override { return x * y; }
    int  GetNumVar() override { return man->manager()->GetVariableCount(); }
    void PrintStats() override
    {
      std::cout << "Shared BDD nodes = " << man->NodeCount() << std::endl;
      std::cout << "Sum of BDD nodes = #####" << std::endl;
      man->ShowInfo();
    }

    uint64_t Id( cacBDD::BDD const & x ) { return (uint64_t)x.Node(); }
  };
}

#endif
