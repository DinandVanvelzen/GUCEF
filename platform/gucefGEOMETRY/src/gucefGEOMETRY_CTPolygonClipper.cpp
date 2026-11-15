/*
 *  gucefGEOMETRY: GUCEF module providing geometry: shapes, spaces and spatial queries
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <algorithm>

#ifndef GUCEF_CORE_CINT128_H
#include "gucefCORE_CInt128.h"
#define GUCEF_CORE_CINT128_H
#endif /* GUCEF_CORE_CINT128_H ? */

#ifndef GUCEF_CORE_CBIGINTEGER_H
#include "gucefCORE_CBigInteger.h"
#define GUCEF_CORE_CBIGINTEGER_H
#endif /* GUCEF_CORE_CBIGINTEGER_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYGONCLIPPER_H
#include "gucefGEOMETRY_CTPolygonClipper.h"
#define GUCEF_GEOMETRY_CTPOLYGONCLIPPER_H
#endif /* GUCEF_GEOMETRY_CTPOLYGONCLIPPER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A point of the integer grid */
struct SPolygonClipperGridPoint
{
    Int64 x;
    Int64 y;
};
typedef SPolygonClipperGridPoint SGridPoint;

namespace {

/** The grid limit: |x|, |y| <= 2^52, so doubled coordinates and their differences fit Int64 */
const Int64 GridLimit = (Int64) 1 << 52;

/** Below this magnitude crossing roundings stay within CInt128 */
const Int64 FastLimit = (Int64) 1 << 40;

inline bool
GridLess( const SGridPoint& a, const SGridPoint& b )
{
    return a.x < b.x || ( a.x == b.x && a.y < b.y );
}

inline bool
GridEqual( const SGridPoint& a, const SGridPoint& b )
{
    return a.x == b.x && a.y == b.y;
}

struct CGridLess
{
    bool operator()( const SGridPoint& a, const SGridPoint& b ) const { return GridLess( a, b ); }
};

/** Orders grid points by x only (for the searches of an x range) */
struct CGridXLess
{
    bool operator()( const SGridPoint& a, Int64 x ) const { return a.x < x; }
};

/** Orders grid points by y only (for the searches of a y range within a column) */
struct CGridYLess
{
    bool operator()( const SGridPoint& a, Int64 y ) const { return a.y < y; }
};

/** Orders the first hot pixels of the columns by x */
struct CColumnLess
{
    explicit CColumnLess( const SGridPoint* hot ) : m_hot( hot ) {}
    bool operator()( UInt32 column, Int64 x ) const { return m_hot[ column ].x < x; }
    const SGridPoint* m_hot;
};

/** The exact sign of ( b - a ) x ( c - a ) */
inline Int32
Orient( const SGridPoint& a, const SGridPoint& b, const SGridPoint& c )
{
    const CORE::CInt128 left = CORE::CInt128::Multiply( b.x - a.x, c.y - a.y );
    const CORE::CInt128 right = CORE::CInt128::Multiply( b.y - a.y, c.x - a.x );
    return CORE::CInt128::Compare( left, right );
}

/** The exact sign of n1 / d1 - n2 / d2 for positive d1, d2 */
inline Int32
CompareRatio( Int64 n1, Int64 d1, Int64 n2, Int64 d2 )
{
    return CORE::CInt128::Compare( CORE::CInt128::Multiply( n1, d2 ), CORE::CInt128::Multiply( n2, d1 ) );
}

/** floor( n / d ) for d > 0 */
Int64
FloorDivide( const CORE::CBigInteger& n, const CORE::CBigInteger& d )
{
    CORE::CBigInteger quotient;
    CORE::CBigInteger remainder;
    n.DivModFloor( d, quotient, remainder );
    Int64 result = 0;
    quotient.ToInt64( result );
    return result;
}

/**
 *  The grid point nearest to the crossing of the segments p1-p2 and p3-p4 (they cross properly): each coordinate is
 *  floor( exact + 1/2 ), so the point is the centre of the half open pixel that holds the crossing
 */
SGridPoint
RoundedCrossing( const SGridPoint& p1, const SGridPoint& p2, const SGridPoint& p3, const SGridPoint& p4 )
{
    const Int64 rx = p2.x - p1.x, ry = p2.y - p1.y, sx = p4.x - p3.x, sy = p4.y - p3.y, qx = p3.x - p1.x, qy = p3.y - p1.y;
    CORE::CInt128 den = CORE::CInt128::Subtract( CORE::CInt128::Multiply( rx, sy ), CORE::CInt128::Multiply( ry, sx ) );
    CORE::CInt128 num = CORE::CInt128::Subtract( CORE::CInt128::Multiply( qx, sy ), CORE::CInt128::Multiply( qy, sx ) );
    if ( den.IsNegative() )
    {
        den = CORE::CInt128::Negate( den );
        num = CORE::CInt128::Negate( num );
    }
    SGridPoint result;
    const bool fast = std::max( std::max( std::max( p1.x < 0 ? -p1.x : p1.x, p1.y < 0 ? -p1.y : p1.y ), std::max( p2.x < 0 ? -p2.x : p2.x, p2.y < 0 ? -p2.y : p2.y ) ),
                                std::max( std::max( p3.x < 0 ? -p3.x : p3.x, p3.y < 0 ? -p3.y : p3.y ), std::max( p4.x < 0 ? -p4.x : p4.x, p4.y < 0 ? -p4.y : p4.y ) ) ) < FastLimit;
    const Int64 r[ 2 ] = { rx, ry };
    const Int64 base[ 2 ] = { p1.x, p1.y };
    Int64 out[ 2 ] = { 0, 0 };
    for ( UInt32 axis=0; axis<2; ++axis )
    {
        if ( fast )
        {
            // differences below 2^41: num and den below 2^83, 2 num r + den below 2^126
            CORE::CInt128 product;
            CORE::CInt128::MultiplyChecked( num, r[ axis ], product );
            const CORE::CInt128 n = CORE::CInt128::Add( CORE::CInt128::ShiftLeft( product, 1 ), den );
            const CORE::CInt128 d = CORE::CInt128::ShiftLeft( den, 1 );
            CORE::CInt128 quotient;
            CORE::CInt128 remainder;
            CORE::CInt128::DivModFloor( n, d, quotient, remainder );
            out[ axis ] = base[ axis ] + (Int64) quotient.low;
        }
        else
        {
            CORE::CBigInteger bigNum;
            CORE::CBigInteger bigDen;
            CORE::CBigInteger::FromCInt128( num, bigNum );
            CORE::CBigInteger::FromCInt128( den, bigDen );
            CORE::CBigInteger n;
            bigNum.Multiply( r[ axis ], n );
            n <<= 1;
            n += bigDen;
            bigDen <<= 1;
            out[ axis ] = base[ axis ] + FloorDivide( n, bigDen );
        }
    }
    result.x = out[ 0 ];
    result.y = out[ 1 ];
    return result;
}

/**
 *  True when the segment a-b meets the half open pixel [ c - 1/2, c + 1/2 ) x [ c - 1/2, c + 1/2 ): Liang-Barsky
 *  clipping against the closed square in doubled coordinates with exact rational parameters, then the pieces that lie
 *  only on the excluded right or top edge are rejected
 */
bool
PixelHit( const SGridPoint& a, const SGridPoint& b, const SGridPoint& c )
{
    const Int64 p[ 2 ] = { 2 * a.x, 2 * a.y };
    const Int64 d[ 2 ] = { 2 * ( b.x - a.x ), 2 * ( b.y - a.y ) };
    const Int64 lo[ 2 ] = { 2 * c.x - 1, 2 * c.y - 1 };
    const Int64 hi[ 2 ] = { 2 * c.x + 1, 2 * c.y + 1 };
    Int64 n0 = 0, d0 = 1, n1 = 1, d1 = 1;
    for ( UInt32 axis=0; axis<2; ++axis )
    {
        if ( 0 == d[ axis ] )
        {
            if ( p[ axis ] < lo[ axis ] || p[ axis ] >= hi[ axis ] )
                return false;
            continue;
        }
        Int64 enter, exit, den;
        if ( d[ axis ] > 0 )
        {
            enter = lo[ axis ] - p[ axis ];
            exit = hi[ axis ] - p[ axis ];
            den = d[ axis ];
        }
        else
        {
            enter = p[ axis ] - hi[ axis ];
            exit = p[ axis ] - lo[ axis ];
            den = -d[ axis ];
        }
        if ( CompareRatio( enter, den, n0, d0 ) > 0 )
        {
            n0 = enter;
            d0 = den;
        }
        if ( CompareRatio( exit, den, n1, d1 ) < 0 )
        {
            n1 = exit;
            d1 = den;
        }
    }
    const Int32 order = CompareRatio( n0, d0, n1, d1 );
    if ( order > 0 )
        return false;
    if ( order < 0 )
        return true;
    // a single point at t = n0 / d0: excluded on the right or top edge
    for ( UInt32 axis=0; axis<2; ++axis )
    {
        const CORE::CInt128 value = CORE::CInt128::Add( CORE::CInt128::Multiply( p[ axis ], d0 ), CORE::CInt128::Multiply( n0, d[ axis ] ) );
        if ( 0 == CORE::CInt128::Compare( value, CORE::CInt128::Multiply( hi[ axis ], d0 ) ) )
            return false;
    }
    return true;
}

/** Twice the signed area of a ring of grid points, exact */
void
RingDoubleArea( const SGridPoint* ring, UInt32 count, CORE::CBigInteger& area )
{
    area = CORE::CBigInteger::Zero();
    CORE::CInt128 sum;
    bool wide = false;
    for ( UInt32 i=1; i+1<count; ++i )
    {
        const SGridPoint& o = ring[ 0 ];
        const CORE::CInt128 term = CORE::CInt128::Subtract( CORE::CInt128::Multiply( ring[ i ].x - o.x, ring[ i + 1 ].y - o.y ), CORE::CInt128::Multiply( ring[ i ].y - o.y, ring[ i + 1 ].x - o.x ) );
        CORE::CInt128 next;
        if ( !wide && CORE::CInt128::AddChecked( sum, term, next ) )
        {
            sum = next;
            continue;
        }
        if ( !wide )
        {
            CORE::CBigInteger::FromCInt128( sum, area );
            wide = true;
        }
        CORE::CBigInteger big;
        CORE::CBigInteger::FromCInt128( term, big );
        area += big;
    }
    if ( !wide )
        CORE::CBigInteger::FromCInt128( sum, area );
}

inline bool
Fill( Int32 winding, TPolygonFillRule rule )
{
    switch ( rule )
    {
        case POLYGONFILL_EVEN_ODD: return 0 != ( winding & 1 );
        case POLYGONFILL_NON_ZERO: return 0 != winding;
        case POLYGONFILL_POSITIVE: return winding > 0;
        default: return winding < 0;
    }
}

}  /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The integer engine: snap rounding of the arrangement, the planar graph of the fragments, the winding numbers of
 *  the faces and the output rings (see CTPolygonClipper)
 */
