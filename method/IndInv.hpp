#pragma once

#include <fstream>
#include <algorithm>
#include <chrono>

#include "NtkBdd.hpp"

namespace Bdd {
  void CopyAigRec(mockturtle::aig_network & aig, mockturtle::aig_network & aig2, std::map<mockturtle::aig_network::node, mockturtle::aig_network::signal> & m, mockturtle::aig_network::node const & x) {
    if(aig.is_ci(x))
      return;
    std::vector<mockturtle::aig_network::signal> fanin;
    aig.foreach_fanin(x, [&](auto fi) {
			   CopyAigRec(aig, aig2, m, aig.get_node(fi));
			   auto fi2 = m[aig.get_node(fi)];
			   if(aig.is_complemented(fi))
			     fanin.push_back(aig2.create_not(fi2));
			   else 
			     fanin.push_back(fi2);
			 });
    assert(fanin.size() == 2);
    m[x] = aig2.create_and(fanin[0], fanin[1]);
  }

  void CombNoPo(mockturtle::aig_network & aig, mockturtle::aig_network & aig2) {
    std::map<mockturtle::aig_network::node, mockturtle::aig_network::signal> m;
    aig.foreach_pi([&](auto pi) {
		     m[pi] = aig2.create_pi();
		   });
    aig.foreach_register([&](auto reg) {
			   m[reg.second] = aig2.create_pi();
			 });
    aig.foreach_register([&](auto reg) {
			   CopyAigRec(aig, aig2, m, aig.get_node(reg.first));
			   auto regsig = m[aig.get_node(reg.first)];
			   if(aig.is_complemented(reg.first))
			     aig2.create_po(aig2.create_not(regsig));
			   else
			     aig2.create_po(regsig);
			 });
  }

  template <typename node>
  void show_bdd_rec(BddMan<node> & bdd, node x, int npis, std::string & s, std::ofstream * f, std::string & count) {
    if(x == bdd.Const1())
      {
	if(f) 
	  *f << s << std::endl;
	int idx = 0;
	for(int i = 0; i < s.size(); i++)
	  if(s[i] == '-')
	    idx++;
	while(count[idx] != '0') {
	  count[idx] = '0';
	  idx++;
	}
	count[idx] = '1';
	return;
      }
    if(x == bdd.Const0())
      return;
    s[bdd.Var(x) - npis] = '1';
    show_bdd_rec(bdd, bdd.Then(x), npis, s, f, count);
    s[bdd.Var(x) - npis] = '0';
    show_bdd_rec(bdd, bdd.Else(x), npis, s, f, count);
    s[bdd.Var(x) - npis] = '-';
  }

  template <typename node> 
  void show_bdd(BddMan<node> & bdd, node & x, int npis, int nregs, std::ofstream * f) {
    std::string s;
    for(int i = 0; i < nregs; i++)
      s += "-";
    std::string count = "";
    for(int i = 0; i < nregs + 1; i++)
      count += "0";
    show_bdd_rec(bdd, x, npis, s, f, count);
  
    std::cout << "iig size : ";
    for(int i = 0; i < nregs / 32 + 1; i++) {
      uint count_int = 0;
      for(int j = 0 ; j < 32 && j + i * 32 < count.size(); j++)
	if(count[j + i * 32] == '1')
	  count_int += 1 << j;
      if(!i)
	std::cout << count_int;
      else
	std::cout << " + " << count_int << " * 2^" << 32 * i;
    }

    std::cout << " / ";
    std::cout << ((uint)1 << (nregs % 32));
    if(nregs / 32)
      std::cout << " * 2^" << 32 * (nregs / 32);
    std::cout << std::endl;
  }

  std::string d2b(std::string decimal) {
    std::string binary;
    while(decimal.size() > 32) {
      unsigned long s = std::stoul(decimal.substr(decimal.size() - 32));
      for(int i = 0; i < 32; i++) {
	if(s % 2)
	  binary += "1";
	else
	  binary += "0";
	s = s >> 1;
      }
      decimal = decimal.substr(0, decimal.size() - 32);
    }
    unsigned long s = std::stoul(decimal);
    for(int i = 0; i < 32; i++) {
      if(s % 2)
	binary += "1";
      else
	binary += "0";
      s = s >> 1;
    }
    return binary;
  }

