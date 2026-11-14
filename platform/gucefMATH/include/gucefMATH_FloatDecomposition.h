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

#ifndef GUCEF_MATH_FLOATDECOMPOSITION_H
#define GUCEF_MATH_FLOATDECOMPOSITION_H

/*
 *  Exact decomposition of binary floating point values into an integer mantissa and a power of two, read from the
 *  IEEE 754 bit pattern (no frexp): value == mantissa * 2^exponent exactly. Used by the conversions to exact kinds
 *  (fractions, big integers) and by exact geometric predicates. Realtime-safe, allocation free.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_NUMERICROUNDINGFUNCTIONS_H
#include "gucefMATH_NumericRoundingFunctions.h"
#define GUCEF_MATH_NUMERICROUNDINGFUNCTIONS_H
#endif /* GUCEF_MATH_NUMERICROUNDINGFUNCTIONS_H ? */

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

/** Removes the trailing zero bits of a non-zero mantissa into the exponent, so the decomposition is unique */
inline void
MathCanonicalizeDecomposition( UInt64& magnitude, Int32& exponent ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == magnitude )
    {
        exponent = 0;
        return;
    }
    UInt32 zeros = MathCountTrailingZeros( magnitude );
    magnitude >>= zeros;
    exponent += (Int32) zeros;
}

/*-------------------------------------------------------------------------*/

/**
 *  value == mantissa * 2^exponent exactly, in canonical form: the mantissa is odd (|mantissa| < 2^53), or 0 with
 *  exponent 0 for both zeros (the sign of zero is not represented). Returns false for NaN and infinity, which leave
 *  the outputs unchanged.
 */
inline bool
DecomposeFloat64( Float64 value, Int64& mantissa, Int32& exponent ) GUCEF_MATH_REALTIME_CONTEXT
{
    TNumericValueParts parts = MathFloat64ToParts( value );
    if ( parts.isNaN || parts.isInfinite )
        return false;
    UInt64 magnitude = parts.magnitude;
    Int32 e = parts.exponent2;
    MathCanonicalizeDecomposition( magnitude, e );
    mantissa = parts.negative ? -(Int64) magnitude : (Int64) magnitude;
    exponent = e;
    return true;
}

/*-------------------------------------------------------------------------*/

/** As DecomposeFloat64 for Float32: the mantissa is odd and |mantissa| < 2^24 */
inline bool
DecomposeFloat32( Float32 value, Int32& mantissa, Int32& exponent ) GUCEF_MATH_REALTIME_CONTEXT
{
    TNumericValueParts parts = MathFloat32ToParts( value );
    if ( parts.isNaN || parts.isInfinite )
        return false;
    UInt64 magnitude = parts.magnitude;
    Int32 e = parts.exponent2;
    MathCanonicalizeDecomposition( magnitude, e );
    mantissa = parts.negative ? -(Int32) magnitude : (Int32) magnitude;
    exponent = e;
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

#endif /* GUCEF_MATH_FLOATDECOMPOSITION_H ? */
