#include <SimpleBddMan.hpp>
#include <CuddMan.hpp>
#include <BuddyMan.hpp>
#include <CacBddMan.hpp>
#include <AtBddMan.hpp>

#include <NtkBdd.hpp>
#include <BddGraph.hpp>
#include <mockturtle/mockturtle.hpp>
#include <lorina/lorina.hpp>

#include <string>
#include <chrono>

template <typename node>
void run( Bdd::BddMan<node> & bdd, mockturtle::aig_network & aig, mockturtle::klut_network * klut, bool dvr, std::vector<std::string> & pi_names, std::vector<std::string> & po_names, std::string dotname, bool cedge, int verbose ) {
  if(dvr) {
    bdd.Dvr();
  }
  auto start = std::chrono::system_clock::now();
  auto vNodes = Aig2Bdd( aig, bdd, verbose > 1 );
  auto end = std::chrono::system_clock::now();
  if(verbose) {
    std::cout << "time : " << std::chrono::duration_cast<std::chrono::milliseconds>( end - start ).count() << " ms" << std::endl;
    bdd.PrintStats( vNodes );
    if(dvr) {
      std::cout << "Ordering :" << std::endl;
      std::vector<int> v( aig.num_pis() );
      for ( int i = 0; i < aig.num_pis(); i++ )
	{
	  v[bdd.Level( i )] = i;
	}
      for ( int i : v )
	{
	  if ( pi_names.empty() || pi_names[i].empty() ) {
	    std::cout << "pi" << i + 2 << " "; // + 2 matches mocuturtle write_blif
	  }
	  else {
	    std::cout << pi_names[i] << " ";
	  }
	}
      std::cout << std::endl;
    }
  }
  if(klut) {
    Bdd2Ntk( *klut, bdd, vNodes, cedge );
  }
  if(!dotname.empty()) {
    Bdd2Dot( dotname, bdd, vNodes, pi_names, po_names, cedge );
  }
}

