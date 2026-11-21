/*
 *  gucefMATH_TestApp: GUCEF test application for the Math module
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

#include <float.h>
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

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef GUCEF_MATH_CCOORDINATESYSTEM_H
#include "gucefMATH_CCoordinateSystem.h"
#define GUCEF_MATH_CCOORDINATESYSTEM_H
#endif /* GUCEF_MATH_CCOORDINATESYSTEM_H ? */

#ifndef GUCEF_MATH_CTORTHONORMALFRAME3D_H
#include "gucefMATH_CTOrthonormalFrame3D.h"
#define GUCEF_MATH_CTORTHONORMALFRAME3D_H
#endif /* GUCEF_MATH_CTORTHONORMALFRAME3D_H ? */

#include "TestMathReference.h"
#include "TestFrames.h"

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const Float64 TestPi = 3.14159265358979323846264338327950288;

/*-------------------------------------------------------------------------*/

static bool
Near( Float64 a, Float64 b, Float64 tolerance )
{
    return fabs( a - b ) <= tolerance;
}

/*-------------------------------------------------------------------------*/

static Float64
AngleDistance( Float64 a, Float64 b )
{
    Float64 d = fmod( a - b, 2.0 * TestPi );
    if ( d > TestPi ) d -= 2.0 * TestPi;
    if ( d < -TestPi ) d += 2.0 * TestPi;
    return fabs( d );
}

/*-------------------------------------------------------------------------*/

static TFloat64Quaternion
RandomUnitQuaternion( MathReference::CSeededSequence& generator )
{
    for ( ;; )
    {
        const Float64 x = generator.NextFloat64( -1.0, 1.0 );
        const Float64 y = generator.NextFloat64( -1.0, 1.0 );
        const Float64 z = generator.NextFloat64( -1.0, 1.0 );
        const Float64 w = generator.NextFloat64( -1.0, 1.0 );
        const Float64 sum = x * x + y * y + z * z + w * w;
        if ( sum > 1e-4 && sum <= 1.0 )
        {
            const Float64 n = sqrt( sum );
            return TFloat64Quaternion::FromXYZW( x / n, y / n, z / n, w / n );
        }
    }
}

/*-------------------------------------------------------------------------*/

/** The largest component difference of the axes of two frames */
template < typename TFrame >
static Float64
FrameDistance( const TFrame& a, const TFrame& b )
{
    Float64 largest = 0.0;
    const Float64 da[ 9 ] = { (Float64) a.GetRight().GetX(), (Float64) a.GetRight().GetY(), (Float64) a.GetRight().GetZ(), (Float64) a.GetUp().GetX(), (Float64) a.GetUp().GetY(), (Float64) a.GetUp().GetZ(),
                              (Float64) a.GetForward().GetX(), (Float64) a.GetForward().GetY(), (Float64) a.GetForward().GetZ() };
    const Float64 db[ 9 ] = { (Float64) b.GetRight().GetX(), (Float64) b.GetRight().GetY(), (Float64) b.GetRight().GetZ(), (Float64) b.GetUp().GetX(), (Float64) b.GetUp().GetY(), (Float64) b.GetUp().GetZ(),
                              (Float64) b.GetForward().GetX(), (Float64) b.GetForward().GetY(), (Float64) b.GetForward().GetZ() };
    for ( UInt32 i = 0; i < 9; ++i )
    {
        if ( fabs( da[ i ] - db[ i ] ) > largest )
            largest = fabs( da[ i ] - db[ i ] );
    }
    return largest;
}

/*-------------------------------------------------------------------------*/

