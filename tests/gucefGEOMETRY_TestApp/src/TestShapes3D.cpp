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

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#include "TestGeometryReference.h"
#include "TestShapes3D.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/* near and far are macros of the Windows headers, undefine them so they can be used as identifiers */
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

using namespace GUCEF;
using namespace GUCEF::MATH;
using namespace GUCEF::GEOMETRY;
using namespace GeometryReference;

typedef TFloat64Point3D P3;
typedef TFloat64Vector3D V3;
typedef TFloat64DirectionVector3D D3;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
NearPoint( const P3& p, Float64 x, Float64 y, Float64 z, Float64 tolerance )
{
    return Near( p.x, x, tolerance ) && Near( p.y, y, tolerance ) && Near( p.z, z, tolerance );
}

/*-------------------------------------------------------------------------*/

static bool
NearVector( const V3& v, Float64 x, Float64 y, Float64 z, Float64 tolerance )
{
    return Near( v.x, x, tolerance ) && Near( v.y, y, tolerance ) && Near( v.z, z, tolerance );
}

/*-------------------------------------------------------------------------*/

static SVec3
ToRef( const P3& p )
{
    return SVec3( p.x, p.y, p.z );
}

/*-------------------------------------------------------------------------*/

static D3
RandomDirection( CSeededSequence& random )
{
    for ( ;; )
    {
        V3 v( random.NextFloat64( -1, 1 ), random.NextFloat64( -1, 1 ), random.NextFloat64( -1, 1 ) );
        D3 d;
        if ( v.GetSquaredLength() > 0.01 && D3::TryCreate( v, d ) )
            return d;
    }
}

/*-------------------------------------------------------------------------*/

static P3
RandomPoint( CSeededSequence& random, Float64 range )
{
    return P3( random.NextFloat64( -range, range ), random.NextFloat64( -range, range ), random.NextFloat64( -range, range ) );
}

/*-------------------------------------------------------------------------*/

