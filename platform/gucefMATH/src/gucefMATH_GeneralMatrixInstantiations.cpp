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
 *  The exported instantiations of the general matrices, the decompositions, the least-squares fits and the spherical
 *  harmonics for the predefined kinds (section 5.10 of the design), compiled here without floating-point contraction.
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

#ifndef GUCEF_MATH_CTLUDECOMPOSITION_H
#include "gucefMATH_CTLuDecomposition.h"
#define GUCEF_MATH_CTLUDECOMPOSITION_H
#endif /* GUCEF_MATH_CTLUDECOMPOSITION_H ? */

#ifndef GUCEF_MATH_CTLEASTSQUARES_H
#include "gucefMATH_CTLeastSquares.h"
#define GUCEF_MATH_CTLEASTSQUARES_H
#endif /* GUCEF_MATH_CTLEASTSQUARES_H ? */

#ifndef GUCEF_MATH_CTSVD3_H
#include "gucefMATH_CTSvd3.h"
#define GUCEF_MATH_CTSVD3_H
#endif /* GUCEF_MATH_CTSVD3_H ? */

#ifndef GUCEF_MATH_CTSPHERICALHARMONICSROTATION_H
#include "gucefMATH_CTSphericalHarmonicsRotation.h"
#define GUCEF_MATH_CTSPHERICALHARMONICSROTATION_H
#endif /* GUCEF_MATH_CTSPHERICALHARMONICSROTATION_H ? */

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

GUCEF_MATH_CTMATRIXKERNELS_PREDEFINED_KINDS( GUCEF_MATH_INSTANTIATE_TEMPLATE )
GUCEF_MATH_CTDYNAMICVECTOR_PREDEFINED_KINDS( GUCEF_MATH_INSTANTIATE_TEMPLATE )
GUCEF_MATH_CTDYNAMICMATRIX_PREDEFINED_KINDS( GUCEF_MATH_INSTANTIATE_TEMPLATE )
GUCEF_MATH_CTLUDECOMPOSITION_PREDEFINED_KINDS( GUCEF_MATH_INSTANTIATE_TEMPLATE )
GUCEF_MATH_CTQRDECOMPOSITION_PREDEFINED_KINDS( GUCEF_MATH_INSTANTIATE_TEMPLATE )
GUCEF_MATH_CTCHOLESKYDECOMPOSITION_PREDEFINED_KINDS( GUCEF_MATH_INSTANTIATE_TEMPLATE )
GUCEF_MATH_CTSYMMETRICEIGEN3_PREDEFINED_KINDS( GUCEF_MATH_INSTANTIATE_TEMPLATE )
GUCEF_MATH_CTSVD3_PREDEFINED_KINDS( GUCEF_MATH_INSTANTIATE_TEMPLATE )
GUCEF_MATH_CTLEASTSQUARES_PREDEFINED_KINDS( GUCEF_MATH_INSTANTIATE_TEMPLATE )
GUCEF_MATH_CTSPHERICALHARMONICS_PREDEFINED_KINDS( GUCEF_MATH_INSTANTIATE_TEMPLATE )
GUCEF_MATH_CTSPHERICALHARMONICSROTATION_PREDEFINED_KINDS( GUCEF_MATH_INSTANTIATE_TEMPLATE )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
