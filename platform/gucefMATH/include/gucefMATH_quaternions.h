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

#ifndef GUCEF_MATH_QUATERNIONS_H
#define GUCEF_MATH_QUATERNIONS_H

/*
 *  The rotation typedef family: T<kind>Quaternion, T<kind>EulerAngles and T<kind>AxisAngle for the kinds with a
 *  square root and transcendentals (Float32, Float64 and the fixed point kinds), for example TFloat32Quaternion is
 *  CTQuaternion< Float32 >.
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

#ifndef GUCEF_MATH_CTQUATERNION_H
#include "gucefMATH_CTQuaternion.h"
#define GUCEF_MATH_CTQUATERNION_H
#endif /* GUCEF_MATH_CTQUATERNION_H ? */

#ifndef GUCEF_MATH_CTEULERANGLES_H
#include "gucefMATH_CTEulerAngles.h"
#define GUCEF_MATH_CTEULERANGLES_H
#endif /* GUCEF_MATH_CTEULERANGLES_H ? */

#ifndef GUCEF_MATH_CTAXISANGLE_H
#include "gucefMATH_CTAxisAngle.h"
#define GUCEF_MATH_CTAXISANGLE_H
#endif /* GUCEF_MATH_CTAXISANGLE_H ? */

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

typedef CTQuaternion< Float32 > TFloat32Quaternion;
typedef CTQuaternion< Float64 > TFloat64Quaternion;
typedef CTQuaternion< TFixed32 > TFixed32Quaternion;
typedef CTQuaternion< TFixed64 > TFixed64Quaternion;
typedef CTQuaternion< TFixed48_16 > TFixed48_16Quaternion;

typedef CTEulerAngles< Float32 > TFloat32EulerAngles;
typedef CTEulerAngles< Float64 > TFloat64EulerAngles;
typedef CTEulerAngles< TFixed32 > TFixed32EulerAngles;
typedef CTEulerAngles< TFixed64 > TFixed64EulerAngles;
typedef CTEulerAngles< TFixed48_16 > TFixed48_16EulerAngles;

typedef CTAxisAngle< Float32 > TFloat32AxisAngle;
typedef CTAxisAngle< Float64 > TFloat64AxisAngle;
typedef CTAxisAngle< TFixed32 > TFixed32AxisAngle;
typedef CTAxisAngle< TFixed64 > TFixed64AxisAngle;
typedef CTAxisAngle< TFixed48_16 > TFixed48_16AxisAngle;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_QUATERNIONS_H ? */
