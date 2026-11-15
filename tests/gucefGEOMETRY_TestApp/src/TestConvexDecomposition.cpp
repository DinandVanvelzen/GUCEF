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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GEOMETRY_CONVEXDECOMPOSITION2D_H
#include "gucefGEOMETRY_ConvexDecomposition2D.h"
#define GUCEF_GEOMETRY_CONVEXDECOMPOSITION2D_H
#endif /* GUCEF_GEOMETRY_CONVEXDECOMPOSITION2D_H ? */

#include "TestConvexReference.h"
#include "TestPolygonReference.h"
#include "TestConvexDecomposition.h"

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

/** True when every other vertex of the ring lies left of or on every edge (a convex counter-clockwise ring) and the area is positive */
template < typename T >
static bool
IsConvexPiece( const CTPoint2D< T >* ring, UInt32 count )
{
    if ( count < 3 || SpatialReference::SignOfFraction( RingDoubleArea( ring, count ) ) <= 0 )
        return false;
    for ( UInt32 i=0; i<count; ++i )
        for ( UInt32 k=0; k<count; ++k )
            if ( SpatialReference::SignOfFraction( DoubleArea( ring[ i ], ring[ ( i + 1 ) % count ], ring[ k ] ) ) < 0 )
                return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Decomposes the rings (outer first, then holes) in kind T and checks: every piece convex and counter-clockwise, the
 *  exact total area equal to the region area, the pieces disjoint (fan triangles compared pairwise), and the count
 *  bounds: at least ceil( r / 2 ) + 1 - h and, without holes, at most min( n - 2, 2 r + 1 ).
 */
template < typename T >
static bool
CheckDecomposition( const char* name, const TGridRings& rings, T ( *make )( G ), const char* context, CTConvexDecomposition2DWorkspace< T >& workspace )
{
    GUCEF::vector< CTPoint2D< T > > points = ToKind( rings[ 0 ], make );
    const UInt32 n = (UInt32) points.size();
    GUCEF::vector< UInt32 > holeCounts;
    F area = Abs( RingDoubleArea( &points[ 0 ], n ) );
    // reflex vertices of the region: those of the counter-clockwise outer ring and the convex ones of the clockwise holes
    UInt32 reflex = 0;
    for ( UInt32 i=0; i<n; ++i )
        if ( SpatialReference::SignOfFraction( DoubleArea( points[ ( i + n - 1 ) % n ], points[ i ], points[ ( i + 1 ) % n ] ) ) < 0 )
            ++reflex;
    for ( size_t h=1; h<rings.size(); ++h )
    {
        const GUCEF::vector< CTPoint2D< T > > hole = ToKind( rings[ h ], make );
        const UInt32 m = (UInt32) hole.size();
        area = area - Abs( RingDoubleArea( &hole[ 0 ], m ) );
        const Int32 orientation = SpatialReference::SignOfFraction( RingDoubleArea( &hole[ 0 ], m ) );
        for ( UInt32 i=0; i<m; ++i )
            if ( SpatialReference::SignOfFraction( DoubleArea( hole[ ( i + m - 1 ) % m ], hole[ i ], hole[ ( i + 1 ) % m ] ) ) == orientation )
                ++reflex;
        points.insert( points.end(), hole.begin(), hole.end() );
        holeCounts.push_back( m );
    }
    const UInt32 holeCount = (UInt32) holeCounts.size();
    GUCEF::vector< UInt32 > indices;
    GUCEF::vector< UInt32 > sizes;
    const TTriangulationStatus status = DecomposeIntoConvex( &points[ 0 ], n, holeCounts.empty() ? GUCEF_NULL : &holeCounts[ 0 ], holeCount, indices, sizes, workspace );
    if ( TRIANGULATION_OK != status )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": status " << (Int32) status );
        return false;
    }
    F total( 0, 1 );
    GUCEF::vector< UInt32 > fan;
    size_t offset = 0;
    for ( size_t p=0; p<sizes.size(); ++p )
    {
        GUCEF::vector< CTPoint2D< T > > piece;
        for ( UInt32 i=0; i<sizes[ p ]; ++i )
            piece.push_back( points[ indices[ offset + i ] ] );
        if ( !IsConvexPiece( &piece[ 0 ], sizes[ p ] ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": piece " << p << " of " << sizes[ p ] << " points is not convex" );
            return false;
        }
        total = total + RingDoubleArea( &piece[ 0 ], sizes[ p ] );
        for ( UInt32 i=1; i+1<sizes[ p ]; ++i )
        {
            fan.push_back( indices[ offset ] );
            fan.push_back( indices[ offset + i ] );
            fan.push_back( indices[ offset + i + 1 ] );
        }
        offset += sizes[ p ];
    }
    if ( !( total == area ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": total area " << ToFloat64( total ) / 2 << " differs from " << ToFloat64( area ) / 2 );
        return false;
    }
    // a convex piece may hold straight-on vertices: its fan can have triangles without area, which the overlap test ignores
    GUCEF::vector< UInt32 > properFan;
    for ( size_t t=0; t<fan.size(); t+=3 )
        if ( 0 != SpatialReference::SignOfFraction( DoubleArea( points[ fan[ t ] ], points[ fan[ t + 1 ] ], points[ fan[ t + 2 ] ] ) ) )
            properFan.insert( properFan.end(), fan.begin() + t, fan.begin() + t + 3 );
    if ( !TrianglesDisjoint( &points[ 0 ], properFan, 300 ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": pieces overlap" );
        return false;
    }
    const UInt32 pieces = (UInt32) sizes.size();
    const Int32 lower = (Int32) ( ( reflex + 1 ) / 2 ) + 1 - (Int32) holeCount;
    const bool lowerOk = (Int32) pieces >= lower;
    const bool upperOk = 0 != holeCount || ( pieces <= n - 2 && pieces <= 2 * reflex + 1 );
    if ( !lowerOk || !upperOk )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": " << pieces << " pieces for n " << n << " and " << reflex << " reflex vertices are outside the bounds" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static TGridRing
