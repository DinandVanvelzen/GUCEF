/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

#ifndef GUCEF_MATH_NUMERICROUNDINGFUNCTIONS_H
#define GUCEF_MATH_NUMERICROUNDINGFUNCTIONS_H

/*
 *  The rounding kernels shared by every numeric kind of gucefMATH.
 *
 *  Every conversion between kinds goes through one exact intermediate, SNumericValueParts:
 *
 *      value = ( negative ? -1 : 1 ) * magnitude * 2^exponent2 / denominator
 *
 *  Floats, fixed point and integers produce it with denominator 1; fractions and decimals with their denominator.
 *  The kernels below round such a value once, with a TRoundingMode, into an integer of at most 64 bits (fixed point
 *  raw values and integers) or into a Float32 / Float64. Everything is integer arithmetic on CUInt128, so the
 *  results are identical on every platform. All functions are header only, allocation free and realtime-safe.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#ifndef GUCEF_CORE_CUINT128_H
#include "gucefCORE_CUInt128.h"
#define GUCEF_CORE_CUINT128_H
#endif /* GUCEF_CORE_CUINT128_H ? */

#ifndef GUCEF_MATH_MATHSTATUS_H
#include "gucefMATH_MathStatus.h"
#define GUCEF_MATH_MATHSTATUS_H
#endif /* GUCEF_MATH_MATHSTATUS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Exact value of any kind that fits 64 bit integers:
 *  value = ( negative ? -1 : 1 ) * magnitude * 2^exponent2 / denominator, denominator >= 1.
 *  NaN and infinity are flags; 'negative' gives the sign of an infinity.
 */
struct SNumericValueParts
{
    bool isNaN;
    bool isInfinite;
    bool negative;
    UInt64 magnitude;
    Int32 exponent2;
    UInt64 denominator;
};
typedef struct SNumericValueParts TNumericValueParts;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

inline TNumericValueParts
MathMakeValueParts( bool negative, UInt64 magnitude, Int32 exponent2, UInt64 denominator ) GUCEF_MATH_REALTIME_CONTEXT
{
    TNumericValueParts parts;
    parts.isNaN = false;
    parts.isInfinite = false;
    parts.negative = negative && 0 != magnitude;
    parts.magnitude = magnitude;
    parts.exponent2 = exponent2;
    parts.denominator = denominator;
    return parts;
}

/*-------------------------------------------------------------------------*/

/** Number of significant bits of value, 0 for 0 */
GUCEF_MATH_FORCE_INLINE UInt32
MathBitLength( UInt64 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    return 64 - CORE::CUInt128::CountLeadingZeros( value );
}

/*-------------------------------------------------------------------------*/

/** Number of trailing zero bits, 64 for 0 */
GUCEF_MATH_FORCE_INLINE UInt32
MathCountTrailingZeros( UInt64 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CORE::CUInt128::CountTrailingZeros( value );
}

/*-------------------------------------------------------------------------*/

/** Two's complement magnitude of a signed 64 bit value, valid for the minimum (2^63) */
GUCEF_MATH_FORCE_INLINE UInt64
MathMagnitude( Int64 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    return value < 0 ? (UInt64) 0 - (UInt64) value : (UInt64) value;
}

/*-------------------------------------------------------------------------*/

/** Applies a sign to a magnitude <= 2^63 without signed overflow */
GUCEF_MATH_FORCE_INLINE Int64
MathApplySign( bool negative, UInt64 magnitude ) GUCEF_MATH_REALTIME_CONTEXT
{
    UInt64 bits = negative ? (UInt64) 0 - magnitude : magnitude;
    Int64 result;
    memcpy( &result, &bits, sizeof( result ) );
    return result;
}

/*-------------------------------------------------------------------------*/

/** Quiet NaN without a floating point operation */
inline Float64
MathQuietNaN64( void ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt64 bits = 0x7FF8000000000000ULL;
    Float64 value;
    memcpy( &value, &bits, sizeof( value ) );
    return value;
}

/*-------------------------------------------------------------------------*/

/** Positive infinity without a floating point operation */
inline Float64
MathInfinity64( void ) GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt64 bits = 0x7FF0000000000000ULL;
    Float64 value;
    memcpy( &value, &bits, sizeof( value ) );
    return value;
}

