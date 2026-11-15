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

#ifndef GUCEF_GEOMETRY_CTGRIDTRAVERSAL2D_H
#include "gucefGEOMETRY_CTGridTraversal2D.h"
#define GUCEF_GEOMETRY_CTGRIDTRAVERSAL2D_H
#endif /* GUCEF_GEOMETRY_CTGRIDTRAVERSAL2D_H ? */

#ifndef GUCEF_GEOMETRY_CTGRIDTRAVERSAL3D_H
#include "gucefGEOMETRY_CTGridTraversal3D.h"
#define GUCEF_GEOMETRY_CTGRIDTRAVERSAL3D_H
#endif /* GUCEF_GEOMETRY_CTGRIDTRAVERSAL3D_H ? */

#ifndef GUCEF_GEOMETRY_CTBRESENHAMLINE2D_H
#include "gucefGEOMETRY_CTBresenhamLine2D.h"
#define GUCEF_GEOMETRY_CTBRESENHAMLINE2D_H
#endif /* GUCEF_GEOMETRY_CTBRESENHAMLINE2D_H ? */

#ifndef GUCEF_GEOMETRY_CTBRESENHAMLINE3D_H
#include "gucefGEOMETRY_CTBresenhamLine3D.h"
#define GUCEF_GEOMETRY_CTBRESENHAMLINE3D_H
#endif /* GUCEF_GEOMETRY_CTBRESENHAMLINE3D_H ? */


#include "TestSpatialReference.h"
#include "TestConvexReference.h"
#include "TestGridTraversal.h"

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

typedef CORE::CTFraction< Int64, Int64 > TFrac;

/** A value of a lattice of eighths as a fraction */
static TFrac
Eighths( Int32 k )
{
    return TFrac( (Int64) k, 8 );
}

static TFrac
FloorFrac( const TFrac& value )
{
    // numerator and denominator are exposed as GetNumerator / GetDenominator
    Int64 n = value.GetNumerator();
    const Int64 d = value.GetDenominator();
    Int64 q = n / d;
    if ( n % d != 0 && ( n < 0 ) != ( d < 0 ) )
        --q;
    return TFrac( q, 1 );
}

/*-------------------------------------------------------------------------*/

/**
 *  The reference: the cells (of Dim axes, cell i of an axis covering [ o + i s, o + ( i + 1 ) s ]) that a segment touches,
 *  by clipping the segment with every closed cell of the bounding range in exact fractions. Supercover: every cell the
 *  clip is not empty for. Standard: the clip has a positive length, or (a segment of zero length) the half open cell of
 *  the point; a segment lying on a grid line belongs to the cell on its positive side.
 */
template < UInt32 Dim >
static void
ReferenceCells( const TFrac* origin, const TFrac* size, const TFrac* from, const TFrac* to, bool supercover, GUCEF::vector< Int32 >& cells )
{
    cells.clear();
    TFrac delta[ Dim ];
    bool point = true;
    Int32 lo[ Dim ];
    Int32 hi[ Dim ];
    for ( UInt32 a=0; a<Dim; ++a )
    {
        delta[ a ] = to[ a ] - from[ a ];
        if ( !( delta[ a ] == TFrac( 0, 1 ) ) )
            point = false;
        const TFrac f0 = ( from[ a ] - origin[ a ] ) / size[ a ];
        const TFrac f1 = ( to[ a ] - origin[ a ] ) / size[ a ];
        const TFrac low = f0 < f1 ? f0 : f1;
        const TFrac high = f0 < f1 ? f1 : f0;
        lo[ a ] = (Int32) FloorFrac( low ).GetNumerator() - 1;
        hi[ a ] = (Int32) FloorFrac( high ).GetNumerator() + 1;
    }
    Int32 cell[ Dim ];
    for ( UInt32 a=0; a<Dim; ++a )
        cell[ a ] = lo[ a ];
    for ( ;; )
    {
        TFrac tMin( 0, 1 );
        TFrac tMax( 1, 1 );
        bool empty = false;
        bool onLineHigh = false;      // a constant axis that lies on the upper boundary of this cell
        for ( UInt32 a=0; a<Dim && !empty; ++a )
        {
            const TFrac cellLo = origin[ a ] + TFrac( (Int64) cell[ a ], 1 ) * size[ a ];
            const TFrac cellHi = cellLo + size[ a ];
            if ( delta[ a ] == TFrac( 0, 1 ) )
            {
                if ( from[ a ] < cellLo || cellHi < from[ a ] )
                    empty = true;
                else if ( from[ a ] == cellHi )
                    onLineHigh = true;
            }
            else
            {
                TFrac t1 = ( cellLo - from[ a ] ) / delta[ a ];
                TFrac t2 = ( cellHi - from[ a ] ) / delta[ a ];
                if ( t2 < t1 )
                {
                    const TFrac swap = t1; t1 = t2; t2 = swap;
                }
                if ( tMin < t1 ) tMin = t1;
                if ( t2 < tMax ) tMax = t2;
            }
        }
        if ( !empty && tMax < tMin )
            empty = true;
        bool accept = false;
        if ( !empty )
        {
            if ( supercover )
                accept = true;
            else if ( point )
                accept = !onLineHigh;
            else
                accept = tMin < tMax && !onLineHigh;
        }
        if ( accept )
            for ( UInt32 a=0; a<Dim; ++a )
                cells.push_back( cell[ a ] );
        UInt32 a = 0;
        for ( ; a<Dim; ++a )
        {
            if ( cell[ a ] < hi[ a ] )
            {
                ++cell[ a ];
                break;
            }
            cell[ a ] = lo[ a ];
        }
        if ( a == Dim )
            break;
    }
}

