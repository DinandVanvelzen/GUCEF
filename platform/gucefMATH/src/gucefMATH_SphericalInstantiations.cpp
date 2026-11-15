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
 *  The exported instantiations of the precision-sensitive members of CTPolarCoordinates, CTSphericalCoordinates and
 *  CTOrthonormalFrame3D and of the vector Doppler ratio for the predefined kinds (section 5.10 of the design), compiled
 *  here without floating-point contraction.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTPOLARCOORDINATES_H
#include "gucefMATH_CTPolarCoordinates.h"
#define GUCEF_MATH_CTPOLARCOORDINATES_H
#endif /* GUCEF_MATH_CTPOLARCOORDINATES_H ? */

#ifndef GUCEF_MATH_CTSPHERICALCOORDINATES_H
#include "gucefMATH_CTSphericalCoordinates.h"
#define GUCEF_MATH_CTSPHERICALCOORDINATES_H
#endif /* GUCEF_MATH_CTSPHERICALCOORDINATES_H ? */

#ifndef GUCEF_MATH_CTORTHONORMALFRAME3D_H
#include "gucefMATH_CTOrthonormalFrame3D.h"
#define GUCEF_MATH_CTORTHONORMALFRAME3D_H
#endif /* GUCEF_MATH_CTORTHONORMALFRAME3D_H ? */

#ifndef GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H
#include "gucefMATH_SpatialAudioVectorFunctions.h"
#define GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H
#endif /* GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_H ? */

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

GUCEF_MATH_CTPOLARCOORDINATES_PREDEFINED_KINDS( GUCEF_MATH_INSTANTIATE_TEMPLATE )
GUCEF_MATH_CTSPHERICALCOORDINATES_PREDEFINED_KINDS( GUCEF_MATH_INSTANTIATE_TEMPLATE )
GUCEF_MATH_CTORTHONORMALFRAME3D_PREDEFINED_KINDS( GUCEF_MATH_INSTANTIATE_TEMPLATE )
GUCEF_MATH_SPATIALAUDIOVECTORFUNCTIONS_PREDEFINED_KINDS( GUCEF_MATH_INSTANTIATE_TEMPLATE )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
