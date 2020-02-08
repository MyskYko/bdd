#include "SimpleBdd.hpp"
#include <mockturtle/mockturtle.hpp>
#include <lorina/lorina.hpp>

std::vector<int> BuildBdd( mockturtle::aig_network & aig, SimpleBdd::SimpleBdd<> & man )
{
  int * pFanouts = NULL;
  if ( man.pvFrontiers )
    {
      pFanouts = (int *)calloc( aig.size(), sizeof(int) );
      if ( !pFanouts )
	throw "Allocation failed";
      aig.foreach_gate( [&]( auto gate )
        {
	  pFanouts[aig.node_to_index( gate )] = aig.fanout_size( gate );
	});
    }
  aig.set_value( aig.get_node( aig.get_constant( 0 ) ), man.LitConst0() );
  aig.foreach_pi( [&]( auto pi, int i )
    {
      aig.set_value( pi, man.LitIthVar( i ) );
    });
  aig.foreach_gate( [&]( auto gate, int i )
    {
      unsigned x;
      while ( 1 )
	{
	  x = man.LitConst1();
	  aig.foreach_fanin( gate, [&]( auto fanin )
	    {
	      x = man.And( x, man.LitNotCond( aig.value( aig.get_node( fanin ) ), aig.is_complemented( fanin ) ) );
	    });
	  if ( man.LitIsInvalid( x ) )
	    {
	      if ( man.Refresh() )
		{
		  if ( pFanouts )
		    free( pFanouts );
		  throw "Node overflow";
		}
	    }
	  else
	    break;
	}
      man.InitRefresh();
      aig.set_value( gate, x );
      if ( man.pvFrontiers )
	{
	  man.pvFrontiers->push_back( x );
	  aig.foreach_fanin( gate, [&]( auto fanin )
	    {
	      auto node = aig.get_node( fanin );
	      pFanouts[aig.node_to_index( node )] -= 1;
	      if ( pFanouts[aig.node_to_index( node )] == 0 )
		{
		  auto it = std::find( man.pvFrontiers->begin(), man.pvFrontiers->end(), aig.value( node ) );
		  assert( it != man.pvFrontiers->end() );
		  man.pvFrontiers->erase( it );
		}
	    });
	}
    });
  if ( pFanouts )
    free( pFanouts );
  std::vector<int> vNodes;
  aig.foreach_po( [&]( auto po )
    {
      vNodes.push_back( man.LitNotCond( aig.value( aig.get_node( po ) ), aig.is_complemented( po ) ) );
    });
  return vNodes;
}

/*
void GiaTest( Gia_Man_t * pGia, int nVerbose, int nMem, char * pFileName, int fSort, int fRealloc, int fGC, int nReoThold, int nFinalReo )
{
  int i, j;
  Gia_Obj_t * pObj;
  Abc_BddMan * p;
  Vec_Int_t * vNodes;
  abctime clk, time;
  clk = Abc_Clock();
  p = Abc_BddManAlloc( Gia_ManCiNum( pGia ), 1 << nMem, fRealloc, NULL, nVerbose );
  Abc_BddGiaRefreshConfig( p, fRealloc, fGC, nReoThold );
  if ( Abc_BddGia( pGia, p ) )
    {
      printf( "Error: BDD construction failed\n" );
      Abc_BddManFree( p );
      return;
    }
  time = Abc_Clock() - clk;
  vNodes = Vec_IntAlloc( Gia_ManCoNum( pGia ) );
  Gia_ManForEachCo( pGia, pObj, i )
    Vec_IntPush( vNodes, pObj->Value );
  if ( nFinalReo )
    {
      i = Abc_BddCountNodesArrayShared( p, vNodes );
      clk = Abc_Clock();
      Abc_BddReorderConfig( p, nFinalReo );
      Abc_BddReorder( p, vNodes );
      time += Abc_Clock() - clk;
      j = Abc_BddCountNodesArrayShared( p, vNodes );
      printf( "Final Reorder Gain %d -> %d (%d)\n", i, j, j - i );
    }
  if ( p->ReoThold )
    {
      printf( "Ordering:\n" );
      for ( i = 0; i < p->nVars; i++ )
	if ( p->nVars <= 10 )
	  printf( "pi%d ", Vec_IntEntry( p->vOrdering, i ) );
	else if ( p->nVars <= 100 )
	  printf( "pi%02d ", Vec_IntEntry( p->vOrdering, i ) );
	else if ( p->nVars <= 1000 )
	  printf( "pi%03d ", Vec_IntEntry( p->vOrdering, i ) );
	else
	  printf( "pi%04d ", Vec_IntEntry( p->vOrdering, i ) );
      printf( "\n" );
    }
  printf( "Shared BDD size = %6d nodes.", Abc_BddCountNodesArrayShared( p, vNodes ) );
  ABC_PRT( "  BDD construction time", time );
  printf( "Sum of BDD nodes for each BDD = %d", Abc_BddCountNodesArrayIndependent( p, vNodes ) );
  printf( "  Used nodes = %d  Allocated nodes = %u\n", p->nObjs, ( p->nObjsAlloc == 1 << 31 ) ? p->nObjsAlloc - 1 : p->nObjsAlloc );
  if ( pFileName )
    Abc_BddWriteBlif( p, vNodes, pFileName, fSort );
  Vec_IntFree( vNodes );
  Abc_BddManFree( p );
}
*/

