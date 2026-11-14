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

#include <string.h>

#ifndef GUCEF_MATH_NUMERICROUNDINGFUNCTIONS_H
#include "gucefMATH_NumericRoundingFunctions.h"
#define GUCEF_MATH_NUMERICROUNDINGFUNCTIONS_H
#endif /* GUCEF_MATH_NUMERICROUNDINGFUNCTIONS_H ? */

#include "gucefMATH_CFloat16.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFloat16
CFloat16::FromFloat32( Float32 value, TRoundingMode mode ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !MathIsValidRoundingMode( mode ) || CORE::ROUNDINGMODE_UNNECESSARY == mode )
        mode = CORE::ROUNDINGMODE_HALF_EVEN;
    CFloat16 result;
    FromFloat32( value, mode, result );
    return result;
}

/*-------------------------------------------------------------------------*/

TMathStatus
CFloat16::FromFloat32( Float32 value, TRoundingMode mode, CFloat16& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !MathIsValidRoundingMode( mode ) )
        return MATHSTATUS_INVALID_OPERAND;

    UInt32 bits32 = 0;
    memcpy( &bits32, &value, sizeof( bits32 ) );
    const UInt16 sign = (UInt16) ( ( bits32 >> 16 ) & 0x8000 );

    TNumericValueParts parts = MathFloat32ToParts( value );
    if ( parts.isNaN )
    {
        // quiet NaN keeping the top 9 payload bits
        out.m_bits = (UInt16) ( sign | 0x7C00 | 0x0200 | ( ( bits32 >> 13 ) & 0x03FF ) );
        return MATHSTATUS_OK;
    }

    UInt64 significand = 0;
    Int32 exponent = 0;
    bool isInfinite = false;
    TMathStatus status = MathRoundPartsToBinaryFloat( parts, 11, -14, 15, mode, significand, exponent, isInfinite );
    if ( 0 != ( status & MATHSTATUS_ROUNDING_REQUIRED ) )
        return status;
    if ( isInfinite )
    {
        out.m_bits = (UInt16) ( sign | 0x7C00 );
        return status;
    }
    if ( 0 == significand )
    {
        out.m_bits = sign;
        return status;
    }

    // value = significand * 2^exponent with at most 11 significant bits
    const Int32 bitLength = (Int32) MathBitLength( significand );
    const Int32 lead = bitLength - 1 + exponent;
    UInt16 encoded = 0;
    if ( lead < -14 )
    {
        // subnormal: the unit is 2^-24 and the rounding guarantees exponent >= -24
        encoded = (UInt16) ( significand << ( exponent + 24 ) );
    }
    else
    {
        // the rounding may have carried into a twelfth bit: the low bit is then zero and the shift is exact
        UInt64 normalized = bitLength <= 11 ? significand << ( 11 - bitLength ) : significand >> ( bitLength - 11 );
        encoded = (UInt16) ( ( (UInt32) ( lead + 15 ) << 10 ) | (UInt32) ( normalized & 0x03FF ) );
    }
    out.m_bits = (UInt16) ( sign | encoded );
    return status;
}

/*-------------------------------------------------------------------------*/

Float32
CFloat16::ToFloat32( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt32 sign = (UInt32) ( m_bits & 0x8000 ) << 16;
    const UInt32 biased = ( m_bits >> 10 ) & 0x1F;
    const UInt32 fraction = m_bits & 0x03FF;
    UInt32 bits32 = 0;
    if ( 0x1F == biased )
    {
        bits32 = sign | 0x7F800000 | ( fraction << 13 );
    }
    else if ( 0 == biased )
    {
        if ( 0 == fraction )
        {
            bits32 = sign;
        }
        else
        {
            // subnormal half: normal in Float32. fraction * 2^-24 with the leading bit moved to the hidden position
            UInt32 shift = 11 - MathBitLength( fraction );            // 1 .. 10
            UInt32 exponent32 = (UInt32) ( 127 - 14 - shift );
            bits32 = sign | ( exponent32 << 23 ) | ( ( ( fraction << shift ) & 0x03FF ) << 13 );
        }
    }
    else
    {
        bits32 = sign | ( ( biased - 15 + 127 ) << 23 ) | ( fraction << 13 );
    }
    Float32 result;
    memcpy( &result, &bits32, sizeof( result ) );
    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
