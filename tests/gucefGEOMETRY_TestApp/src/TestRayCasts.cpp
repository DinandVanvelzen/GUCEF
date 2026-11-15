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

#include "TestQueryReference.h"
#include "TestRayCasts.h"

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The counts of a ray cast sweep */
struct SRaySweep
{
    UInt32 rays;
    UInt32 hits;                /**< rays that the reference and the function both hit */
    UInt32 misses;              /**< rays that both missed */
    UInt32 inside;              /**< rays that start inside the solid */
    UInt32 grazing;             /**< hits that the marching reference stepped over (a thin chord of the solid) */
    UInt32 bad;                 /**< disagreements */
    UInt32 normalChecks;
    UInt32 notFinite;

    SRaySweep( void ) : rays( 0 ), hits( 0 ), misses( 0 ), inside( 0 ), grazing( 0 ), bad( 0 ), normalChecks( 0 ), notFinite( 0 ) {}
};

/*-------------------------------------------------------------------------*/

/** The outward normal of a Float64 solid at a surface point from its nearest-point member: false near an edge or corner where it is not unique */
template < typename TShape >
static bool
ReferenceNormal( const TShape& shape, const P3& surfacePoint, const V3& rayDirection, V3& normal )
{
    const GUCEF::Float64 step = 1.0e-7;
    const P3 outside = surfacePoint - rayDirection * step;
    const P3 nearest = shape.ClosestPoint( outside );
    const V3 offset = outside - nearest;
    const GUCEF::Float64 length = offset.GetLength();
    if ( !( length > 0 ) || ( nearest - surfacePoint ).GetLength() > 4.0 * step )
        return false;
    normal = offset / length;
    return true;
}

/** The cone has no normal at its apex: a hit that close to it is not checked */
static bool
ReferenceNormal( const CTCone< Float64 >& shape, const P3& surfacePoint, const V3& rayDirection, V3& normal )
{
    if ( ( surfacePoint - shape.apex ).GetLength() < 0.02 )
        return false;
    const GUCEF::Float64 step = 1.0e-7;
    const P3 outside = surfacePoint - rayDirection * step;
    const P3 nearest = shape.ClosestPoint( outside );
    const V3 offset = outside - nearest;
    const GUCEF::Float64 length = offset.GetLength();
    if ( !( length > 0 ) || ( nearest - surfacePoint ).GetLength() > 4.0 * step )
        return false;
    normal = offset / length;
    return true;
}

/** The same from the planes for a region bounded by planes */
static bool
PlaneSetNormal( const CTPlane< Float64 >* planes, UInt32 count, const P3& surfacePoint, V3& normal )
{
    UInt32 best = 0;
    Float64 bestDistance = 1.0e300;
    UInt32 near = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const Float64 d = fabs( planes[ i ].GetSignedDistance( surfacePoint ) );
        if ( d < bestDistance )
        {
            bestDistance = d;
            best = i;
        }
        if ( d < 1.0e-9 )
            ++near;
    }
    if ( near != 1 )
        return false;                       /* on an edge or a corner */
    normal = -planes[ best ].normal.AsVector();
    return true;
}

static bool
ReferenceNormal( const CTFrustum< Float64 >& shape, const P3& surfacePoint, const V3& /*rayDirection*/, V3& normal )
{
    return PlaneSetNormal( shape.GetPlanes(), 6, surfacePoint, normal );
}

static bool
ReferenceNormal( const CTPlaneBoundedVolume< Float64 >& shape, const P3& surfacePoint, const V3& /*rayDirection*/, V3& normal )
{
    return PlaneSetNormal( shape.GetPlanes(), shape.GetPlaneCount(), surfacePoint, normal );
}

/*-------------------------------------------------------------------------*/

/** A point in the middle of a shape to aim rays at */
template < typename TShape >
static P3
AimPoint( const TShape& shape )
{
    return shape.GetBounds().GetCentre();
}

static P3
AimPoint( const CTPlaneBoundedVolume< Float64 >& volume )
{
    CTAxisAlignedBox3D< Float64 > bounds;
    return volume.TryGetBounds( bounds ) ? bounds.GetCentre() : P3();
}

/** True when the solid is entered within a short distance after 'start' ( a thin chord that a coarse marching step steps over ), by a fine march on Contains */
template < typename TShape >
static bool
HasChordNear( const TShape& shape, const CTRay3D< Float64 >& ray, Float64 start )
{
    for ( UInt32 k=0; k<3000; ++k )
    {
        if ( shape.Contains( ray.GetPoint( start - 1.0e-4 + 2.0e-5 * (Float64) k ) ) )
            return true;
    }
    return false;
}

