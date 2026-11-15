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

/*
 *  The exported instantiations of the audio geometry classes (sections 5.10 and 7.11 of the design): the occlusion
 *  estimator, the image source walls, paths and model, compiled here without floating-point contraction.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_CTAUDIOOCCLUSIONESTIMATOR_H
#include "gucefGEOMETRY_CTAudioOcclusionEstimator.h"
#define GUCEF_GEOMETRY_CTAUDIOOCCLUSIONESTIMATOR_H
#endif /* GUCEF_GEOMETRY_CTAUDIOOCCLUSIONESTIMATOR_H ? */

#ifndef GUCEF_GEOMETRY_CTIMAGESOURCEMODEL_H
#include "gucefGEOMETRY_CTImageSourceModel.h"
#define GUCEF_GEOMETRY_CTIMAGESOURCEMODEL_H
#endif /* GUCEF_GEOMETRY_CTIMAGESOURCEMODEL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      INSTANTIATIONS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( SAudioOcclusionEstimate< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( SAudioOcclusionEstimate< Float64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTAudioOcclusionEstimator< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTAudioOcclusionEstimator< Float64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTImageSourceWall< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTImageSourceWall< Float64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( SImageSourcePath< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( SImageSourcePath< Float64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTImageSourceModel< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTImageSourceModel< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
