#include <SimpleBddMan.hpp>
#include <CuddMan.hpp>
#include <BuddyMan.hpp>
#include <CacBddMan.hpp>
#include <AtBddMan.hpp>

#include <NtkBdd.hpp>
#include <mockturtle/mockturtle.hpp>
#include <lorina/lorina.hpp>

#include <string>
#include <chrono>

template <typename node>
void run( Bdd::BddMan<node> & bdd, mockturtle::aig_network & aig, mockturtle::klut_network * klut, bool dvr, std::vector<std::string> & pi_names ) {
  if(dvr) {
    bdd.Dvr();
  }
  auto start = std::chrono::system_clock::now();
  auto vNodes = Aig2Bdd( aig, bdd );
  auto end = std::chrono::system_clock::now();
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
	  std::cout << "pi" << i << " ";
	}
	else {
	  std::cout << pi_names[i] << " ";
	}
      }
    std::cout << std::endl;
  }
  if(klut) {
    Bdd2Ntk( *klut, bdd, vNodes );
  }
}

int main( int argc, char ** argv ) {
  std::string aigname;
  std::string blifname;
  int package = 0;
  bool supportname = 1;
  bool dvr = 0;
  
  for(int i = 1; i < argc; i++) {
    if(argv[i][0] != '-') {
      if(aigname.empty()) {
	aigname = argv[i];
	continue;
      }
      else {
	std::cout << "invalid option " << argv[i] << std::endl;
	return 1;
      }
    }
    else if(argv[i][1] == '\0') {
      std::cout << "invalid option " << argv[i] << std::endl;
      return 1;
    }
    int i_ = i;
    for(int j = 1; argv[i_][j] != '\0'; j++) {
      if(i != i_) {
	std::cout << "invalid option " << argv[i_] << std::endl;
	return 1;
      }
      switch(argv[i_][j]) {
      case 'o':
	if(i+1 >= argc) {
	  std::cout << "-o must be followed by file name" << std::endl;
	  return 1;
	}
	blifname = argv[++i];
	break;
      case 'p':
	try {
	  package = std::stoi(argv[++i]);
	}
	catch(...) {
	  std::cout << "-n must be followed by integer" << std::endl;
	  return 1;
	}
	break;
      case 's':
	supportname ^= 1;
	break;
      case 'r':
	dvr ^= 1;
	break;
      case 'h':
	std::cout << "usage : aig2bdd <options> your.aig" << std::endl;
	std::cout << "\t-h       : show this usage" << std::endl;
	std::cout << "\t-o <str> : dump BDD as a blif file " << std::endl;
	std::cout << "\t-p <int> : package [default = " << package << "]" << std::endl;
	std::cout << "\t           \t0 : cudd" << std::endl;
	std::cout << "\t           \t1 : buddy" << std::endl;
	std::cout << "\t           \t2 : cacbdd" << std::endl;
	std::cout << "\t           \t3 : simplebdd" << std::endl;
	std::cout << "\t           \t4 : custombdd" << std::endl;
	std::cout << "\t-s       : toggle keeping name of PI/PO [default = " << supportname << "]" << std::endl;
	std::cout << "\t-r       : toggle dynamic variable reordering [default = " << dvr << "]" << std::endl;
	return 0;
      default:
	std::cout << "invalid option " << argv[i] << std::endl;
      }
    }
  }
  if(aigname.empty()) {
    std::cout << "specify aigname" << std::endl;
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
  switch(package) {
  case 0:
    {
      Bdd::CuddMan bdd( aig.num_pis() );
      run( bdd, aig, klut, dvr, pi_names );
    }
    break;
  case 1:
    {
      Bdd::BuddyMan bdd( aig.num_pis() );
      run( bdd, aig, klut, dvr, pi_names );
    }
    break;
  case 2:
    {
      Bdd::CacBddMan bdd( aig.num_pis() );
      run( bdd, aig, klut, dvr, pi_names );
    }
    break;
  case 3:
    {
      Bdd::SimpleBddMan bdd( aig.num_pis() );
      run( bdd, aig, klut, dvr, pi_names );
    }
    break;
  case 4:
    {
      Bdd::AtBddMan bdd( aig.num_pis() );
      run( bdd, aig, klut, dvr, pi_names );
    }
    break;
  default:
    std::cout << "unknown package number " << package << std::endl;
    break;
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
