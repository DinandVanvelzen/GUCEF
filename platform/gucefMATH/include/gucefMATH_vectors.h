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

#ifndef GUCEF_MATH_VECTORS_H
#define GUCEF_MATH_VECTORS_H

/*
 *  The vector typedef family: T<kind>Vector2D, T<kind>Vector3D and T<kind>Vector4D for the integer, float and fixed
 *  point kinds (for example TFloat64Vector3D is CTVector3D< Float64 >). The fraction kinds are in
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

#ifndef GUCEF_MATH_CTVECTOR2D_H
#include "gucefMATH_CTVector2D.h"
#define GUCEF_MATH_CTVECTOR2D_H
#endif /* GUCEF_MATH_CTVECTOR2D_H ? */

#ifndef GUCEF_MATH_CTVECTOR3D_H
#include "gucefMATH_CTVector3D.h"
#define GUCEF_MATH_CTVECTOR3D_H
#endif /* GUCEF_MATH_CTVECTOR3D_H ? */

#ifndef GUCEF_MATH_CTVECTOR4D_H
#include "gucefMATH_CTVector4D.h"
#define GUCEF_MATH_CTVECTOR4D_H
#endif /* GUCEF_MATH_CTVECTOR4D_H ? */

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

typedef CTVector2D< Int8 > TInt8Vector2D;
typedef CTVector2D< UInt8 > TUInt8Vector2D;
typedef CTVector2D< Int16 > TInt16Vector2D;
typedef CTVector2D< UInt16 > TUInt16Vector2D;
typedef CTVector2D< Int32 > TInt32Vector2D;
typedef CTVector2D< UInt32 > TUInt32Vector2D;
typedef CTVector2D< Int64 > TInt64Vector2D;
typedef CTVector2D< UInt64 > TUInt64Vector2D;
typedef CTVector2D< Float32 > TFloat32Vector2D;
typedef CTVector2D< Float64 > TFloat64Vector2D;
typedef CTVector2D< TFixed32 > TFixed32Vector2D;
typedef CTVector2D< TFixed64 > TFixed64Vector2D;
typedef CTVector2D< TFixed48_16 > TFixed48_16Vector2D;

typedef CTVector3D< Int8 > TInt8Vector3D;
typedef CTVector3D< UInt8 > TUInt8Vector3D;
typedef CTVector3D< Int16 > TInt16Vector3D;
typedef CTVector3D< UInt16 > TUInt16Vector3D;
typedef CTVector3D< Int32 > TInt32Vector3D;
typedef CTVector3D< UInt32 > TUInt32Vector3D;
typedef CTVector3D< Int64 > TInt64Vector3D;
typedef CTVector3D< UInt64 > TUInt64Vector3D;
typedef CTVector3D< Float32 > TFloat32Vector3D;
typedef CTVector3D< Float64 > TFloat64Vector3D;
typedef CTVector3D< TFixed32 > TFixed32Vector3D;
typedef CTVector3D< TFixed64 > TFixed64Vector3D;
typedef CTVector3D< TFixed48_16 > TFixed48_16Vector3D;

typedef CTVector4D< Int8 > TInt8Vector4D;
typedef CTVector4D< UInt8 > TUInt8Vector4D;
typedef CTVector4D< Int16 > TInt16Vector4D;
typedef CTVector4D< UInt16 > TUInt16Vector4D;
typedef CTVector4D< Int32 > TInt32Vector4D;
typedef CTVector4D< UInt32 > TUInt32Vector4D;
typedef CTVector4D< Int64 > TInt64Vector4D;
typedef CTVector4D< UInt64 > TUInt64Vector4D;
typedef CTVector4D< Float32 > TFloat32Vector4D;
typedef CTVector4D< Float64 > TFloat64Vector4D;
typedef CTVector4D< TFixed32 > TFixed32Vector4D;
typedef CTVector4D< TFixed64 > TFixed64Vector4D;
typedef CTVector4D< TFixed48_16 > TFixed48_16Vector4D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_VECTORS_H ? */
