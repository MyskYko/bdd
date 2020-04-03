#ifndef CACBDD_MAN_HPP_
#define CACBDD_MAN_HPP_

#include <iostream>
#include "BddMan.hpp"
#include <BDDNode.h>


using namespace cacBDD;

namespace Bdd
{
  struct CacBddParam
  {
    // Param
    int nVars = 0; // None 0
    // end

    CacBddParam( std::string fname = "_CacBddMan.hpp_setting.txt" )
    {
      std::ifstream f( fname );
      if ( !f )
	return;
      std::string str;
      if ( std::getline( f, str ) )
	nVars = std::stoi( str );
    }
  };
    
  class CacBddMan : public BddMan<BDD>
  {
  private:
    XBDDManager * man;
    
  public:
    CacBddMan( CacBddParam p ) { man = new XBDDManager( p.nVars ); };
    ~CacBddMan()
    {
      vNodes.clear();
      delete man;
    }
    BDD  Const0() override { return man->BddZero(); }
    BDD  Const1() override { return man->BddOne(); }
    BDD  IthVar( int i ) override { return man->BddVar( i+1 ); }
    BDD  Regular( BDD x ) override { return x.IsComp()? !x: x; }
    bool IsCompl( BDD x ) override { return x.IsComp(); }
    int  Var( BDD x ) override { return x.Variable(); }
    BDD  Then( BDD x ) override { return x.Then(); }
    BDD  Else( BDD x ) override { return x.Else(); }
    void Ref( BDD x ) override { (void)x; }
    void Deref( BDD x ) override { (void)x; }
    BDD  NotCond( BDD x, bool c ) override { return c? !x: x; }
    BDD  And( BDD x, BDD y ) override { return x * y; }
    int  GetNumVar() override { return man->manager()->GetVariableCount(); }
    void PrintStats() override
    {
      std::cout << "Shared BDD nodes = " << man->NodeCount() << std::endl;
      std::cout << "Sum of BDD nodes = #####" << std::endl;
      man->ShowInfo();
    }
  };
}

#endif
