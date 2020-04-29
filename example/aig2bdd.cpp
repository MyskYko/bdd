#include <SimpleBddMan.hpp>
#include <CuddMan.hpp>
#include <BuddyMan.hpp>
#include <CacBddMan.hpp>
#include <AtBddMan.hpp>

#include <NtkBdd.hpp>
#include <mockturtle/mockturtle.hpp>
#include <lorina/lorina.hpp>
#include <string>

using std::cout;
using std::endl;

int main( int argc, char ** argv )
{
  std::string aigname;
  std::string blifname;
  int package = 0;
  bool supportname = 1;
  
  for(int i = 1; i < argc; i++) {
    if(argv[i][0] != '-') {
      if(aigname.empty()) {
	aigname = argv[i];
	continue;
      }
      else {
	cout << "invalid option " << argv[i] << endl;
	return 1;
      }
    }
    else if(argv[i][1] == '\0') {
      cout << "invalid option " << argv[i] << endl;
      return 1;
    }
    int i_ = i;
    for(int j = 1; argv[i_][j] != '\0'; j++) {
      if(i != i_) {
	cout << "invalid option " << argv[i_] << endl;
	return 1;
      }
      switch(argv[i_][j]) {
      case 'o':
	if(i+1 >= argc) {
	  cout << "-o must be followed by file name" << endl;
	  return 1;
	}
	blifname = argv[++i];
	break;
      case 'p':
	try {
	  package = std::stoi(argv[++i]);
	}
	catch(...) {
	  cout << "-n must be followed by integer" << endl;
	  return 1;
	}
	break;
      case 's':
	supportname ^= 1;
	break;
      case 'h':
	cout << "usage : aig2bdd <options> your.aig" << endl;
	cout << "\t-h       : show this usage" << endl;
	cout << "\t-o <str> : dump BDD as a blif file " << endl;
	cout << "\t-p <int> : package [default = " << package << "]" << endl;
	cout << "\t           \t0 : cudd" << endl;
	cout << "\t           \t1 : buddy" << endl;
	cout << "\t           \t2 : cacbdd" << endl;
	cout << "\t           \t3 : simplebdd" << endl;
	cout << "\t           \t4 : custombdd" << endl;
	cout << "\t-s       : toggle keeping name of PI/PO [default = " << supportname << "]" << endl;
	return 0;
      default:
	cout << "invalid option " << argv[i] << endl;
      }
    }
  }
  if(aigname.empty()) {
    cout << "specify aigname" << endl;
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
		     po_names.push_back(namevec[0]);
		   });
  }
  else {
    lorina::read_aiger(aigname, mockturtle::aiger_reader(aig));
  }
  
  mockturtle::klut_network klut;
  switch(package) {
  case 0:
    {
      Bdd::CuddMan bdd( aig.num_pis() );
      auto vNodes = Aig2Bdd( aig, bdd );
      bdd.PrintStats( vNodes );
      if(!blifname.empty()) {
	Bdd2Ntk( klut, bdd, vNodes );
      }
    }
    break;
  case 1:
    {
      Bdd::BuddyMan bdd( aig.num_pis() );
      auto vNodes = Aig2Bdd( aig, bdd );
      bdd.PrintStats( vNodes );
      if(!blifname.empty()) {
	Bdd2Ntk( klut, bdd, vNodes );
      }
    }
    break;
  case 2:
    {
      Bdd::CacBddMan bdd( aig.num_pis() );
      auto vNodes = Aig2Bdd( aig, bdd );
      bdd.PrintStats( vNodes );
      if(!blifname.empty()) {
	Bdd2Ntk( klut, bdd, vNodes );
      }
    }
    break;
  case 3:
    {
      Bdd::SimpleBddMan bdd( aig.num_pis() );
      auto vNodes = Aig2Bdd( aig, bdd );
      bdd.PrintStats( vNodes );
      if(!blifname.empty()) {
	Bdd2Ntk( klut, bdd, vNodes );
      }
    }
    break;
  case 4:
    {
      Bdd::AtBddMan bdd( aig.num_pis() );
      auto vNodes = Aig2Bdd( aig, bdd );
      bdd.PrintStats( vNodes );
      if(!blifname.empty()) {
	Bdd2Ntk( klut, bdd, vNodes );
      }
    }
    break;
  default:
    cout << "unknown package number " << package << endl;
    break;
  }

  if(!blifname.empty()) {
    if(supportname) {
      mockturtle::names_view klut_{klut};
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
      mockturtle::write_blif( klut, blifname );
    }
  }

  return 0;
}
