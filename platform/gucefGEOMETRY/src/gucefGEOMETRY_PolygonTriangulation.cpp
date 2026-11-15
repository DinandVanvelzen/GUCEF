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
 *  The exported instantiations of the ear clipping triangulator for the predefined kinds (section 5.10 of the design),
 *  compiled here without floating-point contraction.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_POLYGONTRIANGULATION_H
#include "gucefGEOMETRY_PolygonTriangulation.h"
#define GUCEF_GEOMETRY_POLYGONTRIANGULATION_H
#endif /* GUCEF_GEOMETRY_POLYGONTRIANGULATION_H ? */

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

GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTPolygonTriangulator< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTPolygonTriangulator< Float64 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTPolygonTriangulator< Int32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTPolygonTriangulator< MATH::TFixed32_32 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
