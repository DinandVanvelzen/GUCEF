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
#include <algorithm>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GEOMETRY_MESHQUERIES_H
#include "gucefGEOMETRY_MeshQueries.h"
#define GUCEF_GEOMETRY_MESHQUERIES_H
#endif /* GUCEF_GEOMETRY_MESHQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_CTDYNAMICAABBTREE_H
#include "gucefGEOMETRY_CTDynamicAabbTree.h"
#define GUCEF_GEOMETRY_CTDYNAMICAABBTREE_H
#endif /* GUCEF_GEOMETRY_CTDYNAMICAABBTREE_H ? */

#include "TestMeshReference.h"
#include "TestConvexReference.h"
#include "TestGeometryReference.h"
#include "TestGeometryAllocationCounter.h"
#include "TestMeshBvh.h"

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
using namespace MeshReference;
using ConvexReference::ScaledCount;

/*
 *  Functions marked GUCEF_TEST_CONTRACTING are compiled for FMA with floating-point contraction enabled (the test
 *  application builds with contraction off), as in TestQueryFpContraction.cpp. Only on x86 with GCC or Clang.
 */
#if ( defined( __GNUC__ ) || defined( __clang__ ) ) && ( defined( __x86_64__ ) || defined( __i386__ ) )
  #define GUCEF_TEST_MESH_CONTRACTION_SUPPORTED
  #if defined( __clang__ )
    #define GUCEF_TEST_MESH_CONTRACTING  __attribute__(( target( "fma" ), noinline ))
    #define GUCEF_TEST_MESH_CONTRACT_HERE  _Pragma( "clang fp contract(fast)" )
  #else
    #define GUCEF_TEST_MESH_CONTRACTING  __attribute__(( target( "fma" ), optimize( "fp-contract=fast" ), noinline ))
    #define GUCEF_TEST_MESH_CONTRACT_HERE
  #endif
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Counts the visited proxies of a tree query */
struct SProxyCountVisitor
{
    UInt32 count;
    bool Visit( Int32 ) { ++count; return true; }
};

/** Counts the visited proxies of a tree ray cast */
template < typename T >
struct SProxyCountRayVisitor
{
    UInt32 count;
    bool Visit( Int32, T& ) { ++count; return true; }
};

/*-------------------------------------------------------------------------*/

/** Orders hits by triangle */
template < typename T >
struct SByTriangle
{
    bool operator()( const SRayCastResult< T >& a, const SRayCastResult< T >& b ) const { return a.featureIndex < b.featureIndex; }
};

/*-------------------------------------------------------------------------*/

/** A random ray: from a random origin, in a random direction or aimed at a random vertex or edge midpoint of the mesh */
template < typename T >
static CTRay3D< T >
RandomMeshRay( CRandom& random, const CTTriangleMesh< T >& mesh, Float64 extent )
{
    const P3 origin = random.Point( extent );
    const Float64 pick = random.Uniform( 0.0, 1.0 );
    V3 direction;
    if ( pick < 0.5 )
    {
        direction = random.Direction().AsVector();
    }
    else
    {
        const Int32 triangle = random.Index( (Int32) mesh.GetTriangleCount() );
        const Int32 corner = random.Index( 3 );
        const CTTriangle3D< T > t = mesh.GetTriangle( (UInt32) triangle );
        const P3 a = ToP64( t.GetVertex( (UInt32) corner ) );
        const P3 b = ToP64( t.GetVertex( ( (UInt32) corner + 1 ) % 3 ) );
        const P3 target = pick < 0.75 ? a : P3( ( a.ToPositionVector() + b.ToPositionVector() ) * 0.5 );
        direction = target - origin;
    }
    D3 unit;
    if ( !D3::TryCreate( direction, unit ) )
        unit = D3( V3( 1, 0, 0 ) );
    return CTRay3D< T >( ToP< T >( origin ), ToD< T >( unit ) );
}

/*-------------------------------------------------------------------------*/