struct SPolygonClipperEngine
{
    struct SRing
    {
        UInt32 first;
        UInt32 count;
        UInt32 operand;
        Int32 orientation;
    };

    struct SSegment
    {
        SGridPoint a;       /**< the lexicographically smaller end */
        SGridPoint b;
        Int32 delta[ 2 ];   /**< the winding change per operand for traversal a -> b */
    };

    struct SFragment
    {
        UInt32 a;           /**< hot pixel indices, a < b */
        UInt32 b;
        Int32 delta[ 2 ];

        bool operator<( const SFragment& other ) const { return a < other.a || ( a == other.a && b < other.b ); }
    };

    struct SHit
    {
        CORE::CInt128 projection;
        UInt32 pixel;

        bool operator<( const SHit& other ) const { return CORE::CInt128::Compare( projection, other.projection ) < 0; }
    };

    /** Orders segments by the x of their smaller end */
    struct CSegmentOrder
    {
        explicit CSegmentOrder( const SSegment* segments ) : m_segments( segments ) {}
        bool operator()( UInt32 a, UInt32 b ) const { return m_segments[ a ].a.x < m_segments[ b ].a.x || ( m_segments[ a ].a.x == m_segments[ b ].a.x && a < b ); }
        const SSegment* m_segments;
    };

    /** Orders the outgoing half edges of a vertex counter-clockwise from the +x direction */
    struct CAngleOrder
    {
        CAngleOrder( const SPolygonClipperEngine* engine ) : m_engine( engine ) {}

        bool operator()( UInt32 h, UInt32 g ) const
        {
            Int64 hx, hy, gx, gy;
            m_engine->Direction( h, hx, hy );
            m_engine->Direction( g, gx, gy );
            const int hh = ( hy < 0 || ( 0 == hy && hx < 0 ) ) ? 1 : 0;
            const int gh = ( gy < 0 || ( 0 == gy && gx < 0 ) ) ? 1 : 0;
            if ( hh != gh )
                return hh < gh;
            const Int32 cross = CORE::CInt128::Compare( CORE::CInt128::Multiply( hx, gy ), CORE::CInt128::Multiply( hy, gx ) );
            return cross > 0 || ( 0 == cross && h < g );
        }

        const SPolygonClipperEngine* m_engine;
    };

    /** Orders fragment indices by the y of their lower end (start), or of their upper end (end) */
    struct CFragmentYOrder
    {
        CFragmentYOrder( const SPolygonClipperEngine* engine, bool upper ) : m_engine( engine ), m_upper( upper ) {}

        bool operator()( UInt32 f, UInt32 g ) const
        {
            const Int64 fy = m_upper ? m_engine->UpperY( f ) : m_engine->LowerY( f );
            const Int64 gy = m_upper ? m_engine->UpperY( g ) : m_engine->LowerY( g );
            return fy < gy || ( fy == gy && f < g );
        }

        const SPolygonClipperEngine* m_engine;
        bool m_upper;
    };

    /** Orders hot pixel indices by y */
    struct CHotYOrder
    {
        explicit CHotYOrder( const SGridPoint* hot ) : m_hot( hot ) {}
        bool operator()( UInt32 a, UInt32 b ) const { return m_hot[ a ].y < m_hot[ b ].y || ( m_hot[ a ].y == m_hot[ b ].y && a < b ); }
        const SGridPoint* m_hot;
    };

    enum { NoIndex = 0xFFFFFFFFU };

    // input
    GUCEF::vector< SGridPoint > points;
    GUCEF::vector< SRing > rings;
    bool keepCollinear;

    // arrangement
    GUCEF::vector< SSegment > segments;
    GUCEF::vector< UInt32 > order;
    GUCEF::vector< UInt32 > active;
    GUCEF::vector< SGridPoint > hot;
    GUCEF::vector< SHit > hits;
    GUCEF::vector< SFragment > fragments;
    GUCEF::vector< GUCEF::vector< UInt32 > > buckets;   /**< the active segments of the sweep per band in y */
    GUCEF::vector< UInt32 > bandOf;                     /**< the first band of each segment */
    GUCEF::vector< UInt32 > columns;                    /**< the first hot pixel of each distinct x */
    GUCEF::vector< SGridPoint > ringScratch;
    GUCEF::vector< UInt32 > ringIndices;
    GUCEF::vector< UInt32 > degree;

    // graph
    GUCEF::vector< UInt32 > outOffset;
    GUCEF::vector< UInt32 > outList;
    GUCEF::vector< UInt32 > position;
    GUCEF::vector< UInt32 > face;
    GUCEF::vector< UInt32 > faceFirst;
    GUCEF::vector< Int32 > faceWinding;
    GUCEF::vector< UInt8 > faceKnown;
    GUCEF::vector< UInt8 > faceResult;
    GUCEF::vector< UInt32 > component;
    GUCEF::vector< UInt32 > queue;
    GUCEF::vector< UInt8 > boundary;
    GUCEF::vector< UInt8 > used;

    // the sweep of the component windings
    GUCEF::vector< UInt32 > sweepStarts;
    GUCEF::vector< UInt32 > sweepEnds;
    GUCEF::vector< UInt32 > sweepRoots;
    GUCEF::vector< UInt32 > treapLeft;      /**< the sweep status: a treap of fragments in x order with the sums of their crossings */
    GUCEF::vector< UInt32 > treapRight;
    GUCEF::vector< Int32 > treapSum;
    GUCEF::vector< Int32 > rootWinding;
    GUCEF::vector< UInt32 > rootContaining;  /**< per component (its smallest vertex): the face of the other components that holds it */
    GUCEF::vector< UInt32 > rootFace;        /**< per component: its outer face */

    // the regions of the result (faces joined across edges inside the result)
    GUCEF::vector< UInt32 > region;
    GUCEF::vector< UInt32 > ringFace;
    GUCEF::vector< UInt32 > edgeScratch;
    GUCEF::vector< UInt32 > ringEdges;      /**< the half edges of each output ring, in the order of the rings */
    GUCEF::vector< UInt32 > ringEdgeCount;
    GUCEF::vector< UInt32 > regionOuter;
    GUCEF::vector< UInt32 > ringFirst;
    GUCEF::vector< Int32 > polygonOf;

