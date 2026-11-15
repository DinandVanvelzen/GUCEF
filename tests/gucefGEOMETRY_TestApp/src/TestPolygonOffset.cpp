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
#include <limits>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYGONOFFSETTER_H
#include "gucefGEOMETRY_CTPolygonOffsetter.h"
#define GUCEF_GEOMETRY_CTPOLYGONOFFSETTER_H
#endif /* GUCEF_GEOMETRY_CTPOLYGONOFFSETTER_H ? */

#include "TestConvexReference.h"
#include "TestPolygonReference.h"
#include "TestPolygonOffset.h"

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
using namespace PolygonReference;
using ConvexReference::ScaledCount;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const Float64 Pi = 3.14159265358979323846;

typedef CTPoint2D< Float64 > P;
typedef GUCEF::vector< P > TPath64;
typedef GUCEF::vector< TPath64 > TPaths64;

/*-------------------------------------------------------------------------*/

/** The area of result rings (holes are clockwise) in Float64 */
template < typename T >
static Float64
Area( const GUCEF::vector< GUCEF::vector< CTPoint2D< T > > >& rings )
{
    typedef MATH::CTNumericTraits< T > TTraits;
    Float64 sum = 0.0;
    for ( size_t r=0; r<rings.size(); ++r )
    {
        const GUCEF::vector< CTPoint2D< T > >& ring = rings[ r ];
        for ( size_t i=1; i+1<ring.size(); ++i )
        {
            const Float64 ax = TTraits::ToFloat64( ring[ i ].x ) - TTraits::ToFloat64( ring[ 0 ].x ), ay = TTraits::ToFloat64( ring[ i ].y ) - TTraits::ToFloat64( ring[ 0 ].y );
            const Float64 bx = TTraits::ToFloat64( ring[ i + 1 ].x ) - TTraits::ToFloat64( ring[ 0 ].x ), by = TTraits::ToFloat64( ring[ i + 1 ].y ) - TTraits::ToFloat64( ring[ 0 ].y );
            sum += 0.5 * ( ax * by - ay * bx );
        }
    }
    return sum;
}

/*-------------------------------------------------------------------------*/

template < typename T >
static GUCEF::vector< CTPoint2D< T > >
Square( Float64 side, T ( *convert )( Float64 ) )
{
    GUCEF::vector< CTPoint2D< T > > ring;
    ring.push_back( CTPoint2D< T >( convert( 0.0 ), convert( 0.0 ) ) );
    ring.push_back( CTPoint2D< T >( convert( side ), convert( 0.0 ) ) );
    ring.push_back( CTPoint2D< T >( convert( side ), convert( side ) ) );
    ring.push_back( CTPoint2D< T >( convert( 0.0 ), convert( side ) ) );
    return ring;
}

static Float64 ToF64( Float64 v ) { return v; }
static Float32 ToF32( Float64 v ) { return (Float32) v; }
static Int32 ToI32( Float64 v ) { return (Int32) floor( v + 0.5 ); }
static Int64 ToI64( Float64 v ) { return (Int64) floor( v + 0.5 ); }
static TFixed32_32 ToFixed( Float64 v ) { TFixed32_32 r; MATH::CTNumericTraits< TFixed32_32 >::FromFloat64( v, CORE::ROUNDINGMODE_HALF_EVEN, r ); return r; }

/*-------------------------------------------------------------------------*/

/** Checks an area: |area - expected| <= tolerance */
static bool
AreaWithin( const char* name, const char* context, Float64 area, Float64 expected, Float64 tolerance )
{
    if ( fabs( area - expected ) <= tolerance )
        return true;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": area " << area << " expected " << expected << " within " << tolerance );
    return false;
}

/*-------------------------------------------------------------------------*/

/**
 *  The closed form areas of a square of side s offset by d: round joins s^2 + 4 s d + pi d^2 within the arc tolerance
 *  (the arcs are inscribed: at most the arc length times the tolerance less), miter (s + 2 d)^2, bevel at a miter limit
 *  below sqrt( 2 ) s^2 + 4 s d + 2 d^2, square joins s^2 + 4 s d + ( 8 sqrt( 2 ) - 8 ) d^2; shrinking gives ( s - 2 d )^2
 *  and nothing at d >= s / 2. 'grid' is the area error the grid of the kind allows.
 */
