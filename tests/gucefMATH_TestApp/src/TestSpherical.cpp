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

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_CTPOLARCOORDINATES_H
#include "gucefMATH_CTPolarCoordinates.h"
#define GUCEF_MATH_CTPOLARCOORDINATES_H
#endif /* GUCEF_MATH_CTPOLARCOORDINATES_H ? */

#ifndef GUCEF_MATH_CTSPHERICALCOORDINATES_H
#include "gucefMATH_CTSphericalCoordinates.h"
#define GUCEF_MATH_CTSPHERICALCOORDINATES_H
#endif /* GUCEF_MATH_CTSPHERICALCOORDINATES_H ? */

#ifndef GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H
#include "gucefMATH_SpatialAudioFunctions.h"
#define GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H
#endif /* GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H ? */

#ifndef GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H
#include "gucefMATH_SpatialAudioVectorFunctions.h"
#define GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H
#endif /* GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H ? */

#include "TestMathReference.h"
#include "TestSpherical.h"

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

/** The angle difference between a and b with the period of 2 pi taken out */
static Float64
AngleDistance( Float64 a, Float64 b )
{
    Float64 d = fmod( a - b, 2.0 * TestPi );
    if ( d > TestPi ) d -= 2.0 * TestPi;
    if ( d < -TestPi ) d += 2.0 * TestPi;
    return fabs( d );
}

/*-------------------------------------------------------------------------*/

/** Conversions and the vector Doppler ratio from a realtime context: a RealtimeSanitizer build reports any allocation */
static Float32
RealtimeSphericalWork( Float32 x ) GUCEF_MATH_REALTIME_CONTEXT
{
    TFloat32PolarCoordinates polar = TFloat32PolarCoordinates::FromCartesian( TFloat32Vector2D( x, 2.0f ) );
    TFloat32Vector2D cartesian = polar.ToCartesian();
    TFloat32SphericalCoordinates spherical = TFloat32SphericalCoordinates::FromFrameCoordinates( x, 1.0f, -2.0f );
    TFloat32Vector3D frame = spherical.ToFrameCoordinates();
    TFloat32Radian theta;
    TFloat32Radian phi;
    spherical.ToMathematical( theta, phi );
    TFloat32SphericalCoordinates back = TFloat32SphericalCoordinates::FromMathematical( spherical.radius, theta, phi );
    const Float32 ratio = DopplerRatio( TFloat32Point3D( 0, 0, 0 ), TFloat32Vector3D( 1, 0, 0 ), TFloat32Point3D( -5, 0, 0 ), TFloat32Vector3D( 2, 0, 0 ), 343.3f, 1.0f, 0.5f, 2.0f );
    return cartesian.x + frame.y + theta.GetValue() + back.radius + ratio;
}

/*-------------------------------------------------------------------------*/

