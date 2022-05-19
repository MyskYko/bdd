#pragma once

#include <algorithm>
#include <map>
#include <limits>
#include <chrono>

#include "NtkBdd.hpp"

namespace Bdd {
  template <typename node>
  class TransductionNetwork
  {
  public:
    bool fRemove = 0;
    bool fWeak = 0;
    bool fMspf = 0;
    int  nVerbose = 0;
  
  private:
    int nObjsAlloc;
    int Const0;
  
    std::vector<int> vPIs;
    std::vector<int> vPOs;
    std::vector<int> vObjs;
    std::vector<std::vector<int> > vvFIs;
    std::vector<std::vector<int> > vvFOs;
  
    std::vector<char> vMarks;

    BddMan<node> & bdd;
  
    std::vector<std::optional<node> > vFs;
    std::vector<std::optional<node> > vGs;
    std::vector<std::vector<node> > vvCs;


    void SortEnter( int id )
    {
      auto it = vObjs.end();
      for ( int id_ : vvFOs[id] )
	{
	  if ( vvFOs[id_].empty() )
	    {
	      continue;
	    }
	  auto it_ = std::find( vObjs.begin(), vObjs.end(), id_ );
	  if ( it_ < it )
	    {
	      it = it_;
	    }
	}
      it = vObjs.insert( it, id );
      for ( int id_ : vvFIs[id] )
	{
	  if ( vvFIs[id_].empty() )
	    {
	      continue;
	    }
	  auto it_ = std::find( vObjs.begin(), vObjs.end(), id_ );
	  if ( it_ > it )
	    {
	      vObjs.erase( it_ );
	      SortEnter( id_ );
	      it = std::find( vObjs.begin(), vObjs.end(), id );
	    }
	}
    }
    void Connect( int fanin, int fanout, bool fSort )
    { // uniqueness of conenction must be confirmed beforehand
      vvFIs[fanout].push_back( fanin );
      vvFOs[fanin].push_back( fanout );
      if ( fSort )
	{
	  if ( vvFOs[fanout].empty() || vvFIs[fanin].empty() )
	    {
	      return;
	    }
	  auto it_fanin = std::find( vObjs.begin(), vObjs.end(), fanin );
	  auto it_fanout = std::find( vObjs.begin(), vObjs.end(), fanout );
	  if ( it_fanout < it_fanin )
	    {
	      vObjs.erase( it_fanin );
	      SortEnter( fanin );
	    }
	}
    }
    void Remove( int id )
    {
      if ( nVerbose > 3 )
	{
	  std::cout << "\t\t\tRemove gate" << id << std::endl;
	}
      auto it = std::find( vObjs.begin(), vObjs.end(), id );
      if ( it == vObjs.end() )
	{
	  return;
	}
      vObjs.erase( it );
      for ( int id_ : vvFIs[id] )
	{
	  auto it = std::find( vvFOs[id_].begin(), vvFOs[id_].end(), id );
	  vvFOs[id_].erase( it );
	}
      for ( int id_ : vvFOs[id] )
	{
	  auto it = std::find( vvFIs[id_].begin(), vvFIs[id_].end(), id );
	  vvFIs[id_].erase( it );
	}
      if ( vFs[id] )
	{
	  vFs[id] = std::nullopt;
	}
      if ( vGs[id] )
	{
	  vGs[id] = std::nullopt;
	}
      vvCs[id].clear();
    }
    void Disconnect( int fanin, int fanout )
    {
      auto it_fanout = std::find( vvFOs[fanin].begin(), vvFOs[fanin].end(), fanout );
      vvFOs[fanin].erase( it_fanout );
      auto it_fanin = std::find( vvFIs[fanout].begin(), vvFIs[fanout].end(), fanin );
      vvFIs[fanout].erase( it_fanin );
    }

    void DescendantList_rec( std::vector<std::vector<int> > & children, std::vector<int> & list, int id )
    {
      for ( int id_ : children[id] )
	{
	  if ( !( vMarks[id_] >> 1 ) && !children[id_].empty() )
	    {
	      vMarks[id_] += 2;
	      list.push_back( id_ );
	      DescendantList_rec( children, list, id_ );
	    }
	}
    }
    void DescendantList( std::vector<std::vector<int> > & children, std::vector<int> & list, int id )
    {
      DescendantList_rec( children, list, id );
      for ( int id_ : list )
	{
	  vMarks[id_] -= 2;
	}
    }
    void SortList( std::vector<int> & list )
    {
      std::vector<int> list_new( list.size() );
      for ( int id : vObjs )
	{
	  auto it = std::find( list.begin(), list.end(), id );
	  if ( it != list.end() )
	    {
	      list_new.push_back( id );
	    }
	}
      list = list_new;
    }

