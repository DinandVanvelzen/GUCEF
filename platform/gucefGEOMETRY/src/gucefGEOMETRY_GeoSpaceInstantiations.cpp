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
 *  The exported instantiations of the relative transform tree, its snapshots, the linked objects and their pair
 *  queries (sections 5.10, 7.3 and 7.4 of the design; the TFixed32_32 tree and snapshots by amendment A-1 of section
 *  11.2), compiled here without floating-point contraction.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_GEOSPACES_H
#include "gucefGEOMETRY_geospaces.h"
#define GUCEF_GEOMETRY_GEOSPACES_H
#endif /* GUCEF_GEOMETRY_GEOSPACES_H ? */

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

GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTGeoSpaceSnapshot< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTGeoSpaceSnapshot< Float64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTGeoSpaceSnapshot< MATH::TFixed32_32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTGeoSpace< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTGeoSpace< Float64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTGeoSpace< MATH::TFixed32_32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTGeoSpaceSnapshotSet< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTGeoSpaceSnapshotSet< Float64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTGeoSpaceSnapshotSet< MATH::TFixed32_32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTGeoObjectBase< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTGeoObjectBase< Float64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTGeoObjectStructure< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTGeoObjectStructure< Float64 > )
GUCEF_GEOMETRY_CTGEOOBJECT_KIND( GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS, Float32 )
GUCEF_GEOMETRY_CTGEOOBJECT_KIND( GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS, Float64 )
GUCEF_GEOMETRY_CTGEOOBJECT2D_KIND( GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS, Float32 )
GUCEF_GEOMETRY_CTGEOOBJECT2D_KIND( GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS, Float64 )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTGeoPlane< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTGeoPlane< Float64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTGeoPairDispatch< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTGeoPairDispatch< Float64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTGeoObjectQueries< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTGeoObjectQueries< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
