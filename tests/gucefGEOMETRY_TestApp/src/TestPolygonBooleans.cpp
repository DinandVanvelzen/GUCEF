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
#include <algorithm>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYGONCLIPPER_H
#include "gucefGEOMETRY_CTPolygonClipper.h"
#define GUCEF_GEOMETRY_CTPOLYGONCLIPPER_H
#endif /* GUCEF_GEOMETRY_CTPOLYGONCLIPPER_H ? */

#include "TestConvexReference.h"
#include "TestPolygonReference.h"
#include "TestPolygonBooleans.h"

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

/** Operands on the grid: rings with a fill rule each */
struct SOperands
{
    TGridRings subject;
    TGridRings clip;
    TPolygonFillRule subjectFill;
    TPolygonFillRule clipFill;
};

/*-------------------------------------------------------------------------*/

static TGridRing
GridRing( const G* xy, UInt32 count )
{
    TGridRing ring;
    for ( UInt32 i=0; i<count; ++i )
        ring.push_back( CTPoint2D< G >( xy[ 2 * i ], xy[ 2 * i + 1 ] ) );
    return ring;
}

static TGridRing
Box( G x0, G y0, G x1, G y1 )
{
    const G xy[] = { x0, y0, x1, y0, x1, y1, x0, y1 };
    return GridRing( xy, 4 );
}

static TGridRing
Reversed( TGridRing ring )
{
    std::reverse( ring.begin(), ring.end() );
    return ring;
}

/*-------------------------------------------------------------------------*/

/** Twice the area of a result: the sum of the signed ring areas (holes are clockwise) */
template < typename T >
static F
ResultDoubleArea( const GUCEF::vector< GUCEF::vector< CTPoint2D< T > > >& rings )
{
    F sum( 0, 1 );
    for ( size_t r=0; r<rings.size(); ++r )
        sum = sum + RingDoubleArea( &rings[ r ][ 0 ], (UInt32) rings[ r ].size() );
    return sum;
}

/*-------------------------------------------------------------------------*/

/** True when no two result edges cross or overlap (they may share end points) and every ring has three or more points */
template < typename T >
static bool
ResultIsPlanar( const GUCEF::vector< GUCEF::vector< CTPoint2D< T > > >& rings )
{
    typedef CTPoint2D< T > TPoint;
    GUCEF::vector< TPoint > a, b;
    for ( size_t r=0; r<rings.size(); ++r )
    {
        if ( rings[ r ].size() < 3 )
            return false;
        for ( size_t i=0; i<rings[ r ].size(); ++i )
        {
            a.push_back( rings[ r ][ i ] );
            b.push_back( rings[ r ][ ( i + 1 ) % rings[ r ].size() ] );
        }
    }
    for ( size_t i=0; i<a.size(); ++i )
        for ( size_t j=i+1; j<a.size(); ++j )
        {
            if ( ( a[ i ] == a[ j ] && b[ i ] == b[ j ] ) || ( a[ i ] == b[ j ] && b[ i ] == a[ j ] ) )
                return false;
            const bool aShared = a[ i ] == a[ j ] || a[ i ] == b[ j ];
            const bool bShared = b[ i ] == a[ j ] || b[ i ] == b[ j ];
            if ( !aShared && !bShared )
            {
                if ( SegmentsMeet( a[ i ], b[ i ], a[ j ], b[ j ] ) )
                    return false;
                continue;
            }
            // sharing one end point: only an overlap along the same line is wrong
            const TPoint& shared = aShared ? a[ i ] : b[ i ];
            const TPoint& p = aShared ? b[ i ] : a[ i ];
            const TPoint& q = shared == a[ j ] ? b[ j ] : a[ j ];
            if ( 0 == ReferenceOrient( shared, p, q ) && ( SegmentsMeet( shared, p, q, q ) || SegmentsMeet( shared, q, p, p ) ) )
                return false;
        }
    return true;
}

/*-------------------------------------------------------------------------*/

/** The region of grid rings in Float64 for the scanline reference */
static SRegion
Region( const TGridRings& rings, int fill )
{
    SRegion region;
    region.fill = fill;
    for ( size_t r=0; r<rings.size(); ++r )
    {
        GUCEF::vector< CTPoint2D< Float64 > > ring;
        for ( size_t i=0; i<rings[ r ].size(); ++i )
            ring.push_back( CTPoint2D< Float64 >( (Float64) rings[ r ][ i ].x, (Float64) rings[ r ][ i ].y ) );
        region.rings.push_back( ring );
    }
    return region;
}

/*-------------------------------------------------------------------------*/

/**
 *  Runs the four operations in kind T on the operands and checks: valid planar output; the exact identities among the
 *  results ( U = (S - C) + (C - S) + I, X = U - I ); with 'exactInputs' also against the input areas
 *  ( U = S + C - I, S - C + I = S ); with 'rows' > 0 the scanline reference of every operation within 1e-3 relative
 *  (the measure of the symmetric difference of result and reference), plus 'roundingUnit' times the perimeter of the
 *  operands for kinds whose grid is the test grid (integers: a crossing moves by at most half a unit diagonally).
 */