/** The first distance along the ray at which the solid contains the point, by marching and bisection on Contains only */
template < typename TShape >
static bool
MarchToSolid( const TShape& shape, const CTRay3D< Float64 >& ray, Float64 maxDistance, Float64& distance )
{
    const Float64 step = 0.01;
    if ( shape.Contains( ray.origin ) )
    {
        distance = 0;
        return true;
    }
    Float64 previous = 0;
    for ( Float64 t=step; t<=maxDistance + step; t+=step )
    {
        if ( shape.Contains( ray.GetPoint( t ) ) )
        {
            Float64 lo = previous;
            Float64 hi = t;
            for ( UInt32 i=0; i<60; ++i )
            {
                Float64 mid = 0.5 * ( lo + hi );
                if ( shape.Contains( ray.GetPoint( mid ) ) )
                    hi = mid;
                else
                    lo = mid;
            }
            distance = hi;
            return true;
        }
        previous = t;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** Compares RayCast of a solid with marching on Contains for 'count' seeded rays, returns the number of disagreements */
template < typename TShape >
static UInt32
CountRayCastDisagreements( const TShape& shape, UInt64 seed, UInt32 count, UInt32& hits )
{
    CSeededSequence random( seed );
    UInt32 bad = 0;
    hits = 0;
    const Float64 maxDistance = 20;
    for ( UInt32 i=0; i<count; ++i )
    {
        // every other ray is aimed at a random point near the shapes, so that many rays hit
        const P3 origin = RandomPoint( random, 6 );
        D3 direction = RandomDirection( random );
        if ( 0 == i % 2 )
            D3::TryCreate( RandomPoint( random, 2 ) - origin, direction );
        CTRay3D< Float64 > ray( origin, direction );
        SRayCastResult< Float64 > result;
        const bool cast = shape.RayCast( ray, maxDistance, result );
        Float64 marched = 0;
        const bool found = MarchToSolid( shape, ray, maxDistance, marched );
        if ( found )
        {
            if ( !cast || !Near( result.distance, marched, 1.0e-6 ) )
                ++bad;
            else if ( 0 == result.distance && !result.StartedInside() )
                ++bad;
            else if ( !Near( result.point.x, ray.GetPoint( result.distance ).x, 1.0e-9 ) )
                ++bad;
            else if ( !Near( result.normal.GetLength(), 1.0, 1.0e-9 ) )
                ++bad;
        }
        else if ( cast )
        {
            /* a graze that the marching step skipped: the hit must lie on the solid */
            if ( shape.GetDistance( result.point ) > 1.0e-6 )
                ++bad;
        }
        if ( cast )
            ++hits;
        if ( bad > 0 && bad < 4 && ( found != cast ) )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Shapes3D: ray cast disagreement, seed " << seed << " ray " << i );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Shapes3D: ray cast sweep seed " << seed << ": " << hits << " hits of " << count << ", " << bad << " disagreements" );
    return bad;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformShapes3DTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Shapes3D" );

    GUCEF_TESTFW_TESTCASE( "Shapes3D: triangle closest point in each of the 7 Voronoi regions" )
    try
    {
        const CTTriangle3D< Float64 > t( P3( 0, 0, 0 ), P3( 4, 0, 0 ), P3( 0, 4, 0 ) );
        ASSERT_TRUE( NearPoint( t.ClosestPoint( P3( -1, -1, 2 ) ), 0, 0, 0, 1e-15 ) );      // vertex a
        ASSERT_TRUE( NearPoint( t.ClosestPoint( P3( 6, -1, 1 ) ), 4, 0, 0, 1e-15 ) );       // vertex b
        ASSERT_TRUE( NearPoint( t.ClosestPoint( P3( -1, 6, -1 ) ), 0, 4, 0, 1e-15 ) );      // vertex c
        ASSERT_TRUE( NearPoint( t.ClosestPoint( P3( 2, -3, 1 ) ), 2, 0, 0, 1e-15 ) );       // edge ab
        ASSERT_TRUE( NearPoint( t.ClosestPoint( P3( -2, 2, 5 ) ), 0, 2, 0, 1e-15 ) );       // edge ac
        ASSERT_TRUE( NearPoint( t.ClosestPoint( P3( 4, 4, 1 ) ), 2, 2, 0, 1e-15 ) );        // edge bc
        ASSERT_TRUE( NearPoint( t.ClosestPoint( P3( 1, 1, 3 ) ), 1, 1, 0, 1e-15 ) );        // face, in front
        ASSERT_TRUE( NearPoint( t.ClosestPoint( P3( 1, 2, -7 ) ), 1, 2, 0, 1e-15 ) );       // face, behind
        ASSERT_TRUE( Near( t.GetDistance( P3( 1, 1, 3 ) ), 3, 1e-15 ) );
        ASSERT_TRUE( Near( t.GetSquaredDistance( P3( -1, -1, 2 ) ), 6, 1e-14 ) );
        ASSERT_TRUE( t.Contains( P3( 1, 1, 0 ) ) && !t.Contains( P3( 1, 1, 0.5 ) ) && t.Contains( P3( 1, 1, 0.5 ), 0.5 ) );

        // the same triangle in Float32
        const CTTriangle3D< Float32 > t32( TFloat32Point3D( 0, 0, 0 ), TFloat32Point3D( 4, 0, 0 ), TFloat32Point3D( 0, 4, 0 ) );
        const TFloat32Point3D c32 = t32.ClosestPoint( TFloat32Point3D( 4, 4, 1 ) );
        ASSERT_TRUE( 2.0f == c32.x && 2.0f == c32.y && 0.0f == c32.z );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes3D: degenerate and zero area triangles give the closest point on the segment or point" )
    try
    {
        const CTTriangle3D< Float64 > collinear( P3( 0, 0, 0 ), P3( 1, 0, 0 ), P3( 3, 0, 0 ) );
        ASSERT_TRUE( collinear.IsDegenerate() );
        ASSERT_TRUE( 0.0 == collinear.GetArea() );
        ASSERT_TRUE( NearPoint( collinear.ClosestPoint( P3( 2, 1, 0 ) ), 2, 0, 0, 1e-15 ) );
        ASSERT_TRUE( NearPoint( collinear.ClosestPoint( P3( 5, 1, 0 ) ), 3, 0, 0, 1e-15 ) );
        ASSERT_TRUE( NearPoint( collinear.ClosestPoint( P3( -2, 0, 4 ) ), 0, 0, 0, 1e-15 ) );
        const CTTriangle3D< Float64 > point( P3( 1, 1, 1 ), P3( 1, 1, 1 ), P3( 1, 1, 1 ) );
        ASSERT_TRUE( point.IsDegenerate() );
        ASSERT_TRUE( NearPoint( point.ClosestPoint( P3( 4, 5, 1 ) ), 1, 1, 1, 0 ) );
        ASSERT_TRUE( Near( point.GetDistance( P3( 4, 5, 1 ) ), 5, 1e-15 ) );
        const CTTriangle3D< Float64 > twoSame( P3( 0, 0, 0 ), P3( 0, 0, 0 ), P3( 0, 2, 0 ) );
        ASSERT_TRUE( twoSame.IsDegenerate() );
        ASSERT_TRUE( NearPoint( twoSame.ClosestPoint( P3( 1, 1, 0 ) ), 0, 1, 0, 1e-15 ) );

        const CTTriangle3D< Float64 > proper( P3( 0, 0, 0 ), P3( 4, 0, 0 ), P3( 0, 4, 0 ) );
        ASSERT_FALSE( proper.IsDegenerate() );
        D3 n;
        ASSERT_TRUE( proper.TryGetNormal( n ) && n == D3::UnitZ() );
        ASSERT_FALSE( collinear.TryGetNormal( n ) );
        ASSERT_TRUE( collinear.GetNormal() == D3::Forward() );
        V3 barycentric;
        ASSERT_FALSE( collinear.TryGetBarycentric( P3( 1, 0, 0 ), barycentric ) );
        ASSERT_TRUE( 0.0 == collinear.GetBarycentric( P3( 1, 0, 0 ) ).x );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes3D: triangle closest point, 10000 seeded queries against Float64 references" )
    try
    {
        const UInt64 seed = 20261114;
        CSeededSequence random( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<10000; ++i )
        {
            const Float64 range = 10;
            const P3 a = RandomPoint( random, range );
            const P3 b = RandomPoint( random, range );
            // every 7th triangle is thin, every 11th has two nearly equal vertices
            P3 c = RandomPoint( random, range );
            if ( 0 == i % 7 )
                c = a + ( b - a ) * random.NextFloat64( -1, 2 ) + V3( random.NextFloat64( -1e-3, 1e-3 ), random.NextFloat64( -1e-3, 1e-3 ), 0 );
            else if ( 0 == i % 11 )
                c = a + V3( random.NextFloat64( -1e-6, 1e-6 ), random.NextFloat64( -1e-6, 1e-6 ), random.NextFloat64( -1e-6, 1e-6 ) );
            const P3 q = RandomPoint( random, 2 * range );
            const CTTriangle3D< Float64 > t( a, b, c );
            const P3 closest = t.ClosestPoint( q );
            const Float64 distance = ( q - closest ).GetLength();
            const Float64 analytic = TriangleDistance( ToRef( q ), ToRef( a ), ToRef( b ), ToRef( c ) );
            const Float64 sampled = TriangleSampledDistance( ToRef( q ), ToRef( a ), ToRef( b ), ToRef( c ), 24 );
            // the closest point is on the triangle: its distance to the triangle is 0
            const Float64 onTriangle = TriangleDistance( ToRef( closest ), ToRef( a ), ToRef( b ), ToRef( c ) );
            if ( !Near( distance, analytic, 1.0e-9 ) || distance > sampled + 1.0e-9 || onTriangle > 1.0e-9 )
            {
                ++failures;
                if ( failures < 4 )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Shapes3D: triangle closest point mismatch, seed " << seed << " query " << i << ": " << distance << " vs " << analytic << " sampled " << sampled );
            }
        }
        ASSERT_TRUE( 0 == failures );

        // Float32 inputs against the Float64 reference of the same values
        UInt32 failures32 = 0;
        for ( UInt32 i=0; i<10000; ++i )
        {
            const Float32 range = 10;
            TFloat32Point3D a( (Float32) random.NextFloat64( -range, range ), (Float32) random.NextFloat64( -range, range ), (Float32) random.NextFloat64( -range, range ) );
            TFloat32Point3D b( (Float32) random.NextFloat64( -range, range ), (Float32) random.NextFloat64( -range, range ), (Float32) random.NextFloat64( -range, range ) );
            TFloat32Point3D c( (Float32) random.NextFloat64( -range, range ), (Float32) random.NextFloat64( -range, range ), (Float32) random.NextFloat64( -range, range ) );
            TFloat32Point3D q( (Float32) random.NextFloat64( -2 * range, 2 * range ), (Float32) random.NextFloat64( -2 * range, 2 * range ), (Float32) random.NextFloat64( -2 * range, 2 * range ) );
            const TFloat32Point3D closest = CTTriangle3D< Float32 >( a, b, c ).ClosestPoint( q );
            const Float64 distance = (Float64) ( q - closest ).GetLength();
            const Float64 analytic = TriangleDistance( SVec3( q.x, q.y, q.z ), SVec3( a.x, a.y, a.z ), SVec3( b.x, b.y, b.z ), SVec3( c.x, c.y, c.z ) );
            if ( !Near( distance, analytic, 3.0e-5 ) )         /* about 10 units in the last place of values up to 40 */
                ++failures32;
        }
        ASSERT_TRUE( 0 == failures32 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes3D: triangle area, barycentric coordinates, bounds, support and ray cast" )
    try
    {
        const CTTriangle3D< Float64 > t( P3( 0, 0, 0 ), P3( 4, 0, 0 ), P3( 0, 4, 0 ) );
        ASSERT_TRUE( Near( t.GetArea(), 8, 1e-15 ) );
        ASSERT_TRUE( NearPoint( t.GetCentroid(), 4.0 / 3, 4.0 / 3, 0, 1e-15 ) );
        V3 bc;
        ASSERT_TRUE( t.TryGetBarycentric( P3( 1, 1, 5 ), bc ) );
        ASSERT_TRUE( NearVector( bc, 0.5, 0.25, 0.25, 1e-15 ) );
        ASSERT_TRUE( NearVector( t.GetBarycentric( P3( 4, 0, 0 ) ), 0, 1, 0, 1e-15 ) );
        const CTAxisAlignedBox3D< Float64 > bounds = t.GetBounds();
        ASSERT_TRUE( bounds.min == P3( 0, 0, 0 ) && bounds.max == P3( 4, 4, 0 ) );
        ASSERT_TRUE( t.GetSupport( V3( 1, 0, 0 ) ) == P3( 4, 0, 0 ) && t.GetSupport( V3( 0, 1, 0 ) ) == P3( 0, 4, 0 ) && t.GetSupport( V3( -1, -1, 0 ) ) == P3( 0, 0, 0 ) );

        SRayCastResult< Float64 > hit;
        ASSERT_FALSE( t.RayCast( CTRay3D< Float64 >( P3( 1, 1, 5 ), D3::Down() ), 100, hit ) );                // Down is -Y: parallel to the triangle plane
        ASSERT_TRUE( t.RayCast( CTRay3D< Float64 >( P3( 1, 1, 5 ), D3::Forward() ), 100, hit ) );          // Forward is -Z
        ASSERT_TRUE( Near( hit.distance, 5, 1e-12 ) && Near( hit.u, 0.25, 1e-12 ) && Near( hit.v, 0.25, 1e-12 ) );
        ASSERT_TRUE( NearVector( hit.normal, 0, 0, 1, 1e-12 ) && NearPoint( hit.point, 1, 1, 0, 1e-12 ) && !hit.StartedInside() );
        ASSERT_TRUE( t.RayCast( CTRay3D< Float64 >( P3( 1, 1, -5 ), D3::Back() ), 100, hit ) );            // Back is +Z, from below
        ASSERT_TRUE( Near( hit.distance, 5, 1e-12 ) && NearVector( hit.normal, 0, 0, -1, 1e-12 ) );
        ASSERT_FALSE( t.RayCast( CTRay3D< Float64 >( P3( 3, 3, 5 ), D3::Forward() ), 100, hit ) );          // beside the hypotenuse
        ASSERT_TRUE( t.RayCast( CTRay3D< Float64 >( P3( 2, 2, 5 ), D3::Forward() ), 100, hit ) );          // exactly on it
        ASSERT_FALSE( t.RayCast( CTRay3D< Float64 >( P3( 1, 1, 5 ), D3::Forward() ), 4, hit ) );            // too far
        ASSERT_FALSE( t.RayCast( CTRay3D< Float64 >( P3( 1, 1, 5 ), D3::Back() ), 100, hit ) );             // behind
        ASSERT_FALSE( t.RayCast( CTRay3D< Float64 >( P3( 1, 1, 5 ), D3::Right() ), 100, hit ) );            // parallel
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes3D: plane construction, signed distance, projection and mirror" )
    try
    {
        const CTPlane< Float64 > xy = CTPlane< Float64 >::FromPoints( P3( 0, 0, 0 ), P3( 1, 0, 0 ), P3( 0, 1, 0 ) );
        ASSERT_TRUE( xy.normal == D3::UnitZ() && 0.0 == xy.offset );
        ASSERT_TRUE( Near( xy.GetSignedDistance( P3( 5, 6, 3 ) ), 3, 1e-15 ) && Near( xy.GetSignedDistance( P3( 5, 6, -2 ) ), -2, 1e-15 ) );
        ASSERT_TRUE( Near( xy.GetDistance( P3( 1, 1, -4 ) ), 4, 1e-15 ) && Near( xy.GetSquaredDistance( P3( 1, 1, -4 ) ), 16, 1e-14 ) );
        ASSERT_TRUE( NearPoint( xy.Project( P3( 1, 2, 5 ) ), 1, 2, 0, 1e-15 ) && NearPoint( xy.ClosestPoint( P3( 1, 2, -5 ) ), 1, 2, 0, 1e-15 ) );
        ASSERT_TRUE( NearPoint( xy.Mirror( P3( 1, 2, 5 ) ), 1, 2, -5, 1e-15 ) );
        ASSERT_TRUE( NearPoint( xy.Mirror( xy.Mirror( P3( 1.3, -2.7, 5.1 ) ) ), 1.3, -2.7, 5.1, 1e-14 ) );
        ASSERT_TRUE( xy.Contains( P3( 9, 9, 0 ) ) && !xy.Contains( P3( 9, 9, 0.1 ) ) && xy.Contains( P3( 9, 9, 0.1 ), 0.2 ) );

        // a plane that does not pass through the origin: n . p = d
        const CTPlane< Float64 > offset = CTPlane< Float64 >::FromPointNormal( P3( 7, 8, 3 ), D3::UnitZ() );
        ASSERT_TRUE( Near( offset.offset, 3, 1e-15 ) && Near( offset.GetSignedDistance( P3( 0, 0, 5 ) ), 2, 1e-15 ) );
        ASSERT_TRUE( Near( offset.Flipped().GetSignedDistance( P3( 0, 0, 5 ) ), -2, 1e-15 ) );

        // winding: clockwise points flip the normal
        ASSERT_TRUE( CTPlane< Float64 >::FromPoints( P3( 0, 0, 0 ), P3( 0, 1, 0 ), P3( 1, 0, 0 ) ).normal == -D3::UnitZ() );

        CTPlane< Float64 > untouched = offset;
        ASSERT_FALSE( CTPlane< Float64 >::TryFromPoints( P3( 0, 0, 0 ), P3( 1, 1, 1 ), P3( 2, 2, 2 ), untouched ) );
        ASSERT_TRUE( untouched == offset );

        // reflections of a direction and a vector at the x = 0 plane
        const CTPlane< Float64 > yz = CTPlane< Float64 >::FromPointNormal( P3( 0, 0, 0 ), D3::UnitX() );
        const D3 diagonal( V3( 1, 1, 0 ) );
        const D3 mirrored = yz.Mirror( diagonal );
        ASSERT_TRUE( NearVector( mirrored.AsVector(), -sqrt( 0.5 ), sqrt( 0.5 ), 0, 1e-15 ) );
        ASSERT_TRUE( NearVector( yz.Mirror( V3( 2, 3, 4 ) ), -2, 3, 4, 1e-15 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes3D: plane intersections with lines, planes and rays" )
    try
    {
        const CTPlane< Float64 > px = CTPlane< Float64 >::FromPointNormal( P3( 0, 0, 0 ), D3::UnitX() );
        const CTPlane< Float64 > py = CTPlane< Float64 >::FromPointNormal( P3( 0, 0, 0 ), D3::UnitY() );
        const CTPlane< Float64 > pz = CTPlane< Float64 >::FromPointNormal( P3( 0, 0, 0 ), D3::UnitZ() );
        P3 point( 9, 9, 9 );
        ASSERT_TRUE( CTPlane< Float64 >::IntersectPlanes( px, py, pz, point ) && NearPoint( point, 0, 0, 0, 1e-15 ) );
        const CTPlane< Float64 > p1 = CTPlane< Float64 >::FromPointNormal( P3( 1, 0, 0 ), D3::UnitX() );
        const CTPlane< Float64 > p2 = CTPlane< Float64 >::FromPointNormal( P3( 0, 2, 0 ), D3::UnitY() );
        const CTPlane< Float64 > p3 = CTPlane< Float64 >::FromPointNormal( P3( 0, 0, 3 ), D3::UnitZ() );
        ASSERT_TRUE( CTPlane< Float64 >::IntersectPlanes( p1, p2, p3, point ) && NearPoint( point, 1, 2, 3, 1e-14 ) );
        point = P3( 9, 9, 9 );
        ASSERT_FALSE( CTPlane< Float64 >::IntersectPlanes( px, p1, pz, point ) );                  // two parallel planes
        ASSERT_TRUE( NearPoint( point, 9, 9, 9, 0 ) );

        // parallel planes report no line, and so do equal planes
        CTLine3D< Float64 > line;
        ASSERT_FALSE( px.IntersectPlane( p1, line ) );
        ASSERT_FALSE( px.IntersectPlane( px, line ) );
        ASSERT_FALSE( px.IntersectPlane( px.Flipped(), line ) );

        // two planes meeting in a line: both contain its points
        ASSERT_TRUE( p1.IntersectPlane( p2, line ) );
        for ( Float64 t=-3; t<=3; t+=1.5 )
        {
            ASSERT_TRUE( Near( p1.GetSignedDistance( line.GetPoint( t ) ), 0, 1e-14 ) && Near( p2.GetSignedDistance( line.GetPoint( t ) ), 0, 1e-14 ) );
        }
        ASSERT_TRUE( Near( fabs( line.direction.GetZ() ), 1, 1e-15 ) );
        // slanted planes
        const CTPlane< Float64 > s1 = CTPlane< Float64 >::FromPointNormal( P3( 1, 2, 3 ), D3( V3( 1, 2, 3 ) ) );
        const CTPlane< Float64 > s2 = CTPlane< Float64 >::FromPointNormal( P3( 1, 2, 3 ), D3( V3( -2, 1, 5 ) ) );
        ASSERT_TRUE( s1.IntersectPlane( s2, line ) );
        for ( Float64 t=-3; t<=3; t+=1.0 )
        {
            ASSERT_TRUE( Near( s1.GetSignedDistance( line.GetPoint( t ) ), 0, 1e-13 ) && Near( s2.GetSignedDistance( line.GetPoint( t ) ), 0, 1e-13 ) );
        }

        // line and ray
        ASSERT_TRUE( p3.IntersectLine( CTLine3D< Float64 >( P3( 1, 1, 7 ), D3::Back() ), point ) && NearPoint( point, 1, 1, 3, 1e-14 ) );
        P3 unchanged( 4, 4, 4 );
        ASSERT_FALSE( p3.IntersectLine( CTLine3D< Float64 >( P3( 1, 1, 7 ), D3::Right() ), unchanged ) );
        ASSERT_TRUE( NearPoint( unchanged, 4, 4, 4, 0 ) );

        SRayCastResult< Float64 > hit;
        ASSERT_TRUE( pz.RayCast( CTRay3D< Float64 >( P3( 1, 2, 5 ), D3::Forward() ), 10, hit ) && Near( hit.distance, 5, 1e-14 ) && NearVector( hit.normal, 0, 0, 1, 0 ) );
        ASSERT_TRUE( pz.RayCast( CTRay3D< Float64 >( P3( 1, 2, -5 ), D3::Back() ), 10, hit ) && Near( hit.distance, 5, 1e-14 ) && NearVector( hit.normal, 0, 0, -1, 0 ) );
        ASSERT_FALSE( pz.RayCast( CTRay3D< Float64 >( P3( 1, 2, 5 ), D3::Back() ), 10, hit ) );
        ASSERT_FALSE( pz.RayCast( CTRay3D< Float64 >( P3( 1, 2, 5 ), D3::Right() ), 10, hit ) );
        ASSERT_FALSE( pz.RayCast( CTRay3D< Float64 >( P3( 1, 2, 5 ), D3::Forward() ), 4, hit ) );
        ASSERT_TRUE( pz.RayCast( CTRay3D< Float64 >( P3( 1, 2, 0 ), D3::Forward() ), 4, hit ) && 0.0 == hit.distance );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes3D: sphere contains, closest point, distance, measures and ray cast" )
    try
    {
        const CTSphere< Float64 > s( P3( 1, 2, 3 ), 2 );
        ASSERT_TRUE( s.Contains( P3( 1, 2, 5 ) ) && !s.Contains( P3( 1, 2, 5.001 ) ) && s.Contains( P3( 1, 2, 3 ) ) );
        ASSERT_TRUE( NearPoint( s.ClosestPoint( P3( 1, 2, 10 ) ), 1, 2, 5, 1e-14 ) && NearPoint( s.ClosestPoint( P3( 1, 3, 3 ) ), 1, 3, 3, 0 ) );
        ASSERT_TRUE( Near( s.GetDistance( P3( 1, 2, 10 ) ), 5, 1e-14 ) && 0.0 == s.GetDistance( P3( 1, 2, 4 ) ) && Near( s.GetSquaredDistance( P3( 1, 2, 10 ) ), 25, 1e-13 ) );
        ASSERT_TRUE( Near( s.GetSignedDistance( P3( 1, 2, 3 ) ), -2, 1e-15 ) && Near( s.GetSignedDistance( P3( 1, 2, 6 ) ), 1, 1e-15 ) );
        ASSERT_TRUE( NearPoint( s.ClosestPointOnSurface( P3( 1, 2, 4 ) ), 1, 2, 5, 1e-14 ) && NearPoint( s.ClosestPointOnSurface( P3( 1, 2, 3 ) ), 3, 2, 3, 1e-15 ) );
        ASSERT_TRUE( Near( s.GetVolume(), 4.0 / 3.0 * 3.14159265358979323846 * 8, 1e-12 ) && Near( s.GetSurfaceArea(), 16 * 3.14159265358979323846, 1e-12 ) );
        ASSERT_TRUE( NearPoint( s.GetSupport( V3( 0, 0, 5 ) ), 1, 2, 5, 1e-15 ) && NearPoint( s.GetSupport( V3( 0, 0, 0 ) ), 3, 2, 3, 0 ) );
        const CTAxisAlignedBox3D< Float64 > bounds = s.GetBounds();
        ASSERT_TRUE( bounds.min == P3( -1, 0, 1 ) && bounds.max == P3( 3, 4, 5 ) );
        ASSERT_TRUE( s.Intersects( CTSphere< Float64 >( P3( 5, 2, 3 ), 2 ) ) && !s.Intersects( CTSphere< Float64 >( P3( 5.1, 2, 3 ), 2 ) ) );
        ASSERT_FALSE( CTSphere< Float64 >( P3( 0, 0, 0 ), -1 ).Contains( P3( 0, 0, 0 ) ) );

        const CTSphere< Float64 > unit( P3( 0, 0, 0 ), 2 );
        SRayCastResult< Float64 > hit;
        ASSERT_TRUE( unit.RayCast( CTRay3D< Float64 >( P3( -5, 0, 0 ), D3::Right() ), 100, hit ) );
        ASSERT_TRUE( Near( hit.distance, 3, 1e-14 ) && NearVector( hit.normal, -1, 0, 0, 1e-14 ) && NearPoint( hit.point, -2, 0, 0, 1e-14 ) && !hit.StartedInside() );
        ASSERT_TRUE( unit.RayCast( CTRay3D< Float64 >( P3( -5, 2, 0 ), D3::Right() ), 100, hit ) && Near( hit.distance, 5, 1e-12 ) );       // grazing
        ASSERT_FALSE( unit.RayCast( CTRay3D< Float64 >( P3( -5, 2.001, 0 ), D3::Right() ), 100, hit ) );
        ASSERT_FALSE( unit.RayCast( CTRay3D< Float64 >( P3( 5, 0, 0 ), D3::Right() ), 100, hit ) );                                          // behind
        ASSERT_FALSE( unit.RayCast( CTRay3D< Float64 >( P3( -5, 0, 0 ), D3::Right() ), 2.9, hit ) );                                           // too far
        ASSERT_TRUE( unit.RayCast( CTRay3D< Float64 >( P3( 0.5, 0, 0 ), D3::Right() ), 100, hit ) );
        ASSERT_TRUE( 0.0 == hit.distance && hit.StartedInside() && NearVector( hit.normal, -1, 0, 0, 0 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes3D: axis aligned box contains, closest point, distance, FromPoints, Merged and the empty box" )
    try
    {
        const CTAxisAlignedBox3D< Float64 > b( P3( -1, -2, -3 ), P3( 3, 4, 5 ) );
        ASSERT_TRUE( b.Contains( P3( 0, 0, 0 ) ) && b.Contains( P3( 3, 4, 5 ) ) && b.Contains( P3( -1, -2, -3 ) ) && !b.Contains( P3( 3.001, 0, 0 ) ) );
        ASSERT_TRUE( NearPoint( b.ClosestPoint( P3( 5, 0, 0 ) ), 3, 0, 0, 0 ) && NearPoint( b.ClosestPoint( P3( 1, 1, 1 ) ), 1, 1, 1, 0 ) );
        ASSERT_TRUE( Near( b.GetDistance( P3( 5, 6, -7 ) ), sqrt( 24.0 ), 1e-14 ) && Near( b.GetSquaredDistance( P3( 5, 6, -7 ) ), 24, 1e-13 ) && 0.0 == b.GetDistance( P3( 0, 0, 0 ) ) );
        ASSERT_TRUE( Near( b.GetVolume(), 192, 1e-12 ) && Near( b.GetSurfaceArea(), 208, 1e-12 ) );
        ASSERT_TRUE( b.GetCentre() == P3( 1, 1, 1 ) && b.GetExtents() == V3( 4, 6, 8 ) && b.GetHalfExtents() == V3( 2, 3, 4 ) );
        ASSERT_TRUE( b.GetCorner( 0 ) == b.min && b.GetCorner( 7 ) == b.max && b.GetCorner( 1 ) == P3( 3, -2, -3 ) && b.GetCorner( 2 ) == P3( -1, 4, -3 ) && b.GetCorner( 4 ) == P3( -1, -2, 5 ) );
        ASSERT_TRUE( b.GetSupport( V3( 1, -1, 1 ) ) == P3( 3, -2, 5 ) && b.GetBounds() == b );

        const P3 points[ 4 ] = { P3( 1, 2, 3 ), P3( -4, 5, 0 ), P3( 2, -6, 7 ), P3( 0, 0, 0 ) };
        const CTAxisAlignedBox3D< Float64 > fromPoints = CTAxisAlignedBox3D< Float64 >::FromPoints( points, 4 );
        ASSERT_TRUE( fromPoints.min == P3( -4, -6, 0 ) && fromPoints.max == P3( 2, 5, 7 ) );
        ASSERT_TRUE( CTAxisAlignedBox3D< Float64 >::FromPoints( points, 0 ).IsEmpty() );

        // the empty box is the identity of Merged
        const CTAxisAlignedBox3D< Float64 > empty;
        ASSERT_TRUE( empty.IsEmpty() && CTAxisAlignedBox3D< Float64 >::Empty().IsEmpty() );
        ASSERT_TRUE( empty.Merged( b ) == b && b.Merged( empty ) == b && empty.Merged( empty ).IsEmpty() );
        ASSERT_FALSE( empty.Contains( P3( 0, 0, 0 ) ) );
        ASSERT_TRUE( 0.0 == empty.GetVolume() && 0.0 == empty.GetSurfaceArea() && V3() == empty.GetExtents() && P3() == empty.GetCentre() );
        ASSERT_TRUE( empty.GetDistance( P3( 1, 2, 3 ) ) == CTNumericTraits< Float64 >::Max() && empty.ClosestPoint( P3( 1, 2, 3 ) ) == P3( 1, 2, 3 ) );
        ASSERT_TRUE( b.Contains( empty ) && !empty.Contains( b ) && !empty.Intersects( b ) && !b.Intersects( empty ) );

        const CTAxisAlignedBox3D< Float64 > other( P3( 2, 3, 4 ), P3( 9, 9, 9 ) );
        ASSERT_TRUE( b.Intersects( other ) && b.Intersection( other ) == CTAxisAlignedBox3D< Float64 >( P3( 2, 3, 4 ), P3( 3, 4, 5 ) ) );
        ASSERT_FALSE( b.Intersects( CTAxisAlignedBox3D< Float64 >( P3( 4, 0, 0 ), P3( 5, 1, 1 ) ) ) );
        ASSERT_TRUE( b.Intersection( CTAxisAlignedBox3D< Float64 >( P3( 4, 0, 0 ), P3( 5, 1, 1 ) ) ).IsEmpty() );
        ASSERT_TRUE( b.Intersects( CTAxisAlignedBox3D< Float64 >( P3( 3, 4, 5 ), P3( 6, 6, 6 ) ) ) );          // touching is closed
        ASSERT_TRUE( b.Contains( CTAxisAlignedBox3D< Float64 >( P3( 0, 0, 0 ), P3( 1, 1, 1 ) ) ) && !b.Contains( other ) );
        ASSERT_TRUE( b.Expanded( 1 ) == CTAxisAlignedBox3D< Float64 >( P3( -2, -3, -4 ), P3( 4, 5, 6 ) ) && b.Expanded( -3 ).IsEmpty() && empty.Expanded( 1 ).IsEmpty() );
        ASSERT_TRUE( b.ExpandedToInclude( P3( 10, 0, 0 ) ).max.x == 10 && b.ExpandedToInclude( P3( 0, 0, 0 ) ) == b );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes3D: axis aligned box ray cast, slab test with parallel axes" )
    try
    {
        const CTAxisAlignedBox3D< Float64 > b( P3( -1, -1, -1 ), P3( 1, 1, 1 ) );
        SRayCastResult< Float64 > hit;
        ASSERT_TRUE( b.RayCast( CTRay3D< Float64 >( P3( -5, 0, 0 ), D3::Right() ), 100, hit ) );
        ASSERT_TRUE( Near( hit.distance, 4, 1e-15 ) && NearVector( hit.normal, -1, 0, 0, 0 ) && NearPoint( hit.point, -1, 0, 0, 1e-15 ) && 0 == hit.featureIndex && !hit.StartedInside() );
        ASSERT_TRUE( b.RayCast( CTRay3D< Float64 >( P3( 5, 0, 0 ), D3::Left() ), 100, hit ) && Near( hit.distance, 4, 1e-15 ) && NearVector( hit.normal, 1, 0, 0, 0 ) );
        ASSERT_TRUE( b.RayCast( CTRay3D< Float64 >( P3( 0, -5, 0.5 ), D3::Up() ), 100, hit ) && Near( hit.distance, 4, 1e-15 ) && NearVector( hit.normal, 0, -1, 0, 0 ) && 1 == hit.featureIndex );
        ASSERT_TRUE( b.RayCast( CTRay3D< Float64 >( P3( 0.2, 0.3, 7 ), D3::Forward() ), 100, hit ) && Near( hit.distance, 6, 1e-15 ) && NearVector( hit.normal, 0, 0, 1, 0 ) && 2 == hit.featureIndex );
        ASSERT_TRUE( b.RayCast( CTRay3D< Float64 >( P3( 0, 0, 0 ), D3::Right() ), 100, hit ) );
        ASSERT_TRUE( 0.0 == hit.distance && hit.StartedInside() && NearVector( hit.normal, -1, 0, 0, 0 ) && -1 == hit.featureIndex );
        // starting on a face and running along it: no NaN from 0 * infinity, the ray is inside
        ASSERT_TRUE( b.RayCast( CTRay3D< Float64 >( P3( -1, 0, 0 ), D3::Up() ), 100, hit ) && 0.0 == hit.distance && hit.StartedInside() );
        ASSERT_TRUE( b.RayCast( CTRay3D< Float64 >( P3( -1, -5, 0 ), D3::Up() ), 100, hit ) && Near( hit.distance, 4, 1e-15 ) && NearVector( hit.normal, 0, -1, 0, 0 ) );
        ASSERT_FALSE( b.RayCast( CTRay3D< Float64 >( P3( -3, -5, 0 ), D3::Up() ), 100, hit ) );                  // parallel, outside the slab
        ASSERT_FALSE( b.RayCast( CTRay3D< Float64 >( P3( 5, 0, 0 ), D3::Right() ), 100, hit ) );                 // behind
        ASSERT_FALSE( b.RayCast( CTRay3D< Float64 >( P3( -5, 0, 0 ), D3::Right() ), 3.9, hit ) );                // too far
        ASSERT_TRUE( b.RayCast( CTRay3D< Float64 >( P3( -5, 0, 0 ), D3::Right() ), 4, hit ) );                   // exactly in range
        ASSERT_FALSE( b.RayCast( CTRay3D< Float64 >( P3( -5, 1.001, 0 ), D3::Right() ), 100, hit ) );
        ASSERT_TRUE( b.RayCast( CTRay3D< Float64 >( P3( -5, 1, 0 ), D3::Right() ), 100, hit ) && Near( hit.distance, 4, 1e-15 ) );   // touching the top edge
        ASSERT_TRUE( b.RayCast( CTRay3D< Float64 >( P3( -5, -5, 0 ), D3( V3( 1, 1, 0 ) ) ), 100, hit ) && Near( hit.distance, 4 * sqrt( 2.0 ), 1e-12 ) );
        ASSERT_FALSE( CTAxisAlignedBox3D< Float64 >().RayCast( CTRay3D< Float64 >( P3( -5, 0, 0 ), D3::Right() ), 100, hit ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes3D: capsule contains, closest point, distance, measures and bounds" )
    try
    {
        const CTCapsule< Float64 > c( P3( 0, 0, 0 ), P3( 0, 4, 0 ), 1 );
        ASSERT_TRUE( c.Contains( P3( 0.5, 2, 0 ) ) && c.Contains( P3( 0, 5, 0 ) ) && !c.Contains( P3( 0, 5.1, 0 ) ) && !c.Contains( P3( 1.0001, 2, 0 ) ) && c.Contains( P3( 1, 2, 0 ) ) );
        ASSERT_TRUE( NearPoint( c.ClosestPoint( P3( 3, 2, 0 ) ), 1, 2, 0, 1e-14 ) && NearPoint( c.ClosestPoint( P3( 0, 7, 0 ) ), 0, 5, 0, 1e-14 ) && NearPoint( c.ClosestPoint( P3( 0, -3, 0 ) ), 0, -1, 0, 1e-14 ) );
        ASSERT_TRUE( NearPoint( c.ClosestPoint( P3( 0.2, 1, 0.1 ) ), 0.2, 1, 0.1, 0 ) );
        ASSERT_TRUE( Near( c.GetDistance( P3( 3, 2, 0 ) ), 2, 1e-14 ) && Near( c.GetDistance( P3( 0, 7, 0 ) ), 2, 1e-14 ) && 0.0 == c.GetDistance( P3( 0, 2, 0 ) ) );
        ASSERT_TRUE( Near( c.GetDistance( P3( 3, 6, 0 ) ), sqrt( 13.0 ) - 1, 1e-14 ) && Near( c.GetSignedDistance( P3( 0, 2, 0 ) ), -1, 1e-15 ) );
        const Float64 pi = 3.14159265358979323846;
        ASSERT_TRUE( Near( c.GetVolume(), 4 * pi + 4.0 / 3.0 * pi, 1e-12 ) && Near( c.GetSurfaceArea(), 12 * pi, 1e-12 ) );
        ASSERT_TRUE( c.GetBounds() == CTAxisAlignedBox3D< Float64 >( P3( -1, -1, -1 ), P3( 1, 5, 1 ) ) );
        ASSERT_TRUE( NearPoint( c.GetSupport( V3( 0, 2, 0 ) ), 0, 5, 0, 1e-15 ) && NearPoint( c.GetSupport( V3( 3, -1, 0 ) ), 3 / sqrt( 10.0 ), -1 / sqrt( 10.0 ), 0, 1e-14 ) );
        // a capsule of zero length is a sphere
        const CTCapsule< Float64 > ball( P3( 1, 1, 1 ), P3( 1, 1, 1 ), 2 );
        ASSERT_TRUE( ball.Contains( P3( 1, 1, 3 ) ) && !ball.Contains( P3( 1, 1, 3.1 ) ) && Near( ball.GetVolume(), 4.0 / 3.0 * pi * 8, 1e-12 ) );
        SRayCastResult< Float64 > hit;
        ASSERT_TRUE( ball.RayCast( CTRay3D< Float64 >( P3( -5, 1, 1 ), D3::Right() ), 100, hit ) && Near( hit.distance, 4, 1e-13 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes3D: ray casts against sphere, box and capsule agree with marching on Contains" )
    try
    {
        const UInt64 seed = 0x5EED3D;
        UInt32 hits = 0;
        const CTSphere< Float64 > sphere( P3( 0.3, -0.2, 0.5 ), 1.7 );
        ASSERT_TRUE( 0 == CountRayCastDisagreements( sphere, seed, 1500, hits ) && hits > 400 );
        const CTAxisAlignedBox3D< Float64 > box( P3( -1.5, -0.5, -1 ), P3( 2, 1.5, 0.75 ) );
        ASSERT_TRUE( 0 == CountRayCastDisagreements( box, seed + 1, 1500, hits ) && hits > 400 );
        const CTCapsule< Float64 > capsule( P3( -1, 0, 0.5 ), P3( 1.5, 1, -1 ), 0.8 );
        ASSERT_TRUE( 0 == CountRayCastDisagreements( capsule, seed + 2, 1500, hits ) && hits > 250 );
        const CTCapsule< Float64 > thin( P3( 0, -2, 0 ), P3( 0, 2, 0 ), 0.25 );
        ASSERT_TRUE( 0 == CountRayCastDisagreements( thin, seed + 3, 1500, hits ) && hits > 80 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes3D: segment, ray, line and polyline queries" )
    try
    {
        const CTSegment3D< Float64 > s( P3( 0, 0, 0 ), P3( 4, 0, 0 ) );
        ASSERT_TRUE( Near( s.GetLength(), 4, 1e-15 ) && Near( s.GetSquaredLength(), 16, 1e-14 ) && s.GetDirection() == D3::UnitX() && s.GetVector() == V3( 4, 0, 0 ) );
        ASSERT_TRUE( NearPoint( s.ClosestPoint( P3( 2, 3, 0 ) ), 2, 0, 0, 1e-15 ) && NearPoint( s.ClosestPoint( P3( -1, 1, 0 ) ), 0, 0, 0, 0 ) && NearPoint( s.ClosestPoint( P3( 9, 0, 0 ) ), 4, 0, 0, 0 ) );
        ASSERT_TRUE( Near( s.GetDistance( P3( 2, 3, 4 ) ), 5, 1e-14 ) && s.Contains( P3( 1, 0, 0 ) ) && !s.Contains( P3( 1, 0.1, 0 ) ) && s.Contains( P3( 1, 0.1, 0 ), 0.2 ) );
        ASSERT_TRUE( NearPoint( s.GetPoint( 0.25 ), 1, 0, 0, 1e-15 ) && NearPoint( s.GetMidpoint(), 2, 0, 0, 1e-15 ) && s.Reversed().start == s.end );
        ASSERT_TRUE( s.GetBounds() == CTAxisAlignedBox3D< Float64 >( P3( 0, 0, 0 ), P3( 4, 0, 0 ) ) && s.GetSupport( V3( 1, 0, 0 ) ) == P3( 4, 0, 0 ) && s.GetSupport( V3( -1, 0, 0 ) ) == P3( 0, 0, 0 ) );
        const CTSegment3D< Float64 > zero( P3( 2, 2, 2 ), P3( 2, 2, 2 ) );
        ASSERT_TRUE( 0.0 == zero.GetLength() && zero.GetDirection() == D3::Forward() && NearPoint( zero.ClosestPoint( P3( 5, 5, 5 ) ), 2, 2, 2, 0 ) );
        D3 direction = D3::Up();
        ASSERT_FALSE( zero.TryGetDirection( direction ) );
        ASSERT_TRUE( direction == D3::Up() );

        const CTRay3D< Float64 > r( P3( 0, 0, 0 ), D3::Right() );
        ASSERT_TRUE( NearPoint( r.ClosestPoint( P3( -2, 1, 0 ) ), 0, 0, 0, 0 ) && NearPoint( r.ClosestPoint( P3( 3, 1, 0 ) ), 3, 0, 0, 1e-15 ) );
        ASSERT_TRUE( Near( r.GetDistance( P3( -3, 4, 0 ) ), 5, 1e-14 ) && r.Contains( P3( 7, 0, 0 ) ) && !r.Contains( P3( -1, 0, 0 ) ) && NearPoint( r.GetPoint( 2.5 ), 2.5, 0, 0, 0 ) );
        CTRay3D< Float64 > fromPoints;
        ASSERT_TRUE( CTRay3D< Float64 >::TryFromPoints( P3( 1, 1, 1 ), P3( 1, 1, 5 ), fromPoints ) && fromPoints.direction == D3::UnitZ() && fromPoints.origin == P3( 1, 1, 1 ) );
        ASSERT_FALSE( CTRay3D< Float64 >::TryFromPoints( P3( 1, 1, 1 ), P3( 1, 1, 1 ), fromPoints ) );

        const CTLine3D< Float64 > l( P3( 0, 0, 0 ), D3::Right() );
        ASSERT_TRUE( NearPoint( l.ClosestPoint( P3( -2, 1, 0 ) ), -2, 0, 0, 0 ) && Near( l.GetDistance( P3( -3, 4, 0 ) ), 4, 1e-14 ) && l.Contains( P3( -7, 0, 0 ) ) );
        ASSERT_TRUE( Near( l.GetClosestParameter( P3( 3, 9, 9 ) ), 3, 0 ) );

        CTPolyline3D< Float64 > line;
        line.AddPoint( P3( 0, 0, 0 ) );
        line.AddPoint( P3( 3, 0, 0 ) );
        line.AddPoint( P3( 3, 4, 0 ) );
        ASSERT_TRUE( Near( line.GetLength(), 7, 1e-14 ) && 3 == line.GetPointCount() && 2 == line.GetSegmentCount() );
        ASSERT_TRUE( NearPoint( line.GetPointAtDistance( 3 ), 3, 0, 0, 1e-14 ) && NearPoint( line.GetPointAtDistance( 5 ), 3, 2, 0, 1e-14 ) && NearPoint( line.GetPointAtDistance( -1 ), 0, 0, 0, 0 ) && NearPoint( line.GetPointAtDistance( 100 ), 3, 4, 0, 0 ) );
        ASSERT_TRUE( NearPoint( line.ClosestPoint( P3( 1, 1, 0 ) ), 1, 0, 0, 1e-15 ) && Near( line.GetDistance( P3( 1, 1, 0 ) ), 1, 1e-15 ) && NearPoint( line.ClosestPoint( P3( 4, 5, 0 ) ), 3, 4, 0, 1e-15 ) );
        ASSERT_TRUE( line.Contains( P3( 3, 2, 0 ) ) && !line.Contains( P3( 1, 1, 0 ) ) && line.GetBounds() == CTAxisAlignedBox3D< Float64 >( P3( 0, 0, 0 ), P3( 3, 4, 0 ) ) );
        ASSERT_TRUE( line.GetSupport( V3( 0, 1, 0 ) ) == P3( 3, 4, 0 ) && line.GetSegment( 1 ).start == P3( 3, 0, 0 ) );
        const CTPolyline3D< Float64 > emptyLine;
        ASSERT_TRUE( 0.0 == emptyLine.GetLength() && 0 == emptyLine.GetSegmentCount() && emptyLine.ClosestPoint( P3( 1, 2, 3 ) ) == P3( 1, 2, 3 ) && emptyLine.GetBounds().IsEmpty() && !emptyLine.Contains( P3( 0, 0, 0 ) ) );
        CTPolyline3D< Float64 > one;
        one.AddPoint( P3( 5, 5, 5 ) );
        ASSERT_TRUE( 0.0 == one.GetLength() && one.ClosestPoint( P3( 0, 0, 0 ) ) == P3( 5, 5, 5 ) && one.GetPointAtDistance( 3 ) == P3( 5, 5, 5 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes3D: support points are extreme and inside the bounds" )
    try
    {
        const UInt64 seed = 0x5099027;
        CSeededSequence random( seed );
        const CTSphere< Float64 > sphere( P3( 1, -1, 2 ), 1.5 );
        const CTAxisAlignedBox3D< Float64 > box( P3( -1, -2, -3 ), P3( 3, 4, 5 ) );
        const CTTriangle3D< Float64 > triangle( P3( 0, 0, 0 ), P3( 4, 1, 0 ), P3( 1, 5, 2 ) );
        const CTCapsule< Float64 > capsule( P3( 0, 0, 0 ), P3( 2, 3, 1 ), 0.7 );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<2000; ++i )
        {
            const D3 d = RandomDirection( random );
            const V3 dv = d.AsVector();
            // sphere: c . d + r
            if ( !Near( sphere.GetSupport( dv ).ToPositionVector().Dot( dv ), sphere.centre.ToPositionVector().Dot( dv ) + sphere.radius, 1e-12 ) )
                ++failures;
            // box: the largest dot of the eight corners
            Float64 best = -1e300;
            for ( UInt32 corner=0; corner<8; ++corner )
                best = box.GetCorner( corner ).ToPositionVector().Dot( dv ) > best ? box.GetCorner( corner ).ToPositionVector().Dot( dv ) : best;
            if ( !Near( box.GetSupport( dv ).ToPositionVector().Dot( dv ), best, 1e-12 ) )
                ++failures;
            // triangle: the largest dot of the vertices
            Float64 bestVertex = triangle.a.ToPositionVector().Dot( dv );
            bestVertex = triangle.b.ToPositionVector().Dot( dv ) > bestVertex ? triangle.b.ToPositionVector().Dot( dv ) : bestVertex;
            bestVertex = triangle.c.ToPositionVector().Dot( dv ) > bestVertex ? triangle.c.ToPositionVector().Dot( dv ) : bestVertex;
            if ( !Near( triangle.GetSupport( dv ).ToPositionVector().Dot( dv ), bestVertex, 1e-12 ) )
                ++failures;
            // capsule: the larger end dot plus the radius
            Float64 bestEnd = capsule.start.ToPositionVector().Dot( dv ) > capsule.end.ToPositionVector().Dot( dv ) ? capsule.start.ToPositionVector().Dot( dv ) : capsule.end.ToPositionVector().Dot( dv );
            if ( !Near( capsule.GetSupport( dv ).ToPositionVector().Dot( dv ), bestEnd + capsule.radius, 1e-12 ) )
                ++failures;
            // the support points of the capsule and the sphere are inside their bounds
            if ( !capsule.GetBounds().Expanded( 1e-12 ).Contains( capsule.GetSupport( dv ) ) || !sphere.GetBounds().Expanded( 1e-12 ).Contains( sphere.GetSupport( dv ) ) )
                ++failures;
        }
        if ( failures > 0 )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Shapes3D: " << failures << " support point failures, seed " << seed );
        ASSERT_TRUE( 0 == failures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes3D: wireframes are closed outlines with the requested segment counts" )
    try
    {
        // sphere: three closed great circles of n segments, all points on the sphere
        const CTSphere< Float64 > sphere( P3( 1, 2, 3 ), 2 );
        GUCEF::vector< CTSegment3D< Float64 > > lines;
        sphere.GetWireframeLines( lines, 12 );
        ASSERT_TRUE( 36 == lines.size() );
        for ( UInt32 circle=0; circle<3; ++circle )
        {
            for ( UInt32 i=0; i<12; ++i )
            {
                const CTSegment3D< Float64 >& a = lines[ circle * 12 + i ];
                const CTSegment3D< Float64 >& b = lines[ circle * 12 + ( i + 1 ) % 12 ];
                ASSERT_TRUE( a.end == b.start );
                ASSERT_TRUE( Near( ( a.start - sphere.centre ).GetLength(), 2, 1e-13 ) );
            }
        }
        lines.clear();
        sphere.GetWireframeLines( lines, 1 );
        ASSERT_TRUE( 9 == lines.size() );              // at least 3 segments per circle

        // box: the 12 edges, each of the length of one extent, every corner used by three edges
        const CTAxisAlignedBox3D< Float64 > box( P3( 0, 0, 0 ), P3( 1, 2, 3 ) );
        lines.clear();
        box.GetWireframeLines( lines, 8 );
        ASSERT_TRUE( 12 == lines.size() );
        for ( UInt32 corner=0; corner<8; ++corner )
        {
            UInt32 uses = 0;
            for ( UInt32 i=0; i<12; ++i )
                uses += ( lines[ i ].start == box.GetCorner( corner ) ? 1 : 0 ) + ( lines[ i ].end == box.GetCorner( corner ) ? 1 : 0 );
            ASSERT_TRUE( 3 == uses );
        }
        lines.clear();
        CTAxisAlignedBox3D< Float64 >().GetWireframeLines( lines, 8 );
        ASSERT_TRUE( lines.empty() );

        // triangle, segment, polyline
        lines.clear();
        CTTriangle3D< Float64 >( P3( 0, 0, 0 ), P3( 1, 0, 0 ), P3( 0, 1, 0 ) ).GetWireframeLines( lines, 8 );
        ASSERT_TRUE( 3 == lines.size() && lines[ 0 ].end == lines[ 1 ].start && lines[ 1 ].end == lines[ 2 ].start && lines[ 2 ].end == lines[ 0 ].start );

        // capsule: two closed end circles of n segments (n rounded up to an even count of at least 4), 4 wall lines, 4 half circles
        const CTCapsule< Float64 > capsule( P3( 0, 0, 0 ), P3( 0, 4, 0 ), 1 );
        lines.clear();
        capsule.GetWireframeLines( lines, 16 );
        ASSERT_TRUE( 2 * 16 + 4 + 4 * 8 == lines.size() );
        for ( UInt32 cap=0; cap<2; ++cap )
        {
            for ( UInt32 i=0; i<16; ++i )
            {
                const CTSegment3D< Float64 >& a = lines[ cap * 16 + i ];
                const CTSegment3D< Float64 >& b = lines[ cap * 16 + ( i + 1 ) % 16 ];
                ASSERT_TRUE( a.end == b.start );
                ASSERT_TRUE( Near( capsule.GetSegment().GetDistance( a.start ), 1, 1e-13 ) );
            }
        }
        for ( UInt32 i=32; i<lines.size(); ++i )
            ASSERT_TRUE( Near( capsule.GetSegment().GetDistance( lines[ i ].start ), 1, 1e-13 ) && Near( capsule.GetSegment().GetDistance( lines[ i ].end ), 1, 1e-13 ) );
        lines.clear();
        capsule.GetWireframeLines( lines, 7 );
        ASSERT_TRUE( 2 * 8 + 4 + 4 * 4 == lines.size() );

        CTPolyline3D< Float64 > poly;
        poly.AddPoint( P3( 0, 0, 0 ) );
        poly.AddPoint( P3( 1, 0, 0 ) );
        poly.AddPoint( P3( 1, 1, 0 ) );
        lines.clear();
        poly.GetWireframeLines( lines, 8 );
        ASSERT_TRUE( 2 == lines.size() );
        lines.clear();
        CTSegment3D< Float64 >( P3( 0, 0, 0 ), P3( 1, 1, 1 ) ).GetWireframeLines( lines, 8 );
        ASSERT_TRUE( 1 == lines.size() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes3D: any empty box is the identity of Merged and ExpandedToInclude, not only the default one" )
    try
    {
        const CTAxisAlignedBox3D< Float64 > unit( P3( 0, 0, 0 ), P3( 1, 1, 1 ) );
        const CTAxisAlignedBox3D< Float64 > shrunk = unit.Expanded( -2 );
        const CTAxisAlignedBox3D< Float64 > far( P3( 10, 10, 10 ), P3( 11, 11, 11 ) );
        ASSERT_TRUE( shrunk.IsEmpty() );
        ASSERT_TRUE( shrunk.Merged( far ) == far && far.Merged( shrunk ) == far );
        ASSERT_FALSE( shrunk.Merged( far ).Contains( P3( 5, 5, 5 ) ) );
        ASSERT_TRUE( shrunk.Merged( shrunk ).IsEmpty() );
        ASSERT_TRUE( shrunk.ExpandedToInclude( P3( 3, 4, 5 ) ) == CTAxisAlignedBox3D< Float64 >( P3( 3, 4, 5 ), P3( 3, 4, 5 ) ) );
        ASSERT_TRUE( CTAxisAlignedBox3D< Float64 >().ExpandedToInclude( P3( 3, 4, 5 ) ) == CTAxisAlignedBox3D< Float64 >( P3( 3, 4, 5 ), P3( 3, 4, 5 ) ) );
        // an empty box from an empty intersection is just as much an identity
        const CTAxisAlignedBox3D< Float64 > none = unit.Intersection( far );
        ASSERT_TRUE( none.IsEmpty() && none.Merged( unit ) == unit && unit.Merged( none ) == unit );
        // a negative margin that leaves a box on one axis only empties it
        ASSERT_TRUE( CTAxisAlignedBox3D< Float64 >( P3( 0, 0, 0 ), P3( 10, 10, 1 ) ).Expanded( -1 ).IsEmpty() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes3D: segment end points are exact at the ends of the parameter range, also far from the origin" )
    try
    {
        // Float32 end points of very different magnitude: start + ( end - start ) * 1 would not give the end
        const TFloat32Point3D a( 0, 0, 1.0e7f );
        const TFloat32Point3D b( 0, 0, -0.1f );
        const TFloat32Segment3D segment( a, b );
        ASSERT_TRUE( segment.GetPoint( 1.0f ) == b && segment.GetPoint( 0.0f ) == a && segment.GetPoint( 2.0f ) == b && segment.GetPoint( -1.0f ) == a );
        ASSERT_TRUE( segment.ClosestPoint( TFloat32Point3D( 0, 0, -5 ) ) == b && segment.ClosestPoint( TFloat32Point3D( 0, 0, 2.0e7f ) ) == a );
        ASSERT_TRUE( segment.ClosestPoint( TFloat32Point3D( 0, 0, -5 ) ).z == -0.1f );
        // capsules and polylines inherit the exact end points
        const TFloat32Capsule capsule( a, b, 0.5f );
        ASSERT_TRUE( Near( capsule.ClosestPoint( TFloat32Point3D( 0, 0, -5 ) ).z, -0.6, 1e-6 ) );
        ASSERT_TRUE( capsule.Contains( TFloat32Point3D( 0, 0, -0.55f ) ) && !capsule.Contains( TFloat32Point3D( 0, 0, -0.65f ) ) );
        TFloat32Polyline3D line;
        line.AddPoint( a );
        line.AddPoint( b );
        ASSERT_TRUE( line.GetPointAtDistance( line.GetLength() ) == b && line.GetPointAtDistance( 1.0e30f ) == b && line.GetPointAtDistance( 0 ) == a );
        // the same in 2D, and in Float64
        const TFloat32Segment2D segment2( TFloat32Point2D( 1.0e7f, 0 ), TFloat32Point2D( -0.1f, 3 ) );
        ASSERT_TRUE( segment2.GetPoint( 1.0f ) == segment2.end && segment2.GetPoint( 0.0f ) == segment2.start && segment2.ClosestPoint( TFloat32Point2D( -5, 9 ) ) == segment2.end );
        const TFloat64Segment3D far64( TFloat64Point3D( 1.0e17, 0, 0 ), TFloat64Point3D( 0.1, 0, 0 ) );
        ASSERT_TRUE( far64.GetPoint( 1.0 ) == far64.end && far64.ClosestPoint( TFloat64Point3D( 0, 1, 0 ) ) == far64.end );
        TFloat32Polyline2D line2;
        line2.AddPoint( TFloat32Point2D( 1.0e7f, 0 ) );
        line2.AddPoint( TFloat32Point2D( -0.1f, 0 ) );
        ASSERT_TRUE( line2.GetPointAtDistance( line2.GetLength() ) == line2.points[ 1 ] );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes3D: spheres and capsules of radius 0 are a miss for ray casts, never a NaN normal" )
    try
    {
        SRayCastResult< Float64 > hit;
        const CTRay3D< Float64 > through( P3( -5, 0, 0 ), D3::Right() );
        ASSERT_FALSE( CTSphere< Float64 >( P3( 0, 0, 0 ), 0 ).RayCast( through, 100, hit ) );
        ASSERT_FALSE( CTSphere< Float64 >( P3( 0, 0, 0 ), -1 ).RayCast( through, 100, hit ) );
        ASSERT_FALSE( CTSphere< Float64 >( P3( -5, 0, 0 ), 0 ).RayCast( through, 100, hit ) );          // starting on it
        ASSERT_FALSE( CTCapsule< Float64 >( P3( 0, -1, 0 ), P3( 0, 1, 0 ), 0 ).RayCast( through, 100, hit ) );
        ASSERT_FALSE( CTCapsule< Float64 >( P3( 0, 0, 0 ), P3( 0, 0, 0 ), 0 ).RayCast( through, 100, hit ) );
        ASSERT_FALSE( CTCapsule< Float64 >( P3( -5, 0, 0 ), P3( 5, 0, 0 ), 0 ).RayCast( through, 100, hit ) );   // along its segment
        // the point queries of a radius 0 solid are still defined
        ASSERT_TRUE( CTSphere< Float64 >( P3( 1, 2, 3 ), 0 ).Contains( P3( 1, 2, 3 ) ) && !CTSphere< Float64 >( P3( 1, 2, 3 ), 0 ).Contains( P3( 1, 2, 3.5 ) ) );
        ASSERT_TRUE( Near( CTSphere< Float64 >( P3( 1, 2, 3 ), 0 ).GetDistance( P3( 1, 2, 5 ) ), 2, 1e-15 ) );
        ASSERT_TRUE( Near( CTCapsule< Float64 >( P3( 0, 0, 0 ), P3( 0, 4, 0 ), 0 ).GetDistance( P3( 3, 2, 0 ) ), 3, 1e-15 ) && CTCapsule< Float64 >( P3( 0, 0, 0 ), P3( 0, 4, 0 ), 0 ).Contains( P3( 0, 2, 0 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shapes3D: Transformed and TryTransformed agree with transforming sample points, seeded affine matrices" )
    try
    {
        typedef TFloat64Matrix4D M;
        const UInt64 seed = 0x7A45F0;
        CSeededSequence random( seed );
        UInt32 failures = 0;
        UInt32 categories[ 40 ] = { 0 };
        for ( UInt32 i=0; i<300; ++i )
        {
            const D3 axis = RandomDirection( random );
            const Float64 angle = random.NextFloat64( -3, 3 );
            const V3 translation( random.NextFloat64( -5, 5 ), random.NextFloat64( -5, 5 ), random.NextFloat64( -5, 5 ) );
            const Float64 uniform = random.NextFloat64( 0.3, 3 );
            const V3 stretch( random.NextFloat64( 0.3, 3 ), random.NextFloat64( 0.3, 3 ), random.NextFloat64( 0.3, 3 ) );
            const M rotation = M::RotationAxisAngle( axis, CTRadian< Float64 >( angle ) );
            const M similarity = M::Translation( translation ) * rotation * M::Scale( V3( uniform, uniform, uniform ) );
            const M general = M::Translation( translation ) * rotation * M::Scale( stretch );
            const M mirrored = general * M::Scale( V3( -1, 1, 1 ) );
            const M matrices[ 3 ] = { similarity, general, mirrored };

            for ( UInt32 k=0; k<3; ++k )
            {
                const M& m = matrices[ k ];
                const P3 p = RandomPoint( random, 4 );
                const P3 q = RandomPoint( random, 4 );

                const CTSegment3D< Float64 > segment( p, q );
                const CTSegment3D< Float64 > movedSegment = segment.Transformed( m );
                if ( !( movedSegment.start == m * p ) || !( movedSegment.end == m * q ) )
                    { ++failures; ++categories[ 1 ]; }
                const CTTriangle3D< Float64 > triangle( p, q, RandomPoint( random, 4 ) );
                const CTTriangle3D< Float64 > movedTriangle = triangle.Transformed( m );
                if ( !( movedTriangle.a == m * triangle.a ) || !( movedTriangle.c == m * triangle.c ) )
                    { ++failures; ++categories[ 2 ]; }
                CTPolyline3D< Float64 > polyline;
                polyline.AddPoint( p );
                polyline.AddPoint( q );
                if ( polyline.Transformed( m ).GetPointCount() != 2 || !( polyline.Transformed( m ).GetPoint( 1 ) == m * polyline.GetPoint( 1 ) ) )
                    { ++failures; ++categories[ 3 ]; }

                // rays and lines: the image of a point on the ray is on the image ray, which points the way the matrix moves the direction
                const CTRay3D< Float64 > ray( p, RandomDirection( random ) );
                const CTRay3D< Float64 > movedRay = ray.Transformed( m );
                const Float64 t = random.NextFloat64( 0, 5 );
                if ( !( movedRay.origin == m * p ) || movedRay.GetDistance( m * ray.GetPoint( t ) ) > 1e-9 )
                    { ++failures; ++categories[ 4 ]; }
                const CTLine3D< Float64 > line( p, RandomDirection( random ) );
                if ( line.Transformed( m ).GetDistance( m * line.GetPoint( t - 2 ) ) > 1e-9 )
                    { ++failures; ++categories[ 5 ]; }

                // the box: the tightest box around the images of the eight corners, and every image of a point of the box inside it
                const CTAxisAlignedBox3D< Float64 > box( p.Min( q ), p.Max( q ) );
                const CTAxisAlignedBox3D< Float64 > movedBox = box.Transformed( m );
                P3 corners[ 8 ];
                for ( UInt32 c=0; c<8; ++c )
                    corners[ c ] = m * box.GetCorner( c );
                const CTAxisAlignedBox3D< Float64 > expected = CTAxisAlignedBox3D< Float64 >::FromPoints( corners, 8 );
                if ( !movedBox.Equals( expected ) || !movedBox.Expanded( 1e-9 ).Contains( m * box.GetCentre() ) )
                    { ++failures; ++categories[ 6 ]; }
                if ( !CTAxisAlignedBox3D< Float64 >().Transformed( m ).IsEmpty() )
                    { ++failures; ++categories[ 7 ]; }

                // the plane: the image of every point of the plane is on the image plane, and the front stays the front
                const CTPlane< Float64 > plane = CTPlane< Float64 >::FromPointNormal( p, RandomDirection( random ) );
                CTPlane< Float64 > movedPlane;
                if ( !plane.TryTransformed( m, movedPlane ) )
                {
                    { ++failures; ++categories[ 8 ]; }
                    continue;
                }
                for ( UInt32 s=0; s<4; ++s )
                {
                    const P3 sample = RandomPoint( random, 5 );
                    const Float64 distance = plane.GetSignedDistance( sample );
                    const P3 onPlane = plane.Project( sample );
                    const Float64 movedDistance = movedPlane.GetSignedDistance( m * sample );
                    if ( movedPlane.GetDistance( m * onPlane ) > 1e-9 || ( fabs( distance ) > 1e-6 && ( distance > 0 ) != ( movedDistance > 0 ) ) )
                        { ++failures; ++categories[ 9 ]; }
                }
                if ( !( plane.Transformed( m ) == movedPlane ) )
                    { ++failures; ++categories[ 10 ]; }

                // sphere and capsule: exact for the similarity, refused (but still bounded) for the others
                const CTSphere< Float64 > sphere( p, random.NextFloat64( 0.1, 2 ) );
                const CTCapsule< Float64 > capsule( p, q, random.NextFloat64( 0.1, 2 ) );
                CTSphere< Float64 > movedSphere;
                CTCapsule< Float64 > movedCapsule;
                const bool sphereExact = sphere.TryTransformed( m, movedSphere );
                const bool capsuleExact = capsule.TryTransformed( m, movedCapsule );
                if ( sphereExact != ( 0 == k ) || capsuleExact != ( 0 == k ) )
                    { ++failures; ++categories[ 11 ]; }
                const CTSphere< Float64 > boundingSphere = sphere.Transformed( m );
                const CTCapsule< Float64 > boundingCapsule = capsule.Transformed( m );
                for ( UInt32 s=0; s<6; ++s )
                {
                    // a point on the surface of the sphere and of the capsule
                    const D3 d = RandomDirection( random );
                    const P3 onSphere = sphere.centre + d * sphere.radius;
                    const Float64 along = random.NextFloat64( 0, 1 );
                    const P3 onAxis = capsule.GetSegment().GetPoint( along );
                    const P3 onCapsule = onAxis + d * capsule.radius;
                    if ( ( m * onSphere - boundingSphere.centre ).GetLength() > boundingSphere.radius * ( 1 + 1e-9 ) )
                        { ++failures; ++categories[ 12 ]; }
                    if ( boundingCapsule.GetSegment().GetDistance( m * onCapsule ) > boundingCapsule.radius * ( 1 + 1e-9 ) )
                        { ++failures; ++categories[ 13 ]; }
                    if ( sphereExact && !Near( ( m * onSphere - movedSphere.centre ).GetLength(), movedSphere.radius, 1e-9 ) )
                        { ++failures; ++categories[ 14 ]; }
                    D3 perpendicular;
                    const bool havePerpendicular = D3::TryCreate( capsule.GetSegment().GetDirection().AsVector().Cross( d.AsVector() ), perpendicular );
                    const P3 onWall = onAxis + perpendicular * capsule.radius;
                    if ( capsuleExact && ( movedCapsule.GetSegment().GetDistance( m * onCapsule ) > movedCapsule.radius * ( 1 + 1e-9 ) ||
                         ( havePerpendicular && !Near( movedCapsule.GetSegment().GetDistance( m * onWall ), movedCapsule.radius, 1e-9 ) ) ) )
                        { ++failures; ++categories[ 15 ]; }
                }
            }
        }
        if ( failures > 0 )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Shapes3D: " << failures << " transform failures, seed " << seed ); for ( UInt32 c=1; c<40; ++c ) if ( categories[ c ] > 0 ) GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Shapes3D: transform check " << c << " failed " << categories[ c ] << " times" );
        ASSERT_TRUE( 0 == failures );

        // known cases: a quarter turn about Z maps the box ( 0, 0, 0 ) - ( 2, 1, 1 ) to ( -1, 0, 0 ) - ( 0, 2, 1 )
        const M quarter = M::RotationZ( CTRadian< Float64 >( 3.14159265358979323846 / 2 ) );
        const CTAxisAlignedBox3D< Float64 > turned = CTAxisAlignedBox3D< Float64 >( P3( 0, 0, 0 ), P3( 2, 1, 1 ) ).Transformed( quarter );
        ASSERT_TRUE( NearPoint( turned.min, -1, 0, 0, 1e-12 ) && NearPoint( turned.max, 0, 2, 1, 1e-12 ) );
        // a singular matrix has no image plane
        CTPlane< Float64 > flat;
        ASSERT_FALSE( CTPlane< Float64 >().TryTransformed( M::Scale( V3( 1, 0, 1 ) ), flat ) );
        // the plane y = 3 scaled by 2 along Y is the plane y = 6
        CTPlane< Float64 > scaled;
        ASSERT_TRUE( CTPlane< Float64 >::FromPointNormal( P3( 0, 3, 0 ), D3::Up() ).TryTransformed( M::Scale( V3( 1, 2, 1 ) ), scaled ) && scaled.normal == D3::Up() && Near( scaled.offset, 6, 1e-12 ) );
        // non-uniform scale: no exact sphere or capsule, a bounding one with the largest scale
        CTSphere< Float64 > ellipsoid;
        ASSERT_FALSE( CTSphere< Float64 >( P3( 1, 1, 1 ), 2 ).TryTransformed( M::Scale( V3( 1, 2, 3 ) ), ellipsoid ) );
        ASSERT_TRUE( Near( CTSphere< Float64 >( P3( 1, 1, 1 ), 2 ).Transformed( M::Scale( V3( 1, 2, 3 ) ) ).radius, 6, 1e-12 ) );
        ASSERT_TRUE( CTSphere< Float64 >( P3( 1, 1, 1 ), 2 ).TryTransformed( M::Scale( V3( 2, 2, 2 ) ), ellipsoid ) && Near( ellipsoid.radius, 4, 1e-12 ) && NearPoint( ellipsoid.centre, 2, 2, 2, 1e-12 ) );
        // the Float32 instances
        const TFloat32AxisAlignedBox3D box32 = TFloat32AxisAlignedBox3D( TFloat32Point3D( 0, 0, 0 ), TFloat32Point3D( 1, 1, 1 ) ).Transformed( TFloat32Matrix4D::Translation( TFloat32Vector3D( 1, 2, 3 ) ) );
        ASSERT_TRUE( box32.min == TFloat32Point3D( 1, 2, 3 ) && box32.max == TFloat32Point3D( 2, 3, 4 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