/*-------------------------------------------------------------------------*/

/** Classifies remainder r of a division by d (0 <= r < d) against d / 2 */
GUCEF_MATH_FORCE_INLINE TMathRemainderClass
MathClassifyRemainder( UInt64 r, UInt64 d ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == r )
        return MATHREMAINDER_ZERO;
    UInt64 rest = d - r;
    if ( r < rest )
        return MATHREMAINDER_BELOW_HALF;
    if ( r == rest )
        return MATHREMAINDER_HALF;
    return MATHREMAINDER_ABOVE_HALF;
}

/*-------------------------------------------------------------------------*/

inline TMathRemainderClass
MathClassifyRemainder( const CORE::CUInt128& r, const CORE::CUInt128& d ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( r.IsZero() )
        return MATHREMAINDER_ZERO;
    CORE::CUInt128 rest = CORE::CUInt128::Subtract( d, r );
    Int32 c = CORE::CUInt128::Compare( r, rest );
    if ( c < 0 )
        return MATHREMAINDER_BELOW_HALF;
    if ( 0 == c )
        return MATHREMAINDER_HALF;
    return MATHREMAINDER_ABOVE_HALF;
}

/*-------------------------------------------------------------------------*/

/**
 *  Classifies the 'drop' lowest bits of value against half of 2^drop, with a sticky flag for non-zero bits below them.
 *  drop may be 0 (then only the sticky flag counts, as a value below half) and may exceed 64.
 */
inline TMathRemainderClass
MathClassifyLowBits( UInt64 value, UInt32 drop, bool sticky ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == drop )
        return sticky ? MATHREMAINDER_BELOW_HALF : MATHREMAINDER_ZERO;
    if ( drop > 64 )
        return ( 0 != value || sticky ) ? MATHREMAINDER_BELOW_HALF : MATHREMAINDER_ZERO;
    UInt64 rem = 64 == drop ? value : ( value & ( ( (UInt64) 1 << drop ) - 1 ) );
    UInt64 half = (UInt64) 1 << ( drop - 1 );
    if ( 0 == rem )
        return sticky ? MATHREMAINDER_BELOW_HALF : MATHREMAINDER_ZERO;
    if ( rem < half )
        return MATHREMAINDER_BELOW_HALF;
    if ( rem == half )
        return sticky ? MATHREMAINDER_ABOVE_HALF : MATHREMAINDER_HALF;
    return MATHREMAINDER_ABOVE_HALF;
}

/*-------------------------------------------------------------------------*/

/**
 *  Rounds magnitude * 2^shift / denominator (denominator >= 1) to an integer magnitude with 'mode', for a 128 bit
 *  magnitude below 2^126. The sign is only used by the directed modes. ROUNDINGMODE_UNNECESSARY truncates and
 *  reports through 'inexact'.
 *
 *  @return false when the rounded magnitude is 2^64 or more ('result' is then undefined)
 */
inline bool
MathRoundScaledRatio128( bool negative                      ,
                         const CORE::CUInt128& magnitude    ,
                         Int32 shift                        ,
                         UInt64 denominator                 ,
                         TRoundingMode mode                 ,
                         UInt64& result                     ,
                         bool& inexact                      ) GUCEF_MATH_REALTIME_CONTEXT
{
    inexact = false;
    if ( magnitude.IsZero() )
    {
        result = 0;
        return true;
    }

    CORE::CUInt128 q;
    TMathRemainderClass remainderClass = MATHREMAINDER_ZERO;
    if ( shift >= 0 )
    {
        if ( magnitude.BitLength() + (UInt32) shift > 128 )
            return false;
        CORE::CUInt128 numerator = CORE::CUInt128::ShiftLeft( magnitude, (UInt32) shift );
        if ( 1 == denominator )
        {
            q = numerator;
        }
        else
        {
            UInt64 r = 0;
            CORE::CUInt128::DivMod( numerator, denominator, q, r );
            remainderClass = MathClassifyRemainder( r, denominator );
        }
    }
    else
    {
        UInt32 k = (UInt32) ( -( shift + 1 ) ) + 1;
        if ( MathBitLength( denominator ) + k > 127 )
        {
            // the divisor is >= 2^127 > 2 * magnitude: the quotient is 0 and the remainder below half
            q = CORE::CUInt128();
            remainderClass = MATHREMAINDER_BELOW_HALF;
        }
        else
        {
            CORE::CUInt128 divisor = CORE::CUInt128::ShiftLeft( CORE::CUInt128( denominator ), k );
            CORE::CUInt128 r;
            CORE::CUInt128::DivMod( magnitude, divisor, q, r );
            remainderClass = MathClassifyRemainder( r, divisor );
        }
    }

    if ( 0 != q.high )
        return false;
    UInt64 kept = q.low;
    inexact = MATHREMAINDER_ZERO != remainderClass;
    if ( MathRoundingIncrementsMagnitude( mode, negative, 0 != ( kept & 1 ), remainderClass ) )
    {
        ++kept;
        if ( 0 == kept )
            return false;
    }
    result = kept;
    return true;
}

