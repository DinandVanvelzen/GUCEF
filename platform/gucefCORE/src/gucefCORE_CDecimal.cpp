/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include <math.h>
#include <string.h>

#ifndef GUCEF_INVALID_ARG_EXCEPTION_H
#include "gucef_invalid_arg_exception.h"
#define GUCEF_INVALID_ARG_EXCEPTION_H
#endif /* GUCEF_INVALID_ARG_EXCEPTION_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDECIMALACCUMULATOR_H
#include "gucefCORE_CDecimalAccumulator.h"
#define GUCEF_CORE_CDECIMALACCUMULATOR_H
#endif /* GUCEF_CORE_CDECIMALACCUMULATOR_H ? */

#ifndef GUCEF_CORE_CUINT128_H
#include "gucefCORE_CUInt128.h"
#define GUCEF_CORE_CUINT128_H
#endif /* GUCEF_CORE_CUINT128_H ? */

#include "gucefCORE_CDecimal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** 10^0 .. 10^19, 10^19 < 2^64 */
static const UInt64 g_decimalPow10[ 20 ] =
{
    1ULL,
    10ULL,
    100ULL,
    1000ULL,
    10000ULL,
    100000ULL,
    1000000ULL,
    10000000ULL,
    100000000ULL,
    1000000000ULL,
    10000000000ULL,
    100000000000ULL,
    1000000000000ULL,
    10000000000000ULL,
    100000000000000ULL,
    1000000000000000ULL,
    10000000000000000ULL,
    100000000000000000ULL,
    1000000000000000000ULL,
    10000000000000000000ULL
};

/** ( 2^63 - 1 ) / 10^k: magnitudes up to these can be upscaled by 10^k in 64 bit without overflow */
static const UInt64 g_decimalMaxUnscaledForUpscale[ 19 ] =
{
    9223372036854775807ULL,
    922337203685477580ULL,
    92233720368547758ULL,
    9223372036854775ULL,
    922337203685477ULL,
    92233720368547ULL,
    9223372036854ULL,
    922337203685ULL,
    92233720368ULL,
    9223372036ULL,
    922337203ULL,
    92233720ULL,
    9223372ULL,
    922337ULL,
    92233ULL,
    9223ULL,
    922ULL,
    92ULL,
    9ULL
};

/** Powers of ten that are exact doubles */
static const Float64 g_decimalPow10Float64[ 19 ] =
{
    1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
    1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18
};

/** floor( sqrt( 2^63 - 1 ) ): the product of two magnitudes up to this cannot overflow 64 bit signed */
static const UInt64 g_decimalMaxFastMultiplyOperand = 3037000499ULL;

static const UInt64 g_decimalMaxMagnitude = 9223372036854775807ULL;

const CDecimal CDecimal::Zero;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Where the discarded part of a division lies relative to one half of the last unit kept */
enum ERoundRelation
{
    ROUNDRELATION_EXACT = 0,
    ROUNDRELATION_BELOW_HALF,
    ROUNDRELATION_HALF,
    ROUNDRELATION_ABOVE_HALF
};

/*-------------------------------------------------------------------------*/

static inline UInt64
MagnitudeOf( Int64 value )
{
    return value < 0 ? 0 - (UInt64) value : (UInt64) value;
}

/*-------------------------------------------------------------------------*/

static inline bool
IsValidRoundingMode( TRoundingMode mode )
{
    return (UInt32) mode <= (UInt32) ROUNDINGMODE_UNNECESSARY;
}

/*-------------------------------------------------------------------------*/

/**
 *  Classifies the remainder of the last division of a chain.
 *  'sticky' tells whether any earlier division in the chain had a non-zero remainder.
 *  This is exact when the chain has one division, or when the last divisor is even (the powers of ten).
 */
static ERoundRelation
ClassifyRemainder( UInt64 remainder, UInt64 divisor, bool sticky )
{
    if ( 0 == remainder && !sticky )
        return ROUNDRELATION_EXACT;
    UInt64 rest = divisor - remainder;
    if ( remainder < rest )
        return ROUNDRELATION_BELOW_HALF;
    if ( remainder > rest )
        return ROUNDRELATION_ABOVE_HALF;
    return sticky ? ROUNDRELATION_ABOVE_HALF : ROUNDRELATION_HALF;
}

/*-------------------------------------------------------------------------*/

/** Whether the magnitude quotient is incremented, the sign is applied afterwards */
static bool
RoundingIncrements( TRoundingMode mode, bool negative, bool quotientIsOdd, ERoundRelation relation )
{
    if ( ROUNDRELATION_EXACT == relation )
        return false;

    switch ( mode )
    {
        case ROUNDINGMODE_TOWARD_ZERO:      return false;
        case ROUNDINGMODE_AWAY_FROM_ZERO:   return true;
        case ROUNDINGMODE_FLOOR:            return negative;
        case ROUNDINGMODE_CEILING:          return !negative;
        case ROUNDINGMODE_HALF_UP:          return ROUNDRELATION_BELOW_HALF != relation;
        case ROUNDINGMODE_HALF_DOWN:        return ROUNDRELATION_ABOVE_HALF == relation;
        case ROUNDINGMODE_HALF_EVEN:        return ROUNDRELATION_ABOVE_HALF == relation || ( ROUNDRELATION_HALF == relation && quotientIsOdd );
        case ROUNDINGMODE_UNNECESSARY:
        default:                            return false;
    }
}

/*-------------------------------------------------------------------------*/

