#ifndef AT_BDD_HPP_
#define AT_BDD_HPP_

#include <vector>
#include <cassert>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <cstring>

namespace AtBdd
{
/**Function*************************************************************

   Synopsis    [Data type]

   Description [var = Variable, lit = Literal, bvar = BddVariable = Literal >> 1]

   SideEffects []

   SeeAlso     []

***********************************************************************/
  typedef uint32_t lit;
  typedef int bvar; // signed lit
  typedef uint8_t mark;
  typedef uint32_t edge;
  
/**Function*************************************************************
   
   Synopsis    [Hash]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  lit Hash( lit Arg0, lit Arg1, lit Arg2 ) { return 12582917 * Arg0 + 4256249 * Arg1 + 741457 * Arg2; }
  //  lit Hash( lit Arg0, lit Arg1, lit Arg2 ) { return 67280421310721ull * Arg0 + 2147483647ull * Arg1 + 12582917 * Arg2; }

/**Function*************************************************************
   
   Synopsis    [Wrapper class]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/

class BddManWrap
{
public:
  virtual ~BddManWrap() {};
  virtual int  get_nVars() = 0;
  virtual int  get_order( int v ) = 0;
  virtual int  get_pvNodesExists() = 0;
  virtual void Ref( lit x ) = 0;
  //  virtual void Pop() = 0;
  virtual void Deref( lit x ) = 0;
  //  virtual bvar BvarConst() = 0;
  //  virtual int  VarInvalid_() = 0;
  //  virtual bvar BvarInvalid() = 0;
  //  virtual edge EdgeInvalid() = 0;
  //  virtual mark MarkInvalid() = 0;
  //  virtual lit  Bvar2Lit( bvar a, int c ) = 0;
  //  virtual bvar Lit2Bvar( lit x ) = 0;
  //  virtual bvar BvarIthVar_( int v ) = 0;
  //  virtual int  BvarIsEq( bvar a, bvar b ) = 0;
  //  virtual int  BvarIsConst( bvar a ) = 0;
  //  virtual int  BvarIsInvalid( bvar a ) = 0;
  //  virtual int  VarOfBvar_( bvar a ) = 0;
  //  virtual lit  ThenOfBvar( bvar a ) = 0;
  //  virtual lit  ElseOfBvar( bvar a ) = 0;
  //  virtual bvar NextOfBvar( bvar a ) = 0;
  //  virtual mark MarkOfBvar( bvar a ) = 0;
  //  virtual edge EdgeOfBvar( bvar a ) = 0;
  //  virtual void SetVarOfBvar_( bvar a, int v ) = 0;
  //  virtual void SetThenOfBvar( bvar a, lit x1 ) = 0;
  //  virtual void SetElseOfBvar( bvar a, lit x0 ) = 0;
  //  virtual void SetNextOfBvar( bvar a, bvar b ) = 0;
  //  virtual void SetMarkOfBvar( bvar a, mark m ) = 0;
  //  virtual void SetEdgeOfBvar( bvar a, edge e ) = 0;
  //  virtual int  BvarIsRemoved( bvar a ) = 0;
  //  virtual void SetVarOfBvarRemoved( bvar a ) = 0;
  //  virtual int  BvarIsVar( bvar a ) = 0;
  virtual lit  LitRegular( lit x ) = 0;
  virtual lit  LitNot( lit x ) = 0;
  //  virtual lit  LitNotCond( lit x, int c ) = 0;
  virtual lit  LitConst0() = 0;
  virtual lit  LitConst1() = 0;
  //  virtual lit  LitInvalid() = 0;
  virtual lit  LitIthVar_( int v ) = 0;
  virtual int  LitIsCompl( lit x ) = 0;
  //  virtual int  LitIsEq( lit x, lit y ) = 0;
  //  virtual int  LitIsConst0( lit x ) = 0;
  //  virtual int  LitIsConst1( lit x ) = 0;
  //  virtual int  LitIsConst( lit x ) = 0;
  //  virtual int  LitIsInvalid( lit x ) = 0;
  //  virtual int  LitIsRemoved( lit x ) = 0;
  //  virtual int  LitIsVar( lit x )     = 0;
  virtual int  Var_( lit x ) = 0;
  virtual lit  Then( lit x ) = 0;
  virtual lit  Else( lit x ) = 0;
  //  virtual bvar Next( lit x ) = 0;
  //  virtual bvar Mark( lit x ) = 0;
  //  virtual edge Edge( lit x ) = 0;
  //  virtual void SetMark( lit x, mark m ) = 0;
  //  virtual void IncMark( lit x ) = 0;
  //  virtual void DecMark( lit x ) = 0;
  //  virtual void IncEdge( lit x ) = 0;
  //  virtual void DecEdge( lit x ) = 0;
  //  virtual void IncEdgeNonConst( lit x ) = 0;
  //  virtual void DecEdgeNonConst( lit x ) = 0;
  //  virtual int  IsLimit() = 0;
  //  virtual void Mark_rec( lit x ) = 0;
  //  virtual void Unmark_rec( lit x ) = 0;
  //  virtual uint64_t Count_rec( lit x ) = 0;
  virtual uint64_t CountNodes( lit x ) = 0;
  virtual uint64_t CountNodesArrayShared( std::vector<lit> & vNodes ) = 0;
  //  virtual uint64_t CountNodesArrayIndependent( std::vector<lit> & vNodes ) = 0;
  //  virtual void CountEdge_rec( lit x ) = 0;
  //  virtual void CountEdge( std::vector<lit> & vNodes ) = 0;
  //  virtual void UncountEdge_rec( lit x ) = 0;
  //  virtual void UncountEdge( std::vector<lit> & vNodes ) = 0;
  //  virtual void CountEdgeAndBvar_rec( lit x ) = 0;
  //  virtual void CountEdgeAndBvar( std::vector<lit> & vNodes ) = 0;
  //  virtual lit UniqueCreateInt_( int v, lit x1, lit x0 ) = 0;
  //  virtual lit UniqueCreate_( int v, lit x1, lit x0 ) = 0;
  //  virtual lit CacheLookup( lit Arg1, lit Arg2 ) = 0;
  //  virtual lit CacheInsert( lit Arg1, lit Arg2, lit Res ) = 0;
  //  virtual void CacheClear() = 0;
  //  virtual lit And_rec( lit x, lit y ) = 0;
  virtual lit And( lit x, lit y ) = 0;
  //  virtual lit Or( lit x, lit y ) = 0;
  //  virtual lit Xnor( lit x, lit y ) = 0;
  virtual void RefreshConfig( int fRealloc_, int fGC_, int nMaxGrowth ) = 0;
  //  virtual int Refresh() = 0;
  //  virtual void Rehash() = 0;
  //  virtual void Realloc() = 0;
  //  virtual void RemoveBvar( bvar a ) = 0;
  //  virtual void GarbageCollect() = 0;
  //  virtual void PrintOrdering( std::vector<var> & new2old ) = 0;
  //  virtual void ShiftBvar( bvar a, int d ) = 0;
  //  virtual void SwapBvar( bvar a, int fRestore ) = 0;
  //  virtual int Swap_( int v, bvar & nNodes, int64_t dLimit ) = 0;
  //  virtual void Shift_( int & pos, bvar & nNodes, int nSwap, int fUp, int & bestPos, bvar & nBestNodes, std::vector<var> & new2old, uint64_t nLimit ) = 0;
  //  virtual void Reorder() = 0;
};
/**Function*************************************************************
   
   Synopsis    [Class]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  
template <typename var = uint8_t>
class BddMan : public BddManWrap
{
private:
  var    nVars;         // the number of variables
  bvar   nObjs;         // the number of nodes used
  lit    nObjsAlloc;    // the number of nodes allocated
  lit    nCache;        // the number of cache spots
  bvar * pUnique;       // unique table for nodes
  bvar * pNexts;        // next pointer for nodes
  lit *  pCache;        // array of triples <arg0, arg1, AND(arg0, arg1)>
  lit *  pObjs;         // array of pairs cof0 for nodes
  var *  pVars;         // array of variables for nodes
  mark * pMarks;        // array of marks for nodes
  lit    nUniqueMask;   // selection mask for unique table
  lit    nCacheMask;    // selection mask for computed table
  lit    nMinRemoved;   // the minimum int of removed nodes

  uint64_t nCacheHit;
  uint64_t nCacheFind;
  uint64_t nCall;
  uint64_t nCallThold;
  double   HitRateOld;
  
  int    nRefresh;      // the number of refresh tried
  int    fGC;           // flag of garbage collection
  int    fRealloc;      // flag of reallocation
  uint64_t nReo;          // threshold to run reordering
  double MaxGrowth;     // threshold to terminate reordering. 0=off
  edge * pEdges;        // array of number of incoming edges for nodes
  
  std::vector<var>                vOrdering; // variable ordering : new 2 old
  std::vector<std::vector<bvar> > liveBvars; // array of live Bvars for each layer
  std::vector<lit> *              pvNodes;   // vector of live nodes (only top of tree)
  
  int    nVerbose;      // the level of verbosing information
  
public:
  int  get_nVars() { return nVars; }
  int  get_order( int v ) { return vOrdering[v]; }
  int  get_pvNodesExists() { return pvNodes != NULL; }
  lit  LitIthVar_( int v ) { return LitIthVar( (var)v ); }
  int  Var_( lit x ) { return Var( x ); }
  
/**Function*************************************************************
   
   Synopsis    [Reference]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  void Ref( lit x ) { if ( pvNodes ) pvNodes->push_back( x ); }
  void Pop()        { if ( pvNodes ) pvNodes->pop_back();     }
  void Deref( lit x ) {
    if ( pvNodes )
      {
	auto it = std::find( pvNodes->begin(), pvNodes->end(), x );
	assert( it != pvNodes->end() );
	pvNodes->erase( it );
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
  lit  Bvar2Lit( bvar a, int c ) { return a + a + (int)( c > 0 ); }
  bvar Lit2Bvar( lit x )         { return x >> 1;                 }

/**Function*************************************************************
   
   Synopsis    [Utilities for Bvar]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  bvar BvarIthVar( var v )        { return v + 1;                        }
  int  BvarIsEq( bvar a, bvar b ) { return a == b;                       }
  int  BvarIsConst( bvar a )      { return BvarIsEq( a, BvarConst() );   }
  int  BvarIsInvalid( bvar a )    { return BvarIsEq( a, BvarInvalid() ); }

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

  int  BvarIsRemoved( bvar a )       { return VarOfBvar( a ) == VarInvalid();        }
  void SetVarOfBvarRemoved( bvar a ) { SetVarOfBvar( a, VarInvalid() );              }
  int  BvarIsVar( bvar a )           { return a <= (bvar)nVars && !BvarIsConst( a ); }
  
/**Function*************************************************************
   
   Synopsis    [Utilities for Lit]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  lit  LitRegular( lit x )        { return x & ~01;                        }
  lit  LitNot( lit x )            { return x ^ 1;                          }
  lit  LitNotCond( lit x, int c ) { return x ^ (int)( c > 0 );             }
  lit  LitConst0()                { return Bvar2Lit( BvarConst(), 0 );     }
  lit  LitConst1()                { return LitNot( LitConst0() );          }
  lit  LitInvalid()               { return Bvar2Lit( BvarInvalid(), 0 );   }
  lit  LitIthVar( var v )         { return Bvar2Lit( BvarIthVar( v ), 0 ); }
  int  LitIsCompl( lit x )        { return x & 1;                          }
  int  LitIsEq( lit x, lit y )    { return x == y;                         }
  int  LitIsConst0( lit x )       { return LitIsEq( x, LitConst0() );      }
  int  LitIsConst1( lit x )       { return LitIsEq( x, LitConst1() );      }
  int  LitIsConst( lit x )        { return BvarIsConst( Lit2Bvar( x ) );   }
  int  LitIsInvalid( lit x )      { return BvarIsInvalid( Lit2Bvar( x ) ); }
  int  LitIsRemoved( lit x )      { return Var( x ) == VarInvalid();       }
  int  LitIsVar( lit x )          { return BvarIsVar( Lit2Bvar( x ) );     }
  
  var  Var( lit x )  { return VarOfBvar( Lit2Bvar( x ) ); }
  lit  Then( lit x ) { return LitNotCond( pObjs[LitRegular( x )], LitIsCompl( x ) ); }
  lit  Else( lit x ) { return LitNotCond( pObjs[LitNot( LitRegular( x ) )], LitIsCompl( x ) ); }
  bvar Next( lit x ) { return NextOfBvar( Lit2Bvar( x ) ); }
  bvar Mark( lit x ) { return MarkOfBvar( Lit2Bvar( x ) ); }
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
  int  IsLimit() { return (lit)nObjs == nObjsAlloc || nObjs == BvarInvalid(); }
  
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
  uint64_t Count_rec( lit x )
  {
    if ( /*LitIsConst( x ) ||*/ Mark( x ) )
      return 0;
    SetMark( x, 1 );
    return 1 + Count_rec( Else( x ) ) + Count_rec( Then( x ) );
  }
  uint64_t CountNodes( lit x )
  {
    uint64_t count = Count_rec( x );
    Unmark_rec( x );
    SetMark( 0, 0 );
    return count;
  }
  uint64_t CountNodesArrayShared( std::vector<lit> & vNodes )
  {
    uint64_t count = 0;
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
  uint64_t CountNodesArrayIndependent( std::vector<lit> & vNodes )
  {
    uint64_t count = 0;
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
  void CountEdge( std::vector<lit> & vNodes )
  {
    for ( lit x : vNodes )
      CountEdge_rec( x );
    for ( var v = 0; v < nVars; v++ )
      CountEdge_rec( LitIthVar( v ) );
    for ( lit x : vNodes )
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
  void UncountEdge( std::vector<lit> & vNodes )
  {
    for ( lit x : vNodes )
    UncountEdge_rec( x );
    for ( var v = 0; v < nVars; v++ )
      UncountEdge_rec( LitIthVar( v ) );
    for ( lit x : vNodes )
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
  void CountEdgeAndBvar( std::vector<lit> & vNodes )
  {
    for ( lit x : vNodes )
      CountEdgeAndBvar_rec( x );
    for ( var v = 0; v < nVars; v++ )
      CountEdgeAndBvar_rec( LitIthVar( v ) );
    for ( lit x : vNodes )
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
  BddMan( var nVars, lit nObjsAlloc_, lit nCache_, std::vector<var> * pvOrdering, int nVerbose ) : nVars(nVars), nObjsAlloc(nObjsAlloc_), nCache(nCache_), nVerbose(nVerbose)
  {
    if ( nVars == VarInvalid() )
      throw "Varable overflow";
    while ( nObjsAlloc < (lit)nVars + 1 )
      {
	if ( !nObjsAlloc || nObjsAlloc > (lit)BvarInvalid() )
	  throw "Node overflow just for Variables\n";
	nObjsAlloc = nObjsAlloc + nObjsAlloc;
      }
    if ( nVerbose )
      std::cout << "Allocate " << nObjsAlloc << " nodes" << std::endl;
    nRefresh    = 0;
    fRealloc    = 0;
    fGC         = 0;
    MaxGrowth   = 0;
    nReo        = 4000;
    nCacheHit   = 0;
    nCacheFind  = 0;
    nCall       = 0;
    nCallThold  = 1000000;
    HitRateOld  = 0;
    nMinRemoved = nObjsAlloc;
    nUniqueMask = ( (lit)1 << (int)log2( nObjsAlloc ) ) - 1;
    nCacheMask  = ( (lit)1 << (int)log2( nCache ) ) - 1;
    pVars       = (var *)calloc( nObjsAlloc, sizeof(var) );
    pUnique     = (bvar *)calloc( nUniqueMask + 1, sizeof(bvar) );
    pNexts      = (bvar *)calloc( nUniqueMask + 1, sizeof(bvar) );
    pCache      = (lit *)calloc( 3 * (uint64_t)( nCacheMask + 1 ), sizeof(lit) );
    pObjs       = (lit *)calloc( 2 * (uint64_t)nObjsAlloc, sizeof(lit) );
    pMarks      = (mark *)calloc( nObjsAlloc, sizeof(mark) );
    if ( !pVars || !pUnique || !pNexts || !pCache || !pObjs || !pMarks )
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
    for ( var v = 0; v < nVars; v++ ) {
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
	  std::cout << "Free : Var = " << (uint64_t)nVars << " Obj = " << (uint64_t)nObjs << " Alloc = " << (uint64_t)BvarInvalid() << std::endl;
	else
	  std::cout << "Free : Var = " << (uint64_t)nVars << " Obj = " << (uint64_t)nObjs << " Alloc = " << (uint64_t)nObjsAlloc << std::endl;
      }
    free( pUnique );
    free( pNexts );
    free( pCache );
    free( pObjs );
    if ( pVars )
      free( pVars );
    if ( pvNodes )
      delete pvNodes;
    if ( pEdges )
      free( pEdges );
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
      std::cout << "Add " << (uint64_t)*q << " : Var = " << (uint64_t)v << " Then = " << (uint64_t)x1 << " Else = " << (uint64_t)x0 << " MinRemoved = " << (uint64_t)nMinRemoved << std::endl;
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
    lit * p = pCache + 3 * (uint64_t)( Hash( 0, Arg1, Arg2 ) & nCacheMask );
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
    lit * p = pCache + 3 * (uint64_t)( Hash( 0, Arg1, Arg2 ) & nCacheMask );
    p[0] = Arg1;
    p[1] = Arg2;
    p[2] = Res;
    return Res;
  }
  void CacheResize()
  {
    if ( nCache > (lit)BvarInvalid() )
      return;
    lit nCacheMaskOld = nCacheMask;
    nCache = nCache + nCache;
    nCacheMask = ( (lit)1 << (int)log2( nCache ) ) - 1;
    pCache = (lit *)realloc( pCache, sizeof(lit) * 3 * (uint64_t)( nCacheMask + 1 ) );
    memset( pCache + 3 * (uint64_t)( nCacheMaskOld + 1 ), 0, sizeof(lit) * 3 * (uint64_t)( nCacheMaskOld + 1 ) );
    if ( !pCache )
      throw "Allocation failed";
    for ( lit i = 0; i <= nCacheMaskOld; i++ )
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
    if ( HitRateOld && HitRate > HitRateOld )
      CacheResize();
    HitRateOld = HitRate;
    nCallThold = nCallThold + nCallThold;
  }
  void CacheClear()
  {
    free( pCache );
    pCache = (lit *)calloc( 3 * (uint64_t)( nCacheMask + 1 ), sizeof(lit) );
    nCacheHit   = 0;
    nCacheFind  = 0;
    nCall       = 0;
    nCallThold  = 1000000;
    HitRateOld  = 0;
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
  lit Or( lit x, lit y )
  {
    return LitNot( And_rec( LitNot( x ), LitNot( y ) ) );
  }
  lit Xnor( lit x, lit y )
  {
    lit z1 = And( x, y );
    Ref( z1 );
    lit z0 = And( LitNot( x ), LitNot( y ) );
    Ref( z0 );
    lit z = Or( z0, z1 );
    Pop();
    Pop();
    return z;
  }
  
/**Function*************************************************************

   Synopsis    [Refresh]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  void RefreshConfig( int fRealloc_, int fGC_, int nMaxGrowth )
  {
    fRealloc = fRealloc_;
    fGC = fGC_;
    MaxGrowth = 0.01 * nMaxGrowth;
    if ( pvNodes )
      delete pvNodes;
    if ( pEdges )
      free( pEdges );
    if ( liveBvars.size() )
      liveBvars.clear();
    if ( fGC || MaxGrowth )
      pvNodes = new std::vector<lit>;
    if ( MaxGrowth )
      {
	pEdges = (edge *)calloc( nObjsAlloc, sizeof(edge) );
	if ( !pEdges )
	  throw "Allocation failed";
	liveBvars.resize( nVars + 2 );
      }
  }
  int Refresh()
  {
    nRefresh += 1;
    if ( nVerbose )
      std::cout << "Refresh " << nRefresh << std::endl;
    if ( nRefresh <= 1 && fGC )
      {
	GarbageCollect();
	return 0;
      }
    if ( nRefresh <= 2 && MaxGrowth && (uint64_t)nObjs > nReo )
      {
	Reorder();
	nReo = nReo + nReo;
	return -1;
      }
    if ( fRealloc && nObjsAlloc <= (lit)BvarInvalid() )
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
  void Rehash()
  {
    lit nUniqueMaskOld = nUniqueMask >> 1; // assuming it has been doubled
    for ( lit i = 0; i <= nUniqueMaskOld; i++ )
      {
	bvar * q = pUnique + i;
	bvar * tail1 = q;
	bvar * tail2 = q + nUniqueMaskOld + 1;
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
  }
  void Realloc()
  {
    lit nObjsAllocOld = nObjsAlloc;
    lit nUniqueMaskOld = nUniqueMask;
    assert( nObjsAlloc <= (lit)BvarInvalid() );
    nObjsAlloc  = nObjsAlloc + nObjsAlloc;
    if ( nVerbose )
      std::cout << "\tReallocate " << nObjsAlloc << " nodes" << std::endl;
    nUniqueMask = ( 1 << (int)log2( nObjsAlloc ) ) - 1;
    assert( ((nUniqueMaskOld << 1) ^ 01) == nUniqueMask );
    pVars       = (var *)realloc( pVars, sizeof(var) * nObjsAlloc );
    pUnique     = (bvar *)realloc( pUnique, sizeof(bvar) * ( nUniqueMask + 1 ) );
    pNexts      = (bvar *)realloc( pNexts, sizeof(bvar) * ( nUniqueMask + 1 ) );
    pObjs       = (lit *)realloc( pObjs, sizeof(lit) * 2 * (uint64_t)nObjsAlloc );
    pMarks      = (mark *)realloc( pMarks, sizeof(mark) * nObjsAlloc );
    if ( !pVars || !pUnique || !pNexts || !pObjs || !pMarks )
      throw "Reallocation failed";
    memset( pVars + nObjsAllocOld, 0, sizeof(var) * nObjsAllocOld );
    memset( pUnique + ( nUniqueMaskOld + 1 ), 0, sizeof(bvar) * ( nUniqueMaskOld + 1 ) );
    memset( pNexts + ( nUniqueMaskOld + 1 ), 0, sizeof(bvar) * ( nUniqueMaskOld + 1 ) );
    memset( pObjs + 2 * (uint64_t)nObjsAllocOld, 0, sizeof(lit) * 2 * (uint64_t)nObjsAllocOld );
    memset( pMarks + nObjsAllocOld, 0, sizeof(mark) * nObjsAllocOld );
    Rehash();
    if ( pEdges )
      {
	pEdges = (edge *)realloc( pEdges, sizeof(edge) * nObjsAlloc );
	if ( !pEdges )
	  throw "Reallocation failed";
	memset ( pEdges + nObjsAllocOld, 0, sizeof(edge) * nObjsAllocOld );
      }
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
  void SwapBvar( bvar a, int fRestore )
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
  int Swap( var v, bvar & nNodes, int64_t dLimit )
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
	    if ( (int64_t)liveBvars[nVars].size()
		 + (int64_t)liveBvars[nVars + 1].size()
		 - (int64_t)liveBvars[v].size()
		 - (int64_t)liveBvars[v + 1].size()
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
	nNodes += (int64_t)liveBvars[nVars].size() + liveBvars[nVars + 1].size() - liveBvars[v].size() - liveBvars[v + 1].size();
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
  void Shift( var & pos, bvar & nNodes, var nSwap, int fUp, var & bestPos, bvar & nBestNodes, std::vector<var> & new2old, uint64_t nLimit )
  {
    double MaxGrowth_ = MaxGrowth;
    int nRefresh_ = nRefresh;
    MaxGrowth = 0;
    for ( var i = 0; i < nSwap; i++ )
      {
	int64_t dLimit = nLimit - nNodes;
	nRefresh = 0;
	if ( fUp )
	  pos -= 1;
	if ( Swap( pos, nNodes, dLimit ) )
	  {
	    if ( fUp )
	      pos += 1;
	    MaxGrowth = MaxGrowth_;
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
    MaxGrowth = MaxGrowth_;
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
    std::vector<var> descendingOrder;
    std::vector<var> new2old;
    if ( nVerbose )
      std::cout << "\tReordering" << std::endl;
    // initialize
    for ( var v = 0; v <= nVars + 1; v++ )
      {
	liveBvars[v].clear();
	liveBvars[v].reserve( nObjs / nVars );
      }
    CountEdgeAndBvar( *pvNodes );
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
    uint64_t nLimit = (uint64_t)nNodes * MaxGrowth + nNodes;
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
	int fUp = 0;
	var nSwap;
	var bestPos = pos;
	bvar nBestNodes = nNodes;
	if ( nVerbose >= 2 )
	  std::cout << "\tBegin shift " << (uint64_t)vOrdering[descendingOrder[i]] << " (" << (uint64_t)i + 1 << "/" << (uint64_t)nVars << std::endl;
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
    UncountEdge( *pvNodes ); // may slow it
    CacheClear();
  }
};

}

#endif