/*-------------------------------------------------------------------------*/

/** MathRoundScaledRatio128 for a 64 bit magnitude */
GUCEF_MATH_FORCE_INLINE bool
MathRoundScaledRatio( bool negative, UInt64 magnitude, Int32 shift, UInt64 denominator, TRoundingMode mode, UInt64& result, bool& inexact ) GUCEF_MATH_REALTIME_CONTEXT
{
    return MathRoundScaledRatio128( negative, CORE::CUInt128( magnitude ), shift, denominator, mode, result, inexact );
}

/*-------------------------------------------------------------------------*/

/**
 *  Rounds an exact value to a signed integer of 'bits' bits (2..64) with 'mode', saturating.
 *  Used for integer kinds (exponentShift 0) and fixed point raw values (exponentShift = fraction bits).
 *  NaN and infinity give 0 with MATHSTATUS_INVALID_OPERAND. Overflow takes precedence over ROUNDING_REQUIRED (as in
 *  CDecimal). ROUNDINGMODE_UNNECESSARY with an inexact result that fits and an invalid mode leave 'out' unchanged.
 */
inline TMathStatus
MathRoundPartsToSigned( const TNumericValueParts& parts, Int32 exponentShift, UInt32 bits, TRoundingMode mode, Int64& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !MathIsValidRoundingMode( mode ) )
        return MATHSTATUS_INVALID_OPERAND;
    if ( parts.isNaN || parts.isInfinite )
    {
        out = 0;
        return MATHSTATUS_INVALID_OPERAND;
    }

    const UInt64 maxPositive = ( (UInt64) 1 << ( bits - 1 ) ) - 1;
    const UInt64 maxNegative = (UInt64) 1 << ( bits - 1 );
    UInt64 magnitude = 0;
    bool inexact = false;
    bool fits = MathRoundScaledRatio( parts.negative, parts.magnitude, parts.exponent2 + exponentShift, parts.denominator, mode, magnitude, inexact );
    if ( !fits || magnitude > ( parts.negative ? maxNegative : maxPositive ) )
    {
        out = parts.negative ? MathApplySign( true, maxNegative ) : (Int64) maxPositive;
        return MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT;
    }
    if ( inexact && CORE::ROUNDINGMODE_UNNECESSARY == mode )
        return MATHSTATUS_ROUNDING_REQUIRED;
    out = MathApplySign( parts.negative, magnitude );
    return inexact ? MATHSTATUS_INEXACT : MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

/** As MathRoundPartsToSigned for an unsigned integer of 'bits' bits (1..64); negative non-zero results saturate to 0 */
inline TMathStatus
MathRoundPartsToUnsigned( const TNumericValueParts& parts, UInt32 bits, TRoundingMode mode, UInt64& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !MathIsValidRoundingMode( mode ) )
        return MATHSTATUS_INVALID_OPERAND;
    if ( parts.isNaN || parts.isInfinite )
    {
        out = 0;
        return MATHSTATUS_INVALID_OPERAND;
    }

    const UInt64 maxValue = 64 == bits ? ~(UInt64) 0 : ( ( (UInt64) 1 << bits ) - 1 );
    UInt64 magnitude = 0;
    bool inexact = false;
    bool fits = MathRoundScaledRatio( parts.negative, parts.magnitude, parts.exponent2, parts.denominator, mode, magnitude, inexact );
    if ( parts.negative && fits && 0 != magnitude )
    {
        out = 0;
        return MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT;
    }
    if ( !fits || magnitude > maxValue )
    {
        out = parts.negative ? 0 : maxValue;
        return MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT;
    }
    if ( inexact && CORE::ROUNDINGMODE_UNNECESSARY == mode )
        return MATHSTATUS_ROUNDING_REQUIRED;
    out = magnitude;
    return inexact ? MATHSTATUS_INEXACT : MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

