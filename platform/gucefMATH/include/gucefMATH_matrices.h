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

#ifndef GUCEF_MATH_MATRICES_H
#define GUCEF_MATH_MATRICES_H

/*
 *  The matrix typedef family: T<kind>Matrix2D, T<kind>Matrix3D and T<kind>Matrix4D for the integer, float and fixed
 *  point kinds (for example TFloat32Matrix4D is CTMatrix4D< Float32 >). The fraction kinds belong in
 *  gucefMATH_fractiontypes.h, so that including this header does not pull in gucefCORE_CBigInteger.h.
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

#ifndef GUCEF_MATH_CTMATRIX2D_H
#include "gucefMATH_CTMatrix2D.h"
#define GUCEF_MATH_CTMATRIX2D_H
#endif /* GUCEF_MATH_CTMATRIX2D_H ? */

#ifndef GUCEF_MATH_CTMATRIX3D_H
#include "gucefMATH_CTMatrix3D.h"
#define GUCEF_MATH_CTMATRIX3D_H
#endif /* GUCEF_MATH_CTMATRIX3D_H ? */

#ifndef GUCEF_MATH_CTMATRIX4D_H
#include "gucefMATH_CTMatrix4D.h"
#define GUCEF_MATH_CTMATRIX4D_H
#endif /* GUCEF_MATH_CTMATRIX4D_H ? */

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

typedef CTMatrix2D< Int8 > TInt8Matrix2D;
typedef CTMatrix2D< UInt8 > TUInt8Matrix2D;
typedef CTMatrix2D< Int16 > TInt16Matrix2D;
typedef CTMatrix2D< UInt16 > TUInt16Matrix2D;
typedef CTMatrix2D< Int32 > TInt32Matrix2D;
typedef CTMatrix2D< UInt32 > TUInt32Matrix2D;
typedef CTMatrix2D< Int64 > TInt64Matrix2D;
typedef CTMatrix2D< UInt64 > TUInt64Matrix2D;
typedef CTMatrix2D< Float32 > TFloat32Matrix2D;
typedef CTMatrix2D< Float64 > TFloat64Matrix2D;
typedef CTMatrix2D< TFixed32 > TFixed32Matrix2D;
typedef CTMatrix2D< TFixed64 > TFixed64Matrix2D;
typedef CTMatrix2D< TFixed48_16 > TFixed48_16Matrix2D;

typedef CTMatrix3D< Int8 > TInt8Matrix3D;
typedef CTMatrix3D< UInt8 > TUInt8Matrix3D;
typedef CTMatrix3D< Int16 > TInt16Matrix3D;
typedef CTMatrix3D< UInt16 > TUInt16Matrix3D;
typedef CTMatrix3D< Int32 > TInt32Matrix3D;
typedef CTMatrix3D< UInt32 > TUInt32Matrix3D;
typedef CTMatrix3D< Int64 > TInt64Matrix3D;
typedef CTMatrix3D< UInt64 > TUInt64Matrix3D;
typedef CTMatrix3D< Float32 > TFloat32Matrix3D;
typedef CTMatrix3D< Float64 > TFloat64Matrix3D;
typedef CTMatrix3D< TFixed32 > TFixed32Matrix3D;
typedef CTMatrix3D< TFixed64 > TFixed64Matrix3D;
typedef CTMatrix3D< TFixed48_16 > TFixed48_16Matrix3D;

typedef CTMatrix4D< Int8 > TInt8Matrix4D;
typedef CTMatrix4D< UInt8 > TUInt8Matrix4D;
typedef CTMatrix4D< Int16 > TInt16Matrix4D;
typedef CTMatrix4D< UInt16 > TUInt16Matrix4D;
typedef CTMatrix4D< Int32 > TInt32Matrix4D;
typedef CTMatrix4D< UInt32 > TUInt32Matrix4D;
typedef CTMatrix4D< Int64 > TInt64Matrix4D;
typedef CTMatrix4D< UInt64 > TUInt64Matrix4D;
typedef CTMatrix4D< Float32 > TFloat32Matrix4D;
typedef CTMatrix4D< Float64 > TFloat64Matrix4D;
typedef CTMatrix4D< TFixed32 > TFixed32Matrix4D;
typedef CTMatrix4D< TFixed64 > TFixed64Matrix4D;
typedef CTMatrix4D< TFixed48_16 > TFixed48_16Matrix4D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_MATRICES_H ? */
