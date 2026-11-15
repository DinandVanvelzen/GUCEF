/*
 *  gucefGEOMETRY_TestApp: GUCEF test application for the Geometry module
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

#ifndef GUCEF_GEOMETRY_TESTAPP_TESTPOLYGONREFERENCE_H
#define GUCEF_GEOMETRY_TESTAPP_TESTPOLYGONREFERENCE_H

/*
 *  Helpers of the polygon algorithm tests of the gucefGEOMETRY test application (triangulation, Delaunay, booleans,
 *  offsetting, convex decomposition): exact areas as fractions, exact checks of triangle sets, seeded generators of
 *  simple polygons on an integer grid that every kind represents exactly, and a scanline reference that measures a
 *  region given as rings with a fill rule (exact crossings per scanline, so the only error is the sampling in y).
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <algorithm>

#ifndef GUCEF_GEOMETRY_CTPREDICATES_H
#include "gucefGEOMETRY_CTPredicates.h"
#define GUCEF_GEOMETRY_CTPREDICATES_H
#endif /* GUCEF_GEOMETRY_CTPREDICATES_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "TestSpatialReference.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace PolygonReference {

typedef GUCEF::MATH::TMathBigFraction F;
typedef GUCEF::Int64 G;
typedef GUCEF::vector< GUCEF::MATH::CTPoint2D< GUCEF::Int64 > > TGridRing;
typedef GUCEF::vector< TGridRing > TGridRings;

using SpatialReference::ExactFraction;

inline F ExactFraction( const F& value ) { return value; }

/*-------------------------------------------------------------------------*/

/** Grid values of the kinds: integers as they are, floats and fractions in steps of 1/8, Q32.32 in steps of 1/8 */
inline GUCEF::Int32 MakeInt32( G g ) { return (GUCEF::Int32) g; }
inline GUCEF::Int64 MakeInt64( G g ) { return g; }
inline GUCEF::Float64 MakeFloat64( G g ) { return ldexp( (GUCEF::Float64) g, -3 ); }
inline GUCEF::Float32 MakeFloat32( G g ) { return (GUCEF::Float32) ldexp( (GUCEF::Float64) g, -3 ); }
inline GUCEF::MATH::TFixed32_32 MakeFixed( G g ) { return GUCEF::MATH::TFixed32_32::FromRaw( g * 536870912 ); }
inline F MakeFraction( G g ) { return F( g, 8 ); }

/*-------------------------------------------------------------------------*/

/** Milliseconds since an arbitrary start */
inline GUCEF::Float64 NowMs( void ) { return GUCEF::MT::PrecisionTimerTicksToMs( GUCEF::MT::PrecisionTickCount() ); }

/*-------------------------------------------------------------------------*/

/** Twice the signed area of a, b, c as a fraction */
template < typename T >
F
DoubleArea( const GUCEF::MATH::CTPoint2D< T >& a, const GUCEF::MATH::CTPoint2D< T >& b, const GUCEF::MATH::CTPoint2D< T >& c )
{
    const F ax = ExactFraction( a.x ), ay = ExactFraction( a.y );
    return ( ExactFraction( b.x ) - ax ) * ( ExactFraction( c.y ) - ay ) - ( ExactFraction( b.y ) - ay ) * ( ExactFraction( c.x ) - ax );
}

/*-------------------------------------------------------------------------*/

/** Twice the signed area of a ring (shoelace) as a fraction */
template < typename T >
F
RingDoubleArea( const GUCEF::MATH::CTPoint2D< T >* ring, GUCEF::UInt32 count )
{
    F sum( 0, 1 );
    for ( GUCEF::UInt32 i=1; i+1<count; ++i )
        sum = sum + DoubleArea( ring[ 0 ], ring[ i ], ring[ i + 1 ] );
    return sum;
}

/*-------------------------------------------------------------------------*/

inline F Abs( const F& value ) { return SpatialReference::SignOfFraction( value ) < 0 ? F( 0, 1 ) - value : value; }

inline GUCEF::Float64 ToFloat64( const F& value ) { return GUCEF::MATH::CTNumericTraits< F >::ToFloat64( value ); }

/*-------------------------------------------------------------------------*/

