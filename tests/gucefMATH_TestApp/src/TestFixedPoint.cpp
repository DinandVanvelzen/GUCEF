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

#ifndef GUCEF_MATH_FIXEDPOINT_H
#include "gucefMATH_fixedpoint.h"
#define GUCEF_MATH_FIXEDPOINT_H
#endif /* GUCEF_MATH_FIXEDPOINT_H ? */

#ifndef GUCEF_CORE_CBIGINTEGER_H
#include "gucefCORE_CBigInteger.h"
#define GUCEF_CORE_CBIGINTEGER_H
#endif /* GUCEF_CORE_CBIGINTEGER_H ? */

#include "TestMathReference.h"
#include "TestFixedPoint.h"

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


/**
 *  Independent reference: n / d (d != 0) rounded with 'mode' in plain 64 bit integer arithmetic of the language
 *  (truncating / and %, |n| < 2^62), sharing no code with the kernels under test. Returns whether it was inexact.
 */
static bool
ReferenceRound( Int64 n, Int64 d, UInt32 mode, Int64& q )
{
    bool negative = ( n < 0 ) != ( d < 0 ) && 0 != n;
    Int64 an = n < 0 ? -n : n;
    Int64 ad = d < 0 ? -d : d;
    Int64 k = an / ad;
    Int64 r = an % ad;
    bool increment = false;
    if ( 0 != r )
    {
        int c = 2 * r < ad ? -1 : ( 2 * r == ad ? 0 : 1 );
        switch ( mode )
        {
            case 0: increment = c > 0 || ( 0 == c && 1 == ( k & 1 ) ); break;     // HALF_EVEN
            case 1: increment = c >= 0; break;                                  // HALF_UP
            case 2: increment = c > 0; break;                                   // HALF_DOWN
            case 3: increment = false; break;                                   // TOWARD_ZERO
            case 4: increment = true; break;                                    // AWAY_FROM_ZERO
            case 5: increment = negative; break;                                // FLOOR
            case 6: increment = !negative; break;                               // CEILING
            default: break;
        }
    }
    if ( increment )
        ++k;
    q = negative ? -k : k;
    return 0 != r;
}

/*-------------------------------------------------------------------------*/

/** Checks one named operation result of Q16.16 against the reference quotient: value, saturation and status bits */
static bool
MatchesReference( Int64 exact, bool inexact, TMathStatus status, const TFixed16_16& result )
{
    const Int64 lowest = -(Int64) 0x80000000LL;
    const Int64 highest = (Int64) 0x7FFFFFFFLL;
    bool overflow = exact < lowest || exact > highest;
    Int64 expected = overflow ? ( exact < 0 ? lowest : highest ) : exact;
    TMathStatus expectedStatus = overflow ? ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) : ( inexact ? MATHSTATUS_INEXACT : MATHSTATUS_OK );
    return (Int64) result.GetRaw() == expected && status == expectedStatus;
}

/*-------------------------------------------------------------------------*/

/**
 *  10^6 seeded products and quotients of a format against CBigInteger: the exact product a * b (or scaled dividend
 *  a * 2^F) divided by 2^F (or b) with DivideRounded in the same mode, then range checked. Returns the failure count.
 */
