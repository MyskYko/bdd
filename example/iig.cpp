#include <SimpleBddMan.hpp>
#include <CuddMan.hpp>
#include <BuddyMan.hpp>
#include <CacBddMan.hpp>
#include <AtBddMan.hpp>

#include <iig.hpp>
#include <mockturtle/mockturtle.hpp>
#include <lorina/lorina.hpp>
#include <string>
#include <random>

using std::cout;
using std::endl;

int main( int argc, char ** argv )
{
  std::string aigname;
  int package = 0;
  std::string init;
  int exclude = 1;
  
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
      case 'i':
	if(i+1 >= argc) {
	  cout << "-i must be followed by string" << endl;
	  return 1;
	}
	init = argv[++i];
	for(int j = 0; j < init.size(); j++) {
	  if(init[j] != '0' && init[j] != '1') {
	    cout << "-i must be followed by boolean vector" << endl;
	    return 1;
	  }
	}
	break;
      case 'n':
	try {
	  exclude = std::stoi(argv[++i]);
	}
	catch(...) {
	  cout << "-n must be followed by integer" << endl;
	  return 1;
	}
	break;
      case 'p':
	try {
	  package = std::stoi(argv[++i]);
	}
	catch(...) {
	  cout << "-p must be followed by integer" << endl;
	  return 1;
	}
	break;
      case 'h':
	cout << "usage : iig <options> your.aig" << endl;
	cout << "\t-h       : show this usage" << endl;
	cout << "\t-i <str> : initial states as boolean vector [default = 0...0]" << endl;
	cout << "\t-n <int> : number of states initially excluded [default = " << exclude << "]" << endl;
	cout << "\t-p <int> : package [default = " << package << "]" << endl;
	cout << "\t           \t0 : cudd" << endl;
	cout << "\t           \t1 : buddy" << endl;
	cout << "\t           \t2 : cacbdd" << endl;
	cout << "\t           \t3 : simplebdd" << endl;
	cout << "\t           \t4 : custombdd" << endl;
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
  lorina::read_aiger(aigname, mockturtle::aiger_reader(aig));
  
  if(init.empty()) {
    for(int i = 0; i < aig.num_registers(); i++) {
      init += "0";
    }
  }
  else if(init.size() != aig.num_registers()) {
    cout << "the size of boolean bector is not equal to the number of registers" << endl;
    return 1;
  }
  
  switch(package) {
  case 0:
    {
      Bdd::CuddMan bdd( aig.num_pis() );
      IIG(aig, bdd, init, exclude);
    }
    break;
  case 1:
    {
      Bdd::BuddyMan bdd( aig.num_pis() );
      IIG(aig, bdd, init, exclude);  
    }
    break;
  case 2:
    {
      Bdd::CacBddMan bdd( aig.num_pis() );
      IIG(aig, bdd, init, exclude);
    }
    break;
  case 3:
    {
      Bdd::SimpleBddMan bdd( aig.num_pis() );
      IIG(aig, bdd, init, exclude);
    }
    break;
  case 4:
    {
      Bdd::AtBddMan bdd( aig.num_pis() );
      IIG(aig, bdd, init, exclude);
    }
    break;
  default:
    cout << "unknown package number " << package << endl;
    break;
  }
  
  return 0;
}