/** BVH ray casts against the scan over all triangles: same hit or miss, same distance, the triangle equal except ties */
template < typename T >
static bool
RaySweep( const char* name, const CTTriangleMesh< T >& mesh, UInt64 seed, UInt32 rayCount, Float64 extent )
{
    CTMeshBvh< T > bvh;
    if ( !bvh.Build( mesh ) )
        return false;
    CRandom random( seed );
    UInt32 bad = 0;
    UInt32 hits = 0;
    UInt32 ties = 0;
    GUCEF::vector< SRayCastResult< T > > all;
    GUCEF::vector< SRayCastResult< T > > reference;
    for ( UInt32 n=0; n<rayCount && bad<10; ++n )
    {
        const CTRay3D< T > ray = RandomMeshRay( random, mesh, extent );
        const bool limited = random.Chance( 0.3 );
        const Float64 limit = random.Uniform( 0.0, 2.0 * extent );
        const T maxDistance = limited ? (T) limit : CTNumericTraits< T >::Max();

        SRayCastResult< T > expected;
        SRayCastResult< T > actual;
        const bool expectHit = BruteRayCast( mesh, ray, maxDistance, expected );
        const bool gotHit = bvh.RayCast( ray, maxDistance, actual );
        if ( expectHit != gotHit || ( expectHit && !( expected.distance == actual.distance ) ) || gotHit != bvh.RayTest( ray, maxDistance ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": ray " << n << " (seed " << seed << ") hit " << gotHit << " at " << actual.distance << " triangle " << actual.featureIndex <<
                             ", scan " << expectHit << " at " << expected.distance << " triangle " << expected.featureIndex );
            ++bad;
            continue;
        }
        if ( expectHit )
        {
            ++hits;
            if ( expected.featureIndex != actual.featureIndex )
            {
                /* a tie: the other triangle is hit at exactly the same distance */
                SRayCastResult< T > other;
                if ( !RayCastTriangleWatertight( ray, mesh.GetTriangle( (UInt32) actual.featureIndex ), maxDistance, other ) || !( other.distance == expected.distance ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": ray " << n << " (seed " << seed << ") triangle " << actual.featureIndex << " instead of " << expected.featureIndex );
                    ++bad;
                }
                ++ties;
            }
            const bool back = (T) 0 < mesh.GetTriangle( (UInt32) actual.featureIndex ).GetNormalVector().Dot( ray.direction.AsVector() );
            if ( back != ( 0 != ( actual.flags & RAYHIT_BACKFACE ) ) )
                ++bad;
        }
        if ( 0 == n % 10 )
        {
            bvh.RayCastAll( ray, maxDistance, all, false );
            BruteRayCastAll( mesh, ray, maxDistance, reference );
            std::sort( all.begin(), all.end(), SByTriangle< T >() );
            bool same = all.size() == reference.size();
            for ( size_t i=0; same && i<all.size(); ++i )
                same = all[ i ].featureIndex == reference[ i ].featureIndex && all[ i ].distance == reference[ i ].distance;
            /* sorted: ascending distances */
            bvh.RayCastAll( ray, maxDistance, all, true );
            for ( size_t i=1; same && i<all.size(); ++i )
                same = !( all[ i ].distance < all[ i - 1 ].distance );
            if ( !same )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": RayCastAll of ray " << n << " (seed " << seed << ") has " << all.size() << " hits, the scan " << reference.size() );
                ++bad;
            }
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << rayCount << " rays, " << hits << " hits, " << ties << " exact ties, " << mesh.GetTriangleCount() << " triangles, BVH depth " << bvh.GetDepth() << ", " << bvh.GetNodeCount() << " nodes" );
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** Closest points against the scan, and a few closest points with a limit that excludes the mesh */
template < typename T >
static bool
ClosestPointSweep( const char* name, const CTTriangleMesh< T >& mesh, UInt64 seed, UInt32 pointCount, Float64 extent, Float64 tolerance )
{
    CTMeshBvh< T > bvh;
    bvh.Build( mesh );
    CRandom random( seed );
    UInt32 bad = 0;
    for ( UInt32 n=0; n<pointCount && bad<10; ++n )
    {
        const P3 p64 = random.Point( extent );
        const CTPoint3D< T > p = ToP< T >( p64 );
        UInt32 triangle = 0;
        const T expected = BruteClosest( mesh, p, triangle );
        SMeshClosestPoint< T > closest;
        if ( !bvh.GetClosestPoint( p, CTNumericTraits< T >::Max(), closest ) || !( fabs( (Float64) closest.squaredDistance - (Float64) expected ) <= tolerance * ( 1.0 + (Float64) expected ) ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": point " << n << " (seed " << seed << ") squared distance " << closest.squaredDistance << " triangle " << closest.triangle << ", scan " << expected << " triangle " << triangle );
            ++bad;
            continue;
        }
        /* the point lies on the reported triangle */
        const T onTriangle = ( mesh.GetTriangle( closest.triangle ).ClosestPoint( p ) - p ).GetSquaredLength();
        if ( !( onTriangle == closest.squaredDistance ) )
            ++bad;
        /* a limit below the distance finds nothing, a limit above finds it */
        const T distance = (T) sqrt( (Float64) expected );
        SMeshClosestPoint< T > limited;
        if ( (T) 0 < distance && bvh.GetClosestPoint( p, distance * (T) 0.99, limited ) )
            ++bad;
        if ( !bvh.GetClosestPoint( p, distance * (T) 1.01 + (T) 1e-6, limited ) )
            ++bad;
    }
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** Signed distance of the closed sphere: the sign inside and outside, close to vertices, edge midpoints and faces */
template < typename T >
static bool
SignedDistanceSweep( const char* name, UInt64 seed, UInt32 pointCount )
{
    CTTriangleMesh< T > sphere;
    MakeSphere( sphere, CTPoint3D< T >( 0, 0, 0 ), 1.0, 100, 100 );
    CTMeshBvh< T > bvh;
    bvh.Build( sphere );
    CTTriangleMesh< T > box;
    MakeBox( box, CTPoint3D< T >( -1, -1, -1 ), CTPoint3D< T >( 1, 1, 1 ), 4 );
    CTMeshBvh< T > boxBvh;
    boxBvh.Build( box );
    CRandom random( seed );
    UInt32 bad = 0;
    for ( UInt32 n=0; n<pointCount && bad<10; ++n )
    {
        /* far from the surface: well inside or outside */
        const D3 direction = random.Direction();
        const bool inside = random.Chance( 0.5 );
        const Float64 radius = inside ? random.Uniform( 0.0, 0.9 ) : random.Uniform( 1.1, 3.0 );
        CTPoint3D< T > p = ToP< T >( P3( direction.AsVector() * radius ) );
        T signedDistance = 0;
        if ( !bvh.GetSignedDistance( p, CTNumericTraits< T >::Max(), signedDistance, GUCEF_NULL ) || inside != ( signedDistance < (T) 0 ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": point " << n << " (seed " << seed << ") at radius " << radius << " signed distance " << signedDistance );
            ++bad;
        }

        /* just inside and outside a vertex, an edge midpoint and a face centroid (the polyhedron is convex around the origin) */
        const Int32 triangle = random.Index( (Int32) sphere.GetTriangleCount() );
        const CTTriangle3D< T > t = sphere.GetTriangle( (UInt32) triangle );
        const Int32 which = random.Index( 3 );
        const P3 a = ToP64( t.a );
        const P3 b = ToP64( t.b );
        const P3 c = ToP64( t.c );
        P3 feature = a;
        if ( 1 == which )
            feature = P3( ( a.ToPositionVector() + b.ToPositionVector() ) * 0.5 );
        else if ( 2 == which )
            feature = P3( ( a.ToPositionVector() + b.ToPositionVector() + c.ToPositionVector() ) / 3.0 );
        const bool out = random.Chance( 0.5 );
        p = ToP< T >( P3( feature.ToPositionVector() * ( out ? 1.001 : 0.999 ) ) );
        SMeshClosestPoint< T > closest;
        if ( !bvh.GetSignedDistance( p, CTNumericTraits< T >::Max(), signedDistance, &closest ) || out != !( signedDistance < (T) 0 ) || ( out && (T) 0 == signedDistance ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": near feature " << which << " of triangle " << triangle << " (seed " << seed << ", " << ( out ? "outside" : "inside" ) << ") signed distance " << signedDistance <<
                             " feature " << (Int32) closest.feature );
            ++bad;
        }

        /* the box: near its edges and corners */
        const V3 corner( random.Chance( 0.5 ) ? 1.0 : -1.0, random.Chance( 0.5 ) ? 1.0 : -1.0, random.Chance( 0.5 ) ? 1.0 : -1.0 );
        const Float64 along = random.Uniform( -1.0, 1.0 );
        V3 q = corner;
        if ( random.Chance( 0.5 ) )
            q.x = along;            /* on an edge, else the corner itself */
        const bool boxOut = random.Chance( 0.5 );
        const P3 probe( q * ( boxOut ? 1.001 : 0.999 ) );
        if ( !boxBvh.GetSignedDistance( ToP< T >( probe ), CTNumericTraits< T >::Max(), signedDistance, GUCEF_NULL ) || boxOut == ( signedDistance < (T) 0 ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": box probe " << probe.ToString() << " (seed " << seed << ") signed distance " << signedDistance );
            ++bad;
        }
    }
    /* the pseudo-normals of the closed sphere: unit vertex normals pointing out, edge normals the sum of two face normals */
    for ( UInt32 v=0; v<sphere.GetVertexCount(); ++v )
    {
        const CTVector3D< T >& normal = bvh.GetVertexPseudoNormal( v );
        if ( fabs( (Float64) normal.GetLength() - 1.0 ) > 1e-5 || !( (T) 0 < normal.Dot( sphere.vertices[ v ].ToPositionVector() ) ) )
            ++bad;
    }
    for ( UInt32 tri=0; tri<sphere.GetTriangleCount(); tri+=97 )
    {
        for ( UInt32 e=0; e<3; ++e )
        {
            const Float64 length = (Float64) bvh.GetEdgePseudoNormal( tri, e ).GetLength();
            if ( !( length > 1.9 && length <= 2.0 + 1e-5 ) )
                ++bad;
        }
    }
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** Overlap of spheres, boxes, oriented boxes and capsules against the scan of Intersects over all triangles */
template < typename T >
static bool
OverlapSweep( const char* name, const CTTriangleMesh< T >& mesh, UInt64 seed, UInt32 queryCount, Float64 extent )
{
    CTMeshBvh< T > bvh;
    bvh.Build( mesh );
    CRandom random( seed );
    UInt32 bad = 0;
    GUCEF::vector< UInt32 > found;
    GUCEF::vector< UInt32 > expected;
    for ( UInt32 n=0; n<queryCount && bad<10; ++n )
    {
        const P3 centre = random.Point( extent );
        const Float64 size = random.Uniform( 0.01, extent * 0.3 );
        const Int32 kind = random.Index( 4 );
        found.clear();
        expected.clear();
        bool any = false;
        if ( 0 == kind )
        {
            const CTSphere< T > shape( ToP< T >( centre ), (T) size );
            bvh.Overlap( shape, found );
            any = bvh.Overlaps( shape );
            for ( UInt32 t=0; t<mesh.GetTriangleCount(); ++t )
                if ( Intersects( mesh.GetTriangle( t ), shape ) )
                    expected.push_back( t );
        }
        else if ( 1 == kind )
        {
            const V3 half( size, size * 0.5, size * 1.5 );
            const CTAxisAlignedBox3D< T > shape( ToP< T >( centre - half ), ToP< T >( centre + half ) );
            bvh.Overlap( shape, found );
            any = bvh.Overlaps( shape );
            for ( UInt32 t=0; t<mesh.GetTriangleCount(); ++t )
                if ( Intersects( mesh.GetTriangle( t ), shape ) )
                    expected.push_back( t );
        }
        else if ( 2 == kind )
        {
            const Q4 rotation = random.Rotation();
            const CTOrientedBox3D< T > shape( ToP< T >( centre ), ToQ< T >( rotation ), CTVector3D< T >( (T) size, (T) ( size * 0.3 ), (T) ( size * 0.7 ) ) );
            bvh.Overlap( shape, found );
            any = bvh.Overlaps( shape );
            for ( UInt32 t=0; t<mesh.GetTriangleCount(); ++t )
                if ( Intersects( mesh.GetTriangle( t ), shape ) )
                    expected.push_back( t );
        }
        else
        {
            const V3 axis = random.Direction().AsVector() * size;
            const CTCapsule< T > shape( ToP< T >( centre - axis ), ToP< T >( centre + axis ), (T) ( size * 0.2 ) );
            bvh.Overlap( shape, found );
            any = bvh.Overlaps( shape );
            for ( UInt32 t=0; t<mesh.GetTriangleCount(); ++t )
                if ( Intersects( mesh.GetTriangle( t ), shape ) )
                    expected.push_back( t );
        }
        std::sort( found.begin(), found.end() );
        if ( found != expected || any != !expected.empty() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": overlap query " << n << " kind " << kind << " (seed " << seed << ") found " << found.size() << " triangles, the scan " << expected.size() );
            ++bad;
        }
    }

    /* QueryBox reports exactly the triangles whose boxes touch the box, and stops when asked */
    for ( UInt32 n=0; n<50; ++n )
    {
        const P3 centre = random.Point( extent );
        const V3 half( extent * 0.2, extent * 0.1, extent * 0.3 );
        const CTAxisAlignedBox3D< T > box( ToP< T >( centre - half ), ToP< T >( centre + half ) );
        found.clear();
        expected.clear();
        SCollectTriangles collect;
        collect.triangles = &found;
        collect.stopAfter = 0xFFFFFFFF;
        bvh.QueryBox( box, collect );
        for ( UInt32 t=0; t<mesh.GetTriangleCount(); ++t )
            if ( mesh.GetTriangle( t ).GetBounds().Intersects( box ) )
                expected.push_back( t );
        std::sort( found.begin(), found.end() );
        if ( found != expected )
            ++bad;
        found.clear();
        collect.stopAfter = 1;
        bvh.QueryBox( box, collect );
        if ( found.size() > 1 || ( expected.empty() != found.empty() ) )
            ++bad;
    }
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** The mesh queries of section 7.5.7 against the scans: closest points, deepest contact, sphere and capsule sweeps */
template < typename T >
static bool
MeshQuerySweep( const char* name, const CTTriangleMesh< T >& mesh, UInt64 seed, UInt32 queryCount, Float64 extent, Float64 tolerance )
{
    CTMeshBvh< T > bvh;
    bvh.Build( mesh );
    CRandom random( seed );
    UInt32 bad = 0;
    GUCEF::vector< SMeshTriangleContact< T > > contacts;
    for ( UInt32 n=0; n<queryCount && bad<10; ++n )
    {
        const P3 centre = random.Point( extent );
        const Float64 radius = random.Uniform( 0.01, extent * 0.2 );
        const CTSphere< T > sphere( ToP< T >( centre ), (T) radius );

        /* closest points: the smallest distance over the triangles */
        Float64 bestDistance = 1e300;
        for ( UInt32 t=0; t<mesh.GetTriangleCount(); ++t )
        {
            CTPoint3D< T > a;
            CTPoint3D< T > b;
            if ( ClosestPoints( mesh.GetTriangle( t ), sphere, a, b ) )
                bestDistance = std::min( bestDistance, (Float64) ( b - a ).GetLength() );
        }
        CTPoint3D< T > onMesh;
        CTPoint3D< T > onShape;
        UInt32 triangle = 0;
        if ( !MeshClosestPoints( bvh, sphere, onMesh, onShape, &triangle ) || fabs( (Float64) ( onShape - onMesh ).GetLength() - bestDistance ) > tolerance * ( 1.0 + bestDistance ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": closest points " << n << " (seed " << seed << ") distance " << ( onShape - onMesh ).GetLength() << ", scan " << bestDistance );
            ++bad;
        }

        /* contact: the touching triangles and the deepest depth */
        UInt32 touching = 0;
        Float64 deepest = -1e300;
        for ( UInt32 t=0; t<mesh.GetTriangleCount(); ++t )
        {
            CTContactManifold< T > manifold;
            if ( ComputeContact( mesh.GetTriangle( t ), sphere, manifold ) && manifold.IsTouching() )
            {
                ++touching;
                deepest = std::max( deepest, (Float64) manifold.GetMaxDepth() );
            }
        }
        CTContactManifold< T > manifold;
        contacts.clear();
        const bool touches = MeshComputeContact( bvh, sphere, manifold, &contacts );
        if ( touches != ( 0 < touching ) || contacts.size() != touching || ( touches && fabs( (Float64) manifold.GetMaxDepth() - deepest ) > tolerance * ( 1.0 + fabs( deepest ) ) ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": contact " << n << " (seed " << seed << ") " << contacts.size() << " touching, scan " << touching << ", depth " << manifold.GetMaxDepth() << " scan " << deepest );
            ++bad;
        }
        if ( touches != MeshIntersects( bvh, sphere ) )
            ++bad;

        /* sphere cast and oriented box cast: the nearest hit over the triangles */
        const D3 direction = random.Direction();
        const CTRay3D< T > ray( ToP< T >( centre ), ToD< T >( direction ) );
        const T maxDistance = (T) ( extent * 2.0 );
        bool expectHit = false;
        Float64 expectDistance = 0;
        for ( UInt32 t=0; t<mesh.GetTriangleCount(); ++t )
        {
            SRayCastResult< T > hit;
            if ( SphereCast( ray, (T) radius, mesh.GetTriangle( t ), maxDistance, hit ) && ( !expectHit || (Float64) hit.distance < expectDistance ) )
            {
                expectHit = true;
                expectDistance = (Float64) hit.distance;
            }
        }
        SRayCastResult< T > cast;
        const bool gotHit = MeshSphereCast( ray, (T) radius, bvh, maxDistance, cast );
        if ( gotHit != expectHit || ( gotHit && !( (Float64) cast.distance == expectDistance ) ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": sphere cast " << n << " (seed " << seed << ") hit " << gotHit << " at " << cast.distance << ", scan " << expectHit << " at " << expectDistance );
            ++bad;
        }

        /* a capsule (shapes without margin: BoxCastHitsTouch) */
        const V3 capsuleAxis = random.Direction().AsVector() * radius;
        const CTCapsule< T > box( ToP< T >( centre - capsuleAxis ), ToP< T >( centre + capsuleAxis ), (T) ( radius * 0.5 ) );
        expectHit = false;
        for ( UInt32 t=0; t<mesh.GetTriangleCount(); ++t )
        {
            SRayCastResult< T > hit;
            if ( ShapeCast( box, ToD< T >( direction ), maxDistance, mesh.GetTriangle( t ), hit ) && ( !expectHit || (Float64) hit.distance < expectDistance ) )
            {
                expectHit = true;
                expectDistance = (Float64) hit.distance;
            }
        }
        const bool gotBoxHit = MeshShapeCast( box, ToD< T >( direction ), maxDistance, bvh, cast );
        if ( gotBoxHit != expectHit || ( gotBoxHit && !( (Float64) cast.distance == expectDistance ) ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": capsule cast " << n << " (seed " << seed << ") hit " << gotBoxHit << " at " << cast.distance << ", scan " << expectHit << " at " << expectDistance );
            ++bad;
        }
    }
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** Refit after the vertices move: ray casts equal the scan over the moved mesh; a changed triangle count is refused */
template < typename T >
static bool
RefitCase( const char* name, UInt64 seed )
{
    CRandom random( seed );
    CTTriangleMesh< T > mesh;
    MakeSoup( mesh, random, 2000, 10.0, 1.0 );
    CTMeshBvh< T > bvh;
    bvh.Build( mesh );
    /* every vertex moves: a scale, a shift and a wobble */
    for ( UInt32 v=0; v<mesh.GetVertexCount(); ++v )
    {
        const V3 wobble = random.Vector( 0.5 );
        mesh.vertices[ v ] = ToP< T >( P3( ToP64( mesh.vertices[ v ] ).ToPositionVector() * 1.5 + V3( 3, -2, 1 ) + wobble ) );
    }
    if ( !bvh.Refit() )
        return false;
    UInt32 bad = 0;
    for ( UInt32 n=0; n<ScaledCount( 3000 ); ++n )
    {
        const CTRay3D< T > ray = RandomMeshRay( random, mesh, 20.0 );
        SRayCastResult< T > expected;
        SRayCastResult< T > actual;
        const bool expectHit = BruteRayCast( mesh, ray, CTNumericTraits< T >::Max(), expected );
        if ( expectHit != bvh.RayCast( ray, CTNumericTraits< T >::Max(), actual ) || ( expectHit && !( expected.distance == actual.distance ) ) )
            ++bad;
    }
    mesh.AddTriangle( 0, 1, 2 );
    if ( bvh.Refit() )
        ++bad;
    bvh.Clear();
    if ( bvh.IsBuilt() || 0 != bvh.GetNodeCount() || bvh.Refit() )
        ++bad;
    SRayCastResult< T > hit;
    if ( bvh.RayCast( CTRay3D< T >( CTPoint3D< T >( 0, 0, 0 ), CTDirectionVector3D< T >::UnitX() ), (T) 10, hit ) )
        ++bad;
    if ( 0 != bad )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": refit failed " << bad << " times (seed " << seed << ")" );
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** BuildAsync on a pool thread gives the tree of Build; also with an explicit pool */
template < typename T >
static bool
AsyncBuildCase( const char* name )
{
    CTTriangleMesh< T > mesh;
    MakeSphere( mesh, CTPoint3D< T >( 0, 0, 0 ), 1.0, 100, 100 );
    CTMeshBvh< T > sync;
    sync.Build( mesh );
    if ( sync.GetBuildThreadId() != MT::GetCurrentThreadID() )
        return false;

    CTMeshBvh< T > async;
    CORE::CFutureResult future = async.BuildAsync( mesh );
    if ( future.HasNoFuture() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": BuildAsync did not start" );
        return false;
    }
    future.Await( 60000 );
    if ( !future.IsReady() || !async.HasSameTree( sync ) || async.GetBuildThreadId() == MT::GetCurrentThreadID() || 0 == async.GetBuildThreadId() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": BuildAsync ready " << future.IsReady() << ", same tree " << async.HasSameTree( sync ) );
        return false;
    }

    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "GeometryBvhTestPool" );
    CTMeshBvh< T > pooled;
    CORE::CFutureResult other = pooled.BuildAsync( mesh, pool );
    other.Await( 60000 );
    if ( !other.IsReady() || !pooled.HasSameTree( sync ) || pooled.GetBuildThreadId() == MT::GetCurrentThreadID() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": BuildAsync on the explicit pool failed" );
        return false;
    }

    /* a different mesh gives a different tree */
    CTTriangleMesh< T > box;
    MakeBox( box, CTPoint3D< T >( 0, 0, 0 ), CTPoint3D< T >( 1, 1, 1 ), 3 );
    CTMeshBvh< T > boxBvh;
    boxBvh.Build( box );
    return !boxBvh.HasSameTree( sync );
}