template < typename T >
static bool
CheckOperands( const char* name, const char* context, const SOperands& operands, T ( *make )( G ), bool exactInputs, UInt32 rows, CTPolygonClipper< T >& clipper, bool planarCheck = true, Float64 roundingUnit = 0.0 )
{
    typedef GUCEF::vector< CTPoint2D< T > > TPath;
    typedef GUCEF::vector< TPath > TPaths;
    TPaths results[ 5 ];
    const TPolygonBooleanOperation operations[ 5 ] = { POLYGONBOOLEAN_UNION, POLYGONBOOLEAN_INTERSECTION, POLYGONBOOLEAN_DIFFERENCE, POLYGONBOOLEAN_XOR, POLYGONBOOLEAN_DIFFERENCE };
    F areas[ 5 ];
    for ( UInt32 k=0; k<5; ++k )
    {
        clipper.Clear();
        const TGridRings& first = 4 == k ? operands.clip : operands.subject;
        const TGridRings& second = 4 == k ? operands.subject : operands.clip;
        for ( size_t r=0; r<first.size(); ++r )
            clipper.AddSubject( ToKind( first[ r ], make ) );
        for ( size_t r=0; r<second.size(); ++r )
            clipper.AddClip( ToKind( second[ r ], make ) );
        const TPolygonFillRule fillFirst = 4 == k ? operands.clipFill : operands.subjectFill;
        const TPolygonFillRule fillSecond = 4 == k ? operands.subjectFill : operands.clipFill;
        const TPolygonClipperStatus status = clipper.Execute( operations[ k ], fillFirst, fillSecond, results[ k ] );
        if ( POLYGONCLIPPER_OK != status )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": operation " << k << " status " << (Int32) status );
            return false;
        }
        if ( planarCheck && !ResultIsPlanar( results[ k ] ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": the result of operation " << k << " is not planar" );
            return false;
        }
        areas[ k ] = ResultDoubleArea( results[ k ] );
        if ( SpatialReference::SignOfFraction( areas[ k ] ) < 0 )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": the result of operation " << k << " has a negative area" );
            return false;
        }
    }
    const F& U = areas[ 0 ];
    const F& I = areas[ 1 ];
    const F& D = areas[ 2 ];
    const F& X = areas[ 3 ];
    const F& D2 = areas[ 4 ];
    if ( !( U == D + D2 + I ) || !( X == U - I ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": the results are not consistent: U " << ToFloat64( U ) / 2 << " I " << ToFloat64( I ) / 2 << " S-C " << ToFloat64( D ) / 2 << " C-S " << ToFloat64( D2 ) / 2 << " X " << ToFloat64( X ) / 2 );
        return false;
    }
    if ( exactInputs )
    {
        // the input areas by the shoelace sums of the rings (their orientation carries the fill of these inputs)
        F subjectArea( 0, 1 ), clipArea( 0, 1 );
        for ( size_t r=0; r<operands.subject.size(); ++r )
        {
            const TPath ring = ToKind( operands.subject[ r ], make );
            subjectArea = subjectArea + RingDoubleArea( &ring[ 0 ], (UInt32) ring.size() );
        }
        for ( size_t r=0; r<operands.clip.size(); ++r )
        {
            const TPath ring = ToKind( operands.clip[ r ], make );
            clipArea = clipArea + RingDoubleArea( &ring[ 0 ], (UInt32) ring.size() );
        }
        if ( !( U == subjectArea + clipArea - I ) || !( D + I == subjectArea ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": U " << ToFloat64( U ) / 2 << " I " << ToFloat64( I ) / 2 << " S " << ToFloat64( subjectArea ) / 2 << " C " << ToFloat64( clipArea ) / 2 << ": the identities with the inputs do not hold" );
            return false;
        }
    }
    if ( rows > 0 )
    {
        const SRegion s = Region( operands.subject, (int) operands.subjectFill );
        const SRegion c = Region( operands.clip, (int) operands.clipFill );
        Float64 yMin = 1e300, yMax = -1e300;
        for ( UInt32 side=0; side<2; ++side )
        {
            const TGridRings& rings = 0 == side ? operands.subject : operands.clip;
            for ( size_t r=0; r<rings.size(); ++r )
                for ( size_t i=0; i<rings[ r ].size(); ++i )
                {
                    yMin = std::min( yMin, (Float64) rings[ r ][ i ].y );
                    yMax = std::max( yMax, (Float64) rings[ r ][ i ].y );
                }
        }
        Float64 perimeter = 0.0;
        for ( UInt32 side=0; side<2; ++side )
        {
            const TGridRings& rings = 0 == side ? operands.subject : operands.clip;
            for ( size_t r=0; r<rings.size(); ++r )
                for ( size_t i=0; i<rings[ r ].size(); ++i )
                {
                    const CTPoint2D< G >& p = rings[ r ][ i ];
                    const CTPoint2D< G >& q = rings[ r ][ ( i + 1 ) % rings[ r ].size() ];
                    perimeter += sqrt( (Float64) ( q.x - p.x ) * (Float64) ( q.x - p.x ) + (Float64) ( q.y - p.y ) * (Float64) ( q.y - p.y ) );
                }
        }
        const int ops[ 5 ] = { 0, 1, 2, 3, 2 };
        for ( UInt32 k=0; k<5; ++k )
        {
            SRegion result;
            result.fill = 1;
            for ( size_t r=0; r<results[ k ].size(); ++r )
            {
                GUCEF::vector< CTPoint2D< Float64 > > ring;
                for ( size_t i=0; i<results[ k ][ r ].size(); ++i )
                {
                    // back to grid units
                    const Float64 x = ToFloat64( ExactFraction( results[ k ][ r ][ i ].x ) ) / ToFloat64( ExactFraction( make( 1 ) ) );
                    const Float64 y = ToFloat64( ExactFraction( results[ k ][ r ][ i ].y ) ) / ToFloat64( ExactFraction( make( 1 ) ) );
                    ring.push_back( CTPoint2D< Float64 >( x, y ) );
                }
                result.rings.push_back( ring );
            }
            Float64 reference = 0.0, measured = 0.0, mismatch = 0.0;
            ScanlineMeasure( 4 == k ? c : s, 4 == k ? s : c, ops[ k ], result, yMin, yMax, rows, reference, measured, mismatch );
            const Float64 scale = std::max( reference, 1e-12 * ( yMax - yMin ) * ( yMax - yMin ) );
            if ( mismatch > 1e-3 * scale + roundingUnit * perimeter && mismatch > 1e-9 * ( yMax - yMin ) * ( yMax - yMin ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": operation " << k << " differs from the scanline reference: mismatch " << mismatch << " of " << reference );
                return false;
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Fixed cases on the grid (every crossing a grid point): exact identities with the input areas and specific areas */
template < typename T >
static bool
FixedCases( const char* name, T ( *make )( G ) )
{
    CTPolygonClipper< T > clipper;
    bool ok = true;
    SOperands o;
    o.subjectFill = POLYGONFILL_NON_ZERO;
    o.clipFill = POLYGONFILL_NON_ZERO;

    // overlapping squares
    o.subject.assign( 1, Box( 0, 0, 40, 40 ) );
    o.clip.assign( 1, Box( 20, 20, 60, 60 ) );
    ok = CheckOperands( name, "overlapping squares", o, make, true, 400, clipper ) && ok;
    {
        GUCEF::vector< GUCEF::vector< CTPoint2D< T > > > rings;
        clipper.Clear();
        clipper.AddSubject( ToKind( o.subject[ 0 ], make ) );
        clipper.AddClip( ToKind( o.clip[ 0 ], make ) );
        clipper.Execute( POLYGONBOOLEAN_INTERSECTION, POLYGONFILL_NON_ZERO, POLYGONFILL_NON_ZERO, rings );
        const bool square = 1 == rings.size() && 4 == rings[ 0 ].size() && ResultDoubleArea( rings ) == DoubleArea( CTPoint2D< T >( make( 0 ), make( 0 ) ), CTPoint2D< T >( make( 20 ), make( 0 ) ), CTPoint2D< T >( make( 0 ), make( 20 ) ) ) * F( 2, 1 );
        if ( !square )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the intersection of the squares is not the 20 x 20 square" );
        ok = square && ok;
    }
    // touching along an edge, and along part of an edge
    o.clip.assign( 1, Box( 40, 0, 80, 40 ) );
    ok = CheckOperands( name, "shared edge", o, make, true, 400, clipper ) && ok;
    o.clip.assign( 1, Box( 40, 10, 80, 30 ) );
    ok = CheckOperands( name, "partly shared edge", o, make, true, 400, clipper ) && ok;
    // a shared vertex only
    o.clip.assign( 1, Box( 40, 40, 80, 80 ) );
    ok = CheckOperands( name, "shared vertex", o, make, true, 400, clipper ) && ok;
    // containment, identity
    o.clip.assign( 1, Box( 10, 10, 30, 30 ) );
    ok = CheckOperands( name, "contained", o, make, true, 400, clipper ) && ok;
    o.clip.assign( 1, Box( 0, 0, 40, 40 ) );
    ok = CheckOperands( name, "identical", o, make, true, 400, clipper ) && ok;
    // a subject with a hole (clockwise ring) and a clip that fills the hole and sticks out
    o.subject.clear();
    o.subject.push_back( Box( 0, 0, 60, 60 ) );
    o.subject.push_back( Reversed( Box( 20, 20, 40, 40 ) ) );
    o.clip.assign( 1, Box( 20, 10, 40, 50 ) );
    ok = CheckOperands( name, "hole filled", o, make, true, 400, clipper ) && ok;
    o.clip.assign( 1, Box( 25, 25, 35, 35 ) );
    ok = CheckOperands( name, "island in the hole", o, make, true, 400, clipper ) && ok;
    // a self-touching ring (two squares meeting at a vertex in one ring) against a square over the touching point
    static const G touching[] = { 0, 0, 20, 0, 20, 20, 40, 20, 40, 40, 20, 40, 20, 20, 0, 20 };
    o.subject.assign( 1, GridRing( touching, 8 ) );
    o.clip.assign( 1, Box( 10, 10, 30, 30 ) );
    ok = CheckOperands( name, "self-touching", o, make, true, 400, clipper ) && ok;
    // diagonal edges whose crossings are grid points (45 degrees on even coordinates)
    static const G diamond[] = { 20, 0, 40, 20, 20, 40, 0, 20 };
    o.subject.assign( 1, GridRing( diamond, 4 ) );
    o.clip.assign( 1, Box( 10, 10, 30, 30 ) );
    ok = CheckOperands( name, "diamond", o, make, true, 400, clipper ) && ok;
    // fill rules: a bow tie (even-odd and non-zero agree), two overlapping counter-clockwise rings in one operand
    static const G bowtie[] = { 0, 0, 40, 40, 40, 0, 0, 40 };
    o.subject.assign( 1, GridRing( bowtie, 4 ) );
    o.subjectFill = POLYGONFILL_EVEN_ODD;
    o.clip.assign( 1, Box( 10, 0, 30, 40 ) );
    ok = CheckOperands( name, "bow tie even-odd", o, make, false, 400, clipper ) && ok;
    o.subject.clear();
    o.subject.push_back( Box( 0, 0, 40, 40 ) );
    o.subject.push_back( Box( 20, 0, 60, 40 ) );
    o.subjectFill = POLYGONFILL_NON_ZERO;
    o.clip.assign( 1, Box( 10, 10, 50, 30 ) );
    ok = CheckOperands( name, "overlapping rings non-zero", o, make, false, 400, clipper ) && ok;
    o.subjectFill = POLYGONFILL_EVEN_ODD;
    ok = CheckOperands( name, "overlapping rings even-odd", o, make, false, 400, clipper ) && ok;
    o.subjectFill = POLYGONFILL_POSITIVE;
    o.subject.push_back( Reversed( Box( 0, 0, 60, 40 ) ) );
    ok = CheckOperands( name, "positive", o, make, false, 400, clipper ) && ok;
    o.subjectFill = POLYGONFILL_NEGATIVE;
    ok = CheckOperands( name, "negative", o, make, false, 400, clipper ) && ok;

    // the polygon form: a square with a hole minus nothing keeps its hole, assigned to its outer ring
    CTPolygonWithHoles2D< T > holed;
    holed.outer.points = ToKind( Box( 0, 0, 60, 60 ), make );
    holed.AddHole( CTPolygon2D< T >( &ToKind( Box( 20, 20, 40, 40 ), make )[ 0 ], 4 ) );    // counter-clockwise on input
    CTPolygonWithHoles2D< T > other;
    other.outer.points = ToKind( Box( 100, 0, 120, 20 ), make );
    GUCEF::vector< CTPolygonWithHoles2D< T > > polygons;
    const bool formOk = POLYGONCLIPPER_OK == PolygonBoolean( holed, other, POLYGONBOOLEAN_UNION, polygons ) && 2 == polygons.size() &&
                        polygons.size() == 2 && ( polygons[ 0 ].holes.size() + polygons[ 1 ].holes.size() ) == 1;
    if ( !formOk )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the polygon form gave " << polygons.size() << " polygons" );
    ok = formOk && ok;
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Seeded star polygons (crossings off the grid), some with holes */
template < typename T >
static bool
RandomCases( const char* name, UInt64 seed, UInt32 sets, T ( *make )( G ), Float64 radius, UInt32 maxVertices, UInt32 rows, Float64 roundingUnit )
{
    QueryReference::CRandom random( seed );
    CTPolygonClipper< T > clipper;
    for ( UInt32 set=0; set<sets; ++set )
    {
        SOperands o;
        o.subjectFill = POLYGONFILL_POSITIVE;
        o.clipFill = POLYGONFILL_POSITIVE;
        const Int32 n1 = 3 + random.Index( (Int32) maxVertices );
        const Int32 n2 = 3 + random.Index( (Int32) maxVertices );
        const Float64 shift = random.Uniform( -0.8, 0.8 ) * radius;
        const Float64 spiky = random.Uniform( 0.1, 0.9 );
        o.subject.push_back( StarRing( random, (UInt32) n1, 0, 0, radius * ( 1.0 - spiky ), radius ) );
        o.clip.push_back( StarRing( random, (UInt32) n2, (G) shift, (G) ( shift / 2 ), radius * 0.3, radius * 0.9 ) );
        if ( random.Chance( 0.3 ) )
            o.subject.push_back( Reversed( StarRing( random, 6, 0, 0, radius * 0.05, radius * 0.1 ) ) );
        const CORE::CString context = "seed " + CORE::ToString( seed ) + " set " + CORE::ToString( set );
        if ( !CheckOperands( name, context.C_String(), o, make, false, rows, clipper, n1 + n2 < 80, roundingUnit ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Invalid input and the grid range */
static bool
InvalidInput( void )
{
    bool ok = true;
    CTPolygonClipper< Float64 > clipper;
    typedef CTPoint2D< Float64 > P;
    GUCEF::vector< GUCEF::vector< P > > rings;
    const P bad[] = { P( 0, 0 ), P( 1, 0 ), P( std::numeric_limits< Float64 >::quiet_NaN(), 1 ) };
    clipper.AddSubject( bad, 3 );
    if ( !( POLYGONCLIPPER_INVALID_INPUT == clipper.Execute( POLYGONBOOLEAN_UNION, POLYGONFILL_NON_ZERO, POLYGONFILL_NON_ZERO, rings ) && rings.empty() ) ) { GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: invalid input check 1 failed" ); ok = false; }
    clipper.Clear();
    if ( !( POLYGONCLIPPER_OK == clipper.Execute( POLYGONBOOLEAN_UNION, POLYGONFILL_NON_ZERO, POLYGONFILL_NON_ZERO, rings ) && rings.empty() ) ) { GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: invalid input check 2 failed" ); ok = false; }
    if ( !( POLYGONCLIPPER_INVALID_INPUT == clipper.Execute( (TPolygonBooleanOperation) 9, POLYGONFILL_NON_ZERO, POLYGONFILL_NON_ZERO, rings ) ) ) { GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: invalid input check 3 failed" ); ok = false; }
    // a caller chosen scale that does not fit, then the automatic scale
    const P big[] = { P( 0, 0 ), P( 1e6, 0 ), P( 0, 1e6 ) };
    clipper.AddSubject( big, 3 );
    clipper.SetGridScale( 1e12 );
    if ( !( POLYGONCLIPPER_OUT_OF_RANGE == clipper.Execute( POLYGONBOOLEAN_UNION, POLYGONFILL_NON_ZERO, POLYGONFILL_NON_ZERO, rings ) ) ) { GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: invalid input check 4 failed" ); ok = false; }
    clipper.SetGridScale( 0.0 );
    if ( !( POLYGONCLIPPER_OK == clipper.Execute( POLYGONBOOLEAN_UNION, POLYGONFILL_NON_ZERO, POLYGONFILL_NON_ZERO, rings ) && 1 == rings.size() && 3 == rings[ 0 ].size() ) ) { GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: invalid input check 5 failed" ); ok = false; }
    if ( !( clipper.GetGridScale() == ldexp( 1.0, 20 ) && std::find( rings[ 0 ].begin(), rings[ 0 ].end(), P( 1e6, 0 ) ) != rings[ 0 ].end() ) ) { GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: invalid input check 6 failed" ); ok = false; }
    CTPolygonClipper< Int64 > integer;
    typedef CTPoint2D< Int64 > PI;
    const PI huge[] = { PI( 0, 0 ), PI( (Int64) 1 << 53, 0 ), PI( 0, 1 ) };
    integer.AddSubject( huge, 3 );
    GUCEF::vector< GUCEF::vector< PI > > integerRings;
    if ( !( POLYGONCLIPPER_OUT_OF_RANGE == integer.Execute( POLYGONBOOLEAN_UNION, POLYGONFILL_NON_ZERO, POLYGONFILL_NON_ZERO, integerRings ) ) ) { GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: invalid input check 7 failed" ); ok = false; }
    // degenerate rings: fewer than three points, zero area
    clipper.Clear();
    const P two[] = { P( 0, 0 ), P( 1, 1 ) };
    clipper.AddSubject( two, 2 );
    const P flat[] = { P( 0, 0 ), P( 1, 1 ), P( 2, 2 ) };
    clipper.AddClip( flat, 3 );
    if ( !( POLYGONCLIPPER_OK == clipper.Execute( POLYGONBOOLEAN_UNION, POLYGONFILL_NON_ZERO, POLYGONFILL_NON_ZERO, rings ) && rings.empty() ) ) { GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: invalid input check 8 failed" ); ok = false; }
    // collinear points kept on request
    clipper.Clear();
    const P square[] = { P( 0, 0 ), P( 1, 0 ), P( 2, 0 ), P( 2, 2 ), P( 0, 2 ) };
    clipper.AddSubject( square, 5 );
    clipper.SetKeepCollinear( true );
    if ( !( POLYGONCLIPPER_OK == clipper.Execute( POLYGONBOOLEAN_UNION, POLYGONFILL_NON_ZERO, POLYGONFILL_NON_ZERO, rings ) && 1 == rings.size() && 5 == rings[ 0 ].size() ) ) { GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: invalid input check 9 failed" ); ok = false; }
    clipper.SetKeepCollinear( false );
    if ( !( POLYGONCLIPPER_OK == clipper.Execute( POLYGONBOOLEAN_UNION, POLYGONFILL_NON_ZERO, POLYGONFILL_NON_ZERO, rings ) && 1 == rings.size() && 4 == rings[ 0 ].size() ) ) { GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: invalid input check 10 failed" ); ok = false; }
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: invalid input or the grid range not handled" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Coordinates near the bottom of the Float64 range: the automatic grid scale stays finite */
static bool
TinyCoordinates( void )
{
    bool ok = true;
    typedef CTPoint2D< Float64 > P;
    CTPolygonClipper< Float64 > clipper;
    GUCEF::vector< GUCEF::vector< P > > rings;
    const Float64 side = 1e-300;
    const P square[] = { P( 0, 0 ), P( side, 0 ), P( side, side ), P( 0, side ) };
    clipper.AddSubject( square, 4 );
    const TPolygonClipperStatus status = clipper.Execute( POLYGONBOOLEAN_UNION, POLYGONFILL_NON_ZERO, POLYGONFILL_NON_ZERO, rings );
    const Float64 scale = clipper.GetGridScale();
    bool shape = POLYGONCLIPPER_OK == status && scale > 0.0 && scale <= std::numeric_limits< Float64 >::max() && 1 == rings.size() && 4 == rings[ 0 ].size();
    if ( shape )
    {
        // the area within the grid precision (the scale is capped at 2^1023, so about 2^27 units span the side)
        Float64 area = 0.0;
        for ( size_t i=0; i<4; ++i )
            area += ( rings[ 0 ][ i ].x / side ) * ( rings[ 0 ][ ( i + 1 ) % 4 ].y / side ) - ( rings[ 0 ][ ( i + 1 ) % 4 ].x / side ) * ( rings[ 0 ][ i ].y / side );
        shape = fabs( 0.5 * area - 1.0 ) < 1e-6;
    }
    if ( !shape )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: a square of side 1e-300 gives status " << (Int32) status << ", grid scale " << scale << ", " << rings.size() << " rings" );
        ok = false;
    }
    // subnormal coordinates: still a finite scale and status OK (the square is below the grid resolution)
    clipper.Clear();
    const Float64 tinySide = 100.0 * std::numeric_limits< Float64 >::denorm_min();
    const P subnormal[] = { P( 0, 0 ), P( tinySide, 0 ), P( tinySide, tinySide ), P( 0, tinySide ) };
    clipper.AddSubject( subnormal, 4 );
    const TPolygonClipperStatus tinyStatus = clipper.Execute( POLYGONBOOLEAN_UNION, POLYGONFILL_NON_ZERO, POLYGONFILL_NON_ZERO, rings );
    if ( !( POLYGONCLIPPER_OK == tinyStatus && clipper.GetGridScale() <= std::numeric_limits< Float64 >::max() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: a subnormal square gives status " << (Int32) tinyStatus << ", grid scale " << clipper.GetGridScale() );
        ok = false;
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Regions that touch themselves at a vertex: the ring forms the header documents */
static bool
TouchingRegions( void )
{
    bool ok = true;
    typedef CTPoint2D< Int64 > P;
    CTPolygonClipper< Int64 > clipper;
    GUCEF::vector< GUCEF::vector< P > > rings;
    // two squares that touch at a corner: two rings
    const P a[] = { P( 0, 0 ), P( 2, 0 ), P( 2, 2 ), P( 0, 2 ) };
    const P b[] = { P( 2, 2 ), P( 4, 2 ), P( 4, 4 ), P( 2, 4 ) };
    clipper.AddSubject( a, 4 );
    clipper.AddSubject( b, 4 );
    clipper.Execute( POLYGONBOOLEAN_UNION, POLYGONFILL_NON_ZERO, POLYGONFILL_NON_ZERO, rings );
    if ( !( 2 == rings.size() && 4 == rings[ 0 ].size() && 4 == rings[ 1 ].size() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: two squares touching at a corner give " << rings.size() << " rings" );
        ok = false;
    }
    // a square minus a notch that touches its boundary at ( 4, 4 ): one ring of 7 points through ( 4, 4 ) twice
    clipper.Clear();
    const P square[] = { P( 0, 0 ), P( 4, 0 ), P( 4, 4 ), P( 0, 4 ) };
    const P notch[] = { P( 4, 4 ), P( 2, 1 ), P( 2, 3 ) };
    clipper.AddSubject( square, 4 );
    clipper.AddClip( notch, 3 );
    clipper.Execute( POLYGONBOOLEAN_DIFFERENCE, POLYGONFILL_NON_ZERO, POLYGONFILL_NON_ZERO, rings );
    const bool one = 1 == rings.size() && 7 == rings[ 0 ].size() && 2 == std::count( rings[ 0 ].begin(), rings[ 0 ].end(), P( 4, 4 ) );
    if ( !one || !( RingDoubleArea( &rings[ 0 ][ 0 ], 7 ) == F( 28, 1 ) ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: a square minus a touching notch gives " << rings.size() << " rings" );
        ok = false;
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/**
 *  The union of 'count' separate squares with a square hole each on a grid, in polygon form: one polygon with one hole
 *  per square, the exact total area; 'elapsed' receives the best of 'repeats' timings in milliseconds
 */
static bool
SeparateSquares( UInt32 count, UInt32 repeats, Float64& elapsed )
{
    typedef CTPoint2D< Int64 > P;
    UInt32 side = 1;
    while ( side * side < count )
        ++side;
    CTPolygonClipper< Int64 > clipper;
    for ( UInt32 i=0; i<count; ++i )
    {
        const Int64 x = (Int64) ( i % side ) * 5, y = (Int64) ( i / side ) * 5;
        const P outer[] = { P( x, y ), P( x + 3, y ), P( x + 3, y + 3 ), P( x, y + 3 ) };
        const P hole[] = { P( x + 1, y + 1 ), P( x + 1, y + 2 ), P( x + 2, y + 2 ), P( x + 2, y + 1 ) };
        clipper.AddSubject( outer, 4 );
        clipper.AddSubject( hole, 4 );
    }
    GUCEF::vector< CTPolygonWithHoles2D< Int64 > > polygons;
    elapsed = 0.0;
    for ( UInt32 r=0; r<repeats; ++r )
    {
        const Float64 start = NowMs();
        clipper.Execute( POLYGONBOOLEAN_UNION, POLYGONFILL_NON_ZERO, POLYGONFILL_NON_ZERO, polygons );
        const Float64 time = NowMs() - start;
        elapsed = 0 == r || time < elapsed ? time : elapsed;
    }
    bool ok = polygons.size() == count;
    for ( size_t p=0; p<polygons.size() && ok; ++p )
    {
        ok = 1 == polygons[ p ].holes.size() && 4 == polygons[ p ].outer.points.size() &&
             RingDoubleArea( &polygons[ p ].outer.points[ 0 ], 4 ) == F( 18, 1 ) && RingDoubleArea( &polygons[ p ].holes[ 0 ].points[ 0 ], 4 ) == F( -2, 1 );
        if ( !ok )
            break;
        // the hole lies inside its own square
        Int64 minX = polygons[ p ].outer.points[ 0 ].x, minY = polygons[ p ].outer.points[ 0 ].y;
        for ( size_t i=1; i<4; ++i )
        {
            minX = std::min( minX, polygons[ p ].outer.points[ i ].x );
            minY = std::min( minY, polygons[ p ].outer.points[ i ].y );
        }
        for ( size_t i=0; ok && i<polygons[ p ].holes[ 0 ].points.size(); ++i )
        {
            const P& h = polygons[ p ].holes[ 0 ].points[ i ];
            ok = h.x > minX && h.x < minX + 3 && h.y > minY && h.y < minY + 3;
        }
    }
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: " << count << " separate squares with holes give " << polygons.size() << " polygons" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/**
 *  The cost grows about as n log n for many separate pieces (the windings of the components and the holes of the
 *  polygon form were quadratic): 4096 against 16384 squares with holes, the best of three runs each; four times the
 *  input must cost less than eight times as much (a quadratic cost gives sixteen)
 */
static bool
ScalingWithPieces( void )
{
    Float64 small = 0.0, large = 0.0;
    bool ok = SeparateSquares( 4096, 3, small );
    ok = SeparateSquares( 16384, 3, large ) && ok;
    const Float64 ratio = large / ( small > 0.01 ? small : 0.01 );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: union of 4096 separate squares with holes " << small << " ms, 16384: " << large << " ms, ratio " << ratio );
    if ( !( ratio < 8.0 ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: 4 times the separate pieces cost " << ratio << " times as much" );
        ok = false;
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Two seeded star polygons of 10^3 vertices: union and intersection timing (reported), checked against the scanline reference */
static bool
LargeCase( UInt64 seed )
{
    QueryReference::CRandom random( seed );
    const UInt32 n = ScaledCount( 1000 );
    SOperands o;
    o.subjectFill = POLYGONFILL_POSITIVE;
    o.clipFill = POLYGONFILL_POSITIVE;
    o.subject.push_back( StarRing( random, n, 0, 0, 500000.0, 1000000.0 ) );
    o.clip.push_back( StarRing( random, n, 300000, 200000, 500000.0, 1000000.0 ) );
    CTPolygonClipper< Float64 > clipper;
    const GUCEF::vector< CTPoint2D< Float64 > > a = ToKind( o.subject[ 0 ], MakeFloat64 );
    const GUCEF::vector< CTPoint2D< Float64 > > b = ToKind( o.clip[ 0 ], MakeFloat64 );
    GUCEF::vector< GUCEF::vector< CTPoint2D< Float64 > > > rings;
    Float64 times[ 2 ] = { 0.0, 0.0 };
    for ( UInt32 k=0; k<2; ++k )
    {
        clipper.Clear();
        clipper.AddSubject( a );
        clipper.AddClip( b );
        const Float64 start = NowMs();
        clipper.Execute( 0 == k ? POLYGONBOOLEAN_UNION : POLYGONBOOLEAN_INTERSECTION, POLYGONFILL_POSITIVE, POLYGONFILL_POSITIVE, rings );
        times[ k ] = NowMs() - start;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: two star polygons of " << n << " vertices: union " << times[ 0 ] << " ms, intersection " << times[ 1 ] << " ms (" << rings.size() << " rings), seed " << seed );
    CTPolygonClipper< Float64 > checker;
    return CheckOperands( "Float64", "large", o, MakeFloat64, false, 4000, checker, false );
}

/*-------------------------------------------------------------------------*/

void
PerformPolygonBooleansTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "PolygonBooleans" );

    GUCEF_TESTFW_TESTCASE( "PolygonBooleans: grid cases (touching edges, shared vertices, holes, self-touching, fill rules): exact identities with the inputs, scanline reference, in every kind" )
    try
    {
        ASSERT_TRUE( ( FixedCases< Int32 >( "Int32", MakeInt32 ) ) );
        ASSERT_TRUE( ( FixedCases< Int64 >( "Int64", MakeInt64 ) ) );
        ASSERT_TRUE( ( FixedCases< Float64 >( "Float64", MakeFloat64 ) ) );
        ASSERT_TRUE( ( FixedCases< Float32 >( "Float32", MakeFloat32 ) ) );
        ASSERT_TRUE( ( FixedCases< TFixed32_32 >( "TFixed32_32", MakeFixed ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PolygonBooleans: seeded star polygons with holes: planar results, exact identities among the results, scanline reference within 1e-3" )
    try
    {
        ASSERT_TRUE( ( RandomCases< Int32 >( "Int32", 8501, ScaledCount( 150 ), MakeInt32, 100000.0, 40, 600, 1.0 ) ) );
        ASSERT_TRUE( ( RandomCases< Float64 >( "Float64", 8502, ScaledCount( 150 ), MakeFloat64, 1000000.0, 40, 600, 0.0 ) ) );
        ASSERT_TRUE( ( RandomCases< Float32 >( "Float32", 8503, ScaledCount( 60 ), MakeFloat32, 100000.0, 40, 600, 0.0 ) ) );
        ASSERT_TRUE( ( RandomCases< TFixed32_32 >( "TFixed32_32", 8504, ScaledCount( 60 ), MakeFixed, 100000.0, 40, 600, 0.0 ) ) );
        ASSERT_TRUE( ( RandomCases< Int32 >( "Int32 small", 8505, ScaledCount( 150 ), MakeInt32, 40.0, 12, 400, 1.0 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PolygonBooleans: invalid input, the grid range and scale, degenerate rings, collinear points" )
    try
    {
        ASSERT_TRUE( InvalidInput() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PolygonBooleans: tiny coordinates keep a finite automatic grid scale; regions touching themselves at a vertex" )
    try
    {
        ASSERT_TRUE( TinyCoordinates() );
        ASSERT_TRUE( TouchingRegions() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PolygonBooleans: many separate pieces with holes scale about as n log n (4096 against 16384, time ratio below 8)" )
    try
    {
        ASSERT_TRUE( ScalingWithPieces() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PolygonBooleans: 65536 and 262144 separate pieces with holes (timing reported; opt-in: GUCEF_TEST_MATH_BENCH=1)" )
    GUCEF_TESTFW_REQUIRE_ENV( bench, "GUCEF_TEST_MATH_BENCH" );
    GUCEF_TESTFW_SKIP_IF( bench != "1", "GUCEF_TEST_MATH_BENCH is not 1" );
    try
    {
        Float64 medium = 0.0, large = 0.0;
        ASSERT_TRUE( SeparateSquares( 65536, 1, medium ) );
        ASSERT_TRUE( SeparateSquares( 262144, 1, large ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PolygonBooleans: union of 65536 separate squares with holes " << medium << " ms, 262144: " << large << " ms" );
        // quadratic would be 16; caches make the largest sizes slower than n log n alone
        ASSERT_TRUE( large < 10.0 * medium );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PolygonBooleans: two polygons of 10^3 vertices (timing reported) against the scanline reference" )
    try
    {
        ASSERT_TRUE( LargeCase( 8506 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