auto Bdd2Aig_rec( mockturtle::aig_network & aig, SimpleBdd::SimpleBdd<> & man, int a, std::map<int, mockturtle::aig_network::signal> & m )
{
  if ( man.BvarIsConst( a ) )
    return aig.get_constant( 0 );
  if ( m.count( a ) )
    return m[a];
  int v = man.VarOfBvar( a );
  unsigned x1 = man.ThenOfBvar( a );
  unsigned x0 = man.ElseOfBvar( a );
  auto f1 = Bdd2Aig_rec( aig, man, man.Lit2Bvar( x1 ), m );
  if ( man.LitIsCompl( x1 ) )
    f1 = aig.create_not( f1 );
  auto f0 = Bdd2Aig_rec( aig, man, man.Lit2Bvar( x0 ), m );
  if ( man.LitIsCompl( x0 ) )
    f0 = aig.create_not( f0 );
  auto c = aig.make_signal( aig.pi_at( man.get_order( v ) ) );
  auto f = aig.create_ite( c, f1, f0 );
  m[a] = f;
  return f;
}

void Bdd2Aig( mockturtle::aig_network & aig, SimpleBdd::SimpleBdd<> & man, std::vector<int> & vNodes )
{
  for ( int i = 0; i < man.get_nVars(); i++ )
    aig.create_pi();
  std::map<int, mockturtle::aig_network::signal> m;
  for ( unsigned x : vNodes )
    {
      auto f = Bdd2Aig_rec( aig, man, man.Lit2Bvar( x ), m );
      if ( man.LitIsCompl( x ) )
	f = aig.create_not( f );
      aig.create_po( f );
    }
}

int main()
{
  mockturtle::aig_network aig;
  lorina::read_aiger( "file.aig", mockturtle::aiger_reader( aig ) );
  mockturtle::topo_view aig_topo{aig};
  
  try {
    SimpleBdd::SimpleBdd<> man(aig.num_pis(), 1, 1, NULL, 2);
    man.RefreshConfig( 1, 1, 10 );
    std::vector<int> vNodes = BuildBdd( aig_topo, man );
    std::cout << "Shared BDD nodes = " << man.CountNodesArrayShared( vNodes ) << std::endl;
    std::cout << "Sum of BDD nodes = " << man.CountNodesArrayIndependent( vNodes ) << std::endl;
    
    mockturtle::aig_network aig2;
    Bdd2Aig( aig2, man, vNodes );
    mockturtle::write_bench( aig2, "file2.bench" );
  }
  catch ( char const * error ) {
    std::cout << error << std::endl;
  }

  
  return 0;
}
