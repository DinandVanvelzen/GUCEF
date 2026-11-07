/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include <locale.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_INVALID_ARG_EXCEPTION_H
#include "gucef_invalid_arg_exception.h"
#define GUCEF_INVALID_ARG_EXCEPTION_H
#endif /* GUCEF_INVALID_ARG_EXCEPTION_H ? */

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

#ifndef GUCEF_CORE_CDECIMALACCUMULATOR_H
#include "gucefCORE_CDecimalAccumulator.h"
#define GUCEF_CORE_CDECIMALACCUMULATOR_H
#endif /* GUCEF_CORE_CDECIMALACCUMULATOR_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTDECIMALREFERENCE_H
#include "TestDecimalReference.h"
#define GUCEF_CORE_TESTAPP_TESTDECIMALREFERENCE_H
#endif /* GUCEF_CORE_TESTAPP_TESTDECIMALREFERENCE_H ? */

#include "TestDecimal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace TestDecimalReference;

typedef CORE::CDecimal           CDecimal;
typedef CORE::CDecimalContext    CDecimalContext;
typedef CORE::CDecimalAccumulator CDecimalAccumulator;
typedef CORE::TDecimalStatus     TDecimalStatus;
typedef CORE::TRoundingMode      TRoundingMode;

/*-------------------------------------------------------------------------*/

static const UInt64 g_testSeedDecimal = 0xDEC1AA15EEDULL;

#ifdef GUCEF_DEBUG_MODE
static const UInt32 g_fuzzIterations = 20000;
static const UInt32 g_propertyIterations = 4000;
#else
static const UInt32 g_fuzzIterations = 200000;
static const UInt32 g_propertyIterations = 40000;
#endif

static const TRoundingMode g_allModes[ 8 ] =
{
    CORE::ROUNDINGMODE_HALF_EVEN,
    CORE::ROUNDINGMODE_HALF_UP,
    CORE::ROUNDINGMODE_HALF_DOWN,
    CORE::ROUNDINGMODE_TOWARD_ZERO,
    CORE::ROUNDINGMODE_AWAY_FROM_ZERO,
    CORE::ROUNDINGMODE_FLOOR,
    CORE::ROUNDINGMODE_CEILING,
    CORE::ROUNDINGMODE_UNNECESSARY
};

/** The seven rounding modes of the reference table, in its column order */
static const TRoundingMode g_tableModes[ 7 ] =
{
    CORE::ROUNDINGMODE_HALF_EVEN,
    CORE::ROUNDINGMODE_HALF_UP,
    CORE::ROUNDINGMODE_HALF_DOWN,
    CORE::ROUNDINGMODE_TOWARD_ZERO,
    CORE::ROUNDINGMODE_AWAY_FROM_ZERO,
    CORE::ROUNDINGMODE_FLOOR,
    CORE::ROUNDINGMODE_CEILING
};

/** Reference table, rounding to scale 0 */
struct SRoundingRow
{
    const char* input;
    Int64 expected[ 7 ];
};

static const SRoundingRow g_roundingTable[] =
{
    { "2.5",        {  2,  3,  2,  2,  3,  2,  3 } },
    { "3.5",        {  4,  4,  3,  3,  4,  3,  4 } },
    { "-2.5",       { -2, -3, -2, -2, -3, -3, -2 } },
    { "2.4",        {  2,  2,  2,  2,  3,  2,  3 } },
    { "2.6",        {  3,  3,  3,  2,  3,  2,  3 } },
    { "-2.6",       { -3, -3, -3, -2, -3, -3, -2 } },
    { "2.5000001",  {  3,  3,  3,  2,  3,  2,  3 } },
    { "-0.4",       {  0,  0,  0,  0, -1, -1,  0 } },
    { "7",          {  7,  7,  7,  7,  7,  7,  7 } }
};

static const UInt32 g_roundingTableRows = (UInt32) ( sizeof( g_roundingTable ) / sizeof( g_roundingTable[ 0 ] ) );

/*-------------------------------------------------------------------------*/

static CDecimal
D( const char* text )
{
    return CDecimal::Parse( text );
}

/*-------------------------------------------------------------------------*/

static CDecimal
U( Int64 unscaled, UInt8 scale )
{
    CDecimal result;
    CDecimal::FromUnscaled( unscaled, scale, result );
    return result;
}

/*-------------------------------------------------------------------------*/

static bool
TextIs( const CDecimal& value, const char* text )
{
    return value.ToString() == CORE::CString( text );
}

/*-------------------------------------------------------------------------*/

static bool
IsError( TDecimalStatus status )
{
    return CORE::DecimalStatusIsError( status );
}

/*-------------------------------------------------------------------------*/

static TDecimalStatus
ParseStrict( const char* text, CDecimal& result, UInt32 flags = CORE::DECIMALPARSE_STRICT )
{
    return CDecimal::FromChars( text, (UInt32) strlen( text ), result, flags );
}

/*-------------------------------------------------------------------------*/

static UInt64
Pow10Of( UInt32 k )
{
    UInt64 v = 1;
    for ( UInt32 i=0; i<k; ++i )
        v *= 10;
    return v;
}

/*-------------------------------------------------------------------------*/

/** Random decimal: random digit count, sign and scale, with extra weight on the extremes */
static CDecimal
RandomDecimal( CSplitMix64& rng, UInt8 maxScale = CDecimal::MaxScale, UInt32 maxDigits = 19 )
{
    UInt64 magnitude = 0;
    UInt32 pick = (UInt32) rng.Below( 32 );
    if ( 0 == pick )
    {
        magnitude = 9223372036854775807ULL - rng.Below( 3 );
    }
    else
    if ( 1 == pick )
    {
        magnitude = rng.Below( 3 );
    }
    else
    {
        UInt32 digits = 1 + (UInt32) rng.Below( maxDigits );
        if ( digits >= 19 )
            magnitude = rng.Next() & 0x7FFFFFFFFFFFFFFFULL;
        else
            magnitude = rng.Below( Pow10Of( digits ) );
        if ( 0x8000000000000000ULL == magnitude )
            magnitude = 0;
    }
    if ( magnitude > 9223372036854775807ULL )
        magnitude = 9223372036854775807ULL;
    Int64 unscaled = (Int64) magnitude;
    if ( 0 != rng.Below( 2 ) )
        unscaled = -unscaled;
    return U( unscaled, (UInt8) rng.Below( (UInt64) maxScale + 1 ) );
}

/*-------------------------------------------------------------------------*/

/** The library outcome matches the reference outcome */
static bool
Matches( TDecimalStatus status, const CDecimal& result, const SRefOutcome& expected )
{
    if ( status != expected.status )
        return false;
    if ( IsError( status ) )
        return true;
    return result.GetUnscaledValue() == expected.unscaled && result.GetScale() == expected.scale;
}

/*-------------------------------------------------------------------------*/

static SRefOutcome
RefAdd( const CDecimal& a, const CDecimal& b, bool subtract, UInt8 targetScale, TRoundingMode mode )
{
    UInt8 scale = a.GetScale() > b.GetScale() ? a.GetScale() : b.GetScale();
    CRefInt alignedA = CRefInt::Multiply( UnscaledOf( a ), CRefInt::PowerOfTen( scale - a.GetScale() ) );
    CRefInt alignedB = CRefInt::Multiply( UnscaledOf( b ), CRefInt::PowerOfTen( scale - b.GetScale() ) );
    CRefInt sum = subtract ? CRefInt::Subtract( alignedA, alignedB ) : CRefInt::Add( alignedA, alignedB );
    CRefInt n, d;
    ScaledFraction( sum, (Int32) targetScale - (Int32) scale, n, d );
    return ExpectRounded( n, d, mode, targetScale );
}

/*-------------------------------------------------------------------------*/

static SRefOutcome
RefMultiply( const CDecimal& a, const CDecimal& b, UInt8 targetScale, TRoundingMode mode )
{
    CRefInt product = CRefInt::Multiply( UnscaledOf( a ), UnscaledOf( b ) );
    CRefInt n, d;
    ScaledFraction( product, (Int32) targetScale - (Int32) a.GetScale() - (Int32) b.GetScale(), n, d );
    return ExpectRounded( n, d, mode, targetScale );
}

/*-------------------------------------------------------------------------*/

static SRefOutcome
RefMulDiv( const CDecimal& a, const CDecimal& b, const CDecimal& c, UInt8 targetScale, TRoundingMode mode )
{
    // a * b / c at scale t: a * b * 10^( t + sc - sa - sb ) / c
    Int32 e = (Int32) targetScale + (Int32) c.GetScale() - (Int32) a.GetScale() - (Int32) b.GetScale();
    CRefInt product = CRefInt::Multiply( UnscaledOf( a ), UnscaledOf( b ) );
    CRefInt n = e >= 0 ? CRefInt::Multiply( product, CRefInt::PowerOfTen( (UInt32) e ) ) : product;
    CRefInt d = CRefInt::Abs( UnscaledOf( c ) );
    if ( e < 0 )
        d = CRefInt::Multiply( d, CRefInt::PowerOfTen( (UInt32) -e ) );
    if ( c.IsNegative() )
        n = CRefInt::Negated( n );
    return ExpectRounded( n, d, mode, targetScale );
}

/*-------------------------------------------------------------------------*/

static SRefOutcome
RefDivide( const CDecimal& a, const CDecimal& b, UInt8 targetScale, TRoundingMode mode )
{
    return RefMulDiv( a, U( 1, 0 ), b, targetScale, mode );
}

/*-------------------------------------------------------------------------*/

static SRefOutcome
RefRescale( const CDecimal& a, UInt8 targetScale, TRoundingMode mode )
{
    CRefInt n, d;
    ScaledFraction( UnscaledOf( a ), (Int32) targetScale - (Int32) a.GetScale(), n, d );
    return ExpectRounded( n, d, mode, targetScale );
}

/*-------------------------------------------------------------------------*/

static Int32
RefCompare( const CDecimal& a, const CDecimal& b )
{
    UInt8 scale = a.GetScale() > b.GetScale() ? a.GetScale() : b.GetScale();
    CRefInt alignedA = CRefInt::Multiply( UnscaledOf( a ), CRefInt::PowerOfTen( scale - a.GetScale() ) );
    CRefInt alignedB = CRefInt::Multiply( UnscaledOf( b ), CRefInt::PowerOfTen( scale - b.GetScale() ) );
    return CRefInt::Compare( alignedA, alignedB );
}

/*-------------------------------------------------------------------------*/

/** The digits of a canonical text, for the locale and strtod comparisons */
static CORE::CString
Canonical( const CDecimal& value )
{
    return value.ToString();
}

/*-------------------------------------------------------------------------*/

