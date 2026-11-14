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

#ifndef GUCEF_MATH_CTNUMERICCONVERTER_H
#define GUCEF_MATH_CTNUMERICCONVERTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTNUMERICTRAITS_H
#include "gucefMATH_CTNumericTraits.h"
#define GUCEF_MATH_CTNUMERICTRAITS_H
#endif /* GUCEF_MATH_CTNUMERICTRAITS_H ? */

#ifndef GUCEF_MATH_DECIMALCONVERSIONS_H
#include "gucefMATH_DecimalConversions.h"
#define GUCEF_MATH_DECIMALCONVERSIONS_H
#endif /* GUCEF_MATH_DECIMALCONVERSIONS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The single conversion point between numeric kinds; vectors, matrices and shapes convert component-wise through it.
 *
 *  Convert( in, out, mode ) rounds the exact value of 'in' once with 'mode' into TTo:
 *  - OVERFLOW: the output is saturated to the nearest representable bound of TTo (for floats: infinity or the largest
 *    finite value as IEEE 754 specifies per mode);
 *  - INVALID_OPERAND (NaN or infinity into a kind without them): the output is zero;
 *  - ROUNDINGMODE_UNNECESSARY with an inexact result: ROUNDING_REQUIRED and the output is unchanged;
 *  - an invalid mode: INVALID_OPERAND and the output is unchanged.
 *  Fractions that cannot hold a value exactly receive the best rational approximation (INEXACT).
 *  CDecimal on either side: see gucefMATH_DecimalConversions.h (decimal targets keep the CDecimal convention of an
 *  unchanged result on error and use the automatic scale).
 *  The kinds involved need their traits header included (fixed point, fractions).
 */
template < typename TFrom, typename TTo >
class CTNumericConverter
{
    public:

    static GUCEF_MATH_FORCE_INLINE TMathStatus Convert( const TFrom& in, TTo& out, TRoundingMode mode ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTNumericTraits< TTo >::FromParts( CTNumericTraits< TFrom >::ToParts( in ), mode, out );
    }
};

/*-------------------------------------------------------------------------*/

template < typename TFrom >
class CTNumericConverter< TFrom, CORE::CDecimal >
{
    public:

    static TMathStatus Convert( const TFrom& in, CORE::CDecimal& out, TRoundingMode mode )
    {
        return ToDecimalAutoScale( in, mode, out );
    }
};

/*-------------------------------------------------------------------------*/

template < typename TTo >
class CTNumericConverter< CORE::CDecimal, TTo >
{
    public:

    static TMathStatus Convert( const CORE::CDecimal& in, TTo& out, TRoundingMode mode )
    {
        return FromDecimal( in, mode, out );
    }
};

/*-------------------------------------------------------------------------*/

template <>
class CTNumericConverter< CORE::CDecimal, CORE::CDecimal >
{
    public:

    /** Identity; the mode is accepted for the common signature */
    static TMathStatus Convert( const CORE::CDecimal& in, CORE::CDecimal& out, TRoundingMode mode )
    {
        if ( !MathIsValidRoundingMode( mode ) )
            return MATHSTATUS_INVALID_OPERAND;
        out = in;
        return MATHSTATUS_OK;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Nearest Float32 (ties to even), one rounding */
template < typename T >
GUCEF_MATH_FORCE_INLINE Float32
ToFloat32( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float32 result = 0.0f;
    CTNumericConverter< T, Float32 >::Convert( value, result, CORE::ROUNDINGMODE_HALF_EVEN );
    return result;
}

/*-------------------------------------------------------------------------*/

/** Nearest Float64 (ties to even), one rounding */
template < typename T >
GUCEF_MATH_FORCE_INLINE Float64
ToFloat64( const T& value ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 result = 0.0;
    CTNumericConverter< T, Float64 >::Convert( value, result, CORE::ROUNDINGMODE_HALF_EVEN );
    return result;
}

/*-------------------------------------------------------------------------*/

/** value as fixed point, rounded with 'mode', saturating (NaN gives 0) */
template < typename TFixed, typename T >
GUCEF_MATH_FORCE_INLINE TFixed
ToFixed( const T& value, TRoundingMode mode ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< TFixed >::IsFixedPoint, ToFixed_requires_a_fixed_point_target );
    TFixed result;
    CTNumericConverter< T, TFixed >::Convert( value, result, mode );
    return result;
}

/*-------------------------------------------------------------------------*/

/** value as an integer kind, rounded with 'mode', saturating (NaN gives 0) */
template < typename TInt, typename T >
GUCEF_MATH_FORCE_INLINE TInt
ToInteger( const T& value, TRoundingMode mode ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< TInt >::IsInteger, ToInteger_requires_an_integer_target );
    TInt result = TInt( 0 );
    CTNumericConverter< T, TInt >::Convert( value, result, mode );
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

#endif /* GUCEF_MATH_CTNUMERICCONVERTER_H ? */
