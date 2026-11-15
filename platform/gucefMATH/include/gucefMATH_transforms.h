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

#ifndef GUCEF_MATH_TRANSFORMS_H
#define GUCEF_MATH_TRANSFORMS_H

/*
 *  The transform typedef family: T<kind>Transform2D and T<kind>Transform3D for the kinds with a square root and
 *  transcendentals, and the mixed-precision TFloat32Float64Transform3D (Float32 rotation and scale, Float64 position).
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_FIXEDPOINT_H
#include "gucefMATH_fixedpoint.h"
#define GUCEF_MATH_FIXEDPOINT_H
#endif /* GUCEF_MATH_FIXEDPOINT_H ? */

#ifndef GUCEF_MATH_CTTRANSFORM2D_H
#include "gucefMATH_CTTransform2D.h"
#define GUCEF_MATH_CTTRANSFORM2D_H
#endif /* GUCEF_MATH_CTTRANSFORM2D_H ? */

#ifndef GUCEF_MATH_CTTRANSFORM3D_H
#include "gucefMATH_CTTransform3D.h"
#define GUCEF_MATH_CTTRANSFORM3D_H
#endif /* GUCEF_MATH_CTTRANSFORM3D_H ? */

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

typedef CTTransform2D< Float32 > TFloat32Transform2D;
typedef CTTransform2D< Float64 > TFloat64Transform2D;
typedef CTTransform2D< TFixed32 > TFixed32Transform2D;
typedef CTTransform2D< TFixed64 > TFixed64Transform2D;
typedef CTTransform2D< TFixed48_16 > TFixed48_16Transform2D;

typedef CTTransform3D< Float32 > TFloat32Transform3D;
typedef CTTransform3D< Float64 > TFloat64Transform3D;
typedef CTTransform3D< TFixed32 > TFixed32Transform3D;
typedef CTTransform3D< TFixed64 > TFixed64Transform3D;
typedef CTTransform3D< TFixed48_16 > TFixed48_16Transform3D;

/** Rotation and scale in Float32, positions in Float64: large worlds (section 4.5.5) */
typedef CTTransform3D< Float32, Float64 > TFloat32Float64Transform3D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_TRANSFORMS_H ? */
