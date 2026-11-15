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

#ifndef GUCEF_MATH_CTQUANTIZER_H
#include "gucefMATH_CTQuantizer.h"
#define GUCEF_MATH_CTQUANTIZER_H
#endif /* GUCEF_MATH_CTQUANTIZER_H ? */

#include "TestRandomStatistics.h"
#include "TestQuantizer.h"

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
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformQuantizerTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Quantizer TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "Quantizer" );

    GUCEF_TESTFW_TESTCASE( "Quantizer: encode and decode agree within half a step for every number of bits, over ranges of both signs" )
        const UInt64 seed = 4711;
        MathReference::CSeededSequence sequence( seed );
        const Float64 ranges[ 4 ][ 2 ] = { { 0.0, 1.0 }, { -1.0, 1.0 }, { -250.0, 1750.0 }, { 100.0, 100.5 } };
        for ( UInt32 r=0; r<4; ++r )
        {
            for ( UInt32 bits=1; bits<=32; ++bits )
            {
                const CTQuantizer< Float64 > q( ranges[ r ][ 0 ], ranges[ r ][ 1 ], bits );
                const Float64 span = ranges[ r ][ 1 ] - ranges[ r ][ 0 ];
                const Float64 step = span / (Float64) q.GetMaxCode();
                ASSERT_TRUE( bits == q.GetBits() && fabs( q.GetStep() - step ) <= step * 1e-15 );
                Float64 worst = 0.0;
                for ( UInt32 i=0; i<2000; ++i )
                {
                    const Float64 value = ranges[ r ][ 0 ] + span * ( (Float64) ( sequence.NextUInt64() >> 11 ) * ldexp( 1.0, -53 ) );
                    const UInt32 code = q.Encode( value );
                    ASSERT_SEEDED( code <= q.GetMaxCode(), seed );
                    worst = fmax( worst, fabs( q.Decode( code ) - value ) );
                }
                // half a step plus the rounding of the arithmetic
                ASSERT_SEEDED( worst <= 0.5 * step * ( 1.0 + 1e-9 ) + span * 1e-15, seed );
            }
        }
        // the Float32 quantizer
        const CTQuantizer< Float32 > q32( -10.0f, 10.0f, 12 );
        const Float32 step32 = 20.0f / 4095.0f;
        for ( UInt32 i=0; i<=1000; ++i )
        {
            const Float32 value = -10.0f + 20.0f * (Float32) i / 1000.0f;
            ASSERT_TRUE( fabs( q32.Decode( q32.Encode( value ) ) - value ) <= 0.5f * step32 * 1.001f );
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Quantizer: the ends of the range map to the first and last code and out of range values clamp" )
        const CTQuantizer< Float64 > q( -2.0, 6.0, 10 );
        ASSERT_TRUE( 0 == q.Encode( -2.0 ) && 1023 == q.Encode( 6.0 ) && 1023 == q.GetMaxCode() );
        ASSERT_TRUE( -2.0 == q.Decode( 0 ) && 6.0 == q.Decode( 1023 ) );
        ASSERT_TRUE( 0 == q.Encode( -1000.0 ) && 1023 == q.Encode( 1000.0 ) );
        ASSERT_TRUE( 0 == q.Encode( -std::numeric_limits< Float64 >::infinity() ) && 1023 == q.Encode( std::numeric_limits< Float64 >::infinity() ) );
        ASSERT_TRUE( 0 == q.Encode( std::numeric_limits< Float64 >::quiet_NaN() ) );
        // codes above the last decode as the maximum
        ASSERT_TRUE( 6.0 == q.Decode( 1024 ) && 6.0 == q.Decode( 0xFFFFFFFFU ) );
        // codes are monotone in the value
        UInt32 previous = 0;
        for ( UInt32 i=0; i<=8000; ++i )
        {
            const UInt32 code = q.Encode( -2.0 + 8.0 * i / 8000.0 );
            ASSERT_TRUE( code >= previous );
            previous = code;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Quantizer: 32 bit precision is accepted over the whole range of codes" )
        const CTQuantizer< Float64 > q( 0.0, 1.0, 32 );
        ASSERT_TRUE( 32 == q.GetBits() && 0xFFFFFFFFU == q.GetMaxCode() );
        ASSERT_TRUE( 0 == q.Encode( 0.0 ) && 0xFFFFFFFFU == q.Encode( 1.0 ) );
        ASSERT_TRUE( 1.0 == q.Decode( 0xFFFFFFFFU ) && 0.0 == q.Decode( 0 ) );
        // 2^31 / ( 2^32 - 1 ) is the value of code 2^31
        const Float64 middle = q.Decode( 0x80000000U );
        ASSERT_TRUE( fabs( middle - 2147483648.0 / 4294967295.0 ) < 1e-15 && 0x80000000U == q.Encode( middle ) );
        for ( UInt32 code=0xFFFFFFF0U; code!=0; ++code )
            ASSERT_TRUE( q.Encode( q.Decode( code ) ) == code );
        const CTQuantizer< Float32 > q32( -1.0f, 1.0f, 32 );
        ASSERT_TRUE( 0 == q32.Encode( -1.0f ) && 0xFFFFFFFFU == q32.Encode( 1.0f ) );
        ASSERT_TRUE( fabs( q32.Decode( q32.Encode( 0.3f ) ) - 0.3f ) < 1e-6f );
        // 33 and more bits and 0 bits are limited
        ASSERT_TRUE( 32 == CTQuantizer< Float64 >( 0.0, 1.0, 40 ).GetBits() && 1 == CTQuantizer< Float64 >( 0.0, 1.0, 0 ).GetBits() );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Quantizer: ties round to the even code" )
        // range 0..3 with 2 bits: step 1, the values 0.5, 1.5, 2.5 are ties
        const CTQuantizer< Float64 > q( 0.0, 3.0, 2 );
        ASSERT_TRUE( 1.0 == q.GetStep() );
        ASSERT_TRUE( 0 == q.Encode( 0.5 ) && 2 == q.Encode( 1.5 ) && 2 == q.Encode( 2.5 ) );
        ASSERT_TRUE( 1 == q.Encode( 0.5000001 ) && 1 == q.Encode( 1.4999999 ) );
        // a single bit: codes 0 and 1 at the ends, the midpoint is a tie to 0
        const CTQuantizer< Float64 > one( 10.0, 20.0, 1 );
        ASSERT_TRUE( 0 == one.Encode( 15.0 ) && 1 == one.Encode( 15.5 ) && 0 == one.Encode( 14.5 ) );
        ASSERT_TRUE( 10.0 == one.Decode( 0 ) && 20.0 == one.Decode( 1 ) );
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Quantizer: reversed and empty ranges, defaults and the vector form" )
        const CTQuantizer< Float64 > reversed( 5.0, -5.0, 8 );
        ASSERT_TRUE( -5.0 == reversed.GetMinimum() && 5.0 == reversed.GetMaximum() && 255 == reversed.Encode( 5.0 ) );
        const CTQuantizer< Float64 > empty( 3.0, 3.0, 8 );
        ASSERT_TRUE( 0.0 == empty.GetStep() && 0 == empty.Encode( 3.0 ) && 0 == empty.Encode( 9.0 ) && 3.0 == empty.Decode( 0 ) && 3.0 == empty.Decode( 200 ) );
        const CTQuantizer< Float32 > defaults;
        ASSERT_TRUE( 0.0f == defaults.GetMinimum() && 1.0f == defaults.GetMaximum() && 8 == defaults.GetBits() && 255 == defaults.GetMaxCode() );
        ASSERT_TRUE( 255 == defaults.Encode( 1.0f ) && 128 == defaults.Encode( 0.5f ) );

        const CTQuantizer< Float32 > q( -100.0f, 100.0f, 16 );
        const TFloat32Vector3D v( -99.5f, 0.25f, 100.0f );
        UInt32 codes[ 3 ];
        q.EncodeVector3( v, codes );
        ASSERT_TRUE( codes[ 0 ] == q.Encode( v.x ) && codes[ 1 ] == q.Encode( v.y ) && 65535 == codes[ 2 ] );
        const TFloat32Vector3D back = q.DecodeVector3( codes );
        const Float32 half = 0.5f * q.GetStep();
        ASSERT_TRUE( fabs( back.x - v.x ) <= half * 1.001f && fabs( back.y - v.y ) <= half * 1.001f && 100.0f == back.z );
        const CTQuantizer< Float64 > q64( -1.0, 1.0, 20 );
        UInt32 codes64[ 3 ];
        q64.EncodeVector3( TFloat64Vector3D( 0.1, -0.7, 0.9999 ), codes64 );
        const TFloat64Vector3D back64 = q64.DecodeVector3( codes64 );
        ASSERT_TRUE( fabs( back64.x - 0.1 ) <= 0.5 * q64.GetStep() * 1.0000001 && fabs( back64.y + 0.7 ) <= 0.5 * q64.GetStep() * 1.0000001 );
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