/*-------------------------------------------------------------------------*/

static bool
SameCellSets( GUCEF::vector< Int32 > a, GUCEF::vector< Int32 > b, UInt32 dim )
{
    if ( a.size() != b.size() )
        return false;
    // sort the cells lexicographically: build index order
    GUCEF::vector< GUCEF::vector< Int32 > > ca, cb;
    for ( size_t i=0; i<a.size(); i+=dim )
    {
        ca.push_back( GUCEF::vector< Int32 >( a.begin() + i, a.begin() + i + dim ) );
        cb.push_back( GUCEF::vector< Int32 >( b.begin() + i, b.begin() + i + dim ) );
    }
    std::sort( ca.begin(), ca.end() );
    std::sort( cb.begin(), cb.end() );
    return ca == cb;
}

/*-------------------------------------------------------------------------*/

template < typename T, UInt32 Dim >
struct STraversalRunner;

template < typename T >
struct STraversalRunner< T, 2 >
{
    static void Run( const Int32* originEighths, Int32 sizeEighths, const Int32* from, const Int32* to, bool supercover, GUCEF::vector< Int32 >& cells )
    {
        CTGridTraversal2D< T > traversal;
        const bool ok = traversal.StartSegment( CTPoint2D< T >( (T) originEighths[ 0 ] / (T) 8, (T) originEighths[ 1 ] / (T) 8 ), CTVector2D< T >( (T) sizeEighths / (T) 8, (T) sizeEighths / (T) 8 ),
                                                CTPoint2D< T >( (T) from[ 0 ] / (T) 8, (T) from[ 1 ] / (T) 8 ), CTPoint2D< T >( (T) to[ 0 ] / (T) 8, (T) to[ 1 ] / (T) 8 ),
                                                supercover ? GRIDTRAVERSAL_SUPERCOVER : GRIDTRAVERSAL_STANDARD );
        cells.clear();
        if ( !ok )
            return;
        CTPoint2D< Int32 > cell;
        T previous = (T) -1;
        while ( traversal.Next( cell, previous ) )
        {
            cells.push_back( cell.x );
            cells.push_back( cell.y );
        }
    }
};

template < typename T >
struct STraversalRunner< T, 3 >
{
    static void Run( const Int32* originEighths, Int32 sizeEighths, const Int32* from, const Int32* to, bool supercover, GUCEF::vector< Int32 >& cells )
    {
        CTGridTraversal3D< T > traversal;
        const bool ok = traversal.StartSegment( CTPoint3D< T >( (T) originEighths[ 0 ] / (T) 8, (T) originEighths[ 1 ] / (T) 8, (T) originEighths[ 2 ] / (T) 8 ), CTVector3D< T >( (T) sizeEighths / (T) 8, (T) sizeEighths / (T) 8, (T) sizeEighths / (T) 8 ),
                                                CTPoint3D< T >( (T) from[ 0 ] / (T) 8, (T) from[ 1 ] / (T) 8, (T) from[ 2 ] / (T) 8 ), CTPoint3D< T >( (T) to[ 0 ] / (T) 8, (T) to[ 1 ] / (T) 8, (T) to[ 2 ] / (T) 8 ),
                                                supercover ? GRIDTRAVERSAL_SUPERCOVER : GRIDTRAVERSAL_STANDARD );
        cells.clear();
        if ( !ok )
            return;
        CTPoint3D< Int32 > cell;
        while ( traversal.Next( cell ) )
        {
            cells.push_back( cell.x );
            cells.push_back( cell.y );
            cells.push_back( cell.z );
        }
    }
};

