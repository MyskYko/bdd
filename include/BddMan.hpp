#ifndef BDD_MAN_HPP_
#define BDD_MAN_HPP_

#include <vector>

namespace Bdd
{
  template <typename node>
  class BddMan
  {
  public:
    virtual node Const0() = 0;
    virtual node Const1() = 0;
    virtual node IthVar( int i ) = 0;
    virtual node Regular( node const & x ) = 0;
    virtual bool IsCompl( node const & x ) = 0;
    virtual node Not( node const & x ) = 0;
    virtual int  Var( node const & x ) = 0;
    virtual node Then( node const & x ) = 0;
    virtual node Else( node const & x ) = 0;
    
    virtual void Ref( node const & x ) = 0;
    virtual void Deref( node const & x ) = 0;
    
    virtual node And( node const & x, node const & y ) = 0;
    virtual node Or( node const & x, node const & y ) = 0;
    virtual node Xor( node const & x, node const & y ) = 0;

    virtual void Reorder() = 0;
    
    virtual int  GetNumVar() = 0;
    virtual void PrintStats( std::vector<node> & vNodes ) = 0;
    
    virtual uint64_t Id( node const & x ) = 0;
    
    virtual void RefNot( node const & x ) { (void)x; }
    virtual void DerefNot( node const & x ) { (void)x; }

    virtual void SupportRef() {}
  };
}

#endif