GridRing( const G* xy, UInt32 count )
{
    TGridRing ring;
    for ( UInt32 i=0; i<count; ++i )
        ring.push_back( CTPoint2D< G >( xy[ 2 * i ], xy[ 2 * i + 1 ] ) );
    return ring;
}

/*-------------------------------------------------------------------------*/

/** Fixed shapes: a convex polygon is one piece, an L two, a comb its teeth, a square with a hole, collinear points */
template < typename T >
static bool
FixedShapes( const char* name, T ( *make )( G ) )
{
    CTConvexDecomposition2DWorkspace< T > workspace;
    bool ok = true;
    static const G hexagon[] = { 0, 0, 40, -8, 80, 0, 88, 40, 40, 72, -8, 40 };
    static const G lshape[] = { 0, 0, 80, 0, 80, 24, 24, 24, 24, 80, 0, 80 };
    static const G collinear[] = { 0, 0, 16, 0, 32, 0, 48, 0, 64, 0, 64, 32, 64, 64, 32, 64, 0, 64, 0, 32 };
    TGridRings rings( 1 );
    rings[ 0 ] = GridRing( hexagon, 6 );        ok = CheckDecomposition( name, rings, make, "hexagon", workspace ) && ok;
    GUCEF::vector< UInt32 > indices;
    GUCEF::vector< UInt32 > sizes;
    const GUCEF::vector< CTPoint2D< T > > hex = ToKind( rings[ 0 ], make );
    ok = TRIANGULATION_OK == DecomposeIntoConvex( &hex[ 0 ], 6, indices, sizes ) && 1 == sizes.size() && 6 == sizes[ 0 ] && ok;
    rings[ 0 ] = GridRing( lshape, 6 );         ok = CheckDecomposition( name, rings, make, "L shape", workspace ) && ok;
    const GUCEF::vector< CTPoint2D< T > > l = ToKind( rings[ 0 ], make );
    ok = TRIANGULATION_OK == DecomposeIntoConvex( &l[ 0 ], 6, indices, sizes ) && 2 == sizes.size() && ok;
    rings[ 0 ] = GridRing( collinear, 10 );     ok = CheckDecomposition( name, rings, make, "collinear square", workspace ) && ok;
    const GUCEF::vector< CTPoint2D< T > > sq = ToKind( rings[ 0 ], make );
    ok = TRIANGULATION_OK == DecomposeIntoConvex( &sq[ 0 ], 10, indices, sizes ) && 1 == sizes.size() && 10 == sizes[ 0 ] && ok;
    rings[ 0 ].clear();
    for ( G t=0; t<12; ++t )
    {
        rings[ 0 ].push_back( CTPoint2D< G >( 8 * t, 0 ) );
        rings[ 0 ].push_back( CTPoint2D< G >( 8 * t + 4, 40 ) );
    }
    rings[ 0 ].push_back( CTPoint2D< G >( 96, 0 ) );
    rings[ 0 ].push_back( CTPoint2D< G >( 96, -16 ) );
    rings[ 0 ].push_back( CTPoint2D< G >( 0, -16 ) );
    ok = CheckDecomposition( name, rings, make, "comb", workspace ) && ok;
    static const G square[] = { 0, 0, 200, 0, 200, 200, 0, 200 };
    static const G hole[] = { 60, 60, 60, 140, 140, 140, 140, 60 };
    rings[ 0 ] = GridRing( square, 4 );
    rings.push_back( GridRing( hole, 4 ) );
    ok = CheckDecomposition( name, rings, make, "square with a hole", workspace ) && ok;

    // holes that touch the outer ring at a vertex and on an edge (joined there by the triangulation)
    static const G touchSquare[] = { 0, 0, 32, 0, 32, 32, 0, 32 };
    static const G atCorner[] = { 32, 32, 16, 8, 16, 24 };
    static const G onEdge[] = { 32, 16, 16, 8, 16, 24 };
    TGridRings touching( 2 );
    touching[ 0 ] = GridRing( touchSquare, 4 );
    touching[ 1 ] = GridRing( atCorner, 3 );     ok = CheckDecomposition( name, touching, make, "hole at an outer vertex", workspace ) && ok;
    touching[ 1 ] = GridRing( onEdge, 3 );       ok = CheckDecomposition( name, touching, make, "hole on an outer edge", workspace ) && ok;

    // the polygon forms
    CTPolygonWithHoles2D< T > polygon( CTPolygon2D< T >( &ToKind( rings[ 0 ], make )[ 0 ], 4 ) );
    const GUCEF::vector< CTPoint2D< T > > h = ToKind( rings[ 1 ], make );
    polygon.AddHole( CTPolygon2D< T >( &h[ 0 ], 4 ) );
    GUCEF::vector< CTPolygon2D< T > > pieces;
    const bool formOk = TRIANGULATION_OK == DecomposeIntoConvex( polygon, pieces, workspace ) && pieces.size() >= 3 && pieces.size() <= 8;
    if ( !formOk )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the polygon with holes form gave " << pieces.size() << " pieces" );
    ok = formOk && ok;
    GUCEF::vector< CTPolygon2D< T > > lPieces;
    ok = TRIANGULATION_OK == DecomposeIntoConvex( CTPolygon2D< T >( &l[ 0 ], 6 ), lPieces ) && 2 == lPieces.size() && ok;
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Seeded star polygons (many reflex vertices), some with holes */
template < typename T >
static bool
RandomPolygons( const char* name, UInt64 seed, UInt32 sets, T ( *make )( G ) )
{
    QueryReference::CRandom random( seed );
    CTConvexDecomposition2DWorkspace< T > workspace;
    for ( UInt32 set=0; set<sets; ++set )
    {
        const Int32 nDraw = random.Index( 80 );
        const UInt32 n = 3 + (UInt32) nDraw;
        const Float64 spiky = random.Uniform( 0.05, 0.9 );
        const Int32 holeDraw = random.Index( 4 );
        TGridRings rings( 1, StarRing( random, n, 0, 0, 1000000.0 * ( 1.0 - spiky ), 1000000.0 ) );
        if ( n >= 12 && spiky < 0.5 )
        {
            for ( Int32 h=0; h<holeDraw; ++h )
            {
                const Int32 holeN = 3 + random.Index( 8 );
                TGridRing hole = StarRing( random, (UInt32) holeN, ( (G) h - 1 ) * 120000, 0, 20000.0, 50000.0 );
                std::reverse( hole.begin(), hole.end() );
                rings.push_back( hole );
            }
        }
        if ( !RingsSimpleAndDisjoint( rings ) )
            continue;
        const CORE::CString context = "seed " + CORE::ToString( seed ) + " set " + CORE::ToString( set );
        if ( !CheckDecomposition( name, rings, make, context.C_String(), workspace ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
PerformConvexDecompositionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "ConvexDecomposition" );

    GUCEF_TESTFW_TESTCASE( "ConvexDecomposition: fixed shapes: convex pieces, exact total area, counts ceil( r / 2 ) + 1 to min( n - 2, 2 r + 1 ), holes" )
    try
    {
        ASSERT_TRUE( ( FixedShapes< Int32 >( "Int32", MakeInt32 ) ) );
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

    GUCEF_TESTFW_TESTCASE( "ConvexDecomposition: seeded star polygons with holes: convex disjoint pieces, exact total area, count bounds" )
    try
    {
        ASSERT_TRUE( ( RandomPolygons< Int32 >( "Int32", 7501, ScaledCount( 300 ), MakeInt32 ) ) );
        ASSERT_TRUE( ( RandomPolygons< Float64 >( "Float64", 7502, ScaledCount( 300 ), MakeFloat64 ) ) );
        ASSERT_TRUE( ( RandomPolygons< TFixed32_32 >( "TFixed32_32", 7503, ScaledCount( 150 ), MakeFixed ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
