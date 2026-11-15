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

#include "TestGeoSpaceReference.h"
#include "TestGeoPlane.h"

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
using namespace ConvexReference;
using namespace GeoSpaceReference;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
static bool
Near3( const CTPoint3D< T >& p, Float64 x, Float64 y, Float64 z, Float64 tolerance )
{
    return fabs( (Float64) p.x - x ) <= tolerance && fabs( (Float64) p.y - y ) <= tolerance && fabs( (Float64) p.z - z ) <= tolerance;
}

/*-------------------------------------------------------------------------*/

/**
 *  Acceptance item 7: a plane at ( 0, 1, 0 ) rotated 90 degrees about +X hosts a circle of radius 1 at 2D ( 2, 0 ).
 *  The rotation maps local +X to +X, local +Y to +Z and the normal +Z to -Y, so the plane is the world plane y = 1 with
 *  its front towards -Y, and 2D ( u, v ) is world ( u, 1, v ).
 */
template < typename T >
static bool
CircleOnRotatedPlane( Float64 tolerance )
{
    typedef CTPoint2D< T > TP2;
    CTGeoPlane< T > plane( CTTransform3D< T >( CTVector3D< T >( T( 0 ), T( 1 ), T( 0 ) ), CTQuaternion< T >::FromAxisAngle( CTDirectionVector3D< T >::UnitX(), Degrees( T( 90 ) ) ),
                                               CTVector3D< T >::One() ) );
    /* the circle placed two ways: by its shape centre, and by its 2D transform */
    CTGeoObject2D< CTCircle< T > > byShape( CTCircle< T >( TP2( T( 2 ), T( 0 ) ), T( 1 ) ) );
    CTGeoObject2D< CTCircle< T > > byTransform( CTCircle< T >( TP2(), T( 1 ) ) );
    byShape.SetParent( &plane, false );
    byTransform.SetParent( &plane, false );
    byTransform.SetLocalTransform2D( CTTransform2D< T >( CTVector2D< T >( T( 2 ), T( 0 ) ), CTRadian< T >( T( 0.3 ) ), CTVector2D< T >::One() ) );
    bool ok = Near3( byShape.ToWorld( byShape.GetShape().centre ), 2.0, 1.0, 0.0, tolerance ) && Near3( byTransform.GetWorldPosition(), 2.0, 1.0, 0.0, tolerance );
    ok = ok && fabs( (Float64) byTransform.GetLocalTransform2D().rotation.GetValue() - 0.3 ) <= tolerance && fabs( (Float64) byTransform.GetLocalTransform2D().position.x - 2.0 ) <= tolerance;
    const CTPlane< T > world = plane.GetPlane();
    ok = ok && fabs( (Float64) world.normal.GetY() + 1.0 ) <= tolerance && fabs( (Float64) world.GetSignedDistance( CTPoint3D< T >( T( 5 ), T( 1 ), T( -3 ) ) ) ) <= tolerance;

    /* a ray from below the plane along +Y hits the circle at ( 2.5, 1, 0.3 ), 6 away */
    CTGeoObject2D< CTCircle< T > >* circles[ 2 ] = { &byShape, &byTransform };
    for ( UInt32 i=0; i<2; ++i )
    {
        SRayCastResult< T > hit;
        const CTRay3D< T > ray( CTPoint3D< T >( T( 2.5 ), T( -5 ), T( 0.3 ) ), CTDirectionVector3D< T >::UnitY() );
        ok = ok && circles[ i ]->RayCast( ray, T( 100 ), hit ) && fabs( (Float64) hit.distance - 6.0 ) <= tolerance && Near3( hit.point, 2.5, 1.0, 0.3, tolerance );
        /* the normal faces the ray: -Y */
        ok = ok && fabs( (Float64) hit.normal.y + 1.0 ) <= tolerance;
        /* a ray that meets the plane outside the circle misses; a short ray stops before the plane */
        ok = ok && !circles[ i ]->RayCast( CTRay3D< T >( CTPoint3D< T >( T( 3.5 ), T( -5 ), T( 0 ) ), CTDirectionVector3D< T >::UnitY() ), T( 100 ), hit );
        ok = ok && !circles[ i ]->RayCast( ray, T( 5 ), hit );

        /* the distance from a point at height 3 above the rim is 3: the rim point ( 3, 0 ) is world ( 3, 1, 0 ), height along -Y */
        const CTPoint3D< T > aboveRim( T( 3 ), T( -2 ), T( 0 ) );
        ok = ok && fabs( (Float64) Distance( aboveRim, *circles[ i ] ) - 3.0 ) <= tolerance && Near3( circles[ i ]->ClosestPoint( aboveRim ), 3.0, 1.0, 0.0, tolerance );
        /* beyond the rim: sqrt( h^2 + d2D^2 ) */
        ok = ok && fabs( (Float64) Distance( CTPoint3D< T >( T( 5 ), T( -2 ), T( 0 ) ), *circles[ i ] ) - sqrt( 9.0 + 4.0 ) ) <= tolerance;
        /* a point in the disk is contained, one off the plane is not */
        ok = ok && circles[ i ]->Contains( CTPoint3D< T >( T( 2.2 ), T( 1 ), T( 0.4 ) ) ) && !circles[ i ]->Contains( CTPoint3D< T >( T( 2.2 ), T( 1.1 ), T( 0.4 ) ) );
    }

    /* a sphere above the circle: the flat support mapping lets GJK answer the pair */
    CTGeoObject< CTSphere< T > > sphere( CTSphere< T >( CTPoint3D< T >( T( 2 ), T( -2 ), T( 0 ) ), T( 0.5 ) ) );
    /* GJK on the flat support: its distance tolerance, at least 1e-9 */
    const Float64 gjkTolerance = tolerance * 10.0 > 1.0e-9 ? tolerance * 10.0 : 1.0e-9;
    const Float64 gjkDistance = (Float64) Distance( sphere, byShape );
    if ( fabs( gjkDistance - 2.5 ) > gjkTolerance )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoPlane: sphere above the circle at " << gjkDistance << " instead of 2.5" );
    ok = ok && fabs( gjkDistance - 2.5 ) <= gjkTolerance && !Intersects( sphere, byShape );
    sphere.SetLocalPosition( CTVector3D< T >( T( 0 ), T( 2.6 ), T( 0 ) ) );
    ok = ok && Intersects( sphere, byShape );
    return ok;
}

