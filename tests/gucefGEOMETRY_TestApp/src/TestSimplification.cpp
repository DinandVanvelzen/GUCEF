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

#ifndef GUCEF_GEOMETRY_POLYLINESIMPLIFICATION_H
#include "gucefGEOMETRY_PolylineSimplification.h"
#define GUCEF_GEOMETRY_POLYLINESIMPLIFICATION_H
#endif /* GUCEF_GEOMETRY_POLYLINESIMPLIFICATION_H ? */


#include "TestSpatialReference.h"
#include "TestConvexReference.h"
#include "TestSimplification.h"

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

/** The distance from a point to a segment in Float64, computed by projection with clamping (its own code, not the one under test) */
static Float64
ReferenceDistance( const Float64* p, const Float64* a, const Float64* b, UInt32 dim )
{
    Float64 ab[ 3 ] = { 0, 0, 0 };
    Float64 ap[ 3 ] = { 0, 0, 0 };
    Float64 abab = 0.0;
    Float64 apab = 0.0;
    for ( UInt32 i=0; i<dim; ++i )
    {
        ab[ i ] = b[ i ] - a[ i ];
        ap[ i ] = p[ i ] - a[ i ];
        abab += ab[ i ] * ab[ i ];
        apab += ap[ i ] * ab[ i ];
    }
    Float64 t = abab > 0.0 ? apab / abab : 0.0;
    t = t < 0.0 ? 0.0 : ( t > 1.0 ? 1.0 : t );
    Float64 sum = 0.0;
    for ( UInt32 i=0; i<dim; ++i )
    {
        const Float64 d = p[ i ] - ( a[ i ] + ab[ i ] * t );
        sum += d * d;
    }
    return sqrt( sum );
}

/** The distance from a point to a polyline given by the kept indices */
static Float64
DistanceToPolyline( const GUCEF::vector< Float64 >& coordinates, UInt32 dim, UInt32 pointIndex, const GUCEF::vector< UInt32 >& kept )
{
    Float64 best = 1.0e300;
    for ( size_t k=0; k+1<kept.size(); ++k )
    {
        const Float64 d = ReferenceDistance( &coordinates[ (size_t) pointIndex * dim ], &coordinates[ (size_t) kept[ k ] * dim ], &coordinates[ (size_t) kept[ k + 1 ] * dim ], dim );
        best = d < best ? d : best;
    }
    if ( kept.size() == 1 )
        best = ReferenceDistance( &coordinates[ (size_t) pointIndex * dim ], &coordinates[ (size_t) kept[ 0 ] * dim ], &coordinates[ (size_t) kept[ 0 ] * dim ], dim );
    return best;
}

/*-------------------------------------------------------------------------*/