/** Applies the rounding decision to the magnitude quotient and narrows it to the symmetric 64 bit range */
static TDecimalStatus
FinishRounded( bool negative                ,
               CUInt128 quotient            ,
               ERoundRelation relation      ,
               TRoundingMode mode           ,
               UInt8 resultScale            ,
               CDecimal& result             )
{
    // A truncated quotient that does not fit overflows in every rounding mode
    if ( !quotient.FitsUInt64() || quotient.low > g_decimalMaxMagnitude )
        return DECIMALSTATUS_OVERFLOW;

    TDecimalStatus status = DECIMALSTATUS_OK;
    if ( ROUNDRELATION_EXACT != relation )
    {
        if ( ROUNDINGMODE_UNNECESSARY == mode )
            return DECIMALSTATUS_ROUNDING_REQUIRED;

        status = DECIMALSTATUS_INEXACT;
        if ( RoundingIncrements( mode, negative, quotient.IsOdd(), relation ) )
            quotient = CUInt128::Add( quotient, CUInt128( 1 ) );

        // The check also runs after rounding: an increment can carry over the limit
        if ( quotient.low > g_decimalMaxMagnitude )
            return DECIMALSTATUS_OVERFLOW;
    }

    Int64 magnitude = (Int64) quotient.low;
    TDecimalStatus setStatus = CDecimal::FromUnscaled( negative ? -magnitude : magnitude, resultScale, result );
    return status | setStatus;
}

/*-------------------------------------------------------------------------*/

/**
 *  The shared kernel: result = round( sign * magnitude * 10^exponent10 / divisor ) at resultScale.
 *  A positive exponent multiplies first (checked: beyond 128 bit the quotient cannot fit 64 bit since divisor < 2^64).
 *  A negative exponent divides by powers of ten after the divisor. Every divisor stays within 64 bit and the
 *  arbitrary divisor goes first so the last divisor of a chain is even (a power of ten) or the only one, which keeps
 *  the half way classification exact.
 */
static TDecimalStatus
ScaleDivideRound( bool negative             ,
                  const CUInt128& magnitude ,
                  Int32 exponent10          ,
                  UInt64 divisor            ,
                  TRoundingMode mode        ,
                  UInt8 resultScale         ,
                  CDecimal& result          )
{
    if ( resultScale > CDecimal::MaxScale )
        return DECIMALSTATUS_INVALID_SCALE;
    if ( !IsValidRoundingMode( mode ) )
        return DECIMALSTATUS_INVALID_OPERAND;
    if ( 0 == divisor )
        return DECIMALSTATUS_DIVISION_BY_ZERO;

    CUInt128 value = magnitude;
    if ( exponent10 > 0 )
    {
        if ( !CUInt128::MultiplyByPow10Checked( value, (UInt32) exponent10 ) )
            return DECIMALSTATUS_OVERFLOW;
    }

    bool divided = false;
    bool sticky = false;
    UInt64 lastRemainder = 0;
    UInt64 lastDivisor = 1;

    if ( divisor > 1 )
    {
        CUInt128::DivMod( value, divisor, value, lastRemainder );
        lastDivisor = divisor;
        divided = true;
    }
    if ( exponent10 < 0 )
    {
        UInt32 remaining = (UInt32) ( -exponent10 );
        while ( remaining > 0 )
        {
            UInt32 step = remaining > 19 ? 19 : remaining;
            if ( divided && 0 != lastRemainder )
                sticky = true;
            CUInt128::DivMod( value, g_decimalPow10[ step ], value, lastRemainder );
            lastDivisor = g_decimalPow10[ step ];
            divided = true;
            remaining -= step;
        }
    }

    ERoundRelation relation = divided ? ClassifyRemainder( lastRemainder, lastDivisor, sticky ) : ROUNDRELATION_EXACT;
    return FinishRounded( negative, value, relation, mode, resultScale, result );
}

/*-------------------------------------------------------------------------*/

/** Sign-magnitude addition of 128 bit magnitudes that cannot overflow 128 bit (each < 2^126) */
static void
SignedAdd128( bool negativeA, const CUInt128& a, bool negativeB, const CUInt128& b, bool& negative, CUInt128& magnitude )
{
    if ( negativeA == negativeB )
    {
        magnitude = CUInt128::Add( a, b );
        negative = negativeA;
    }
    else
    if ( CUInt128::Compare( a, b ) >= 0 )
    {
        magnitude = CUInt128::Subtract( a, b );
        negative = negativeA;
    }
    else
    {
        magnitude = CUInt128::Subtract( b, a );
        negative = negativeB;
    }
    if ( magnitude.IsZero() )
        negative = false;
}

/*-------------------------------------------------------------------------*/

/** Exact sum or difference aligned to the largest scale: magnitudes < 2^63 * 10^18 < 2^123 */
static void
AlignedSum( const CDecimal& a, const CDecimal& b, bool subtract, bool& negative, CUInt128& magnitude, UInt8& scale )
{
    scale = a.GetScale() > b.GetScale() ? a.GetScale() : b.GetScale();
    CUInt128 alignedA = CUInt128::Multiply( MagnitudeOf( a.GetUnscaledValue() ), g_decimalPow10[ scale - a.GetScale() ] );
    CUInt128 alignedB = CUInt128::Multiply( MagnitudeOf( b.GetUnscaledValue() ), g_decimalPow10[ scale - b.GetScale() ] );
    bool negativeB = b.IsNegative() != subtract;
    if ( b.IsZero() )
        negativeB = false;
    SignedAdd128( a.IsNegative(), alignedA, negativeB, alignedB, negative, magnitude );
}

/*-------------------------------------------------------------------------*/

static inline bool
IsDigit( char c )
{
    return c >= '0' && c <= '9';
}

/*-------------------------------------------------------------------------*/

static inline bool
IsSpaceOrTab( char c )
{
    return ' ' == c || '\t' == c;
}

/*-------------------------------------------------------------------------*/