/*-------------------------------------------------------------------------*/

/** BuildAsync of an invalid mesh (an index past the vertices) fails as Build does: no future, the hierarchy cleared */
template < typename T >
static bool
AsyncBuildInvalidCase( const char* name )
{
    CTTriangleMesh< T > valid;
    MakeBox( valid, CTPoint3D< T >( 0, 0, 0 ), CTPoint3D< T >( 1, 1, 1 ), 2 );
    CTTriangleMesh< T > invalid = valid;
    invalid.indices[ 0 ] = invalid.GetVertexCount() + 5;
    if ( invalid.IsValid() )
        return false;

    CTMeshBvh< T > sync;
    sync.Build( valid );
    const bool syncResult = sync.Build( invalid );

    CTMeshBvh< T > async;
    async.Build( valid );
    CORE::CFutureResult future = async.BuildAsync( invalid );
    future.Await( 60000 );
    const bool failed = future.IsReady() && future.HasNoFuture();
    if ( syncResult || sync.IsBuilt() || !failed || async.IsBuilt() || 0 != async.GetNodeCount() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": invalid mesh: Build " << syncResult << ", BuildAsync failed " << failed << ", built afterwards " << async.IsBuilt() );
        return false;
    }
    /* a valid mesh still succeeds afterwards */
    CORE::CFutureResult again = async.BuildAsync( valid );
    again.Await( 60000 );
    return again.IsReady() && again.HasAFuture() && async.IsBuilt();
}

