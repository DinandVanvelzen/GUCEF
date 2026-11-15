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
 *  The exported instantiations of the smoothing functions and the integrator functions for the predefined value types
 *  (section 5.10 of the design), compiled here without floating-point contraction.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_SMOOTHINGFUNCTIONS_H
#include "gucefMATH_SmoothingFunctions.h"
#define GUCEF_MATH_SMOOTHINGFUNCTIONS_H
#endif /* GUCEF_MATH_SMOOTHINGFUNCTIONS_H ? */

#ifndef GUCEF_MATH_INTEGRATORFUNCTIONS_H
#include "gucefMATH_IntegratorFunctions.h"
#define GUCEF_MATH_INTEGRATORFUNCTIONS_H
#endif /* GUCEF_MATH_INTEGRATORFUNCTIONS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      INSTANTIATIONS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_MATH_INSTANTIATE_SMOOTHING( kind, scalar ) GUCEF_MATH_SMOOTHING_FUNCTIONS( GUCEF_MATH_INSTANTIATE_TEMPLATE, kind, scalar )
#define GUCEF_MATH_INSTANTIATE_INTEGRATOR( kind, scalar ) GUCEF_MATH_INTEGRATOR_FUNCTIONS( GUCEF_MATH_INSTANTIATE_TEMPLATE, kind, scalar )

GUCEF_MATH_SMOOTHING_PREDEFINED( GUCEF_MATH_INSTANTIATE_SMOOTHING )
GUCEF_MATH_INTEGRATOR_PREDEFINED( GUCEF_MATH_INSTANTIATE_INTEGRATOR )

GUCEF_MATH_INSTANTIATE_TEMPLATE CTRadian< Float32 > SmoothDamp< Float32 >( const CTRadian< Float32 >&, const CTRadian< Float32 >&, SSmoothDampState< Float32 >&, const Float32&, const Float32&, const Float32& ) GUCEF_MATH_REALTIME_CONTEXT;
GUCEF_MATH_INSTANTIATE_TEMPLATE CTRadian< Float64 > SmoothDamp< Float64 >( const CTRadian< Float64 >&, const CTRadian< Float64 >&, SSmoothDampState< Float64 >&, const Float64&, const Float64&, const Float64& ) GUCEF_MATH_REALTIME_CONTEXT;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
