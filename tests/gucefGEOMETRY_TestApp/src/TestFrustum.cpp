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

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#include "TestQueryReference.h"
#include "TestFrustum.h"

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
using namespace QueryReference;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T > static Float64 FrustumTolerance( void ) { return sizeof( T ) < 8 ? 2.0e-3 : 1.0e-8; }

/** The unit vector to the right of a camera: view space +X, which is forward x up for a right-handed view and up x forward for a left-handed one */
template < typename T >
static V3
CameraRight( const SCamera< T >& camera )
{
    const V3 f = camera.forward.AsVector();
    const V3 u = camera.up.AsVector();
    return THandedness( MATH::HANDEDNESS_RIGHT ) == camera.convention.handedness ? f.Cross( u ) : u.Cross( f );
}

/*-------------------------------------------------------------------------*/

/**
 *  Points around random cameras: Contains of the frustum extracted from the combined matrix must follow the geometry of
 *  the camera (the point lies between the planes of depth and within the tangents of the half angles), also behind
 *  the camera and beyond the far plane.
 */
template < typename T >
static UInt32
PerspectiveContainsSweep( UInt64 seed, UInt32 cameras, UInt32& inside, UInt32& outside )
{
    CRandom random( seed );
    const Float64 tolerance = FrustumTolerance< T >();
    UInt32 bad = 0;
    inside = 0;
    outside = 0;
    for ( UInt32 i=0; i<cameras; ++i )
    {
        SCamera< T > camera;
        RandomCamera( random, camera );
        const V3 right = CameraRight( camera );
        for ( UInt32 k=0; k<100; ++k )
        {
            const Float64 depth = random.Uniform( -2.0, camera.farDistance * 1.3 );
            const Float64 x = random.Uniform( -1.0, 1.0 ) * camera.tanHalfX * fmax( depth, 0.5 ) * 1.3;
            const Float64 y = random.Uniform( -1.0, 1.0 ) * camera.tanHalfY * fmax( depth, 0.5 ) * 1.3;
            const P3 p = camera.eye + camera.forward.AsVector() * depth + right * x + camera.up.AsVector() * y;
            const P3 pT = Widen( ToP< T >( p ) );
            const Float64 margin = CameraMargin( camera, pT );
            if ( fabs( margin ) < tolerance )
                continue;
            const bool expected = margin > 0;
            if ( camera.frustum.Contains( ToP< T >( p ) ) != expected )
                ++bad;
            ++( expected ? inside : outside );
        }
        if ( camera.convention.infiniteFar )
        {
            /* beyond any finite far distance the point is still inside */
            const P3 far = camera.eye + camera.forward.AsVector() * 1.0e6;
            if ( !camera.frustum.Contains( ToP< T >( far ) ) || !camera.frustum.IsFarInfinite() )
                ++bad;
        }
        else if ( camera.frustum.IsFarInfinite() )
            ++bad;
    }
    return bad;
}

/*-------------------------------------------------------------------------*/

/** The corners of the frustum are where the geometry puts them: near and far plane distances, tangents of the half angles, right and top as bits 0 and 1 */
template < typename T >
static UInt32
PerspectiveCornersSweep( UInt64 seed, UInt32 cameras )
{
    CRandom random( seed );
    const Float64 tolerance = FrustumTolerance< T >();
    UInt32 bad = 0;
    for ( UInt32 i=0; i<cameras; ++i )
    {
        SCamera< T > camera;
        RandomCamera( random, camera );
        if ( camera.convention.infiniteFar )
            continue;
        const V3 right = CameraRight( camera );
        for ( UInt32 c=0; c<8; ++c )
        {
            const Float64 depth = 0 != ( c & 4 ) ? camera.farDistance : camera.nearDistance;
            const Float64 sx = 0 != ( c & 1 ) ? 1.0 : -1.0;
            /* the clip space Y of the convention points down when it is flipped, the corner named top is still the one above the view axis */
            const Float64 sy = 0 != ( c & 2 ) ? 1.0 : -1.0;
            const P3 expected = camera.eye + camera.forward.AsVector() * depth + right * ( sx * camera.tanHalfX * depth ) + camera.up.AsVector() * ( sy * camera.tanHalfY * depth );
            const P3 got = Widen( camera.frustum.GetCorner( c ) );
            if ( ( got - expected ).GetLength() > tolerance * ( 1.0 + depth ) )
                ++bad;
        }
        /* the volume of a pyramid frustum */
        const Float64 volume = 4.0 / 3.0 * camera.tanHalfX * camera.tanHalfY * ( camera.farDistance * camera.farDistance * camera.farDistance - camera.nearDistance * camera.nearDistance * camera.nearDistance );
        if ( !NearTo( (Float64) camera.frustum.GetVolume(), volume, tolerance * volume ) )
            ++bad;
        /* the bounds hold all corners, and the centre of the corners is inside */
        const CTAxisAlignedBox3D< T > bounds = camera.frustum.GetBounds();
        P3 centroid;
        for ( UInt32 c=0; c<8; ++c )
        {
            if ( !Widen( bounds ).Expanded( tolerance ).Contains( Widen( camera.frustum.GetCorner( c ) ) ) )
                ++bad;
            centroid = centroid + Widen( camera.frustum.GetCorner( c ) ).ToPositionVector() * 0.125;
        }
        if ( !camera.frustum.Contains( ToP< T >( centroid ) ) )
            ++bad;
    }
    return bad;
}

