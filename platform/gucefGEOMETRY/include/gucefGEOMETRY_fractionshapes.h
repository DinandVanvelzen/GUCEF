/*
 *  gucefGEOMETRY: GUCEF module providing geometry: shapes, spaces and spatial queries
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

#ifndef GUCEF_GEOMETRY_FRACTIONSHAPES_H
#define GUCEF_GEOMETRY_FRACTIONSHAPES_H

/*
 *  The shape typedefs of the fraction kinds: T<kind><Shape> for the straight-line shapes (for example
 *  TBigFractionSegment2D is CTSegment2D< TMathBigFraction >, TInt64FractionRectangle is CTRectangle< CORE::TInt64Fraction >).
 *  A separate header, like gucefMATH_fractiontypes.h, so that including gucefGEOMETRY_shapes.h does not pull in
 *  gucefCORE_CBigInteger.h. Members that need a square root (lengths, distances) do not compile for these kinds; the
 *  exact members (containment, intersection, merging, orientation tests) do. The
 *  rectangle and box of the big fraction need a largest value of the kind, so only the 64 bit fraction has them.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_FRACTIONTYPES_H
#include "gucefMATH_fractiontypes.h"
#define GUCEF_MATH_FRACTIONTYPES_H
#endif /* GUCEF_MATH_FRACTIONTYPES_H ? */

#ifndef GUCEF_GEOMETRY_SHAPES_H
#include "gucefGEOMETRY_shapes.h"
#define GUCEF_GEOMETRY_SHAPES_H
#endif /* GUCEF_GEOMETRY_SHAPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTSegment2D< CORE::TInt64Fraction > TInt64FractionSegment2D;
typedef CTSegment3D< CORE::TInt64Fraction > TInt64FractionSegment3D;
typedef CTRectangle< CORE::TInt64Fraction > TInt64FractionRectangle;
typedef CTAxisAlignedBox3D< CORE::TInt64Fraction > TInt64FractionAxisAlignedBox3D;
typedef CTPolygon2D< CORE::TInt64Fraction > TInt64FractionPolygon2D;

typedef CTSegment2D< MATH::TMathBigFraction > TBigFractionSegment2D;
typedef CTSegment3D< MATH::TMathBigFraction > TBigFractionSegment3D;
typedef CTPolygon2D< MATH::TMathBigFraction > TBigFractionPolygon2D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_FRACTIONSHAPES_H ? */
