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

#ifndef GUCEF_MATH_POINTS_H
#define GUCEF_MATH_POINTS_H

/*
 *  The point typedef family: T<kind>Point2D and T<kind>Point3D for the integer, float and fixed point kinds (for
 *  example TFloat32Point3D is CTPoint3D< Float32 >). The fraction kinds are in gucefMATH_fractiontypes.h.
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

#ifndef GUCEF_MATH_CTPOINT2D_H
#include "gucefMATH_CTPoint2D.h"
#define GUCEF_MATH_CTPOINT2D_H
#endif /* GUCEF_MATH_CTPOINT2D_H ? */

#ifndef GUCEF_MATH_CTPOINT3D_H
#include "gucefMATH_CTPoint3D.h"
#define GUCEF_MATH_CTPOINT3D_H
#endif /* GUCEF_MATH_CTPOINT3D_H ? */

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

typedef CTPoint2D< Int8 > TInt8Point2D;
typedef CTPoint2D< UInt8 > TUInt8Point2D;
typedef CTPoint2D< Int16 > TInt16Point2D;
typedef CTPoint2D< UInt16 > TUInt16Point2D;
typedef CTPoint2D< Int32 > TInt32Point2D;
typedef CTPoint2D< UInt32 > TUInt32Point2D;
typedef CTPoint2D< Int64 > TInt64Point2D;
typedef CTPoint2D< UInt64 > TUInt64Point2D;
typedef CTPoint2D< Float32 > TFloat32Point2D;
typedef CTPoint2D< Float64 > TFloat64Point2D;
typedef CTPoint2D< TFixed32 > TFixed32Point2D;
typedef CTPoint2D< TFixed64 > TFixed64Point2D;
typedef CTPoint2D< TFixed48_16 > TFixed48_16Point2D;

typedef CTPoint3D< Int8 > TInt8Point3D;
typedef CTPoint3D< UInt8 > TUInt8Point3D;
typedef CTPoint3D< Int16 > TInt16Point3D;
typedef CTPoint3D< UInt16 > TUInt16Point3D;
typedef CTPoint3D< Int32 > TInt32Point3D;
typedef CTPoint3D< UInt32 > TUInt32Point3D;
typedef CTPoint3D< Int64 > TInt64Point3D;
typedef CTPoint3D< UInt64 > TUInt64Point3D;
typedef CTPoint3D< Float32 > TFloat32Point3D;
typedef CTPoint3D< Float64 > TFloat64Point3D;
typedef CTPoint3D< TFixed32 > TFixed32Point3D;
typedef CTPoint3D< TFixed64 > TFixed64Point3D;
typedef CTPoint3D< TFixed48_16 > TFixed48_16Point3D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_POINTS_H ? */