/*-------------------------------------------------------------------------*/

/** Orthographic projections in every convention: the view volume is a box in view space */
template < typename T >
static UInt32
OrthographicSweep( UInt64 seed, UInt32 cameras, UInt32& inside, UInt32& outside )
{
    typedef CTMatrix4D< T > TMatrix;
    CRandom random( seed );
    const Float64 tolerance = FrustumTolerance< T >();
    UInt32 bad = 0;
    inside = 0;
    outside = 0;
    for ( UInt32 i=0; i<cameras; ++i )
    {
        SProjectionConvention convention = RandomConvention( random );
        convention.infiniteFar = false;                 /* an orthographic projection has no infinite form */
        const P3 eye = random.Point( 3.0 );
        const D3 forward = random.Direction();
        const D3 sideways = random.Direction();
        D3 up;
        if ( !D3::TryCreate( sideways.AsVector() - forward.AsVector() * sideways.Dot( forward.AsVector() ), up ) )
            continue;
        const Float64 halfWidth = random.Uniform( 0.5, 3.0 );
        const Float64 halfHeight = random.Uniform( 0.5, 3.0 );
        const Float64 nearDistance = random.Uniform( 0.1, 1.0 );
        const Float64 farDistance = random.Uniform( 3.0, 10.0 );
        const TMatrix view = TMatrix::LookAt( ToP< T >( eye ), ToP< T >( eye + forward.AsVector() ), ToV< T >( up.AsVector() ), convention.handedness );
        const TMatrix projection = TMatrix::Orthographic( (T) -halfWidth, (T) halfWidth, (T) -halfHeight, (T) halfHeight, (T) nearDistance, (T) farDistance, convention );
        CTFrustum< T > frustum;
        if ( !CTFrustum< T >::TryFromViewProjection( projection * view, convention, frustum ) )
        {
            ++bad;
            continue;
        }
        const V3 right = MATH::HANDEDNESS_RIGHT == convention.handedness ? forward.AsVector().Cross( up.AsVector() ) : up.AsVector().Cross( forward.AsVector() );
        for ( UInt32 k=0; k<100; ++k )
        {
            const Float64 depth = random.Uniform( -1.0, farDistance * 1.3 );
            const Float64 x = random.Uniform( -1.3, 1.3 ) * halfWidth;
            const Float64 y = random.Uniform( -1.3, 1.3 ) * halfHeight;
            const P3 p = eye + forward.AsVector() * depth + right * x + up.AsVector() * y;
            const P3 pT = Widen( ToP< T >( p ) );
            const V3 d = pT - eye;
            const Float64 margin = fmin( fmin( d.Dot( forward.AsVector() ) - nearDistance, farDistance - d.Dot( forward.AsVector() ) ), fmin( halfWidth - fabs( d.Dot( right ) ), halfHeight - fabs( d.Dot( up.AsVector() ) ) ) );
            if ( fabs( margin ) < tolerance )
                continue;
            if ( frustum.Contains( ToP< T >( p ) ) != ( margin > 0 ) )
                ++bad;
            ++( margin > 0 ? inside : outside );
        }
        if ( !NearTo( (Float64) frustum.GetVolume(), 4.0 * halfWidth * halfHeight * ( farDistance - nearDistance ), tolerance * 4.0 * halfWidth * halfHeight * farDistance ) )
            ++bad;
    }
    return bad;
}

