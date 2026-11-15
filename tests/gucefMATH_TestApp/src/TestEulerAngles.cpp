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

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#include "TestMathReference.h"
#include "TestEulerAngles.h"

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

static const Float64 g_pi = 3.14159265358979323846;

/*-------------------------------------------------------------------------*/

/** |a - b| as angles: the difference wrapped to (-pi, pi] */
static Float64
AngleDifference( Float64 a, Float64 b )
{
    Float64 d = fmod( a - b, 2.0 * g_pi );
    if ( d > g_pi ) d -= 2.0 * g_pi;
    if ( d < -g_pi ) d += 2.0 * g_pi;
    return fabs( d );
}

/*-------------------------------------------------------------------------*/

/** Random angles of an order: first and third in (-pi, pi), second away from the singularity by 'margin' radians */
static void
RandomAngles( MathReference::CSeededSequence& generator, TEulerOrder order, Float64 margin, Float64 ( &angles )[ 3 ] )
{
    angles[ 0 ] = generator.NextFloat64( -g_pi + 1e-6, g_pi - 1e-6 );
    angles[ 2 ] = generator.NextFloat64( -g_pi + 1e-6, g_pi - 1e-6 );
    if ( TFloat64EulerAngles::IsProperOrder( order ) )
        angles[ 1 ] = generator.NextFloat64( margin, g_pi - margin );
    else
        angles[ 1 ] = generator.NextFloat64( -g_pi / 2 + margin, g_pi / 2 - margin );
}

/*-------------------------------------------------------------------------*/

/** The elementary rotation matrix about axis 0, 1 or 2, built independently of CTEulerAngles */
static TFloat64Matrix3D
Elementary( UInt32 axis, Float64 angle )
{
    Float64 c = cos( angle );
    Float64 s = sin( angle );
    Float64 m[ 9 ] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };    // row-major
    UInt32 j = ( axis + 1 ) % 3;
    UInt32 k = ( axis + 2 ) % 3;
    m[ j * 3 + j ] = c;
    m[ k * 3 + k ] = c;
    m[ k * 3 + j ] = s;
    m[ j * 3 + k ] = -s;
    return TFloat64Matrix3D::FromRowMajor( m );
}

/*-------------------------------------------------------------------------*/

static Float64
Matrix3Distance( const TFloat64Matrix3D& a, const TFloat64Matrix3D& b )
{
    Float64 largest = 0.0;
    for ( UInt32 r=0; r<3; ++r )
        for ( UInt32 c=0; c<3; ++c )
            if ( fabs( a( r, c ) - b( r, c ) ) > largest )
                largest = fabs( a( r, c ) - b( r, c ) );
    return largest;
}

/*-------------------------------------------------------------------------*/

