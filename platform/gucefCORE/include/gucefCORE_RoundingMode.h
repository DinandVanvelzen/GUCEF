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

#ifndef GUCEF_CORE_ROUNDINGMODE_H
#define GUCEF_CORE_ROUNDINGMODE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  How a value is rounded when digits have to be discarded.
 *  Reference, rounding to scale 0:
 *
 *    input      HALF_EVEN  HALF_UP  HALF_DOWN  TOWARD_ZERO  AWAY_FROM_ZERO  FLOOR  CEILING
 *     2.5           2         3         2           2              3          2       3
 *    -2.5          -2        -3        -2          -2             -3         -3      -2
 *     2.6           3         3         3           2              3          2       3
 *    -0.4           0         0         0           0             -1         -1       0
 */
enum ERoundingMode
{
    ROUNDINGMODE_HALF_EVEN = 0,     /**< ties to the even neighbour (banker's rounding) */
    ROUNDINGMODE_HALF_UP,           /**< ties away from zero: -2.5 -> -3 (java.math.RoundingMode.HALF_UP semantics) */
    ROUNDINGMODE_HALF_DOWN,         /**< ties toward zero */
    ROUNDINGMODE_TOWARD_ZERO,       /**< truncation */
    ROUNDINGMODE_AWAY_FROM_ZERO,    /**< any discarded non-zero digit increments the magnitude */
    ROUNDINGMODE_FLOOR,             /**< toward -infinity */
    ROUNDINGMODE_CEILING,           /**< toward +infinity */
    ROUNDINGMODE_UNNECESSARY,       /**< the result must be exact; rounding gives DECIMALSTATUS_ROUNDING_REQUIRED */

    /**
     *  Not a rounding mode: what a byte that holds no rounding mode reads as. Operations given this mode, or any other
     *  value above ROUNDINGMODE_UNNECESSARY, report DECIMALSTATUS_INVALID_OPERAND. The value makes every byte a value
     *  of this enumeration, so reading one from storage is never undefined
     */
    ROUNDINGMODE_INVALID = 255
};
typedef enum ERoundingMode TRoundingMode;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_ROUNDINGMODE_H ? */
