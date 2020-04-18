#ifndef TRANSDUCTION_HPP_
#define TRANSDUCTION_HPP_

#include <algorithm>
#include <map>
#include <limits>
#include <optional>
#include <BddMan.hpp>
#include <mockturtle/mockturtle.hpp>

template <typename node>
class TransductionNetwork
{
public:
  bool fRemove = 0;
  bool fWeak = 0;
  bool fMspf = 0;
  bool fReo = 1;

private:
  int nObjsAlloc;
  int Const0;
  
  std::vector<int> vPIs;
  std::vector<int> vPOs;
  std::vector<int> vObjs;
  std::vector<std::vector<int> > vvFIs;
  std::vector<std::vector<int> > vvFOs;
  
  std::vector<int> vRanks;
  std::vector<char> vMarks;

  Bdd::BddMan<node> & bdd;
  
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
	bdd.Deref( *vFs[id] );
	vFs[id] = std::nullopt;
      }
    if ( vGs[id] )
      {
	bdd.Deref( *vGs[id] );
	vGs[id] = std::nullopt;
      }
    for ( auto & x : vvCs[id] )
      {
	bdd.Deref( x );
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
  TransductionNetwork( mockturtle::aig_network & aig_, Bdd::BddMan<node> & bdd_ ) : bdd(bdd_)
  {
    mockturtle::topo_view aig{aig_};
    int nRegular = aig.size();
    nObjsAlloc = ( nRegular << 1 ) + aig.num_pos();
    vvFIs.resize( nObjsAlloc );
    vvFOs.resize( nObjsAlloc );
    vRanks.resize( nObjsAlloc );
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
    if ( vFs_[id] )
      {
	bdd.Deref( *vFs_[id] );
      }
    vFs_[id] = bdd.Const1();
    bdd.Ref( *vFs_[id] );
    for ( int id_ : vvFIs[id] )
      {
	node x = bdd.And( *vFs_[id], *vFs_[id_] );
	bdd.Ref( x );
	bdd.Deref( *vFs_[id] );
	vFs_[id] = x;
      }
    node x = bdd.Not( *vFs_[id] );
    bdd.RefNot( x );
    bdd.DerefNot( *vFs_[id] );
    vFs_[id] = x;
  }
  void Build()
  {
    for ( int id : vObjs )
      {
	BuildNode( id, vFs );
      }
  }
  
  void Rank()
  {
    for ( int id : vPIs )
      {
	vRanks[id] = std::numeric_limits<int>::max();
      }
    for ( int id : vObjs )
      {
	vRanks[id] = vvFOs[id].size();
      }
  }

  void SortFIsNode( int id )
  {
    std::sort( vvFIs[id].begin(), vvFIs[id].end(), [&]( int a, int b ) { return vRanks[a] < vRanks[b]; } );
  }
  void SortFIs()
  {
    for ( int id : vObjs )
      {
	SortFIsNode( id );
      }
  }

  void SetEXDC()
  {
    for ( int id : vPOs )
      {
	for ( int id_ : vvFIs[id] )
	  {
	    (void)id_;
	    vvCs[id].push_back( bdd.Const0() );
	  }
      }
  }

  void CalcG( int id )
  {
    if ( vGs[id] )
      {
	bdd.Deref( *vGs[id] );
      }
    vGs[id] = bdd.Const1();
    bdd.Ref( *vGs[id] );
    for (int id_ : vvFOs[id] )
      {
	auto it = std::find( vvFIs[id_].begin(), vvFIs[id_].end(), id );
	int index = std::distance( vvFIs[id_].begin(), it );
	node x = bdd.And( *vGs[id], vvCs[id_][index] );
	bdd.Ref( x );
	bdd.Deref( *vGs[id] );
	vGs[id] = x;
      }
  }
  void CalcC( int id )
  {
    if ( fRemove && RemoveRedundantFIs( id ) )
      {
	return;
      }
    for ( node & x : vvCs[id] )
      {
	bdd.Deref( x );
      }
    vvCs[id].clear();
    for ( int i = 0; i < (int)vvFIs[id].size(); i++ )
      {
	// x = and ( FIs with smaller rank )
	node x = bdd.Const1();
	bdd.Ref( x );
	for ( int j = i + 1; j < (int)vvFIs[id].size(); j++ )
	  {
	    node y = bdd.And( x, *vFs[vvFIs[id][j]] );
	    bdd.Ref( y );
	    bdd.Deref( x );
	    x = y;
	  }
	// c = (not x) or (f[id] and f[idi]) or g[id]
	node y = bdd.Not( x );
	bdd.RefNot( y );
	bdd.DerefNot( x );
	x = y;
	y = bdd.And( *vFs[id], *vFs[vvFIs[id][i]] );
	bdd.Ref( y );
	node z = bdd.Or( x, y );
	bdd.Ref( z );
	bdd.Deref( x );
	bdd.Deref( y );
	x = z;
	y = bdd.Or( x, *vGs[id] );
	bdd.Ref( y );
	bdd.Deref( x );
	x = y;
	// c or f[idi] == const1 -> redundant
	y = bdd.Or( x, *vFs[vvFIs[id][i]] );
	bdd.Ref( y );
	if ( y == bdd.Const1() )
	  {
	    bdd.Deref( x );
	    bdd.Deref( y );
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
	bdd.Deref( y );
	vvCs[id].push_back( x );
      }
  }
  void Cspf()
  {
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
    for ( int i = 0; i < (int)vvFIs[id].size(); i++ )
      {
	node x = bdd.Const1();
	bdd.Ref( x );
	for ( int j = 0; j < (int)vvFIs[id].size(); j++ )
	  {
	    if ( i == j )
	      {
		continue;
	      }
	    node y = bdd.And( x, *vFs[vvFIs[id][j]] );
	    bdd.Ref( y );
	    bdd.Deref( x );
	    x = y;
	  }
	node y = bdd.Not( x );
	bdd.RefNot( y );
	bdd.DerefNot( x );
	x = y;
	y = bdd.Or( x, *vGs[id] );
	bdd.Ref( y );
	bdd.Deref( x );
	x = y;
	y = bdd.Or( x, *vFs[vvFIs[id][i]] );
	bdd.Ref( y );
	bdd.Deref( x );
	if ( y == bdd.Const1() )
	  {
	    bdd.Deref( y );
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
	bdd.Deref( y );
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
    bdd.RefNot( *vInvFs[id] );
    // build
    for ( int id_ : targets )
      {
	vInvFs[id_] = std::nullopt;
	BuildNode( id_, vInvFs );
      }
    for ( int id_ : vPOs )
      {
	node x = *vInvFs[vvFIs[id_][0]];
	bdd.Ref( x );
	vInvFsPO.push_back( x );
      }
    for ( int id_ : targets )
      {
	bdd.Deref( *vInvFs[id_] );
      }    
  }
  void CalcGMspf( int id )
  {
    if ( !IsFOConeShared( id ) )
      {
	CalcG( id );
	return;
      }
    std::vector<node> vInvFsPO;
    BuildPOsInverted( id, vInvFsPO );
    if ( vGs[id] )
      {
	bdd.Deref( *vGs[id] );
      }
    vGs[id] = bdd.Const1();
    bdd.Ref( *vGs[id] );
    for ( int i = 0; i < (int)vPOs.size(); i++ )
      {
	int id_ = vvFIs[vPOs[i]][0];
	node x = bdd.Xor( *vFs[id_], vInvFsPO[i] );
	bdd.Ref( x );
	if ( id != id_ )
	  {
	    node y = bdd.Not( x );
	    bdd.RefNot( y );
	    bdd.DerefNot( x );
	    x = y;
	  }
	node y = bdd.Or( x, *vGs[vPOs[i]] );
	bdd.Ref( y );
	bdd.Deref( x );
	x = y;
	y = bdd.And( *vGs[id], x );
	bdd.Ref( y );
	bdd.Deref( *vGs[id] );
	bdd.Deref( x );
	vGs[id] = y;
      }
    for ( node & x : vInvFsPO )
      {
	bdd.Deref( x );
      }
  }
  bool CalcCMspf( int id )
  {
    for ( node & x : vvCs[id] )
      {
	bdd.Deref( x );
      }
    vvCs[id].clear();
    for ( int i = 0; i < (int)vvFIs[id].size(); i++ )
      {
	node x = bdd.Const1();
	bdd.Ref( x );
	for ( int j = 0; j < (int)vvFIs[id].size(); j++ )
	  {
	    if ( i == j )
	      {
		continue;
	      }
	    node y = bdd.And( x, *vFs[vvFIs[id][j]] );
	    bdd.Ref( y );
	    bdd.Deref( x );
	    x = y;
	  }
	node y = bdd.Not( x );
	bdd.RefNot( y );
	bdd.DerefNot( x );
	x = y;
	y = bdd.Or( x, *vGs[id] );
	bdd.Ref( y );
	bdd.Deref( x );
	x = y;
	y = bdd.Or( x, *vFs[vvFIs[id][i]] );
	bdd.Ref( y );
	if ( y == bdd.Const1() )
	  {
	    bdd.Deref( x );
	    bdd.Deref( y );
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
	bdd.Deref( y );
	vvCs[id].push_back( x );
      }
    return 0;
  }
  void Mspf()
  {
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
    bdd.Ref( x );
    node y = bdd.Or( x, *vFs[fanin] );
    bdd.Ref( y );
    bdd.Deref( x );
    if ( y == bdd.Const1() )
      {
	bdd.Deref( y );
	Connect( fanin, fanout, 1 );
	return 1;
      }
    bdd.Deref( y );
    return 0;
  }
  void CspfFICone( int id )
  {
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
    int wire = CountWire();
    CspfFICone( fanout );
    if ( wire == CountWire() )
      {
	Disconnect( fanin, fanout );
	CspfFICone( fanout );
	return;
      }
    Build();
    Cspf();
  }
  void G1Weak( int fanin, int fanout )
  {
    (void)fanin;
    RemoveRedundantFIs( fanout );
  }
  void G1Mspf( int fanin, int fanout )
  {
    (void)fanin;
    BuildFOCone( fanout );
    Mspf();
  }
  void G1()
  {
    std::vector<int> targets = vObjs;
    for ( int i = targets.size() - 1; i >= 0; i-- )
      {
	int id = targets[i];
	//std::cout << "gate" << i << ", id" << id << std::endl;
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
	for ( int j = targets.size() - 1; j >= 0; j-- )
	  {
	    if ( vvFOs[id].empty() )
	      {
		break;
	      }
	    int id_ = targets[j];
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
	    Build();
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
};

template <typename node>
void Transduction( mockturtle::aig_network &aig, Bdd::BddMan<node> & bdd, bool fRepeat = 0, bool fCheck = 0, bool fVerbose = 0 )
{
  auto net = TransductionNetwork( aig, bdd );
  if ( fVerbose )
    {
      std::cout << "gate " << net.CountGate() << ", wire " << net.CountWire() << ", node " << net.CountWire() - net.CountGate() << std::endl;
    }
  
  if ( net.fReo )
    {
      bdd.SupportRef();
    }
  
  net.Build();

  if ( net.fReo )
    {
      bdd.Reorder();
      bdd.UnsupportRef();
    }

  net.SetEXDC();
  net.Rank();
  net.SortFIs();

  if ( net.fMspf )
    {
      net.Mspf();
    }
  else
    {
      net.Cspf();
    }

  while ( 1 )
    {
      if ( fVerbose )
	{
	  std::cout << "gate " << net.CountGate() << ", wire " << net.CountWire() << ", node " << net.CountWire() - net.CountGate() << std::endl;
	}
      int wire = net.CountWire();
      net.G1();
      if ( !fRepeat || wire == net.CountWire() )
	{
	  break;
	}
    }

  mockturtle::aig_network aig_new;
  net.Aig( aig_new );

  if ( fCheck )
    {
      auto miter = *mockturtle::miter<mockturtle::aig_network>( aig, aig_new );
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

#endif
