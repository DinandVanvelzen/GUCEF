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

#ifndef GUCEF_CORE_CBIGINTEGER_H
#include "gucefCORE_CBigInteger.h"
#define GUCEF_CORE_CBIGINTEGER_H
#endif /* GUCEF_CORE_CBIGINTEGER_H ? */

#ifndef GUCEF_MATH_FIXEDPOINT_H
#include "gucefMATH_fixedpoint.h"
#define GUCEF_MATH_FIXEDPOINT_H
#endif /* GUCEF_MATH_FIXEDPOINT_H ? */

#ifndef GUCEF_MATH_CTNUMERICTRAITSFRACTION_H
#include "gucefMATH_CTNumericTraitsFraction.h"
#define GUCEF_MATH_CTNUMERICTRAITSFRACTION_H
#endif /* GUCEF_MATH_CTNUMERICTRAITSFRACTION_H ? */

#ifndef GUCEF_MATH_CTNUMERICCONVERTER_H
#include "gucefMATH_CTNumericConverter.h"
#define GUCEF_MATH_CTNUMERICCONVERTER_H
#endif /* GUCEF_MATH_CTNUMERICCONVERTER_H ? */

#ifndef GUCEF_MATH_FLOATDECOMPOSITION_H
#include "gucefMATH_FloatDecomposition.h"
#define GUCEF_MATH_FLOATDECOMPOSITION_H
#endif /* GUCEF_MATH_FLOATDECOMPOSITION_H ? */

#include "TestMathReference.h"
#include "TestNumericConversions.h"

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
typedef TMathBigFraction TBigFraction;

/** Converts with the converter and checks value and status in one expression */
template < typename TFrom, typename TTo >
static bool
Converts( const TFrom& in, TRoundingMode mode, const TTo& expected, TMathStatus expectedStatus )
{
    TTo out = expected;
    out = TTo();
    TMathStatus status = CTNumericConverter< TFrom, TTo >::Convert( in, out, mode );
    return status == expectedStatus && out == expected;
}

/*-------------------------------------------------------------------------*/

/** A to B to A gives back the same value (exact conversions only) */
template < typename TA, typename TB >
static bool
RoundTrips( const TA& value )
{
    TB middle = TB();
    TA back = TA();
    if ( MATHSTATUS_OK != CTNumericConverter< TA, TB >::Convert( value, middle, CORE::ROUNDINGMODE_UNNECESSARY ) )
        return false;
    if ( MATHSTATUS_OK != CTNumericConverter< TB, TA >::Convert( middle, back, CORE::ROUNDINGMODE_UNNECESSARY ) )
        return false;
    return back == value;
}

/*-------------------------------------------------------------------------*/

static Float64 Float64FromBits( UInt64 bits ) { Float64 value; memcpy( &value, &bits, 8 ); return value; }
static Float32 Float32FromBits( UInt32 bits ) { Float32 value; memcpy( &value, &bits, 4 ); return value; }

