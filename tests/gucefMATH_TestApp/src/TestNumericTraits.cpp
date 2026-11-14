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

#include "TestMathReference.h"
#include "TestNumericTraits.h"

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

/** Compile time type identity, independent of the library under test */
template < typename A, typename B > struct STypesAreSame { enum { value = 0 }; };
template < typename A > struct STypesAreSame< A, A > { enum { value = 1 }; };

/*-------------------------------------------------------------------------*/

/* The status values are pinned at compile time against gucefCORE as well as at run time below */
GUCEF_MATH_STATIC_ASSERT( (UInt32) MATHSTATUS_INEXACT == (UInt32) CORE::DECIMALSTATUS_INEXACT, MathStatusInexactEqualsDecimal );
GUCEF_MATH_STATIC_ASSERT( (UInt32) MATHSTATUS_OVERFLOW == (UInt32) CORE::DECIMALSTATUS_OVERFLOW, MathStatusOverflowEqualsDecimal );
GUCEF_MATH_STATIC_ASSERT( (UInt32) MATHSTATUS_DIVISION_BY_ZERO == (UInt32) CORE::DECIMALSTATUS_DIVISION_BY_ZERO, MathStatusDivisionByZeroEqualsDecimal );
GUCEF_MATH_STATIC_ASSERT( (UInt32) MATHSTATUS_INVALID_SCALE == (UInt32) CORE::DECIMALSTATUS_INVALID_SCALE, MathStatusInvalidScaleEqualsDecimal );
GUCEF_MATH_STATIC_ASSERT( (UInt32) MATHSTATUS_INVALID_FORMAT == (UInt32) CORE::DECIMALSTATUS_INVALID_FORMAT, MathStatusInvalidFormatEqualsDecimal );
GUCEF_MATH_STATIC_ASSERT( (UInt32) MATHSTATUS_INVALID_OPERAND == (UInt32) CORE::DECIMALSTATUS_INVALID_OPERAND, MathStatusInvalidOperandEqualsDecimal );
GUCEF_MATH_STATIC_ASSERT( (UInt32) MATHSTATUS_ROUNDING_REQUIRED == (UInt32) CORE::DECIMALSTATUS_ROUNDING_REQUIRED, MathStatusRoundingRequiredEqualsDecimal );
GUCEF_MATH_STATIC_ASSERT( (UInt32) MATHSTATUS_INEXACT == (UInt32) CORE::BIGINTEGERSTATUS_INEXACT, MathStatusInexactEqualsBigInteger );
GUCEF_MATH_STATIC_ASSERT( (UInt32) MATHSTATUS_OVERFLOW == (UInt32) CORE::BIGINTEGERSTATUS_OVERFLOW, MathStatusOverflowEqualsBigInteger );
GUCEF_MATH_STATIC_ASSERT( (UInt32) MATHSTATUS_DIVISION_BY_ZERO == (UInt32) CORE::BIGINTEGERSTATUS_DIVISION_BY_ZERO, MathStatusDivisionByZeroEqualsBigInteger );
GUCEF_MATH_STATIC_ASSERT( (UInt32) MATHSTATUS_INVALID_SCALE == (UInt32) CORE::BIGINTEGERSTATUS_INVALID_SCALE, MathStatusInvalidScaleEqualsBigInteger );
GUCEF_MATH_STATIC_ASSERT( (UInt32) MATHSTATUS_INVALID_FORMAT == (UInt32) CORE::BIGINTEGERSTATUS_INVALID_FORMAT, MathStatusInvalidFormatEqualsBigInteger );
GUCEF_MATH_STATIC_ASSERT( (UInt32) MATHSTATUS_INVALID_OPERAND == (UInt32) CORE::BIGINTEGERSTATUS_INVALID_OPERAND, MathStatusInvalidOperandEqualsBigInteger );
GUCEF_MATH_STATIC_ASSERT( (UInt32) MATHSTATUS_ROUNDING_REQUIRED == (UInt32) CORE::BIGINTEGERSTATUS_ROUNDING_REQUIRED, MathStatusRoundingRequiredEqualsBigInteger );
GUCEF_MATH_STATIC_ASSERT( (UInt32) MATHSTATUS_OUT_OF_MEMORY == (UInt32) CORE::BIGINTEGERSTATUS_OUT_OF_MEMORY, MathStatusOutOfMemoryEqualsBigInteger );

/* Properties usable in static assertions: the purpose of the enum form */
GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< Float32 >::IsSimdCandidate && !CTNumericTraits< Float64 >::IsSimdCandidate, NumericTraitsSimdCandidateIsCompileTime );
GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< TFixed32 >::IsDeterministic && !CTNumericTraits< Float32 >::IsDeterministic, NumericTraitsDeterminismIsCompileTime );

/*-------------------------------------------------------------------------*/

/** The property row of a kind in the order of the table of section 3.2 */
template < typename T >
static void
GetProperties( int* p )
{
    typedef CTNumericTraits< T > TT;
    p[ 0 ] = TT::IsFloatingPoint; p[ 1 ] = TT::IsFixedPoint; p[ 2 ] = TT::IsInteger; p[ 3 ] = TT::IsSigned; p[ 4 ] = TT::IsExact;
    p[ 5 ] = TT::HasSqrt; p[ 6 ] = TT::HasTranscendentals; p[ 7 ] = TT::HasDivision; p[ 8 ] = TT::IsDeterministic;
    p[ 9 ] = TT::IsRealtimeSafe; p[ 10 ] = TT::IsSimdCandidate;
}

template < typename T >
static bool
PropertiesAre( int f, int x, int i, int s, int e, int sq, int tr, int dv, int de, int rt, int sd )
{
    int expected[ 11 ] = { f, x, i, s, e, sq, tr, dv, de, rt, sd };
    int actual[ 11 ];
    GetProperties< T >( actual );
    return 0 == memcmp( expected, actual, sizeof( expected ) );
}

