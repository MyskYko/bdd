#include <SimpleBddMan.hpp>
#include <mockturtle/mockturtle.hpp>
#include <lorina/lorina.hpp>

std::vector<uint64_t> BuildBdd( mockturtle::aig_network & aig, Bdd::BddMan & man )
{
  int * pFanouts = (int *)calloc( aig.size(), sizeof(int) );
  if ( !pFanouts )
    throw "Allocation failed";
  aig.foreach_gate( [&]( auto gate )
    {
      pFanouts[aig.node_to_index( gate )] = aig.fanout_size( gate );
    });
  std::map<uint32_t, uint64_t> m;
  m[aig.node_to_index( aig.get_node( aig.get_constant( 0 ) ) )] = man.Const0();
  aig.foreach_pi( [&]( auto pi, int i )
    {
      m[aig.node_to_index( pi )] =  man.IthVar( i );
    });
  aig.foreach_gate( [&]( auto gate )
    {
      uint64_t x;
      x = man.Const1();
      aig.foreach_fanin( gate, [&]( auto fanin )
        {
	  man.Ref( x );
	  uint64_t y = man.And( x, man.NotCond( m[aig.node_to_index( aig.get_node( fanin ) )], aig.is_complemented( fanin ) ) );
	  man.Deref( x );
	  x = y;
	});
      m[aig.node_to_index( gate )] = x;
      man.Ref( x );
      aig.foreach_fanin( gate, [&]( auto fanin )
        {
	  auto index = aig.node_to_index( aig.get_node( fanin ) );
	  pFanouts[index] -= 1;
	  if ( pFanouts[index] == 0 )
	    man.Deref( m[index] );
	});
    });
  std::vector<uint64_t> vNodes;
  aig.foreach_po( [&]( auto po )
    {
      auto index = aig.node_to_index( aig.get_node( po ) );
      vNodes.push_back( man.NotCond( m[index], aig.is_complemented( po ) ) );
      man.Ref( vNodes.back() );
      pFanouts[index] -= 1;
      if ( pFanouts[index] == 0 )
	man.Deref( m[index] );
    });
  free( pFanouts );
  return vNodes;
}

auto Bdd2Aig_rec( mockturtle::aig_network & aig, Bdd::BddMan & man, uint64_t x, std::map<uint64_t, mockturtle::aig_network::signal> & m )
{
  if ( x == man.Const0() )
    return aig.get_constant( 0 );
  if ( x == man.Const1() )
    return aig.get_constant( 1 );
  if ( m.count( man.Regular( x ) ) )
    {
      auto f = m[man.Regular( x )];
      if ( man.IsCompl( x ) )
	f = aig.create_not( f );
      return f;
    }
  int v = man.Var( x );
  uint64_t x1 = man.Then( x );
  uint64_t x0 = man.Else( x );
  auto f1 = Bdd2Aig_rec( aig, man, x1, m );
  auto f0 = Bdd2Aig_rec( aig, man, x0, m );
  auto c = aig.make_signal( aig.pi_at( v ) );
  auto f = aig.create_ite( c, f1, f0 );
  if ( man.IsCompl( x ) )
    {
      m[man.Regular( x )] = aig.create_not( f );
      return f;
    }
  m[man.Regular( x )] = f;
  return f;
}

void Bdd2Aig( mockturtle::aig_network & aig, Bdd::BddMan & man, std::vector<uint64_t> & vNodes )
{
  for ( int i = 0; i < man.GetNumVar(); i++ )
    aig.create_pi();
  std::map<uint64_t, mockturtle::aig_network::signal> m;
  for ( uint64_t x : vNodes )
    {
      auto f = Bdd2Aig_rec( aig, man, x, m );
      aig.create_po( f );
    }
}

int main()
{
  mockturtle::aig_network aig;
  lorina::read_aiger( "file.aig", mockturtle::aiger_reader( aig ) );
  mockturtle::topo_view aig_topo{aig};

  try {
    Bdd::SimpleBddMan<> man( aig.num_pis() );
    std::vector<uint64_t> vNodes = BuildBdd( aig_topo, man );
    man.PrintStats( vNodes );
    mockturtle::aig_network aig2;
    Bdd2Aig( aig2, man, vNodes );
    mockturtle::write_bench( aig2, "file_simple.bench" );
  }
  catch ( char const * error ) {
    std::cout << error << std::endl;
  }
  /*
  try {
    Bdd::CuddMan man( aig.num_pis() );
    std::vector<uint64_t> vNodes = BuildBdd( aig_topo, man );
    man.PrintStats( vNodes );
    mockturtle::aig_network aig2;
    Bdd2Aig( aig2, man, vNodes );
    mockturtle::write_bench( aig2, "file_cudd.bench" );
  }
  catch ( char const * error ) {
    std::cout << error << std::endl;
  }

  try {
    Bdd::BuddyMan man( aig.num_pis() );
    std::vector<uint64_t> vNodes = BuildBdd( aig_topo, man );
    man.PrintStats( vNodes );
    mockturtle::aig_network aig2;
    Bdd2Aig( aig2, man, vNodes );
    mockturtle::write_bench( aig2, "file_buddy.bench" );
  }
  catch ( char const * error ) {
    std::cout << error << std::endl;
  }
  */
  return 0;
}
