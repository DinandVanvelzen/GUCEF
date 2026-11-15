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
 *  The exported instantiations of GJK, EPA, the box SAT manifold, the convex hull, the G/E query cells, the sweeps,
 *  time of impact and the bounding-volume fitting for the predefined kinds (section 5.10 of the design; TFixed32_32 by
 *  amendment A-3 of section 11.2), compiled here without floating-point contraction.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_SWEEPQUERIES_H
#include "gucefGEOMETRY_SweepQueries.h"
#define GUCEF_GEOMETRY_SWEEPQUERIES_H
#endif /* GUCEF_GEOMETRY_SWEEPQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_TIMEOFIMPACT_H
#include "gucefGEOMETRY_TimeOfImpact.h"
#define GUCEF_GEOMETRY_TIMEOFIMPACT_H
#endif /* GUCEF_GEOMETRY_TIMEOFIMPACT_H ? */

#ifndef GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_H
#include "gucefGEOMETRY_BoundingVolumeFitting.h"
#define GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_H
#endif /* GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_H ? */

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

GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTContactManifold< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTContactManifold< Float64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTContactManifold< MATH::TFixed32_32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTEpa< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTEpa< Float64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTEpa< MATH::TFixed32_32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTSatBoxes< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTSatBoxes< Float64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTSatBoxes< MATH::TFixed32_32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTMotion3D< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTMotion3D< Float64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTMotion3D< MATH::TFixed32_32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTConvexHull3D< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTConvexHull3D< Float64 > )

GUCEF_GEOMETRY_CTGJK_PREDEFINED_KINDS( GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE )
GUCEF_GEOMETRY_CONVEXQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE )
GUCEF_GEOMETRY_SWEEPQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE )
GUCEF_GEOMETRY_TIMEOFIMPACT_PREDEFINED_KINDS( GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE )
GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_PREDEFINED_KINDS( GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
