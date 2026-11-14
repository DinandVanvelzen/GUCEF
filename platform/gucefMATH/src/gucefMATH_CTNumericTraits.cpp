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

/*
 *  The precision-sensitive functions of the Float32 and Float64 numeric traits, compiled in the module (section 5.10)
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefMATH_CTNumericTraits.h"

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

#ifndef GUCEF_MATH_INLINE_ALL

/*
 *  Compiled with floating point contraction disabled (-ffp-contract=off, /fp:precise): the product is rounded before
 *  the addition in every build of the calling code.
 */
Float32
CTNumericTraits< Float32 >::MultiplyAdd( Float32 a, Float32 b, Float32 c ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float32 product = a * b;
    return product + c;
}

/*-------------------------------------------------------------------------*/

Float64
CTNumericTraits< Float64 >::MultiplyAdd( Float64 a, Float64 b, Float64 c ) GUCEF_MATH_REALTIME_CONTEXT
{
    Float64 product = a * b;
    return product + c;
}

#endif /* GUCEF_MATH_INLINE_ALL ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
