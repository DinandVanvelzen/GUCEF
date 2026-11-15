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

#ifndef GUCEF_GEOMETRY_SHAPES_H
#include "gucefGEOMETRY_shapes.h"
#define GUCEF_GEOMETRY_SHAPES_H
#endif /* GUCEF_GEOMETRY_SHAPES_H ? */

#include "TestGeometryReference.h"
#include "TestShapes2D.h"

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
using namespace GeometryReference;

typedef TFloat64Point2D P2;
typedef TFloat64Vector2D V2;
typedef TFloat64DirectionVector2D D2;
typedef CTPolygon2D< Float64 > Polygon;
typedef GUCEF::vector< CTSegment2D< Float64 > > SegmentList;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
NearPoint( const P2& p, Float64 x, Float64 y, Float64 tolerance )
{
    return Near( p.x, x, tolerance ) && Near( p.y, y, tolerance );
}

/*-------------------------------------------------------------------------*/

static Polygon
MakePolygon( const Float64* xy, UInt32 count )
{
    Polygon polygon;
    for ( UInt32 i=0; i<count; ++i )
        polygon.AddPoint( P2( xy[ 2 * i ], xy[ 2 * i + 1 ] ) );
    return polygon;
}

/*-------------------------------------------------------------------------*/

static Polygon
UnitSquare( void )
{
    const Float64 xy[ 8 ] = { 0, 0, 1, 0, 1, 1, 0, 1 };
    return MakePolygon( xy, 4 );
}

/*-------------------------------------------------------------------------*/

static Polygon
Square( Float64 x0, Float64 y0, Float64 x1, Float64 y1 )
{
    const Float64 xy[ 8 ] = { x0, y0, x1, y0, x1, y1, x0, y1 };
    return MakePolygon( xy, 4 );
}

/*-------------------------------------------------------------------------*/

/** Even-odd containment by ray crossing in Float64 for a simple ring, independent of the winding number code */
static bool
EvenOddContains( const Polygon& polygon, const P2& p )
{
    bool inside = false;
    const UInt32 n = polygon.GetPointCount();
    for ( UInt32 i=0, j=n-1; i<n; j=i++ )
    {
        const P2& a = polygon.GetPoint( i );
        const P2& b = polygon.GetPoint( j );
        if ( ( a.y > p.y ) != ( b.y > p.y ) && p.x < ( b.x - a.x ) * ( p.y - a.y ) / ( b.y - a.y ) + a.x )
            inside = !inside;
    }
    return inside;
}

/*-------------------------------------------------------------------------*/

static Float64
BoundaryDistance( const Polygon& polygon, const P2& p )
{
    Float64 best = 1e300;
    for ( UInt32 i=0; i<polygon.GetPointCount(); ++i )
    {
        const P2& a = polygon.GetPoint( i );
        const P2& b = polygon.GetPoint( ( i + 1 ) % polygon.GetPointCount() );
        SVec3 closest;
        Float64 d = SegmentDistance( SVec3( p.x, p.y, 0 ), SVec3( a.x, a.y, 0 ), SVec3( b.x, b.y, 0 ), closest );
        best = d < best ? d : best;
    }
    return best;
}

/*-------------------------------------------------------------------------*/

