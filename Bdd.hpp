#ifndef BDD
#define BDD

#include "SimpleBdd.hpp"

namespace Bdd {

  class BddMan {
    virtual uint64_t And( uint64_t x, uint64_t y ) = 0;
  };

  class SimpleBddMan : public BddMan {
    SimpleBdd::SimpleBdd * a;
    SimpleBddMan() {
      a = new SimpleBdd::SimpleBdd(10, 1, NULL, 1);
    };
    uint64_t And( uint64_t x, uint64_t y ) override {
      return 0;
    }
  };
}

#endif
