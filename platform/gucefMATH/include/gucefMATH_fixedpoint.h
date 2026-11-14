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

#ifndef GUCEF_MATH_FIXEDPOINT_H
#define GUCEF_MATH_FIXEDPOINT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTNUMERICTRAITSFIXEDPOINT_H
#include "gucefMATH_CTNumericTraitsFixedPoint.h"
#define GUCEF_MATH_CTNUMERICTRAITSFIXEDPOINT_H
#endif /* GUCEF_MATH_CTNUMERICTRAITSFIXEDPOINT_H ? */

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

typedef CTFixedPoint< Int32, 16 > TFixed16_16;     /**< Q16.16: range -32768 .. 32767.99998, resolution 2^-16 */
typedef CTFixedPoint< Int64, 32 > TFixed32_32;     /**< Q32.32: range +-2^31, resolution 2^-32 */
typedef CTFixedPoint< Int64, 16 > TFixed48_16;     /**< Q48.16: range +-1.4 * 10^14, resolution 2^-16 */

typedef TFixed16_16 TFixed32;
typedef TFixed32_32 TFixed64;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_FIXEDPOINT_H ? */