    void MarkDescendant_rec( std::vector<std::vector<int> > & children, int id )
    {
      for ( int id_ : children[id] )
	{
	  if ( !vMarks[id_] && !children[id_].empty() )
	    {
	      vMarks[id_] = 1;
	      MarkDescendant_rec( children, id_ );
	    }
	}
    }
    void MarkClear()
    {
      vMarks.clear();
      vMarks.resize( nObjsAlloc );
    }
  

  public:
    TransductionNetwork( mockturtle::aig_network & aig_, BddMan<node> & bdd_ ) : bdd(bdd_)
    {
      mockturtle::topo_view aig{aig_};
      int nRegular = aig.size();
      nObjsAlloc = ( nRegular << 1 ) + aig.num_pos();
      vvFIs.resize( nObjsAlloc );
      vvFOs.resize( nObjsAlloc );
      vMarks.resize( nObjsAlloc );
      vFs.resize( nObjsAlloc );
      vGs.resize( nObjsAlloc );
      vvCs.resize( nObjsAlloc );
      // constant
      int id = aig.node_to_index( aig.get_node( aig.get_constant( 0 ) ) );
      Const0 = id;
      vFs[id] = bdd.Const0();
      int id_ = id + nRegular;
      Connect( id, id_, 0 );
      vObjs.push_back( id_ );
      // pi
      aig.foreach_pi( [&]( auto pi, int i )
		      {
			int id = aig.node_to_index( pi );
			vFs[id] = bdd.IthVar( i );
			vPIs.push_back( id );
			int id_ = id + nRegular;
			Connect( id, id_, 0 );
			vObjs.push_back( id_ );
		      });
      // gate
      aig.foreach_gate( [&]( auto gate )
			{
			  int id = aig.node_to_index( gate );
			  int idc = id + nRegular;
			  aig.foreach_fanin( gate, [&]( auto fanin )
						   {
						     int id_ = aig.node_to_index( aig.get_node( fanin ) );
						     if ( aig.is_complemented( fanin ) )
						       {
							 id_ += nRegular;
						       }
						     Connect( id_, idc, 0 );
						   });
			  vObjs.push_back( idc );
			  Connect( idc, id, 0 );
			  vObjs.push_back( id );
			});
      // po
      id = nRegular << 1;
      aig.foreach_po( [&]( auto po )
		      {
			int id_ = aig.node_to_index( aig.get_node( po ) );
			if ( aig.is_complemented( po ) )
			  {
			    id_ += nRegular;
			  }
			Connect( id_, id, 0 );
			vPOs.push_back( id );
			id++;
		      });
      // remove redundant nodes
      for ( int i = vObjs.size() - 1; i >= 0; i-- )
	{
	  int id = vObjs[i];
	  if ( vvFOs[id].empty() )
	    {
	      Remove( id );
	    }
	}
    }
    ~TransductionNetwork()
    {
      for ( int i = vObjs.size() - 1; i >= 0; i-- )
	{
	  Remove( vObjs[i] );
	}
    }

    void Aig( mockturtle::aig_network & aig )
    {
      std::map<int, mockturtle::aig_network::signal> m;
      for ( int id : vPIs )
	{
	  m[id] = aig.create_pi();
	}
      for ( int id : vObjs )
	{
	  if ( vvFOs[id].empty() )
	    {
	      continue;
	    }
	  if ( vvFIs[id].size() == 1 )
	    {
	      m[id] = aig.create_not( m[vvFIs[id][0]] );
	      continue;
	    }
	  int id0 = vvFIs[id][0];
	  int id1 = vvFIs[id][1];
	  m[id] = aig.create_and( m[id0], m[id1] );
	  for ( auto it = vvFIs[id].begin() + 2; it != vvFIs[id].end(); it++ )
	    {
	      m[id] = aig.create_and( m[id], m[*it] );
	    }
	  m[id] = aig.create_not( m[id] );
	}
      for ( int id : vPOs )
	{
	  aig.create_po( m[vvFIs[id][0]] );
	}
    }

