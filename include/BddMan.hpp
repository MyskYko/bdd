#ifndef BDD_MAN_HPP_
#define BDD_MAN_HPP_

#include <cstdint>
#include <vector>

namespace Bdd
{
  template <typename node>
  class BddMan
  {
  public:
    std::vector<node> vNodes;
    virtual node Const0() = 0;
    virtual node Const1() = 0;
    virtual node IthVar( int i ) = 0;
    virtual node Regular( node x ) = 0;
    virtual bool IsCompl( node x ) = 0;
    virtual int  Var( node x ) = 0;
    virtual node Then( node x ) = 0;
    virtual node Else( node x ) = 0;
    virtual void Ref( node x ) = 0;
    virtual void Deref( node x ) = 0;
    virtual node NotCond( node x, bool c ) = 0;
    virtual node And( node x, node y ) = 0;
    virtual int  GetNumVar() = 0;
    virtual void PrintStats() = 0;
  };
}

#endif
