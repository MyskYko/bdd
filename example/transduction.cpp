#include <SimpleBddMan.hpp>
#include <CuddMan.hpp>
#include <BuddyMan.hpp>
#include <CacBddMan.hpp>
#include <AtBddMan.hpp>

#include <Transduction.hpp>
#include <mockturtle/mockturtle.hpp>
#include <lorina/lorina.hpp>
#include <string>

int main( int argc, char ** argv )
{
  std::string aigname;
  std::string blifname;
  std::string dcname;
  int package = 0;
  bool supportname = 1;
  bool reorder = 0;
  bool repeat = 0;
  bool mspf = 0;
  bool check = 0;
  int verbose = 0;
  int pverbose = 0;
  
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
	check ^= 1;
	break;
      case 'm':
	mspf ^= 1;
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
	  std::cerr << "-p must be followed by integer" << std::endl;
	  return 1;
	}
	break;
      case 'q':
	repeat ^= 1;
	break;
      case 'r':
	reorder ^= 1;
	break;
      case 's':
	supportname ^= 1;
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
      case 'x':
	if(i+1 >= argc) {
	  std::cerr << "-x must be followed by file name" << std::endl;
	  return 1;
	}
	dcname = argv[++i];
	break;
      case 'h':
	std::cout << "usage : transduction <options> your.aig" << std::endl;
	std::cout << "\t-c       : toggle checking equivalence [default = " << check << "]" << std::endl;
	std::cout << "\t-h       : show this usage" << std::endl;
	std::cout << "\t-m       : toggle applying mspf [default = " << mspf << "]" << std::endl;
	std::cout << "\t-o <str> : output the resulting circuit as a blif file " << std::endl;
	std::cout << "\t-p <int> : package [default = " << package << "]" << std::endl;
	std::cout << "\t           \t0 : cudd" << std::endl;
	std::cout << "\t           \t1 : buddy" << std::endl;
	std::cout << "\t           \t2 : cacbdd" << std::endl;
	std::cout << "\t           \t3 : simplebdd" << std::endl;
	std::cout << "\t           \t4 : custombdd" << std::endl;
	std::cout << "\t-q       : toggle repeating optimization [default = " << repeat << "]" << std::endl;
	std::cout << "\t-r       : toggle dynamic variable reordering in prep step [default = " << reorder << "]" << std::endl;
	std::cout << "\t-s       : toggle keeping name of PI/PO [default = " << supportname << "]" << std::endl;
	std::cout << "\t-v       : toggle verbose information [default = " << verbose << "]" << std::endl;
	std::cout << "\t-x <str> : aig file representing external don't cares [default = " << dcname << "]" << std::endl;
	std::cout << "\t-V       : toggle verbose information inside BDD package [default = " << pverbose << "]" << std::endl;
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

  mockturtle::aig_network * dcaig = NULL;
  if(!dcname.empty()) {
    dcaig = new mockturtle::aig_network;
    lorina::read_aiger(dcname, mockturtle::aiger_reader(*dcaig));
    assert(aig.num_pis() == dcaig->num_pis());
    assert(aig.num_pos() == dcaig->num_pos());
  }

  try {
  switch(package) {
  case 0:
    {
      Bdd::CuddMan bdd( aig.num_pis(), pverbose );
      Transduction( aig, bdd, reorder, repeat, mspf, check, verbose, dcaig );
    }
    break;
  case 1:
    {
      Bdd::BuddyMan bdd( aig.num_pis(), pverbose );
      Transduction( aig, bdd, reorder, repeat, mspf, check, verbose, dcaig );
    }
    break;
  case 2:
    {
      if(pverbose) {
	std::cerr << "the package doesn't have verbose system" << std::endl;
      }
      Bdd::CacBddMan bdd( aig.num_pis() );
      Transduction( aig, bdd, reorder, repeat, mspf, check, verbose, dcaig );
    }
    break;
  case 3:
    {
      Bdd::SimpleBddMan bdd( aig.num_pis(), pverbose );
      Transduction( aig, bdd, reorder, repeat, mspf, check, verbose, dcaig );
    }
    break;
  case 4:
    {
      Bdd::AtBddMan bdd( aig.num_pis(), pverbose );
      Transduction( aig, bdd, reorder, repeat, mspf, check, verbose, dcaig );
    }
    break;
  default:
    std::cerr << "unknown package number " << package << std::endl;
    return 1;
  }
  }
  catch ( const char * e ) {
    std::cerr << e << std::endl;
    return 1;
  }
  catch ( ... ) {
    std::cerr << "error" << std::endl;
    return 1;
  }


  if(!blifname.empty()) {
    if(supportname) {
      mockturtle::names_view aig_{aig};
      aig_.foreach_pi([&](auto pi, int i) {
			 if(!pi_names[i].empty()) {
			   aig_.set_name(aig_.make_signal(pi), pi_names[i]);
			 }
		       });
      for(int i = 0; i < aig_.num_pos(); i++) {
	if(!po_names[i].empty()) {
	  aig_.set_output_name( i, po_names[i] );
	}
      }
      mockturtle::write_blif( aig_, blifname );
    }
    else {
      mockturtle::write_blif( aig, blifname );
    }
  }

  if(dcaig) {
    delete dcaig;
  }

  return 0;
}