/*-------------------------------------------------------------------------*/

/**
 *  Classify of boxes and spheres: tiny ones deep inside are INSIDE, ones behind the camera, beyond the far plane or
 *  beside a side plane are OUTSIDE, and ones centred on each of the planes straddle it and are INTERSECTING.
 */
template < typename T >
static UInt32
ClassifySweep( UInt64 seed, UInt32 cameras )
{
    CRandom random( seed );
    UInt32 bad = 0;
    for ( UInt32 i=0; i<cameras; ++i )
    {
        SCamera< T > camera;
        RandomCamera( random, camera );
        const V3 right = CameraRight( camera );
        const V3 up = camera.up.AsVector();
        const V3 forward = camera.forward.AsVector();
        const Float64 middle = 0.5 * ( camera.nearDistance + ( camera.convention.infiniteFar ? 8.0 : camera.farDistance ) );
        const P3 axisPoint = camera.eye + forward * middle;
        const Float64 tiny = 0.01 * camera.nearDistance;

        /* a spec of dust deep inside */
        const CTAxisAlignedBox3D< T > inner = CTAxisAlignedBox3D< T >::FromCentreHalfExtents( ToP< T >( axisPoint ), CTVector3D< T >( (T) tiny, (T) tiny, (T) tiny ) );
        const CTSphere< T > innerSphere( ToP< T >( axisPoint ), (T) tiny );
        const CTOrientedBox3D< T > innerOriented( ToP< T >( axisPoint ), ToQ< T >( random.Rotation() ), CTVector3D< T >( (T) tiny, (T) tiny, (T) tiny ) );
        if ( FRUSTUMCLASSIFICATION_INSIDE != camera.frustum.Classify( inner ) || FRUSTUMCLASSIFICATION_INSIDE != camera.frustum.Classify( innerSphere ) ||
             FRUSTUMCLASSIFICATION_INSIDE != camera.frustum.Classify( innerOriented ) )
            ++bad;

        /* outside: behind the camera, beyond the far plane, beside each of the four side planes */
        P3 outsideCentres[ 5 ];
        UInt32 outsideCount = 0;
        outsideCentres[ outsideCount++ ] = camera.eye - forward * 2.0;
        if ( !camera.convention.infiniteFar )
            outsideCentres[ outsideCount++ ] = camera.eye + forward * ( camera.farDistance + 1.0 );
        outsideCentres[ outsideCount++ ] = axisPoint + right * ( camera.tanHalfX * middle + 2.0 );
        outsideCentres[ outsideCount++ ] = axisPoint - right * ( camera.tanHalfX * middle + 2.0 );
        outsideCentres[ outsideCount++ ] = axisPoint + up * ( camera.tanHalfY * middle + 2.0 );
        for ( UInt32 k=0; k<outsideCount; ++k )
        {
            if ( FRUSTUMCLASSIFICATION_OUTSIDE != camera.frustum.Classify( CTAxisAlignedBox3D< T >::FromCentreHalfExtents( ToP< T >( outsideCentres[ k ] ), CTVector3D< T >( (T) 0.3, (T) 0.3, (T) 0.3 ) ) ) ||
                 FRUSTUMCLASSIFICATION_OUTSIDE != camera.frustum.Classify( CTSphere< T >( ToP< T >( outsideCentres[ k ] ), (T) 0.3 ) ) ||
                 FRUSTUMCLASSIFICATION_OUTSIDE != camera.frustum.Classify( CTOrientedBox3D< T >( ToP< T >( outsideCentres[ k ] ), ToQ< T >( random.Rotation() ), CTVector3D< T >( (T) 0.3, (T) 0.3, (T) 0.3 ) ) ) )
                ++bad;
        }

        /* straddling: centred on a point of each plane, well inside the others */
        P3 planeCentres[ 6 ];
        UInt32 planeCount = 0;
        planeCentres[ planeCount++ ] = camera.eye + forward * camera.nearDistance;
        if ( !camera.convention.infiniteFar )
            planeCentres[ planeCount++ ] = camera.eye + forward * camera.farDistance;
        planeCentres[ planeCount++ ] = axisPoint + right * ( camera.tanHalfX * middle );
        planeCentres[ planeCount++ ] = axisPoint - right * ( camera.tanHalfX * middle );
        planeCentres[ planeCount++ ] = axisPoint + up * ( camera.tanHalfY * middle );
        planeCentres[ planeCount++ ] = axisPoint - up * ( camera.tanHalfY * middle );
        for ( UInt32 k=0; k<planeCount; ++k )
        {
            const Float64 size = 0.02 * middle * fmin( 1.0, fmin( camera.tanHalfX, camera.tanHalfY ) );
            if ( FRUSTUMCLASSIFICATION_INTERSECTING != camera.frustum.Classify( CTAxisAlignedBox3D< T >::FromCentreHalfExtents( ToP< T >( planeCentres[ k ] ), CTVector3D< T >( (T) size, (T) size, (T) size ) ) ) ||
                 FRUSTUMCLASSIFICATION_INTERSECTING != camera.frustum.Classify( CTSphere< T >( ToP< T >( planeCentres[ k ] ), (T) size ) ) ||
                 FRUSTUMCLASSIFICATION_INTERSECTING != camera.frustum.Classify( CTOrientedBox3D< T >( ToP< T >( planeCentres[ k ] ), ToQ< T >( random.Rotation() ), CTVector3D< T >( (T) size, (T) size, (T) size ) ) ) )
                ++bad;
        }
    }
    return bad;
}