    // output
    GUCEF::vector< SGridPoint > outPoints;
    GUCEF::vector< UInt32 > outSizes;
    GUCEF::vector< Int32 > outParents;
    GUCEF::vector< CORE::CBigInteger > outAreas;

    SPolygonClipperEngine( void ) : keepCollinear( false ) {}

    GUCEF_GEOMETRY_FORCE_INLINE UInt32 Origin( UInt32 h ) const { return 0 == ( h & 1 ) ? fragments[ h >> 1 ].a : fragments[ h >> 1 ].b; }
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 Target( UInt32 h ) const { return 0 == ( h & 1 ) ? fragments[ h >> 1 ].b : fragments[ h >> 1 ].a; }
    GUCEF_GEOMETRY_FORCE_INLINE Int32 Delta( UInt32 h, UInt32 operand ) const { return 0 == ( h & 1 ) ? fragments[ h >> 1 ].delta[ operand ] : -fragments[ h >> 1 ].delta[ operand ]; }

    void Direction( UInt32 h, Int64& dx, Int64& dy ) const
    {
        const SGridPoint& o = hot[ Origin( h ) ];
        const SGridPoint& t = hot[ Target( h ) ];
        dx = t.x - o.x;
        dy = t.y - o.y;
    }

    UInt32 FindRoot( UInt32 v )
    {
        while ( component[ v ] != v )
        {
            component[ v ] = component[ component[ v ] ];
            v = component[ v ];
        }
        return v;
    }

    UInt32 FindRegion( UInt32 f )
    {
        while ( region[ f ] != f )
        {
            region[ f ] = region[ region[ f ] ];
            f = region[ f ];
        }
        return f;
    }

    /** The end points of fragment f with the smaller and the larger y */
    GUCEF_GEOMETRY_FORCE_INLINE const SGridPoint& Lower( UInt32 f ) const { const SGridPoint& a = hot[ fragments[ f ].a ]; const SGridPoint& b = hot[ fragments[ f ].b ]; return a.y <= b.y ? a : b; }
    GUCEF_GEOMETRY_FORCE_INLINE const SGridPoint& Upper( UInt32 f ) const { const SGridPoint& a = hot[ fragments[ f ].a ]; const SGridPoint& b = hot[ fragments[ f ].b ]; return a.y <= b.y ? b : a; }
    GUCEF_GEOMETRY_FORCE_INLINE Int64 LowerY( UInt32 f ) const { return Lower( f ).y; }
    GUCEF_GEOMETRY_FORCE_INLINE Int64 UpperY( UInt32 f ) const { return Upper( f ).y; }

    /**
     *  True when fragment g (in the sweep status at the height of p, p not above its upper end) lies left of the point
     *  p: p is strictly right of g. A fragment that starts at p is not left of it.
     */
    bool LeftOfPoint( UInt32 g, const SGridPoint& p ) const { return Orient( Lower( g ), Upper( g ), p ) < 0; }

    /**
     *  True when fragment g lies left of fragment f; both span a common open interval of heights (snapped fragments
     *  meet only at their end points, so the order is the same over the whole interval)
     */
    bool StatusBefore( UInt32 g, UInt32 f ) const
    {
        const SGridPoint& gl = Lower( g );
        const SGridPoint& fl = Lower( f );
        if ( fl.y >= gl.y )
        {
            const Int32 side = Orient( gl, Upper( g ), fl );
            if ( 0 != side )
                return side < 0;
            // the same lower end: by direction
            return Orient( gl, Upper( g ), Upper( f ) ) < 0;
        }
        return Orient( fl, Upper( f ), gl ) > 0;
    }

    GUCEF_GEOMETRY_FORCE_INLINE UInt32 TreapPriority( UInt32 f ) const { UInt32 h = f * 2654435761U; h ^= h >> 15; h *= 2246822519U; h ^= h >> 13; return h; }

    GUCEF_GEOMETRY_FORCE_INLINE Int32 TreapTotal( UInt32 t, UInt32 operand ) const { return NoIndex == t ? 0 : treapSum[ 2 * t + operand ]; }

    void TreapUpdate( UInt32 t )
    {
        for ( UInt32 operand=0; operand<2; ++operand )
            treapSum[ 2 * t + operand ] = UpDelta( t, operand ) + TreapTotal( treapLeft[ t ], operand ) + TreapTotal( treapRight[ t ], operand );
    }

    UInt32 TreapMerge( UInt32 a, UInt32 b )
    {
        if ( NoIndex == a )
            return b;
        if ( NoIndex == b )
            return a;
        if ( TreapPriority( a ) > TreapPriority( b ) )
        {
            treapRight[ a ] = TreapMerge( treapRight[ a ], b );
            TreapUpdate( a );
            return a;
        }
        treapLeft[ b ] = TreapMerge( a, treapLeft[ b ] );
        TreapUpdate( b );
        return b;
    }

    /** Splits t into the fragments before f ('before') and the others ('after') */
    void TreapSplit( UInt32 t, UInt32 f, UInt32& before, UInt32& after )
    {
        if ( NoIndex == t )
        {
            before = after = NoIndex;
            return;
        }
        if ( StatusBefore( t, f ) )
        {
            TreapSplit( treapRight[ t ], f, treapRight[ t ], after );
            before = t;
        }
        else
        {
            TreapSplit( treapLeft[ t ], f, before, treapLeft[ t ] );
            after = t;
        }
        TreapUpdate( t );
    }

    /** Removes the first fragment of t */
    UInt32 TreapRemoveFirst( UInt32 t )
    {
        if ( NoIndex == treapLeft[ t ] )
            return treapRight[ t ];
        treapLeft[ t ] = TreapRemoveFirst( treapLeft[ t ] );
        TreapUpdate( t );
        return t;
    }

    /** The last fragment of t that is left of q; NoIndex when there is none */
    UInt32 TreapLastLeftOf( UInt32 t, const SGridPoint& q ) const
    {
        UInt32 last = NoIndex;
        while ( NoIndex != t )
        {
            if ( LeftOfPoint( t, q ) )
            {
                last = t;
                t = treapRight[ t ];
            }
            else
                t = treapLeft[ t ];
        }
        return last;
    }

    /** The sum of the crossings of the fragments of t that are not left of q */
    Int32 TreapSumRightOf( UInt32 t, const SGridPoint& q, UInt32 operand ) const
    {
        Int32 sum = 0;
        while ( NoIndex != t )
        {
            if ( LeftOfPoint( t, q ) )
                t = treapRight[ t ];
            else
            {
                sum += UpDelta( t, operand ) + TreapTotal( treapRight[ t ], operand );
                t = treapLeft[ t ];
            }
        }
        return sum;
    }

    /** The winding change of fragment f for a crossing from right to left of an upward ray at its height */
    GUCEF_GEOMETRY_FORCE_INLINE Int32 UpDelta( UInt32 f, UInt32 operand ) const { return hot[ fragments[ f ].b ].y > hot[ fragments[ f ].a ].y ? fragments[ f ].delta[ operand ] : -fragments[ f ].delta[ operand ]; }

    /** The next half edge of the face left of h: clockwise from the twin at the target */
    UInt32 Next( UInt32 h ) const
    {
        const UInt32 twin = h ^ 1;
        const UInt32 v = Origin( twin );
        const UInt32 degree = outOffset[ v + 1 ] - outOffset[ v ];
        return outList[ outOffset[ v ] + ( position[ twin ] + degree - 1 ) % degree ];
    }

    /** Adds hot pixel 'pixel' to the hits of segment s when the segment passes through it */
    GUCEF_GEOMETRY_FORCE_INLINE void AddHit( const SSegment& s, UInt32 pixel, Float64 fdx, Float64 fdy, Float64 length )
    {
        const SGridPoint& c = hot[ pixel ];
        // a Float64 bound on the distance to the line first: a hit needs a distance below sqrt( 2 ) / 2
        const Float64 t1 = fdx * (Float64) ( c.y - s.a.y );
        const Float64 t2 = fdy * (Float64) ( c.x - s.a.x );
        const Float64 error = ( fabs( t1 ) + fabs( t2 ) ) * 4.5e-16;
        if ( fabs( t1 - t2 ) - error > 0.75 * length )
            return;
        if ( !PixelHit( s.a, s.b, c ) )
            return;
        SHit hit;
        hit.projection = CORE::CInt128::Add( CORE::CInt128::Multiply( c.x - s.a.x, s.b.x - s.a.x ), CORE::CInt128::Multiply( c.y - s.a.y, s.b.y - s.a.y ) );
        hit.pixel = pixel;
        hits.push_back( hit );
    }