/** Casts 'count' seeded rays against random shapes of one kind and compares with the marching reference */
template < typename T, typename TShape >
static void
RayCastSweep( const char* name, UInt64 seed, UInt32 count, SRaySweep& sweep )
{
    CRandom random( seed );
    const Float64 maxDistance = 20.0;
    for ( UInt32 i=0; i<count; ++i )
    {
        TShape shape;
        RandomShape( random, shape );
        const P3 origin = random.Point( 5.0 );
        D3 direction = random.Direction();
        if ( 0 == i % 2 )
        {
            /* every other ray is aimed at a point near the shape, so that many rays hit */
            const V3 scatter = random.Vector( 0.6 );
            D3::TryCreate( AimPoint( Widen( shape ) ) + scatter - origin, direction );
        }
        const CTRay3D< T > ray( ToP< T >( origin ), ToD< T >( direction ) );
        SRayCastResult< T > hit;
        const bool got = shape.RayCast( ray, (T) maxDistance, hit );

        const CTRay3D< Float64 > ray64 = Widen( ray );
        const Float64 refMax = maxDistance;
        Float64 refT = 0;
        const bool refHit = MarchToSolid( Widen( shape ), ray64, refMax, 0.02, refT );
        ++sweep.rays;

        if ( got && ( !CTNumericTraits< T >::IsFinite( hit.distance ) || !hit.point.IsFinite() || !hit.normal.IsFinite() ) )
        {
            ++sweep.notFinite;
            ++sweep.bad;
            continue;
        }

        if ( refHit )
        {
            if ( !got )
            {
                /* the reference found the solid right at the end of the range */
                if ( refT < maxDistance - 0.05 )
                {
                    ++sweep.bad;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": missed a hit at " << refT << ", seed " << seed << " ray " << i );
                }
                continue;
            }
            ++sweep.hits;
            const Float64 tolerance = 1.0e-4 * ( refT > 1.0 ? refT : 1.0 );
            if ( !NearTo( hit.distance, refT, tolerance ) )
            {
                ++sweep.bad;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": distance " << hit.distance << " against " << refT << ", seed " << seed << " ray " << i );
                continue;
            }
            if ( 0 == refT )
            {
                ++sweep.inside;
                const V3 n = Widen( hit.normal );
                if ( !hit.StartedInside() || !NearTo( n.Dot( direction.AsVector() ), -1.0, 1.0e-4 ) )
                {
                    ++sweep.bad;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": started inside not reported right, seed " << seed << " ray " << i );
                }
                continue;
            }
            if ( hit.StartedInside() )
            {
                ++sweep.bad;
                continue;
            }
            const P3 surface = ray64.GetPoint( refT );
            if ( ( Widen( hit.point ) - surface ).GetLength() > 4.0e-4 )
            {
                ++sweep.bad;
                continue;
            }
            V3 expected;
            if ( ReferenceNormal( Widen( shape ), surface, ray64.direction.AsVector(), expected ) && fabs( expected.Dot( ray64.direction.AsVector() ) ) > 0.02 )
            {
                ++sweep.normalChecks;
                const V3 n = Widen( hit.normal );
                if ( ( n - expected ).GetLength() > 1.0e-3 )
                {
                    ++sweep.bad;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": normal off by " << ( n - expected ).GetLength() << " feature " << hit.featureIndex << " distance error " << ( hit.distance - refT ) << " n (" << n.x << "," << n.y << "," << n.z << ") expected (" << expected.x << "," << expected.y << "," << expected.z << ") at " << refT << ", seed " << seed << " ray " << i );
                }
            }
        }
        else if ( got )
        {
            /* a thin chord that the marching step skipped: a finer march must find the solid right behind the hit */
            if ( HasChordNear( Widen( shape ), ray64, hit.distance ) )
                ++sweep.grazing;
            else
            {
                ++sweep.bad;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": hit at " << hit.distance << " that the reference does not find, seed " << seed << " ray " << i );
            }
        }
        else
        {
            ++sweep.misses;
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RayCasts: " << name << " seed " << seed << ": " << sweep.rays << " rays, " << sweep.hits << " hits, " << sweep.inside << " inside, " << sweep.misses
               << " misses, " << sweep.grazing << " grazing, " << sweep.normalChecks << " normals checked, " << sweep.bad << " disagreements" );
}

/*-------------------------------------------------------------------------*/

/** The sweep passes when nothing disagrees and the rays did hit often enough and miss often enough to mean something */
static bool
SweepPassed( const SRaySweep& sweep )
{
    return 0 == sweep.bad && sweep.hits > sweep.rays / 20 && sweep.misses > sweep.rays / 10 && sweep.normalChecks > sweep.hits / 4 && sweep.grazing < sweep.rays / 50;
}

/*-------------------------------------------------------------------------*/

/** Float64 evaluation of a ray against a triangle by Cramer's rule, two-sided */
static bool
ReferenceTriangleHit( const CTTriangle3D< Float64 >& t, const CTRay3D< Float64 >& ray, Float64 maxDistance, Float64& distance, Float64& u, Float64& v )
{
    const V3 e1 = t.b - t.a;
    const V3 e2 = t.c - t.a;
    const V3 d = ray.direction.AsVector();
    const V3 p = ray.origin - t.a;
    const Float64 det = e1.Dot( d.Cross( e2 ) );
    if ( 0.0 == det )
        return false;
    u = p.Dot( d.Cross( e2 ) ) / det;
    v = d.Dot( p.Cross( e1 ) ) / det;
    distance = e2.Dot( p.Cross( e1 ) ) / det;
    return u >= 0 && v >= 0 && u + v <= 1 && distance >= 0 && distance <= maxDistance;
}

/*-------------------------------------------------------------------------*/

/** Seeded rays against random triangles and planes with a Float64 analytic reference */
template < typename T >
static UInt32
TrianglePlaneSweep( UInt64 seed, UInt32 count, UInt32& hits )
{
    CRandom random( seed );
    UInt32 bad = 0;
    hits = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTTriangle3D< T > triangle;
        RandomShape( random, triangle );
        CTPlane< T > plane;
        RandomShape( random, plane );
        const P3 origin = random.Point( 4.0 );
        D3 direction = random.Direction();
        if ( 0 == i % 2 )
        {
            const P3 target = random.Point( 1.5 );
            D3::TryCreate( target - origin, direction );
        }
        const CTRay3D< T > ray( ToP< T >( origin ), ToD< T >( direction ) );
        const CTRay3D< Float64 > ray64 = Widen( ray );

        Float64 refT = 0;
        Float64 refU = 0;
        Float64 refV = 0;
        const CTTriangle3D< Float64 > triangle64 = Widen( triangle );
        const bool refTriangle = !triangle.IsDegenerate() && ReferenceTriangleHit( triangle64, ray64, 20.0, refT, refU, refV );
        SRayCastResult< T > hit;
        const bool gotTriangle = triangle.RayCast( ray, (T) 20, hit );
        if ( refTriangle != gotTriangle )
        {
            /* a ray through an edge or a vertex may be decided either way: only the clear cases count */
            const Float64 margin = fmin( fmin( refU, refV ), 1.0 - refU - refV );
            if ( fabs( margin ) > 1.0e-4 || !( refT > 1.0e-4 ) )
            {
                ++bad;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCasts: triangle hit " << gotTriangle << " reference " << refTriangle << ", seed " << seed << " ray " << i );
            }
        }
        else if ( refTriangle )
        {
            ++hits;
            if ( !NearTo( hit.distance, refT, 1.0e-4 * ( refT > 1.0 ? refT : 1.0 ) ) || !NearTo( hit.u, refU, 1.0e-3 ) || !NearTo( hit.v, refV, 1.0e-3 ) )
                ++bad;
        }

        SRayCastResult< T > planeHit;
        const bool gotPlane = plane.RayCast( ray, (T) 20, planeHit );
        const CTPlane< Float64 > plane64 = Widen( plane );
        const Float64 denominator = plane64.normal.AsVector().Dot( ray64.direction.AsVector() );
        const Float64 refPlaneT = 0.0 == denominator ? -1.0 : ( plane64.offset - plane64.normal.AsVector().Dot( ray64.origin.ToPositionVector() ) ) / denominator;
        const bool refPlane = refPlaneT >= 0 && refPlaneT <= 20.0;
        if ( refPlane != gotPlane && fabs( refPlaneT - 20.0 ) > 1.0e-3 && fabs( refPlaneT ) > 1.0e-4 )
            ++bad;
        else if ( refPlane && gotPlane )
        {
            const V3 n = Widen( planeHit.normal );
            if ( !NearTo( planeHit.distance, refPlaneT, 1.0e-4 * ( refPlaneT > 1.0 ? refPlaneT : 1.0 ) ) || n.Dot( ray64.direction.AsVector() ) > 0 )
                ++bad;
        }
    }
    return bad;
}

