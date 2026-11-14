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

#ifndef GUCEF_MATH_MATHSTATUS_H
#define GUCEF_MATH_MATHSTATUS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ROUNDINGMODE_H
#include "gucefCORE_RoundingMode.h"
#define GUCEF_CORE_ROUNDINGMODE_H
#endif /* GUCEF_CORE_ROUNDINGMODE_H ? */

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

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
 *  Outcome of a numeric operation of gucefMATH, as a bit set: the statuses of a sequence of operations can be OR'ed.
 *  The values of the shared meanings equal CORE::EDecimalStatus and CORE::EBigIntegerStatus, so statuses of
 *  conversions that involve CDecimal or CBigInteger are OR'ed in without mapping.
 */
enum EMathStatus
{
    MATHSTATUS_OK                   = 0x000,
    MATHSTATUS_INEXACT              = 0x001,    /**< informational: the result was rounded */
    MATHSTATUS_OVERFLOW             = 0x002,    /**< the result does not fit the target: the output is saturated */
    MATHSTATUS_DIVISION_BY_ZERO     = 0x004,
    MATHSTATUS_INVALID_SCALE        = 0x008,    /**< a decimal scale above CDecimal::MaxScale */
    MATHSTATUS_INVALID_FORMAT       = 0x010,    /**< text does not match the grammar */
    MATHSTATUS_INVALID_OPERAND      = 0x020,    /**< NaN or infinity into a kind without it, a negative square root, an unknown rounding mode */
    MATHSTATUS_ROUNDING_REQUIRED    = 0x040,    /**< ROUNDINGMODE_UNNECESSARY and the result is not exact: the output is unchanged */
    MATHSTATUS_OUT_OF_MEMORY        = 0x080,
    MATHSTATUS_NOT_CONVERGED        = 0x100,    /**< iteration cap reached in an iterative algorithm; the result is the best estimate */
    MATHSTATUS_DEGENERATE           = 0x200     /**< geometric degeneracy: zero area triangle, parallel lines, singular matrix */
};
typedef UInt32 TMathStatus;    /**< bit set of EMathStatus */

/** The rounding modes of gucefCORE are the rounding vocabulary of every numeric kind */
typedef CORE::TRoundingMode TRoundingMode;

/*-------------------------------------------------------------------------*/

/** True when the status holds any bit other than the informational MATHSTATUS_INEXACT */
inline bool
MathStatusIsError( TMathStatus status ) GUCEF_MATH_REALTIME_CONTEXT
{
    return 0 != ( status & ~( (TMathStatus) MATHSTATUS_INEXACT ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  Where the discarded part of a value lies relative to half a unit of the last kept digit.
 *  Every rounding of gucefMATH classifies the discarded part first and then asks MathRoundingIncrementsMagnitude
 *  whether the kept magnitude grows by one unit, so all kinds share one rounding semantics.
 */
enum EMathRemainderClass
{
    MATHREMAINDER_ZERO = 0 ,        /**< nothing non-zero was discarded: the value is exact */
    MATHREMAINDER_BELOW_HALF ,
    MATHREMAINDER_HALF ,            /**< exactly half a unit: a tie */
    MATHREMAINDER_ABOVE_HALF
};
typedef enum EMathRemainderClass TMathRemainderClass;

/*-------------------------------------------------------------------------*/

/** True when 'mode' is one of the defined rounding modes, ROUNDINGMODE_UNNECESSARY included */
inline bool
MathIsValidRoundingMode( TRoundingMode mode ) GUCEF_MATH_REALTIME_CONTEXT
{
    return (UInt32) mode <= (UInt32) CORE::ROUNDINGMODE_UNNECESSARY;
}

/*-------------------------------------------------------------------------*/

/**
 *  Decides whether a magnitude that was truncated grows by one unit.
 *
 *  @param mode the rounding mode, must be valid and not ROUNDINGMODE_UNNECESSARY (that gives false)
 *  @param negative the sign of the value whose magnitude was truncated
 *  @param keptIsOdd whether the truncated magnitude is odd (used for ties to even)
 *  @param remainderClass the class of the discarded part
 */
inline bool
MathRoundingIncrementsMagnitude( TRoundingMode mode, bool negative, bool keptIsOdd, TMathRemainderClass remainderClass ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( MATHREMAINDER_ZERO == remainderClass )
        return false;
    switch ( mode )
    {
        case CORE::ROUNDINGMODE_HALF_EVEN:      return MATHREMAINDER_ABOVE_HALF == remainderClass || ( MATHREMAINDER_HALF == remainderClass && keptIsOdd );
        case CORE::ROUNDINGMODE_HALF_UP:        return MATHREMAINDER_BELOW_HALF != remainderClass;
        case CORE::ROUNDINGMODE_HALF_DOWN:      return MATHREMAINDER_ABOVE_HALF == remainderClass;
        case CORE::ROUNDINGMODE_TOWARD_ZERO:    return false;
        case CORE::ROUNDINGMODE_AWAY_FROM_ZERO: return true;
        case CORE::ROUNDINGMODE_FLOOR:          return negative;
        case CORE::ROUNDINGMODE_CEILING:        return !negative;
        default:                                return false;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_MATHSTATUS_H ? */
