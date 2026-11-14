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

#ifndef GUCEF_MATH_CFLOAT16_H
#include "gucefMATH_CFloat16.h"
#define GUCEF_MATH_CFLOAT16_H
#endif /* GUCEF_MATH_CFLOAT16_H ? */

#include "TestFloat16.h"

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


static UInt16 Half( Float32 value, TRoundingMode mode = CORE::ROUNDINGMODE_HALF_EVEN )
{
    return CFloat16::FromFloat32( value, mode ).GetBits();
}

static Float32 FromBits32( UInt32 bits )
{
    Float32 value;
    memcpy( &value, &bits, 4 );
    return value;
}

static UInt32 Bits32( Float32 value )
{
    UInt32 bits;
    memcpy( &bits, &value, 4 );
    return bits;
}

/** Independent decoding of a binary16 pattern with Float64 arithmetic: ( -1 )^s 2^( e - 15 ) ( 1 + m / 1024 ) */
static Float64 ReferenceDecode( UInt16 bits )
{
    Float64 sign = 0 != ( bits & 0x8000 ) ? -1.0 : 1.0;
    UInt32 e = ( bits >> 10 ) & 0x1F;
    UInt32 m = bits & 0x3FF;
    if ( 0 == e )
        return sign * ldexp( (Float64) m, -24 );
    return sign * ldexp( 1024.0 + (Float64) m, (int) e - 25 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformFloat16Tests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Float16" );

    GUCEF_TESTFW_TESTCASE( "Float16: encodings of the acceptance list" )
    try
    {
        ASSERT_TRUE( 2 == sizeof( CFloat16 ) && 0 == CFloat16().GetBits() );
        ASSERT_TRUE( 0x3C00 == Half( 1.0f ) && 0xC000 == Half( -2.0f ) );
        ASSERT_TRUE( 0x7BFF == Half( 65504.0f ) );
        ASSERT_TRUE( 0x7C00 == Half( 65520.0f ) && 0x7C00 == Half( 65520.0f, CORE::ROUNDINGMODE_HALF_UP ) );
        ASSERT_TRUE( 0x7BFF == Half( 65520.0f, CORE::ROUNDINGMODE_TOWARD_ZERO ) );
        ASSERT_TRUE( 0x7BFF == Half( 65519.0f ) );                    // below the midpoint
        ASSERT_TRUE( 0xFBFF == Half( -1e9f, CORE::ROUNDINGMODE_CEILING ) && 0xFC00 == Half( -1e9f, CORE::ROUNDINGMODE_FLOOR ) );
        ASSERT_TRUE( 0x0001 == Half( ldexpf( 1.0f, -24 ) ) );
        ASSERT_TRUE( 0x0000 == Half( ldexpf( 1.0f, -25 ) ) );             // tie between 0 and the smallest subnormal: even
        ASSERT_TRUE( 0x0001 == Half( 3.0f * ldexpf( 1.0f, -26 ) ) );
        ASSERT_TRUE( 0x0001 == Half( ldexpf( 1.0f, -25 ), CORE::ROUNDINGMODE_HALF_UP ) );
        ASSERT_TRUE( 0x0400 == Half( ldexpf( 1.0f, -14 ) ) && 0x03FF == Half( ldexpf( 1023.0f, -24 ) ) );
        ASSERT_TRUE( 0x0400 == Half( ldexpf( 2047.0f, -25 ) ) );          // the largest subnormal rounds up into the normals
        ASSERT_TRUE( 0x8000 == Half( -0.0f ) && 0x0000 == Half( 0.0f ) && 0x8000 == Half( -1e-30f ) );
        ASSERT_TRUE( 0x7C00 == Half( FromBits32( 0x7F800000 ) ) && 0xFC00 == Half( FromBits32( 0xFF800000 ) ) );
        ASSERT_TRUE( 0x3555 == Half( 1.0f / 3.0f ) );

        CFloat16 nan = CFloat16::FromFloat32( FromBits32( 0x7FC00001 ) );
        ASSERT_TRUE( nan.IsNaN() && !nan.IsInfinite() && 0 != ( nan.GetBits() & 0x0200 ) );
        CFloat16 signalling = CFloat16::FromFloat32( FromBits32( 0xFF802000 ) );     // signalling NaN with a payload
        ASSERT_TRUE( signalling.IsNaN() && 0x8000 == ( signalling.GetBits() & 0x8000 ) && 0 != ( signalling.GetBits() & 0x0200 ) );
        ASSERT_TRUE( CFloat16::FromBits( 0x7C00 ).IsInfinite() && !CFloat16::FromBits( 0x7BFF ).IsInfinite() && !CFloat16::FromBits( 0x7BFF ).IsNaN() );
        Float32 back = nan.ToFloat32();
        ASSERT_TRUE( back != back );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Float16: statuses of the checked conversion" )
    try
    {
        CFloat16 out = CFloat16::FromBits( 0x1234 );
        ASSERT_TRUE( MATHSTATUS_OK == CFloat16::FromFloat32( 0.5f, CORE::ROUNDINGMODE_UNNECESSARY, out ) && 0x3800 == out.GetBits() );
        ASSERT_TRUE( MATHSTATUS_ROUNDING_REQUIRED == CFloat16::FromFloat32( 0.1f, CORE::ROUNDINGMODE_UNNECESSARY, out ) && 0x3800 == out.GetBits() );
        ASSERT_TRUE( MATHSTATUS_INEXACT == CFloat16::FromFloat32( 0.1f, CORE::ROUNDINGMODE_HALF_EVEN, out ) && 0x2E66 == out.GetBits() );
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == CFloat16::FromFloat32( 1e6f, CORE::ROUNDINGMODE_HALF_EVEN, out ) && 0x7C00 == out.GetBits() );
        ASSERT_TRUE( MATHSTATUS_INVALID_OPERAND == CFloat16::FromFloat32( 1.0f, (TRoundingMode) 42, out ) && 0x7C00 == out.GetBits() );
        ASSERT_TRUE( 0x3C00 == CFloat16::FromFloat32( 1.0f, (TRoundingMode) 42 ).GetBits() );       // the convenience form falls back to nearest even
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Float16: all 65536 patterns decode exactly and re-encode to themselves" )
    try
    {
        UInt32 failures = 0;
        for ( UInt32 bits=0; bits<65536; ++bits )
        {
            CFloat16 half = CFloat16::FromBits( (UInt16) bits );
            Float32 value = half.ToFloat32();
            CFloat16 again = CFloat16::FromFloat32( value );
            if ( half.IsNaN() )
            {
                if ( !again.IsNaN() || value == value || ( again.GetBits() & 0x8000 ) != ( bits & 0x8000 ) )
                    ++failures;
                continue;
            }
            if ( half.IsInfinite() )
            {
                if ( again.GetBits() != bits || ( Bits32( value ) & 0x7FFFFFFF ) != 0x7F800000 )
                    ++failures;
                continue;
            }
            if ( again.GetBits() != bits || (Float64) value != ReferenceDecode( (UInt16) bits ) )
                ++failures;
            if ( 0x8000 == bits && 0x80000000u != Bits32( value ) )        // negative zero keeps its sign
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Float16: every mode rounds once, checked against the neighbouring halves for 10^5 seeded floats" )
    try
    {
        UInt32 failures = 0;
        UInt64 state = 0x1234567ULL;
        for ( UInt32 i=0; i<100000; ++i )
        {
            state = state * 6364136223846793005ULL + 1442695040888963407ULL;
            // floats in the range of the halves, with random sign
            Float32 value = ldexpf( (Float32) ( ( state >> 40 ) & 0xFFFFFF ) / 16777216.0f, (int) ( ( state >> 20 ) % 44 ) - 27 );
            if ( 0 != ( state & 1 ) )
                value = -value;
            UInt16 down = Half( value, CORE::ROUNDINGMODE_FLOOR );
            UInt16 up = Half( value, CORE::ROUNDINGMODE_CEILING );
            Float64 lo = ReferenceDecode( down );
            Float64 hi = 0x7C00 == ( up & 0x7FFF ) ? 1e300 : ReferenceDecode( up );
            if ( 0x7C00 == ( up & 0x7FFF ) && 0 != ( up & 0x8000 ) )
                hi = -1e300;
            // floor <= value <= ceiling, and the nearest mode picks the closer of the two
            if ( !( lo <= value && value <= hi ) && 0x7C00 != ( down & 0x7FFF ) )
                ++failures;
            UInt16 nearest = Half( value );
            if ( 0x7C00 != ( up & 0x7FFF ) && 0x7C00 != ( down & 0x7FFF ) )
            {
                Float64 dlo = value - lo;
                Float64 dhi = hi - value;
                UInt16 expected = dlo < dhi ? down : ( dhi < dlo ? up : ( 0 == ( down & 1 ) ? down : up ) );
                if ( nearest != expected )
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
}

/*-------------------------------------------------------------------------*/
