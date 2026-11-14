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

#ifndef GUCEF_MATH_CFLOAT16_H
#define GUCEF_MATH_CFLOAT16_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  IEEE 754 binary16 half precision value, for storage only: there are no arithmetic operators (compute in Float32).
 *  The conversions are a software implementation in integer arithmetic, bit-identical on every platform:
 *  - FromFloat32 rounds once with the given mode. Overflow gives infinity for the nearest modes and for the directed
 *    modes that round away from zero, the largest finite value 65504 otherwise (IEEE per mode). Subnormal halves are
 *    produced and consumed. NaN stays NaN with the quiet bit set and the top payload bits kept. The sign of zero is kept.
 *  - ToFloat32 is exact.
 *  Realtime-safe, no allocation.
 */
class GUCEF_MATH_PUBLIC_CPP CFloat16
{
    public:

    /** +0 */
    CFloat16( void )
        : m_bits( 0 )
    {
    }

    /** The value rounded once with 'mode'; ROUNDINGMODE_UNNECESSARY and invalid modes round to nearest, ties to even */
    static CFloat16 FromFloat32( Float32 value, TRoundingMode mode = CORE::ROUNDINGMODE_HALF_EVEN ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The value rounded once with 'mode', with the status: INEXACT, OVERFLOW | INEXACT; ROUNDING_REQUIRED for
     *  ROUNDINGMODE_UNNECESSARY with an inexact result and INVALID_OPERAND for an invalid mode leave 'out' unchanged.
     */
    static TMathStatus FromFloat32( Float32 value, TRoundingMode mode, CFloat16& out ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Exact */
    Float32 ToFloat32( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    static CFloat16 FromBits( UInt16 bits )
    {
        CFloat16 result;
        result.m_bits = bits;
        return result;
    }

    UInt16 GetBits( void ) const { return m_bits; }

    bool IsNaN( void ) const { return 0x7C00 == ( m_bits & 0x7C00 ) && 0 != ( m_bits & 0x03FF ); }
    bool IsInfinite( void ) const { return 0x7C00 == ( m_bits & 0x7FFF ); }

    private:

    UInt16 m_bits;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CFLOAT16_H ? */
