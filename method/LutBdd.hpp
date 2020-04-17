#ifndef LUT_BDD_HPP_
#define LUT_BDD_HPP_

#include <BddMan.hpp>
#include <mockturtle/mockturtle.hpp>

template <typename node>
auto Bdd2Lut_rec( mockturtle::klut_network & lut, Bdd::BddMan<node> & bdd, node & x, std::map<uint64_t, mockturtle::klut_network::signal> & m )
{
  if ( x == bdd.Const0() )
    {
      return lut.get_constant( 0 );
    }
  if ( x == bdd.Const1() )
    {
      return lut.get_constant( 1 );
    }
  if ( m.count( bdd.Id( bdd.Regular( x ) ) ) )
    {
      if ( bdd.IsCompl( x ) )
	{
	  if ( m.count( bdd.Id( x ) ) )
	    {
	      return m[bdd.Id( x )];
	    }
	  auto f = m[bdd.Id( bdd.Regular( x ) )];
	  f = lut.create_not( f );
	  m[bdd.Id( x )] = f;
	  return f;
	}
      return m[bdd.Id( bdd.Regular( x ) )];
    }
  int v = bdd.Var( x );
  node x1 = bdd.Then( x );
  node x0 = bdd.Else( x );
  auto c = lut.make_signal( lut.pi_at( v ) );
  auto f1 = Bdd2Lut_rec( lut, bdd, x1, m );
  auto f0 = Bdd2Lut_rec( lut, bdd, x0, m );
  auto f = lut.create_ite( c, f1, f0 );
  if ( bdd.IsCompl( x ) )
    {
      m[bdd.Id( x )] = f;
      m[bdd.Id( bdd.Regular( x ) )] = lut.create_not( f );
      return f;
    }
  m[bdd.Id( x )] = f;
  return f;
}

template <typename node>
void Bdd2Lut( mockturtle::klut_network & lut, Bdd::BddMan<node> & bdd, std::vector<node> & vNodes )
{
  for ( int i = 0; i < bdd.GetNumVar(); i++ )
    {
      lut.create_pi();
    }
  std::map<uint64_t, mockturtle::klut_network::signal> m;
  for ( node & x : vNodes )
    {
      auto f = Bdd2Lut_rec( lut, bdd, x, m );
      lut.create_po( f );
    }
}

#endif