/*-------------------------------------------------------------------------*/

/**
 *  Float32 triangles whose centroids lie 2e-38 apart along x (the bin scale 16 / extent overflows to infinity): the
 *  binning still separates the two centroid groups, so the root's first child holds one group only; the input
 *  interleaves the groups, so the fallback split at the middle of the input order would mix them
 */
static bool
TinyCentroidExtentCase( void )
{
    CTTriangleMesh< Float32 > mesh;
    const Float32 tiny = 2e-38f;
    for ( UInt32 t=0; t<8; ++t )
    {
        const Float32 x = 0 == ( t & 1 ) ? 0.0f : tiny;
        const UInt32 a = mesh.AddVertex( CTPoint3D< Float32 >( x, 0.0f, 0.0f ) );
        const UInt32 b = mesh.AddVertex( CTPoint3D< Float32 >( x, 1.0f, 0.0f ) );
        const UInt32 c = mesh.AddVertex( CTPoint3D< Float32 >( x, 0.0f, 1.0f ) );
        mesh.AddTriangle( a, b, c );
    }
    CTMeshBvh< Float32 > bvh;
    if ( !bvh.Build( mesh ) || bvh.GetNodeCount() < 3 )
        return false;
    const CTMeshBvh< Float32 >::SNode& first = bvh.GetNode( 1 );
    if ( 0 == first.count )
        return false;
    const UInt32 group = bvh.GetPrimitive( first.offset ) & 1;
    for ( UInt32 i=0; i<first.count; ++i )
    {
        if ( ( bvh.GetPrimitive( first.offset + i ) & 1 ) != group )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MeshBvh: tiny centroid extent: the first child mixes the centroid groups" );
            return false;
        }
    }
    /* the hierarchy still answers: a ray along x hits the triangles at distance 1 */
    SRayCastResult< Float32 > hit;
    const CTRay3D< Float32 > ray( CTPoint3D< Float32 >( -1.0f, 0.25f, 0.25f ), CTDirectionVector3D< Float32 >::UnitX() );
    return bvh.RayCast( ray, 10.0f, hit ) && 1.0f == hit.distance;
}

