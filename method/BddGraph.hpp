#pragma once

#include "BddMan.hpp"
#include "mockturtle/mockturtle.hpp"

template <typename node>
uint32_t Bdd2Dot_rec( std::ofstream & f, Bdd::BddMan<node> & bdd, node x, std::map<uint64_t, uint32_t> & m, std::vector<std::vector<int> > & levels, int & numid ) {
  if ( x == bdd.Const0() ) {
    return 0;
  }
  if ( x == bdd.Const1() ) {
    return 1;
  }
  if ( m.count( bdd.Id( x ) ) ) {
    return m[bdd.Id( x )];
  }
  uint32_t t = Bdd2Dot_rec( f, bdd, bdd.Then( x ), m, levels, numid );
  uint32_t e = Bdd2Dot_rec( f, bdd, bdd.Else( x ), m, levels, numid );
  levels[bdd.Var( x )].push_back( numid );
  f << "n" << numid << ";" << std::endl;
  f << "n" << numid << " -> n" << t << ";" << std::endl;
  f << "n" << numid << " -> n" << e << " [style = dashed];" << std::endl;
  m[bdd.Id( x )] = numid;
  return numid++;
}

template <typename node>
uint32_t Bdd2DotCedge_rec( std::ofstream & f, Bdd::BddMan<node> & bdd, node x, std::map<uint64_t, uint32_t> & m, std::vector<std::vector<int> > & levels, int & numid ) {
  x = bdd.Regular( x );
  if ( x == bdd.Const0() || x == bdd.Const1() ) {
    return 0;
  }
  if ( m.count( bdd.Id( x ) ) ) {
    return m[bdd.Id( x )];
  }
  uint32_t t = Bdd2DotCedge_rec( f, bdd, bdd.Then( x ), m, levels, numid );
  uint32_t e = Bdd2DotCedge_rec( f, bdd, bdd.Else( x ), m, levels, numid );
  levels[bdd.Var( x )].push_back( numid );
  f << "n" << numid << ";" << std::endl;
  f << "n" << numid << " -> n" << t;
  auto y = bdd.Then( x );
  if ( bdd.IsCompl( y ) ) {
    f << " [style = dotted]";
  }
  f << ";" << std::endl;
  f << "n" << numid << " -> n" << e;
  y = bdd.Else( x );
  if ( bdd.IsCompl( y ) ) {
    f << " [style = dotted]";
  }
  else {
    f << " [style = dashed]";
  }
  f << ";" << std::endl;
  m[bdd.Id( x )] = numid;
  return numid++;
}

template <typename node>
void Bdd2Dot( std::string dotname, Bdd::BddMan<node> & bdd, std::vector<node> & vNodes, std::vector<std::string> & pi_names, std::vector<std::string> & po_names, bool cedge ) {
  std::ofstream f(dotname);
  f << "digraph bdd {" << std::endl;
  for ( int i = 0; i < bdd.GetNumVar(); i++ ) {
    f << "pi" << i << " [shape = box";
    if ( !pi_names.empty() && !pi_names[i].empty() ) {
      f << ", label = \"" << pi_names[i] << "\"";
    }
    f << "];" << std::endl;
  }
  for ( int i = 0; i < vNodes.size(); i++ ) {
    f << "po" << i << " [shape = box";
    if ( !po_names.empty() && !po_names[i].empty() ) {
      f << ", label = \"" << po_names[i] << "\"";
    }
    f << "];" << std::endl;
  }
  std::map<uint64_t, uint32_t> m;
  std::vector<std::vector<int> > levels( bdd.GetNumVar() + 1 );
  if ( cedge ) {
    int root;
    if ( bdd.Regular( bdd.Const0() ) == bdd.Const1() ) {
      root = 1;
    }
    else if ( bdd.Regular( bdd.Const1() ) == bdd.Const0() ) {
      root = 0;
    }
    else {
      throw "complemented edge not used";
    }
    f << "n0 [label = \"" << root << "\"];" << std::endl;
    levels[bdd.GetNumVar()].push_back(0);
    int numid = 1;
    for ( int i = 0; i < vNodes.size(); i++ ) {
      uint32_t po = Bdd2DotCedge_rec( f, bdd, vNodes[i], m, levels, numid );
      f << "po" << i << " -> n" << po;
      if ( bdd.IsCompl( vNodes[i] ) ) {
        f << " [style = \"dotted\"]";
      }
      f << ";" << std::endl;
    }
  }
  else {
    f << "n0 [label = \"0\"];" << std::endl;
    f << "n1 [label = \"1\"];" << std::endl;
    levels[bdd.GetNumVar()].push_back(0);
    levels[bdd.GetNumVar()].push_back(1);
    int numid = 2;
    for ( int i = 0; i < vNodes.size(); i++ ) {
      uint32_t po = Bdd2Dot_rec( f, bdd, vNodes[i], m, levels, numid );
      f << "po" << i << " -> n" << po << ";" << std::endl;
    }
  }
  f << "top [style = \"invis\"];" << std::endl;
  f << "{rank = source;top;";
  for ( int i = 0; i < vNodes.size(); i++ ) {
    f << "po" << i << ";";
  }
  f << "}" << std::endl;
  for ( int i = 0; i < bdd.GetNumVar(); i++ ) {
    f << "{rank = same;pi" << i << ";";
    for ( int j : levels[i] ) {
      f << "n" << j << ";";
    }
    f << "}" << std::endl;
  }
  f << "bottom [style = \"invis\"];" << std::endl;
  f << "{rank = sink;bottom;";
  for ( int j : levels[bdd.GetNumVar()] ) {
    f << "n" << j << ";";
  }
  f << "}" << std::endl;
  {
    std::vector<int> v( bdd.GetNumVar() );
    for ( int i = 0; i < bdd.GetNumVar(); i++ ) {
      v[bdd.Level( i )] = i;
    }
    f << "top -> ";
    for ( int i : v ) {
      f << "pi" << i << " -> ";
    }
    f << "bottom [style = \"invis\"];" << std::endl;
  }  
  f << "}" << std::endl;
}
