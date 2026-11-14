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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CBIGINTEGER_H
#include "gucefCORE_CBigInteger.h"
#define GUCEF_CORE_CBIGINTEGER_H
#endif /* GUCEF_CORE_CBIGINTEGER_H ? */

#include "gucefMATH_DecimalConversions.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

TMathStatus
PartsToDecimal( const TNumericValueParts& parts, UInt8 scale, TRoundingMode mode, CORE::CDecimal& result )
{
    if ( !MathIsValidRoundingMode( mode ) || parts.isNaN || parts.isInfinite )
        return MATHSTATUS_INVALID_OPERAND;
    if ( scale > CORE::CDecimal::MaxScale )
        return MATHSTATUS_INVALID_SCALE;

    // value * 10^scale = magnitude * 10^scale * 2^exponent2 / denominator; magnitude * 10^18 < 2^124
    CORE::CUInt128 scaled = CORE::CUInt128::Multiply( parts.magnitude, CORE::CUInt128::Pow10( scale ) );
    UInt64 unscaledMagnitude = 0;
    bool inexact = false;
    bool fits = MathRoundScaledRatio128( parts.negative, scaled, parts.exponent2, parts.denominator, mode, unscaledMagnitude, inexact );
    if ( !fits || unscaledMagnitude > 0x7FFFFFFFFFFFFFFFULL )
        return MATHSTATUS_OVERFLOW;
    if ( inexact && CORE::ROUNDINGMODE_UNNECESSARY == mode )
        return MATHSTATUS_ROUNDING_REQUIRED;

    Int64 unscaled = parts.negative ? -(Int64) unscaledMagnitude : (Int64) unscaledMagnitude;
    TMathStatus status = (TMathStatus) CORE::CDecimal::FromUnscaled( unscaled, scale, result );
    if ( MATHSTATUS_OK != status )
        return status;
    return inexact ? MATHSTATUS_INEXACT : MATHSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

bool
MathParseDecimalFloat32( const CORE::CString& text, Float32& value )
{GUCEF_TRACE;

    const char* data = text.C_String();
    const UInt32 length = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    if ( GUCEF_NULL == data || 0 == length )
        return false;

    UInt32 i = 0;
    bool negative = false;
    if ( '+' == data[ 0 ] || '-' == data[ 0 ] )
    {
        negative = '-' == data[ 0 ];
        ++i;
    }

    // significant digits without the point and without leading zeros; the decimal exponent of the last digit
    CORE::CString digits;
    Int64 decimalExponent = 0;
    UInt32 digitCount = 0;
    bool seenPoint = false;
    for ( ; i<length; ++i )
    {
        char c = data[ i ];
        if ( c >= '0' && c <= '9' )
        {
            ++digitCount;
            if ( seenPoint )
                --decimalExponent;
            if ( '0' != c || !digits.IsNULLOrEmpty() )
                digits += c;
        }
        else if ( '.' == c && !seenPoint )
        {
            seenPoint = true;
        }
        else
        {
            break;
        }
    }
    if ( 0 == digitCount )
        return false;
    if ( i < length )
    {
        if ( 'e' != data[ i ] && 'E' != data[ i ] )
            return false;
        ++i;
        bool exponentNegative = false;
        if ( i < length && ( '+' == data[ i ] || '-' == data[ i ] ) )
        {
            exponentNegative = '-' == data[ i ];
            ++i;
        }
        if ( i >= length )
            return false;
        Int64 exponent = 0;
        for ( ; i<length; ++i )
        {
            char c = data[ i ];
            if ( c < '0' || c > '9' )
                return false;
            if ( exponent < 1000000 )
                exponent = exponent * 10 + ( c - '0' );
        }
        decimalExponent += exponentNegative ? -exponent : exponent;
    }

    if ( digits.IsNULLOrEmpty() )
    {
        value = negative ? -0.0f : 0.0f;
        return true;
    }

    // value < 10^lead and >= 10^( lead - 1 ): beyond 10^40 it overflows, below 10^-50 it rounds to zero
    const Int64 lead = decimalExponent + (Int64) digits.Length();
    if ( lead > 40 )
        return false;
    if ( lead < -50 )
    {
        value = negative ? -0.0f : 0.0f;
        return true;
    }

    CORE::CBigInteger numerator;
    if ( CORE::BIGINTEGERSTATUS_OK != CORE::CBigInteger::FromString( digits, 10, numerator ) )
        return false;
    CORE::CBigInteger power;
    CORE::CBigInteger( 10 ).Pow( (UInt32) ( decimalExponent < 0 ? -decimalExponent : decimalExponent ), power );
    CORE::CBigInteger denominator( 1 );
    if ( decimalExponent >= 0 )
        numerator.Multiply( power, numerator );
    else
        denominator = power;

    Float32 result = 0.0f;
    CORE::TBigIntegerStatus status = CORE::CBigInteger::RatioToFloat32( numerator, denominator, CORE::ROUNDINGMODE_HALF_EVEN, result );
    if ( 0 != ( status & ~( (CORE::TBigIntegerStatus) CORE::BIGINTEGERSTATUS_INEXACT ) ) )
        return false;
    value = negative ? -result : result;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
