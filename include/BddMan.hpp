#ifndef BDD_MAN_HPP_
#define BDD_MAN_HPP_

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
    virtual node Regular( node const & x ) = 0;
    virtual bool IsCompl( node const & x ) = 0;
    virtual int  Var( node const & x ) = 0;
    virtual node Then( node const & x ) = 0;
    virtual node Else( node const & x ) = 0;
    virtual node Not( node const & x ) = 0;
    
    virtual void Ref( node const & x ) { (void)x; }
    virtual void Deref( node const & x ) { (void)x; }
    virtual void RefNot( node const & x ) { (void)x; }
    virtual void DerefNot( node const & x ) { (void)x; }
    virtual void SupportRef() {}
    virtual void UnsupportRef() {}

    virtual int Perm( int i ) { return i; }
    virtual void Reorder() {};
        
    virtual node And( node const & x, node const & y );
    virtual node Or( node const & x, node const & y );
    virtual node Xor( node const & x, node const & y );
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
    node nx = Not( x );
    RefNot( nx );
    node ny = Not( y );
    RefNot( ny );
    node z = And( nx, ny );
    RefNot( z );
    DerefNot( nx );
    DerefNot( ny );
    node nz = Not( z );
    DerefNot( z );
    return nz;
  }
  template <typename node>
  node BddMan<node>::Xor( node const & x, node const & y )
  {
    node nx = Not( x );
    RefNot( x );
    node z0 = And( nx, y );
    Ref( z0 );
    DerefNot( x );
    node ny = Not( y );
    RefNot( ny );
    node z1 = And( x, ny );
    Ref( z1 );
    node z = Or( z0, z1 );
    Deref( z0 );
    Deref( z1 );
    return z;
  }
  template <typename node>
  node BddMan<node>::Ite( node const & c, node const & x, node const & y )
  {
    node z0 = And( c, x );
    Ref( z0 );
    node nc = Not( c );
    RefNot( nc );
    node z1 = And( nc, y );
    Ref( z1 );
    DerefNot( nc );
    node z = Or( z0, z1 );
    Deref( z0 );
    Deref( z1 );
    return z;
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
    // TODO : var may be different from level, perm
    while ( Var( x ) > Var( c ) )
      { 
	c = Then( c );
	if ( c == Const1() )
	  {
	    return x;
	  }
      }
    Ref( c );
    node z;
    if ( Var( x ) == Var( c ) )
      {
	node tc = Then( c );
	Ref( tc );
	node tx = Then( x );
	Ref( tx );
	node z0 = Exist( tc, tx );
	Ref( z0 );
	Deref( tx );
	node ex = Else( x );
	Ref( ex );
	node z1 = Exist( tx, ex );
	Ref( z1 );
	Deref( ex );
	Deref( tc );
	z = Or( z0, z1 );
	Deref( z0 );
	Deref( z1 );
      }
    else
      {
	node tx = Then( x );
	Ref( tx );
	node t = Exist( tx, c );
	Ref( t );
	Deref( tx );
	node ex = Else( x );
	Ref( ex );
	node e = Exist( ex, c );
	Ref( e );
	Deref( ex );
	z = Ite( IthVar( Var( x ) ), t, e );
	Deref( t );
	Deref( e );
      }
    Deref( c );
    return z;
  }
  template <typename node>
  node BddMan<node>::Univ( node const & x, node const & cube )
  {
    node nx = Not( x );
    RefNot( nx );
    node y = Exist( nx, cube );
    RefNot( y );
    DerefNot( nx );
    node ny = Not( y );
    DerefNot( y );
    return ny;
  }
  template <typename node>
  node BddMan<node>::AndExist( node const & x, node const & y, node const & cube )
  {
    node z = And( x, y );
    Ref( z );
    node ez = Exist( z, cube );
    Deref( z );
    return ez;
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
    // TODO : var may be different from level, perm
    if ( Var( x ) > i )
      {
	return x;
      }
    if ( Var( x ) == i )
      {
	node tx = Then( x );
	Ref( tx );
	node ex = Else( x );
	Ref( ex );
	node y = Ite( c, tx, ex );
	Deref( tx );
	Deref( ex );
	return y;
      }
    // TODO : var may be different from level, perm
    node y;
    if ( Var( x ) < Var( c ) )
      {
	node tx = Then( x );
	Ref( tx );
	node t = Compose( tx, i, c );
	Ref( t );
	Deref( tx );
	node ex = Else( x );
	Ref( ex );
	node e = Compose( ex, i, c );
	Ref( e );
	Deref( ex );
	y = Ite( IthVar( Var( x ) ), t, e );
	Deref( t );
	Deref( e );
      }
    else if ( Var( x ) > Var( c ) )
      {
	node tc = Then( c );
	Ref( tc );
	node t = Compose( x, i, tc );
	Ref( t );
	Deref( tc );
	node ec = Else( c );
	Ref( ec );
	node e = Compose( x, i, ec );
	Ref( e );
	Deref( ec );
	y = Ite( IthVar( Var( c ) ), t, e );
	Deref( t );
	Deref( e );
      }
    else // if ( Var( x ) == Var( c ) )
      {
	node tx = Then( x );
	Ref( tx );
	node tc = Then( c );
	Ref( tc );
	node t = Compose( tx, i, tc );
	Ref( t );
	Deref( tx );
	Deref( tc );
	node ex = Else( x );
	Ref( ex );
	node ec = Else( c );
	Ref( ec );
	node e = Compose( ex, i, ec );
	Ref( e );
	Deref( ex );
	Deref( ec );
	y = Ite( IthVar( Var( x ) ), t, e );
	Deref( t );
	Deref( e );
      }
    return y;
  }
  template <typename node>
  node BddMan<node>::VecCompose( node const & x, std::vector<node> & cs )
  {
    if ( x == Const0() || x == Const1() )
      {
	return x;
      }
    node tx = Then( x );
    Ref( tx );
    node t = VecCompose( tx, cs );
    Ref( t );
    Deref( tx );
    node ex = Else( x );
    Ref( ex );
    node e = VecCompose( ex, cs );
    Ref( e );
    Deref( ex );
    node y = Ite( cs[Var( x )], t, e );
    Deref( t );
    Deref( e );
    return y;
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