  template <typename node> 
  node initial_function(BddMan<node> & bdd, node & init, std::string nzero, int npis, int nregs, int seed) {
    std::mt19937 rnd(seed);
    bool rev = 0;
    if(nzero[0] == '-') {
      nzero = nzero.substr(1);
      rev = 1;
    }
    assert(nzero != "0");
    nzero = d2b(nzero);
    node x = bdd.Const1();
    if(rev)
      x = init;
    std::string i;
    for(int j = 0; j < nzero.size(); j++)
      i += "0";
    if(rev)
      i[0] = '1';
    while(1) {
      if(i == nzero)
	break;
      while(1) {
	node y = bdd.Const1();
	for(int j = 0; j < nregs; j++) {
	  if((int)rnd() > 0)
	    y = bdd.And(y, bdd.IthVar(npis+j));
	  else
	    y = bdd.And(y, bdd.Not(bdd.IthVar(npis+j)));
	}
	if(!rev) {
	  if(y == init)
	    continue;
	  if(bdd.And(bdd.Not(x), y) != bdd.Const0())
	    continue;
	  x = bdd.And(x, bdd.Not(y));
	  break;
	}
	else {
	  if(bdd.And(x, y) != bdd.Const0())
	    continue;
	  x = bdd.Or(x, y);
	  break;
	}
      }
      int idx = 0;
      while(i[idx] != '0') {
	i[idx] = '0';
	idx++;
      }
      i[idx] = '1';
    }
    return x;
  }

  template <typename node> 
  node initial_function_fast(BddMan<node> & bdd, node & init, std::string nzero, int npis, int nregs, int seed) {
    std::mt19937 rnd(seed);
    bool rev = 0;
    if(nzero[0] == '-') {
      nzero = nzero.substr(1);
      rev = 1;
    }
    assert(nzero != "0");
    nzero = d2b(nzero);
    node x = bdd.Const1();
    if(rev)
      x = init;
    if(rev) {
      for(int i = 0; 1; i++) {
	if(nzero[i] == '1') {
	  nzero[i] = '0';
	  break;
	}
	nzero[i] = '1';
      }
    }
    while(1) {
      while(!nzero.empty() && nzero[nzero.size() - 1] == '0')
	nzero = nzero.substr(0, nzero.size() - 1);
      if(nzero.empty()) {
	break;
      }
      while(1) {
	std::set<int> vars;
	while(vars.size() <= nregs - nzero.size()) {
	  int j = rnd() % nregs;
	  assert(j >= 0 && j < nregs);
	  vars.insert(j);
	}
	node y = bdd.Const1();
	for(int var : vars) {
	  if((int)rnd() > 0)
	    y = bdd.And(y, bdd.IthVar(npis+var));
	  else
	    y = bdd.And(y, bdd.Not(bdd.IthVar(npis+var)));
	}
	if(!rev) {
	  if(bdd.And(init, y) != bdd.Const0())
	    continue;
	  if(bdd.And(bdd.Not(x), y) != bdd.Const0())
	    continue;
	  x = bdd.And(x, bdd.Not(y));
	  break;
	}
	else {
	  if(bdd.And(x, y) != bdd.Const0())
	    continue;
	  x = bdd.Or(x, y);
	  break;
	}
      }
      nzero = nzero.substr(0, nzero.size() - 1);
    }
    return x;
  }

  std::chrono::system_clock::time_point show_time(std::chrono::system_clock::time_point & t1) {
    auto t2 = std::chrono::system_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count() / 1000.0 << " sec" << std::endl;
    return t2;
  }

