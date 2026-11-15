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
 *  The exported instantiations of the curve classes (Bezier, Hermite, Catmull-Rom, B-spline, keyframed curve, path,
 *  arc length table) for the predefined point types (section 5.10 of the design), compiled here without
 *  floating-point contraction.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTBEZIERCURVE2_H
#include "gucefMATH_CTBezierCurve2.h"
#define GUCEF_MATH_CTBEZIERCURVE2_H
#endif /* GUCEF_MATH_CTBEZIERCURVE2_H ? */

#ifndef GUCEF_MATH_CTBEZIERCURVE3_H
#include "gucefMATH_CTBezierCurve3.h"
#define GUCEF_MATH_CTBEZIERCURVE3_H
#endif /* GUCEF_MATH_CTBEZIERCURVE3_H ? */

#ifndef GUCEF_MATH_CTHERMITESPLINE_H
#include "gucefMATH_CTHermiteSpline.h"
#define GUCEF_MATH_CTHERMITESPLINE_H
#endif /* GUCEF_MATH_CTHERMITESPLINE_H ? */

#ifndef GUCEF_MATH_CTCATMULLROMSPLINE_H
#include "gucefMATH_CTCatmullRomSpline.h"
#define GUCEF_MATH_CTCATMULLROMSPLINE_H
#endif /* GUCEF_MATH_CTCATMULLROMSPLINE_H ? */

#ifndef GUCEF_MATH_CTBSPLINE_H
#include "gucefMATH_CTBSpline.h"
#define GUCEF_MATH_CTBSPLINE_H
#endif /* GUCEF_MATH_CTBSPLINE_H ? */

#ifndef GUCEF_MATH_CTARCLENGTHTABLE_H
#include "gucefMATH_CTArcLengthTable.h"
#define GUCEF_MATH_CTARCLENGTHTABLE_H
#endif /* GUCEF_MATH_CTARCLENGTHTABLE_H ? */

#ifndef GUCEF_MATH_CTCURVE_H
#include "gucefMATH_CTCurve.h"
#define GUCEF_MATH_CTCURVE_H
#endif /* GUCEF_MATH_CTCURVE_H ? */

#ifndef GUCEF_MATH_CTPATH_H
#include "gucefMATH_CTPath.h"
#define GUCEF_MATH_CTPATH_H
#endif /* GUCEF_MATH_CTPATH_H ? */

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

#define GUCEF_MATH_INSTANTIATE_BEZIER2( kind ) GUCEF_MATH_INSTANTIATE_TEMPLATE_CLASS( CTBezierCurve2< kind > )
#define GUCEF_MATH_INSTANTIATE_BEZIER3( kind ) GUCEF_MATH_INSTANTIATE_TEMPLATE_CLASS( CTBezierCurve3< kind > )
#define GUCEF_MATH_INSTANTIATE_HERMITE( kind ) GUCEF_MATH_INSTANTIATE_TEMPLATE_CLASS( CTHermiteSpline< kind > )
#define GUCEF_MATH_INSTANTIATE_CATMULLROM( kind ) GUCEF_MATH_INSTANTIATE_TEMPLATE_CLASS( CTCatmullRomSpline< kind > )
#define GUCEF_MATH_INSTANTIATE_BSPLINE( kind ) GUCEF_MATH_INSTANTIATE_TEMPLATE_CLASS( CTBSpline< kind > )
#define GUCEF_MATH_INSTANTIATE_PATH( kind ) GUCEF_MATH_INSTANTIATE_TEMPLATE_CLASS( CTPath< kind > )

GUCEF_MATH_CURVE_PREDEFINED_POINTS( GUCEF_MATH_INSTANTIATE_BEZIER2 )
GUCEF_MATH_CURVE_PREDEFINED_POINTS( GUCEF_MATH_INSTANTIATE_BEZIER3 )
GUCEF_MATH_CURVE_PREDEFINED_POINTS( GUCEF_MATH_INSTANTIATE_HERMITE )
GUCEF_MATH_CURVE_PREDEFINED_POINTS( GUCEF_MATH_INSTANTIATE_CATMULLROM )
GUCEF_MATH_CURVE_PREDEFINED_POINTS( GUCEF_MATH_INSTANTIATE_BSPLINE )
GUCEF_MATH_CURVE_PREDEFINED_POINTS( GUCEF_MATH_INSTANTIATE_PATH )

GUCEF_MATH_INSTANTIATE_TEMPLATE_CLASS( CTArcLengthTable< Float32 > )
GUCEF_MATH_INSTANTIATE_TEMPLATE_CLASS( CTArcLengthTable< Float64 > )
GUCEF_MATH_INSTANTIATE_TEMPLATE_CLASS( CTCurve< Float32 > )
GUCEF_MATH_INSTANTIATE_TEMPLATE_CLASS( CTCurve< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