static void
PerformDecimalConstructionAndTextTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: construction, zero at every scale, one unit at every scale, extremes" )
        try
        {
            CDecimal defaultValue;
            ASSERT_TRUE( 0 == defaultValue.GetUnscaledValue() && 0 == defaultValue.GetScale() );
            ASSERT_TRUE( CDecimal::Zero.IsIdentical( defaultValue ) );
            ASSERT_TRUE( 2 == sizeof( CDecimalContext ) );
            ASSERT_TRUE( sizeof( CDecimal ) <= 16 );

            CDecimal fromInt( (Int32) -42 );
            ASSERT_TRUE( -42 == fromInt.GetUnscaledValue() && 0 == fromInt.GetScale() );
            ASSERT_TRUE( TextIs( CDecimal( (Int32) GUCEF_INT32MIN ), "-2147483648" ) );

            for ( UInt8 scale=0; scale<=CDecimal::MaxScale; ++scale )
            {
                CDecimal zero;
                ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromUnscaled( 0, scale, zero ) );
                ASSERT_TRUE( zero.IsZero() && !zero.IsNegative() && !zero.IsPositive() && 0 == zero.GetSign() );
                ASSERT_TRUE( scale == zero.GetScale() );
                char zeroText[ CDecimal::MaxTextLength + 1 ];
                ASSERT_TRUE( zero.ToChars( zeroText, sizeof( zeroText ) ) > 0 && GUCEF_NULL == strchr( zeroText, '-' ) );

                CDecimal plusUlp;
                CDecimal minusUlp;
                ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromUnscaled( 1, scale, plusUlp ) );
                ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromUnscaled( -1, scale, minusUlp ) );
                ASSERT_TRUE( plusUlp.IsPositive() && minusUlp.IsNegative() && 1 == plusUlp.GetSign() && -1 == minusUlp.GetSign() );
                ASSERT_TRUE( plusUlp.Negate().IsIdentical( minusUlp ) && minusUlp.Abs().IsIdentical( plusUlp ) );

                CDecimal reparsed;
                ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromString( plusUlp.ToString(), reparsed ) && reparsed.IsIdentical( plusUlp ) );
                ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromString( minusUlp.ToString(), reparsed ) && reparsed.IsIdentical( minusUlp ) );
            }
            ASSERT_TRUE( TextIs( U( 1, 18 ), "0.000000000000000001" ) );
            ASSERT_TRUE( TextIs( U( -1, 18 ), "-0.000000000000000001" ) );
            ASSERT_TRUE( TextIs( U( 0, 3 ), "0.000" ) );
            ASSERT_TRUE( TextIs( U( 825, 4 ), "0.0825" ) );
            ASSERT_TRUE( TextIs( U( -500, 2 ), "-5.00" ) );

            ASSERT_TRUE( TextIs( U( GUCEF_INT64MAX, 0 ), "9223372036854775807" ) );
            ASSERT_TRUE( TextIs( U( -GUCEF_INT64MAX, 0 ), "-9223372036854775807" ) );
            ASSERT_TRUE( TextIs( U( GUCEF_INT64MAX, 2 ), "92233720368547758.07" ) );
            ASSERT_TRUE( TextIs( U( -GUCEF_INT64MAX, 6 ), "-9223372036854.775807" ) );
            ASSERT_TRUE( TextIs( U( GUCEF_INT64MAX, 18 ), "9.223372036854775807" ) );
            ASSERT_TRUE( TextIs( U( -GUCEF_INT64MAX, 18 ), "-9.223372036854775807" ) );

            CDecimal sentinel = U( 77, 1 );
            CDecimal result = sentinel;
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == CDecimal::FromUnscaled( GUCEF_INT64MIN, 0, result ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == CDecimal::FromUnscaled( 1, 19, result ) );
            ASSERT_TRUE( ( CORE::DECIMALSTATUS_INVALID_SCALE | CORE::DECIMALSTATUS_INVALID_OPERAND ) == CDecimal::FromUnscaled( GUCEF_INT64MIN, 200, result ) );
            ASSERT_TRUE( result.IsIdentical( sentinel ) );

            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromInt64( 5, 2, result ) && TextIs( result, "5.00" ) && 500 == result.GetUnscaledValue() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromInt64( -5, 0, result ) && TextIs( result, "-5" ) );
            result = sentinel;
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == CDecimal::FromInt64( GUCEF_INT64MAX, 1, result ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == CDecimal::FromInt64( GUCEF_INT64MIN, 0, result ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == CDecimal::FromInt64( 1, 19, result ) );
            ASSERT_TRUE( result.IsIdentical( sentinel ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromInt64( 9, 18, result ) && TextIs( result, "9.000000000000000000" ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == CDecimal::FromInt64( 10, 18, result ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: accessors, digit count, precision, integral and normalized forms" )
        try
        {
            ASSERT_TRUE( 1 == U( 0, 5 ).GetDigitCount() );
            ASSERT_TRUE( 1 == U( -9, 0 ).GetDigitCount() );
            ASSERT_TRUE( 2 == U( 10, 0 ).GetDigitCount() );
            ASSERT_TRUE( 19 == U( GUCEF_INT64MAX, 3 ).GetDigitCount() );
            ASSERT_TRUE( 18 == U( 999999999999999999LL, 0 ).GetDigitCount() );
            ASSERT_TRUE( 19 == U( 1000000000000000000LL, 0 ).GetDigitCount() );

            ASSERT_TRUE( U( 999999999999999999LL, 2 ).FitsPrecision( 18 ) );
            ASSERT_FALSE( U( 1000000000000000000LL, 2 ).FitsPrecision( 18 ) );
            ASSERT_TRUE( U( -GUCEF_INT64MAX, 2 ).FitsPrecision( 19 ) );
            ASSERT_TRUE( U( 0, 0 ).FitsPrecision( 0 ) );
            ASSERT_FALSE( U( 1, 0 ).FitsPrecision( 0 ) );
            ASSERT_TRUE( U( -99, 0 ).FitsPrecision( 2 ) && !U( -100, 0 ).FitsPrecision( 2 ) );

            ASSERT_TRUE( D( "30.00" ).IsIntegral() && D( "7" ).IsIntegral() && D( "0.000" ).IsIntegral() );
            ASSERT_FALSE( D( "30.01" ).IsIntegral() );
            ASSERT_FALSE( D( "-0.5" ).IsIntegral() );

            ASSERT_TRUE( D( "2.2000" ).Normalized().IsIdentical( D( "2.2" ) ) );
            ASSERT_TRUE( D( "30.00" ).Normalized().IsIdentical( D( "30" ) ) );
            ASSERT_TRUE( D( "-1.10" ).Normalized().IsIdentical( D( "-1.1" ) ) );
            ASSERT_TRUE( D( "0.000" ).Normalized().IsIdentical( CDecimal() ) );
            ASSERT_TRUE( D( "100" ).Normalized().IsIdentical( D( "100" ) ) );

            ASSERT_TRUE( U( -GUCEF_INT64MAX, 4 ).Negate().IsIdentical( U( GUCEF_INT64MAX, 4 ) ) );
            ASSERT_TRUE( U( -GUCEF_INT64MAX, 4 ).Abs().IsIdentical( U( GUCEF_INT64MAX, 4 ) ) );
            ASSERT_TRUE( ( -D( "1.25" ) ).IsIdentical( D( "-1.25" ) ) );
            ASSERT_TRUE( ( -CDecimal() ).IsIdentical( CDecimal() ) );

            CDecimalContext ctx( 2, CORE::ROUNDINGMODE_CEILING );
            ASSERT_TRUE( 2 == ctx.GetResultScale() && CORE::ROUNDINGMODE_CEILING == ctx.GetRoundingMode() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: strict grammar table" )
        try
        {
            CDecimal r;
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "0", r ) && r.IsIdentical( U( 0, 0 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "-0.000", r ) && r.IsIdentical( U( 0, 3 ) ) && TextIs( r, "0.000" ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "1.", r ) && r.IsIdentical( U( 1, 0 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( ".5", r ) && r.IsIdentical( U( 5, 1 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "007.50", r ) && r.IsIdentical( U( 750, 2 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "+5", r ) && r.IsIdentical( U( 5, 0 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "-.25", r ) && r.IsIdentical( U( -25, 2 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "12.50", r ) && 2 == r.GetScale() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "9223372036854775807", r ) && r.IsIdentical( U( GUCEF_INT64MAX, 0 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "-9223372036854775807", r ) && r.IsIdentical( U( -GUCEF_INT64MAX, 0 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "0.000000000000000001", r ) && r.IsIdentical( U( 1, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "9.223372036854775807", r ) && r.IsIdentical( U( GUCEF_INT64MAX, 18 ) ) );

            CDecimal sentinel = U( 31337, 3 );
            r = sentinel;
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == ParseStrict( "9223372036854775808", r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == ParseStrict( "-9223372036854775808", r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == ParseStrict( "99999999999999999999999", r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == ParseStrict( "9.223372036854775808", r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == ParseStrict( "0.0000000000000000001", r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == ParseStrict( "1.0000000000000000000", r ) );

            const char* rejected[] = { "", "-", "+", ".", " 1", "1 ", "1,5", "1e5", "NaN", "Inf", "-Inf", "0x10", "1_000",
                                       "--1", "+-1", "1.2.3", "-.", "e5", "1e", "\t1", "1\n", "١", "1.5.", "..5", "5-" };
            for ( UInt32 i=0; i<sizeof( rejected ) / sizeof( rejected[ 0 ] ); ++i )
            {
                bool ok = CORE::DECIMALSTATUS_INVALID_FORMAT == ParseStrict( rejected[ i ], r );
                if ( !ok )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Decimal grammar: expected INVALID_FORMAT for \"" << rejected[ i ] << "\"" );
                ASSERT_TRUE( ok );
            }
            ASSERT_TRUE( r.IsIdentical( sentinel ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_FORMAT == CDecimal::FromChars( GUCEF_NULL, 0, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_FORMAT == CDecimal::FromChars( "12", 0, r ) );

            // the length delimits the text, there need not be a terminator
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromChars( "12.345xyz", 6, r ) && r.IsIdentical( U( 12345, 3 ) ) );

            // a long run of leading zeros
            CORE::CString longText;
            for ( UInt32 i=0; i<1000; ++i )
                longText += '0';
            longText += "1.25";
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromString( longText, r ) && r.IsIdentical( U( 125, 2 ) ) );
            CORE::CString longFraction( "0." );
            for ( UInt32 i=0; i<1000; ++i )
                longFraction += '0';
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == CDecimal::FromString( longFraction, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromString( longFraction, 2, CORE::ROUNDINGMODE_UNNECESSARY, r ) && r.IsIdentical( U( 0, 2 ) ) );

            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromString( "-0", r ) && TextIs( r, "0" ) && !r.IsNegative() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromString( "-0.00", r ) && TextIs( r, "0.00" ) && 2 == r.GetScale() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: parse flags for exponents and surrounding whitespace" )
        try
        {
            const UInt32 exp = CORE::DECIMALPARSE_ALLOW_EXPONENT;
            CDecimal r;
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "1.5e-3", r, exp ) && r.IsIdentical( U( 15, 4 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "2E2", r, exp ) && r.IsIdentical( U( 200, 0 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "1E+2", r, exp ) && r.IsIdentical( U( 100, 0 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "1.5e1", r, exp ) && r.IsIdentical( U( 15, 0 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "1.50e1", r, exp ) && r.IsIdentical( U( 150, 1 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "-1e18", r, exp ) && r.IsIdentical( U( -1000000000000000000LL, 0 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "1e-18", r, exp ) && r.IsIdentical( U( 1, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "0e999999999999", r, exp ) && r.IsIdentical( U( 0, 0 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == ParseStrict( "1e-19", r, exp ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == ParseStrict( "1e19", r, exp ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == ParseStrict( "1e999999999999999999999", r, exp ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_FORMAT == ParseStrict( "1e", r, exp ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_FORMAT == ParseStrict( "1e+", r, exp ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_FORMAT == ParseStrict( "e5", r, exp ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_FORMAT == ParseStrict( "1e5.0", r, exp ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_FORMAT == ParseStrict( "1e5", r ) );

            // rounding form with an exponent: 1.5e-20 at scale 18
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromChars( "1.5e-20", 7, 18, CORE::ROUNDINGMODE_HALF_UP, r, exp ) && r.IsIdentical( U( 0, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromChars( "1.5e-20", 7, 18, CORE::ROUNDINGMODE_CEILING, r, exp ) && r.IsIdentical( U( 1, 18 ) ) );

            const UInt32 ws = CORE::DECIMALPARSE_ALLOW_SURROUNDING_WHITESPACE;
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( " \t1.5 \t", r, ws ) && r.IsIdentical( U( 15, 1 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_FORMAT == ParseStrict( " \t1.5 \t", r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_FORMAT == ParseStrict( "   ", r, ws ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_FORMAT == ParseStrict( "1 .5", r, ws ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_FORMAT == ParseStrict( "- 1", r, ws ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == ParseStrict( "  -2.5e1  ", r, ws | exp ) && r.IsIdentical( U( -25, 0 ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: parsing to a given scale rounds once or pads" )
        try
        {
            CDecimal r;
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromString( "1.005", 2, CORE::ROUNDINGMODE_HALF_EVEN, r ) && r.IsIdentical( U( 100, 2 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromString( "1.005", 2, CORE::ROUNDINGMODE_HALF_UP, r ) && r.IsIdentical( U( 101, 2 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromString( "1.0051", 2, CORE::ROUNDINGMODE_HALF_EVEN, r ) && r.IsIdentical( U( 101, 2 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromString( "1.00500000000000000000000001", 2, CORE::ROUNDINGMODE_HALF_DOWN, r ) && r.IsIdentical( U( 101, 2 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromString( "1.00500000000000000000000000", 2, CORE::ROUNDINGMODE_HALF_DOWN, r ) && r.IsIdentical( U( 100, 2 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromString( "1.2", 4, CORE::ROUNDINGMODE_UNNECESSARY, r ) && r.IsIdentical( U( 12000, 4 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromString( "123.456", 0, CORE::ROUNDINGMODE_FLOOR, r ) && r.IsIdentical( U( 123, 0 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromString( "-123.456", 0, CORE::ROUNDINGMODE_FLOOR, r ) && r.IsIdentical( U( -124, 0 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_ROUNDING_REQUIRED == CDecimal::FromString( "123.456", 2, CORE::ROUNDINGMODE_UNNECESSARY, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromString( "123.4500", 2, CORE::ROUNDINGMODE_UNNECESSARY, r ) && r.IsIdentical( U( 12345, 2 ) ) );

            // more than 18 fractional digits: 0 at scale 18 INEXACT, or one unit for CEILING
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromString( "0.0000000000000000001", 18, CORE::ROUNDINGMODE_HALF_EVEN, r ) && r.IsIdentical( U( 0, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromString( "0.0000000000000000001", 18, CORE::ROUNDINGMODE_CEILING, r ) && r.IsIdentical( U( 1, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromString( "-0.0000000000000000001", 18, CORE::ROUNDINGMODE_CEILING, r ) && r.IsIdentical( U( 0, 18 ) ) && !r.IsNegative() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromString( "-0.0000000000000000001", 18, CORE::ROUNDINGMODE_FLOOR, r ) && r.IsIdentical( U( -1, 18 ) ) );

            // a rounding carry over the limit
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == CDecimal::FromString( "9223372036854775807.5", 0, CORE::ROUNDINGMODE_HALF_UP, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromString( "9223372036854775807.5", 0, CORE::ROUNDINGMODE_HALF_DOWN, r ) && r.IsIdentical( U( GUCEF_INT64MAX, 0 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == CDecimal::FromString( "1", 19, CORE::ROUNDINGMODE_HALF_UP, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == CDecimal::FromString( "1", 2, (TRoundingMode) 42, r ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: ToChars buffer sizes, ToString with a display scale and Parse" )
        try
        {
            CDecimal longest = U( -GUCEF_INT64MAX, 18 );
            char buffer[ 32 ];
            memset( buffer, 'x', sizeof( buffer ) );
            ASSERT_TRUE( 0 == longest.ToChars( buffer, 0 ) );
            ASSERT_TRUE( 0 == longest.ToChars( GUCEF_NULL, 22 ) );
            ASSERT_TRUE( 0 == longest.ToChars( buffer, 21 ) );
            ASSERT_TRUE( 'x' == buffer[ 0 ] );
            ASSERT_TRUE( 21 == longest.ToChars( buffer, 22 ) );
            ASSERT_TRUE( 0 == strcmp( buffer, "-9.223372036854775807" ) );
            ASSERT_TRUE( CDecimal::MaxTextLength == 21 );
            ASSERT_TRUE( 1 == U( 5, 0 ).ToChars( buffer, 2 ) && 0 == strcmp( buffer, "5" ) );
            ASSERT_TRUE( 0 == U( 5, 0 ).ToChars( buffer, 1 ) );
            ASSERT_TRUE( 21 == U( -1, 18 ).ToChars( buffer, sizeof( buffer ) ) && 0 == strcmp( buffer, "-0.000000000000000001" ) );

            CORE::CString text( "unchanged" );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "12.5" ).ToString( 2, CORE::ROUNDINGMODE_UNNECESSARY, text ) && text == "12.50" );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == D( "12.345" ).ToString( 2, CORE::ROUNDINGMODE_HALF_EVEN, text ) && text == "12.34" );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == D( "12.345" ).ToString( 2, CORE::ROUNDINGMODE_HALF_UP, text ) && text == "12.35" );
            text = "unchanged";
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == D( "12.345" ).ToString( 19, CORE::ROUNDINGMODE_HALF_UP, text ) && text == "unchanged" );
            ASSERT_TRUE( CORE::DECIMALSTATUS_ROUNDING_REQUIRED == D( "12.345" ).ToString( 1, CORE::ROUNDINGMODE_UNNECESSARY, text ) && text == "unchanged" );

            ASSERT_TRUE( D( "0.0825" ).IsIdentical( U( 825, 4 ) ) );
            bool threw = false;
            try
            {
                D( "1,5" );
            }
            catch ( const GUCEF::invalid_argument& )
            {
                threw = true;
            }
            ASSERT_TRUE( threw );
            threw = false;
            try
            {
                D( "9223372036854775808" );
            }
            catch ( const std::invalid_argument& )
            {
                threw = true;
            }
            ASSERT_TRUE( threw );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: text round trip fuzz: random value, ToString, FromString gives the identical value" )
        try
        {
            CSplitMix64 rng( g_testSeedDecimal );
            UInt32 failures = 0;
            for ( UInt32 i=0; i<g_fuzzIterations && 0 == failures; ++i )
            {
                CDecimal value = RandomDecimal( rng );
                CORE::CString text = value.ToString();
                CDecimal reparsed;
                TDecimalStatus status = CDecimal::FromString( text, reparsed );
                char chars[ CDecimal::MaxTextLength + 1 ];
                UInt32 length = value.ToChars( chars, sizeof( chars ) );
                CDecimal fromChars;
                TDecimalStatus charsStatus = CDecimal::FromChars( chars, length, fromChars );
                if ( CORE::DECIMALSTATUS_OK != status || !reparsed.IsIdentical( value ) ||
                     CORE::DECIMALSTATUS_OK != charsStatus || !fromChars.IsIdentical( value ) ||
                     CORE::CString( chars, length ) != text || length > CDecimal::MaxTextLength )
                {
                    ++failures;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Decimal text round trip failed for " << text << " iteration " << i << " seed " << g_testSeedDecimal );
                }
            }
            ASSERT_TRUE( 0 == failures );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: grammar fuzz: random and mutated strings parse the same as the reference parser" )
        try
        {
            const UInt64 seed = g_testSeedDecimal + 7;
            CSplitMix64 rng( seed );
            const char alphabet[] = "0123456789.-+e x,";
            UInt32 failures = 0;
            for ( UInt32 i=0; i<g_fuzzIterations && 0 == failures; ++i )
            {
                char text[ 48 ];
                UInt32 length = 0;
                if ( 0 == rng.Below( 2 ) )
                {
                    // grammar shaped: sign, integer digits, point, fraction digits
                    if ( 0 == rng.Below( 3 ) )
                        text[ length++ ] = 0 == rng.Below( 2 ) ? '-' : '+';
                    UInt32 intDigits = (UInt32) rng.Below( 22 );
                    for ( UInt32 n=0; n<intDigits; ++n )
                        text[ length++ ] = (char) ( '0' + rng.Below( 10 ) );
                    if ( 0 != rng.Below( 3 ) )
                    {
                        text[ length++ ] = '.';
                        UInt32 fracDigits = (UInt32) rng.Below( 21 );
                        for ( UInt32 n=0; n<fracDigits; ++n )
                            text[ length++ ] = (char) ( '0' + rng.Below( 10 ) );
                    }
                    // and sometimes a mutation
                    if ( length > 0 && 0 == rng.Below( 4 ) )
                        text[ rng.Below( length ) ] = alphabet[ rng.Below( sizeof( alphabet ) - 1 ) ];
                }
                else
                {
                    length = (UInt32) rng.Below( 12 );
                    for ( UInt32 n=0; n<length; ++n )
                        text[ n ] = alphabet[ rng.Below( sizeof( alphabet ) - 1 ) ];
                }

                CDecimal parsed;
                TDecimalStatus status = CDecimal::FromChars( text, length, parsed );
                SRefOutcome expected = ReferenceParse( text, length );
                if ( !Matches( status, parsed, expected ) )
                {
                    ++failures;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Decimal grammar fuzz mismatch for \"" << CORE::CString( text, length ) << "\" status " << status << " expected " << expected.status << " iteration " << i << " seed " << seed );
                }
            }
            ASSERT_TRUE( 0 == failures );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    {
        // probe for the locale first: a skip must not be caught by the try of the test case
        bool haveLocale = GUCEF_NULL != setlocale( LC_NUMERIC, "de_DE.UTF-8" );
        setlocale( LC_NUMERIC, "C" );

        GUCEF_TESTFW_TESTCASE( "Test: text conversions are locale independent (de_DE uses a decimal comma)" )
            GUCEF_TESTFW_SKIP_IF( !haveLocale, "the de_DE.UTF-8 locale is not installed" );
            try
            {
                setlocale( LC_NUMERIC, "de_DE.UTF-8" );
                CSplitMix64 rng( g_testSeedDecimal + 11 );
                UInt32 failures = 0;
                for ( UInt32 i=0; i<2000; ++i )
                {
                    CDecimal value = RandomDecimal( rng );
                    CDecimal reparsed;
                    if ( CORE::DECIMALSTATUS_OK != CDecimal::FromString( value.ToString(), reparsed ) || !reparsed.IsIdentical( value ) )
                        ++failures;
                }
                CDecimal comma;
                bool commaRejected = CORE::DECIMALSTATUS_INVALID_FORMAT == ParseStrict( "1,5", comma );
                bool pointText = TextIs( D( "1234.56" ), "1234.56" );
                setlocale( LC_NUMERIC, "C" );
                ASSERT_TRUE( 0 == failures );
                ASSERT_TRUE( commaRejected );
                ASSERT_TRUE( pointText );
            }
            catch ( ... )
            {
                setlocale( LC_NUMERIC, "C" );
                ERRORHERE;
            }
        GUCEF_TESTFW_TESTCASE_END
    }
}

/*-------------------------------------------------------------------------*/

static void
PerformDecimalRoundingTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: the rounding reference table for every mode at target scales 0, 2 and 6" )
        try
        {
            const UInt8 targetScales[] = { 0, 2, 6 };
            for ( UInt32 t=0; t<3; ++t )
            {
                UInt8 target = targetScales[ t ];
                for ( UInt32 row=0; row<g_roundingTableRows; ++row )
                {
                    CDecimal input;
                    ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( g_roundingTable[ row ].input ).MovePointLeft( target, input ) );
                    for ( UInt32 m=0; m<7; ++m )
                    {
                        CDecimal rounded;
                        TDecimalStatus status = input.Rescale( target, g_tableModes[ m ], rounded );
                        bool exact = input.IsIntegral() || 0 == strcmp( "7", g_roundingTable[ row ].input );
                        bool ok = !IsError( status ) && rounded.GetScale() == target && rounded.GetUnscaledValue() == g_roundingTable[ row ].expected[ m ] &&
                                  ( exact ? CORE::DECIMALSTATUS_OK == status : CORE::DECIMALSTATUS_INEXACT == status );
                        if ( !ok )
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Rounding table mismatch: input " << g_roundingTable[ row ].input << " target scale " << (UInt32) target << " mode " << (UInt32) g_tableModes[ m ] << " got " << rounded.ToString() );
                        ASSERT_TRUE( ok );
                    }

                    // UNNECESSARY: exact passes, inexact gives ROUNDING_REQUIRED with the result untouched
                    CDecimal sentinel = U( 4242, 1 );
                    CDecimal result = sentinel;
                    TDecimalStatus status = input.Rescale( target, CORE::ROUNDINGMODE_UNNECESSARY, result );
                    if ( 0 == strcmp( "7", g_roundingTable[ row ].input ) )
                        ASSERT_TRUE( CORE::DECIMALSTATUS_OK == status && 7 == result.GetUnscaledValue() );
                    else
                        ASSERT_TRUE( CORE::DECIMALSTATUS_ROUNDING_REQUIRED == status && result.IsIdentical( sentinel ) );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: ties built through every operation round per the table" )
        try
        {
            // the rows 2.5, 3.5 and -2.5 of the reference table
            struct STie { UInt32 row; Int64 numerator; };
            const STie ties[] = { { 0, 5 }, { 1, 7 }, { 2, -5 } };
            for ( UInt32 t=0; t<3; ++t )
            {
                const SRoundingRow& row = g_roundingTable[ ties[ t ].row ];
                CDecimal numerator = U( ties[ t ].numerator, 0 );
                CDecimal half = U( 5, 1 );
                for ( UInt32 m=0; m<7; ++m )
                {
                    CDecimalContext ctx( 0, g_tableModes[ m ] );
                    Int64 expected = row.expected[ m ];
                    CDecimal r;

                    // Multiply: numerator * 0.5
                    ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == numerator.Multiply( half, ctx, r ) && expected == r.GetUnscaledValue() );
                    // Divide: numerator / 2, decimal and integer divisor
                    ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == numerator.Divide( U( 2, 0 ), ctx, r ) && expected == r.GetUnscaledValue() );
                    ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == numerator.Divide( (Int64) 2, ctx, r ) && expected == r.GetUnscaledValue() );
                    // MulDiv: numerator * 1 / 2
                    ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == numerator.MulDiv( U( 1, 0 ), U( 2, 0 ), ctx, r ) && expected == r.GetUnscaledValue() );
                    // Add with context: ( numerator - 0.25 ) / 2 + 0.25 is x.5 at scale 2
                    CDecimal left = U( ties[ t ].numerator * 50 - 25, 2 );
                    ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == left.Add( U( 25, 2 ), ctx, r ) && expected == r.GetUnscaledValue() );
                    ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == left.Subtract( U( -25, 2 ), ctx, r ) && expected == r.GetUnscaledValue() );
                    // PercentOf: 1 percent of 50 * numerator
                    ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == U( ties[ t ].numerator * 50, 0 ).PercentOf( U( 1, 0 ), ctx, r ) && expected == r.GetUnscaledValue() );
                    // FromString with rounding
                    ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromString( row.input, 0, g_tableModes[ m ], r ) && expected == r.GetUnscaledValue() );
                    // FromFloat64: x.5 is exact in binary
                    ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromFloat64( (Float64) ties[ t ].numerator / 2.0, 0, g_tableModes[ m ], r ) && expected == r.GetUnscaledValue() );
                    // ToInt64
                    Int64 integer = 0;
                    ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == D( row.input ).ToInt64( g_tableModes[ m ], integer ) && expected == integer );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: sticky digits beyond 10^19 decide ties in division chains" )
        try
        {
            const CDecimal half18 = U( 500000000000000000LL, 18 );          // 0.5 at scale 18
            const CDecimal one18 = U( 1000000000000000000LL, 18 );          // 1 at scale 18
            const CDecimal oneUlpAbove18 = U( 1000000000000000001LL, 18 );  // 1.000000000000000001
            CDecimal r;

            // scale 36 product rounded to scale 0: an exact tie
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == half18.Multiply( one18, CDecimalContext( 0, CORE::ROUNDINGMODE_HALF_DOWN ), r ) && 0 == r.GetUnscaledValue() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == half18.Multiply( one18, CDecimalContext( 0, CORE::ROUNDINGMODE_HALF_EVEN ), r ) && 0 == r.GetUnscaledValue() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == half18.Multiply( one18, CDecimalContext( 0, CORE::ROUNDINGMODE_HALF_UP ), r ) && 1 == r.GetUnscaledValue() );
            // 0.5000000000000000005: the tie digit is followed by a non-zero digit 18 places further
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == half18.Multiply( oneUlpAbove18, CDecimalContext( 0, CORE::ROUNDINGMODE_HALF_DOWN ), r ) && 1 == r.GetUnscaledValue() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == half18.Multiply( oneUlpAbove18, CDecimalContext( 0, CORE::ROUNDINGMODE_HALF_EVEN ), r ) && 1 == r.GetUnscaledValue() );
            // just below one half
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == U( 499999999999999999LL, 18 ).Multiply( oneUlpAbove18, CDecimalContext( 0, CORE::ROUNDINGMODE_HALF_UP ), r ) && 0 == r.GetUnscaledValue() );

            // MulDiv with an odd divisor first and powers of ten after it: 1.5 * 1 / 3 at scale 0 = 0.5
            const CDecimal oneAndHalf18 = U( 1500000000000000000LL, 18 );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == oneAndHalf18.MulDiv( one18, U( 3, 0 ), CDecimalContext( 0, CORE::ROUNDINGMODE_HALF_EVEN ), r ) && 0 == r.GetUnscaledValue() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == oneAndHalf18.MulDiv( one18, U( 3, 0 ), CDecimalContext( 0, CORE::ROUNDINGMODE_HALF_UP ), r ) && 1 == r.GetUnscaledValue() );
            // 1.500000000000000001 / 3 is just above one half: the remainder of the division by 3 is sticky
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == U( 1500000000000000001LL, 18 ).MulDiv( one18, U( 3, 0 ), CDecimalContext( 0, CORE::ROUNDINGMODE_HALF_DOWN ), r ) && 1 == r.GetUnscaledValue() );
            // 1.499999999999999999 / 3 just below
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == U( 1499999999999999999LL, 18 ).MulDiv( one18, U( 3, 0 ), CDecimalContext( 0, CORE::ROUNDINGMODE_HALF_UP ), r ) && 0 == r.GetUnscaledValue() );

            // an odd divisor with a remainder just below half of it, followed by more divisions
            // 5 / 3 = 1.666..: q1 = 1 r1 = 2 (above half of 3) then 10^k
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == U( 5, 18 ).Divide( U( 3, 0 ), CDecimalContext( 0, CORE::ROUNDINGMODE_HALF_DOWN ), r ) && 0 == r.GetUnscaledValue() );
            // ( 3 * 10^18 / 2 + 1 ) / 3 at scale 0: just above 0.5 after the division by 3 leaves 1/3 behind
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == U( 1500000000000000001LL, 18 ).Divide( U( 3, 0 ), CDecimalContext( 0, CORE::ROUNDINGMODE_HALF_EVEN ), r ) && 1 == r.GetUnscaledValue() );

            // the same cases checked against the reference for every mode
            const CDecimal operands[] = { half18, one18, oneUlpAbove18, oneAndHalf18, U( 1500000000000000001LL, 18 ), U( 1499999999999999999LL, 18 ), U( 3, 0 ), U( 7, 0 ), U( -9, 1 ) };
            const UInt32 count = (UInt32) ( sizeof( operands ) / sizeof( operands[ 0 ] ) );
            for ( UInt32 a=0; a<count; ++a )
                for ( UInt32 b=0; b<count; ++b )
                    for ( UInt32 c=0; c<count; ++c )
                        for ( UInt32 m=0; m<8; ++m )
                        {
                            CDecimalContext ctx( 0, g_allModes[ m ] );
                            TDecimalStatus status = operands[ a ].MulDiv( operands[ b ], operands[ c ], ctx, r );
                            ASSERT_TRUE( Matches( status, r, RefMulDiv( operands[ a ], operands[ b ], operands[ c ], 0, g_allModes[ m ] ) ) );
                        }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: negative results that round to zero are zero without a sign" )
        try
        {
            CDecimal input = D( "-0.004" );
            for ( UInt32 m=0; m<7; ++m )
            {
                CDecimal r;
                TDecimalStatus status = input.Rescale( 2, g_tableModes[ m ], r );
                ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == status );
                if ( CORE::ROUNDINGMODE_FLOOR == g_tableModes[ m ] || CORE::ROUNDINGMODE_AWAY_FROM_ZERO == g_tableModes[ m ] )
                {
                    ASSERT_TRUE( TextIs( r, "-0.01" ) );
                }
                else
                {
                    ASSERT_TRUE( TextIs( r, "0.00" ) && !r.IsNegative() && 0 == r.GetSign() );
                }
            }
            CDecimal r;
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == U( -1, 0 ).Multiply( CDecimal(), r ) && TextIs( r, "0" ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal().Multiply( U( -5, 2 ), r ) && TextIs( r, "0.00" ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == U( -5, 2 ).Add( U( 5, 2 ), r ) && TextIs( r, "0.00" ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == U( -50, 2 ).Add( U( 5, 1 ), CDecimalContext( 3, CORE::ROUNDINGMODE_UNNECESSARY ), r ) && TextIs( r, "0.000" ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == U( -1, 0 ).Divide( U( 3, 0 ), CDecimalContext( 0, CORE::ROUNDINGMODE_HALF_EVEN ), r ) && TextIs( r, "0" ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromFloat64( -0.0001, 2, CORE::ROUNDINGMODE_CEILING, r ) && TextIs( r, "0.00" ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromFloat64( -0.0, 2, CORE::ROUNDINGMODE_CEILING, r ) && TextIs( r, "0.00" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformDecimalArithmeticTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: overflow boundaries at +-9.22e18" )
        try
        {
            CDecimal sentinel = U( 12345, 2 );
            CDecimal r = sentinel;
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == U( GUCEF_INT64MAX, 0 ).Add( U( 1, 0 ), r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == D( "92233720368547758.07" ).Add( D( "0.01" ), r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == U( -GUCEF_INT64MAX, 0 ).Subtract( U( 1, 0 ), r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == U( -GUCEF_INT64MAX, 0 ).Add( U( -1, 0 ), r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == U( GUCEF_INT64MAX, 0 ).Subtract( U( -1, 0 ), r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == D( "92233720368547758.07" ).Rescale( 3, CORE::ROUNDINGMODE_UNNECESSARY, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == U( 3037000500LL, 0 ).Multiply( U( 3037000500LL, 0 ), r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == U( GUCEF_INT64MAX, 0 ).Divide( U( 1, 0 ), CDecimalContext( 1, CORE::ROUNDINGMODE_HALF_EVEN ), r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == U( GUCEF_INT64MAX, 0 ).Multiply( (Int64) 2, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == U( 1, 0 ).Multiply( GUCEF_INT64MIN, r ) );
            ASSERT_TRUE( r.IsIdentical( sentinel ) );

            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == U( GUCEF_INT64MAX - 1, 0 ).Add( U( 1, 0 ), r ) && r.IsIdentical( U( GUCEF_INT64MAX, 0 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == U( -GUCEF_INT64MAX + 1, 0 ).Subtract( U( 1, 0 ), r ) && r.IsIdentical( U( -GUCEF_INT64MAX, 0 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == U( 3037000499LL, 0 ).Multiply( U( 3037000499LL, 0 ), r ) && 9223372030926249001LL == r.GetUnscaledValue() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == U( 3037000499LL, 0 ).Multiply( U( -3037000499LL, 0 ), r ) && -9223372030926249001LL == r.GetUnscaledValue() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal().Multiply( GUCEF_INT64MIN, r ) && r.IsZero() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == U( GUCEF_INT64MAX, 0 ).Divide( U( 1, 0 ), CDecimalContext( 0, CORE::ROUNDINGMODE_HALF_EVEN ), r ) && r.IsIdentical( U( GUCEF_INT64MAX, 0 ) ) );

            // a rounding carry: 6148914691236517205 * 3 / 2 = 9223372036854775807.5
            const CDecimal a = U( 6148914691236517205LL, 0 );
            const CDecimal three = U( 3, 0 );
            const CDecimal two = U( 2, 0 );
            const TRoundingMode overflowing[] = { CORE::ROUNDINGMODE_HALF_EVEN, CORE::ROUNDINGMODE_HALF_UP, CORE::ROUNDINGMODE_AWAY_FROM_ZERO, CORE::ROUNDINGMODE_CEILING };
            const TRoundingMode fitting[] = { CORE::ROUNDINGMODE_TOWARD_ZERO, CORE::ROUNDINGMODE_FLOOR, CORE::ROUNDINGMODE_HALF_DOWN };
            for ( UInt32 m=0; m<4; ++m )
            {
                r = sentinel;
                ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == a.MulDiv( three, two, CDecimalContext( 0, overflowing[ m ] ), r ) && r.IsIdentical( sentinel ) );
            }
            for ( UInt32 m=0; m<3; ++m )
                ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == a.MulDiv( three, two, CDecimalContext( 0, fitting[ m ] ), r ) && r.IsIdentical( U( GUCEF_INT64MAX, 0 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_ROUNDING_REQUIRED == a.MulDiv( three, two, CDecimalContext( 0, CORE::ROUNDINGMODE_UNNECESSARY ), r ) );
            // negative: -9223372036854775807.5, FLOOR is now the overflowing direction
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == a.Negate().MulDiv( three, two, CDecimalContext( 0, CORE::ROUNDINGMODE_FLOOR ), r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == a.Negate().MulDiv( three, two, CDecimalContext( 0, CORE::ROUNDINGMODE_CEILING ), r ) && r.IsIdentical( U( -GUCEF_INT64MAX, 0 ) ) );

            // the intermediate product exceeds 64 bit while the result fits
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == U( GUCEF_INT64MAX, 0 ).MulDiv( U( GUCEF_INT64MAX, 0 ), U( GUCEF_INT64MAX, 0 ), CDecimalContext( 0, CORE::ROUNDINGMODE_UNNECESSARY ), r ) && r.IsIdentical( U( GUCEF_INT64MAX, 0 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == U( GUCEF_INT64MAX, 10 ).Multiply( U( GUCEF_INT64MAX, 10 ), CDecimalContext( 0, CORE::ROUNDINGMODE_FLOOR ), r ) && 850705917302346158LL == r.GetUnscaledValue() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == U( GUCEF_INT64MAX, 9 ).Multiply( U( GUCEF_INT64MAX, 9 ), CDecimalContext( 0, CORE::ROUNDINGMODE_FLOOR ), r ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: scale alignment of sums and the 128 bit path for alignment overflow" )
        try
        {
            CDecimal r;
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "1.5" ).Add( D( "0.25" ), r ) && r.IsIdentical( D( "1.75" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "12.50" ).Add( D( "0.25" ), r ) && r.IsIdentical( D( "12.75" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "0.25" ).Subtract( D( "1.5" ), r ) && r.IsIdentical( D( "-1.25" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "1.10" ).Multiply( D( "2.00" ), r ) && r.IsIdentical( D( "2.2000" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "10.00" ).Multiply( (Int64) 3, r ) && r.IsIdentical( D( "30.00" ) ) );

            // 10 + 0.000000000000000001 needs unscaled 10^19 + 1 at scale 18
            CDecimal sentinel = U( 1, 1 );
            r = sentinel;
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == D( "10" ).Add( D( "0.000000000000000001" ), r ) && r.IsIdentical( sentinel ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == D( "10" ).Add( D( "0.000000000000000001" ), CDecimalContext( 2, CORE::ROUNDINGMODE_HALF_EVEN ), r ) && r.IsIdentical( D( "10.00" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == D( "10" ).Add( D( "0.000000000000000001" ), CDecimalContext( 2, CORE::ROUNDINGMODE_CEILING ), r ) && r.IsIdentical( D( "10.01" ) ) );

            // the smaller scale operand does not fit 64 bit when aligned, the sum does
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "922337203685477581" ).Add( D( "-100.0" ), r ) && r.IsIdentical( D( "922337203685477481.0" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "-100.0" ).Add( D( "922337203685477581" ), r ) && r.IsIdentical( D( "922337203685477481.0" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "922337203685477581" ).Subtract( D( "100.0" ), r ) && r.IsIdentical( D( "922337203685477481.0" ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: numeric comparison across scales, identity, representation order and hashing" )
        try
        {
            ASSERT_TRUE( D( "1.5" ) == D( "1.50" ) && D( "1.50" ) == D( "1.500000000000000000" ) && D( "1.5" ) == D( "1.500000000000000000" ) );
            ASSERT_TRUE( D( "1.49" ) < D( "1.5" ) && D( "1.5" ) > D( "1.49" ) && D( "1.5" ) <= D( "1.50" ) && D( "1.50" ) >= D( "1.5" ) );
            ASSERT_TRUE( D( "0.1" ) < D( "0.10000000000000001" ) );
            ASSERT_TRUE( U( GUCEF_INT64MAX, 18 ) < U( GUCEF_INT64MAX, 0 ) );
            ASSERT_TRUE( U( -GUCEF_INT64MAX, 0 ) < U( -GUCEF_INT64MAX, 18 ) );
            ASSERT_TRUE( U( GUCEF_INT64MAX, 1 ) < D( "922337203685477581" ) );     // the 128 bit alignment path
            ASSERT_TRUE( D( "-0.01" ) < D( "0" ) && D( "0.000" ) == D( "0" ) && D( "-3" ) < D( "-2.99" ) );
            ASSERT_TRUE( 0 == D( "2" ).Compare( D( "2.000" ) ) && -1 == D( "-2" ).Compare( D( "2.000" ) ) && 1 == D( "2.0001" ).Compare( D( "2" ) ) );
            ASSERT_TRUE( D( "1.5" ) != D( "1.51" ) );

            ASSERT_FALSE( D( "1.5" ).IsIdentical( D( "1.50" ) ) );
            ASSERT_TRUE( D( "1.50" ).IsIdentical( U( 150, 2 ) ) );

            CDecimal::RepresentationLess less;
            ASSERT_TRUE( less( D( "1.5" ), D( "1.50" ) ) && !less( D( "1.50" ), D( "1.5" ) ) );
            ASSERT_TRUE( less( D( "1.4" ), D( "1.50" ) ) && !less( D( "1.5" ), D( "1.5" ) ) );

            GUCEF::set< CDecimal > numericSet;
            numericSet.insert( D( "1.5" ) );
            numericSet.insert( D( "1.50" ) );
            numericSet.insert( D( "2" ) );
            ASSERT_TRUE( 2 == numericSet.size() );
            std::set< CDecimal, CDecimal::RepresentationLess > representationSet;
            representationSet.insert( D( "1.5" ) );
            representationSet.insert( D( "1.50" ) );
            representationSet.insert( D( "2" ) );
            ASSERT_TRUE( 3 == representationSet.size() );

            ASSERT_TRUE( D( "1.5" ).GetHash() == D( "1.50" ).GetHash() && D( "1.5" ).GetHash() == D( "1.500000000000000000" ).GetHash() );
            ASSERT_TRUE( D( "0" ).GetHash() == D( "0.000" ).GetHash() && D( "-2.50" ).GetHash() == D( "-2.5" ).GetHash() );
            ASSERT_TRUE( D( "1.5" ).GetHash() != D( "1.6" ).GetHash() && D( "1.5" ).GetHash() != D( "-1.5" ).GetHash() );
            #ifdef GUCEF_STL_HAVE_CXX11_FEATURES
            std::hash< CDecimal > hasher;
            ASSERT_TRUE( hasher( D( "12.50" ) ) == hasher( D( "12.5" ) ) );
            #endif
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: division by zero, tiny by huge, and the integer divisor" )
        try
        {
            CDecimal sentinel = U( 999, 3 );
            CDecimal r = sentinel;
            CDecimalContext ctx( 2, CORE::ROUNDINGMODE_HALF_EVEN );
            ASSERT_TRUE( CORE::DECIMALSTATUS_DIVISION_BY_ZERO == D( "5" ).Divide( D( "0" ), ctx, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_DIVISION_BY_ZERO == D( "0" ).Divide( D( "0.00" ), ctx, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_DIVISION_BY_ZERO == D( "5" ).Divide( (Int64) 0, ctx, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_DIVISION_BY_ZERO == D( "5" ).MulDiv( D( "2" ), D( "0" ), ctx, r ) );
            ASSERT_TRUE( r.IsIdentical( sentinel ) );

            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == D( "100.00" ).Divide( (Int64) 3, ctx, r ) && r.IsIdentical( D( "33.33" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == D( "100.00" ).Divide( D( "3" ), ctx, r ) && r.IsIdentical( D( "33.33" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == D( "2" ).Divide( D( "3" ), ctx, r ) && r.IsIdentical( D( "0.67" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == D( "1" ).Divide( D( "3" ), CDecimalContext( 18, CORE::ROUNDINGMODE_HALF_EVEN ), r ) && TextIs( r, "0.333333333333333333" ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "1" ).Divide( D( "0.008" ), CDecimalContext( 0, CORE::ROUNDINGMODE_UNNECESSARY ), r ) && r.IsIdentical( D( "125" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "-7.5" ).Divide( (Int64) -3, CDecimalContext( 1, CORE::ROUNDINGMODE_UNNECESSARY ), r ) && r.IsIdentical( D( "2.5" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == D( "1" ).Divide( GUCEF_INT64MIN, CDecimalContext( 18, CORE::ROUNDINGMODE_TOWARD_ZERO ), r ) && r.IsIdentical( U( 0, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == D( "1" ).Divide( GUCEF_INT64MIN, CDecimalContext( 18, CORE::ROUNDINGMODE_FLOOR ), r ) && r.IsIdentical( U( -1, 18 ) ) );

            const CDecimal tiny = U( 1, 18 );
            const CDecimal huge = U( GUCEF_INT64MAX, 0 );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == tiny.Divide( huge, CDecimalContext( 18, CORE::ROUNDINGMODE_HALF_EVEN ), r ) && r.IsIdentical( U( 0, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == tiny.Divide( huge, CDecimalContext( 18, CORE::ROUNDINGMODE_TOWARD_ZERO ), r ) && r.IsIdentical( U( 0, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == tiny.Divide( huge, CDecimalContext( 18, CORE::ROUNDINGMODE_FLOOR ), r ) && r.IsIdentical( U( 0, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == tiny.Divide( huge, CDecimalContext( 18, CORE::ROUNDINGMODE_AWAY_FROM_ZERO ), r ) && r.IsIdentical( U( 1, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == tiny.Divide( huge, CDecimalContext( 18, CORE::ROUNDINGMODE_CEILING ), r ) && r.IsIdentical( U( 1, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == tiny.Negate().Divide( huge, CDecimalContext( 18, CORE::ROUNDINGMODE_FLOOR ), r ) && r.IsIdentical( U( -1, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == tiny.Negate().Divide( huge, CDecimalContext( 18, CORE::ROUNDINGMODE_CEILING ), r ) && r.IsIdentical( U( 0, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == tiny.Divide( huge, CDecimalContext( 0, CORE::ROUNDINGMODE_AWAY_FROM_ZERO ), r ) && r.IsIdentical( U( 1, 0 ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: moving the decimal point, percentages, integer factors and conversion to integers" )
        try
        {
            CDecimal r;
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "825" ).MovePointLeft( 4, r ) && r.IsIdentical( D( "0.0825" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == D( "0.0825" ).MovePointLeft( 15, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == D( "1" ).MovePointLeft( 255, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "0.0825" ).MovePointRight( 4, r ) && r.IsIdentical( D( "825" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "0.0825" ).MovePointRight( 2, r ) && r.IsIdentical( D( "8.25" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "0.0825" ).MovePointRight( 6, r ) && r.IsIdentical( D( "82500" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "-1" ).MovePointRight( 18, r ) && r.IsIdentical( U( -1000000000000000000LL, 0 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == D( "1" ).MovePointRight( 19, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == D( "1" ).MovePointRight( 255, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "0.00" ).MovePointRight( 200, r ) && r.IsIdentical( D( "0" ) ) );

            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "200.00" ).PercentOf( D( "5" ), CDecimalContext( 2, CORE::ROUNDINGMODE_UNNECESSARY ), r ) && r.IsIdentical( D( "10.00" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == D( "33.33" ).PercentOf( D( "15" ), CDecimalContext( 2, CORE::ROUNDINGMODE_HALF_EVEN ), r ) && r.IsIdentical( D( "5.00" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "1234.56" ).PercentOf( D( "-12.5" ), CDecimalContext( 3, CORE::ROUNDINGMODE_UNNECESSARY ), r ) && r.IsIdentical( D( "-154.320" ) ) );

            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "-2.50" ).Multiply( (Int64) -4, r ) && r.IsIdentical( D( "10.00" ) ) );

            Int64 integer = 0;
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == D( "12.5" ).ToInt64( CORE::ROUNDINGMODE_HALF_EVEN, integer ) && 12 == integer );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == D( "-12.5" ).ToInt64( CORE::ROUNDINGMODE_HALF_UP, integer ) && -13 == integer );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == U( GUCEF_INT64MAX, 0 ).ToInt64( CORE::ROUNDINGMODE_UNNECESSARY, integer ) && GUCEF_INT64MAX == integer );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "42.000" ).ToInt64( CORE::ROUNDINGMODE_UNNECESSARY, integer ) && 42 == integer );
            integer = 7;
            ASSERT_TRUE( CORE::DECIMALSTATUS_ROUNDING_REQUIRED == D( "42.001" ).ToInt64( CORE::ROUNDINGMODE_UNNECESSARY, integer ) && 7 == integer );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: domain examples as golden values" )
        try
        {
            CDecimal r;
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "0.0825" ).Multiply( D( "123.456789" ), r ) && r.IsIdentical( D( "10.1851850925" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == D( "0.0825" ).Multiply( D( "123.456789" ), CDecimalContext( 2, CORE::ROUNDINGMODE_HALF_EVEN ), r ) && r.IsIdentical( D( "10.19" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "10.19" ).Multiply( D( "0.92345678" ), r ) && r.IsIdentical( D( "9.4100245882" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == D( "10.19" ).Multiply( D( "0.92345678" ), CDecimalContext( 2, CORE::ROUNDINGMODE_HALF_EVEN ), r ) && r.IsIdentical( D( "9.41" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "1234.56" ).MulDiv( D( "10.5" ), D( "31.5" ), CDecimalContext( 2, CORE::ROUNDINGMODE_HALF_EVEN ), r ) && r.IsIdentical( D( "411.52" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == D( "100.00" ).Divide( D( "3" ), CDecimalContext( 2, CORE::ROUNDINGMODE_HALF_EVEN ), r ) && r.IsIdentical( D( "33.33" ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: results may alias the operands" )
        try
        {
            CDecimal a = D( "1.25" );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == a.Add( D( "0.5" ), a ) && a.IsIdentical( D( "1.75" ) ) );
            CDecimal b = D( "0.25" );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == D( "1.5" ).Add( b, b ) && b.IsIdentical( D( "1.75" ) ) );
            a = D( "2.5" );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == a.MulDiv( a, a, CDecimalContext( 1, CORE::ROUNDINGMODE_UNNECESSARY ), a ) && a.IsIdentical( D( "2.5" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == a.Multiply( a, a ) && a.IsIdentical( D( "6.25" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == a.Subtract( a, a ) && a.IsIdentical( D( "0.00" ) ) );
            a = D( "3.3" );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == a.Divide( a, CDecimalContext( 0, CORE::ROUNDINGMODE_UNNECESSARY ), a ) && a.IsIdentical( D( "1" ) ) );
            a = D( "3.3" );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == a.Add( a, CDecimalContext( 2, CORE::ROUNDINGMODE_UNNECESSARY ), a ) && a.IsIdentical( D( "6.60" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == a.Rescale( 1, CORE::ROUNDINGMODE_UNNECESSARY, a ) && a.IsIdentical( D( "6.6" ) ) );
            CDecimal parts[ 2 ] = { D( "10.00" ), D( "0" ) };
            const CDecimal weights[ 2 ] = { D( "1" ), D( "1" ) };
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::Allocate( parts[ 0 ], weights, 2, parts ) && parts[ 0 ].IsIdentical( D( "5.00" ) ) && parts[ 1 ].IsIdentical( D( "5.00" ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: strong guarantee: every error status leaves the result unchanged" )
        try
        {
            const CDecimal sentinel = U( -123456789, 7 );
            const CDecimalContext badScale( 19, CORE::ROUNDINGMODE_HALF_EVEN );
            const CDecimalContext badMode( 2, (TRoundingMode) 99 );
            const CDecimalContext exact( 0, CORE::ROUNDINGMODE_UNNECESSARY );
            const CDecimal big = U( GUCEF_INT64MAX, 0 );
            UInt32 checked = 0;
            for ( UInt32 op=0; op<26; ++op )
            {
                CDecimal r = sentinel;
                TDecimalStatus status = CORE::DECIMALSTATUS_OK;
                switch ( op )
                {
                    case 0: status = big.Add( U( 1, 0 ), r ); break;
                    case 1: status = big.Add( U( 1, 1 ), r ); break;
                    case 2: status = big.Add( U( 1, 0 ), badScale, r ); break;
                    case 3: status = big.Subtract( U( -1, 0 ), r ); break;
                    case 4: status = big.Subtract( U( 1, 0 ), badMode, r ); break;
                    case 5: status = big.Multiply( U( 2, 0 ), r ); break;
                    case 6: status = U( 1, 10 ).Multiply( U( 1, 9 ), r ); break;
                    case 7: status = big.Multiply( U( 2, 0 ), exact, r ); break;
                    case 8: status = big.Multiply( (Int64) -2, r ); break;
                    case 9: status = big.Divide( U( 0, 0 ), exact, r ); break;
                    case 10: status = U( 1, 0 ).Divide( U( 3, 0 ), exact, r ); break;
                    case 11: status = big.Divide( (Int64) 0, exact, r ); break;
                    case 12: status = big.MulDiv( big, U( 1, 0 ), exact, r ); break;
                    case 13: status = big.PercentOf( U( 1000, 0 ), exact, r ); break;
                    case 14: status = big.Rescale( 1, CORE::ROUNDINGMODE_HALF_EVEN, r ); break;
                    case 15: status = U( 15, 1 ).Rescale( 0, CORE::ROUNDINGMODE_UNNECESSARY, r ); break;
                    case 16: status = U( 1, 0 ).Rescale( 19, CORE::ROUNDINGMODE_HALF_EVEN, r ); break;
                    case 17: status = U( 1, 10 ).MovePointLeft( 9, r ); break;
                    case 18: status = big.MovePointRight( 1, r ); break;
                    case 19: status = CDecimal::FromString( "abc", r ); break;
                    case 20: status = CDecimal::FromString( "1.5", 0, CORE::ROUNDINGMODE_UNNECESSARY, r ); break;
                    case 21: status = CDecimal::FromFloat64( 1e300, 0, CORE::ROUNDINGMODE_HALF_EVEN, r ); break;
                    case 22: status = CDecimal::FromFloat64Shortest( 1e-30, r ); break;
                    case 23: status = CDecimal::FromInt64( GUCEF_INT64MIN, 0, r ); break;
                    case 24: { CDecimal values[ 2 ] = { big, big }; status = CDecimal::Sum( values, 2, r ); break; }
                    case 25: { CDecimal values[ 2 ] = { big, U( 1, 0 ) }; status = CDecimal::Sum( values, 2, exact, r ); break; }
                }
                ASSERT_TRUE( IsError( status ) );
                ASSERT_TRUE( r.IsIdentical( sentinel ) );
                ++checked;
            }
            ASSERT_TRUE( 26 == checked );

            CDecimal parts[ 2 ] = { sentinel, sentinel };
            const CDecimal negativeWeights[ 2 ] = { D( "1" ), D( "-1" ) };
            const CDecimal zeroWeights[ 2 ] = { D( "0" ), D( "0.0" ) };
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == CDecimal::Allocate( D( "10" ), negativeWeights, 2, parts ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_DIVISION_BY_ZERO == CDecimal::Allocate( D( "10" ), zeroWeights, 2, parts ) );
            ASSERT_TRUE( parts[ 0 ].IsIdentical( sentinel ) && parts[ 1 ].IsIdentical( sentinel ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: invalid contexts are reported" )
        try
        {
            CDecimal r;
            const CDecimalContext badScale( 19, CORE::ROUNDINGMODE_HALF_EVEN );
            const CDecimalContext badMode( 2, (TRoundingMode) 8 );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == D( "1" ).Add( D( "1" ), badScale, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == D( "1" ).Multiply( D( "1" ), badScale, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == D( "1" ).Divide( D( "1" ), badScale, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == D( "1" ).MulDiv( D( "1" ), D( "1" ), badScale, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == D( "1" ).PercentOf( D( "1" ), badScale, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == D( "1" ).Add( D( "1" ), badMode, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == D( "1" ).Rescale( 3, (TRoundingMode) 200, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == CDecimal::FromFloat64( 1.0, 2, (TRoundingMode) 9, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == D( "1.5" ).Multiply( D( "0.000000000000000001" ), r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == D( "1.5" ).Multiply( D( "0.000000000000000001" ), CDecimalContext( 18, CORE::ROUNDINGMODE_HALF_EVEN ), r ) && r.IsIdentical( U( 2, 18 ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformDecimalSumAndAllocateTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: Allocate splits a total exactly by weights" )
        try
        {
            CDecimal parts[ 4 ];
            const CDecimal equal[ 3 ] = { D( "1" ), D( "1" ), D( "1" ) };
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::Allocate( D( "100.00" ), equal, 3, parts ) );
            ASSERT_TRUE( parts[ 0 ].IsIdentical( D( "33.34" ) ) && parts[ 1 ].IsIdentical( D( "33.33" ) ) && parts[ 2 ].IsIdentical( D( "33.33" ) ) );

            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::Allocate( D( "-100.00" ), equal, 3, parts ) );
            ASSERT_TRUE( parts[ 0 ].IsIdentical( D( "-33.34" ) ) && parts[ 1 ].IsIdentical( D( "-33.33" ) ) && parts[ 2 ].IsIdentical( D( "-33.33" ) ) );

            const CDecimal withZero[ 3 ] = { D( "0" ), D( "1" ), D( "1" ) };
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::Allocate( D( "0.05" ), withZero, 3, parts ) );
            ASSERT_TRUE( parts[ 0 ].IsIdentical( D( "0.00" ) ) && parts[ 1 ].IsIdentical( D( "0.03" ) ) && parts[ 2 ].IsIdentical( D( "0.02" ) ) );

            // mixed weight scales: 0.5 and 1.25 and 2 aligned to scale 2 are 50, 125 and 200 of 375: three equal remainders
            const CDecimal mixed[ 3 ] = { D( "0.5" ), D( "1.25" ), D( "2" ) };
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::Allocate( D( "10.00" ), mixed, 3, parts ) );
            ASSERT_TRUE( parts[ 0 ].IsIdentical( D( "1.34" ) ) && parts[ 1 ].IsIdentical( D( "3.33" ) ) && parts[ 2 ].IsIdentical( D( "5.33" ) ) );

            // the largest remainder wins over the lowest index
            const CDecimal uneven[ 3 ] = { D( "1" ), D( "2" ), D( "2" ) };
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::Allocate( D( "0.01" ), uneven, 3, parts ) );
            ASSERT_TRUE( parts[ 0 ].IsIdentical( D( "0.00" ) ) && parts[ 1 ].IsIdentical( D( "0.01" ) ) && parts[ 2 ].IsIdentical( D( "0.00" ) ) );

            const CDecimal single[ 1 ] = { D( "3.7" ) };
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::Allocate( U( GUCEF_INT64MAX, 2 ), single, 1, parts ) && parts[ 0 ].IsIdentical( U( GUCEF_INT64MAX, 2 ) ) );

            CDecimal untouched = D( "1" );
            const CDecimal zeros[ 2 ] = { D( "0" ), D( "0" ) };
            const CDecimal negative[ 2 ] = { D( "1" ), D( "-0.01" ) };
            const CDecimal tooLarge[ 3 ] = { U( GUCEF_INT64MAX, 0 ), U( GUCEF_INT64MAX, 0 ), U( GUCEF_INT64MAX, 0 ) };
            const CDecimal alignedTooLarge[ 2 ] = { U( GUCEF_INT64MAX, 0 ), U( 1, 1 ) };
            ASSERT_TRUE( CORE::DECIMALSTATUS_DIVISION_BY_ZERO == CDecimal::Allocate( D( "1" ), zeros, 2, parts ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_DIVISION_BY_ZERO == CDecimal::Allocate( D( "1" ), zeros, 0, &untouched ) && untouched.IsIdentical( D( "1" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == CDecimal::Allocate( D( "1" ), negative, 2, parts ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == CDecimal::Allocate( D( "1" ), GUCEF_NULL, 2, parts ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == CDecimal::Allocate( D( "1" ), tooLarge, 3, parts ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == CDecimal::Allocate( D( "1" ), alignedTooLarge, 2, parts ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::Allocate( D( "1.00" ), tooLarge, 2, parts ) && parts[ 0 ].IsIdentical( D( "0.50" ) ) && parts[ 1 ].IsIdentical( D( "0.50" ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Sum and the accumulator are exact when intermediates exceed 64 bit" )
        try
        {
            CDecimal r;
            const CDecimal values[ 5 ] = { U( GUCEF_INT64MAX, 0 ), U( GUCEF_INT64MAX, 0 ), U( -GUCEF_INT64MAX, 0 ), U( -GUCEF_INT64MAX, 0 ), D( "5.25" ) };
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::Sum( values, 5, r ) && r.IsIdentical( D( "5.25" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::Sum( values, 5, CDecimalContext( 1, CORE::ROUNDINGMODE_HALF_EVEN ), r ) && r.IsIdentical( D( "5.2" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::Sum( values, 0, r ) && r.IsIdentical( CDecimal() ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == CDecimal::Sum( GUCEF_NULL, 3, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == CDecimal::Sum( values, 2, r ) );
            const CDecimal scales[ 3 ] = { D( "1.5" ), D( "0.25" ), D( "2" ) };
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::Sum( scales, 3, r ) && r.IsIdentical( D( "3.75" ) ) );

            CDecimalAccumulator accumulator( 0 );
            ASSERT_TRUE( 0 == accumulator.GetScale() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == accumulator.Add( D( "1.5" ) ) && 1 == accumulator.GetScale() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == accumulator.Add( D( "2" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == accumulator.Subtract( D( "0.25" ) ) && 2 == accumulator.GetScale() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == accumulator.GetResult( r ) && r.IsIdentical( D( "3.25" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == accumulator.GetResult( CDecimalContext( 4, CORE::ROUNDINGMODE_UNNECESSARY ), r ) && r.IsIdentical( D( "3.2500" ) ) );

            // sum of quantity * price at the product scale
            accumulator.Reset( 0 );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == accumulator.AddProduct( D( "123.456789" ), D( "0.0825" ) ) && 10 == accumulator.GetScale() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == accumulator.AddProduct( D( "-2" ), D( "1.5" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == accumulator.GetResult( r ) && r.IsIdentical( D( "7.1851850925" ) ) );

            // a product scale above 18: exact narrowing only when the digits allow it
            accumulator.Reset( 0 );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == accumulator.AddProduct( U( 1000000000000000000LL, 18 ), U( 2000000000000000000LL, 18 ) ) && 36 == accumulator.GetScale() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == accumulator.GetResult( r ) && r.IsIdentical( U( 2000000000000000000LL, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == accumulator.AddProduct( U( 1, 18 ), U( 1, 1 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_ROUNDING_REQUIRED == accumulator.GetResult( r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == accumulator.GetResult( CDecimalContext( 2, CORE::ROUNDINGMODE_CEILING ), r ) && r.IsIdentical( D( "2.01" ) ) );

            // the 128 bit overflow is sticky
            accumulator.Reset( 0 );
            const CDecimal maxValue = U( GUCEF_INT64MAX, 0 );
            // ( 2^63 - 1 )^2 twice is still below 2^127, a third time is not
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == accumulator.AddProduct( maxValue, maxValue ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == accumulator.AddProduct( maxValue, maxValue ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == accumulator.AddProduct( maxValue, maxValue ) );
            accumulator.AddProduct( maxValue, maxValue.Negate() );
            accumulator.AddProduct( maxValue, maxValue.Negate() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == accumulator.GetResult( r ) );
            accumulator.Reset( 3 );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == accumulator.GetResult( r ) && r.IsIdentical( U( 0, 3 ) ) );

            CDecimalAccumulator invalid( 37 );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == invalid.Add( D( "1" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == invalid.GetResult( r ) );

            // span and single unscaled adds at the accumulator scale
            const Int64 span[ 6 ] = { GUCEF_INT64MAX, GUCEF_INT64MAX, -5, GUCEF_INT64MAX, -GUCEF_INT64MAX, -GUCEF_INT64MAX };
            CDecimalAccumulator bySpan( 2 );
            CDecimalAccumulator byValue( 2 );
            bySpan.AddUnscaled( span, 6 );
            for ( UInt32 i=0; i<6; ++i )
                byValue.AddUnscaled( span[ i ] );
            bySpan.AddUnscaled( span, 2 );
            byValue.AddUnscaled( span[ 0 ] );
            byValue.AddUnscaled( span[ 1 ] );
            bySpan.Subtract( U( GUCEF_INT64MAX, 2 ) );
            byValue.Subtract( U( GUCEF_INT64MAX, 2 ) );
            bySpan.Subtract( U( GUCEF_INT64MAX, 2 ) );
            byValue.Subtract( U( GUCEF_INT64MAX, 2 ) );
            bySpan.Subtract( U( GUCEF_INT64MAX, 2 ) );
            byValue.Subtract( U( GUCEF_INT64MAX, 2 ) );
            CDecimal fromSpan, fromValues;
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == bySpan.GetResult( fromSpan ) && CORE::DECIMALSTATUS_OK == byValue.GetResult( fromValues ) );
            // 3 * max - 5 added, 3 * max subtracted
            ASSERT_TRUE( fromSpan.IsIdentical( fromValues ) && fromSpan.IsIdentical( U( -5, 2 ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformDecimalFloatTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: FromFloat64Shortest gives the shortest decimal that converts back" )
        try
        {
            CDecimal r;
            ASSERT_TRUE( !IsError( CDecimal::FromFloat64Shortest( 0.0825, r ) ) && TextIs( r, "0.0825" ) );
            ASSERT_TRUE( !IsError( CDecimal::FromFloat64Shortest( 0.1, r ) ) && TextIs( r, "0.1" ) );
            ASSERT_TRUE( !IsError( CDecimal::FromFloat64Shortest( 0.3, r ) ) && TextIs( r, "0.3" ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromFloat64Shortest( 0.1, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromFloat64Shortest( 0.5, r ) && TextIs( r, "0.5" ) );
            // 1e15 + 0.25 is exact in binary, but 1000000000000000.2 already converts back to it (the spacing of doubles there is 0.125)
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromFloat64Shortest( 1e15 + 0.25, r ) && TextIs( r, "1000000000000000.2" ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromFloat64( 1e15 + 0.25, 2, CORE::ROUNDINGMODE_UNNECESSARY, r ) && TextIs( r, "1000000000000000.25" ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromFloat64( 1e15 + 0.125, 3, CORE::ROUNDINGMODE_UNNECESSARY, r ) && TextIs( r, "1000000000000000.125" ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromFloat64Shortest( 9007199254740992.0, r ) && TextIs( r, "9007199254740992" ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromFloat64Shortest( 9223372036854774784.0, r ) && TextIs( r, "9223372036854774784" ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == CDecimal::FromFloat64Shortest( 9223372036854775808.0, r ) );
            ASSERT_TRUE( !IsError( CDecimal::FromFloat64Shortest( 1e-18, r ) ) && TextIs( r, "0.000000000000000001" ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == CDecimal::FromFloat64Shortest( 1e-19, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == CDecimal::FromFloat64Shortest( 4.9e-324, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromFloat64Shortest( -0.0, r ) && TextIs( r, "0" ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromFloat64Shortest( -2.5, r ) && TextIs( r, "-2.5" ) );
            ASSERT_TRUE( !IsError( CDecimal::FromFloat64Shortest( 123456.789, r ) ) && TextIs( r, "123456.789" ) );

            CDecimal sentinel = U( 5, 5 );
            r = sentinel;
            Float64 zero = 0.0;
            Float64 nan = zero / zero;
            Float64 inf = 1.0 / zero;
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == CDecimal::FromFloat64Shortest( nan, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == CDecimal::FromFloat64Shortest( inf, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == CDecimal::FromFloat64Shortest( -inf, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == CDecimal::FromFloat64( nan, 2, CORE::ROUNDINGMODE_HALF_EVEN, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == CDecimal::FromFloat32Shortest( (Float32) inf, r ) );
            ASSERT_TRUE( r.IsIdentical( sentinel ) );

            ASSERT_TRUE( !IsError( CDecimal::FromFloat32Shortest( 0.1f, r ) ) && TextIs( r, "0.1" ) );
            ASSERT_TRUE( !IsError( CDecimal::FromFloat32Shortest( 0.0825f, r ) ) && TextIs( r, "0.0825" ) );
            ASSERT_TRUE( !IsError( CDecimal::FromFloat32Shortest( 3.14159f, r ) ) && TextIs( r, "3.14159" ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromFloat32Shortest( 16777216.0f, r ) && TextIs( r, "16777216" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: FromFloat64 rounds the exact binary value" )
        try
        {
            CDecimal r;
            // 2.675 is 2.67499999999999982236431605997495353221893310546875 as a double
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromFloat64( 2.675, 2, CORE::ROUNDINGMODE_HALF_EVEN, r ) && r.IsIdentical( D( "2.67" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromFloat64( 2.675, 2, CORE::ROUNDINGMODE_HALF_UP, r ) && r.IsIdentical( D( "2.67" ) ) );
            for ( UInt32 m=0; m<7; ++m )
            {
                ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromFloat64( 2.5, 0, g_tableModes[ m ], r ) && g_roundingTable[ 0 ].expected[ m ] == r.GetUnscaledValue() );
                ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromFloat64( -2.5, 0, g_tableModes[ m ], r ) && g_roundingTable[ 2 ].expected[ m ] == r.GetUnscaledValue() );
            }
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromFloat64( 2.5, 3, CORE::ROUNDINGMODE_UNNECESSARY, r ) && r.IsIdentical( D( "2.500" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_ROUNDING_REQUIRED == CDecimal::FromFloat64( 0.1, 18, CORE::ROUNDINGMODE_UNNECESSARY, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromFloat64( 0.1, 18, CORE::ROUNDINGMODE_HALF_EVEN, r ) && TextIs( r, "0.100000000000000006" ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == CDecimal::FromFloat64( 1.0, 19, CORE::ROUNDINGMODE_HALF_EVEN, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == CDecimal::FromFloat64( 1e300, 0, CORE::ROUNDINGMODE_HALF_EVEN, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == CDecimal::FromFloat64( 1e18, 1, CORE::ROUNDINGMODE_HALF_EVEN, r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromFloat64( 1e-300, 18, CORE::ROUNDINGMODE_HALF_EVEN, r ) && r.IsIdentical( U( 0, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromFloat64( 1e-300, 18, CORE::ROUNDINGMODE_CEILING, r ) && r.IsIdentical( U( 1, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == CDecimal::FromFloat64( -4.9e-324, 2, CORE::ROUNDINGMODE_FLOOR, r ) && r.IsIdentical( U( -1, 2 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromFloat64( 9223372036854774784.0, 0, CORE::ROUNDINGMODE_UNNECESSARY, r ) && 9223372036854774784LL == r.GetUnscaledValue() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == CDecimal::FromFloat64( -1024.0, 15, CORE::ROUNDINGMODE_UNNECESSARY, r ) && r.IsIdentical( U( -1024000000000000000LL, 15 ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: ToFloat64 is correctly rounded" )
        try
        {
            ASSERT_TRUE( 0.0825 == D( "0.0825" ).ToFloat64() );
            ASSERT_TRUE( 0.1 == D( "0.1" ).ToFloat64() );
            ASSERT_TRUE( -2.5 == D( "-2.50" ).ToFloat64() );
            ASSERT_TRUE( 0.0 == D( "0.000" ).ToFloat64() );
            ASSERT_TRUE( 9223372036854775807.0 == U( GUCEF_INT64MAX, 0 ).ToFloat64() );

            CSplitMix64 rng( g_testSeedDecimal + 21 );
            UInt32 failures = 0;
            for ( UInt32 i=0; i<g_fuzzIterations && 0 == failures; ++i )
            {
                CDecimal value = RandomDecimal( rng );
                Float64 converted = value.ToFloat64();
                UInt64 magnitude = value.GetUnscaledValue() < 0 ? (UInt64) -value.GetUnscaledValue() : (UInt64) value.GetUnscaledValue();
                if ( magnitude <= ( 1ULL << 53 ) )
                {
                    // both operands exact: the formula of the fast path
                    Float64 expected = (Float64) value.GetUnscaledValue() / pow( 10.0, (Float64) value.GetScale() );
                    if ( expected != converted )
                        ++failures;
                }
                #ifdef __GLIBC__
                // glibc strtod is correctly rounded
                Float64 reference = strtod( Canonical( value ).C_String(), GUCEF_NULL );
                if ( reference != converted )
                    ++failures;
                #endif
                if ( 0 != failures )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ToFloat64 mismatch for " << value.ToString() << " iteration " << i << " seed " << ( g_testSeedDecimal + 21 ) );
            }
            ASSERT_TRUE( 0 == failures );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: FromFloat64Shortest round trips random doubles within range" )
        try
        {
            CSplitMix64 rng( g_testSeedDecimal + 33 );
            UInt32 failures = 0;
            for ( UInt32 i=0; i<g_fuzzIterations && 0 == failures; ++i )
            {
                // magnitudes 2^-6 .. 2^49 have a decimal of at most 17 significant digits within scale 18 and 64 bit
                UInt64 mantissa = ( rng.Next() & ( ( 1ULL << 52 ) - 1 ) ) | ( 1ULL << 52 );
                int exponent = -6 - 52 + (int) rng.Below( 55 );
                Float64 value = ldexp( (Float64) mantissa, exponent );
                if ( 0 != rng.Below( 2 ) )
                    value = -value;
                CDecimal shortest;
                TDecimalStatus status = CDecimal::FromFloat64Shortest( value, shortest );
                if ( IsError( status ) || shortest.ToFloat64() != value )
                {
                    ++failures;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FromFloat64Shortest failed to round trip iteration " << i << " status " << status << " seed " << ( g_testSeedDecimal + 33 ) );
                }
                else
                if ( shortest.GetScale() > 0 )
                {
                    // shortest: one digit less does not round trip
                    CDecimal shorter;
                    if ( !IsError( CDecimal::FromFloat64( value, (UInt8) ( shortest.GetScale() - 1 ), CORE::ROUNDINGMODE_HALF_EVEN, shorter ) ) && shorter.ToFloat64() == value )
                        ++failures;
                }
            }
            ASSERT_TRUE( 0 == failures );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformDecimalPropertyTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: properties of the exact operations against the reference" )
        try
        {
            const UInt64 seed = g_testSeedDecimal + 101;
            CSplitMix64 rng( seed );
            UInt32 failures = 0;
            for ( UInt32 i=0; i<g_propertyIterations * 4 && failures < 5; ++i )
            {
                CDecimal a = RandomDecimal( rng );
                CDecimal b = RandomDecimal( rng );
                CDecimal sum, sumSwapped, difference, product, productSwapped;
                TDecimalStatus sumStatus = a.Add( b, sum );
                TDecimalStatus differenceStatus = a.Subtract( b, difference );
                TDecimalStatus productStatus = a.Multiply( b, product );
                bool ok = true;
                UInt8 maxScale = a.GetScale() > b.GetScale() ? a.GetScale() : b.GetScale();
                ok = ok && Matches( sumStatus, sum, RefAdd( a, b, false, maxScale, CORE::ROUNDINGMODE_UNNECESSARY ) );
                ok = ok && Matches( differenceStatus, difference, RefAdd( a, b, true, maxScale, CORE::ROUNDINGMODE_UNNECESSARY ) );
                ok = ok && Matches( productStatus, product, RefMultiply( a, b, (UInt8) ( a.GetScale() + b.GetScale() ), CORE::ROUNDINGMODE_UNNECESSARY ) );

                // commutativity
                ok = ok && b.Add( a, sumSwapped ) == sumStatus && ( IsError( sumStatus ) || sumSwapped.IsIdentical( sum ) );
                ok = ok && b.Multiply( a, productSwapped ) == productStatus && ( IsError( productStatus ) || productSwapped.IsIdentical( product ) );

                // ( a + b ) - b is a at the larger scale, when that fits
                if ( !IsError( sumStatus ) )
                {
                    CDecimal back, aRescaled;
                    TDecimalStatus backStatus = sum.Subtract( b, back );
                    TDecimalStatus rescaleStatus = a.Rescale( maxScale, CORE::ROUNDINGMODE_UNNECESSARY, aRescaled );
                    if ( CORE::DECIMALSTATUS_OK == rescaleStatus )
                        ok = ok && CORE::DECIMALSTATUS_OK == backStatus && back.IsIdentical( aRescaled );
                    else
                        ok = ok && CORE::DECIMALSTATUS_OVERFLOW == backStatus && CORE::DECIMALSTATUS_OVERFLOW == rescaleStatus;
                }

                // a - a == 0 at the same scale
                CDecimal zero;
                ok = ok && CORE::DECIMALSTATUS_OK == a.Subtract( a, zero ) && zero.IsIdentical( U( 0, a.GetScale() ) );

                // Compare agrees with the sign of the exact difference
                ok = ok && a.Compare( b ) == RefCompare( a, b ) && b.Compare( a ) == -RefCompare( a, b );
                ok = ok && ( a == b ) == ( 0 == RefCompare( a, b ) ) && ( a < b ) == ( RefCompare( a, b ) < 0 );
                ok = ok && ( a == b ? a.GetHash() == b.GetHash() : true );

                // Rescale up then down with UNNECESSARY gives the identical value
                UInt8 up = (UInt8) ( a.GetScale() + rng.Below( CDecimal::MaxScale - a.GetScale() + 1 ) );
                CDecimal upScaled, downScaled;
                TDecimalStatus upStatus = a.Rescale( up, CORE::ROUNDINGMODE_UNNECESSARY, upScaled );
                ok = ok && Matches( upStatus, upScaled, RefRescale( a, up, CORE::ROUNDINGMODE_UNNECESSARY ) );
                if ( !IsError( upStatus ) )
                    ok = ok && CORE::DECIMALSTATUS_OK == upScaled.Rescale( a.GetScale(), CORE::ROUNDINGMODE_UNNECESSARY, downScaled ) && downScaled.IsIdentical( a );

                // Multiply by one keeps the value
                CDecimal timesOne;
                ok = ok && CORE::DECIMALSTATUS_OK == a.Multiply( CDecimal( (Int32) 1 ), timesOne ) && timesOne.IsIdentical( a );

                if ( !ok )
                {
                    ++failures;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Decimal exact property failed for a=" << a.ToString() << " b=" << b.ToString() << " iteration " << i << " seed " << seed );
                }
            }
            ASSERT_TRUE( 0 == failures );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Divide, MulDiv, Multiply and Add with a context equal the exactly rounded reference for every mode" )
        try
        {
            const UInt64 seed = g_testSeedDecimal + 202;
            CSplitMix64 rng( seed );
            UInt32 failures = 0;
            for ( UInt32 i=0; i<g_propertyIterations && failures < 5; ++i )
            {
                CDecimal a = RandomDecimal( rng );
                CDecimal b = RandomDecimal( rng );
                CDecimal c = RandomDecimal( rng );
                if ( c.IsZero() )
                    c = U( 7, 3 );
                if ( b.IsZero() )
                    b = U( -3, 1 );
                UInt8 target = (UInt8) rng.Below( CDecimal::MaxScale + 1 );

                Int64 results[ 4 ][ 8 ];
                bool valid[ 4 ][ 8 ];
                bool ok = true;
                for ( UInt32 m=0; m<8; ++m )
                {
                    CDecimalContext ctx( target, g_allModes[ m ] );
                    CDecimal r;
                    TDecimalStatus status = a.Divide( b, ctx, r );
                    ok = ok && Matches( status, r, RefDivide( a, b, target, g_allModes[ m ] ) );
                    valid[ 0 ][ m ] = !IsError( status );
                    results[ 0 ][ m ] = r.GetUnscaledValue();

                    status = a.MulDiv( b, c, ctx, r );
                    ok = ok && Matches( status, r, RefMulDiv( a, b, c, target, g_allModes[ m ] ) );
                    valid[ 1 ][ m ] = !IsError( status );
                    results[ 1 ][ m ] = r.GetUnscaledValue();

                    status = a.Multiply( b, ctx, r );
                    ok = ok && Matches( status, r, RefMultiply( a, b, target, g_allModes[ m ] ) );
                    valid[ 2 ][ m ] = !IsError( status );
                    results[ 2 ][ m ] = r.GetUnscaledValue();

                    status = a.Add( b, ctx, r );
                    ok = ok && Matches( status, r, RefAdd( a, b, false, target, g_allModes[ m ] ) );
                    valid[ 3 ][ m ] = !IsError( status );
                    results[ 3 ][ m ] = r.GetUnscaledValue();

                    status = a.Subtract( c, ctx, r );
                    ok = ok && Matches( status, r, RefAdd( a, c, true, target, g_allModes[ m ] ) );

                    status = a.Rescale( target, g_allModes[ m ], r );
                    ok = ok && Matches( status, r, RefRescale( a, target, g_allModes[ m ] ) );
                }

                // FLOOR <= TOWARD_ZERO/AWAY/HALF_* <= CEILING, all within one unit, |TOWARD_ZERO| <= |AWAY|
                for ( UInt32 op=0; op<4; ++op )
                {
                    bool allValid = true;
                    for ( UInt32 m=0; m<7; ++m )
                        allValid = allValid && valid[ op ][ m ];
                    if ( !allValid )
                        continue;
                    Int64 floorValue = results[ op ][ 5 ];
                    Int64 ceilingValue = results[ op ][ 6 ];
                    ok = ok && floorValue <= ceilingValue && ceilingValue - floorValue <= 1;
                    for ( UInt32 m=0; m<7; ++m )
                        ok = ok && results[ op ][ m ] >= floorValue && results[ op ][ m ] <= ceilingValue;
                    Int64 towardZero = results[ op ][ 3 ];
                    Int64 away = results[ op ][ 4 ];
                    ok = ok && ( towardZero < 0 ? -towardZero : towardZero ) <= ( away < 0 ? -away : away );
                }

                // the HALF_* modes are within half a unit of the exact quotient (division by c)
                if ( valid[ 1 ][ 0 ] )
                {
                    Int32 e = (Int32) target + (Int32) c.GetScale() - (Int32) a.GetScale() - (Int32) b.GetScale();
                    CRefInt product = CRefInt::Multiply( UnscaledOf( a ), UnscaledOf( b ) );
                    CRefInt n = e >= 0 ? CRefInt::Multiply( product, CRefInt::PowerOfTen( (UInt32) e ) ) : product;
                    CRefInt d = CRefInt::Abs( UnscaledOf( c ) );
                    if ( e < 0 )
                        d = CRefInt::Multiply( d, CRefInt::PowerOfTen( (UInt32) -e ) );
                    if ( c.IsNegative() )
                        n = CRefInt::Negated( n );
                    for ( UInt32 m=0; m<3; ++m )
                    {
                        if ( !valid[ 1 ][ m ] )
                            continue;
                        CRefInt error = CRefInt::Subtract( CRefInt::Multiply( CRefInt::FromInt64( results[ 1 ][ m ] ), d ), n );
                        ok = ok && CRefInt::CompareMagnitude( CRefInt::MultiplySmall( error, 2 ), d ) <= 0;
                    }
                }

                if ( !ok )
                {
                    ++failures;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Decimal rounded property failed for a=" << a.ToString() << " b=" << b.ToString() << " c=" << c.ToString() << " target " << (UInt32) target << " iteration " << i << " seed " << seed );
                }
            }
            ASSERT_TRUE( 0 == failures );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Sum and the accumulator equal the reference sum independent of order" )
        try
        {
            const UInt64 seed = g_testSeedDecimal + 303;
            CSplitMix64 rng( seed );
            UInt32 failures = 0;
            for ( UInt32 i=0; i<g_propertyIterations / 4 && failures < 5; ++i )
            {
                UInt32 count = 1 + (UInt32) rng.Below( 24 );
                GUCEF::vector< CDecimal > values;
                CRefInt total;
                UInt8 maxScale = 0;
                for ( UInt32 n=0; n<count; ++n )
                {
                    values.push_back( RandomDecimal( rng, 12 ) );
                    if ( values.back().GetScale() > maxScale )
                        maxScale = values.back().GetScale();
                }
                for ( UInt32 n=0; n<count; ++n )
                    total = CRefInt::Add( total, CRefInt::Multiply( UnscaledOf( values[ n ] ), CRefInt::PowerOfTen( maxScale - values[ n ].GetScale() ) ) );

                CDecimal sum;
                TDecimalStatus status = CDecimal::Sum( &values[ 0 ], count, sum );
                bool ok = Matches( status, sum, ExpectRounded( total, CRefInt::FromUInt64( 1 ), CORE::ROUNDINGMODE_UNNECESSARY, maxScale ) );

                // another order, with the accumulator
                for ( UInt32 n=count; n>1; --n )
                {
                    UInt32 j = (UInt32) rng.Below( n );
                    CDecimal tmp = values[ n-1 ];
                    values[ n-1 ] = values[ j ];
                    values[ j ] = tmp;
                }
                CDecimalAccumulator accumulator( 0 );
                for ( UInt32 n=0; n<count; ++n )
                    accumulator.Add( values[ n ] );
                CDecimal accumulated;
                TDecimalStatus accumulatedStatus = accumulator.GetResult( accumulated );
                ok = ok && accumulatedStatus == status && ( IsError( status ) || accumulated.IsIdentical( sum ) );

                // rounded once with a context
                UInt8 target = (UInt8) rng.Below( CDecimal::MaxScale + 1 );
                TRoundingMode mode = g_allModes[ rng.Below( 8 ) ];
                CDecimal rounded;
                CRefInt n, d;
                ScaledFraction( total, (Int32) target - (Int32) maxScale, n, d );
                ok = ok && Matches( CDecimal::Sum( &values[ 0 ], count, CDecimalContext( target, mode ), rounded ), rounded, ExpectRounded( n, d, mode, target ) );

                // the unscaled span equals the individual adds
                GUCEF::vector< Int64 > span;
                CDecimalAccumulator bySpan( maxScale );
                CDecimalAccumulator byValue( maxScale );
                for ( UInt32 k=0; k<count; ++k )
                    span.push_back( values[ k ].GetUnscaledValue() );
                bySpan.AddUnscaled( &span[ 0 ], count );
                for ( UInt32 k=0; k<count; ++k )
                    byValue.AddUnscaled( span[ k ] );
                CDecimal spanResult, valueResult;
                TDecimalStatus spanStatus = bySpan.GetResult( spanResult );
                ok = ok && spanStatus == byValue.GetResult( valueResult ) && ( IsError( spanStatus ) || spanResult.IsIdentical( valueResult ) );

                if ( !ok )
                {
                    ++failures;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Decimal sum property failed at iteration " << i << " seed " << seed );
                }
            }
            ASSERT_TRUE( 0 == failures );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Allocate parts sum to the total, stay within one unit of the exact share and are deterministic" )
        try
        {
            const UInt64 seed = g_testSeedDecimal + 404;
            CSplitMix64 rng( seed );
            UInt32 failures = 0;
            for ( UInt32 i=0; i<g_propertyIterations / 8 && failures < 5; ++i )
            {
                UInt32 count = 1 + (UInt32) rng.Below( 20 );
                GUCEF::vector< CDecimal > weights;
                for ( UInt32 n=0; n<count; ++n )
                {
                    if ( 0 == rng.Below( 5 ) )
                        weights.push_back( U( 0, (UInt8) rng.Below( 4 ) ) );
                    else
                        weights.push_back( RandomDecimal( rng, 6, 9 ).Abs() );
                }
                CDecimal total = RandomDecimal( rng, 6, 17 );
                GUCEF::vector< CDecimal > parts( count );
                GUCEF::vector< CDecimal > again( count );
                TDecimalStatus status = CDecimal::Allocate( total, &weights[ 0 ], count, &parts[ 0 ] );
                TDecimalStatus statusAgain = CDecimal::Allocate( total, &weights[ 0 ], count, &again[ 0 ] );
                bool ok = status == statusAgain;

                UInt8 weightScale = 0;
                for ( UInt32 n=0; n<count; ++n )
                    if ( weights[ n ].GetScale() > weightScale )
                        weightScale = weights[ n ].GetScale();
                CRefInt weightSum;
                bool alignedFits = true;
                for ( UInt32 n=0; n<count; ++n )
                {
                    CRefInt aligned = CRefInt::Multiply( UnscaledOf( weights[ n ] ), CRefInt::PowerOfTen( weightScale - weights[ n ].GetScale() ) );
                    alignedFits = alignedFits && CRefInt::CompareMagnitude( aligned, CRefInt::FromUInt64( 9223372036854775807ULL ) ) <= 0;
                    weightSum = CRefInt::Add( weightSum, aligned );
                }
                bool sumFits = alignedFits && CRefInt::CompareMagnitude( weightSum, CRefInt::FromUInt64( 18446744073709551615ULL ) ) <= 0;

                if ( !sumFits )
                {
                    ok = ok && CORE::DECIMALSTATUS_OVERFLOW == status;
                }
                else
                if ( !IsError( status ) )
                {
                    CRefInt partsSum;
                    for ( UInt32 n=0; n<count; ++n )
                    {
                        ok = ok && parts[ n ].IsIdentical( again[ n ] ) && parts[ n ].GetScale() == total.GetScale();
                        partsSum = CRefInt::Add( partsSum, UnscaledOf( parts[ n ] ) );
                        // | part * W - T * w | < W
                        CRefInt w = CRefInt::Multiply( UnscaledOf( weights[ n ] ), CRefInt::PowerOfTen( weightScale - weights[ n ].GetScale() ) );
                        CRefInt error = CRefInt::Subtract( CRefInt::Multiply( UnscaledOf( parts[ n ] ), weightSum ), CRefInt::Multiply( UnscaledOf( total ), w ) );
                        ok = ok && CRefInt::CompareMagnitude( error, weightSum ) < 0;
                        if ( weights[ n ].IsZero() )
                            ok = ok && parts[ n ].IsZero();
                    }
                    ok = ok && 0 == CRefInt::Compare( partsSum, UnscaledOf( total ) );
                }
                else
                {
                    bool allZero = true;
                    for ( UInt32 n=0; n<count; ++n )
                        allZero = allZero && weights[ n ].IsZero();
                    ok = ok && allZero && CORE::DECIMALSTATUS_DIVISION_BY_ZERO == status;
                }
                if ( !ok )
                {
                    ++failures;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Decimal allocate property failed for total " << total.ToString() << " at iteration " << i << " seed " << seed );
                }
            }
            ASSERT_TRUE( 0 == failures );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  Capacity of the accumulator at a high internal scale and the order in which its statuses are reported
 */
static void
PerformDecimalAccumulatorLimitTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: accumulator capacity depends on the internal scale: sums of products with a large and a tiny term" )
        try
        {
            CDecimal r;
            CDecimalAccumulator accumulator( 0 );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == accumulator.AddProduct( D( "0.000025" ), D( "-92233720.36854774933" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == accumulator.AddProduct( D( "0.0000000000025" ), D( "0.0000000000000005" ) ) );
            ASSERT_TRUE( 29 == accumulator.GetScale() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == accumulator.GetResult( CDecimalContext( 15, CORE::ROUNDINGMODE_HALF_EVEN ), r ) );
            ASSERT_TRUE( r.IsIdentical( D( "-2305.843009213693733" ) ) );

            // the capacity of the 128 bit total is 2^127 / 10^scale: about 1.7e20 at scale 18 and 170 at scale 36
            CDecimalAccumulator wide( 0 );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == wide.AddProduct( U( 1, 18 ), U( 1, 18 ) ) && 36 == wide.GetScale() );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == wide.AddProduct( D( "100" ), D( "1" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == wide.AddProduct( D( "100" ), D( "1" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == wide.GetResult( r ) );

            // the exact total 1000.000000000000000000000000000000000001 needs more than 127 bits at scale 36
            CDecimalAccumulator tiny( 0 );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == tiny.AddProduct( D( "1000" ), D( "1" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == tiny.AddProduct( U( 1, 18 ), U( 1, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == tiny.GetResult( CDecimalContext( 18, CORE::ROUNDINGMODE_HALF_EVEN ), r ) );

            // the same sum in the other order has the same capacity limit: it is a property of the total, not of the order
            CDecimalAccumulator reversed( 0 );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == reversed.AddProduct( U( 1, 18 ), U( 1, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == reversed.AddProduct( D( "1000" ), D( "1" ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // Reported by GetResult, first match wins: INVALID_SCALE (accumulator scale above MaxScale), then the sticky OVERFLOW,
    // then ROUNDING_REQUIRED or INVALID_OPERAND of the narrowing, then INEXACT as the only non-error bit
    GUCEF_TESTFW_TESTCASE( "Test: accumulator status precedence: INVALID_SCALE, then OVERFLOW, then ROUNDING_REQUIRED" )
        try
        {
            CDecimal r;

            CDecimalAccumulator invalid( 37 );
            invalid.AddUnscaled( 5 );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == invalid.GetResult( r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == invalid.GetResult( CDecimalContext( 2, CORE::ROUNDINGMODE_HALF_EVEN ), r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == invalid.AddProduct( D( "1" ), D( "1" ) ) );

            // an overflowed total that would also need rounding reports only the overflow
            CDecimalAccumulator overflowed( 0 );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == overflowed.AddProduct( U( 1, 18 ), U( 1, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == overflowed.AddProduct( D( "1000" ), D( "1" ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == overflowed.GetResult( r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == overflowed.GetResult( CDecimalContext( 2, CORE::ROUNDINGMODE_UNNECESSARY ), r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OVERFLOW == overflowed.GetResult( CDecimalContext( 2, CORE::ROUNDINGMODE_HALF_EVEN ), r ) );

            // not overflowed: the same total needs rounding
            CDecimalAccumulator rounding( 0 );
            ASSERT_TRUE( CORE::DECIMALSTATUS_OK == rounding.AddProduct( U( 1, 18 ), U( 1, 18 ) ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_ROUNDING_REQUIRED == rounding.GetResult( r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_ROUNDING_REQUIRED == rounding.GetResult( CDecimalContext( 2, CORE::ROUNDINGMODE_UNNECESSARY ), r ) );
            ASSERT_TRUE( CORE::DECIMALSTATUS_INEXACT == rounding.GetResult( CDecimalContext( 2, CORE::ROUNDINGMODE_HALF_EVEN ), r ) );

            // the result scale of a context above MaxScale is an invalid scale, not an overflow
            ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_SCALE == rounding.GetResult( CDecimalContext( 19, CORE::ROUNDINGMODE_HALF_EVEN ), r ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // A context holds the mode as a byte. A byte outside the enumeration must be refused without ever being read as an
    // ERoundingMode, which is undefined for such values: the test writes the byte in place so it never forms one itself
    GUCEF_TESTFW_TESTCASE( "Test: a context with a rounding mode byte outside the enumeration gives INVALID_OPERAND" )
        try
        {
            const UInt8 badBytes[] = { 8, 9, 16, 127, 128, 200, 255 };
            const CDecimal a = D( "10.5" );
            const CDecimal b = D( "3" );
            for ( UInt32 i=0; i<sizeof( badBytes ); ++i )
            {
                CDecimalContext ctx( 2, CORE::ROUNDINGMODE_HALF_EVEN );
                ASSERT_TRUE( 2 == ctx.GetResultScale() );
                UInt8 bytes[ sizeof( CDecimalContext ) ];
                memcpy( bytes, &ctx, sizeof( ctx ) );
                bytes[ 1 ] = badBytes[ i ];       // m_roundingMode, right behind the UInt8 result scale
                memcpy( &ctx, bytes, sizeof( ctx ) );
                ASSERT_TRUE( 2 == ctx.GetResultScale() );

                CDecimal r;
                ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == a.Divide( b, ctx, r ) );
                ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == a.Divide( 3, ctx, r ) );
                ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == a.Multiply( b, ctx, r ) );
                ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == a.Add( b, ctx, r ) );
                ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == a.Subtract( b, ctx, r ) );
                ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == a.MulDiv( b, b, ctx, r ) );
                ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == a.PercentOf( b, ctx, r ) );
                const CDecimal values[ 2 ] = { a, b };
                ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == CDecimal::Sum( values, 2, ctx, r ) );
                CDecimalAccumulator accumulator( 0 );
                ASSERT_TRUE( CORE::DECIMALSTATUS_OK == accumulator.Add( a ) );
                ASSERT_TRUE( CORE::DECIMALSTATUS_INVALID_OPERAND == accumulator.GetResult( ctx, r ) );
            }

            // the valid modes keep working
            for ( UInt32 m=0; m<8; ++m )
            {
                CDecimalContext ctx( 2, g_allModes[ m ] );
                ASSERT_TRUE( g_allModes[ m ] == ctx.GetRoundingMode() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformDecimalTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CDecimal TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CDecimal" );

    PerformDecimalConstructionAndTextTests();
    PerformDecimalRoundingTests();
    PerformDecimalArithmeticTests();
    PerformDecimalSumAndAllocateTests();
    PerformDecimalFloatTests();
    PerformDecimalPropertyTests();
    PerformDecimalAccumulatorLimitTests();

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CDecimal TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
