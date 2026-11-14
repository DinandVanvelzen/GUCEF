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
 *  Exported Float32 and Float64 instantiations of the precision-sensitive angle functions (section 5.10)
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefMATH_AngleFunctions.h"

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

GUCEF_MATH_INSTANTIATE_TEMPLATE Float32 WrapPeriodicSigned< Float32 >( const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTRadian< Float32 > WrapAngle< Float32 >( const CTRadian< Float32 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTRadian< Float32 > WrapAnglePositive< Float32 >( const CTRadian< Float32 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTRadian< Float32 > DeltaAngle< Float32 >( const CTRadian< Float32 >&, const CTRadian< Float32 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTRadian< Float32 > LerpAngle< Float32 >( const CTRadian< Float32 >&, const CTRadian< Float32 >&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTRadian< Float32 > MoveTowardsAngle< Float32 >( const CTRadian< Float32 >&, const CTRadian< Float32 >&, const CTRadian< Float32 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTDegree< Float32 > WrapAngle< Float32 >( const CTDegree< Float32 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTDegree< Float32 > WrapAnglePositive< Float32 >( const CTDegree< Float32 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTDegree< Float32 > DeltaAngle< Float32 >( const CTDegree< Float32 >&, const CTDegree< Float32 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTDegree< Float32 > LerpAngle< Float32 >( const CTDegree< Float32 >&, const CTDegree< Float32 >&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTDegree< Float32 > MoveTowardsAngle< Float32 >( const CTDegree< Float32 >&, const CTDegree< Float32 >&, const CTDegree< Float32 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float64 WrapPeriodicSigned< Float64 >( const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTRadian< Float64 > WrapAngle< Float64 >( const CTRadian< Float64 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTRadian< Float64 > WrapAnglePositive< Float64 >( const CTRadian< Float64 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTRadian< Float64 > DeltaAngle< Float64 >( const CTRadian< Float64 >&, const CTRadian< Float64 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTRadian< Float64 > LerpAngle< Float64 >( const CTRadian< Float64 >&, const CTRadian< Float64 >&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTRadian< Float64 > MoveTowardsAngle< Float64 >( const CTRadian< Float64 >&, const CTRadian< Float64 >&, const CTRadian< Float64 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTDegree< Float64 > WrapAngle< Float64 >( const CTDegree< Float64 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTDegree< Float64 > WrapAnglePositive< Float64 >( const CTDegree< Float64 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTDegree< Float64 > DeltaAngle< Float64 >( const CTDegree< Float64 >&, const CTDegree< Float64 >& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTDegree< Float64 > LerpAngle< Float64 >( const CTDegree< Float64 >&, const CTDegree< Float64 >&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTDegree< Float64 > MoveTowardsAngle< Float64 >( const CTDegree< Float64 >&, const CTDegree< Float64 >&, const CTDegree< Float64 >& ) GUCEF_MATH_REALTIME_CONTEXT;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
