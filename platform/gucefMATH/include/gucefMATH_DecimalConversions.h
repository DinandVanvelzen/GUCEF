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

#ifndef GUCEF_MATH_DECIMALCONVERSIONS_H
#define GUCEF_MATH_DECIMALCONVERSIONS_H

/*
 *  Conversions between CORE::CDecimal and the numeric kinds of gucefMATH, each with one rounding.
 *
 *  - ToDecimal( value, scale, mode, result ): the exact value of any kind (float, fixed point, integer, fraction)
 *    rounded once to 'scale' decimal digits. Same semantics as CDecimal::FromFloat64 for floats (the exact binary value
 *    is rounded, not its shortest text). Follows the CDecimal conventions: on any error status (OVERFLOW,
 *    INVALID_SCALE, INVALID_OPERAND for NaN / infinity / an invalid mode, ROUNDING_REQUIRED) the result is unchanged.
 *  - ToDecimalAutoScale( value, mode, result ): the exact value at the smallest scale that holds it, or, when no scale
 *    up to CDecimal::MaxScale does, at the largest scale whose result fits, rounded with 'mode'.
 *  - FromDecimal( decimal, mode, result ): unscaled / 10^scale rounded once into the kind, with the status and
 *    saturation rules of the kind (CTNumericTraits< T >::FromParts). Correctly rounded for every kind, also Float32.
 *  The TMathStatus bits equal the TDecimalStatus bits, so the statuses are interchangeable.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

#ifndef GUCEF_MATH_CTNUMERICTRAITS_H
#include "gucefMATH_CTNumericTraits.h"
#define GUCEF_MATH_CTNUMERICTRAITS_H
#endif /* GUCEF_MATH_CTNUMERICTRAITS_H ? */

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

/** An exact value rounded once to 'scale' decimal digits, see the file comment */
GUCEF_MATH_PUBLIC_CPP TMathStatus
PartsToDecimal( const TNumericValueParts& parts, UInt8 scale, TRoundingMode mode, CORE::CDecimal& result );

/*-------------------------------------------------------------------------*/

/** The exact value of a decimal: |unscaled| / 10^scale */
inline TNumericValueParts
DecimalToParts( const CORE::CDecimal& value )
{
    Int64 unscaled = value.GetUnscaledValue();
    return MathMakeValueParts( unscaled < 0, MathMagnitude( unscaled ), 0, CORE::CUInt128::Pow10( value.GetScale() ) );
}

/*-------------------------------------------------------------------------*/

/** value of any kind rounded once to 'scale' decimal digits */
template < typename T >
inline TMathStatus
ToDecimal( const T& value, UInt8 scale, TRoundingMode mode, CORE::CDecimal& result )
{
    return PartsToDecimal( CTNumericTraits< T >::ToParts( value ), scale, mode, result );
}

/*-------------------------------------------------------------------------*/

/** The exact value at the smallest scale that holds it, else rounded at the largest scale that fits */
template < typename T >
inline TMathStatus
ToDecimalAutoScale( const T& value, TRoundingMode mode, CORE::CDecimal& result )
{
    TNumericValueParts parts = CTNumericTraits< T >::ToParts( value );
    CORE::CDecimal candidate;
    for ( UInt8 scale=0; scale<=CORE::CDecimal::MaxScale; ++scale )
    {
        TMathStatus status = PartsToDecimal( parts, scale, CORE::ROUNDINGMODE_UNNECESSARY, candidate );
        if ( MATHSTATUS_OK == status )
        {
            result = candidate;
            return status;
        }
        if ( 0 != ( status & ( MATHSTATUS_OVERFLOW | MATHSTATUS_INVALID_OPERAND ) ) )
            break;
    }
    TMathStatus status = MATHSTATUS_OVERFLOW;
    for ( Int32 scale=CORE::CDecimal::MaxScale; scale>=0; --scale )
    {
        status = PartsToDecimal( parts, (UInt8) scale, mode, candidate );
        if ( !MathStatusIsError( status ) )
        {
            result = candidate;
            return status;
        }
        if ( 0 == ( status & MATHSTATUS_OVERFLOW ) )
            return status;
    }
    return status;
}

/*-------------------------------------------------------------------------*/

/** unscaled / 10^scale rounded once into the kind T */
template < typename T >
inline TMathStatus
FromDecimal( const CORE::CDecimal& value, TRoundingMode mode, T& result )
{
    return CTNumericTraits< T >::FromParts( DecimalToParts( value ), mode, result );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_DECIMALCONVERSIONS_H ? */