/** Twice the total area of the triangles (three indices each) as a fraction; 'allPositive' false when a triangle is not counter-clockwise */
template < typename T >
F
TrianglesDoubleArea( const GUCEF::MATH::CTPoint2D< T >* points, const GUCEF::vector< GUCEF::UInt32 >& triangles, bool& allPositive )
{
    F sum( 0, 1 );
    allPositive = true;
    for ( size_t t=0; t+2<triangles.size(); t+=3 )
    {
        const F area = DoubleArea( points[ triangles[ t ] ], points[ triangles[ t + 1 ] ], points[ triangles[ t + 2 ] ] );
        if ( SpatialReference::SignOfFraction( area ) <= 0 )
            allPositive = false;
        sum = sum + area;
    }
    return sum;
}

/*-------------------------------------------------------------------------*/

/*
 *  Reference predicates, independent of CTPredicates (the code under test): a Float64 evaluation with a coarse error
 *  bound in the largest coordinate magnitude, and exact fractions when the bound does not decide the sign. The bounds
 *  cover the rounding of the conversion to Float64 (Q32.32 and fractions are not always exact there) and of every
 *  operation, with a wide margin.
 */

template < typename T >
inline GUCEF::Float64
ReferenceFloat64( const T& value )
{
    return GUCEF::MATH::CTNumericTraits< T >::ToFloat64( value );
}

/** The sign of twice the signed area of a, b, c: 1 counter-clockwise, -1 clockwise, 0 collinear */
template < typename T >
GUCEF::Int32
ReferenceOrient( const GUCEF::MATH::CTPoint2D< T >& a, const GUCEF::MATH::CTPoint2D< T >& b, const GUCEF::MATH::CTPoint2D< T >& c )
{
    if ( ( c.x == a.x && c.y == a.y ) || ( c.x == b.x && c.y == b.y ) || ( a.x == b.x && a.y == b.y ) )
        return 0;
    const GUCEF::Float64 ax = ReferenceFloat64( a.x ), ay = ReferenceFloat64( a.y );
    const GUCEF::Float64 bx = ReferenceFloat64( b.x ), by = ReferenceFloat64( b.y );
    const GUCEF::Float64 cx = ReferenceFloat64( c.x ), cy = ReferenceFloat64( c.y );
    GUCEF::Float64 m = fabs( ax );
    m = std::max( m, fabs( ay ) ); m = std::max( m, fabs( bx ) ); m = std::max( m, fabs( by ) );
    m = std::max( m, fabs( cx ) ); m = std::max( m, fabs( cy ) );
    const GUCEF::Float64 det = ( bx - ax ) * ( cy - ay ) - ( by - ay ) * ( cx - ax );
    const GUCEF::Float64 bound = 128.0 * m * m * ldexp( 1.0, -53 );
    if ( det > bound )
        return 1;
    if ( det < -bound )
        return -1;
    return SpatialReference::SignOfFraction( DoubleArea( a, b, c ) );
}

