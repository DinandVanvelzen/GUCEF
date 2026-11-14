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

#ifndef GUCEF_MATH_ANGLEFUNCTIONS_H
#include "gucefMATH_AngleFunctions.h"
#define GUCEF_MATH_ANGLEFUNCTIONS_H
#endif /* GUCEF_MATH_ANGLEFUNCTIONS_H ? */

#include "TestAngles.h"

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


/** |a - b| within 'ulps' units in the last place of Float64 / Float32 */
static bool NearUlp( Float64 a, Float64 b, UInt32 ulps ) { return CTNumericTraits< Float64 >::EqualsUlp( a, b, ulps ); }
static bool NearUlp32( Float32 a, Float32 b, UInt32 ulps ) { return CTNumericTraits< Float32 >::EqualsUlp( a, b, ulps ); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAnglesTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Angles" );

    GUCEF_TESTFW_TESTCASE( "Angles: strong types, conversions between radians and degrees, layout" )
    try
    {
        ASSERT_TRUE( sizeof( TFloat32Radian ) == sizeof( Float32 ) && sizeof( TFloat64Degree ) == sizeof( Float64 ) && sizeof( TFixed64Radian ) == 8 );
        ASSERT_TRUE( 0.0f == TFloat32Radian().GetValue() && 0.0 == TFloat64Degree().GetValue() );

        TFloat64Radian halfTurn = TFloat64Degree( 180.0 );
        ASSERT_TRUE( NearUlp( CTMathConstants< Float64 >::Pi(), halfTurn.GetValue(), 1 ) );
        TFloat32Radian halfTurn32 = Degrees( 180 );
        ASSERT_TRUE( NearUlp32( CTMathConstants< Float32 >::Pi(), halfTurn32.GetValue(), 1 ) );
        TFloat64Degree back = halfTurn;
        ASSERT_TRUE( fabs( back.GetValue() - 180.0 ) < 1e-12 && fabs( halfTurn.GetDegrees() - 180.0 ) < 1e-12 );
        ASSERT_TRUE( fabs( TFloat64Degree( 90.0 ).GetRadians() - CTMathConstants< Float64 >::HalfPi() ) < 1e-15 );
        ASSERT_TRUE( 1.5 == Radians( 1.5 ).GetValue() && 1.5f == Radians( 1.5f ).GetValue() && 90.0f == Degrees( 90 ).GetValue() );

        TFixed32Radian fixedQuarter = TFixed32Degree( TFixed32( 90 ) );
        ASSERT_TRUE( CTNumericTraits< TFixed32 >::EqualsUlp( CTMathConstants< TFixed32 >::HalfPi(), fixedQuarter.GetValue(), 2 ) );
        ASSERT_TRUE( TFixed32( 1 ) == Sin( fixedQuarter ) );
        // fixed point converts with one rounding of the exact product with the pi literal
        ASSERT_TRUE( CTMathConstants< TFixed32 >::Pi() == TFixed32Radian( TFixed32Degree( TFixed32( 180 ) ) ).GetValue() );
        ASSERT_TRUE( CTMathConstants< TFixed64 >::Pi() == TFixed64Radian( TFixed64Degree( TFixed64( 180 ) ) ).GetValue() );
        ASSERT_TRUE( CTMathConstants< TFixed64 >::HalfPi() == TFixed64Radian( TFixed64Degree( TFixed64( -270 ) ) ).GetValue() + CTMathConstants< TFixed64 >::TwoPi() );
        // and both directions are within one raw unit of the Float64 value of the exact input, for 10^4 seeded angles
        UInt32 failures = 0;
        UInt64 state = 77;
        for ( UInt32 i=0; i<10000; ++i )
        {
            state = state * 6364136223846793005ULL + 1442695040888963407ULL;
            TFixed32 degrees = TFixed32::FromRaw( (Int32) ( state >> 33 ) - 0x40000000 );          // |degrees| < 16384
            TFixed32 radians = TFixed32::FromRaw( (Int32) ( state >> 40 ) - 0x00800000 );          // |radians| < 128
            Float64 toRadians = degrees.ToFloat64() * ( 3.14159265358979323846 / 180.0 );
            Float64 toDegrees = radians.ToFloat64() * ( 180.0 / 3.14159265358979323846 );
            if ( fabs( (Float64) TFixed32Radian( TFixed32Degree( degrees ) ).GetValue().GetRaw() - toRadians * 65536.0 ) > 1.0 )
                ++failures;
            if ( fabs( (Float64) TFixed32Radian( radians ).GetDegrees().GetRaw() - toDegrees * 65536.0 ) > 1.0 )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Angles: arithmetic and comparison operators" )
    try
    {
        TFloat64Radian a( 1.0 );
        TFloat64Radian b( 0.25 );
        ASSERT_TRUE( 1.25 == ( a + b ).GetValue() && 0.75 == ( a - b ).GetValue() && -1.0 == ( -a ).GetValue() && a == +a );
        ASSERT_TRUE( 2.0 == ( a * 2.0 ).GetValue() && 2.0 == ( 2.0 * a ).GetValue() && 0.5 == ( a / 2.0 ).GetValue() );
        a += b;
        a -= TFloat64Radian( 0.5 );
        a *= 4.0;
        a /= 2.0;
        ASSERT_TRUE( 1.5 == a.GetValue() );
        ASSERT_TRUE( b < a && a > b && a <= a && a >= a && a != b && !( a == b ) );
        TFloat32Degree d( 10.0f );
        d += Degrees( 20 );
        ASSERT_TRUE( 30.0f == d.GetValue() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Angles: trigonometric overloads on radians and degrees, inverse functions return radians" )
    try
    {
        ASSERT_TRUE( 1.0f == Sin( Degrees( 90 ) ) );
        ASSERT_TRUE( fabs( Cos( Radians( CTMathConstants< Float64 >::Pi() ) ) + 1.0 ) < 1e-15 );
        ASSERT_TRUE( fabs( Tan( TFloat64Degree( 45.0 ) ) - 1.0 ) < 1e-15 );
        ASSERT_TRUE( fabs( Atan2( 1.0, 1.0 ).GetDegrees() - 45.0 ) < 1e-12 );
        ASSERT_TRUE( fabs( Asin( 1.0 ).GetValue() - CTMathConstants< Float64 >::HalfPi() ) < 1e-15 );
        ASSERT_TRUE( fabs( Acos( -1.0 ).GetValue() - CTMathConstants< Float64 >::Pi() ) < 1e-15 );
        ASSERT_TRUE( fabs( Atan( 1.0f ).GetValue() - CTMathConstants< Float32 >::QuarterPi() ) < 1e-6f );
        ASSERT_TRUE( CTMathConstants< TFixed64 >::Pi() == Atan2( TFixed64(), TFixed64( -1 ) ).GetValue() );
        ASSERT_TRUE( TFixed64() == Sin( TFixed64Radian() ) && TFixed64( 1 ) == Cos( TFixed64Radian() ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Angles: WrapAngle, WrapAnglePositive, DeltaAngle, LerpAngle, MoveTowardsAngle" )
    try
    {
        const Float64 pi = CTMathConstants< Float64 >::Pi();
        ASSERT_TRUE( pi == WrapAngle( Radians( 3.0 * pi ) ).GetValue() );
        ASSERT_TRUE( NearUlp32( CTMathConstants< Float32 >::Pi(), WrapAngle( Radians( 3.0f * CTMathConstants< Float32 >::Pi() ) ).GetValue(), 1 ) );
        ASSERT_TRUE( pi == WrapAngle( Radians( -pi ) ).GetValue() );          // ( -pi, pi ]: -pi maps to pi
        // just above an odd multiple of pi, the subtraction can round to exactly -pi: it must stay inside ( -pi, pi ]
        UInt32 outside = 0;
        for ( Int32 k=-50; k<=50; ++k )
        {
            Float64 base = ( 2.0 * k + 1.0 ) * pi;
            for ( Int32 step=-3; step<=3; ++step )
            {
                Float64 x = base;
                for ( Int32 n=0; n<( step < 0 ? -step : step ); ++n )
                    x = nextafter( x, step < 0 ? -1e300 : 1e300 );
                Float64 wrapped = WrapAngle( Radians( x ) ).GetValue();
                if ( !( -pi < wrapped && wrapped <= pi ) )
                    ++outside;
                Float32 wrapped32 = WrapAngle( Radians( (Float32) x ) ).GetValue();
                if ( !( -CTMathConstants< Float32 >::Pi() < wrapped32 && wrapped32 <= CTMathConstants< Float32 >::Pi() ) )
                    ++outside;
            }
        }
        ASSERT_TRUE( 0 == outside );
        ASSERT_TRUE( 180.0 == WrapAngle( TFloat64Degree( -180.0 ) ).GetValue() && -90.0 == WrapAngle( TFloat64Degree( 270.0 ) ).GetValue() );
        ASSERT_TRUE( 350.0f == WrapAnglePositive( Degrees( -10 ) ).GetValue() && 0.0f == WrapAnglePositive( Degrees( 720 ) ).GetValue() );
        ASSERT_TRUE( fabs( WrapAnglePositive( Radians( -0.5 ) ).GetValue() - ( 2.0 * pi - 0.5 ) ) < 1e-15 );

        ASSERT_TRUE( 20.0f == DeltaAngle( Degrees( 350 ), Degrees( 10 ) ).GetValue() );
        ASSERT_TRUE( -20.0f == DeltaAngle( Degrees( 10 ), Degrees( 350 ) ).GetValue() );
        ASSERT_TRUE( 180.0f == DeltaAngle( Degrees( 0 ), Degrees( 180 ) ).GetValue() );
        TFloat64Radian deltaRadians = DeltaAngle( TFloat64Radian( TFloat64Degree( 350.0 ) ), TFloat64Radian( TFloat64Degree( 10.0 ) ) );
        ASSERT_TRUE( fabs( deltaRadians.GetDegrees() - 20.0 ) < 1e-12 );

        TFloat32Degree halfway = LerpAngle( Degrees( 350 ), Degrees( 10 ), 0.5f );
        ASSERT_TRUE( 0.0f == WrapAnglePositive( halfway ).GetValue() );
        ASSERT_TRUE( 0.0f == WrapAngle( halfway ).GetValue() );
        TFloat64Radian halfwayRadians = LerpAngle( TFloat64Radian( TFloat64Degree( 350.0 ) ), TFloat64Radian( TFloat64Degree( 10.0 ) ), 0.5 );
        ASSERT_TRUE( fabs( WrapAngle( halfwayRadians ).GetValue() ) < 1e-12 );

        ASSERT_TRUE( 355.0f == MoveTowardsAngle( Degrees( 350 ), Degrees( 10 ), Degrees( 5 ) ).GetValue() );
        ASSERT_TRUE( 10.0f == MoveTowardsAngle( Degrees( 350 ), Degrees( 10 ), Degrees( 30 ) ).GetValue() );
        ASSERT_TRUE( 345.0f == MoveTowardsAngle( Degrees( 350 ), Degrees( 300 ), Degrees( 5 ) ).GetValue() );
        ASSERT_TRUE( fabs( MoveTowardsAngle( Radians( 0.0 ), Radians( 1.0 ), Radians( 0.25 ) ).GetValue() - 0.25 ) < 1e-15 );

        // exact in fixed point: the wrap is a remainder of raw values
        TFixed32Degree fixedDelta = DeltaAngle( TFixed32Degree( TFixed32( 350 ) ), TFixed32Degree( TFixed32( 10 ) ) );
        ASSERT_TRUE( TFixed32( 20 ) == fixedDelta.GetValue() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
