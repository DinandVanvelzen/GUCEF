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
#include "TestIntegerRanges.h"

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

typedef TInt32Point2D P2;
typedef TInt32Vector2D V2;
typedef TInt32Point3D P3;
typedef TInt32Vector3D V3;

/** The cells around the origin that the brute-force references enumerate: coordinates -GRID .. GRID */
#define GRID 24
#define GRIDSIZE ( 2 * GRID + 1 )

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CTIntegerRect< Int32 >
Rect( Int32 x, Int32 y, Int32 w, Int32 h )
{
    return CTIntegerRect< Int32 >( x, y, w, h );
}

/*-------------------------------------------------------------------------*/

/** Marks the cells of a rectangle that lie in the reference grid, by the half-open definition */
static void
MarkCells( const CTIntegerRect< Int32 >& rect, bool ( &grid )[ GRIDSIZE ][ GRIDSIZE ] )
{
    memset( grid, 0, sizeof( grid ) );
    for ( Int32 y=-GRID; y<=GRID; ++y )
    {
        for ( Int32 x=-GRID; x<=GRID; ++x )
            grid[ y + GRID ][ x + GRID ] = x >= rect.position.x && x < rect.position.x + rect.size.x && y >= rect.position.y && y < rect.position.y + rect.size.y;
    }
}

/*-------------------------------------------------------------------------*/

static UInt32
CountMarked( const bool ( &grid )[ GRIDSIZE ][ GRIDSIZE ] )
{
    UInt32 count = 0;
    for ( UInt32 y=0; y<GRIDSIZE; ++y )
        for ( UInt32 x=0; x<GRIDSIZE; ++x )
            count += grid[ y ][ x ] ? 1 : 0;
    return count;
}

/*-------------------------------------------------------------------------*/

static bool
SameCells( const CTIntegerRect< Int32 >& rect, const bool ( &expected )[ GRIDSIZE ][ GRIDSIZE ] )
{
    bool grid[ GRIDSIZE ][ GRIDSIZE ];
    MarkCells( rect, grid );
    return 0 == memcmp( grid, expected, sizeof( grid ) );
}

/*-------------------------------------------------------------------------*/

