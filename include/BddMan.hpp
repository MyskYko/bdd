#ifndef BDD_MAN_HPP_
#define BDD_MAN_HPP_

#include <cstdint>
#include <vector>

namespace Bdd
{
  class BddMan
  {
  public:
    std::vector<uint64_t> vNodes;
    virtual uint64_t Const0() = 0;
    virtual uint64_t Const1() = 0;
    virtual uint64_t IthVar( int i ) = 0;
    virtual uint64_t Regular( uint64_t x ) = 0;
    virtual int      IsCompl( uint64_t x ) = 0;
    virtual int      Var( uint64_t x ) = 0;
    virtual uint64_t Then( uint64_t x ) = 0;
    virtual uint64_t Else( uint64_t x ) = 0;
    virtual void     Ref( uint64_t x ) = 0;
    virtual void     Deref( uint64_t x ) = 0;
    virtual uint64_t NotCond( uint64_t x, int c ) = 0;
    virtual uint64_t And( uint64_t x, uint64_t y ) = 0;
    virtual int      GetNumVar() = 0;
    virtual void     PrintStats() = 0;
  };
}

#endif