    void BuildNode( int id, std::vector<std::optional<node> > & vFs_ )
    {
      if ( nVerbose > 3 )
	{
	  std::cout << "\t\t\tBuild gate" << id << std::endl;
	}
      vFs_[id] = bdd.Const1();
      for ( int id_ : vvFIs[id] )
	{
	  vFs_[id] = bdd.And( *vFs_[id], *vFs_[id_] );
	}
      vFs_[id] = bdd.Not( *vFs_[id] );
    }
    void Build()
    {
      if ( nVerbose > 2 )
	{
	  std::cout << "\t\tBuild" << std::endl;
	}
      for ( int id : vObjs )
	{
	  BuildNode( id, vFs );
	}
    }
  
    int Rank( int id )
    {
      if ( vvFIs[id].empty() )
	{
	  return std::numeric_limits<int>::max();
	}
      return vvFOs[id].size();
    }

    void SortFIsNode( int id )
    {
      std::sort( vvFIs[id].begin(), vvFIs[id].end(), [&]( int a, int b ) { return Rank( a ) < Rank( b ); } );
    }
    void SortFIs()
    {
      for ( int id : vObjs )
	{
	  SortFIsNode( id );
	}
    }

    void SetEXDC( std::vector<node> & vNodes )
    {
      for ( int i = 0; i < vPOs.size(); i++ )
	{
	  int id = vPOs[i];
          if ( vNodes.empty() )
            {
              vGs[id] = bdd.Const0();
            }
          else
            {
              vGs[id] = vNodes[i];
            }
	  for ( int j = 0; j < vvFIs[id].size(); j++ )
	    {
	      if ( vNodes.empty() )
		{
		  vvCs[id].push_back( bdd.Const0() );
		}
	      else
		{
		  vvCs[id].push_back( vNodes[i] );
		}
	    }
	}
    }

    void CalcG( int id )
    {
      if ( nVerbose > 3 )
	{
	  std::cout << "\t\t\tCspf propagate from gate" << id << std::endl;
	}
      vGs[id] = bdd.Const1();
      for (int id_ : vvFOs[id] )
	{
	  auto it = std::find( vvFIs[id_].begin(), vvFIs[id_].end(), id );
	  int index = std::distance( vvFIs[id_].begin(), it );
	  vGs[id] = bdd.And( *vGs[id], vvCs[id_][index] );
	}
    }
    void CalcC( int id )
    {
      if ( nVerbose > 3 )
	{
	  std::cout << "\t\t\tCspf at fanins of gate" << id << std::endl;
	}
      if ( fRemove && RemoveRedundantFIs( id ) )
	{
	  return;
	}
      vvCs[id].clear();
      for ( int i = 0; i < (int)vvFIs[id].size(); i++ )
	{
	  // x = and ( FIs with smaller rank )
	  node x = bdd.Const1();
	  for ( int j = i + 1; j < (int)vvFIs[id].size(); j++ )
	    {
	      x = bdd.And( x, *vFs[vvFIs[id][j]] );
	    }
	  // c = (not x) or (f[id] and f[idi]) or g[id]
	  x = bdd.Not( x );
	  node y = bdd.And( *vFs[id], *vFs[vvFIs[id][i]] );
	  x = bdd.Or( x, y );
	  x = bdd.Or( x, *vGs[id] );
	  // c or f[idi] == const1 -> redundant
	  y = bdd.Or( x, *vFs[vvFIs[id][i]] );
	  if ( y == bdd.Const1() )
	    {
	      Disconnect( vvFIs[id][i], id );
	      if ( vvFIs[id].empty() )
		{
		  for ( int id_ : vvFOs[id] )
		    {
		      if ( std::find( vvFIs[id_].begin(), vvFIs[id_].end(), Const0 ) == vvFIs[id_].end() )
			{
			  Connect( Const0, id_, 0 );
			}
		    }
		  Remove( id );
		  return;
		}
	      i--;
	      continue;
	    }
	  vvCs[id].push_back( x );
	}
    }
    void Cspf()
    {
      if ( nVerbose > 2 )
	{
	  std::cout << "\t\tCspf" << std::endl;
	}
      for ( int i = vObjs.size() - 1; i >= 0; i-- )
	{
	  int id = vObjs[i];
	  if ( vvFOs[id].empty() )
	    {
	      Remove( id );
	      continue;
	    }
	  CalcG( id );
	  CalcC( id );
	}
      Build();
    }

