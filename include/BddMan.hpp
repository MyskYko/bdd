#ifndef BDD_MAN_HPP_
#define BDD_MAN_HPP_

#include <iostream>
#include <vector>
#include <map>

namespace Bdd
{
  template <typename node>
  class BddMan
  {
  public:
    virtual int  GetNumVar() = 0;
    virtual uint64_t Id( node const & x ) = 0;
    
    virtual node Const0() = 0;
    virtual node Const1() = 0;
    virtual node IthVar( int i ) = 0;
    
    virtual int  Var( node const & x ) = 0;
    virtual node Then( node const & x ) = 0;
    virtual node Else( node const & x ) = 0;
    
    virtual node Regular( node const & x ) { return x; }
    virtual bool IsCompl( node const & x ) { (void)x; return 0; }
    
    virtual int Level( int i ) { return i; }
    virtual void Reorder() { std::cerr << "Reorder is not implemented" << std::endl; }
    virtual void Dvr() { std::cerr << "Dvr is not implemented" << std::endl; }
    virtual void DvrOff() {}
        
    virtual node Not( node const & x ) = 0;
    virtual node And( node const & x, node const & y );
    virtual node Or( node const & x, node const & y );
    virtual node Xor( node const & x, node const & y );
    virtual node Nand( node const & x, node const & y );
    virtual node Nor( node const & x, node const & y );
    virtual node Xnor( node const & x, node const & y );
    virtual node Ite( node const & c, node const & x, node const & y );
    virtual node Exist( node const & x, node const & cube );
    virtual node Univ( node const & x, node const & cube );
    virtual node AndExist( node const & x, node const & y, node const & cube );
    virtual node Restrict( node const & x, node const & c );
    virtual node Compose( node const & x, int i, node const & c );
    virtual node VecCompose( node const & x, std::vector<node> & cs );

    virtual void Support( node const & x, std::vector<int> & vVars );
    
    virtual void PrintStats( std::vector<node> & vNodes ) { (void)vNodes; }
  };
  
  template <typename node>
  node BddMan<node>::And( node const & x, node const & y )
  {
    return Ite( x, y, Const0() );
  }
  template <typename node>
  node BddMan<node>::Or( node const & x, node const & y )
  {
    return Not( And( Not( x ), Not( y ) ) );
  }
  template <typename node>
  node BddMan<node>::Xor( node const & x, node const & y )
  {
    node z0 = And( Not( x ), y );
    node z1 = And( x, Not( y ) );
    return Or( z0, z1 );
  }
  template <typename node>
  node BddMan<node>::Nand( node const & x, node const & y )
  {
    return Not( And( x, y ) );
  }
  template <typename node>
  node BddMan<node>::Nor( node const & x, node const & y )
  {
    return Not( Or( x, y ) );
  }
  template <typename node>
  node BddMan<node>::Xnor( node const & x, node const & y )
  {
    return Not( Xor( x, y ) );
  }
  template <typename node>
  node BddMan<node>::Ite( node const & c, node const & x, node const & y )
  {
    node z0 = And( c, x );
    node z1 = And( Not( c ), y );
    return Or( z0, z1 );
  }
  template <typename node>
  node BddMan<node>::Exist( node const & x, node const & cube )
  {
    // TODO : reo must be disabled
    if ( x == Const0() || x == Const1() || cube == Const1() )
      {
	return x;
      }
    node c = cube;
    while ( Level( Var( x ) ) > Level( Var( c ) ) )
      { 
	c = Then( c );
	if ( c == Const1() )
	  {
	    return x;
	  }
      }
    if ( Var( x ) == Var( c ) )
      {
	node z0 = Exist( Then( x ), Then( c ) );
	node z1 = Exist( Else( x ), Then( c ) );
	return Or( z0, z1 );
      }
    node t = Exist( Then( x ), c );
    node e = Exist( Else( x ), c );
    return Ite( IthVar( Var( x ) ), t, e );
  }
  template <typename node>
  node BddMan<node>::Univ( node const & x, node const & cube )
  {
    return Not( Exist( Not( x ), cube ) );
  }
  template <typename node>
  node BddMan<node>::AndExist( node const & x, node const & y, node const & cube )
  {
    node z = And( x, y );
    return Exist( z, cube );
  }
  template <typename node>
  node BddMan<node>::Restrict( node const & x, node const & c )
  {
    (void)c;
    return x;
  }
  template <typename node>
  node BddMan<node>::Compose( node const & x, int i, node const & c )
  {
    // TODO : reo must be disabled
    if ( Level( Var( x ) ) > Level( i ) )
      {
	return x;
      }
    if ( Var( x ) == i )
      {
	return Ite( c, Then( x ), Else( x ) );
      }
    node v, t, e;
    if ( Level( Var( x ) ) < Level( Var( c ) ) )
      {
	v = IthVar( Var( x ) );
	t = Compose( Then( x ), i, c );
	e = Compose( Else( x ), i, c );
      }
    else if ( Level( Var( x ) ) > Level( Var( c ) ) )
      {
	v = IthVar( Var( c ) );
	t = Compose( x, i, Then( c ) );
	e = Compose( x, i, Else( c ) );
      }
    else // if ( Var( x ) == Var( c ) )
      {
	v = IthVar( Var( x ) );
	t = Compose( Then( x ), i, Then( c ) );
	e = Compose( Else( x ), i, Else( c ) );
      }
    return Ite( v, t, e );
  }
  template <typename node>
  node BddMan<node>::VecCompose( node const & x, std::vector<node> & cs )
  {
    if ( x == Const0() || x == Const1() )
      {
	return x;
      }
    node t = VecCompose( Then( x ), cs );
    node e = VecCompose( Else( x ), cs );
    return Ite( cs[Var( x )], t, e );
  }
  
  template <typename node>
  void BddMan<node>::Support( node const & x, std::vector<int> & vVars )
  {
    std::vector<bool> fVars( GetNumVar() );
    std::map<uint64_t, bool> fNodes;
    std::vector<node> vNodes;
    vNodes.push_back( x );
    while( !vNodes.empty() )
      {
	node y = Regular( vNodes.back() );
	vNodes.pop_back();
	if ( y == Const0() || y == Const1() || fNodes[Id( y )] )
	  {
	    continue;
	  }
	fNodes[Id( y )] = 1;
	fVars[Var( y )] = 1;
	vNodes.push_back( Then( y ) );
	vNodes.push_back( Else( y ) );
      }
    for ( int i = 0; i < GetNumVar(); i++ )
      {
	if ( fVars[i] )
	  {
	    vVars.push_back( i );
	  }
      }
  }
}

#endif