/*-------------------------------------------------------------------------*/

/**
 *  The reference of the test: even-odd ray crossing of a ring, in Float64, written out here independently of the
 *  polygon classes (a horizontal ray to +x; an edge counts when it straddles the ray's line, half-open in y)
 */
static bool
CrossingNumberInside( const Float64* xs, const Float64* ys, UInt32 count, Float64 x, Float64 y )
{
    bool inside = false;
    for ( UInt32 i=0, j=count-1; i<count; j=i++ )
    {
        if ( ( ys[ i ] > y ) != ( ys[ j ] > y ) )
        {
            const Float64 crossingX = xs[ j ] + ( y - ys[ j ] ) * ( xs[ i ] - xs[ j ] ) / ( ys[ i ] - ys[ j ] );
            if ( x < crossingX )
                inside = !inside;
        }
    }
    return inside;
}

/*-------------------------------------------------------------------------*/

/** Acceptance item 7: point in polygon through the plane for a concave polygon (an L shape), by containment and by ray */
template < typename T >
static bool
ConcavePolygonThroughPlane( UInt64 seed, UInt32 count )
{
    typedef CTPoint2D< T > TP2;
    CRandom r( seed );
    CTGeoPlane< T > plane( RandomTransform< T >( r, 5.0, false, 0.5, 2.0, 0.0 ) );
    const TP2 ring[ 6 ] = { TP2( T( 0 ), T( 0 ) ), TP2( T( 4 ), T( 0 ) ), TP2( T( 4 ), T( 1 ) ), TP2( T( 1 ), T( 1 ) ), TP2( T( 1 ), T( 3 ) ), TP2( T( 0 ), T( 3 ) ) };
    const Float64 ringX[ 6 ] = { 0.0, 4.0, 4.0, 1.0, 1.0, 0.0 };
    const Float64 ringY[ 6 ] = { 0.0, 0.0, 1.0, 1.0, 3.0, 3.0 };
    const CTPolygon2D< T > polygon( ring, 6 );
    CTGeoObject2D< CTPolygon2D< T > > object( polygon );
    object.SetParent( &plane, false );
    /* the reference itself: inside the arm, inside the base, outside in the notch and beyond the right end */
    bool ok = !object.IsConvex() && CrossingNumberInside( ringX, ringY, 6, 0.5, 2.0 ) && CrossingNumberInside( ringX, ringY, 6, 3.5, 0.5 ) &&
              !CrossingNumberInside( ringX, ringY, 6, 2.0, 2.0 ) && !CrossingNumberInside( ringX, ringY, 6, 4.5, 0.5 );
    UInt32 bad = 0;
    UInt32 inside = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const Float64 u = r.Uniform( -0.5, 4.5 );
        const Float64 v = r.Uniform( -0.5, 3.5 );
        const TP2 p( (T) u, (T) v );
        const bool expected = CrossingNumberInside( ringX, ringY, 6, (Float64) p.x, (Float64) p.y );
        inside += expected ? 1 : 0;
        const CTPoint3D< T > world = plane.FromPlane( p );
        const bool contained = object.Contains( world );
        /* a ray through the point from the front side hits exactly when the point is inside */
        const CTDirectionVector3D< T > normal = plane.GetPlane().normal;
        const CTRay3D< T > ray( world + normal.AsVector() * T( 3 ), -normal );
        SRayCastResult< T > hit;
        const bool hitIt = object.RayCast( ray, T( 100 ), hit );
        /* the notch: points with 1 < x < 4 and 1 < y < 3 are outside */
        if ( contained != expected || hitIt != expected || ( hitIt && fabs( (Float64) hit.distance - 3.0 ) > 1.0e-3 ) )
            ++bad;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GeoPlane: concave polygon through a plane, seed " << seed << ": " << count << " points, " << inside << " inside, " << bad << " bad" );
    return ok && 0 == bad && inside > count / 4 && inside < count;
}

