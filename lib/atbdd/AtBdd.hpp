#ifndef AT_BDD_HPP_
#define AT_BDD_HPP_

#include <vector>
#include <cassert>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <forward_list>

namespace AtBdd
{
/**Function*************************************************************

   Synopsis    [Data type]

   Description [var = Variable, lit = Literal, bvar = BddVariable = Literal >> 1]

   SideEffects []

   SeeAlso     []

***********************************************************************/
  typedef uint16_t var;
  typedef uint32_t lit;
  typedef int bvar; // signed lit
  typedef uint8_t mark;
  typedef uint32_t edge;
  typedef uint64_t size;
  typedef int64_t ssize;
  
/**Function*************************************************************
   
   Synopsis    [Hash]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
//  lit Hash( lit Arg0, lit Arg1, lit Arg2 ) { return 12582917 * Arg0 + 4256249 * Arg1 + 741457 * Arg2; }
  lit Hash( lit Arg0, lit Arg1, lit Arg2 ) { return 12582917 * Arg0 + Arg1 + 4256249 * Arg2; }
  //  lit Hash( lit Arg0, lit Arg1, lit Arg2 ) { return 67280421310721ull * Arg0 + 2147483647ull * Arg1 + 12582917 * Arg2; }
  
/**Function*************************************************************
   
   Synopsis    [Class]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
class BddMan
{
private:
  var    nVars;         // the number of variables
  bvar   nObjs;         // the number of nodes used
  lit    nObjsAlloc;    // the number of nodes allocated
  lit    nUnique;       // the number of buckets in unique table
  lit    nCache;        // the number of cache
  var *  pVars;         // array of variables for nodes
  lit *  pObjs;         // array of pairs cof0 for nodes
  mark * pMarks;        // array of marks for nodes
  bvar * pNexts;        // next pointer for nodes
  bvar * pUnique;       // unique table for nodes
  lit *  pCache;        // array of triples <arg0, arg1, AND(arg0, arg1)>
  bvar   nUniqueMask;   // selection mask for unique table
  bvar   nCacheMask;    // selection mask for computed table
  lit    nMinRemoved;   // the minimum int of removed nodes

  double UniqueMinRate;
  
  size   nCacheHit;
  size   nCacheFind;
  size   nCall;
  size   nCallThold;
  double HitRateOld;
  
  int    nRefresh;      // the number of refresh tried
  bool   fRealloc;      // flag of reallocation
  bool   fGC;           // flag of garbage collection
  lit    nGC;           // threshold to run garbage collection
  bool   fReo;          // flag or reordering
  size   nReo;          // threshold to run reordering
  double MaxGrowth;     // threshold to terminate reordering. 0=off
  edge * pEdges;        // array of number of incoming edges for nodes
  
  std::vector<var>                vOrdering; // variable ordering : new 2 old
  std::vector<std::vector<bvar> > liveBvars; // array of live Bvars for each layer
  std::forward_list<lit> *        pvNodes;   // live nodes (only top of tree)
  
  int    nVerbose;      // the level of verbosing information
  
public:
  int  get_nVars() { return nVars; }
  int  get_order( int v ) { return vOrdering[v]; }
  int  get_nObjs() { return nObjs; }
  int  get_pvNodesExists() { return pvNodes != NULL; }
  
/**Function*************************************************************
   
   Synopsis    [Reference]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  void Ref( lit x ) { if ( pvNodes ) pvNodes->push_front( LitRegular( x ) ); }
  void Pop()        { if ( pvNodes ) pvNodes->pop_front(); }
  void Deref( lit x )
  {
    if ( pvNodes )
      {
	auto it = pvNodes->begin();
	if ( *it == LitRegular( x ) )
	  {
	    pvNodes->pop_front();
	    return;
	  }
	auto itnext = it;
	itnext++;
	while ( itnext != pvNodes->end() )
	  {
	    if ( *itnext == LitRegular( x ) )
	      {
		pvNodes->erase_after( it );
		return;
	      }
	    it++;
	    itnext++;
	  }
	std::cout << "cannot find " << LitRegular( x ) << std::endl;
	for ( lit x : *pvNodes )
	  std::cout << x << ",";
	std::cout << std::endl;
	throw "Deref non-referenced node";
      }
  }

/**Function*************************************************************
   
   Synopsis    [Definition of constant and invalid values]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  bvar BvarConst()   { return 0;                                }
  var  VarInvalid()  { return std::numeric_limits<var>::max();  }
  bvar BvarInvalid() { return std::numeric_limits<bvar>::max(); }
  edge EdgeInvalid() { return std::numeric_limits<edge>::max(); }
  mark MarkInvalid() { return std::numeric_limits<mark>::max(); }
  
/**Function*************************************************************
   
   Synopsis    [Bvar to/from Lit]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  lit  Bvar2Lit( bvar a, bool c ) { return a + a + (int)c; }
  bvar Lit2Bvar( lit x )          { return x >> 1;         }

/**Function*************************************************************
   
   Synopsis    [Utilities for Bvar]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  bvar BvarIthVar( var v )        { return v + 1;                        }
  bool BvarIsEq( bvar a, bvar b ) { return a == b;                       }
  bool BvarIsConst( bvar a )      { return BvarIsEq( a, BvarConst() );   }
  bool BvarIsInvalid( bvar a )    { return BvarIsEq( a, BvarInvalid() ); }

  var  VarOfBvar( bvar a )        { return pVars[a];                     }
  lit  ThenOfBvar( bvar a )       { return pObjs[Bvar2Lit( a, 0 )];      }
  lit  ElseOfBvar( bvar a )       { return pObjs[Bvar2Lit( a, 1 )];      }
  bvar NextOfBvar( bvar a )       { return pNexts[a];                    }
  mark MarkOfBvar( bvar a )       { return pMarks[a];                    }
  edge EdgeOfBvar( bvar a )       { return pEdges[a];                    }

  void SetVarOfBvar( bvar a, var v )   { pVars[a] = v;                 }
  void SetThenOfBvar( bvar a, lit x1 ) { pObjs[Bvar2Lit( a, 0 )] = x1; }
  void SetElseOfBvar( bvar a, lit x0 ) { pObjs[Bvar2Lit( a, 1 )] = x0; }
  void SetNextOfBvar( bvar a, bvar b ) { pNexts[a] = b;                }
  void SetMarkOfBvar( bvar a, mark m ) { pMarks[a] = m;                }
  void SetEdgeOfBvar( bvar a, edge e ) { pEdges[a] = e;                }

  bool BvarIsRemoved( bvar a )       { return VarOfBvar( a ) == VarInvalid();        }
  void SetVarOfBvarRemoved( bvar a ) { SetVarOfBvar( a, VarInvalid() );              }
  bool BvarIsVar( bvar a )           { return a <= (bvar)nVars && !BvarIsConst( a ); }
  
/**Function*************************************************************
   
   Synopsis    [Utilities for Lit]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  lit  LitRegular( lit x )         { return x & ~01;                        }
  lit  LitNot( lit x )             { return x ^ 1;                          }
  lit  LitNotCond( lit x, bool c ) { return x ^ (int)c;                     }
  lit  LitConst0()                 { return Bvar2Lit( BvarConst(), 0 );     }
  lit  LitConst1()                 { return LitNot( LitConst0() );          }
  lit  LitInvalid()                { return Bvar2Lit( BvarInvalid(), 0 );   }
  lit  LitIthVar( var v )          { return Bvar2Lit( BvarIthVar( v ), 0 ); }
  bool LitIsCompl( lit x )         { return x & 1;                          }
  bool LitIsEq( lit x, lit y )     { return x == y;                         }
  bool LitIsConst0( lit x )        { return LitIsEq( x, LitConst0() );      }
  bool LitIsConst1( lit x )        { return LitIsEq( x, LitConst1() );      }
  bool LitIsConst( lit x )         { return BvarIsConst( Lit2Bvar( x ) );   }
  bool LitIsInvalid( lit x )       { return BvarIsInvalid( Lit2Bvar( x ) ); }
  bool LitIsRemoved( lit x )       { return Var( x ) == VarInvalid();       }
  bool LitIsVar( lit x )           { return BvarIsVar( Lit2Bvar( x ) );     }
  
  var  Var( lit x )  { return VarOfBvar( Lit2Bvar( x ) ); }
  lit  Then( lit x ) { return LitNotCond( pObjs[LitRegular( x )], LitIsCompl( x ) ); }
  lit  Else( lit x ) { return LitNotCond( pObjs[LitNot( LitRegular( x ) )], LitIsCompl( x ) ); }
  bvar Next( lit x ) { return NextOfBvar( Lit2Bvar( x ) ); }
  mark Mark( lit x ) { return MarkOfBvar( Lit2Bvar( x ) ); }
  edge Edge( lit x ) { return EdgeOfBvar( Lit2Bvar( x ) ); }

  void SetMark( lit x, mark m ) { SetMarkOfBvar( Lit2Bvar( x ), m ); }
  void IncMark( lit x ) { if ( ++pMarks[Lit2Bvar( x )] == MarkInvalid() ) throw "Mark overflow"; }
  void DecMark( lit x ) { assert( --pMarks[Lit2Bvar( x )] != MarkInvalid() ); }

  void IncEdge( lit x ) { if ( ++pEdges[Lit2Bvar( x )] == EdgeInvalid() ) throw "Edge overflow"; }
  void DecEdge( lit x ) { assert( --pEdges[Lit2Bvar( x )] != EdgeInvalid() ); }
  void IncEdgeNonConst( lit x ) { if ( !LitIsConst( x ) ) IncEdge( x ); }
  void DecEdgeNonConst( lit x ) { if ( !LitIsConst( x ) ) DecEdge( x ); }

/**Function*************************************************************
   
   Synopsis    [Node overflow]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  bool IsLimit() { return (lit)nObjs == nObjsAlloc || nObjs == BvarInvalid(); }
  
/**Function*************************************************************
   
   Synopsis    [Mark node]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  void Mark_rec( lit x )
  {
    if ( LitIsConst( x ) || Mark( x ) )
      return;
    SetMark( x, 1 );
    Mark_rec( Else( x ) );
    Mark_rec( Then( x ) );
  }
  void Unmark_rec( lit x )
  {
    if ( LitIsConst( x ) || !Mark( x ) )
      return;
    SetMark( x, 0 );
    Unmark_rec( Else( x ) );
    Unmark_rec( Then( x ) );
  }

/**Function*************************************************************
   
   Synopsis    [Count node]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  size Count_rec( lit x )
  {
    if ( /*LitIsConst( x ) ||*/ Mark( x ) )
      return 0;
    SetMark( x, 1 );
    return 1 + Count_rec( Else( x ) ) + Count_rec( Then( x ) );
  }
  size CountNodes( lit x )
  {
    size count = Count_rec( x );
    Unmark_rec( x );
    SetMark( 0, 0 );
    return count;
  }
  size CountNodesArrayShared( std::vector<lit> & vNodes )
  {
    size count = 0;
    for ( lit x : vNodes )
      count += Count_rec( x );
    //for ( var v = 0; v < nVars; v++ )
    //      count += Count_rec( LitIthVar( v ) );
    for ( lit x : vNodes )  
      Unmark_rec( x );
    //    for ( var v = 0; v < nVars; v++ )
    //      Unmark_rec( LitIthVar( v ) );
    SetMark( 0, 0 );
    return count; // +4
  }
  size CountNodesArrayIndependent( std::vector<lit> & vNodes )
  {
    size count = 0;
    for ( lit x : vNodes )
      {
	if ( LitIsConst( x ) || LitIsVar( x ) )
	  continue; 
	count += Count_rec( x );
	Unmark_rec( x );
	SetMark( 0, 0 );
    }
  return count;
  }