    bool RemoveRedundantFIs( int id )
    {
      if ( nVerbose > 3 )
	{
	  std::cout << "\t\t\tRemove redundant fanins of gate" << id << std::endl;
	}
      for ( int i = 0; i < (int)vvFIs[id].size(); i++ )
	{
	  node x = bdd.Const1();
	  for ( int j = 0; j < (int)vvFIs[id].size(); j++ )
	    {
	      if ( i == j )
		{
		  continue;
		}
	      x = bdd.And( x, *vFs[vvFIs[id][j]] );
	    }
	  x = bdd.Not( x );
	  x = bdd.Or( x, *vGs[id] );
	  x = bdd.Or( x, *vFs[vvFIs[id][i]] );
	  if ( x == bdd.Const1() )
	    {
	      Disconnect( vvFIs[id][i], id );
	      if ( vvFIs[id].empty() )
		{
		  for ( int id_ : vvFOs[id] )
		    {
		      if ( std::find( vvFIs[id_].begin(), vvFIs[id_].end(), Const0 ) == vvFIs[id_].end() )
			{
			  Connect( Const0, id_, 0 );
			}
		    }
		  Remove( id );
		  return 1;
		}
	      i--;
	      continue;
	    }
	}
      return 0;
    }
  
    bool IsFOConeShared_rec( int id, int stage )
    {
      if ( vvFOs[id].empty() )
	{
	  return 0;
	}
      int m = vMarks[id] >> 1;
      if ( m == stage )
	{
	  return 0;
	}
      if ( m )
	{
	  return 1;
	}
      vMarks[id] += stage << 1;
      for ( int id_ : vvFOs[id] )
	{
	  if ( IsFOConeShared_rec( id_, stage ) )
	    {
	      return 1;
	    }
	}
      return 0;
    }
    bool IsFOConeShared( int id )
    {
      bool r = 0;
      for ( int i = 0; i < (int)vvFOs[id].size(); i++ )
	{
	  if ( IsFOConeShared_rec( vvFOs[id][i], i + 1 ) )
	    {
	      r = 1;
	      break;
	    }
	}
      for ( int id_ : vObjs )
	{
	  if ( vMarks[id_] % 2 )
	    {
	      vMarks[id_] = 1;
	    }
	  else
	    {
	      vMarks[id_] = 0;
	    }
	}
      return r;
    }
    void BuildPOsInverted( int id, std::vector<node> & vInvFsPO )
    { 
      std::vector<int> targets;
      DescendantList( vvFOs, targets, id );
      SortList( targets );
      // insert inverters just after id    
      std::vector<std::optional<node> > vInvFs = vFs;
      vInvFs[id] = bdd.Not( *vInvFs[id] );
      // build
      for ( int id_ : targets )
	{
	  BuildNode( id_, vInvFs );
	}
      for ( int id_ : vPOs )
	{
	  vInvFsPO.push_back( *vInvFs[vvFIs[id_][0]] );
	}
    }
    void CalcGMspf( int id )
    {
      if ( !IsFOConeShared( id ) )
	{
	  if ( nVerbose > 3 )
	    {
	      std::cout << "\t\t\tMspf propagate from gate" << id << std::endl;
	    }
	  CalcG( id );
	  return;
	}
      if ( nVerbose > 3 )
	{
	  std::cout << "\t\t\tMspf DC miter at gate" << id << std::endl;
	}
      std::vector<node> vInvFsPO;
      BuildPOsInverted( id, vInvFsPO );
      vGs[id] = bdd.Const1();
      for ( int i = 0; i < (int)vPOs.size(); i++ )
	{
	  int id_ = vvFIs[vPOs[i]][0];
	  node x;
	  if ( id != id_ )
	    {
	      x = bdd.Const0();
	    }
	  else
	    {
	      x = bdd.Xor( *vFs[id_], vInvFsPO[i] );
	      x = bdd.Not( x );
	    }
	  x = bdd.Or( x, *vGs[vPOs[i]] );
	  vGs[id] = bdd.And( *vGs[id], x );
	}
    }
    bool CalcCMspf( int id )
    {
      if ( nVerbose > 3 )
	{
	  std::cout << "\t\t\tMspf at fanins of gate" << id << std::endl;
	}
      vvCs[id].clear();
      for ( int i = 0; i < (int)vvFIs[id].size(); i++ )
	{
	  node x = bdd.Const1();
	  for ( int j = 0; j < (int)vvFIs[id].size(); j++ )
	    {
	      if ( i == j )
		{
		  continue;
		}
	      x = bdd.And( x, *vFs[vvFIs[id][j]] );
	    }
	  x = bdd.Not( x );
	  x = bdd.Or( x, *vGs[id] );
	  node y = bdd.Or( x, *vFs[vvFIs[id][i]] );
	  if ( y == bdd.Const1() )
	    {
	      Disconnect( vvFIs[id][i], id );
	      if ( vvFIs[id].empty() )
		{
		  for ( int id_ : vvFOs[id] )
		    {
		      if ( std::find( vvFIs[id_].begin(), vvFIs[id_].end(), Const0 ) == vvFIs[id_].end() )
			{
			  Connect( Const0, id_, 0 );
			}
		    }
		  Remove( id );
		}
	      return 1;
	    }
	  vvCs[id].push_back( x );
	}
      return 0;
    }
    void Mspf()
    {
      if ( nVerbose > 2 )
	{
	  std::cout << "\t\tMspf" << std::endl;
	}
      for ( int i = vObjs.size() - 1; i >= 0; i-- )
	{
	  int id = vObjs[i];
	  if ( vvFOs[id].empty() )
	    {
	      Remove( id );
	      continue;
	    }
	  CalcGMspf( id );
	  if ( CalcCMspf( id ) )
	    {
	      Build();
	      i = vObjs.size();
	    }
	}
    }