/**
 *  A camera with an infinite far plane against the same camera with a finite far plane of 1000: for objects within
 *  500 of the eye the exact overlap tests must give the same answers, including objects at the near plane right in front
 *  of the apex, where the infinite far plane (at 1e18) once made the corners of the near plane merge.
 */
template < typename T >
static UInt32
InfiniteFarSweep( UInt64 seed, UInt32 count, UInt32& visible )
{
    typedef CTMatrix4D< T > TMatrix;
    CRandom random( seed );
    UInt32 bad = 0;
    visible = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const Float64 fov = random.Uniform( 0.5, 1.6 );
        const Float64 aspect = random.Uniform( 0.7, 2.0 );
        const Float64 nearDistance = random.Uniform( 0.05, 0.5 );
        const SProjectionConvention infinite = SProjectionConvention::        ReversedZInfinite( MATH::HANDEDNESS_RIGHT );
                const SProjectionConvention finite = SProjectionConvention::Make( MATH::HANDEDNESS_RIGHT, DEPTHRANGE_ZERO_TO_ONE, true, false, false );
        const CTFrustum< T > inf = CTFrustum< T >::FromViewProjection( TMatrix::Perspective( CTRadian< T >( (T) fov ), (T) aspect, (T) nearDistance, (T) 1000, infinite ), infinite );
        const CTFrustum< T > fin = CTFrustum< T >::FromViewProjection( TMatrix::Perspective( CTRadian< T >( (T) fov ), (T) aspect, (T) nearDistance, (T) 1000, finite ), finite );
        if ( !inf.IsFarInfinite() || fin.IsFarInfinite() )
        {
            ++bad;
            continue;
        }
        for ( UInt32 k=0; k<40; ++k )
        {
            /* view space looks down -Z: objects around the apex and the near plane, and further out */
            const Float64 scale = 0 == k % 2 ? 0.6 : 20.0;
            const P3 centre = P3( random.Uniform( -scale, scale ), random.Uniform( -scale, scale ), -random.Uniform( 0.0, scale ) );
            const Float64 size = random.Uniform( 0.02, 0.3 ) * ( 0 == k % 2 ? 0.3 : 3.0 );
            CTSphere< T > sphere( ToP< T >( centre ), (T) size );
            CTAxisAlignedBox3D< T > box = CTAxisAlignedBox3D< T >::FromCentreHalfExtents( ToP< T >( centre ), CTVector3D< T >( (T) size, (T) size, (T) size ) );
            CTOrientedBox3D< T > oriented( ToP< T >( centre ), ToQ< T >( random.Rotation() ), CTVector3D< T >( (T) size, (T) size, (T) size ) );
            const bool a = IntersectsExact( inf, sphere );
            const bool b = IntersectsExact( fin, sphere );
            const bool c = IntersectsExact( inf, box );
            const bool d = IntersectsExact( fin, box );
            const bool e = IntersectsExact( inf, oriented );
            const bool f = IntersectsExact( fin, oriented );
            if ( a != b || c != d || e != f )
                ++bad;
            if ( a || c || e )
                ++visible;
        }
    }
    return bad;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformFrustumTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Frustum" );

    typedef TFloat64Point3D P;
    typedef TFloat64Vector3D V;

    GUCEF_TESTFW_TESTCASE( "Frustum: FromViewProjection follows the geometry of the camera for every projection convention, Float32 and Float64" )
    try
    {
        UInt32 inside = 0;
        UInt32 outside = 0;
        ASSERT_TRUE( 0 == PerspectiveContainsSweep< Float32 >( 701, 400, inside, outside ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Frustum: Float32 perspective, " << inside << " points inside and " << outside << " outside checked" );
        ASSERT_TRUE( inside > 5000 && outside > 5000 );
        ASSERT_TRUE( 0 == PerspectiveContainsSweep< Float64 >( 702, 400, inside, outside ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Frustum: Float64 perspective, " << inside << " points inside and " << outside << " outside checked" );
        ASSERT_TRUE( inside > 5000 && outside > 5000 );
        ASSERT_TRUE( 0 == OrthographicSweep< Float32 >( 703, 300, inside, outside ) );
        ASSERT_TRUE( inside > 3000 && outside > 3000 );
        ASSERT_TRUE( 0 == OrthographicSweep< Float64 >( 704, 300, inside, outside ) );
        ASSERT_TRUE( inside > 3000 && outside > 3000 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Frustum: each named projection convention gives its planes, points beyond near and far are outside" )
    try
    {
        typedef CTMatrix4D< Float64 > M;
        const SProjectionConvention conventions[ 5 ] = { SProjectionConvention::OpenGL(), SProjectionConvention::Vulkan(), SProjectionConvention::Direct3D(),
                                                         SProjectionConvention::ReversedZInfinite( MATH::HANDEDNESS_RIGHT ), SProjectionConvention::ReversedZInfinite( MATH::HANDEDNESS_LEFT ) };
        for ( UInt32 c=0; c<5; ++c )
        {
            const SProjectionConvention convention = conventions[ c ];
            const Float64 forwardZ = MATH::HANDEDNESS_RIGHT == convention.handedness ? -1.0 : 1.0;      /* the view looks down -Z or +Z */
            const M projection = M::Perspective( CTRadian< Float64 >( 1.5707963267948966 ), 1.0, 1.0, 10.0, convention );
            const CTFrustum< Float64 > frustum = CTFrustum< Float64 >::FromViewProjection( projection, convention );
            /* a 90 degree field of view: the half width at depth d is d */
            ASSERT_TRUE( frustum.Contains( P( 0, 0, forwardZ * 5.0 ) ) && frustum.Contains( P( 4.9, 4.9, forwardZ * 5.0 ) ) && frustum.Contains( P( -4.9, -4.9, forwardZ * 5.0 ) ) );
            ASSERT_FALSE( frustum.Contains( P( 5.1, 0, forwardZ * 5.0 ) ) || frustum.Contains( P( 0, 5.1, forwardZ * 5.0 ) ) || frustum.Contains( P( -5.1, 0, forwardZ * 5.0 ) ) || frustum.Contains( P( 0, -5.1, forwardZ * 5.0 ) ) );
            /* beyond the near plane and behind the camera */
            ASSERT_FALSE( frustum.Contains( P( 0, 0, forwardZ * 0.9 ) ) || frustum.Contains( P( 0, 0, 0 ) ) || frustum.Contains( P( 0, 0, -forwardZ * 3.0 ) ) );
            ASSERT_TRUE( frustum.Contains( P( 0, 0, forwardZ * 1.1 ) ) );
            if ( convention.infiniteFar )
                ASSERT_TRUE( frustum.Contains( P( 0, 0, forwardZ * 1.0e9 ) ) && frustum.IsFarInfinite() );
            else
            {
                ASSERT_FALSE( frustum.Contains( P( 0, 0, forwardZ * 10.1 ) ) );
                ASSERT_TRUE( frustum.Contains( P( 0, 0, forwardZ * 9.9 ) ) && !frustum.IsFarInfinite() );
                ASSERT_TRUE( NearTo( frustum.GetVolume(), 4.0 / 3.0 * ( 1000.0 - 1.0 ), 1e-9 ) );
            }
            /* the planes face inwards and are unit */
            for ( UInt32 p=0; p<6; ++p )
                ASSERT_TRUE( NearTo( frustum.GetPlane( p ).normal.AsVector().GetLength(), 1.0, 1e-12 ) );
            ASSERT_TRUE( frustum.GetPlane( FRUSTUMPLANE_NEAR ).GetSignedDistance( P( 0, 0, forwardZ * 3.0 ) ) > 0 );
            ASSERT_TRUE( NearTo( frustum.GetPlane( FRUSTUMPLANE_NEAR ).GetSignedDistance( P( 0, 0, forwardZ * 3.0 ) ), 2.0, 1e-12 ) );
        }
        /* the clip space flip of Vulkan: top stays the plane above the view axis */
        const SProjectionConvention vulkan = SProjectionConvention::Vulkan();
        const CTFrustum< Float64 > frustum = CTFrustum< Float64 >::FromViewProjection( M::Perspective( CTRadian< Float64 >( 1.5707963267948966 ), 1.0, 1.0, 10.0, vulkan ), vulkan );
        ASSERT_TRUE( frustum.GetPlane( FRUSTUMPLANE_TOP ).GetSignedDistance( P( 0, -1, -5 ) ) > 0 && frustum.GetPlane( FRUSTUMPLANE_TOP ).GetSignedDistance( P( 0, 6, -5 ) ) < 0 );
        ASSERT_TRUE( frustum.GetPlane( FRUSTUMPLANE_BOTTOM ).GetSignedDistance( P( 0, 1, -5 ) ) > 0 && frustum.GetPlane( FRUSTUMPLANE_BOTTOM ).GetSignedDistance( P( 0, -6, -5 ) ) < 0 );

        /* a degenerate matrix gives no frustum and the clip cube from the infallible form */
        CTFrustum< Float64 > out;
        const CTFrustum< Float64 > before = out;
        ASSERT_FALSE( CTFrustum< Float64 >::TryFromViewProjection( M::Zero(), SProjectionConvention::OpenGL(), out ) );
        ASSERT_TRUE( out == before );
        ASSERT_TRUE( before.Contains( P( 0, 0, 0 ) ) && before.Contains( P( 1, 1, 1 ) ) && !before.Contains( P( 1.1, 0, 0 ) ) && NearTo( before.GetVolume(), 8.0, 1e-12 ) );
        ASSERT_TRUE( before.GetPlane( 99 ) == before.GetPlane( 0 ) || before.GetPlane( 99 ).Equals( before.GetPlane( 0 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Frustum: corners, volume, bounds and centre against the geometry of the camera" )
    try
    {
        ASSERT_TRUE( 0 == PerspectiveCornersSweep< Float32 >( 711, 300 ) );
        ASSERT_TRUE( 0 == PerspectiveCornersSweep< Float64 >( 712, 300 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Frustum: Classify of boxes, oriented boxes and spheres fully inside, outside and straddling each plane" )
    try
    {
        ASSERT_TRUE( 0 == ClassifySweep< Float32 >( 721, 400 ) );
        ASSERT_TRUE( 0 == ClassifySweep< Float64 >( 722, 400 ) );
        /* an empty box and a negative sphere are outside */
        const CTFrustum< Float64 > cube;
        ASSERT_TRUE( FRUSTUMCLASSIFICATION_OUTSIDE == cube.Classify( CTAxisAlignedBox3D< Float64 >() ) && FRUSTUMCLASSIFICATION_OUTSIDE == cube.Classify( CTSphere< Float64 >( P( 0, 0, 0 ), -1.0 ) ) );
        /* the box that equals the frustum is inside, one that is a little larger straddles all the planes */
        ASSERT_TRUE( FRUSTUMCLASSIFICATION_INSIDE == cube.Classify( CTAxisAlignedBox3D< Float64 >( P( -1, -1, -1 ), P( 1, 1, 1 ) ) ) );
        ASSERT_TRUE( FRUSTUMCLASSIFICATION_INTERSECTING == cube.Classify( CTAxisAlignedBox3D< Float64 >( P( -1.1, -1, -1 ), P( 1, 1, 1 ) ) ) );
        ASSERT_TRUE( FRUSTUMCLASSIFICATION_OUTSIDE == cube.Classify( CTAxisAlignedBox3D< Float64 >( P( 1.1, -1, -1 ), P( 2, 1, 1 ) ) ) );
        ASSERT_TRUE( FRUSTUMCLASSIFICATION_INTERSECTING == cube.Classify( CTSphere< Float64 >( P( 0, 0, 0 ), 2.0 ) ) && FRUSTUMCLASSIFICATION_INSIDE == cube.Classify( CTSphere< Float64 >( P( 0, 0, 0 ), 1.0 ) ) );
        /* the conservative classification of a box near an edge of the frustum: a box that overlaps is never OUTSIDE */
        CRandom random( 723 );
        for ( UInt32 i=0; i<2000; ++i )
        {
            CTAxisAlignedBox3D< Float64 > box;
            RandomShape( random, box );
            const CTAxisAlignedBox3D< Float64 > scaled( P( box.min.x * 0.6, box.min.y * 0.6, box.min.z * 0.6 ), P( box.max.x * 0.6, box.max.y * 0.6, box.max.z * 0.6 ) );
            const TFrustumClassification c = cube.Classify( scaled );
            GUCEF::vector< P3 > samples;
            SampleShape( scaled, random, 50, samples );
            for ( size_t k=0; k<samples.size(); ++k )
            {
                if ( cube.Contains( samples[ k ] ) && FRUSTUMCLASSIFICATION_OUTSIDE == c )
                    ASSERT_TRUE( false );
            }
            if ( FRUSTUMCLASSIFICATION_INSIDE == c )
            {
                for ( size_t k=0; k<samples.size(); ++k )
                    ASSERT_TRUE( cube.Contains( samples[ k ] ) );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Frustum: transforms keep the volume, support and wireframe, and the text and DataNode forms round trip" )
    try
    {
        typedef CTMatrix4D< Float64 > M;
        CRandom random( 731 );
        for ( UInt32 i=0; i<100; ++i )
        {
            SCamera< Float64 > camera;
            RandomCamera( random, camera );
            const M similarity = M::FromTRS( random.Vector( 3.0 ), random.Rotation(), V( 1.5, 1.5, 1.5 ) );
            CTFrustum< Float64 > moved;
            ASSERT_TRUE( camera.frustum.TryTransformed( similarity, moved ) );
            ASSERT_TRUE( moved == camera.frustum.Transformed( similarity ) );
            for ( UInt32 k=0; k<50; ++k )
            {
                const P3 p = random.Point( 6.0 );
                if ( fabs( CameraMargin( camera, p ) ) > 1e-6 )
                    ASSERT_TRUE( camera.frustum.Contains( p ) == moved.Contains( similarity.TransformPoint( p ) ) );
            }
            if ( !camera.frustum.IsFarInfinite() )
                ASSERT_TRUE( NearTo( moved.GetVolume(), camera.frustum.GetVolume() * 3.375, 1e-6 * camera.frustum.GetVolume() ) );

            /* the support point along a direction is the corner furthest along it */
            const D3 direction = random.Direction();
            const P3 support = camera.frustum.GetSupport( direction.AsVector() );
            for ( UInt32 c=0; c<8; ++c )
                ASSERT_TRUE( direction.AsVector().Dot( camera.frustum.GetCorner( c ).ToPositionVector() ) <= direction.AsVector().Dot( support.ToPositionVector() ) + 1e-9 );

            /* text and DataNode */
            CTFrustum< Float64 > fromText;
            ASSERT_TRUE( fromText.FromString( camera.frustum.ToString() ) && fromText == camera.frustum );
            CORE::CDataNodeSerializableSettings settings;
            CORE::CDataNode node;
            ASSERT_TRUE( camera.frustum.Serialize( node, settings ) );
            CTFrustum< Float64 > fromNode;
            ASSERT_TRUE( fromNode.Deserialize( node, settings ) && fromNode == camera.frustum );
            CTFrustum< Float64 > unchanged = camera.frustum;
            ASSERT_FALSE( unchanged.FromString( "Frustum(near Plane(normal (0, 0, 1), offset 1))" ) || unchanged.FromString( "" ) );
            CORE::CDataNode empty;
            ASSERT_FALSE( unchanged.Deserialize( empty, settings ) );
            ASSERT_TRUE( unchanged == camera.frustum );
        }
        CTFrustum< Float64 > cube;
        CTFrustum< Float64 > flattened;
        ASSERT_FALSE( cube.TryTransformed( M::Scale( V( 1, 0, 1 ) ), flattened ) );
        GUCEF::vector< CTSegment3D< Float64 > > lines;
        cube.GetWireframeLines( lines, 8 );
        ASSERT_TRUE( 12 == lines.size() );
        ASSERT_TRUE( cube.Equals( CTFrustum< Float64 >() ) && ( cube != CTFrustum< Float64 >::FromViewProjection( M::Perspective( CTRadian< Float64 >( 1.0 ), 1.0, 1.0, 5.0, SProjectionConvention::OpenGL() ), SProjectionConvention::OpenGL() ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    GUCEF_TESTFW_TESTCASE( "Frustum: an infinite far plane does not change the exact overlap tests, objects at the near plane included" )
    try
    {
        typedef CTMatrix4D< Float32 > M32;
        /* the case of the review: a sphere and a box overlapping the near face right at the apex */
        const SProjectionConvention infinite =         SProjectionConvention::ReversedZInfinite( MATH::HANDEDNESS_RIGHT );
        const SProjectionConvention finite = SProjectionConvention::OpenGL();
        const CTFrustum< Float32 > inf = CTFrustum< Float32 >::FromViewProjection( M32::Perspective( CTRadian< Float32 >( 1.0f ), 1.0f, 0.1f, 1000.0f, infinite ), infinite );
        const CTFrustum< Float32 > fin = CTFrustum< Float32 >::FromViewProjection( M32::Perspective( CTRadian< Float32 >( 1.0f ), 1.0f, 0.1f, 1000.0f, finite ), finite );
        const CTSphere< Float32 > sphere( TFloat32Point3D( 0, 0, -0.05f ), 0.06f );
        const CTSphere< Float32 > outsideSphere( TFloat32Point3D( 0.2f, 0, -0.05f ), 0.06f );
        const CTOrientedBox3D< Float32 > box = CTOrientedBox3D< Float32 >::FromAxisAlignedBox( CTAxisAlignedBox3D< Float32 >( TFloat32Point3D( -0.01f, -0.01f, -0.12f ), TFloat32Point3D( 0.01f, 0.01f, -0.09f ) ) );
        ASSERT_TRUE( IntersectsExact( inf, sphere ) && IntersectsExact( fin, sphere ) && IntersectsExact( inf, box ) && IntersectsExact( fin, box ) );
        ASSERT_TRUE( IntersectsExact( inf, outsideSphere ) == IntersectsExact( fin, outsideSphere ) );
        /* the body of the volume of the infinite frustum: the corners of the near plane are kept apart from each other */
        GUCEF::vector< CTPoint3D< Float32 > > vertices;
        GUCEF::vector< SConvexBodyFace > faces;
        CTPlaneBoundedVolume< Float32 >::FromFrustum( inf ).BuildConvexBody( vertices, faces );
        UInt32 nearCorners = 0;
        for ( size_t v=0; v<vertices.size(); ++v )
        {
            if ( fabs( vertices[ v ].z ) < 1.0f )
                ++nearCorners;
        }
        ASSERT_TRUE( 4 == nearCorners || vertices.empty() );
        UInt32 visible = 0;
        ASSERT_TRUE( 0 == InfiniteFarSweep< Float32 >( 751, 500, visible ) );
        ASSERT_TRUE( visible > 3000 );
        ASSERT_TRUE( 0 == InfiniteFarSweep< Float64 >( 752, 500, visible ) );
        ASSERT_TRUE( visible > 3000 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
