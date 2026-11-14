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

#include <math.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H
#include "gucefMATH_SpatialAudioFunctions.h"
#define GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H
#endif /* GUCEF_MATH_SPATIALAUDIOFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTDEGREE_H
#include "gucefMATH_CTDegree.h"
#define GUCEF_MATH_CTDEGREE_H
#endif /* GUCEF_MATH_CTDEGREE_H ? */

#ifndef GUCEF_MATH_CTDISTANCEATTENUATION_H
#include "gucefMATH_CTDistanceAttenuation.h"
#define GUCEF_MATH_CTDISTANCEATTENUATION_H
#endif /* GUCEF_MATH_CTDISTANCEATTENUATION_H ? */

#ifndef GUCEF_MATH_CTDIRECTIVITYPATTERN_H
#include "gucefMATH_CTDirectivityPattern.h"
#define GUCEF_MATH_CTDIRECTIVITYPATTERN_H
#endif /* GUCEF_MATH_CTDIRECTIVITYPATTERN_H ? */

#ifndef GUCEF_MATH_CTPIECEWISELINEARCURVE_H
#include "gucefMATH_CTPiecewiseLinearCurve.h"
#define GUCEF_MATH_CTPIECEWISELINEARCURVE_H
#endif /* GUCEF_MATH_CTPIECEWISELINEARCURVE_H ? */

#ifndef GUCEF_MATH_CXOSHIRO256STARSTAR_H
#include "gucefMATH_CXoshiro256StarStar.h"
#define GUCEF_MATH_CXOSHIRO256STARSTAR_H
#endif /* GUCEF_MATH_CXOSHIRO256STARSTAR_H ? */

#include "TestSpatialAudioFunctions.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static const MATH::Float64 Sqrt2 = 1.41421356237309504880;

/*-------------------------------------------------------------------------*/

static bool
Near( MATH::Float64 a, MATH::Float64 b, MATH::Float64 tolerance )
{
    return fabs( a - b ) <= tolerance;
}

/*-------------------------------------------------------------------------*/

