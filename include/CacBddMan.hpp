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
	{
	  return;
	}
      std::string str;
      std::getline( f, str );
      slotSize = std::stoi( str );
      std::getline( f, str );
      uSize = std::stoi( str );
      std::getline( f, str );
      cSize = std::stoi( str );
    }
  };
    
  class CacBddMan : public BddMan<cacBDD::BDD>
  {
  private:
    cacBDD::XBDDManager * man;
    CacBddParam param;
    
  public:
    CacBddMan( int nVars, CacBddParam param ) : param( param )
    {
      man = new cacBDD::XBDDManager( nVars, param.slotSize, param.uSize, param.cSize );
    };
    CacBddMan( int nVars ) : CacBddMan( nVars, CacBddParam() ) {}
    ~CacBddMan() { delete man; }
    
    int GetNumVar() override { return man->manager()->GetVariableCount(); }
    uint64_t Id( cacBDD::BDD const & x ) { return (uint64_t)x.Node(); }
    
    cacBDD::BDD Const0() override { return man->BddZero(); }
    cacBDD::BDD Const1() override { return man->BddOne(); }
    cacBDD::BDD IthVar( int i ) override { return man->BddVar( i + 1 ); }

    int Var( cacBDD::BDD const & x ) override { cacBDD::BDD x_ = x; return x_.Variable() - 1; }
    cacBDD::BDD Then( cacBDD::BDD const & x ) override { return x.Then(); }
    cacBDD::BDD Else( cacBDD::BDD const & x ) override { return x.Else(); }
    
    cacBDD::BDD Regular( cacBDD::BDD const & x ) override { cacBDD::BDD y = x; return y.IsComp()? !x: x; }
    bool IsCompl( cacBDD::BDD const & x ) override { cacBDD::BDD x_ = x; return x_.IsComp(); }
    
    cacBDD::BDD Not( cacBDD::BDD const & x ) override { return !x; }
    cacBDD::BDD And( cacBDD::BDD const & x, cacBDD::BDD const & y ) override { return x * y; }
    cacBDD::BDD Or( cacBDD::BDD const & x, cacBDD::BDD const & y ) override { return x + y; }
    cacBDD::BDD Xor( cacBDD::BDD const & x, cacBDD::BDD const & y ) override { return x ^ y; }
    cacBDD::BDD Nand( cacBDD::BDD const & x, cacBDD::BDD const & y ) override { return x | y; }
    cacBDD::BDD Nor( cacBDD::BDD const & x, cacBDD::BDD const & y ) override { return x % y; }
    cacBDD::BDD Xnor( cacBDD::BDD const & x, cacBDD::BDD const & y ) override { return x & y; }
    cacBDD::BDD Ite( cacBDD::BDD const & c, cacBDD::BDD const & x, cacBDD::BDD const & y ) override { return man->Ite( c, x, y ); }
    cacBDD::BDD Exist( cacBDD::BDD const & x, cacBDD::BDD const & cube ) override { cacBDD::BDD x_ = x; return x_.Exist( cube ); }
    cacBDD::BDD Univ( cacBDD::BDD const & x, cacBDD::BDD const & cube ) override { cacBDD::BDD x_ = x; return x_.Universal( cube ); }
    cacBDD::BDD AndExist( cacBDD::BDD const & x, cacBDD::BDD const & y, cacBDD::BDD const & cube ) override { cacBDD::BDD x_ = x; return x_.AndExist( y, cube ); }
    cacBDD::BDD Restrict( cacBDD::BDD const & x, cacBDD::BDD const & c ) override { return x.Restrict( c ); }
    cacBDD::BDD Compose( cacBDD::BDD const & x, int i, cacBDD::BDD const & c ) override { return x.Compose( i, c ); }
    // cacBDD::BDD VecCompose( cacBDD::BDD const & x, std::vector<cacBDD::BDD> & cs ) override {}
    
    void Support( cacBDD::BDD const & x, std::vector<int> & vVars ) override { cacBDD::BDD x_ = x; x_.Support( vVars ); }
    
    void PrintStats( std::vector<cacBDD::BDD> & vNodes ) override
    {
      (void)vNodes;
      std::cout << "UTable = " << man->GetUTableCount() << std::endl;
      man->ShowInfo();
    }
  };
}

#endif