/** How far the axes of the frame are from orthonormal: the largest deviation of a length from 1 or of a dot product from 0 */
template < typename TFrame >
static Float64
Orthonormality( const TFrame& frame )
{
    const TFloat64Vector3D r( (Float64) frame.GetRight().GetX(), (Float64) frame.GetRight().GetY(), (Float64) frame.GetRight().GetZ() );
    const TFloat64Vector3D u( (Float64) frame.GetUp().GetX(), (Float64) frame.GetUp().GetY(), (Float64) frame.GetUp().GetZ() );
    const TFloat64Vector3D f( (Float64) frame.GetForward().GetX(), (Float64) frame.GetForward().GetY(), (Float64) frame.GetForward().GetZ() );
    Float64 largest = fabs( r.GetLength() - 1.0 );
    if ( fabs( u.GetLength() - 1.0 ) > largest ) largest = fabs( u.GetLength() - 1.0 );
    if ( fabs( f.GetLength() - 1.0 ) > largest ) largest = fabs( f.GetLength() - 1.0 );
    if ( fabs( r.Dot( u ) ) > largest ) largest = fabs( r.Dot( u ) );
    if ( fabs( r.Dot( f ) ) > largest ) largest = fabs( r.Dot( f ) );
    if ( fabs( u.Dot( f ) ) > largest ) largest = fabs( u.Dot( f ) );
    return largest;
}

/*-------------------------------------------------------------------------*/