/*-------------------------------------------------------------------------*/

/** A vertex of the octahedron with the given sign pattern is at distance 1 on one axis */
static void
MakeOctahedron( Float64 scale, const P3& centre, P3 ( &vertices )[ 6 ], UInt32 ( &faces )[ 8 ][ 3 ] )
{
    vertices[ 0 ] = centre + V3( scale, 0, 0 );
    vertices[ 1 ] = centre + V3( -scale, 0, 0 );
    vertices[ 2 ] = centre + V3( 0, scale, 0 );
    vertices[ 3 ] = centre + V3( 0, -scale, 0 );
    vertices[ 4 ] = centre + V3( 0, 0, scale );
    vertices[ 5 ] = centre + V3( 0, 0, -scale );
    /* counter-clockwise seen from outside */
    static const UInt32 table[ 8 ][ 3 ] = { { 0, 2, 4 }, { 2, 1, 4 }, { 1, 3, 4 }, { 3, 0, 4 }, { 2, 0, 5 }, { 1, 2, 5 }, { 3, 1, 5 }, { 0, 3, 5 } };
    memcpy( faces, table, sizeof( table ) );
}

/*-------------------------------------------------------------------------*/

/** The number of the triangles that the ray hits, by the watertight test and by the Moeller-Trumbore member, over a list of triangles */
template < typename T >
static void
CountHits( const CTTriangle3D< T >* triangles, UInt32 count, const CTRay3D< T >& ray, UInt32& watertight, UInt32& moeller )
{
    watertight = 0;
    moeller = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        SRayCastResult< T > hit;
        if ( RayCastTriangleWatertight( ray, triangles[ i ], (T) 1000, hit ) )
            ++watertight;
        if ( triangles[ i ].RayCast( ray, (T) 1000, hit ) )
            ++moeller;
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  Rays through shared edges and vertices: a flat quad of two triangles (shared edge), a flat fan of six triangles (shared
 *  vertex) and a closed octahedron (shared edges and vertices): the watertight test must hit at least one triangle
 *  for every ray ( two for a closed mesh that the ray enters and leaves ).
 */
template < typename T >
static void
WatertightSweep( UInt64 seed, UInt32 count, UInt32& misses, UInt32& moellerMisses )
{
    CRandom random( seed );
    misses = 0;
    moellerMisses = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTTriangle3D< T > mesh[ 8 ];
        UInt32 triangleCount = 0;
        P3 through;
        UInt32 required = 1;
        const UInt32 kind = i % 3;
        P3 centre = random.Point( 2.0 );
        const Float64 scale = random.Uniform( 0.5, 2.0 );
        if ( 0 == kind )
        {
            /* a flat quad in a random plane: the triangles ( a, b, c ) and ( b, a, d ) share the edge a b */
            const D3 normal = random.Direction();
            V3 u = normal.AsVector().GetAnyPerpendicular().Normalized();
            const V3 w = normal.AsVector().Cross( u );
            const GUCEF::Float64 r1 = random.Uniform( 0.3, 1.0 );
            const GUCEF::Float64 r2 = random.Uniform( 0.3, 1.0 );
            const GUCEF::Float64 angle = random.Uniform( 0.0, 6.28 );
            u = u * cos( angle ) + w * sin( angle );
            const V3 v = normal.AsVector().Cross( u );
            const P3 a = centre + u * ( -scale * r1 );
            const P3 b = centre + u * ( scale * r1 );
            const P3 c = centre + v * ( scale * r2 );
            const P3 d = centre - v * ( scale * r2 );
            mesh[ 0 ] = CTTriangle3D< T >( ToP< T >( a ), ToP< T >( b ), ToP< T >( c ) );
            mesh[ 1 ] = CTTriangle3D< T >( ToP< T >( b ), ToP< T >( a ), ToP< T >( d ) );
            triangleCount = 2;
            const GUCEF::Float64 s = random.Uniform( 0.05, 0.95 );
            const P3 pa = Widen( mesh[ 0 ].a );
            const P3 pb = Widen( mesh[ 0 ].b );
            through = pa + ( pb - pa ) * s;
        }
        else if ( 1 == kind )
        {
            /* a flat fan of six triangles around the centre vertex */
            const D3 normal = random.Direction();
            const V3 u = normal.AsVector().GetAnyPerpendicular().Normalized();
            const V3 v = normal.AsVector().Cross( u );
            const GUCEF::Float64 offsetAngle = random.Uniform( 0.0, 1.0 );
            P3 rim[ 6 ];
            for ( UInt32 k=0; k<6; ++k )
            {
                const GUCEF::Float64 angle = offsetAngle + 1.0471975511965976 * (GUCEF::Float64) k;
                rim[ k ] = centre + ( u * cos( angle ) + v * sin( angle ) ) * ( scale * random.Uniform( 0.5, 1.0 ) );
            }
            const P3 hub = centre;
            for ( UInt32 k=0; k<6; ++k )
                mesh[ k ] = CTTriangle3D< T >( ToP< T >( hub ), ToP< T >( rim[ k ] ), ToP< T >( rim[ ( k + 1 ) % 6 ] ) );
            triangleCount = 6;
            through = Widen( mesh[ 0 ].a );
        }
        else
        {
            /* a closed octahedron, a ray through a vertex or the middle of an edge enters and leaves it */
            P3 vertices[ 6 ];
            UInt32 faces[ 8 ][ 3 ];
            MakeOctahedron( scale, centre, vertices, faces );
            for ( UInt32 k=0; k<8; ++k )
                mesh[ k ] = CTTriangle3D< T >( ToP< T >( vertices[ faces[ k ][ 0 ] ] ), ToP< T >( vertices[ faces[ k ][ 1 ] ] ), ToP< T >( vertices[ faces[ k ][ 2 ] ] ) );
            triangleCount = 8;
            const UInt32 vertexIndex = (UInt32) random.Index( 6 );
            if ( 0 == i % 2 )
                through = Widen( mesh[ 0 ].a ) + ( Widen( mesh[ 0 ].b ) - Widen( mesh[ 0 ].a ) ) * random.Uniform( 0.05, 0.95 );
            else
                through = Widen( mesh[ 0 ].c );
            (void) vertexIndex;
            required = 2;
        }
        /* the ray comes from outside and runs through the shared edge or vertex into the mesh: along the normal of a flat mesh, towards the middle of the octahedron */
        V3 approach = 2 == kind ? through - centre : centre - centre;
        if ( 2 != kind )
        {
            D3 planeNormal;
            D3::TryCreate( ( Widen( mesh[ 0 ].b ) - Widen( mesh[ 0 ].a ) ).Cross( Widen( mesh[ 0 ].c ) - Widen( mesh[ 0 ].a ) ), planeNormal );
            approach = planeNormal.AsVector() * ( random.Chance( 0.5 ) ? 1.0 : -1.0 );
        }
        D3 approachDirection;
        D3::TryCreate( approach, approachDirection );
        const V3 wobble = random.Direction().AsVector() * 0.3;
        D3 outward;
        D3::TryCreate( approachDirection.AsVector() + wobble, outward );
        const P3 origin = through + outward.AsVector() * random.Uniform( 2.0, 6.0 );
        D3 direction;
        D3::TryCreate( through - origin, direction );
        const CTRay3D< T > ray( ToP< T >( origin ), ToD< T >( direction ) );
        UInt32 watertight = 0;
        UInt32 moeller = 0;
        CountHits( mesh, triangleCount, ray, watertight, moeller );
        if ( watertight < required )
        {
            ++misses;
            if ( misses < 4 )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RayCasts: watertight ray misses (" << watertight << " hits, mesh kind " << kind << "), seed " << seed << " ray " << i );
        }
        if ( moeller < required )
            ++moellerMisses;
    }
}