/**
 *  Rounds an exact value once into an IEEE binary format with 'precision' significand bits and exponent range
 *  [ minExponent, maxExponent ] (of the leading bit of normal numbers). Subnormal results are rounded at their own
 *  binade, so there is no double rounding. The result is returned as a signed integer significand and a power of two
 *  ( value = significand * 2^exponent, |significand| <= 2^precision ), or as an infinity / NaN flag.
 *
 *  Overflow follows IEEE 754 per mode: infinity for the nearest modes (and ROUNDINGMODE_UNNECESSARY) and for the
 *  directed modes that round away from zero, the largest finite value otherwise; all report
 *  MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT.
 */
inline TMathStatus
MathRoundPartsToBinaryFloat( const TNumericValueParts& parts ,
                             UInt32 precision                ,
                             Int32 minExponent               ,
                             Int32 maxExponent               ,
                             TRoundingMode mode              ,
                             UInt64& significand             ,
                             Int32& exponent                 ,
                             bool& isInfinite                ) GUCEF_MATH_REALTIME_CONTEXT
{
    isInfinite = false;
    significand = 0;
    exponent = 0;
    if ( !MathIsValidRoundingMode( mode ) )
        return MATHSTATUS_INVALID_OPERAND;
    if ( parts.isInfinite )
    {
        isInfinite = true;
        return MATHSTATUS_OK;
    }
    if ( 0 == parts.magnitude )
        return MATHSTATUS_OK;

    // obtain an integer q and exponent e with value = ( q + f ) * 2^e, 0 <= f < 1, sticky = ( f != 0 )
    UInt64 q = parts.magnitude;
    Int32 e = parts.exponent2;
    bool sticky = false;
    if ( 1 != parts.denominator )
    {
        Int32 shift = (Int32) ( precision + 3 ) - ( (Int32) MathBitLength( parts.magnitude ) - (Int32) MathBitLength( parts.denominator ) );
        MathRoundScaledRatio( parts.negative, parts.magnitude, shift, parts.denominator, CORE::ROUNDINGMODE_TOWARD_ZERO, q, sticky );
        e -= shift;
    }

    const Int32 b = (Int32) MathBitLength( q );
    const Int32 lead = b - 1 + e;
    bool overflow = lead > maxExponent;
    UInt64 kept = 0;
    Int32 keptExponent = 0;
    bool inexact = false;
    if ( !overflow )
    {
        Int32 bits = (Int32) precision;
        if ( lead < minExponent )
            bits -= ( minExponent - lead );
        Int32 drop = b - bits;
        if ( drop <= 0 )
        {
            // only reachable for dyadic inputs (denominator 1): exact
            kept = q;
            keptExponent = e;
        }
        else
        {
            TMathRemainderClass remainderClass = MathClassifyLowBits( q, (UInt32) drop, sticky );
            kept = drop >= 64 ? 0 : ( q >> drop );
            keptExponent = e + drop;
            inexact = MATHREMAINDER_ZERO != remainderClass;
            if ( MathRoundingIncrementsMagnitude( mode, parts.negative, 0 != ( kept & 1 ), remainderClass ) )
                ++kept;
            if ( 0 != kept && (Int32) MathBitLength( kept ) - 1 + keptExponent > maxExponent )
                overflow = true;
        }
    }

    if ( overflow )
    {
        // as for every other kind, OVERFLOW takes precedence over ROUNDING_REQUIRED: ROUNDINGMODE_UNNECESSARY gives the
        // result of the nearest modes (infinity)
        bool toInfinity = true;
        switch ( mode )
        {
            case CORE::ROUNDINGMODE_TOWARD_ZERO:    toInfinity = false; break;
            case CORE::ROUNDINGMODE_FLOOR:          toInfinity = parts.negative; break;
            case CORE::ROUNDINGMODE_CEILING:        toInfinity = !parts.negative; break;
            default:                                toInfinity = true; break;
        }
        if ( toInfinity )
        {
            isInfinite = true;
        }
        else
        {
            significand = ( (UInt64) 1 << precision ) - 1;
            exponent = maxExponent - (Int32) precision + 1;
        }
        return MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT;
    }

    if ( inexact && CORE::ROUNDINGMODE_UNNECESSARY == mode )
        return MATHSTATUS_ROUNDING_REQUIRED;
    significand = kept;
    exponent = keptExponent;
    return inexact ? MATHSTATUS_INEXACT : MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

/**
 *  Rounds an exact value once into Float64 (precision 53) or Float32 (precision 24).
 *  NaN parts give a quiet NaN. ROUNDINGMODE_UNNECESSARY with an inexact result and an invalid mode leave 'out' unchanged.
 */
template < typename TFloat >
inline TMathStatus
MathRoundPartsToFloat( const TNumericValueParts& parts, TRoundingMode mode, TFloat& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    const bool isDouble = sizeof( TFloat ) == 8;
    if ( parts.isNaN )
    {
        if ( !MathIsValidRoundingMode( mode ) )
            return MATHSTATUS_INVALID_OPERAND;
        out = (TFloat) MathQuietNaN64();
        return MATHSTATUS_OK;
    }
    UInt64 significand = 0;
    Int32 exponent = 0;
    bool isInfinite = false;
    TMathStatus status = MathRoundPartsToBinaryFloat( parts, isDouble ? 53 : 24, isDouble ? -1022 : -126, isDouble ? 1023 : 127,
                                                      mode, significand, exponent, isInfinite );
    if ( 0 != ( status & ( MATHSTATUS_ROUNDING_REQUIRED | MATHSTATUS_INVALID_OPERAND ) ) )
        return status;
    Float64 value = 0.0;
    if ( isInfinite )
    {
        value = MathInfinity64();
    }
    else
    {
        // the significand has at most 53 bits and the result is representable: both steps are exact
        value = ldexp( (Float64) significand, exponent );
    }
    if ( parts.negative )
        value = -value;
    out = (TFloat) value;
    return status;
}

/*-------------------------------------------------------------------------*/

/** Exact parts of a Float64 (the sign of zero is kept): magnitude is the integer significand, NaN and infinity are flagged */
inline TNumericValueParts
MathFloat64ToParts( Float64 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    UInt64 bits = 0;
    memcpy( &bits, &value, sizeof( bits ) );
    TNumericValueParts parts = MathMakeValueParts( false, 0, 0, 1 );
    parts.negative = 0 != ( bits >> 63 );
    UInt32 biased = (UInt32) ( ( bits >> 52 ) & 0x7FF );
    UInt64 fraction = bits & 0x000FFFFFFFFFFFFFULL;
    if ( 0x7FF == biased )
    {
        parts.isNaN = 0 != fraction;
        parts.isInfinite = 0 == fraction;
        return parts;
    }
    if ( 0 == biased )
    {
        parts.magnitude = fraction;
        parts.exponent2 = -1074;
    }
    else
    {
        parts.magnitude = fraction | 0x0010000000000000ULL;
        parts.exponent2 = (Int32) biased - 1075;
    }
    return parts;
}

/*-------------------------------------------------------------------------*/

/** Exact parts of a Float32 (the sign of zero is kept) */
inline TNumericValueParts
MathFloat32ToParts( Float32 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    UInt32 bits = 0;
    memcpy( &bits, &value, sizeof( bits ) );
    TNumericValueParts parts = MathMakeValueParts( false, 0, 0, 1 );
    parts.negative = 0 != ( bits >> 31 );
    UInt32 biased = ( bits >> 23 ) & 0xFF;
    UInt32 fraction = bits & 0x007FFFFF;
    if ( 0xFF == biased )
    {
        parts.isNaN = 0 != fraction;
        parts.isInfinite = 0 == fraction;
        return parts;
    }
    if ( 0 == biased )
    {
        parts.magnitude = fraction;
        parts.exponent2 = -149;
    }
    else
    {
        parts.magnitude = fraction | 0x00800000;
        parts.exponent2 = (Int32) biased - 150;
    }
    return parts;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_NUMERICROUNDINGFUNCTIONS_H ? */
