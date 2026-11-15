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
#include <limits>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GEOMETRY_POLYGONTRIANGULATION_H
#include "gucefGEOMETRY_PolygonTriangulation.h"
#define GUCEF_GEOMETRY_POLYGONTRIANGULATION_H
#endif /* GUCEF_GEOMETRY_POLYGONTRIANGULATION_H ? */

#ifndef GUCEF_GEOMETRY_CTGEOMETRY2D_H
#include "gucefGEOMETRY_CTGeometry2D.h"
#define GUCEF_GEOMETRY_CTGEOMETRY2D_H
#endif /* GUCEF_GEOMETRY_CTGEOMETRY2D_H ? */

#ifndef GUCEF_GEOMETRY_CTGEOMETRY3D_H
#include "gucefGEOMETRY_CTGeometry3D.h"
#define GUCEF_GEOMETRY_CTGEOMETRY3D_H
#endif /* GUCEF_GEOMETRY_CTGEOMETRY3D_H ? */

#include "TestConvexReference.h"
#include "TestPolygonReference.h"
#include "TestTriangulation.h"

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

/** A grid polygon: outer ring and holes */
struct SGridPolygon
{
    TGridRing outer;
    TGridRings holes;
};

/*-------------------------------------------------------------------------*/

static TGridRing
Ring( const G* xy, UInt32 count )
{
    TGridRing ring;
    for ( UInt32 i=0; i<count; ++i )
        ring.push_back( CTPoint2D< G >( xy[ 2 * i ], xy[ 2 * i + 1 ] ) );
    return ring;
}

/*-------------------------------------------------------------------------*/