/*-------------------------------------------------------------------------*/

/** Segments between lattice points of eighths: many of them pass exactly through cell corners and along grid lines */
template < typename T, UInt32 Dim >
static bool
TraversalSweep( const char* name, UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    for ( UInt32 n=0; n<count; ++n )
    {
        Int32 origin[ 3 ] = { 0, 0, 0 };
        const Int32 sizeChoices[ 4 ] = { 8, 16, 4, 24 };
        const Int32 size = sizeChoices[ random.Index( 4 ) ];
        Int32 from[ 3 ];
        Int32 to[ 3 ];
        for ( UInt32 a=0; a<Dim; ++a )
        {
            origin[ a ] = random.Index( 3 ) * 8 - 8;
            // lattice points: half of them on the cell lattice itself
            const Int32 range = 8 * 6;
            from[ a ] = random.Chance( 0.5 ) ? ( random.Index( 7 ) * size ) : random.Index( range );
            to[ a ] = random.Chance( 0.5 ) ? ( random.Index( 7 ) * size ) : random.Index( range );
            if ( random.Chance( 0.15 ) )
                to[ a ] = from[ a ];                // runs along a grid line or stands still on that axis
        }
        if ( random.Chance( 0.03 ) )
            for ( UInt32 a=0; a<Dim; ++a )
                to[ a ] = from[ a ];                // a point
        TFrac fo[ 3 ], fs[ 3 ], ff[ 3 ], ft[ 3 ];
        for ( UInt32 a=0; a<Dim; ++a )
        {
            fo[ a ] = Eighths( origin[ a ] );
            fs[ a ] = Eighths( size );
            ff[ a ] = Eighths( from[ a ] );
            ft[ a ] = Eighths( to[ a ] );
        }
        for ( UInt32 mode=0; mode<2; ++mode )
        {
            GUCEF::vector< Int32 > expected;
            GUCEF::vector< Int32 > actual;
            ReferenceCells< Dim >( fo, fs, ff, ft, 1 == mode, expected );
            STraversalRunner< T, Dim >::Run( origin, size, from, to, 1 == mode, actual );
            if ( !SameCellSets( expected, actual, Dim ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": cells differ from the exact reference in mode " << mode << ", seed " << seed << " segment " << n
                                 << " from (" << from[ 0 ] << "," << from[ 1 ] << "," << from[ 2 ] << ")/8 to (" << to[ 0 ] << "," << to[ 1 ] << "," << to[ 2 ] << ")/8 size " << size << "/8 origin (" << origin[ 0 ] << "," << origin[ 1 ] << ","
                                 << origin[ 2 ] << ")/8, expected " << expected.size() / Dim << " cells, got " << actual.size() / Dim );
                return false;
            }
            // no cell twice
            GUCEF::vector< UInt32 > keys;
            for ( size_t i=0; i<actual.size(); i+=Dim )
            {
                UInt32 key = 0;
                for ( UInt32 a=0; a<Dim; ++a )
                    key = key * 1009 + (UInt32) ( actual[ i + a ] + 100 );
                keys.push_back( key );
            }
            if ( !NoDuplicates( keys ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": a cell twice, seed " << seed << " segment " << n );
                return false;
            }
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GridTraversal: " << name << ": " << count << " segments in both modes equal the exact reference" );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The corner cases by hand: a diagonal through exact corners in both modes */
#define FAILHERE do { GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GridTraversal: corner case failed at line " << __LINE__ ); return false; } while ( 0 )

static bool
CornerCases( void )
{
    GUCEF::vector< Int32 > cells;
    const Int32 origin[ 2 ] = { 0, 0 };
    {
        // ( 0.5, 0.5 ) to ( 3.5, 3.5 ) over unit cells: through the corners ( 1, 1 ) ( 2, 2 ) ( 3, 3 )
        const Int32 from[ 2 ] = { 4, 4 };
        const Int32 to[ 2 ] = { 28, 28 };
        STraversalRunner< Float64, 2 >::Run( origin, 8, from, to, false, cells );
        const Int32 expected[ 8 ] = { 0, 0, 1, 1, 2, 2, 3, 3 };
        if ( cells.size() != 8 || 0 != memcmp( &cells[ 0 ], expected, sizeof( expected ) ) )
            FAILHERE;
        STraversalRunner< Float64, 2 >::Run( origin, 8, from, to, true, cells );
        // corner crossings add the two neighbours before the diagonal cell, in the order x then y
        const Int32 expectedSuper[ 20 ] = { 0, 0,  1, 0, 0, 1, 1, 1,  2, 1, 1, 2, 2, 2,  3, 2, 2, 3, 3, 3 };
        if ( cells.size() != 20 || 0 != memcmp( &cells[ 0 ], expectedSuper, sizeof( expectedSuper ) ) )
            FAILHERE;
    }
    {
        // a segment along the grid line y = 1, from x = 0.5 to 2.5: the cells above the line only in the standard mode
        const Int32 from[ 2 ] = { 4, 8 };
        const Int32 to[ 2 ] = { 20, 8 };
        STraversalRunner< Float32, 2 >::Run( origin, 8, from, to, false, cells );
        const Int32 expected[ 6 ] = { 0, 1, 1, 1, 2, 1 };
        if ( cells.size() != 6 || 0 != memcmp( &cells[ 0 ], expected, sizeof( expected ) ) )
            FAILHERE;
        STraversalRunner< Float32, 2 >::Run( origin, 8, from, to, true, cells );
        if ( cells.size() != 12 )
            FAILHERE;
    }
    {
        // a ray that ends exactly on a boundary: the standard mode does not step over, the supercover mode does
        const Int32 from[ 2 ] = { 4, 4 };
        const Int32 to[ 2 ] = { 8, 4 };
        STraversalRunner< Float64, 2 >::Run( origin, 8, from, to, false, cells );
        if ( cells.size() != 2 )
            FAILHERE;
        STraversalRunner< Float64, 2 >::Run( origin, 8, from, to, true, cells );
        if ( cells.size() != 4 )
            FAILHERE;
    }
    {
        // cell bounds end the traversal; the enter parameter of the first cell is 0 and increases
        CTGridTraversal2D< Float64 > traversal;
        traversal.SetCellBounds( CTPoint2D< Int32 >( 0, 0 ), CTPoint2D< Int32 >( 2, 2 ) );
        if ( !traversal.StartRay( CTPoint2D< Float64 >( 0, 0 ), CTVector2D< Float64 >( 1, 1 ), CTPoint2D< Float64 >( 0.25, 0.5 ), CTVector2D< Float64 >( 1, 0.25 ), 1.0e9 ) )
            FAILHERE;
        CTPoint2D< Int32 > cell;
        Float64 enter = -1.0, last = -1.0;
        UInt32 n = 0;
        while ( traversal.Next( cell, enter ) )
        {
            if ( ( 0 == n && enter != 0.0 ) || enter < last || cell.x < 0 || cell.x > 2 || cell.y < 0 || cell.y > 2 )
                FAILHERE;
            last = enter;
            ++n;
        }
        // ( 0, 0 ) ( 1, 0 ) ( 2, 0 ) and ( 2, 1 ) once y reaches 1 at x = 2.25
        if ( 4 != n )
            FAILHERE;
    }
    {
        // refused input
        CTGridTraversal2D< Float64 > traversal;
        if ( traversal.StartSegment( CTPoint2D< Float64 >( 0, 0 ), CTVector2D< Float64 >( 0, 1 ), CTPoint2D< Float64 >( 0, 0 ), CTPoint2D< Float64 >( 1, 1 ) ) )
            FAILHERE;
        const Float64 nan = 0.0 / ( (Float64) cells.size() - (Float64) cells.size() );
        if ( traversal.StartSegment( CTPoint2D< Float64 >( 0, 0 ), CTVector2D< Float64 >( 1, 1 ), CTPoint2D< Float64 >( nan, 0 ), CTPoint2D< Float64 >( 1, 1 ) ) )
            FAILHERE;
        // an unbounded ray without cell bounds is refused
        if ( traversal.StartRay( CTPoint2D< Float64 >( 0, 0 ), CTVector2D< Float64 >( 1, 1 ), CTPoint2D< Float64 >( 0, 0 ), CTVector2D< Float64 >( 1, 1 ), 1.0 / ( (Float64) cells.size() - (Float64) cells.size() ) ) )
            FAILHERE;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

#undef FAILHERE

/** The classic table of one octant and its images in the other seven, a line over a lattice and the formula over random lines in every octant */
static Int32
IdealMinor( Int64 k, Int64 minorDifference, Int64 majorDifference )
{
    return (Int32) ( ( 2 * minorDifference * k + majorDifference ) / ( 2 * majorDifference ) );
}

static bool
Bresenham2DTables( UInt64 seed, UInt32 count )
{
    typedef CTBresenhamLine2D::TCell TCell;
    {
        // the classic table of the line from ( 0, 0 ) to ( 7, 3 ): y = round( 3 x / 7 )
        const Int32 expected[ 16 ] = { 0,0, 1,0, 2,1, 3,1, 4,2, 5,2, 6,3, 7,3 };
        CTBresenhamLine2D line( TCell( 0, 0 ), TCell( 7, 3 ) );
        if ( 8 != line.GetCellCount() )
            return false;
        TCell cell;
        for ( UInt32 i=0; i<8; ++i )
            if ( !line.Next( cell ) || cell.x != expected[ 2 * i ] || cell.y != expected[ 2 * i + 1 ] )
                return false;
        if ( line.Next( cell ) )
            return false;
        // a tie of the ideal line goes to the cell with the minor step: ( 0, 0 ) to ( 4, 2 )
        const Int32 tie[ 10 ] = { 0,0, 1,1, 2,1, 3,2, 4,2 };
        CTBresenhamLine2D tieLine( TCell( 0, 0 ), TCell( 4, 2 ) );
        for ( UInt32 i=0; i<5; ++i )
            if ( !tieLine.Next( cell ) || cell.x != tie[ 2 * i ] || cell.y != tie[ 2 * i + 1 ] )
                return false;
    }
    // the table in the eight octants: the images of ( 7, 3 ) under the sign changes and the swap
    const Int32 images[ 8 ][ 2 ] = { { 7, 3 }, { 3, 7 }, { -3, 7 }, { -7, 3 }, { -7, -3 }, { -3, -7 }, { 3, -7 }, { 7, -3 } };
    for ( UInt32 o=0; o<8; ++o )
    {
        CTBresenhamLine2D line( TCell( 10, -10 ), TCell( 10 + images[ o ][ 0 ], -10 + images[ o ][ 1 ] ) );
        TCell cell;
        for ( UInt32 k=0; k<8; ++k )
        {
            if ( !line.Next( cell ) )
                return false;
            const Int32 ax = images[ o ][ 0 ] < 0 ? -images[ o ][ 0 ] : images[ o ][ 0 ];
            const Int32 ay = images[ o ][ 1 ] < 0 ? -images[ o ][ 1 ] : images[ o ][ 1 ];
            Int32 dx, dy;
            if ( ax >= ay )
            {
                dx = (Int32) k;
                dy = IdealMinor( k, ay, ax );
            }
            else
            {
                dy = (Int32) k;
                dx = IdealMinor( k, ax, ay );
            }
            if ( cell.x != 10 + ( images[ o ][ 0 ] < 0 ? -dx : dx ) || cell.y != -10 + ( images[ o ][ 1 ] < 0 ? -dy : dy ) )
                return false;
        }
        if ( line.Next( cell ) )
            return false;
    }
    // random lines in all octants against the formula, ties included
    CRandom random( seed );
    for ( UInt32 n=0; n<count; ++n )
    {
        const Int32 x0 = random.Index( 201 ) - 100;
        const Int32 y0 = random.Index( 201 ) - 100;
        const Int32 x1 = random.Index( 201 ) - 100;
        const Int32 y1 = random.Index( 201 ) - 100;
        const Int32 ax = x1 > x0 ? x1 - x0 : x0 - x1;
        const Int32 ay = y1 > y0 ? y1 - y0 : y0 - y1;
        CTBresenhamLine2D line( TCell( x0, y0 ), TCell( x1, y1 ) );
        const Int32 major = ax > ay ? ax : ay;
        if ( (UInt64) major + 1 != line.GetCellCount() )
            return false;
        TCell cell;
        for ( Int32 k=0; k<=major; ++k )
        {
            if ( !line.Next( cell ) )
                return false;
            Int32 dx, dy;
            if ( ax >= ay )
            {
                dx = k;
                dy = 0 == ax ? 0 : IdealMinor( k, ay, ax );
            }
            else
            {
                dy = k;
                dx = IdealMinor( k, ax, ay );
            }
            if ( cell.x != x0 + ( x1 < x0 ? -dx : dx ) || cell.y != y0 + ( y1 < y0 ? -dy : dy ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Bresenham2D: cell " << k << " of the line ( " << x0 << ", " << y0 << " ) to ( " << x1 << ", " << y1 << " ) is ( " << cell.x << ", " << cell.y << " ), seed " << seed << " line " << n );
                return false;
            }
        }
        if ( line.Next( cell ) )
            return false;
    }
    // the whole Int32 range
    {
        CTBresenhamLine2D line( TCell( -2147483647 - 1, 0 ), TCell( 2147483647, 1 ) );
        TCell cell;
        if ( !line.Next( cell ) || cell.x != -2147483647 - 1 || cell.y != 0 )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
Bresenham3DTables( UInt64 seed, UInt32 count )
{
    typedef CTBresenhamLine3D::TCell TCell;
    CRandom random( seed );
    for ( UInt32 n=0; n<count; ++n )
    {
        Int32 p[ 3 ], q[ 3 ];
        for ( UInt32 a=0; a<3; ++a )
        {
            p[ a ] = random.Index( 101 ) - 50;
            q[ a ] = random.Chance( 0.1 ) ? p[ a ] : random.Index( 101 ) - 50;
        }
        Int32 d[ 3 ];
        Int32 dm = 0;
        for ( UInt32 a=0; a<3; ++a )
        {
            d[ a ] = q[ a ] > p[ a ] ? q[ a ] - p[ a ] : p[ a ] - q[ a ];
            if ( d[ a ] > dm )
                dm = d[ a ];
        }
        CTBresenhamLine3D line( TCell( p[ 0 ], p[ 1 ], p[ 2 ] ), TCell( q[ 0 ], q[ 1 ], q[ 2 ] ) );
        if ( (UInt64) dm + 1 != line.GetCellCount() )
            return false;
        const Int32 e0 = dm / 2;
        TCell cell;
        for ( Int32 k=0; k<=dm; ++k )
        {
            if ( !line.Next( cell ) )
                return false;
            const Int32 got[ 3 ] = { cell.x, cell.y, cell.z };
            for ( UInt32 a=0; a<3; ++a )
            {
                // the steps taken on the axis after k cells: the closed form of the error term that starts at dm / 2
                const Int32 steps = (Int64) k * d[ a ] > e0 ? (Int32) ( ( (Int64) k * d[ a ] - e0 + dm - 1 ) / dm ) : 0;
                const Int32 expected = p[ a ] + ( q[ a ] < p[ a ] ? -steps : steps );
                if ( got[ a ] != expected )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Bresenham3D: axis " << a << " of cell " << k << " is " << got[ a ] << " instead of " << expected << ", seed " << seed << " line " << n );
                    return false;
                }
            }
        }
        if ( line.Next( cell ) )
            return false;
        // the end point is the last cell and the steps never skip a cell (neighbours differ by at most 1 per axis)
    }
    // the diagonal and the axes
    {
        CTBresenhamLine3D line( TCell( 0, 0, 0 ), TCell( 5, 5, 5 ) );
        TCell cell;
        for ( Int32 k=0; k<=5; ++k )
            if ( !line.Next( cell ) || cell.x != k || cell.y != k || cell.z != k )
                return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGridTraversalTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "GridTraversal" );

    GUCEF_TESTFW_TESTCASE( "GridTraversal: the DDA cells equal the exact rational reference in 2D, standard and supercover mode, Float64 and Float32" )
    try
    {
        ASSERT_TRUE( ( TraversalSweep< Float64, 2 >( "2D Float64", 4101, ScaledCount( 4000 ) ) ) );
        ASSERT_TRUE( ( TraversalSweep< Float32, 2 >( "2D Float32", 4102, ScaledCount( 2000 ) ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridTraversal: the DDA cells equal the exact rational reference in 3D, standard and supercover mode" )
    try
    {
        ASSERT_TRUE( ( TraversalSweep< Float64, 3 >( "3D Float64", 4103, ScaledCount( 2000 ) ) ) );
        ASSERT_TRUE( ( TraversalSweep< Float32, 3 >( "3D Float32", 4104, ScaledCount( 1000 ) ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridTraversal: rays through exact cell corners, along grid lines, ending on boundaries, cell bounds and refused input" )
    try
    {
        ASSERT_TRUE( CornerCases() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridTraversal: Bresenham 2D matches the classic octant table and the closed form over random lines in every octant" )
    try
    {
        ASSERT_TRUE( Bresenham2DTables( 4105, ScaledCount( 20000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridTraversal: Bresenham 3D matches the closed form of the error terms over random lines" )
    try
    {
        ASSERT_TRUE( Bresenham3DTables( 4106, ScaledCount( 10000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