/** Float64 with a random exponent in [ -2^e, 2^e ] and a random 53 bit mantissa */
static Float64 RandomFloat64( MathReference::CSeededSequence& generator, int maxExponent )
{
    Float64 mantissa = (Float64) ( generator.NextUInt64() >> 11 ) * ( 1.0 / 9007199254740992.0 );
    int exponent = (int) ( generator.NextUInt64() % (UInt64) ( 2 * maxExponent + 1 ) ) - maxExponent;
    Float64 value = ldexp( mantissa, exponent );
    return 0 != ( generator.NextUInt64() & 1 ) ? -value : value;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformNumericConversionsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "NumericConversions" );

    GUCEF_TESTFW_TESTCASE( "NumericConversions: Float64 to Float32 with every mode, one rounding" )
    try
    {
        const Float64 x = 1.0 + ldexp( 1.0, -24 );
        const Float32 up = 1.0f + ldexpf( 1.0f, -23 );
        ASSERT_TRUE( ( Converts< Float64, Float32 >( x, CORE::ROUNDINGMODE_HALF_EVEN, 1.0f, MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( ( Converts< Float64, Float32 >( x, CORE::ROUNDINGMODE_HALF_UP, up, MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( ( Converts< Float64, Float32 >( x, CORE::ROUNDINGMODE_CEILING, up, MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( ( Converts< Float64, Float32 >( x, CORE::ROUNDINGMODE_FLOOR, 1.0f, MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( ( Converts< Float64, Float32 >( x, CORE::ROUNDINGMODE_TOWARD_ZERO, 1.0f, MATHSTATUS_INEXACT ) ) );
        Float32 unchanged = 7.0f;
        ASSERT_TRUE( MATHSTATUS_ROUNDING_REQUIRED == ( CTNumericConverter< Float64, Float32 >::Convert( x, unchanged, CORE::ROUNDINGMODE_UNNECESSARY ) ) && 7.0f == unchanged );
        ASSERT_TRUE( ( Converts< Float64, Float32 >( -x, CORE::ROUNDINGMODE_FLOOR, -up, MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( ( Converts< Float64, Float32 >( 0.5, CORE::ROUNDINGMODE_UNNECESSARY, 0.5f, MATHSTATUS_OK ) ) );

        // subnormal Float32 results are rounded at their own binade: 2^-149 * 1.5 is a tie between 1 and 2 units
        ASSERT_TRUE( ( Converts< Float64, Float32 >( ldexp( 1.5, -149 ), CORE::ROUNDINGMODE_HALF_EVEN, Float32FromBits( 2 ), MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( ( Converts< Float64, Float32 >( ldexp( 1.0, -151 ), CORE::ROUNDINGMODE_HALF_EVEN, 0.0f, MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( ( Converts< Float64, Float32 >( ldexp( 1.0, -151 ), CORE::ROUNDINGMODE_CEILING, Float32FromBits( 1 ), MATHSTATUS_INEXACT ) ) );
        // overflow as IEEE per mode
        Float32 out = 0;
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == ( CTNumericConverter< Float64, Float32 >::Convert( 1e39, out, CORE::ROUNDINGMODE_HALF_EVEN ) ) && out > FLT_MAX );
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == ( CTNumericConverter< Float64, Float32 >::Convert( 1e39, out, CORE::ROUNDINGMODE_TOWARD_ZERO ) ) && FLT_MAX == out );
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == ( CTNumericConverter< Float64, Float32 >::Convert( -1e39, out, CORE::ROUNDINGMODE_CEILING ) ) && -FLT_MAX == out );
        // OVERFLOW takes precedence over ROUNDING_REQUIRED, as for every other kind, with the IEEE result of the nearest modes
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == ( CTNumericConverter< Float64, Float32 >::Convert( -1e39, out, CORE::ROUNDINGMODE_UNNECESSARY ) ) && out < -FLT_MAX );
        Int32 i32 = 5;
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == ( CTNumericConverter< Float64, Int32 >::Convert( 3e9 + 0.5, i32, CORE::ROUNDINGMODE_UNNECESSARY ) ) && 2147483647 == i32 );
        ASSERT_TRUE( MATHSTATUS_OK == ( CTNumericConverter< Float64, Float32 >::Convert( -0.0, out, CORE::ROUNDINGMODE_HALF_EVEN ) ) && 0x80000000u == ( *(UInt32*) &out ) );
        ASSERT_TRUE( MATHSTATUS_OK == ( CTNumericConverter< Float64, Float32 >::Convert( MathQuietNaN64(), out, CORE::ROUNDINGMODE_HALF_EVEN ) ) && out != out );

        // seeded: the nearest even mode equals the hardware conversion, the directed modes bracket it
        MathReference::CSeededSequence generator( 404 );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<10000; ++i )
        {
            Float64 value = RandomFloat64( generator, 140 );
            Float32 nearest = 0, down = 0, upper = 0;
            CTNumericConverter< Float64, Float32 >::Convert( value, nearest, CORE::ROUNDINGMODE_HALF_EVEN );
            CTNumericConverter< Float64, Float32 >::Convert( value, down, CORE::ROUNDINGMODE_FLOOR );
            CTNumericConverter< Float64, Float32 >::Convert( value, upper, CORE::ROUNDINGMODE_CEILING );
            if ( nearest != (Float32) value || !( (Float64) down <= value && value <= (Float64) upper ) )
                ++failures;
        }
        ASSERT_TRUE( 0 == failures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NumericConversions: floats to integers and fixed point" )
    try
    {
        ASSERT_TRUE( ( Converts< Float64, Int32 >( 2.5, CORE::ROUNDINGMODE_HALF_EVEN, 2, MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( ( Converts< Float64, Int32 >( 2.5, CORE::ROUNDINGMODE_HALF_UP, 3, MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( ( Converts< Float64, Int32 >( -2.5, CORE::ROUNDINGMODE_HALF_UP, -3, MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( ( Converts< Float64, Int32 >( -2.5, CORE::ROUNDINGMODE_FLOOR, -3, MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( ( Converts< Float64, Int32 >( MathQuietNaN64(), CORE::ROUNDINGMODE_HALF_EVEN, 0, MATHSTATUS_INVALID_OPERAND ) ) );
        ASSERT_TRUE( ( Converts< Float64, Int32 >( 3e9, CORE::ROUNDINGMODE_HALF_EVEN, 2147483647, MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( ( Converts< Float32, Int64 >( -3e30f, CORE::ROUNDINGMODE_HALF_EVEN, (Int64) 0x8000000000000000ULL, MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( ( Converts< Float64, TFixed16_16 >( 0.1, CORE::ROUNDINGMODE_HALF_EVEN, TFixed16_16::FromRaw( 6554 ), MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( ( Converts< Float64, TFixed32_32 >( 0.1, CORE::ROUNDINGMODE_HALF_EVEN, TFixed32_32::FromRaw( 429496730 ), MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( 3 == ToInteger< Int32 >( 2.5, CORE::ROUNDINGMODE_HALF_UP ) && TFixed32( 0.5 ) == ( ToFixed< TFixed32 >( 0.5f, CORE::ROUNDINGMODE_HALF_EVEN ) ) );
        ASSERT_TRUE( 0.5f == ToFloat32( TFixed64( 0.5 ) ) && 0.25 == ToFloat64( TFraction64( 1, 4 ) ) && 3.0 == ToFloat64( 3 ) );
        // between fixed point formats: a shift with the mode
        ASSERT_TRUE( ( Converts< TFixed32_32, TFixed16_16 >( TFixed32_32::FromRaw( 0x18000 ), CORE::ROUNDINGMODE_HALF_EVEN, TFixed16_16::FromRaw( 2 ), MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( ( Converts< TFixed16_16, TFixed32_32 >( TFixed16_16::FromRaw( -3 ), CORE::ROUNDINGMODE_UNNECESSARY, TFixed32_32::FromRaw( -3 * 65536 ), MATHSTATUS_OK ) ) );
        ASSERT_TRUE( ( Converts< TFixed48_16, TFixed16_16 >( TFixed48_16( 40000 ), CORE::ROUNDINGMODE_HALF_EVEN, TFixed16_16::Highest(), MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( ( Converts< UInt64, Int8 >( 200, CORE::ROUNDINGMODE_HALF_EVEN, (Int8) 127, MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( ( Converts< Int64, Float64 >( ( (Int64) 1 << 53 ) + 1, CORE::ROUNDINGMODE_CEILING, 9007199254740994.0, MATHSTATUS_INEXACT ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NumericConversions: fractions and decimals of the acceptance list" )
    try
    {
        ASSERT_TRUE( ( Converts< TFixed16_16, TFraction64 >( TFixed16_16::FromRaw( 1 ), CORE::ROUNDINGMODE_UNNECESSARY, TFraction64( 1, 65536 ), MATHSTATUS_OK ) ) );
        CORE::CDecimal decimal;
        ASSERT_TRUE( MATHSTATUS_OK == ( CTNumericConverter< TFixed16_16, CORE::CDecimal >::Convert( TFixed16_16::FromRaw( 1 ), decimal, CORE::ROUNDINGMODE_UNNECESSARY ) ) );
        ASSERT_TRUE( 16 == decimal.GetScale() && "0.0000152587890625" == decimal.ToString() );
        ASSERT_TRUE( MATHSTATUS_OK == ToDecimal( TFixed16_16::FromRaw( 1 ), 16, CORE::ROUNDINGMODE_UNNECESSARY, decimal ) && "0.0000152587890625" == decimal.ToString() );

        const TFraction64 third( 1, 3 );
        ASSERT_TRUE( 1.0 / 3.0 == ToFloat64( third ) );
        ASSERT_TRUE( 0.33333334f == ToFloat32( third ) );
        ASSERT_TRUE( 0.33333334f == CTNumericTraits< TFraction64 >::ToFloat32( third ) && 1.0 / 3.0 == CTNumericTraits< TFraction64 >::ToFloat64( third ) );

        // T-F10 of the CTFraction design: the correctly rounded quotient, not ( double ) n / d
        const TFraction64 large( 3706778661852469502LL, 239877 );
        ASSERT_TRUE( 15452830666768.676 == ToFloat64( large ) );
        ASSERT_TRUE( 15452830666768.674 == large.ToFloat64() );       // CTFraction itself rounds twice (existing behaviour)

        // CDecimal 1.234567890123 into Q32.32: one rounding of 1234567890123 * 2^32 / 10^12 (pinned value computed with exact rationals)
        TFixed32_32 fixed;
        ASSERT_TRUE( MATHSTATUS_INEXACT == FromDecimal( CORE::CDecimal::Parse( "1.234567890123" ), CORE::ROUNDINGMODE_HALF_EVEN, fixed ) );
        ASSERT_TRUE( 5302428713LL == fixed.GetRaw() );
        ASSERT_TRUE( MATHSTATUS_INEXACT == FromDecimal( CORE::CDecimal::Parse( "1.234567890123" ), CORE::ROUNDINGMODE_FLOOR, fixed ) && 5302428712LL == fixed.GetRaw() );
        // and the CBigInteger reference of acceptance 7
        CORE::CBigInteger q;
        CORE::CBigInteger r;
        CORE::CBigInteger numerator;
        CORE::CBigInteger( (Int64) 1234567890123LL ).ShiftLeft( 32, numerator );
        numerator.DivideRounded( CORE::CBigInteger( (Int64) 1000000000000LL ), CORE::ROUNDINGMODE_HALF_EVEN, q, r );
        Int64 reference = 0;
        ASSERT_TRUE( CORE::BIGINTEGERSTATUS_OK == q.ToInt64( reference ) && reference == 5302428713LL );

        // CDecimal to Float32 is correctly rounded (no double rounding through Float64)
        Float32 f32 = 0;
        ASSERT_TRUE( MATHSTATUS_INEXACT == FromDecimal( CORE::CDecimal::Parse( "0.1" ), CORE::ROUNDINGMODE_HALF_EVEN, f32 ) && 0.1f == f32 );
        TFraction64 fromDecimal;
        ASSERT_TRUE( MATHSTATUS_OK == FromDecimal( CORE::CDecimal::Parse( "-12.250" ), CORE::ROUNDINGMODE_UNNECESSARY, fromDecimal ) && TFraction64( -49, 4 ) == fromDecimal );
        ASSERT_TRUE( MATHSTATUS_OK == ToDecimal( TFraction64( 1, 8 ), 3, CORE::ROUNDINGMODE_UNNECESSARY, decimal ) && "0.125" == decimal.ToString() );
        ASSERT_TRUE( MATHSTATUS_INEXACT == ToDecimal( third, 4, CORE::ROUNDINGMODE_HALF_EVEN, decimal ) && "0.3333" == decimal.ToString() );
        ASSERT_TRUE( MATHSTATUS_INEXACT == ( CTNumericConverter< TFraction64, CORE::CDecimal >::Convert( third, decimal, CORE::ROUNDINGMODE_HALF_EVEN ) ) && 18 == decimal.GetScale() );
        ASSERT_TRUE( MATHSTATUS_OVERFLOW == ToDecimal( 1e30, 0, CORE::ROUNDINGMODE_HALF_EVEN, decimal ) && "0.333333333333333333" == decimal.ToString() );
        ASSERT_TRUE( MATHSTATUS_INVALID_SCALE == ToDecimal( 1.0, 19, CORE::ROUNDINGMODE_HALF_EVEN, decimal ) );
        ASSERT_TRUE( MATHSTATUS_INVALID_OPERAND == ToDecimal( MathQuietNaN64(), 2, CORE::ROUNDINGMODE_HALF_EVEN, decimal ) );
        ASSERT_TRUE( MATHSTATUS_OK == ToDecimal( (Int64) 0x7FFFFFFFFFFFFFFFULL, 0, CORE::ROUNDINGMODE_UNNECESSARY, decimal ) );
        ASSERT_TRUE( MATHSTATUS_OVERFLOW == ToDecimal( (Int64) 0x8000000000000000ULL, 0, CORE::ROUNDINGMODE_UNNECESSARY, decimal ) );
        ASSERT_TRUE( MATHSTATUS_OK == ( CTNumericConverter< Float64, CORE::CDecimal >::Convert( 1e15 + 0.3, decimal, CORE::ROUNDINGMODE_HALF_EVEN ) ) && 2 == decimal.GetScale() );   // the double is 1000000000000000.25
        ASSERT_TRUE( MATHSTATUS_OK == ( CTNumericConverter< CORE::CDecimal, CORE::CDecimal >::Convert( decimal, decimal, CORE::ROUNDINGMODE_HALF_EVEN ) ) );
        Int32 i32 = 0;
        ASSERT_TRUE( MATHSTATUS_INEXACT == ( CTNumericConverter< CORE::CDecimal, Int32 >::Convert( CORE::CDecimal::Parse( "-2.5" ), i32, CORE::ROUNDINGMODE_HALF_EVEN ) ) && -2 == i32 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NumericConversions: Int64 fractions from floats exactly or as the best rational approximation" )
    try
    {
        ASSERT_TRUE( ( Converts< Float64, TFraction64 >( 0.1, CORE::ROUNDINGMODE_UNNECESSARY, TFraction64( 3602879701896397LL, 36028797018963968LL ), MATHSTATUS_OK ) ) );
        ASSERT_TRUE( ( Converts< Float64, TFraction64 >( -1.5e18, CORE::ROUNDINGMODE_UNNECESSARY, TFraction64( -1500000000000000000LL, 1 ), MATHSTATUS_OK ) ) );
        ASSERT_TRUE( ( Converts< Float64, TFraction64 >( 1e19, CORE::ROUNDINGMODE_HALF_EVEN, TFraction64( (Int64) 0x7FFFFFFFFFFFFFFFULL, 1 ), MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( ( Converts< Float64, TFraction64 >( -9223372036854775808.0, CORE::ROUNDINGMODE_UNNECESSARY, TFraction64( (Int64) 0x8000000000000000ULL, 1 ), MATHSTATUS_OK ) ) );
        ASSERT_TRUE( ( Converts< Float64, TFraction64 >( 1e-30, CORE::ROUNDINGMODE_HALF_EVEN, TFraction64( 0, 1 ), MATHSTATUS_INEXACT ) ) );
        ASSERT_TRUE( ( Converts< Float64, TFraction64 >( MathInfinity64(), CORE::ROUNDINGMODE_HALF_EVEN, TFraction64( 0, 1 ), MATHSTATUS_INVALID_OPERAND ) ) );
        TFraction64 unchanged( 5, 7 );
        ASSERT_TRUE( MATHSTATUS_ROUNDING_REQUIRED == ( CTNumericConverter< Float64, TFraction64 >::Convert( 1e-20, unchanged, CORE::ROUNDINGMODE_UNNECESSARY ) ) && TFraction64( 5, 7 ) == unchanged );

        // 1e-20 needs a denominator of 2^119: the best approximation with Int64 terms is within 1 / ( d * dmax ) of the value
        TFraction64 approximation;
        ASSERT_TRUE( MATHSTATUS_INEXACT == ( CTNumericConverter< Float64, TFraction64 >::Convert( 1e-17, approximation, CORE::ROUNDINGMODE_HALF_EVEN ) ) );
        ASSERT_TRUE( approximation.GetDenominator() > 1 && fabs( ToFloat64( approximation ) - 1e-17 ) <= 1e-17 * 1e-15 );
        // pi needs only 2^-51: exact
        ASSERT_TRUE( ( Converts< Float64, TFraction64 >( 3.141592653589793, CORE::ROUNDINGMODE_UNNECESSARY, TFraction64( 884279719003555LL, 281474976710656LL ), MATHSTATUS_OK ) ) );

        // the best approximation agrees with brute force on small bounds: convergents of 0.1 truncated to 2^-60 ...
        MathReference::CSeededSequence generator( 707 );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<10000; ++i )
        {
            Float64 value = RandomFloat64( generator, 80 );
            TFraction64 fraction;
            TMathStatus status = CTNumericConverter< Float64, TFraction64 >::Convert( value, fraction, CORE::ROUNDINGMODE_HALF_EVEN );
            if ( MATHSTATUS_OK == status )
            {
                if ( ToFloat64( fraction ) != value )
                    ++failures;
            }
            else if ( MATHSTATUS_INEXACT == status )
            {
                // an approximation is only used when the exact value does not fit: |value| < 2^-10 or more digits than Int64
                Float64 back = ToFloat64( fraction );
                if ( fabs( back - value ) > fabs( value ) * 1e-12 + ldexp( 1.0, -62 ) )
                    ++failures;
            }
            else if ( 0 == ( status & MATHSTATUS_OVERFLOW ) || fabs( value ) < 9.2e18 )
            {
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

    GUCEF_TESTFW_TESTCASE( "NumericConversions: every exact pair of the table round-trips for 10^4 seeded values" )
    try
    {
        const UInt64 seed = 112358;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<10000; ++i )
        {
            UInt64 r = generator.NextUInt64();
            Float32 f = Float32FromBits( (UInt32) r );
            if ( f == f && !( RoundTrips< Float32, Float64 >( f ) ) )
                ++failures;
            Int64 small = generator.NextSignedWithRandomLength( 53 );
            if ( !( RoundTrips< Int64, Float64 >( small ) ) || !( RoundTrips< Int32, Float64 >( (Int32) small ) ) )
                ++failures;
            if ( !( RoundTrips< Int32, Float32 >( (Int32) ( small >> 30 ) ) ) )
                ++failures;
            TFixed16_16 q16 = TFixed16_16::FromRaw( (Int32) r );
            TFixed32_32 q32 = TFixed32_32::FromRaw( small );
            TFixed48_16 q48 = TFixed48_16::FromRaw( (Int64) r );
            if ( !( RoundTrips< TFixed16_16, Float64 >( q16 ) ) || !( RoundTrips< TFixed16_16, Float32 >( TFixed16_16::FromRaw( (Int32) ( small >> 29 ) ) ) ) )
                ++failures;
            if ( !( RoundTrips< TFixed32_32, Float64 >( q32 ) ) || !( RoundTrips< TFixed48_16, Float64 >( TFixed48_16::FromRaw( small ) ) ) )
                ++failures;
            if ( !( RoundTrips< TFixed16_16, TFraction64 >( q16 ) ) || !( RoundTrips< TFixed32_32, TFraction64 >( TFixed32_32::FromRaw( (Int64) r ) ) ) || !( RoundTrips< TFixed48_16, TFraction64 >( q48 ) ) )
                ++failures;
            // Q16.16 values below 2^9 in magnitude need at most 16 + 3 decimal digits: they fit CDecimal exactly
            if ( !( RoundTrips< TFixed16_16, TFixed32_32 >( q16 ) ) || !( RoundTrips< TFixed16_16, TFixed48_16 >( q16 ) ) || !( RoundTrips< TFixed16_16, CORE::CDecimal >( TFixed16_16::FromRaw( (Int32) ( (Int64) r >> 39 ) ) ) ) )
                ++failures;
            if ( !( RoundTrips< Int64, TFraction64 >( (Int64) r ) ) || !( RoundTrips< UInt8, Int64 >( (UInt8) r ) ) || !( RoundTrips< Int64, CORE::CDecimal >( small ) ) )
                ++failures;
            Float64 d = Float64FromBits( r );
            Int64 m = 0;
            Int32 e = 0;
            if ( DecomposeFloat64( d, m, e ) && e >= -62 && e <= 0 && !( RoundTrips< Float64, TFraction64 >( d ) ) )
                ++failures;
            CORE::CDecimal decimal;
            if ( CORE::DECIMALSTATUS_OK == CORE::CDecimal::FromUnscaled( small, (UInt8) ( r % 19 ), decimal ) )
            {
                if ( !( RoundTrips< CORE::CDecimal, TFraction64 >( decimal ) ) )
                    ++failures;
                CORE::CDecimal back;
                TFraction64 fraction;
                FromDecimal( decimal, CORE::ROUNDINGMODE_UNNECESSARY, fraction );
                if ( MATHSTATUS_OK != ToDecimal( fraction, decimal.GetScale(), CORE::ROUNDINGMODE_UNNECESSARY, back ) || back.GetUnscaledValue() != decimal.GetUnscaledValue() )
                    ++failures;
            }
        }
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "NumericConversions: round trip seed " << seed << " failures " << failures );
        ASSERT_TRUE( 0 == failures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NumericConversions: decimal conversions agree with the independent CDecimal and CBigInteger kernels" )
    try
    {
        const UInt64 seed = 99991;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<10000; ++i )
        {
            TRoundingMode mode = (TRoundingMode) ( generator.NextUInt64() % 8 );
            UInt8 scale = (UInt8) ( generator.NextUInt64() % 19 );

            // floats: CDecimal::FromFloat64 rounds the exact binary value once as well
            Float64 value = RandomFloat64( generator, 40 );
            CORE::CDecimal mine, reference;
            TMathStatus myStatus = ToDecimal( value, scale, mode, mine );
            TMathStatus referenceStatus = (TMathStatus) CORE::CDecimal::FromFloat64( value, scale, mode, reference );
            if ( myStatus != referenceStatus || ( !MathStatusIsError( myStatus ) && ( mine.GetUnscaledValue() != reference.GetUnscaledValue() || mine.GetScale() != reference.GetScale() ) ) )
                ++failures;

            // fixed point: CDecimal::Divide of the raw value by 2^F, the path of section 6.6
            Int64 raw = generator.NextSignedWithRandomLength( 63 );
            CORE::CDecimal rawDecimal;
            CORE::CDecimal::FromInt64( raw, 0, rawDecimal );
            myStatus = ToDecimal( TFixed32_32::FromRaw( raw ), scale, mode, mine );
            referenceStatus = (TMathStatus) rawDecimal.Divide( (Int64) 4294967296LL, CORE::CDecimalContext( scale, mode ), reference );
            if ( myStatus != referenceStatus || ( !MathStatusIsError( myStatus ) && mine.GetUnscaledValue() != reference.GetUnscaledValue() ) )
                ++failures;

            // fractions: CTFraction::ToDecimal
            Int64 n = generator.NextSignedWithRandomLength( 63 );
            Int64 den = generator.NextSignedWithRandomLength( 40 );
            den = den < 0 ? -den : den;
            TFraction64 fraction( n, den );
            myStatus = ToDecimal( fraction, scale, mode, mine );
            referenceStatus = (TMathStatus) fraction.ToDecimal( CORE::CDecimalContext( scale, mode ), reference );
            if ( myStatus != referenceStatus || ( !MathStatusIsError( myStatus ) && mine.GetUnscaledValue() != reference.GetUnscaledValue() ) )
                ++failures;

            // fraction to floats: CBigInteger::RatioToFloat64 / RatioToFloat32, correctly rounded with the mode
            if ( mode != CORE::ROUNDINGMODE_UNNECESSARY )
            {
                Float64 myDouble = 0, referenceDouble = 0;
                Float32 myFloat = 0, referenceFloat = 0;
                CTNumericConverter< TFraction64, Float64 >::Convert( fraction, myDouble, mode );
                CTNumericConverter< TFraction64, Float32 >::Convert( fraction, myFloat, mode );
                CORE::CBigInteger::RatioToFloat64( CORE::CBigInteger( fraction.GetNumerator() ), CORE::CBigInteger( fraction.GetDenominator() ), mode, referenceDouble );
                CORE::CBigInteger::RatioToFloat32( CORE::CBigInteger( fraction.GetNumerator() ), CORE::CBigInteger( fraction.GetDenominator() ), mode, referenceFloat );
                if ( myDouble != referenceDouble || myFloat != referenceFloat )
                    ++failures;
            }

            // decimal to Q16.16 and Float32 against CBigInteger DivideRounded / RatioToFloat32
            CORE::CDecimal source;
            if ( CORE::DECIMALSTATUS_OK == CORE::CDecimal::FromUnscaled( generator.NextSignedWithRandomLength( 40 ), scale, source ) && mode != CORE::ROUNDINGMODE_UNNECESSARY )
            {
                TFixed48_16 fixed;
                FromDecimal( source, mode, fixed );
                CORE::CBigInteger scaled, q, r, power;
                CORE::CBigInteger( source.GetUnscaledValue() ).ShiftLeft( 16, scaled );
                CORE::CBigInteger( 10 ).Pow( source.GetScale(), power );
                scaled.DivideRounded( power, mode, q, r );
                Int64 expected = 0;
                q.ToInt64( expected );
                Float32 myFloat = 0, referenceFloat = 0;
                FromDecimal( source, mode, myFloat );
                CORE::CBigInteger::RatioToFloat32( CORE::CBigInteger( source.GetUnscaledValue() ), power, mode, referenceFloat );
                if ( expected != fixed.GetRaw() || myFloat != referenceFloat )
                    ++failures;
            }
        }
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "NumericConversions: cross check seed " << seed << " failures " << failures );
        ASSERT_TRUE( 0 == failures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NumericConversions: unbounded fractions hold every value exactly and convert out with one rounding" )
    try
    {
        const UInt64 seed = 271828;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<10000; ++i )
        {
            UInt64 r = generator.NextUInt64();
            Float64 d = Float64FromBits( r );
            if ( d == d && d - d == 0.0 && !( RoundTrips< Float64, TBigFraction >( d ) ) )
                ++failures;
            Float32 f = Float32FromBits( (UInt32) r );
            if ( f == f && f - f == 0.0f && !( RoundTrips< Float32, TBigFraction >( f ) ) )
                ++failures;
            if ( !( RoundTrips< TFixed32_32, TBigFraction >( TFixed32_32::FromRaw( (Int64) r ) ) ) || !( RoundTrips< TFixed16_16, TBigFraction >( TFixed16_16::FromRaw( (Int32) r ) ) ) )
                ++failures;
            if ( !( RoundTrips< Int64, TBigFraction >( (Int64) r ) ) || !( RoundTrips< UInt64, TBigFraction >( r ) ) )
                ++failures;
            Int64 n = generator.NextSignedWithRandomLength( 64 );
            Int64 den = generator.NextSignedWithRandomLength( 63 );
            TFraction64 fraction( n, den < 0 ? -den : den );
            if ( !( RoundTrips< TFraction64, TBigFraction >( fraction ) ) )
                ++failures;
            CORE::CDecimal decimal;
            if ( CORE::DECIMALSTATUS_OK == CORE::CDecimal::FromUnscaled( n, (UInt8) ( r % 19 ), decimal ) && !( RoundTrips< CORE::CDecimal, TBigFraction >( decimal ) ) )
                ++failures;

            // out of the unbounded kind: the same result as the Int64 fraction path for values both hold
            TRoundingMode mode = (TRoundingMode) ( r % 7 );
            TBigFraction big;
            CTNumericConverter< TFraction64, TBigFraction >::Convert( fraction, big, mode );
            Float64 viaBig = 0, viaInt64 = 0;
            Float32 viaBig32 = 0, viaInt6432 = 0;
            TFixed48_16 fixedViaBig, fixedViaInt64;
            TMathStatus s1 = CTNumericConverter< TBigFraction, Float64 >::Convert( big, viaBig, mode );
            TMathStatus s2 = CTNumericConverter< TFraction64, Float64 >::Convert( fraction, viaInt64, mode );
            CTNumericConverter< TBigFraction, Float32 >::Convert( big, viaBig32, mode );
            CTNumericConverter< TFraction64, Float32 >::Convert( fraction, viaInt6432, mode );
            TMathStatus s3 = CTNumericConverter< TBigFraction, TFixed48_16 >::Convert( big, fixedViaBig, mode );
            TMathStatus s4 = CTNumericConverter< TFraction64, TFixed48_16 >::Convert( fraction, fixedViaInt64, mode );
            if ( viaBig != viaInt64 || viaBig32 != viaInt6432 || s1 != s2 || s3 != s4 || fixedViaBig != fixedViaInt64 )
                ++failures;
        }
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "NumericConversions: unbounded fraction seed " << seed << " failures " << failures );
        ASSERT_TRUE( 0 == failures );

        // values beyond Int64 terms: exact in the unbounded kind, approximated or saturated in the bounded one
        TBigFraction tiny;
        ASSERT_TRUE( MATHSTATUS_OK == ( CTNumericConverter< Float64, TBigFraction >::Convert( ldexp( 1.0, -1074 ), tiny, CORE::ROUNDINGMODE_UNNECESSARY ) ) );
        ASSERT_TRUE( 1075 == tiny.GetDenominator().GetBitLength() && ldexp( 1.0, -1074 ) == CTNumericTraits< TBigFraction >::ToFloat64( tiny ) );
        TFraction64 bounded;
        ASSERT_TRUE( MATHSTATUS_INEXACT == ( CTNumericConverter< TBigFraction, TFraction64 >::Convert( tiny, bounded, CORE::ROUNDINGMODE_HALF_EVEN ) ) && TFraction64( 0, 1 ) == bounded );
        TBigFraction huge( CORE::CBigInteger::Parse( "-100000000000000000000000000000" ), CORE::CBigInteger( 3 ) );
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == ( CTNumericConverter< TBigFraction, TFraction64 >::Convert( huge, bounded, CORE::ROUNDINGMODE_HALF_EVEN ) ) );
        ASSERT_TRUE( CTNumericTraits< TFraction64 >::Lowest() == bounded );
        TFixed32 fixed;
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == ( CTNumericConverter< TBigFraction, TFixed32 >::Convert( huge, fixed, CORE::ROUNDINGMODE_HALF_EVEN ) ) && TFixed32::Lowest() == fixed );
        TBigFraction third( CORE::CBigInteger::Parse( "100000000000000000000000000001" ), CORE::CBigInteger::Parse( "300000000000000000000000000000" ) );
        ASSERT_TRUE( MATHSTATUS_INEXACT == ( CTNumericConverter< TBigFraction, TFraction64 >::Convert( third, bounded, CORE::ROUNDINGMODE_HALF_EVEN ) ) && TFraction64( 1, 3 ) == bounded );
        CORE::CDecimal decimal;
        ASSERT_TRUE( MATHSTATUS_INEXACT == ( CTNumericConverter< TBigFraction, CORE::CDecimal >::Convert( third, decimal, CORE::ROUNDINGMODE_HALF_EVEN ) ) && "0.333333333333333333" == decimal.ToString() );
        Int64 integer = 0;
        ASSERT_TRUE( MATHSTATUS_INEXACT == ( CTNumericConverter< TBigFraction, Int64 >::Convert( TBigFraction( 7, 2 ), integer, CORE::ROUNDINGMODE_HALF_EVEN ) ) && 4 == integer );
        integer = 9;
        ASSERT_TRUE( MATHSTATUS_ROUNDING_REQUIRED == ( CTNumericConverter< TBigFraction, Int64 >::Convert( TBigFraction( 7, 2 ), integer, CORE::ROUNDINGMODE_UNNECESSARY ) ) && 9 == integer );

        // the harmonic number H(100) of the CTFraction design, summed in the unbounded kind, and its nearest double
        TBigFraction harmonic( 0, 1 );
        for ( Int32 k=1; k<=100; ++k )
            harmonic = harmonic + TBigFraction( 1, k );
        ASSERT_TRUE( "14466636279520351160221518043104131447711/2788815009188499086581352357412492142272" == CTNumericTraits< TBigFraction >::ToString( harmonic ) );
        ASSERT_TRUE( fabs( CTNumericTraits< TBigFraction >::ToFloat64( harmonic ) - 5.187377517639621 ) < 1e-15 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NumericConversions: exact float decomposition" )
    try
    {
        Int64 mantissa = 0;
        Int32 exponent = 0;
        ASSERT_TRUE( DecomposeFloat64( 0.1, mantissa, exponent ) && 3602879701896397LL == mantissa && -55 == exponent );
        ASSERT_TRUE( DecomposeFloat64( -12.0, mantissa, exponent ) && -3 == mantissa && 2 == exponent );
        ASSERT_TRUE( DecomposeFloat64( ldexp( 1.0, -1074 ), mantissa, exponent ) && 1 == mantissa && -1074 == exponent );
        ASSERT_TRUE( DecomposeFloat64( DBL_MAX, mantissa, exponent ) && 9007199254740991LL == mantissa && 971 == exponent );
        ASSERT_TRUE( DecomposeFloat64( -0.0, mantissa, exponent ) && 0 == mantissa && 0 == exponent );
        mantissa = 77;
        ASSERT_FALSE( DecomposeFloat64( MathQuietNaN64(), mantissa, exponent ) || DecomposeFloat64( -MathInfinity64(), mantissa, exponent ) );
        ASSERT_TRUE( 77 == mantissa );
        Int32 mantissa32 = 0;
        ASSERT_TRUE( DecomposeFloat32( 0.1f, mantissa32, exponent ) && 13421773 == mantissa32 && -27 == exponent );
        ASSERT_TRUE( DecomposeFloat32( -ldexpf( 1.0f, -149 ), mantissa32, exponent ) && -1 == mantissa32 && -149 == exponent );
        MathReference::CSeededSequence generator( 4242 );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<10000; ++i )
        {
            Float64 value = Float64FromBits( generator.NextUInt64() );
            if ( !DecomposeFloat64( value, mantissa, exponent ) )
            {
                if ( value == value && value - value == 0.0 )
                    ++failures;
                continue;
            }
            if ( ( 0 != mantissa && 0 == ( mantissa & 1 ) ) || ldexp( (Float64) mantissa, exponent ) != value )
                ++failures;
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
