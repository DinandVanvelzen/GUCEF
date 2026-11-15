/*
 *  computebridgeMATH: GUCEF compute bridge plugin of gucefMATH: multi-core and accelerator providers for the batch kernels
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

#ifndef GUCEF_COMPUTEBRIDGEMATH_ETYPES_H
#define GUCEF_COMPUTEBRIDGEMATH_ETYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef GUCEF_COMPUTE_ETYPES_H
#include "gucefCOMPUTE_ETypes.h"
#define GUCEF_COMPUTE_ETYPES_H
#endif /* GUCEF_COMPUTE_ETYPES_H ? */

#ifndef GUCEF_MATH_MATHBATCHTYPES_H
#include "gucefMATH_MathBatchTypes.h"
#define GUCEF_MATH_MATHBATCHTYPES_H
#endif /* GUCEF_MATH_MATHBATCHTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTEBRIDGEMATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::UInt8 UInt8;
typedef CORE::Int8 Int8;
typedef CORE::UInt16 UInt16;
typedef CORE::Int16 Int16;
typedef CORE::UInt32 UInt32;
typedef CORE::Int32 Int32;
typedef CORE::UInt64 UInt64;
typedef CORE::Int64 Int64;
typedef CORE::Float32 Float32;
typedef CORE::Float64 Float64;
typedef CORE::CString CString;

/** The number of MATH batch kernels: the size of the per kernel tables of the bridge (kernels are numbered from 0) */
enum { MATHBATCHKERNEL_SLOTS = MATH::MATHBATCHKERNEL_MATH_COUNT };

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTEBRIDGEMATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTEBRIDGEMATH_ETYPES_H ? */
