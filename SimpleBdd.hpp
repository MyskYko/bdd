#ifndef SIMPLE_BDD
#define SIMPLE_BDD

#include <vector>
#include <cassert>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <cstring>

namespace SimpleBdd
{
class BddMan
{
/**Function*************************************************************
   
   Synopsis    [Data type]

   Description [var = Variable, lit = Literal, bvar = BddVariable = Literal >> 1]
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  typedef uint32_t lit;
  typedef int bvar;
  typedef uint8_t var;
  typedef uint8_t mark;
  typedef uint32_t edge;
  
/**Function*************************************************************
   
   Synopsis    [Hash]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  uint32_t Hash( uint32_t Arg0, uint32_t Arg1, uint32_t Arg2 ) { return 12582917 * Arg0 + 4256249 * Arg1 + 741457 * Arg2; }
  //  uint64_t Hash( uint64_t Arg0, uint64_t Arg1, uint64_t Arg2 ) { return 67280421310721ull * Arg0 + 2147483647ull * Arg1 + 12582917 * Arg2; }
  
/**Function*************************************************************
   
   Synopsis    [Member variables]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
private:
  var    nVars;         // the number of variables
  bvar   nObjs;         // the number of nodes used
  lit    nObjsAlloc;    // the number of nodes allocated
  bvar * pUnique;       // unique table for nodes
  bvar * pNexts;        // next pointer for nodes
  lit *  pCache;        // array of triples <arg0, arg1, AND(arg0, arg1)>
  lit *  pObjs;         // array of pairs cof0 for nodes
  var *  pVars;         // array of variables for nodes
  mark * pMarks;        // array of marks for nodes
  lit    nUniqueMask;   // selection mask for unique table
  lit    nCacheMask;    // selection mask for computed table
  bvar   nMinRemoved;   // the minimum int of removed nodes
  int    nVerbose;      // the level of verbosing information

  int    nRefresh;      // the number of refresh tried
  int    fGC;           // flag of garbage collection
  int    fRealloc;      // flag of reallocation
  int    nReo;          // threshold to run reordering
  double MaxGrowth;     // threshold to terminate reordering. 0=off
  edge * pEdges;        // array of number of incoming edges for nodes
  
  std::vector<var>                vOrdering; // variable ordering : new 2 old
  std::vector<std::vector<bvar> > liveBvars; // array of live Bvars for each layer
  std::vector<lit> *              pvNodes;   // vector of live nodes (only top of tree)
  
public:
  var  get_nVars()         { return nVars;           }
  var  get_order( var v )  { return vOrdering[v];    }
  int  get_pvNodesExists() { return pvNodes != NULL; }

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
  var  VarConst()    { return std::numeric_limits<var>::max();  }
  bvar BvarInvalid() { return std::numeric_limits<bvar>::max(); }
  edge EdgeInvalid() { return std::numeric_limits<edge>::max(); }
  mark MarkInvalid() { return std::numeric_limits<mark>::max(); }
  var  VarRemoved()  { return std::numeric_limits<var>::max();  }
  
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

  int  BvarIsRemoved( bvar a )  { return VarOfBvar( a ) == VarRemoved();        }
  void SetBvarRemoved( bvar a ) { SetVarOfBvar( a, VarRemoved() );              }
  int  BvarIsVar( bvar a )      { return a <= (bvar)nVars && !BvarIsConst( a ); }
  
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
  int  LitIsRemoved( lit x )      { return Var( x ) == VarRemoved();       }
  
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
   
   Synopsis    [Functions]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  BddMan( var nVars, lit nObjsAlloc_, std::vector<var> * pvOrdering, int nVerbose );
  ~BddMan();
  
  lit  UniqueCreateInt( var v, lit x1, lit x0 );
  lit  UniqueCreate( var v, lit x1, lit x0 );
  
  lit  CacheLookup( lit Arg1, lit Arg2 );
  lit  CacheInsert( lit Arg1, lit Arg2, lit Res );
  void CacheClear();
  
  lit  And_rec( lit x, lit y );
  lit  And( lit x, lit y );
  lit  Or( lit x, lit y );
  lit  Xnor( lit x, lit y );
  
  void Rehash();
  void Realloc();
  
  void RemoveNodeByBvar( bvar a );
  void GarbageCollect();
  
  void ShiftBvar( bvar a, int d );
  void SwapBvar( bvar a, int fRestore );
  int  Swap( var v, bvar & nNodes, lit dLimit );
  void Shift( var & pos, bvar & nNodes, var nSwap, int fUp, var & bestPos, bvar & nBestNodes, std::vector<var> & new2old, lit nLimit );
  void Reorder();
  
  void RefreshConfig( int fRealloc_, int fGC_, int nMaxGrowth );
  int  Refresh();
  
  int  Count_rec( lit x );
  void Mark_rec( lit x );
  void Unmark_rec( lit x );
  void CountEdge_rec( lit x );
  void CountEdge( std::vector<lit> & vNodes );
  void UncountEdge_rec( lit x );
  void UncountEdge( std::vector<lit> & vNodes );
  void CountEdgeAndBvar_rec( lit x );
  void CountEdgeAndBvar( std::vector<lit> & vNodes );
  //  void CheckLiveBvar_rec( unsigned x );
  //  void CheckLiveBvar();
  int  CountNodes( lit x );
  int  CountNodesArrayShared( std::vector<lit> & vNodes );
  int  CountNodesArrayIndependent( std::vector<lit> & vNodes );
  void PrintOrdering( std::vector<var> & new2old );
};

/**Function*************************************************************
   
   Synopsis    [Create new node]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
inline unsigned BddMan::UniqueCreateInt( var v, unsigned x1, unsigned x0 )
{
  int * q = pUnique + ( Hash( v, x1, x0 ) & nUniqueMask );
  for ( ; *q; q = pNexts + *q )
    if ( VarOfBvar( *q ) == v &&
	 ThenOfBvar( *q ) == x1 &&
	 ElseOfBvar( *q ) == x0 )
      return Bvar2Lit( *q, 0 );
  q = pUnique + ( Hash( v, x1, x0 ) & nUniqueMask );
  int head = *q;
  if ( IsLimit() )
    {
      for ( ; nMinRemoved < (unsigned)nObjs; nMinRemoved++ )
	if ( BvarIsRemoved( nMinRemoved ) )
	  break;
      if ( nMinRemoved == nObjs )
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
    std::cout << "Add " << *q << " : Var = " << v << " Then = " << x1 << " Else = " << x0 << " MinRemoved = " << nMinRemoved << std::endl;
  return Bvar2Lit( *q, 0 );
}
  
inline unsigned BddMan::UniqueCreate( var v, lit x1, lit x0 )
{
  if ( LitIsEq( x1, x0 ) )
    return x0;
  unsigned x;
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
 
   Synopsis    [Allocation/free]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
BddMan::BddMan( var nVars, lit nObjsAlloc_, std::vector<var> * pvOrdering, int nVerbose ) : nVars(nVars), nObjsAlloc(nObjsAlloc_), nVerbose(nVerbose)
{
  while ( nObjsAlloc < nVars + 1 )
    {
      if ( !nObjsAlloc || nObjsAlloc > 1 << 31 )
	throw "Node overflow just for Variables\n";
      nObjsAlloc = nObjsAlloc + nObjsAlloc;
    }
  if ( nVerbose )
    std::cout << "Allocate " << nObjsAlloc << " nodes" << std::endl;
  nUniqueMask = ( 1 << (int)log2( nObjsAlloc ) ) - 1;
  nCacheMask  = ( 1 << (int)log2( nObjsAlloc ) ) - 1;
  nMinRemoved = nObjsAlloc;
  pUnique     = (int *)calloc( nUniqueMask + 1, sizeof(int) );
  pNexts      = (int *)calloc( nUniqueMask + 1, sizeof(int) );
  pCache      = (unsigned *)calloc( 3 * (long long)( nCacheMask + 1 ), sizeof(unsigned) );
  pObjs       = (unsigned *)calloc( 2 * (long long)nObjsAlloc, sizeof(unsigned) );
  pMarks      = (unsigned char *)calloc( nObjsAlloc, sizeof(unsigned char) );
  nRefresh    = 0;
  fRealloc    = 0;
  fGC         = 0;
  MaxGrowth    = 0;
  nReo        = 4000;
  pvNodes     = NULL;
  pEdges      = NULL;
  pVars   = (unsigned char *)calloc( nObjsAlloc, sizeof(unsigned char) );
  if ( !pUnique || !pNexts || !pCache || !pObjs || !pMarks || !pVars )
    throw "Allocation failed";
  SetVarOfBvar( BvarConst(), VarConst() );
  nObjs = 1;
  vOrdering.clear();
  if ( pvOrdering )
    {
      for ( int i : *pvOrdering )
      	vOrdering.push_back( i );
      if ( vOrdering.size() != nVars )
	throw "Wrong number of Variables in the ordering";
    }
  else
    for ( int i = 0; i < nVars; i++ )
      vOrdering.push_back( i );
  for ( int i = 0; i < nVars; i++ ) {
    int j = std::distance( vOrdering.begin(), std::find( vOrdering.begin(), vOrdering.end(), i ) );
    if( j == nVars )
      throw "Invalid Ordering";
    UniqueCreate( j, LitConst1(), LitConst0() );
  }
}

BddMan::~BddMan()
{
  if ( nVerbose )
    std::cout << "Free : Var = " << nVars << " Obj = " << nObjs << " Alloc = " << nObjsAlloc - 1 << std::endl;
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

   Synopsis    [Cache for AND operation]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
inline unsigned BddMan::CacheLookup( unsigned Arg1, unsigned Arg2 )
{
  unsigned * p = pCache + 3 * (long long)( Hash( 0, Arg1, Arg2 ) & nCacheMask );
  if ( p[0] == Arg1 && p[1] == Arg2 )
    return p[2];
  return LitInvalid();
}
inline unsigned BddMan::CacheInsert( unsigned Arg1, unsigned Arg2, unsigned Res )
{
  if ( LitIsInvalid( Res ) )
    return Res;
  unsigned * p = pCache + 3 * (long long)( Hash( 0, Arg1, Arg2 ) & nCacheMask );
  p[0] = Arg1;
  p[1] = Arg2;
  p[2] = Res;
  return Res;
}
inline void BddMan::CacheClear() {
  free( pCache );
  pCache = (unsigned *)calloc( 3 * (long long)( nCacheMask + 1 ), sizeof(unsigned) );
}

/**Function*************************************************************

   Synopsis    [Reallocate]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
inline void BddMan::Rehash()
{
  unsigned nUniqueMaskOld = nUniqueMask >> 1; // assuming it has been doubled
  for ( unsigned i = 0; i < nUniqueMaskOld + 1; i++ )
    {
      int * q = pUnique + i;
      int * tail1 = q;
      int * tail2 = q + nUniqueMaskOld + 1;
      while ( *q )
	{
	  unsigned hash = Hash( VarOfBvar( *q ), ThenOfBvar( *q ), ElseOfBvar( *q ) ) & nUniqueMask;
	  int * tail;
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
inline void BddMan::Realloc()
{
  unsigned nObjsAllocOld = nObjsAlloc;
  unsigned nUniqueMaskOld = nUniqueMask;
  nObjsAlloc  = nObjsAlloc + nObjsAlloc;
  if ( !nObjsAlloc || nObjsAlloc > 1 << 31 )
    throw "Node overflow";
  if ( nVerbose )
    std::cout << "\tReallocate " << nObjsAlloc << " nodes" << std::endl;
  nUniqueMask = ( 1 << (int)log2( nObjsAlloc ) ) - 1;
  nCacheMask  = ( 1 << (int)log2( nObjsAlloc ) ) - 1;
  assert( ((nUniqueMaskOld << 1) ^ 01) == nUniqueMask );
  pUnique     = (int *)realloc( pUnique, sizeof(int) * ( nUniqueMask + 1 ) );
  pNexts      = (int *)realloc( pNexts, sizeof(int) * ( nUniqueMask + 1 ) );
  pObjs       = (unsigned *)realloc( pObjs, sizeof(unsigned) * 2 * (long long)nObjsAlloc );
  pMarks      = (unsigned char *)realloc( pMarks, sizeof(unsigned char) * nObjsAlloc );
  pVars     = (unsigned char *)realloc( pVars, sizeof(unsigned char) * nObjsAlloc );
  if ( !pUnique || !pNexts || !pObjs || !pMarks || !pVars )
    throw "Reallocation failed";
  memset( pUnique + ( nUniqueMaskOld + 1 ), 0, sizeof(int) * ( nUniqueMaskOld + 1 ) );
  memset( pNexts + ( nUniqueMaskOld + 1 ), 0, sizeof(int) * ( nUniqueMaskOld + 1 ) );
  memset( pObjs + 2 * (long long)nObjsAllocOld, 0, sizeof(unsigned) * 2 * (long long)nObjsAllocOld );
  memset( pMarks + nObjsAllocOld, 0, sizeof(unsigned char) * nObjsAllocOld );
  memset( pVars + nObjsAllocOld, 0, sizeof(unsigned char) * nObjsAllocOld );
  CacheClear();
  Rehash();
  if ( pEdges )
    {
      pEdges = (unsigned *)realloc( pEdges, sizeof(unsigned) * nObjsAlloc );
      if ( !pEdges )
	throw "Reallocation failed";
      memset ( pEdges + nObjsAllocOld, 0, sizeof(unsigned) * nObjsAllocOld );
    }
}

/**Function*************************************************************
   
   Synopsis    [AND]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
unsigned BddMan::And_rec( unsigned x, unsigned y )
{
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
  unsigned z = CacheLookup( x, y );
  if ( !LitIsInvalid( z ) )
    return z;
  unsigned x0, x1, y0, y1;
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
  unsigned z1 = And_rec( x1, y1 );
  if ( LitIsInvalid( z1 ) )
    return z1;
  Ref( z1 );
  unsigned z0 = And_rec( x0, y0 );
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
inline unsigned BddMan::And( unsigned x, unsigned y )
{
  nRefresh = 0;
  unsigned z = LitInvalid();
  while( LitIsInvalid( z ) )
    z = And_rec( x, y );
  return z;
}
inline unsigned BddMan::Or( unsigned x, unsigned y )
{
  return LitNot( And_rec( LitNot( x ), LitNot( y ) ) );
}
inline unsigned BddMan::Xnor( unsigned x, unsigned y )
{
  unsigned z1 = And( x, y );
  Ref( z1 );
  unsigned z0 = And( LitNot( x ), LitNot( y ) );
  Ref( z0 );
  unsigned z = Or( z0, z1 );
  Pop();
  Pop();
  return z;
}

/**Function*************************************************************
   
   Synopsis    [Count node]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
int BddMan::Count_rec( unsigned x )
{
  if ( LitIsConst( x ) || Mark( x ) )
    return 0;
  SetMark( x, 1 );
  return 1 + Count_rec( Else( x ) ) + Count_rec( Then( x ) );
}
void BddMan::Mark_rec( unsigned x )
{
  if ( LitIsConst( x ) || Mark( x ) )
    return;
  SetMark( x, 1 );
  Mark_rec( Else( x ) );
  Mark_rec( Then( x ) );
}
void BddMan::Unmark_rec( unsigned x )
{
  if ( LitIsConst( x ) || !Mark( x ) )
    return;
  SetMark( x, 0 );
  Unmark_rec( Else( x ) );
  Unmark_rec( Then( x ) );
}
inline int BddMan::CountNodes( unsigned x )
{
  int count = Count_rec( x );
  Unmark_rec( x );
  return count;
}
inline int BddMan::CountNodesArrayShared( std::vector<unsigned> & vNodes )
{
  unsigned x;
  int count = 0;
  for ( unsigned x : vNodes )
    count += Count_rec( x );
  for ( int i = 0; i < nVars; i++ )
    count += Count_rec( LitIthVar( i ) );
  for ( unsigned x : vNodes )  
    Unmark_rec( x );
  for ( int i = 0; i < nVars; i++ )
    Unmark_rec( LitIthVar( i ) );
  return count + 4; // add 4 to make the number comparable to command "collapse -v"
}
inline int BddMan::CountNodesArrayIndependent( std::vector<unsigned> & vNodes )
{
  int count = 0;
  for ( unsigned x : vNodes )
    {
      // exclude variables to make the number comparable to command "print_stats" after "collapse"
      if ( LitRegular( x ) <= LitIthVar( nVars - 1 ) )
	continue; 
      count += Count_rec( x );
      Unmark_rec( x );
    }
  return count;
}

/**Function*************************************************************

   Synopsis    [Garbage collection]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
inline void BddMan::RemoveNodeByBvar( int a )
{
  int * q = pUnique + ( Hash( VarOfBvar( a ), ThenOfBvar( a ), ElseOfBvar( a ) ) & nUniqueMask );
  for ( ; *q; q = pNexts + *q )
    if ( *q == a )
      break;
  int * next = pNexts + *q;
  *q = *next;
  *next = 0;
  SetBvarRemoved( a );
  if ( nMinRemoved > a )
    nMinRemoved = a;
}
inline void BddMan::GarbageCollect()
{
  unsigned x;
  if ( nVerbose )
    std::cout <<  "\tGarbage collect" << std::endl;
  for ( unsigned x : *pvNodes )
    Mark_rec( x );
  for ( int i = nVars + 1; i < nObjs; i++ )
    if ( !MarkOfBvar( i ) && !BvarIsRemoved( i ) )
      RemoveNodeByBvar( i );
  for ( unsigned x : *pvNodes )
    Unmark_rec( x );
  CacheClear();
}

/**Function*************************************************************

   Synopsis    [Refresh]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
inline void BddMan::RefreshConfig( int fRealloc_, int fGC_, int nMaxGrowth )
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
inline int BddMan::Refresh()
{
  nRefresh += 1;
  if ( nVerbose )
    std::cout << "Refresh " << nRefresh << std::endl;
  if ( nRefresh <= 1 && fGC )
    {
      GarbageCollect();
      return 0;
    }
  if ( nRefresh <= 2 && MaxGrowth && nObjs > nReo )
    {
      Reorder();
      nReo = nReo + nReo;
      return -1;
    }
  if ( fRealloc && nObjsAlloc < 1 << 31 )
    {
      Realloc();
      return 0;
    }
  throw "Node overflow";
}

/**Function*************************************************************
   
   Synopsis    [Edge]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
void BddMan::CountEdge_rec( unsigned x )
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
inline void BddMan::CountEdge( std::vector<unsigned> & vNodes )
{
  for ( unsigned x : vNodes )
    CountEdge_rec( x );
  for ( int i = 0; i < nVars; i++ )
    CountEdge_rec( LitIthVar( i ) );
  for ( unsigned x : vNodes )
    Unmark_rec( x );
  for ( int i = 0; i < nVars; i++ )
    Unmark_rec( LitIthVar( i ) );
}
void BddMan::UncountEdge_rec( unsigned x )
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
inline void BddMan::UncountEdge( std::vector<unsigned> & vNodes )
{
  for ( unsigned x : vNodes )
    UncountEdge_rec( x );
  for ( int i = 0; i < nVars; i++ )
    UncountEdge_rec( LitIthVar( i ) );
  for ( unsigned x : vNodes )
    Unmark_rec( x );
  for ( int i = 0; i < nVars; i++ )
    Unmark_rec( LitIthVar( i ) );
}

void BddMan::CountEdgeAndBvar_rec( unsigned x )
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
inline void BddMan::CountEdgeAndBvar( std::vector<unsigned> & vNodes )
{
  for ( unsigned x : vNodes )
    CountEdgeAndBvar_rec( x );
  for ( int i = 0; i < nVars; i++ )
    CountEdgeAndBvar_rec( LitIthVar( i ) );
  for ( unsigned x : vNodes )
    Unmark_rec( x );
  for ( int i = 0; i < nVars; i++ )
    Unmark_rec( LitIthVar( i ) );
}

/**Function*************************************************************
   
   Synopsis    [Print Ordering]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
inline void BddMan::PrintOrdering( std::vector<var> & new2old )
{
  std::cout << "Ordering :" << std::endl;
  for ( int i : new2old )
    std::cout << vOrdering[i] << ",";
  std::cout << std::endl << "----------" << std::endl;
}

/**Function*************************************************************
   
   Synopsis    [Shift/Swap Bvar]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
inline void BddMan::ShiftBvar( int a, int d )
{
  int v = VarOfBvar( a );
  unsigned x1 = ThenOfBvar( a );
  unsigned x0 = ElseOfBvar( a );
  // remove
  unsigned hash = Hash( v, x1, x0 ) & nUniqueMask;
  int * q = pUnique + hash;
  int * next = pNexts + a;
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
inline void BddMan::SwapBvar( int a, int fRestore )
{
  int v = VarOfBvar( a );
  unsigned x1 = ThenOfBvar( a );
  unsigned x0 = ElseOfBvar( a );
  // new chlidren
  unsigned f00, f01, f10, f11;
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
  unsigned y1 = UniqueCreate( v + 1, f11, f01 );
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
  unsigned y0 = UniqueCreate( v + 1, f10, f00 );
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
  unsigned hash = Hash( v, x1, x0 ) & nUniqueMask;
  int * q = pUnique + hash;
  int * next = pNexts + a;
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
inline int BddMan::Swap( var v, bvar & nNodes, lit dLimit )
{
  liveBvars[nVars].clear();
  liveBvars[nVars + 1].clear();
  // walk upper level
  for ( int a : liveBvars[v] )
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
  for ( int a : liveBvars[v + 1] )
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
  int nSwapHead = liveBvars[nVars].size();
  int fOutOfLimit = 0;
  int nOut;
  // walk upper level again
  for ( int i = 0; i < liveBvars[v].size(); i++ )
    {
      int a = liveBvars[v][i];
      if ( VarOfBvar( a ) == v )
	{
	  SwapBvar( a, 0 );
	  liveBvars[nVars].push_back( a );
	  if ( liveBvars[nVars].size()
	       + liveBvars[nVars + 1].size()
	       > liveBvars[v].size()
	       + liveBvars[v + 1].size()
	       + dLimit )
	    {
	      nOut = i + 1;
	      fOutOfLimit = 1;
	      break;
	    }
	}
    }
  if ( !fOutOfLimit )
    {
      // swap liveBvars
      nNodes += (int)liveBvars[nVars].size() + (int)liveBvars[nVars + 1].size() - (int)liveBvars[v].size() - (int)liveBvars[v + 1].size();
      std::iter_swap(liveBvars.begin() + v, liveBvars.begin() + nVars);
      std::iter_swap(liveBvars.begin() + v + 1, liveBvars.begin() + nVars + 1);
      return 0;
    }
  // restore previous tree
  std::vector<int> vTmp(liveBvars[v].begin() + nOut, liveBvars[v].end());
  liveBvars[v].clear();
  liveBvars[v + 1].clear();
  // walk new upper level where swapped
  for ( int i = nSwapHead; i < liveBvars[nVars].size(); i++ )
    {
      int a = liveBvars[nVars][i];
      DecEdgeNonConst( ThenOfBvar( a ) );
      DecEdgeNonConst( ElseOfBvar( a ) );
    }
  // walk new lower level
  for ( int a : liveBvars[nVars + 1] )
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
  for ( int i = 0; i < nSwapHead; i++ )
    {
      int a = liveBvars[nVars][i];
      ShiftBvar( a, 1 );
      liveBvars[v + 1].push_back( a );
    }
  // walk old upper level from where out of nodes
  for ( int a : vTmp )
    {
      if ( Var( ThenOfBvar( a ) ) == v + 1 ||
	   Var( ElseOfBvar( a ) ) == v + 1 )
	{
	  liveBvars[v].push_back( a );
	  unsigned x1 = ThenOfBvar( a );
	  if ( !Edge( x1 ) && Var( x1 ) == v + 1 )
	    {
	      IncEdgeNonConst( Then( x1 ) );
	      IncEdgeNonConst( Else( x1 ) );
	      liveBvars[v + 1].push_back( Lit2Bvar( x1 ) );
	    }
	  IncEdgeNonConst( x1 );
	  unsigned x0 = ElseOfBvar( a );
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
  for ( int i = nSwapHead; i < liveBvars[nVars].size(); i++ )
    {
      int a = liveBvars[nVars][i];
      SwapBvar( a, 1 );
      liveBvars[v].push_back( a );
    }
  return -1; // if ( fOutOfLimit );
}
inline void BddMan::Shift( var & pos, bvar & nNodes, var nSwap, int fUp, var & bestPos, bvar & nBestNodes, std::vector<var> & new2old, lit nLimit )
{
  float MaxGrowth_ = MaxGrowth;
  int nRefresh_ = nRefresh;
  MaxGrowth = 0;
  for ( int i = 0; i < nSwap; i++ )
    {
      lit dLimit = nLimit - nNodes;
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
      //      CheckLiveBvar();
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
void BddMan::Reorder()
{
  std::vector<var> descendingOrder;
  std::vector<var> new2old;
  if ( nVerbose )
    std::cout << "\tReordering" << std::endl;
  // initialize
  for ( int i = 0; i < nVars + 2; i++ )
    {
      liveBvars[i].clear();
      liveBvars[i].reserve( nObjs / nVars );
    }
  CountEdgeAndBvar( *pvNodes );
  for ( int i = 0; i < nVars; i++ )
    {
      new2old.push_back( i );
      descendingOrder.push_back( i );
    }
  std::sort( descendingOrder.begin(), descendingOrder.end(), [&]( int i, int j )
    {
      return liveBvars[i].size() > liveBvars[j].size();
    });
  bvar nNodes = 0;
  for ( int i = 0; i < nVars; i++ )
    nNodes += liveBvars[i].size();
  if ( nVerbose  >= 2 )
    {
      std::cout << "nNode for each level :" << std::endl;
      for ( int i = 0; i < nVars; i++ )
	std::cout << liveBvars[i].size() << ",";
      std::cout << std::endl;
      PrintOrdering( new2old );
    }
  // shift
  for ( int i = 0; i < nVars; i++ )
    {
      var pos = std::distance( new2old.begin(), std::find( new2old.begin(), new2old.end(), descendingOrder[i] ) );
      int fUp = 0;
      var nSwap;
      lit nLimit = nNodes * MaxGrowth + nNodes;
      var bestPos = pos;
      bvar nBestNodes = nNodes;
      if ( nVerbose >= 2 )
	std::cout << "\tBegin shift " << vOrdering[descendingOrder[i]] << " (" << i + 1 << "/" << nVars << std::endl;
      if( pos < nVars >> 1 )
	{
	  fUp ^= 1;
	  nSwap = pos;
	}
      else nSwap = nVars - pos - 1;
      Shift( pos, nNodes, nSwap, fUp, bestPos, nBestNodes, new2old, nLimit );
      fUp ^= 1;
      if ( fUp ) nSwap = pos;
      else nSwap = nVars - pos - 1;
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
  for ( int i : new2old )
    vOrdering.push_back( vTmp[i] );
  UncountEdge( *pvNodes ); // may slow it
  /*
  for ( int i = 0; i < nObjs; i++ )
    {
      if ( EdgeOfBvar( i ) )
	std::cout << i << " " << EdgeOfBvar( i ) << std::endl;
      assert ( !EdgeOfBvar(i) );
    }
  */
  CacheClear();
}

/*void BddMan::CheckLiveBvar_rec( unsigned x )
{
  if ( LitIsConst( x ) )
    return;
  if ( Mark( x ) )
    return;
  if( std::find( liveBvars[Var( x )].begin(), liveBvars[Var(x)].end(), Lit2Bvar( x ) )  == liveBvars[Var(x)].end() )
    {
      std::cout << x << std::endl;
      abort();
    }
  assert(Var( Then(x)) > Var(x));
  assert(Var( Else(x) ) > Var(x));
  SetMark( x, 1 );
  CheckLiveBvar_rec( Else( x ) );
  CheckLiveBvar_rec( Then( x ) );
}
inline void BddMan::CheckLiveBvar()
{
  return;
  for ( unsigned x : *pvNodes )
    CheckLiveBvar_rec( x );
  for ( int i = 0; i < nVars; i++ )
    CheckLiveBvar_rec( LitIthVar( i ) );
  for ( unsigned x : *pvNodes )
    Unmark_rec( x );
  for ( int i = 0; i < nVars; i++ )
    Unmark_rec( LitIthVar( i ) );
}
*/
}

#endif
