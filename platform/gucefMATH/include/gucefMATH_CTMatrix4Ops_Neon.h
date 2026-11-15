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

#ifndef GUCEF_MATH_CTMATRIX4OPS_NEON_H
#define GUCEF_MATH_CTMATRIX4OPS_NEON_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTMATRIX4OPS_H
#include "gucefMATH_CTMatrix4Ops.h"
#define GUCEF_MATH_CTMATRIX4OPS_H
#endif /* GUCEF_MATH_CTMATRIX4OPS_H ? */

#ifndef GUCEF_MATH_CTMATRIX4OPS_LANES_H
#include "gucefMATH_CTMatrix4Ops_Lanes.h"
#define GUCEF_MATH_CTMATRIX4OPS_LANES_H
#endif /* GUCEF_MATH_CTMATRIX4OPS_LANES_H ? */

#ifndef GUCEF_MATH_CTVECTOR4OPS_NEON_H
#include "gucefMATH_CTVector4Ops_Neon.h"
#define GUCEF_MATH_CTVECTOR4OPS_NEON_H
#endif /* GUCEF_MATH_CTVECTOR4OPS_NEON_H ? */

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

/** NEON (AArch64) implementation of CTMatrix4Ops for Float32: the same results bit for bit as the primary template */
template <>
class GUCEF_MATH_TEMPLATE_PUBLIC CTMatrix4Ops< Float32, SMathSimdNeon > : public CTMatrix4OpsLanes< CMathNeon >
{
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTMATRIX4OPS_NEON_H ? */
