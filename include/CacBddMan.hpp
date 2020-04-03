#ifndef CACBDD_MAN_HPP_
#define CACBDD_MAN_HPP_

#include <iostream>
#include <fstream>
#include "BddMan.hpp"
#include <BDDNode.h>

using namespace cacBDD;

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
    
  class CacBddMan : public BddMan<BDD>
  {
  private:
    XBDDManager * man;
    
  public:
    CacBddMan( int nVars ) {
      CacBddParam p;
      (void)p;
      man = new XBDDManager( nVars );
    };
    CacBddMan( int nVars, CacBddParam p )
    {
      (void)p;
      man = new XBDDManager( nVars );
    };
    ~CacBddMan()
    {
      vNodes.clear();
      delete man;
    }
    BDD  Const0() override { return man->BddZero(); }
    BDD  Const1() override { return man->BddOne(); }
    BDD  IthVar( int i ) override { return man->BddVar( i+1 ); }
    BDD  Regular( BDD const & x ) override { BDD y = x; return y.IsComp()? !x: x; }
    bool IsCompl( BDD const & x ) override { BDD y = x; return y.IsComp(); }
    int  Var( BDD const & x ) override { BDD y = x; return y.Variable()-1; }
    BDD  Then( BDD const & x ) override { return x.Then(); }
    BDD  Else( BDD const & x ) override { return x.Else(); }
    void Ref( BDD const & x ) override { (void)x; }
    void Deref( BDD const & x ) override { (void)x; }
    BDD  NotCond( BDD const & x, bool c ) override { return c? !x: x; }
    BDD  And( BDD const & x, BDD const & y ) override { return x * y; }
    int  GetNumVar() override { return man->manager()->GetVariableCount(); }
    void PrintStats() override
    {
      std::cout << "Shared BDD nodes = " << man->NodeCount() << std::endl;
      std::cout << "Sum of BDD nodes = #####" << std::endl;
      man->ShowInfo();
    }

    uint64_t Id( BDD const & x ) { return (uint64_t)x.Node(); }
  };
}

#endif