/*-------------------------------------------------------------------------*/

/** Structure: every triangle once, leaves of at most 4 triangles, boxes that hold their triangles and children, 32 byte Float32 nodes */
template < typename T >
static bool
StructureCase( const char* name, const CTTriangleMesh< T >& mesh )
{
    CTMeshBvh< T > bvh;
    bvh.Build( mesh );
    bool ok = bvh.GetPrimitiveCount() == mesh.GetTriangleCount();
    GUCEF::vector< UInt32 > seen( mesh.GetTriangleCount(), 0 );
    for ( UInt32 i=0; i<bvh.GetPrimitiveCount(); ++i )
        ++seen[ bvh.GetPrimitive( i ) ];
    for ( size_t i=0; i<seen.size(); ++i )
        if ( 1 != seen[ i ] )
            ok = false;
    for ( UInt32 n=0; n<bvh.GetNodeCount(); ++n )
    {
        const typename CTMeshBvh< T >::SNode& node = bvh.GetNode( n );
        if ( 0 < node.count )
        {
            if ( node.count > 4 )
                ok = false;
            for ( UInt32 i=0; i<node.count; ++i )
                if ( !node.bounds.Contains( mesh.GetTriangle( bvh.GetPrimitive( node.offset + i ) ).GetBounds() ) )
                    ok = false;
        }
        else
        {
            if ( node.offset <= n + 1 || node.offset >= bvh.GetNodeCount() )
                ok = false;
            else if ( !node.bounds.Contains( bvh.GetNode( n + 1 ).bounds ) || !node.bounds.Contains( bvh.GetNode( node.offset ).bounds ) )
                ok = false;
        }
    }
    if ( !( bvh.GetBounds() == mesh.GetBounds() ) )
        ok = false;
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the BVH structure is inconsistent" );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** The empty mesh: built, no nodes, every query empty */
template < typename T >
static bool
EmptyCase( void )
{
    CTTriangleMesh< T > mesh;
    CTMeshBvh< T > bvh;
    if ( !bvh.Build( mesh ) || !bvh.IsBuilt() || 0 != bvh.GetNodeCount() || 0 != bvh.GetDepth() || !bvh.GetBounds().IsEmpty() )
        return false;
    SRayCastResult< T > hit;
    SMeshClosestPoint< T > closest;
    GUCEF::vector< SRayCastResult< T > > hits;
    T signedDistance = 0;
    const CTRay3D< T > ray( CTPoint3D< T >( 0, 0, 0 ), CTDirectionVector3D< T >::UnitX() );
    return !bvh.RayCast( ray, (T) 10, hit ) && !bvh.RayTest( ray, (T) 10 ) && 0 == bvh.RayCastAll( ray, (T) 10, hits, true ) &&
           !bvh.GetClosestPoint( CTPoint3D< T >( 0, 0, 0 ), CTNumericTraits< T >::Max(), closest ) &&
           !bvh.GetSignedDistance( CTPoint3D< T >( 0, 0, 0 ), CTNumericTraits< T >::Max(), signedDistance, GUCEF_NULL ) &&
           !bvh.Overlaps( CTSphere< T >( CTPoint3D< T >( 0, 0, 0 ), (T) 100 ) );
}

/*-------------------------------------------------------------------------*/

/** Results of the precision-sensitive mesh members as raw values, compared bit for bit */
template < typename T >
struct SMeshResults
{
    T values[ 128 ];
    UInt32 count;

    SMeshResults( void ) : count( 0 ) { memset( values, 0, sizeof( values ) ); }
    void Add( const T& v ) { if ( count < 128 ) values[ count++ ] = v; }
    template < typename V > void Add3( const V& v ) { Add( v.x ); Add( v.y ); Add( v.z ); }
};

/** A small mesh from the probe values a and c (shared by the plain and the contracting caller) */
template < typename T >
static void
MakeProbeMesh( CTTriangleMesh< T >& mesh, T a, T c )
{
    const T zero = T( 0 );
    MakeBox( mesh, CTPoint3D< T >( -a, -c, zero ), CTPoint3D< T >( c, a, a + c ), 2 );
    mesh.AddVertex( CTPoint3D< T >( a, c, a + a ) );
    mesh.AddVertex( CTPoint3D< T >( c + c, a, c ) );
    mesh.AddVertex( CTPoint3D< T >( zero, a + c, c ) );
    const UInt32 n = mesh.GetVertexCount();
    mesh.AddTriangle( n - 3, n - 2, n - 1 );
}

/** The calls: a macro, so that each caller compiles them in its own context (inlined there with GUCEF_GEOMETRY_INLINE_ALL) */
#define GUCEF_TEST_CALL_PRECISE_MESH( T, a, c, mesh, bvh, out )                                                          \
    {                                                                                                                     \
        typedef CTPoint3D< T > P;                                                                                         \
        typedef CTVector3D< T > V;                                                                                        \
        typedef CTDirectionVector3D< T > D;                                                                               \
        out.count = 0;                                                                                                    \
        out.Add( mesh.GetVolume() );                                                                                      \
        out.Add( mesh.GetSurfaceArea() );                                                                                 \
        out.Add3( mesh.GetCentroid() );                                                                                   \
        GUCEF::vector< V > normals;                                                                                       \
        mesh.ComputeVertexNormals( normals );                                                                             \
        out.Add3( normals[ 0 ] ); out.Add3( normals[ normals.size() - 1 ] );                                              \
        mesh.ComputeFaceNormals( normals );                                                                               \
        out.Add3( normals[ normals.size() - 1 ] );                                                                        \
        const CTRay3D< T > ray( P( -c - c, a, c ), D( V( c, a * a, a - c ) ) );                                         \
        SRayCastResult< T > hit;                                                                                          \
        out.Add( bvh.RayCast( ray, T( 100 ), hit ) ? hit.distance : T( -1 ) ); out.Add( hit.u ); out.Add( hit.v ); out.Add3( hit.normal ); \
        SMeshClosestPoint< T > closest;                                                                                   \
        out.Add( bvh.GetClosestPoint( P( a + c, a * c, -a ), T( 100 ), closest ) ? closest.squaredDistance : T( -1 ) ); out.Add3( closest.point ); \
        T signedDistance = T( 0 );                                                                                        \
        out.Add( bvh.GetSignedDistance( P( a * c, a, c * T( 0.5 ) ), T( 100 ), signedDistance, GUCEF_NULL ) ? signedDistance : T( -100 ) ); \
        CTPoint3D< T > onMesh; CTPoint3D< T > onShape;                                                                    \
        out.Add( MeshClosestPoints( bvh, CTSphere< T >( P( a * T( 3 ), c, a ), a ), onMesh, onShape, GUCEF_NULL ) ? T( 1 ) : T( 0 ) ); out.Add3( onMesh ); \
        out.Add( MeshSphereCast( ray, c * T( 0.25 ), bvh, T( 100 ), hit ) ? hit.distance : T( -1 ) );                     \
        out.Add( CTDynamicAabbTree< T, UInt32 >::RayOverlapsBox( ray, a * T( 3 ), CTAxisAlignedBox3D< T >( P( -a, -a, -a ), P( a, c, a ) ) ) ? T( 1 ) : T( 0 ) ); \
    }

#ifdef GUCEF_TEST_MESH_CONTRACTION_SUPPORTED

template < typename T >
static void
CallPreciseMeshPlain( T a, T c, const CTTriangleMesh< T >& mesh, const CTMeshBvh< T >& bvh, SMeshResults< T >& out ) GUCEF_TEST_CALL_PRECISE_MESH( T, a, c, mesh, bvh, out )

template < typename T >
GUCEF_TEST_MESH_CONTRACTING static void
CallPreciseMeshContracted( T a, T c, const CTTriangleMesh< T >& mesh, const CTMeshBvh< T >& bvh, SMeshResults< T >& out )
{
    GUCEF_TEST_MESH_CONTRACT_HERE
    GUCEF_TEST_CALL_PRECISE_MESH( T, a, c, mesh, bvh, out )
}

template < typename T >
GUCEF_TEST_MESH_CONTRACTING static T
MeshContractedMultiplyAdd( T a, T b, T c )
{
    GUCEF_TEST_MESH_CONTRACT_HERE
    return a * b + c;
}

/** Plain and contracting callers of one kind: the number of results that differ */
template < typename T >
static UInt32
MeshContractionDifferences( T a, T c, UInt32& count )
{
    CTTriangleMesh< T > mesh;
    MakeProbeMesh( mesh, a, c );
    CTMeshBvh< T > bvh;
    bvh.Build( mesh );
    SMeshResults< T > plain;
    SMeshResults< T > contracted;
    CallPreciseMeshPlain( a, c, mesh, bvh, plain );
    CallPreciseMeshContracted( a, c, mesh, bvh, contracted );
    UInt32 differences = plain.count == contracted.count ? 0 : 1;
    for ( UInt32 i=0; i<plain.count; ++i )
        if ( !GeometryReference::SameBits( plain.values[ i ], contracted.values[ i ] ) )
            ++differences;
    count = plain.count;
    return differences;
}

static bool
MeshCpuHasFma( void )
{
    __builtin_cpu_init();
    return 0 != __builtin_cpu_supports( "fma" );
}

#endif /* GUCEF_TEST_MESH_CONTRACTION_SUPPORTED ? */

/*-------------------------------------------------------------------------*/

/** The queries of a built hierarchy and a tree: no allocation (the accumulator keeps the calls) */
template < typename T >
static void
CallAllocationFreeQueries( const CTMeshBvh< T >& bvh, const CTDynamicAabbTree< T, UInt32 >& tree, UInt32& accumulator )
{
    typedef CTPoint3D< T > P;
    const CTRay3D< T > ray( P( -5, (T) 0.1, (T) 0.2 ), CTDirectionVector3D< T >::UnitX() );
    SRayCastResult< T > hit;
    SMeshClosestPoint< T > closest;
    T signedDistance = 0;
    UInt32 bits = 0;
    bits += bvh.RayCast( ray, (T) 100, hit ) ? 1 : 0;
    bits += bvh.RayTest( ray, (T) 100 ) ? 1 : 0;
    bits += bvh.GetClosestPoint( P( 3, 2, 1 ), (T) 100, closest ) ? 1 : 0;
    bits += bvh.GetSignedDistance( P( (T) 0.1, (T) 0.2, (T) 0.3 ), (T) 100, signedDistance, GUCEF_NULL ) ? 1 : 0;
    bits += bvh.Overlaps( CTSphere< T >( P( 1, 0, 0 ), (T) 0.5 ) ) ? 1 : 0;
    bits += MeshSphereCast( ray, (T) 0.25, bvh, (T) 100, hit ) ? 1 : 0;
    P onMesh;
    P onShape;
    bits += MeshClosestPoints( bvh, CTSphere< T >( P( 4, 0, 0 ), (T) 0.5 ), onMesh, onShape, GUCEF_NULL ) ? 1 : 0;
    SProxyCountVisitor visitor;
    visitor.count = 0;
    tree.Query( CTAxisAlignedBox3D< T >( P( -2, -2, -2 ), P( 2, 2, 2 ) ), visitor );
    SProxyCountRayVisitor< T > rayVisitor;
    rayVisitor.count = 0;
    tree.RayCast( ray, (T) 100, rayVisitor );
    accumulator += bits + visitor.count + rayVisitor.count;
}

/*-------------------------------------------------------------------------*/

/**
 *  Regression of the GJK ray cast (ShapeCast of gucefGEOMETRY_SweepQueries.h, CTGjk::Cast) for shapes without
 *  margin: an oriented box moved against a triangle once reported hits at distances where the moved box did not even
 *  touch the target's bounds (a stop on a repeated support right after an advance). The independent check: at the
 *  reported distance the bounds of the moved box touch the bounds of the target.
 */
static bool
BoxCastHitsTouch( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    UInt32 hits = 0;
    UInt32 bogus = 0;
    for ( UInt32 n=0; n<count; ++n )
    {
        const P3 centre = random.Point( 3.0 );
        const Float64 radius = random.Uniform( 0.05, 0.6 );
        const D3 direction = random.Direction();
        const Q4 rotation = random.Rotation();
        const CTOrientedBox3D< Float64 > moving( centre, rotation, V3( radius, radius * 0.5, radius * 0.8 ) );
        const P3 a = random.Point( 2.0 );
        const V3 e1 = random.Vector( 0.3 );
        const V3 e2 = random.Vector( 0.3 );
        const CTTriangle3D< Float64 > triangle( a, a + e1, a + e2 );
        SRayCastResult< Float64 > hit;
        if ( !ShapeCast( moving, direction, 6.0, triangle, hit ) )
            continue;
        ++hits;
        const V3 move = direction.AsVector() * hit.distance;
        const CTAxisAlignedBox3D< Float64 > bounds = moving.GetBounds();
        if ( !CTAxisAlignedBox3D< Float64 >( bounds.min + move, bounds.max + move ).Expanded( 1e-6 ).Intersects( triangle.GetBounds() ) )
        {
            if ( 0 == bogus )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MeshBvh: ShapeCast of an oriented box against a triangle reports a hit at " << hit.distance << " where the boxes do not touch (seed " << seed << " cast " << n << ")" );
            ++bogus;
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MeshBvh: oriented box casts against triangles: " << hits << " hits, " << bogus << " where the moved box does not touch the triangle" );
    return 0 == bogus;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMeshBvhTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "MeshBvh" );

    CTTriangleMesh< Float64 > sphere;
    MakeSphere( sphere, CTPoint3D< Float64 >( 0.25, -0.5, 0.125 ), 2.0, 100, 100 );
    CTTriangleMesh< Float64 > box;
    MakeBox( box, CTPoint3D< Float64 >( -1.5, -1.0, -2.0 ), CTPoint3D< Float64 >( 1.5, 1.0, 2.0 ), 20 );
    CTTriangleMesh< Float64 > soup;
    CRandom soupRandom( 20701 );
    MakeSoup( soup, soupRandom, 5000, 3.0, 0.6 );
    CTTriangleMesh< Float32 > sphere32;
    MakeSphere( sphere32, CTPoint3D< Float32 >( 0.25f, -0.5f, 0.125f ), 2.0, 100, 100 );
    CTTriangleMesh< Float32 > box32;
    MakeBox( box32, CTPoint3D< Float32 >( -1.5f, -1.0f, -2.0f ), CTPoint3D< Float32 >( 1.5f, 1.0f, 2.0f ), 20 );
    CTTriangleMesh< Float32 > soup32;
    CRandom soupRandom32( 20702 );
    MakeSoup( soup32, soupRandom32, 5000, 3.0, 0.6 );

    GUCEF_TESTFW_TESTCASE( "MeshBvh: structure: every triangle once, leaves of at most 4, boxes hold their content; 32 byte Float32 nodes" )
    try
    {
        ASSERT_TRUE( StructureCase( "sphere", sphere ) );
        ASSERT_TRUE( StructureCase( "box", box ) );
        ASSERT_TRUE( StructureCase( "soup", soup ) );
        ASSERT_TRUE( StructureCase( "soup32", soup32 ) );
        ASSERT_TRUE( 32 == sizeof( CTMeshBvh< Float32 >::SNode ) );
        ASSERT_TRUE( EmptyCase< Float32 >() );
        ASSERT_TRUE( EmptyCase< Float64 >() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MeshBvh: 10^5 seeded rays on a 20k triangle sphere, a soup and a closed box equal the scan (hit, distance, triangle except ties, RayCastAll sets)" )
    try
    {
        ASSERT_TRUE( RaySweep( "rays sphere Float64", sphere, 20711, ScaledCount( 100000 ), 4.0 ) );
        ASSERT_TRUE( RaySweep( "rays soup Float64", soup, 20712, ScaledCount( 100000 ), 5.0 ) );
        ASSERT_TRUE( RaySweep( "rays box Float64", box, 20713, ScaledCount( 100000 ), 4.0 ) );
        ASSERT_TRUE( RaySweep( "rays sphere Float32", sphere32, 20714, ScaledCount( 20000 ), 4.0 ) );
        ASSERT_TRUE( RaySweep( "rays soup Float32", soup32, 20715, ScaledCount( 20000 ), 5.0 ) );
        ASSERT_TRUE( RaySweep( "rays box Float32", box32, 20716, ScaledCount( 20000 ), 4.0 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MeshBvh: 10^4 seeded closest points equal the scan (relative 1e-12 Float64, 1e-6 Float32); limits honoured" )
    try
    {
        ASSERT_TRUE( ClosestPointSweep( "closest sphere Float64", sphere, 20721, ScaledCount( 10000 ), 4.0, 1e-12 ) );
        ASSERT_TRUE( ClosestPointSweep( "closest soup Float64", soup, 20722, ScaledCount( 10000 ), 5.0, 1e-12 ) );
        ASSERT_TRUE( ClosestPointSweep( "closest box Float64", box, 20723, ScaledCount( 5000 ), 4.0, 1e-12 ) );
        ASSERT_TRUE( ClosestPointSweep( "closest sphere Float32", sphere32, 20724, ScaledCount( 3000 ), 4.0, 1e-6 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MeshBvh: signed distance sign inside and outside the closed sphere and box, near vertices, edges and faces; pseudo-normals" )
    try
    {
        ASSERT_TRUE( SignedDistanceSweep< Float64 >( "signed Float64", 20731, ScaledCount( 10000 ) ) );
        ASSERT_TRUE( SignedDistanceSweep< Float32 >( "signed Float32", 20732, ScaledCount( 3000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MeshBvh: Overlap / Overlaps of spheres, boxes, oriented boxes, capsules and QueryBox equal the scan" )
    try
    {
        ASSERT_TRUE( OverlapSweep( "overlap sphere Float64", sphere, 20741, ScaledCount( 2000 ), 3.0 ) );
        ASSERT_TRUE( OverlapSweep( "overlap soup Float64", soup, 20742, ScaledCount( 2000 ), 4.0 ) );
        ASSERT_TRUE( OverlapSweep( "overlap box Float32", box32, 20743, ScaledCount( 2000 ), 3.0 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MeshBvh: mesh queries (closest points, deepest contact, sphere and capsule casts) equal the scans over the triangles" )
    try
    {
        ASSERT_TRUE( MeshQuerySweep( "mesh queries box Float64", box, 20751, ScaledCount( 300 ), 3.0, 1e-9 ) );
        ASSERT_TRUE( MeshQuerySweep( "mesh queries soup Float64", soup, 20752, ScaledCount( 200 ), 4.0, 1e-9 ) );
        ASSERT_TRUE( MeshQuerySweep( "mesh queries box Float32", box32, 20753, ScaledCount( 200 ), 3.0, 1e-4 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MeshBvh: Refit after moving every vertex gives the ray casts of the scan; changed triangle counts and Clear refuse" )
    try
    {
        ASSERT_TRUE( RefitCase< Float64 >( "refit Float64", 20761 ) );
        ASSERT_TRUE( RefitCase< Float32 >( "refit Float32", 20762 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MeshBvh: BuildAsync completes on a pool thread with the same tree as Build (default and explicit pool)" )
    try
    {
        ASSERT_TRUE( AsyncBuildCase< Float64 >( "async Float64" ) );
        ASSERT_TRUE( AsyncBuildCase< Float32 >( "async Float32" ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MeshBvh: BuildAsync of an invalid mesh fails like Build (no future, hierarchy cleared)" )
    try
    {
        ASSERT_TRUE( AsyncBuildInvalidCase< Float64 >( "async invalid Float64" ) );
        ASSERT_TRUE( AsyncBuildInvalidCase< Float32 >( "async invalid Float32" ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MeshBvh: Float32 centroids 2e-38 apart (bin scale overflows) are still split by the binning" )
    try
    {
        ASSERT_TRUE( TinyCentroidExtentCase() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MeshBvh: exported mesh, BVH and tree members give the same bits from contracting FMA code" )
    #ifndef GUCEF_TEST_MESH_CONTRACTION_SUPPORTED
    GUCEF_TESTFW_SKIP( "contraction test needs GCC or Clang on x86" );
    #else
    GUCEF_TESTFW_SKIP_IF( !MeshCpuHasFma(), "the CPU has no FMA" );
    {
        /* the probe values of TestQueryFpContraction: a a - c is 0 unfused and the last bit fused */
        volatile Float32 a32 = 1.0f + ldexpf( 1.0f, -12 );
        volatile Float32 c32 = 1.0f + ldexpf( 1.0f, -11 );
        volatile Float64 a64 = 1.0 + ldexp( 1.0, -27 );
        volatile Float64 c64 = 1.0 + ldexp( 1.0, -26 );
        const Float32 fused32 = MeshContractedMultiplyAdd< Float32 >( a32, a32, -c32 );
        const Float64 fused64 = MeshContractedMultiplyAdd< Float64 >( a64, a64, -c64 );
        GUCEF_TESTFW_SKIP_IF( ldexpf( 1.0f, -24 ) != fused32 || ldexp( 1.0, -54 ) != fused64, "this build does not contract a * b + c (no optimisation), the test cannot detect fusion" );
        try
        {
            UInt32 count32 = 0;
            UInt32 count64 = 0;
            const UInt32 differences = MeshContractionDifferences< Float32 >( a32, c32, count32 ) + MeshContractionDifferences< Float64 >( a64, c64, count64 );
            ASSERT_TRUE( count32 > 20 && count64 > 20 );
            #ifndef GUCEF_GEOMETRY_INLINE_ALL
            /* the module's copies run in both cases: bit identical */
            ASSERT_TRUE( 0 == differences );
            #else
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MeshBvh: GUCEF_GEOMETRY_INLINE_ALL build, " << differences << " of " << ( count32 + count64 ) << " results differ when the caller contracts" );
            #endif
        }
        catch( ... )
        {
            ERRORHERE;
        }
    }
    #endif /* GUCEF_TEST_MESH_CONTRACTION_SUPPORTED ? */
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MeshBvh: BVH queries, mesh queries and tree queries do not allocate" )
    GUCEF_TESTFW_SKIP_IF( !GeometryAllocationCounter::IsAvailable(), "no allocation counter with a sanitizer that replaces the allocator" );
    try
    {
        CTTriangleMesh< Float32 > mesh32;
        MakeBox( mesh32, CTPoint3D< Float32 >( -1, -1, -1 ), CTPoint3D< Float32 >( 1, 1, 1 ), 4 );
        CTMeshBvh< Float32 > bvh32;
        bvh32.Build( mesh32 );
        CTTriangleMesh< Float64 > mesh64;
        MakeBox( mesh64, CTPoint3D< Float64 >( -1, -1, -1 ), CTPoint3D< Float64 >( 1, 1, 1 ), 4 );
        CTMeshBvh< Float64 > bvh64;
        bvh64.Build( mesh64 );
        CTDynamicAabbTree< Float32, UInt32 > tree32;
        CTDynamicAabbTree< Float64, UInt32 > tree64;
        for ( Int32 i=0; i<200; ++i )
        {
            const Float32 x = (Float32) ( i % 10 ) - 5.0f;
            const Float32 y = (Float32) ( i / 10 ) * 0.5f - 5.0f;
            tree32.CreateProxy( CTAxisAlignedBox3D< Float32 >( CTPoint3D< Float32 >( x, y, -0.5f ), CTPoint3D< Float32 >( x + 0.5f, y + 0.4f, 0.5f ) ), (UInt32) i );
            tree64.CreateProxy( CTAxisAlignedBox3D< Float64 >( CTPoint3D< Float64 >( x, y, -0.5 ), CTPoint3D< Float64 >( x + 0.5, y + 0.4, 0.5 ) ), (UInt32) i );
        }
        UInt32 accumulator = 0;
        CallAllocationFreeQueries( bvh32, tree32, accumulator );
        CallAllocationFreeQueries( bvh64, tree64, accumulator );
        GeometryAllocationCounter::StartCounting();
        CallAllocationFreeQueries( bvh32, tree32, accumulator );
        CallAllocationFreeQueries( bvh64, tree64, accumulator );
        const UInt32 allocations = GeometryAllocationCounter::StopCounting();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MeshBvh: " << allocations << " allocations in the queries (" << accumulator << ")" );
        ASSERT_TRUE( 0 == allocations );
        ASSERT_TRUE( accumulator > 20 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MeshBvh: ShapeCast of an oriented box (no margin) hits only where the moved box touches the target" )
    try
    {
        ASSERT_TRUE( BoxCastHitsTouch( 20771, 20000 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