  template <typename node> 
  int IIG(mockturtle::aig_network & aig_, BddMan<node> & bdd, std::string initstr, std::string nzero, std::string filename, int seed, bool fastrnd) {
    int npis = aig_.num_pis();
    int nregs = aig_.num_registers();
    std::cout << "PI : " << npis << " , REG : " << nregs << std::endl;
    mockturtle::aig_network aig;
    CombNoPo(aig_, aig);

    node pis = bdd.Const1();
    for(int j = 0; j < npis; j++)
      pis = bdd.And(pis, bdd.IthVar(j));

    node init = bdd.Const1();
    for(int i = 0; i < nregs; i++) {
      if(initstr[i] == '0')
	init = bdd.And(init, bdd.Not(bdd.IthVar(npis+i)));
      else
	init = bdd.And(init, bdd.IthVar(npis+i));
    }

    auto t1 = std::chrono::system_clock::now();
  
    std::cout << "init rnd func ";
    node x;
    if(fastrnd)
      x = initial_function_fast(bdd, init, nzero, npis, nregs, seed);
    else
      x = initial_function(bdd, init, nzero, npis, nregs, seed);
    t1 = show_time(t1);

    std::cout << "build latch   ";
    std::vector<node> vNodes = Aig2Bdd( aig, bdd );
    for(int i = 0; i < npis; i++)
      vNodes.insert(vNodes.begin(), bdd.Const0());
    t1 = show_time(t1);

    std::cout << std::endl << "##### begin iig #####" << std::endl;
    auto start = t1;
    int itr = 0;
    while(1) {
      std::cout << "iteration " << itr++ << "   ";
      node y = bdd.VecCompose(x, vNodes);
      node z = bdd.Or(bdd.Not(x), y);
      if(z == bdd.Const1())
	break;
      node k = bdd.Univ(z, pis);
      x = bdd.And(x, k);
      if(bdd.And(x, init) == bdd.Const0()) {
	x = bdd.Const0();
	break;
      }
      t1 = show_time(t1);
    }
    t1 = show_time(t1);

    std::cout << "total ";
    show_time(start);
  
    if(x == bdd.Const0()) {
      std::cout << "fail ... function excluded initial state" << std::endl;
      std::cout << "##### end iig #####" << std::endl << std::endl;
      return 0;
    }
    if(x == bdd.Const1()) {
      std::cout << "fail ... function is const 1" << std::endl;
      std::cout << "##### end iig #####" << std::endl << std::endl;
      return 0;
    }

    std::cout << "success" << std::endl;
    std::ofstream * f = NULL;
    if(!filename.empty())
      f = new std::ofstream(filename);
    show_bdd(bdd, x, npis, nregs, f);
    if(f) {
      f->close();
      delete f;
    }
    std::cout << "##### end iig #####" << std::endl << std::endl;
    return 1;
  }

  template <typename node> 
  int RIIG(mockturtle::aig_network & aig_, BddMan<node> & bdd, std::string initstr, std::string nzero, std::string filename, int seed, bool fastrnd) {
    int npis = aig_.num_pis();
    int nregs = aig_.num_registers();
    std::cout << "PI : " << npis << " , REG : " << nregs << std::endl;
    mockturtle::aig_network aig;
    CombNoPo(aig_, aig);
  
    node cis = bdd.Const1();
    for(int j = 0; j < npis+nregs; j++)
      cis = bdd.And(cis, bdd.IthVar(j));
  
    node init = bdd.Const1();
    for(int i = 0; i < nregs; i++) {
      if(initstr[i] == '0')
	init = bdd.And(init, bdd.Not(bdd.IthVar(npis+i)));
      else
	init = bdd.And(init, bdd.IthVar(npis+i));
    }
  
    std::vector<node> shift;
    for(int i = 0; i < npis+nregs; i++)
      shift.push_back(bdd.Const0());
    for(int i = 0; i < nregs; i++)
      shift.push_back(bdd.IthVar(npis+i));

    auto t1 = std::chrono::system_clock::now();
  
    std::cout << "init rnd func ";
    node x;
    if(fastrnd)
      x = initial_function_fast(bdd, init, nzero, npis, nregs, seed);
    else
      x = initial_function(bdd, init, nzero, npis, nregs, seed);
    t1 = show_time(t1);

    std::cout << "build latch   ";
    std::vector<node> vNodes = Aig2Bdd( aig, bdd );
    node tra = bdd.Const1();
    for(int i = 0; i < nregs; i++) 
      tra = bdd.And(tra, bdd.Ite(vNodes[i], bdd.IthVar(npis+nregs+i), bdd.Not(bdd.IthVar(npis+nregs+i))));
    for(int i = 0; i < npis; i++)
      vNodes.insert(vNodes.begin() + i, bdd.Const0());
    for(int i = 0; i < nregs; i++)
      vNodes.push_back(bdd.Const0());
    t1 = show_time(t1);
  
    std::cout << std::endl << "##### begin iig #####" << std::endl;
    auto start = t1;
    int itr = 0;
    while(1) {
      std::cout << "iteration " << itr++ << "   ";
      node y = bdd.VecCompose(x, vNodes);
      node z = bdd.Or(bdd.Not(x), y);
      if(z == bdd.Const1())
	break;
      node ns = bdd.And(tra, bdd.Not(z));
      ns = bdd.Exist(ns, cis);
      ns = bdd.VecCompose(ns, shift);
      x = bdd.Or(x, ns);
      if(x == bdd.Const1())
	break;
      t1 = show_time(t1);
    }
    t1 = show_time(t1);

    std::cout << "total ";
    show_time(start);
  
    if(x == bdd.Const1()) {
      std::cout << "fail ... function became const 1" << std::endl;
      std::cout << "##### end iig #####" << std::endl << std::endl;
      return 0;
    }
  
    std::cout << "success" << std::endl;
    std::ofstream * f = NULL;
    if(!filename.empty())
      f = new std::ofstream(filename);
    show_bdd(bdd, x, npis, nregs, f);
    if(f) {
      f->close();
      delete f;
    }
    std::cout << "##### end iig #####" << std::endl << std::endl;
    return 1;
  }

