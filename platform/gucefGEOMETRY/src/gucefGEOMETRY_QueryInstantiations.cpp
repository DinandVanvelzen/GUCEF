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
 *  The exported instantiations of the query functions for the predefined kinds (section 5.10 of the design), compiled
 *  here without floating-point contraction so that the precision-sensitive functions give identical results in every
 *  build (the watertight ray triangle test depends on it).
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_QUERIES_H
#include "gucefGEOMETRY_queries.h"
#define GUCEF_GEOMETRY_QUERIES_H
#endif /* GUCEF_GEOMETRY_QUERIES_H ? */

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

GUCEF_GEOMETRY_SEGMENTQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE )
GUCEF_GEOMETRY_RAYQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE )
GUCEF_GEOMETRY_PLANEQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE )
GUCEF_GEOMETRY_TRIANGLEQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE )
GUCEF_GEOMETRY_SPHEREQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE )
GUCEF_GEOMETRY_CAPSULEQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE )
GUCEF_GEOMETRY_AXISALIGNEDBOXQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE )
GUCEF_GEOMETRY_ORIENTEDBOXQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE )
GUCEF_GEOMETRY_VOLUMEQUERIES_PREDEFINED_KINDS( GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE )
GUCEF_GEOMETRY_QUERIES2D_PREDEFINED_KINDS( GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