/** A ray with a direction that is not a number (the direction type does not allow it through its constructors) */
template < typename T >
static CTRay3D< T >
RayWithBadDirection( const CTPoint3D< T >& origin )
{
    CTRay3D< T > ray( origin, CTDirectionVector3D< T >::Forward() );
    const T bad[ 3 ] = { std::numeric_limits< T >::quiet_NaN(), (T) 0, (T) 1 };
    memcpy( &ray.direction, bad, sizeof( bad ) );
    return ray;
}

/** A cast that must miss without producing a hit that is not finite, for a ray or a shape that contains a NaN */
template < typename TShape, typename T >
static bool
MissesNotANumber( const TShape& shape, const CTRay3D< T >& ray )
{
    SRayCastResult< T > hit;
    hit.distance = (T) 12345;
    const bool got = shape.RayCast( ray, (T) 100, hit );
    return !got && (T) 12345 == hit.distance;
}

/** Rays that start at a NaN or point in a NaN direction, and shapes with a NaN in them, against every solid and the plane and the triangle */
template < typename T >
static bool
NotANumberCasts( void )
{
    typedef CTPoint3D< T > P;
    typedef CTVector3D< T > V;
    typedef CTDirectionVector3D< T > D;
    const T nan = std::numeric_limits< T >::quiet_NaN();
    const CTRay3D< T > badOrigin( P( nan, 0, 0 ), D( V( 0, 0, 1 ) ) );
    const CTRay3D< T > badOriginInside( P( 0, nan, 0 ), D( V( 1, 0, 0 ) ) );
    const CTRay3D< T > badDirection = RayWithBadDirection( P( 0, 0, 5 ) );
    const CTRay3D< T > badDirectionInside = RayWithBadDirection( P( 0, 0, 0 ) );
    const CTRay3D< T >* rays[ 4 ] = { &badOrigin, &badOriginInside, &badDirection, &badDirectionInside };
    bool ok = true;

    const CTSphere< T > sphere( P( 0, 0, 0 ), 1 );
    const CTAxisAlignedBox3D< T > box( P( -1, -1, -1 ), P( 1, 1, 1 ) );
    const CTOrientedBox3D< T > oriented( P( 0, 0, 0 ), CTQuaternion< T >::Identity(), V( 1, 1, 1 ) );
    const CTCapsule< T > capsule( P( 0, 0, -1 ), P( 0, 0, 1 ), 1 );
    const CTCylinder< T > cylinder( P( 0, 0, -1 ), P( 0, 0, 1 ), 1 );
    const CTCone< T > cone( P( 0, 0, -1 ), D( V( 0, 0, 1 ) ), 2, 1 );
    const CTFrustum< T > frustum;
    const CTPlaneBoundedVolume< T > volume = CTPlaneBoundedVolume< T >::FromAxisAlignedBox( box );
    const CTPlaneBoundedVolume< T > everything;
    const CTPlane< T > plane( D( V( 0, 0, 1 ) ), 0 );
    const CTTriangle3D< T > triangle( P( -1, -1, 0 ), P( 1, -1, 0 ), P( 0, 1, 0 ) );
    for ( UInt32 r=0; r<4; ++r )
    {
        ok = ok && MissesNotANumber( sphere, *rays[ r ] ) && MissesNotANumber( box, *rays[ r ] ) && MissesNotANumber( oriented, *rays[ r ] ) && MissesNotANumber( capsule, *rays[ r ] );
        ok = ok && MissesNotANumber( cylinder, *rays[ r ] ) && MissesNotANumber( cone, *rays[ r ] ) && MissesNotANumber( frustum, *rays[ r ] ) && MissesNotANumber( volume, *rays[ r ] );
        ok = ok && MissesNotANumber( everything, *rays[ r ] ) && MissesNotANumber( plane, *rays[ r ] ) && MissesNotANumber( triangle, *rays[ r ] );
        SRayCastResult< T > hit;
        ok = ok && !RayCastTriangleWatertight( *rays[ r ], triangle, (T) 100, hit );
    }

    /* shapes with a NaN in them are hit by nothing */
    const CTRay3D< T > ray( P( 0, 0, 5 ), D( V( 0, 0, -1 ) ) );
    const CTRay3D< T > inside( P( 0, 0, 0 ), D( V( 0, 0, -1 ) ) );
    const CTRay3D< T >* good[ 2 ] = { &ray, &inside };
    for ( UInt32 r=0; r<2; ++r )
    {
        ok = ok && MissesNotANumber( CTSphere< T >( P( 0, 0, 0 ), nan ), *good[ r ] ) && MissesNotANumber( CTSphere< T >( P( nan, 0, 0 ), 1 ), *good[ r ] );
        ok = ok && MissesNotANumber( CTAxisAlignedBox3D< T >( P( -1, nan, -1 ), P( 1, 1, 1 ) ), *good[ r ] );
        ok = ok && MissesNotANumber( CTOrientedBox3D< T >( P( 0, 0, 0 ), CTQuaternion< T >::Identity(), V( 1, nan, 1 ) ), *good[ r ] ) && MissesNotANumber( CTOrientedBox3D< T >( P( nan, 0, 0 ), CTQuaternion< T >::Identity(), V( 1, 1, 1 ) ), *good[ r ] );
        ok = ok && MissesNotANumber( CTCapsule< T >( P( 0, 0, -1 ), P( 0, 0, 1 ), nan ), *good[ r ] );
        ok = ok && MissesNotANumber( CTCylinder< T >( P( 0, 0, -1 ), P( 0, 0, 1 ), nan ), *good[ r ] ) && MissesNotANumber( CTCylinder< T >( P( 0, 0, -1 ), P( nan, 0, 1 ), 1 ), *good[ r ] );
        ok = ok && MissesNotANumber( CTCone< T >( P( 0, 0, -1 ), D( V( 0, 0, 1 ) ), nan, 1 ), *good[ r ] ) && MissesNotANumber( CTCone< T >( P( 0, 0, -1 ), D( V( 0, 0, 1 ) ), 2, nan ), *good[ r ] ) &&
              MissesNotANumber( CTCone< T >( P( nan, 0, -1 ), D( V( 0, 0, 1 ) ), 2, 1 ), *good[ r ] );
        const CTPlane< T > badPlane( D( V( 0, 0, 1 ) ), nan );
        ok = ok && MissesNotANumber( CTPlaneBoundedVolume< T >( &badPlane, 1 ), *good[ r ] );
        ok = ok && MissesNotANumber( CTTriangle3D< T >( P( -1, -1, 0 ), P( nan, -1, 0 ), P( 0, 1, 0 ) ), *good[ r ] );
    }
    return ok;
}

