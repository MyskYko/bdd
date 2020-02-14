#include <SimpleBddMan.hpp>
#include <CuddMan.hpp>
#include <BuddyMan.hpp>
#include <AigBdd.hpp>
#include <mockturtle/mockturtle.hpp>
#include <lorina/lorina.hpp>

int main()
{
  mockturtle::aig_network aig;
  lorina::read_aiger( "file.aig", mockturtle::aiger_reader( aig ) );
  mockturtle::topo_view aig_topo{aig};

  try {
    Bdd::SimpleBddMan<> man( aig.num_pis() );
    std::vector<uint64_t> vNodes = Bdd::Aig2Bdd( aig_topo, man );
    man.PrintStats( vNodes );
    mockturtle::aig_network aig2;
    Bdd::Bdd2Aig( aig2, man, vNodes );
    mockturtle::write_bench( aig2, "file_simple.bench" );
  }
  catch ( char const * error ) {
    std::cout << error << std::endl;
  }

  try {
    Bdd::CuddMan man( aig.num_pis() );
    std::vector<uint64_t> vNodes = Bdd::Aig2Bdd( aig_topo, man );
    man.PrintStats( vNodes );
    mockturtle::aig_network aig2;
    Bdd::Bdd2Aig( aig2, man, vNodes );
    mockturtle::write_bench( aig2, "file_cudd.bench" );
  }
  catch ( char const * error ) {
    std::cout << error << std::endl;
  }

  try {
    Bdd::BuddyMan man( aig.num_pis() );
    std::vector<uint64_t> vNodes = Bdd::Aig2Bdd( aig_topo, man );
    man.PrintStats( vNodes );
    mockturtle::aig_network aig2;
    Bdd::Bdd2Aig( aig2, man, vNodes );
    mockturtle::write_bench( aig2, "file_buddy.bench" );
  }
  catch ( char const * error ) {
    std::cout << error << std::endl;
  }

  return 0;
}