template < typename TStorage, UInt32 F >
static UInt32
CountBigIntegerReferenceMismatches( UInt64 seed, UInt32 count )
{
    typedef CTFixedPoint< TStorage, F > T;
    MathReference::CSeededSequence generator( seed );
    const UInt32 bits = sizeof( TStorage ) * 8;
    CORE::CBigInteger lowest( (Int64) T::Lowest().GetRaw() );
    CORE::CBigInteger highest( (Int64) T::Highest().GetRaw() );
    CORE::CBigInteger scale;
    CORE::CBigInteger( 1 ).ShiftLeft( F, scale );
    UInt32 failures = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        TStorage a = (TStorage) generator.NextSignedWithRandomLength( bits );
        TStorage b = (TStorage) generator.NextSignedWithRandomLength( bits );
        TRoundingMode mode = (TRoundingMode) ( generator.NextUInt64() % 7 );
        for ( UInt32 operation=0; operation<2; ++operation )
        {
            CORE::CBigInteger numerator, divisor, quotient, remainder;
            T result;
            TMathStatus status;
            if ( 0 == operation )
            {
                CORE::CBigInteger( (Int64) a ).Multiply( CORE::CBigInteger( (Int64) b ), numerator );
                divisor = scale;
                status = T::Multiply( T::FromRaw( a ), T::FromRaw( b ), mode, result );
            }
            else
            {
                CORE::CBigInteger( (Int64) a ).ShiftLeft( F, numerator );
                divisor = CORE::CBigInteger( (Int64) b );
                status = T::Divide( T::FromRaw( a ), T::FromRaw( b ), mode, result );
            }
            TMathStatus referenceStatus = (TMathStatus) numerator.DivideRounded( divisor, mode, quotient, remainder );
            bool overflow = quotient.Compare( lowest ) < 0 || quotient.Compare( highest ) > 0;
            CORE::CBigInteger expected = overflow ? ( quotient.IsNegative() ? lowest : highest ) : quotient;
            TMathStatus expectedStatus = overflow ? ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) : referenceStatus;
            if ( 0 != expected.Compare( CORE::CBigInteger( (Int64) result.GetRaw() ) ) || status != expectedStatus )
            {
                if ( ++failures < 5 )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FixedPoint: seed " << seed << " operation " << operation << " " << (Int64) a << " " << (Int64) b << " mode " << (Int32) mode << " gave " << (Int64) result.GetRaw() << " expected " << expected.ToString() );
            }
        }
    }
    return failures;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformFixedPointTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "FixedPoint" );

    GUCEF_TESTFW_TESTCASE( "FixedPoint: size equals the storage, construction and raw access" )
    try
    {
        ASSERT_TRUE( 4 == sizeof( TFixed16_16 ) && 8 == sizeof( TFixed32_32 ) && 8 == sizeof( TFixed48_16 ) );
        TFixed16_16 array[ 10 ];
        ASSERT_TRUE( 40 == sizeof( array ) );
        ASSERT_TRUE( 0 == TFixed32().GetRaw() );
        ASSERT_TRUE( 3 * 65536 == TFixed32( 3 ).GetRaw() && -3 * 65536 == TFixed32( -3 ).GetRaw() );
        ASSERT_TRUE( (Int64) 5 << 32 == TFixed64( 5 ).GetRaw() && -( (Int64) 7 << 16 ) == TFixed48_16( -7 ).GetRaw() );
        ASSERT_TRUE( 123 == TFixed64::FromRaw( 123 ).GetRaw() );
        ASSERT_TRUE( 98304 == TFixed32( 1.5 ).GetRaw() && 0 == TFixed32( MathQuietNaN64() ).GetRaw() );
        ASSERT_TRUE( TFixed32::Highest() == TFixed32( 1e9 ) && TFixed32::Lowest() == TFixed32( -1e9 ) );      // saturating
        ASSERT_TRUE( 2 == TFixed32( ldexp( 3.0, -17 ) ).GetRaw() );     // 1.5 raw units: tie to even gives 2
        ASSERT_TRUE( 2 == TFixed32( ldexp( 5.0, -17 ) ).GetRaw() );     // 2.5 raw units: tie to even gives 2
        TFixed32 x = 2;
        TFixed32 y = x * 2 + 1;
        ASSERT_TRUE( TFixed32( 5 ) == y );
        ASSERT_TRUE( TFixed32( 10 ) == 2 * y );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FixedPoint: Q16.16 products and quotients of the acceptance list" )
    try
    {
        ASSERT_TRUE( TFixed32( 3.375 ) == TFixed32( 1.5 ) * TFixed32( 2.25 ) );
        ASSERT_TRUE( TFixed32( -3.375 ) == TFixed32( -1.5 ) * TFixed32( 2.25 ) );
        ASSERT_TRUE( 1 == ( TFixed32::FromRaw( 1 ) * TFixed32( 0.5 ) ).GetRaw() );       // 0.5 raw: ties away from zero
        ASSERT_TRUE( -1 == ( TFixed32::FromRaw( -1 ) * TFixed32( 0.5 ) ).GetRaw() );
        ASSERT_TRUE( 21845 == ( TFixed32( 1 ) / TFixed32( 3 ) ).GetRaw() );
        ASSERT_TRUE( 43691 == ( TFixed32( 2 ) / TFixed32( 3 ) ).GetRaw() );
        ASSERT_TRUE( -43691 == ( TFixed32( -2 ) / TFixed32( 3 ) ).GetRaw() );            // symmetric under negation
        ASSERT_TRUE( -21845 == ( TFixed32( 1 ) / TFixed32( -3 ) ).GetRaw() );

        TFixed32 out = 9;
        ASSERT_TRUE( MATHSTATUS_DIVISION_BY_ZERO == TFixed32::Divide( TFixed32( 5 ), TFixed32(), CORE::ROUNDINGMODE_HALF_UP, out ) && TFixed32::Highest() == out );
        ASSERT_TRUE( MATHSTATUS_DIVISION_BY_ZERO == TFixed32::Divide( TFixed32( -5 ), TFixed32(), CORE::ROUNDINGMODE_HALF_UP, out ) && TFixed32::Lowest() == out );
        ASSERT_TRUE( MATHSTATUS_DIVISION_BY_ZERO == TFixed32::Divide( TFixed32(), TFixed32(), CORE::ROUNDINGMODE_HALF_UP, out ) && TFixed32() == out );
        ASSERT_TRUE( TFixed64::Highest() == TFixed64::DivideSaturated( TFixed64( 1 ), TFixed64() ) );
        ASSERT_TRUE( TFixed48_16::Lowest() == TFixed48_16::DivideSaturated( TFixed48_16( -1 ), TFixed48_16() ) );

        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == TFixed32::MultiplyChecked( TFixed32( 256 ), TFixed32( 128 ), out ) && TFixed32::Highest() == out );
        ASSERT_TRUE( MATHSTATUS_OK == TFixed32::MultiplyChecked( TFixed32( -256 ), TFixed32( 128 ), out ) && TFixed32::Lowest() == out );    // -32768 fits
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == TFixed32::MultiplyChecked( TFixed32( -256 ), TFixed32( 129 ), out ) && TFixed32::Lowest() == out );
        ASSERT_TRUE( MATHSTATUS_OK == TFixed32::MultiplyChecked( TFixed32( -256 ), TFixed32( 127 ), out ) && TFixed32( -32512 ) == out );
        ASSERT_TRUE( TFixed32::Highest() == TFixed32::MultiplySaturated( TFixed32( 300 ), TFixed32( 300 ) ) );
        ASSERT_TRUE( MATHSTATUS_INEXACT == TFixed32::DivideChecked( TFixed32( 1 ), TFixed32( 3 ), out ) && 21845 == out.GetRaw() );

        out = TFixed32( 7 );
        ASSERT_TRUE( MATHSTATUS_ROUNDING_REQUIRED == TFixed32::Divide( TFixed32( 1 ), TFixed32( 3 ), CORE::ROUNDINGMODE_UNNECESSARY, out ) && TFixed32( 7 ) == out );
        ASSERT_TRUE( MATHSTATUS_OK == TFixed32::Divide( TFixed32( 1 ), TFixed32( 4 ), CORE::ROUNDINGMODE_UNNECESSARY, out ) && TFixed32( 0.25 ) == out );
        ASSERT_TRUE( MATHSTATUS_INVALID_OPERAND == TFixed32::Multiply( TFixed32( 1 ), TFixed32( 3 ), (TRoundingMode) 99, out ) && TFixed32( 0.25 ) == out );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FixedPoint: 10^6 seeded Q16.16 products and quotients equal the exactly rounded result, every mode" )
    try
    {
        const UInt64 seed = 20261113;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<1000000; ++i )
        {
            Int32 a = (Int32) generator.NextSignedWithRandomLength( 32 );
            Int32 b = (Int32) generator.NextSignedWithRandomLength( 32 );
            UInt32 mode = (UInt32) ( generator.NextUInt64() % 7 );
            TFixed16_16 result;
            Int64 exact = 0;
            bool inexact = ReferenceRound( (Int64) a * (Int64) b, 65536, mode, exact );
            TMathStatus status = TFixed16_16::Multiply( TFixed16_16::FromRaw( a ), TFixed16_16::FromRaw( b ), (TRoundingMode) mode, result );
            if ( !MatchesReference( exact, inexact, status, result ) )
            {
                if ( ++failures < 5 )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FixedPoint: seed " << seed << " multiply " << a << " " << b << " mode " << mode << " gave " << result.GetRaw() );
            }
            if ( 1 == mode )
            {
                // the operator rounds half up and wraps
                TFixed16_16 viaOperator = TFixed16_16::FromRaw( a ) * TFixed16_16::FromRaw( b );
                if ( exact >= -(Int64) 0x80000000LL && exact <= 0x7FFFFFFFLL && (Int64) viaOperator.GetRaw() != exact )
                    ++failures;
            }
            inexact = ReferenceRound( (Int64) a * 65536, (Int64) b, mode, exact );
            status = TFixed16_16::Divide( TFixed16_16::FromRaw( a ), TFixed16_16::FromRaw( b ), (TRoundingMode) mode, result );
            if ( !MatchesReference( exact, inexact, status, result ) )
            {
                if ( ++failures < 5 )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FixedPoint: seed " << seed << " divide " << a << " " << b << " mode " << mode << " gave " << result.GetRaw() );
            }
        }
        ASSERT_TRUE( 0 == failures );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FixedPoint: 10^6 seeded products and quotients per format equal the CBigInteger DivideRounded reference" )
    try
    {
        ASSERT_TRUE( 0 == ( CountBigIntegerReferenceMismatches< Int32, 16 >( 1616, 1000000 ) ) );
        ASSERT_TRUE( 0 == ( CountBigIntegerReferenceMismatches< Int64, 32 >( 3232, 1000000 ) ) );
        ASSERT_TRUE( 0 == ( CountBigIntegerReferenceMismatches< Int64, 16 >( 4816, 1000000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FixedPoint: Add and Subtract are exact and saturate on overflow" )
    try
    {
        TFixed32 out;
        ASSERT_TRUE( MATHSTATUS_OK == TFixed32::Add( TFixed32( 1.5 ), TFixed32( 2.25 ), CORE::ROUNDINGMODE_HALF_EVEN, out ) && TFixed32( 3.75 ) == out );
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == TFixed32::Add( TFixed32( 30000 ), TFixed32( 30000 ), CORE::ROUNDINGMODE_HALF_EVEN, out ) && TFixed32::Highest() == out );
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == TFixed32::Subtract( TFixed32( -30000 ), TFixed32( 30000 ), CORE::ROUNDINGMODE_HALF_EVEN, out ) && TFixed32::Lowest() == out );
        ASSERT_TRUE( MATHSTATUS_OK == TFixed32::Subtract( TFixed32( -16384 ), TFixed32( 16384 ), CORE::ROUNDINGMODE_HALF_EVEN, out ) && TFixed32::Lowest() == out );
        TFixed64 out64;
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == TFixed64::Add( TFixed64::Highest(), TFixed64::FromRaw( 1 ), CORE::ROUNDINGMODE_HALF_EVEN, out64 ) && TFixed64::Highest() == out64 );
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == TFixed64::Subtract( TFixed64( 1 ), TFixed64::Lowest(), CORE::ROUNDINGMODE_HALF_EVEN, out64 ) && TFixed64::Highest() == out64 );
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == TFixed64::Subtract( TFixed64::Lowest(), TFixed64( 1 ), CORE::ROUNDINGMODE_HALF_EVEN, out64 ) && TFixed64::Lowest() == out64 );
        ASSERT_TRUE( MATHSTATUS_OK == TFixed64::Subtract( TFixed64( -1 ), TFixed64::Lowest(), CORE::ROUNDINGMODE_HALF_EVEN, out64 ) && TFixed64::Highest() - TFixed64( 1 ) + TFixed64::FromRaw( 1 ) == out64 );
        ASSERT_TRUE( MATHSTATUS_OK == TFixed64::Add( TFixed64::Lowest(), TFixed64::Highest(), CORE::ROUNDINGMODE_HALF_EVEN, out64 ) && -1 == out64.GetRaw() );

        TFixed32 a( 1.25 );
        a += TFixed32( 1 );
        a -= TFixed32( 0.5 );
        a *= TFixed32( 2 );
        a /= TFixed32( 4 );
        ASSERT_TRUE( TFixed32( 0.875 ) == a );
        ASSERT_TRUE( TFixed32( -0.875 ) == -a && a == +a );
        ASSERT_TRUE( TFixed32( 1 ) < TFixed32( 2 ) && TFixed32( 2 ) > TFixed32( 1 ) && TFixed32( 1 ) <= TFixed32( 1 ) && TFixed32( 1 ) >= TFixed32( 1 ) && TFixed32( 1 ) != TFixed32( 2 ) );
        ASSERT_TRUE( TFixed48_16( -1 ) < TFixed48_16( 0.5 ) && TFixed64( 1000000 ) > TFixed64( 999999.9999 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FixedPoint: FromFloat64 with every mode, saturation and invalid input; ToFloat64 and ToFloat32" )
    try
    {
        TFixed32 out = 3;
        ASSERT_TRUE( MATHSTATUS_INEXACT == TFixed32::FromFloat64( 0.1, CORE::ROUNDINGMODE_HALF_EVEN, out ) && 6554 == out.GetRaw() );
        ASSERT_TRUE( MATHSTATUS_INEXACT == TFixed32::FromFloat64( 0.1, CORE::ROUNDINGMODE_FLOOR, out ) && 6553 == out.GetRaw() );
        ASSERT_TRUE( MATHSTATUS_INEXACT == TFixed32::FromFloat64( -0.1, CORE::ROUNDINGMODE_FLOOR, out ) && -6554 == out.GetRaw() );
        ASSERT_TRUE( MATHSTATUS_INEXACT == TFixed32::FromFloat64( -0.1, CORE::ROUNDINGMODE_TOWARD_ZERO, out ) && -6553 == out.GetRaw() );
        ASSERT_TRUE( MATHSTATUS_ROUNDING_REQUIRED == TFixed32::FromFloat64( 0.1, CORE::ROUNDINGMODE_UNNECESSARY, out ) && -6553 == out.GetRaw() );
        ASSERT_TRUE( MATHSTATUS_OK == TFixed32::FromFloat64( -32768.0, CORE::ROUNDINGMODE_UNNECESSARY, out ) && TFixed32::Lowest() == out );
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == TFixed32::FromFloat64( 32768.0, CORE::ROUNDINGMODE_HALF_EVEN, out ) && TFixed32::Highest() == out );
        ASSERT_TRUE( MATHSTATUS_INVALID_OPERAND == TFixed32::FromFloat64( MathQuietNaN64(), CORE::ROUNDINGMODE_HALF_EVEN, out ) && TFixed32() == out );
        ASSERT_TRUE( MATHSTATUS_INVALID_OPERAND == TFixed32::FromFloat64( -MathInfinity64(), CORE::ROUNDINGMODE_HALF_EVEN, out ) && TFixed32() == out );
        TFixed64 out64;
        ASSERT_TRUE( MATHSTATUS_INEXACT == TFixed64::FromFloat64( 1e-300, CORE::ROUNDINGMODE_CEILING, out64 ) && 1 == out64.GetRaw() );
        ASSERT_TRUE( MATHSTATUS_INEXACT == TFixed64::FromFloat64( -1e-300, CORE::ROUNDINGMODE_HALF_EVEN, out64 ) && 0 == out64.GetRaw() );

        ASSERT_TRUE( 1.5 == TFixed32( 1.5 ).ToFloat64() && -32768.0 == TFixed32::Lowest().ToFloat64() );
        ASSERT_TRUE( ldexp( 1.0, -32 ) == TFixed64::FromRaw( 1 ).ToFloat64() );
        // raw 2^53 + 1 is not a Float64: nearest even
        ASSERT_TRUE( ldexp( 1.0, 21 ) == TFixed64::FromRaw( ( (Int64) 1 << 53 ) + 1 ).ToFloat64() );
        // Q32.32 raw 2^24 + 1 + 2^-? : one rounding to Float32 (no double rounding through Float64)
        ASSERT_TRUE( 1.0f == TFixed64( 1 ).ToFloat32() && 0.5f == TFixed32( 0.5 ).ToFloat32() );
        const Int64 doubleRoundingRaw = ( (Int64) 1 << 56 ) + ( (Int64) 1 << 32 ) + 1;    // 2^24 + 1 + 2^-32 in Q32.32
        ASSERT_TRUE( 16777218.0f == TFixed64::FromRaw( doubleRoundingRaw ).ToFloat32() );
        ASSERT_TRUE( 16777216.0f == (Float32) TFixed64::FromRaw( doubleRoundingRaw ).ToFloat64() );   // what a cast of the double would give
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FixedPoint: ToString is the exact decimal value and FromString rounds to nearest even" )
    try
    {
        ASSERT_TRUE( "0.0000152587890625" == TFixed32::FromRaw( 1 ).ToString() );
        ASSERT_TRUE( "-32768" == TFixed32::Lowest().ToString() && "32767.9999847412109375" == TFixed32::Highest().ToString() );
        ASSERT_TRUE( "0.00000000023283064365386962890625" == TFixed64::FromRaw( 1 ).ToString() );
        ASSERT_TRUE( "-2147483648" == TFixed64::Lowest().ToString() && "-140737488355328" == TFixed48_16::Lowest().ToString() );
        TFixed32 value;
        ASSERT_TRUE( value.FromString( "0.1" ) && 6554 == value.GetRaw() );
        ASSERT_TRUE( value.FromString( "-0.1" ) && -6554 == value.GetRaw() );
        ASSERT_TRUE( value.FromString( "+3.375" ) && TFixed32( 3.375 ) == value );
        ASSERT_TRUE( value.FromString( ".5" ) && TFixed32( 0.5 ) == value && value.FromString( "7." ) == false );
        ASSERT_TRUE( value.FromString( "0.00002288818359375" ) && 2 == value.GetRaw() );            // 1.5 raw units: tie to even
        ASSERT_TRUE( value.FromString( "0.00003814697265625" ) && 2 == value.GetRaw() );            // 2.5 raw units: tie to even
        ASSERT_TRUE( value.FromString( "0.000038146972656250001" ) && 3 == value.GetRaw() );        // just above the tie
        ASSERT_TRUE( value.FromString( "-0.00003814697265625" ) && -2 == value.GetRaw() );
        ASSERT_TRUE( value.FromString( "-32768" ) && TFixed32::Lowest() == value );
        ASSERT_TRUE( value.FromString( "32767.99999" ) && TFixed32::Highest() == value );
        ASSERT_FALSE( value.FromString( "32767.99999999" ) );            // rounds to 32768: out of range
        // whole part 2^( 64 - F ) - 1 with a fraction that rounds up: the magnitude must not wrap to 0
        TFixed32 bound( 5 );
        ASSERT_FALSE( bound.FromString( "281474976710655.99999999999" ) || bound.FromString( "-281474976710655.99999999999" ) );
        ASSERT_FALSE( bound.FromString( "32767.999992370605468750001" ) || bound.FromString( "-32768.00000762939453125001" ) || bound.FromString( "32768" ) );
        ASSERT_TRUE( TFixed32( 5 ) == bound );
        ASSERT_TRUE( bound.FromString( "-32768.0000076293945312" ) && TFixed32::Lowest() == bound );        // below half a unit beyond: rounds to the bound
        TFixed64 value64;
        ASSERT_FALSE( value64.FromString( "4294967295.99999999999999" ) || value64.FromString( "2147483648" ) );
        TFixed48_16 value48;
        ASSERT_FALSE( value48.FromString( "281474976710655.99999999999" ) );
        ASSERT_TRUE( value48.FromString( "140737488355327.9999847412109375" ) && TFixed48_16::Highest() == value48 );
        ASSERT_TRUE( value64.FromString( "-2147483648" ) && TFixed64::Lowest() == value64 );
        ASSERT_TRUE( value64.FromString( "0.00000000023283064365386962890625" ) && 1 == value64.GetRaw() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