/** Shared parser. Without a target scale the natural scale of the text is used and the result has to be exact */
static TDecimalStatus
ParseDecimalChars( const char* text         ,
                   UInt32 length            ,
                   bool useTargetScale      ,
                   UInt8 targetScale        ,
                   TRoundingMode mode       ,
                   UInt32 parseFlags        ,
                   CDecimal& result         )
{
    if ( GUCEF_NULL == text )
        return DECIMALSTATUS_INVALID_FORMAT;

    UInt32 i = 0;
    UInt32 end = length;
    if ( 0 != ( parseFlags & DECIMALPARSE_ALLOW_SURROUNDING_WHITESPACE ) )
    {
        while ( i < end && IsSpaceOrTab( text[ i ] ) )
            ++i;
        while ( end > i && IsSpaceOrTab( text[ end-1 ] ) )
            --end;
    }

    bool negative = false;
    if ( i < end && ( '-' == text[ i ] || '+' == text[ i ] ) )
    {
        negative = '-' == text[ i ];
        ++i;
    }

    UInt32 intStart = i;
    while ( i < end && IsDigit( text[ i ] ) )
        ++i;
    UInt32 intDigits = i - intStart;

    UInt32 fracStart = i;
    UInt32 fracDigits = 0;
    if ( i < end && '.' == text[ i ] )
    {
        ++i;
        fracStart = i;
        while ( i < end && IsDigit( text[ i ] ) )
            ++i;
        fracDigits = i - fracStart;
    }

    if ( 0 == intDigits && 0 == fracDigits )
        return DECIMALSTATUS_INVALID_FORMAT;

    Int64 exponent = 0;
    if ( i < end && ( 'e' == text[ i ] || 'E' == text[ i ] ) )
    {
        if ( 0 == ( parseFlags & DECIMALPARSE_ALLOW_EXPONENT ) )
            return DECIMALSTATUS_INVALID_FORMAT;
        ++i;
        bool negativeExponent = false;
        if ( i < end && ( '-' == text[ i ] || '+' == text[ i ] ) )
        {
            negativeExponent = '-' == text[ i ];
            ++i;
        }
        UInt32 expStart = i;
        while ( i < end && IsDigit( text[ i ] ) )
        {
            // clamped: beyond this the value overflows or rounds to zero at any scale
            if ( exponent < 1000000000000LL )
                exponent = exponent * 10 + ( text[ i ] - '0' );
            ++i;
        }
        if ( i == expStart )
            return DECIMALSTATUS_INVALID_FORMAT;
        if ( negativeExponent )
            exponent = -exponent;
    }

    if ( i != end )
        return DECIMALSTATUS_INVALID_FORMAT;

    // value = D * 10^-naturalScale where D is the digit sequence integer digits followed by fraction digits
    Int64 naturalScale = (Int64) fracDigits - exponent;

    Int64 scale = 0;
    if ( useTargetScale )
    {
        if ( targetScale > CDecimal::MaxScale )
            return DECIMALSTATUS_INVALID_SCALE;
        if ( !IsValidRoundingMode( mode ) )
            return DECIMALSTATUS_INVALID_OPERAND;
        scale = targetScale;
    }
    else
    {
        if ( naturalScale > (Int64) CDecimal::MaxScale )
            return DECIMALSTATUS_INVALID_SCALE;
        scale = naturalScale < 0 ? 0 : naturalScale;
        mode = ROUNDINGMODE_UNNECESSARY;
    }

    UInt32 totalDigits = intDigits + fracDigits;
    Int64 shift = scale - naturalScale;     // the result unscaled value is D * 10^shift

    UInt32 keptDigits = totalDigits;
    ERoundRelation relation = ROUNDRELATION_EXACT;
    if ( shift < 0 )
    {
        Int64 drop = -shift;
        if ( drop > (Int64) totalDigits )
        {
            // the first discarded digit is an implied leading zero
            keptDigits = 0;
            for ( UInt32 n=0; n<totalDigits; ++n )
            {
                char c = n < intDigits ? text[ intStart + n ] : text[ fracStart + n - intDigits ];
                if ( '0' != c )
                {
                    relation = ROUNDRELATION_BELOW_HALF;
                    break;
                }
            }
        }
        else
        {
            keptDigits = totalDigits - (UInt32) drop;
            bool restNonZero = false;
            char firstDiscarded = '0';
            for ( UInt32 n=keptDigits; n<totalDigits; ++n )
            {
                char c = n < intDigits ? text[ intStart + n ] : text[ fracStart + n - intDigits ];
                if ( n == keptDigits )
                    firstDiscarded = c;
                else
                if ( '0' != c )
                    restNonZero = true;
            }
            if ( firstDiscarded > '5' )
                relation = ROUNDRELATION_ABOVE_HALF;
            else
            if ( '5' == firstDiscarded )
                relation = restNonZero ? ROUNDRELATION_ABOVE_HALF : ROUNDRELATION_HALF;
            else
            if ( '0' != firstDiscarded || restNonZero )
                relation = ROUNDRELATION_BELOW_HALF;
        }
        shift = 0;
    }

    // accumulate the kept digits, leading zeros cost nothing
    UInt64 accumulator = 0;
    for ( UInt32 n=0; n<keptDigits; ++n )
    {
        char c = n < intDigits ? text[ intStart + n ] : text[ fracStart + n - intDigits ];
        UInt64 digit = (UInt64) ( c - '0' );
        if ( accumulator > ( g_decimalMaxMagnitude - digit ) / 10 )
            return DECIMALSTATUS_OVERFLOW;
        accumulator = accumulator * 10 + digit;
    }

    CUInt128 magnitude( accumulator );
    if ( shift > 0 && 0 != accumulator )
    {
        if ( shift > 19 || !CUInt128::MultiplyByPow10Checked( magnitude, (UInt32) shift ) )
            return DECIMALSTATUS_OVERFLOW;
    }
    return FinishRounded( negative, magnitude, relation, mode, (UInt8) scale, result );
}

/*-------------------------------------------------------------------------*/

