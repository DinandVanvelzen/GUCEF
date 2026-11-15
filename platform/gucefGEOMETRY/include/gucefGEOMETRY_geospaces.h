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

#ifndef GUCEF_GEOMETRY_GEOSPACES_H
#define GUCEF_GEOMETRY_GEOSPACES_H

/*
 *  The relative transform tree, linked objects and their queries (sections 7.3 and 7.4 of the design) with the
 *  typedefs of the predefined kinds: the default typedefs drop the kind (Float32), the "64" ones are Float64 (large
 *  worlds), CGeoSpaceFixed is the deterministic TFixed32_32 tree (amendment A-1 of section 11.2).
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_CTGEOSPACESNAPSHOTSET_H
#include "gucefGEOMETRY_CTGeoSpaceSnapshotSet.h"
#define GUCEF_GEOMETRY_CTGEOSPACESNAPSHOTSET_H
#endif /* GUCEF_GEOMETRY_CTGEOSPACESNAPSHOTSET_H ? */

#ifndef GUCEF_GEOMETRY_GEOOBJECTQUERIES_H
#include "gucefGEOMETRY_GeoObjectQueries.h"
#define GUCEF_GEOMETRY_GEOOBJECTQUERIES_H
#endif /* GUCEF_GEOMETRY_GEOOBJECTQUERIES_H ? */

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

typedef CTGeoSpace< Float32 > CGeoSpace;
typedef CTGeoSpace< Float64 > CGeoSpace64;
typedef CTGeoSpace< MATH::TFixed32_32 > CGeoSpaceFixed;

typedef CTGeoSpaceSnapshotSet< Float32 > CGeoSpaceSnapshotSet;
typedef CTGeoSpaceSnapshotSet< Float64 > CGeoSpaceSnapshotSet64;
typedef CTGeoSpaceSnapshotSet< MATH::TFixed32_32 > CGeoSpaceSnapshotSetFixed;

typedef CTGeoObjectBase< Float32 > CGeoObjectBase;
typedef CTGeoObjectBase< Float64 > CGeoObjectBase64;
typedef CTGeoObjectStructure< Float32 > CGeoObjectStructure;
typedef CTGeoObjectStructure< Float64 > CGeoObjectStructure64;

typedef CTGeoObject< CTSphere< Float32 > > CGeoSphere;
typedef CTGeoObject< CTAxisAlignedBox3D< Float32 > > CGeoAxisAlignedBox;
typedef CTGeoObject< CTOrientedBox3D< Float32 > > CGeoOrientedBox;
typedef CTGeoObject< CTCapsule< Float32 > > CGeoCapsule;
typedef CTGeoObject< CTCylinder< Float32 > > CGeoCylinder;
typedef CTGeoObject< CTCone< Float32 > > CGeoCone;
typedef CTGeoObject< CTTriangle3D< Float32 > > CGeoTriangle;
typedef CTGeoObject< CTConvexHull3D< Float32 > > CGeoConvexHull;
typedef CTGeoObject< CTPlaneBoundedVolume< Float32 > > CGeoPlaneBoundedVolume;

typedef CTGeoObject< CTSphere< Float64 > > CGeoSphere64;
typedef CTGeoObject< CTAxisAlignedBox3D< Float64 > > CGeoAxisAlignedBox64;
typedef CTGeoObject< CTOrientedBox3D< Float64 > > CGeoOrientedBox64;
typedef CTGeoObject< CTCapsule< Float64 > > CGeoCapsule64;
typedef CTGeoObject< CTCylinder< Float64 > > CGeoCylinder64;
typedef CTGeoObject< CTCone< Float64 > > CGeoCone64;
typedef CTGeoObject< CTTriangle3D< Float64 > > CGeoTriangle64;
typedef CTGeoObject< CTConvexHull3D< Float64 > > CGeoConvexHull64;
typedef CTGeoObject< CTPlaneBoundedVolume< Float64 > > CGeoPlaneBoundedVolume64;

typedef CTGeoObject2D< CTCircle< Float32 > > CGeoCircle;
typedef CTGeoObject2D< CTRectangle< Float32 > > CGeoRectangle;
typedef CTGeoObject2D< CTOrientedRectangle< Float32 > > CGeoOrientedRectangle;
typedef CTGeoObject2D< CTTriangle2D< Float32 > > CGeoTriangle2D;
typedef CTGeoObject2D< CTPolygon2D< Float32 > > CGeoPolygon;
typedef CTGeoObject2D< CTPolygonWithHoles2D< Float32 > > CGeoPolygonWithHoles;
typedef CTGeoObject2D< CTPolyline2D< Float32 > > CGeoPolyline2D;

typedef CTGeoObject2D< CTCircle< Float64 > > CGeoCircle64;
typedef CTGeoObject2D< CTRectangle< Float64 > > CGeoRectangle64;
typedef CTGeoObject2D< CTOrientedRectangle< Float64 > > CGeoOrientedRectangle64;
typedef CTGeoObject2D< CTTriangle2D< Float64 > > CGeoTriangle2D64;
typedef CTGeoObject2D< CTPolygon2D< Float64 > > CGeoPolygon64;
typedef CTGeoObject2D< CTPolygonWithHoles2D< Float64 > > CGeoPolygonWithHoles64;
typedef CTGeoObject2D< CTPolyline2D< Float64 > > CGeoPolyline2D64;

typedef CTGeoPlane< Float32 > CGeoPlane;
typedef CTGeoPlane< Float64 > CGeoPlane64;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_GEOSPACES_H ? */