    bool TryConnect( int fanin, int fanout )
    {
      if ( std::find( vvFIs[fanout].begin(), vvFIs[fanout].end(), fanin ) != vvFIs[fanout].end() )
	{
	  return 0;
	}
      node x = bdd.Or( *vFs[fanout], *vGs[fanout] );
      x = bdd.Or( x, *vFs[fanin] );
      if ( x == bdd.Const1() )
	{
	  Connect( fanin, fanout, 1 );
	  return 1;
	}
      return 0;
    }
    void CspfFICone( int id )
    {
      if ( nVerbose > 2 )
	{
	  std::cout << "\t\tCspf fanin cone of gate" << id << std::endl;
	}
      CalcC( id );
      std::vector<int> targets;
      DescendantList( vvFIs, targets, id );
      SortList( targets );
      for ( int i = targets.size() - 1; i >= 0; i-- )
	{
	  int id_ = targets[i];
	  if ( vvFOs[id_].empty() )
	    {
	      Remove( id_ );
	      continue;
	    }
	  CalcG( id_ );
	  CalcC( id_ );
	}
      Build();
    }
    void BuildFOCone( int id )
    {
      if ( nVerbose > 2 )
	{
	  std::cout << "\t\tBuild fanout cone of gate" << id << std::endl;
	}
      BuildNode( id, vFs );
      std::vector<int> targets;
      DescendantList( vvFOs, targets, id );
      SortList( targets );
      for ( int id_ : targets )
	{
	  BuildNode( id_, vFs );
	}
    }
    void G1Eager( int fanin, int fanout )
    {
      if ( nVerbose > 2 )
	{
	  std::cout << "\tEager reduce with a connection from gate" << fanin << " to gate" << fanout << std::endl;
	}
      int wire = CountWire();
      CspfFICone( fanout );
      if ( wire == CountWire() )
	{
	  Disconnect( fanin, fanout );
	  CspfFICone( fanout );
	  return;
	}
      Cspf();
    }
    void G1Weak( int fanin, int fanout )
    {
      if ( nVerbose > 2 )
	{
	  std::cout << "\tWeak reduce with a connection from gate" << fanin << " to gate" << fanout << std::endl;
	}
      (void)fanin;
      RemoveRedundantFIs( fanout );
    }
    void G1Mspf( int fanin, int fanout )
    {
      if ( nVerbose > 2 )
	{
	  std::cout << "\tMspf reduce with a connection from gate" << fanin << " to gate" << fanout << std::endl;
	}
      (void)fanin;
      BuildFOCone( fanout );
      Mspf();
    }
    void G1()
    {
      if ( nVerbose )
	{
	  std::cout << "Transduction method adding new wires" << std::endl;
	}
      std::vector<int> targets = vObjs;
      for ( int i = targets.size() - 1; i >= 0; i-- )
	{
	  int id = targets[i];
	  if ( nVerbose )
	    {
	      std::cout << "gate" << id << " " << targets.size() - i << " / " << targets.size() << std::endl;
	    }
	  if ( vvFOs[id].empty() )
	    {
	      continue;
	    }
	  MarkClear();
	  vMarks[id] = 1;
	  MarkDescendant_rec( vvFOs, id );
	  // try connecting PI
	  for ( int id_ : vPIs )
	    {
	      if ( nVerbose > 1 )
		{
		  std::cout << "\tconnect pi" << id_ << std::endl;
		}
	      if ( vvFOs[id].empty() )
		{
		  break;
		}
	      if ( TryConnect( id_, id ) )
		{
		  if ( fWeak )
		    {
		      G1Weak( id_, id );
		    }
		  else if ( fMspf )
		    {
		      G1Mspf( id_, id );
		    }
		  else
		    {
		      G1Eager( id_, id );
		    }
		}
	    }
	  // try connecting gate
	  for ( int id_ : targets )
	    {
	      if ( nVerbose > 1 )
		{
		  std::cout << "\tconnect gate" << id_ << std::endl;
		}
	      if ( vvFOs[id].empty() )
		{
		  break;
		}
	      if ( vvFOs[id_].empty() || vMarks[id_] )
		{
		  continue;
		}
	      if ( TryConnect( id_, id ) )
		{
		  if ( fWeak )
		    {
		      G1Weak( id_, id );
		    }
		  else if ( fMspf )
		    {
		      G1Mspf( id_, id );
		    }
		  else
		    {
		      G1Eager( id_, id );
		    }
		}
	    }
	  // recalculate for weak
	  if ( fWeak )
	    {
	      CspfFICone( id );
	    }
	}
    }
    
    
    int CountGate()
    {
      return vObjs.size();
    }
    int CountWire()
    {
      int count = 0;
      for ( int id : vObjs )
	{
	  count += vvFIs[id].size();
	}
      return count;
    }
    void PrintStats( std::string prefix, std::chrono::system_clock::time_point & start )
    {
      double time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-start).count() / 1000.0;
      std::cout << prefix << " gate " << CountGate() << ", wire " << CountWire() << ", node " << CountWire() - CountGate() << ", time " << time << std::endl;
    
    }
  };

  template <typename node>
  void Transduction( mockturtle::aig_network & aig, BddMan<node> & bdd, bool fReo = 0, bool fRepeat = 0, bool fMspf = 0, bool fCheck = 0, int nVerbose = 0, mockturtle::aig_network * dcaig = NULL )
  {
    auto start = std::chrono::system_clock::now();
  
    auto net = TransductionNetwork( aig, bdd );
    net.nVerbose = nVerbose ? nVerbose - 1 : 0;
    if ( nVerbose )
      {
	net.PrintStats( "init", start );
      }
  
    if ( fReo )
      {
	bdd.Dvr();
      }

    if ( nVerbose > 1 )
      {
	std::cout << "Build entire circuit" << std::endl;
      }
    net.Build();

    std::vector<node> vDCs;
    if ( dcaig )
      {
	if ( nVerbose > 1 )
	  {
	    std::cout << "Build External DC" << std::endl;
	  }
	vDCs = Aig2Bdd( *dcaig, bdd, nVerbose > 3 );
      }
    net.SetEXDC( vDCs );

    if ( fReo )
      {
	bdd.Reorder();
	bdd.DvrOff();
      }

    if ( nVerbose )
      {
	net.PrintStats( "prep", start );
      }
  
    if ( fRepeat )
      {
	if ( nVerbose > 1 )
	  {
	    std::cout << "Apply Transduction repeatedly" << std::endl;
	  }
	while ( 1 )
	  {
	    if ( nVerbose > 1 )
	      {
		std::cout << "Apply Transduction with Weak reduce repeatedly" << std::endl;
	      }
	    int wire2 = net.CountWire();
	    net.fWeak = 1;
	    while ( 1 )
	      {
		int wire = 0;
		while ( wire != net.CountWire() )
		  {
		    wire = net.CountWire();
		    net.SortFIs();
		    net.Cspf();
		  } 
		net.G1();
		if ( nVerbose )
		  {
		    net.PrintStats( "weak", start );
		  }
		if ( wire == net.CountWire() )
		  {
		    break;
		  }
	      }
	    net.fWeak = 0;
	    if ( nVerbose > 1 )
	      {
		std::cout << "Apply Transduction with Eager reduce repeatedly" << std::endl;
	      }
	    while ( 1 )
	      {
		int wire = 0;
		while ( wire != net.CountWire() )
		  {
		    wire = net.CountWire();
		    net.SortFIs();
		    net.Cspf();
		  }
		net.G1();
		if ( nVerbose )
		  {
		    net.PrintStats( "cspf", start );
		  }
		if ( wire == net.CountWire() )
		  {
		    break;
		  }
	      }
	    if ( wire2 == net.CountWire() )
	      {
		break;
	      }
	  }
      }

    if ( fMspf )
      {
	if ( nVerbose > 1 )
	  {
	    std::cout << "Apply Transduction with Mspf reduce";
	    if ( fRepeat )
	      {
		std::cout << " repeatedly" << std::endl;
	      }
	    else
	      {
		std::cout << " once" << std::endl;
	      }
	  }
	net.fMspf = 1;
	while ( 1 )
	  {
	    int wire = net.CountWire();
	    net.SortFIs();
	    net.Mspf();
	    net.G1();
	    if ( nVerbose )
	      {
		net.PrintStats( "mspf", start );
	      }
	    if ( !fRepeat || wire == net.CountWire() )
	      {
		break;
	      }
	  }
      }

    if ( !fRepeat && !fMspf )
      {
	if ( nVerbose > 1 )
	  {
	    std::cout << "Apply Cspf and Transduction once" << std::endl;
	  }
	net.SortFIs();
	net.Cspf();
	net.G1();
      }

    if ( nVerbose )
      {
	net.PrintStats( "end ", start );
      }
  
    mockturtle::aig_network aig_new;
    net.Aig( aig_new );

    if ( fCheck )
      {
	if ( nVerbose > 1 )
	  {
	    std::cout << "Check equivalence" << std::endl;
	  }
	mockturtle::aig_network miter;
	if ( !dcaig )
	  {
	    miter = *mockturtle::miter<mockturtle::aig_network>( aig, aig_new );
	  }
	else
	  {
	    std::vector<mockturtle::aig_network::signal> pis;
	    for ( int i = 0; i < aig.num_pis(); i++ )
	      {
		pis.push_back( miter.create_pi() );
	      }
	    auto pos1 = cleanup_dangling( aig, miter, pis.begin(), pis.end() );
	    auto pos2 = cleanup_dangling( aig_new, miter, pis.begin(), pis.end() );
	    auto posdc = cleanup_dangling( *dcaig, miter, pis.begin(), pis.end() );	  
	    std::vector<mockturtle::aig_network::signal> xors;
	    std::transform( pos1.begin(), pos1.end(), pos2.begin(), std::back_inserter( xors ), [&]( auto const & o1, auto const & o2 ) { return miter.create_xor( o1, o2 ); } );
	    std::vector<mockturtle::aig_network::signal> pos;
	    std::transform( xors.begin(), xors.end(), posdc.begin(), std::back_inserter( pos ), [&]( auto const & o1, auto const & o2 ) { return miter.create_and( o1, miter.create_not( o2 ) ); } );
	    miter.create_po( miter.create_nary_or( pos ) );
	  }
	auto result = mockturtle::equivalence_checking( miter );
	if ( result && *result )
	  {
	    std::cout << "networks are equivalent\n";
	  }
	else if ( result )
	  {
	    std::cout << "networks are NOT equivalent\n";
	  }
	else
	  {
	    std::cout << "equivalence checking unfinished\n";
	  }
      }
  
    aig = aig_new;
  }
}