/** Calls realtime-safe Euler and axis-angle operations from a realtime context */
static Float32
RealtimeEulerWork( const TFloat32EulerAngles& e ) GUCEF_MATH_REALTIME_CONTEXT
{
    TFloat32Quaternion q = e.ToQuaternion();
    TFloat32Matrix3D m = e.ToRotationMatrix3D();
    TFloat32EulerAngles back = TFloat32EulerAngles::FromQuaternion( q, EULERORDER_ZXZ, EULERFRAME_EXTRINSIC );
    TFloat32EulerAngles fromMatrix = TFloat32EulerAngles::FromRotationMatrix( m, EULERORDER_XZY, EULERFRAME_INTRINSIC );
    TFloat32AxisAngle axisAngle = TFloat32AxisAngle::FromQuaternion( q );
    return back.first.GetValue() + fromMatrix.second.GetValue() + axisAngle.angle.GetValue() + axisAngle.ToRotationMatrix3D()( 1, 2 ) + axisAngle.ToQuaternion().w;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformEulerAnglesTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "EulerAngles" );

    GUCEF_TESTFW_TESTCASE( "EulerAngles: YawPitchRoll is intrinsic Y-X-Z, a positive yaw turns left" )
    try
    {
        // acceptance 2
        TFloat32EulerAngles ypr = TFloat32EulerAngles::YawPitchRoll( TFloat32Radian( TFloat32Degree( 90.0f ) ), TFloat32Radian(), TFloat32Radian() );
        ASSERT_TRUE( EULERORDER_YXZ == ypr.order && EULERFRAME_INTRINSIC == ypr.frame );
        TFloat32Vector3D turned = ypr.ToQuaternion().Rotate( TFloat32Vector3D::Forward() );
        ASSERT_TRUE( fabs( turned.x + 1.0f ) < 1e-6f && fabs( turned.y ) < 1e-6f && fabs( turned.z ) < 1e-6f );
        turned = ypr.ToRotationMatrix3D() * TFloat32Vector3D::Forward();
        ASSERT_TRUE( fabs( turned.x + 1.0f ) < 1e-6f && fabs( turned.y ) < 1e-6f && fabs( turned.z ) < 1e-6f );
        // pitch about the turned right axis: yaw 90 then pitch 90 looks up
        TFloat64EulerAngles look = TFloat64EulerAngles::YawPitchRoll( TFloat64Radian( g_pi / 2 ), TFloat64Radian( g_pi / 2 ), TFloat64Radian( 0.3 ) );
        TFloat64Vector3D forward = look.ToQuaternion().Rotate( TFloat64Vector3D::Forward() );
        ASSERT_TRUE( fabs( forward.y - 1.0 ) < 1e-15 );
        TFloat64EulerAngles defaults;
        ASSERT_TRUE( EULERORDER_YXZ == defaults.order && defaults.ToQuaternion() == TFloat64Quaternion() );
        ASSERT_TRUE( 0 == strcmp( "ZYX", TFloat64EulerAngles::GetOrderName( EULERORDER_ZYX ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "EulerAngles: the matrices of all 12 orders and both frames against independent products" )
    try
    {
        const UInt64 seed = 0xE01E;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 order=0; order<12; ++order )
        {
            UInt32 a, b, c;
            TFloat64EulerAngles::GetAxes( (TEulerOrder) order, a, b, c );
            for ( UInt32 i=0; i<100; ++i )
            {
                Float64 angles[ 3 ];
                RandomAngles( generator, (TEulerOrder) order, 0.1, angles );
                TFloat64EulerAngles intrinsic( TFloat64Radian( angles[ 0 ] ), TFloat64Radian( angles[ 1 ] ), TFloat64Radian( angles[ 2 ] ), (TEulerOrder) order, EULERFRAME_INTRINSIC );
                TFloat64EulerAngles extrinsic( TFloat64Radian( angles[ 0 ] ), TFloat64Radian( angles[ 1 ] ), TFloat64Radian( angles[ 2 ] ), (TEulerOrder) order, EULERFRAME_EXTRINSIC );
                TFloat64Matrix3D intrinsicReference = Elementary( a, angles[ 0 ] ) * Elementary( b, angles[ 1 ] ) * Elementary( c, angles[ 2 ] );
                TFloat64Matrix3D extrinsicReference = Elementary( c, angles[ 2 ] ) * Elementary( b, angles[ 1 ] ) * Elementary( a, angles[ 0 ] );
                if ( !( Matrix3Distance( intrinsic.ToRotationMatrix3D(), intrinsicReference ) < 1e-14 ) || !( Matrix3Distance( intrinsic.ToQuaternion().ToRotationMatrix3D(), intrinsicReference ) < 1e-14 ) )
                    ++failures;
                if ( !( Matrix3Distance( extrinsic.ToRotationMatrix3D(), extrinsicReference ) < 1e-14 ) || !( Matrix3Distance( extrinsic.ToQuaternion().ToRotationMatrix3D(), extrinsicReference ) < 1e-14 ) )
                    ++failures;
            }
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "EulerAngles: " << failures << " matrices differ from the reference products, seed " << seed );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "EulerAngles: round trip for all 12 orders x 2 frames at 10^3 seeded triples away from the singularity" )
    try
    {
        // acceptance 7: +-1e-5 rad (Float32), and 1e-9 rad in Float64
        const UInt64 seed = 0xE7A2;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        Float64 worst32 = 0.0;
        for ( UInt32 order=0; order<12; ++order )
        {
            for ( UInt32 frame=0; frame<2; ++frame )
            {
                for ( UInt32 i=0; i<1000; ++i )
                {
                    Float64 angles[ 3 ];
                    RandomAngles( generator, (TEulerOrder) order, 0.05, angles );
                    TFloat64EulerAngles e( TFloat64Radian( angles[ 0 ] ), TFloat64Radian( angles[ 1 ] ), TFloat64Radian( angles[ 2 ] ), (TEulerOrder) order, (TEulerFrame) frame );
                    TFloat64EulerAngles back = TFloat64EulerAngles::FromQuaternion( e.ToQuaternion(), (TEulerOrder) order, (TEulerFrame) frame );
                    if ( back.order != e.order || back.frame != e.frame || AngleDifference( back.first.GetValue(), angles[ 0 ] ) > 1e-9 ||
                         AngleDifference( back.second.GetValue(), angles[ 1 ] ) > 1e-9 || AngleDifference( back.third.GetValue(), angles[ 2 ] ) > 1e-9 )
                        ++failures;
                    TFloat64EulerAngles fromMatrix = TFloat64EulerAngles::FromRotationMatrix( e.ToRotationMatrix3D(), (TEulerOrder) order, (TEulerFrame) frame );
                    if ( AngleDifference( fromMatrix.second.GetValue(), angles[ 1 ] ) > 1e-9 )
                        ++failures;

                    TFloat32EulerAngles e32( TFloat32Radian( (Float32) angles[ 0 ] ), TFloat32Radian( (Float32) angles[ 1 ] ), TFloat32Radian( (Float32) angles[ 2 ] ), (TEulerOrder) order, (TEulerFrame) frame );
                    TFloat32EulerAngles back32 = e32.ToQuaternion().ToEuler( (TEulerOrder) order, (TEulerFrame) frame );
                    Float64 d = AngleDifference( back32.first.GetValue(), e32.first.GetValue() );
                    if ( AngleDifference( back32.second.GetValue(), e32.second.GetValue() ) > d ) d = AngleDifference( back32.second.GetValue(), e32.second.GetValue() );
                    if ( AngleDifference( back32.third.GetValue(), e32.third.GetValue() ) > d ) d = AngleDifference( back32.third.GetValue(), e32.third.GetValue() );
                    if ( d > worst32 ) worst32 = d;
                    if ( d > 1e-5 )
                        ++failures;
                }
            }
        }
        ASSERT_TRUE( 0 == failures );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EulerAngles: largest Float32 round trip error in units of 1e-6 rad: " << worst32 * 1e6 );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "EulerAngles: " << failures << " round trips failed, seed " << seed );
        // intrinsic ( A, B, C ) equals extrinsic ( C, B, A ) with the angles reversed
        TFloat64EulerAngles xyz( TFloat64Radian( 0.3 ), TFloat64Radian( -0.7 ), TFloat64Radian( 1.9 ), EULERORDER_XYZ, EULERFRAME_INTRINSIC );
        TFloat64EulerAngles zyx( TFloat64Radian( 1.9 ), TFloat64Radian( -0.7 ), TFloat64Radian( 0.3 ), EULERORDER_ZYX, EULERFRAME_EXTRINSIC );
        ASSERT_TRUE( xyz.ToQuaternion().EqualsRotation( zyx.ToQuaternion(), TFloat64Radian( 1e-14 ) ) );
        ASSERT_TRUE( TFloat64Quaternion::FromEuler( xyz ) == xyz.ToQuaternion() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "EulerAngles: at the singularity the third angle is 0 and the rotation is preserved" )
    try
    {
        const UInt64 seed = 0x5106;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 order=0; order<12; ++order )
        {
            const bool proper = TFloat64EulerAngles::IsProperOrder( (TEulerOrder) order );
            for ( UInt32 frame=0; frame<2; ++frame )
            {
                for ( UInt32 i=0; i<200; ++i )
                {
                    const Float64 singular[ 2 ] = { proper ? 0.0 : g_pi / 2, proper ? g_pi : -g_pi / 2 };
                    TFloat64EulerAngles e( TFloat64Radian( generator.NextFloat64( -3, 3 ) ), TFloat64Radian( singular[ i % 2 ] ), TFloat64Radian( generator.NextFloat64( -3, 3 ) ), (TEulerOrder) order, (TEulerFrame) frame );
                    TFloat64Quaternion q = e.ToQuaternion();
                    TFloat64EulerAngles back = TFloat64EulerAngles::FromQuaternion( q, (TEulerOrder) order, (TEulerFrame) frame );
                    if ( 0.0 != back.third.GetValue() || !back.ToQuaternion().EqualsRotation( q, TFloat64Radian( 1e-7 ) ) )
                        ++failures;
                    TFloat32EulerAngles e32( TFloat32Radian( (Float32) e.first.GetValue() ), TFloat32Radian( (Float32) e.second.GetValue() ), TFloat32Radian( (Float32) e.third.GetValue() ), (TEulerOrder) order, (TEulerFrame) frame );
                    TFloat32Quaternion q32 = e32.ToQuaternion();
                    TFloat32EulerAngles back32 = TFloat32EulerAngles::FromQuaternion( q32, (TEulerOrder) order, (TEulerFrame) frame );
                    if ( 0.0f != back32.third.GetValue() || !back32.ToQuaternion().EqualsRotation( q32, TFloat32Radian( 1e-3f ) ) )
                        ++failures;
                }
            }
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "EulerAngles: " << failures << " singular extractions failed, seed " << seed );
        // deterministic: the same input gives the same output
        TFloat64Quaternion gimbal = TFloat64EulerAngles::YawPitchRoll( TFloat64Radian( 0.4 ), TFloat64Radian( g_pi / 2 ), TFloat64Radian( 0.2 ) ).ToQuaternion();
        ASSERT_TRUE( gimbal.ToEuler( EULERORDER_YXZ, EULERFRAME_INTRINSIC ) == gimbal.ToEuler( EULERORDER_YXZ, EULERFRAME_INTRINSIC ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "EulerAngles: close to gimbal lock the extracted angles still rebuild the rotation" )
    try
    {
        // regression: second angle 90 degrees - delta for delta 1e-1 .. 1e-7, every order and frame, Float32;
        // the outer angles are ill-conditioned separately, their combination must be preserved (1e-5 rad)
        const UInt64 seed = 0x61B1;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        Float64 worst = 0.0;
        for ( UInt32 order=0; order<12; ++order )
        {
            const bool proper = TFloat32EulerAngles::IsProperOrder( (TEulerOrder) order );
            for ( UInt32 frame=0; frame<2; ++frame )
            {
                for ( Int32 exponent=-1; exponent>=-7; --exponent )
                {
                    for ( UInt32 i=0; i<20; ++i )
                    {
                        const Float64 delta = pow( 10.0, (Float64) exponent ) * ( 0 == ( i & 1 ) ? 1.0 : -1.0 );
                        const Float64 second = proper ? ( 0 == ( i & 2 ) ? fabs( delta ) : g_pi - fabs( delta ) ) : ( 0 == ( i & 2 ) ? g_pi / 2 : -g_pi / 2 ) - delta;
                        TFloat32EulerAngles e( TFloat32Radian( (Float32) generator.NextFloat64( -3, 3 ) ), TFloat32Radian( (Float32) second ),
                                               TFloat32Radian( (Float32) generator.NextFloat64( -3, 3 ) ), (TEulerOrder) order, (TEulerFrame) frame );
                        TFloat32Quaternion q = e.ToQuaternion();
                        TFloat32EulerAngles back = TFloat32EulerAngles::FromQuaternion( q, (TEulerOrder) order, (TEulerFrame) frame );
                        Float64 error = TFloat64Quaternion( back.ToQuaternion() ).GetAngleTo( TFloat64Quaternion( q ) ).GetValue();
                        if ( error > worst )
                            worst = error;
                        if ( error > 1e-5 )
                            ++failures;
                    }
                }
            }
        }
        ASSERT_TRUE( 0 == failures );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EulerAngles: largest Float32 rotation error near gimbal lock in units of 1e-6 rad: " << worst * 1e6 );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "EulerAngles: " << failures << " near-lock round trips lost the rotation, seed " << seed );
        // the case of the review and YawPitchRoll at pitch 90 degrees
        TFloat32EulerAngles xyz( TFloat32Radian( 1.0f ), TFloat32Radian( 1.5707943f ), TFloat32Radian( -2.0f ), EULERORDER_XYZ, EULERFRAME_INTRINSIC );
        TFloat32Quaternion qx = xyz.ToQuaternion();
        ASSERT_TRUE( TFloat64Quaternion( qx.ToEuler( EULERORDER_XYZ, EULERFRAME_INTRINSIC ).ToQuaternion() ).GetAngleTo( TFloat64Quaternion( qx ) ).GetValue() < 1e-5 );
        TFloat32Quaternion lookUp = TFloat32EulerAngles::YawPitchRoll( TFloat32Radian( 0.3f ), TFloat32Radian( TFloat32Degree( 90.0f ) ), TFloat32Radian( 0.2f ) ).ToQuaternion();
        ASSERT_TRUE( TFloat64Quaternion( lookUp.ToEuler( EULERORDER_YXZ, EULERFRAME_INTRINSIC ).ToQuaternion() ).GetAngleTo( TFloat64Quaternion( lookUp ) ).GetValue() < 1e-5 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "EulerAngles: fixed point round trip against the Float64 reference" )
    try
    {
        const UInt64 seed = 0xF1E0;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<500; ++i )
        {
            Float64 angles[ 3 ];
            RandomAngles( generator, EULERORDER_ZYX, 0.1, angles );
            TFixed64EulerAngles e( TFixed64Radian( TFixed64( angles[ 0 ] ) ), TFixed64Radian( TFixed64( angles[ 1 ] ) ), TFixed64Radian( TFixed64( angles[ 2 ] ) ), EULERORDER_ZYX, EULERFRAME_INTRINSIC );
            TFixed64EulerAngles back = TFixed64EulerAngles::FromQuaternion( e.ToQuaternion(), EULERORDER_ZYX, EULERFRAME_INTRINSIC );
            if ( AngleDifference( back.first.GetValue().ToFloat64(), angles[ 0 ] ) > 1e-6 || AngleDifference( back.second.GetValue().ToFloat64(), angles[ 1 ] ) > 1e-6 ||
                 AngleDifference( back.third.GetValue().ToFloat64(), angles[ 2 ] ) > 1e-6 )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "EulerAngles: " << failures << " Q32.32 round trips off by more than 1e-6 rad, seed " << seed );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AxisAngle: identity default, quaternion and Rodrigues matrix, round trip" )
    try
    {
        TFloat64AxisAngle identity;
        ASSERT_TRUE( identity.axis == TFloat64DirectionVector3D::UnitX() && 0.0 == identity.angle.GetValue() && identity.ToQuaternion() == TFloat64Quaternion() );
        ASSERT_TRUE( TFloat64AxisAngle::FromQuaternion( TFloat64Quaternion() ) == identity );
        const UInt64 seed = 0xA4A6;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<1000; ++i )
        {
            TFloat64DirectionVector3D axis( TFloat64Vector3D( generator.NextFloat64( -1, 1 ), generator.NextFloat64( -1, 1 ), generator.NextFloat64( 0.1, 1 ) ) );
            TFloat64AxisAngle axisAngle( axis, TFloat64Radian( generator.NextFloat64( 0.001, g_pi - 0.001 ) ) );
            if ( !( Matrix3Distance( axisAngle.ToRotationMatrix3D(), axisAngle.ToQuaternion().ToRotationMatrix3D() ) < 1e-14 ) )
                ++failures;
            TFloat64AxisAngle back = TFloat64AxisAngle::FromQuaternion( axisAngle.ToQuaternion() );
            if ( !back.axis.Equals( axis ) || !( fabs( back.angle.GetValue() - axisAngle.angle.GetValue() ) < 1e-12 ) )
                ++failures;
            // a negative angle comes back as the positive angle about the opposite axis
            TFloat64AxisAngle negative( axis, TFloat64Radian( -axisAngle.angle.GetValue() ) );
            back = TFloat64AxisAngle::FromQuaternion( negative.ToQuaternion() );
            if ( !back.axis.Equals( TFloat64DirectionVector3D( -axis.AsVector() ) ) || !( fabs( back.angle.GetValue() - axisAngle.angle.GetValue() ) < 1e-12 ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AxisAngle: " << failures << " checks failed, seed " << seed );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "EulerAngles: realtime-safe operations run in a realtime context" )
    try
    {
        Float32 result = RealtimeEulerWork( TFloat32EulerAngles( TFloat32Radian( 0.1f ), TFloat32Radian( 0.2f ), TFloat32Radian( 0.3f ), EULERORDER_YZX, EULERFRAME_EXTRINSIC ) );
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( result ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
