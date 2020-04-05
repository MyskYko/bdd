#include <BddMan.hpp>
#include <mockturtle/mockturtle.hpp>

namespace Bdd
{
  template <typename node>
  void Aig2Bdd( mockturtle::aig_network & aig_, BddMan<node> & bdd )
  {
    mockturtle::topo_view aig{aig_};
    int * pFanouts = (int *)calloc( aig.size(), sizeof(int) );
    if ( !pFanouts )
      throw "Allocation failed";
    aig.foreach_gate( [&]( auto gate )
      {
	pFanouts[aig.node_to_index( gate )] = aig.fanout_size( gate );
      });
    std::map<uint32_t, node> m;
    m[aig.node_to_index( aig.get_node( aig.get_constant( 0 ) ) )] = bdd.Const0();
    aig.foreach_pi( [&]( auto pi, int i )
      {
	m[aig.node_to_index( pi )] =  bdd.IthVar( i );
      });
    aig.foreach_gate( [&]( auto gate )
      {
	node x = bdd.Const1();
	aig.foreach_fanin( gate, [&]( auto fanin )
	  {
	    bdd.Ref( x );
	    node y = m[aig.node_to_index( aig.get_node( fanin ) )];
	    if ( aig.is_complemented( fanin ) )
	      {
		y = bdd.Not( y );
		bdd.RefNot( y );
	      }
	    node z = bdd.And( x, y );
	    bdd.Deref( x );
	    if ( aig.is_complemented( fanin ) )
	      bdd.DerefNot( y );
	    x = z;
	  });
	m[aig.node_to_index( gate )] = x;
	bdd.Ref( x );
	aig.foreach_fanin( gate, [&]( auto fanin )
	  {
	    auto index = aig.node_to_index( aig.get_node( fanin ) );
	    pFanouts[index] -= 1;
	    if ( pFanouts[index] == 0 )
	      bdd.Deref( m[index] );
	  });
      });
    aig.foreach_po( [&]( auto po )
      {
	auto index = aig.node_to_index( aig.get_node( po ) );
	node x = m[index];
	if ( aig.is_complemented( po ) )
	  x = bdd.Not( x );
	bdd.vNodes.push_back( x );
	bdd.Ref( x );
	pFanouts[index] -= 1;
	if ( pFanouts[index] == 0 )
	  bdd.Deref( m[index] );
      });
    free( pFanouts );
  }
  
  template <typename node>
  auto Bdd2Aig_rec( mockturtle::aig_network & aig, BddMan<node> & bdd, node x, std::map<uint64_t, mockturtle::aig_network::signal> & m )
  {
    if ( x == bdd.Const0() )
      return aig.get_constant( 0 );
    if ( x == bdd.Const1() )
      return aig.get_constant( 1 );
    if ( m.count( bdd.Id( bdd.Regular( x ) ) ) )
      {
	auto f = m[bdd.Id( bdd.Regular( x ) )];
	if ( bdd.IsCompl( x ) )
	  f = aig.create_not( f );
	return f;
      }
    int v = bdd.Var( x );
    node x1 = bdd.Then( x );
    node x0 = bdd.Else( x );
    auto c = aig.make_signal( aig.pi_at( v ) );
    auto f1 = Bdd2Aig_rec( aig, bdd, x1, m );
    auto f0 = Bdd2Aig_rec( aig, bdd, x0, m );
    auto f = aig.create_ite( c, f1, f0 );
    if ( bdd.IsCompl( x ) )
      {
	m[bdd.Id( bdd.Regular( x ) )] = aig.create_not( f );
	return f;
      }
    m[bdd.Id( bdd.Regular( x ) )] = f;
    return f;
  }

  template <typename node>
  void Bdd2Aig( mockturtle::aig_network & aig, BddMan<node> & bdd )
  {
    for ( int i = 0; i < bdd.GetNumVar(); i++ )
      aig.create_pi();
    std::map<uint64_t, mockturtle::aig_network::signal> m;
    for ( node x : bdd.vNodes )
      {
	auto f = Bdd2Aig_rec( aig, bdd, x, m );
	aig.create_po( f );
      }
  }
}
