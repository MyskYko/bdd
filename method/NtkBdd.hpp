#ifndef NTK_BDD_HPP_
#define NTK_BDD_HPP_

#include <optional>
#include "BddMan.hpp"
#include "mockturtle/mockturtle.hpp"

template <typename node>
std::vector<node> Aig2Bdd( mockturtle::aig_network & aig_, Bdd::BddMan<node> & bdd, bool fVerbose = 0 )
{
  mockturtle::topo_view aig{aig_};
  int * pFanouts = (int *)calloc( aig.size(), sizeof(int) );
  if ( !pFanouts )
    {
      throw "Allocation failed";
    }
  aig.foreach_gate( [&]( auto gate )
    {
      pFanouts[aig.node_to_index( gate )] = aig.fanout_size( gate );
    });
  std::map<uint32_t, std::optional<node> > m;
  m[aig.node_to_index( aig.get_node( aig.get_constant( 0 ) ) )] = bdd.Const0();
  aig.foreach_pi( [&]( auto pi, int i )
    {
      m[aig.node_to_index( pi )] = bdd.IthVar( i );
    });
  aig.foreach_gate( [&]( auto gate, int i )
    {
      if ( fVerbose )
	{
	  std::cout << "gate " << i + 1 << " / " << aig.num_gates() << std::endl;
	}
      node x = bdd.Const1();
      aig.foreach_fanin( gate, [&]( auto fanin )
        {
	  node y = *m[aig.node_to_index( aig.get_node( fanin ) )];
	  if ( aig.is_complemented( fanin ) )
	    {
	      y = bdd.Not( y );
	    }
	  x = bdd.And( x, y );
	});
      m[aig.node_to_index( gate )] = x;
      aig.foreach_fanin( gate, [&]( auto fanin )
        {
	  auto index = aig.node_to_index( aig.get_node( fanin ) );
	  pFanouts[index] -= 1;
	  if ( !pFanouts[index] )
	    {
	      m[index] = std::nullopt;
	    }
	});
    });
  std::vector<node> vNodes;
  aig.foreach_po( [&]( auto po )
    {
      auto index = aig.node_to_index( aig.get_node( po ) );
      node x = *m[index];
      if ( aig.is_complemented( po ) )
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
auto Bdd2Ntk_rec( Ntk & ntk, Bdd::BddMan<node> & bdd, node x, std::map<uint64_t, typename Ntk::signal> & m )
{
  if ( x == bdd.Const0() )
    {
      return ntk.get_constant( 0 );
    }
  if ( x == bdd.Const1() )
    {
      return ntk.get_constant( 1 );
    }
  if ( m.count( bdd.Id( x ) ) )
    {
      return m[bdd.Id( x )];
    }
  if ( bdd.IsCompl( x ) )
    {
      auto f = Bdd2Ntk_rec( ntk, bdd, bdd.Regular( x ), m );
      f = ntk.create_not( f );
      m[bdd.Id( x )] = f;
      return f;
    }
  auto c = ntk.make_signal( ntk.pi_at( bdd.Var( x ) ) );
  auto f1 = Bdd2Ntk_rec( ntk, bdd, bdd.Then( x ), m );
  auto f0 = Bdd2Ntk_rec( ntk, bdd, bdd.Else( x ), m );
  auto f = ntk.create_ite( c, f1, f0 );
  m[bdd.Id( x )] = f;
  return f;
}

template <typename node, typename Ntk>
void Bdd2Ntk( Ntk & ntk, Bdd::BddMan<node> & bdd, std::vector<node> & vNodes )
{
  for ( int i = 0; i < bdd.GetNumVar(); i++ )
    {
      ntk.create_pi();
    }
  std::map<uint64_t, typename Ntk::signal> m;
  for ( node & x : vNodes )
    {
      auto f = Bdd2Ntk_rec( ntk, bdd, x, m );
      ntk.create_po( f );
    }
}

#endif