/*-------------------------------------------------------------------------*/

/** Rounds the four reference inputs of gucefCORE_RoundingMode.h with every mode and compares with that table */
template < typename T >
static bool
RoundingTableHolds( void )
{
    typedef CTNumericTraits< T > TT;
    // inputs 2.5, -2.5, 2.6, -0.4 as exact kind values; expected per mode in the order of ERoundingMode
    T inputs[ 4 ];
    TT::FromParts( MathMakeValueParts( false, 5, -1, 1 ), CORE::ROUNDINGMODE_UNNECESSARY, inputs[ 0 ] );
    TT::FromParts( MathMakeValueParts( true, 5, -1, 1 ), CORE::ROUNDINGMODE_UNNECESSARY, inputs[ 1 ] );
    TT::FromParts( MathMakeValueParts( false, 13, 0, 5 ), CORE::ROUNDINGMODE_HALF_EVEN, inputs[ 2 ] );
    TT::FromParts( MathMakeValueParts( true, 2, 0, 5 ), CORE::ROUNDINGMODE_HALF_EVEN, inputs[ 3 ] );
    static const Int32 expected[ 4 ][ 7 ] =
    {
        {  2,  3,  2,  2,  3,  2,  3 },
        { -2, -3, -2, -2, -3, -3, -2 },
        {  3,  3,  3,  2,  3,  2,  3 },
        {  0,  0,  0,  0, -1, -1,  0 }
    };
    for ( UInt32 i=0; i<4; ++i )
        for ( UInt32 m=0; m<7; ++m )
            if ( !( TT::Round( inputs[ i ], (TRoundingMode) m ) == TT::FromInt32( expected[ i ][ m ] ) ) )
                return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/** Text round trip of one value: ToString then FromString gives the same value */
template < typename T >
static bool
TextRoundTrips( const T& value )
{
    typedef CTNumericTraits< T > TT;
    T back = TT::Zero();
    return TT::FromString( TT::ToString( value ), back ) && TT::ToString( back ) == TT::ToString( value ) && ( back == value || !( value == value ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
static bool
IntegerDivideRoundedTable( void )
{
    typedef CTNumericTraits< T > TT;
    // 7 / 2 and -7 / 2 with every mode
    static const Int32 positive[ 7 ] = { 4, 4, 3, 3, 4, 3, 4 };
    static const Int32 negative[ 7 ] = { -4, -4, -3, -3, -4, -4, -3 };
    for ( UInt32 m=0; m<7; ++m )
    {
        T q = 0;
        if ( MATHSTATUS_INEXACT != TT::DivideRounded( (T) 7, (T) 2, (TRoundingMode) m, q ) || q != (T) positive[ m ] )
            return false;
        if ( TT::IsSigned && ( MATHSTATUS_INEXACT != TT::DivideRounded( (T) -7, (T) 2, (TRoundingMode) m, q ) || q != (T) negative[ m ] ) )
            return false;
    }
    T q = 5;
    if ( MATHSTATUS_DIVISION_BY_ZERO != TT::DivideRounded( (T) 7, (T) 0, CORE::ROUNDINGMODE_HALF_EVEN, q ) || q != 5 )
        return false;
    if ( MATHSTATUS_ROUNDING_REQUIRED != TT::DivideRounded( (T) 7, (T) 2, CORE::ROUNDINGMODE_UNNECESSARY, q ) || q != 5 )
        return false;
    return MATHSTATUS_OK == TT::DivideRounded( (T) 8, (T) 2, CORE::ROUNDINGMODE_UNNECESSARY, q ) && q == 4;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformNumericTraitsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "NumericTraits" );

    GUCEF_TESTFW_TESTCASE( "NumericTraits: property constants per kind as tabulated in section 3.2" )
    try
    {
        //                                        float fixed int signed exact sqrt trans div determ rt simd
        ASSERT_TRUE( ( PropertiesAre< Float32 >(     1,    0,   0,   1,     0,    1,   1,    1,  0,     1, 1 ) ) );
        ASSERT_TRUE( ( PropertiesAre< Float64 >(     1,    0,   0,   1,     0,    1,   1,    1,  0,     1, 0 ) ) );
        ASSERT_TRUE( ( PropertiesAre< TFixed16_16 >( 0,    1,   0,   1,     0,    1,   1,    1,  1,     1, 0 ) ) );
        ASSERT_TRUE( ( PropertiesAre< TFixed32_32 >( 0,    1,   0,   1,     0,    1,   1,    1,  1,     1, 0 ) ) );
        ASSERT_TRUE( ( PropertiesAre< TFixed48_16 >( 0,    1,   0,   1,     0,    1,   1,    1,  1,     1, 0 ) ) );
        ASSERT_TRUE( ( PropertiesAre< Int8 >(        0,    0,   1,   1,     1,    0,   0,    0,  1,     1, 0 ) ) );
        ASSERT_TRUE( ( PropertiesAre< Int16 >(       0,    0,   1,   1,     1,    0,   0,    0,  1,     1, 0 ) ) );
        ASSERT_TRUE( ( PropertiesAre< Int32 >(       0,    0,   1,   1,     1,    0,   0,    0,  1,     1, 0 ) ) );
        ASSERT_TRUE( ( PropertiesAre< Int64 >(       0,    0,   1,   1,     1,    0,   0,    0,  1,     1, 0 ) ) );
        ASSERT_TRUE( ( PropertiesAre< UInt8 >(       0,    0,   1,   0,     1,    0,   0,    0,  1,     1, 0 ) ) );
        ASSERT_TRUE( ( PropertiesAre< UInt16 >(      0,    0,   1,   0,     1,    0,   0,    0,  1,     1, 0 ) ) );
        ASSERT_TRUE( ( PropertiesAre< UInt32 >(      0,    0,   1,   0,     1,    0,   0,    0,  1,     1, 0 ) ) );
        ASSERT_TRUE( ( PropertiesAre< UInt64 >(      0,    0,   1,   0,     1,    0,   0,    0,  1,     1, 0 ) ) );
        ASSERT_TRUE( ( PropertiesAre< TFraction64 >( 0,    0,   0,   1,     1,    0,   0,    1,  1,     1, 0 ) ) );
        ASSERT_TRUE( ( PropertiesAre< TBigFraction >(0,    0,   0,   1,     1,    0,   0,    1,  1,     0, 0 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NumericTraits: TWide and TFloat types" )
    try
    {
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< Int8 >::TWide, Int32 >::value ) );
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< Int16 >::TWide, Int32 >::value ) );
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< Int32 >::TWide, Int64 >::value ) );
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< Int64 >::TWide, Int64 >::value ) );
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< UInt8 >::TWide, UInt32 >::value ) );
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< UInt16 >::TWide, UInt32 >::value ) );
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< UInt32 >::TWide, UInt64 >::value ) );
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< UInt64 >::TWide, UInt64 >::value ) );
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< Float32 >::TWide, Float32 >::value ) );
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< TFixed32 >::TWide, TFixed32 >::value ) );
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< Float32 >::TFloat, Float32 >::value ) );
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< Float64 >::TFloat, Float64 >::value ) );
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< TFixed16_16 >::TFloat, Float32 >::value ) );
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< TFixed32_32 >::TFloat, Float64 >::value ) );
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< TFixed48_16 >::TFloat, Float64 >::value ) );
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< Int16 >::TFloat, Float32 >::value ) );
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< UInt16 >::TFloat, Float32 >::value ) );
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< Int32 >::TFloat, Float64 >::value ) );
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< TFraction64 >::TFloat, Float64 >::value ) );
        ASSERT_TRUE( ( STypesAreSame< CTNumericTraits< TFraction64 >::TWide, TFraction64 >::value ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NumericTraits: default tolerances, epsilon, half and bounds as tabulated" )
    try
    {
        ASSERT_TRUE( 1e-6f == CTNumericTraits< Float32 >::DefaultAbsoluteTolerance() && 1e-5f == CTNumericTraits< Float32 >::DefaultRelativeTolerance() );
        ASSERT_TRUE( 1e-12 == CTNumericTraits< Float64 >::DefaultAbsoluteTolerance() && 1e-10 == CTNumericTraits< Float64 >::DefaultRelativeTolerance() );
        ASSERT_TRUE( 2 == CTNumericTraits< TFixed16_16 >::DefaultAbsoluteTolerance().GetRaw() && 0 == CTNumericTraits< TFixed16_16 >::DefaultRelativeTolerance().GetRaw() );
        ASSERT_TRUE( 16 == CTNumericTraits< TFixed32_32 >::DefaultAbsoluteTolerance().GetRaw() && 0 == CTNumericTraits< TFixed32_32 >::DefaultRelativeTolerance().GetRaw() );
        ASSERT_TRUE( 16 == CTNumericTraits< TFixed48_16 >::DefaultAbsoluteTolerance().GetRaw() && 0 == CTNumericTraits< TFixed48_16 >::DefaultRelativeTolerance().GetRaw() );
        ASSERT_TRUE( 0 == CTNumericTraits< Int32 >::DefaultAbsoluteTolerance() && 0 == CTNumericTraits< UInt8 >::DefaultRelativeTolerance() );
        ASSERT_TRUE( CTNumericTraits< TFraction64 >::Zero() == CTNumericTraits< TFraction64 >::DefaultAbsoluteTolerance() );
        ASSERT_TRUE( CTNumericTraits< TFraction64 >::Zero() == CTNumericTraits< TFraction64 >::DefaultRelativeTolerance() );
        ASSERT_TRUE( CTNumericTraits< TBigFraction >::Zero() == CTNumericTraits< TBigFraction >::DefaultAbsoluteTolerance() && TBigFraction( 1, 2 ) == CTNumericTraits< TBigFraction >::Half() );

        ASSERT_TRUE( FLT_EPSILON == CTNumericTraits< Float32 >::Epsilon() && DBL_EPSILON == CTNumericTraits< Float64 >::Epsilon() );
        ASSERT_TRUE( 1 == CTNumericTraits< TFixed32 >::Epsilon().GetRaw() && 1 == CTNumericTraits< TFixed64 >::Epsilon().GetRaw() );
        ASSERT_TRUE( 0 == CTNumericTraits< Int64 >::Epsilon() && CTNumericTraits< TFraction64 >::Zero() == CTNumericTraits< TFraction64 >::Epsilon() );
        ASSERT_TRUE( 32768 == CTNumericTraits< TFixed32 >::Half().GetRaw() && TFraction64( 1, 2 ) == CTNumericTraits< TFraction64 >::Half() && 0.5 == CTNumericTraits< Float64 >::Half() );

        ASSERT_TRUE( -128 == CTNumericTraits< Int8 >::Lowest() && 127 == CTNumericTraits< Int8 >::Max() );
        ASSERT_TRUE( (Int64) 0x8000000000000000ULL == CTNumericTraits< Int64 >::Lowest() && (Int64) 0x7FFFFFFFFFFFFFFFULL == CTNumericTraits< Int64 >::Max() );
        ASSERT_TRUE( 0 == CTNumericTraits< UInt64 >::Lowest() && ~(UInt64) 0 == CTNumericTraits< UInt64 >::Max() );
        ASSERT_TRUE( 65535 == CTNumericTraits< UInt16 >::Max() && 0 == CTNumericTraits< UInt16 >::Lowest() );
        ASSERT_TRUE( (Int32) 0x80000000u == CTNumericTraits< TFixed32 >::Lowest().GetRaw() && 0x7FFFFFFF == CTNumericTraits< TFixed32 >::Max().GetRaw() );
        ASSERT_TRUE( -FLT_MAX == CTNumericTraits< Float32 >::Lowest() && DBL_MAX == CTNumericTraits< Float64 >::Max() );
        ASSERT_TRUE( (Int64) 0x7FFFFFFFFFFFFFFFULL == CTNumericTraits< TFraction64 >::Max().GetNumerator() && 1 == CTNumericTraits< TFraction64 >::Lowest().GetDenominator() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NumericTraits: TMathStatus values equal EDecimalStatus and EBigIntegerStatus" )
    try
    {
        ASSERT_TRUE( 0x00 == MATHSTATUS_OK && 0x01 == MATHSTATUS_INEXACT && 0x02 == MATHSTATUS_OVERFLOW && 0x04 == MATHSTATUS_DIVISION_BY_ZERO );
        ASSERT_TRUE( 0x08 == MATHSTATUS_INVALID_SCALE && 0x10 == MATHSTATUS_INVALID_FORMAT && 0x20 == MATHSTATUS_INVALID_OPERAND );
        ASSERT_TRUE( 0x40 == MATHSTATUS_ROUNDING_REQUIRED && 0x80 == MATHSTATUS_OUT_OF_MEMORY && 0x100 == MATHSTATUS_NOT_CONVERGED && 0x200 == MATHSTATUS_DEGENERATE );
        ASSERT_TRUE( (UInt32) MATHSTATUS_OVERFLOW == (UInt32) CORE::DECIMALSTATUS_OVERFLOW && (UInt32) MATHSTATUS_OVERFLOW == (UInt32) CORE::BIGINTEGERSTATUS_OVERFLOW );
        ASSERT_TRUE( (UInt32) MATHSTATUS_ROUNDING_REQUIRED == (UInt32) CORE::DECIMALSTATUS_ROUNDING_REQUIRED && (UInt32) MATHSTATUS_OUT_OF_MEMORY == (UInt32) CORE::BIGINTEGERSTATUS_OUT_OF_MEMORY );
        ASSERT_FALSE( MathStatusIsError( MATHSTATUS_OK ) );
        ASSERT_FALSE( MathStatusIsError( MATHSTATUS_INEXACT ) );
        ASSERT_TRUE( MathStatusIsError( MATHSTATUS_INEXACT | MATHSTATUS_OVERFLOW ) );
        ASSERT_TRUE( MathStatusIsError( MATHSTATUS_DEGENERATE ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NumericTraits: Round with every mode matches the reference table of the rounding modes" )
    try
    {
        ASSERT_TRUE( RoundingTableHolds< Float32 >() );
        ASSERT_TRUE( RoundingTableHolds< Float64 >() );
        ASSERT_TRUE( RoundingTableHolds< TFixed16_16 >() );
        ASSERT_TRUE( RoundingTableHolds< TFixed32_32 >() );
        ASSERT_TRUE( RoundingTableHolds< TFixed48_16 >() );
        ASSERT_TRUE( RoundingTableHolds< TFraction64 >() );
        ASSERT_TRUE( RoundingTableHolds< TBigFraction >() );

        // the largest double below 0.5: x + 0.5 rounds to 1 in double arithmetic, the magnitude based tie detection must not
        const Float64 belowHalf = 0.49999999999999994;
        ASSERT_TRUE( 0.0 == CTNumericTraits< Float64 >::Round( belowHalf, CORE::ROUNDINGMODE_HALF_UP ) );
        ASSERT_TRUE( 0.0 == CTNumericTraits< Float64 >::Round( -belowHalf, CORE::ROUNDINGMODE_HALF_UP ) );
        ASSERT_TRUE( 4503599627370497.0 == CTNumericTraits< Float64 >::Round( 4503599627370497.0, CORE::ROUNDINGMODE_HALF_EVEN ) );
        ASSERT_TRUE( 2.0 == CTNumericTraits< Float64 >::Round( 2.5, CORE::ROUNDINGMODE_UNNECESSARY ) );
        ASSERT_TRUE( 7 == CTNumericTraits< Int32 >::Round( 7, CORE::ROUNDINGMODE_CEILING ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NumericTraits: Abs, Min, Max, Sign, Floor, Ceil, Truncate, ModFloor" )
    try
    {
        ASSERT_TRUE( 2.5f == CTNumericTraits< Float32 >::Abs( -2.5f ) && -1.0 == CTNumericTraits< Float64 >::Sign( -3.0 ) && 0.0 == CTNumericTraits< Float64 >::Sign( 0.0 ) );
        ASSERT_TRUE( 3 == CTNumericTraits< Int32 >::Abs( -3 ) && -1 == CTNumericTraits< Int8 >::Sign( (Int8) -100 ) && 1 == CTNumericTraits< UInt8 >::Sign( (UInt8) 200 ) );
        ASSERT_TRUE( (Int64) 0x8000000000000000ULL == CTNumericTraits< Int64 >::Abs( (Int64) 0x8000000000000000ULL ) );  // documented wrap of the minimum
        ASSERT_TRUE( 2 == CTNumericTraits< Int32 >::Min( 2, 5 ) && 5 == CTNumericTraits< Int32 >::Max( 2, 5 ) );
        ASSERT_TRUE( TFixed32( -1 ) == CTNumericTraits< TFixed32 >::Sign( TFixed32( -0.25 ) ) && TFixed32( 1 ) == CTNumericTraits< TFixed32 >::Abs( TFixed32( -1 ) ) );
        ASSERT_TRUE( TFraction64( 3, 4 ) == CTNumericTraits< TFraction64 >::Abs( TFraction64( -3, 4 ) ) && TFraction64( -1, 1 ) == CTNumericTraits< TFraction64 >::Sign( TFraction64( -3, 4 ) ) );
        ASSERT_TRUE( CTNumericTraits< TFraction64 >::Max() == CTNumericTraits< TFraction64 >::Abs( CTNumericTraits< TFraction64 >::Lowest() ) );   // saturates

        ASSERT_TRUE( -3.0 == CTNumericTraits< Float64 >::Floor( -2.5 ) && -2.0 == CTNumericTraits< Float64 >::Ceil( -2.5 ) && -2.0 == CTNumericTraits< Float64 >::Truncate( -2.5 ) );
        ASSERT_TRUE( TFixed32( -3 ) == CTNumericTraits< TFixed32 >::Floor( TFixed32( -2.5 ) ) && TFixed32( -2 ) == CTNumericTraits< TFixed32 >::Ceil( TFixed32( -2.5 ) ) );
        ASSERT_TRUE( TFixed64( -2 ) == CTNumericTraits< TFixed64 >::Truncate( TFixed64( -2.75 ) ) && TFixed64( 3 ) == CTNumericTraits< TFixed64 >::Ceil( TFixed64( 2.25 ) ) );
        ASSERT_TRUE( TFixed32::Highest() == CTNumericTraits< TFixed32 >::Ceil( TFixed32::Highest() ) );     // saturates at the top
        ASSERT_TRUE( TFraction64( -3, 1 ) == CTNumericTraits< TFraction64 >::Floor( TFraction64( -5, 2 ) ) && TFraction64( -2, 1 ) == CTNumericTraits< TFraction64 >::Ceil( TFraction64( -5, 2 ) ) );

        ASSERT_TRUE( 2.0 == CTNumericTraits< Float64 >::ModFloor( -1.0, 3.0 ) && 1.0 == CTNumericTraits< Float64 >::ModFloor( 7.0, 3.0 ) );
        ASSERT_TRUE( 0.0f == CTNumericTraits< Float32 >::ModFloor( -1e-30f, 3.0f ) || CTNumericTraits< Float32 >::ModFloor( -1e-30f, 3.0f ) < 3.0f );
        ASSERT_TRUE( 2 == CTNumericTraits< Int32 >::ModFloor( -1, 3 ) && 1 == CTNumericTraits< Int32 >::ModFloor( 7, 3 ) && -1 == CTNumericTraits< Int32 >::ModFloor( 5, -3 ) );
        ASSERT_TRUE( TFixed32( 2 ) == CTNumericTraits< TFixed32 >::ModFloor( TFixed32( -1 ), TFixed32( 3 ) ) );
        ASSERT_TRUE( TFraction64( 1, 2 ) == CTNumericTraits< TFraction64 >::ModFloor( TFraction64( -5, 2 ), TFraction64( 3, 1 ) ) );
        // a period of -1 with the lowest value: the remainder operator would trap
        ASSERT_TRUE( 0 == CTNumericTraits< Int64 >::ModFloor( (Int64) 0x8000000000000000ULL, -1 ) && 0 == CTNumericTraits< Int32 >::ModFloor( (Int32) 0x80000000u, -1 ) );
        ASSERT_TRUE( TFixed64() == CTNumericTraits< TFixed64 >::ModFloor( TFixed64::Lowest(), TFixed64::FromRaw( -1 ) ) );
        ASSERT_TRUE( 0 == CTNumericTraits< Int8 >::ModFloor( (Int8) -128, (Int8) -1 ) && -1 == CTNumericTraits< Int32 >::ModFloor( 5, -2 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NumericTraits: MultiplyAdd rounds twice and is never fused" )
    try
    {
        // ( 1 + 2^-27 ) ( 1 - 2^-27 ) = 1 - 2^-54 rounds to 1 (tie to even), so the unfused result is 0, the fused one -2^-54
        const Float64 a = 1.0 + ldexp( 1.0, -27 );
        const Float64 b = 1.0 - ldexp( 1.0, -27 );
        ASSERT_TRUE( 0.0 == CTNumericTraits< Float64 >::MultiplyAdd( a, b, -1.0 ) );
        ASSERT_TRUE( 11 == CTNumericTraits< Int32 >::MultiplyAdd( 3, 3, 2 ) );
        ASSERT_TRUE( 0 == CTNumericTraits< Int32 >::MultiplyAdd( 65536, 65536, 0 ) );     // wraps as the built-in type, without undefined behaviour
        ASSERT_TRUE( TFixed32( 3.25 ) == CTNumericTraits< TFixed32 >::MultiplyAdd( TFixed32( 1.5 ), TFixed32( 1.5 ), TFixed32( 1 ) ) );
        // narrow types promote to int in C++: the product 65535 * 65535 must not overflow int (UBSan)
        ASSERT_TRUE( 2 == CTNumericTraits< UInt16 >::MultiplyAdd( 65535, 65535, 1 ) );          // ( 2^32 - 2^17 + 1 + 1 ) mod 2^16
        ASSERT_TRUE( (Int16) 1 == CTNumericTraits< Int16 >::MultiplyAdd( (Int16) -32768, (Int16) -32768, (Int16) 1 ) );
        ASSERT_TRUE( (UInt8) 2 == CTNumericTraits< UInt8 >::MultiplyAdd( 255, 255, 1 ) );
        ASSERT_TRUE( (UInt64) 0 == CTNumericTraits< UInt64 >::MultiplyAdd( 0x8000000000000000ULL, 2, 0 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NumericTraits: EqualsWithin and EqualsUlp, NaN never equal" )
    try
    {
        const Float32 one = 1.0f;
        Float32 next = one;
        UInt32 bits = 0;
        memcpy( &bits, &next, 4 );
        ++bits;
        memcpy( &next, &bits, 4 );
        ASSERT_TRUE( CTNumericTraits< Float32 >::EqualsUlp( one, next, 1 ) );
        ASSERT_FALSE( CTNumericTraits< Float32 >::EqualsUlp( one, next, 0 ) );
        ASSERT_TRUE( CTNumericTraits< Float32 >::EqualsUlp( -0.0f, 0.0f, 0 ) );
        ASSERT_TRUE( CTNumericTraits< Float64 >::EqualsUlp( -0.0, 0.0, 0 ) );
        ASSERT_TRUE( CTNumericTraits< Float64 >::EqualsUlp( 1.0, 1.0 + DBL_EPSILON, 1 ) );
        ASSERT_TRUE( CTNumericTraits< Float64 >::EqualsUlp( -ldexp( 1.0, -1074 ), ldexp( 1.0, -1074 ), 2 ) );     // across zero
        ASSERT_FALSE( CTNumericTraits< Float64 >::EqualsUlp( -1.0, 1.0, 0xFFFFFFFFu ) );
        const Float64 nan = MathQuietNaN64();
        ASSERT_FALSE( CTNumericTraits< Float64 >::EqualsUlp( nan, nan, 0xFFFFFFFFu ) );
        ASSERT_FALSE( CTNumericTraits< Float64 >::EqualsWithin( nan, nan, 1e300, 1e300 ) );
        ASSERT_FALSE( CTNumericTraits< Float32 >::EqualsWithin( (Float32) nan, 1.0f, 1e30f, 1.0f ) );
        ASSERT_TRUE( CTNumericTraits< Float64 >::EqualsWithin( 1000.0, 1000.5, 0.0, 1e-3 ) );
        ASSERT_FALSE( CTNumericTraits< Float64 >::EqualsWithin( 1000.0, 1002.0, 1.0, 1e-3 ) );
        ASSERT_TRUE( CTNumericTraits< Float64 >::EqualsWithin( 0.0, 1e-13, 1e-12, 0.0 ) );

        ASSERT_TRUE( CTNumericTraits< TFixed32 >::EqualsUlp( TFixed32::FromRaw( 10 ), TFixed32::FromRaw( 12 ), 2 ) );
        ASSERT_FALSE( CTNumericTraits< TFixed32 >::EqualsUlp( TFixed32::FromRaw( 10 ), TFixed32::FromRaw( 13 ), 2 ) );
        ASSERT_TRUE( CTNumericTraits< TFixed64 >::EqualsUlp( TFixed64::Lowest(), TFixed64::Highest(), 0xFFFFFFFFu ) == false );
        ASSERT_TRUE( CTNumericTraits< TFixed32 >::EqualsWithin( TFixed32( 100 ), TFixed32( 101 ), TFixed32(), TFixed32( 0.01 ) ) );
        ASSERT_FALSE( CTNumericTraits< TFixed32 >::EqualsWithin( TFixed32( 100 ), TFixed32( 102 ), TFixed32( 1 ), TFixed32( 0.01 ) ) );

        ASSERT_FALSE( CTNumericTraits< Int64 >::EqualsWithin( (Int64) 0x8000000000000000ULL, (Int64) 0x7FFFFFFFFFFFFFFFULL, 0, 1 ) );    // distance 2^64 - 1 > 2^63
        ASSERT_TRUE( CTNumericTraits< Int64 >::EqualsWithin( (Int64) 0x8000000000000000ULL, (Int64) 0x7FFFFFFFFFFFFFFFULL, 0, 2 ) );     // without overflow
        ASSERT_FALSE( CTNumericTraits< Int32 >::EqualsWithin( 10, 13, 2, 0 ) );
        ASSERT_TRUE( CTNumericTraits< UInt8 >::EqualsWithin( 10, 13, 3, 0 ) );
        ASSERT_TRUE( CTNumericTraits< TFraction64 >::EqualsWithin( TFraction64( 1, 3 ), TFraction64( 1, 3 ), TFraction64(), TFraction64() ) );
        ASSERT_TRUE( CTNumericTraits< TFraction64 >::EqualsWithin( TFraction64( 1, 3 ), TFraction64( 1, 2 ), TFraction64( 1, 6 ), TFraction64() ) );
        ASSERT_FALSE( CTNumericTraits< TFraction64 >::EqualsWithin( TFraction64( 1, 3 ), TFraction64( 1, 2 ), TFraction64( 1, 7 ), TFraction64() ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NumericTraits: ToString and FromString round trip exactly for 10^4 seeded values per kind" )
    try
    {
        const UInt64 seed = 31337;
        MathReference::CSeededSequence generator( seed );
        UInt32 failures = 0;
        for ( UInt32 i=0; i<10000; ++i )
        {
            UInt64 r = generator.NextUInt64();
            Float64 d;
            memcpy( &d, &r, 8 );
            UInt32 r32 = (UInt32) r;
            Float32 f;
            memcpy( &f, &r32, 4 );
            if ( !TextRoundTrips( d ) || !TextRoundTrips( f ) )
                ++failures;
            if ( !TextRoundTrips( TFixed16_16::FromRaw( (Int32) r32 ) ) || !TextRoundTrips( TFixed32_32::FromRaw( (Int64) r ) ) || !TextRoundTrips( TFixed48_16::FromRaw( (Int64) r ) ) )
                ++failures;
            if ( !TextRoundTrips( (Int64) r ) || !TextRoundTrips( r ) || !TextRoundTrips( (Int8) r ) || !TextRoundTrips( (UInt16) r ) )
                ++failures;
            Int64 n = generator.NextSignedWithRandomLength( 64 );
            Int64 den = generator.NextSignedWithRandomLength( 63 );
            if ( den != 0 && !TextRoundTrips( TFraction64( n, den < 0 ? -den : den ) ) )
                ++failures;
            CORE::CBigInteger bigNumerator;
            CORE::CBigInteger( n ).ShiftLeft( (UInt32) ( r % 300 ), bigNumerator );
            if ( den != 0 && !TextRoundTrips( TBigFraction( bigNumerator, CORE::CBigInteger( den < 0 ? -den : den ) ) ) )
                ++failures;
        }
        if ( 0 != failures )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "NumericTraits: text round trip seed " << seed << " failures " << failures );
        ASSERT_TRUE( 0 == failures );

        ASSERT_TRUE( "0.1" == CTNumericTraits< Float64 >::ToString( 0.1 ) && "0.1" == CTNumericTraits< Float32 >::ToString( 0.1f ) );
        ASSERT_TRUE( "-inf" == CTNumericTraits< Float64 >::ToString( -MathInfinity64() ) && "nan" == CTNumericTraits< Float32 >::ToString( (Float32) MathQuietNaN64() ) );
        ASSERT_TRUE( "-1.5" == CTNumericTraits< TFixed32 >::ToString( TFixed32( -1.5 ) ) && "3" == CTNumericTraits< TFixed64 >::ToString( TFixed64( 3 ) ) );
        ASSERT_TRUE( "-7/3" == CTNumericTraits< TFraction64 >::ToString( TFraction64( 14, -6 ) ) && "5" == CTNumericTraits< TFraction64 >::ToString( TFraction64( 10, 2 ) ) );
        ASSERT_TRUE( "-9223372036854775808" == CTNumericTraits< Int64 >::ToString( (Int64) 0x8000000000000000ULL ) );

        // malformed text is rejected and leaves the value unchanged
        Int32 i32 = 42;
        ASSERT_FALSE( CTNumericTraits< Int32 >::FromString( "12a", i32 ) );
        ASSERT_FALSE( CTNumericTraits< Int32 >::FromString( "", i32 ) );
        ASSERT_FALSE( CTNumericTraits< Int32 >::FromString( "-", i32 ) );
        ASSERT_FALSE( CTNumericTraits< Int32 >::FromString( "2147483648", i32 ) );
        ASSERT_TRUE( 42 == i32 );
        ASSERT_TRUE( CTNumericTraits< Int32 >::FromString( "-2147483648", i32 ) && (Int32) 0x80000000u == i32 );
        UInt8 u8 = 7;
        ASSERT_FALSE( CTNumericTraits< UInt8 >::FromString( "-1", u8 ) || CTNumericTraits< UInt8 >::FromString( "256", u8 ) );
        ASSERT_TRUE( 7 == u8 );
        // Float32 text is rounded once from the exact decimal value: this text lies just above the midpoint between
        // 1 and 1 + 2^-23, where parsing as Float64 first would round down to the midpoint and then to even (1.0)
        Float32 f32 = 0.0f;
        ASSERT_TRUE( CTNumericTraits< Float32 >::FromString( "1.00000005960464477539062500000001", f32 ) && 1.00000012f == f32 );
        ASSERT_TRUE( CTNumericTraits< Float32 >::FromString( "1.000000059604644775390625", f32 ) && 1.0f == f32 );           // the exact tie: even
        ASSERT_TRUE( CTNumericTraits< Float32 >::FromString( "-1.5e-45", f32 ) && 0x80000001u == *(UInt32*) &f32 );
        ASSERT_TRUE( CTNumericTraits< Float32 >::FromString( "3.4028235e38", f32 ) && FLT_MAX == f32 );
        ASSERT_TRUE( CTNumericTraits< Float32 >::FromString( "1e-60", f32 ) && 0.0f == f32 && CTNumericTraits< Float32 >::FromString( "+.5E+1", f32 ) && 5.0f == f32 );
        ASSERT_FALSE( CTNumericTraits< Float32 >::FromString( "3.5e38", f32 ) || CTNumericTraits< Float32 >::FromString( "1e", f32 ) || CTNumericTraits< Float32 >::FromString( "1.2.3", f32 ) || CTNumericTraits< Float32 >::FromString( "", f32 ) );
        ASSERT_TRUE( 5.0f == f32 );
        Float64 f64 = 2.0;
        ASSERT_FALSE( CTNumericTraits< Float64 >::FromString( "1.5x", f64 ) || CTNumericTraits< Float64 >::FromString( " 1", f64 ) );
        ASSERT_TRUE( 2.0 == f64 );
        TFixed32 fixed( 2 );
        ASSERT_FALSE( fixed.FromString( "1.5.2" ) || fixed.FromString( "." ) || fixed.FromString( "40000" ) || fixed.FromString( "1e3" ) );
        ASSERT_TRUE( TFixed32( 2 ) == fixed );
        TFraction64 fraction( 1, 2 );
        ASSERT_FALSE( CTNumericTraits< TFraction64 >::FromString( "abc/3", fraction ) || CTNumericTraits< TFraction64 >::FromString( "1/0", fraction ) );
        ASSERT_FALSE( CTNumericTraits< TFraction64 >::FromString( "/3", fraction ) || CTNumericTraits< TFraction64 >::FromString( "1/", fraction ) );
        ASSERT_TRUE( TFraction64( 1, 2 ) == fraction );
        TBigFraction bigFraction( 1, 2 );
        ASSERT_FALSE( CTNumericTraits< TBigFraction >::FromString( "abc/3", bigFraction ) || CTNumericTraits< TBigFraction >::FromString( "1/0", bigFraction ) );
        ASSERT_FALSE( CTNumericTraits< TBigFraction >::FromString( "/3", bigFraction ) || CTNumericTraits< TBigFraction >::FromString( "1/", bigFraction ) );
        ASSERT_TRUE( TBigFraction( 1, 2 ) == bigFraction );
        ASSERT_TRUE( CTNumericTraits< TBigFraction >::FromString( "123456789012345678901234567890/9876543210", bigFraction ) );
        ASSERT_TRUE( "123456789012345678901234567890/9876543210" != CTNumericTraits< TBigFraction >::ToString( bigFraction ) );      // reduced
        ASSERT_TRUE( TBigFraction( CORE::CBigInteger::Parse( "123456789012345678901234567890" ), CORE::CBigInteger::Parse( "9876543210" ) ) == bigFraction );
        ASSERT_TRUE( CTNumericTraits< TFraction64 >::FromString( "6/-4", fraction ) == false );
        ASSERT_TRUE( CTNumericTraits< TFraction64 >::FromString( "-6/4", fraction ) && TFraction64( -3, 2 ) == fraction );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NumericTraits: integer DivideRounded with every mode, saturating FromInt64 and FromFloat64" )
    try
    {
        ASSERT_TRUE( IntegerDivideRoundedTable< Int8 >() );
        ASSERT_TRUE( IntegerDivideRoundedTable< Int32 >() );
        ASSERT_TRUE( IntegerDivideRoundedTable< Int64 >() );
        ASSERT_TRUE( IntegerDivideRoundedTable< UInt16 >() );
        Int64 q = 0;
        ASSERT_TRUE( MATHSTATUS_OVERFLOW == ( MATHSTATUS_OVERFLOW & CTNumericTraits< Int64 >::DivideRounded( (Int64) 0x8000000000000000ULL, -1, CORE::ROUNDINGMODE_HALF_EVEN, q ) ) );
        ASSERT_TRUE( (Int64) 0x7FFFFFFFFFFFFFFFULL == q );

        Int8 i8 = 0;
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == CTNumericTraits< Int8 >::FromInt64( 300, CORE::ROUNDINGMODE_HALF_EVEN, i8 ) && 127 == i8 );
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == CTNumericTraits< Int8 >::FromInt64( -300, CORE::ROUNDINGMODE_HALF_EVEN, i8 ) && -128 == i8 );
        UInt32 u32 = 5;
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == CTNumericTraits< UInt32 >::FromInt64( -1, CORE::ROUNDINGMODE_HALF_EVEN, u32 ) && 0 == u32 );
        ASSERT_TRUE( MATHSTATUS_INEXACT == CTNumericTraits< UInt32 >::FromFloat64( -0.25, CORE::ROUNDINGMODE_HALF_EVEN, u32 ) && 0 == u32 );
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == CTNumericTraits< UInt32 >::FromFloat64( -0.75, CORE::ROUNDINGMODE_HALF_EVEN, u32 ) && 0 == u32 );
        UInt64 u64 = 0;
        ASSERT_TRUE( MATHSTATUS_OK == CTNumericTraits< UInt64 >::FromFloat64( 18446744073709549568.0, CORE::ROUNDINGMODE_UNNECESSARY, u64 ) && 18446744073709549568ULL == u64 );
        ASSERT_TRUE( ( MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT ) == CTNumericTraits< UInt64 >::FromFloat64( 18446744073709551616.0, CORE::ROUNDINGMODE_HALF_EVEN, u64 ) && ~(UInt64) 0 == u64 );
        Int32 i32 = 9;
        ASSERT_TRUE( MATHSTATUS_INVALID_OPERAND == CTNumericTraits< Int32 >::FromFloat64( 1.0, (TRoundingMode) 77, i32 ) && 9 == i32 );
        ASSERT_TRUE( MATHSTATUS_INVALID_OPERAND == CTNumericTraits< Int32 >::FromFloat64( MathInfinity64(), CORE::ROUNDINGMODE_HALF_EVEN, i32 ) && 0 == i32 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "NumericTraits: float traits functions and constants" )
    try
    {
        ASSERT_TRUE( 3.0 == CTNumericTraits< Float64 >::Log2( 8.0 ) && -1074.0 == CTNumericTraits< Float64 >::Log2( ldexp( 1.0, -1074 ) ) );
        ASSERT_TRUE( 8.0 == CTNumericTraits< Float64 >::Exp2( 3.0 ) && 0.25f == CTNumericTraits< Float32 >::Exp2( -2.0f ) );
        ASSERT_TRUE( 1.5f == CTNumericTraits< Float32 >::Sqrt( 2.25f ) && 0.5 == CTNumericTraits< Float64 >::InverseSqrt( 4.0 ) );
        ASSERT_TRUE( fabs( CTNumericTraits< Float64 >::Sin( CTNumericTraits< Float64 >::HalfPi() ) - 1.0 ) < 1e-15 );
        ASSERT_TRUE( CTNumericTraits< Float64 >::TwoPi() == 2.0 * CTNumericTraits< Float64 >::Pi() );
        ASSERT_TRUE( (Float32) 3.14159265358979323846 == CTNumericTraits< Float32 >::Pi() );
        ASSERT_TRUE( 2.718281828459045 == CTNumericTraits< Float64 >::E() && 0.6931471805599453 == CTNumericTraits< Float64 >::Ln2() );
        ASSERT_TRUE( 1.4142135623730951 == CTNumericTraits< Float64 >::Sqrt2() );
        Float32 f = 0;
        ASSERT_TRUE( MATHSTATUS_OK == CTNumericTraits< Float32 >::FromInt64( 16777216, CORE::ROUNDINGMODE_UNNECESSARY, f ) && 16777216.0f == f );
        ASSERT_TRUE( MATHSTATUS_ROUNDING_REQUIRED == CTNumericTraits< Float32 >::FromInt64( 16777217, CORE::ROUNDINGMODE_UNNECESSARY, f ) && 16777216.0f == f );
        ASSERT_TRUE( MATHSTATUS_INEXACT == CTNumericTraits< Float32 >::FromInt64( 16777217, CORE::ROUNDINGMODE_CEILING, f ) && 16777218.0f == f );
        ASSERT_TRUE( CTNumericTraits< Float64 >::IsFinite( 1e308 ) && !CTNumericTraits< Float64 >::IsFinite( MathInfinity64() ) && !CTNumericTraits< Float64 >::IsFinite( MathQuietNaN64() ) );
        ASSERT_TRUE( CTNumericTraits< TFixed32 >::IsFinite( TFixed32::Highest() ) && CTNumericTraits< Int32 >::IsFinite( 1 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