    void BuildSegments( void );
    void FindHotPixels( void );
    void BuildFragments( void );
    void BuildGraph( void );
    void ComputeWindings( bool needContaining );
    void ExtractRings( TPolygonBooleanOperation operation, TPolygonFillRule subjectFill, TPolygonFillRule clipFill );
    void AssignParents( void );

    void Execute( TPolygonBooleanOperation operation, TPolygonFillRule subjectFill, TPolygonFillRule clipFill, bool parents )
    {
        outPoints.clear();
        outSizes.clear();
        outParents.clear();
        outAreas.clear();
        BuildSegments();
        if ( segments.empty() )
            return;
        FindHotPixels();
        BuildFragments();
        if ( fragments.empty() )
            return;
        BuildGraph();
        ComputeWindings( parents );
        ExtractRings( operation, subjectFill, clipFill );
        if ( parents )
            AssignParents();
    }
};

/*-------------------------------------------------------------------------*/

void
SPolygonClipperEngine::BuildSegments( void )
{
    segments.clear();
    for ( size_t r=0; r<rings.size(); ++r )
    {
        const SRing& ring = rings[ r ];
        // without consecutive duplicates
        ringScratch.clear();
        for ( UInt32 i=0; i<ring.count; ++i )
        {
            const SGridPoint& p = points[ ring.first + i ];
            if ( ringScratch.empty() || !GridEqual( ringScratch.back(), p ) )
                ringScratch.push_back( p );
        }
        while ( ringScratch.size() > 1 && GridEqual( ringScratch.back(), ringScratch.front() ) )
            ringScratch.pop_back();
        const UInt32 n = (UInt32) ringScratch.size();
        if ( n < 2 )
            continue;
        Int32 direction = 1;
        if ( 0 != ring.orientation )
        {
            CORE::CBigInteger area;
            RingDoubleArea( &ringScratch[ 0 ], n, area );
            if ( area.GetSign() * ring.orientation < 0 )
                direction = -1;
        }
        for ( UInt32 i=0; i<n; ++i )
        {
            const SGridPoint& p = ringScratch[ i ];
            const SGridPoint& q = ringScratch[ ( i + 1 ) % n ];
            SSegment s;
            const bool forward = GridLess( p, q );
            s.a = forward ? p : q;
            s.b = forward ? q : p;
            s.delta[ 0 ] = 0;
            s.delta[ 1 ] = 0;
            s.delta[ ring.operand ] = forward ? direction : -direction;
            segments.push_back( s );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
SPolygonClipperEngine::FindHotPixels( void )
{
    hot.clear();
    for ( size_t i=0; i<segments.size(); ++i )
    {
        hot.push_back( segments[ i ].a );
        hot.push_back( segments[ i ].b );
    }
    /*
     *  The proper crossings: a sweep over x with the active segments kept per band of y (equal bands over the y range of
     *  the input), so a segment is tested only against active segments in the bands it spans. A pair is tested in the
     *  first band both span. The band count is lowered while the segments would span more than eight bands each on
     *  average, so the bands hold O( n ) entries.
     */
    const UInt32 n = (UInt32) segments.size();
    order.resize( n );
    for ( UInt32 i=0; i<n; ++i )
        order[ i ] = i;
    std::sort( order.begin(), order.end(), CSegmentOrder( &segments[ 0 ] ) );
    Int64 minY = segments[ 0 ].a.y, maxY = minY;
    for ( UInt32 i=0; i<n; ++i )
    {
        minY = std::min( minY, std::min( segments[ i ].a.y, segments[ i ].b.y ) );
        maxY = std::max( maxY, std::max( segments[ i ].a.y, segments[ i ].b.y ) );
    }
    const Int64 extent = maxY - minY + 1;
    Int64 bandCount = n < 1024 ? 1 : std::max( (Int64) 1, std::min( (Int64) n / 8, extent ) );
    Int64 bandHeight = 1;
    for ( ;; )
    {
        bandHeight = ( extent + bandCount - 1 ) / bandCount;
        if ( 1 == bandCount )
            break;
        UInt64 spans = 0;
        for ( UInt32 i=0; i<n; ++i )
            spans += (UInt64) ( ( std::max( segments[ i ].a.y, segments[ i ].b.y ) - minY ) / bandHeight - ( std::min( segments[ i ].a.y, segments[ i ].b.y ) - minY ) / bandHeight + 1 );
        if ( spans <= 8 * (UInt64) n )
            break;
        bandCount = std::max( (Int64) 1, bandCount / 4 );
    }
    if ( buckets.size() < (size_t) bandCount )
        buckets.resize( (size_t) bandCount );
    bandOf.resize( n );
    for ( UInt32 i=0; i<n; ++i )
        bandOf[ i ] = (UInt32) ( ( std::min( segments[ i ].a.y, segments[ i ].b.y ) - minY ) / bandHeight );
    for ( Int64 b=0; b<bandCount; ++b )
        buckets[ (size_t) b ].clear();
    for ( UInt32 k=0; k<n; ++k )
    {
        const SSegment& s = segments[ order[ k ] ];
        const Int64 sMinY = std::min( s.a.y, s.b.y ), sMaxY = std::max( s.a.y, s.b.y );
        const Int64 first = ( sMinY - minY ) / bandHeight, last = ( sMaxY - minY ) / bandHeight;
        for ( Int64 band=first; band<=last; ++band )
        {
            GUCEF::vector< UInt32 >& list = buckets[ (size_t) band ];
            size_t keep = 0;
            for ( size_t j=0; j<list.size(); ++j )
            {
                const SSegment& t = segments[ list[ j ] ];
                if ( t.b.x < s.a.x )
                    continue;
                list[ keep++ ] = list[ j ];
                const Int64 tMinY = std::min( t.a.y, t.b.y ), tMaxY = std::max( t.a.y, t.b.y );
                if ( tMaxY < sMinY || tMinY > sMaxY )
                    continue;
                // the first band both span
                if ( band != std::max( first, (Int64) bandOf[ list[ keep - 1 ] ] ) )
                    continue;
                const Int32 o1 = Orient( s.a, s.b, t.a ), o2 = Orient( s.a, s.b, t.b );
                if ( o1 * o2 >= 0 )
                    continue;
                const Int32 o3 = Orient( t.a, t.b, s.a ), o4 = Orient( t.a, t.b, s.b );
                if ( o3 * o4 >= 0 )
                    continue;
                hot.push_back( RoundedCrossing( s.a, s.b, t.a, t.b ) );
            }
            list.resize( keep );
            list.push_back( order[ k ] );
        }
    }
    std::sort( hot.begin(), hot.end(), CGridLess() );
    size_t unique = 0;
    for ( size_t i=0; i<hot.size(); ++i )
        if ( 0 == unique || !GridEqual( hot[ unique - 1 ], hot[ i ] ) )
            hot[ unique++ ] = hot[ i ];
    hot.resize( unique );
}

/*-------------------------------------------------------------------------*/

void
SPolygonClipperEngine::BuildFragments( void )
{
    fragments.clear();
    // the columns: hot is sorted by x, then y
    columns.clear();
    for ( UInt32 i=0; i<(UInt32) hot.size(); ++i )
        if ( 0 == i || hot[ i ].x != hot[ i - 1 ].x )
            columns.push_back( i );
    for ( size_t si=0; si<segments.size(); ++si )
    {
        const SSegment& s = segments[ si ];
        const Int64 minY = std::min( s.a.y, s.b.y ), maxY = std::max( s.a.y, s.b.y );
        const Int64 dx = s.b.x - s.a.x, dy = s.b.y - s.a.y;
        const Float64 fdx = (Float64) dx, fdy = (Float64) dy;
        const Float64 length = sqrt( fdx * fdx + fdy * fdy );
        hits.clear();
        /*
         *  The candidates: the hot pixels in [ a.x, b.x ] within the y range of the segment; once many pixels were
         *  outside that range (long columns of pixels), per further column only the pixels within the y range of the
         *  segment over [ x - 1/2, x + 1/2 ], widened by a margin for the Float64 evaluation (exact up to 2^53)
         */
        const UInt32 hotCount = (UInt32) hot.size();
        UInt32 i = (UInt32) ( std::lower_bound( hot.begin(), hot.end(), s.a.x, CGridXLess() ) - hot.begin() );
        UInt32 skipped = 0;
        for ( ; i<hotCount && hot[ i ].x <= s.b.x; ++i )
        {
            if ( hot[ i ].y < minY || hot[ i ].y > maxY )
            {
                if ( ++skipped > 32 )
                    break;
                continue;
            }
            AddHit( s, i, fdx, fdy, length );
        }
        if ( i < hotCount && hot[ i ].x <= s.b.x )
        {
            // many pixels outside the y range: from here on the columns by their y windows (pixels before i are done)
            const UInt32 resume = i;
            UInt32 column = (UInt32) ( std::lower_bound( columns.begin(), columns.end(), hot[ i ].x, CColumnLess( &hot[ 0 ] ) ) - columns.begin() );
            for ( ; column<(UInt32) columns.size() && hot[ columns[ column ] ].x <= s.b.x; ++column )
            {
                const Int64 cx = hot[ columns[ column ] ].x;
                const UInt32 columnEnd = column + 1 == (UInt32) columns.size() ? hotCount : columns[ column + 1 ];
                Int64 low = minY, high = maxY;
                if ( 0 != dx )
                {
                    const Float64 x0 = std::max( (Float64) cx - 0.5, (Float64) s.a.x ), x1 = std::min( (Float64) cx + 0.5, (Float64) s.b.x );
                    const Float64 y0 = (Float64) s.a.y + ( x0 - (Float64) s.a.x ) * fdy / fdx;
                    const Float64 y1 = (Float64) s.a.y + ( x1 - (Float64) s.a.x ) * fdy / fdx;
                    const Float64 margin = 4.0 + 1e-15 * ( fabs( y0 ) + fabs( y1 ) );
                    const Float64 lowF = floor( std::min( y0, y1 ) - margin ), highF = ceil( std::max( y0, y1 ) + margin );
                    low = std::max( low, lowF > (Float64) minY ? (Int64) lowF : minY );
                    high = std::min( high, highF < (Float64) maxY ? (Int64) highF : maxY );
                }
                UInt32 k = (UInt32) ( std::lower_bound( hot.begin() + std::max( columns[ column ], resume ), hot.begin() + columnEnd, low, CGridYLess() ) - hot.begin() );
                for ( ; k<columnEnd && hot[ k ].y <= high; ++k )
                    AddHit( s, k, fdx, fdy, length );
            }
        }
        std::sort( hits.begin(), hits.end() );
        for ( size_t k=0; k+1<hits.size(); ++k )
        {
            const UInt32 p = hits[ k ].pixel, q = hits[ k + 1 ].pixel;
            if ( p == q )
                continue;
            SFragment f;
            const bool forward = p < q;
            f.a = forward ? p : q;
            f.b = forward ? q : p;
            f.delta[ 0 ] = forward ? s.delta[ 0 ] : -s.delta[ 0 ];
            f.delta[ 1 ] = forward ? s.delta[ 1 ] : -s.delta[ 1 ];
            fragments.push_back( f );
        }
    }
    // equal fragments merge with their winding changes summed; those that cancel are dropped
    std::sort( fragments.begin(), fragments.end() );
    size_t unique = 0;
    for ( size_t i=0; i<fragments.size(); ++i )
    {
        if ( unique > 0 && fragments[ unique - 1 ].a == fragments[ i ].a && fragments[ unique - 1 ].b == fragments[ i ].b )
        {
            fragments[ unique - 1 ].delta[ 0 ] += fragments[ i ].delta[ 0 ];
            fragments[ unique - 1 ].delta[ 1 ] += fragments[ i ].delta[ 1 ];
            continue;
        }
        if ( unique > 0 && 0 == fragments[ unique - 1 ].delta[ 0 ] && 0 == fragments[ unique - 1 ].delta[ 1 ] )
            --unique;
        fragments[ unique++ ] = fragments[ i ];
    }
    if ( unique > 0 && 0 == fragments[ unique - 1 ].delta[ 0 ] && 0 == fragments[ unique - 1 ].delta[ 1 ] )
        --unique;
    fragments.resize( unique );
}

/*-------------------------------------------------------------------------*/

void
SPolygonClipperEngine::BuildGraph( void )
{
    const UInt32 vertexCount = (UInt32) hot.size();
    const UInt32 halfEdges = 2 * (UInt32) fragments.size();
    outOffset.assign( vertexCount + 1, 0 );
    for ( UInt32 h=0; h<halfEdges; ++h )
        ++outOffset[ Origin( h ) + 1 ];
    for ( UInt32 v=0; v<vertexCount; ++v )
        outOffset[ v + 1 ] += outOffset[ v ];
    outList.resize( halfEdges );
    position.resize( halfEdges );
    GUCEF::vector< UInt32 >& fill = queue;
    fill.assign( outOffset.begin(), outOffset.end() - 1 );
    for ( UInt32 h=0; h<halfEdges; ++h )
        outList[ fill[ Origin( h ) ]++ ] = h;
    const CAngleOrder angleOrder( this );
    for ( UInt32 v=0; v<vertexCount; ++v )
    {
        if ( outOffset[ v + 1 ] - outOffset[ v ] > 1 )
            std::sort( outList.begin() + outOffset[ v ], outList.begin() + outOffset[ v + 1 ], angleOrder );
        for ( UInt32 i=outOffset[ v ]; i<outOffset[ v + 1 ]; ++i )
            position[ outList[ i ] ] = i - outOffset[ v ];
    }
    // the faces: the cycles of Next
    face.assign( halfEdges, (UInt32) NoIndex );
    faceFirst.clear();
    for ( UInt32 h=0; h<halfEdges; ++h )
    {
        if ( NoIndex != face[ h ] )
            continue;
        const UInt32 id = (UInt32) faceFirst.size();
        faceFirst.push_back( h );
        UInt32 e = h;
        do
        {
            face[ e ] = id;
            e = Next( e );
        }
        while ( e != h );
    }
}

/*-------------------------------------------------------------------------*/

void
SPolygonClipperEngine::ComputeWindings( bool needContaining )
{
    const UInt32 vertexCount = (UInt32) hot.size();
    const UInt32 faces = (UInt32) faceFirst.size();
    faceWinding.assign( 2 * (size_t) faces, 0 );
    faceKnown.assign( faces, 0 );
    // the connected components (union-find over the fragments)
    component.resize( vertexCount );
    for ( UInt32 v=0; v<vertexCount; ++v )
        component[ v ] = v;
    for ( size_t f=0; f<fragments.size(); ++f )
    {
        const UInt32 a = FindRoot( fragments[ f ].a ), b = FindRoot( fragments[ f ].b );
        if ( a != b )
            component[ a < b ? b : a ] = a < b ? a : b;
    }
    /*
     *  The winding numbers at the smallest vertex q of each component (just left of it, in the outer face of the
     *  component): the sum of the crossings of the ray from q to +x, half open in y. One sweep upwards over the heights
     *  of the hot pixels keeps the fragments that span the current height ordered by x in a treap with the sums of
     *  their crossings (snapped fragments never cross, so the order only changes where fragments start or end), and the
     *  crossings of a ray are the fragments after the position of q: O( log n ) per event. With few components the
     *  rays are summed directly. The same pass finds the face of the other components that holds each component.
     */
    sweepRoots.clear();
    for ( UInt32 v=0; v<vertexCount; ++v )
        if ( outOffset[ v + 1 ] != outOffset[ v ] && FindRoot( v ) == v )
            sweepRoots.push_back( v );
    rootWinding.assign( 2 * (size_t) vertexCount, 0 );
    rootContaining.assign( vertexCount, (UInt32) NoIndex );
    rootFace.assign( vertexCount, (UInt32) NoIndex );
    if ( sweepRoots.size() <= 256 )
    {
        // few components: the rays directly
        for ( size_t r=0; r<sweepRoots.size(); ++r )
        {
            const UInt32 v = sweepRoots[ r ];
            const SGridPoint& q = hot[ v ];
            UInt32 left = NoIndex;
            for ( UInt32 f=0; f<(UInt32) fragments.size(); ++f )
            {
                const SGridPoint& a = hot[ fragments[ f ].a ];
                const SGridPoint& b = hot[ fragments[ f ].b ];
                const bool up = a.y <= q.y && b.y > q.y;
                if ( !up && !( b.y <= q.y && a.y > q.y ) )
                    continue;
                const Int32 side = Orient( a, b, q );
                if ( up ? side >= 0 : side <= 0 )
                {
                    rootWinding[ 2 * v ] += up ? fragments[ f ].delta[ 0 ] : -fragments[ f ].delta[ 0 ];
                    rootWinding[ 2 * v + 1 ] += up ? fragments[ f ].delta[ 1 ] : -fragments[ f ].delta[ 1 ];
                }
                else
                if ( needContaining && ( NoIndex == left || StatusBefore( left, f ) ) )
                    left = f;
            }
            rootContaining[ v ] = NoIndex == left ? (UInt32) NoIndex : face[ ( hot[ fragments[ left ].b ].y > hot[ fragments[ left ].a ].y ? 2 * left : 2 * left + 1 ) ^ 1 ];
        }
    }
    else
    {
        sweepStarts.clear();
        for ( UInt32 f=0; f<(UInt32) fragments.size(); ++f )
            if ( hot[ fragments[ f ].a ].y != hot[ fragments[ f ].b ].y )
                sweepStarts.push_back( f );
        sweepEnds.assign( sweepStarts.begin(), sweepStarts.end() );
        std::sort( sweepStarts.begin(), sweepStarts.end(), CFragmentYOrder( this, false ) );
        std::sort( sweepEnds.begin(), sweepEnds.end(), CFragmentYOrder( this, true ) );
        std::sort( sweepRoots.begin(), sweepRoots.end(), CHotYOrder( &hot[ 0 ] ) );
        treapLeft.assign( fragments.size(), (UInt32) NoIndex );
        treapRight.assign( fragments.size(), (UInt32) NoIndex );
        treapSum.assign( 2 * fragments.size(), 0 );
        UInt32 root = NoIndex;
        size_t nextStart = 0, nextEnd = 0, nextRoot = 0;
        while ( nextRoot < sweepRoots.size() )
        {
            // the next height with an event
            Int64 y = hot[ sweepRoots[ nextRoot ] ].y;
            if ( nextStart < sweepStarts.size() && LowerY( sweepStarts[ nextStart ] ) < y )
                y = LowerY( sweepStarts[ nextStart ] );
            if ( nextEnd < sweepEnds.size() && UpperY( sweepEnds[ nextEnd ] ) < y )
                y = UpperY( sweepEnds[ nextEnd ] );
            // the fragments that end here leave (half open in y)
            while ( nextEnd < sweepEnds.size() && UpperY( sweepEnds[ nextEnd ] ) == y )
            {
                const UInt32 f = sweepEnds[ nextEnd++ ];
                UInt32 before = NoIndex, after = NoIndex;
                TreapSplit( root, f, before, after );
                root = TreapMerge( before, NoIndex == after ? after : TreapRemoveFirst( after ) );
            }
            // the fragments that start here enter at their place in x
            while ( nextStart < sweepStarts.size() && LowerY( sweepStarts[ nextStart ] ) == y )
            {
                const UInt32 f = sweepStarts[ nextStart++ ];
                TreapUpdate( f );
                UInt32 before = NoIndex, after = NoIndex;
                TreapSplit( root, f, before, after );
                root = TreapMerge( TreapMerge( before, f ), after );
            }
            // the components whose smallest vertex is at this height
            while ( nextRoot < sweepRoots.size() && hot[ sweepRoots[ nextRoot ] ].y == y )
            {
                const UInt32 v = sweepRoots[ nextRoot++ ];
                rootWinding[ 2 * v ] = TreapSumRightOf( root, hot[ v ], 0 );
                rootWinding[ 2 * v + 1 ] = TreapSumRightOf( root, hot[ v ], 1 );
                // the face that holds the component: right of the nearest fragment left of v (the component lies at x >= v.x)
                const UInt32 left = needContaining ? TreapLastLeftOf( root, hot[ v ] ) : (UInt32) NoIndex;
                rootContaining[ v ] = NoIndex == left ? (UInt32) NoIndex : face[ ( hot[ fragments[ left ].b ].y > hot[ fragments[ left ].a ].y ? 2 * left : 2 * left + 1 ) ^ 1 ];
            }
        }
    }

    for ( UInt32 v=0; v<vertexCount; ++v )
    {
        if ( outOffset[ v + 1 ] == outOffset[ v ] || FindRoot( v ) != v )
            continue;
        // v is the lexicographically smallest vertex of its component: its outer face holds the point v - ( eps, 0 )
        UInt32 outer = outList[ outOffset[ v + 1 ] - 1 ];
        for ( UInt32 i=outOffset[ v ]; i<outOffset[ v + 1 ]; ++i )
        {
            Int64 dx, dy;
            Direction( outList[ i ], dx, dy );
            if ( !( dy < 0 || ( 0 == dy && dx < 0 ) ) )
                outer = outList[ i ];
        }
        const UInt32 start = face[ outer ];
        rootFace[ v ] = start;
        const Int32 winding[ 2 ] = { rootWinding[ 2 * v ], rootWinding[ 2 * v + 1 ] };
        if ( 0 != faceKnown[ start ] )
            continue;
        faceKnown[ start ] = 1;
        faceWinding[ 2 * start ] = winding[ 0 ];
        faceWinding[ 2 * start + 1 ] = winding[ 1 ];
        queue.clear();
        queue.push_back( start );
        for ( size_t head=0; head<queue.size(); ++head )
        {
            const UInt32 f = queue[ head ];
            const UInt32 first = faceFirst[ f ];
            UInt32 e = first;
            do
            {
                const UInt32 g = face[ e ^ 1 ];
                if ( 0 == faceKnown[ g ] )
                {
                    faceKnown[ g ] = 1;
                    faceWinding[ 2 * g ] = faceWinding[ 2 * f ] - Delta( e, 0 );
                    faceWinding[ 2 * g + 1 ] = faceWinding[ 2 * f + 1 ] - Delta( e, 1 );
                    queue.push_back( g );
                }
                e = Next( e );
            }
            while ( e != first );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
SPolygonClipperEngine::ExtractRings( TPolygonBooleanOperation operation, TPolygonFillRule subjectFill, TPolygonFillRule clipFill )
{
    const UInt32 faces = (UInt32) faceFirst.size();
    faceResult.resize( faces );
    for ( UInt32 f=0; f<faces; ++f )
    {
        const bool s = Fill( faceWinding[ 2 * f ], subjectFill );
        const bool c = Fill( faceWinding[ 2 * f + 1 ], clipFill );
        bool inside = false;
        switch ( operation )
        {
            case POLYGONBOOLEAN_UNION: inside = s || c; break;
            case POLYGONBOOLEAN_INTERSECTION: inside = s && c; break;
            case POLYGONBOOLEAN_DIFFERENCE: inside = s && !c; break;
            default: inside = s != c; break;
        }
        faceResult[ f ] = inside ? 1 : 0;
    }
    const UInt32 halfEdges = (UInt32) face.size();
    boundary.resize( halfEdges );
    for ( UInt32 h=0; h<halfEdges; ++h )
        boundary[ h ] = ( 0 != faceResult[ face[ h ] ] && 0 == faceResult[ face[ h ^ 1 ] ] ) ? 1 : 0;
    used.assign( halfEdges, 0 );
    ringFace.clear();
    ringEdges.clear();
    ringEdgeCount.clear();
    // the boundary degree of each vertex: a straight-on vertex is removed only where no other output ring touches
    degree.assign( hot.size(), 0 );
    for ( UInt32 h=0; h<halfEdges; ++h )
        if ( 0 != boundary[ h ] )
            ++degree[ Origin( h ) ];
    for ( UInt32 h=0; h<halfEdges; ++h )
    {
        if ( 0 == boundary[ h ] || 0 != used[ h ] )
            continue;
        ringScratch.clear();
        ringIndices.clear();
        edgeScratch.clear();
        UInt32 e = h;
        do
        {
            used[ e ] = 1;
            edgeScratch.push_back( e );
            ringScratch.push_back( hot[ Origin( e ) ] );
            ringIndices.push_back( Origin( e ) );
            // the next boundary half edge: clockwise from the twin at the target
            const UInt32 twin = e ^ 1;
            const UInt32 v = Origin( twin );
            const UInt32 degree = outOffset[ v + 1 ] - outOffset[ v ];
            UInt32 next = NoIndex;
            for ( UInt32 i=1; i<=degree && NoIndex == next; ++i )
            {
                const UInt32 g = outList[ outOffset[ v ] + ( position[ twin ] + degree - i ) % degree ];
                if ( 0 != boundary[ g ] )
                    next = g;
            }
            e = next;
        }
        while ( NoIndex != e && e != h && 0 == used[ e ] );
        if ( !keepCollinear )
        {
            // straight-on vertices go (boundary rings have no fold backs)
            size_t n = 0;
            for ( size_t i=0; i<ringScratch.size(); ++i )
            {
                while ( n >= 2 && 1 == degree[ ringIndices[ n - 1 ] ] && 0 == Orient( ringScratch[ n - 2 ], ringScratch[ n - 1 ], ringScratch[ i ] ) )
                    --n;
                ringScratch[ n ] = ringScratch[ i ];
                ringIndices[ n++ ] = ringIndices[ i ];
            }
            size_t begin = 0;
            bool changed = true;
            while ( changed && n - begin >= 3 )
            {
                changed = false;
                if ( 1 == degree[ ringIndices[ n - 1 ] ] && 0 == Orient( ringScratch[ n - 2 ], ringScratch[ n - 1 ], ringScratch[ begin ] ) )
                {
                    --n;
                    changed = true;
                }
                else
                if ( 1 == degree[ ringIndices[ begin ] ] && 0 == Orient( ringScratch[ n - 1 ], ringScratch[ begin ], ringScratch[ begin + 1 ] ) )
                {
                    ++begin;
                    changed = true;
                }
            }
            if ( n - begin < 3 )
                continue;
            outPoints.insert( outPoints.end(), ringScratch.begin() + begin, ringScratch.begin() + n );
            outSizes.push_back( (UInt32) ( n - begin ) );
            ringFace.push_back( face[ h ] );
            ringEdges.insert( ringEdges.end(), edgeScratch.begin(), edgeScratch.end() );
            ringEdgeCount.push_back( (UInt32) edgeScratch.size() );
        }
        else
        {
            if ( ringScratch.size() < 3 )
                continue;
            outPoints.insert( outPoints.end(), ringScratch.begin(), ringScratch.end() );
            outSizes.push_back( (UInt32) ringScratch.size() );
            ringFace.push_back( face[ h ] );
            ringEdges.insert( ringEdges.end(), edgeScratch.begin(), edgeScratch.end() );
            ringEdgeCount.push_back( (UInt32) edgeScratch.size() );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
SPolygonClipperEngine::AssignParents( void )
{
    /*
     *  The result faces joined across the edges inside the result form the pieces of the region, together with the
     *  outer face of each component and the face of the other components around it (the same area); an outer ring
     *  (counter-clockwise) may run through several pieces that touch at vertices, so the faces left of all its edges are
     *  joined as well. Each joined region then has one outer ring, and its holes have a face of the region on their
     *  left: a hole belongs to the outer ring of its region, which is the smallest outer ring around it.
     */
    const size_t ringCount = outSizes.size();
    const UInt32 faces = (UInt32) faceFirst.size();
    const UInt32 halfEdges = (UInt32) face.size();
    region.resize( faces );
    for ( UInt32 f=0; f<faces; ++f )
        region[ f ] = f;
    for ( UInt32 h=0; h<halfEdges; h+=2 )
    {
        if ( 0 == faceResult[ face[ h ] ] || 0 == faceResult[ face[ h ^ 1 ] ] )
            continue;
        const UInt32 a = FindRegion( face[ h ] ), b = FindRegion( face[ h ^ 1 ] );
        if ( a != b )
            region[ a < b ? b : a ] = a < b ? a : b;
    }
    // the outer face of a component and the face of the other components around it are the same area
    for ( size_t i=0; i<sweepRoots.size(); ++i )
    {
        const UInt32 v = sweepRoots[ i ];
        if ( NoIndex == rootContaining[ v ] || NoIndex == rootFace[ v ] || 0 == faceResult[ rootFace[ v ] ] || 0 == faceResult[ rootContaining[ v ] ] )
            continue;
        const UInt32 a = FindRegion( rootFace[ v ] ), b = FindRegion( rootContaining[ v ] );
        if ( a != b )
            region[ a < b ? b : a ] = a < b ? a : b;
    }
    outParents.assign( ringCount, -1 );
    outAreas.resize( ringCount );
    regionOuter.assign( faces, (UInt32) NoIndex );
    UInt32 offset = 0, edgeOffset = 0;
    for ( size_t r=0; r<ringCount; ++r )
    {
        RingDoubleArea( &outPoints[ offset ], outSizes[ r ], outAreas[ r ] );
        if ( outAreas[ r ].GetSign() > 0 )
        {
            for ( UInt32 i=1; i<ringEdgeCount[ r ]; ++i )
            {
                const UInt32 a = FindRegion( face[ ringEdges[ edgeOffset ] ] ), b = FindRegion( face[ ringEdges[ edgeOffset + i ] ] );
                if ( a != b )
                    region[ a < b ? b : a ] = a < b ? a : b;
            }
        }
        offset += outSizes[ r ];
        edgeOffset += ringEdgeCount[ r ];
    }
    for ( size_t r=0; r<ringCount; ++r )
        if ( outAreas[ r ].GetSign() > 0 )
            regionOuter[ FindRegion( ringFace[ r ] ) ] = (UInt32) r;
    for ( size_t r=0; r<ringCount; ++r )
    {
        if ( outAreas[ r ].GetSign() >= 0 )
            continue;
        const UInt32 outer = regionOuter[ FindRegion( ringFace[ r ] ) ];
        outParents[ r ] = NoIndex == outer ? -1 : (Int32) outer;
    }
}

/*-------------------------------------------------------------------------*/

namespace {

/** The grid conversions of a kind: 0 floats (scaled by a power of two), 1 integers, 2 fixed point (raw values) */
template < typename T, int Strategy = MATH::CTNumericTraits< T >::IsFloatingPoint ? 0 : ( MATH::CTNumericTraits< T >::IsFixedPoint ? 2 : 1 ) >
struct CTClipperGrid;

template < typename T >
struct CTClipperGrid< T, 0 >
{
    static bool IsFinite( const T& v ) { return MATH::CTNumericTraits< T >::IsFinite( v ); }
    static Float64 Magnitude( const T& v ) { return fabs( (Float64) v ); }
    static bool ToGrid( const T& v, Float64 scale, Int64& out )
    {
        const Float64 s = floor( (Float64) v * scale + 0.5 );
        if ( !( fabs( s ) <= (Float64) GridLimit ) )
            return false;
        out = (Int64) s;
        return true;
    }
    static T FromGrid( Int64 g, Float64 scale ) { return (T) ( (Float64) g / scale ); }
    static Float64 FixedScale( void ) { return 0.0; }
};

template < typename T >
struct CTClipperGrid< T, 1 >
{
    static bool IsFinite( const T& ) { return true; }
    static Float64 Magnitude( const T& v ) { return fabs( (Float64) v ); }
    static bool ToGrid( const T& v, Float64, Int64& out )
    {
        const Int64 value = (Int64) v;
        if ( value > GridLimit || value < -GridLimit )
            return false;
        out = value;
        return true;
    }
    static T FromGrid( Int64 g, Float64 ) { return (T) g; }
    static Float64 FixedScale( void ) { return 1.0; }
};

template < typename T >
struct CTClipperGrid< T, 2 >
{
    static bool IsFinite( const T& ) { return true; }
    static Float64 Magnitude( const T& v ) { return fabs( MATH::CTNumericTraits< T >::ToFloat64( v ) ); }
    static bool ToGrid( const T& v, Float64, Int64& out )
    {
        const Int64 value = (Int64) v.GetRaw();
        if ( value > GridLimit || value < -GridLimit )
            return false;
        out = value;
        return true;
    }
    static T FromGrid( Int64 g, Float64 ) { return T::FromRaw( (typename T::TStorageType) g ); }
    static Float64 FixedScale( void ) { return ldexp( 1.0, (int) T::FRACTION_BITS ); }
};

}  /* anonymous namespace */

/*-------------------------------------------------------------------------*/

template < typename T >
CTPolygonClipper< T >::CTPolygonClipper( void )
    : m_points()
    , m_rings()
    , m_engine( new SPolygonClipperEngine() )
    , m_requestedScale( 0.0 )
    , m_scale( 1.0 )
    , m_keepCollinear( false )
{
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTPolygonClipper< T >::~CTPolygonClipper()
{
    delete m_engine;
    m_engine = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTPolygonClipper< T >::SetGridScale( Float64 scale )
{
    if ( scale > 0.0 && scale == scale )
    {
        int exponent = 0;
        frexp( scale, &exponent );
        m_requestedScale = ldexp( 1.0, exponent - 1 );
    }
    else
        m_requestedScale = 0.0;
}

/*-------------------------------------------------------------------------*/

template < typename T >
Float64
CTPolygonClipper< T >::GetGridScale( void ) const
{
    return m_scale;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTPolygonClipper< T >::SetKeepCollinear( bool keep )
{
    m_keepCollinear = keep;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTPolygonClipper< T >::Clear( void )
{
    m_points.clear();
    m_rings.clear();
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTPolygonClipper< T >::AddRing( const TPoint* ring, UInt32 count, UInt32 operand, Int32 orientation )
{
    if ( GUCEF_NULL == ring || 0 == count )
        return;
    SRing info;
    info.first = (UInt32) m_points.size();
    info.count = count;
    info.operand = operand;
    info.orientation = orientation;
    m_points.insert( m_points.end(), ring, ring + count );
    m_rings.push_back( info );
}

/*-------------------------------------------------------------------------*/

template < typename T >
void CTPolygonClipper< T >::AddSubject( const TPoint* ring, UInt32 count ) { AddRing( ring, count, 0, 0 ); }

template < typename T >
void CTPolygonClipper< T >::AddSubject( const TPath& ring ) { AddRing( ring.empty() ? GUCEF_NULL : &ring[ 0 ], (UInt32) ring.size(), 0, 0 ); }

template < typename T >
void CTPolygonClipper< T >::AddSubject( const CTPolygon2D< T >& polygon ) { AddSubject( polygon.points ); }

template < typename T >
void
CTPolygonClipper< T >::AddSubject( const TPolygon& polygon )
{
    AddRing( polygon.outer.points.empty() ? GUCEF_NULL : &polygon.outer.points[ 0 ], (UInt32) polygon.outer.points.size(), 0, 1 );
    for ( size_t h=0; h<polygon.holes.size(); ++h )
        AddRing( polygon.holes[ h ].points.empty() ? GUCEF_NULL : &polygon.holes[ h ].points[ 0 ], (UInt32) polygon.holes[ h ].points.size(), 0, -1 );
}

template < typename T >
void CTPolygonClipper< T >::AddClip( const TPoint* ring, UInt32 count ) { AddRing( ring, count, 1, 0 ); }

template < typename T >
void CTPolygonClipper< T >::AddClip( const TPath& ring ) { AddRing( ring.empty() ? GUCEF_NULL : &ring[ 0 ], (UInt32) ring.size(), 1, 0 ); }

template < typename T >
void CTPolygonClipper< T >::AddClip( const CTPolygon2D< T >& polygon ) { AddClip( polygon.points ); }

template < typename T >
void
CTPolygonClipper< T >::AddClip( const TPolygon& polygon )
{
    AddRing( polygon.outer.points.empty() ? GUCEF_NULL : &polygon.outer.points[ 0 ], (UInt32) polygon.outer.points.size(), 1, 1 );
    for ( size_t h=0; h<polygon.holes.size(); ++h )
        AddRing( polygon.holes[ h ].points.empty() ? GUCEF_NULL : &polygon.holes[ h ].points[ 0 ], (UInt32) polygon.holes[ h ].points.size(), 1, -1 );
}

/*-------------------------------------------------------------------------*/

template < typename T >
TPolygonClipperStatus
CTPolygonClipper< T >::Run( TPolygonBooleanOperation operation, TPolygonFillRule subjectFill, TPolygonFillRule clipFill )
{
    typedef CTClipperGrid< T > TGrid;
    if ( (UInt32) operation > (UInt32) POLYGONBOOLEAN_XOR || (UInt32) subjectFill > (UInt32) POLYGONFILL_NEGATIVE || (UInt32) clipFill > (UInt32) POLYGONFILL_NEGATIVE )
        return POLYGONCLIPPER_INVALID_INPUT;
    Float64 largest = 0.0;
    for ( size_t i=0; i<m_points.size(); ++i )
    {
        if ( !TGrid::IsFinite( m_points[ i ].x ) || !TGrid::IsFinite( m_points[ i ].y ) )
            return POLYGONCLIPPER_INVALID_INPUT;
        largest = std::max( largest, std::max( TGrid::Magnitude( m_points[ i ].x ), TGrid::Magnitude( m_points[ i ].y ) ) );
    }
    m_scale = TGrid::FixedScale();
    if ( 0.0 == m_scale )
    {
        if ( m_requestedScale > 0.0 )
            m_scale = m_requestedScale;
        else
        if ( largest > 0.0 )
        {
            // the largest coordinate below 2^40; at most 2^1023, the largest finite power of two (tiny inputs)
            int exponent = 0;
            frexp( largest, &exponent );
            m_scale = ldexp( 1.0, std::min( 40 - exponent, 1023 ) );
        }
        else
            m_scale = 1.0;
    }
    SPolygonClipperEngine& engine = *m_engine;
    engine.points.resize( m_points.size() );
    for ( size_t i=0; i<m_points.size(); ++i )
    {
        if ( !TGrid::ToGrid( m_points[ i ].x, m_scale, engine.points[ i ].x ) || !TGrid::ToGrid( m_points[ i ].y, m_scale, engine.points[ i ].y ) )
            return POLYGONCLIPPER_OUT_OF_RANGE;
    }
    engine.rings.resize( m_rings.size() );
    for ( size_t r=0; r<m_rings.size(); ++r )
    {
        engine.rings[ r ].first = m_rings[ r ].first;
        engine.rings[ r ].count = m_rings[ r ].count;
        engine.rings[ r ].operand = m_rings[ r ].operand;
        engine.rings[ r ].orientation = m_rings[ r ].orientation;
    }
    engine.keepCollinear = m_keepCollinear;
    return POLYGONCLIPPER_OK;
}

/*-------------------------------------------------------------------------*/

template < typename T >
TPolygonClipperStatus
CTPolygonClipper< T >::Execute( TPolygonBooleanOperation operation, TPolygonFillRule subjectFill, TPolygonFillRule clipFill, TPaths& rings )
{
    typedef CTClipperGrid< T > TGrid;
    rings.clear();
    const TPolygonClipperStatus status = Run( operation, subjectFill, clipFill );
    if ( POLYGONCLIPPER_OK != status )
        return status;
    SPolygonClipperEngine& engine = *m_engine;
    engine.Execute( operation, subjectFill, clipFill, false );
    rings.resize( engine.outSizes.size() );
    size_t offset = 0;
    for ( size_t r=0; r<engine.outSizes.size(); ++r )
    {
        TPath& ring = rings[ r ];
        ring.resize( engine.outSizes[ r ] );
        for ( UInt32 i=0; i<engine.outSizes[ r ]; ++i )
        {
            ring[ i ].x = TGrid::FromGrid( engine.outPoints[ offset + i ].x, m_scale );
            ring[ i ].y = TGrid::FromGrid( engine.outPoints[ offset + i ].y, m_scale );
        }
        offset += engine.outSizes[ r ];
    }
    return POLYGONCLIPPER_OK;
}

/*-------------------------------------------------------------------------*/

template < typename T >
TPolygonClipperStatus
CTPolygonClipper< T >::Execute( TPolygonBooleanOperation operation, TPolygonFillRule subjectFill, TPolygonFillRule clipFill, TPolygons& polygons )
{
    typedef CTClipperGrid< T > TGrid;
    polygons.clear();
    const TPolygonClipperStatus status = Run( operation, subjectFill, clipFill );
    if ( POLYGONCLIPPER_OK != status )
        return status;
    SPolygonClipperEngine& engine = *m_engine;
    engine.Execute( operation, subjectFill, clipFill, true );
    const size_t ringCount = engine.outSizes.size();
    GUCEF::vector< UInt32 >& firsts = engine.ringFirst;
    GUCEF::vector< Int32 >& polygonOf = engine.polygonOf;
    firsts.resize( ringCount );
    polygonOf.assign( ringCount, -1 );
    UInt32 offset = 0;
    for ( size_t r=0; r<ringCount; ++r )
    {
        firsts[ r ] = offset;
        offset += engine.outSizes[ r ];
    }
    for ( size_t r=0; r<ringCount; ++r )
    {
        if ( engine.outAreas[ r ].GetSign() <= 0 )
            continue;
        polygonOf[ r ] = (Int32) polygons.size();
        polygons.push_back( TPolygon() );
        TPath& outer = polygons.back().outer.points;
        outer.resize( engine.outSizes[ r ] );
        for ( UInt32 i=0; i<engine.outSizes[ r ]; ++i )
        {
            outer[ i ].x = TGrid::FromGrid( engine.outPoints[ firsts[ r ] + i ].x, m_scale );
            outer[ i ].y = TGrid::FromGrid( engine.outPoints[ firsts[ r ] + i ].y, m_scale );
        }
    }
    for ( size_t r=0; r<ringCount; ++r )
    {
        if ( engine.outAreas[ r ].GetSign() >= 0 || engine.outParents[ r ] < 0 )
            continue;
        TPolygon& polygon = polygons[ (size_t) polygonOf[ (size_t) engine.outParents[ r ] ] ];
        polygon.holes.push_back( CTPolygon2D< T >() );
        TPath& hole = polygon.holes.back().points;
        hole.resize( engine.outSizes[ r ] );
        for ( UInt32 i=0; i<engine.outSizes[ r ]; ++i )
        {
            hole[ i ].x = TGrid::FromGrid( engine.outPoints[ firsts[ r ] + i ].x, m_scale );
            hole[ i ].y = TGrid::FromGrid( engine.outPoints[ firsts[ r ] + i ].y, m_scale );
        }
    }
    return POLYGONCLIPPER_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      INSTANTIATIONS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTPolygonClipper< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTPolygonClipper< Float64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTPolygonClipper< Int32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTPolygonClipper< Int64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTPolygonClipper< MATH::TFixed32_32 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