template < UInt32 Dim >
static bool
PolylineSweep( const char* name, UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    for ( UInt32 n=0; n<count; ++n )
    {
        const UInt32 pointCount = (UInt32) random.Index( 80 );
        GUCEF::vector< Float64 > coordinates;
        // a random walk with straight runs and a little noise
        Float64 position[ 3 ] = { 0, 0, 0 };
        Float64 step[ 3 ] = { 1, 0.3, 0.1 };
        for ( UInt32 i=0; i<pointCount; ++i )
        {
            if ( random.Chance( 0.15 ) )
                for ( UInt32 a=0; a<3; ++a )
                    step[ a ] = random.Uniform( -1.0, 1.0 );
            for ( UInt32 a=0; a<Dim; ++a )
            {
                position[ a ] += step[ a ] + ( random.Chance( 0.5 ) ? random.Uniform( -0.05, 0.05 ) : 0.0 );
                coordinates.push_back( position[ a ] );
            }
        }
        const Float64 epsilon = random.Chance( 0.1 ) ? 0.0 : random.Uniform( 0.0, 0.6 );
        GUCEF::vector< UInt32 > kept;
        UInt32 result;
        if ( 2 == Dim )
        {
            GUCEF::vector< CTPoint2D< Float64 > > points;
            for ( UInt32 i=0; i<pointCount; ++i )
                points.push_back( CTPoint2D< Float64 >( coordinates[ 2 * i ], coordinates[ 2 * i + 1 ] ) );
            result = SimplifyPolylineIndices( pointCount > 0 ? &points[ 0 ] : static_cast< const CTPoint2D< Float64 >* >( GUCEF_NULL ), pointCount, epsilon, kept );
        }
        else
        {
            GUCEF::vector< CTPoint3D< Float64 > > points;
            for ( UInt32 i=0; i<pointCount; ++i )
                points.push_back( CTPoint3D< Float64 >( coordinates[ 3 * i ], coordinates[ 3 * i + 1 ], coordinates[ 3 * i + 2 ] ) );
            result = SimplifyPolylineIndices( pointCount > 0 ? &points[ 0 ] : static_cast< const CTPoint3D< Float64 >* >( GUCEF_NULL ), pointCount, epsilon, kept );
        }
        if ( result != kept.size() )
            return false;
        if ( 0 == pointCount )
        {
            if ( !kept.empty() )
                return false;
            continue;
        }
        if ( kept.front() != 0 || kept.back() != pointCount - 1 )
            return false;
        for ( size_t k=1; k<kept.size(); ++k )
            if ( kept[ k ] <= kept[ k - 1 ] )
                return false;
        // no removed point is farther than epsilon from the simplified polyline
        size_t next = 0;
        for ( UInt32 i=0; i<pointCount; ++i )
        {
            if ( next < kept.size() && kept[ next ] == i )
            {
                ++next;
                continue;
            }
            const Float64 d = DistanceToPolyline( coordinates, Dim, i, kept );
            if ( d > epsilon * ( 1.0 + 1.0e-12 ) + 1.0e-12 )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the removed point " << i << " is " << d << " from the simplified polyline, epsilon " << epsilon << ", seed " << seed << " case " << n );
                return false;
            }
        }
        // the result does not depend on the way it is asked for
        if ( 2 == Dim )
        {
            CTPolyline2D< Float64 > in, out;
            for ( UInt32 i=0; i<pointCount; ++i )
                in.points.push_back( CTPoint2D< Float64 >( coordinates[ 2 * i ], coordinates[ 2 * i + 1 ] ) );
            if ( SimplifyPolyline( in, epsilon, out ) != kept.size() )
                return false;
            for ( size_t k=0; k<kept.size(); ++k )
                if ( !( out.points[ k ] == in.points[ kept[ k ] ] ) )
                    return false;
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Simplification: " << name << ": " << count << " polylines keep the end points and every removed point is within epsilon" );
    return true;
}

/*-------------------------------------------------------------------------*/

/** Epsilon 0 keeps every corner; epsilon above the sizes keeps the end points; the extreme cases */
static bool
SpecialCases( void )
{
    typedef CTPoint2D< Float64 > P;
    GUCEF::vector< UInt32 > kept;
    // collinear points on the x axis: exact zero distance, the interior ones go with epsilon 0
    GUCEF::vector< P > line;
    for ( Int32 i=0; i<10; ++i )
        line.push_back( P( i, 0 ) );
    if ( 2 != SimplifyPolylineIndices( &line[ 0 ], 10, 0.0, kept ) || kept[ 0 ] != 0 || kept[ 1 ] != 9 )
        return false;
    // a staircase: every corner is kept at epsilon 0 and none when epsilon is large
    GUCEF::vector< P > stairs;
    for ( Int32 i=0; i<8; ++i )
    {
        stairs.push_back( P( i, i ) );
        stairs.push_back( P( i + 1, i ) );
    }
    if ( 16 != SimplifyPolylineIndices( &stairs[ 0 ], 16, 0.0, kept ) )
        return false;
    if ( 2 != SimplifyPolylineIndices( &stairs[ 0 ], 16, 100.0, kept ) )
        return false;
    // the points on a circle are all corners
    GUCEF::vector< P > circle;
    for ( Int32 i=0; i<40; ++i )
        circle.push_back( P( cos( i * 0.05 ), sin( i * 0.05 ) ) );
    if ( 40 != SimplifyPolylineIndices( &circle[ 0 ], 40, 0.0, kept ) )
        return false;
    // a negative epsilon and not a number keep every point
    if ( 40 != SimplifyPolylineIndices( &circle[ 0 ], 40, -1.0, kept ) )
        return false;
    const Float64 zero = 0.0;
    if ( 40 != SimplifyPolylineIndices( &circle[ 0 ], 40, zero / zero, kept ) )
        return false;
    // one and two points
    if ( 1 != SimplifyPolylineIndices( &circle[ 0 ], 1, 0.5, kept ) || 2 != SimplifyPolylineIndices( &circle[ 0 ], 2, 0.5, kept ) )
        return false;
    // a closed ring: a square with extra points on its edges becomes the square
    CTPolygon2D< Float64 > ring, simplified;
    const Int32 sx[ 12 ] = { 0, 1, 2, 3, 4, 4, 4, 4, 3, 2, 1, 0 };
    const Int32 sy[ 12 ] = { 0, 0, 0, 0, 0, 1, 3, 4, 4, 4, 4, 4 };
    for ( UInt32 i=0; i<12; ++i )
        ring.points.push_back( P( sx[ i ], sy[ i ] ) );
    const UInt32 size = SimplifyPolygon( ring, 0.0, simplified );
    if ( size != simplified.points.size() || size > 6 || size < 4 )
        return false;
    // the corners of the square survive
    UInt32 corners = 0;
    for ( size_t i=0; i<simplified.points.size(); ++i )
    {
        const P& p = simplified.points[ i ];
        if ( ( p.x == 0 || p.x == 4 ) && ( p.y == 0 || p.y == 4 ) )
            ++corners;
    }
    if ( 4 != corners )
        return false;
    // a small ring is kept as it is
    CTPolygon2D< Float64 > triangle, same;
    triangle.points.push_back( P( 0, 0 ) );
    triangle.points.push_back( P( 1, 0 ) );
    triangle.points.push_back( P( 0, 1 ) );
    if ( 3 != SimplifyPolygon( triangle, 5.0, same ) )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/** Random rings: every removed vertex is within epsilon of the simplified ring */
static bool
PolygonSweep( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    for ( UInt32 n=0; n<count; ++n )
    {
        const UInt32 pointCount = 4 + (UInt32) random.Index( 80 );
        CTPolygon2D< Float64 > ring;
        GUCEF::vector< Float64 > coordinates;
        for ( UInt32 i=0; i<pointCount; ++i )
        {
            const Float64 angle = 6.2831853 * i / pointCount;
            const Float64 radius = 10.0 + random.Uniform( -0.4, 0.4 );
            ring.points.push_back( CTPoint2D< Float64 >( radius * cos( angle ), radius * sin( angle ) ) );
            coordinates.push_back( ring.points.back().x );
            coordinates.push_back( ring.points.back().y );
        }
        const Float64 epsilon = random.Uniform( 0.0, 1.0 );
        GUCEF::vector< UInt32 > kept;
        SimplifyPolygonIndices( &ring.points[ 0 ], pointCount, epsilon, kept );
        if ( kept.size() < 2 || kept.size() > pointCount )
            return false;
        // the ring closes: the segments of the kept points including the last to the first
        GUCEF::vector< UInt32 > closed = kept;
        closed.push_back( kept[ 0 ] );
        for ( UInt32 i=0; i<pointCount; ++i )
        {
            const Float64 d = DistanceToPolyline( coordinates, 2, i, closed );
            if ( d > epsilon * ( 1.0 + 1.0e-12 ) + 1.0e-12 )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Simplification: ring point " << i << " is " << d << " from the simplified ring, epsilon " << epsilon << ", seed " << seed << " case " << n );
                return false;
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSimplificationTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Simplification" );

    GUCEF_TESTFW_TESTCASE( "Simplification: Douglas-Peucker keeps the end points and no removed point is farther than epsilon, in 2D and 3D" )
    try
    {
        ASSERT_TRUE( ( PolylineSweep< 2 >( "2D", 7101, ScaledCount( 1500 ) ) ) );
        ASSERT_TRUE( ( PolylineSweep< 3 >( "3D", 7102, ScaledCount( 1500 ) ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Simplification: epsilon 0 keeps every non-collinear point, large epsilon keeps the end points, negative and not a number keep all, polygons" )
    try
    {
        ASSERT_TRUE( SpecialCases() );
        ASSERT_TRUE( PolygonSweep( 7103, ScaledCount( 800 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
