#ifndef BDD
#define BDD

#include "SimpleBdd.hpp"
#include <cudd.h>

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
    virtual void     PrintStats( std::vector<uint64_t> & vNodes_ ) = 0;
  };

  template<typename var = uint8_t>
  class SimpleBddMan : public BddMan {
  private:
    SimpleBdd::BddMan<var> * man;
    
  public:
    SimpleBddMan( int nVars ) {
      assert( nVars < (int)std::numeric_limits<var>::max() );
      man = new SimpleBdd::BddMan<var>( nVars, 1, NULL, 0 );
      man->RefreshConfig( 1, 1, 0 );
    };
    ~SimpleBddMan() { delete man; }
    uint64_t Const0() override { return man->LitConst0(); }
    uint64_t Const1() override { return man->LitConst1(); }
    uint64_t IthVar( int i ) override { return man->LitIthVar( i ); }
    uint64_t Regular( uint64_t x ) override { return man->LitRegular( x ); }
    int IsCompl( uint64_t x ) override { return man->LitIsCompl( x ); }
    int Var( uint64_t x ) override { return man->get_order( man->Var( x ) ); }
    uint64_t Then( uint64_t x ) override { return man->Then( x ); }
    uint64_t Else( uint64_t x ) override { return man->Else( x ); }
    void Ref( uint64_t x ) override { man->Ref( x ); }
    void Deref( uint64_t x ) override { man->Deref( x ); }
    uint64_t NotCond( uint64_t x, int c ) override { return man->LitNotCond( x, c ); }
    uint64_t And( uint64_t x, uint64_t y ) override { return man->And( x, y ); }
    int GetNumVar() override { return man->get_nVars(); }
    void PrintStats( std::vector<uint64_t> & vNodes_ ) override
    {
      std::vector<uint32_t> vNodes( vNodes_.size() );
      for ( int i = 0; i < vNodes_.size(); i++ )
	vNodes.push_back( vNodes_[i] );
      std::cout << "Shared BDD nodes = " << man->CountNodesArrayShared( vNodes ) << std::endl;
      std::cout << "Sum of BDD nodes = " << man->CountNodesArrayIndependent( vNodes ) << std::endl;
    }
  };
  
  class CuddMan : public BddMan {
  private:
    DdManager * man;
    
  public:
    CuddMan( int nVars ) {
      man = Cudd_Init( nVars, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0 );
    }
    ~CuddMan() { Cudd_Quit( man ); }
    uint64_t Const0() override { return (uint64_t)Cudd_Not( Cudd_ReadOne( man ) ); }
    uint64_t Const1() override { return (uint64_t)Cudd_ReadOne( man ); }
    uint64_t IthVar( int i ) override { return (uint64_t)Cudd_ReadVars( man, i ); }
    uint64_t Regular( uint64_t x ) override { return (uint64_t)Cudd_Regular( (DdNode *)x ); }
    int IsCompl( uint64_t x ) override { return Cudd_IsComplement( (DdNode *)x ); }
    int Var( uint64_t x ) override { return Cudd_NodeReadIndex( (DdNode *)x ); }
    uint64_t Then( uint64_t x ) override { return (uint64_t)Cudd_NotCond( Cudd_T( (DdNode *)x ), IsCompl( x ) ); }
    uint64_t Else( uint64_t x ) override { return (uint64_t)Cudd_NotCond( Cudd_E( (DdNode *)x ), IsCompl( x ) ); }
    void Ref( uint64_t x ) override { Cudd_Ref( (DdNode *)x ); }
    void Deref( uint64_t x ) override { Cudd_RecursiveDeref( man, (DdNode *)x ); }
    uint64_t NotCond( uint64_t x, int c ) override { return (uint64_t)Cudd_NotCond( (DdNode *)x, c ); }
    uint64_t And( uint64_t x, uint64_t y ) override { return (uint64_t)Cudd_bddAnd( man, (DdNode *)x, (DdNode *)y ); }
    int GetNumVar() override { return Cudd_ReadSize( man ); }
    void PrintStats( std::vector<uint64_t> & vNodes_ ) override
    {
      DdNode ** nodeArray = (DdNode **)malloc( sizeof(DdNode*) * vNodes_.size() );
      uint64_t count = 0;
      for ( int i = 0; i < vNodes_.size(); i++ )
	{
	  nodeArray[i] = (DdNode *)vNodes_[i];
	  count += Cudd_DagSize( nodeArray[i] );
	}
      std::cout << "Shared BDD nodes = " << Cudd_SharingSize( nodeArray, vNodes_.size() ) << std::endl;
      std::cout << "Sum of BDD nodes = " << count << std::endl;
      free( nodeArray );
    }
  };

}

#endif
