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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GEOMETRY_POINTINPOLYGON_H
#include "gucefGEOMETRY_PointInPolygon.h"
#define GUCEF_GEOMETRY_POINTINPOLYGON_H
#endif /* GUCEF_GEOMETRY_POINTINPOLYGON_H ? */


#include "TestSpatialReference.h"
#include "TestConvexReference.h"
#include "TestPointInPolygon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::MATH;
using namespace GUCEF::GEOMETRY;
using namespace QueryReference;
using namespace SpatialReference;
using ConvexReference::ScaledCount;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef TMathBigFraction F;

static Int32 MakeInt32( Int64 g ) { return (Int32) g; }
static Int64 MakeInt64( Int64 g ) { return g * 1000003; }
static TFixed32_32 MakeFixed32_32( Int64 g ) { return TFixed32_32::FromRaw( g * 16777216 ); }
static TFixed16_16 MakeFixed16_16( Int64 g ) { return TFixed16_16::FromRaw( (Int32) ( g * 16 ) ); }
static Float32 MakeFloat32( Int64 g ) { return (Float32) g / 16.0f; }
static Float64 MakeFloat64( Int64 g ) { return ldexp( (Float64) g, -20 ); }

/*-------------------------------------------------------------------------*/

struct SExactPoint
{
    F x;
    F y;
};

template < typename T >
static SExactPoint
ToExact( const CTPoint2D< T >& p )
{
    SExactPoint e;
    e.x = ExactFraction( p.x );
    e.y = ExactFraction( p.y );
    return e;
}

/** The reference: boundary by an exact cross product and the box of the edge, otherwise the signed crossings of the ray towards +x at exact intersection positions */
static TPointInPolygon
ReferenceRing( const GUCEF::vector< SExactPoint >& ring, const SExactPoint& p )
{
    const F zero( 0, 1 );
    Int32 winding = 0;
    for ( size_t i=0; i<ring.size(); ++i )
    {
        const SExactPoint& a = ring[ i ];
        const SExactPoint& b = ring[ ( i + 1 ) % ring.size() ];
        const F cross = ( b.x - a.x ) * ( p.y - a.y ) - ( b.y - a.y ) * ( p.x - a.x );
        const bool inX = ( a.x < b.x ? a.x : b.x ) <= p.x && p.x <= ( a.x < b.x ? b.x : a.x );
        const bool inY = ( a.y < b.y ? a.y : b.y ) <= p.y && p.y <= ( a.y < b.y ? b.y : a.y );
        if ( cross == zero && inX && inY )
            return POINTINPOLYGON_ON_BOUNDARY;
        if ( ( a.y <= p.y ) != ( b.y <= p.y ) )
        {
            const F crossX = a.x + ( p.y - a.y ) * ( b.x - a.x ) / ( b.y - a.y );
            if ( crossX > p.x )
                winding += b.y > a.y ? 1 : -1;
        }
    }
    return 0 != winding ? POINTINPOLYGON_INSIDE : POINTINPOLYGON_OUTSIDE;
}

/*-------------------------------------------------------------------------*/

static Int64
Gcd( Int64 a, Int64 b )
{
    a = a < 0 ? -a : a;
    b = b < 0 ? -b : b;
    while ( 0 != b )
    {
        const Int64 t = a % b;
        a = b;
        b = t;
    }
    return a;
}

/** A star shaped ring around the origin on the integer lattice: radii between low and high, angles sorted (the ring may still touch itself where rounding makes edges meet) */
static void
RandomRing( CRandom& random, UInt32 count, Float64 low, Float64 high, GUCEF::vector< Int64 >& xs, GUCEF::vector< Int64 >& ys )
{
    GUCEF::vector< Float64 > angles;
    for ( UInt32 i=0; i<count; ++i )
        angles.push_back( random.Uniform( 0.0, 6.2831853 ) );
    std::sort( angles.begin(), angles.end() );
    xs.clear();
    ys.clear();
    for ( UInt32 i=0; i<count; ++i )
    {
        const Float64 r = random.Uniform( low, high );
        xs.push_back( (Int64) floor( r * cos( angles[ i ] ) + 0.5 ) );
        ys.push_back( (Int64) floor( r * sin( angles[ i ] ) + 0.5 ) );
    }
}

