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

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#include "TestQueryReference.h"
#include "TestPlaneBoundedVolume.h"

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

typedef TFloat64Point3D P;
typedef TFloat64Vector3D V;
typedef TFloat64DirectionVector3D D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** How far inside the box the point is, positive inside */
static Float64
ReferenceMarginBox( const CTAxisAlignedBox3D< Float64 >& box, const P3& p )
{
    return fmin( fmin( fmin( p.x - box.min.x, box.max.x - p.x ), fmin( p.y - box.min.y, box.max.y - p.y ) ), fmin( p.z - box.min.z, box.max.z - p.z ) );
}

static Float64
ReferenceMarginOriented( const CTOrientedBox3D< Float64 >& box, const P3& p )
{
    const V3 local = box.rotation.Conjugate().Rotate( p - box.centre );
    return fmin( fmin( box.halfExtents.x - fabs( local.x ), box.halfExtents.y - fabs( local.y ) ), box.halfExtents.z - fabs( local.z ) );
}

/** The six planes of the box [ -1, 1 ]^3, normals inwards */
static CTPlaneBoundedVolume< Float64 >
UnitCubeVolume( void )
{
    return CTPlaneBoundedVolume< Float64 >::FromAxisAlignedBox( CTAxisAlignedBox3D< Float64 >( P( -1, -1, -1 ), P( 1, 1, 1 ) ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  The projection of a point onto the intersection of half spaces by Dykstra's algorithm: cyclic projections with the
 *  correction terms that make the limit the nearest point of the intersection (not just a point of it). Independent of
 *  the code under test: it uses only the signed distance of the planes.
 */
static P3
DykstraProjection( const CTPlane< Float64 >* planes, UInt32 count, const P3& start, UInt32 sweeps )
{
    V3 increments[ 64 ];
    P3 x = start;
    for ( UInt32 s=0; s<sweeps; ++s )
    {
        for ( UInt32 i=0; i<count; ++i )
        {
            const P3 y = x + increments[ i ];
            const Float64 d = planes[ i ].GetSignedDistance( y );
            const P3 z = d < 0 ? y - planes[ i ].normal.AsVector() * d : y;
            increments[ i ] = y - z;
            x = z;
        }
    }
    return x;
}

/** The largest violation of a plane by the point ( 0 when it is in front of all ) */
static Float64
LargestViolation( const CTPlane< Float64 >* planes, UInt32 count, const P3& p )
{
    Float64 worst = 0;
    for ( UInt32 i=0; i<count; ++i )
        worst = fmax( worst, -planes[ i ].GetSignedDistance( p ) );
    return worst;
}

/*-------------------------------------------------------------------------*/

/** Whether two shapes overlap, by the plane sets that describe them: 1 when a common point is found, -1 when the planes cannot be satisfied together by more than 1e-3, 0 in between */
static int
ReferencePlaneSetOverlap( const CTPlane< Float64 >* a, UInt32 countA, const CTPlane< Float64 >* b, UInt32 countB, const P3& start )
{
    CTPlane< Float64 > all[ 64 ];
    UInt32 count = 0;
    for ( UInt32 i=0; i<countA; ++i )
        all[ count++ ] = a[ i ];
    for ( UInt32 i=0; i<countB; ++i )
        all[ count++ ] = b[ i ];
    const P3 x = DykstraProjection( all, count, start, 4000 );
    const Float64 violation = LargestViolation( all, count, x );
    if ( violation < 1.0e-9 )
        return 1;
    if ( violation > 1.0e-3 )
        return -1;
    return 0;
}

/*-------------------------------------------------------------------------*/

/** The area of a planar polygon from the cross products of its fan */
static Float64
PolygonArea( const P3* points, UInt32 count )
{
    V3 sum;
    for ( UInt32 i=1; i+1<count; ++i )
        sum = sum + ( points[ i ] - points[ 0 ] ).Cross( points[ i + 1 ] - points[ 0 ] );
    return 0.5 * sum.GetLength();
}

/*-------------------------------------------------------------------------*/

/** Seeded random bounded volumes: the convex body has the right shape and agrees with the planes, the sampling and Euler's formula */
template < typename T >
static UInt32
ConvexBodySweep( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    const Float64 tolerance = sizeof( T ) < 8 ? 2.0e-4 : 1.0e-8;
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTPlaneBoundedVolume< T > volume;
        RandomShape( random, volume );
        const CTPlaneBoundedVolume< Float64 > volume64 = Widen( volume );
        GUCEF::vector< CTPoint3D< T > > vertices;
        GUCEF::vector< SConvexBodyFace > faces;
        if ( !volume.BuildConvexBody( vertices, faces ) || vertices.size() < 4 || faces.size() < 4 )
        {
            ++bad;
            continue;
        }

        /* every corner is in the volume and lies on at least three planes */
        for ( size_t v=0; v<vertices.size(); ++v )
        {
            const P3 corner = Widen( vertices[ v ] );
            if ( LargestViolation( volume64.GetPlanes(), volume64.GetPlaneCount(), corner ) > tolerance )
                ++bad;
            UInt32 on = 0;
            for ( UInt32 p=0; p<volume64.GetPlaneCount(); ++p )
            {
                if ( fabs( volume64.GetPlane( p ).GetSignedDistance( corner ) ) < tolerance )
                    ++on;
            }
            if ( on < 3 )
                ++bad;
        }

        /* faces are planar, lie on their plane, run counter-clockwise seen from outside, and the surface closes: V - E + F = 2 */
        UInt32 edgeUses = 0;
        Float64 volumeFromFaces = 0;
        for ( size_t f=0; f<faces.size(); ++f )
        {
            const SConvexBodyFace& face = faces[ f ];
            const size_t n = face.vertexIndices.size();
            if ( n < 3 )
            {
                ++bad;
                continue;
            }
            edgeUses += (UInt32) n;
            P3 ring[ 64 ];
            for ( size_t k=0; k<n && k<64; ++k )
                ring[ k ] = Widen( vertices[ face.vertexIndices[ k ] ] );
            const V3 outward = volume64.GetPlane( face.planeIndex ).normal.AsVector() * -1.0;
            V3 crossSum;
            for ( size_t k=1; k+1<n; ++k )
                crossSum = crossSum + ( ring[ k ] - ring[ 0 ] ).Cross( ring[ k + 1 ] - ring[ 0 ] );
            if ( crossSum.Dot( outward ) <= 0 )
                ++bad;
            volumeFromFaces += ring[ 0 ].ToPositionVector().Dot( outward ) * 0.5 * crossSum.Dot( outward ) / 3.0;
            for ( size_t k=0; k<n && k<64; ++k )
            {
                if ( fabs( volume64.GetPlane( face.planeIndex ).GetSignedDistance( ring[ k ] ) ) > tolerance )
                    ++bad;
            }
        }
        const Float64 edges = (Float64) edgeUses / 2.0;
        if ( (Float64) vertices.size() - edges + (Float64) faces.size() != 2.0 )
            ++bad;

        /* the volume by the divergence theorem equals the volume by counting samples of the bounds */
        CTAxisAlignedBox3D< T > bounds;
        if ( !volume.TryGetBounds( bounds ) )
        {
            ++bad;
            continue;
        }
        const CTAxisAlignedBox3D< Float64 > bounds64 = Widen( bounds );
        for ( size_t v=0; v<vertices.size(); ++v )
        {
            if ( !bounds64.Expanded( tolerance ).Contains( Widen( vertices[ v ] ) ) )
                ++bad;
        }
        if ( 0 == i % 10 )
        {
            UInt32 inside = 0;
            const UInt32 samples = 40000;
            for ( UInt32 s=0; s<samples; ++s )
            {
                const P3 p( random.Uniform( bounds64.min.x, bounds64.max.x ), random.Uniform( bounds64.min.y, bounds64.max.y ), random.Uniform( bounds64.min.z, bounds64.max.z ) );
                if ( volume64.Contains( p ) )
                    ++inside;
            }
            const Float64 estimate = bounds64.GetVolume() * (Float64) inside / (Float64) samples;
            if ( fabs( estimate - volumeFromFaces ) > 0.05 * volumeFromFaces + 1.0e-3 )
                ++bad;
        }
    }
    return bad;
}

/*-------------------------------------------------------------------------*/

/** ClipPolygon of random triangles against random volumes: inside, the area of the clip is the area of the part of the triangle in the volume, and the buffer form equals the vector form */
template < typename T >
static UInt32
ClipPolygonSweep( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    const Float64 tolerance = sizeof( T ) < 8 ? 2.0e-4 : 1.0e-8;
    UInt32 bad = 0;
    UInt32 partial = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTPlaneBoundedVolume< T > volume;
        RandomShape( random, volume );
        const CTPlaneBoundedVolume< Float64 > volume64 = Widen( volume );
        CTPoint3D< T > triangle[ 3 ];
        for ( UInt32 k=0; k<3; ++k )
            triangle[ k ] = ToP< T >( random.Point( 2.0 ) );
        const P3 a = Widen( triangle[ 0 ] );
        const P3 b = Widen( triangle[ 1 ] );
        const P3 c = Widen( triangle[ 2 ] );

        CTPoint3D< T > out[ 40 ];
        CTPoint3D< T > scratch[ 40 ];
        UInt32 outCount = 0;
        if ( !volume.ClipPolygon( triangle, 3, out, scratch, 40, outCount ) )
        {
            ++bad;
            continue;
        }
        GUCEF::vector< CTPoint3D< T > > viaVector;
        volume.ClipPolygon( triangle, 3, viaVector );
        if ( viaVector.size() != outCount )
            ++bad;
        else
        {
            for ( UInt32 k=0; k<outCount; ++k )
            {
                if ( !( viaVector[ k ] == out[ k ] ) )
                    ++bad;
            }
        }

        P3 clipped[ 40 ];
        for ( UInt32 k=0; k<outCount; ++k )
        {
            clipped[ k ] = Widen( out[ k ] );
            if ( LargestViolation( volume64.GetPlanes(), volume64.GetPlaneCount(), clipped[ k ] ) > tolerance )
                ++bad;
        }
        const Float64 area = outCount >= 3 ? PolygonArea( clipped, outCount ) : 0.0;
        const Float64 whole = 0.5 * ( b - a ).Cross( c - a ).GetLength();
        if ( area > whole * ( 1.0 + 1.0e-6 ) + tolerance )
            ++bad;
        if ( whole > 0.05 )
        {
            UInt32 inside = 0;
            const UInt32 samples = 4000;
            for ( UInt32 s=0; s<samples; ++s )
            {
                Float64 u = random.Uniform( 0.0, 1.0 );
                Float64 v = random.Uniform( 0.0, 1.0 );
                if ( u + v > 1.0 )
                {
                    u = 1.0 - u;
                    v = 1.0 - v;
                }
                if ( volume64.Contains( a + ( b - a ) * u + ( c - a ) * v ) )
                    ++inside;
            }
            const Float64 estimate = whole * (Float64) inside / (Float64) samples;
            if ( fabs( estimate - area ) > 0.05 * whole + 1.0e-3 )
                ++bad;
            if ( area > 0.1 * whole && area < 0.9 * whole )
                ++partial;
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PlaneBoundedVolume: clip sweep seed " << seed << ": " << partial << " triangles partly inside, " << bad << " failed checks" );
    return bad + ( partial > count / 20 ? 0 : 1 );
}

/*-------------------------------------------------------------------------*/

/** IntersectsExact against references by the plane sets and by Dykstra's projection, and the conservative tests that bound it */
template < typename T >
static UInt32
ExactOverlapSweep( UInt64 seed, UInt32 count, UInt32& overlapping, UInt32& separated )
{
    CRandom random( seed );
    UInt32 bad = 0;
    overlapping = 0;
    separated = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTPlaneBoundedVolume< T > volume;
        RandomShape( random, volume );
        const CTPlaneBoundedVolume< Float64 > volume64 = Widen( volume );
        CTSphere< T > sphere;
        CTAxisAlignedBox3D< T > box;
        CTOrientedBox3D< T > oriented;
        RandomShape( random, sphere );
        RandomShape( random, box );
        RandomShape( random, oriented );
        CTAxisAlignedBox3D< Float64 > volumeBounds;
        volume64.TryGetBounds( volumeBounds );
        const P3 inside = volumeBounds.GetCentre();

        /* the sphere: the distance from the centre to the volume against the radius */
        const CTSphere< Float64 > sphere64 = Widen( sphere );
        const P3 nearest = DykstraProjection( volume64.GetPlanes(), volume64.GetPlaneCount(), sphere64.centre, 3000 );
        const Float64 distance = ( nearest - sphere64.centre ).GetLength();
        const Float64 gap = distance - sphere64.radius;
        const TFrustumClassification coarse = volume.Classify( sphere );
        if ( fabs( gap ) > 1.0e-3 )
        {
            const bool expected = gap < 0;
            if ( volume.IntersectsExact( sphere ) != expected || IntersectsExact( volume, sphere ) != expected )
                ++bad;
            if ( expected && FRUSTUMCLASSIFICATION_OUTSIDE == coarse )
                ++bad;                                  /* the conservative test must not reject an overlap */
            if ( FRUSTUMCLASSIFICATION_INSIDE == coarse && ( !expected || !volume64.Contains( sphere64.centre ) ) )
                ++bad;
            if ( volume.Intersects( sphere ) != ( FRUSTUMCLASSIFICATION_OUTSIDE != coarse ) )
                ++bad;
            if ( expected )
                ++overlapping;
            else
                ++separated;
        }

        /* the boxes: whether the planes of the box and of the volume can be satisfied together */
        CTPlaneBoundedVolume< Float64 > boxVolume = CTPlaneBoundedVolume< Float64 >::FromAxisAlignedBox( Widen( box ) );
        const int reference = ReferencePlaneSetOverlap( volume64.GetPlanes(), volume64.GetPlaneCount(), boxVolume.GetPlanes(), boxVolume.GetPlaneCount(), inside );
        if ( 0 != reference )
        {
            const bool expected = reference > 0;
            if ( volume.IntersectsExact( box ) != expected || IntersectsExact( volume, box ) != expected )
                ++bad;
            if ( expected && FRUSTUMCLASSIFICATION_OUTSIDE == volume.Classify( box ) )
                ++bad;
            if ( FRUSTUMCLASSIFICATION_INSIDE == volume.Classify( box ) && !expected )
                ++bad;
            if ( expected )
                ++overlapping;
            else
                ++separated;
        }
        const CTPlaneBoundedVolume< Float64 > orientedVolume = CTPlaneBoundedVolume< Float64 >::FromOrientedBox( Widen( oriented ) );
        const int orientedReference = ReferencePlaneSetOverlap( volume64.GetPlanes(), volume64.GetPlaneCount(), orientedVolume.GetPlanes(), orientedVolume.GetPlaneCount(), inside );
        if ( 0 != orientedReference )
        {
            const bool expected = orientedReference > 0;
            if ( volume.IntersectsExact( oriented ) != expected )
                ++bad;
            if ( expected && FRUSTUMCLASSIFICATION_OUTSIDE == volume.Classify( oriented ) )
                ++bad;
        }
    }
    return bad;
}

/** Seeded random unbounded volumes: few planes, so that most are open: the exact tests must still agree with Dykstra's projection */
template < typename T >
static UInt32
UnboundedOverlapSweep( UInt64 seed, UInt32 count, UInt32& overlapping, UInt32& separated, UInt32& unbounded )
{
    CRandom random( seed );
    UInt32 bad = 0;
    overlapping = 0;
    separated = 0;
    unbounded = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTPlaneBoundedVolume< T > volume;
        const UInt32 planeCount = 1 + (UInt32) random.Index( 5 );
        for ( UInt32 k=0; k<planeCount; ++k )
        {
            const P3 through = random.Point( 1.2 );
            const D3 normal = random.Direction();
            volume.AddPlane( CTPlane< T >::FromPointNormal( ToP< T >( through ), ToD< T >( normal ) ) );
        }
        const CTPlaneBoundedVolume< Float64 > volume64 = Widen( volume );
        GUCEF::vector< CTPoint3D< T > > vertices;
        GUCEF::vector< SConvexBodyFace > faces;
        CTAxisAlignedBox3D< T > bounds;
        const bool closed = volume.BuildConvexBody( vertices, faces );
        if ( closed != volume.TryGetBounds( bounds ) || ( closed && ( vertices.size() < 4 || faces.size() < 4 ) ) )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PlaneBoundedVolume: body of the open volume " << i << " seed " << seed << ": closed " << closed );
        }
        if ( !closed )
            ++unbounded;

        CTSphere< T > sphere;
        CTAxisAlignedBox3D< T > box;
        CTOrientedBox3D< T > oriented;
        RandomShape( random, sphere );
        RandomShape( random, box );
        RandomShape( random, oriented );
        const CTSphere< Float64 > sphere64 = Widen( sphere );
        const P3 nearest = DykstraProjection( volume64.GetPlanes(), volume64.GetPlaneCount(), sphere64.centre, 3000 );
        const Float64 gap = ( nearest - sphere64.centre ).GetLength() - sphere64.radius;
        /* random planes can leave nothing: the projection is only a reference when it ended in the volume */
        const bool feasible = LargestViolation( volume64.GetPlanes(), volume64.GetPlaneCount(), nearest ) < 1.0e-7;
        if ( feasible && fabs( gap ) > 1.0e-3 )
        {
            if ( volume.IntersectsExact( sphere ) != ( gap < 0 ) )
            {
                ++bad;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PlaneBoundedVolume: open volume " << i << " seed " << seed << " sphere gap " << gap << " planes " << volume.GetPlaneCount() );
            }
            ++( gap < 0 ? overlapping : separated );
        }
        const CTAxisAlignedBox3D< Float64 > box64 = Widen( box );
        const CTPlaneBoundedVolume< Float64 > boxVolume = CTPlaneBoundedVolume< Float64 >::FromAxisAlignedBox( box64 );
        const int reference = ReferencePlaneSetOverlap( volume64.GetPlanes(), volume64.GetPlaneCount(), boxVolume.GetPlanes(), boxVolume.GetPlaneCount(), box64.GetCentre() );
        if ( 0 != reference )
        {
            if ( volume.IntersectsExact( box ) != ( reference > 0 ) )
            {
                ++bad;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PlaneBoundedVolume: open volume " << i << " seed " << seed << " box reference " << reference << " planes " << volume.GetPlaneCount() );
            }
            ++( reference > 0 ? overlapping : separated );
        }
        const CTPlaneBoundedVolume< Float64 > orientedVolume = CTPlaneBoundedVolume< Float64 >::FromOrientedBox( Widen( oriented ) );
        const int orientedReference = ReferencePlaneSetOverlap( volume64.GetPlanes(), volume64.GetPlaneCount(), orientedVolume.GetPlanes(), orientedVolume.GetPlaneCount(), Widen( oriented ).centre );
        if ( 0 != orientedReference && volume.IntersectsExact( oriented ) != ( orientedReference > 0 ) )
            ++bad;
    }
    return bad;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPlaneBoundedVolumeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "PlaneBoundedVolume" );

    GUCEF_TESTFW_TESTCASE( "PlaneBoundedVolume: a unit square clipped by the plane through its centre gives the expected half with exact vertices" )
    try
    {
        const CTPoint3D< Float64 > square[ 4 ] = { P( 0, 0, 0 ), P( 1, 0, 0 ), P( 1, 1, 0 ), P( 0, 1, 0 ) };
        CTPlaneBoundedVolume< Float64 > volume;
        ASSERT_TRUE( volume.AddPlane( CTPlane< Float64 >::FromPointNormal( P( 0.5, 0.5, 0 ), D( V( 1, 0, 0 ) ) ) ) );
        CTPoint3D< Float64 > out[ 8 ];
        CTPoint3D< Float64 > scratch[ 8 ];
        UInt32 count = 0;
        ASSERT_TRUE( volume.ClipPolygon( square, 4, out, scratch, 8, count ) );
        ASSERT_TRUE( 4 == count && out[ 0 ] == P( 0.5, 0, 0 ) && out[ 1 ] == P( 1, 0, 0 ) && out[ 2 ] == P( 1, 1, 0 ) && out[ 3 ] == P( 0.5, 1, 0 ) );

        /* the other half: the plane faces the other way */
        CTPlaneBoundedVolume< Float64 > other;
        other.AddPlane( CTPlane< Float64 >::FromPointNormal( P( 0.5, 0.5, 0 ), D( V( -1, 0, 0 ) ) ) );
        ASSERT_TRUE( other.ClipPolygon( square, 4, out, scratch, 8, count ) && 4 == count );
        ASSERT_TRUE( out[ 0 ] == P( 0, 0, 0 ) && out[ 1 ] == P( 0.5, 0, 0 ) && out[ 2 ] == P( 0.5, 1, 0 ) && out[ 3 ] == P( 0, 1, 0 ) );

        /* the vector form, the diagonal plane, a plane that keeps everything, a plane that keeps nothing, the polygon on the plane */
        GUCEF::vector< CTPoint3D< Float64 > > viaVector;
        volume.ClipPolygon( square, 4, viaVector );
        ASSERT_TRUE( 4 == viaVector.size() && viaVector[ 0 ] == P( 0.5, 0, 0 ) );
        CTPlaneBoundedVolume< Float64 > diagonal;
        diagonal.AddPlane( CTPlane< Float64 >::FromPointNormal( P( 0, 0, 0 ), D( V( 1, -1, 0 ) ) ) );
        ASSERT_TRUE( diagonal.ClipPolygon( square, 4, out, scratch, 8, count ) && 3 == count );
        CTPlaneBoundedVolume< Float64 > all;
        all.AddPlane( CTPlane< Float64 >::FromPointNormal( P( -1, 0, 0 ), D( V( 1, 0, 0 ) ) ) );
        ASSERT_TRUE( all.ClipPolygon( square, 4, out, scratch, 8, count ) && 4 == count && out[ 0 ] == square[ 0 ] && out[ 3 ] == square[ 3 ] );
        CTPlaneBoundedVolume< Float64 > none;
        none.AddPlane( CTPlane< Float64 >::FromPointNormal( P( 2, 0, 0 ), D( V( 1, 0, 0 ) ) ) );
        ASSERT_TRUE( none.ClipPolygon( square, 4, out, scratch, 8, count ) && 0 == count );
        CTPlaneBoundedVolume< Float64 > onPlane;
        onPlane.AddPlane( CTPlane< Float64 >::FromPointNormal( P( 0, 0, 0 ), D( V( 0, 0, 1 ) ) ) );
        ASSERT_TRUE( onPlane.ClipPolygon( square, 4, out, scratch, 8, count ) && 4 == count );
        /* the box volume clips the square to the part inside: here the whole square lies in the cube [ -1, 1 ] */
        ASSERT_TRUE( UnitCubeVolume().ClipPolygon( square, 4, out, scratch, 8, count ) && 4 == count );
        /* the capacity: too small for the result is reported, and an empty polygon stays empty */
        ASSERT_FALSE( volume.ClipPolygon( square, 4, out, scratch, 3, count ) || 0 != count );
        ASSERT_TRUE( volume.ClipPolygon( square, 0, out, scratch, 3, count ) && 0 == count );
        ASSERT_TRUE( CTPlaneBoundedVolume< Float64 >().ClipPolygon( square, 4, out, scratch, 4, count ) && 4 == count );

        /* Float32 gives the same exact vertices */
        const CTPoint3D< Float32 > square32[ 4 ] = { TFloat32Point3D( 0, 0, 0 ), TFloat32Point3D( 1, 0, 0 ), TFloat32Point3D( 1, 1, 0 ), TFloat32Point3D( 0, 1, 0 ) };
        CTPlaneBoundedVolume< Float32 > volume32;
        volume32.AddPlane( CTPlane< Float32 >::FromPointNormal( TFloat32Point3D( 0.5f, 0.5f, 0 ), TFloat32DirectionVector3D( TFloat32Vector3D( 1, 0, 0 ) ) ) );
        CTPoint3D< Float32 > out32[ 8 ];
        CTPoint3D< Float32 > scratch32[ 8 ];
        ASSERT_TRUE( volume32.ClipPolygon( square32, 4, out32, scratch32, 8, count ) && 4 == count );
        ASSERT_TRUE( out32[ 0 ] == TFloat32Point3D( 0.5f, 0, 0 ) && out32[ 1 ] == TFloat32Point3D( 1, 0, 0 ) && out32[ 2 ] == TFloat32Point3D( 1, 1, 0 ) && out32[ 3 ] == TFloat32Point3D( 0.5f, 1, 0 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PlaneBoundedVolume: BuildConvexBody of the six box planes gives the 8 vertices and 6 faces of the box, and of other classical polyhedra" )
    try
    {
        GUCEF::vector< CTPoint3D< Float64 > > vertices;
        GUCEF::vector< SConvexBodyFace > faces;
        ASSERT_TRUE( UnitCubeVolume().BuildConvexBody( vertices, faces ) );
        ASSERT_TRUE( 8 == vertices.size() && 6 == faces.size() );
        for ( UInt32 c=0; c<8; ++c )
        {
            const P expected( 0 != ( c & 1 ) ? 1.0 : -1.0, 0 != ( c & 2 ) ? 1.0 : -1.0, 0 != ( c & 4 ) ? 1.0 : -1.0 );
            bool found = false;
            for ( size_t v=0; v<vertices.size(); ++v )
                found = found || vertices[ v ] == expected;                                /* exact: the planes meet in exactly representable points */
            ASSERT_TRUE( found );
        }
        for ( size_t f=0; f<faces.size(); ++f )
            ASSERT_TRUE( 4 == faces[ f ].vertexIndices.size() );
        /* a seventh plane that cuts nothing, and a repeated plane, change nothing */
        CTPlaneBoundedVolume< Float64 > redundant = UnitCubeVolume();
        redundant.AddPlane( CTPlane< Float64 >::FromPointNormal( P( 5, 0, 0 ), D( V( -1, 0, 0 ) ) ) );
        redundant.AddPlane( CTPlane< Float64 >::FromPointNormal( P( 1, 0, 0 ), D( V( -1, 0, 0 ) ) ) );
        ASSERT_TRUE( redundant.BuildConvexBody( vertices, faces ) && 8 == vertices.size() && 6 == faces.size() );
        /* a plane that cuts a corner adds a face and three vertices and removes one: 10 vertices, 7 faces */
        CTPlaneBoundedVolume< Float64 > chamfered = UnitCubeVolume();
        chamfered.AddPlane( CTPlane< Float64 >::FromPointNormal( P( 0.5, 0.5, 0.5 ), D( V( -1, -1, -1 ) ) ) );
        ASSERT_TRUE( chamfered.BuildConvexBody( vertices, faces ) && 10 == vertices.size() && 7 == faces.size() );
        /* a regular tetrahedron and an octahedron */
        const Float64 s = 0.5773502691896258;
        CTPlaneBoundedVolume< Float64 > tetrahedron;
        const V normals[ 4 ] = { V( s, s, s ), V( s, -s, -s ), V( -s, s, -s ), V( -s, -s, s ) };
        for ( UInt32 k=0; k<4; ++k )
            tetrahedron.AddPlane( CTPlane< Float64 >::FromPointNormal( P( 0, 0, 0 ) + normals[ k ] * 1.0, D( normals[ k ] * -1.0 ) ) );
        ASSERT_TRUE( tetrahedron.BuildConvexBody( vertices, faces ) && 4 == vertices.size() && 4 == faces.size() );
        CTPlaneBoundedVolume< Float64 > octahedron;
        for ( UInt32 k=0; k<8; ++k )
        {
            const V n( 0 != ( k & 1 ) ? s : -s, 0 != ( k & 2 ) ? s : -s, 0 != ( k & 4 ) ? s : -s );
            octahedron.AddPlane( CTPlane< Float64 >::FromPointNormal( P( 0, 0, 0 ) + n * 1.0, D( n * -1.0 ) ) );
        }
        ASSERT_TRUE( octahedron.BuildConvexBody( vertices, faces ) && 6 == vertices.size() && 8 == faces.size() );
        /* fewer than four planes, the whole space, an empty region */
        CTPlaneBoundedVolume< Float64 > slab;
        slab.AddPlane( CTPlane< Float64 >::FromPointNormal( P( 0, 0, 0 ), D( V( 0, 1, 0 ) ) ) );
        slab.AddPlane( CTPlane< Float64 >::FromPointNormal( P( 0, 1, 0 ), D( V( 0, -1, 0 ) ) ) );
        ASSERT_FALSE( slab.BuildConvexBody( vertices, faces ) || !vertices.empty() || !faces.empty() );
        ASSERT_FALSE( CTPlaneBoundedVolume< Float64 >().BuildConvexBody( vertices, faces ) );
        CTAxisAlignedBox3D< Float64 > bounds;
        ASSERT_FALSE( slab.TryGetBounds( bounds ) );
        CTPlaneBoundedVolume< Float64 > emptyRegion = UnitCubeVolume();
        emptyRegion.AddPlane( CTPlane< Float64 >::FromPointNormal( P( 5, 0, 0 ), D( V( 1, 0, 0 ) ) ) );
        ASSERT_FALSE( emptyRegion.BuildConvexBody( vertices, faces ) );
        ASSERT_TRUE( UnitCubeVolume().TryGetBounds( bounds ) && bounds.Equals( CTAxisAlignedBox3D< Float64 >( P( -1, -1, -1 ), P( 1, 1, 1 ) ) ) );

        /* the frustum converts: its body has the eight corners */
        typedef CTMatrix4D< Float64 > M;
        const SProjectionConvention convention = SProjectionConvention::OpenGL();
        const CTFrustum< Float64 > frustum = CTFrustum< Float64 >::FromViewProjection( M::Perspective( CTRadian< Float64 >( 1.0 ), 1.5, 0.5, 20.0, convention ), convention );
        const CTPlaneBoundedVolume< Float64 > converted = CTPlaneBoundedVolume< Float64 >::FromFrustum( frustum );
        ASSERT_TRUE( 6 == converted.GetPlaneCount() && converted.BuildConvexBody( vertices, faces ) && 8 == vertices.size() && 6 == faces.size() );
        for ( UInt32 c=0; c<8; ++c )
        {
            bool found = false;
            for ( size_t v=0; v<vertices.size(); ++v )
                found = found || ( vertices[ v ] - frustum.GetCorner( c ) ).GetLength() < 1e-9;
            ASSERT_TRUE( found );
        }
        ASSERT_TRUE( converted.Contains( P( 0, 0, -5 ) ) && !converted.Contains( P( 0, 0, -25 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PlaneBoundedVolume: the convex body of 300 seeded volumes: corners on three planes, faces counter-clockwise, Euler's formula, volume by counting" )
    try
    {
        ASSERT_TRUE( 0 == ConvexBodySweep< Float32 >( 801, 300 ) );
        ASSERT_TRUE( 0 == ConvexBodySweep< Float64 >( 802, 300 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PlaneBoundedVolume: polygon clipping of 2000 seeded triangles against seeded volumes" )
    try
    {
        ASSERT_TRUE( 0 == ClipPolygonSweep< Float32 >( 811, 2000 ) );
        ASSERT_TRUE( 0 == ClipPolygonSweep< Float64 >( 812, 2000 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PlaneBoundedVolume: conservative and exact overlap tests of spheres and boxes against references by Dykstra's projection" )
    try
    {
        UInt32 overlapping = 0;
        UInt32 separated = 0;
        ASSERT_TRUE( 0 == ExactOverlapSweep< Float32 >( 821, 600, overlapping, separated ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PlaneBoundedVolume: Float32 exact overlap, " << overlapping << " overlapping and " << separated << " separated checked" );
        ASSERT_TRUE( overlapping > 200 && separated > 200 );
        ASSERT_TRUE( 0 == ExactOverlapSweep< Float64 >( 822, 600, overlapping, separated ) );
        ASSERT_TRUE( overlapping > 200 && separated > 200 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PlaneBoundedVolume: capacity, containment, construction from boxes, segment clipping and transforms" )
    try
    {
        CTPlaneBoundedVolume< Float64 > volume;
        ASSERT_TRUE( 0 == volume.GetPlaneCount() && volume.Contains( P( 1e9, 1e9, 1e9 ) ) );
        const CTPlane< Float64 > plane = CTPlane< Float64 >::FromPointNormal( P( 0, 0, 0 ), D( V( 1, 0, 0 ) ) );
        for ( UInt32 i=0; i<(UInt32) CTPlaneBoundedVolume< Float64 >::MaxPlanes; ++i )
            ASSERT_TRUE( volume.AddPlane( plane ) );
        ASSERT_TRUE( 32 == volume.GetPlaneCount() && !volume.AddPlane( plane ) && 32 == volume.GetPlaneCount() );
        CTPlane< Float64 > many[ 40 ];
        const CTPlaneBoundedVolume< Float64 > capped( many, 40 );
        ASSERT_TRUE( 32 == capped.GetPlaneCount() && 0 == CTPlaneBoundedVolume< Float64 >( GUCEF_NULL, 5 ).GetPlaneCount() );
        volume.Clear();
        ASSERT_TRUE( 0 == volume.GetPlaneCount() );

        /* the volumes of boxes contain what the boxes contain */
        CRandom random( 831 );
        for ( UInt32 i=0; i<300; ++i )
        {
            CTAxisAlignedBox3D< Float64 > box;
            CTOrientedBox3D< Float64 > oriented;
            RandomShape( random, box );
            RandomShape( random, oriented );
            const CTPlaneBoundedVolume< Float64 > fromBox = CTPlaneBoundedVolume< Float64 >::FromAxisAlignedBox( box );
            const CTPlaneBoundedVolume< Float64 > fromOriented = CTPlaneBoundedVolume< Float64 >::FromOrientedBox( oriented );
            ASSERT_TRUE( 6 == fromBox.GetPlaneCount() && 6 == fromOriented.GetPlaneCount() );
            for ( UInt32 k=0; k<40; ++k )
            {
                const P3 p = random.Point( 3.0 );
                if ( fabs( ReferenceMarginBox( box, p ) ) > 1e-9 )
                    ASSERT_TRUE( fromBox.Contains( p ) == box.Contains( p ) );
                if ( fabs( ReferenceMarginOriented( oriented, p ) ) > 1e-9 )
                    ASSERT_TRUE( fromOriented.Contains( p ) == oriented.Contains( p ) );
            }
        }
        ASSERT_FALSE( CTPlaneBoundedVolume< Float64 >::FromAxisAlignedBox( CTAxisAlignedBox3D< Float64 >() ).Contains( P( 0, 0, 0 ) ) );

        /* segments: the clipped interval has its ends on the boundary and inside the volume */
        const CTPlaneBoundedVolume< Float64 > cube = UnitCubeVolume();
        Float64 enter = 0;
        Float64 leave = 0;
        ASSERT_TRUE( cube.ClipSegment( CTSegment3D< Float64 >( P( -3, 0, 0 ), P( 3, 0, 0 ) ), enter, leave ) && NearTo( enter, 1.0 / 3.0, 1e-12 ) && NearTo( leave, 2.0 / 3.0, 1e-12 ) );
        ASSERT_TRUE( cube.ClipSegment( CTSegment3D< Float64 >( P( 0, 0, 0 ), P( 0.5, 0, 0 ) ), enter, leave ) && 0.0 == enter && 1.0 == leave );
        ASSERT_FALSE( cube.ClipSegment( CTSegment3D< Float64 >( P( 2, 0, 0 ), P( 3, 0, 0 ) ), enter, leave ) );
        ASSERT_TRUE( cube.ClipSegment( CTSegment3D< Float64 >( P( 0, 0, 0 ), P( 0, 0, 0 ) ), enter, leave ) && !cube.ClipSegment( CTSegment3D< Float64 >( P( 4, 0, 0 ), P( 4, 0, 0 ) ), enter, leave ) );
        ASSERT_TRUE( cube.ClipSegment( CTSegment3D< Float64 >( P( -3, 1, 0 ), P( 3, 1, 0 ) ), enter, leave ) );            /* along a boundary plane */

        /* transforms: a similarity moves the region, a singular matrix has no image */
        typedef CTMatrix4D< Float64 > M;
        const M similarity = M::FromTRS( V( 1, 2, 3 ), CTQuaternion< Float64 >::FromAxisAngle( D( V( 1, 1, 0 ) ), CTRadian< Float64 >( 0.8 ) ), V( 2, 2, 2 ) );
        CTPlaneBoundedVolume< Float64 > moved;
        ASSERT_TRUE( cube.TryTransformed( similarity, moved ) && moved.GetPlaneCount() == 6 );
        for ( UInt32 k=0; k<500; ++k )
        {
            const P3 p = random.Point( 2.0 );
            if ( fabs( ReferenceMarginBox( CTAxisAlignedBox3D< Float64 >( P( -1, -1, -1 ), P( 1, 1, 1 ) ), p ) ) > 1e-9 )
                ASSERT_TRUE( cube.Contains( p ) == moved.Contains( similarity.TransformPoint( p ) ) );
        }
        CTPlaneBoundedVolume< Float64 > untouched = cube;
        ASSERT_FALSE( cube.TryTransformed( M::Scale( V( 1, 0, 1 ) ), untouched ) );
        ASSERT_TRUE( untouched == cube );

        /* text and DataNode */
        CTPlaneBoundedVolume< Float64 > randomVolume;
        RandomShape( random, randomVolume );
        CTPlaneBoundedVolume< Float64 > fromText;
        ASSERT_TRUE( fromText.FromString( randomVolume.ToString() ) && fromText == randomVolume );
        CORE::CDataNodeSerializableSettings settings;
        CORE::CDataNode node;
        ASSERT_TRUE( randomVolume.Serialize( node, settings ) );
        CTPlaneBoundedVolume< Float64 > fromNode;
        ASSERT_TRUE( fromNode.Deserialize( node, settings ) && fromNode == randomVolume );
        CTPlaneBoundedVolume< Float64 > unchanged = randomVolume;
        ASSERT_FALSE( unchanged.FromString( "PlaneBoundedVolume(Plane(normal (0, 1, 0), offset 1)" ) || unchanged.FromString( "Frustum()" ) );
        CORE::CDataNode empty;
        ASSERT_FALSE( unchanged.Deserialize( empty, settings ) );
        ASSERT_TRUE( unchanged == randomVolume );
        CTPlaneBoundedVolume< Float64 > none;
        ASSERT_TRUE( none.FromString( "PlaneBoundedVolume()" ) && 0 == none.GetPlaneCount() && CORE::CString( "PlaneBoundedVolume()" ) == none.ToString() );
        ASSERT_TRUE( randomVolume.Equals( randomVolume ) && !randomVolume.Equals( none ) );
        GUCEF::vector< CTSegment3D< Float64 > > lines;
        cube.GetWireframeLines( lines, 8 );
        ASSERT_TRUE( 12 == lines.size() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    GUCEF_TESTFW_TESTCASE( "PlaneBoundedVolume: unbounded volumes have no convex body and no bounds, and the exact overlap tests still decide them" )
    try
    {
        /* the open-topped unit box: five planes, z >= 0 side open upwards */
        const CTPlane< Float64 > open[ 5 ] = { CTPlane< Float64 >( D( V( 1, 0, 0 ) ), 0 ), CTPlane< Float64 >( D( V( -1, 0, 0 ) ), -1 ), CTPlane< Float64 >( D( V( 0, 1, 0 ) ), 0 ),
                                               CTPlane< Float64 >( D( V( 0, -1, 0 ) ), -1 ), CTPlane< Float64 >( D( V( 0, 0, 1 ) ), 0 ) };
        const CTPlaneBoundedVolume< Float64 > chimney( open, 5 );
        GUCEF::vector< CTPoint3D< Float64 > > vertices;
        GUCEF::vector< SConvexBodyFace > faces;
        ASSERT_FALSE( chimney.BuildConvexBody( vertices, faces ) );
        ASSERT_TRUE( vertices.empty() && faces.empty() );
        CTAxisAlignedBox3D< Float64 > bounds;
        ASSERT_FALSE( chimney.TryGetBounds( bounds ) );
        GUCEF::vector< CTSegment3D< Float64 > > lines;
        chimney.GetWireframeLines( lines, 8 );
        ASSERT_TRUE( lines.empty() );
        /* a sphere far up the chimney and a box in it overlap, ones outside the walls or below the floor do not */
        ASSERT_TRUE( chimney.IntersectsExact( CTSphere< Float64 >( P( 1.5, 0.5, 100 ), 0.6 ) ) );
        ASSERT_FALSE( chimney.IntersectsExact( CTSphere< Float64 >( P( 1.7, 0.5, 100 ), 0.6 ) ) );
        ASSERT_FALSE( chimney.IntersectsExact( CTSphere< Float64 >( P( 0.5, 0.5, -0.7 ), 0.6 ) ) );
        ASSERT_TRUE( chimney.IntersectsExact( CTSphere< Float64 >( P( 0.5, 0.5, -0.5 ), 0.6 ) ) );
        ASSERT_TRUE( chimney.IntersectsExact( CTAxisAlignedBox3D< Float64 >( P( 0.9, 0, 99 ), P( 2, 1, 101 ) ) ) );
        ASSERT_FALSE( chimney.IntersectsExact( CTAxisAlignedBox3D< Float64 >( P( 1.1, 0, 99 ), P( 2, 1, 101 ) ) ) );
        ASSERT_TRUE( chimney.IntersectsExact( CTOrientedBox3D< Float64 >::FromAxisAlignedBox( CTAxisAlignedBox3D< Float64 >( P( 0.9, 0, 99 ), P( 2, 1, 101 ) ) ) ) );
        ASSERT_FALSE( chimney.IntersectsExact( CTOrientedBox3D< Float64 >::FromAxisAlignedBox( CTAxisAlignedBox3D< Float64 >( P( 1.1, 0, 99 ), P( 2, 1, 101 ) ) ) ) );
        ASSERT_TRUE( IntersectsExact( chimney, CTSphere< Float64 >( P( 1.5, 0.5, 100 ), 0.6 ) ) );
        /* the same in Float32 */
        CTPlane< Float32 > open32[ 5 ];
        for ( UInt32 i=0; i<5; ++i )
            open32[ i ] = CTPlane< Float32 >( TFloat32DirectionVector3D( TFloat32Vector3D( (Float32) open[ i ].normal.GetX(), (Float32) open[ i ].normal.GetY(), (Float32) open[ i ].normal.GetZ() ) ), (Float32) open[ i ].offset );
        const CTPlaneBoundedVolume< Float32 > chimney32( open32, 5 );
        ASSERT_TRUE( chimney32.IntersectsExact( CTSphere< Float32 >( TFloat32Point3D( 1.5f, 0.5f, 100.0f ), 0.6f ) ) && !chimney32.IntersectsExact( CTSphere< Float32 >( TFloat32Point3D( 1.7f, 0.5f, 100.0f ), 0.6f ) ) );
        /* the whole space, one plane, a slab: unbounded too */
        const CTPlaneBoundedVolume< Float64 > whole;
        ASSERT_TRUE( whole.IntersectsExact( CTSphere< Float64 >( P( 5, 5, 5 ), 1.0 ) ) && !whole.BuildConvexBody( vertices, faces ) );
        CTPlaneBoundedVolume< Float64 > halfSpace;
        halfSpace.AddPlane( CTPlane< Float64 >( D( V( 0, 0, 1 ) ), 0 ) );
        ASSERT_TRUE( halfSpace.IntersectsExact( CTSphere< Float64 >( P( 0, 0, -0.5 ), 0.6 ) ) && !halfSpace.IntersectsExact( CTSphere< Float64 >( P( 0, 0, -0.7 ), 0.6 ) ) );

        UInt32 overlapping = 0;
        UInt32 separated = 0;
        UInt32 unbounded = 0;
        ASSERT_TRUE( 0 == UnboundedOverlapSweep< Float32 >( 841, 800, overlapping, separated, unbounded ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PlaneBoundedVolume: Float32 unbounded sweep, " << unbounded << " open volumes, " << overlapping << " overlapping and " << separated << " separated checked" );
        ASSERT_TRUE( unbounded > 700 && overlapping > 500 && separated > 500 );
        ASSERT_TRUE( 0 == UnboundedOverlapSweep< Float64 >( 842, 800, overlapping, separated, unbounded ) );
        ASSERT_TRUE( unbounded > 700 && overlapping > 500 && separated > 500 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PlaneBoundedVolume: values that are not numbers are inside nothing and hit nothing, in every plane set query" )
    try
    {
        typedef CTPlaneBoundedVolume< Float64 > Volume;
        const Float64 nan = std::numeric_limits< Float64 >::quiet_NaN();
        ASSERT_TRUE( nan != nan );
        const P notANumber( nan, 0, 0 );
        const Volume cube = UnitCubeVolume();
        const Volume empty;
        const CTFrustum< Float64 > clip;
        ASSERT_FALSE( cube.Contains( notANumber ) || empty.Contains( notANumber ) || clip.Contains( notANumber ) );
        ASSERT_FALSE( PlaneSetContains( cube.GetPlanes(), 6, notANumber ) );
        ASSERT_TRUE( FRUSTUMCLASSIFICATION_OUTSIDE == cube.Classify( CTSphere< Float64 >( notANumber, 1.0 ) ) && FRUSTUMCLASSIFICATION_OUTSIDE == cube.Classify( CTSphere< Float64 >( P( 0, 0, 0 ), nan ) ) );
        ASSERT_TRUE( FRUSTUMCLASSIFICATION_OUTSIDE == cube.Classify( CTAxisAlignedBox3D< Float64 >( notANumber, P( 1, 1, 1 ) ) ) && FRUSTUMCLASSIFICATION_OUTSIDE == clip.Classify( CTAxisAlignedBox3D< Float64 >( P( 0, 0, 0 ), P( nan, 1, 1 ) ) ) );
        ASSERT_TRUE( FRUSTUMCLASSIFICATION_OUTSIDE == clip.Classify( CTOrientedBox3D< Float64 >( notANumber, CTQuaternion< Float64 >::Identity(), V( 1, 1, 1 ) ) ) && FRUSTUMCLASSIFICATION_OUTSIDE == clip.Classify( CTOrientedBox3D< Float64 >( P( 0, 0, 0 ), CTQuaternion< Float64 >::Identity(), V( 1, nan, 1 ) ) ) );
        ASSERT_TRUE( FRUSTUMCLASSIFICATION_OUTSIDE == empty.Classify( CTSphere< Float64 >( notANumber, 1.0 ) ) );
        ASSERT_FALSE( cube.IntersectsExact( CTSphere< Float64 >( notANumber, 1.0 ) ) || cube.IntersectsExact( CTAxisAlignedBox3D< Float64 >( notANumber, P( 1, 1, 1 ) ) ) );
        ASSERT_FALSE( Intersects( cube, CTCapsule< Float64 >( notANumber, P( 0, 0, 0 ), 1.0 ) ) || Intersects( clip, CTCylinder< Float64 >( P( 0, 0, 0 ), P( 0, 1, 0 ), nan ) ) );
        ASSERT_FALSE( Intersects( cube, CTSegment3D< Float64 >( notANumber, P( 0, 0, 0 ) ) ) || Intersects( empty, CTSegment3D< Float64 >( P( 0, 0, 0 ), notANumber ) ) || Intersects( clip, CTSegment3D< Float64 >( notANumber, notANumber ) ) );
        ASSERT_FALSE( Intersects( cube, CTTriangle3D< Float64 >( P( 0, 0, 0 ), notANumber, P( 0, 1, 0 ) ) ) || Intersects( empty, CTTriangle3D< Float64 >( P( 0, 0, 0 ), P( 0, 1, 0 ), notANumber ) ) );
        Float64 enter = 0;
        Float64 leave = 0;
        ASSERT_FALSE( cube.ClipSegment( CTSegment3D< Float64 >( notANumber, P( 0, 0, 0 ) ), enter, leave ) || empty.ClipSegment( CTSegment3D< Float64 >( P( 0, 0, 0 ), notANumber ), enter, leave ) );
        CTPoint3D< Float64 > poly[ 3 ] = { P( 0, 0, 0 ), notANumber, P( 0, 1, 0 ) };
        CTPoint3D< Float64 > out[ 16 ];
        CTPoint3D< Float64 > scratch[ 16 ];
        UInt32 count = 7;
        ASSERT_FALSE( cube.ClipPolygon( poly, 3, out, scratch, 16, count ) );
        ASSERT_TRUE( 0 == count );
        /* a plane that is not a number: the volume contains nothing and is hit by nothing */
        const CTPlane< Float64 > bad( D( V( 0, 0, 1 ) ), nan );
        const Volume broken( &bad, 1 );
        SRayCastResult< Float64 > hit;
        ASSERT_FALSE( broken.Contains( P( 0, 0, 0 ) ) || broken.RayCast( CTRay3D< Float64 >( P( 0, 0, -3 ), D( V( 0, 0, 1 ) ) ), 10.0, hit ) );
        ASSERT_FALSE( broken.IntersectsExact( CTSphere< Float64 >( P( 0, 0, 0 ), 1.0 ) ) );
        GUCEF::vector< CTPoint3D< Float64 > > vertices;
        GUCEF::vector< SConvexBodyFace > faces;
        CTPlane< Float64 > withNaN[ 4 ] = { CTPlane< Float64 >( D( V( 1, 0, 0 ) ), 0 ), CTPlane< Float64 >( D( V( 0, 1, 0 ) ), 0 ), CTPlane< Float64 >( D( V( 0, 0, 1 ) ), 0 ), CTPlane< Float64 >( D( V( -1, -1, -1 ) ), nan ) };
        ASSERT_FALSE( Volume( withNaN, 4 ).BuildConvexBody( vertices, faces ) );
        /* the tolerant forms agree */
        const CTGeometryTolerance< Float64 > tolerance( 1.0e-3, 1.0e-6, 1.0e-6 );
        ASSERT_FALSE( cube.Contains( notANumber, tolerance ) || FRUSTUMCLASSIFICATION_OUTSIDE != cube.Classify( CTSphere< Float64 >( notANumber, 1.0 ), tolerance ) );
        ASSERT_FALSE( Intersects( cube, CTSphere< Float64 >( notANumber, 1.0 ), tolerance ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PlaneBoundedVolume: the tolerant plane set and box queries count shapes within the tolerance as touching, the exact ones do not" )
    try
    {
        const CTPlaneBoundedVolume< Float64 > cube = UnitCubeVolume();
        const CTGeometryTolerance< Float64 > tolerance( 1.0e-3, 0.0, 1.0e-9 );
        ASSERT_FALSE( cube.Contains( P( 1.0005, 0, 0 ) ) );
        ASSERT_TRUE( cube.Contains( P( 1.0005, 0, 0 ), tolerance ) && !cube.Contains( P( 1.01, 0, 0 ), tolerance ) );
        ASSERT_TRUE( PlaneSetContains( cube.GetPlanes(), 6, P( 1.0005, 0, 0 ), tolerance ) && !PlaneSetContains( cube.GetPlanes(), 6, P( 1.0005, 0, 0 ) ) );
        const CTAxisAlignedBox3D< Float64 > touching( P( 1.0005, -0.5, -0.5 ), P( 2, 0.5, 0.5 ) );
        ASSERT_TRUE( FRUSTUMCLASSIFICATION_OUTSIDE == cube.Classify( touching ) && FRUSTUMCLASSIFICATION_INTERSECTING == cube.Classify( touching, tolerance ) );
        ASSERT_FALSE( cube.Intersects( touching ) );
        ASSERT_TRUE( Intersects( cube, touching, tolerance ) && Intersects( touching, cube, tolerance ) );
        ASSERT_TRUE( FRUSTUMCLASSIFICATION_OUTSIDE == cube.Classify( CTSphere< Float64 >( P( 1.5005, 0, 0 ), 0.5 ) ) && FRUSTUMCLASSIFICATION_INTERSECTING == cube.Classify( CTSphere< Float64 >( P( 1.5005, 0, 0 ), 0.5 ), tolerance ) );
        ASSERT_TRUE( FRUSTUMCLASSIFICATION_OUTSIDE == cube.Classify( CTOrientedBox3D< Float64 >( P( 1.5005, 0, 0 ), CTQuaternion< Float64 >::Identity(), V( 0.5, 0.5, 0.5 ) ) ) &&
                     FRUSTUMCLASSIFICATION_INTERSECTING == cube.Classify( CTOrientedBox3D< Float64 >( P( 1.5005, 0, 0 ), CTQuaternion< Float64 >::Identity(), V( 0.5, 0.5, 0.5 ) ), tolerance ) );
        /* a shape that is inside by less than the tolerance counts as inside */
        ASSERT_TRUE( FRUSTUMCLASSIFICATION_INTERSECTING == cube.Classify( CTSphere< Float64 >( P( 0, 0, 0 ), 1.0005 ) ) && FRUSTUMCLASSIFICATION_INSIDE == cube.Classify( CTSphere< Float64 >( P( 0, 0, 0 ), 1.0005 ), tolerance ) );
        ASSERT_TRUE( FRUSTUMCLASSIFICATION_INSIDE == cube.Classify( CTSphere< Float64 >( P( 0, 0, 0 ), 1.0 - 1.0e-4 ) ) );
        /* the tolerant convex shapes of the volume queries */
        ASSERT_FALSE( Intersects( cube, CTCapsule< Float64 >( P( 1.0005, 0, 0 ), P( 2, 0, 0 ), 0.0 ) ) );
        ASSERT_TRUE( Intersects( cube, CTCapsule< Float64 >( P( 1.0005, 0, 0 ), P( 2, 0, 0 ), 0.0 ), tolerance ) );

        /* the oriented box tests: a gap of 5e-4 apart */
        typedef CTOrientedBox3D< Float64 > Oriented;
        const Oriented unit( P( 0, 0, 0 ), CTQuaternion< Float64 >::Identity(), V( 1, 1, 1 ) );
        const Oriented gapped( P( 2.0005, 0, 0 ), CTQuaternion< Float64 >::Identity(), V( 1, 1, 1 ) );
        ASSERT_FALSE( Intersects( unit, gapped ) );
        ASSERT_TRUE( Intersects( unit, gapped, tolerance ) && Intersects( gapped, unit, tolerance ) );
        ASSERT_FALSE( Intersects( CTAxisAlignedBox3D< Float64 >( P( -1, -1, -1 ), P( 1, 1, 1 ) ), gapped ) );
        ASSERT_TRUE( Intersects( CTAxisAlignedBox3D< Float64 >( P( -1, -1, -1 ), P( 1, 1, 1 ) ), gapped, tolerance ) );
        ASSERT_FALSE( Intersects( unit, Oriented( P( 2.01, 0, 0 ), CTQuaternion< Float64 >::Identity(), V( 1, 1, 1 ) ), tolerance ) );
        /* the exact forms keep their behaviour: touching counts */
        ASSERT_TRUE( Intersects( unit, Oriented( P( 2, 0, 0 ), CTQuaternion< Float64 >::Identity(), V( 1, 1, 1 ) ) ) );
        /* the same in Float32 */
        const CTPlaneBoundedVolume< Float32 > cube32 = CTPlaneBoundedVolume< Float32 >::FromAxisAlignedBox( CTAxisAlignedBox3D< Float32 >( TFloat32Point3D( -1, -1, -1 ), TFloat32Point3D( 1, 1, 1 ) ) );
        const CTGeometryTolerance< Float32 > tolerance32( 1.0e-3f, 0.0f, 1.0e-6f );
        ASSERT_TRUE( !cube32.Contains( TFloat32Point3D( 1.0005f, 0, 0 ) ) && cube32.Contains( TFloat32Point3D( 1.0005f, 0, 0 ), tolerance32 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