template < typename T >
static bool
SquareAreas( const char* name, Float64 s, Float64 d, T ( *convert )( Float64 ), Float64 grid )
{
    CTPolygonOffsetter< T > offsetter;
    GUCEF::vector< GUCEF::vector< CTPoint2D< T > > > rings;
    bool ok = true;
    offsetter.AddPolygon( Square( s, convert ) );
    const Float64 tolerance = d / 400.0;
    ok = POLYGONCLIPPER_OK == offsetter.Execute( convert( d ), POLYGONJOIN_ROUND, POLYLINEEND_BUTT, rings ) && 1 == rings.size() && ok;
    const Float64 round = Area( rings );
    const Float64 exact = s * s + 4.0 * s * d + Pi * d * d;
    ok = AreaWithin( name, "round joins", round, exact - Pi * d * tolerance, Pi * d * tolerance + grid ) && ok;
    offsetter.SetArcTolerance( d / 10000.0 );
    ok = POLYGONCLIPPER_OK == offsetter.Execute( convert( d ), POLYGONJOIN_ROUND, POLYLINEEND_BUTT, rings ) && ok;
    ok = AreaWithin( name, "round joins, fine tolerance", Area( rings ), exact - Pi * d * d / 10000.0, Pi * d * d / 10000.0 + grid ) && ok;
    offsetter.SetArcTolerance( 0.0 );
    ok = POLYGONCLIPPER_OK == offsetter.Execute( convert( d ), POLYGONJOIN_MITER, POLYLINEEND_BUTT, rings ) && 1 == rings.size() && 4 == rings[ 0 ].size() && ok;
    ok = AreaWithin( name, "miter joins", Area( rings ), ( s + 2.0 * d ) * ( s + 2.0 * d ), grid ) && ok;
    offsetter.SetMiterLimit( 1.2 );
    ok = POLYGONCLIPPER_OK == offsetter.Execute( convert( d ), POLYGONJOIN_MITER, POLYLINEEND_BUTT, rings ) && 1 == rings.size() && 8 == rings[ 0 ].size() && ok;
    ok = AreaWithin( name, "miter limit (bevel)", Area( rings ), s * s + 4.0 * s * d + 2.0 * d * d, grid ) && ok;
    offsetter.SetMiterLimit( 2.0 );
    ok = POLYGONCLIPPER_OK == offsetter.Execute( convert( d ), POLYGONJOIN_SQUARE, POLYLINEEND_BUTT, rings ) && 1 == rings.size() && 8 == rings[ 0 ].size() && ok;
    ok = AreaWithin( name, "square joins", Area( rings ), s * s + 4.0 * s * d + ( 8.0 * sqrt( 2.0 ) - 8.0 ) * d * d, grid ) && ok;
    for ( UInt32 join=0; join<3; ++join )
    {
        ok = POLYGONCLIPPER_OK == offsetter.Execute( convert( -d ), (TPolygonJoinType) join, POLYLINEEND_BUTT, rings ) && ok;
        ok = AreaWithin( name, "shrinking", Area( rings ), ( s - 2.0 * d ) * ( s - 2.0 * d ), grid ) && ok;
        ok = POLYGONCLIPPER_OK == offsetter.Execute( convert( -s / 2.0 ), (TPolygonJoinType) join, POLYLINEEND_BUTT, rings ) && rings.empty() && ok;
        ok = POLYGONCLIPPER_OK == offsetter.Execute( convert( -s ), (TPolygonJoinType) join, POLYLINEEND_BUTT, rings ) && rings.empty() && ok;
    }
    // zero: the polygon itself
    ok = POLYGONCLIPPER_OK == offsetter.Execute( convert( 0.0 ), POLYGONJOIN_ROUND, POLYLINEEND_BUTT, rings ) && ok;
    ok = AreaWithin( name, "zero offset", Area( rings ), s * s, grid ) && ok;
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the square offsets failed" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** A square of side s with a centred square hole of side h, grown by d with miter joins: ( s + 2 d )^2 - ( h - 2 d )^2, the hole gone at d >= h / 2 */
static bool
HoleAreas( void )
{
    CTPolygonOffsetter< Float64 > offsetter;
    CTPolygonWithHoles2D< Float64 > polygon;
    polygon.outer.points = Square( 100.0, ToF64 );
    GUCEF::vector< P > hole = Square( 40.0, ToF64 );
    for ( size_t i=0; i<hole.size(); ++i )
        hole[ i ] = hole[ i ] + CTVector2D< Float64 >( 30.0, 30.0 );
    polygon.AddHole( CTPolygon2D< Float64 >( &hole[ 0 ], 4 ) );        // counter-clockwise on input: made clockwise
    offsetter.AddPolygon( polygon );
    bool ok = true;
    GUCEF::vector< CTPolygonWithHoles2D< Float64 > > polygons;
    ok = POLYGONCLIPPER_OK == offsetter.Execute( 5.0, POLYGONJOIN_MITER, POLYLINEEND_BUTT, polygons ) && 1 == polygons.size() && 1 == polygons[ 0 ].holes.size() && ok;
    TPaths64 rings;
    ok = POLYGONCLIPPER_OK == offsetter.Execute( 5.0, POLYGONJOIN_MITER, POLYLINEEND_BUTT, rings ) && ok;
    ok = AreaWithin( "Float64", "grown with a hole", Area( rings ), 110.0 * 110.0 - 30.0 * 30.0, 1e-6 ) && ok;
    ok = POLYGONCLIPPER_OK == offsetter.Execute( 25.0, POLYGONJOIN_MITER, POLYLINEEND_BUTT, polygons ) && 1 == polygons.size() && polygons[ 0 ].holes.empty() && ok;
    ok = POLYGONCLIPPER_OK == offsetter.Execute( -5.0, POLYGONJOIN_MITER, POLYLINEEND_BUTT, rings ) && ok;
    ok = AreaWithin( "Float64", "shrunk with a hole", Area( rings ), 90.0 * 90.0 - 50.0 * 50.0, 1e-6 ) && ok;
    // the free function
    ok = POLYGONCLIPPER_OK == OffsetPolygon( polygon, 5.0, POLYGONJOIN_MITER, polygons ) && 1 == polygons.size() && ok;
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonOffset: the polygon with a hole failed" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Strokes: a straight polyline of length L at half width d: butt 2 d L, square 2 d ( L + 2 d ), round 2 d L + pi d^2; single points */
template < typename T >
static bool
StrokeAreas( const char* name, Float64 L, Float64 d, T ( *convert )( Float64 ), Float64 grid )
{
    CTPolygonOffsetter< T > offsetter;
    GUCEF::vector< CTPoint2D< T > > line;
    // collinear interior points must not matter
    line.push_back( CTPoint2D< T >( convert( 0.0 ), convert( 0.0 ) ) );
    line.push_back( CTPoint2D< T >( convert( L / 3.0 ), convert( 0.0 ) ) );
    line.push_back( CTPoint2D< T >( convert( L / 3.0 ), convert( 0.0 ) ) );
    line.push_back( CTPoint2D< T >( convert( L ), convert( 0.0 ) ) );
    offsetter.AddPolyline( line );
    GUCEF::vector< GUCEF::vector< CTPoint2D< T > > > rings;
    bool ok = true;
    const Float64 tolerance = d / 400.0;
    ok = POLYGONCLIPPER_OK == offsetter.Execute( convert( d ), POLYGONJOIN_ROUND, POLYLINEEND_BUTT, rings ) && ok;
    ok = AreaWithin( name, "butt caps", Area( rings ), 2.0 * d * L, grid ) && ok;
    ok = POLYGONCLIPPER_OK == offsetter.Execute( convert( d ), POLYGONJOIN_ROUND, POLYLINEEND_SQUARE, rings ) && ok;
    ok = AreaWithin( name, "square caps", Area( rings ), 2.0 * d * ( L + 2.0 * d ), grid ) && ok;
    ok = POLYGONCLIPPER_OK == offsetter.Execute( convert( d ), POLYGONJOIN_ROUND, POLYLINEEND_ROUND, rings ) && ok;
    ok = AreaWithin( name, "round caps", Area( rings ), 2.0 * d * L + Pi * d * d - Pi * d * tolerance, Pi * d * tolerance + grid ) && ok;
    ok = POLYGONCLIPPER_OK == offsetter.Execute( convert( -d ), POLYGONJOIN_ROUND, POLYLINEEND_ROUND, rings ) && ok;
    ok = AreaWithin( name, "round caps, negative half width", Area( rings ), 2.0 * d * L + Pi * d * d - Pi * d * tolerance, Pi * d * tolerance + grid ) && ok;
    ok = POLYGONCLIPPER_OK == offsetter.Execute( convert( 0.0 ), POLYGONJOIN_ROUND, POLYLINEEND_ROUND, rings ) && rings.empty() && ok;
    offsetter.Clear();
    const CTPoint2D< T > point( convert( 5.0 ), convert( 5.0 ) );
    offsetter.AddPolyline( &point, 1 );
    ok = POLYGONCLIPPER_OK == offsetter.Execute( convert( d ), POLYGONJOIN_ROUND, POLYLINEEND_ROUND, rings ) && 1 == rings.size() && ok;
    ok = AreaWithin( name, "a point, round", Area( rings ), Pi * d * d - Pi * d * tolerance, Pi * d * tolerance + grid ) && ok;
    ok = POLYGONCLIPPER_OK == offsetter.Execute( convert( d ), POLYGONJOIN_ROUND, POLYLINEEND_SQUARE, rings ) && ok;
    ok = AreaWithin( name, "a point, square", Area( rings ), 4.0 * d * d, grid ) && ok;
    ok = POLYGONCLIPPER_OK == offsetter.Execute( convert( d ), POLYGONJOIN_ROUND, POLYLINEEND_BUTT, rings ) && rings.empty() && ok;
    // the free function on a CTPolyline2D
    CTPolyline2D< T > polyline( &line[ 0 ], (UInt32) line.size() );
    GUCEF::vector< CTPolygonWithHoles2D< T > > polygons;
    ok = POLYGONCLIPPER_OK == StrokePolyline( polyline, convert( d ), POLYGONJOIN_ROUND, POLYLINEEND_BUTT, polygons ) && 1 == polygons.size() && ok;
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the strokes failed" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** The squared distance from p to the segment a-b */
static Float64
DistanceSquared( const P& p, const P& a, const P& b )
{
    const Float64 dx = b.x - a.x, dy = b.y - a.y;
    const Float64 length = dx * dx + dy * dy;
    Float64 t = length > 0.0 ? ( ( p.x - a.x ) * dx + ( p.y - a.y ) * dy ) / length : 0.0;
    t = t < 0.0 ? 0.0 : ( t > 1.0 ? 1.0 : t );
    const Float64 ex = a.x + t * dx - p.x, ey = a.y + t * dy - p.y;
    return ex * ex + ey * ey;
}

/** The winding number of p in the rings */
static Int32
Winding( const TPaths64& rings, const P& p )
{
    Int32 winding = 0;
    for ( size_t r=0; r<rings.size(); ++r )
        for ( size_t i=0; i<rings[ r ].size(); ++i )
        {
            const P& a = rings[ r ][ i ];
            const P& b = rings[ r ][ ( i + 1 ) % rings[ r ].size() ];
            const Float64 cross = ( b.x - a.x ) * ( p.y - a.y ) - ( b.y - a.y ) * ( p.x - a.x );
            if ( a.y <= p.y ) { if ( b.y > p.y && cross > 0.0 ) ++winding; }
            else if ( b.y <= p.y && cross < 0.0 ) --winding;
        }
    return winding;
}

/*-------------------------------------------------------------------------*/

/**
 *  Seeded star polygons offset with round joins (grown and shrunk) and seeded polylines stroked with round joins and
 *  caps, against the distance reference at sample points: a point farther than the arc tolerance plus a margin from the
 *  reference boundary must be classified as the reference says (inside the grown region: inside the polygon or within d
 *  of its boundary; inside the shrunk region: inside and farther than |d| from the boundary; inside a stroke: within d of
 *  the polyline).
 */
static bool
SampledReference( UInt64 seed, UInt32 sets, UInt32 samples )
{
    QueryReference::CRandom random( seed );
    CTPolygonOffsetter< Float64 > offsetter;
    for ( UInt32 set=0; set<sets; ++set )
    {
        const Int32 n = 3 + random.Index( 30 );
        const TGridRing grid = StarRing( random, (UInt32) n, 0, 0, 300.0, 1000.0 );
        TPath64 ring;
        for ( size_t i=0; i<grid.size(); ++i )
            ring.push_back( P( (Float64) grid[ i ].x, (Float64) grid[ i ].y ) );
        const Float64 dDraw = random.Uniform( 5.0, 200.0 );
        const bool polyline = random.Chance( 0.4 );
        const Float64 d = polyline ? dDraw : ( random.Chance( 0.5 ) ? dDraw : -dDraw );
        offsetter.Clear();
        if ( polyline )
            offsetter.AddPolyline( ring );
        else
            offsetter.AddPolygon( ring );
        TPaths64 rings;
        if ( POLYGONCLIPPER_OK != offsetter.Execute( d, POLYGONJOIN_ROUND, POLYLINEEND_ROUND, rings ) )
            return false;
        const Float64 margin = fabs( d ) / 400.0 + 1e-6 * 1000.0;
        TPaths64 original( 1, ring );
        for ( UInt32 k=0; k<samples; ++k )
        {
            const Float64 sx = random.Uniform( -1300.0, 1300.0 );
            const Float64 sy = random.Uniform( -1300.0, 1300.0 );
            const P p( sx, sy );
            Float64 nearest = 1e300;
            const UInt32 edges = polyline ? (UInt32) ring.size() - 1 : (UInt32) ring.size();
            for ( UInt32 i=0; i<edges; ++i )
                nearest = std::min( nearest, DistanceSquared( p, ring[ i ], ring[ ( i + 1 ) % ring.size() ] ) );
            nearest = sqrt( nearest );
            const bool inside = !polyline && 0 != Winding( original, p );
            Float64 signedDistance = 0.0;     // positive inside the reference region
            if ( polyline )
                signedDistance = d - nearest;
            else
            if ( d > 0.0 )
                signedDistance = inside ? d + nearest : d - nearest;
            else
                signedDistance = inside ? nearest + d : -( nearest - d );
            if ( fabs( signedDistance ) <= margin )
                continue;
            const bool expected = signedDistance > 0.0;
            const bool actual = 0 != Winding( rings, p );
            if ( expected != actual )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonOffset: seed " << seed << " set " << set << ( polyline ? " stroke" : " offset" ) << " d " << d << ": the point (" << sx << ", " << sy << ") at signed distance " << signedDistance << " is classified wrongly" );
                return false;
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  The pinned FNV-1a hashes of the bits of the round-join and round-cap output (RoundJoinHash) for Float32 and Float64:
 *  identical in every build (gcc, clang, ASan, GUCEF_MATH_INLINE_ALL) and on every platform, because the arcs use no
 *  C library function (section 5.5 of the design)
 */
static const UInt64 PinnedRoundJoinHashFloat32 = 0x0E1C41CE8913ED9DULL;
static const UInt64 PinnedRoundJoinHashFloat64 = 0xB544C2455644A018ULL;

/*-------------------------------------------------------------------------*/

static void
AddBits( ConvexReference::CRawHash& hash, Float32 value )
{
    UInt32 bits = 0;
    memcpy( &bits, &value, sizeof( bits ) );
    hash.AddRaw( (Int64) bits );
}

static void
AddBits( ConvexReference::CRawHash& hash, Float64 value )
{
    UInt64 bits = 0;
    memcpy( &bits, &value, sizeof( bits ) );
    hash.AddRaw( (Int64) bits );
}

/** Hashes the rings of one result: the ring count, then per ring the point count and the bits of the coordinates */
template < typename T >
static UInt32
AddRings( ConvexReference::CRawHash& hash, const GUCEF::vector< GUCEF::vector< CTPoint2D< T > > >& rings )
{
    UInt32 points = 0;
    hash.AddRaw( (Int64) rings.size() );
    for ( size_t r=0; r<rings.size(); ++r )
    {
        hash.AddRaw( (Int64) rings[ r ].size() );
        for ( size_t i=0; i<rings[ r ].size(); ++i )
        {
            AddBits( hash, rings[ r ][ i ].x );
            AddBits( hash, rings[ r ][ i ].y );
        }
        points += (UInt32) rings[ r ].size();
    }
    return points;
}

/**
 *  Round joins and caps over a fixed set of inputs (concave L, a sharp spike, a star, polylines with a U-turn and a
 *  single point), grown and shrunk, with the default and an explicit arc tolerance: the result bits are hashed
 */
template < typename T >
static bool
RoundJoinHash( UInt64& hash, UInt32& pointCount )
{
    typedef CTPoint2D< T > TP;
    typedef GUCEF::vector< GUCEF::vector< TP > > TRings;
    ConvexReference::CRawHash raw;
    pointCount = 0;
    bool ok = true;

    const TP lShape[ 6 ] = { TP( T( 0 ), T( 0 ) ), TP( T( 40 ), T( 0 ) ), TP( T( 40 ), T( 10 ) ), TP( T( 10 ), T( 10 ) ), TP( T( 10 ), T( 40 ) ), TP( T( 0 ), T( 40 ) ) };
    const TP spike[ 3 ] = { TP( T( 0 ), T( 0 ) ), TP( T( 20 ), T( 1 ) ), TP( T( 0 ), T( 2 ) ) };
    const TP star[ 8 ] = { TP( T( 20 ), T( 0 ) ), TP( T( 6 ), T( 6 ) ), TP( T( 0 ), T( 20 ) ), TP( T( -6 ), T( 6 ) ), TP( T( -20 ), T( 0 ) ), TP( T( -6 ), T( -6 ) ), TP( T( 0 ), T( -20 ) ), TP( T( 6 ), T( -6 ) ) };
    const TP zigzag[ 5 ] = { TP( T( 0 ), T( 0 ) ), TP( T( 10 ), T( 5 ) ), TP( T( 20 ), T( -3 ) ), TP( T( 30 ), T( 4 ) ), TP( T( 31 ), T( 10 ) ) };
    const TP uTurn[ 3 ] = { TP( T( 0 ), T( 0 ) ), TP( T( 10 ), T( 0 ) ), TP( T( 1 ), T( 0.5 ) ) };
    const TP single[ 1 ] = { TP( T( 5 ), T( 5 ) ) };

    for ( UInt32 variant=0; variant<2; ++variant )
    {
        const Float64 arcTolerance = 0 == variant ? 0.0 : 0.01;
        {
            CTPolygonOffsetter< T > offsetter;
            offsetter.SetArcTolerance( arcTolerance );
            offsetter.AddPolygon( lShape, 6 );
            TRings grown;
            TRings shrunk;
            ok = POLYGONCLIPPER_OK == offsetter.Execute( T( 3 ), POLYGONJOIN_ROUND, POLYLINEEND_BUTT, grown ) && ok;
            ok = POLYGONCLIPPER_OK == offsetter.Execute( T( -1.5 ), POLYGONJOIN_ROUND, POLYLINEEND_BUTT, shrunk ) && ok;
            pointCount += AddRings( raw, grown );
            pointCount += AddRings( raw, shrunk );
        }
        {
            CTPolygonOffsetter< T > offsetter;
            offsetter.SetArcTolerance( arcTolerance );
            offsetter.AddPolygon( spike, 3 );
            TRings grown;
            ok = POLYGONCLIPPER_OK == offsetter.Execute( T( 2 ), POLYGONJOIN_ROUND, POLYLINEEND_BUTT, grown ) && ok;
            pointCount += AddRings( raw, grown );
        }
        {
            CTPolygonOffsetter< T > offsetter;
            offsetter.SetArcTolerance( arcTolerance );
            offsetter.AddPolygon( star, 8 );
            TRings grown;
            TRings shrunk;
            ok = POLYGONCLIPPER_OK == offsetter.Execute( T( 2.5 ), POLYGONJOIN_ROUND, POLYLINEEND_BUTT, grown ) && ok;
            ok = POLYGONCLIPPER_OK == offsetter.Execute( T( -1 ), POLYGONJOIN_ROUND, POLYLINEEND_BUTT, shrunk ) && ok;
            pointCount += AddRings( raw, grown );
            pointCount += AddRings( raw, shrunk );
        }
        {
            CTPolygonOffsetter< T > offsetter;
            offsetter.SetArcTolerance( arcTolerance );
            offsetter.AddPolyline( zigzag, 5 );
            offsetter.AddPolyline( uTurn, 3 );
            offsetter.AddPolyline( single, 1 );
            TRings stroked;
            ok = POLYGONCLIPPER_OK == offsetter.Execute( T( 1.25 ), POLYGONJOIN_ROUND, POLYLINEEND_ROUND, stroked ) && ok;
            pointCount += AddRings( raw, stroked );
        }
    }
    hash = raw.GetHash();
    return ok && pointCount > 400;
}

/*-------------------------------------------------------------------------*/

/** The round-join output is the same bits in every build: the two pinned hashes */
static bool
RoundJoinsAreDeterministic( void )
{
    UInt64 hash32 = 0;
    UInt64 hash64 = 0;
    UInt32 points32 = 0;
    UInt32 points64 = 0;
    bool ok = RoundJoinHash< Float32 >( hash32, points32 );
    ok = RoundJoinHash< Float64 >( hash64, points64 ) && ok;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PolygonOffset: round joins and caps: Float32 " << points32 << " points, hash high " << (UInt32) ( hash32 >> 32 ) << " low " << (UInt32) ( hash32 & 0xFFFFFFFFULL )
               << ", Float64 " << points64 << " points, hash high " << (UInt32) ( hash64 >> 32 ) << " low " << (UInt32) ( hash64 & 0xFFFFFFFFULL ) );
    ok = ok && PinnedRoundJoinHashFloat32 == hash32 && PinnedRoundJoinHashFloat64 == hash64;
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonOffset: round join output differs from the pinned hashes" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Invalid input */
static bool
InvalidInput( void )
{
    CTPolygonOffsetter< Float64 > offsetter;
    offsetter.AddPolygon( Square( 10.0, ToF64 ) );
    TPaths64 rings;
    bool ok = POLYGONCLIPPER_INVALID_INPUT == offsetter.Execute( std::numeric_limits< Float64 >::quiet_NaN(), POLYGONJOIN_ROUND, POLYLINEEND_BUTT, rings );
    ok = POLYGONCLIPPER_INVALID_INPUT == offsetter.Execute( 1.0, (TPolygonJoinType) 7, POLYLINEEND_BUTT, rings ) && ok;
    offsetter.SetGridScale( 1e15 );
    ok = POLYGONCLIPPER_OUT_OF_RANGE == offsetter.Execute( 1.0, POLYGONJOIN_ROUND, POLYLINEEND_BUTT, rings ) && ok;
    offsetter.SetGridScale( 0.0 );
    ok = POLYGONCLIPPER_OK == offsetter.Execute( 1.0, POLYGONJOIN_ROUND, POLYLINEEND_BUTT, rings ) && 1 == rings.size() && ok;
    offsetter.Clear();
    const P two[] = { P( 0, 0 ), P( 1, 1 ) };
    offsetter.AddPolygon( two, 2 );
    ok = POLYGONCLIPPER_OK == offsetter.Execute( 1.0, POLYGONJOIN_ROUND, POLYLINEEND_BUTT, rings ) && rings.empty() && ok;
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonOffset: invalid input not reported" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Results beyond the range of the kind are reported, not wrapped (Int32) or made infinite (Float32) */
static bool
KindRange( void )
{
    bool ok = true;
    typedef CTPoint2D< Int32 > PI;
    const Int32 high = std::numeric_limits< Int32 >::max();
    const PI nearMax[] = { PI( high - 1000, 0 ), PI( high, 0 ), PI( high, 1000 ), PI( high - 1000, 1000 ) };
    CTPolygonOffsetter< Int32 > offsetter;
    offsetter.AddPolygon( nearMax, 4 );
    GUCEF::vector< GUCEF::vector< PI > > rings;
    GUCEF::vector< CTPolygonWithHoles2D< Int32 > > polygons;
    TPolygonClipperStatus status = offsetter.Execute( 200, POLYGONJOIN_MITER, POLYLINEEND_BUTT, rings );
    if ( !( POLYGONCLIPPER_OUT_OF_RANGE == status && rings.empty() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonOffset: an Int32 offset beyond the Int32 range gives status " << (Int32) status << " and " << rings.size() << " rings" );
        ok = false;
    }
    status = offsetter.Execute( 200, POLYGONJOIN_MITER, POLYLINEEND_BUTT, polygons );
    if ( !( POLYGONCLIPPER_OUT_OF_RANGE == status && polygons.empty() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonOffset: an Int32 offset beyond the Int32 range gives status " << (Int32) status << " in polygon form" );
        ok = false;
    }
    // shrinking the same square stays in range; growing it towards the lower end of the range is fine as well
    status = offsetter.Execute( -200, POLYGONJOIN_MITER, POLYLINEEND_BUTT, rings );
    const bool inside = POLYGONCLIPPER_OK == status && 1 == rings.size() && 4 == rings[ 0 ].size() && Area( rings ) == 600.0 * 600.0;
    offsetter.Clear();
    const Int32 low = std::numeric_limits< Int32 >::min();
    const PI nearMin[] = { PI( low + 1000, 0 ), PI( low + 2000, 0 ), PI( low + 2000, 1000 ), PI( low + 1000, 1000 ) };
    offsetter.AddPolygon( nearMin, 4 );
    status = offsetter.Execute( 200, POLYGONJOIN_MITER, POLYLINEEND_BUTT, rings );
    const bool lowSide = POLYGONCLIPPER_OK == status && 1 == rings.size() && Area( rings ) == 1400.0 * 1400.0;
    status = offsetter.Execute( 1001, POLYGONJOIN_MITER, POLYLINEEND_BUTT, rings );
    const bool beyondLow = POLYGONCLIPPER_OUT_OF_RANGE == status && rings.empty();
    if ( !inside || !lowSide || !beyondLow )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonOffset: Int32 offsets near the range ends: " << inside << lowSide << beyondLow );
        ok = false;
    }
    // Float32 beyond its finite range
    typedef CTPoint2D< Float32 > PF;
    const Float32 big = 3.0e38f;
    const PF square[] = { PF( 0.0f, 0.0f ), PF( big, 0.0f ), PF( big, big ), PF( 0.0f, big ) };
    CTPolygonOffsetter< Float32 > floats;
    floats.AddPolygon( square, 4 );
    GUCEF::vector< GUCEF::vector< PF > > floatRings;
    status = floats.Execute( 1.0e38f, POLYGONJOIN_MITER, POLYLINEEND_BUTT, floatRings );
    if ( !( POLYGONCLIPPER_OUT_OF_RANGE == status && floatRings.empty() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonOffset: a Float32 offset beyond the Float32 range gives status " << (Int32) status );
        ok = false;
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Coordinates near the bottom of the Float64 range: the automatic grid scale stays finite */
static bool
TinyCoordinates( void )
{
    const Float64 side = 1e-300;
    CTPolygonOffsetter< Float64 > offsetter;
    offsetter.AddPolygon( Square( side, ToF64 ) );
    TPaths64 rings;
    const TPolygonClipperStatus status = offsetter.Execute( 0.25 * side, POLYGONJOIN_MITER, POLYLINEEND_BUTT, rings );
    bool ok = POLYGONCLIPPER_OK == status && offsetter.GetGridScale() <= std::numeric_limits< Float64 >::max() && 1 == rings.size();
    if ( ok )
    {
        // ( 1.5 side )^2 within the grid precision, in units of side
        Float64 area = 0.0;
        for ( size_t i=0; i<rings[ 0 ].size(); ++i )
        {
            const P& a = rings[ 0 ][ i ];
            const P& b = rings[ 0 ][ ( i + 1 ) % rings[ 0 ].size() ];
            area += ( a.x / side ) * ( b.y / side ) - ( b.x / side ) * ( a.y / side );
        }
        ok = fabs( 0.5 * area - 2.25 ) < 1e-6;
    }
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonOffset: a square of side 1e-300 gives status " << (Int32) status << ", grid scale " << offsetter.GetGridScale() << ", " << rings.size() << " rings" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/**
 *  Rings without area give nothing, whether they have two distinct points or more points on one line (they bound no
 *  region); the same points as a polyline give the stroke
 */
static bool
RingsWithoutArea( void )
{
    bool ok = true;
    CTPolygonOffsetter< Float64 > offsetter;
    TPaths64 rings;
    const P collinear[] = { P( 0, 0 ), P( 5, 0 ), P( 10, 0 ) };
    const P two[] = { P( 0, 0 ), P( 10, 0 ), P( 10, 0 ), P( 0, 0 ) };
    const P slanted[] = { P( 0, 0 ), P( 1, 3 ), P( 2, 6 ), P( 3, 9 ), P( 1, 3 ) };
    const TPolylineEndType ends[] = { POLYLINEEND_BUTT, POLYLINEEND_ROUND };
    for ( UInt32 k=0; k<3; ++k )
    {
        offsetter.Clear();
        if ( 0 == k ) offsetter.AddPolygon( collinear, 3 );
        if ( 1 == k ) offsetter.AddPolygon( two, 4 );
        if ( 2 == k ) offsetter.AddPolygon( slanted, 5 );
        for ( UInt32 e=0; e<2; ++e )
        {
            const TPolygonClipperStatus status = offsetter.Execute( 1.0, POLYGONJOIN_ROUND, ends[ e ], rings );
            if ( !( POLYGONCLIPPER_OK == status && rings.empty() ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonOffset: ring " << k << " without area offset by 1 gives " << rings.size() << " rings of area " << Area( rings ) );
                ok = false;
            }
        }
        const TPolygonClipperStatus shrink = offsetter.Execute( -1.0, POLYGONJOIN_ROUND, POLYLINEEND_BUTT, rings );
        if ( !( POLYGONCLIPPER_OK == shrink && rings.empty() ) )
            ok = false;
    }
    // as a polyline the collinear points are stroked: 2 d L + pi d^2
    offsetter.Clear();
    offsetter.AddPolyline( collinear, 3 );
    const TPolygonClipperStatus stroke = offsetter.Execute( 1.0, POLYGONJOIN_ROUND, POLYLINEEND_ROUND, rings );
    if ( !( POLYGONCLIPPER_OK == stroke && 1 == rings.size() && fabs( Area( rings ) - ( 20.0 + Pi ) ) < 0.02 ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonOffset: the collinear points as a polyline give area " << Area( rings ) );
        ok = false;
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

void
PerformPolygonOffsetTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "PolygonOffset" );

    GUCEF_TESTFW_TESTCASE( "PolygonOffset: a square with round, miter (with limit) and square joins, shrinking and vanishing, in every kind" )
    try
    {
        ASSERT_TRUE( ( SquareAreas< Float64 >( "Float64", 10.0, 2.0, ToF64, 1e-9 ) ) );
        ASSERT_TRUE( ( SquareAreas< Float32 >( "Float32", 10.0, 2.0, ToF32, 1e-3 ) ) );
        ASSERT_TRUE( ( SquareAreas< Int32 >( "Int32", 10000.0, 2000.0, ToI32, 2.0 * 4.0 * 14000.0 ) ) );
        ASSERT_TRUE( ( SquareAreas< Int64 >( "Int64", 10000.0, 2000.0, ToI64, 2.0 * 4.0 * 14000.0 ) ) );
        ASSERT_TRUE( ( SquareAreas< TFixed32_32 >( "TFixed32_32", 10.0, 2.0, ToFixed, 1e-6 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PolygonOffset: polygons with holes grow and shrink, holes vanish" )
    try
    {
        ASSERT_TRUE( HoleAreas() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PolygonOffset: polyline strokes with butt, square and round caps: 2 d L + pi d^2 and the other closed forms, single points" )
    try
    {
        ASSERT_TRUE( ( StrokeAreas< Float64 >( "Float64", 30.0, 2.0, ToF64, 1e-9 ) ) );
        ASSERT_TRUE( ( StrokeAreas< Float32 >( "Float32", 30.0, 2.0, ToF32, 1e-3 ) ) );
        ASSERT_TRUE( ( StrokeAreas< Int32 >( "Int32", 30000.0, 2000.0, ToI32, 2.0 * 4.0 * 34000.0 ) ) );
        ASSERT_TRUE( ( StrokeAreas< TFixed32_32 >( "TFixed32_32", 30.0, 2.0, ToFixed, 1e-6 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PolygonOffset: seeded star polygons and polylines with round joins against the distance reference" )
    try
    {
        ASSERT_TRUE( SampledReference( 8601, ScaledCount( 200 ), 400 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PolygonOffset: round joins and caps give pinned bits in Float32 and Float64, identical in every build" )
    try
    {
        ASSERT_TRUE( RoundJoinsAreDeterministic() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PolygonOffset: invalid input and the grid range" )
    try
    {
        ASSERT_TRUE( InvalidInput() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PolygonOffset: results beyond the range of the kind are OUT_OF_RANGE (Int32, Float32); tiny coordinates keep a finite grid scale" )
    try
    {
        ASSERT_TRUE( KindRange() );
        ASSERT_TRUE( TinyCoordinates() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PolygonOffset: rings without area (two points, or all on one line) give nothing; as a polyline they are stroked" )
    try
    {
        ASSERT_TRUE( RingsWithoutArea() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
