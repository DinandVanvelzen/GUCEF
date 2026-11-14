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

#ifndef GUCEF_MATH_FRACTIONTYPES_H
#define GUCEF_MATH_FRACTIONTYPES_H

/*
 *  The typedef families of the exact fraction kinds: T<kind>Vector2D/3D/4D and T<kind>Point2D/3D for
 *  Int64Fraction (CORE::CTFraction< Int64, Int64 >, bounded) and BigFraction (CORE::CTFraction< CBigInteger,
 *  CBigInteger >, unbounded, heap beyond 256 bits). Separate from the other family headers because it includes
 *  gucefCORE_CBigInteger.h and the fraction traits. Fractions have no square root: no lengths and no directions.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTNUMERICTRAITSFRACTION_H
#include "gucefMATH_CTNumericTraitsFraction.h"
#define GUCEF_MATH_CTNUMERICTRAITSFRACTION_H
#endif /* GUCEF_MATH_CTNUMERICTRAITSFRACTION_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

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

typedef CTVector2D< CORE::TInt64Fraction > TInt64FractionVector2D;
typedef CTVector2D< TMathBigFraction > TBigFractionVector2D;

typedef CTVector3D< CORE::TInt64Fraction > TInt64FractionVector3D;
typedef CTVector3D< TMathBigFraction > TBigFractionVector3D;

typedef CTVector4D< CORE::TInt64Fraction > TInt64FractionVector4D;
typedef CTVector4D< TMathBigFraction > TBigFractionVector4D;

typedef CTPoint2D< CORE::TInt64Fraction > TInt64FractionPoint2D;
typedef CTPoint2D< TMathBigFraction > TBigFractionPoint2D;

typedef CTPoint3D< CORE::TInt64Fraction > TInt64FractionPoint3D;
typedef CTPoint3D< TMathBigFraction > TBigFractionPoint3D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* The members of the fraction kinds instantiated in the module (see the class headers and section 5.10 of the design) */
#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
GUCEF_MATH_CTVECTOR2D_FRACTION_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
GUCEF_MATH_CTVECTOR3D_FRACTION_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
GUCEF_MATH_CTVECTOR4D_FRACTION_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
GUCEF_MATH_CTPOINT2D_FRACTION_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
GUCEF_MATH_CTPOINT3D_FRACTION_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_FRACTIONTYPES_H ? */