/*-------------------------------------------------------------------------*/

/** One unit of the last place up or down; the integer and fixed-point kinds have no such neighbour in the lattice and stay */
template < typename T > static void Nudge( T& /* value */, bool /* up */ ) {}
static void Nudge( Float64& value, bool up ) { value = nextafter( value, up ? 1.0e300 : -1.0e300 ); }
static void Nudge( Float32& value, bool up ) { value = nextafterf( value, up ? 1.0e30f : -1.0e30f ); }

template < typename T >
static bool
PointInPolygonSweep( const char* name, UInt64 seed, UInt32 polygons, UInt32 pointsPerPolygon, T ( *make )( Int64 ), bool perturb )
{
    CRandom random( seed );
    UInt32 counts[ 3 ] = { 0, 0, 0 };
    for ( UInt32 n=0; n<polygons; ++n )
    {
        GUCEF::vector< Int64 > xs, ys, hxs, hys;
        RandomRing( random, 3 + random.Index( 10 ), 100.0, 1000.0, xs, ys );
        const bool withHole = random.Chance( 0.4 );
        if ( withHole )
            RandomRing( random, 3 + random.Index( 6 ), 20.0, 90.0, hxs, hys );

        CTPolygonWithHoles2D< T > polygon;
        GUCEF::vector< SExactPoint > outerExact, holeExact;
        for ( size_t i=0; i<xs.size(); ++i )
        {
            polygon.outer.points.push_back( CTPoint2D< T >( make( xs[ i ] ), make( ys[ i ] ) ) );
            outerExact.push_back( ToExact( polygon.outer.points.back() ) );
        }
        if ( withHole )
        {
            CTPolygon2D< T > hole;
            for ( size_t i=0; i<hxs.size(); ++i )
            {
                hole.points.push_back( CTPoint2D< T >( make( hxs[ i ] ), make( hys[ i ] ) ) );
                holeExact.push_back( ToExact( hole.points.back() ) );
            }
            polygon.AddHole( hole );
        }

        for ( UInt32 q=0; q<pointsPerPolygon; ++q )
        {
            Int64 px, py;
            const UInt32 kind = (UInt32) random.Index( 5 );
            if ( 0 == kind )
            {
                px = random.Index( 2201 ) - 1100;
                py = random.Index( 2201 ) - 1100;
            }
            else if ( 1 == kind )
            {
                const size_t v = (size_t) random.Index( (Int32) xs.size() );       // a vertex
                px = xs[ v ];
                py = ys[ v ];
            }
            else if ( 2 == kind || 3 == kind )
            {
                // a lattice point on an edge of the outer ring, or one lattice step off it
                const size_t v = (size_t) random.Index( (Int32) xs.size() );
                const size_t w = ( v + 1 ) % xs.size();
                const Int64 dx = xs[ w ] - xs[ v ];
                const Int64 dy = ys[ w ] - ys[ v ];
                const Int64 g = Gcd( dx, dy );
                const Int64 m = g > 0 ? random.Index( (Int32) g + 1 ) : 0;
                px = g > 0 ? xs[ v ] + dx / g * m : xs[ v ];
                py = g > 0 ? ys[ v ] + dy / g * m : ys[ v ];
                if ( 3 == kind )
                {
                    px += random.Index( 3 ) - 1;
                    py += random.Index( 3 ) - 1;
                }
            }
            else if ( withHole )
            {
                const size_t v = (size_t) random.Index( (Int32) hxs.size() );
                const size_t w = ( v + 1 ) % hxs.size();
                const Int64 dx = hxs[ w ] - hxs[ v ];
                const Int64 dy = hys[ w ] - hys[ v ];
                const Int64 g = Gcd( dx, dy );
                const Int64 m = g > 0 ? random.Index( (Int32) g + 1 ) : 0;
                px = g > 0 ? hxs[ v ] + dx / g * m : hxs[ v ];
                py = g > 0 ? hys[ v ] + dy / g * m : hys[ v ];
            }
            else
            {
                px = random.Index( 2001 ) - 1000;
                py = random.Index( 2001 ) - 1000;
            }
            CTPoint2D< T > point( make( px ), make( py ) );
            if ( perturb && random.Chance( 0.3 ) )
            {
                // one unit of the last place off the lattice point
                Nudge( point.x, random.Chance( 0.5 ) );
                Nudge( point.y, random.Chance( 0.5 ) );
            }
            const SExactPoint exact = ToExact( point );
            TPointInPolygon expected = ReferenceRing( outerExact, exact );
            if ( POINTINPOLYGON_INSIDE == expected && withHole )
            {
                const TPointInPolygon inHole = ReferenceRing( holeExact, exact );
                if ( POINTINPOLYGON_ON_BOUNDARY == inHole )
                    expected = POINTINPOLYGON_ON_BOUNDARY;
                else if ( POINTINPOLYGON_INSIDE == inHole )
                    expected = POINTINPOLYGON_OUTSIDE;
            }
            const TPointInPolygon actual = ClassifyPointInPolygon( polygon, point );
            ++counts[ (UInt32) expected ];
            if ( actual != expected )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": point ( " << px << ", " << py << " ) is " << (Int32) actual << " instead of " << (Int32) expected << ", seed " << seed << " polygon " << n << " point " << q );
                return false;
            }
            // the simple ring form agrees with the form with holes when there are none
            if ( !withHole && ClassifyPointInPolygon( polygon.outer, point ) != expected )
                return false;
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PointInPolygon: " << name << ": " << counts[ 0 ] << " outside, " << counts[ 1 ] << " inside and " << counts[ 2 ] << " on the boundary agree with the exact reference" );
    return counts[ 2 ] > 0 && counts[ 1 ] > 0 && counts[ 0 ] > 0;
}