/** Rays through the apex of a cone with a direction inside the cone hit the apex at the distance of the apex, from all sides and at all distances */
template < typename T >
static UInt32
ApexRaySweep( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTCone< T > cone;
        RandomShape( random, cone );
        const CTCone< Float64 > cone64 = Widen( cone );
        const Float64 halfAngle = atan( cone64.baseRadius / cone64.height );
        /* a direction inside the cone: the axis turned by less than the half angle (0.9 of it at most) */
        const Float64 tilt = random.Uniform( 0.0, 0.9 * halfAngle );
        const V3 u = cone64.axis.AsVector();
        const V3 side = RandomPerpendicular( random, u );
        V3 d = u * cos( tilt ) + side * sin( tilt );
        D3 direction;
        D3::TryCreate( d, direction );
        const Float64 distance = random.Uniform( 0.5, 6.0 );
        const P3 origin = cone64.apex - direction.AsVector() * distance;
        CTRay3D< T > ray( ToP< T >( origin ), ToD< T >( direction ) );
        SRayCastResult< T > hit;
        const bool got = cone.RayCast( ray, (T) 20, hit );
        if ( !got || !CTNumericTraits< T >::IsFinite( hit.distance ) || !NearTo( hit.distance, distance, 2.0e-3 * distance ) )
            ++bad;
        else if ( ( Widen( hit.point ) - cone64.apex ).GetLength() > 3.0e-3 * distance )
            ++bad;
    }
    return bad;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformRayCastsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "RayCasts" );

    GUCEF_TESTFW_TESTCASE( "RayCasts: sphere, AABB, OBB and capsule, 10000 seeded rays each in Float32 and Float64 against marching on Contains" )
    try
    {
        SRaySweep s1;
        RayCastSweep< Float32, CTSphere< Float32 > >( "sphere32", 101, 10000, s1 );
        ASSERT_TRUE( SweepPassed( s1 ) );
        SRaySweep s2;
        RayCastSweep< Float64, CTSphere< Float64 > >( "sphere64", 102, 10000, s2 );
        ASSERT_TRUE( SweepPassed( s2 ) );
        SRaySweep s3;
        RayCastSweep< Float32, CTAxisAlignedBox3D< Float32 > >( "aabb32", 103, 10000, s3 );
        ASSERT_TRUE( SweepPassed( s3 ) );
        SRaySweep s4;
        RayCastSweep< Float64, CTAxisAlignedBox3D< Float64 > >( "aabb64", 104, 10000, s4 );
        ASSERT_TRUE( SweepPassed( s4 ) );
        SRaySweep s5;
        RayCastSweep< Float32, CTOrientedBox3D< Float32 > >( "obb32", 105, 10000, s5 );
        ASSERT_TRUE( SweepPassed( s5 ) );
        SRaySweep s6;
        RayCastSweep< Float64, CTOrientedBox3D< Float64 > >( "obb64", 106, 10000, s6 );
        ASSERT_TRUE( SweepPassed( s6 ) );
        SRaySweep s7;
        RayCastSweep< Float32, CTCapsule< Float32 > >( "capsule32", 107, 10000, s7 );
        ASSERT_TRUE( SweepPassed( s7 ) );
        SRaySweep s8;
        RayCastSweep< Float64, CTCapsule< Float64 > >( "capsule64", 108, 10000, s8 );
        ASSERT_TRUE( SweepPassed( s8 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RayCasts: cylinder and cone, 10000 seeded rays each in Float32 and Float64 against marching on Contains" )
    try
    {
        SRaySweep s1;
        RayCastSweep< Float32, CTCylinder< Float32 > >( "cylinder32", 111, 10000, s1 );
        ASSERT_TRUE( SweepPassed( s1 ) );
        SRaySweep s2;
        RayCastSweep< Float64, CTCylinder< Float64 > >( "cylinder64", 112, 10000, s2 );
        ASSERT_TRUE( SweepPassed( s2 ) );
        SRaySweep s3;
        RayCastSweep< Float32, CTCone< Float32 > >( "cone32", 113, 10000, s3 );
        ASSERT_TRUE( SweepPassed( s3 ) );
        SRaySweep s4;
        RayCastSweep< Float64, CTCone< Float64 > >( "cone64", 114, 10000, s4 );
        ASSERT_TRUE( SweepPassed( s4 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RayCasts: frustum and plane-bounded volume, 10000 seeded rays each in Float32 and Float64 against marching on Contains" )
    try
    {
        SRaySweep s1;
        RayCastSweep< Float32, CTFrustum< Float32 > >( "frustum32", 121, 10000, s1 );
        ASSERT_TRUE( 0 == s1.bad && s1.hits > s1.rays / 20 && s1.misses > s1.rays / 10 );
        SRaySweep s2;
        RayCastSweep< Float64, CTFrustum< Float64 > >( "frustum64", 122, 10000, s2 );
        ASSERT_TRUE( 0 == s2.bad && s2.hits > s2.rays / 20 && s2.misses > s2.rays / 10 );
        SRaySweep s3;
        RayCastSweep< Float32, CTPlaneBoundedVolume< Float32 > >( "volume32", 123, 10000, s3 );
        ASSERT_TRUE( SweepPassed( s3 ) );
        SRaySweep s4;
        RayCastSweep< Float64, CTPlaneBoundedVolume< Float64 > >( "volume64", 124, 10000, s4 );
        ASSERT_TRUE( SweepPassed( s4 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RayCasts: triangle and plane, 10000 seeded rays against Float64 analytic references" )
    try
    {
        UInt32 hits = 0;
        ASSERT_TRUE( 0 == TrianglePlaneSweep< Float32 >( 131, 10000, hits ) );
        ASSERT_TRUE( hits > 100 );
        ASSERT_TRUE( 0 == TrianglePlaneSweep< Float64 >( 132, 10000, hits ) );
        ASSERT_TRUE( hits > 100 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RayCasts: watertight triangles, 10000 seeded rays through shared edges and vertices hit at least one triangle" )
    try
    {
        UInt32 misses = 0;
        UInt32 moellerMisses = 0;
        WatertightSweep< Float32 >( 141, 10000, misses, moellerMisses );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RayCasts: Float32 watertight misses " << misses << ", the Moeller-Trumbore member misses " << moellerMisses << " of 10000 rays" );
        ASSERT_TRUE( 0 == misses );
        WatertightSweep< Float64 >( 142, 10000, misses, moellerMisses );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RayCasts: Float64 watertight misses " << misses << ", the Moeller-Trumbore member misses " << moellerMisses << " of 10000 rays" );
        ASSERT_TRUE( 0 == misses );

        /* the watertight test and the member agree on rays that hit the inside of a triangle */
        const CTTriangle3D< Float64 > t( TFloat64Point3D( 0, 0, 0 ), TFloat64Point3D( 4, 0, 0 ), TFloat64Point3D( 0, 4, 0 ) );
        const CTRay3D< Float64 > ray( TFloat64Point3D( 1, 1, 5 ), TFloat64DirectionVector3D( TFloat64Vector3D( 0, 0, -1 ) ) );
        SRayCastResult< Float64 > a;
        SRayCastResult< Float64 > b;
        ASSERT_TRUE( RayCastTriangleWatertight( ray, t, 100.0, a ) && t.RayCast( ray, 100.0, b ) );
        ASSERT_TRUE( NearTo( a.distance, 5.0, 1e-12 ) && NearTo( a.distance, b.distance, 1e-12 ) && NearTo( a.u, b.u, 1e-12 ) && NearTo( a.v, b.v, 1e-12 ) );
        ASSERT_TRUE( NearTo( a.normal.z, 1.0, 1e-12 ) );
        /* from behind the normal faces the ray */
        const CTRay3D< Float64 > behind( TFloat64Point3D( 1, 1, -5 ), TFloat64DirectionVector3D( TFloat64Vector3D( 0, 0, 1 ) ) );
        ASSERT_TRUE( RayCastTriangleWatertight( behind, t, 100.0, a ) && NearTo( a.normal.z, -1.0, 1e-12 ) );
        /* beyond the maximum distance, parallel to the plane, and a degenerate triangle miss */
        ASSERT_FALSE( RayCastTriangleWatertight( ray, t, 4.0, a ) );
        ASSERT_FALSE( RayCastTriangleWatertight( CTRay3D< Float64 >( TFloat64Point3D( 1, 1, 0 ), TFloat64DirectionVector3D( TFloat64Vector3D( 1, 0, 0 ) ) ), t, 100.0, a ) );
        const CTTriangle3D< Float64 > flat( TFloat64Point3D( 0, 0, 0 ), TFloat64Point3D( 1, 0, 0 ), TFloat64Point3D( 3, 0, 0 ) );
        ASSERT_FALSE( RayCastTriangleWatertight( ray, flat, 100.0, a ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RayCasts: rays that start inside, graze, run along a face or are parallel on the boundary give finite results" )
    try
    {
        typedef TFloat64Point3D P;
        typedef TFloat64DirectionVector3D D;
        typedef TFloat64Vector3D V;
        SRayCastResult< Float64 > hit;

        /* the box: a ray along an edge-parallel face on the boundary, inside, tangent */
        const CTAxisAlignedBox3D< Float64 > box( P( -1, -1, -1 ), P( 1, 1, 1 ) );
        ASSERT_TRUE( box.RayCast( CTRay3D< Float64 >( P( -3, 1, 0.5 ), D( V( 1, 0, 0 ) ) ), 10.0, hit ) && NearTo( hit.distance, 2.0, 1e-12 ) && hit.point.IsFinite() && hit.normal.IsFinite() );
        ASSERT_TRUE( box.RayCast( CTRay3D< Float64 >( P( 0, 0, 0 ), D( V( 1, 2, 3 ) ) ), 10.0, hit ) && 0.0 == hit.distance && hit.StartedInside() );

        /* the oriented box: rotated a quarter turn about Z, the same rays */
        const CTOrientedBox3D< Float64 > obb( P( 0, 0, 0 ), CTQuaternion< Float64 >::FromAxisAngle( D( V( 0, 0, 1 ) ), CTRadian< Float64 >( 1.5707963267948966 ) ), V( 1, 2, 1 ) );
        ASSERT_TRUE( obb.RayCast( CTRay3D< Float64 >( P( -5, 1, 0 ), D( V( 1, 0, 0 ) ) ), 20.0, hit ) && hit.point.IsFinite() && NearTo( hit.distance, 3.0, 1e-9 ) );
        ASSERT_TRUE( obb.RayCast( CTRay3D< Float64 >( P( 0, 0, 0.5 ), D( V( 0, 1, 0 ) ) ), 20.0, hit ) && hit.StartedInside() );
        ASSERT_FALSE( obb.RayCast( CTRay3D< Float64 >( P( -5, 1.01, 0 ), D( V( 1, 0, 0 ) ) ), 20.0, hit ) );

        /* the cylinder: along the axis, along the wall, in the plane of a cap, a ray that touches the rim */
        const CTCylinder< Float64 > cylinder( P( 0, 0, 0 ), P( 0, 0, 4 ), 1.0 );
        ASSERT_TRUE( cylinder.RayCast( CTRay3D< Float64 >( P( 0.5, 0, -3 ), D( V( 0, 0, 1 ) ) ), 20.0, hit ) && NearTo( hit.distance, 3.0, 1e-12 ) && 1 == hit.featureIndex );
        ASSERT_TRUE( cylinder.RayCast( CTRay3D< Float64 >( P( 3, 0, 4 ), D( V( -1, 0, 0 ) ) ), 20.0, hit ) && hit.point.IsFinite() && hit.normal.IsFinite() );
        ASSERT_TRUE( cylinder.RayCast( CTRay3D< Float64 >( P( 1, 0, -3 ), D( V( 0, 0, 1 ) ) ), 20.0, hit ) && hit.point.IsFinite() );
        ASSERT_FALSE( cylinder.RayCast( CTRay3D< Float64 >( P( 1.01, 0, -3 ), D( V( 0, 0, 1 ) ) ), 20.0, hit ) );
        ASSERT_TRUE( cylinder.RayCast( CTRay3D< Float64 >( P( -3, 0, 2 ), D( V( 1, 0, 0 ) ) ), 20.0, hit ) && NearTo( hit.distance, 2.0, 1e-12 ) && 0 == hit.featureIndex && NearTo( hit.normal.x, -1.0, 1e-12 ) );

        /* the cone: along a generator, from the apex axis, into the base */
        const CTCone< Float64 > cone( P( 0, 0, 0 ), D( V( 0, 0, 1 ) ), 2.0, 1.0 );
        ASSERT_TRUE( cone.RayCast( CTRay3D< Float64 >( P( 0, 0, 5 ), D( V( 0, 0, -1 ) ) ), 20.0, hit ) && NearTo( hit.distance, 3.0, 1e-12 ) && 1 == hit.featureIndex );
        ASSERT_TRUE( cone.RayCast( CTRay3D< Float64 >( P( -3, 0, 1 ), D( V( 1, 0, 0 ) ) ), 20.0, hit ) && NearTo( hit.distance, 2.5, 1e-12 ) && 0 == hit.featureIndex );
        ASSERT_FALSE( cone.RayCast( CTRay3D< Float64 >( P( 0, 0, -1 ), D( V( 0, 0, -1 ) ) ), 20.0, hit ) );            /* behind the apex: the mirrored cone is not hit */
        ASSERT_FALSE( cone.RayCast( CTRay3D< Float64 >( P( -3, 0, -1 ), D( V( 1, 0, 0 ) ) ), 20.0, hit ) );
        {
            /* along the slant line itself: the ray from outside lies on the surface */
            const Float64 s = 1.0 / sqrt( 5.0 );
            const bool along = cone.RayCast( CTRay3D< Float64 >( P( -1, 0, 2 ), D( V( 1 * s, 0, -2 * s ) ) ), 20.0, hit );
            ASSERT_TRUE( !along || ( hit.point.IsFinite() && hit.normal.IsFinite() ) );
        }

        /* the sphere: tangent rays and rays starting on the surface */
        const CTSphere< Float64 > sphere( P( 0, 0, 0 ), 1.0 );
        ASSERT_TRUE( sphere.RayCast( CTRay3D< Float64 >( P( -3, 1, 0 ), D( V( 1, 0, 0 ) ) ), 20.0, hit ) && hit.point.IsFinite() && hit.normal.IsFinite() );
        ASSERT_FALSE( sphere.RayCast( CTRay3D< Float64 >( P( -3, 1.0001, 0 ), D( V( 1, 0, 0 ) ) ), 20.0, hit ) );

        /* the frustum and the plane-bounded volume: a ray in a boundary plane, starting inside, parallel to a pair of planes */
        const CTFrustum< Float64 > clip;
        ASSERT_TRUE( clip.RayCast( CTRay3D< Float64 >( P( -3, 1, 0 ), D( V( 1, 0, 0 ) ) ), 20.0, hit ) && hit.point.IsFinite() && hit.normal.IsFinite() && NearTo( hit.distance, 2.0, 1e-12 ) );
        ASSERT_TRUE( clip.RayCast( CTRay3D< Float64 >( P( 0, 0, 0 ), D( V( 0, 0, 1 ) ) ), 20.0, hit ) && hit.StartedInside() && 0.0 == hit.distance );
        ASSERT_FALSE( clip.RayCast( CTRay3D< Float64 >( P( -3, 1.0001, 0 ), D( V( 1, 0, 0 ) ) ), 20.0, hit ) );
        ASSERT_FALSE( clip.RayCast( CTRay3D< Float64 >( P( -3, 0, 0 ), D( V( 1, 0, 0 ) ) ), 1.0, hit ) );             /* too short */
        const CTPlaneBoundedVolume< Float64 > everything;
        ASSERT_TRUE( everything.RayCast( CTRay3D< Float64 >( P( 7, 7, 7 ), D( V( 1, 0, 0 ) ) ), 20.0, hit ) && hit.StartedInside() );

        /* a hit beyond the maximum distance is a miss, also when it starts inside nothing */
        ASSERT_FALSE( box.RayCast( CTRay3D< Float64 >( P( -3, 0, 0 ), D( V( 1, 0, 0 ) ) ), 1.5, hit ) );
        ASSERT_TRUE( box.RayCast( CTRay3D< Float64 >( P( -3, 0, 0 ), D( V( 1, 0, 0 ) ) ), 2.0, hit ) );

        /* the facade forwards to the shapes */
        ASSERT_TRUE( RayCast( CTRay3D< Float64 >( P( -3, 0, 0 ), D( V( 1, 0, 0 ) ) ), box, 10.0, hit ) && NearTo( hit.distance, 2.0, 1e-12 ) );
        ASSERT_TRUE( Intersects( CTRay3D< Float64 >( P( -3, 0, 0 ), D( V( 1, 0, 0 ) ) ), sphere ) && Intersects( cylinder, CTRay3D< Float64 >( P( -3, 0, 1 ), D( V( 1, 0, 0 ) ) ) ) );
        ASSERT_FALSE( Intersects( CTRay3D< Float64 >( P( -3, 0, 0 ), D( V( -1, 0, 0 ) ) ), box ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    GUCEF_TESTFW_TESTCASE( "RayCasts: a ray through the apex of a cone with a direction inside the cone hits the apex" )
    try
    {
        typedef TFloat64Point3D P;
        typedef TFloat64DirectionVector3D D;
        typedef TFloat64Vector3D V;
        const CTCone< Float64 > cone( P( 0, 0, 0 ), D( V( 0, 1, 0 ) ), 2.0, 1.0 );
        SRayCastResult< Float64 > hit;
        ASSERT_TRUE( cone.RayCast( CTRay3D< Float64 >( P( 0, -3, 0 ), D( V( 0, 1, 0 ) ) ), 20.0, hit ) && NearTo( hit.distance, 3.0, 1e-12 ) && 0 == hit.featureIndex );
        ASSERT_TRUE( NearTo( hit.point.y, 0.0, 1e-12 ) && NearTo( hit.normal.y, -1.0, 1e-12 ) );
        ASSERT_FALSE( cone.RayCast( CTRay3D< Float64 >( P( 0, -3, 0 ), D( V( 0, 1, 0 ) ) ), 2.5, hit ) );
        /* the same ray in Float32, and from the other side along the axis (the base is entered) */
        const CTCone< Float32 > cone32( TFloat32Point3D( 0, 0, 0 ), TFloat32DirectionVector3D( TFloat32Vector3D( 0, 1, 0 ) ), 2.0f, 1.0f );
        SRayCastResult< Float32 > hit32;
        ASSERT_TRUE( cone32.RayCast( CTRay3D< Float32 >( TFloat32Point3D( 0, -3, 0 ), TFloat32DirectionVector3D( TFloat32Vector3D( 0, 1, 0 ) ) ), 20.0f, hit32 ) && hit32.distance == 3.0f );
        ASSERT_TRUE( cone.RayCast( CTRay3D< Float64 >( P( 0, 5, 0 ), D( V( 0, -1, 0 ) ) ), 20.0, hit ) && NearTo( hit.distance, 3.0, 1e-12 ) && 1 == hit.featureIndex );
        /* a ray that passes the apex outside the cone, and one that misses the apex by a little, do not hit the apex */
        ASSERT_FALSE( cone.RayCast( CTRay3D< Float64 >( P( -3, 0, 0 ), D( V( 1, 0, 0 ) ) ), 20.0, hit ) );
        ASSERT_FALSE( cone.RayCast( CTRay3D< Float64 >( P( 0.1, -3, 0 ), D( V( 0, 1, 0 ) ) ), 20.0, hit ) && hit.distance < 3.0 );
        ASSERT_TRUE( 0 == ApexRaySweep< Float32 >( 151, 3000 ) );
        ASSERT_TRUE( 0 == ApexRaySweep< Float64 >( 152, 3000 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "RayCasts: rays and shapes that contain a NaN hit nothing, for every solid, the plane, the triangle and the watertight test" )
    try
    {
        ASSERT_TRUE( NotANumberCasts< Float32 >() );
        ASSERT_TRUE( NotANumberCasts< Float64 >() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
