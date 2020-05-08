#ifndef IND_INV_HPP_
#define IND_INV_HPP_

#include "NtkBdd.hpp"

void CopyAigRec(mockturtle::aig_network & aig, mockturtle::aig_network & aig2, std::map<mockturtle::aig_network::signal, mockturtle::aig_network::signal> & m, mockturtle::aig_network::node const & x) {
  if(aig.is_ci(x))
    return;
  std::vector<mockturtle::aig_network::signal> fanin;
  aig.foreach_fanin(x, [&](auto fi) {
			 CopyAigRec(aig, aig2, m, aig.get_node(fi));
			 if(aig.is_complemented(fi))
			   fanin.push_back(m[aig.create_not(fi)]);
			 else 
			   fanin.push_back(m[fi]);
		       });
  assert(fanin.size() == 2);
  m[aig.make_signal(x)] = aig2.create_and(fanin[0], fanin[1]);
}

void CombNoPo(mockturtle::aig_network & aig, mockturtle::aig_network & aig2) {
  std::map<mockturtle::aig_network::signal, mockturtle::aig_network::signal> m;
  aig.foreach_pi([&](auto pi) {
		   m[aig.make_signal(pi)] = aig2.create_pi();
		 });
  aig.foreach_register([&](auto reg) {
			 m[aig.make_signal(reg.second)] = aig2.create_pi();
		       });
  aig.foreach_register([&](auto reg) {
			 CopyAigRec(aig, aig2, m, aig.get_node(reg.first));
			 if(aig.is_complemented(reg.first))
			   aig2.create_po(m[aig.create_not(reg.first)]);
			 else
			   aig2.create_po(m[reg.first]);
		       });
}

template <typename node>
void show_bdd_rec(Bdd::BddMan<node> & bdd, node x, int npis, std::string & s) {
  if(x == bdd.Const1())
    {
      std::cout << s << std::endl;
      return;
    }
  if(x == bdd.Const0())
    return;
  s[bdd.Var(x) - npis] = '1';
  show_bdd_rec(bdd, bdd.Then(x), npis, s);
  s[bdd.Var(x) - npis] = '0';
  show_bdd_rec(bdd, bdd.Else(x), npis, s);
  s[bdd.Var(x) - npis] = '-';
}

template <typename node> 
void show_bdd(Bdd::BddMan<node> & bdd, node & x, int npis, int nregs) {
  std::string s;
  for(int i = 0; i < nregs; i++) {
    s += "-";
  }
  show_bdd_rec(bdd, x, npis, s);
}

template <typename node> 
void IIG(mockturtle::aig_network & aig_, Bdd::BddMan<node> & bdd, std::string initstr, int nzero) {
  int npis = aig_.num_pis();
  int nregs = aig_.num_registers();
  mockturtle::aig_network aig;
  CombNoPo(aig_, aig);

  node pis = bdd.Const1();
  for(int j = 0; j < npis; j++) {
    pis = bdd.And(pis, bdd.IthVar(j));
  }

  node init = bdd.Const1();
  for(int i = 0; i < nregs; i++) {
    if(initstr[i] == '0')
      init = bdd.And(init, bdd.Not(bdd.IthVar(npis+i)));
    else
      init = bdd.And(init, bdd.IthVar(npis+i));
  }
  
  std::cout << "initialize function at random" << std::endl;
  node x = bdd.Const1();
  std::random_device rnd;
  for(int i = 0; i < nzero; i++) {
    node y = bdd.Const1();
    for(int j = 0; j < nregs; j++) {
      if((int)rnd() > 0)
	y = bdd.And(y, bdd.IthVar(npis+j));
      else
	y = bdd.And(y, bdd.Not(bdd.IthVar(npis+j)));
    }
    x = bdd.And(x, bdd.Not(y));
  }
  x = bdd.Or(x, init);

  std::cout << "build latch" << std::endl;
  std::vector<node> vNodes = Aig2Bdd( aig, bdd );
  for(int i = 0; i < npis; i++) {
    vNodes.insert(vNodes.begin() + i, bdd.IthVar(i));
  }

  int itr = 0;
  while(1) {
    std::cout << "itr " << itr++ << std::endl;
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
  }
  
  if(x == bdd.Const0())
    std::cout << "fail ... function excluded initial state" << std::endl;
  else {
    std::cout << "success" << std::endl;
    show_bdd(bdd, x, npis, nregs);
  }
}

template <typename node> 
void RIIG(mockturtle::aig_network & aig_, Bdd::BddMan<node> & bdd, std::string initstr, int nzero) {
  int npis = aig_.num_pis();
  int nregs = aig_.num_registers();
  mockturtle::aig_network aig;
  CombNoPo(aig_, aig);
  
  node cis = bdd.Const1();
  for(int j = 0; j < npis+nregs; j++) {
    cis = bdd.And(cis, bdd.IthVar(j));
  }
  
  node init = bdd.Const1();
  for(int i = 0; i < nregs; i++) {
    if(initstr[i] == '0')
      init = bdd.And(init, bdd.Not(bdd.IthVar(npis+i)));
    else
      init = bdd.And(init, bdd.IthVar(npis+i));
  }
  
  std::cout << "initialize function at random" << std::endl;
  node x = bdd.Const1();
  std::random_device rnd;
  for(int i = 0; i < nzero; i++) {
    node y = bdd.Const1();
    for(int j = 0; j < nregs; j++) {
      if((int)rnd() > 0)
	y = bdd.And(y, bdd.IthVar(npis+j));
      else
	y = bdd.And(y, bdd.Not(bdd.IthVar(npis+j)));
    }
    x = bdd.And(x, bdd.Not(y));
  }
  x = bdd.Or(x, init);

  std::cout << "build latch" << std::endl;
  std::vector<node> vNodes = Aig2Bdd( aig, bdd );
  
  node tra = bdd.Const0();
  for(int i = 0; i < nregs; i++) {
    tra = bdd.Or(tra, bdd.Ite(vNodes[i], bdd.IthVar(npis+nregs+i), bdd.Not(bdd.IthVar(npis+nregs+i))));
  }
  
  for(int i = 0; i < npis; i++) {
    vNodes.insert(vNodes.begin() + i, bdd.IthVar(i));
  }
  for(int i = 0; i < nregs; i++) {
    vNodes.push_back(bdd.IthVar(npis+nregs+i));
  }

  std::vector<node> shift;
  for(int i = 0; i < npis+nregs; i++) {
    shift.push_back(bdd.IthVar(i));
  }
  for(int i = 0; i < nregs; i++) {
    shift.push_back(bdd.IthVar(npis+i));
  }

  int itr = 0;
  while(1) {
    std::cout << "itr " << itr++ << std::endl;
    node y = bdd.VecCompose(x, vNodes);
    node z = bdd.Or(bdd.Not(x), y);
    if(z == bdd.Const1())
      break;
    node ns = bdd.And(tra, bdd.Not(z));
    ns = bdd.Exist(ns, cis);
    ns = bdd.VecCompose(ns, shift);
    x = bdd.Or(x, ns);
    if(x == bdd.Const1()) {
      break;
    }
  }
  
  if(x == bdd.Const1())
    std::cout << "fail ... function became const 1" << std::endl;
  else {
    std::cout << "success" << std::endl;
    show_bdd(bdd, x, npis, nregs);
  }
}
#endif
