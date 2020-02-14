#include <BddMan.hpp>
#include <mockturtle/mockturtle.hpp>

namespace Bdd
{
  void Aig2Bdd( mockturtle::aig_network & aig_, BddMan & bdd )
  {
    mockturtle::topo_view aig{aig_};
    int * pFanouts = (int *)calloc( aig.size(), sizeof(int) );
    if ( !pFanouts )
      throw "Allocation failed";
    aig.foreach_gate( [&]( auto gate )
      {
	pFanouts[aig.node_to_index( gate )] = aig.fanout_size( gate );
      });
    std::map<uint32_t, uint64_t> m;
    m[aig.node_to_index( aig.get_node( aig.get_constant( 0 ) ) )] = bdd.Const0();
    aig.foreach_pi( [&]( auto pi, int i )
      {
	m[aig.node_to_index( pi )] =  bdd.IthVar( i );
      });
    aig.foreach_gate( [&]( auto gate )
      {
	uint64_t x;
	x = bdd.Const1();
	aig.foreach_fanin( gate, [&]( auto fanin )
	  {
	    bdd.Ref( x );
	    uint64_t y = bdd.And( x, bdd.NotCond( m[aig.node_to_index( aig.get_node( fanin ) )], aig.is_complemented( fanin ) ) );
	    bdd.Deref( x );
	    x = y;
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
	bdd.vNodes.push_back( bdd.NotCond( m[index], aig.is_complemented( po ) ) );
	bdd.Ref( bdd.vNodes.back() );
	pFanouts[index] -= 1;
	if ( pFanouts[index] == 0 )
	  bdd.Deref( m[index] );
      });
    free( pFanouts );
  }
  
  auto Bdd2Aig_rec( mockturtle::aig_network & aig, BddMan & bdd, uint64_t x, std::map<uint64_t, mockturtle::aig_network::signal> & m )
  {
    if ( x == bdd.Const0() )
      return aig.get_constant( 0 );
    if ( x == bdd.Const1() )
      return aig.get_constant( 1 );
    if ( m.count( bdd.Regular( x ) ) )
      {
	auto f = m[bdd.Regular( x )];
	if ( bdd.IsCompl( x ) )
	  f = aig.create_not( f );
	return f;
      }
    int v = bdd.Var( x );
    uint64_t x1 = bdd.Then( x );
    uint64_t x0 = bdd.Else( x );
    auto c = aig.make_signal( aig.pi_at( v ) );
    auto f1 = Bdd2Aig_rec( aig, bdd, x1, m );
    auto f0 = Bdd2Aig_rec( aig, bdd, x0, m );
    auto f = aig.create_ite( c, f1, f0 );
    if ( bdd.IsCompl( x ) )
      {
	m[bdd.Regular( x )] = aig.create_not( f );
	return f;
      }
    m[bdd.Regular( x )] = f;
    return f;
  }
  
  void Bdd2Aig( mockturtle::aig_network & aig, BddMan & bdd )
  {
    for ( int i = 0; i < bdd.GetNumVar(); i++ )
      aig.create_pi();
    std::map<uint64_t, mockturtle::aig_network::signal> m;
    for ( uint64_t x : bdd.vNodes )
      {
	auto f = Bdd2Aig_rec( aig, bdd, x, m );
	aig.create_po( f );
      }
  }
}