/** Character data of a CString for parsing, converted only where CString is not a narrow string */
static TDecimalStatus
ParseDecimalString( const CString& text, bool useTargetScale, UInt8 targetScale, TRoundingMode mode, UInt32 parseFlags, CDecimal& result )
{
    #if ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_ASCII_STRING ) || ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_UTF8_STRING )
    const CString& narrow = text;
    #else
    CUtf8String narrow = ToUtf8String( text );
    #endif

    UInt32 byteSize = narrow.ByteSize();
    const char* chars = narrow.C_String();
    if ( GUCEF_NULL == chars || 0 == byteSize )
        return ParseDecimalChars( "", 0, useTargetScale, targetScale, mode, parseFlags, result );
    if ( '\0' == chars[ byteSize-1 ] )
        --byteSize;
    return ParseDecimalChars( chars, byteSize, useTargetScale, targetScale, mode, parseFlags, result );
}

/*-------------------------------------------------------------------------*/

static inline bool
IsFiniteFloat64( Float64 value )
{
    // false for NaN and both infinities, without relying on C99 / C++11 classification functions
    return ( value - value ) == ( value - value ) && value == value;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDecimalContext::CDecimalContext( UInt8 resultScale, TRoundingMode roundingMode )
    : m_resultScale( resultScale )
    , m_roundingMode( (UInt8) roundingMode )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::FromUnscaled( Int64 unscaledValue, UInt8 scale, CDecimal& result )
{GUCEF_TRACE;

    TDecimalStatus status = DECIMALSTATUS_OK;
    if ( GUCEF_INT64MIN == unscaledValue )
        status |= DECIMALSTATUS_INVALID_OPERAND;
    if ( scale > MaxScale )
        status |= DECIMALSTATUS_INVALID_SCALE;
    if ( DECIMALSTATUS_OK != status )
        return status;

    result.m_unscaled = unscaledValue;
    result.m_scale = scale;
    return DECIMALSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::FromInt64( Int64 value, UInt8 scale, CDecimal& result )
{GUCEF_TRACE;

    if ( GUCEF_INT64MIN == value )
        return DECIMALSTATUS_INVALID_OPERAND;
    return ScaleDivideRound( value < 0, CUInt128( MagnitudeOf( value ) ), (Int32) scale, 1, ROUNDINGMODE_UNNECESSARY, scale, result );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::FromString( const CString& text, CDecimal& result, UInt32 parseFlags )
{GUCEF_TRACE;

    return ParseDecimalString( text, false, 0, ROUNDINGMODE_UNNECESSARY, parseFlags, result );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::FromString( const CString& text, UInt8 scale, TRoundingMode mode, CDecimal& result, UInt32 parseFlags )
{GUCEF_TRACE;

    return ParseDecimalString( text, true, scale, mode, parseFlags, result );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::FromChars( const char* text, UInt32 length, CDecimal& result, UInt32 parseFlags )
{GUCEF_TRACE;

    return ParseDecimalChars( text, length, false, 0, ROUNDINGMODE_UNNECESSARY, parseFlags, result );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::FromChars( const char* text, UInt32 length, UInt8 scale, TRoundingMode mode, CDecimal& result, UInt32 parseFlags )
{GUCEF_TRACE;

    return ParseDecimalChars( text, length, true, scale, mode, parseFlags, result );
}

/*-------------------------------------------------------------------------*/

CDecimal
CDecimal::Parse( const char* literal )
{GUCEF_TRACE;

    CDecimal result;
    UInt32 length = GUCEF_NULL != literal ? (UInt32) strlen( literal ) : 0;
    TDecimalStatus status = ParseDecimalChars( literal, length, false, 0, ROUNDINGMODE_UNNECESSARY, DECIMALPARSE_STRICT, result );
    if ( DECIMALSTATUS_OK != status )
        throw GUCEF::invalid_argument( __FILE__, __LINE__, "CDecimal::Parse: the literal is not a valid decimal in the strict grammar" );
    return result;
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::FromFloat64( Float64 value, UInt8 scale, TRoundingMode mode, CDecimal& result )
{GUCEF_TRACE;

    if ( !IsFiniteFloat64( value ) )
        return DECIMALSTATUS_INVALID_OPERAND;
    if ( scale > MaxScale )
        return DECIMALSTATUS_INVALID_SCALE;
    if ( !IsValidRoundingMode( mode ) )
        return DECIMALSTATUS_INVALID_OPERAND;
    if ( 0.0 == value )
        return FromUnscaled( 0, scale, result );

    bool negative = value < 0.0;
    int binaryExponent = 0;
    Float64 fraction = frexp( negative ? -value : value, &binaryExponent );     // value = fraction * 2^binaryExponent, fraction in [0.5, 1)
    UInt64 mantissa = (UInt64) ldexp( fraction, 53 );                           // exact integer below 2^53
    Int32 exponent = (Int32) binaryExponent - 53;                               // value = mantissa * 2^exponent

    // mantissa * 10^scale < 2^53 * 2^60 = 2^113
    CUInt128 magnitude = CUInt128::Multiply( mantissa, g_decimalPow10[ scale ] );
    if ( exponent >= 0 )
    {
        if ( magnitude.BitLength() + (UInt32) exponent > 128 )
            return DECIMALSTATUS_OVERFLOW;
        magnitude = CUInt128::ShiftLeft( magnitude, (UInt32) exponent );
        return FinishRounded( negative, magnitude, ROUNDRELATION_EXACT, mode, scale, result );
    }

    UInt32 shift = (UInt32) ( -exponent );
    if ( shift >= 128 )
    {
        // magnitude < 2^113 so the value is below 2^-15: below one half of a unit
        return FinishRounded( negative, CUInt128(), ROUNDRELATION_BELOW_HALF, mode, scale, result );
    }

    CUInt128 quotient = CUInt128::ShiftRight( magnitude, shift );
    CUInt128 discarded = CUInt128::Subtract( magnitude, CUInt128::ShiftLeft( quotient, shift ) );
    ERoundRelation relation = ROUNDRELATION_EXACT;
    if ( !discarded.IsZero() )
    {
        CUInt128 half = CUInt128::ShiftLeft( CUInt128( 1 ), shift - 1 );
        Int32 cmp = CUInt128::Compare( discarded, half );
        relation = cmp < 0 ? ROUNDRELATION_BELOW_HALF : ( cmp > 0 ? ROUNDRELATION_ABOVE_HALF : ROUNDRELATION_HALF );
    }
    return FinishRounded( negative, quotient, relation, mode, scale, result );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::FromFloat64Shortest( Float64 value, CDecimal& result )
{GUCEF_TRACE;

    if ( !IsFiniteFloat64( value ) )
        return DECIMALSTATUS_INVALID_OPERAND;

    for ( UInt8 scale=0; scale<=MaxScale; ++scale )
    {
        CDecimal candidate;
        TDecimalStatus status = FromFloat64( value, scale, ROUNDINGMODE_HALF_EVEN, candidate );
        if ( DecimalStatusIsError( status ) )
            return status;      // an overflow only gets worse at a larger scale
        if ( candidate.ToFloat64() == value )
        {
            result = candidate;
            return status;
        }
    }
    return DECIMALSTATUS_INVALID_SCALE;
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::FromFloat32Shortest( Float32 value, CDecimal& result )
{GUCEF_TRACE;

    Float64 wide = (Float64) value;
    if ( !IsFiniteFloat64( wide ) )
        return DECIMALSTATUS_INVALID_OPERAND;

    for ( UInt8 scale=0; scale<=MaxScale; ++scale )
    {
        CDecimal candidate;
        TDecimalStatus status = FromFloat64( wide, scale, ROUNDINGMODE_HALF_EVEN, candidate );
        if ( DecimalStatusIsError( status ) )
            return status;
        if ( (Float32) candidate.ToFloat64() == value )
        {
            result = candidate;
            return status;
        }
    }
    return DECIMALSTATUS_INVALID_SCALE;
}

/*-------------------------------------------------------------------------*/

UInt8
CDecimal::GetDigitCount( void ) const
{GUCEF_TRACE;

    UInt64 magnitude = MagnitudeOf( m_unscaled );
    UInt8 digits = 1;
    while ( digits < 20 && magnitude >= g_decimalPow10[ digits ] )
        ++digits;
    return digits;
}

/*-------------------------------------------------------------------------*/

bool
CDecimal::FitsPrecision( UInt8 precision ) const
{GUCEF_TRACE;

    if ( precision >= 19 )
        return true;    // every magnitude is below 2^63 < 10^19 - 1
    return MagnitudeOf( m_unscaled ) <= g_decimalPow10[ precision ] - 1;
}

/*-------------------------------------------------------------------------*/

bool
CDecimal::IsIntegral( void ) const
{GUCEF_TRACE;

    return 0 == m_scale || 0 == ( MagnitudeOf( m_unscaled ) % g_decimalPow10[ m_scale ] );
}

/*-------------------------------------------------------------------------*/

CDecimal
CDecimal::Normalized( void ) const
{GUCEF_TRACE;

    if ( 0 == m_unscaled )
        return CDecimal();

    Int64 unscaled = m_unscaled;
    UInt8 scale = m_scale;
    while ( scale > 0 && 0 == unscaled % 10 )
    {
        unscaled /= 10;
        --scale;
    }
    return CDecimal( unscaled, scale, true );
}

/*-------------------------------------------------------------------------*/

UInt32
CDecimal::ToChars( char* buffer, UInt32 bufferSize ) const
{GUCEF_TRACE;

    char digits[ MaxTextLength + 3 ];
    UInt32 pos = (UInt32) sizeof( digits );
    UInt64 magnitude = MagnitudeOf( m_unscaled );
    UInt32 produced = 0;
    do
    {
        digits[ --pos ] = (char) ( '0' + (char) ( magnitude % 10 ) );
        magnitude /= 10;
        ++produced;
        if ( produced == m_scale )
            digits[ --pos ] = '.';
    }
    while ( 0 != magnitude || produced <= m_scale );

    if ( m_unscaled < 0 )
        digits[ --pos ] = '-';

    UInt32 length = (UInt32) sizeof( digits ) - pos;
    if ( GUCEF_NULL == buffer || bufferSize < length + 1 )
        return 0;
    memcpy( buffer, digits + pos, length );
    buffer[ length ] = '\0';
    return length;
}

/*-------------------------------------------------------------------------*/

CString
CDecimal::ToString( void ) const
{GUCEF_TRACE;

    char buffer[ MaxTextLength + 1 ];
    UInt32 length = ToChars( buffer, (UInt32) sizeof( buffer ) );

    #if ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_ASCII_STRING ) || ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_UTF8_STRING )
    return CString( buffer, length );
    #else
    return CORE::ToString( CUtf8String( buffer, length ) );
    #endif
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::ToString( UInt8 displayScale, TRoundingMode mode, CString& text ) const
{GUCEF_TRACE;

    CDecimal display;
    TDecimalStatus status = Rescale( displayScale, mode, display );
    if ( DecimalStatusIsError( status ) )
        return status;
    text = display.ToString();
    return status;
}

/*-------------------------------------------------------------------------*/

Float64
CDecimal::ToFloat64( void ) const
{GUCEF_TRACE;

    if ( 0 == m_unscaled )
        return 0.0;

    UInt64 magnitude = MagnitudeOf( m_unscaled );
    Float64 value = 0.0;
    if ( magnitude <= ( 1ULL << 53 ) )
    {
        // both operands are exact doubles, the IEEE division rounds once
        value = (Float64) magnitude / g_decimalPow10Float64[ m_scale ];
    }
    else
    if ( 0 == m_scale )
    {
        // the integer to double conversion rounds once, to nearest even
        value = (Float64) magnitude;
    }
    else
    {
        // scaled division producing a quotient of at least 68 significant bits plus a sticky bit, rounded once to 53 bits
        UInt32 bitLength = 64 - CUInt128::CountLeadingZeros( magnitude );
        UInt32 shift = 128 - bitLength;
        CUInt128 numerator = CUInt128::ShiftLeft( CUInt128( magnitude ), shift );
        CUInt128 quotient;
        UInt64 remainder = 0;
        CUInt128::DivMod( numerator, g_decimalPow10[ m_scale ], quotient, remainder );

        UInt32 drop = quotient.BitLength() - 53;
        CUInt128 significand = CUInt128::ShiftRight( quotient, drop );
        CUInt128 discarded = CUInt128::Subtract( quotient, CUInt128::ShiftLeft( significand, drop ) );
        CUInt128 half = CUInt128::ShiftLeft( CUInt128( 1 ), drop - 1 );
        Int32 cmp = CUInt128::Compare( discarded, half );
        bool roundUp = cmp > 0 || ( 0 == cmp && ( 0 != remainder || significand.IsOdd() ) );
        UInt64 bits = significand.low;
        if ( roundUp )
            ++bits;     // 2^53 is still exact
        value = ldexp( (Float64) bits, (int) drop - (int) shift );
    }
    return m_unscaled < 0 ? -value : value;
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::ToInt64( TRoundingMode mode, Int64& result ) const
{GUCEF_TRACE;

    CDecimal integral;
    TDecimalStatus status = Rescale( 0, mode, integral );
    if ( DecimalStatusIsError( status ) )
        return status;
    result = integral.m_unscaled;
    return status;
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::AddAligned( const CDecimal& other, bool subtract, CDecimal& result ) const
{GUCEF_TRACE;

    bool negative = false;
    CUInt128 magnitude;
    UInt8 scale = 0;
    AlignedSum( *this, other, subtract, negative, magnitude, scale );
    return ScaleDivideRound( negative, magnitude, 0, 1, ROUNDINGMODE_UNNECESSARY, scale, result );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::Add( const CDecimal& other, const CDecimalContext& ctx, CDecimal& result ) const
{GUCEF_TRACE;

    bool negative = false;
    CUInt128 magnitude;
    UInt8 scale = 0;
    AlignedSum( *this, other, false, negative, magnitude, scale );
    return ScaleDivideRound( negative, magnitude, (Int32) ctx.GetResultScale() - (Int32) scale, 1, ctx.GetRoundingMode(), ctx.GetResultScale(), result );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::Subtract( const CDecimal& other, const CDecimalContext& ctx, CDecimal& result ) const
{GUCEF_TRACE;

    bool negative = false;
    CUInt128 magnitude;
    UInt8 scale = 0;
    AlignedSum( *this, other, true, negative, magnitude, scale );
    return ScaleDivideRound( negative, magnitude, (Int32) ctx.GetResultScale() - (Int32) scale, 1, ctx.GetRoundingMode(), ctx.GetResultScale(), result );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::Multiply( const CDecimal& other, CDecimal& result ) const
{GUCEF_TRACE;

    UInt32 scale = (UInt32) m_scale + (UInt32) other.m_scale;
    if ( scale > MaxScale )
        return DECIMALSTATUS_INVALID_SCALE;

    UInt64 a = MagnitudeOf( m_unscaled );
    UInt64 b = MagnitudeOf( other.m_unscaled );
    bool negative = ( m_unscaled < 0 ) != ( other.m_unscaled < 0 );
    if ( a <= g_decimalMaxFastMultiplyOperand && b <= g_decimalMaxFastMultiplyOperand )
    {
        // the 64 bit product cannot overflow
        Int64 product = (Int64) ( a * b );
        result.m_unscaled = negative ? -product : product;
        result.m_scale = (UInt8) scale;
        return DECIMALSTATUS_OK;
    }
    return ScaleDivideRound( negative, CUInt128::Multiply( a, b ), 0, 1, ROUNDINGMODE_UNNECESSARY, (UInt8) scale, result );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::Multiply( const CDecimal& other, const CDecimalContext& ctx, CDecimal& result ) const
{GUCEF_TRACE;

    bool negative = ( m_unscaled < 0 ) != ( other.m_unscaled < 0 );
    CUInt128 product = CUInt128::Multiply( MagnitudeOf( m_unscaled ), MagnitudeOf( other.m_unscaled ) );
    Int32 exponent = (Int32) ctx.GetResultScale() - (Int32) m_scale - (Int32) other.m_scale;
    return ScaleDivideRound( negative, product, exponent, 1, ctx.GetRoundingMode(), ctx.GetResultScale(), result );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::Multiply( Int64 factor, CDecimal& result ) const
{GUCEF_TRACE;

    bool negative = ( m_unscaled < 0 ) != ( factor < 0 );
    CUInt128 product = CUInt128::Multiply( MagnitudeOf( m_unscaled ), MagnitudeOf( factor ) );
    return ScaleDivideRound( negative, product, 0, 1, ROUNDINGMODE_UNNECESSARY, m_scale, result );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::Divide( const CDecimal& divisor, const CDecimalContext& ctx, CDecimal& result ) const
{GUCEF_TRACE;

    if ( 0 == divisor.m_unscaled )
        return DECIMALSTATUS_DIVISION_BY_ZERO;

    // q = a * 10^( t + sb - sa ) / b
    bool negative = ( m_unscaled < 0 ) != ( divisor.m_unscaled < 0 );
    Int32 exponent = (Int32) ctx.GetResultScale() + (Int32) divisor.m_scale - (Int32) m_scale;
    return ScaleDivideRound( negative, CUInt128( MagnitudeOf( m_unscaled ) ), exponent, MagnitudeOf( divisor.m_unscaled ), ctx.GetRoundingMode(), ctx.GetResultScale(), result );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::Divide( Int64 divisor, const CDecimalContext& ctx, CDecimal& result ) const
{GUCEF_TRACE;

    if ( 0 == divisor )
        return DECIMALSTATUS_DIVISION_BY_ZERO;

    bool negative = ( m_unscaled < 0 ) != ( divisor < 0 );
    Int32 exponent = (Int32) ctx.GetResultScale() - (Int32) m_scale;
    return ScaleDivideRound( negative, CUInt128( MagnitudeOf( m_unscaled ) ), exponent, MagnitudeOf( divisor ), ctx.GetRoundingMode(), ctx.GetResultScale(), result );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::MulDiv( const CDecimal& factor, const CDecimal& divisor, const CDecimalContext& ctx, CDecimal& result ) const
{GUCEF_TRACE;

    if ( 0 == divisor.m_unscaled )
        return DECIMALSTATUS_DIVISION_BY_ZERO;

    // q = a * b * 10^( t + sc - sa - sb ) / c
    bool negative = ( ( m_unscaled < 0 ) != ( factor.m_unscaled < 0 ) ) != ( divisor.m_unscaled < 0 );
    CUInt128 product = CUInt128::Multiply( MagnitudeOf( m_unscaled ), MagnitudeOf( factor.m_unscaled ) );
    Int32 exponent = (Int32) ctx.GetResultScale() + (Int32) divisor.m_scale - (Int32) m_scale - (Int32) factor.m_scale;
    return ScaleDivideRound( negative, product, exponent, MagnitudeOf( divisor.m_unscaled ), ctx.GetRoundingMode(), ctx.GetResultScale(), result );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::PercentOf( const CDecimal& percent, const CDecimalContext& ctx, CDecimal& result ) const
{GUCEF_TRACE;

    // a * p / 100 at scale t: a * p * 10^( t - sa - sp - 2 )
    bool negative = ( m_unscaled < 0 ) != ( percent.m_unscaled < 0 );
    CUInt128 product = CUInt128::Multiply( MagnitudeOf( m_unscaled ), MagnitudeOf( percent.m_unscaled ) );
    Int32 exponent = (Int32) ctx.GetResultScale() - (Int32) m_scale - (Int32) percent.m_scale - 2;
    return ScaleDivideRound( negative, product, exponent, 1, ctx.GetRoundingMode(), ctx.GetResultScale(), result );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::Rescale( UInt8 newScale, TRoundingMode mode, CDecimal& result ) const
{GUCEF_TRACE;

    return ScaleDivideRound( m_unscaled < 0, CUInt128( MagnitudeOf( m_unscaled ) ), (Int32) newScale - (Int32) m_scale, 1, mode, newScale, result );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::MovePointLeft( UInt8 places, CDecimal& result ) const
{GUCEF_TRACE;

    UInt32 scale = (UInt32) m_scale + (UInt32) places;
    if ( scale > MaxScale )
        return DECIMALSTATUS_INVALID_SCALE;
    result.m_unscaled = m_unscaled;
    result.m_scale = (UInt8) scale;
    return DECIMALSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::MovePointRight( UInt8 places, CDecimal& result ) const
{GUCEF_TRACE;

    if ( places <= m_scale )
    {
        result.m_unscaled = m_unscaled;
        result.m_scale = (UInt8) ( m_scale - places );
        return DECIMALSTATUS_OK;
    }
    return ScaleDivideRound( m_unscaled < 0, CUInt128( MagnitudeOf( m_unscaled ) ), (Int32) places - (Int32) m_scale, 1, ROUNDINGMODE_UNNECESSARY, 0, result );
}

/*-------------------------------------------------------------------------*/

Int32
CDecimal::CompareAligned( const CDecimal& other ) const
{GUCEF_TRACE;

    Int32 sign = GetSign();
    Int32 otherSign = other.GetSign();
    if ( sign != otherSign )
        return sign < otherSign ? -1 : 1;
    if ( 0 == sign )
        return 0;

    // same sign: compare the magnitudes aligned to the larger scale, never overflows 128 bit (2^63 * 10^18 < 2^123)
    UInt8 scale = m_scale > other.m_scale ? m_scale : other.m_scale;
    UInt8 diffThis = (UInt8) ( scale - m_scale );
    UInt8 diffOther = (UInt8) ( scale - other.m_scale );
    UInt64 magnitudeThis = MagnitudeOf( m_unscaled );
    UInt64 magnitudeOther = MagnitudeOf( other.m_unscaled );

    Int32 cmp = 0;
    if ( magnitudeThis <= g_decimalMaxUnscaledForUpscale[ diffThis ] && magnitudeOther <= g_decimalMaxUnscaledForUpscale[ diffOther ] )
    {
        UInt64 a = magnitudeThis * g_decimalPow10[ diffThis ];
        UInt64 b = magnitudeOther * g_decimalPow10[ diffOther ];
        cmp = a < b ? -1 : ( a > b ? 1 : 0 );
    }
    else
    {
        cmp = CUInt128::Compare( CUInt128::Multiply( magnitudeThis, g_decimalPow10[ diffThis ] ),
                                 CUInt128::Multiply( magnitudeOther, g_decimalPow10[ diffOther ] ) );
    }
    return sign < 0 ? -cmp : cmp;
}

/*-------------------------------------------------------------------------*/

UInt64
CDecimal::GetHash( void ) const
{GUCEF_TRACE;

    CDecimal normalized = Normalized();
    UInt64 z = (UInt64) normalized.m_unscaled + 0x9E3779B97F4A7C15ULL * ( (UInt64) normalized.m_scale + 1 );
    z = ( z ^ ( z >> 30 ) ) * 0xBF58476D1CE4E5B9ULL;
    z = ( z ^ ( z >> 27 ) ) * 0x94D049BB133111EBULL;
    return z ^ ( z >> 31 );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::Sum( const CDecimal* values, UInt32 count, CDecimal& result )
{GUCEF_TRACE;

    if ( GUCEF_NULL == values && count > 0 )
        return DECIMALSTATUS_INVALID_OPERAND;

    UInt8 scale = 0;
    for ( UInt32 i=0; i<count; ++i )
        if ( values[ i ].m_scale > scale )
            scale = values[ i ].m_scale;

    CDecimalAccumulator accumulator( scale );
    for ( UInt32 i=0; i<count; ++i )
        accumulator.Add( values[ i ] );
    return accumulator.GetResult( result );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::Sum( const CDecimal* values, UInt32 count, const CDecimalContext& ctx, CDecimal& result )
{GUCEF_TRACE;

    if ( GUCEF_NULL == values && count > 0 )
        return DECIMALSTATUS_INVALID_OPERAND;

    UInt8 scale = 0;
    for ( UInt32 i=0; i<count; ++i )
        if ( values[ i ].m_scale > scale )
            scale = values[ i ].m_scale;

    CDecimalAccumulator accumulator( scale );
    for ( UInt32 i=0; i<count; ++i )
        accumulator.Add( values[ i ] );
    return accumulator.GetResult( ctx, result );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::Allocate( const CDecimal& total, const CDecimal* weights, UInt32 count, CDecimal* parts )
{GUCEF_TRACE;

    if ( count > 0 && ( GUCEF_NULL == weights || GUCEF_NULL == parts ) )
        return DECIMALSTATUS_INVALID_OPERAND;

    // validation pass: nothing is written before every error condition has been ruled out
    UInt8 weightScale = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( weights[ i ].IsNegative() )
            return DECIMALSTATUS_INVALID_OPERAND;
        if ( weights[ i ].m_scale > weightScale )
            weightScale = weights[ i ].m_scale;
    }

    UInt64 weightSum = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        UInt64 w = (UInt64) weights[ i ].m_unscaled;
        UInt8 diff = (UInt8) ( weightScale - weights[ i ].m_scale );
        if ( w > g_decimalMaxUnscaledForUpscale[ diff ] )
            return DECIMALSTATUS_OVERFLOW;
        w *= g_decimalPow10[ diff ];
        if ( weightSum + w < weightSum )
            return DECIMALSTATUS_OVERFLOW;
        weightSum += w;
    }
    if ( 0 == weightSum )
        return DECIMALSTATUS_DIVISION_BY_ZERO;

    const Int64 totalUnscaled = total.m_unscaled;     // total may live in the parts array
    const UInt8 totalScale = total.m_scale;
    const UInt64 totalMagnitude = MagnitudeOf( totalUnscaled );
    const bool negative = totalUnscaled < 0;

    // share_i = T * w_i / W: the floors and the exact remainders r_i < W
    UInt64 floorSum = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        UInt64 w = (UInt64) weights[ i ].m_unscaled * g_decimalPow10[ weightScale - weights[ i ].m_scale ];
        CUInt128 quotient;
        UInt64 remainder = 0;
        CUInt128::DivMod( CUInt128::Multiply( totalMagnitude, w ), weightSum, quotient, remainder );
        parts[ i ].m_unscaled = (Int64) quotient.low;     // <= T
        parts[ i ].m_scale = totalScale;
        floorSum += quotient.low;
    }

    // the floors leave 'leftover' units (fewer than count), one each for the largest remainders, ties to the lowest index
    UInt64 leftover = totalMagnitude - floorSum;
    if ( leftover > 0 )
    {
        // largest threshold R with at least 'leftover' remainders >= R, found by binary search without extra memory
        UInt64 low = 1;
        UInt64 high = weightSum - 1;
        while ( low < high )
        {
            UInt64 mid = low + ( high - low + 1 ) / 2;
            UInt64 atLeast = 0;
            for ( UInt32 i=0; i<count; ++i )
            {
                UInt64 w = (UInt64) weights[ i ].m_unscaled * g_decimalPow10[ weightScale - weights[ i ].m_scale ];
                CUInt128 quotient;
                UInt64 remainder = 0;
                CUInt128::DivMod( CUInt128::Multiply( totalMagnitude, w ), weightSum, quotient, remainder );
                if ( remainder >= mid )
                    ++atLeast;
            }
            if ( atLeast >= leftover )
                low = mid;
            else
                high = mid - 1;
        }
        UInt64 threshold = low;

        // every remainder above the threshold gets a unit, then the ones equal to it from the lowest index
        UInt64 given = 0;
        for ( UInt32 pass=0; pass<2 && given<leftover; ++pass )
        {
            for ( UInt32 i=0; i<count && given<leftover; ++i )
            {
                UInt64 w = (UInt64) weights[ i ].m_unscaled * g_decimalPow10[ weightScale - weights[ i ].m_scale ];
                CUInt128 quotient;
                UInt64 remainder = 0;
                CUInt128::DivMod( CUInt128::Multiply( totalMagnitude, w ), weightSum, quotient, remainder );
                bool select = 0 == pass ? remainder > threshold : remainder == threshold;
                if ( select )
                {
                    ++parts[ i ].m_unscaled;
                    ++given;
                }
            }
        }
    }

    if ( negative )
    {
        for ( UInt32 i=0; i<count; ++i )
            parts[ i ].m_unscaled = -parts[ i ].m_unscaled;
    }
    return DECIMALSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CDecimal::RoundScaled( bool negative, UInt64 magnitudeHigh, UInt64 magnitudeLow, Int32 exponent10, TRoundingMode mode, UInt8 resultScale, CDecimal& result )
{GUCEF_TRACE;

    return ScaleDivideRound( negative, CUInt128( magnitudeHigh, magnitudeLow ), exponent10, 1, mode, resultScale, result );
}

/*-------------------------------------------------------------------------*/

bool
CDecimal::RepresentationLess::operator()( const CDecimal& a, const CDecimal& b ) const
{GUCEF_TRACE;

    Int32 cmp = a.Compare( b );
    if ( 0 != cmp )
        return cmp < 0;
    return a.GetScale() < b.GetScale();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
