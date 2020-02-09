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
class SimpleBdd
{
/**Function*************************************************************
   
   Synopsis    [Data structure]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
private:
  int                nVars;         // the number of variables
  int                nObjs;         // the number of nodes used
  unsigned           nObjsAlloc;    // the number of nodes allocated
  int *              pUnique;       // unique table for nodes
  int *              pNexts;        // next pointer for nodes
  unsigned *         pCache;        // array of triples <arg0, arg1, AND(arg0, arg1)>
  unsigned *         pObjs;         // array of pairs cof0 for nodes
  unsigned char *    pVars;         // array of variables for nodes
  unsigned short *   pSVars;        // array of variables for nodes when more than 255 vars
  unsigned char *    pMarks;        // array of marks for nodes
  unsigned           nUniqueMask;   // selection mask for unique table
  unsigned           nCacheMask;    // selection mask for computed table
  int                nMinRemoved;   // the minimum int of removed nodes
  int                nVerbose;      // the level of verbosing information

  int                nRefresh;      // the number of refresh tried
  int                fGC;           // flag of garbage collection
  int                fRealloc;      // flag of reallocation
  
  std::vector<int>   vOrdering;     // variable ordering : new 2 old
  float              ReoThold;      // threshold to terminate reordering. 0=off
  unsigned *         pEdges;        // array of number of incoming edges for nodes
  std::vector<std::vector<int> > liveBvars; // array of live Bvars for each layer

public:
  std::vector<unsigned> * pvFrontiers;   // vector of frontier nodes

  int get_nVars() { return nVars; }
  int get_order( int v ) { return vOrdering[v]; }
  
/**Function*************************************************************
   
   Synopsis    [Hash]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  unsigned Hash( int Arg0, int Arg1, int Arg2 ) { return 12582917 * Arg0 + 4256249 * Arg1 + 741457 * Arg2; }

/**Function*************************************************************
   
   Synopsis    [Definition of constant and invalid values]

   Description [Var = Variable, Lit = Literal, Bvar = BddVariable = Lit >> 1]
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  int      BvarConst()   { return 0;                   }
  int      VarConst()    { return pVars? 0xff: 0xffff; }
  int      BvarInvalid() { return 0x7fffffff;          }
  unsigned EdgeInvalid() { return 0xffffffff;          }
  int      MarkInvalid() { return 0xff;                }
  int      VarRemoved()  { return pVars? 0xff: 0xffff; }
  
/**Function*************************************************************
   
   Synopsis    [Utilities for Bvar]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  unsigned BvarIthVar( int a )      { return a + 1;                        }
  int      BvarIsEq( int a, int b ) { return a == b;                       }
  int      BvarIsConst( int a )     { return BvarIsEq( a, BvarConst() );   }
  int      BvarIsInvalid( int a )   { return BvarIsEq( a, BvarInvalid() ); }

/**Function*************************************************************
   
   Synopsis    [Bvar to/from Lit]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  unsigned Bvar2Lit( int a, int c ) { return a + a + (int)( c > 0 ); }
  int      Lit2Bvar( unsigned x )   { return x >> 1;                 }

/**Function*************************************************************
   
   Synopsis    [Utilities for Lit]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  unsigned LitRegular( unsigned x )          { return x & ~01;                        }
  unsigned LitNot( unsigned x )              { return x ^ 1;                          }
  unsigned LitNotCond( unsigned x, int c )   { return x ^ (int)( c > 0 );             }
  unsigned LitConst0()                       { return Bvar2Lit( BvarConst(), 0 );     }
  unsigned LitConst1()                       { return LitNot( LitConst0() );          }
  unsigned LitInvalid()                      { return Bvar2Lit( BvarInvalid(), 0 );   }
  int      LitIsCompl( unsigned x )          { return x & 1;                          }
  unsigned LitIthVar( int a )                { return Bvar2Lit( BvarIthVar( a ), 0 ); }
  int      LitIsEq( unsigned x, unsigned y ) { return x == y;                         }
  int      LitIsConst0( unsigned x )         { return LitIsEq( x, LitConst0() );      }
  int      LitIsConst1( unsigned x )         { return LitIsEq( x, LitConst1() );      }
  int      LitIsConst( unsigned x )          { return BvarIsConst( Lit2Bvar( x ) );   }
  int      LitIsInvalid( unsigned x )        { return BvarIsInvalid( Lit2Bvar( x ) ); }
  
/**Function*************************************************************
   
   Synopsis    [Utilities for Lit]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  int      Var( unsigned x )     { return pVars? pVars[Lit2Bvar( x )]: pSVars[Lit2Bvar( x )]; }
  unsigned Then( unsigned x )    { return LitNotCond( pObjs[LitRegular( x )], LitIsCompl( x ) ); }
  unsigned Else( unsigned x )    { return LitNotCond( pObjs[LitNot( LitRegular( x ) )], LitIsCompl( x ) ); }
  int      Next( unsigned x )    { return pNexts[Lit2Bvar( x )]; }
  int      Mark( unsigned x )    { return pMarks[Lit2Bvar( x )]; }
  unsigned Edge( unsigned x )    { return pEdges[Lit2Bvar( x )]; }

  void     SetMark( unsigned x, int m ) { pMarks[Lit2Bvar( x )] = m; }
  void     IncMark( unsigned x ) { if ( ++pMarks[Lit2Bvar( x )] == MarkInvalid() ) throw "Mark overflow"; }
  void     DecMark( unsigned x ) { assert( --pMarks[Lit2Bvar( x )] != MarkInvalid() ); }

  void     IncEdge( unsigned x ) { if ( ++pEdges[Lit2Bvar( x )] == EdgeInvalid() ) throw "Edge overflow"; }
  void     DecEdge( unsigned x ) { assert( --pEdges[Lit2Bvar( x )] != EdgeInvalid() ); }
  void     IncEdgeNonConst( unsigned x) { if ( !LitIsConst( x ) ) IncEdge( x ); }
  void     DecEdgeNonConst( unsigned x) { if ( !LitIsConst( x ) ) DecEdge( x ); }

/**Function*************************************************************
   
   Synopsis    [Utilities for Bvar]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
  int      VarOfBvar( int a )  { return pVars? pVars[a]: pSVars[a]; }
  unsigned ThenOfBvar( int a ) { return pObjs[Bvar2Lit( a, 0 )];    }
  unsigned ElseOfBvar( int a ) { return pObjs[Bvar2Lit( a, 1 )];    }
  int      NextOfBvar( int a ) { return pNexts[a];                  }
  int      MarkOfBvar( int a ) { return pMarks[a];                  }
  unsigned EdgeOfBvar( int a ) { return pEdges[a];                  }

  void     SetVarOfBvar( int a, int v ) { if ( pVars ) pVars[a] = v; else pSVars[a] = v; }
  void     SetThenOfBvar( int a, unsigned x1 ) { pObjs[Bvar2Lit( a, 0 )] = x1; }
  void     SetElseOfBvar( int a, unsigned x0 ) { pObjs[Bvar2Lit( a, 1 )] = x0; }
  void     SetNextOfBvar( int a, int b ) { pNexts[a] = b; }
  void     SetMarkOfBvar( int a, int m ) { pMarks[a] = m; }
  void     SetEdgeOfBvar( int a, int e ) { pEdges[a] = e; }

  int      BvarIsRemoved( int a ) { return VarOfBvar( a ) == VarRemoved(); }
  void     SetBvarRemoved( int a ) { SetVarOfBvar( a, VarRemoved() ); }
  int      BvarIsVar( int a ) { return a <= nVars && !BvarIsConst( a ); }

/**Function*************************************************************
   
   Synopsis    [Functions]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
private:
  unsigned UniqueCreateInt( int v, unsigned x1, unsigned x0 );
  unsigned CacheLookup( unsigned Arg1, unsigned Arg2 );
  unsigned CacheInsert( unsigned Arg1, unsigned Arg2, unsigned Res );
  void     CacheClear();
  void     Rehash();
  unsigned And_rec( unsigned x, unsigned y );
  int      Count_rec( unsigned x );
  void     Mark_rec( unsigned x );
  void     Unmark_rec( unsigned x );
  void     RemoveNodeByBvar( int a );
  void     CountEdge_rec( unsigned x );
  void     CountEdge( std::vector<unsigned> & vNodes );
  void     UncountEdge_rec( unsigned x );
  void     UncountEdge( std::vector<unsigned> & vNodes );
  void     CountEdgeAndBvar_rec( unsigned x );
  void     CountEdgeAndBvar( std::vector<unsigned> & vNodes );
  void     ShiftBvar( int a, int d );
  int      SwapBvar( int a, int fRestore );
  int      Swap( int v, int & nNodes, int dLimit );
  void     Shift( int & pos, int & nNodes, int nSwap, int fUp, int & bestPos, int & nBestNodes, std::vector<int> & new2old, std::vector<unsigned> & vNodes, int nLimit );
  
public:
  SimpleBdd( int nVars, unsigned nObjsAlloc_, int fDynAlloc, std::vector<int> * pvOrdering, int nVerbose );
  ~SimpleBdd();
  int      IsLimit() { return (unsigned)nObjs == nObjsAlloc || nObjs == BvarInvalid(); }
  void     InitRefresh() { nRefresh = 0; }
  unsigned UniqueCreate( int v, unsigned x1, unsigned x0 );
  void     Realloc();
  unsigned And( unsigned x, unsigned y );
  unsigned Or( unsigned x, unsigned y );
  unsigned Xnor( unsigned x, unsigned y );
  int      CountNodes( unsigned x );
  int      CountNodesArrayShared( std::vector<unsigned> & vNodes );
  int      CountNodesArrayIndependent( std::vector<unsigned> & vNodes );
  void     GarbageCollect( std::vector<unsigned> & vNodes );
  void     RefreshConfig( int fRealloc_, int fGC_, int nReoThold );
  int      Refresh();
  void     PrintOrdering( std::vector<int> & new2old );
  void     ReorderConfig( int nReoThold );
  void     Reorder( std::vector<unsigned> & vNodes );
};

/**Function*************************************************************
   
   Synopsis    [Create new node]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
inline unsigned SimpleBdd::UniqueCreateInt( int v, unsigned x1, unsigned x0 )
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
      for ( ; nMinRemoved < nObjs; nMinRemoved++ )
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
  
inline unsigned SimpleBdd::UniqueCreate( int v, unsigned x1, unsigned x0 )
{
  if ( LitIsEq( x1, x0 ) )
    return x0;
  if ( !LitIsCompl( x0 ) )
    return UniqueCreateInt( v, x1, x0 );
  return LitNot( UniqueCreateInt( v, LitNot( x1 ), LitNot( x0 ) ) );
}

/**Function*************************************************************
 
   Synopsis    [Allocation/free]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
SimpleBdd::SimpleBdd( int nVars, unsigned nObjsAlloc_, int fDynAlloc, std::vector<int> * pvOrdering, int nVerbose ) : nVars(nVars), nObjsAlloc(nObjsAlloc_), nVerbose(nVerbose)
{
  while ( nObjsAlloc < nVars + 1 )
    {
      if ( !fDynAlloc || !nObjsAlloc || nObjsAlloc > 1 << 31 )
	throw "Node overflow just for Variables\n";
      nObjsAlloc = nObjsAlloc + nObjsAlloc;
    }
  if ( nVerbose )
    std::cout << "Allocate " << nObjsAlloc << " nodes" << std::endl;
  nUniqueMask = ( 1 << (int)log2( nObjsAlloc ) ) - 1;
  nCacheMask  = ( 1 << (int)log2( nObjsAlloc ) ) - 1;
  nMinRemoved = nObjsAlloc - 1;
  pUnique     = (int *)calloc( nUniqueMask + 1, sizeof(int) );
  pNexts      = (int *)calloc( nUniqueMask + 1, sizeof(int) );
  pCache      = (unsigned *)calloc( 3 * (long long)( nCacheMask + 1 ), sizeof(unsigned) );
  pObjs       = (unsigned *)calloc( 2 * (long long)nObjsAlloc, sizeof(unsigned) );
  pMarks      = (unsigned char *)calloc( nObjsAlloc, sizeof(unsigned char) );
  nRefresh    = 0;
  fRealloc    = 0;
  fGC         = 0;
  ReoThold    = 0;
  pvFrontiers = NULL;
  pEdges      = NULL;
  if ( nVars < 0xff )
    {
      pVars   = (unsigned char *)calloc( nObjsAlloc, sizeof(unsigned char) );
      pSVars  = NULL;
    }
  else
    {
      pSVars  = (unsigned short *)calloc( nObjsAlloc, sizeof(unsigned short) );
      pVars   = NULL;
    }
  if ( nVars > VarConst() )
    throw "Variable overflow";
  if ( !pUnique || !pNexts || !pCache || !pObjs || !pMarks || (!pVars && !pSVars) )
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

SimpleBdd::~SimpleBdd()
{
  if ( nVerbose )
    std::cout << "Free : Var = " << nVars << " Obj = " << nObjs << " Alloc = " << nObjsAlloc - 1 << std::endl;
  free( pUnique );
  free( pNexts );
  free( pCache );
  free( pObjs );
  if ( pVars )
    free( pVars );
  if ( pSVars )
    free( pSVars );
  if ( pvFrontiers )
    delete pvFrontiers;
  if ( pEdges )
    free( pEdges );
}


/**Function*************************************************************

   Synopsis    [Cache for AND operation]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
inline unsigned SimpleBdd::CacheLookup( unsigned Arg1, unsigned Arg2 )
{
  unsigned * p = pCache + 3 * (long long)( Hash( 0, Arg1, Arg2 ) & nCacheMask );
  if ( p[0] == Arg1 && p[1] == Arg2 )
    return p[2];
  return LitInvalid();
}
inline unsigned SimpleBdd::CacheInsert( unsigned Arg1, unsigned Arg2, unsigned Res )
{
  if ( LitIsInvalid( Res ) )
    return Res;
  unsigned * p = pCache + 3 * (long long)( Hash( 0, Arg1, Arg2 ) & nCacheMask );
  p[0] = Arg1;
  p[1] = Arg2;
  p[2] = Res;
  return Res;
}
inline void SimpleBdd::CacheClear() {
  free( pCache );
  pCache = (unsigned *)calloc( 3 * (long long)( nCacheMask + 1 ), sizeof(unsigned) );
}

/**Function*************************************************************

   Synopsis    [Reallocate]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
inline void SimpleBdd::Rehash()
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
inline void SimpleBdd::Realloc()
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
  if ( pVars )
    pVars     = (unsigned char *)realloc( pVars, sizeof(unsigned char) * nObjsAlloc );
  else
    pSVars    = (unsigned short *)realloc( pSVars, sizeof(unsigned short) * nObjsAlloc );
  if ( !pUnique || !pNexts || !pObjs || !pMarks || (!pVars && !pSVars) )
    throw "Reallocation failed";
  memset( pUnique + ( nUniqueMaskOld + 1 ), 0, sizeof(int) * ( nUniqueMaskOld + 1 ) );
  memset( pNexts + ( nUniqueMaskOld + 1 ), 0, sizeof(int) * ( nUniqueMaskOld + 1 ) );
  memset( pObjs + 2 * (long long)nObjsAllocOld, 0, sizeof(unsigned) * 2 * (long long)nObjsAllocOld );
  memset( pMarks + nObjsAllocOld, 0, sizeof(unsigned char) * nObjsAllocOld );
  if ( pVars )
    memset( pVars + nObjsAllocOld, 0, sizeof(unsigned char) * nObjsAllocOld );
  else
    memset( pSVars + nObjsAllocOld, 0, sizeof(unsigned short) * nObjsAllocOld );
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
unsigned SimpleBdd::And_rec( unsigned x, unsigned y )
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
  unsigned z0 = And_rec( x0, y0 );
  if ( LitIsInvalid( z0 ) )
    return z0;
  unsigned z1 = And_rec( x1, y1 );
  if ( LitIsInvalid( z1 ) )
    return z1;
  z = UniqueCreate( std::min( Var( x ), Var( y ) ), z1, z0 );
  return CacheInsert( x, y, z );
}
inline unsigned SimpleBdd::And( unsigned x, unsigned y )
{
  if ( LitIsInvalid( x ) )
    return x;
  if ( LitIsInvalid( y ) )
    return y;
  return And_rec( x, y );
}
inline unsigned SimpleBdd::Or( unsigned x, unsigned y )
{
  if ( LitIsInvalid( x ) )
    return x;
  if ( LitIsInvalid( y ) )
    return y;
  return LitNot( And_rec( LitNot( x ), LitNot( y ) ) );
}
inline unsigned SimpleBdd::Xnor( unsigned x, unsigned y )
{
  unsigned z0 = And( LitNot( x ), LitNot( y ) );
  if ( LitIsInvalid( z0 ) )
    return z0;
  unsigned z1 = And( x, y );
  if ( LitIsInvalid( z1 ) )
    return z1;
  return Or( z0, z1 );
}

/**Function*************************************************************
   
   Synopsis    [Count node]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
int SimpleBdd::Count_rec( unsigned x )
{
  if ( LitIsConst( x ) || Mark( x ) )
    return 0;
  SetMark( x, 1 );
  return 1 + Count_rec( Else( x ) ) + Count_rec( Then( x ) );
}
void SimpleBdd::Mark_rec( unsigned x )
{
  if ( LitIsConst( x ) || Mark( x ) )
    return;
  SetMark( x, 1 );
  Mark_rec( Else( x ) );
  Mark_rec( Then( x ) );
}
void SimpleBdd::Unmark_rec( unsigned x )
{
  if ( LitIsConst( x ) || !Mark( x ) )
    return;
  SetMark( x, 0 );
  Unmark_rec( Else( x ) );
  Unmark_rec( Then( x ) );
}
inline int SimpleBdd::CountNodes( unsigned x )
{
  int count = Count_rec( x );
  Unmark_rec( x );
  return count;
}
inline int SimpleBdd::CountNodesArrayShared( std::vector<unsigned> & vNodes )
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
inline int SimpleBdd::CountNodesArrayIndependent( std::vector<unsigned> & vNodes )
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
inline void SimpleBdd::RemoveNodeByBvar( int a )
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
inline void SimpleBdd::GarbageCollect( std::vector<unsigned> & vNodes )
{
  unsigned x;
  if ( nVerbose )
    std::cout <<  "\tGarbage collect" << std::endl;
  for ( unsigned x : vNodes )
    Mark_rec( x );
  for ( int i = nVars + 1; i < nObjs; i++ )
    if ( !MarkOfBvar( i ) && !BvarIsRemoved( i ) )
      RemoveNodeByBvar( i );
  for ( unsigned x : vNodes )
    Unmark_rec( x );
  CacheClear();
}

/**Function*************************************************************

   Synopsis    [Refresh]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
inline void SimpleBdd::RefreshConfig( int fRealloc_, int fGC_, int nReoThold )
{
  fRealloc = fRealloc_;
  fGC = fGC_;
  if ( pvFrontiers )
    delete pvFrontiers;
  if ( fGC || nReoThold )
    pvFrontiers = new std::vector<unsigned>;
  ReorderConfig( nReoThold );
}
inline int SimpleBdd::Refresh()
{
  nRefresh += 1;
  if ( nVerbose )
    std::cout << "Refresh " << nRefresh << std::endl;
  if ( nRefresh <= 1 && fGC )
    {
      GarbageCollect( *pvFrontiers );
      return 0;
    }
  if ( nRefresh <= 2 && ReoThold && nObjsAlloc > 4000 )
    {
      Reorder( *pvFrontiers );
      GarbageCollect( *pvFrontiers );
      return 0;
    }
  if ( fRealloc && nObjsAlloc < 1 << 31 )
    {
      Realloc();
      return 0;
    }
  return -1;
}

/**Function*************************************************************
   
   Synopsis    [Edge]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
void SimpleBdd::CountEdge_rec( unsigned x )
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
inline void SimpleBdd::CountEdge( std::vector<unsigned> & vNodes )
{
  for ( unsigned x : vNodes )
    CountEdge_rec( x );
  for ( unsigned x : vNodes )
    Unmark_rec( x );
}
void SimpleBdd::UncountEdge_rec( unsigned x )
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
inline void SimpleBdd::UncountEdge( std::vector<unsigned> & vNodes )
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

void SimpleBdd::CountEdgeAndBvar_rec( unsigned x )
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
inline void SimpleBdd::CountEdgeAndBvar( std::vector<unsigned> & vNodes )
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
inline void SimpleBdd::PrintOrdering( std::vector<int> & new2old )
{
  std::cout << "Ordering :" << std::endl;
  for ( int i : new2old )
    std::cout << vOrdering[i] << ",";
  std::cout << std::endl << "----------" << std::endl;
}

/**Function*************************************************************
   
   Synopsis    [Reorder config]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
void SimpleBdd::ReorderConfig( int nReoThold )
{
  ReoThold = 0.01 * nReoThold;
  if ( pEdges )
    free( pEdges );
  if ( liveBvars.size() )
    liveBvars.clear();
  if ( ReoThold )
    {
      pEdges = (unsigned *)calloc( nObjsAlloc, sizeof(unsigned) );
      if ( !pEdges )
	throw "Allocation failed";
      liveBvars.resize( nVars + 2 );
    }
}

/**Function*************************************************************
   
   Synopsis    [Shift/Swap Bvar]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
inline void SimpleBdd::ShiftBvar( int a, int d )
{
  int v = VarOfBvar( a );
  unsigned x1 = ThenOfBvar( a );
  unsigned x0 = ElseOfBvar( a );
  int * next = pNexts + a;
  // remove
  unsigned hash = Hash( v, x1, x0 ) & nUniqueMask;
  int * q = pUnique + hash;
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
inline int SimpleBdd::SwapBvar( int a, int fRestore )
{
  int v = VarOfBvar( a );
  unsigned x1 = ThenOfBvar( a );
  unsigned x0 = ElseOfBvar( a );
  int * next = pNexts + a;
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
  if ( LitIsInvalid( y1 ) )
    return 1;
  unsigned y0 = UniqueCreate( v + 1, f10, f00 );
  if ( LitIsInvalid( y0 ) )
    return 1;
  if ( !Edge( y1 ) && Var( y1 ) == v + 1 )
    {
      if ( !fRestore )
	liveBvars[nVars + 1].push_back( Lit2Bvar( y1 ) );
      IncEdgeNonConst( f11 );
      IncEdgeNonConst( f01 );
    }
  IncEdgeNonConst( y1 );
  if ( !Edge( y0 ) && Var( y0 ) == v + 1 )
    {
      if ( !fRestore )
	liveBvars[nVars + 1].push_back( Lit2Bvar( y0 ) );
      IncEdgeNonConst( f10 );
      IncEdgeNonConst( f00 );
    }
  IncEdgeNonConst( y0 );
  // remove
  unsigned hash = Hash( v, x1, x0 ) & nUniqueMask;
  int * q = pUnique + hash;
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
  return 0;
}

/**Function*************************************************************
   
   Synopsis    [Shift/Swap layer]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
inline int SimpleBdd::Swap( int v, int & nNodes, int dLimit )
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
  int fOutOfNodes = 0;
  int fOutOfLimit = 0;
  int nOut;
  // walk upper level again
  for ( int i = 0; i < liveBvars[v].size(); i++ )
    {
      int a = liveBvars[v][i];
      if ( VarOfBvar( a ) == v )
	{
	  if ( SwapBvar( a, 0 ) )
	    {
	      nOut = i;
	      fOutOfNodes = 1;
	      break;
	    }
	  liveBvars[nVars].push_back( a );
	  if ( (int)liveBvars[nVars].size()
	       + (int)liveBvars[nVars + 1].size()
	       - (int)liveBvars[v].size()
	       - (int)liveBvars[v + 1].size()
	       > dLimit )
	    {
	      nOut = i + 1;
	      fOutOfLimit = 1;
	      break;
	    }
	}
    }
  if ( !fOutOfNodes && !fOutOfLimit )
    {
      // swap liveBvars
      nNodes += (int)liveBvars[nVars].size() + (int)liveBvars[nVars + 1].size() - (int)liveBvars[v].size() - (int)liveBvars[v + 1].size();
      std::iter_swap(liveBvars.begin() + v, liveBvars.begin() + nVars);
      std::iter_swap(liveBvars.begin() + v + 1, liveBvars.begin() + nVars + 1);
      return 0;
    }
  // restore previous tree
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
      ShiftBvar( a, -1 );
  // walk new upper level where shifted
  for ( int i = 0; i < nSwapHead; i++ )
    {
      int a = liveBvars[nVars][i];
      ShiftBvar( a, 1 );
    }
  // walk upper level from where out of nodes
  for ( int i = nOut; i < liveBvars[v].size(); i++ )
    {
      int a = liveBvars[v][i];
      if ( Var( ThenOfBvar( a ) ) == v + 1 ||
	   Var( ElseOfBvar( a ) ) == v + 1 )
	{
	  unsigned x1 = ThenOfBvar( a );
	  unsigned x0 = ElseOfBvar( a );
	  if ( !Edge( x1 ) && Var( x1 ) == v + 1 )
	    {
	      IncEdgeNonConst( Then( x1 ) );
	      IncEdgeNonConst( Else( x1 ) );
	    }
	  IncEdgeNonConst( x1 );
	  if ( !Edge( x0 ) && Var( x0 ) == v + 1 )
	    {
	      IncEdgeNonConst( Then( x0 ) );
	      IncEdgeNonConst( Else( x0 ) );
	    }
	  IncEdgeNonConst( x0 );
	}
    }
  // walk new upper level where swapped
  for ( int i = nSwapHead; i < liveBvars[nVars].size(); i++ )
    {
      int a = liveBvars[nVars][i];
      assert( !SwapBvar( a, 1 ) );
    }
  if ( fOutOfNodes )
    return -1;
  return 1; // if ( fOutOfLimit );
}
inline void SimpleBdd::Shift( int & pos, int & nNodes, int nSwap, int fUp, int & bestPos, int & nBestNodes, std::vector<int> & new2old, std::vector<unsigned> & vNodes, int nLimit )
{
  int fRefresh = 0;
  for ( int i = 0; i < nSwap; i++ )
    {
      int dLimit = nLimit - nNodes;
      if ( fUp )
	pos -= 1;
      int r = Swap( pos, nNodes, dLimit );
      if ( r == 1 )
	{
	  if ( fUp )
	    pos += 1;
	  return;
	}
      if ( r == -1 )
	{
	  if ( fUp )
	    pos += 1;
	  if ( fGC && !fRefresh )
	    {
	      GarbageCollect( vNodes );
	      fRefresh = 1;
	    }
	  else if ( fRealloc )
	    Realloc();
	  else
	    throw "Node overflow";
	  i--;
	  continue;
	}
      fRefresh = 0;
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
}

/**Function*************************************************************
   
   Synopsis    [Reorder]

   Description []
               
   SideEffects []

   SeeAlso     []

***********************************************************************/
void SimpleBdd::Reorder( std::vector<unsigned> & vNodes )
{
  std::vector<int> descendingOrder;
  std::vector<int> new2old;
  if ( nVerbose )
    std::cout << "\tReordering" << std::endl;
  // initialize
  for ( int i = 0; i < nVars + 2; i++ )
    {
      liveBvars[i].clear();
      liveBvars[i].reserve( nObjs / nVars );
    }
  CountEdgeAndBvar( vNodes );
  for ( int i = 0; i < nVars; i++ )
    {
      new2old.push_back( i );
      descendingOrder.push_back( i );
    }
  std::sort( descendingOrder.begin(), descendingOrder.end(), [&]( int i, int j )
    {
      return liveBvars[i].size() > liveBvars[j].size();
    });
  int nNodes = 0;
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
      int pos = std::distance( new2old.begin(), std::find( new2old.begin(), new2old.end(), descendingOrder[i] ) );
      int fUp = 0;
      int nSwap;
      int nLimit = nNodes * ReoThold + nNodes;
      int bestPos = pos;
      int nBestNodes = nNodes;
      if ( nVerbose >= 2 )
	std::cout << "\tBegin shift " << vOrdering[descendingOrder[i]] << " (" << i + 1 << "/" << nVars << std::endl;
      if( pos < nVars >> 1 )
	{
	  fUp ^= 1;
	  nSwap = pos;
	}
      else nSwap = nVars - pos - 1;
      Shift( pos, nNodes, nSwap, fUp, bestPos, nBestNodes, new2old, vNodes, nLimit );
      fUp ^= 1;
      if ( fUp ) nSwap = pos;
      else nSwap = nVars - pos - 1;
      Shift( pos, nNodes, nSwap, fUp, bestPos, nBestNodes, new2old, vNodes, nLimit );
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
      Shift( pos, nNodes, nSwap, fUp, bestPos, nBestNodes, new2old, vNodes, nLimit );
    }
  // finish
  std::vector<int> vTmp( vOrdering );
  vOrdering.clear();
  for ( int i : new2old )
    vOrdering.push_back( vTmp[i] );
  UncountEdge( vNodes );
  CacheClear();
}


}

#endif