/** The Doppler ratio from first principles in Float64: the speeds along the axis from the source to the listener */
static Float64
ReferenceDoppler( const TFloat64Point3D& listener, const TFloat64Vector3D& listenerVelocity, const TFloat64Point3D& source,
                  const TFloat64Vector3D& sourceVelocity, Float64 c, Float64 factor, Float64 minRatio, Float64 maxRatio )
{
    const Float64 sx = listener.x - source.x;
    const Float64 sy = listener.y - source.y;
    const Float64 sz = listener.z - source.z;
    const Float64 length = sqrt( sx * sx + sy * sy + sz * sz );
    if ( 0.0 == length || factor <= 0.0 || c <= 0.0 )
        return 1.0;
    const Float64 limit = c > 1.0 ? ( c - 1.0 ) / factor : c * 0.5 / factor;
    Float64 vls = ( listenerVelocity.x * sx + listenerVelocity.y * sy + listenerVelocity.z * sz ) / length;
    Float64 vss = ( sourceVelocity.x * sx + sourceVelocity.y * sy + sourceVelocity.z * sz ) / length;
    if ( vls > limit ) vls = limit;
    if ( vss > limit ) vss = limit;
    Float64 ratio = ( c - factor * vls ) / ( c - factor * vss );
    if ( ratio < minRatio ) ratio = minRatio;
    if ( ratio > maxRatio ) ratio = maxRatio;
    return ratio;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSphericalTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Spherical" );

    GUCEF_TESTFW_TESTCASE( "Spherical: polar coordinates of the axes, the zero vector and the opposite of a negative radius" )
    try
    {
        typedef TFloat64PolarCoordinates TPolar;
        TPolar east = TPolar::FromCartesian( TFloat64Vector2D( 2.0, 0.0 ) );
        TPolar north = TPolar::FromCartesian( TFloat64Vector2D( 0.0, 3.0 ) );
        TPolar west = TPolar::FromCartesian( TFloat64Vector2D( -4.0, 0.0 ) );
        TPolar south = TPolar::FromCartesian( TFloat64Vector2D( 0.0, -5.0 ) );
        ASSERT_TRUE( 2.0 == east.radius && 0.0 == east.angle.GetValue() );
        ASSERT_TRUE( 3.0 == north.radius && Near( north.angle.GetValue(), TestPi / 2.0, 1e-15 ) );
        ASSERT_TRUE( 4.0 == west.radius && Near( west.angle.GetValue(), TestPi, 1e-15 ) );
        ASSERT_TRUE( 5.0 == south.radius && Near( south.angle.GetValue(), -TestPi / 2.0, 1e-15 ) );
        // counter-clockwise from +X: ( 1, 1 ) is 45 degrees
        TPolar diagonal = TPolar::FromCartesian( TFloat64Vector2D( 1.0, 1.0 ) );
        ASSERT_TRUE( Near( diagonal.radius, sqrt( 2.0 ), 1e-15 ) && Near( diagonal.angle.GetValue(), TestPi / 4.0, 1e-15 ) );
        // the zero vector
        TPolar zero = TPolar::FromCartesian( TFloat64Vector2D( 0.0, 0.0 ) );
        ASSERT_TRUE( 0.0 == zero.radius && 0.0 == zero.angle.GetValue() );
        TPolar negativeZero = TPolar::FromCartesian( TFloat64Vector2D( -0.0, -0.0 ) );
        ASSERT_TRUE( 0.0 == negativeZero.radius && 0.0 == negativeZero.angle.GetValue() );
        // to Cartesian, also with a negative radius
        TFloat64Vector2D v = TPolar( 2.0, TFloat64Radian( TestPi / 2.0 ) ).ToCartesian();
        ASSERT_TRUE( Near( v.x, 0.0, 1e-15 ) && Near( v.y, 2.0, 1e-15 ) );
        TFloat64Vector2D opposite = TPolar( -2.0, TFloat64Radian( TestPi / 2.0 ) ).ToCartesian();
        ASSERT_TRUE( Near( opposite.x, 0.0, 1e-15 ) && Near( opposite.y, -2.0, 1e-15 ) );
        // the default value, equality and the sizes
        ASSERT_TRUE( TPolar() == TPolar( 0.0, TFloat64Radian( 0.0 ) ) && TPolar() != east );
        ASSERT_TRUE( sizeof( TFloat32PolarCoordinates ) == 2 * sizeof( Float32 ) && sizeof( TFloat64PolarCoordinates ) == 2 * sizeof( Float64 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Spherical: polar round trip and the reference formulas, including very small and very large vectors (seeded)" )
    try
    {
        const UInt64 seed = 0x501A;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i = 0; i < 5000; ++i )
        {
            const Float64 scale = pow( 10.0, generator.NextFloat64( -150.0, 150.0 ) );
            const Float64 x = generator.NextFloat64( -1.0, 1.0 ) * scale;
            const Float64 y = generator.NextFloat64( -1.0, 1.0 ) * scale;
            if ( 0.0 == x && 0.0 == y )
                continue;
            const TFloat64PolarCoordinates polar = TFloat64PolarCoordinates::FromCartesian( TFloat64Vector2D( x, y ) );
            // the reference works on the scaled values so that its squares stay finite
            const Float64 sx = x / scale;
            const Float64 sy = y / scale;
            const Float64 referenceRadius = sqrt( sx * sx + sy * sy ) * scale;
            const Float64 referenceAngle = atan2( sy, sx );
            const TFloat64Vector2D back = polar.ToCartesian();
            if ( fabs( polar.radius - referenceRadius ) > 4e-15 * referenceRadius || AngleDistance( polar.angle.GetValue(), referenceAngle ) > 1e-15 ||
                 fabs( back.x - x ) > 4e-15 * referenceRadius || fabs( back.y - y ) > 4e-15 * referenceRadius )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Spherical: " << failures << " polar checks failed, seed " << seed );

        // Float32
        const TFloat32PolarCoordinates p32 = TFloat32PolarCoordinates::FromCartesian( TFloat32Vector2D( 3.0f, -4.0f ) );
        ASSERT_TRUE( Near( p32.radius, 5.0, 1e-6 ) && Near( p32.angle.GetValue(), atan2( -4.0, 3.0 ), 1e-6 ) );
        TFloat32Vector2D b32 = p32.ToCartesian();
        ASSERT_TRUE( Near( b32.x, 3.0, 1e-5 ) && Near( b32.y, -4.0, 1e-5 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Spherical: the frame coordinates of the axes: azimuth 0 ahead, positive right; elevation positive up; the horizontal-less and zero cases" )
    try
    {
        typedef TFloat64SphericalCoordinates TSpherical;
        // ( x right, y up, z forward )
        TSpherical ahead = TSpherical::FromFrameCoordinates( 0.0, 0.0, 2.0 );
        TSpherical right = TSpherical::FromFrameCoordinates( 3.0, 0.0, 0.0 );
        TSpherical left = TSpherical::FromFrameCoordinates( -3.0, 0.0, 0.0 );
        TSpherical behind = TSpherical::FromFrameCoordinates( 0.0, 0.0, -4.0 );
        TSpherical up = TSpherical::FromFrameCoordinates( 0.0, 5.0, 0.0 );
        TSpherical down = TSpherical::FromFrameCoordinates( 0.0, -5.0, 0.0 );
        ASSERT_TRUE( 2.0 == ahead.radius && 0.0 == ahead.azimuth.GetValue() && 0.0 == ahead.elevation.GetValue() );
        ASSERT_TRUE( 3.0 == right.radius && Near( right.azimuth.GetValue(), TestPi / 2.0, 1e-15 ) && 0.0 == right.elevation.GetValue() );
        ASSERT_TRUE( Near( left.azimuth.GetValue(), -TestPi / 2.0, 1e-15 ) );
        ASSERT_TRUE( 4.0 == behind.radius && Near( behind.azimuth.GetValue(), TestPi, 1e-15 ) );
        ASSERT_TRUE( 5.0 == up.radius && 0.0 == up.azimuth.GetValue() && Near( up.elevation.GetValue(), TestPi / 2.0, 1e-15 ) );
        ASSERT_TRUE( Near( down.elevation.GetValue(), -TestPi / 2.0, 1e-15 ) && 0.0 == down.azimuth.GetValue() );
        // 45 degrees to the right and 45 degrees up
        TSpherical diagonal = TSpherical::FromFrameCoordinates( 1.0, 1.0, 1.0 );
        ASSERT_TRUE( Near( diagonal.radius, sqrt( 3.0 ), 1e-15 ) && Near( diagonal.azimuth.GetValue(), TestPi / 4.0, 1e-15 ) && Near( diagonal.elevation.GetValue(), atan( 1.0 / sqrt( 2.0 ) ), 1e-15 ) );
        // the zero vector, also with negative zeros, is radius 0 and angles 0
        TSpherical zero = TSpherical::FromFrameCoordinates( 0.0, 0.0, 0.0 );
        TSpherical negativeZero = TSpherical::FromFrameCoordinates( -0.0, -0.0, -0.0 );
        ASSERT_TRUE( 0.0 == zero.radius && 0.0 == zero.azimuth.GetValue() && 0.0 == zero.elevation.GetValue() );
        ASSERT_TRUE( 0.0 == negativeZero.radius && 0.0 == negativeZero.azimuth.GetValue() && 0.0 == negativeZero.elevation.GetValue() );
        // the vector overload
        TSpherical fromVector = TSpherical::FromFrameCoordinates( TFloat64Vector3D( 1.0, 2.0, 3.0 ) );
        TSpherical fromScalars = TSpherical::FromFrameCoordinates( 1.0, 2.0, 3.0 );
        ASSERT_TRUE( fromVector == fromScalars );
        // to frame coordinates
        TFloat64Vector3D v = TSpherical( 2.0, TFloat64Radian( TestPi / 2.0 ), TFloat64Radian( 0.0 ) ).ToFrameCoordinates();
        ASSERT_TRUE( Near( v.x, 2.0, 1e-15 ) && Near( v.y, 0.0, 1e-15 ) && Near( v.z, 0.0, 1e-15 ) );
        v = TSpherical( 2.0, TFloat64Radian( 0.0 ), TFloat64Radian( TestPi / 2.0 ) ).ToFrameCoordinates();
        ASSERT_TRUE( Near( v.x, 0.0, 1e-15 ) && Near( v.y, 2.0, 1e-15 ) && Near( v.z, 0.0, 1e-15 ) );
        ASSERT_TRUE( sizeof( TFloat32SphericalCoordinates ) == 3 * sizeof( Float32 ) && sizeof( TFloat64SphericalCoordinates ) == 3 * sizeof( Float64 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Spherical: round trip of the frame coordinates and the reference formulas, also at the extremes of the kind (seeded)" )
    try
    {
        const UInt64 seed = 0x5FE1;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        Float64 worst = 0.0;
        for ( UInt32 i = 0; i < 10000; ++i )
        {
            const Float64 scale = pow( 10.0, generator.NextFloat64( -140.0, 140.0 ) );
            const Float64 fx = generator.NextFloat64( -1.0, 1.0 );
            const Float64 fy = generator.NextFloat64( -1.0, 1.0 );
            const Float64 fz = generator.NextFloat64( -1.0, 1.0 );
            const Float64 x = fx * scale;
            const Float64 y = fy * scale;
            const Float64 z = fz * scale;
            const Float64 length = sqrt( fx * fx + fy * fy + fz * fz );
            if ( length < 1e-3 )
                continue;
            const TFloat64SphericalCoordinates s = TFloat64SphericalCoordinates::FromFrameCoordinates( x, y, z );
            // the reference: the scaled values, so that no square leaves the range
            const Float64 referenceAzimuth = atan2( fx, fz );
            const Float64 referenceElevation = asin( fy / length );
            const TFloat64Vector3D back = s.ToFrameCoordinates();
            const Float64 error = ( fabs( back.x - x ) + fabs( back.y - y ) + fabs( back.z - z ) ) / ( length * scale );
            if ( error > worst )
                worst = error;
            const bool azimuthOk = AngleDistance( s.azimuth.GetValue(), referenceAzimuth ) < 1e-12 || ( fabs( fx ) < 1e-9 && fabs( fz ) < 1e-9 );
            if ( fabs( s.radius - length * scale ) > 1e-14 * length * scale || !azimuthOk || fabs( s.elevation.GetValue() - referenceElevation ) > 1e-12 || error > 1e-14 )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        ASSERT_TRUE( worst < 1e-14 );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Spherical: " << failures << " spherical checks failed, seed " << seed << ", worst round trip error " << worst );

        // Float32: ordinary values and the extremes of the kind
        UInt32 failures32 = 0;
        for ( UInt32 i = 0; i < 2000; ++i )
        {
            const Float32 scale = (Float32) pow( 10.0, generator.NextFloat64( -30.0, 30.0 ) );
            const Float32 x = (Float32) generator.NextFloat64( -1.0, 1.0 ) * scale;
            const Float32 y = (Float32) generator.NextFloat64( -1.0, 1.0 ) * scale;
            const Float32 z = (Float32) generator.NextFloat64( -1.0, 1.0 ) * scale;
            const Float64 length = sqrt( ( (Float64) x * x + (Float64) y * y + (Float64) z * z ) / ( (Float64) scale * scale ) ) * scale;
            if ( length < 1e-3 * scale )
                continue;
            const TFloat32SphericalCoordinates s = TFloat32SphericalCoordinates::FromFrameCoordinates( x, y, z );
            const TFloat32Vector3D back = s.ToFrameCoordinates();
            if ( fabs( s.radius - length ) > 1e-5 * length || fabs( back.x - x ) > 1e-5 * length || fabs( back.y - y ) > 1e-5 * length || fabs( back.z - z ) > 1e-5 * length ||
                 AngleDistance( s.azimuth.GetValue(), atan2( (Float64) x, (Float64) z ) ) > 1e-5 )
                ++failures32;
        }
        ASSERT_TRUE( 0 == failures32 );
        // near the limits of Float32 the squares leave the range, the result does not
        const TFloat32SphericalCoordinates huge = TFloat32SphericalCoordinates::FromFrameCoordinates( 1.0e38f, 1.0e38f, 1.0e38f );
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( huge.radius ) && Near( huge.radius, 1.0e38 * sqrt( 3.0 ), 1.0e32 ) );
        ASSERT_TRUE( Near( huge.azimuth.GetValue(), TestPi / 4.0, 1e-6 ) && Near( huge.elevation.GetValue(), atan( 1.0 / sqrt( 2.0 ) ), 1e-6 ) );
        // the radius beyond the range of the kind is infinite, the angles are still right
        const TFloat32SphericalCoordinates beyond = TFloat32SphericalCoordinates::FromFrameCoordinates( 3.0e38f, 3.0e38f, 3.0e38f );
        ASSERT_TRUE( Near( beyond.azimuth.GetValue(), TestPi / 4.0, 1e-6 ) && Near( beyond.elevation.GetValue(), atan( 1.0 / sqrt( 2.0 ) ), 1e-6 ) );
        const TFloat32SphericalCoordinates tiny = TFloat32SphericalCoordinates::FromFrameCoordinates( 1.0e-38f, 1.0e-38f, 1.0e-38f );
        ASSERT_TRUE( tiny.radius > 0.0f && Near( tiny.azimuth.GetValue(), TestPi / 4.0, 1e-6 ) && Near( tiny.elevation.GetValue(), atan( 1.0 / sqrt( 2.0 ) ), 1e-6 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Spherical: the mathematical convention: polar angle from +Z, azimuth from +X toward +Y, round trip" )
    try
    {
        typedef TFloat64SphericalCoordinates TSpherical;
        TFloat64Radian theta;
        TFloat64Radian phi;
        TSpherical::FromFrameCoordinates( 0.0, 0.0, 2.0 ).ToMathematical( theta, phi );
        ASSERT_TRUE( Near( theta.GetValue(), 0.0, 1e-15 ) );
        TSpherical::FromFrameCoordinates( 0.0, 0.0, -2.0 ).ToMathematical( theta, phi );
        ASSERT_TRUE( Near( theta.GetValue(), TestPi, 1e-15 ) );
        TSpherical::FromFrameCoordinates( 2.0, 0.0, 0.0 ).ToMathematical( theta, phi );
        ASSERT_TRUE( Near( theta.GetValue(), TestPi / 2.0, 1e-15 ) && Near( phi.GetValue(), 0.0, 1e-15 ) );
        TSpherical::FromFrameCoordinates( 0.0, 2.0, 0.0 ).ToMathematical( theta, phi );
        ASSERT_TRUE( Near( theta.GetValue(), TestPi / 2.0, 1e-15 ) && Near( phi.GetValue(), TestPi / 2.0, 1e-15 ) );
        TSpherical::FromFrameCoordinates( -2.0, 0.0, 0.0 ).ToMathematical( theta, phi );
        ASSERT_TRUE( Near( theta.GetValue(), TestPi / 2.0, 1e-15 ) && Near( phi.GetValue(), TestPi, 1e-15 ) );
        // the zero position and the poles
        TSpherical().ToMathematical( theta, phi );
        ASSERT_TRUE( 0.0 == theta.GetValue() && 0.0 == phi.GetValue() );
        TSpherical::FromFrameCoordinates( 0.0, 0.0, 3.0 ).ToMathematical( theta, phi );
        ASSERT_TRUE( 0.0 == phi.GetValue() );

        // the reference formulas and the round trip
        const UInt64 seed = 0x3A7B;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i = 0; i < 5000; ++i )
        {
            const Float64 x = generator.NextFloat64( -3.0, 3.0 );
            const Float64 y = generator.NextFloat64( -3.0, 3.0 );
            const Float64 z = generator.NextFloat64( -3.0, 3.0 );
            const Float64 length = sqrt( x * x + y * y + z * z );
            if ( length < 1e-2 )
                continue;
            const TSpherical s = TSpherical::FromFrameCoordinates( x, y, z );
            s.ToMathematical( theta, phi );
            const TSpherical back = TSpherical::FromMathematical( s.radius, theta, phi );
            const TFloat64Vector3D v = back.ToFrameCoordinates();
            if ( fabs( theta.GetValue() - acos( z / length ) ) > 1e-12 || ( fabs( x ) + fabs( y ) > 1e-6 && AngleDistance( phi.GetValue(), atan2( y, x ) ) > 1e-12 ) ||
                 fabs( v.x - x ) > 1e-12 || fabs( v.y - y ) > 1e-12 || fabs( v.z - z ) > 1e-12 )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Spherical: " << failures << " mathematical convention checks failed, seed " << seed );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Spherical: text and DataNode forms of the polar and spherical coordinates" )
    try
    {
        CORE::CDataNodeSerializableSettings settings;
        TFloat64PolarCoordinates polar( 2.5, TFloat64Radian( -0.75 ) );
        CORE::CString polarText = polar.ToString();
        ASSERT_TRUE( polarText == "(2.5, -0.75)" );
        TFloat64PolarCoordinates polarRead;
        ASSERT_TRUE( polarRead.FromString( polarText ) && polarRead == polar );
        TFloat64PolarCoordinates polarKeep( 1.0, TFloat64Radian( 2.0 ) );
        ASSERT_FALSE( polarKeep.FromString( "(1.0)" ) );
        ASSERT_TRUE( polarKeep == TFloat64PolarCoordinates( 1.0, TFloat64Radian( 2.0 ) ) );
        CORE::CDataNode polarNode;
        TFloat64PolarCoordinates polarFromNode;
        ASSERT_TRUE( polar.Serialize( polarNode, settings ) && polarFromNode.Deserialize( polarNode, settings ) && polarFromNode == polar );

        TFloat64SphericalCoordinates spherical( 3.0, TFloat64Radian( 0.5 ), TFloat64Radian( -0.25 ) );
        CORE::CString sphericalText = spherical.ToString();
        ASSERT_TRUE( sphericalText == "(3, 0.5, -0.25)" );
        TFloat64SphericalCoordinates sphericalRead;
        ASSERT_TRUE( sphericalRead.FromString( sphericalText ) && sphericalRead == spherical );
        TFloat64SphericalCoordinates sphericalKeep( 1.0, TFloat64Radian( 2.0 ), TFloat64Radian( 3.0 ) );
        ASSERT_FALSE( sphericalKeep.FromString( "(1.0, 2.0)" ) );
        ASSERT_TRUE( sphericalKeep == TFloat64SphericalCoordinates( 1.0, TFloat64Radian( 2.0 ), TFloat64Radian( 3.0 ) ) );
        CORE::CDataNode sphericalNode;
        TFloat64SphericalCoordinates sphericalFromNode;
        ASSERT_TRUE( spherical.Serialize( sphericalNode, settings ) && sphericalFromNode.Deserialize( sphericalNode, settings ) && sphericalFromNode == spherical );
        ASSERT_TRUE( GUCEF_NULL != sphericalNode.GetAttribute( "azimuth" ) && GUCEF_NULL != sphericalNode.GetAttribute( "elevation" ) && GUCEF_NULL != sphericalNode.GetAttribute( "radius" ) );
        CORE::CDataNode incomplete;
        ASSERT_TRUE( incomplete.SetAttribute( "radius", CORE::CVariant( 1.0 ) ) );
        ASSERT_FALSE( sphericalKeep.Deserialize( incomplete, settings ) );
        ASSERT_TRUE( sphericalKeep == TFloat64SphericalCoordinates( 1.0, TFloat64Radian( 2.0 ), TFloat64Radian( 3.0 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Spherical: the vector Doppler ratio reproduces the scalar results (OpenAL 1.1, 343.3 m/s)" )
    try
    {
        // acceptance 4: the geometry of the scalar tests of TestSpatialAudioFunctions on the x axis, the listener at the origin
        const Float64 c = 343.3;
        const Float64 v = 34.33;
        const TFloat64Point3D listener( 0.0, 0.0, 0.0 );
        const TFloat64Point3D source( -100.0, 0.0, 0.0 );
        const TFloat64Vector3D still( 0.0, 0.0, 0.0 );
        const TFloat64Vector3D towardListener( v, 0.0, 0.0 );
        const TFloat64Vector3D awayFromListener( -v, 0.0, 0.0 );
        // the source approaches: ( c ) / ( c - v )
        ASSERT_TRUE( Near( DopplerRatio( listener, still, source, towardListener, c, 1.0, 0.0, 100.0 ), 343.3 / 308.97, 1e-5 ) );
        // the listener approaches: ( c + v ) / c
        ASSERT_TRUE( Near( DopplerRatio( listener, awayFromListener, source, still, c, 1.0, 0.0, 100.0 ), ( c + v ) / c, 1e-5 ) );
        // both approach
        ASSERT_TRUE( Near( DopplerRatio( listener, awayFromListener, source, towardListener, c, 1.0, 0.0, 100.0 ), ( c + v ) / ( c - v ), 1e-5 ) );
        // the source recedes
        ASSERT_TRUE( Near( DopplerRatio( listener, still, source, awayFromListener, c, 1.0, 0.0, 100.0 ), c / ( c + v ), 1e-5 ) );
        // the Doppler factor scales the speeds
        ASSERT_TRUE( Near( DopplerRatio( listener, still, source, towardListener, c, 2.0, 0.0, 100.0 ), c / ( c - 2.0 * v ), 1e-5 ) );
        // a Doppler factor or speed of sound of zero, and nothing moving: 1
        ASSERT_TRUE( 1.0 == DopplerRatio( listener, TFloat64Vector3D( 10, 0, 0 ), source, TFloat64Vector3D( 50, 0, 0 ), c, 0.0, 0.0, 100.0 ) );
        ASSERT_TRUE( 1.0 == DopplerRatio( listener, TFloat64Vector3D( 10, 0, 0 ), source, TFloat64Vector3D( 50, 0, 0 ), 0.0, 1.0, 0.0, 100.0 ) );
        ASSERT_TRUE( 1.0 == DopplerRatio( listener, still, source, still, c, 1.0, 0.0, 100.0 ) );
        // the limits
        ASSERT_TRUE( 1.1 == DopplerRatio( listener, still, source, TFloat64Vector3D( 200, 0, 0 ), c, 1.0, 0.5, 1.1 ) );
        ASSERT_TRUE( 0.9 == DopplerRatio( listener, still, source, TFloat64Vector3D( -200, 0, 0 ), c, 1.0, 0.9, 2.0 ) );
        // a source at and beyond the speed of sound stays finite and inside the limits
        const Float64 extreme = DopplerRatio( listener, still, source, TFloat64Vector3D( 1.0e9, 0, 0 ), c, 1.0, 0.0, 1000.0 );
        ASSERT_TRUE( CTNumericTraits< Float64 >::IsFinite( extreme ) && extreme > 1.0 && extreme <= 1000.0 );
        ASSERT_TRUE( DopplerRatio( listener, still, source, TFloat64Vector3D( c, 0, 0 ), c, 1.0, 0.0, 1000.0 ) == extreme );
        // the result is the scalar function of the speeds along the axis, exactly, for geometry on an axis
        ASSERT_TRUE( DopplerRatio( listener, awayFromListener, source, towardListener, c, 1.0, 0.0, 100.0 ) == DopplerRatio( -v, v, c, 1.0, 0.0, 100.0 ) );
        // motion across the axis does not shift the pitch
        ASSERT_TRUE( 1.0 == DopplerRatio( listener, TFloat64Vector3D( 0, 30, 0 ), source, TFloat64Vector3D( 0, 0, -40 ), c, 1.0, 0.0, 100.0 ) );
        // zero distance gives 1, whatever the velocities
        ASSERT_TRUE( 1.0 == DopplerRatio( listener, towardListener, listener, awayFromListener, c, 1.0, 0.5, 2.0 ) );
        ASSERT_TRUE( 1.0 == DopplerRatio( source, towardListener, source, still, c, 1.0, 0.5, 2.0 ) );
        // not finite positions give 1
        const Float64 nan = CTNumericTraits< Float64 >::Max() * 2.0 - CTNumericTraits< Float64 >::Max() * 2.0;
        ASSERT_TRUE( 1.0 == DopplerRatio( TFloat64Point3D( nan, 0, 0 ), towardListener, source, still, c, 1.0, 0.5, 2.0 ) );
        // the same on another axis and with the listener behind the source: SL changes sign, the result does not depend on the axis
        ASSERT_TRUE( Near( DopplerRatio( TFloat64Point3D( 0, 100, 0 ), still, TFloat64Point3D( 0, 0, 0 ), TFloat64Vector3D( 0, v, 0 ), c, 1.0, 0.0, 100.0 ), 343.3 / 308.97, 1e-5 ) );
        // Float32
        ASSERT_TRUE( Near( DopplerRatio( TFloat32Point3D( 0, 0, 0 ), TFloat32Vector3D( 0, 0, 0 ), TFloat32Point3D( -100, 0, 0 ), TFloat32Vector3D( 34.33f, 0, 0 ), 343.3f, 1.0f, 0.0f, 100.0f ), 343.3 / 308.97, 1e-4 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Spherical: the vector Doppler ratio against the speeds along the axis from first principles (seeded geometry)" )
    try
    {
        const UInt64 seed = 0xD0E1;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i = 0; i < 20000; ++i )
        {
            const TFloat64Point3D listener( generator.NextFloat64( -100, 100 ), generator.NextFloat64( -100, 100 ), generator.NextFloat64( -100, 100 ) );
            const TFloat64Point3D source( generator.NextFloat64( -100, 100 ), generator.NextFloat64( -100, 100 ), generator.NextFloat64( -100, 100 ) );
            const TFloat64Vector3D listenerVelocity( generator.NextFloat64( -400, 400 ), generator.NextFloat64( -400, 400 ), generator.NextFloat64( -400, 400 ) );
            const TFloat64Vector3D sourceVelocity( generator.NextFloat64( -400, 400 ), generator.NextFloat64( -400, 400 ), generator.NextFloat64( -400, 400 ) );
            const Float64 factor = generator.NextFloat64( 0.0, 3.0 );
            const Float64 expected = ReferenceDoppler( listener, listenerVelocity, source, sourceVelocity, 343.3, factor, 0.25, 4.0 );
            const Float64 actual = DopplerRatio( listener, listenerVelocity, source, sourceVelocity, 343.3, factor, 0.25, 4.0 );
            if ( fabs( actual - expected ) > 1e-12 * expected )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Spherical: " << failures << " Doppler checks failed, seed " << seed );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Spherical: realtime-safe functions run in a realtime context" )
    try
    {
        ASSERT_TRUE( CTNumericTraits< Float32 >::IsFinite( RealtimeSphericalWork( 1.5f ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