/** The sign of the in-circle determinant of d against the counter-clockwise a, b, c: 1 strictly inside */
template < typename T >
GUCEF::Int32
ReferenceInCircle( const GUCEF::MATH::CTPoint2D< T >& a, const GUCEF::MATH::CTPoint2D< T >& b, const GUCEF::MATH::CTPoint2D< T >& c, const GUCEF::MATH::CTPoint2D< T >& d )
{
    const GUCEF::Float64 dx = ReferenceFloat64( d.x ), dy = ReferenceFloat64( d.y );
    const GUCEF::Float64 adx = ReferenceFloat64( a.x ) - dx, ady = ReferenceFloat64( a.y ) - dy;
    const GUCEF::Float64 bdx = ReferenceFloat64( b.x ) - dx, bdy = ReferenceFloat64( b.y ) - dy;
    const GUCEF::Float64 cdx = ReferenceFloat64( c.x ) - dx, cdy = ReferenceFloat64( c.y ) - dy;
    GUCEF::Float64 m = fabs( dx );
    m = std::max( m, fabs( dy ) );
    m = std::max( m, fabs( ReferenceFloat64( a.x ) ) ); m = std::max( m, fabs( ReferenceFloat64( a.y ) ) );
    m = std::max( m, fabs( ReferenceFloat64( b.x ) ) ); m = std::max( m, fabs( ReferenceFloat64( b.y ) ) );
    m = std::max( m, fabs( ReferenceFloat64( c.x ) ) ); m = std::max( m, fabs( ReferenceFloat64( c.y ) ) );
    const GUCEF::Float64 alift = adx * adx + ady * ady, blift = bdx * bdx + bdy * bdy, clift = cdx * cdx + cdy * cdy;
    const GUCEF::Float64 det = alift * ( bdx * cdy - cdx * bdy ) + blift * ( cdx * ady - adx * cdy ) + clift * ( adx * bdy - bdx * ady );
    const GUCEF::Float64 bound = 8192.0 * m * m * m * m * ldexp( 1.0, -53 );
    if ( det > bound )
        return 1;
    if ( det < -bound )
        return -1;
    const F fx = ExactFraction( d.x ), fy = ExactFraction( d.y );
    const F eax = ExactFraction( a.x ) - fx, eay = ExactFraction( a.y ) - fy;
    const F ebx = ExactFraction( b.x ) - fx, eby = ExactFraction( b.y ) - fy;
    const F ecx = ExactFraction( c.x ) - fx, ecy = ExactFraction( c.y ) - fy;
    const F ea = eax * eax + eay * eay, eb = ebx * ebx + eby * eby, ec = ecx * ecx + ecy * ecy;
    return SpatialReference::SignOfFraction( ea * ( ebx * ecy - ecx * eby ) + eb * ( ecx * eay - eax * ecy ) + ec * ( eax * eby - ebx * eay ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  True when no two of the counter-clockwise triangles share interior points: for every pair an edge of one of them
 *  has the other triangle on its closed outer side (exact reference orientation). At most 'maxTriangles' triangles
 *  are compared (the first ones).
 */
template < typename T >
bool
TrianglesDisjoint( const GUCEF::MATH::CTPoint2D< T >* points, const GUCEF::vector< GUCEF::UInt32 >& triangles, size_t maxTriangles )
{
    typedef GUCEF::MATH::CTPoint2D< T > TPoint;
    const size_t count = std::min( triangles.size() / 3, maxTriangles );
    for ( size_t i=0; i<count; ++i )
    {
        const TPoint* a[ 3 ] = { &points[ triangles[ 3 * i ] ], &points[ triangles[ 3 * i + 1 ] ], &points[ triangles[ 3 * i + 2 ] ] };
        for ( size_t j=i+1; j<count; ++j )
        {
            const TPoint* b[ 3 ] = { &points[ triangles[ 3 * j ] ], &points[ triangles[ 3 * j + 1 ] ], &points[ triangles[ 3 * j + 2 ] ] };
            bool separated = false;
            for ( int k=0; k<3 && !separated; ++k )
            {
                separated = ReferenceOrient( *a[ k ], *a[ ( k + 1 ) % 3 ], *b[ 0 ] ) <= 0 && ReferenceOrient( *a[ k ], *a[ ( k + 1 ) % 3 ], *b[ 1 ] ) <= 0 && ReferenceOrient( *a[ k ], *a[ ( k + 1 ) % 3 ], *b[ 2 ] ) <= 0;
                if ( !separated )
                    separated = ReferenceOrient( *b[ k ], *b[ ( k + 1 ) % 3 ], *a[ 0 ] ) <= 0 && ReferenceOrient( *b[ k ], *b[ ( k + 1 ) % 3 ], *a[ 1 ] ) <= 0 && ReferenceOrient( *b[ k ], *b[ ( k + 1 ) % 3 ], *a[ 2 ] ) <= 0;
            }
            if ( !separated )
                return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** True when the closed segments share a point (exact) */
template < typename T >
bool
SegmentsMeet( const GUCEF::MATH::CTPoint2D< T >& a, const GUCEF::MATH::CTPoint2D< T >& b, const GUCEF::MATH::CTPoint2D< T >& c, const GUCEF::MATH::CTPoint2D< T >& d )
{
    const GUCEF::Int32 o1 = ReferenceOrient( a, b, c ), o2 = ReferenceOrient( a, b, d );
    const GUCEF::Int32 o3 = ReferenceOrient( c, d, a ), o4 = ReferenceOrient( c, d, b );
    if ( o1 * o2 > 0 || o3 * o4 > 0 )
        return false;
    if ( 0 != o1 || 0 != o2 || 0 != o3 || 0 != o4 )
        return true;
    return !( std::max( a.x, b.x ) < std::min( c.x, d.x ) || std::max( c.x, d.x ) < std::min( a.x, b.x ) ||
              std::max( a.y, b.y ) < std::min( c.y, d.y ) || std::max( c.y, d.y ) < std::min( a.y, b.y ) );
}

/*-------------------------------------------------------------------------*/

/** True when the rings (given as point arrays) are each simple and no two touch: only neighbouring edges of one ring meet, at their shared vertex */
template < typename T >
bool
RingsSimpleAndDisjoint( const GUCEF::vector< GUCEF::vector< GUCEF::MATH::CTPoint2D< T > > >& rings )
{
    for ( size_t r=0; r<rings.size(); ++r )
    {
        const size_t n = rings[ r ].size();
        if ( n < 3 )
            return false;
        for ( size_t s=r; s<rings.size(); ++s )
        {
            const size_t m = rings[ s ].size();
            for ( size_t i=0; i<n; ++i )
            {
                for ( size_t j=( r == s ? i + 1 : 0 ); j<m; ++j )
                {
                    const bool neighbours = r == s && ( j == i + 1 || ( 0 == i && j == n - 1 ) );
                    const GUCEF::MATH::CTPoint2D< T >& a = rings[ r ][ i ];
                    const GUCEF::MATH::CTPoint2D< T >& b = rings[ r ][ ( i + 1 ) % n ];
                    const GUCEF::MATH::CTPoint2D< T >& c = rings[ s ][ j ];
                    const GUCEF::MATH::CTPoint2D< T >& d = rings[ s ][ ( j + 1 ) % m ];
                    if ( neighbours )
                    {
                        // neighbours may only share their common vertex: no fold back
                        const GUCEF::MATH::CTPoint2D< T >& shared = j == i + 1 ? b : a;
                        const GUCEF::MATH::CTPoint2D< T >& p = j == i + 1 ? a : b;
                        const GUCEF::MATH::CTPoint2D< T >& q = j == i + 1 ? d : c;
                        if ( 0 == ReferenceOrient( p, shared, q ) &&
                             ( SegmentsMeet( p, shared, q, q ) || SegmentsMeet( shared, q, p, p ) ) )
                            return false;
                        continue;
                    }
                    if ( SegmentsMeet( a, b, c, d ) )
                        return false;
                }
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  A star shaped ring on the integer grid: 'count' points at increasing angles around 'centre' with radii in
 *  [ rMin, rMax ] (counter-clockwise; may need a simplicity check after rounding)
 */
inline TGridRing
StarRing( QueryReference::CRandom& random, GUCEF::UInt32 count, G cx, G cy, GUCEF::Float64 rMin, GUCEF::Float64 rMax )
{
    TGridRing ring;
    for ( GUCEF::UInt32 i=0; i<count; ++i )
    {
        const GUCEF::Float64 jitter = random.Uniform( 0.05, 0.95 );
        const GUCEF::Float64 angle = ( (GUCEF::Float64) i + jitter ) * 6.283185307179586 / (GUCEF::Float64) count;
        const GUCEF::Float64 radius = random.Uniform( rMin, rMax );
        const G x = cx + (G) floor( radius * cos( angle ) + 0.5 );
        const G y = cy + (G) floor( radius * sin( angle ) + 0.5 );
        ring.push_back( GUCEF::MATH::CTPoint2D< G >( x, y ) );
    }
    return ring;
}

/*-------------------------------------------------------------------------*/

/** The grid ring in a kind */
template < typename T >
GUCEF::vector< GUCEF::MATH::CTPoint2D< T > >
ToKind( const TGridRing& ring, T ( *make )( G ) )
{
    GUCEF::vector< GUCEF::MATH::CTPoint2D< T > > result;
    for ( size_t i=0; i<ring.size(); ++i )
        result.push_back( GUCEF::MATH::CTPoint2D< T >( make( ring[ i ].x ), make( ring[ i ].y ) ) );
    return result;
}

/*-------------------------------------------------------------------------*/

/** A region given as Float64 rings and a fill rule: 0 even-odd, 1 non-zero, 2 positive, 3 negative */
struct SRegion
{
    GUCEF::vector< GUCEF::vector< GUCEF::MATH::CTPoint2D< GUCEF::Float64 > > > rings;
    int fill;
};

/** A crossing of a scanline: x and the winding change for one of two regions */
struct SCrossing
{
    GUCEF::Float64 x;
    int region;
    int delta;

    bool operator<( const SCrossing& other ) const { return x < other.x; }
};

inline bool
FillRule( int winding, int fill )
{
    switch ( fill )
    {
        case 0: return 0 != ( winding & 1 );
        case 1: return 0 != winding;
        case 2: return winding > 0;
        default: return winding < 0;
    }
}

/** The crossings of the line at height y with the rings of a region (half open in y): summed from the left they give the winding number */
inline void
AddCrossings( const SRegion& region, int index, GUCEF::Float64 y, GUCEF::vector< SCrossing >& crossings )
{
    for ( size_t r=0; r<region.rings.size(); ++r )
    {
        const GUCEF::vector< GUCEF::MATH::CTPoint2D< GUCEF::Float64 > >& ring = region.rings[ r ];
        const size_t n = ring.size();
        for ( size_t i=0; i<n; ++i )
        {
            const GUCEF::MATH::CTPoint2D< GUCEF::Float64 >& a = ring[ i ];
            const GUCEF::MATH::CTPoint2D< GUCEF::Float64 >& b = ring[ ( i + 1 ) % n ];
            const bool up = a.y <= y && y < b.y;
            const bool down = b.y <= y && y < a.y;
            if ( !up && !down )
                continue;
            SCrossing c;
            c.x = a.x + ( y - a.y ) * ( b.x - a.x ) / ( b.y - a.y );
            c.region = index;
            // accumulated from the left, so the sign is that of the crossings to the right of a point negated
            c.delta = up ? -1 : 1;
            crossings.push_back( c );
        }
    }
}

/**
 *  The scanline measure of { p : op( inA( p ), inB( p ) ) } together with the measure of its symmetric difference with
 *  region C: 'rows' lines at the centres of equal bands over [ yMin, yMax ], exact intervals along each line. 'op' is 0
 *  union, 1 intersection, 2 difference A - B, 3 xor, 4 A alone (B ignored).
 */
inline void
ScanlineMeasure( const SRegion& a, const SRegion& b, int op, const SRegion& c, GUCEF::Float64 yMin, GUCEF::Float64 yMax, GUCEF::UInt32 rows,
                 GUCEF::Float64& referenceArea, GUCEF::Float64& resultArea, GUCEF::Float64& mismatchArea )
{
    referenceArea = 0.0;
    resultArea = 0.0;
    mismatchArea = 0.0;
    const GUCEF::Float64 band = ( yMax - yMin ) / (GUCEF::Float64) rows;
    GUCEF::vector< SCrossing > crossings;
    for ( GUCEF::UInt32 row=0; row<rows; ++row )
    {
        const GUCEF::Float64 y = yMin + ( (GUCEF::Float64) row + 0.5 ) * band;
        crossings.clear();
        AddCrossings( a, 0, y, crossings );
        if ( 4 != op )
            AddCrossings( b, 1, y, crossings );
        AddCrossings( c, 2, y, crossings );
        std::sort( crossings.begin(), crossings.end() );
        int winding[ 3 ] = { 0, 0, 0 };
        for ( size_t i=0; i<crossings.size(); ++i )
        {
            winding[ crossings[ i ].region ] += crossings[ i ].delta;
            if ( i + 1 == crossings.size() )
                break;
            const GUCEF::Float64 length = crossings[ i + 1 ].x - crossings[ i ].x;
            if ( !( length > 0.0 ) )
                continue;
            const bool inA = FillRule( winding[ 0 ], a.fill );
            const bool inB = 4 != op && FillRule( winding[ 1 ], b.fill );
            const bool inC = FillRule( winding[ 2 ], c.fill );
            bool expected = false;
            switch ( op )
            {
                case 0: expected = inA || inB; break;
                case 1: expected = inA && inB; break;
                case 2: expected = inA && !inB; break;
                case 3: expected = inA != inB; break;
                default: expected = inA; break;
            }
            if ( expected )
                referenceArea += length * band;
            if ( inC )
                resultArea += length * band;
            if ( expected != inC )
                mismatchArea += length * band;
        }
    }
}

} /* namespace PolygonReference */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_TESTAPP_TESTPOLYGONREFERENCE_H ? */