static void
PerformCurveTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SpatialAudioFunctions: piecewise linear curve interpolates, clamps and keeps its points sorted" )
    {
        MATH::CTPiecewiseLinearCurve< MATH::Float64, 4 > curve;
        ASSERT_TRUE( 0 == curve.GetPointCount() );
        ASSERT_TRUE( 0.0 == curve.Evaluate( 3.0 ) );

        ASSERT_TRUE( curve.AddPoint( 1.0, -6.0 ) );
        ASSERT_TRUE( -6.0 == curve.Evaluate( -10.0 ) && -6.0 == curve.Evaluate( 10.0 ) );   // a single point is a constant

        ASSERT_TRUE( curve.AddPoint( 2.0, -20.0 ) );
        ASSERT_TRUE( curve.AddPoint( 0.0, 0.0 ) );                                           // inserted in front
        ASSERT_TRUE( 3 == curve.GetPointCount() );
        ASSERT_TRUE( 0.0 == curve.GetPointX( 0 ) && 1.0 == curve.GetPointX( 1 ) && 2.0 == curve.GetPointX( 2 ) );
        ASSERT_TRUE( 0.0 == curve.GetPointY( 0 ) && -6.0 == curve.GetPointY( 1 ) && -20.0 == curve.GetPointY( 2 ) );
        ASSERT_TRUE( Near( curve.Evaluate( 0.5 ), -3.0, 1e-12 ) );
        ASSERT_TRUE( Near( curve.Evaluate( 1.5 ), -13.0, 1e-12 ) );
        ASSERT_TRUE( Near( curve.Evaluate( 1.0 ), -6.0, 1e-12 ) );
        ASSERT_TRUE( 0.0 == curve.Evaluate( -5.0 ) );                                        // clamped at the ends
        ASSERT_TRUE( -20.0 == curve.Evaluate( 5.0 ) );
        ASSERT_TRUE( 0.0 == curve.GetPointX( 7 ) && 0.0 == curve.GetPointY( 7 ) );

        // The same x replaces, also when the curve is full
        ASSERT_TRUE( curve.AddPoint( 1.0, -10.0 ) );
        ASSERT_TRUE( 3 == curve.GetPointCount() && Near( curve.Evaluate( 1.0 ), -10.0, 1e-12 ) );
        ASSERT_TRUE( curve.AddPoint( 3.0, -30.0 ) );
        ASSERT_FALSE( curve.AddPoint( 4.0, -40.0 ) );                                        // full
        ASSERT_TRUE( 4 == curve.GetPointCount() );
        ASSERT_TRUE( curve.AddPoint( 3.0, -35.0 ) );
        ASSERT_TRUE( Near( curve.Evaluate( 3.0 ), -35.0, 1e-12 ) );

        volatile MATH::Float64 zero = 0.0;
        ASSERT_FALSE( curve.AddPoint( zero / zero, 1.0 ) );                                   // NaN is refused
        ASSERT_FALSE( curve.AddPoint( 1.0, zero / zero ) );

        MATH::CTPiecewiseLinearCurve< MATH::Float64, 4 > copy = curve;
        ASSERT_TRUE( copy == curve );
        curve.Clear();
        ASSERT_TRUE( 0 == curve.GetPointCount() && copy != curve );
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformDistanceTests( void )
{GUCEF_TRACE;

    const MATH::Float64 dMin = 2.0;
    const MATH::Float64 dMax = 50.0;

    GUCEF_TESTFW_TESTCASE( "SpatialAudioFunctions: distance models at dMin, 2 dMin, dMax, beyond dMax and below dMin" )
    {
        const MATH::Float64 rolloff = 1.0;
        MATH::CTDistanceAttenuation< MATH::Float64 > none( MATH::DISTANCEMODEL_NONE, dMin, dMax, rolloff );
        MATH::CTDistanceAttenuation< MATH::Float64 > inverse( MATH::DISTANCEMODEL_INVERSE_CLAMPED, dMin, dMax, rolloff );
        MATH::CTDistanceAttenuation< MATH::Float64 > linear( MATH::DISTANCEMODEL_LINEAR_CLAMPED, dMin, dMax, rolloff );
        MATH::CTDistanceAttenuation< MATH::Float64 > exponent( MATH::DISTANCEMODEL_EXPONENT_CLAMPED, dMin, dMax, rolloff );

        const MATH::Float64 distances[ 5 ] = { 1.0, dMin, 2.0 * dMin, dMax, 80.0 };
        for ( int i = 0; i < 5; ++i )
        {
            const MATH::Float64 d = distances[ i ];
            const MATH::Float64 clamped = d < dMin ? dMin : ( d > dMax ? dMax : d );
            ASSERT_TRUE( 1.0 == none.Evaluate( d ) );
            ASSERT_TRUE( Near( inverse.Evaluate( d ), dMin / ( dMin + rolloff * ( clamped - dMin ) ), 1e-6 ) );
            ASSERT_TRUE( Near( linear.Evaluate( d ), 1.0 - rolloff * ( clamped - dMin ) / ( dMax - dMin ), 1e-6 ) );
            ASSERT_TRUE( Near( exponent.Evaluate( d ), pow( clamped / dMin, -rolloff ), 1e-6 ) );
        }

        // The named values
        ASSERT_TRUE( Near( inverse.Evaluate( dMin ), 1.0, 1e-12 ) );
        ASSERT_TRUE( Near( inverse.Evaluate( 2.0 * dMin ), 0.5, 1e-12 ) );
        ASSERT_TRUE( Near( linear.Evaluate( dMax ), 0.0, 1e-12 ) && Near( linear.Evaluate( 80.0 ), 0.0, 1e-12 ) );
        ASSERT_TRUE( Near( exponent.Evaluate( 2.0 * dMin ), 0.5, 1e-12 ) );

        // A larger rolloff than 1, and the unclamped forms: they continue beyond dMax and below dMin
        MATH::CTDistanceAttenuation< MATH::Float64 > steep( MATH::DISTANCEMODEL_EXPONENT_CLAMPED, dMin, dMax, 2.5 );
        ASSERT_TRUE( Near( steep.Evaluate( 8.0 ), pow( 4.0, -2.5 ), 1e-12 ) );
        MATH::CTDistanceAttenuation< MATH::Float64 > unclampedInverse( MATH::DISTANCEMODEL_INVERSE, dMin, dMax, 1.0 );
        ASSERT_TRUE( Near( unclampedInverse.Evaluate( 100.0 ), dMin / ( dMin + 98.0 ), 1e-12 ) );
        ASSERT_TRUE( unclampedInverse.Evaluate( 1.0 ) > 1.0 );
        MATH::CTDistanceAttenuation< MATH::Float64 > unclampedLinear( MATH::DISTANCEMODEL_LINEAR, dMin, dMax, 1.0 );
        ASSERT_TRUE( 0.0 == unclampedLinear.Evaluate( 100.0 ) );          // never negative
        MATH::CTDistanceAttenuation< MATH::Float64 > unclampedExponent( MATH::DISTANCEMODEL_EXPONENT, dMin, dMax, 1.0 );
        ASSERT_TRUE( Near( unclampedExponent.Evaluate( 100.0 ), 0.02, 1e-12 ) );

        // Monotonic non increasing for the clamped models
        for ( int m = 0; m < 3; ++m )
        {
            MATH::CTDistanceAttenuation< MATH::Float64 > model( (MATH::TDistanceModel) ( MATH::DISTANCEMODEL_INVERSE_CLAMPED + 2 * m ), dMin, dMax, 1.3 );
            MATH::Float64 previous = model.Evaluate( 0.0 );
            bool monotonic = true;
            for ( int i = 1; i <= 1000; ++i )
            {
                const MATH::Float64 value = model.Evaluate( i * 0.1 );
                if ( value > previous + 1e-15 ) monotonic = false;
                previous = value;
            }
            ASSERT_TRUE( monotonic );
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SpatialAudioFunctions: degenerate attenuation settings give finite gains" )
    {
        const MATH::TDistanceModel models[ 8 ] = { MATH::DISTANCEMODEL_NONE, MATH::DISTANCEMODEL_INVERSE, MATH::DISTANCEMODEL_INVERSE_CLAMPED, MATH::DISTANCEMODEL_LINEAR,
                                                   MATH::DISTANCEMODEL_LINEAR_CLAMPED, MATH::DISTANCEMODEL_EXPONENT, MATH::DISTANCEMODEL_EXPONENT_CLAMPED, MATH::DISTANCEMODEL_CURVE };
        const MATH::Float64 settings[ 5 ][ 3 ] = { { 0.0, 0.0, 1.0 }, { 5.0, 5.0, 1.0 }, { 5.0, 1.0, 1.0 }, { -1.0, 10.0, 2.0 }, { 1.0, 10.0, 100.0 } };
        bool finite = true;
        for ( int m = 0; m < 8; ++m )
        {
            for ( int s = 0; s < 5; ++s )
            {
                MATH::CTDistanceAttenuation< MATH::Float64 > model( models[ m ], settings[ s ][ 0 ], settings[ s ][ 1 ], settings[ s ][ 2 ] );
                const MATH::Float64 distances[ 6 ] = { -3.0, 0.0, 0.5, 1.0, 5.0, 1000.0 };
                for ( int d = 0; d < 6; ++d )
                {
                    const MATH::Float64 gain = model.Evaluate( distances[ d ] );
                    if ( !( gain >= 0.0 ) || gain > 1e30 ) finite = false;
                }
            }
        }
        ASSERT_TRUE( finite );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SpatialAudioFunctions: the curve model interpolates in decibels over the normalised distance" )
    {
        MATH::CTDistanceAttenuation< MATH::Float64 > curved( MATH::DISTANCEMODEL_CURVE, dMin, dMax, 1.0 );
        ASSERT_TRUE( Near( curved.Evaluate( 20.0 ), 1.0, 1e-12 ) );                          // no points: 0 dB everywhere
        curved.GetCurve().AddPoint( 0.0, 0.0 );
        curved.GetCurve().AddPoint( 0.5, -6.0 );
        curved.GetCurve().AddPoint( 1.0, -40.0 );
        ASSERT_TRUE( Near( curved.Evaluate( dMin ), 1.0, 1e-12 ) );
        ASSERT_TRUE( Near( curved.Evaluate( 0.5 ), 1.0, 1e-12 ) );                           // below dMin: clamped
        ASSERT_TRUE( Near( curved.Evaluate( dMin + 0.25 * ( dMax - dMin ) ), pow( 10.0, -3.0 / 20.0 ), 1e-9 ) );
        ASSERT_TRUE( Near( curved.Evaluate( dMin + 0.5 * ( dMax - dMin ) ), pow( 10.0, -6.0 / 20.0 ), 1e-9 ) );
        ASSERT_TRUE( Near( curved.Evaluate( dMin + 0.75 * ( dMax - dMin ) ), pow( 10.0, -23.0 / 20.0 ), 1e-9 ) );
        ASSERT_TRUE( Near( curved.Evaluate( dMax ), pow( 10.0, -2.0 ), 1e-9 ) );
        ASSERT_TRUE( Near( curved.Evaluate( 500.0 ), pow( 10.0, -2.0 ), 1e-9 ) );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SpatialAudioFunctions: the distance low pass interpolates the cutoff logarithmically" )
    {
        MATH::CTDistanceAttenuation< MATH::Float64 > model( MATH::DISTANCEMODEL_INVERSE_CLAMPED, dMin, dMax, 1.0 );
        ASSERT_TRUE( Near( model.EvaluateLogLerpCutoff( dMin, 20000.0, 1500.0 ), 20000.0, 1e-6 ) );
        ASSERT_TRUE( Near( model.EvaluateLogLerpCutoff( 0.0, 20000.0, 1500.0 ), 20000.0, 1e-6 ) );
        ASSERT_TRUE( Near( model.EvaluateLogLerpCutoff( dMax, 20000.0, 1500.0 ), 1500.0, 1e-6 ) );
        ASSERT_TRUE( Near( model.EvaluateLogLerpCutoff( 1000.0, 20000.0, 1500.0 ), 1500.0, 1e-6 ) );
        ASSERT_TRUE( Near( model.EvaluateLogLerpCutoff( 0.5 * ( dMin + dMax ), 20000.0, 1500.0 ), 5477.2256, 1e-3 ) );
        ASSERT_TRUE( 20000.0 == model.EvaluateLogLerpCutoff( 30.0, 20000.0, 0.0 ) );          // far cutoff 0: low pass off
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SpatialAudioFunctions: the attenuation settings round trip through a data node (Float32 and Float64)" )
    {
        MATH::CTDistanceAttenuation< MATH::Float64 > source( MATH::DISTANCEMODEL_CURVE, 1.5, 75.25, 0.75 );
        source.GetCurve().AddPoint( 0.0, 0.0 );
        source.GetCurve().AddPoint( 0.3, -4.5 );
        source.GetCurve().AddPoint( 1.0, -60.0 );
        CORE::CDataNode node( "Attenuation" );
        CORE::CDataNodeSerializableSettings settings;
        ASSERT_TRUE( source.Serialize( node, settings ) );

        MATH::CTDistanceAttenuation< MATH::Float64 > loaded;
        ASSERT_TRUE( loaded.Deserialize( node, settings ) );
        ASSERT_TRUE( loaded == source );
        ASSERT_TRUE( loaded.Evaluate( 20.0 ) == source.Evaluate( 20.0 ) );

        // Serializing twice does not duplicate the curve
        ASSERT_TRUE( source.Serialize( node, settings ) );
        MATH::CTDistanceAttenuation< MATH::Float64 > again;
        ASSERT_TRUE( again.Deserialize( node, settings ) && again == source );

        // Every model name round trips
        for ( int m = (int) MATH::DISTANCEMODEL_NONE; m <= (int) MATH::DISTANCEMODEL_CURVE; ++m )
        {
            MATH::CTDistanceAttenuation< MATH::Float64 > model( (MATH::TDistanceModel) m, 1.0, 10.0, 2.0 );
            CORE::CDataNode modelNode( "Attenuation" );
            MATH::CTDistanceAttenuation< MATH::Float64 > back;
            ASSERT_TRUE( model.Serialize( modelNode, settings ) && back.Deserialize( modelNode, settings ) && back == model );
        }

        // The Float32 instantiation
        MATH::CTDistanceAttenuation< MATH::Float32 > narrow( MATH::DISTANCEMODEL_LINEAR_CLAMPED, 1.25f, 40.5f, 1.5f );
        narrow.GetCurve().AddPoint( 0.25f, -3.0f );
        CORE::CDataNode narrowNode( "Attenuation" );
        MATH::CTDistanceAttenuation< MATH::Float32 > narrowLoaded;
        ASSERT_TRUE( narrow.Serialize( narrowNode, settings ) && narrowLoaded.Deserialize( narrowNode, settings ) && narrowLoaded == narrow );

        // A node with a missing attribute or an unknown model name is refused and the object is left unchanged
        CORE::CDataNode broken( "Attenuation" );
        broken.SetAttribute( "model", CORE::CVariant( CORE::CString( "inverse" ) ) );
        MATH::CTDistanceAttenuation< MATH::Float64 > untouched( MATH::DISTANCEMODEL_LINEAR, 3.0, 30.0, 3.0 );
        const MATH::CTDistanceAttenuation< MATH::Float64 > expected = untouched;
        ASSERT_FALSE( untouched.Deserialize( broken, settings ) );
        ASSERT_TRUE( untouched == expected );
        CORE::CDataNode unknownModel( "Attenuation" );
        source.Serialize( unknownModel, settings );
        unknownModel.SetAttribute( "model", CORE::CVariant( CORE::CString( "bogus" ) ) );
        ASSERT_FALSE( untouched.Deserialize( unknownModel, settings ) );
        ASSERT_TRUE( untouched == expected );
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformConeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SpatialAudioFunctions: cone gain and interpolation factor at inner/2, midway, outer/2 and beyond" )
    {
        const MATH::CTRadian< MATH::Float64 > inner = MATH::CTRadian< MATH::Float64 >( MATH::CTDegree< MATH::Float64 >( 60.0 ) );
        const MATH::CTRadian< MATH::Float64 > outer = MATH::CTRadian< MATH::Float64 >( MATH::CTDegree< MATH::Float64 >( 180.0 ) );
        const MATH::Float64 outerGain = 0.25;
        const MATH::Float64 toRad = 3.14159265358979323846 / 180.0;
        // half angles: inner 30 degrees, outer 90 degrees
        ASSERT_TRUE( Near( MATH::ConeGain( 1.0, inner, outer, outerGain ), 1.0, 1e-12 ) );                 // on axis
        ASSERT_TRUE( Near( MATH::ConeGain( cos( 30.0 * toRad ), inner, outer, outerGain ), 1.0, 1e-9 ) );  // at inner/2
        ASSERT_TRUE( Near( MATH::ConeGain( cos( 60.0 * toRad ), inner, outer, outerGain ), 0.625, 1e-9 ) ); // midway: 1 + ( 0.25 - 1 ) * 0.5
        ASSERT_TRUE( Near( MATH::ConeGain( cos( 90.0 * toRad ), inner, outer, outerGain ), outerGain, 1e-9 ) ); // at outer/2
        ASSERT_TRUE( Near( MATH::ConeGain( -1.0, inner, outer, outerGain ), outerGain, 1e-12 ) );          // beyond
        ASSERT_TRUE( Near( MATH::ConeInterpolationFactor( cos( 60.0 * toRad ), inner, outer ), 0.5, 1e-9 ) );
        ASSERT_TRUE( Near( MATH::ConeInterpolationFactor( cos( 45.0 * toRad ), inner, outer ), 0.25, 1e-9 ) );
        ASSERT_TRUE( 0.0 == MATH::ConeInterpolationFactor( 2.0, inner, outer ) );                          // clamped input
        // cutoff interpolated logarithmically with the factor
        ASSERT_TRUE( Near( MATH::LogLerp( 20000.0, 1500.0, MATH::ConeInterpolationFactor( cos( 60.0 * toRad ), inner, outer ) ), 5477.2256, 1e-3 ) );
        // outer not larger than inner: a step
        ASSERT_TRUE( 1.0 == MATH::ConeGain( cos( 31.0 * toRad ), inner, inner, 0.0 ) + 1.0 );
        ASSERT_TRUE( Near( MATH::ConeGain( cos( 29.0 * toRad ), inner, inner, 0.0 ), 1.0, 1e-12 ) );
        // Float32
        ASSERT_TRUE( Near( MATH::ConeGain( 0.5f, MATH::CTRadian< MATH::Float32 >( 1.0471976f ), MATH::CTRadian< MATH::Float32 >( 3.1415927f ), 0.25f ), 0.625, 1e-4 ) );
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformPanDopplerTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SpatialAudioFunctions: constant power pan, sum of squares is scale squared for 201 pans" )
    {
        MATH::Float64 left = 0.0, right = 0.0;
        MATH::ConstantPowerPan( 0.0, Sqrt2, left, right );
        ASSERT_TRUE( Near( left, 1.0, 1e-6 ) && Near( right, 1.0, 1e-6 ) );
        MATH::ConstantPowerPan( 1.0, Sqrt2, left, right );
        ASSERT_TRUE( Near( left, 0.0, 1e-6 ) && Near( right, Sqrt2, 1e-6 ) );
        MATH::ConstantPowerPan( -1.0, Sqrt2, left, right );
        ASSERT_TRUE( Near( left, Sqrt2, 1e-6 ) && Near( right, 0.0, 1e-6 ) );
        MATH::ConstantPowerPan( 7.0, Sqrt2, left, right );                       // clamped
        ASSERT_TRUE( Near( left, 0.0, 1e-6 ) && Near( right, Sqrt2, 1e-6 ) );

        bool powerOk = true;
        for ( int i = 0; i <= 200; ++i )
        {
            const MATH::Float64 pan = -1.0 + 0.01 * i;
            MATH::ConstantPowerPan( pan, Sqrt2, left, right );
            if ( !Near( left * left + right * right, 2.0, 1e-6 ) ) powerOk = false;
        }
        ASSERT_TRUE( powerOk );

        // A scale of 1: unit power, and the Float32 form
        MATH::ConstantPowerPan( 0.3, 1.0, left, right );
        ASSERT_TRUE( Near( left * left + right * right, 1.0, 1e-12 ) );
        MATH::Float32 leftNarrow = 0.0f, rightNarrow = 0.0f;
        MATH::ConstantPowerPan( 0.0f, 1.41421356f, leftNarrow, rightNarrow );
        ASSERT_TRUE( Near( leftNarrow, 1.0, 1e-6 ) && Near( rightNarrow, 1.0, 1e-6 ) );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SpatialAudioFunctions: Doppler ratio, OpenAL 1.1" )
    {
        const MATH::Float64 c = 343.3;
        const MATH::Float64 v = 34.33;
        // source approaching the listener at v: ( c - 0 ) / ( c - v )
        ASSERT_TRUE( Near( MATH::DopplerRatio( 0.0, v, c, 1.0, 0.0, 100.0 ), 343.3 / 308.97, 1e-5 ) );
        // listener approaching the source at v moves against SL: ( c + v ) / c
        ASSERT_TRUE( Near( MATH::DopplerRatio( -v, 0.0, c, 1.0, 0.0, 100.0 ), ( c + v ) / c, 1e-5 ) );
        // both, and a receding source
        ASSERT_TRUE( Near( MATH::DopplerRatio( -v, v, c, 1.0, 0.0, 100.0 ), ( c + v ) / ( c - v ), 1e-5 ) );
        ASSERT_TRUE( Near( MATH::DopplerRatio( 0.0, -v, c, 1.0, 0.0, 100.0 ), c / ( c + v ), 1e-5 ) );
        // the Doppler factor scales the speeds
        ASSERT_TRUE( Near( MATH::DopplerRatio( 0.0, v, c, 2.0, 0.0, 100.0 ), c / ( c - 2.0 * v ), 1e-5 ) );
        // a factor of 0 disables the effect, so does a speed of sound that is not positive
        ASSERT_TRUE( 1.0 == MATH::DopplerRatio( 10.0, 50.0, c, 0.0, 0.0, 100.0 ) );
        ASSERT_TRUE( 1.0 == MATH::DopplerRatio( 10.0, 50.0, 0.0, 1.0, 0.0, 100.0 ) );
        // no relative speed: 1
        ASSERT_TRUE( 1.0 == MATH::DopplerRatio( 0.0, 0.0, c, 1.0, 0.0, 100.0 ) );
        // limits
        ASSERT_TRUE( 1.1 == MATH::DopplerRatio( 0.0, 200.0, c, 1.0, 0.5, 1.1 ) );
        ASSERT_TRUE( 0.9 == MATH::DopplerRatio( 0.0, -200.0, c, 1.0, 0.9, 2.0 ) );
        // a source at or beyond the speed of sound is limited, not a division by zero
        const MATH::Float64 extreme = MATH::DopplerRatio( 0.0, 1.0e9, c, 1.0, 0.0, 1000.0 );
        ASSERT_TRUE( extreme > 1.0 && extreme <= 1000.0 && extreme == extreme );
        const MATH::Float64 atSound = MATH::DopplerRatio( 0.0, c, c, 1.0, 0.0, 1000.0 );
        ASSERT_TRUE( atSound > 1.0 && atSound <= 1000.0 );
        // Float32
        ASSERT_TRUE( Near( MATH::DopplerRatio( 0.0f, 34.33f, 343.3f, 1.0f, 0.0f, 100.0f ), 343.3 / 308.97, 1e-4 ) );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SpatialAudioFunctions: decibel conversions" )
    {
        ASSERT_TRUE( Near( MATH::DecibelsToGain( 0.0 ), 1.0, 1e-15 ) );
        ASSERT_TRUE( Near( MATH::DecibelsToGain( 20.0 ), 10.0, 1e-12 ) );
        ASSERT_TRUE( Near( MATH::DecibelsToGain( -20.0 ), 0.1, 1e-15 ) );
        ASSERT_TRUE( Near( MATH::DecibelsToGain( -6.0206 ), 0.5, 1e-5 ) );
        ASSERT_TRUE( Near( MATH::GainToDecibels( 1.0, -120.0 ), 0.0, 1e-12 ) );
        ASSERT_TRUE( Near( MATH::GainToDecibels( 0.1, -120.0 ), -20.0, 1e-12 ) );
        ASSERT_TRUE( Near( MATH::GainToDecibels( 2.0, -120.0 ), 6.0206, 1e-4 ) );
        ASSERT_TRUE( -120.0 == MATH::GainToDecibels( 0.0, -120.0 ) );
        ASSERT_TRUE( -120.0 == MATH::GainToDecibels( -0.5, -120.0 ) );
        ASSERT_TRUE( -60.0 == MATH::GainToDecibels( 1.0e-6, -60.0 ) );                        // floored
        for ( int i = -100; i <= 40; ++i )
            ASSERT_TRUE( Near( MATH::GainToDecibels( MATH::DecibelsToGain( (MATH::Float64) i ), -200.0 ), (MATH::Float64) i, 1e-9 ) );
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformDirectivityAndBlendTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "SpatialAudioFunctions: directivity presets" )
    {
        const MATH::CTDirectivityPattern< MATH::Float64 > cardioid = MATH::CTDirectivityPattern< MATH::Float64 >::Cardioid();
        ASSERT_TRUE( Near( cardioid.Evaluate( -1.0 ), 0.0, 1e-12 ) );       // 180 degrees
        ASSERT_TRUE( Near( cardioid.Evaluate( 0.0 ), 0.5, 1e-12 ) );        // 90 degrees
        ASSERT_TRUE( Near( cardioid.Evaluate( 1.0 ), 1.0, 1e-12 ) );

        const MATH::CTDirectivityPattern< MATH::Float64 > eight = MATH::CTDirectivityPattern< MATH::Float64 >::FigureEight();
        ASSERT_TRUE( Near( eight.Evaluate( 0.0 ), 0.0, 1e-12 ) );
        ASSERT_TRUE( Near( eight.Evaluate( -1.0 ), 1.0, 1e-12 ) );
        ASSERT_TRUE( Near( eight.Evaluate( 1.0 ), 1.0, 1e-12 ) );

        const MATH::CTDirectivityPattern< MATH::Float64 > omni = MATH::CTDirectivityPattern< MATH::Float64 >::Omnidirectional();
        for ( int i = -10; i <= 10; ++i )
            ASSERT_TRUE( Near( omni.Evaluate( i * 0.1 ), 1.0, 1e-12 ) );

        const MATH::CTDirectivityPattern< MATH::Float64 > super = MATH::CTDirectivityPattern< MATH::Float64 >::Supercardioid();
        ASSERT_TRUE( Near( super.GetAlpha(), ( 3.0 - sqrt( 3.0 ) ) / 2.0, 1e-15 ) && Near( super.GetAlpha(), 0.634, 1e-3 ) );
        ASSERT_TRUE( Near( super.Evaluate( -1.0 ), 2.0 - sqrt( 3.0 ), 1e-12 ) );
        const MATH::CTDirectivityPattern< MATH::Float64 > hyper = MATH::CTDirectivityPattern< MATH::Float64 >::Hypercardioid();
        ASSERT_TRUE( Near( hyper.Evaluate( -1.0 ), 0.5, 1e-12 ) && Near( hyper.Evaluate( 0.0 ), 0.25, 1e-12 ) );

        // The input is clamped, the sharpness is an exponent, the setters clamp
        ASSERT_TRUE( Near( cardioid.Evaluate( 3.0 ), 1.0, 1e-12 ) && Near( cardioid.Evaluate( -3.0 ), 0.0, 1e-12 ) );
        const MATH::CTDirectivityPattern< MATH::Float64 > sharp( 0.5, 2.0 );
        ASSERT_TRUE( Near( sharp.Evaluate( 0.0 ), 0.25, 1e-12 ) );
        const MATH::CTDirectivityPattern< MATH::Float64 > clamped( 7.0, -1.0 );
        ASSERT_TRUE( 1.0 == clamped.GetAlpha() && 0.0 == clamped.GetSharpness() );
        ASSERT_TRUE( MATH::CTDirectivityPattern< MATH::Float64 >() == omni );
        // never negative, also for a figure of eight at the rear lobe
        bool nonNegative = true;
        for ( int i = -100; i <= 100; ++i )
            if ( eight.Evaluate( i * 0.01 ) < 0.0 ) nonNegative = false;
        ASSERT_TRUE( nonNegative );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SpatialAudioFunctions: blending gains toward uniform restores the target power" )
    {
        const MATH::Float64 target = 2.0;
        // f = 0: uniform
        MATH::Float64 gains[ 6 ] = { 1.0, 1.0, 0.0, 0.0, 0.0, 0.0 };
        MATH::BlendGainsTowardUniform( gains, 6, 0.0, target );
        bool uniform = true;
        MATH::Float64 power = 0.0;
        for ( int i = 0; i < 6; ++i )
        {
            if ( !Near( gains[ i ], sqrt( target / 6.0 ), 1e-12 ) ) uniform = false;
            power += gains[ i ] * gains[ i ];
        }
        ASSERT_TRUE( uniform && Near( power, target, 1e-12 ) );

        // f = 1: the shape is kept
        MATH::Float64 kept[ 4 ] = { 1.0, 1.0, 0.0, 0.0 };
        MATH::BlendGainsTowardUniform( kept, 4, 1.0, target );
        ASSERT_TRUE( Near( kept[ 0 ], 1.0, 1e-12 ) && Near( kept[ 1 ], 1.0, 1e-12 ) && Near( kept[ 2 ], 0.0, 1e-12 ) && Near( kept[ 3 ], 0.0, 1e-12 ) );

        // Anything in between: the power is the target, the shape moves monotonically toward uniform, f is clamped
        bool powerOk = true;
        bool monotonic = true;
        MATH::Float64 previousSpread = 1e9;
        for ( int i = 0; i <= 20; ++i )
        {
            const MATH::Float64 f = 1.0 - i * 0.05;
            MATH::Float64 blend[ 4 ] = { 1.2, 0.8, 0.0, 0.0 };
            const MATH::Float64 norm = sqrt( target / ( 1.2 * 1.2 + 0.8 * 0.8 ) );
            blend[ 0 ] *= norm;
            blend[ 1 ] *= norm;
            MATH::BlendGainsTowardUniform( blend, 4, f, target );
            MATH::Float64 sum = 0.0;
            for ( int c = 0; c < 4; ++c ) sum += blend[ c ] * blend[ c ];
            if ( !Near( sum, target, 1e-12 ) ) powerOk = false;
            const MATH::Float64 spread = blend[ 0 ] - blend[ 3 ];
            if ( spread > previousSpread + 1e-12 ) monotonic = false;
            previousSpread = spread;
        }
        ASSERT_TRUE( powerOk );
        ASSERT_TRUE( monotonic );

        MATH::Float64 outOfRange[ 2 ] = { 1.0, 1.0 };
        MATH::BlendGainsTowardUniform( outOfRange, 2, 5.0, 2.0 );
        ASSERT_TRUE( Near( outOfRange[ 0 ], 1.0, 1e-12 ) );
        MATH::BlendGainsTowardUniform( outOfRange, 2, -5.0, 2.0 );
        ASSERT_TRUE( Near( outOfRange[ 0 ], 1.0, 1e-12 ) && Near( outOfRange[ 1 ], 1.0, 1e-12 ) );

        // Zero power input blended with f = 1 falls back to uniform; count 0 is a no-op; Float32 works
        MATH::Float64 silent[ 2 ] = { 0.0, 0.0 };
        MATH::BlendGainsTowardUniform( silent, 2, 1.0, 2.0 );
        ASSERT_TRUE( Near( silent[ 0 ], 1.0, 1e-12 ) && Near( silent[ 1 ], 1.0, 1e-12 ) );
        MATH::BlendGainsTowardUniform( silent, 0, 1.0, 2.0 );
        MATH::Float32 narrow[ 3 ] = { 1.0f, 0.0f, 0.0f };
        MATH::BlendGainsTowardUniform( narrow, 3, 0.25f, 1.0f );
        ASSERT_TRUE( Near( narrow[ 0 ] * narrow[ 0 ] + narrow[ 1 ] * narrow[ 1 ] + narrow[ 2 ] * narrow[ 2 ], 1.0, 1e-6 ) );
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformSpatialAudioFunctionsTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING SpatialAudioFunctions TESTS" );

    PerformCurveTests();
    PerformDistanceTests();
    PerformConeTests();
    PerformPanDopplerTests();
    PerformDirectivityAndBlendTests();

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SpatialAudioFunctions TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