int main( int argc, char ** argv ) {
  std::string aigname;
  std::string blifname;
  std::string dotname;
  int package = 0;
  bool supportname = 1;
  bool dvr = 0;
  int verbose = 0;
  int pverbose = 0;
  bool cedge = 0;
  
  for(int i = 1; i < argc; i++) {
    if(argv[i][0] != '-') {
      if(aigname.empty()) {
	aigname = argv[i];
	continue;
      }
      else {
	std::cerr << "invalid option " << argv[i] << std::endl;
	return 1;
      }
    }
    else if(argv[i][1] == '\0') {
      std::cerr << "invalid option " << argv[i] << std::endl;
      return 1;
    }
    int i_ = i;
    for(int j = 1; argv[i_][j] != '\0'; j++) {
      if(i != i_) {
	std::cerr << "invalid option " << argv[i_] << std::endl;
	return 1;
      }
      switch(argv[i_][j]) {
      case 'c':
	cedge ^= 1;
	break;
      case 'd':
	if(i+1 >= argc) {
	  std::cerr << "-d must be followed by file name" << std::endl;
	  return 1;
	}
	dotname = argv[++i];
	break;
      case 'o':
	if(i+1 >= argc) {
	  std::cerr << "-o must be followed by file name" << std::endl;
	  return 1;
	}
	blifname = argv[++i];
	break;
      case 'p':
	try {
	  package = std::stoi(argv[++i]);
	}
	catch(...) {
	  std::cerr << "-n must be followed by integer" << std::endl;
	  return 1;
	}
	break;
      case 's':
	supportname ^= 1;
	break;
      case 'r':
	dvr ^= 1;
	break;
      case 'v':
	try {
	  verbose = std::stoi(argv[++i]);
	}
	catch(...) {
	  std::cerr << "-v must be followed by integer" << std::endl;
	  return 1;
	}
	break;
      case 'V':
	try {
	  pverbose = std::stoi(argv[++i]);
	}
	catch(...) {
	  std::cerr << "-V must be followed by integer" << std::endl;
	  return 1;
	}
	break;
      case 'h':
	std::cout << "usage : aig2bdd <options> your.aig" << std::endl;
	std::cout << "\t-h       : show this usage" << std::endl;
	std::cout << "\t-c       : toggle using complemented edges in output DOT/BLIF [default = " << cedge << "]" << std::endl;
	std::cout << "\t-d <str> : dump BDD as a DOT file" << std::endl;
	std::cout << "\t-o <str> : dump BDD as a BLIF file" << std::endl;
	std::cout << "\t-p <int> : package [default = " << package << "]" << std::endl;
	std::cout << "\t           \t0 : cudd" << std::endl;
	std::cout << "\t           \t1 : buddy" << std::endl;
	std::cout << "\t           \t2 : cacbdd" << std::endl;
	std::cout << "\t           \t3 : simplebdd" << std::endl;
	std::cout << "\t           \t4 : custombdd" << std::endl;
	std::cout << "\t-s       : toggle keeping name of PI/PO [default = " << supportname << "]" << std::endl;
	std::cout << "\t-r       : toggle dynamic variable reordering [default = " << dvr << "]" << std::endl;
	std::cout << "\t-v <int>  : toggle verbose information [default = " << verbose << "]" << std::endl;
	std::cout << "\t-V <int> : toggle verbose information inside BDD package [default = " << pverbose << "]" << std::endl;
	return 0;
      default:
	std::cerr << "invalid option " << argv[i] << std::endl;
	return 1;
      }
    }
  }
  if(aigname.empty()) {
    std::cerr << "specify aigname" << std::endl;
    return 1;
  }
  
  mockturtle::aig_network aig;
  mockturtle::NameMap<mockturtle::aig_network> namemap;
  std::vector<std::string> pi_names;
  std::vector<std::string> po_names;
  if(supportname) {
    lorina::read_aiger(aigname, mockturtle::aiger_reader(aig, &namemap));
    aig.foreach_pi([&](auto pi) {
		     auto namevec = namemap[aig.make_signal(pi)];
		     if(namevec.empty()) {
		       pi_names.push_back("");
		       return;
		     }
		     pi_names.push_back(namevec[0]);
		   });
    aig.foreach_po([&](auto po) {
		     auto namevec = namemap[po];
		     if(namevec.empty()) {
		       po_names.push_back("");
		       return;
		     }
		     po_names.push_back(namevec[namevec.size() - 1]);
		   });
  }
  else {
    lorina::read_aiger(aigname, mockturtle::aiger_reader(aig));
  }
  
  mockturtle::klut_network * klut = NULL;
  if(!blifname.empty()) {
    klut = new mockturtle::klut_network;
  }
  try {
  switch(package) {
  case 0:
    {
      Bdd::CuddMan bdd( aig.num_pis(), pverbose );
      run( bdd, aig, klut, dvr, pi_names, po_names, dotname, cedge, verbose );
    }
    break;
  case 1:
    {
      if(cedge) {
	std::cerr << "the package doesn't use complemented edges" << std::endl;
      }
      Bdd::BuddyMan bdd( aig.num_pis(), pverbose );
      run( bdd, aig, klut, dvr, pi_names, po_names, dotname, 0, verbose );
    }
    break;
  case 2:
    {
      if(pverbose) {
	std::cerr << "the package doesn't have verbose system" << std::endl;
      }
      Bdd::CacBddMan bdd( aig.num_pis() );
      run( bdd, aig, klut, dvr, pi_names, po_names, dotname, cedge, verbose );
    }
    break;
  case 3:
    {
      Bdd::SimpleBddMan bdd( aig.num_pis(), pverbose );
      run( bdd, aig, klut, dvr, pi_names, po_names, dotname, cedge, verbose );
    }
    break;
  case 4:
    {
      Bdd::AtBddMan bdd( aig.num_pis(), pverbose );
      run( bdd, aig, klut, dvr, pi_names, po_names, dotname, cedge, verbose );
    }
    break;
  default:
    std::cerr << "unknown package number " << package << std::endl;
    return 1;
  }
  }
  catch ( char const * e ) {
    std::cerr << e << std::endl;
    return 1;
  }
  catch ( ... ) {
    std::cerr << "error" << std::endl;
    return 1;
  }

  if(klut) {
    if(supportname) {
      mockturtle::names_view klut_{*klut};
      klut_.foreach_pi([&](auto pi, int i) {
			 if(!pi_names[i].empty()) {
			   klut_.set_name(klut_.make_signal(pi), pi_names[i]);
			 }
		       });
      for(int i = 0; i < klut_.num_pos(); i++) {
	if(!po_names[i].empty()) {
	  klut_.set_output_name( i, po_names[i] );
	}
      }
      mockturtle::write_blif( klut_, blifname );
    }
    else {
      mockturtle::write_blif( *klut, blifname );
    }
    delete klut;
  }

  return 0;
}
