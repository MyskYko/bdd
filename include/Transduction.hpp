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

  bool fRemove = 0;


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
    nObjsAlloc = nRegular << 1 + aig.num_pos();
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
		    if ( std::find( vvFIs[id_].begin(), vvFIs[id_].end(), bdd.Const0() ) == vvFIs[id_].end() )
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
	x = y;
	if ( x == bdd.Const1() )
	  {
	    bdd.Deref( x );
	    Disconnect( vvFIs[id][i], id );
	    if ( vvFIs[id].empty() )
	      {
		for ( int id_ : vvFOs[id] )
		  {
		    if ( std::find( vvFIs[id_].begin(), vvFIs[id_].end(), bdd.Const0() ) == vvFIs[id_].end() )
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
	bdd.Deref( x );
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
		    if ( std::find( vvFIs[id_].begin(), vvFIs[id_].end(), bdd.Const0() ) == vvFIs[id_].end() )
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
    x = y;
    if ( x == bdd.Const1() )
      {
	bdd.Deref( x );
	Connect( fanin, fanout, 1 );
	return 1;
      }
    bdd.Deref( x );
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
    int wire = CountWire();
    RemoveRedundantFIs( fanout );
    if ( wire == CountWire() )
      Disconnect( fanin, fanout );
  }
  void G1( bool fWeak )
  {
    std::vector<int> targets = vObjs;
    for ( int i = targets.size() - 1; i >= 0; i-- )
      {
	int id = targets[i];
	std::cout << "gate" << i << ", id" << id << std::endl;
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
		//else if ( p->nMspf > 1 )
		//  Abc_BddNandG1MspfReduce( p, id, idj );	
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
		//else if ( p->nMspf > 1 )
		//  Abc_BddNandG1MspfReduce( p, id, idj );	
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

/*
typedef struct Abc_NandMan_ Abc_NandMan;
struct Abc_NandMan_ 
{
  
  Abc_BddMan * pBdd;
  Gia_Man_t *  pGia;
  Vec_Int_t *  vPiCkts;
  Vec_Int_t *  vPiIdxs;
  Vec_Ptr_t *  vvDcGias;

  int          fRm;
  int          nMspf;

  Vec_Int_t *  vOrgPis;
  Vec_Int_t *  vOrdering;
};

static inline int      Abc_BddNandConst0() { return 0; }  // = Gia_ObjId( pGia, Gia_ManConst0( pGia ) );

static inline int      Abc_BddNandObjIsPi( Abc_NandMan * p, int id ) { return (int)( p->pvFanins[id] == 0 ); }
static inline int      Abc_BddNandObjIsPo( Abc_NandMan * p, int id ) { return (int)( p->pvFanouts[id] == 0 ); }

static inline int      Abc_BddNandObjIsEmpty( Abc_NandMan * p, int id ) { return (int)( p->pvFanins[id] == 0 && p->pvFanouts[id] == 0 ); }
static inline int      Abc_BddNandObjIsDead( Abc_NandMan * p, int id ) { return (int)( Vec_IntSize( p->pvFanouts[id] ) == 0 ); }
static inline int      Abc_BddNandObjIsEmptyOrDead( Abc_NandMan * p, int id ) { return ( Abc_BddNandObjIsEmpty( p, id ) || Abc_BddNandObjIsDead( p, id ) ); }

static inline void     Abc_BddNandMemIncrease( Abc_NandMan * p ) {
  p->nMem++;
  if ( p->nMem >= 32 )
    {
      printf( "Error: Refresh failed\n" );
      abort();
    }
}




static inline int Abc_BddNandCountNewFanins( Gia_Man_t * pGia, Gia_Obj_t * pObj, int * pParts, int part )
{
  int id0, id1;
  Gia_Obj_t * pObj0, * pObj1;  
  pObj0 = Gia_ObjFanin0( pObj );
  id0 = Gia_ObjId( pGia, pObj0 );
  pObj1 = Gia_ObjFanin1( pObj );
  id1 = Gia_ObjId( pGia, pObj1 );
  return (int)(pParts[id0] != part) + (int)(pParts[id1] != part);
}
static inline int Abc_BddNandBuildFanoutCone( Abc_NandMan * p, int startId )
{ // including startId itself
  int i, id;
  Vec_Int_t * targets = Vec_IntAlloc( 1 );
  p->pMark[startId] += 2;
  Vec_IntPush( targets, startId );
  Abc_BddNandDescendantSortedList( p, p->pvFanouts, targets, startId );
  Vec_IntForEachEntry( targets, id, i )
    if ( Abc_BddNandBuild( p, id ) )
      {
	Vec_IntFree( targets );
	return -1;
      }
  Vec_IntFree( targets );
  return 0;
}
static inline int Abc_BddNandCheck( Abc_NandMan * p )
{
  int i, j, id, idj;
  unsigned x;
  Vec_IntForEachEntry( p->vObjs, id, i )
    {
      x = Abc_BddLitConst1();
      Vec_IntForEachEntry( p->pvFanins[id], idj, j )
	x = Abc_BddAnd( p->pBdd, x, p->pBddFuncs[idj] );
      if ( !Abc_BddLitIsEq( p->pBddFuncs[id], Abc_BddLitNot( x ) ) )
	{
	  printf( "Eq-check faild: different at %d %10u %10u\n", id, p->pBddFuncs[id], Abc_BddLitNot( x ) );
	  return -1;
	}
    }
  return 0;
}

static inline int Abc_BddNandDc( Abc_NandMan * p )
{
  int i, j;
  unsigned x;
  Gia_Obj_t * pObj;
  Gia_Man_t * pGia;
  Vec_Ptr_t * vDcGias;
  Vec_PtrForEachEntry( Vec_Ptr_t *, p->vvDcGias, vDcGias, i )
    {
      if ( !Vec_PtrSize( vDcGias ) )
	continue;
      x = Abc_BddLitConst1();
      Vec_PtrForEachEntry( Gia_Man_t *, vDcGias, pGia, j )
	{
	  if ( Abc_BddGia( pGia, p->pBdd ) )
	    return -1;
	  pObj = Gia_ManCo( pGia, 0 );
	  x = Abc_BddAnd( p->pBdd, x, pObj->Value );
	  if ( Abc_BddLitIsInvalid( x ) )
	    return -1;
	  if ( Abc_BddLitIsConst0( x ) )
	    break;
	}
      p->pGFuncs[Vec_IntEntry( p->vPos, i )] = x;
    }
  return 0;
}







static inline void Abc_BddNandRefresh( Abc_NandMan * p )
{
  int out;
  abctime clk0 = 0;
  if ( p->nVerbose >= 2 )
    {
      printf( "Refresh\n" );
      clk0 = Abc_Clock();
    }
  while ( 1 )
    {
      Abc_BddManFree( p->pBdd );
      p->pBdd = Abc_BddManAlloc( Vec_IntSize( p->vPis ), 1 << p->nMem, 0, p->vOrdering, 0 );
      out = Abc_BddNandDc( p );
      if ( !out )
	out = Abc_BddNandBuildAll( p );
      if ( !out )
	{
	  if ( p->nMspf < 2 )
	    out = Abc_BddNandCspf( p );
	  else
	    out = Abc_BddNandMspf( p );
	}
      if ( !out )
	break;
      Abc_BddNandMemIncrease( p );
    }
  while ( p->pBdd->nObjs > 1 << (p->nMem - 1) )
    {
      Abc_BddNandMemIncrease( p );
      Abc_BddManRealloc( p->pBdd );
    }
  if ( p->nVerbose >= 2 )
    {
      printf( "Allocated by 2^%d\n", p->nMem );
      ABC_PRT( "Refresh took", Abc_Clock() - clk0 );
    }
}
static inline void Abc_BddNandRefreshIfNeeded( Abc_NandMan * p )
{
  if ( Abc_BddIsLimit( p->pBdd ) )
    Abc_BddNandRefresh( p );
}

static inline void Abc_BddNandBuild_Refresh( Abc_NandMan * p, int id ) { if ( Abc_BddNandBuild( p, id ) ) Abc_BddNandRefresh( p ); }
static inline void Abc_BddNandBuildAll_Refresh( Abc_NandMan * p ) { if ( Abc_BddNandBuildAll( p ) ) Abc_BddNandRefresh( p ); }
static inline void Abc_BddNandBuildFanoutCone_Refresh( Abc_NandMan * p, int startId ) { if ( Abc_BddNandBuildFanoutCone( p, startId ) ) Abc_BddNandRefresh( p ); }
static inline void Abc_BddNandCspf_Refresh( Abc_NandMan * p ) { if ( Abc_BddNandCspf( p ) ) Abc_BddNandRefresh( p ); }
static inline void Abc_BddNandCspfFaninCone_Refresh( Abc_NandMan * p, int startId ) { if ( Abc_BddNandCspfFaninCone( p, startId ) ) Abc_BddNandRefresh( p ); }
static inline void Abc_BddNandRemoveRedundantFanin_Refresh( Abc_NandMan * p, int id ) {
  if ( !Abc_BddNandRemoveRedundantFanin( p, id ) )
    return;
  Abc_BddNandRefresh( p );
  if ( Abc_BddNandObjIsEmptyOrDead( p, id ) )
    return;
  while ( Abc_BddNandRemoveRedundantFanin( p, id ) )
    {
      Abc_BddNandMemIncrease( p );
      Abc_BddNandRefresh( p );
      if ( Abc_BddNandObjIsEmptyOrDead( p, id ) )
	return;
    }
}
static inline int Abc_BddNandTryConnect_Refresh( Abc_NandMan * p, int fanin, int fanout )
{
  int c;
  c = Abc_BddNandTryConnect( p, fanin, fanout );
  if ( c == -1 )
    {
      Abc_BddNandRefresh( p );
      if ( Abc_BddNandObjIsEmptyOrDead( p, fanin ) )
	return 0;
      if ( Abc_BddNandObjIsEmptyOrDead( p, fanout ) )
	return 0;
      c = Abc_BddNandTryConnect( p, fanin, fanout );
    }
  while ( c == -1 )
    {
      Abc_BddNandMemIncrease( p );
      Abc_BddNandRefresh( p );
      if ( Abc_BddNandObjIsEmptyOrDead( p, fanin ) )
	return 0;
      if ( Abc_BddNandObjIsEmptyOrDead( p, fanout ) )
	return 0;
      c = Abc_BddNandTryConnect( p, fanin, fanout );
    }
  return c;
}
static inline void Abc_BddNandMspf_Refresh( Abc_NandMan * p )
{
  int out;
  abctime clk0 = 0;
  if ( p->nVerbose >= 2 )
    {
      printf( "Refresh mspf\n" );
      clk0 = Abc_Clock();
    }
  if ( !Abc_BddNandMspf( p ) )
    return;
  while ( 1 )
    {
      Abc_BddManFree( p->pBdd );
      p->pBdd = Abc_BddManAlloc( Vec_IntSize( p->vPis ), 1 << p->nMem, 0, p->vOrdering, 0 );
      out = Abc_BddNandDc( p );
      if ( !out )
	out = Abc_BddNandBuildAll( p );
      if ( !out )
	out = Abc_BddNandMspf( p );
      if ( !out )
	break;
      Abc_BddNandMemIncrease( p );
    }
  while ( p->pBdd->nObjs > 1 << (p->nMem - 1) )
    {
      Abc_BddNandMemIncrease( p );
      Abc_BddManRealloc( p->pBdd );
    }
  if ( p->nVerbose >= 2 )
    {
      printf( "Allocated by 2^%d\n", p->nMem );
      ABC_PRT( "Refresh took", Abc_Clock() - clk0 );
    }
}

static inline void Abc_BddNandCspfEager( Abc_NandMan * p )
{
  int wires;
  wires = 0;
  while ( wires != Abc_BddNandCountWire( p ) )
    {
      wires = Abc_BddNandCountWire( p );
      Abc_BddNandRankAll( p );
      Abc_BddNandSortFaninAll( p );
      Abc_BddNandCspf_Refresh( p );
    }
}

static inline void Abc_BddNandG1MspfReduce( Abc_NandMan * p, int id, int idj )
{
  Abc_BddNandBuildFanoutCone_Refresh( p, id );
  Abc_BddNandMspf_Refresh( p );
}
}

static inline void Abc_BddNandG3( Abc_NandMan * p )
{
  int i, j, k, id, idj, idk, c, wire, new_id;
  unsigned fi, fj, f1, f0, gi, gj, x, y;
  Vec_Int_t * targets;
  c = 0; // for compile warning
  new_id = Vec_IntSize( p->vPis ) + 1;
  while ( !Abc_BddNandObjIsEmpty( p, new_id ) )
    {
      new_id++;
      if ( new_id >= p->nObjsAlloc )
	{
	  printf( "Error: Too many new merged nodes\n" );
	  abort();
	}
    }
  targets = Vec_IntDup( p->vObjs );
  Abc_BddNandCspf_Refresh( p );
  // optimize
  Vec_IntForEachEntryReverse( targets, id, i )
    {
      if ( !i )
	break;
      for ( j = i - 1; (j >= 0) && (((idj) = Vec_IntEntry(targets, j)), 1); j-- )
	{ //  Vec_IntForEachEntryReverseStart(targets, idj, j, i - 1)
	  Abc_BddNandRefreshIfNeeded( p );
	  if ( Abc_BddNandObjIsEmptyOrDead( p, id ) )
	    break;
	  if ( Abc_BddNandObjIsEmptyOrDead( p, idj ) )
	    continue;
	  if ( p->nVerbose >= 3 )
	    printf( "G3 between %d %d in %d gates\n", i, j, Vec_IntSize(targets) );
	  // calculate intersection. if it is impossible, continue.
	  fi = p->pBddFuncs[id];
	  fj = p->pBddFuncs[idj];
	  gi = p->pGFuncs[id];
	  gj = p->pGFuncs[idj];
	  f1 = Abc_BddAnd( p->pBdd, fi, fj );
	  f0 = Abc_BddAnd( p->pBdd, Abc_BddLitNot( fi ), Abc_BddLitNot( fj ) );
	  x = Abc_BddOr( p->pBdd, f1, f0 );
	  y = Abc_BddOr( p->pBdd, gi, gj );
	  x = Abc_BddOr( p->pBdd, x, y );
	  if ( !Abc_BddLitIsConst1( x ) )
	    continue;
	  // create BDD of intersection. both F and G.
	  x = Abc_BddAnd( p->pBdd, fi, Abc_BddLitNot( gi ) );
	  y = Abc_BddAnd( p->pBdd, fj, Abc_BddLitNot( gj ) );
	  x = Abc_BddOr( p->pBdd, x, y );
	  x = Abc_BddOr( p->pBdd, x, f1 );
	  y = Abc_BddAnd( p->pBdd, gi, gj );
	  if ( Abc_BddLitIsInvalid( x ) )
	    continue;
	  if ( Abc_BddLitIsInvalid( y ) )
	    continue;
	  p->pBddFuncs[new_id] = x;
	  p->pGFuncs[new_id] = y;
	  //unsigned x_ = x;
	  //	  unsigned y_ = y;
	  p->pvFanins[new_id] = Vec_IntAlloc( 1 );
	  p->pvFanouts[new_id] = Vec_IntAlloc( 1 );
	  // for all living nodes, if it is not included in fanouts of i and j, and i and j themselves, try connect it to new node.
	  Abc_BddNandMarkClear( p );
	  p->pMark[id] = 1;
	  p->pMark[idj] = 1;
	  Abc_BddNandMarkDescendant_rec( p, p->pvFanouts, id );
	  Abc_BddNandMarkDescendant_rec( p, p->pvFanouts, idj );
	  x = Abc_BddOr( p->pBdd, Abc_BddLitNot( x ), y );
	  y = Abc_BddLitConst1();
	  Vec_IntForEachEntry( p->vPis, idk, k )
	    {
	      c = Abc_BddNandTryConnect( p, idk, new_id );
	      if ( c == 1 )
		{
		  if ( Abc_BddLitIsConst1( x ) ||
		       Abc_BddLitIsInvalid( x ) ||
		       Abc_BddLitIsInvalid( y ) )
		    break;
		  y = Abc_BddAnd( p->pBdd, y, p->pBddFuncs[idk] );
		  x = Abc_BddOr( p->pBdd, x, Abc_BddLitNot( y ) );
		}
	      else if ( c == -1 )
		break;
	    }
	  if ( c == -1 )
	    {
	      Abc_BddNandRemoveNode( p, new_id );
	      continue;
	    }
	  Vec_IntForEachEntry( targets, idk, k )
	    {
	      if ( Abc_BddNandObjIsEmptyOrDead( p, idk ) || p->pMark[idk] )
		continue;
	      c = Abc_BddNandTryConnect( p, idk, new_id );
	      if ( c == 1 )
		{
		  if ( Abc_BddLitIsConst1( x ) ||
		       Abc_BddLitIsInvalid( x ) ||
		       Abc_BddLitIsInvalid( y ) )
		    break;
		  y = Abc_BddAnd( p->pBdd, y, p->pBddFuncs[idk] );
		  x = Abc_BddOr( p->pBdd, x, Abc_BddLitNot( y ) );
		}
	      else if ( c == -1 )
		break;
	    }
	  // check the F of new node satisfies F and G.
	  if ( c == -1 || !Vec_IntSize( p->pvFanins[new_id] ) || !Abc_BddLitIsConst1( x ) )
	    {
	      Abc_BddNandRemoveNode( p, new_id );
	      continue;
	    }
	  //	  assert( Abc_BddOr( p->pBdd, Abc_BddOr( p->pBdd, x_, y_ ), y ) == 1 );
	  //	  unsigned z = Abc_BddOr( p->pBdd, Abc_BddLitNot( x_ ), y_ );
	  //	  z = Abc_BddOr( p->pBdd, z, Abc_BddLitNot( y ) );
	  //	  assert( z == x );
	  // reduce the inputs
	  p->pBddFuncs[new_id] = Abc_BddLitNot( y );
	  Vec_IntForEachEntry( p->pvFanouts[id], idk, k )
	    Abc_BddNandConnect( p, new_id, idk, 0 );
	  Vec_IntForEachEntry( p->pvFanouts[idj], idk, k )
	    if ( Vec_IntFind( p->pvFanouts[new_id], idk ) == -1 )
	      Abc_BddNandConnect( p, new_id, idk, 0 );
	  Abc_BddNandObjEntry( p, new_id );
	  Abc_BddNandSortFanin( p, new_id );
	  c = Abc_BddNandRemoveRedundantFanin( p, new_id );
	  assert( !Abc_BddNandObjIsEmptyOrDead( p, new_id ) );
	  wire = Vec_IntSize( p->pvFanins[id] ) + Vec_IntSize( p->pvFanins[idj] );
	  if ( c || Vec_IntSize( p->pvFanins[new_id] ) > wire - 1 )
	    {
	      Abc_BddNandRemoveNode( p, new_id );
	      continue;
	    }
	  // if inputs < inputs_before - 1, do the followings
	  // remove merged (replaced) nodes
	  Abc_BddNandRemoveNode( p, id );
	  Abc_BddNandRemoveNode( p, idj );
	  // calculate function of new node
	  Abc_BddNandBuildFanoutCone_Refresh( p, new_id );
	  Abc_BddNandCspf_Refresh( p );
	  while ( !Abc_BddNandObjIsEmpty( p, new_id ) )
	    {
	      new_id++;
	      assert( new_id < p->nObjsAlloc );
	    }
	  break;
	}
    }
  Vec_IntFree( targets );
}

static inline void Abc_BddNandPropagateDc( Vec_Ptr_t * vNets, int from, Gia_Man_t * pGlobal, int nDcPropagate )
{
  int i, j, k, l, id, idj, pi, index, nPos, flag, count;
  int * pPos;
  unsigned x, y;
  Vec_Int_t * vVars, * vNodes;
  Vec_Int_t ** pvPis;
  Abc_NandMan * pFrom, * pTo;
  Gia_Man_t * pDc, * pTmp, * pBase;
  vVars = Vec_IntAlloc( 1 );
  vNodes = Vec_IntAlloc( 1 );
  pFrom = Vec_PtrEntry( vNets, from );
  pvPis = ABC_CALLOC( Vec_Int_t *, Vec_PtrSize( vNets ) );
  for ( i = 0; i < Vec_PtrSize( vNets ); i++ )
    pvPis[i] = Vec_IntAlloc( 1 );
  Vec_IntForEachEntry( pFrom->vPis, id, pi )
    if ( Vec_IntEntry( pFrom->vPiCkts, pi ) >= 0 )
      Vec_IntPush( pvPis[Vec_IntEntry( pFrom->vPiCkts, pi )], pi );
  Vec_PtrForEachEntry( Abc_NandMan *, vNets, pTo, k )
    {
      if ( !Vec_IntSize( pvPis[k] ) )
	continue;
      Vec_IntForEachEntry( pvPis[k], pi, i )
	{
	  id = Vec_IntEntry( pFrom->vPis, pi );
	  x = Abc_BddLitConst1();
	  // calculate AND of G of fanouts
	  Vec_IntForEachEntry( pFrom->pvFanouts[id], idj, j )
	    {
	      if ( Abc_BddNandObjIsPo( pFrom, idj ) )
		x = Abc_BddAnd( pFrom->pBdd, x, pFrom->pGFuncs[idj] );
	      else
		{
		  index = Vec_IntFind( pFrom->pvFanins[idj], id );
		  y = Vec_IntEntry( pFrom->pvCFuncs[idj], index );
		  x = Abc_BddAnd( pFrom->pBdd, x, y );
		}
	    }
	  while ( Abc_BddLitIsInvalid( x ) )
	    {
	      Abc_BddNandRefresh( pFrom );
	      x = Abc_BddLitConst1();
	      Vec_IntForEachEntry( pFrom->pvFanouts[id], idj, j )
		{
		  if ( Abc_BddNandObjIsPo( pFrom, idj ) )
		    x = Abc_BddAnd( pFrom->pBdd, x, pFrom->pGFuncs[idj] );
		  else
		    {
		      index = Vec_IntFind( pFrom->pvFanins[idj], id );
		      y = Vec_IntEntry( pFrom->pvCFuncs[idj], index );
		      x = Abc_BddAnd( pFrom->pBdd, x, y );
		    }
		}
	      if ( Abc_BddLitIsInvalid( x ) )
		Abc_BddNandMemIncrease( pFrom );
	    }
	  Vec_IntClear( vNodes );
	  Vec_IntPush( vNodes, x );
	  pDc = Abc_BddGenGia( pFrom->pBdd, vNodes );
	  if ( nDcPropagate == 1 )
	    {
	      // Universally quantify unused inputs
	      Vec_IntClear( vVars );
	      for ( pi = 0; pi < Vec_IntSize( pFrom->vPis ); pi++ )	
		if ( Vec_IntFind( pvPis[k], pi ) == -1 )
		  {
		    pTmp = pDc;
		    pDc = Gia_ManDupUniv( pDc, pi );
		    Gia_ManStop( pTmp );
		    Vec_IntPush( vVars, pi );
		  }
	      // Add inputs to match the number of inputs
	      while( Gia_ManCiNum( pDc ) < Vec_IntSize( pTo->vPos ) )
		{
		  Vec_IntPush( vVars, Gia_ManCiNum( pDc ) );
		  Gia_ManAppendCi( pDc );
		}
	      // Permitate inputs to match them with the outputs of the next partition
	      Vec_IntClear( vNodes );
	      count = 0;
	      for ( j = 0; j < Gia_ManCiNum( pDc ); j++ )
		{
		  flag = 0;
		  Vec_IntForEachEntry( pvPis[k], pi, l )
		    if ( j == Vec_IntEntry( pFrom->vPiIdxs, pi ) )
		      {
			Vec_IntPush( vNodes, pi );
			flag = 1;
			break;
		      }
		  if ( flag )
		    continue;
		  Vec_IntPush( vNodes, Vec_IntEntry( vVars, count ) );
		  count++;
		}
	      pTmp = pDc;
	      pDc = Gia_ManDupPerm( pDc, vNodes );
	      Gia_ManStop( pTmp );
	      // remove unused inputs
	      pTmp = pDc;
	      pDc = Gia_ManDupRemovePis( pDc, Gia_ManCiNum( pDc ) - Vec_IntSize( pTo->vPos ) );
	      Gia_ManStop( pTmp );
	      // Place it on top of the next partition
	      pTmp = pDc;
	      pDc = Gia_ManDupOntop( pTo->pGia, pDc );
	      Gia_ManStop( pTmp );
	      Vec_PtrPush( Vec_PtrEntry( pTo->vvDcGias, Vec_IntEntry( pFrom->vPiIdxs, pi ) ), pDc );
	      continue;
	    }
	  // if nDcPropagate >= 2
	  // create a circuit with inputs of the next circuit and outputs of the previous circuit
	  pBase = Abc_BddNandGiaExpand( pGlobal, pTo->vOrgPis, pFrom->vOrgPis );
	  nPos = Vec_IntSize( pFrom->vOrgPis );
	  pPos = ABC_CALLOC( int, nPos );
	  for ( j = 0; j < nPos; j++ )
	    pPos[j] = Gia_ManCoNum( pGlobal ) + j;
	  pTmp = pBase;
	  pBase = Gia_ManDupCones( pBase, pPos, nPos, 0 );
	  Gia_ManStop( pTmp );
	  ABC_FREE( pPos );
	  // create a DC circuit in terms of inputs of the next circuit
	  pTmp = pDc;
	  pDc = Gia_ManDupOntop( pBase, pDc );
	  Gia_ManStop( pBase );
	  Gia_ManStop( pTmp );
	  // remove unnecessary inputs by universing it
	  for ( j = 0; j < Gia_ManCiNum( pGlobal ); j++ )
	    {
	      pTmp = pDc;
	      pDc = Gia_ManDupUniv( pDc, j );
	      Gia_ManStop( pTmp );
           }
	  pTmp = pDc;
	  pDc = Gia_ManDupLastPis( pDc, Vec_IntSize( pTo->vOrgPis ) );
	  Gia_ManStop( pTmp );
	  
	  // push it to dc list
	  Vec_PtrPush( Vec_PtrEntry( pTo->vvDcGias, Vec_IntEntry( pFrom->vPiIdxs, pi ) ), pDc );
	}
    }
  Vec_IntFree( vVars );
  Vec_IntFree( vNodes );
  for ( i = 0; i < Vec_PtrSize( vNets ); i++ )
    Vec_IntFree( pvPis[i] );
  ABC_FREE( pvPis );
}

static inline void Abc_BddNandPrintStats( Abc_NandMan * p, char * prefix, abctime clk0 )
{
  printf( "\r%-10s: gates = %5d, wires = %5d, AIG node = %5d", prefix, Vec_IntSize( p->vObjs ), Abc_BddNandCountWire( p ), Abc_BddNandCountWire( p ) - Vec_IntSize( p->vObjs ) );
  ABC_PRT( ", time ", Abc_Clock() - clk0 );
}

*/
template <typename node>
void Transduction( mockturtle::aig_network &aig, Bdd::BddMan<node> & bdd, int nVerbose )
{
  auto net = TransductionNetwork( aig, bdd );
  std::cout << "gate " << net.CountGate() << ", wire " << net.CountWire() << ", node " << net.CountWire() - net.CountGate() << std::endl;
  
  net.Build();
  net.SetEXDC();
  net.Rank();
  net.SortFIs();

  //  net.Mspf();
  net.Cspf();

  net.G1(1);
  
  std::cout << "gate " << net.CountGate() << ", wire " << net.CountWire() << ", node " << net.CountWire() - net.CountGate() << std::endl;
  
  mockturtle::aig_network aig_new;
  net.Aig( aig_new );
  
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
  
  aig = aig_new;
  /*
  // optimize
  abctime clk0 = Abc_Clock();
  Vec_PtrForEachEntry( Abc_NandMan *, vNets, p, i )
    {
      if ( fReo )
	{
	  vFuncs = Vec_IntAlloc( 1 );
	  Vec_IntForEachEntry( p->vObjs, id, k )
	    Vec_IntPush( vFuncs, p->pBddFuncs[id] );
	  Vec_IntForEachEntry( p->vPos, id, k )
	    Vec_IntPush( vFuncs, p->pGFuncs[id] );
	  Abc_BddReorderConfig( p->pBdd, 10 );
	  p->pBdd->fGC = 1;
	  p->pBdd->fRealloc = 1;
	  Abc_BddReorder( p->pBdd, vFuncs );
	  Abc_BddReorderConfig( p->pBdd, 0 );
	  p->pBdd->fGC = 0;
	  p->pBdd->fRealloc = 0;
	  p->vOrdering = Vec_IntDup( p->pBdd->vOrdering );
	  Vec_IntFree( vFuncs );
	  p->nMem = Abc_Base2Log( p->pBdd->nObjsAlloc );
	}
      if ( nVerbose >= 2 )
	printf( "Allocated by 2^%d\n", p->nMem );
      if ( nVerbose )
	Abc_BddNandPrintStats( p, "initial", clk0 );
      if ( nVerbose )
	Abc_BddNandPrintStats( p, "pf", clk0 );
      int wire = 0;
      while ( wire != Abc_BddNandCountWire( p ) )
	{
	  wire = Abc_BddNandCountWire( p );
	  switch ( nType ) {
	  case 0:
	    break;
	  case 1:
	    Abc_BddNandG1( p, 0, fSpec );
	    if ( nVerbose ) Abc_BddNandPrintStats( p, "G1", clk0 );
	    break;
	  case 2:
	    Abc_BddNandG1( p, 1, fSpec );
	    if ( nVerbose ) Abc_BddNandPrintStats( p, "G2", clk0 );
	    break;
	  case 3:
	    Abc_BddNandG3( p );
	    if ( nVerbose ) Abc_BddNandPrintStats( p, "G3", clk0 );
	    break;
	  default:
	    printf( "Error: Invalid optimization type %d\n", nType );
	    abort();
	  }
	  if ( p->nMspf )
	    Abc_BddNandMspf_Refresh( p );
	  if ( p->nMspf < 2 )
	    Abc_BddNandCspfEager( p );
	  if ( !fRep )
	    break;
	}
      if ( nWindowSize && nDcPropagate )
	{
	  Abc_BddNandCspfEager( p );
	  Abc_BddNandPropagateDc( vNets, i, pGia, nDcPropagate );
	}
    }
  if ( nVerbose )
    ABC_PRT( "total ", Abc_Clock() - clk0 );
  pNew = Abc_BddNandNets2Gia( vNets, vPoCkts, vPoIdxs, vExternalCs, fDc, pGia );
  Vec_IntFree( vPoCkts );
  Vec_IntFree( vPoIdxs );
  Vec_IntFree( vExternalCs );
  Vec_PtrForEachEntry( Abc_NandMan *, vNets, p, i )
    Abc_BddNandManFree( p );
  Vec_PtrFree( vNets );
  return pNew;
*/
}

#endif