/*-------------------------------------------------------------------------*/

/**
 *  Acceptance item 7: ProjectToPlane and FromPlane round trip; the projection is along the world normal (exact also
 *  under non-uniform scale and shear of the plane's chain); RayCastToPlane meets the plane where the ray does
 */
template < typename T >
static bool
ProjectionRoundTrip( UInt64 seed, UInt32 count, Float64 tolerance )
{
    CRandom r( seed );
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTGeoSpace< T > parent( RandomTransform< T >( r, 5.0, false, 0.5, 2.0, 0.2 ) );
        CTGeoPlane< T > plane( RandomTransform< T >( r, 5.0, false, 0.5, 2.0, 0.2 ) );
        plane.SetParent( &parent, false );
        const P3 p = r.Point( 10.0 );
        CTPoint2D< T > planePoint;
        T height = T( 0 );
        plane.ProjectToPlane( ToP< T >( p ), planePoint, height );
        const CTPoint3D< T > foot = plane.FromPlane( planePoint );
        const CTPlane< T > world = plane.GetPlane();
        const V3 rebuilt = Widen( foot ) + WidenD( world.normal ).AsVector() * (Float64) height - p;
        const Float64 scale = 1.0 + p.ToPositionVector().GetLength();
        bool ok = rebuilt.GetLength() <= tolerance * scale * 10.0 && fabs( (Float64) world.GetSignedDistance( foot ) ) <= tolerance * scale * 10.0;
        /* the foot lies on the plane, and the projection of a plane point is itself */
        CTPoint2D< T > again;
        T zero = T( 1 );
        plane.ProjectToPlane( foot, again, zero );
        ok = ok && fabs( (Float64) zero ) <= tolerance * scale * 10.0 && fabs( (Float64) ( again.x - planePoint.x ) ) <= tolerance * scale * 10.0 && fabs( (Float64) ( again.y - planePoint.y ) ) <= tolerance * scale * 10.0;
        /* a ray towards the plane meets it at the foot when aimed along the normal */
        if ( fabs( (Float64) height ) > 0.1 )
        {
            const CTDirectionVector3D< T > towards = CTNumericTraits< T >::IsNegative( height ) ? world.normal : -world.normal;
            CTPoint2D< T > hit2D;
            T distance = T( 0 );
            ok = ok && plane.RayCastToPlane( CTRay3D< T >( ToP< T >( p ), towards ), hit2D, distance ) && fabs( (Float64) distance - fabs( (Float64) height ) ) <= tolerance * scale * 10.0;
            ok = ok && fabs( (Float64) ( hit2D.x - planePoint.x ) ) <= tolerance * scale * 10.0 && fabs( (Float64) ( hit2D.y - planePoint.y ) ) <= tolerance * scale * 10.0;
            ok = ok && !plane.RayCastToPlane( CTRay3D< T >( ToP< T >( p ), -towards ), hit2D, distance );
            /* the closest point of the unbounded plane is the foot, its distance the height */
            ok = ok && ( Widen( plane.ClosestPoint( ToP< T >( p ) ) ) - Widen( foot ) ).GetLength() <= tolerance * scale * 10.0;
            ok = ok && fabs( (Float64) plane.GetDistance( ToP< T >( p ) ) - fabs( (Float64) height ) ) <= tolerance * scale * 10.0;
        }
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GeoPlane: projection case " << i << " failed, seed " << seed );
        }
    }
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** Bounded planes: extents, containment, ray casts and the flat convex pair queries */
static bool
BoundedPlane( void )
{
    CGeoPlane plane;
    bool ok = !plane.HasExtents() && !plane.IsBounded();
    plane.SetExtents( CTRectangle< Float32 >( CTPoint2D< Float32 >( -1.0f, -2.0f ), CTPoint2D< Float32 >( 1.0f, 2.0f ) ) );
    ok = ok && plane.HasExtents() && plane.IsBounded() && plane.Contains( CGeoSpace::TPoint( 0.5f, 1.5f, 0.0f ) ) && !plane.Contains( CGeoSpace::TPoint( 1.5f, 1.5f, 0.0f ) );
    SRayCastResult< Float32 > hit;
    ok = ok && !plane.RayCast( CTRay3D< Float32 >( CGeoSpace::TPoint( 1.5f, 0.0f, 3.0f ), -CGeoSpace::TDirection::UnitZ() ), 10.0f, hit );
    ok = ok && plane.RayCast( CTRay3D< Float32 >( CGeoSpace::TPoint( 0.5f, 0.0f, 3.0f ), -CGeoSpace::TDirection::UnitZ() ), 10.0f, hit ) && 3.0f == hit.distance && 1.0f == hit.normal.z;
    CGeoSphere sphere( CTSphere< Float32 >( CGeoSpace::TPoint( 3.0f, 0.0f, 0.0f ), 1.0f ) );
    ok = ok && fabs( Distance( plane, sphere ) - 1.0f ) <= 1.0e-4f && IsGeoPairQueryOffered( plane, sphere, GEOPAIRQUERY_CONTACT );
    const CTAxisAlignedBox3D< Float32 >& bounds = plane.GetWorldBounds();
    ok = ok && -1.0f == bounds.min.x && 2.0f == bounds.max.y && 0.0f == bounds.max.z;
    plane.ClearExtents();
    ok = ok && !plane.IsBounded() && fabs( Distance( plane, sphere ) ) <= 1.0e-6f && Intersects( plane, sphere ) && !IsGeoPairQueryOffered( plane, sphere, GEOPAIRQUERY_CONTACT );
    sphere.SetLocalPosition( CGeoSpace::TVector( 0.0f, 0.0f, 4.0f ) );
    ok = ok && fabs( Distance( plane, sphere ) - 3.0f ) <= 1.0e-5f && !Intersects( plane, sphere );
    return ok;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGeoPlaneTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "GeoPlane" );

    GUCEF_TESTFW_TESTCASE( "GeoPlane: a circle on a plane rotated 90 degrees about +X: world centre, ray hit, distance 3 above the rim, flat GJK pair" )
    try
    {
        ASSERT_TRUE( CircleOnRotatedPlane< Float32 >( 1.0e-5 ) );
        ASSERT_TRUE( CircleOnRotatedPlane< Float64 >( 1.0e-12 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoPlane: point in polygon through the plane for a concave polygon, by containment and by ray" )
    try
    {
        ASSERT_TRUE( ConcavePolygonThroughPlane< Float32 >( 9101, ScaledCount( 4000 ) ) );
        ASSERT_TRUE( ConcavePolygonThroughPlane< Float64 >( 9102, ScaledCount( 4000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GeoPlane: ProjectToPlane / FromPlane round trip along the world normal, RayCastToPlane, bounded planes" )
    try
    {
        ASSERT_TRUE( ProjectionRoundTrip< Float32 >( 9201, ScaledCount( 4000 ), 1.0e-5 ) );
        ASSERT_TRUE( ProjectionRoundTrip< Float64 >( 9202, ScaledCount( 4000 ), 1.0e-12 ) );
        ASSERT_TRUE( BoundedPlane() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
