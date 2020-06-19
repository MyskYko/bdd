
#include <SimpleBddMan.hpp>
#include <CuddMan.hpp>
#include <BuddyMan.hpp>
#include <CacBddMan.hpp>
#include <AtBddMan.hpp>

#include <IndInv.hpp>
#include <mockturtle/mockturtle.hpp>
#include <lorina/lorina.hpp>
#include <string>
#include <random>

using std::cout;
using std::endl;

int main( int argc, char ** argv )
{
  std::string aigname;
  std::string dumpfilename;
  int package = 0;
  std::string init;
  std::string exclude = "1";
  bool reverse = 0;
  int seed = 0;
  bool fastrnd = 0;
  bool repeat = 0;
  int numand = 1;
  
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
      case 'f':
	if(i+1 >= argc) {
	  cout << "-f must be followed by string" << endl;
	  return 1;
	}
	dumpfilename = argv[++i];
	break;
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
	if(i+1 >= argc) {
	  cout << "-n must be followed by number" << endl;
	  return 1;
	}
	exclude = argv[++i];
	for(int j = 0; j < exclude.size(); j++) {
	  if(j == 0 && exclude[0] == '-') {
	    continue;
	  }
	  if(exclude[j] < '0'|| exclude[j] > '9') {
	    cout << "-n must be followed by number" << endl;
	    return 1;
	  }
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
      case 'q':
	fastrnd ^= 1;
	break;
      case 'r':
	reverse ^= 1;
	break;
      case 's':
	try {
	  seed = std::stoi(argv[++i]);
	}
	catch(...) {
	  cout << "-s must be followed by integer" << endl;
	  return 1;
	}
	break;
      case 't':
	repeat ^= 1;
	break;
      case 'a':
	try {
	  numand = std::stoi(argv[++i]);
	}
	catch(...) {
	  cout << "-a must be followed by integer" << endl;
	  return 1;
	}
	if(numand <= 0) {
	  cout << "-a must be followed by natural number" << endl;
	  return 1;
	}
	break;
      case 'h':
	cout << "usage : iig <options> your.aig" << endl;
	cout << "\t-h       : show this usage" << endl;
	cout << "\t-f <str> : filename to dump inductive invariant" << endl;
	cout << "\t-i <str> : initial states as boolean vector [default = " << (init.empty() ? "0...0" : init) << "]" << endl;
	cout << "\t-n <int> : number of states initially excluded (minus defines number initially included) [default = " << exclude << "]" << endl;
	cout << "\t-p <int> : package [default = " << package << "]" << endl;
	cout << "\t           \t0 : cudd" << endl;
	cout << "\t           \t1 : buddy" << endl;
	cout << "\t           \t2 : cacbdd" << endl;
	cout << "\t           \t3 : simplebdd" << endl;
	cout << "\t           \t4 : custombdd" << endl;
	cout << "\t-q       : generate initial function faster [default = " << fastrnd << "]" << endl;
	cout << "\t-r       : toggle reverse [default = " << reverse << "]" << endl;
	cout << "\t-s <int> : random seed [default = " << seed << "]" << endl;
	cout << "\t-t       : repeat generation with incrementing seed until success (may cause infinite loop) [default = " << repeat << "]" << endl;
	cout << "\t-a <int> : number of inductive invariants, where the result is AND of them [default = " << numand << "]" << endl;
	return 1;
      default:
	cout << "invalid option " << argv[i] << endl;
	return 1;
      }
    }
  }
  if(aigname.empty()) {
    cout << "specify aigname" << endl;
    return 1;
  }
  if(numand > 1 && reverse) {
    cout << "AND of multiple inductive invariants is not supported in reverse method" << endl;
    return 1;
  }
  if(numand > 1 && !repeat) {
    cout << "when computing multiple invariants, repetition is done with incrementing seed even if -t is not specified" << endl;
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

  int res = 0;
  while(1) {
  switch(package) {
  case 0:
    if(numand > 1) {
      Bdd::CuddMan bdd( aig.num_cis() );
      Bdd::IIGAND(aig, bdd, init, exclude, dumpfilename, seed, fastrnd, numand);
      res = 1;
    }
    else if(!reverse) {
      Bdd::CuddMan bdd( aig.num_cis() );
      res = Bdd::IIG(aig, bdd, init, exclude, dumpfilename, seed, fastrnd);
    }
    else {
      Bdd::CuddMan bdd( aig.num_cis() + aig.num_registers() );
      res = Bdd::RIIG(aig, bdd, init, exclude, dumpfilename, seed, fastrnd);
    }
    break;
  case 1:
    if(numand > 1) {
      Bdd::BuddyMan bdd( aig.num_cis() );
      Bdd::IIGAND(aig, bdd, init, exclude, dumpfilename, seed, fastrnd, numand);
      res = 1;
    }
    else if(!reverse) {
      Bdd::BuddyMan bdd( aig.num_cis() );
      res = Bdd::IIG(aig, bdd, init, exclude, dumpfilename, seed, fastrnd);  
    }
    else {
      Bdd::BuddyMan bdd( aig.num_cis() + aig.num_registers() );
      res = Bdd::RIIG(aig, bdd, init, exclude, dumpfilename, seed, fastrnd);  
    }
    break;
  case 2:
    if(numand > 1) {
      Bdd::CacBddMan bdd( aig.num_cis() );
      Bdd::IIGAND(aig, bdd, init, exclude, dumpfilename, seed, fastrnd, numand);
      res = 1;
    }
    else if(!reverse) {
      Bdd::CacBddMan bdd( aig.num_cis() );
      res = Bdd::IIG(aig, bdd, init, exclude, dumpfilename, seed, fastrnd);
    }
    else {
      Bdd::CacBddMan bdd( aig.num_cis() + aig.num_registers() );
      res = Bdd::RIIG(aig, bdd, init, exclude, dumpfilename, seed, fastrnd);
    }
    break;
  case 3:
    if(numand > 1) {
      Bdd::SimpleBddMan bdd( aig.num_cis() );
      Bdd::IIGAND(aig, bdd, init, exclude, dumpfilename, seed, fastrnd, numand);
      res = 1;
    }    
    else if(!reverse) {
      Bdd::SimpleBddMan bdd( aig.num_cis() );
      res = Bdd::IIG(aig, bdd, init, exclude, dumpfilename, seed, fastrnd);
    }
    else {
      Bdd::SimpleBddMan bdd( aig.num_cis() + aig.num_registers() );
      res = Bdd::RIIG(aig, bdd, init, exclude, dumpfilename, seed, fastrnd);
    }
    break;
  case 4:
    if(numand > 1) {
      Bdd::AtBddMan bdd( aig.num_cis() );
      Bdd::IIGAND(aig, bdd, init, exclude, dumpfilename, seed, fastrnd, numand);
      res = 1;
    }
    else if(!reverse) {
      Bdd::AtBddMan bdd( aig.num_cis() );
      res = Bdd::IIG(aig, bdd, init, exclude, dumpfilename, seed, fastrnd);
    }
    else {
      Bdd::AtBddMan bdd( aig.num_cis() + aig.num_registers() );
      res = Bdd::RIIG(aig, bdd, init, exclude, dumpfilename, seed, fastrnd);
    }
    break;
  default:
    cout << "unknown package number " << package << endl;
    return 1;
  }
  if(repeat && !res) {
    cout << "seed increment to be " << ++seed << endl;
  }
  else {
    break;
  }
  }
  
  return 0;
}