/** The number of points of a ring after merging consecutive duplicates (closing included) */
static UInt32
DistinctCount( const TGridRing& ring )
{
    UInt32 count = 0;
    for ( size_t i=0; i<ring.size(); ++i )
        if ( !( ring[ i ] == ring[ ( i + 1 ) % ring.size() ] ) )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

/**
 *  Triangulates the polygon in kind T and checks: the status, the count n + sum( n_i ) + 2 h - 2 when OK, indices in
 *  range, every triangle counter-clockwise, the exact total area equal to the exact polygon area (fractions), and no
 *  overlap (pairwise exact separation, at most 'maxPairs' triangles).
 */
template < typename T >
static bool
CheckPolygon( const char* name, const SGridPolygon& grid, T ( *make )( G ), TTriangulationStatus expected, const char* context, CTPolygonTriangulator< T >& triangulator, size_t maxPairs = 400 )
{
    GUCEF::vector< CTPoint2D< T > > points = ToKind( grid.outer, make );
    GUCEF::vector< UInt32 > holeCounts;
    UInt32 expectedCount = DistinctCount( grid.outer );
    F area = Abs( RingDoubleArea( &points[ 0 ], (UInt32) points.size() ) );
    for ( size_t h=0; h<grid.holes.size(); ++h )
    {
        const GUCEF::vector< CTPoint2D< T > > hole = ToKind( grid.holes[ h ], make );
        area = area - Abs( RingDoubleArea( &hole[ 0 ], (UInt32) hole.size() ) );
        points.insert( points.end(), hole.begin(), hole.end() );
        holeCounts.push_back( (UInt32) hole.size() );
        expectedCount += DistinctCount( grid.holes[ h ] ) + 2;
    }
    expectedCount -= 2;

    GUCEF::vector< UInt32 > triangles;
    const TTriangulationStatus status = triangulator.Triangulate( &points[ 0 ], (UInt32) grid.outer.size(), holeCounts.empty() ? GUCEF_NULL : &holeCounts[ 0 ], (UInt32) holeCounts.size(), triangles );
    if ( status != expected )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": status " << (Int32) status << " expected " << (Int32) expected );
        return false;
    }
    if ( TRIANGULATION_OK != expected )
        return true;
    for ( size_t i=0; i<triangles.size(); ++i )
        if ( triangles[ i ] >= points.size() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": index out of range" );
            return false;
        }
    if ( triangles.size() != 3 * (size_t) expectedCount || triangulator.GetTriangleCount() != expectedCount )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": " << triangles.size() / 3 << " triangles, expected " << expectedCount );
        return false;
    }
    bool positive = false;
    const F total = TrianglesDoubleArea( &points[ 0 ], triangles, positive );
    if ( !positive )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": a triangle is not counter-clockwise" );
        return false;
    }
    if ( !( total == area ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": total area " << ToFloat64( total ) / 2 << " differs from the polygon area " << ToFloat64( area ) / 2 );
        return false;
    }
    if ( !TrianglesDisjoint( &points[ 0 ], triangles, maxPairs ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": two triangles overlap" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Fixed shapes: convex, concave, comb, clockwise input, collinear and duplicate points, holes (with collinear points) */
template < typename T >
static bool
FixedShapes( const char* name, T ( *make )( G ) )
{
    CTPolygonTriangulator< T > triangulator;
    bool ok = true;
    static const G square[] = { 0, 0, 80, 0, 80, 80, 0, 80 };
    static const G lshape[] = { 0, 0, 80, 0, 80, 24, 24, 24, 24, 80, 0, 80 };
    static const G collinear[] = { 0, 0, 16, 0, 32, 0, 48, 0, 64, 0, 64, 16, 64, 32, 64, 64, 32, 64, 0, 64, 0, 32 };
    static const G duplicates[] = { 0, 0, 0, 0, 64, 0, 64, 0, 64, 0, 64, 64, 0, 64, 0, 0 };
    static const G clockwise[] = { 0, 0, 0, 64, 40, 40, 64, 64, 64, 0 };
    static const G pinchless[] = { 0, 0, 96, 0, 96, 96, 48, 96, 48, 8, 40, 8, 40, 96, 0, 96 };
    SGridPolygon p;
    p.outer = Ring( square, 4 );                                ok = CheckPolygon( name, p, make, TRIANGULATION_OK, "square", triangulator ) && ok;
    p.outer = Ring( lshape, 6 );                                ok = CheckPolygon( name, p, make, TRIANGULATION_OK, "L shape", triangulator ) && ok;
    p.outer = Ring( collinear, 11 );                            ok = CheckPolygon( name, p, make, TRIANGULATION_OK, "collinear points", triangulator ) && ok;
    p.outer = Ring( duplicates, 8 );                            ok = CheckPolygon( name, p, make, TRIANGULATION_OK, "duplicate points", triangulator ) && ok;
    p.outer = Ring( clockwise, 5 );                             ok = CheckPolygon( name, p, make, TRIANGULATION_OK, "clockwise input", triangulator ) && ok;
    p.outer = Ring( pinchless, 8 );                             ok = CheckPolygon( name, p, make, TRIANGULATION_OK, "narrow slot", triangulator ) && ok;

    // a comb: 20 teeth, every inner corner reflex
    p.outer.clear();
    for ( G t=0; t<20; ++t )
    {
        p.outer.push_back( CTPoint2D< G >( 8 * t, 0 ) );
        p.outer.push_back( CTPoint2D< G >( 8 * t + 4, 40 ) );
    }
    p.outer.push_back( CTPoint2D< G >( 160, 0 ) );
    p.outer.push_back( CTPoint2D< G >( 160, -16 ) );
    p.outer.push_back( CTPoint2D< G >( 0, -16 ) );
    ok = CheckPolygon( name, p, make, TRIANGULATION_OK, "comb", triangulator ) && ok;

    // holes: one square hole (counter-clockwise on input, so it is reversed), three holes, a hole with collinear points
    static const G bigSquare[] = { 0, 0, 200, 0, 200, 200, 0, 200 };
    static const G holeA[] = { 20, 20, 60, 20, 60, 60, 20, 60 };
    static const G holeB[] = { 120, 30, 100, 80, 160, 50 };
    static const G holeC[] = { 40, 120, 60, 120, 80, 120, 80, 140, 80, 160, 40, 160 };
    p.outer = Ring( bigSquare, 4 );
    p.holes.clear();
    p.holes.push_back( Ring( holeA, 4 ) );                      ok = CheckPolygon( name, p, make, TRIANGULATION_OK, "one hole", triangulator ) && ok;
    p.holes.push_back( Ring( holeB, 3 ) );
    p.holes.push_back( Ring( holeC, 6 ) );                      ok = CheckPolygon( name, p, make, TRIANGULATION_OK, "three holes", triangulator ) && ok;

    // holes whose rightmost points share the x of an outer vertex and of each other
    static const G holeD[] = { 150, 150, 190, 170, 150, 190 };
    static const G holeE[] = { 150, 100, 190, 110, 150, 130 };
    p.holes.clear();
    p.holes.push_back( Ring( holeD, 3 ) );
    p.holes.push_back( Ring( holeE, 3 ) );                      ok = CheckPolygon( name, p, make, TRIANGULATION_OK, "aligned holes", triangulator ) && ok;

    // the CTPolygon2D and CTPolygonWithHoles2D forms agree with the array form
    CTPolygonWithHoles2D< T > polygon;
    const GUCEF::vector< CTPoint2D< T > > outer = ToKind( Ring( bigSquare, 4 ), make );
    const GUCEF::vector< CTPoint2D< T > > hole = ToKind( Ring( holeA, 4 ), make );
    polygon.outer = CTPolygon2D< T >( &outer[ 0 ], 4 );
    polygon.AddHole( CTPolygon2D< T >( &hole[ 0 ], 4 ) );
    GUCEF::vector< UInt32 > a;
    GUCEF::vector< UInt32 > b;
    const UInt32 holeCount = 4;
    GUCEF::vector< CTPoint2D< T > > all = outer;
    all.insert( all.end(), hole.begin(), hole.end() );
    const bool same = TRIANGULATION_OK == triangulator.Triangulate( polygon, a ) && TRIANGULATION_OK == triangulator.Triangulate( &all[ 0 ], 4, &holeCount, 1, b ) && a == b && 3 * 8 == a.size();
    if ( !same )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the CTPolygonWithHoles2D form differs from the array form" );
    ok = same && ok;
    GUCEF::vector< UInt32 > c;
    ok = TRIANGULATION_OK == TriangulatePolygon( polygon.outer, c ) && 6 == c.size() && ok;
    return ok;
}

/*-------------------------------------------------------------------------*/

/**
 *  Triangulates a polygon whose holes touch the merged ring and checks: status OK, 'expectedCount' triangles (a
 *  zero-length bridge at a shared vertex adds no vertex, one on an edge adds the split vertex), indices in range, every
 *  triangle counter-clockwise, the exact area of the region (outer minus holes, fractions), and no overlap.
 */
template < typename T >
static bool
CheckTouching( const char* name, const SGridPolygon& grid, T ( *make )( G ), UInt32 expectedCount, const char* context, CTPolygonTriangulator< T >& triangulator )
{
    GUCEF::vector< CTPoint2D< T > > points = ToKind( grid.outer, make );
    GUCEF::vector< UInt32 > holeCounts;
    F area = Abs( RingDoubleArea( &points[ 0 ], (UInt32) points.size() ) );
    for ( size_t h=0; h<grid.holes.size(); ++h )
    {
        const GUCEF::vector< CTPoint2D< T > > hole = ToKind( grid.holes[ h ], make );
        area = area - Abs( RingDoubleArea( &hole[ 0 ], (UInt32) hole.size() ) );
        points.insert( points.end(), hole.begin(), hole.end() );
        holeCounts.push_back( (UInt32) hole.size() );
    }
    GUCEF::vector< UInt32 > triangles;
    const TTriangulationStatus status = triangulator.Triangulate( &points[ 0 ], (UInt32) grid.outer.size(), holeCounts.empty() ? GUCEF_NULL : &holeCounts[ 0 ], (UInt32) holeCounts.size(), triangles );
    bool positive = false;
    const F total = TrianglesDoubleArea( &points[ 0 ], triangles, positive );
    bool inRange = true;
    for ( size_t i=0; i<triangles.size(); ++i )
        inRange = inRange && triangles[ i ] < points.size();
    const bool ok = TRIANGULATION_OK == status && inRange && triangles.size() == 3 * (size_t) expectedCount && positive && total == area &&
                    TrianglesDisjoint( &points[ 0 ], triangles, 400 );
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": status " << (Int32) status << ", " << triangles.size() / 3 << " triangles (expected " << expectedCount
                         << "), twice the area " << ToFloat64( total ) << " (expected " << ToFloat64( area ) << ")" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Holes that touch the outer ring or another hole at a vertex, or lie with a vertex on an outer edge */
template < typename T >
static bool
TouchingHoles( const char* name, T ( *make )( G ) )
{
    CTPolygonTriangulator< T > triangulator;
    bool ok = true;
    static const G square[] = { 0, 0, 32, 0, 32, 32, 0, 32 };
    static const G atCorner[] = { 32, 32, 16, 8, 16, 24 };           // the rightmost hole vertex is an outer vertex
    static const G onEdge[] = { 32, 16, 16, 8, 16, 24 };             // the rightmost hole vertex lies on an outer edge
    static const G leftCorner[] = { 0, 0, 16, 24, 24, 8 };          // touching at a vertex that is not the rightmost
    static const G big[] = { 0, 0, 64, 0, 64, 64, 0, 64 };
    static const G first[] = { 64, 32, 40, 16, 40, 48 };            // on the right edge of the outer ring
    static const G second[] = { 40, 16, 16, 8, 16, 24 };            // touches the first hole at ( 40, 16 )
    SGridPolygon p;
    p.outer = Ring( square, 4 );
    p.holes.assign( 1, Ring( atCorner, 3 ) );                    ok = CheckTouching( name, p, make, 5, "hole at an outer vertex", triangulator ) && ok;
    p.holes.assign( 1, Ring( onEdge, 3 ) );                      ok = CheckTouching( name, p, make, 6, "hole on an outer edge", triangulator ) && ok;
    p.holes.assign( 1, Ring( leftCorner, 3 ) );
    {
        // a regular bridge may exist here; then the count is that of a bridged hole
        GUCEF::vector< CTPoint2D< T > > points = ToKind( p.outer, make );
        const GUCEF::vector< CTPoint2D< T > > hole = ToKind( p.holes[ 0 ], make );
        points.insert( points.end(), hole.begin(), hole.end() );
        const UInt32 three = 3;
        GUCEF::vector< UInt32 > triangles;
        triangulator.Triangulate( &points[ 0 ], 4, &three, 1, triangles );
        const UInt32 count = (UInt32) triangles.size() / 3;
        ok = ( 5 == count || 7 == count ) && CheckTouching( name, p, make, count, "hole at an outer vertex that is not its rightmost", triangulator ) && ok;
    }
    p.outer = Ring( big, 4 );
    p.holes.assign( 1, Ring( first, 3 ) );
    p.holes.push_back( Ring( second, 3 ) );
    {
        GUCEF::vector< CTPoint2D< T > > points = ToKind( p.outer, make );
        GUCEF::vector< UInt32 > counts( 2, 3 );
        for ( size_t h=0; h<2; ++h )
        {
            const GUCEF::vector< CTPoint2D< T > > hole = ToKind( p.holes[ h ], make );
            points.insert( points.end(), hole.begin(), hole.end() );
        }
        GUCEF::vector< UInt32 > triangles;
        triangulator.Triangulate( &points[ 0 ], 4, &counts[ 0 ], 2, triangles );
        const UInt32 count = (UInt32) triangles.size() / 3;
        // the first hole joins on the edge (one split vertex: 11 nodes); the second one at the shared vertex or by a bridge (2 more)
        ok = ( 9 == count || 11 == count ) && CheckTouching( name, p, make, count, "a chain of touching holes", triangulator ) && ok;
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Seeded star polygons with and without holes on the grid */
template < typename T >
static bool
RandomPolygons( const char* name, UInt64 seed, UInt32 sets, T ( *make )( G ) )
{
    QueryReference::CRandom random( seed );
    CTPolygonTriangulator< T > triangulator;
    UInt32 checked = 0;
    for ( UInt32 set=0; set<sets; ++set )
    {
        const Int32 nDraw = random.Index( 120 );
        const UInt32 n = 3 + (UInt32) nDraw;
        const Int32 holeDraw = random.Index( 5 );
        const Float64 spiky = random.Uniform( 0.05, 0.9 );
        SGridPolygon p;
        p.outer = StarRing( random, n, 0, 0, 1000000.0 * ( 1.0 - spiky ), 1000000.0 );
        if ( 0 != holeDraw && spiky < 0.5 && n >= 12 )
        {
            // holes in the cells of a 3 x 3 grid inside the inner radius
            for ( Int32 h=0; h<holeDraw; ++h )
            {
                const G cx = ( (G) ( h % 3 ) - 1 ) * 120000;
                const G cy = ( (G) ( h / 3 ) - 1 ) * 120000;
                const Int32 holeN = 3 + random.Index( 12 );
                TGridRing hole = StarRing( random, (UInt32) holeN, cx, cy, 20000.0, 50000.0 );
                std::reverse( hole.begin(), hole.end() );
                p.holes.push_back( hole );
            }
        }
        TGridRings rings( 1, p.outer );
        rings.insert( rings.end(), p.holes.begin(), p.holes.end() );
        if ( !RingsSimpleAndDisjoint( rings ) )
            continue;
        ++checked;
        CORE::CString context = "seed " + CORE::ToString( seed ) + " set " + CORE::ToString( set );
        if ( !CheckPolygon( name, p, make, TRIANGULATION_OK, context.C_String(), triangulator, 200 ) )
        {
            CORE::CString dump;
            for ( size_t r=0; r<rings.size(); ++r )
            {
                dump += "ring:";
                for ( size_t i=0; i<rings[ r ].size(); ++i )
                    dump += " " + CORE::ToString( rings[ r ][ i ].x ) + "," + CORE::ToString( rings[ r ][ i ].y );
                dump += "\n";
            }
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the failing polygon (grid units):\n" << dump );
            return false;
        }
    }
    if ( checked < sets / 2 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": only " << checked << " of " << sets << " generated polygons were simple, seed " << seed );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Degenerate and invalid input */
static bool
DegenerateInput( void )
{
    CTPolygonTriangulator< Float64 > triangulator;
    GUCEF::vector< UInt32 > triangles;
    typedef CTPoint2D< Float64 > P;
    bool ok = true;
    const P two[] = { P( 0, 0 ), P( 1, 0 ) };
    ok = TRIANGULATION_DEGENERATE == triangulator.Triangulate( two, 2, triangles ) && triangles.empty() && ok;
    const P line[] = { P( 0, 0 ), P( 1, 0 ), P( 2, 0 ), P( 3, 0 ) };
    ok = TRIANGULATION_DEGENERATE == triangulator.Triangulate( line, 4, triangles ) && triangles.empty() && ok;
    const P same[] = { P( 1, 1 ), P( 1, 1 ), P( 1, 1 ) };
    ok = TRIANGULATION_DEGENERATE == triangulator.Triangulate( same, 3, triangles ) && ok;
    ok = TRIANGULATION_DEGENERATE == triangulator.Triangulate( GUCEF_NULL, 0, triangles ) && ok;
    ok = TRIANGULATION_INVALID_INPUT == triangulator.Triangulate( GUCEF_NULL, 3, triangles ) && ok;
    const P notFinite[] = { P( 0, 0 ), P( 1, 0 ), P( std::numeric_limits< Float64 >::quiet_NaN(), 1 ) };
    ok = TRIANGULATION_INVALID_INPUT == triangulator.Triangulate( notFinite, 3, triangles ) && triangles.empty() && ok;
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Triangulation: degenerate or invalid input not reported" );

    // a bow tie is not simple: best effort triangles and status NOT_SIMPLE, without hanging
    const P bowtie[] = { P( 0, 0 ), P( 2, 2 ), P( 2, 0 ), P( 0, 2 ) };
    const bool bowtieOk = TRIANGULATION_NOT_SIMPLE == triangulator.Triangulate( bowtie, 4, triangles ) && triangles.size() <= 6;
    if ( !bowtieOk )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Triangulation: the bow tie gave status " << (Int32) triangulator.Triangulate( bowtie, 4, triangles ) );
    ok = bowtieOk && ok;

    // a hole outside the outer ring has no bridge: left out, NOT_SIMPLE; a zero-area hole is ignored (OK)
    const P withHoles[] = { P( 0, 0 ), P( 4, 0 ), P( 4, 4 ), P( 0, 4 ), P( 10, 10 ), P( 11, 10 ), P( 10, 11 ) };
    const UInt32 holeCount = 3;
    const bool outside = TRIANGULATION_NOT_SIMPLE == triangulator.Triangulate( withHoles, 4, &holeCount, 1, triangles ) && 6 == triangles.size();
    const P flatHole[] = { P( 0, 0 ), P( 4, 0 ), P( 4, 4 ), P( 0, 4 ), P( 1, 1 ), P( 2, 2 ), P( 3, 3 ) };
    const bool flat = TRIANGULATION_OK == triangulator.Triangulate( flatHole, 4, &holeCount, 1, triangles ) && 6 == triangles.size();
    if ( !outside || !flat )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Triangulation: a hole outside (" << outside << ") or a hole without area (" << flat << ") was not handled" );
    ok = outside && flat && ok;

    // a spike (zero width fold back) on an otherwise simple ring
    const P spike[] = { P( 0, 0 ), P( 4, 0 ), P( 4, 2 ), P( 8, 2 ), P( 4, 2 ), P( 4, 4 ), P( 0, 4 ) };
    const TTriangulationStatus spikeStatus = triangulator.Triangulate( spike, 7, triangles );
    bool spikeAreaOk = false;
    const F spikeArea = TrianglesDoubleArea( spike, triangles, spikeAreaOk );
    const bool spikeOk = spikeAreaOk && spikeArea == F( 32, 1 );
    if ( !spikeOk )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Triangulation: the spike gave status " << (Int32) spikeStatus << " and twice the area " << ToFloat64( spikeArea ) );
    ok = spikeOk && ok;
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Timing of large star polygons (reported) and their checks without the quadratic overlap test */
static bool
LargePolygons( UInt64 seed )
{
    QueryReference::CRandom random( seed );
    CTPolygonTriangulator< Float64 > triangulator;
    const UInt32 sizes[] = { 1000, 10000 };
    bool ok = true;
    for ( UInt32 s=0; s<2; ++s )
    {
        const UInt32 n = ScaledCount( sizes[ s ] );
        SGridPolygon p;
        p.outer = StarRing( random, n, 0, 0, 600000000.0, 1000000000.0 );
        const GUCEF::vector< CTPoint2D< Float64 > > points = ToKind( p.outer, MakeFloat64 );
        GUCEF::vector< UInt32 > triangles;
        const Float64 start = NowMs();
        const TTriangulationStatus status = triangulator.Triangulate( &points[ 0 ], n, triangles );
        const Float64 elapsed = NowMs() - start;
        bool positive = false;
        const F total = TrianglesDoubleArea( &points[ 0 ], triangles, positive );
        const bool good = TRIANGULATION_OK == status && positive && total == Abs( RingDoubleArea( &points[ 0 ], n ) ) && triangles.size() == 3 * (size_t) ( n - 2 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Triangulation: ear clipping of a star polygon of " << n << " vertices took " << elapsed << " ms, seed " << seed );
        if ( !good )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Triangulation: the large polygon of " << n << " vertices failed, status " << (Int32) status << ", seed " << seed );
        ok = good && ok;
    }
    return ok;
}


/*-------------------------------------------------------------------------*/

/** Every entry point of the 2D and 3D facades on small cases with known answers */
static bool
Facades( void )
{
    typedef CTGeometry2D< Float64 > G2;
    typedef CTGeometry3D< Float64 > G3;
    typedef CTPoint2D< Float64 > P2;
    typedef CTPoint3D< Float64 > P3;
    bool ok = true;
    int check = 0;
#define FACADE_CHECK( condition ) ++check; if ( !( condition ) ) { GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Facades: check " << check << " failed: " #condition ); ok = false; }

    const P2 cloud[] = { P2( 0, 0 ), P2( 4, 0 ), P2( 4, 4 ), P2( 0, 4 ), P2( 2, 2 ), P2( 2, 0 ) };
    GUCEF::vector< P2 > hull;
    FACADE_CHECK( 4 == G2::ConvexHull( cloud, 6, hull ) && 4 == hull.size() );
    FACADE_CHECK( 5 == G2::ConvexHull( cloud, 6, hull, true ) );
    CTPolygon2D< Float64 > square( cloud, 4 );
    CTPolygonWithHoles2D< Float64 > holed( square );
    const P2 holeRing[] = { P2( 1, 1 ), P2( 1, 3 ), P2( 3, 3 ), P2( 3, 1 ) };
    holed.AddHole( CTPolygon2D< Float64 >( holeRing, 4 ) );
    FACADE_CHECK( POINTINPOLYGON_INSIDE == G2::ClassifyPoint( square, P2( 2, 2 ) ) );
    FACADE_CHECK( POINTINPOLYGON_OUTSIDE == G2::ClassifyPoint( holed, P2( 2, 2 ) ) );
    FACADE_CHECK( G2::Contains( holed, P2( 1, 2 ) ) && !G2::Contains( holed, P2( 5, 2 ) ) );
    GUCEF::vector< UInt32 > triangles;
    FACADE_CHECK( TRIANGULATION_OK == G2::Triangulate( square, triangles ) && 6 == triangles.size() );
    FACADE_CHECK( TRIANGULATION_OK == G2::Triangulate( holed, triangles ) && 24 == triangles.size() );
    GUCEF::vector< CTTriangle2D< Float64 > > shapes;
    CTPolygonTriangulator< Float64 > triangulator;
    FACADE_CHECK( TRIANGULATION_OK == TriangulatePolygon( holed, shapes, triangulator, triangles ) && 8 == shapes.size() );
    Float64 shapeArea = 0.0;
    for ( size_t i=0; i<shapes.size(); ++i )
        shapeArea += 0.5 * ( ( shapes[ i ].b - shapes[ i ].a ).Cross( shapes[ i ].c - shapes[ i ].a ) );
    FACADE_CHECK( 12.0 == shapeArea );
    GUCEF::vector< P2 > vertices;
    const UInt32 constraint[] = { 0, 2 };
    FACADE_CHECK( DELAUNAY_OK == G2::Delaunay( cloud, 6, triangles, constraint, 1, &vertices ) && 6 == vertices.size() && 15 == triangles.size() );
    GUCEF::vector< CTPolygon2D< Float64 > > pieces;
    const P2 lshape[] = { P2( 0, 0 ), P2( 4, 0 ), P2( 4, 1 ), P2( 1, 1 ), P2( 1, 4 ), P2( 0, 4 ) };
    FACADE_CHECK( TRIANGULATION_OK == G2::DecomposeIntoConvex( CTPolygon2D< Float64 >( lshape, 6 ), pieces ) && 2 == pieces.size() );
    FACADE_CHECK( TRIANGULATION_OK == G2::DecomposeIntoConvex( holed, pieces ) && pieces.size() >= 4 );
    const P2 wiggle[] = { P2( 0, 0 ), P2( 1, 0.001 ), P2( 2, 0 ), P2( 3, 5 ) };
    CTPolyline2D< Float64 > line( wiggle, 4 );
    CTPolyline2D< Float64 > simpler;
    G2::Simplify( line, 0.01, simpler );
    FACADE_CHECK( 3 == simpler.points.size() );
    const P2 ringWiggle[] = { P2( 0, 0 ), P2( 2, 0.001 ), P2( 4, 0 ), P2( 4, 4 ), P2( 0, 4 ) };
    CTPolygon2D< Float64 > simplerRing;
    G2::Simplify( CTPolygon2D< Float64 >( ringWiggle, 5 ), 0.01, simplerRing );
    FACADE_CHECK( 4 == simplerRing.points.size() );
    const P2 shifted[] = { P2( 2, 2 ), P2( 6, 2 ), P2( 6, 6 ), P2( 2, 6 ) };
    const CTPolygonWithHoles2D< Float64 > a( square ), b( CTPolygon2D< Float64 >( shifted, 4 ) );
    GUCEF::vector< CTPolygonWithHoles2D< Float64 > > result;
    FACADE_CHECK( POLYGONCLIPPER_OK == G2::Union( a, b, result ) && 1 == result.size() && 28.0 == result[ 0 ].GetArea() );
    FACADE_CHECK( POLYGONCLIPPER_OK == G2::Intersection( a, b, result ) && 1 == result.size() && 4.0 == result[ 0 ].GetArea() );
    FACADE_CHECK( POLYGONCLIPPER_OK == G2::Difference( a, b, result ) && 1 == result.size() && 12.0 == result[ 0 ].GetArea() );
    FACADE_CHECK( POLYGONCLIPPER_OK == G2::Xor( a, b, result ) && 2 == result.size() && 24.0 == result[ 0 ].GetArea() + result[ 1 ].GetArea() );
    FACADE_CHECK( POLYGONCLIPPER_OK == G2::Boolean( a, b, POLYGONBOOLEAN_UNION, result ) && 1 == result.size() );
    FACADE_CHECK( POLYGONCLIPPER_OK == G2::Offset( a, 1.0, POLYGONJOIN_MITER, result ) && 1 == result.size() && 36.0 == result[ 0 ].GetArea() );
    FACADE_CHECK( POLYGONCLIPPER_OK == G2::Stroke( CTPolyline2D< Float64 >( cloud, 2 ), 1.0, POLYGONJOIN_ROUND, POLYLINEEND_BUTT, result ) && 1 == result.size() && 8.0 == result[ 0 ].GetArea() );
    SSegmentIntersection2D< Float64 > crossing;
    FACADE_CHECK( SEGMENTINTERSECTION_POINT == G2::IntersectSegments( CTSegment2D< Float64 >( P2( 0, 0 ), P2( 2, 2 ) ), CTSegment2D< Float64 >( P2( 0, 2 ), P2( 2, 0 ) ), crossing ) && crossing.first == P2( 1, 1 ) );

    const P3 cube[] = { P3( 0, 0, 0 ), P3( 1, 0, 0 ), P3( 1, 1, 0 ), P3( 0, 1, 0 ), P3( 0, 0, 1 ), P3( 1, 0, 1 ), P3( 1, 1, 1 ), P3( 0, 1, 1 ), P3( 0.5, 0.5, 0.5 ) };
    CTConvexHull3D< Float64 > hull3;
    FACADE_CHECK( QUICKHULL_OK == G3::ConvexHull( cube, 9, hull3 ) && 8 == hull3.GetVertexCount() && 6 == hull3.GetFaceCount() );
    const CTTriangle3D< Float64 > triangle( P3( 0, 0, 0 ), P3( 2, 0, 0 ), P3( 0, 2, 0 ) );
    FACADE_CHECK( G3::ClosestPointOnTriangle( triangle, P3( 0.5, 0.5, 3 ) ) == P3( 0.5, 0.5, 0 ) );
    const CTSegment3D< Float64 > segment( P3( 0, 0, 1 ), P3( 2, 0, 1 ) );
    FACADE_CHECK( G3::ClosestPointOnSegment( segment, P3( 1, 5, 1 ) ) == P3( 1, 0, 1 ) );
    P3 onA, onB;
    G3::ClosestPoints( segment, CTSegment3D< Float64 >( P3( 1, -1, 3 ), P3( 1, 1, 3 ) ), onA, onB );
    FACADE_CHECK( onA == P3( 1, 0, 1 ) && onB == P3( 1, 0, 3 ) );
    G3::ClosestPoints( segment, triangle, onA, onB );
    FACADE_CHECK( onA.z == 1.0 && onB.z == 0.0 && onA.y == 0.0 );
    G3::ClosestPoints( triangle, CTTriangle3D< Float64 >( P3( 0, 0, 2 ), P3( 1, 0, 2 ), P3( 0, 1, 2 ) ), onA, onB );
    FACADE_CHECK( onB.z - onA.z == 2.0 );
    FACADE_CHECK( 2.0 == G3::TriangleArea( triangle ) );
    CTDirectionVector3D< Float64 > normal;
    FACADE_CHECK( G3::TryGetTriangleNormal( triangle, normal ) && normal.GetZ() == 1.0 );
    CTVector3D< Float64 > weights;
    FACADE_CHECK( G3::TryGetBarycentric( triangle, P3( 0, 0, 0 ), weights ) && weights.x == 1.0 );
    FACADE_CHECK( G3::SegmentIntersectsTriangle( CTSegment3D< Float64 >( P3( 0.5, 0.5, -1 ), P3( 0.5, 0.5, 1 ) ), triangle ) );
    FACADE_CHECK( !G3::SegmentsIntersect( segment, CTSegment3D< Float64 >( P3( 1, -1, 3 ), P3( 1, 1, 3 ) ) ) );
    Float64 t = 0.0;
    FACADE_CHECK( G3::IntersectSegmentWithPlane( CTSegment3D< Float64 >( P3( 0, 0, -1 ), P3( 0, 0, 3 ) ), CTPlane< Float64 >::FromPointNormal( P3( 0, 0, 0 ), CTDirectionVector3D< Float64 >::UnitZ() ), t ) && 0.25 == t );
    const P3 line3[] = { P3( 0, 0, 0 ), P3( 1, 0, 0.001 ), P3( 2, 0, 0 ) };
    CTPolyline3D< Float64 > simpler3;
    G3::Simplify( CTPolyline3D< Float64 >( line3, 3 ), 0.01, simpler3 );
    FACADE_CHECK( 2 == simpler3.points.size() );
#undef FACADE_CHECK
    return ok;
}

/*-------------------------------------------------------------------------*/

void
PerformTriangulationTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Triangulation" );

    GUCEF_TESTFW_TESTCASE( "Triangulation: fixed shapes with holes, collinear and duplicate points: n - 2 + 2h counter-clockwise triangles, exact area, no overlap, in every kind" )
    try
    {
        ASSERT_TRUE( ( FixedShapes< Int32 >( "Int32", MakeInt32 ) ) );
        ASSERT_TRUE( ( FixedShapes< Int64 >( "Int64", MakeInt64 ) ) );
        ASSERT_TRUE( ( FixedShapes< Float64 >( "Float64", MakeFloat64 ) ) );
        ASSERT_TRUE( ( FixedShapes< Float32 >( "Float32", MakeFloat32 ) ) );
        ASSERT_TRUE( ( FixedShapes< TFixed32_32 >( "TFixed32_32", MakeFixed ) ) );
        ASSERT_TRUE( ( FixedShapes< F >( "TMathBigFraction", MakeFraction ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Triangulation: seeded simple star polygons with holes: n - 2 + 2h counter-clockwise triangles, exact area, no overlap" )
    try
    {
        ASSERT_TRUE( ( RandomPolygons< Int32 >( "Int32", 7101, ScaledCount( 400 ), MakeInt32 ) ) );
        ASSERT_TRUE( ( RandomPolygons< Float64 >( "Float64", 7102, ScaledCount( 400 ), MakeFloat64 ) ) );
        ASSERT_TRUE( ( RandomPolygons< Float32 >( "Float32", 7103, ScaledCount( 200 ), MakeFloat32 ) ) );
        ASSERT_TRUE( ( RandomPolygons< TFixed32_32 >( "TFixed32_32", 7104, ScaledCount( 200 ), MakeFixed ) ) );
        ASSERT_TRUE( ( RandomPolygons< F >( "TMathBigFraction", 7105, ScaledCount( 40 ), MakeFraction ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Triangulation: holes touching the outer ring or each other at a vertex or on an edge are joined there: status OK, exact area, no overlap, in every kind" )
    try
    {
        ASSERT_TRUE( ( TouchingHoles< Int32 >( "Int32", MakeInt32 ) ) );
        ASSERT_TRUE( ( TouchingHoles< Float64 >( "Float64", MakeFloat64 ) ) );
        ASSERT_TRUE( ( TouchingHoles< Float32 >( "Float32", MakeFloat32 ) ) );
        ASSERT_TRUE( ( TouchingHoles< TFixed32_32 >( "TFixed32_32", MakeFixed ) ) );
        ASSERT_TRUE( ( TouchingHoles< F >( "TMathBigFraction", MakeFraction ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Triangulation: degenerate, invalid and non-simple input is reported; holes outside or without area" )
    try
    {
        ASSERT_TRUE( DegenerateInput() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Triangulation: the CTGeometry2D and CTGeometry3D facades answer known cases" )
    try
    {
        ASSERT_TRUE( Facades() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Triangulation: star polygons of 10^3 and 10^4 vertices (timing reported)" )
    try
    {
        ASSERT_TRUE( LargePolygons( 7106 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
