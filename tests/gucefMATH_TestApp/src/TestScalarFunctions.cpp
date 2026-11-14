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
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_SCALARFUNCTIONS_H
#include "gucefMATH_ScalarFunctions.h"
#define GUCEF_MATH_SCALARFUNCTIONS_H
#endif /* GUCEF_MATH_SCALARFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTMATHCONSTANTS_H
#include "gucefMATH_CTMathConstants.h"
#define GUCEF_MATH_CTMATHCONSTANTS_H
#endif /* GUCEF_MATH_CTMATHCONSTANTS_H ? */

#ifndef GUCEF_MATH_CTCOMPENSATEDSUM_H
#include "gucefMATH_CTCompensatedSum.h"
#define GUCEF_MATH_CTCOMPENSATEDSUM_H
#endif /* GUCEF_MATH_CTCOMPENSATEDSUM_H ? */

#ifndef GUCEF_MATH_CTNUMERICTRAITSFRACTION_H
#include "gucefMATH_CTNumericTraitsFraction.h"
#define GUCEF_MATH_CTNUMERICTRAITSFRACTION_H
#endif /* GUCEF_MATH_CTNUMERICTRAITSFRACTION_H ? */

#ifndef GUCEF_MATH_FIXEDPOINT_H
#include "gucefMATH_fixedpoint.h"
#define GUCEF_MATH_FIXEDPOINT_H
#endif /* GUCEF_MATH_FIXEDPOINT_H ? */

#include "TestScalarFunctions.h"

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


