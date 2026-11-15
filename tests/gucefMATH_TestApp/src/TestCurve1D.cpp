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

#ifndef GUCEF_MATH_CTCURVE_H
#include "gucefMATH_CTCurve.h"
#define GUCEF_MATH_CTCURVE_H
#endif /* GUCEF_MATH_CTCURVE_H ? */

#ifndef GUCEF_MATH_CTCATMULLROMSPLINE_H
#include "gucefMATH_CTCatmullRomSpline.h"
#define GUCEF_MATH_CTCATMULLROMSPLINE_H
#endif /* GUCEF_MATH_CTCATMULLROMSPLINE_H ? */

#include "TestMathReference.h"
#include "TestCurve1D.h"

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

GUCEF_MATH_STATIC_ASSERT( CORE::TypeHasMemberFunctionForDataNodeSerialization< CTCurve< Float64 > >::value, CTCurve_has_Serialize );

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformCurve1DTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Curve1D TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "Curve1D" );

    GUCEF_TESTFW_TESTCASE( "Curve1D: linear keys evaluate exactly and keys stay sorted" )
        CTCurve< Float64 > curve;
        curve.AddKey( 2.0, 20.0 );
        curve.AddKey( 0.0, 0.0 );
        curve.AddKey( 4.0, 10.0 );
        curve.AddKey( 1.0, 5.0 );
        ASSERT_TRUE( 4 == curve.GetKeyCount() && 0.0 == curve.GetKey( 0 ).time && 1.0 == curve.GetKey( 1 ).time && 4.0 == curve.GetKey( 3 ).time );
        for ( UInt32 i=0; i<4; ++i )
            ASSERT_TRUE( curve.SetTangentMode( i, CURVETANGENT_LINEAR ) );
        ASSERT_TRUE( !curve.SetTangentMode( 9, CURVETANGENT_LINEAR ) );
        ASSERT_TRUE( 0.0 == curve.Evaluate( 0.0 ) && 5.0 == curve.Evaluate( 1.0 ) && 20.0 == curve.Evaluate( 2.0 ) && 10.0 == curve.Evaluate( 4.0 ) );
        ASSERT_TRUE( fabs( curve.Evaluate( 0.5 ) - 2.5 ) < 1e-14 && fabs( curve.Evaluate( 1.5 ) - 12.5 ) < 1e-14 && fabs( curve.Evaluate( 3.0 ) - 15.0 ) < 1e-14 );
        ASSERT_TRUE( fabs( curve.GetDerivative( 1.5 ) - 15.0 ) < 1e-12 && fabs( curve.GetDerivative( 3.0 ) + 5.0 ) < 1e-12 );
        // replacing the key at the same time
        ASSERT_TRUE( 1 == curve.AddKey( 1.0, 7.0 ) && 4 == curve.GetKeyCount() && 7.0 == curve.GetKey( 1 ).value );
        ASSERT_TRUE( curve.RemoveKey( 1 ) && 3 == curve.GetKeyCount() && !curve.RemoveKey( 3 ) );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Curve1D: keys with a time, value or tangent that is not finite are rejected" )
        CTCurve< Float64 > curve;
        curve.AddKey( 1.0, 1.0 );
        curve.AddKey( 2.0, 2.0 );
        const Float64 nan = std::numeric_limits< Float64 >::quiet_NaN();
        const Float64 inf = std::numeric_limits< Float64 >::infinity();
        const UInt32 invalid = (UInt32) CTCurve< Float64 >::INVALID_INDEX;
        ASSERT_TRUE( invalid == curve.AddKey( nan, 3.0 ) && invalid == curve.AddKey( 3.0, nan ) && invalid == curve.AddKey( inf, 1.0 ) && invalid == curve.AddKey( 0.0, -inf ) );
        ASSERT_TRUE( invalid == curve.AddKey( 3.0, 1.0, nan, 0.0 ) && invalid == curve.AddKey( 3.0, 1.0, 0.0, inf ) );
        ASSERT_TRUE( 2 == curve.GetKeyCount() && 1.0 == curve.GetKey( 0 ).time && 2.0 == curve.GetKey( 1 ).time );
        ASSERT_TRUE( curve.Evaluate( 1.5 ) == curve.Evaluate( 1.5 ) );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Curve1D: constant tangent mode steps" )
        CTCurve< Float64 > curve;
        curve.AddKey( 0.0, 1.0 );
        curve.AddKey( 1.0, 2.0 );
        curve.AddKey( 2.0, 5.0 );
        curve.SetTangentMode( 0, CURVETANGENT_CONSTANT );
        curve.SetTangentMode( 1, CURVETANGENT_CONSTANT );
        curve.SetTangentMode( 2, CURVETANGENT_CONSTANT );
        ASSERT_TRUE( 1.0 == curve.Evaluate( 0.0 ) && 1.0 == curve.Evaluate( 0.999999 ) && 2.0 == curve.Evaluate( 1.0 ) && 2.0 == curve.Evaluate( 1.5 ) && 5.0 == curve.Evaluate( 2.0 ) );
        ASSERT_TRUE( 0.0 == curve.GetDerivative( 0.5 ) && 0.0 == curve.GetDerivative( 1.5 ) );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Curve1D: wrap modes clamp, loop, ping-pong and offset loop" )
        CTCurve< Float64 > curve;
        curve.AddKey( 0.0, 0.0 );
        curve.AddKey( 1.0, 2.0 );
        curve.AddKey( 2.0, 3.0 );
        for ( UInt32 i=0; i<3; ++i )
            curve.SetTangentMode( i, CURVETANGENT_LINEAR );
        // clamp
        ASSERT_TRUE( 0.0 == curve.Evaluate( -3.0 ) && 3.0 == curve.Evaluate( 9.0 ) && 0.0 == curve.GetDerivative( 9.0 ) );
        // loop
        curve.SetPreWrapMode( CURVEWRAP_LOOP );
        curve.SetPostWrapMode( CURVEWRAP_LOOP );
        ASSERT_TRUE( fabs( curve.Evaluate( 2.5 ) - curve.Evaluate( 0.5 ) ) < 1e-12 && fabs( curve.Evaluate( -0.5 ) - curve.Evaluate( 1.5 ) ) < 1e-12 && fabs( curve.Evaluate( 7.25 ) - curve.Evaluate( 1.25 ) ) < 1e-12 );
        // ping-pong
        curve.SetPreWrapMode( CURVEWRAP_PINGPONG );
        curve.SetPostWrapMode( CURVEWRAP_PINGPONG );
        ASSERT_TRUE( fabs( curve.Evaluate( 2.5 ) - curve.Evaluate( 1.5 ) ) < 1e-12 && fabs( curve.Evaluate( 3.25 ) - curve.Evaluate( 0.75 ) ) < 1e-12 && fabs( curve.Evaluate( 4.5 ) - curve.Evaluate( 0.5 ) ) < 1e-12 );
        ASSERT_TRUE( fabs( curve.Evaluate( -0.5 ) - curve.Evaluate( 0.5 ) ) < 1e-12 );
        ASSERT_TRUE( fabs( curve.GetDerivative( 2.5 ) + curve.GetDerivative( 1.5 ) ) < 1e-12 );
        // offset loop: every cycle adds 3
        curve.SetPreWrapMode( CURVEWRAP_OFFSET_LOOP );
        curve.SetPostWrapMode( CURVEWRAP_OFFSET_LOOP );
        ASSERT_TRUE( fabs( curve.Evaluate( 2.5 ) - ( curve.Evaluate( 0.5 ) + 3.0 ) ) < 1e-12 && fabs( curve.Evaluate( 4.5 ) - ( curve.Evaluate( 0.5 ) + 6.0 ) ) < 1e-12 && fabs( curve.Evaluate( -0.5 ) - ( curve.Evaluate( 1.5 ) - 3.0 ) ) < 1e-12 );
        // continuous across the cycle border
        ASSERT_TRUE( fabs( curve.Evaluate( 2.0 + 1e-9 ) - curve.Evaluate( 2.0 ) ) < 1e-6 );
        ASSERT_TRUE( 0.0 == CTCurve< Float64 >().Evaluate( 1.0 ) );
        CTCurve< Float64 > single;
        single.AddKey( 1.0, 4.0 );
        single.SetPostWrapMode( CURVEWRAP_OFFSET_LOOP );
        ASSERT_TRUE( 4.0 == single.Evaluate( -3.0 ) && 4.0 == single.Evaluate( 11.0 ) && 0.0 == single.GetDerivative( 2.0 ) );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Curve1D: automatic tangents reproduce the uniform Catmull-Rom spline" )
        const Float64 values[ 7 ] = { 0.0, 2.0, -1.0, 3.0, 3.5, 0.5, 4.0 };
        CTCurve< Float64 > curve;
        CTCatmullRomSpline< Float64 > spline( CATMULLROM_UNIFORM );
        for ( UInt32 i=0; i<7; ++i )
        {
            curve.AddKey( 10.0 + 0.5 * i, values[ i ] );     // equally spaced keys, auto tangents by default
            spline.AddPoint( values[ i ] );
        }
        for ( Float64 t = 10.0; t <= 13.0; t += 0.0625 )
        {
            const Float64 s = ( t - 10.0 ) / 0.5;
            ASSERT_TRUE( fabs( curve.Evaluate( t ) - spline.Evaluate( s ) ) < 1e-12 );
            ASSERT_TRUE( fabs( curve.GetDerivative( t ) - spline.GetDerivative( s ) / 0.5 ) < 1e-10 );
        }
        // non-uniform times: the slope of a key is the difference between its neighbours over their time span
        CTCurve< Float64 > uneven;
        uneven.AddKey( 0.0, 0.0 ); uneven.AddKey( 1.0, 1.0 ); uneven.AddKey( 4.0, 7.0 );
        Float64 in = 0.0, out = 0.0;
        ASSERT_TRUE( uneven.GetEffectiveTangents( 1, in, out ) && fabs( in - 7.0 / 4.0 ) < 1e-14 && in == out );
        ASSERT_TRUE( uneven.GetEffectiveTangents( 0, in, out ) && fabs( out - 1.0 ) < 1e-14 && uneven.GetEffectiveTangents( 2, in, out ) && fabs( in - 2.0 ) < 1e-14 );
        ASSERT_TRUE( !uneven.GetEffectiveTangents( 3, in, out ) );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Curve1D: clamped auto tangents never overshoot the keys" )
        const Float64 values[ 8 ] = { 0.0, 0.1, 0.9, 1.0, 1.0, 0.2, 0.2, 3.0 };
        CTCurve< Float64 > curve;
        for ( UInt32 i=0; i<8; ++i )
        {
            curve.AddKey( (Float64) i, values[ i ] );
            curve.SetTangentMode( i, CURVETANGENT_CLAMPED_AUTO );
        }
        Float64 in = 1.0, out = 1.0;
        ASSERT_TRUE( curve.GetEffectiveTangents( 3, in, out ) && 0.0 == in && 0.0 == out );    // local extremum plateau
        ASSERT_TRUE( curve.GetEffectiveTangents( 0, in, out ) && 0.0 == out );
        ASSERT_TRUE( curve.GetEffectiveTangents( 2, in, out ) && out > 0.0 );
        for ( UInt32 i=0; i+1<8; ++i )
        {
            const Float64 low = values[ i ] < values[ i + 1 ] ? values[ i ] : values[ i + 1 ];
            const Float64 high = values[ i ] < values[ i + 1 ] ? values[ i + 1 ] : values[ i ];
            // every segment stays between its two keys (monotone), also where the central slope alone would overshoot
            for ( Float64 u = 0.0; u <= 1.0; u += 0.0625 )
            {
                const Float64 value = curve.Evaluate( i + u );
                ASSERT_TRUE( value >= low - 1e-12 && value <= high + 1e-12 );
            }
        }
        // between two extremum keys the curve is monotone
        Float64 previous = curve.Evaluate( 5.0 );
        for ( Float64 t = 5.0; t <= 6.0; t += 0.01 )
        {
            ASSERT_TRUE( fabs( curve.Evaluate( t ) - 0.2 ) < 1e-12 );
            previous = curve.Evaluate( t );
        }
        (void) previous;
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Curve1D: free tangents give the Hermite curve and the segment hint changes nothing" )
        CTCurve< Float64 > curve;
        curve.AddKey( 0.0, 0.0, 1.0, 2.0 );
        curve.AddKey( 2.0, 1.0, -1.0, 0.5 );
        curve.AddKey( 3.0, 4.0, 3.0, 3.0 );
        // segment 0: Hermite on [ 0, 2 ] with out tangent 2 and next in tangent -1
        const Float64 u = 0.5;
        const Float64 h10 = u * u * u - 2.0 * u * u + u;
        const Float64 h01 = -2.0 * u * u * u + 3.0 * u * u;
        const Float64 h11 = u * u * u - u * u;
        ASSERT_TRUE( fabs( curve.Evaluate( 1.0 ) - ( h10 * 2.0 * 2.0 + h01 * 1.0 + h11 * -1.0 * 2.0 ) ) < 1e-14 );
        ASSERT_TRUE( fabs( curve.GetDerivative( 0.0 ) - 2.0 ) < 1e-12 );
        // the hint version equals the plain one for increasing, decreasing and random times
        MathReference::CSeededSequence rng( 7 );
        UInt32 hint = 0;
        for ( UInt32 i=0; i<2000; ++i )
        {
            const Float64 t = rng.NextFloat64( -1.0, 5.0 );
            ASSERT_TRUE( curve.Evaluate( t, hint ) == curve.Evaluate( t ) );
        }
        hint = 2;
        for ( Float64 t = 3.0; t >= 0.0; t -= 0.01 )
            ASSERT_TRUE( curve.Evaluate( t, hint ) == curve.Evaluate( t ) );
        // derivative against differences
        for ( Float64 t = 0.1; t < 2.9; t += 0.37 )
            ASSERT_TRUE( fabs( curve.GetDerivative( t ) - ( curve.Evaluate( t + 1e-6 ) - curve.Evaluate( t - 1e-6 ) ) * 0.5e6 ) < 1e-5 );
        ASSERT_TRUE( curve.SetTangents( 1, 0.0, 0.0 ) && 0.0 == curve.GetKey( 1 ).inTangent && CURVETANGENT_FREE == curve.GetKey( 1 ).mode && !curve.SetTangents( 5, 0.0, 0.0 ) );
        ASSERT_TRUE( 99.0 == CTCurve< Float64 >().GetKey( 4 ).time + 99.0 );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Curve1D: Float32 curve and DataNode round trip" )
        CTCurve< Float32 > curve32;
        curve32.AddKey( 0.0f, 0.0f ); curve32.AddKey( 1.0f, 1.0f ); curve32.AddKey( 2.0f, 0.0f );
        ASSERT_TRUE( fabs( curve32.Evaluate( 1.0f ) - 1.0f ) < 1e-6f && curve32.Evaluate( 0.5f ) > 0.0f );

        CTCurve< Float64 > curve;
        curve.AddKey( 0.0, 0.0 ); curve.AddKey( 1.0, 1.0, 0.5, 0.25 ); curve.AddKey( 2.5, -2.0 ); curve.AddKey( 3.0, 8.0 );
        curve.SetTangentMode( 3, CURVETANGENT_CLAMPED_AUTO );
        curve.SetPreWrapMode( CURVEWRAP_PINGPONG );
        curve.SetPostWrapMode( CURVEWRAP_OFFSET_LOOP );
        CORE::CDataNodeSerializableSettings settings;
        CORE::CDataNode node( "curve", GUCEF_DATATYPE_OBJECT );
        CTCurve< Float64 > back;
        ASSERT_TRUE( curve.Serialize( node, settings ) && back.Deserialize( node, settings ) );
        ASSERT_TRUE( 4 == back.GetKeyCount() && CURVEWRAP_PINGPONG == back.GetPreWrapMode() && CURVEWRAP_OFFSET_LOOP == back.GetPostWrapMode() && CURVETANGENT_CLAMPED_AUTO == back.GetKey( 3 ).mode );
        for ( Float64 t = -2.0; t < 8.0; t += 0.3125 )
            ASSERT_TRUE( curve.Evaluate( t ) == back.Evaluate( t ) );
        // a failed read leaves the curve unchanged
        CORE::CDataNode broken( "curve", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( !back.Deserialize( broken, settings ) && 4 == back.GetKeyCount() );
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
