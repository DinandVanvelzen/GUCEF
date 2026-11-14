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
 *  Exported instantiations of the predefined fixed point formats Q16.16, Q32.32 and Q48.16 (section 5.10)
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefMATH_CTFixedPoint.h"

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

#define GUCEF_MATH_FIXEDPOINT_COMMA ,
GUCEF_MATH_INSTANTIATE_TEMPLATE_CLASS( CTFixedPoint< Int32 GUCEF_MATH_FIXEDPOINT_COMMA 16 > )
GUCEF_MATH_INSTANTIATE_TEMPLATE_CLASS( CTFixedPoint< Int64 GUCEF_MATH_FIXEDPOINT_COMMA 32 > )
GUCEF_MATH_INSTANTIATE_TEMPLATE_CLASS( CTFixedPoint< Int64 GUCEF_MATH_FIXEDPOINT_COMMA 16 > )
#undef GUCEF_MATH_FIXEDPOINT_COMMA

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