/** Frame work from a realtime context: a RealtimeSanitizer build reports any allocation */
static Float32
RealtimeFrameWork( Float32 x ) GUCEF_MATH_REALTIME_CONTEXT
{
    TFloat32OrthonormalFrame3D frame = TFloat32OrthonormalFrame3D::FromForwardUp( TFloat32Vector3D( 0.2f, 0.1f, -1.0f ), TFloat32Vector3D::Up() );
    TFloat32OrthonormalFrame3D other;
    const bool ok = TFloat32OrthonormalFrame3D::TryFromForwardUp( TFloat32Vector3D( 1.0f, 0.0f, 0.0f ), TFloat32Vector3D::Up(), MATH::HANDEDNESS_LEFT, other );
    TFloat32OrthonormalFrame3D fromQuaternion = TFloat32OrthonormalFrame3D::FromQuaternion( TFloat32Quaternion::FromAxisAngle( TFloat32DirectionVector3D::Up(), TFloat32Radian( 0.5f ) ), CCoordinateSystem::GucefDefault() );
    TFloat32OrthonormalFrame3D identity = TFloat32OrthonormalFrame3D::Identity( CCoordinateSystem::Unity() );
    const TFloat32Vector3D world( x, 1.0f, -3.0f );
    const TFloat32Vector3D local = frame.ToFrameCoordinates( world );
    const TFloat32Vector3D back = frame.FromFrameCoordinates( local );
    const TFloat32SphericalCoordinates spherical = frame.ToSpherical( world );
    return ( ok ? 1.0f : 0.0f ) + back.x + frame.GetAzimuth( world ).GetValue() + frame.GetElevation( world ).GetValue() + spherical.radius +
           fromQuaternion.GetRight().GetX() + identity.GetForward().GetZ() + ( frame.Equals( other ) ? 1.0f : 0.0f );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformFramesTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Frames" );

    GUCEF_TESTFW_TESTCASE( "Frames: listener forward -Z, up +Y: +X is heard right (azimuth +90), +Y is up (elevation +90), +Z is behind (azimuth 180)" )
    try
    {
        // acceptance 1 (gucefAUDIO 10.2.4)
        const TFloat64OrthonormalFrame3D listener = TFloat64OrthonormalFrame3D::FromForwardUp( TFloat64Vector3D( 0, 0, -1 ), TFloat64Vector3D( 0, 1, 0 ) );
        ASSERT_TRUE( listener == TFloat64OrthonormalFrame3D() );
        ASSERT_TRUE( listener.GetRight() == TFloat64DirectionVector3D::Right() && listener.GetUp() == TFloat64DirectionVector3D::Up() && listener.GetForward() == TFloat64DirectionVector3D::Forward() );

        const TFloat64Vector3D plusX( 1, 0, 0 );
        const TFloat64Vector3D plusY( 0, 1, 0 );
        const TFloat64Vector3D plusZ( 0, 0, 1 );
        ASSERT_TRUE( Near( listener.GetAzimuth( plusX ).GetValue(), TestPi / 2.0, 1e-15 ) && 0.0 == listener.GetElevation( plusX ).GetValue() );
        ASSERT_TRUE( Near( listener.GetElevation( plusY ).GetValue(), TestPi / 2.0, 1e-15 ) && 0.0 == listener.GetAzimuth( plusY ).GetValue() );
        ASSERT_TRUE( Near( fabs( listener.GetAzimuth( plusZ ).GetValue() ), TestPi, 1e-15 ) && 0.0 == listener.GetElevation( plusZ ).GetValue() );
        // straight ahead, left, down
        ASSERT_TRUE( 0.0 == listener.GetAzimuth( TFloat64Vector3D( 0, 0, -5 ) ).GetValue() && 0.0 == listener.GetElevation( TFloat64Vector3D( 0, 0, -5 ) ).GetValue() );
        ASSERT_TRUE( Near( listener.GetAzimuth( TFloat64Vector3D( -2, 0, 0 ) ).GetValue(), -TestPi / 2.0, 1e-15 ) );
        ASSERT_TRUE( Near( listener.GetElevation( TFloat64Vector3D( 0, -2, 0 ) ).GetValue(), -TestPi / 2.0, 1e-15 ) );
        // the coordinates and the spherical form
        const TFloat64Vector3D local = listener.ToFrameCoordinates( TFloat64Vector3D( 1, 2, -3 ) );
        ASSERT_TRUE( local.x == 1.0 && local.y == 2.0 && local.z == 3.0 );
        const TFloat64SphericalCoordinates spherical = listener.ToSpherical( TFloat64Vector3D( 3, 0, 0 ) );
        ASSERT_TRUE( 3.0 == spherical.radius && Near( spherical.azimuth.GetValue(), TestPi / 2.0, 1e-15 ) && 0.0 == spherical.elevation.GetValue() );
        // the zero vector
        ASSERT_TRUE( 0.0 == listener.GetAzimuth( TFloat64Vector3D::Zero() ).GetValue() && 0.0 == listener.GetElevation( TFloat64Vector3D::Zero() ).GetValue() );
        // the vertical axis has no azimuth
        ASSERT_TRUE( 0.0 == listener.GetAzimuth( TFloat64Vector3D( 0.0, 3.0, 0.0 ) ).GetValue() && 0.0 == listener.GetAzimuth( TFloat64Vector3D( -0.0, -3.0, -0.0 ) ).GetValue() );

        // listener rotated 180 degrees about Y: the source at +X is heard left (azimuth -90)
        const TFloat64OrthonormalFrame3D turnedAround = TFloat64OrthonormalFrame3D::FromForwardUp( TFloat64Vector3D( 0, 0, 1 ), TFloat64Vector3D( 0, 1, 0 ) );
        ASSERT_TRUE( Near( turnedAround.GetAzimuth( plusX ).GetValue(), -TestPi / 2.0, 1e-15 ) );
        ASSERT_TRUE( Near( fabs( turnedAround.GetAzimuth( TFloat64Vector3D( 0, 0, -1 ) ).GetValue() ), TestPi, 1e-15 ) && 0.0 == turnedAround.GetAzimuth( plusZ ).GetValue() );
        const TFloat64Quaternion halfTurn = TFloat64Quaternion::FromAxisAngle( TFloat64DirectionVector3D::Up(), TFloat64Radian( TestPi ) );
        const TFloat64OrthonormalFrame3D rotated = TFloat64OrthonormalFrame3D::FromQuaternion( halfTurn, CCoordinateSystem::GucefDefault() );
        ASSERT_TRUE( Near( rotated.GetAzimuth( plusX ).GetValue(), -TestPi / 2.0, 1e-12 ) && FrameDistance( rotated, turnedAround ) < 1e-12 );

        // tilted up by 30 degrees: a source straight ahead along the old forward is 30 degrees below the new forward
        const TFloat64OrthonormalFrame3D tilted = TFloat64OrthonormalFrame3D::FromForwardUp( TFloat64Vector3D( 0, sin( TestPi / 6.0 ), -cos( TestPi / 6.0 ) ), TFloat64Vector3D( 0, 1, 0 ) );
        ASSERT_TRUE( Near( tilted.GetElevation( TFloat64Vector3D( 0, 0, -1 ) ).GetValue(), -TestPi / 6.0, 1e-12 ) && Near( tilted.GetAzimuth( TFloat64Vector3D( 0, 0, -1 ) ).GetValue(), 0.0, 1e-12 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Frames: azimuth and elevation of the default listener in 1 degree steps against first principles" )
    try
    {
        const TFloat64OrthonormalFrame3D listener;
        UInt32 failures = 0;
        for ( Int32 azimuthDegrees = -179; azimuthDegrees <= 180; ++azimuthDegrees )
        {
            for ( Int32 elevationDegrees = -89; elevationDegrees <= 89; elevationDegrees += 2 )
            {
                const Float64 a = (Float64) azimuthDegrees * TestPi / 180.0;
                const Float64 e = (Float64) elevationDegrees * TestPi / 180.0;
                // positive azimuth to the right, 0 ahead along -Z
                const TFloat64Vector3D source( 7.0 * sin( a ) * cos( e ), 7.0 * sin( e ), -7.0 * cos( a ) * cos( e ) );
                if ( AngleDistance( listener.GetAzimuth( source ).GetValue(), a ) > 1e-12 || fabs( listener.GetElevation( source ).GetValue() - e ) > 1e-12 )
                    ++failures;
            }
        }
        ASSERT_TRUE( 0 == failures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Frames: FromQuaternion equals FromForwardUp for 10^3 seeded orientations, in a right-handed and a left-handed system" )
    try
    {
        // acceptance 1
        const UInt64 seed = 0xF4A3;
        MathReference::CSeededSequence generator( seed );
        const CCoordinateSystem gucef = CCoordinateSystem::GucefDefault();
        const CCoordinateSystem unity = CCoordinateSystem::Unity();
        UInt32 failures = 0;
        Float64 worst = 0.0;
        for ( UInt32 i = 0; i < 1000; ++i )
        {
            const TFloat64Quaternion q = RandomUnitQuaternion( generator );
            const TFloat64OrthonormalFrame3D fromQuaternion = TFloat64OrthonormalFrame3D::FromQuaternion( q, gucef );
            const TFloat64OrthonormalFrame3D fromVectors = TFloat64OrthonormalFrame3D::FromForwardUp( q.GetForward( gucef ).AsVector(), q.GetUp( gucef ).AsVector() );
            const Float64 distance = FrameDistance( fromQuaternion, fromVectors );
            if ( distance > worst )
                worst = distance;
            // the left-handed system: forward is +Z
            const TFloat64OrthonormalFrame3D leftFromQuaternion = TFloat64OrthonormalFrame3D::FromQuaternion( q, unity );
            const TFloat64OrthonormalFrame3D leftFromVectors = TFloat64OrthonormalFrame3D::FromForwardUp( q.GetForward( unity ).AsVector(), q.GetUp( unity ).AsVector(), MATH::HANDEDNESS_LEFT );
            if ( distance > 1e-9 || FrameDistance( leftFromQuaternion, leftFromVectors ) > 1e-9 || Orthonormality( fromQuaternion ) > 1e-12 || Orthonormality( leftFromQuaternion ) > 1e-12 )
                ++failures;
            // the frame of a quaternion turns the system axes: its forward is the rotated forward of the system
            const TFloat64Vector3D rotatedForward = q.Rotate( gucef.GetForward< Float64 >() );
            if ( fabs( rotatedForward.x - fromQuaternion.GetForward().GetX() ) > 1e-12 || fabs( rotatedForward.y - fromQuaternion.GetForward().GetY() ) > 1e-12 || fabs( rotatedForward.z - fromQuaternion.GetForward().GetZ() ) > 1e-12 )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        ASSERT_TRUE( worst < 1e-12 );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Frames: " << failures << " quaternion frame checks failed, seed " << seed << ", worst distance " << worst );

        // the unrotated frames of the coordinate systems
        const CCoordinateSystem systems[ 4 ] = { CCoordinateSystem::GucefDefault(), CCoordinateSystem::Unity(), CCoordinateSystem::Unreal(), CCoordinateSystem::Blender() };
        for ( UInt32 s = 0; s < 4; ++s )
        {
            const TFloat64OrthonormalFrame3D identity = TFloat64OrthonormalFrame3D::Identity( systems[ s ] );
            const TFloat64OrthonormalFrame3D fromIdentityQuaternion = TFloat64OrthonormalFrame3D::FromQuaternion( TFloat64Quaternion::Identity(), systems[ s ] );
            const TFloat64Vector3D r = systems[ s ].GetRight< Float64 >();
            const TFloat64Vector3D u = systems[ s ].GetUp< Float64 >();
            const TFloat64Vector3D f = systems[ s ].GetForward< Float64 >();
            ASSERT_TRUE( identity.GetRight().AsVector() == r && identity.GetUp().AsVector() == u && identity.GetForward().AsVector() == f );
            ASSERT_TRUE( FrameDistance( identity, fromIdentityQuaternion ) < 1e-15 );
            // a source along the right axis of the system is at azimuth +90 whatever the handedness, along the forward axis at 0, along up at elevation +90
            ASSERT_TRUE( Near( identity.GetAzimuth( r ).GetValue(), TestPi / 2.0, 1e-15 ) && 0.0 == identity.GetAzimuth( f ).GetValue() && Near( identity.GetElevation( u ).GetValue(), TestPi / 2.0, 1e-15 ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Frames: degenerate input: parallel forward and up, zero and non-finite vectors" )
    try
    {
        const TFloat64OrthonormalFrame3D identity;
        TFloat64OrthonormalFrame3D out = TFloat64OrthonormalFrame3D::FromForwardUp( TFloat64Vector3D( 1, 0, 0 ), TFloat64Vector3D( 0, 1, 0 ) );
        const TFloat64OrthonormalFrame3D before = out;
        const Float64 nan = CTNumericTraits< Float64 >::Max() * 2.0 - CTNumericTraits< Float64 >::Max() * 2.0;
        ASSERT_FALSE( TFloat64OrthonormalFrame3D::TryFromForwardUp( TFloat64Vector3D( 0, 1, 0 ), TFloat64Vector3D( 0, 1, 0 ), out ) );
        ASSERT_FALSE( TFloat64OrthonormalFrame3D::TryFromForwardUp( TFloat64Vector3D( 0, 1, 0 ), TFloat64Vector3D( 0, -3, 0 ), out ) );
        ASSERT_FALSE( TFloat64OrthonormalFrame3D::TryFromForwardUp( TFloat64Vector3D( 0, 0, 0 ), TFloat64Vector3D( 0, 1, 0 ), out ) );
        ASSERT_FALSE( TFloat64OrthonormalFrame3D::TryFromForwardUp( TFloat64Vector3D( 1, 0, 0 ), TFloat64Vector3D( 0, 0, 0 ), out ) );
        ASSERT_FALSE( TFloat64OrthonormalFrame3D::TryFromForwardUp( TFloat64Vector3D( nan, 0, 0 ), TFloat64Vector3D( 0, 1, 0 ), out ) );
        ASSERT_FALSE( TFloat64OrthonormalFrame3D::TryFromForwardUp( TFloat64Vector3D( 1, 0, 0 ),         TFloat64Vector3D( 0, nan, 0 ), MATH::HANDEDNESS_LEFT, out ) );
                ASSERT_FALSE( TFloat64OrthonormalFrame3D::TryFromForwardUp( TFloat64Vector3D( 2, 0, 0 ), TFloat64Vector3D( 5, 0, 0 ), MATH::HANDEDNESS_LEFT, out ) );
        ASSERT_TRUE( out == before );

        #ifndef GUCEF_MATH_DEBUG_MODE
        // the infallible form gives the identity frame of the default convention (debug builds assert)
        ASSERT_TRUE( TFloat64OrthonormalFrame3D::FromForwardUp( TFloat64Vector3D( 0, 1, 0 ), TFloat64Vector3D( 0, 2, 0 ) ) == identity );
        ASSERT_TRUE( TFloat64OrthonormalFrame3D::FromForwardUp( TFloat64Vector3D( 0, 0, 0 ), TFloat64Vector3D( 0, 1, 0 ) ) == identity );
        ASSERT_TRUE( TFloat64OrthonormalFrame3D::FromForwardUp( TFloat64Vector3D( 1, 0, 0 ), TFloat64Vector3D( 1, 0, 0 ), MATH::HANDEDNESS_LEFT ) == identity );
        #endif

        // nearly parallel is fine: still orthonormal, the forward axis is kept
        TFloat64OrthonormalFrame3D nearly;
        ASSERT_TRUE( TFloat64OrthonormalFrame3D::TryFromForwardUp( TFloat64Vector3D( 1, 0, 0 ), TFloat64Vector3D( 1, 1e-4, 0 ), nearly ) );
        ASSERT_TRUE( Orthonormality( nearly ) < 1e-12 && nearly.GetForward() == TFloat64DirectionVector3D::Right() );
        ASSERT_TRUE( nearly.GetUp().GetY() > 0.99999999 );

        // vectors at the extremes of the kind
        const Float64 huge = 1.0e300;
        const Float64 tiny = 1.0e-300;
        TFloat64OrthonormalFrame3D scaled;
        ASSERT_TRUE( TFloat64OrthonormalFrame3D::TryFromForwardUp( TFloat64Vector3D( 0, 0, -huge ), TFloat64Vector3D( 0, tiny, 0 ), scaled ) && scaled == identity );
        ASSERT_TRUE( TFloat64OrthonormalFrame3D::TryFromForwardUp( TFloat64Vector3D( tiny, 0, 0 ), TFloat64Vector3D( 0, huge, 0 ), scaled ) && Orthonormality( scaled ) < 1e-12 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Frames: orthonormal axes for any input; right = normalise( forward x up ), up' = right x forward (seeded, both handedness)" )
    try
    {
        const UInt64 seed = 0x0F21;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        Float64 worst = 0.0;
        for ( UInt32 i = 0; i < 20000; ++i )
        {
            const Float64 scaleF = pow( 10.0, generator.NextFloat64( -8.0, 8.0 ) );
            const Float64 scaleU = pow( 10.0, generator.NextFloat64( -8.0, 8.0 ) );
            const TFloat64Vector3D forward( generator.NextFloat64( -1, 1 ) * scaleF, generator.NextFloat64( -1, 1 ) * scaleF, generator.NextFloat64( -1, 1 ) * scaleF );
            const TFloat64Vector3D up( generator.NextFloat64( -1, 1 ) * scaleU, generator.NextFloat64( -1, 1 ) * scaleU, generator.NextFloat64( -1, 1 ) * scaleU );
            for ( UInt32 h = 0; h < 2; ++h )
            {
                const THandedness handedness = 0 == h ? MATH::HANDEDNESS_RIGHT : MATH::HANDEDNESS_LEFT;
                TFloat64OrthonormalFrame3D frame;
                const bool created = TFloat64OrthonormalFrame3D::TryFromForwardUp( forward, up, handedness, frame );
                const TFloat64Vector3D f = forward.Normalized();
                const TFloat64Vector3D u = up.Normalized();
                const TFloat64Vector3D cross = 0 == h ? f.Cross( u ) : u.Cross( f );
                if ( cross.GetLength() < 1e-6 || f.GetLength() < 0.5 || u.GetLength() < 0.5 )
                    continue;
                if ( !created )
                {
                    ++failures;
                    continue;
                }
                const Float64 deviation = Orthonormality( frame );
                if ( deviation > worst )
                    worst = deviation;
                const TFloat64Vector3D r = cross.Normalized();
                // the forward axis is the forward vector, the right axis its cross product, the up axis on the side of the up hint
                const Float64 dr = fabs( frame.GetRight().GetX() - r.x ) + fabs( frame.GetRight().GetY() - r.y ) + fabs( frame.GetRight().GetZ() - r.z );
                const Float64 df = fabs( frame.GetForward().GetX() - f.x ) + fabs( frame.GetForward().GetY() - f.y ) + fabs( frame.GetForward().GetZ() - f.z );
                const bool upSide = frame.GetUp().AsVector().Dot( u ) > 0.0;
                // the handedness of the axes: right x up is -forward (right-handed) or +forward (left-handed)
                const TFloat64Vector3D rightCrossUp = frame.GetRight().AsVector().Cross( frame.GetUp().AsVector() );
                const Float64 sign = 0 == h ? -1.0 : 1.0;
                const Float64 handedDeviation = ( rightCrossUp - frame.GetForward().AsVector() * sign ).GetLength();
                if ( deviation > 1e-12 || dr > 1e-5 || df > 1e-14 || !upSide || handedDeviation > 1e-12 )
                    ++failures;
            }
        }
        ASSERT_TRUE( 0 == failures );
        ASSERT_TRUE( worst < 1e-12 );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Frames: " << failures << " orthonormal frame checks failed, seed " << seed << ", worst " << worst );

        // Float32
        UInt32 failures32 = 0;
        for ( UInt32 i = 0; i < 5000; ++i )
        {
            const TFloat32Vector3D forward( (Float32) generator.NextFloat64( -1, 1 ), (Float32) generator.NextFloat64( -1, 1 ), (Float32) generator.NextFloat64( -1, 1 ) );
            const TFloat32Vector3D up( (Float32) generator.NextFloat64( -1, 1 ), (Float32) generator.NextFloat64( -1, 1 ), (Float32) generator.NextFloat64( -1, 1 ) );
            if ( forward.GetLength() < 0.1f || up.GetLength() < 0.1f || forward.Normalized().Cross( up.Normalized() ).GetLength() < 1e-2f )
                continue;
            TFloat32OrthonormalFrame3D frame;
            if ( !TFloat32OrthonormalFrame3D::TryFromForwardUp( forward, up, frame ) || Orthonormality( frame ) > 2e-6 )
                ++failures32;
        }
        ASSERT_TRUE( 0 == failures32 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Frames: frame coordinates round trip, lengths and angles against first principles (seeded)" )
    try
    {
        const UInt64 seed = 0xA91C;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i = 0; i < 5000; ++i )
        {
            const TFloat64Quaternion q = RandomUnitQuaternion( generator );
            const TFloat64OrthonormalFrame3D frame = TFloat64OrthonormalFrame3D::FromQuaternion( q, CCoordinateSystem::GucefDefault() );
            const TFloat64Vector3D world( generator.NextFloat64( -50, 50 ), generator.NextFloat64( -50, 50 ), generator.NextFloat64( -50, 50 ) );
            const TFloat64Vector3D local = frame.ToFrameCoordinates( world );
            const TFloat64Vector3D back = frame.FromFrameCoordinates( local );
            // first principles: the source rotated into the frame of the listener by the inverse rotation, forward being -Z
            const TFloat64Vector3D rotated = q.Inverse().Rotate( world );
            const Float64 expectedAzimuth = atan2( rotated.x, -rotated.z );
            const Float64 expectedElevation = atan2( rotated.y, sqrt( rotated.x * rotated.x + rotated.z * rotated.z ) );
            const TFloat64SphericalCoordinates spherical = frame.ToSpherical( world );
            if ( ( back - world ).GetLength() > 1e-12 * world.GetLength() || fabs( local.GetLength() - world.GetLength() ) > 1e-12 * world.GetLength() ||
                 fabs( local.x - rotated.x ) > 1e-11 || fabs( local.y - rotated.y ) > 1e-11 || fabs( local.z + rotated.z ) > 1e-11 ||
                 AngleDistance( frame.GetAzimuth( world ).GetValue(), expectedAzimuth ) > 1e-11 || fabs( frame.GetElevation( world ).GetValue() - expectedElevation ) > 1e-11 ||
                 AngleDistance( spherical.azimuth.GetValue(), expectedAzimuth ) > 1e-11 || fabs( spherical.elevation.GetValue() - expectedElevation ) > 1e-11 ||
                 fabs( spherical.radius - world.GetLength() ) > 1e-11 )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Frames: " << failures << " coordinate checks failed, seed " << seed );

        // Float32
        const TFloat32OrthonormalFrame3D frame32 = TFloat32OrthonormalFrame3D::FromForwardUp( TFloat32Vector3D( 1.0f, 0.0f, -1.0f ), TFloat32Vector3D::Up() );
        const TFloat32Vector3D world32( 1.0f, 0.0f, -1.0f );
        ASSERT_TRUE( Near( frame32.GetAzimuth( world32 ).GetValue(), 0.0, 1e-6 ) );
        ASSERT_TRUE( Near( frame32.GetAzimuth( TFloat32Vector3D( 1.0f, 0.0f, 1.0f ) ).GetValue(), TestPi / 2.0, 1e-6 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Frames: equality, tolerant comparison, text and DataNode forms, reading rejects axes that are not orthonormal" )
    try
    {
        const TFloat64OrthonormalFrame3D a = TFloat64OrthonormalFrame3D::FromForwardUp( TFloat64Vector3D( 1, 2, 3 ), TFloat64Vector3D( 0, 1, 0 ) );
        TFloat64OrthonormalFrame3D b = TFloat64OrthonormalFrame3D::FromForwardUp( TFloat64Vector3D( 1, 2, 3 ), TFloat64Vector3D( 0, 1, 0 ) );
        const TFloat64OrthonormalFrame3D c = TFloat64OrthonormalFrame3D::FromForwardUp( TFloat64Vector3D( 1, 2, 3.001 ), TFloat64Vector3D( 0, 1, 0 ) );
        ASSERT_TRUE( a == b && a.Equals( b ) && a != c && !a.Equals( c ) );

        // text
        const CORE::CString text = a.ToString();
        TFloat64OrthonormalFrame3D read;
        ASSERT_TRUE( read.FromString( text ) && read == a );
        TFloat64OrthonormalFrame3D keep = a;
        ASSERT_FALSE( keep.FromString( "(1, 0, 0, 0, 1, 0, 0, 0, 1" ) );
        ASSERT_FALSE( keep.FromString( "(1, 0, 0, 0, 1, 0)" ) );
        // not orthogonal, not unit length
        ASSERT_FALSE( keep.FromString( "(1, 0, 0, 1, 0, 0, 0, 0, 1)" ) );
        ASSERT_FALSE( keep.FromString( "(2, 0, 0, 0, 1, 0, 0, 0, 1)" ) );
        ASSERT_TRUE( keep == a );
        ASSERT_TRUE( read.FromString( "(1, 0, 0, 0, 1, 0, 0, 0, 1)" ) && read.GetForward() == TFloat64DirectionVector3D::Back() );

        // DataNode
        CORE::CDataNodeSerializableSettings settings;
        CORE::CDataNode node;
        TFloat64OrthonormalFrame3D fromNode;
        ASSERT_TRUE( a.Serialize( node, settings ) && fromNode.Deserialize( node, settings ) && fromNode == a );
        ASSERT_TRUE( GUCEF_NULL != node.GetAttribute( "rx" ) && GUCEF_NULL != node.GetAttribute( "fz" ) );
        CORE::CDataNode broken;
        ASSERT_TRUE( a.Serialize( broken, settings ) );
        ASSERT_TRUE( broken.SetAttribute( "fx", CORE::CVariant( 0.5 ) ) );
        ASSERT_FALSE( keep.Deserialize( broken, settings ) );
        ASSERT_TRUE( keep == a );
        CORE::CDataNode incomplete;
        ASSERT_TRUE( incomplete.SetAttribute( "rx", CORE::CVariant( 1.0 ) ) );
        ASSERT_FALSE( keep.Deserialize( incomplete, settings ) );
        ASSERT_TRUE( keep == a );
        ASSERT_TRUE( sizeof( TFloat32OrthonormalFrame3D ) == 9 * sizeof( Float32 ) && sizeof( TFloat64OrthonormalFrame3D ) == 9 * sizeof( Float64 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Frames: realtime-safe functions run in a realtime context" )
    try
    {
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( RealtimeFrameWork( 2.0f ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