static bool
Is3DCell( const CTIntegerBox3D< Int32 >& box, Int32 x, Int32 y, Int32 z )
{
    return x >= box.position.x && x < box.position.x + box.size.x && y >= box.position.y && y < box.position.y + box.size.y && z >= box.position.z && z < box.position.z + box.size.z;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformIntegerRangesTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "IntegerRanges" );

    GUCEF_TESTFW_TESTCASE( "IntegerRanges: half-open containment, intersection, union and cell iteration" )
    try
    {
        const CTIntegerRect< Int32 > r( P2( 0, 0 ), V2( 4, 3 ) );
        ASSERT_TRUE( r.Contains( P2( 3, 2 ) ) && !r.Contains( P2( 4, 2 ) ) && !r.Contains( P2( 3, 3 ) ) && r.Contains( P2( 0, 0 ) ) && !r.Contains( P2( -1, 0 ) ) && !r.Contains( P2( 0, -1 ) ) );
        ASSERT_TRUE( 12 == r.GetCellCount() && 12 == r.GetArea() && !r.IsEmpty() && r.GetMin() == P2( 0, 0 ) && r.GetEnd() == P2( 4, 3 ) );

        // iteration: row by row, x fastest
        UInt32 seen = 0;
        for ( UInt64 i=0; i<r.GetCellCount(); ++i )
        {
            const P2 cell = r.GetCell( i );
            ASSERT_TRUE( cell.x == (Int32) ( i % 4 ) && cell.y == (Int32) ( i / 4 ) && r.Contains( cell ) );
            ++seen;
        }
        ASSERT_TRUE( 12 == seen && r.GetCell( 12 ) == r.position && r.GetCell( 1000 ) == r.position );
        const CTIntegerRect< Int32 > offset = Rect( -3, 5, 2, 2 );
        ASSERT_TRUE( offset.GetCell( 0 ) == P2( -3, 5 ) && offset.GetCell( 1 ) == P2( -2, 5 ) && offset.GetCell( 2 ) == P2( -3, 6 ) && offset.GetCell( 3 ) == P2( -2, 6 ) );

        const CTIntegerRect< Int32 > other = Rect( 2, 1, 5, 5 );
        ASSERT_TRUE( r.Intersects( other ) && r.Intersection( other ) == Rect( 2, 1, 2, 2 ) && r.Clip( other ) == Rect( 2, 1, 2, 2 ) );
        ASSERT_TRUE( r.Union( other ) == Rect( 0, 0, 7, 6 ) && other.Union( r ) == Rect( 0, 0, 7, 6 ) );
        ASSERT_FALSE( r.Intersects( Rect( 4, 0, 2, 2 ) ) );                 // adjacent rectangles share no cell
        ASSERT_TRUE( r.Intersection( Rect( 4, 0, 2, 2 ) ).IsEmpty() && 0 == r.Intersection( Rect( 4, 0, 2, 2 ) ).GetCellCount() );
        ASSERT_TRUE( r.Contains( Rect( 1, 1, 3, 2 ) ) && !r.Contains( Rect( 1, 1, 4, 2 ) ) && r.Contains( r ) );
        ASSERT_TRUE( r.Expanded( 1 ) == Rect( -1, -1, 6, 5 ) && r.Expanded( -1 ) == Rect( 1, 1, 2, 1 ) && r.Expanded( -2 ).IsEmpty() && r.Expanded( 0 ) == r );
        ASSERT_TRUE( r.Translated( V2( 10, -2 ) ) == Rect( 10, -2, 4, 3 ) );

        // empty rectangles: no cells, identity of Union, inside everything
        const CTIntegerRect< Int32 > empty;
        ASSERT_TRUE( empty.IsEmpty() && 0 == empty.GetCellCount() && !empty.Contains( P2( 0, 0 ) ) && empty.GetCell( 0 ) == P2( 0, 0 ) && 0 == empty.GetArea() );
        ASSERT_TRUE( r.Union( empty ) == r && empty.Union( r ) == r && r.Contains( empty ) && !empty.Contains( r ) && !empty.Intersects( r ) && !r.Intersects( empty ) );
        ASSERT_TRUE( Rect( 3, 3, 0, 5 ).IsEmpty() && Rect( 3, 3, 5, -1 ).IsEmpty() && 0 == Rect( 3, 3, -5, -5 ).GetCellCount() && !Rect( 3, 3, 0, 5 ).Contains( P2( 3, 3 ) ) );
        ASSERT_TRUE( Rect( 3, 3, -5, -5 ).GetArea() == 0 );

        // cell boundaries
        ASSERT_TRUE( r.size == V2( 4, 3 ) && r.position == P2( 0, 0 ) && CTIntegerRect< Int32 >( 1, 2, 3, 4 ) == Rect( 1, 2, 3, 4 ) && Rect( 1, 2, 3, 4 ) != Rect( 1, 2, 3, 5 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "IntegerRanges: rectangles at the limits of the kind do not overflow" )
    try
    {
        const Int32 hi = 2147483647;
        const Int32 lo = -hi - 1;
        // a rectangle at the top of the range: containment uses the unsigned distance
        const CTIntegerRect< Int32 > top = Rect( hi - 2, hi - 2, 3, 3 );
        ASSERT_TRUE( top.Contains( P2( hi, hi ) ) && top.Contains( P2( hi - 2, hi - 1 ) ) && !top.Contains( P2( hi - 3, hi ) ) && !top.Contains( P2( lo, lo ) ) );
        ASSERT_TRUE( 9 == top.GetCellCount() && top.GetCell( 8 ) == P2( hi, hi ) && top.GetCell( 4 ) == P2( hi - 1, hi - 1 ) );
        // the end wraps in the kind, as the unsigned image does
        ASSERT_TRUE( top.GetEnd() == P2( lo, lo ) );
        // the full range: huge sizes, containment of extremes in both directions
        const CTIntegerRect< Int32 > huge = Rect( lo, lo, hi, hi );
        ASSERT_TRUE( huge.Contains( P2( lo, lo ) ) && huge.Contains( P2( lo + hi - 1, lo + hi - 1 ) ) && !huge.Contains( P2( lo + hi, 0 ) ) && !huge.Contains( P2( hi, hi ) ) );
        ASSERT_TRUE( (UInt64) hi * (UInt64) hi == huge.GetCellCount() );
        ASSERT_TRUE( huge.Intersection( huge ) == huge && huge.Union( huge ) == huge && huge.Contains( huge ) && huge.Intersects( huge ) );
        ASSERT_TRUE( huge.GetCell( (UInt64) hi * (UInt64) hi - 1 ) == P2( lo + hi - 1, lo + hi - 1 ) );
        // the union of two far apart rectangles saturates its size at the largest value
        const CTIntegerRect< Int32 > farUnion = Rect( lo, 0, 1, 1 ).Union( Rect( hi, 0, 1, 1 ) );
        ASSERT_TRUE( farUnion.position == P2( lo, 0 ) && farUnion.size == V2( hi, 1 ) );
        ASSERT_FALSE( farUnion.Contains( Rect( hi, 0, 1, 1 ) ) );       // the saturated union does not hold its input: that is what TryUnion reports
        CTIntegerRect< Int32 > tried;
        ASSERT_FALSE( Rect( lo, 0, 10, 1 ).TryUnion( Rect( hi - 10, 0, 10, 1 ), tried ) );
        ASSERT_TRUE( tried == Rect( lo, 0, 10, 1 ).Union( Rect( hi - 10, 0, 10, 1 ) ) && !tried.Contains( Rect( hi - 10, 0, 10, 1 ) ) );
        ASSERT_TRUE( Rect( 0, 0, 4, 3 ).TryUnion( Rect( 2, 1, 5, 5 ), tried ) && tried == Rect( 0, 0, 7, 6 ) );
        ASSERT_TRUE( Rect( 0, 0, 0, 0 ).TryUnion( Rect( 2, 1, 5, 5 ), tried ) && tried == Rect( 2, 1, 5, 5 ) );
        ASSERT_TRUE( Rect( lo, 0, 1, 1 ).TryUnion( Rect( lo + hi - 1, 0, 1, 1 ), tried ) && tried.Contains( Rect( lo, 0, 1, 1 ) ) );   // exactly fits: size hi
        CTIntegerBox3D< Int32 > triedBox;
        ASSERT_FALSE( CTIntegerBox3D< Int32 >( P3( 0, lo, 0 ), V3( 1, 10, 1 ) ).TryUnion( CTIntegerBox3D< Int32 >( P3( 0, hi - 10, 0 ), V3( 1, 10, 1 ) ), triedBox ) );
        ASSERT_TRUE( CTIntegerBox3D< Int32 >( P3( 0, 0, 0 ), V3( 2, 2, 2 ) ).TryUnion( CTIntegerBox3D< Int32 >( P3( 3, 3, 3 ), V3( 1, 1, 1 ) ), triedBox ) && triedBox == CTIntegerBox3D< Int32 >( P3( 0, 0, 0 ), V3( 4, 4, 4 ) ) );
        // expanding saturates the position and the size
        const CTIntegerRect< Int32 > grown = Rect( lo + 1, 0, 1, 1 ).Expanded( hi );
        ASSERT_TRUE( grown.position == P2( lo, -hi ) && grown.size == V2( hi, hi ) );
        ASSERT_TRUE( Rect( 0, 0, 1, 1 ).Expanded( lo ).IsEmpty() );
        // the area wraps as the kind does, never undefined behaviour
        ASSERT_TRUE( 1 == huge.GetArea() );             // ( 2^31 - 1 )^2 = 1 modulo 2^32
        ASSERT_TRUE( Rect( 0, 0, 65536, 65536 ).GetArea() == 0 && 65536 * 65536ULL == Rect( 0, 0, 65536, 65536 ).GetCellCount() );
        // moving past the end wraps
        ASSERT_TRUE( Rect( hi, hi, 1, 1 ).Translated( V2( 1, 1 ) ).position == P2( lo, lo ) );
        // adjacent rectangles at the top
        ASSERT_FALSE( Rect( hi - 1, 0, 1, 1 ).Intersects( Rect( hi, 0, 1, 1 ) ) );
        ASSERT_TRUE( Rect( hi - 1, 0, 1, 1 ).Union( Rect( hi, 0, 1, 1 ) ) == Rect( hi - 1, 0, 2, 1 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "IntegerRanges: rectangle operations agree with cell enumeration, 3000 seeded rectangle pairs" )
    try
    {
        const UInt64 seed = 0x1DC711;
        CSeededSequence random( seed );
        UInt32 failures = 0;
        UInt32 categories[ 16 ] = { 0 };
        for ( UInt32 i=0; i<3000; ++i )
        {
            const CTIntegerRect< Int32 > a = Rect( random.NextInt32( -10, 10 ), random.NextInt32( -10, 10 ), random.NextInt32( -1, 12 ), random.NextInt32( -1, 12 ) );
            const CTIntegerRect< Int32 > b = Rect( random.NextInt32( -10, 10 ), random.NextInt32( -10, 10 ), random.NextInt32( -1, 12 ), random.NextInt32( -1, 12 ) );
            bool cellsA[ GRIDSIZE ][ GRIDSIZE ];
            bool cellsB[ GRIDSIZE ][ GRIDSIZE ];
            bool both[ GRIDSIZE ][ GRIDSIZE ];
            bool either[ GRIDSIZE ][ GRIDSIZE ];
            MarkCells( a, cellsA );
            MarkCells( b, cellsB );
            bool subset = true;
            for ( UInt32 y=0; y<GRIDSIZE; ++y )
            {
                for ( UInt32 x=0; x<GRIDSIZE; ++x )
                {
                    both[ y ][ x ] = cellsA[ y ][ x ] && cellsB[ y ][ x ];
                    either[ y ][ x ] = cellsA[ y ][ x ] || cellsB[ y ][ x ];
                    subset = subset && ( !cellsB[ y ][ x ] || cellsA[ y ][ x ] );
                    if ( a.Contains( P2( (Int32) x - GRID, (Int32) y - GRID ) ) != cellsA[ y ][ x ] )
                        { ++failures; ++categories[ 1 ]; };
                }
            }
            if ( a.GetCellCount() != CountMarked( cellsA ) )
                { ++failures; ++categories[ 2 ]; };
            if ( !SameCells( a.Intersection( b ), both ) || a.Intersects( b ) != ( CountMarked( both ) > 0 ) || !( a.Intersection( b ) == a.Clip( b ) ) )
                { ++failures; ++categories[ 3 ]; };
            if ( a.Contains( b ) != subset )
                { ++failures; ++categories[ 4 ]; };
            // the union holds both and is the smallest such rectangle: its cells are the bounding box of the non-empty ones
            const CTIntegerRect< Int32 > u = a.Union( b );
            bool unionHoldsEither = true;
            for ( UInt32 y=0; y<GRIDSIZE; ++y )
                for ( UInt32 x=0; x<GRIDSIZE; ++x )
                    unionHoldsEither = unionHoldsEither && ( !either[ y ][ x ] || u.Contains( P2( (Int32) x - GRID, (Int32) y - GRID ) ) );
            if ( !unionHoldsEither )
                { ++failures; ++categories[ 5 ]; };
            if ( !a.IsEmpty() && !b.IsEmpty() && ( !u.Contains( a ) || !u.Contains( b ) || u.GetCellCount() > (UInt64) ( ( 12 + 12 + 20 ) * ( 12 + 12 + 20 ) ) ) )
                { ++failures; ++categories[ 6 ]; };
            if ( a.IsEmpty() && !( u == b ) && !b.IsEmpty() )
                { ++failures; ++categories[ 7 ]; };
            // iteration visits every cell once, in order, and only cells of the rectangle
            UInt64 previous = 0;
            bool ordered = true;
            for ( UInt64 c=0; c<a.GetCellCount(); ++c )
            {
                const P2 cell = a.GetCell( c );
                const UInt64 rank = (UInt64) ( cell.y - a.position.y ) * (UInt64) a.size.x + (UInt64) ( cell.x - a.position.x );
                ordered = ordered && a.Contains( cell ) && rank == c && ( 0 == c || rank > previous );
                previous = rank;
            }
            if ( !ordered )
                { ++failures; ++categories[ 8 ]; };
            // expansion: every cell of the original stays, and the margin shows
            const CTIntegerRect< Int32 > e = a.Expanded( 2 );
            if ( !a.IsEmpty() && ( !e.Contains( a ) || e.size.x != a.size.x + 4 || e.size.y != a.size.y + 4 ) )
                { ++failures; ++categories[ 9 ]; };
        }
        if ( failures > 0 )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "IntegerRanges: " << failures << " rectangle failures, seed " << seed << " by check " << categories[1] << " " << categories[2] << " " << categories[3] << " " << categories[4] << " " << categories[5] << " " << categories[6] << " " << categories[7] << " " << categories[8] << " " << categories[9] << " " << categories[10] );
        ASSERT_TRUE( 0 == failures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "IntegerRanges: box containment, intersection, union and cell iteration" )
    try
    {
        const CTIntegerBox3D< Int32 > b( P3( 0, 0, 0 ), V3( 4, 3, 2 ) );
        ASSERT_TRUE( b.Contains( P3( 3, 2, 1 ) ) && !b.Contains( P3( 4, 2, 1 ) ) && !b.Contains( P3( 3, 3, 1 ) ) && !b.Contains( P3( 3, 2, 2 ) ) && b.Contains( P3( 0, 0, 0 ) ) && !b.Contains( P3( 0, 0, -1 ) ) );
        ASSERT_TRUE( 24 == b.GetCellCount() && 24 == b.GetVolume() && b.GetEnd() == P3( 4, 3, 2 ) && b.GetMin() == P3( 0, 0, 0 ) );
        for ( UInt64 i=0; i<24; ++i )
        {
            const P3 cell = b.GetCell( i );
            ASSERT_TRUE( cell.x == (Int32) ( i % 4 ) && cell.y == (Int32) ( ( i / 4 ) % 3 ) && cell.z == (Int32) ( i / 12 ) && b.Contains( cell ) );
        }
        ASSERT_TRUE( b.GetCell( 24 ) == b.position );
        const CTIntegerBox3D< Int32 > other( P3( 2, 1, 1 ), V3( 5, 5, 5 ) );
        ASSERT_TRUE( b.Intersects( other ) && b.Intersection( other ) == CTIntegerBox3D< Int32 >( P3( 2, 1, 1 ), V3( 2, 2, 1 ) ) && b.Union( other ) == CTIntegerBox3D< Int32 >( P3( 0, 0, 0 ), V3( 7, 6, 6 ) ) );
        ASSERT_FALSE( b.Intersects( CTIntegerBox3D< Int32 >( P3( 0, 0, 2 ), V3( 4, 3, 2 ) ) ) );
        ASSERT_TRUE( b.Contains( CTIntegerBox3D< Int32 >( P3( 1, 1, 0 ), V3( 2, 2, 2 ) ) ) && !b.Contains( other ) && b.Expanded( 1 ) == CTIntegerBox3D< Int32 >( P3( -1, -1, -1 ), V3( 6, 5, 4 ) ) );
        ASSERT_TRUE( b.Expanded( -1 ).IsEmpty() && b.Translated( V3( 1, 2, 3 ) ).position == P3( 1, 2, 3 ) && b.Clip( other ) == b.Intersection( other ) );
        const CTIntegerBox3D< Int32 > empty;
        ASSERT_TRUE( empty.IsEmpty() && 0 == empty.GetCellCount() && b.Union( empty ) == b && empty.Union( b ) == b && b.Contains( empty ) && !empty.Contains( b ) && !empty.Intersects( b ) );

        const Int32 hi = 2147483647;
        const CTIntegerBox3D< Int32 > huge( P3( 0, 0, 0 ), V3( hi, hi, hi ) );
        ASSERT_TRUE( ~(UInt64) 0 == huge.GetCellCount() );      // saturates
        const CTIntegerBox3D< Int32 > top( P3( hi - 1, hi - 1, hi - 1 ), V3( 2, 2, 2 ) );
        ASSERT_TRUE( top.Contains( P3( hi, hi, hi ) ) && !top.Contains( P3( hi - 2, hi, hi ) ) && 8 == top.GetCellCount() && top.GetCell( 7 ) == P3( hi, hi, hi ) );

        // brute force against cell enumeration
        const UInt64 seed = 0x30B0C5;
        CSeededSequence random( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<1500; ++i )
        {
            const CTIntegerBox3D< Int32 > a( P3( random.NextInt32( -5, 5 ), random.NextInt32( -5, 5 ), random.NextInt32( -5, 5 ) ), V3( random.NextInt32( -1, 6 ), random.NextInt32( -1, 6 ), random.NextInt32( -1, 6 ) ) );
            const CTIntegerBox3D< Int32 > c( P3( random.NextInt32( -5, 5 ), random.NextInt32( -5, 5 ), random.NextInt32( -5, 5 ) ), V3( random.NextInt32( -1, 6 ), random.NextInt32( -1, 6 ), random.NextInt32( -1, 6 ) ) );
            const CTIntegerBox3D< Int32 > cut = a.Intersection( c );
            UInt32 countA = 0;
            UInt32 countBoth = 0;
            bool subset = true;
            for ( Int32 z=-12; z<=12; ++z )
            {
                for ( Int32 y=-12; y<=12; ++y )
                {
                    for ( Int32 x=-12; x<=12; ++x )
                    {
                        const bool inA = Is3DCell( a, x, y, z );
                        const bool inC = Is3DCell( c, x, y, z );
                        countA += inA ? 1 : 0;
                        countBoth += ( inA && inC ) ? 1 : 0;
                        subset = subset && ( !inC || inA );
                        if ( a.Contains( P3( x, y, z ) ) != inA || cut.Contains( P3( x, y, z ) ) != ( inA && inC ) )
                            ++failures;
                    }
                }
            }
            if ( a.GetCellCount() != countA || cut.GetCellCount() != countBoth || a.Intersects( c ) != ( countBoth > 0 ) || a.Contains( c ) != subset )
                ++failures;
            const CTIntegerBox3D< Int32 > u = a.Union( c );
            if ( !a.IsEmpty() && !c.IsEmpty() && ( !u.Contains( a ) || !u.Contains( c ) ) )
                ++failures;
            for ( UInt64 n=0; n<a.GetCellCount(); ++n )
            {
                const P3 cell = a.GetCell( n );
                const UInt64 rank = ( (UInt64) ( cell.z - a.position.z ) * (UInt64) a.size.y + (UInt64) ( cell.y - a.position.y ) ) * (UInt64) a.size.x + (UInt64) ( cell.x - a.position.x );
                if ( !a.Contains( cell ) || rank != n )
                    ++failures;
            }
        }
        if ( failures > 0 )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "IntegerRanges: " << failures << " box failures, seed " << seed );
        ASSERT_TRUE( 0 == failures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