  template <typename node> 
  void IIGAND(mockturtle::aig_network & aig_, BddMan<node> & bdd, std::string initstr, std::string nzero, std::string filename, int seed, bool fastrnd, int numand) {
    int npis = aig_.num_pis();
    int nregs = aig_.num_registers();
    std::cout << "PI : " << npis << " , REG : " << nregs << std::endl;
    mockturtle::aig_network aig;
    CombNoPo(aig_, aig);

    node pis = bdd.Const1();
    for(int j = 0; j < npis; j++)
      pis = bdd.And(pis, bdd.IthVar(j));

    node init = bdd.Const1();
    for(int i = 0; i < nregs; i++) {
      if(initstr[i] == '0')
	init = bdd.And(init, bdd.Not(bdd.IthVar(npis+i)));
      else
	init = bdd.And(init, bdd.IthVar(npis+i));
    }

    node product = bdd.Const1();
    for(int j = 0; j < numand; j++) {
    while(1) {
    auto t1 = std::chrono::system_clock::now();
    
    std::cout << "seed is " << seed << std::endl;
    
    std::cout << "init rnd func ";
    node x;
    if(fastrnd)
      x = initial_function_fast(bdd, init, nzero, npis, nregs, seed);
    else
      x = initial_function(bdd, init, nzero, npis, nregs, seed);
    t1 = show_time(t1);

    std::cout << "build latch   ";
    std::vector<node> vNodes = Aig2Bdd( aig, bdd );
    for(int i = 0; i < npis; i++)
      vNodes.insert(vNodes.begin(), bdd.Const0());
    t1 = show_time(t1);

    std::cout << std::endl << "##### begin iig #####" << std::endl;
    auto start = t1;
    int itr = 0;
    while(1) {
      std::cout << "iteration " << itr++ << "   ";
      node y = bdd.VecCompose(x, vNodes);
      node z = bdd.Or(bdd.Not(x), y);
      if(z == bdd.Const1())
	break;
      node k = bdd.Univ(z, pis);
      x = bdd.And(x, k);
      if(bdd.And(x, init) == bdd.Const0()) {
	x = bdd.Const0();
	break;
      }
      t1 = show_time(t1);
    }
    t1 = show_time(t1);

    std::cout << "total ";
    show_time(start);
    seed++;
  
    if(x == bdd.Const0()) {
      std::cout << "fail ... function excluded initial state" << std::endl;
      std::cout << "##### end iig #####" << std::endl << std::endl;
      continue;
    }
    if(x == bdd.Const1()) {
      std::cout << "fail ... function is const 1" << std::endl;
      std::cout << "##### end iig #####" << std::endl << std::endl;
      continue;
    }
    std::cout << "success" << std::endl;
    std::cout << "compute AND ";
    product = bdd.And(product, x);
    t1 = show_time(t1);
    std::cout << "##### end iig #####" << std::endl << std::endl;
    break;
    }
    }

    std::ofstream * f = NULL;
    if(!filename.empty())
      f = new std::ofstream(filename);
    show_bdd(bdd, product, npis, nregs, f);
    if(f) {
      f->close();
      delete f;
    }
  }
}