/** A closed outline: every segment starts where the previous one ends, the first starts where the last ends */
static bool
IsClosedOutline( const SegmentList& lines, UInt32 first, UInt32 count )
{
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( !( lines[ first + i ].end == lines[ first + ( i + 1 ) % count ].start ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformShapes2DTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Shapes2D" );

    GUCEF_TESTFW_TESTCASE( "Shapes2D: polygon signed area, centroid, orientation and perimeter" )
    try
    {
        Polygon square = UnitSquare();
        ASSERT_TRUE( Near( square.GetSignedArea(), 1, 1e-15 ) && Near( square.GetArea(), 1, 1e-15 ) && square.IsCounterClockwise() );
        ASSERT_TRUE( NearPoint( square.GetCentroid(), 0.5, 0.5, 1e-15 ) && Near( square.GetPerimeter(), 4, 1e-15 ) );
        square.Reverse();
        ASSERT_TRUE( Near( square.GetSignedArea(), -1, 1e-15 ) && Near( square.GetArea(), 1, 1e-15 ) && !square.IsCounterClockwise() );
        ASSERT_TRUE( NearPoint( square.GetCentroid(), 0.5, 0.5, 1e-15 ) );

        // an L shape: the unit squares at (0,0), (1,0) and (0,1): area 3, centroid ( 5/6, 5/6 )
        const Float64 l[ 12 ] = { 0, 0, 2, 0, 2, 1, 1, 1, 1, 2, 0, 2 };
        const Polygon lShape = MakePolygon( l, 6 );
        ASSERT_TRUE( Near( lShape.GetSignedArea(), 3, 1e-15 ) && NearPoint( lShape.GetCentroid(), 5.0 / 6.0, 5.0 / 6.0, 1e-15 ) && Near( lShape.GetPerimeter(), 8, 1e-15 ) );

        // far from the origin the area does not lose precision
        const Polygon far = Square( 1.0e6, 2.0e6, 1.0e6 + 3, 2.0e6 + 2 );
        ASSERT_TRUE( Near( far.GetSignedArea(), 6, 1e-9 ) && NearPoint( far.GetCentroid(), 1.0e6 + 1.5, 2.0e6 + 1, 1e-9 ) );

        // fewer than three points
        Polygon few;
        ASSERT_TRUE( 0.0 == few.GetSignedArea() && 0.0 == few.GetPerimeter() && NearPoint( few.GetCentroid(), 0, 0, 0 ) && few.GetBounds().IsEmpty() );
        few.AddPoint( P2( 2, 3 ) );
        ASSERT_TRUE( NearPoint( few.GetCentroid(), 2, 3, 0 ) && few.ClosestPoint( P2( 5, 7 ) ) == P2( 2, 3 ) && !few.Contains( P2( 2, 3 ) ) && few.IsOnBoundary( P2( 2, 3 ) ) );
        few.AddPoint( P2( 4, 3 ) );
        ASSERT_TRUE( 0.0 == few.GetSignedArea() && Near( few.GetPerimeter(), 2, 1e-15 ) && NearPoint( few.ClosestPoint( P2( 3, 9 ) ), 3, 3, 1e-15 ) && !few.IsConvex() && !few.IsSimple() );

        // collinear points: zero area, the centroid is the mean of the points
        const Float64 line[ 6 ] = { 0, 0, 1, 1, 2, 2 };
        const Polygon collinear = MakePolygon( line, 3 );
        ASSERT_TRUE( 0.0 == collinear.GetSignedArea() && NearPoint( collinear.GetCentroid(), 1, 1, 1e-15 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes2D: polygon IsConvex and IsSimple on convex, concave and self-intersecting rings" )
    try
    {
        const Polygon square = UnitSquare();
        ASSERT_TRUE( square.IsConvex() && square.IsSimple() );
        Polygon reversed = square;
        reversed.Reverse();
        ASSERT_TRUE( reversed.IsConvex() && reversed.IsSimple() );

        const Float64 l[ 12 ] = { 0, 0, 2, 0, 2, 1, 1, 1, 1, 2, 0, 2 };
        const Polygon lShape = MakePolygon( l, 6 );
        ASSERT_TRUE( !lShape.IsConvex() && lShape.IsSimple() );

        // a bow tie crosses itself
        const Float64 bow[ 8 ] = { 0, 0, 1, 1, 1, 0, 0, 1 };
        const Polygon bowTie = MakePolygon( bow, 4 );
        ASSERT_TRUE( !bowTie.IsConvex() && !bowTie.IsSimple() );

        // a pentagram: every turn is to the same side, but it winds twice
        Polygon star;
        const Float64 pi = 3.14159265358979323846;
        const UInt32 order[ 5 ] = { 0, 2, 4, 1, 3 };
        for ( UInt32 i=0; i<5; ++i )
            star.AddPoint( P2( cos( 2 * pi * order[ i ] / 5 ), sin( 2 * pi * order[ i ] / 5 ) ) );
        ASSERT_TRUE( !star.IsConvex() && !star.IsSimple() );

        // a regular pentagon
        Polygon pentagon;
        for ( UInt32 i=0; i<5; ++i )
            pentagon.AddPoint( P2( cos( 2 * pi * i / 5 ), sin( 2 * pi * i / 5 ) ) );
        ASSERT_TRUE( pentagon.IsConvex() && pentagon.IsSimple() );

        // collinear vertices on an edge keep a ring convex and simple
        const Float64 withMid[ 10 ] = { 0, 0, 0.5, 0, 1, 0, 1, 1, 0, 1 };
        const Polygon mid = MakePolygon( withMid, 5 );
        ASSERT_TRUE( mid.IsConvex() && mid.IsSimple() );

        // an edge that folds back over its neighbour, a repeated vertex and a vertex touching another edge are not simple
        const Float64 fold[ 8 ] = { 0, 0, 2, 0, 1, 0, 1, 1 };
        ASSERT_FALSE( MakePolygon( fold, 4 ).IsSimple() );
        const Float64 repeated[ 8 ] = { 0, 0, 1, 0, 1, 0, 0, 1 };
        ASSERT_FALSE( MakePolygon( repeated, 4 ).IsSimple() );
        const Float64 touching[ 14 ] = { 0, 0, 4, 0, 4, 4, 2, 0, 0, 4, 0, 2, 0, 1 };
        ASSERT_FALSE( MakePolygon( touching, 7 ).IsSimple() );

        // all points collinear is not a convex polygon
        const Float64 line[ 8 ] = { 0, 0, 1, 0, 2, 0, 3, 0 };
        ASSERT_FALSE( MakePolygon( line, 4 ).IsConvex() );
        ASSERT_FALSE( Polygon().IsConvex() || Polygon().IsSimple() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes2D: polygon containment and closest point, 1000 seeded star-shaped rings against even-odd" )
    try
    {
        const Polygon square = UnitSquare();
        ASSERT_TRUE( square.Contains( P2( 0.5, 0.5 ) ) && square.Contains( P2( 1, 0.5 ) ) && square.Contains( P2( 0, 0 ) ) && square.Contains( P2( 1, 1 ) ) );
        ASSERT_FALSE( square.Contains( P2( 1.001, 0.5 ) ) || square.Contains( P2( -0.5, 0.5 ) ) || square.Contains( P2( 0.5, 1.5 ) ) );
        ASSERT_TRUE( NearPoint( square.ClosestPoint( P2( 2, 0.5 ) ), 1, 0.5, 1e-15 ) && NearPoint( square.ClosestPoint( P2( 2, 3 ) ), 1, 1, 1e-15 ) && NearPoint( square.ClosestPoint( P2( 0.25, 0.5 ) ), 0.25, 0.5, 0 ) );
        ASSERT_TRUE( Near( square.GetDistance( P2( 2, 0.5 ) ), 1, 1e-15 ) && 0.0 == square.GetDistance( P2( 0.5, 0.5 ) ) && Near( square.GetSquaredDistance( P2( 3, 1 ) ), 4, 1e-15 ) );
        ASSERT_TRUE( NearPoint( square.GetClosestBoundaryPoint( P2( 0.5, 0.4 ) ), 0.5, 0, 1e-15 ) );
        ASSERT_TRUE( square.IsOnBoundary( P2( 0.5, 0 ) ) && !square.IsOnBoundary( P2( 0.5, 0.5 ) ) );
        ASSERT_TRUE( square.GetBounds() == CTRectangle< Float64 >( P2( 0, 0 ), P2( 1, 1 ) ) && square.GetSupport( V2( 1, 1 ) ) == P2( 1, 1 ) && square.GetSupport( V2( -1, -1 ) ) == P2( 0, 0 ) );

        // the notch of an L shape is outside, the corner point inside
        const Float64 l[ 12 ] = { 0, 0, 2, 0, 2, 1, 1, 1, 1, 2, 0, 2 };
        const Polygon lShape = MakePolygon( l, 6 );
        ASSERT_TRUE( !lShape.Contains( P2( 1.5, 1.5 ) ) && lShape.Contains( P2( 0.5, 1.5 ) ) && lShape.Contains( P2( 1.5, 0.5 ) ) && lShape.Contains( P2( 1, 1 ) ) );
        ASSERT_TRUE( NearPoint( lShape.ClosestPoint( P2( 1.5, 1.5 ) ), 1.5, 1, 1e-15 ) || NearPoint( lShape.ClosestPoint( P2( 1.5, 1.5 ) ), 1, 1.5, 1e-15 ) );

        // a bow tie contains both lobes (non zero winding)
        const Float64 bow[ 8 ] = { 0, 0, 1, 1, 1, 0, 0, 1 };
        const Polygon bowTie = MakePolygon( bow, 4 );
        ASSERT_TRUE( bowTie.Contains( P2( 0.9, 0.5 ) ) && bowTie.Contains( P2( 0.1, 0.5 ) ) );

        const UInt64 seed = 0xB0A7D2;
        CSeededSequence random( seed );
        UInt32 failures = 0;
        UInt32 convexMismatches = 0;
        const Float64 pi = 3.14159265358979323846;
        for ( UInt32 i=0; i<1000; ++i )
        {
            // a star-shaped ring around ( cx, cy ): increasing angles, random radii, so it is simple
            const UInt32 n = 3 + (UInt32) random.NextInt32( 0, 12 );
            const Float64 cx = random.NextFloat64( -5, 5 );
            const Float64 cy = random.NextFloat64( -5, 5 );
            Polygon ring;
            Float64 angle = random.NextFloat64( 0, 2 * pi );
            for ( UInt32 k=0; k<n; ++k )
            {
                const Float64 radius = random.NextFloat64( 0.5, 4 );
                ring.AddPoint( P2( cx + radius * cos( angle ), cy + radius * sin( angle ) ) );
                angle += ( 2 * pi / n ) * random.NextFloat64( 0.6, 1.0 );
                if ( angle - 0 > 1e9 )
                    break;
            }
            // the angles may not span a full turn: close the ring only if it still winds once
            Float64 spanned = 0;
            for ( UInt32 k=0; k<ring.GetPointCount(); ++k )
            {
                const P2& a = ring.GetPoint( k );
                const P2& b = ring.GetPoint( ( k + 1 ) % ring.GetPointCount() );
                Float64 turn = atan2( ( a.x - cx ) * ( b.y - cy ) - ( a.y - cy ) * ( b.x - cx ), ( a.x - cx ) * ( b.x - cx ) + ( a.y - cy ) * ( b.y - cy ) );
                spanned += turn;
            }
            if ( !Near( spanned, 2 * pi, 1e-6 ) )
                continue;
            if ( !ring.IsSimple() )
            {
                ++failures;
                continue;
            }
            for ( UInt32 q=0; q<20; ++q )
            {
                const P2 p( random.NextFloat64( -10, 10 ), random.NextFloat64( -10, 10 ) );
                const Float64 boundary = BoundaryDistance( ring, p );
                if ( boundary < 1e-9 )
                    continue;
                const bool expected = EvenOddContains( ring, p );
                if ( ring.Contains( p ) != expected )
                    ++failures;
                const P2 closest = ring.ClosestPoint( p );
                const Float64 distance = ( p - closest ).GetLength();
                if ( expected ? distance != 0.0 : !Near( distance, boundary, 1e-9 ) )
                    ++failures;
            }
            // for a simple ring the ring is convex when all turns have the same sign
            bool positive = false;
            bool negative = false;
            for ( UInt32 k=0; k<ring.GetPointCount(); ++k )
            {
                const V2 e1 = ring.GetPoint( ( k + 1 ) % ring.GetPointCount() ) - ring.GetPoint( k );
                const V2 e2 = ring.GetPoint( ( k + 2 ) % ring.GetPointCount() ) - ring.GetPoint( ( k + 1 ) % ring.GetPointCount() );
                const Float64 cross = e1.x * e2.y - e1.y * e2.x;
                positive = positive || cross > 0;
                negative = negative || cross < 0;
            }
            if ( ring.IsConvex() != !( positive && negative ) )
                ++convexMismatches;
        }
        if ( failures > 0 || convexMismatches > 0 )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Shapes2D: polygon sweep failures " << failures << " convex mismatches " << convexMismatches << ", seed " << seed );
        ASSERT_TRUE( 0 == failures && 0 == convexMismatches );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes2D: polygon with holes area, containment, closest point and centroid" )
    try
    {
        CTPolygonWithHoles2D< Float64 > region( Square( 0, 0, 10, 10 ) );
        region.AddHole( Square( 4, 4, 6, 6 ) );
        ASSERT_TRUE( 1 == region.GetHoleCount() && Near( region.GetArea(), 96, 1e-13 ) && Near( region.GetPerimeter(), 48, 1e-13 ) );
        ASSERT_TRUE( region.Contains( P2( 1, 1 ) ) && !region.Contains( P2( 5, 5 ) ) && region.Contains( P2( 4, 5 ) ) && region.Contains( P2( 10, 10 ) ) && !region.Contains( P2( 11, 5 ) ) );
        ASSERT_TRUE( Near( region.GetDistance( P2( 5, 5 ) ), 1, 1e-14 ) && NearPoint( region.ClosestPoint( P2( 12, 5 ) ), 10, 5, 1e-14 ) && 0.0 == region.GetDistance( P2( 1, 1 ) ) );
        ASSERT_TRUE( NearPoint( region.GetCentroid(), 5, 5, 1e-13 ) );

        CTPolygonWithHoles2D< Float64 > lopsided( Square( 0, 0, 10, 10 ) );
        lopsided.AddHole( Square( 6, 4, 8, 6 ) );
        ASSERT_TRUE( NearPoint( lopsided.GetCentroid(), 472.0 / 96.0, 5, 1e-13 ) );
        // the hole may be written clockwise: the areas use magnitudes
        Polygon reversedHole = Square( 6, 4, 8, 6 );
        reversedHole.Reverse();
        CTPolygonWithHoles2D< Float64 > reversed( Square( 0, 0, 10, 10 ) );
        reversed.AddHole( reversedHole );
        ASSERT_TRUE( Near( reversed.GetArea(), 96, 1e-13 ) && !reversed.Contains( P2( 7, 5 ) ) && reversed.Contains( P2( 5, 5 ) ) );
        ASSERT_TRUE( region.GetBounds() == CTRectangle< Float64 >( P2( 0, 0 ), P2( 10, 10 ) ) && region.GetSupport( V2( 1, 1 ) ) == P2( 10, 10 ) );
        SegmentList lines;
        region.GetWireframeLines( lines, 8 );
        ASSERT_TRUE( 8 == lines.size() && IsClosedOutline( lines, 0, 4 ) && IsClosedOutline( lines, 4, 4 ) );
        CTPolygonWithHoles2D< Float64 > none;
        ASSERT_TRUE( 0.0 == none.GetArea() && !none.Contains( P2( 0, 0 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes2D: segment intersection, line and ray queries" )
    try
    {
        typedef CTSegment2D< Float64 > Seg;
        const Seg a( P2( 0, 0 ), P2( 4, 4 ) );
        ASSERT_TRUE( a.Intersects( Seg( P2( 0, 4 ), P2( 4, 0 ) ) ) );                       // crossing
        ASSERT_TRUE( a.Intersects( Seg( P2( 4, 4 ), P2( 6, 2 ) ) ) );                       // touching at an end point
        ASSERT_TRUE( a.Intersects( Seg( P2( 2, 2 ), P2( 5, 0 ) ) ) );                       // end point on the segment
        ASSERT_TRUE( a.Intersects( Seg( P2( 1, 1 ), P2( 3, 3 ) ) ) );                       // collinear, inside
        ASSERT_TRUE( a.Intersects( Seg( P2( 3, 3 ), P2( 7, 7 ) ) ) );                       // collinear, overlapping
        ASSERT_FALSE( a.Intersects( Seg( P2( 5, 5 ), P2( 7, 7 ) ) ) );                      // collinear, apart
        ASSERT_FALSE( a.Intersects( Seg( P2( 0, 1 ), P2( 4, 5 ) ) ) );                      // parallel
        ASSERT_FALSE( a.Intersects( Seg( P2( 0, 4 ), P2( 1, 3.5 ) ) ) );                    // would cross if extended
        ASSERT_TRUE( a.Intersects( Seg( P2( 2, 2 ), P2( 2, 2 ) ) ) && !a.Intersects( Seg( P2( 2, 3 ), P2( 2, 3 ) ) ) );   // a point
        ASSERT_TRUE( Seg( P2( 1, 1 ), P2( 1, 1 ) ).Intersects( Seg( P2( 1, 1 ), P2( 1, 1 ) ) ) );
        ASSERT_TRUE( Near( a.GetLength(), sqrt( 32.0 ), 1e-14 ) && NearPoint( a.ClosestPoint( P2( 0, 4 ) ), 2, 2, 1e-15 ) && NearPoint( a.ClosestPoint( P2( -3, -3 ) ), 0, 0, 0 ) );
        ASSERT_TRUE( a.Contains( P2( 1, 1 ) ) && !a.Contains( P2( 1, 1.1 ) ) && a.GetDirection().Equals( D2( V2( 1, 1 ) ) ) && Seg( P2( 1, 1 ), P2( 1, 1 ) ).GetDirection() == D2::Right() );
        ASSERT_TRUE( a.GetBounds() == CTRectangle< Float64 >( P2( 0, 0 ), P2( 4, 4 ) ) && a.GetSupport( V2( -1, 0 ) ) == P2( 0, 0 ) && a.GetSupport( V2( 1, 0 ) ) == P2( 4, 4 ) );
        ASSERT_TRUE( NearPoint( a.GetMidpoint(), 2, 2, 0 ) && NearPoint( a.GetPoint( 0.25 ), 1, 1, 1e-15 ) && a.Reversed().start == a.end );

        const CTLine2D< Float64 > l( P2( 0, 1 ), D2::Right() );
        ASSERT_TRUE( Near( l.GetSignedDistance( P2( 5, 3 ) ), 2, 1e-15 ) && Near( l.GetSignedDistance( P2( 5, -1 ) ), -2, 1e-15 ) && Near( l.GetDistance( P2( 5, -1 ) ), 2, 1e-15 ) );
        ASSERT_TRUE( NearPoint( l.ClosestPoint( P2( 5, 3 ) ), 5, 1, 1e-15 ) && l.Contains( P2( -9, 1 ) ) );
        P2 point;
        ASSERT_TRUE( l.Intersect( CTLine2D< Float64 >( P2( 3, 0 ), D2::Up() ), point ) && NearPoint( point, 3, 1, 1e-15 ) );
        P2 unchanged( 8, 8 );
        ASSERT_FALSE( l.Intersect( CTLine2D< Float64 >( P2( 3, 0 ), D2::Right() ), unchanged ) || unchanged != P2( 8, 8 ) );
        CTLine2D< Float64 > through;
        ASSERT_TRUE( CTLine2D< Float64 >::TryFromPoints( P2( 0, 0 ), P2( 1, 1 ), through ) && through.Contains( P2( 5, 5 ), 1e-12 ) && !CTLine2D< Float64 >::TryFromPoints( P2( 1, 1 ), P2( 1, 1 ), through ) );

        const CTRay2D< Float64 > r( P2( 0, 0 ), D2::Right() );
        ASSERT_TRUE( NearPoint( r.ClosestPoint( P2( -2, 1 ) ), 0, 0, 0 ) && NearPoint( r.ClosestPoint( P2( 3, 1 ) ), 3, 0, 1e-15 ) && r.Contains( P2( 7, 0 ) ) && !r.Contains( P2( -1, 0 ) ) );
        ASSERT_TRUE( Near( r.GetDistance( P2( -3, 4 ) ), 5, 1e-14 ) && NearPoint( r.GetPoint( 2.5 ), 2.5, 0, 0 ) );
        CTRay2D< Float64 > from;
        ASSERT_TRUE( CTRay2D< Float64 >::TryFromPoints( P2( 1, 1 ), P2( 1, 5 ), from ) && from.direction == D2::Up() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes2D: circle, rectangle and their wireframes" )
    try
    {
        const CTCircle< Float64 > c( P2( 1, 2 ), 3 );
        const Float64 pi = 3.14159265358979323846;
        ASSERT_TRUE( c.Contains( P2( 4, 2 ) ) && !c.Contains( P2( 4.001, 2 ) ) && c.Contains( P2( 1, 2 ) ) );
        ASSERT_TRUE( NearPoint( c.ClosestPoint( P2( 11, 2 ) ), 4, 2, 1e-14 ) && NearPoint( c.ClosestPoint( P2( 2, 2 ) ), 2, 2, 0 ) && Near( c.GetDistance( P2( 11, 2 ) ), 7, 1e-14 ) && 0.0 == c.GetDistance( P2( 2, 2 ) ) );
        ASSERT_TRUE( Near( c.GetArea(), 9 * pi, 1e-13 ) && Near( c.GetCircumference(), 6 * pi, 1e-13 ) && Near( c.GetSignedDistance( P2( 1, 2 ) ), -3, 1e-15 ) );
        ASSERT_TRUE( c.GetBounds() == CTRectangle< Float64 >( P2( -2, -1 ), P2( 4, 5 ) ) && NearPoint( c.GetSupport( V2( 0, 7 ) ), 1, 5, 1e-15 ) && NearPoint( c.GetSupport( V2( 0, 0 ) ), 4, 2, 0 ) );
        ASSERT_TRUE( c.Intersects( CTCircle< Float64 >( P2( 7, 2 ), 3 ) ) && !c.Intersects( CTCircle< Float64 >( P2( 7.1, 2 ), 3 ) ) && !CTCircle< Float64 >( P2( 0, 0 ), -1 ).Contains( P2( 0, 0 ) ) );
        SegmentList lines;
        c.GetWireframeLines( lines, 16 );
        ASSERT_TRUE( 16 == lines.size() && IsClosedOutline( lines, 0, 16 ) );
        for ( UInt32 i=0; i<16; ++i )
            ASSERT_TRUE( Near( ( lines[ i ].start - c.centre ).GetLength(), 3, 1e-13 ) );
        lines.clear();
        c.GetWireframeLines( lines, 1 );
        ASSERT_TRUE( 3 == lines.size() && IsClosedOutline( lines, 0, 3 ) );

        const CTRectangle< Float64 > r( P2( -1, -2 ), P2( 3, 4 ) );
        ASSERT_TRUE( r.Contains( P2( 0, 0 ) ) && r.Contains( P2( 3, 4 ) ) && !r.Contains( P2( 3.001, 0 ) ) && NearPoint( r.ClosestPoint( P2( 5, 0 ) ), 3, 0, 0 ) );
        ASSERT_TRUE( Near( r.GetDistance( P2( 6, 8 ) ), 5, 1e-14 ) && 0.0 == r.GetDistance( P2( 0, 0 ) ) && Near( r.GetSquaredDistance( P2( 6, 8 ) ), 25, 1e-13 ) );
        ASSERT_TRUE( Near( r.GetArea(), 24, 1e-14 ) && Near( r.GetPerimeter(), 20, 1e-14 ) && r.GetCentre() == P2( 1, 1 ) && r.GetExtents() == V2( 4, 6 ) && r.GetHalfExtents() == V2( 2, 3 ) );
        ASSERT_TRUE( r.GetCorner( 0 ) == r.min && r.GetCorner( 3 ) == r.max && r.GetCorner( 1 ) == P2( 3, -2 ) && r.GetCorner( 2 ) == P2( -1, 4 ) && r.GetSupport( V2( 1, -1 ) ) == P2( 3, -2 ) );
        const CTRectangle< Float64 > empty;
        ASSERT_TRUE( empty.IsEmpty() && empty.Merged( r ) == r && r.Merged( empty ) == r && !empty.Contains( P2( 0, 0 ) ) && 0.0 == empty.GetArea() && !empty.Intersects( r ) );
        ASSERT_TRUE( r.Contains( empty ) && !empty.Contains( r ) && empty.GetDistance( P2( 1, 1 ) ) == CTNumericTraits< Float64 >::Max() && empty.ClosestPoint( P2( 1, 1 ) ) == P2( 1, 1 ) );
        ASSERT_TRUE( r.Intersection( CTRectangle< Float64 >( P2( 2, 3 ), P2( 9, 9 ) ) ) == CTRectangle< Float64 >( P2( 2, 3 ), P2( 3, 4 ) ) && r.Intersection( CTRectangle< Float64 >( P2( 4, 0 ), P2( 5, 1 ) ) ).IsEmpty() );
        ASSERT_TRUE( r.Intersects( CTRectangle< Float64 >( P2( 3, 4 ), P2( 6, 6 ) ) ) && r.Expanded( 1 ) == CTRectangle< Float64 >( P2( -2, -3 ), P2( 4, 5 ) ) && r.Expanded( -3 ).IsEmpty() );
        const P2 pts[ 3 ] = { P2( 1, 2 ), P2( -4, 5 ), P2( 2, -6 ) };
        ASSERT_TRUE( CTRectangle< Float64 >::FromPoints( pts, 3 ) == CTRectangle< Float64 >( P2( -4, -6 ), P2( 2, 5 ) ) && CTRectangle< Float64 >::FromPoints( pts, 0 ).IsEmpty() );
        ASSERT_TRUE( r.ExpandedToInclude( P2( 10, 0 ) ).max.x == 10 && r.GetBounds() == r );
        lines.clear();
        r.GetWireframeLines( lines, 8 );
        ASSERT_TRUE( 4 == lines.size() && IsClosedOutline( lines, 0, 4 ) && lines[ 0 ].start == r.min && lines[ 0 ].end == P2( 3, -2 ) );
        Polygon outline;
        for ( UInt32 i=0; i<4; ++i )
            outline.AddPoint( lines[ i ].start );
        ASSERT_TRUE( outline.IsCounterClockwise() && Near( outline.GetArea(), 24, 1e-13 ) );
        lines.clear();
        empty.GetWireframeLines( lines, 8 );
        ASSERT_TRUE( lines.empty() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes2D: oriented rectangle rotation, containment and closest point against Float64 references" )
    try
    {
        const Float64 pi = 3.14159265358979323846;
        const CTOrientedRectangle< Float64 > r( P2( 0, 0 ), CTRadian< Float64 >( pi / 2 ), V2( 2, 1 ) );
        ASSERT_TRUE( r.Contains( P2( 0, 1.9 ) ) && r.Contains( P2( 0.9, 0 ) ) && !r.Contains( P2( 1.1, 0 ) ) && !r.Contains( P2( 0, 2.1 ) ) );
        ASSERT_TRUE( Near( r.GetArea(), 8, 1e-15 ) );
        ASSERT_TRUE( NearPoint( r.ClosestPoint( P2( 3, 0 ) ), 1, 0, 1e-12 ) && NearPoint( r.ClosestPoint( P2( 0, -5 ) ), 0, -2, 1e-12 ) && Near( r.GetDistance( P2( 3, 0 ) ), 2, 1e-12 ) );
        const CTRectangle< Float64 > bounds = r.GetBounds();
        ASSERT_TRUE( NearPoint( bounds.min, -1, -2, 1e-12 ) && NearPoint( bounds.max, 1, 2, 1e-12 ) );
        P2 corners[ 4 ];
        r.GetCorners( corners );
        Polygon outline;
        for ( UInt32 i=0; i<4; ++i )
            outline.AddPoint( corners[ i ] );
        ASSERT_TRUE( outline.IsCounterClockwise() && Near( outline.GetArea(), 8, 1e-12 ) && outline.IsConvex() );
        ASSERT_TRUE( NearPoint( corners[ 0 ], 1, -2, 1e-12 ) && NearPoint( corners[ 2 ], -1, 2, 1e-12 ) );
        ASSERT_TRUE( r.GetSupport( V2( 0, 1 ) ).y > 1.999999 && r.GetAxisX() == D2::FromAngle( CTRadian< Float64 >( pi / 2 ) ) );
        const V2 local = r.ToLocal( P2( 0.3, -0.7 ) );
        ASSERT_TRUE( NearPoint( r.FromLocal( local ), 0.3, -0.7, 1e-12 ) && Near( local.x, -0.7, 1e-12 ) && Near( local.y, -0.3, 1e-12 ) );
        SegmentList lines;
        r.GetWireframeLines( lines, 8 );
        ASSERT_TRUE( 4 == lines.size() && IsClosedOutline( lines, 0, 4 ) );

        // seeded sweep: arbitrary rotations against the textbook rotation of the query point
        const UInt64 seed = 0x0E1E27;
        CSeededSequence random( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<5000; ++i )
        {
            const Float64 angle = random.NextFloat64( -7, 7 );
            const P2 centre( random.NextFloat64( -5, 5 ), random.NextFloat64( -5, 5 ) );
            const V2 half( random.NextFloat64( 0.2, 4 ), random.NextFloat64( 0.2, 4 ) );
            const CTOrientedRectangle< Float64 > rect( centre, CTRadian< Float64 >( angle ), half );
            const P2 p( random.NextFloat64( -10, 10 ), random.NextFloat64( -10, 10 ) );
            const Float64 dx = p.x - centre.x;
            const Float64 dy = p.y - centre.y;
            const Float64 lx = dx * cos( angle ) + dy * sin( angle );
            const Float64 ly = -dx * sin( angle ) + dy * cos( angle );
            if ( fabs( fabs( lx ) - half.x ) < 1e-9 || fabs( fabs( ly ) - half.y ) < 1e-9 )
                continue;
            const bool expected = fabs( lx ) <= half.x && fabs( ly ) <= half.y;
            if ( rect.Contains( p ) != expected )
                ++failures;
            const Float64 cx = lx < -half.x ? -half.x : ( lx > half.x ? half.x : lx );
            const Float64 cy = ly < -half.y ? -half.y : ( ly > half.y ? half.y : ly );
            const Float64 distance = sqrt( ( lx - cx ) * ( lx - cx ) + ( ly - cy ) * ( ly - cy ) );
            if ( !Near( rect.GetDistance( p ), distance, 1e-9 ) )
                ++failures;
            if ( !rect.GetBounds().Expanded( 1e-9 ).Contains( rect.ClosestPoint( p ) ) )
                ++failures;
        }
        if ( failures > 0 )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Shapes2D: oriented rectangle sweep failures " << failures << ", seed " << seed );
        ASSERT_TRUE( 0 == failures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes2D: triangle containment for both windings, closest point and degenerate cases" )
    try
    {
        const CTTriangle2D< Float64 > ccw( P2( 0, 0 ), P2( 4, 0 ), P2( 0, 4 ) );
        const CTTriangle2D< Float64 > cw( P2( 0, 0 ), P2( 0, 4 ), P2( 4, 0 ) );
        ASSERT_TRUE( Near( ccw.GetSignedArea(), 8, 1e-15 ) && Near( cw.GetSignedArea(), -8, 1e-15 ) && Near( cw.GetArea(), 8, 1e-15 ) && ccw.IsCounterClockwise() && !cw.IsCounterClockwise() );
        const CTTriangle2D< Float64 >* triangles[ 2 ] = { &ccw, &cw };
        for ( UInt32 i=0; i<2; ++i )
        {
            const CTTriangle2D< Float64 >& t = *triangles[ i ];
            ASSERT_TRUE( t.Contains( P2( 1, 1 ) ) && t.Contains( P2( 2, 2 ) ) && t.Contains( P2( 0, 0 ) ) && t.Contains( P2( 2, 0 ) ) && !t.Contains( P2( 3, 3 ) ) && !t.Contains( P2( -0.1, 1 ) ) );
            ASSERT_TRUE( NearPoint( t.ClosestPoint( P2( 1, 1 ) ), 1, 1, 0 ) && NearPoint( t.ClosestPoint( P2( 3, 3 ) ), 2, 2, 1e-15 ) && NearPoint( t.ClosestPoint( P2( -1, -1 ) ), 0, 0, 1e-15 ) && NearPoint( t.ClosestPoint( P2( 2, -3 ) ), 2, 0, 1e-15 ) );
            ASSERT_TRUE( Near( t.GetDistance( P2( 3, 3 ) ), sqrt( 2.0 ), 1e-14 ) && 0.0 == t.GetDistance( P2( 1, 1 ) ) && NearPoint( t.GetCentroid(), 4.0 / 3, 4.0 / 3, 1e-15 ) );
            ASSERT_TRUE( t.GetBounds() == CTRectangle< Float64 >( P2( 0, 0 ), P2( 4, 4 ) ) );
        }
        TFloat64Vector3D bc;
        ASSERT_TRUE( ccw.TryGetBarycentric( P2( 1, 1 ), bc ) && Near( bc.x, 0.5, 1e-15 ) && Near( bc.y, 0.25, 1e-15 ) && Near( bc.z, 0.25, 1e-15 ) );
        ASSERT_TRUE( ccw.GetSupport( V2( 1, 0 ) ) == P2( 4, 0 ) && ccw.GetSupport( V2( -1, -1 ) ) == P2( 0, 0 ) );
        const CTTriangle2D< Float64 > collinear( P2( 0, 0 ), P2( 1, 1 ), P2( 3, 3 ) );
        ASSERT_TRUE( collinear.IsDegenerate() && !collinear.TryGetBarycentric( P2( 1, 1 ), bc ) && 0.0 == collinear.GetSignedArea() );
        ASSERT_TRUE( collinear.Contains( P2( 2, 2 ) ) && !collinear.Contains( P2( 2, 2.5 ) ) && !collinear.Contains( P2( 4, 4 ) ) );
        ASSERT_TRUE( NearPoint( collinear.ClosestPoint( P2( 0, 4 ) ), 2, 2, 1e-15 ) && NearPoint( collinear.ClosestPoint( P2( 5, 5 ) ), 3, 3, 1e-15 ) );
        SegmentList lines;
        ccw.GetWireframeLines( lines, 8 );
        ASSERT_TRUE( 3 == lines.size() && IsClosedOutline( lines, 0, 3 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes2D: polygon wireframe and polyline queries" )
    try
    {
        SegmentList lines;
        UnitSquare().GetWireframeLines( lines, 8 );
        ASSERT_TRUE( 4 == lines.size() && IsClosedOutline( lines, 0, 4 ) );
        lines.clear();
        Polygon two;
        two.AddPoint( P2( 0, 0 ) );
        two.AddPoint( P2( 1, 0 ) );
        two.GetWireframeLines( lines, 8 );
        ASSERT_TRUE( 1 == lines.size() );
        lines.clear();
        Polygon().GetWireframeLines( lines, 8 );
        ASSERT_TRUE( lines.empty() );

        CTPolyline2D< Float64 > line;
        line.AddPoint( P2( 0, 0 ) );
        line.AddPoint( P2( 3, 0 ) );
        line.AddPoint( P2( 3, 4 ) );
        ASSERT_TRUE( Near( line.GetLength(), 7, 1e-14 ) && 2 == line.GetSegmentCount() && NearPoint( line.GetPointAtDistance( 5 ), 3, 2, 1e-14 ) && NearPoint( line.GetPointAtDistance( 100 ), 3, 4, 0 ) );
        ASSERT_TRUE( NearPoint( line.ClosestPoint( P2( 1, 1 ) ), 1, 0, 1e-15 ) && line.Contains( P2( 3, 2 ) ) && !line.Contains( P2( 1, 1 ) ) && line.GetBounds() == CTRectangle< Float64 >( P2( 0, 0 ), P2( 3, 4 ) ) );
        line.GetWireframeLines( lines, 8 );
        ASSERT_TRUE( 2 == lines.size() && line.GetSupport( V2( 0, 1 ) ) == P2( 3, 4 ) && line.GetSegment( 0 ).end == P2( 3, 0 ) );
        ASSERT_TRUE( 0.0 == CTPolyline2D< Float64 >().GetLength() && CTPolyline2D< Float64 >().GetBounds().IsEmpty() && CTPolyline2D< Float64 >().ClosestPoint( P2( 1, 2 ) ) == P2( 1, 2 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes2D: any empty rectangle is the identity of Merged and ExpandedToInclude" )
    try
    {
        const CTRectangle< Float64 > unit( P2( 0, 0 ), P2( 1, 1 ) );
        const CTRectangle< Float64 > shrunk = unit.Expanded( -2 );
        const CTRectangle< Float64 > far( P2( 10, 10 ), P2( 11, 11 ) );
        ASSERT_TRUE( shrunk.IsEmpty() && shrunk.Merged( far ) == far && far.Merged( shrunk ) == far && !shrunk.Merged( far ).Contains( P2( 5, 5 ) ) );
        ASSERT_TRUE( shrunk.ExpandedToInclude( P2( 3, 4 ) ) == CTRectangle< Float64 >( P2( 3, 4 ), P2( 3, 4 ) ) && CTRectangle< Float64 >().ExpandedToInclude( P2( 3, 4 ) ) == CTRectangle< Float64 >( P2( 3, 4 ), P2( 3, 4 ) ) );
        const CTRectangle< Float64 > none = unit.Intersection( far );
        ASSERT_TRUE( none.IsEmpty() && none.Merged( unit ) == unit && unit.Merged( none ) == unit );
        ASSERT_TRUE( CTRectangle< Float64 >( P2( 0, 0 ), P2( 10, 1 ) ).Expanded( -1 ).IsEmpty() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
