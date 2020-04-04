#ifndef BDD_MAN_HPP_
#define BDD_MAN_HPP_

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
    virtual node Regular( node const & x ) = 0;
    virtual bool IsCompl( node const & x ) = 0;
    virtual int  Var( node const & x ) = 0;
    virtual node Then( node const & x ) = 0;
    virtual node Else( node const & x ) = 0;
    virtual void Ref( node const & x ) = 0;
    virtual void Deref( node const & x ) = 0;
    virtual node NotCond( node const & x, bool c ) = 0;
    virtual node And( node const & x, node const & y ) = 0;
    virtual int  GetNumVar() = 0;
    virtual void PrintStats() = 0;

    virtual uint64_t Id( node const & x ) = 0;
  };
}

#endif