/*-------------------------------------------------------------------------*/

/** Float64 points one unit of the last place around an edge that is not on the lattice: the exact predicate decides */
static bool
NearEdgeFloat64( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    for ( UInt32 n=0; n<count; ++n )
    {
        const CTPoint2D< Float64 > a( random.Uniform( -1.0, 1.0 ), random.Uniform( -1.0, 1.0 ) );
        const CTPoint2D< Float64 > b( random.Uniform( -1.0, 1.0 ), random.Uniform( -1.0, 1.0 ) );
        const CTPoint2D< Float64 > c( random.Uniform( -1.0, 1.0 ), random.Uniform( -1.0, 1.0 ) );
        CTPolygon2D< Float64 > triangle;
        triangle.points.push_back( a );
        triangle.points.push_back( b );
        triangle.points.push_back( c );
        const Float64 t = random.Uniform( 0.0, 1.0 );
        CTPoint2D< Float64 > p( a.x + ( b.x - a.x ) * t, a.y + ( b.y - a.y ) * t );
        for ( UInt32 k=0; k<4; ++k )
        {
            GUCEF::vector< SExactPoint > ring;
            ring.push_back( ToExact( a ) );
            ring.push_back( ToExact( b ) );
            ring.push_back( ToExact( c ) );
            if ( ClassifyPointInPolygon( triangle, p ) != ReferenceRing( ring, ToExact( p ) ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PointInPolygon: near the edge the result differs from the exact reference, seed " << seed << " case " << n );
                return false;
            }
            p.y = nextafter( p.y, k % 2 ? 1.0e300 : -1.0e300 );
            if ( 1 == k )
                p.x = nextafter( p.x, 1.0e300 );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Hand made cases: a concave C, a square with a hole, a degenerate ring, the vertices and edges, points that are not finite */
static bool
HandCases( void )
{
    typedef CTPoint2D< Int32 > P;
    // a C shape: the notch is outside, its edges and corners are on the boundary
    CTPolygon2D< Int32 > c;
    const Int32 cx[ 8 ] = { 0, 10, 10, 4, 4, 10, 10, 0 };
    const Int32 cy[ 8 ] = { 0, 0, 3, 3, 7, 7, 10, 10 };
    for ( UInt32 i=0; i<8; ++i )
        c.points.push_back( P( cx[ i ], cy[ i ] ) );
    if ( POINTINPOLYGON_INSIDE != ClassifyPointInPolygon( c, P( 2, 5 ) ) ) return false;
    if ( POINTINPOLYGON_INSIDE != ClassifyPointInPolygon( c, P( 8, 1 ) ) ) return false;
    if ( POINTINPOLYGON_OUTSIDE != ClassifyPointInPolygon( c, P( 8, 5 ) ) ) return false;
    if ( POINTINPOLYGON_ON_BOUNDARY != ClassifyPointInPolygon( c, P( 4, 5 ) ) ) return false;
    if ( POINTINPOLYGON_ON_BOUNDARY != ClassifyPointInPolygon( c, P( 4, 3 ) ) ) return false;
    if ( POINTINPOLYGON_ON_BOUNDARY != ClassifyPointInPolygon( c, P( 7, 3 ) ) ) return false;
    if ( POINTINPOLYGON_ON_BOUNDARY != ClassifyPointInPolygon( c, P( 0, 0 ) ) ) return false;
    if ( POINTINPOLYGON_OUTSIDE != ClassifyPointInPolygon( c, P( -1, 5 ) ) ) return false;
    if ( POINTINPOLYGON_OUTSIDE != ClassifyPointInPolygon( c, P( 11, 5 ) ) ) return false;
    // ray through vertices of the ring: a point level with the notch corners and left of the ring
    if ( POINTINPOLYGON_OUTSIDE != ClassifyPointInPolygon( c, P( -5, 3 ) ) ) return false;
    if ( POINTINPOLYGON_OUTSIDE != ClassifyPointInPolygon( c, P( -5, 7 ) ) ) return false;
    // the other orientation gives the same classification
    CTPolygon2D< Int32 > reversed;
    for ( UInt32 i=0; i<8; ++i )
        reversed.points.push_back( c.points[ 7 - i ] );
    if ( POINTINPOLYGON_INSIDE != ClassifyPointInPolygon( reversed, P( 2, 5 ) ) || POINTINPOLYGON_OUTSIDE != ClassifyPointInPolygon( reversed, P( 8, 5 ) ) ) return false;

    // a square with a square hole
    CTPolygonWithHoles2D< Int32 > frame;
    const Int32 sx[ 4 ] = { 0, 10, 10, 0 };
    const Int32 sy[ 4 ] = { 0, 0, 10, 10 };
    CTPolygon2D< Int32 > hole;
    for ( UInt32 i=0; i<4; ++i )
    {
        frame.outer.points.push_back( P( sx[ i ], sy[ i ] ) );
        hole.points.push_back( P( 3 + sx[ i ] * 4 / 10, 3 + sy[ i ] * 4 / 10 ) );
    }
    frame.AddHole( hole );
    if ( POINTINPOLYGON_INSIDE != ClassifyPointInPolygon( frame, P( 1, 1 ) ) ) return false;
    if ( POINTINPOLYGON_OUTSIDE != ClassifyPointInPolygon( frame, P( 5, 5 ) ) ) return false;
    if ( POINTINPOLYGON_ON_BOUNDARY != ClassifyPointInPolygon( frame, P( 3, 5 ) ) ) return false;
    if ( POINTINPOLYGON_ON_BOUNDARY != ClassifyPointInPolygon( frame, P( 7, 7 ) ) ) return false;
    if ( POINTINPOLYGON_ON_BOUNDARY != ClassifyPointInPolygon( frame, P( 10, 4 ) ) ) return false;
    if ( POINTINPOLYGON_OUTSIDE != ClassifyPointInPolygon( frame, P( 11, 4 ) ) ) return false;
    if ( !IsPointInPolygon( frame, P( 3, 3 ) ) || IsPointInPolygon( frame, P( 5, 5 ) ) ) return false;

    // degenerate rings
    CTPolygon2D< Int32 > none, one, two;
    one.points.push_back( P( 1, 1 ) );
    two.points.push_back( P( 0, 0 ) );
    two.points.push_back( P( 4, 4 ) );
    if ( POINTINPOLYGON_OUTSIDE != ClassifyPointInPolygon( none, P( 0, 0 ) ) ) return false;
    if ( POINTINPOLYGON_ON_BOUNDARY != ClassifyPointInPolygon( one, P( 1, 1 ) ) || POINTINPOLYGON_OUTSIDE != ClassifyPointInPolygon( one, P( 1, 2 ) ) ) return false;
    if ( POINTINPOLYGON_ON_BOUNDARY != ClassifyPointInPolygon( two, P( 2, 2 ) ) || POINTINPOLYGON_OUTSIDE != ClassifyPointInPolygon( two, P( 2, 3 ) ) || POINTINPOLYGON_OUTSIDE != ClassifyPointInPolygon( two, P( 5, 5 ) ) ) return false;

    // points that are not finite
    CTPolygon2D< Float64 > square;
    square.points.push_back( CTPoint2D< Float64 >( 0, 0 ) );
    square.points.push_back( CTPoint2D< Float64 >( 1, 0 ) );
    square.points.push_back( CTPoint2D< Float64 >( 1, 1 ) );
    square.points.push_back( CTPoint2D< Float64 >( 0, 1 ) );
    const Float64 zero = 0.0;
    const Float64 nan = zero / zero;
    if ( POINTINPOLYGON_OUTSIDE != ClassifyPointInPolygon( square, CTPoint2D< Float64 >( nan, 0.5 ) ) ) return false;
    if ( POINTINPOLYGON_OUTSIDE != ClassifyPointInPolygon( square, CTPoint2D< Float64 >( 0.5, 1.0 / zero ) ) ) return false;
    // the closed ring and the tiny offsets around the boundary
    if ( POINTINPOLYGON_ON_BOUNDARY != ClassifyPointInPolygon( square, CTPoint2D< Float64 >( 1.0, 0.5 ) ) ) return false;
    if ( POINTINPOLYGON_OUTSIDE != ClassifyPointInPolygon( square, CTPoint2D< Float64 >( nextafter( 1.0, 2.0 ), 0.5 ) ) ) return false;
    if ( POINTINPOLYGON_INSIDE != ClassifyPointInPolygon( square, CTPoint2D< Float64 >( nextafter( 1.0, 0.0 ), 0.5 ) ) ) return false;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPointInPolygonTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "PointInPolygon" );

    GUCEF_TESTFW_TESTCASE( "PointInPolygon: concave polygons, holes, vertices and edges give inside, outside and on boundary as stated" )
    try
    {
        ASSERT_TRUE( HandCases() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PointInPolygon: integer, fixed-point and float kinds equal the exact fraction reference on random star shaped rings with holes" )
    try
    {
        ASSERT_TRUE( ( PointInPolygonSweep< Int32 >( "Int32", 5101, ScaledCount( 300 ), 40, MakeInt32, false ) ) );
        ASSERT_TRUE( ( PointInPolygonSweep< Int64 >( "Int64", 5102, ScaledCount( 300 ), 40, MakeInt64, false ) ) );
        ASSERT_TRUE( ( PointInPolygonSweep< TFixed32_32 >( "TFixed32_32", 5103, ScaledCount( 300 ), 40, MakeFixed32_32, false ) ) );
        ASSERT_TRUE( ( PointInPolygonSweep< TFixed16_16 >( "TFixed16_16", 5104, ScaledCount( 300 ), 40, MakeFixed16_16, false ) ) );
        ASSERT_TRUE( ( PointInPolygonSweep< Float32 >( "Float32", 5105, ScaledCount( 300 ), 40, MakeFloat32, true ) ) );
        ASSERT_TRUE( ( PointInPolygonSweep< Float64 >( "Float64", 5106, ScaledCount( 300 ), 40, MakeFloat64, true ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PointInPolygon: Float64 points one unit of the last place around an edge are classified exactly" )
    try
    {
        ASSERT_TRUE( NearEdgeFloat64( 5107, ScaledCount( 4000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