/**Function*************************************************************
   
   Synopsis    [Count edge]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  void CountEdge_rec( lit x )
  {
    if ( LitIsConst( x ) )
      return;
    IncEdge( x );
    if ( Mark( x ) )
      return;
    SetMark( x, 1 );
    CountEdge_rec( Else( x ) );
    CountEdge_rec( Then( x ) );
  }
  void CountEdge()
  {
    for ( lit x : *pvNodes )
      CountEdge_rec( x );
    for ( var v = 0; v < nVars; v++ )
      CountEdge_rec( LitIthVar( v ) );
    for ( lit x : *pvNodes )
      Unmark_rec( x );
    for ( var v = 0; v < nVars; v++ )
      Unmark_rec( LitIthVar( v ) );
  }
  void UncountEdge_rec( lit x )
  {
    if ( LitIsConst( x ) )
      return;
    DecEdge( x );
    if ( Mark( x ) )
      return;
    SetMark( x, 1 );
    UncountEdge_rec( Else( x ) );
    UncountEdge_rec( Then( x ) );
  }
  void UncountEdge()
  {
    for ( lit x : *pvNodes )
    UncountEdge_rec( x );
    for ( var v = 0; v < nVars; v++ )
      UncountEdge_rec( LitIthVar( v ) );
    for ( lit x : *pvNodes )
      Unmark_rec( x );
    for ( var v = 0; v < nVars; v++ )
      Unmark_rec( LitIthVar( v ) );
  }
  void CountEdgeAndBvar_rec( lit x )
  {
    if ( LitIsConst( x ) )
      return;
    IncEdge( x );
    if ( Mark( x ) )
      return;
    liveBvars[Var( x )].push_back( Lit2Bvar( x ) );
    SetMark( x, 1 );
    CountEdgeAndBvar_rec( Else( x ) );
    CountEdgeAndBvar_rec( Then( x ) );
  }
  void CountEdgeAndBvar()
  {
    for ( lit x : *pvNodes )
      CountEdgeAndBvar_rec( x );
    for ( var v = 0; v < nVars; v++ )
      CountEdgeAndBvar_rec( LitIthVar( v ) );
    for ( lit x : *pvNodes )
      Unmark_rec( x );
    for ( var v = 0; v < nVars; v++ )
      Unmark_rec( LitIthVar( v ) );
  }
  
/**Function*************************************************************
 
   Synopsis    [Allocation/free]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  BddMan( var nVars, lit nObjsAlloc_, lit nUnique_, lit nCache, int nUniqueMinRate, int nCallThold, std::vector<var> * pvOrdering, int nVerbose ) : nVars( nVars ), nObjsAlloc( nObjsAlloc_ ), nUnique( nUnique_ ), nCache( nCache ), UniqueMinRate( 0.01 * nUniqueMinRate ), nCallThold( nCallThold ), nVerbose( nVerbose )
  {
    if ( nVars == VarInvalid() )
      throw "Varable overflow";
    if( !nObjsAlloc || nObjsAlloc & ( nObjsAlloc - 1 ) )
      throw "nObjsAlloc must be power of 2";
    if( !nUnique || nUnique & ( nUnique - 1 ) )
      throw "nUnique must be power of 2";
    if( !nCache || nCache & ( nCache - 1 ) )
      throw "nCache must be power of 2";
    while ( nObjsAlloc < (lit)nVars + 1 )
      {
	nObjsAlloc = nObjsAlloc + nObjsAlloc;
	if ( !nObjsAlloc )
	  throw "Node overflow just for Variables";
      }
    while ( nUnique < nObjsAlloc * UniqueMinRate )
      {
	if ( nUnique > (lit)BvarInvalid() )
	  break;
	nUnique = nUnique + nUnique;
	if ( !nUnique )
	  throw "Unique overflow just for init\n";
      }
    if ( nVerbose )
      std::cout << "Allocate " << nObjsAlloc << " nodes, " << nUnique << " unique, and " << nCache<< " cache" << std::endl;
    nRefresh    = 0;
    fRealloc    = 0;
    fGC         = 0;
    nGC         = 0;
    fReo        = 0;
    nReo        = 0;
    MaxGrowth   = 0;
    nCacheHit   = 0;
    nCacheFind  = 0;
    nCall       = 0;
    HitRateOld  = 1;
    nMinRemoved = nObjsAlloc;
    pVars       = (var *)calloc( nObjsAlloc, sizeof(var) );
    pObjs       = (lit *)calloc( 2 * (size)nObjsAlloc, sizeof(lit) );
    pMarks      = (mark *)calloc( nObjsAlloc, sizeof(mark) );
    pNexts      = (bvar *)calloc( nObjsAlloc, sizeof(bvar) );
    nUniqueMask = nUnique - 1;
    pUnique     = (bvar *)calloc( nUnique, sizeof(bvar) );
    nCacheMask  = nCache - 1;
    pCache      = (lit *)calloc( 3 * (size)nCache, sizeof(lit) );
    if ( !pVars || !pObjs || !pMarks || !pNexts || !pUnique || !pCache )
      throw "Allocation failed";
    pvNodes     = NULL;
    pEdges      = NULL;
    SetVarOfBvar( BvarConst(), VarInvalid() );
    nObjs = 1;
    vOrdering.clear();
    if ( pvOrdering )
      {
	for ( var v : *pvOrdering )
	  vOrdering.push_back( v );
	if ( vOrdering.size() != nVars )
	  throw "Wrong number of Variables in the ordering";
      }
    else
      for ( var v = 0; v < nVars; v++ )
	vOrdering.push_back( v );
    for ( var v = 0; v < nVars; v++ )
      {
	var u = std::distance( vOrdering.begin(), std::find( vOrdering.begin(), vOrdering.end(), v ) );
	if( u == nVars )
	  throw "Invalid Ordering";
	UniqueCreate( u, LitConst1(), LitConst0() );
      }
  }
  ~BddMan()
  {
    if ( nVerbose )
      {
	if ( nObjsAlloc > (lit)BvarInvalid() )
	  std::cout << "Free : Var = " << (size)nVars << " Obj = " << (size)nObjs << " Alloc = " << (size)BvarInvalid() << std::endl;
	else
	  std::cout << "Free : Var = " << (size)nVars << " Obj = " << (size)nObjs << " Alloc = " << (size)nObjsAlloc << std::endl;
      }
    free( pUnique );
    free( pCache );
    free( pVars );
    free( pObjs );
    free( pMarks );
    free( pNexts );
    if ( pvNodes )
      delete pvNodes;
  }

/**Function*************************************************************
   
   Synopsis    [Create new node]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  lit UniqueCreateInt( var v, lit x1, lit x0 )
  {
    bvar * q = pUnique + ( Hash( v, x1, x0 ) & nUniqueMask );
    for ( ; *q; q = pNexts + *q )
      if ( VarOfBvar( *q ) == v &&
	   ThenOfBvar( *q ) == x1 &&
	   ElseOfBvar( *q ) == x0 )
	return Bvar2Lit( *q, 0 );
    q = pUnique + ( Hash( v, x1, x0 ) & nUniqueMask );
    bvar head = *q;
    if ( IsLimit() )
      {
	for ( ; nMinRemoved < (lit)nObjs; nMinRemoved++ )
	  if ( BvarIsRemoved( nMinRemoved ) )
	    break;
	if ( nMinRemoved == (lit)nObjs )
	  return LitInvalid();
	*q = nMinRemoved++;
      }
    else
      *q = nObjs++;
    SetVarOfBvar( *q, v );
    SetThenOfBvar( *q, x1 );
    SetElseOfBvar( *q, x0 );
    SetNextOfBvar( *q, head );
    if ( nVerbose >= 3 )
      std::cout << "Node " << (size)*q << " : Var = " << (size)v << " Then = " << (size)x1 << " Else = " << (size)x0 << " MinRemoved = " << (size)nMinRemoved << std::endl;
    return Bvar2Lit( *q, 0 );
  }
  lit UniqueCreate( var v, lit x1, lit x0 )
  {
    if ( LitIsEq( x1, x0 ) )
      return x0;
    lit x;
    while ( 1 )
      {
	if ( !LitIsCompl( x0 ) )
	  x = UniqueCreateInt( v, x1, x0 );
	else
	  x = LitNot( UniqueCreateInt( v, LitNot( x1 ), LitNot( x0 ) ) );
	if ( LitIsInvalid( x ) )
	  {
	    if ( Refresh() )
	      return x;
	  }
	else
	  break;
      }
    return x;
  }

/**Function*************************************************************

   Synopsis    [Cache for AND operation]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  lit CacheLookup( lit Arg1, lit Arg2 )
  {
    nCacheFind++;
    lit * p = pCache + 3 * (size)( Hash( 0, Arg1, Arg2 ) & nCacheMask );
    if ( p[0] == Arg1 && p[1] == Arg2 )
      {
	nCacheHit++;
	return p[2];
      }
    return LitInvalid();
  }
  lit CacheInsert( lit Arg1, lit Arg2, lit Res )
  {
    if ( LitIsInvalid( Res ) )
      return Res;
    lit * p = pCache + 3 * (size)( Hash( 0, Arg1, Arg2 ) & nCacheMask );
    p[0] = Arg1;
    p[1] = Arg2;
    p[2] = Res;
    return Res;
  }
  void CacheResize()
  {
    if ( nCache > (lit)BvarInvalid() )
      return;
    lit nCacheOld = nCache;
    nCache = nCache + nCache;
    nCacheMask = nCache - 1;
    pCache = (lit *)realloc( pCache, sizeof(lit) * 3 * (size)nCache );
    if ( !pCache )
      throw "Cache reallocation failed";
    memset( pCache + 3 * (size)nCacheOld, 0, sizeof(lit) * 3 * (size)nCacheOld );
    for ( lit i = 0; i < nCacheOld; i++ )
      {
	lit * p = pCache + i + i + i;
	if ( !p[0] && !p[1] )
	  continue;
	lit hash = Hash( 0, p[0], p[1] ) & nCacheMask;
	if ( i !=  hash )
	  {
	    lit * q = pCache + hash + hash + hash;
	    q[0] = p[0];
	    q[1] = p[1];
	    q[2] = p[2];
	    p[0] = 0;
	    p[1] = 0;
	    p[2] = 0;
	  }
      }
  }
  void CacheCheck()
  {
    nCall++;
    if ( nCall < nCallThold )
      return;
    double HitRate = nCacheHit * 1.0 / nCacheFind;
    if ( HitRate > HitRateOld )
      CacheResize();
    HitRateOld = HitRate;
    nCallThold = nCallThold + nCallThold;
  }
  void CacheClear()
  {
    free( pCache );
    pCache = (lit *)calloc( 3 * (size)nCache, sizeof(lit) );
  }
  
/**Function*************************************************************
   
   Synopsis    [And]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  lit And_rec( lit x, lit y )
  {
    CacheCheck();
    if ( LitIsConst0( x ) )
      return x;
    if ( LitIsConst0( y ) )
      return y;
    if ( LitIsConst1( x ) )
      return y;
    if ( LitIsConst1( y ) )
      return x;
    if ( LitIsEq( x, y ) )
      return x;
    if ( x > y )
      return And_rec( y, x );
    lit z = CacheLookup( x, y );
    if ( !LitIsInvalid( z ) )
      return z;
    lit x0, x1, y0, y1;
    if ( Var( x ) < Var( y ) )
      {
	x0 = Else( x );
	x1 = Then( x );
	y0 = y;
	y1 = y;
      }
    else if ( Var( x ) > Var( y ) )
      {
	x0 = x;
	x1 = x;
	y0 = Else( y );
	y1 = Then( y );
      }
    else // if ( Var( p, x ) == Var( p, y ) )
      {
	x0 = Else( x );
	x1 = Then( x );
	y0 = Else( y );
	y1 = Then( y );
      }
    lit z1 = And_rec( x1, y1 );
    if ( LitIsInvalid( z1 ) )
      return z1;
    Ref( z1 );
    lit z0 = And_rec( x0, y0 );
    if( LitIsInvalid( z0 ) )
      {
	Pop();
	return z0;
      }
    Ref( z0 );
    z = UniqueCreate( std::min( Var( x ), Var( y ) ), z1, z0 );
    Pop();
    Pop();
    if ( LitIsInvalid( z ) )
      return z;
    return CacheInsert( x, y, z );
  }
  lit And( lit x, lit y )
  {
    nRefresh = 0;
    lit z = LitInvalid();
    while( LitIsInvalid( z ) )
      z = And_rec( x, y );
    return z;
  }
  
/**Function*************************************************************

   Synopsis    [Refresh]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  void SupportRef()
  {
    if ( !pvNodes )
      pvNodes = new std::forward_list<lit>;
  }
  void UnsupportRef()
  {
    if ( !fGC && !fReo && pvNodes )
      {
	delete pvNodes;
	pvNodes = NULL;
      }
  }
  void RefreshConfig( bool fRealloc_, bool fGC_, lit nGC_, bool fReo_, lit nReo_, int nMaxGrowth )
  {
    fRealloc = fRealloc_;
    fGC = fGC_;
    nGC = nGC_;
    fReo = fReo_;
    nReo = nReo_;
    MaxGrowth = 0.01 * nMaxGrowth;
    UnsupportRef();
    if ( fGC || fReo )
      SupportRef();
  }
  void Dvr() { fReo = 1; }
  void DvrOff() { fReo = 0; }
  bool Refresh()
  {
    if ( nVerbose )
      std::cout << "Refresh " << std::endl;
    if ( fReo && (size)nObjs > nReo )
      {
	Reorder();
	nReo = nReo + nReo;
	return 1;
      }
    if ( fRealloc && nObjsAlloc < nGC )
      {
	Realloc();
	return 0;
      }
    nRefresh += 1;
    if ( fGC && nRefresh <= 1 )
      {
	GarbageCollect();
	return 0;
      }
    if ( fRealloc )
      {
	Realloc();
	return 0;
      }
    throw "Node overflow";
  }
  
/**Function*************************************************************

   Synopsis    [Reallocate]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  bool UniqueResize()
  {
    if ( nUnique > (lit)BvarInvalid() )
      return 1;
    lit nUniqueOld = nUnique;
    nUnique = nUnique + nUnique;
    nUniqueMask = nUnique - 1;
    pUnique = (bvar *)realloc( pUnique, sizeof(bvar) * nUnique );
    if ( !pUnique )
      throw "Unique reallocation failed";
    memset( pUnique + nUniqueOld, 0, sizeof(bvar) * nUniqueOld );
    for ( lit i = 0; i < nUniqueOld; i++ )
      {
	bvar * q = pUnique + i;
	bvar * tail1 = q;
	bvar * tail2 = q + nUniqueOld;
	while ( *q )
	  {
	    lit hash = Hash( VarOfBvar( *q ), ThenOfBvar( *q ), ElseOfBvar( *q ) ) & nUniqueMask;
	    bvar * tail;
	    if ( hash == i )
	      tail = tail1;
	    else
	      tail = tail2;
	    if ( tail != q )
	      {
		*tail = *q;
		*q = 0;
	      }
	    q = pNexts + *tail;
	    if ( tail == tail1 )
	      tail1 = q;
	    else
	      tail2 = q;
	  }
      }
    return 0;
  }
  void Realloc()
  {
    lit nObjsAllocOld = nObjsAlloc;
    nObjsAlloc  = nObjsAlloc + nObjsAlloc;
    if ( !nObjsAlloc )
      throw "Node overflow";
    if ( nVerbose )
      std::cout << "\tReallocate " << nObjsAlloc << " nodes" << std::endl;
    pVars       = (var *)realloc( pVars, sizeof(var) * nObjsAlloc );
    pObjs       = (lit *)realloc( pObjs, sizeof(lit) * 2 * (size)nObjsAlloc );
    pMarks      = (mark *)realloc( pMarks, sizeof(mark) * nObjsAlloc );
    pNexts      = (bvar *)realloc( pNexts, sizeof(bvar) * nObjsAlloc );
    if ( !pVars || !pObjs || !pMarks || !pNexts )
      throw "Reallocation failed";
    memset( pVars + nObjsAllocOld, 0, sizeof(var) * nObjsAllocOld );
    memset( pObjs + 2 * (size)nObjsAllocOld, 0, sizeof(lit) * 2 * (size)nObjsAllocOld );
    memset( pMarks + nObjsAllocOld, 0, sizeof(mark) * nObjsAllocOld );
    memset( pNexts + nObjsAllocOld, 0, sizeof(bvar) * nObjsAllocOld );
    if ( pEdges )
      {
	pEdges = (edge *)realloc( pEdges, sizeof(edge) * nObjsAlloc );
	if ( !pEdges )
	  throw "Reallocation failed";
	memset ( pEdges + nObjsAllocOld, 0, sizeof(edge) * nObjsAllocOld );
      }
    while ( nUnique < nObjsAlloc * UniqueMinRate )
      if ( UniqueResize() )
	break;
  }
  
/**Function*************************************************************

   Synopsis    [Garbage collection]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  void RemoveBvar( bvar a )
  {
    bvar * q = pUnique + ( Hash( VarOfBvar( a ), ThenOfBvar( a ), ElseOfBvar( a ) ) & nUniqueMask );
    for ( ; *q; q = pNexts + *q )
      if ( *q == a )
	break;
    bvar * next = pNexts + *q;
    *q = *next;
    *next = 0;
    SetVarOfBvarRemoved( a );
    if ( nMinRemoved > (lit)a )
      nMinRemoved = a;
  }
  void GarbageCollect()
  {
    if ( nVerbose )
      std::cout <<  "\tGarbage collect" << std::endl;
    for ( lit x : *pvNodes )
      Mark_rec( x );
    for ( bvar a = nVars + 1; a < nObjs; a++ )
      if ( !MarkOfBvar( a ) && !BvarIsRemoved( a ) )
	RemoveBvar( a );
    for ( lit x : *pvNodes )
      Unmark_rec( x );
    CacheClear();
  }

/**Function*************************************************************
   
   Synopsis    [Print Ordering]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  void PrintOrdering( std::vector<var> & new2old )
  {
    std::cout << "Ordering :" << std::endl;
    for ( var i : new2old )
      std::cout << vOrdering[i] << ",";
    std::cout << std::endl << "----------" << std::endl;
  }
  
/**Function*************************************************************
   
   Synopsis    [Shift/Swap Bvar]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  void ShiftBvar( bvar a, int d )
  {
    var v = VarOfBvar( a );
    lit x1 = ThenOfBvar( a );
    lit x0 = ElseOfBvar( a );
    // remove
    lit hash = Hash( v, x1, x0 ) & nUniqueMask;
    bvar * q = pUnique + hash;
    bvar * next = pNexts + a;
    for ( ; *q; q = pNexts + *q )
      if ( *q == a )
	{
	  *q = *next;
	  break;
	}
    // change
    v += d;
    SetVarOfBvar( a, v );
    // register
    hash = Hash( v, x1, x0 ) & nUniqueMask;
    q = pUnique + hash;
    *next = *q;
    *q = a;
  }
  void SwapBvar( bvar a, bool fRestore )
  {
    var v = VarOfBvar( a );
    lit x1 = ThenOfBvar( a );
    lit x0 = ElseOfBvar( a );
    // new chlidren
    lit f00, f01, f10, f11;
    if ( Var( x1 ) == v || Var( x1 ) == v + 1 )
      {
	f11 = Then( x1 );
	f10 = Else( x1 );
      }
    else
      {
	f11 = x1;
	f10 = x1;
      }
    if ( Var( x0 ) == v || Var( x0 ) == v + 1 )
      {
	f01 = Then( x0 );
	f00 = Else( x0 );
      }
    else
      {
	f01 = x0;
	f00 = x0;
      }
    lit y1 = UniqueCreate( v + 1, f11, f01 );
    Ref( y1 );
    if ( !Edge( y1 ) && Var( y1 ) == v + 1 )
      {
	if ( !fRestore )
	  liveBvars[nVars + 1].push_back( Lit2Bvar( y1 ) );
	else
	  liveBvars[v + 1].push_back( Lit2Bvar( y1 ) );
	IncEdgeNonConst( f11 );
	IncEdgeNonConst( f01 );
      }
    IncEdgeNonConst( y1 );
    lit y0 = UniqueCreate( v + 1, f10, f00 );
    Pop();
    if ( !Edge( y0 ) && Var( y0 ) == v + 1 )
      {
	if ( !fRestore )
	  liveBvars[nVars + 1].push_back( Lit2Bvar( y0 ) );
	else
	  liveBvars[v + 1].push_back( Lit2Bvar( y0 ) );
	IncEdgeNonConst( f10 );
	IncEdgeNonConst( f00 );
      }
    IncEdgeNonConst( y0 );
    // remove
    lit hash = Hash( v, x1, x0 ) & nUniqueMask;
    bvar * q = pUnique + hash;
    bvar * next = pNexts + a;
    for ( ; *q; q = pNexts + *q )
    if ( *q == a )
      {
	*q = *next;
	break;
      }
    // change
    SetThenOfBvar( a, y1 );
    SetElseOfBvar( a, y0 );
    // register
    hash = Hash( v, y1, y0 ) & nUniqueMask;
    q = pUnique + hash;
    *next = *q;
    *q = a;
  }
  
/**Function*************************************************************
   
   Synopsis    [Shift/Swap layer]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  bool Swap( var v, bvar & nNodes, ssize dLimit )
  {
    liveBvars[nVars].clear();
    liveBvars[nVars + 1].clear();
    // walk upper level
    for ( bvar a : liveBvars[v] )
      if ( Var( ThenOfBvar( a ) ) == v + 1 ||
	   Var( ElseOfBvar( a ) ) == v + 1 )
	{
	  DecEdgeNonConst( ThenOfBvar( a ) );
	  DecEdgeNonConst( ElseOfBvar( a ) );
	}
      else
	{
	  ShiftBvar( a, 1 );
	  liveBvars[nVars + 1].push_back( a );
	}
    // walk lower level
    for ( bvar a : liveBvars[v + 1] )
      if ( !EdgeOfBvar( a ) )
	{
	  DecEdgeNonConst( ThenOfBvar( a ) );
	  DecEdgeNonConst( ElseOfBvar( a ) );
	}
      else
	{
	  ShiftBvar( a, -1 );
	  liveBvars[nVars].push_back( a );
	}
    bvar nSwapHead = liveBvars[nVars].size();
    bvar nOut = 0;
    // walk upper level again
    for ( bvar i = 0; i < (bvar)liveBvars[v].size(); i++ )
      {
	bvar a = liveBvars[v][i];
	if ( VarOfBvar( a ) == v )
	  {
	    SwapBvar( a, 0 );
	    liveBvars[nVars].push_back( a );
	    if ( (ssize)liveBvars[nVars].size()
		 + (ssize)liveBvars[nVars + 1].size()
		 - (ssize)liveBvars[v].size()
		 - (ssize)liveBvars[v + 1].size()
		 >
		 dLimit )
	      {
		nOut = i + 1;
		break;
	      }
	  }
      }
    if ( !nOut )
      {
	// swap liveBvars
	nNodes += (ssize)liveBvars[nVars].size() + liveBvars[nVars + 1].size() - liveBvars[v].size() - liveBvars[v + 1].size();
	std::iter_swap(liveBvars.begin() + v, liveBvars.begin() + nVars);
	std::iter_swap(liveBvars.begin() + v + 1, liveBvars.begin() + nVars + 1);
	return 0;
      }
    // restore previous tree
    std::vector<bvar> vTmp(liveBvars[v].begin() + nOut, liveBvars[v].end());
    liveBvars[v].clear();
    liveBvars[v + 1].clear();
    // walk new upper level where swapped
    for ( bvar i = nSwapHead; i < (bvar)liveBvars[nVars].size(); i++ )
      {
	bvar a = liveBvars[nVars][i];
	DecEdgeNonConst( ThenOfBvar( a ) );
	DecEdgeNonConst( ElseOfBvar( a ) );
      }
    // walk new lower level
    for ( bvar a : liveBvars[nVars + 1] )
      if ( !EdgeOfBvar( a ) )
	{
	  DecEdgeNonConst( ThenOfBvar( a ) );
	  DecEdgeNonConst( ElseOfBvar( a ) );
	}
      else
	{
	  ShiftBvar( a, -1 );
	  liveBvars[v].push_back( a );
	}
    // walk new upper level where shifted
    for ( bvar i = 0; i < nSwapHead; i++ )
      {
	bvar a = liveBvars[nVars][i];
	ShiftBvar( a, 1 );
	liveBvars[v + 1].push_back( a );
      }
    // walk old upper level from where out of nodes
    for ( bvar a : vTmp )
      {
	if ( Var( ThenOfBvar( a ) ) == v + 1 ||
	     Var( ElseOfBvar( a ) ) == v + 1 )
	  {
	    liveBvars[v].push_back( a );
	    lit x1 = ThenOfBvar( a );
	    if ( !Edge( x1 ) && Var( x1 ) == v + 1 )
	      {
		IncEdgeNonConst( Then( x1 ) );
		IncEdgeNonConst( Else( x1 ) );
		liveBvars[v + 1].push_back( Lit2Bvar( x1 ) );
	      }
	    IncEdgeNonConst( x1 );
	    lit x0 = ElseOfBvar( a );
	    if ( !Edge( x0 ) && Var( x0 ) == v + 1 )
	      {
		IncEdgeNonConst( Then( x0 ) );
		IncEdgeNonConst( Else( x0 ) );
		liveBvars[v + 1].push_back( Lit2Bvar( x0 ) );
	      }
	    IncEdgeNonConst( x0 );
	  }
      }
    // walk new upper level where swapped
    for ( bvar i = nSwapHead; i < (bvar)liveBvars[nVars].size(); i++ )
      {
	bvar a = liveBvars[nVars][i];
	SwapBvar( a, 1 );
	liveBvars[v].push_back( a );
      }
    return -1;
  }
  void Shift( var & pos, bvar & nNodes, var nSwap, bool fUp, var & bestPos, bvar & nBestNodes, std::vector<var> & new2old, size nLimit )
  {
    bool fReo_ = fReo;
    int nRefresh_ = nRefresh;
    fReo = 0;
    for ( var i = 0; i < nSwap; i++ )
      {
	ssize dLimit = nLimit - nNodes;
	nRefresh = 0;
	if ( fUp )
	  pos -= 1;
	if ( Swap( pos, nNodes, dLimit ) )
	  {
	    if ( fUp )
	      pos += 1;
	    fReo = fReo_;
	    nRefresh = nRefresh_;
	    return;
	  }
	std::swap( new2old[pos], new2old[pos + 1] );
	if ( !fUp )
	  pos += 1;
	if ( nNodes <= nBestNodes )
	  {
	    nBestNodes = nNodes;
	    bestPos = pos;
	  }
	if ( nVerbose >= 2 )
	  {
	    std::cout << "pos = " << pos << " nNode = " << nNodes << std::endl;
	    PrintOrdering( new2old );
	  }
      }
    fReo = fReo_;
    nRefresh = nRefresh_;
  }

/**Function*************************************************************
   
   Synopsis    [Reorder]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  void Reorder()
  {
    if ( nVerbose )
      std::cout << "\tReordering" << std::endl;
    // initialize
    pEdges = (edge *)calloc( nObjsAlloc, sizeof(edge) );
    if ( !pEdges )
      throw "Allocation failed";
    liveBvars.resize( nVars + 2 );
    for ( var v = 0; v <= nVars + 1; v++ )
      {
	liveBvars[v].clear();
	liveBvars[v].reserve( nObjs / nVars );
      }
    CountEdgeAndBvar();
    std::vector<var> descendingOrder;
    std::vector<var> new2old;
    for ( var v = 0; v < nVars; v++ )
      {
	new2old.push_back( v );
	descendingOrder.push_back( v );
      }
    std::sort( descendingOrder.begin(), descendingOrder.end(), [&]( var v, var u )
      {
	return liveBvars[v].size() > liveBvars[u].size();
      });
    bvar nNodes = 0;
    for ( var v = 0; v < nVars; v++ )
      nNodes += liveBvars[v].size();
    size nLimit = (size)nNodes * MaxGrowth + nNodes;
    if ( nVerbose  >= 2 )
      {
	std::cout << "nNode for each level :" << std::endl;
	for ( var v = 0; v < nVars; v++ )
	  std::cout << liveBvars[v].size() << ",";
	std::cout << std::endl;
	PrintOrdering( new2old );
      }
    // shift
    for ( var i = 0; i < nVars; i++ )
      {
	var pos = std::distance( new2old.begin(), std::find( new2old.begin(), new2old.end(), descendingOrder[i] ) );
	bool fUp = 0;
	var nSwap;
	var bestPos = pos;
	bvar nBestNodes = nNodes;
	if ( nVerbose >= 2 )
	  std::cout << "\tBegin shift " << (size)vOrdering[descendingOrder[i]] << " ( " << (size)i + 1 << " / " << (size)nVars << " )" << std::endl;
	if( pos < nVars >> 1 )
	  {
	    fUp ^= 1;
	    nSwap = pos;
	  }
	else
	  nSwap = nVars - pos - 1;
	Shift( pos, nNodes, nSwap, fUp, bestPos, nBestNodes, new2old, nLimit );
	fUp ^= 1;
	if ( fUp )
	  nSwap = pos;
	else
	  nSwap = nVars - pos - 1;
	Shift( pos, nNodes, nSwap, fUp, bestPos, nBestNodes, new2old, nLimit );
	if ( pos < bestPos )
	  {
	    fUp = 0;
	    nSwap = bestPos - pos;
	  }
	else
	  {
	    fUp = 1;
	    nSwap = pos - bestPos;
	  }
	Shift( pos, nNodes, nSwap, fUp, bestPos, nBestNodes, new2old, nLimit );
      }
    // finish
    std::vector<var> vTmp( vOrdering );
    vOrdering.clear();
    for ( var i : new2old )
      vOrdering.push_back( vTmp[i] );
    free( pEdges );
    pEdges = NULL;
    liveBvars.clear();
    CacheClear();
  }
};

}

#endif
