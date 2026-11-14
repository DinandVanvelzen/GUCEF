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
 *  Exported Float32 and Float64 instantiations of the precision-sensitive scalar functions (section 5.10)
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefMATH_ScalarFunctions.h"

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

GUCEF_MATH_INSTANTIATE_TEMPLATE Float32 Lerp< Float32 >( const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float32 LerpClamped< Float32 >( const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float32 InverseLerp< Float32 >( const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float32 Remap< Float32 >( const Float32&, const Float32&, const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float32 SmoothStep< Float32 >( const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float32 SmootherStep< Float32 >( const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float32 Wrap< Float32 >( const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float32 PingPong< Float32 >( const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float32 MoveTowards< Float32 >( const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float32 LogLerp< Float32 >( const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float64 Lerp< Float64 >( const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float64 LerpClamped< Float64 >( const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float64 InverseLerp< Float64 >( const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float64 Remap< Float64 >( const Float64&, const Float64&, const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float64 SmoothStep< Float64 >( const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float64 SmootherStep< Float64 >( const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float64 Wrap< Float64 >( const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float64 PingPong< Float64 >( const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float64 MoveTowards< Float64 >( const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE Float64 LogLerp< Float64 >( const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