typedef CORE::TInt64Fraction TFraction64;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformScalarFunctionsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "ScalarFunctions" );

    GUCEF_TESTFW_TESTCASE( "ScalarFunctions: Clamp, Clamp01, Lerp, LerpClamped, InverseLerp, Remap" )
    try
    {
        ASSERT_TRUE( 1.0f == Clamp( 2.0f, 0.0f, 1.0f ) && 0.0f == Clamp( -2.0f, 0.0f, 1.0f ) && 0.5f == Clamp( 0.5f, 0.0f, 1.0f ) );
        ASSERT_TRUE( 5 == Clamp( 9, -5, 5 ) && 0.0 == Clamp01( -0.1 ) && 1.0 == Clamp01( 7.0 ) );
        ASSERT_TRUE( TFixed32( 1 ) == Clamp01( TFixed32( 3 ) ) && TFraction64( 1, 2 ) == Clamp01( TFraction64( 1, 2 ) ) );
        ASSERT_TRUE( 1.5f == Lerp( 1.0f, 2.0f, 0.5f ) && 3.0 == Lerp( 1.0, 2.0, 2.0 ) && 2.0 == LerpClamped( 1.0, 2.0, 2.0 ) );
        ASSERT_TRUE( TFixed32( 1.875 ) == Lerp( TFixed32( 1.5 ), TFixed32( 2.25 ), TFixed32( 0.5 ) ) );
        ASSERT_TRUE( TFraction64( 4, 3 ) == Lerp( TFraction64( 1, 1 ), TFraction64( 2, 1 ), TFraction64( 1, 3 ) ) );
        ASSERT_TRUE( 0.0 == InverseLerp( 2.0, 2.0, 7.0 ) && 0.0f == InverseLerp( 2.0f, 2.0f, -3.0f ) );
        ASSERT_TRUE( 0.25 == InverseLerp( 2.0, 6.0, 3.0 ) && 1.5 == InverseLerp( 2.0, 6.0, 8.0 ) );
        ASSERT_TRUE( TFraction64( 1, 3 ) == InverseLerp( TFraction64( 0, 1 ), TFraction64( 3, 1 ), TFraction64( 1, 1 ) ) );
        ASSERT_TRUE( 15.0 == Remap( 0.5, 0.0, 1.0, 10.0, 20.0 ) && -1.0 == Remap( 0.0, 0.0, 10.0, -1.0, 1.0 ) && 10.0 == Remap( 3.0, 3.0, 3.0, 10.0, 20.0 ) );
        // 2D audio pan: clamp( ( xS - xL ) / width, -1, 1 )
        ASSERT_TRUE( -1.0f == Clamp( Remap( -30.0f, -10.0f, 10.0f, -1.0f, 1.0f ), -1.0f, 1.0f ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScalarFunctions: SmoothStep, SmootherStep, Wrap, PingPong, Snap, MoveTowards" )
    try
    {
        ASSERT_TRUE( 0.5 == SmoothStep( 0.0, 1.0, 0.5 ) && 0.0 == SmoothStep( 0.0, 1.0, -1.0 ) && 1.0 == SmoothStep( 0.0, 1.0, 2.0 ) );
        ASSERT_TRUE( 0.15625 == SmoothStep( 0.0, 1.0, 0.25 ) && 0.5f == SmootherStep( 0.0f, 1.0f, 0.5f ) && 0.103515625 == SmootherStep( 0.0, 1.0, 0.25 ) );
        ASSERT_TRUE( TFixed32( 0.5 ) == SmoothStep( TFixed32(), TFixed32( 2 ), TFixed32( 1 ) ) );
        ASSERT_TRUE( TFraction64( 5, 32 ) == SmoothStep( TFraction64( 0, 1 ), TFraction64( 1, 1 ), TFraction64( 1, 4 ) ) );

        ASSERT_TRUE( 2 == Wrap( -1, 0, 3 ) && 2.0 == Wrap( -1.0, 0.0, 3.0 ) && 0 == Wrap( 3, 0, 3 ) && 1.5f == Wrap( 7.5f, 1.0f, 3.0f ) );
        ASSERT_TRUE( 4 == Wrap( 9, 4, 4 ) );          // empty range gives the minimum
        ASSERT_TRUE( TFixed32( 2 ) == Wrap( TFixed32( -1 ), TFixed32(), TFixed32( 3 ) ) );
        ASSERT_TRUE( 2 == PingPong( 4, 3 ) && 2.0 == PingPong( 4.0, 3.0 ) && 1.0 == PingPong( 7.0, 3.0 ) && 0.0 == PingPong( 6.0, 3.0 ) && 0.0 == PingPong( 1.0, 0.0 ) );
        ASSERT_TRUE( 3.0 == PingPong( -3.0, 3.0 ) );
        // unsigned and narrow integers: exact, no wrap of intermediate differences
        ASSERT_TRUE( 1u == PingPong< UInt32 >( 1, 3 ) && 2u == PingPong< UInt32 >( 4, 3 ) && 0u == PingPong< UInt32 >( 6, 3 ) );
        ASSERT_TRUE( (Int8) 80 == PingPong< Int8 >( 120, 100 ) && (Int8) 80 == PingPong< Int8 >( -80, 100 ) );
        ASSERT_TRUE( (Int8) 90 == Wrap< Int8 >( -110, -100, 100 ) && (Int8) -100 == Wrap< Int8 >( 100, -100, 100 ) );
        ASSERT_TRUE( 6u == Wrap< UInt32 >( 2, 5, 9 ) && 6u == Wrap< UInt32 >( 10, 5, 9 ) && 0u == Wrap< UInt64 >( ~(UInt64) 0, 0, 5 ) );
        ASSERT_TRUE( TFixed32( 10000 ) == Wrap( TFixed32( -30000 ), TFixed32( -10000 ), TFixed32( 30000 ) ) );
        // the float range is half-open even where the addition rounds up to the maximum
        const Float64 justBelowOne = 1.0 - ldexp( 1.0, -53 );
        ASSERT_TRUE( Wrap( justBelowOne, 1.0, 2.0 ) < 2.0 && Wrap( justBelowOne, 1.0, 2.0 ) >= 1.0 );
        const Float32 justBelowOne32 = 1.0f - ldexpf( 1.0f, -24 );
        ASSERT_TRUE( Wrap( justBelowOne32, 1.0f, 2.0f ) < 2.0f );

        ASSERT_TRUE( 7.5 == Snap( 7.3, 2.5, CORE::ROUNDINGMODE_HALF_EVEN ) && 5.0 == Snap( 7.3, 2.5, CORE::ROUNDINGMODE_FLOOR ) );
        ASSERT_TRUE( 8 == Snap( 7, 2, CORE::ROUNDINGMODE_HALF_EVEN ) && 6 == Snap( 7, 2, CORE::ROUNDINGMODE_HALF_DOWN ) && -8 == Snap( -7, 2, CORE::ROUNDINGMODE_HALF_UP ) );
        ASSERT_TRUE( 7 == Snap( 7, 0, CORE::ROUNDINGMODE_HALF_EVEN ) );
        ASSERT_TRUE( TFixed32( 0.75 ) == Snap( TFixed32( 0.7 ), TFixed32( 0.25 ), CORE::ROUNDINGMODE_HALF_EVEN ) );
        ASSERT_TRUE( TFraction64( 2, 3 ) == Snap( TFraction64( 3, 5 ), TFraction64( 1, 3 ), CORE::ROUNDINGMODE_HALF_EVEN ) );

        ASSERT_TRUE( 3.0 == MoveTowards( 1.0, 5.0, 2.0 ) && 5.0 == MoveTowards( 4.0, 5.0, 2.0 ) && -1.0 == MoveTowards( 1.0, -5.0, 2.0 ) );
        ASSERT_TRUE( 3 == MoveTowards( 1, 5, 2 ) && 5 == MoveTowards( 1, 5, 10 ) && 1 == MoveTowards( 1, 5, 0 ) );
        // integers are moved exactly: differences that do not fit the type and unsigned steps downward
        ASSERT_TRUE( (Int8) -90 == MoveTowards< Int8 >( -100, 100, 10 ) && (Int8) 27 == MoveTowards< Int8 >( -100, 100, 127 ) && (Int8) 100 == MoveTowards< Int8 >( 90, 100, 127 ) );
        ASSERT_TRUE( 4u == MoveTowards< UInt32 >( 5, 3, 1 ) && 3u == MoveTowards< UInt32 >( 5, 3, 7 ) && 6u == MoveTowards< UInt32 >( 5, 9, 1 ) );
        ASSERT_TRUE( (Int64) 0x8000000000000000ULL + 5 == MoveTowards< Int64 >( (Int64) 0x8000000000000000ULL, (Int64) 0x7FFFFFFFFFFFFFFFULL, 5 ) );
        ASSERT_TRUE( TFixed32( -29990 ) == MoveTowards( TFixed32( -30000 ), TFixed32( 30000 ), TFixed32( 10 ) ) );
        ASSERT_TRUE( (UInt8) 250 == MoveTowards< UInt8 >( 255, 0, 5 ) && 2.5f == MoveTowards( 3.0f, 0.0f, 0.5f ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScalarFunctions: Sign, Square, Cube, Sqrt, InverseSqrt, LogLerp" )
    try
    {
        ASSERT_TRUE( -1 == Sign( -9 ) && 0.0 == Sign( 0.0 ) && 1.0f == Sign( 3.0f ) && TFixed32( -1 ) == Sign( TFixed32( -0.5 ) ) );
        ASSERT_TRUE( 9 == Square( -3 ) && -27.0 == Cube( -3.0 ) && TFraction64( 1, 8 ) == Cube( TFraction64( 1, 2 ) ) );
        ASSERT_TRUE( 3.0 == Sqrt( 9.0 ) && 1.5f == Sqrt( 2.25f ) && TFixed32( 3 ) == Sqrt( TFixed32( 9 ) ) && 0.5 == InverseSqrt( 4.0 ) );
        // the distance low-pass of the spatial audio design: 20 kHz to 1500 Hz on a logarithmic scale
        ASSERT_TRUE( fabs( LogLerp( 20000.0, 1500.0, 0.5 ) - 5477.2256 ) <= 1e-3 );
        ASSERT_TRUE( fabs( LogLerp( 20000.0f, 1500.0f, 0.5f ) - 5477.2256f ) <= 1e-2f );
        ASSERT_TRUE( 20000.0 == LogLerp( 20000.0, 1500.0, 0.0 ) || fabs( LogLerp( 20000.0, 1500.0, 0.0 ) - 20000.0 ) < 1e-9 );
        ASSERT_TRUE( CTNumericTraits< TFixed64 >::EqualsWithin( TFixed64( 4 ), LogLerp( TFixed64( 2 ), TFixed64( 8 ), TFixed64( 0.5 ) ), TFixed64::FromRaw( 64 ), TFixed64() ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScalarFunctions: Approximately, EqualsWithin, EqualsUlp, RoundToInteger" )
    try
    {
        ASSERT_TRUE( Approximately( 1.0f, 1.0f + 5e-7f ) && !Approximately( 1.0f, 1.0001f ) );
        ASSERT_TRUE( Approximately( 1e6, 1e6 + 1e-5 ) && !Approximately( 1.0, 1.0 + 1e-9 ) );
        ASSERT_TRUE( Approximately( TFixed32::FromRaw( 100 ), TFixed32::FromRaw( 102 ) ) && !Approximately( TFixed32::FromRaw( 100 ), TFixed32::FromRaw( 103 ) ) );
        ASSERT_TRUE( Approximately( 5, 5 ) && !Approximately( 5, 6 ) && Approximately( TFraction64( 2, 4 ), TFraction64( 1, 2 ) ) );
        ASSERT_TRUE( EqualsWithin( 10.0, 10.5, 1.0, 0.0 ) && !EqualsWithin( 10.0, 12.0, 1.0, 0.1 ) && EqualsWithin( 10.0, 11.0, 0.0, 0.1 ) );

        Float32 next = nextafterf( 1.0f, 2.0f );
        ASSERT_TRUE( EqualsUlp( 1.0f, next, 1 ) && !EqualsUlp( 1.0f, next, 0 ) );
        ASSERT_TRUE( EqualsUlp( -0.0f, +0.0f, 0 ) );
        const Float32 nan = (Float32) MathQuietNaN64();
        ASSERT_FALSE( EqualsUlp( nan, nan, 1000 ) || Approximately( nan, nan ) || EqualsWithin( nan, 1.0f, 1e30f, 1e30f ) );

        ASSERT_TRUE( 2 == RoundToInteger< Int32 >( 2.5, CORE::ROUNDINGMODE_HALF_EVEN ) && 3 == RoundToInteger< Int32 >( 2.5, CORE::ROUNDINGMODE_HALF_UP ) );
        ASSERT_TRUE( -3 == RoundToInteger< Int32 >( -2.5, CORE::ROUNDINGMODE_HALF_UP ) && -3 == RoundToInteger< Int32 >( -2.5, CORE::ROUNDINGMODE_FLOOR ) );
        ASSERT_TRUE( 127 == RoundToInteger< Int8 >( 1000.0f, CORE::ROUNDINGMODE_HALF_EVEN ) && 0 == RoundToInteger< UInt8 >( -5.0, CORE::ROUNDINGMODE_HALF_EVEN ) );
        ASSERT_TRUE( 0 == RoundToInteger< Int32 >( MathQuietNaN64(), CORE::ROUNDINGMODE_HALF_EVEN ) );
        ASSERT_TRUE( 2 == RoundToInteger< Int64 >( TFixed32( 2.5 ), CORE::ROUNDINGMODE_HALF_EVEN ) && -1 == RoundToInteger< Int16 >( TFraction64( -2, 3 ), CORE::ROUNDINGMODE_HALF_EVEN ) );
        Int32 out = 9;
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == RoundToInteger( 3e9, CORE::ROUNDINGMODE_HALF_EVEN, out ) && 2147483647 == out );
        out = 9;
        ASSERT_TRUE( MATHSTATUS_ROUNDING_REQUIRED == RoundToInteger( 2.5, CORE::ROUNDINGMODE_UNNECESSARY, out ) && 9 == out );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScalarFunctions: CTMathConstants of the float kinds are the nearest values" )
    try
    {
        ASSERT_TRUE( 3.141592653589793 == CTMathConstants< Float64 >::Pi() && 6.283185307179586 == CTMathConstants< Float64 >::TwoPi() );
        ASSERT_TRUE( 0.7853981633974483 == CTMathConstants< Float64 >::QuarterPi() && 0.3183098861837907 == CTMathConstants< Float64 >::InversePi() );
        ASSERT_TRUE( 2.302585092994046 == CTMathConstants< Float64 >::Ln10() && 0.7071067811865476 == CTMathConstants< Float64 >::InverseSqrt2() );
        ASSERT_TRUE( 1.7320508075688772 == CTMathConstants< Float64 >::Sqrt3() && 1.618033988749895 == CTMathConstants< Float64 >::GoldenRatio() );
        ASSERT_TRUE( 57.29577951308232 == CTMathConstants< Float64 >::DegreesPerRadian() && 0.017453292519943295 == CTMathConstants< Float64 >::RadiansPerDegree() );
        // the nearest Float32 of each exact constant, computed independently (the double literal rounds to it)
        ASSERT_TRUE( 3.1415927410125732f == CTMathConstants< Float32 >::Pi() && 1.5707963705062866f == CTMathConstants< Float32 >::HalfPi() );
        ASSERT_TRUE( 2.7182817459106445f == CTMathConstants< Float32 >::E() && 0.6931471824645996f == CTMathConstants< Float32 >::Ln2() );
        ASSERT_TRUE( 57.295780181884766f == CTMathConstants< Float32 >::DegreesPerRadian() && 0.01745329238474369f == CTMathConstants< Float32 >::RadiansPerDegree() );
        ASSERT_TRUE( 1.6180340051651f == CTMathConstants< Float32 >::GoldenRatio() && 0.31830987334251404f == CTMathConstants< Float32 >::InversePi() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ScalarFunctions: CTCompensatedSum" )
    try
    {
        CTCompensatedSum< Float64 > sum;
        sum.Add( 1.0 );
        sum.Add( 1e100 );
        sum.Add( 1.0 );
        sum.Add( -1e100 );
        ASSERT_TRUE( 2.0 == sum.GetSum() );
        Float64 naive = 1.0 + 1e100 + 1.0 - 1e100;
        ASSERT_TRUE( 0.0 == naive );
        sum.Reset();
        ASSERT_TRUE( 0.0 == sum.GetSum() );
        CTCompensatedSum< Float32 > tenths;
        Float32 plain = 0.0f;
        for ( UInt32 i=0; i<1000000; ++i )
        {
            tenths.Add( 0.1f );
            plain += 0.1f;
        }
        // exact sum 100000.0014901161; the compensation term is itself a Float32 sum of 10^6 terms, so a few units remain
        const Float64 exact = 1000000.0 * (Float64) 0.1f;
        ASSERT_TRUE( fabs( tenths.GetSum() - exact ) < 10.0 );
        ASSERT_TRUE( fabs( plain - exact ) > 100.0 * fabs( tenths.GetSum() - exact ) );      // the uncompensated sum drifts much further
        CTCompensatedSum< TFixed32 > fixedSum;
        fixedSum.Add( TFixed32( 1.5 ) );
        fixedSum.Add( TFixed32( -0.25 ) );
        ASSERT_TRUE( TFixed32( 1.25 ) == fixedSum.GetSum() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
