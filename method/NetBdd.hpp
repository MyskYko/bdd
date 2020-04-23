#ifndef NET_BDD_HPP_
#define NET_BDD_HPP_

#include <optional>
#include "BddMan.hpp"
#include "mockturtle/mockturtle.hpp"

template <typename Ntk, typename node>
std::vector<node> Net2Bdd( Ntk & net_, Bdd::BddMan<node> & bdd )
{
  mockturtle::topo_view net{net_};
  int * pFanouts = (int *)calloc( net.size(), sizeof(int) );
  if ( !pFanouts )
    {
      throw "Allocation failed";
    }
  net.foreach_gate( [&]( auto gate )
    {
      pFanouts[net.node_to_index( gate )] = net.fanout_size( gate );
    });
  std::map<uint32_t, std::optional<node> > m;
  m[net.node_to_index( net.get_node( net.get_constant( 0 ) ) )] = bdd.Const0();
  net.foreach_pi( [&]( auto pi, int i )
    {
      m[net.node_to_index( pi )] = bdd.IthVar( i );
    });
  net.foreach_gate( [&]( auto gate )
    {
      node x = bdd.Const1();
      net.foreach_fanin( gate, [&]( auto fanin )
        {
	  node y = *m[net.node_to_index( net.get_node( fanin ) )];
	  if ( net.is_complemented( fanin ) )
	    {
	      y = bdd.Not( y );
	    }
	  // TODO : use node_function
	  x = bdd.And( x, y );
	});
      m[net.node_to_index( gate )] = x;
      net.foreach_fanin( gate, [&]( auto fanin )
        {
	  auto index = net.node_to_index( net.get_node( fanin ) );
	  pFanouts[index] -= 1;
	  if ( !pFanouts[index] )
	    {
	      m[index] = std::nullopt;
	    }
	});
    });
  std::vector<node> vNodes;
  net.foreach_po( [&]( auto po )
    {
      auto index = net.node_to_index( net.get_node( po ) );
      node x = *m[index];
      if ( net.is_complemented( po ) )
	{
	  x = bdd.Not( x );
	}
      vNodes.push_back( x );
      pFanouts[index] -= 1;
      if ( !pFanouts[index] )
	{
	  m[index] = std::nullopt;
	}
    });
  free( pFanouts );
  return vNodes;
}

template <typename node, typename Ntk>
auto Bdd2Net_rec( Ntk & net, Bdd::BddMan<node> & bdd, node & x, std::map<uint64_t, typename Ntk::signal> & m )
{
  if ( x == bdd.Const0() )
    {
      return net.get_constant( 0 );
    }
  if ( x == bdd.Const1() )
    {
      return net.get_constant( 1 );
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
	  f = net.create_not( f );
	  m[bdd.Id( x )] = f;
	  return f;
	}
      return m[bdd.Id( bdd.Regular( x ) )];
    }
  int v = bdd.Var( x );
  node x1 = bdd.Then( x );
  node x0 = bdd.Else( x );
  auto c = net.make_signal( net.pi_at( v ) );
  auto f1 = Bdd2Net_rec( net, bdd, x1, m );
  auto f0 = Bdd2Net_rec( net, bdd, x0, m );
  auto f = net.create_ite( c, f1, f0 );
  if ( bdd.IsCompl( x ) )
    {
      m[bdd.Id( x )] = f;
      m[bdd.Id( bdd.Regular( x ) )] = net.create_not( f );
      return f;
    }
  m[bdd.Id( x )] = f;
  return f;
}

template <typename node, typename Ntk>
void Bdd2Net( Ntk & net, Bdd::BddMan<node> & bdd, std::vector<node> & vNodes )
{
  for ( int i = 0; i < bdd.GetNumVar(); i++ )
    {
      net.create_pi();
    }
  std::map<uint64_t, typename Ntk::signal> m;
  for ( node & x : vNodes )
    {
      auto f = Bdd2Net_rec( net, bdd, x, m );
      net.create_po( f );
    }
}

#endif
