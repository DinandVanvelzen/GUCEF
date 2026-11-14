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

#ifndef GUCEF_MATH_DIRECTIONS_H
#define GUCEF_MATH_DIRECTIONS_H

/*
 *  The direction typedef family: T<kind>DirectionVector2D and T<kind>DirectionVector3D for the kinds with a square
 *  root, floats and fixed point (for example TFloat32DirectionVector3D is CTDirectionVector3D< Float32 >).
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

#ifndef GUCEF_MATH_CTDIRECTIONVECTOR2D_H
#include "gucefMATH_CTDirectionVector2D.h"
#define GUCEF_MATH_CTDIRECTIONVECTOR2D_H
#endif /* GUCEF_MATH_CTDIRECTIONVECTOR2D_H ? */

#ifndef GUCEF_MATH_CTDIRECTIONVECTOR3D_H
#include "gucefMATH_CTDirectionVector3D.h"
#define GUCEF_MATH_CTDIRECTIONVECTOR3D_H
#endif /* GUCEF_MATH_CTDIRECTIONVECTOR3D_H ? */

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

typedef CTDirectionVector2D< Float32 > TFloat32DirectionVector2D;
typedef CTDirectionVector2D< Float64 > TFloat64DirectionVector2D;
typedef CTDirectionVector2D< TFixed32 > TFixed32DirectionVector2D;
typedef CTDirectionVector2D< TFixed64 > TFixed64DirectionVector2D;
typedef CTDirectionVector2D< TFixed48_16 > TFixed48_16DirectionVector2D;

typedef CTDirectionVector3D< Float32 > TFloat32DirectionVector3D;
typedef CTDirectionVector3D< Float64 > TFloat64DirectionVector3D;
typedef CTDirectionVector3D< TFixed32 > TFixed32DirectionVector3D;
typedef CTDirectionVector3D< TFixed64 > TFixed64DirectionVector3D;
typedef CTDirectionVector3D< TFixed48_16 > TFixed48_16DirectionVector3D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_DIRECTIONS_H ? */
