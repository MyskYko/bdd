#ifndef BDD
#define BDD

#include "SimpleBdd.hpp"

namespace Bdd {

  class BddMan {
  public:
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
    virtual int      GetOldVar( int v ) = 0;
    virtual void     PrintStats( std::vector<uint64_t> & vNodes_ ) = 0;
  };

  template<typename var = uint8_t>
  class SimpleBddMan : public BddMan {
  private:
    SimpleBdd::BddMan<var> * man;
    
  public:
    SimpleBddMan( int nVars ) {
      assert( nVars < (int)std::numeric_limits<var>::max() );
      man = new SimpleBdd::BddMan<var>( nVars, 1, NULL, 1 );
      man->RefreshConfig( 1, 1, 10 );
    };
    ~SimpleBddMan() { delete man; }
    uint64_t Const0() override { return man->LitConst0(); }
    uint64_t Const1() override { return man->LitConst1(); }
    uint64_t IthVar( int i ) override { return man->LitIthVar( i ); }
    uint64_t Regular( uint64_t x ) override { return man->LitRegular( x ); }
    int IsCompl( uint64_t x ) override { return man->LitIsCompl( x ); }
    int Var( uint64_t x ) override { return man->Var( x ); }
    uint64_t Then( uint64_t x ) override { return man->Then( x ); }
    uint64_t Else( uint64_t x ) override { return man->Else( x ); }
    void Ref( uint64_t x ) override { man->Ref( x ); }
    void Deref( uint64_t x ) override { man->Deref( x ); }
    uint64_t NotCond( uint64_t x, int c ) override { return man->LitNotCond( x, c ); }
    uint64_t And( uint64_t x, uint64_t y ) override { return man->And( x, y ); }
    int GetNumVar() override { return man->get_nVars(); }
    int GetOldVar( int v ) override { return man->get_order( v ); }
    void PrintStats( std::vector<uint64_t> & vNodes_ ) override
    {
      std::vector<uint32_t> vNodes(vNodes_.size());
      for ( int i = 0; i < vNodes_.size(); i++ )
	vNodes.push_back( vNodes_[i] );
      std::cout << "Shared BDD nodes = " << man->CountNodesArrayShared( vNodes ) << std::endl;
      std::cout << "Sum of BDD nodes = " << man->CountNodesArrayIndependent( vNodes ) << std::endl;
    }
  };
}

#endif
