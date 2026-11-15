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

#ifndef GUCEF_GEOMETRY_SHAPES_H
#define GUCEF_GEOMETRY_SHAPES_H

/*
 *  The shape typedef family: T<kind><Shape> for Float32 and Float64 (for example TFloat32Sphere is CTSphere< Float32 >,
 *  TFloat64AxisAlignedBox3D is CTAxisAlignedBox3D< Float64 >) and for Int32 the integer ranges (TInt32IntegerRect,
 *  TInt32IntegerBox3D). The predefined kinds are instantiated and exported by the module.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#include "gucefGEOMETRY_CTAxisAlignedBox3D.h"
#define GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#endif /* GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H ? */

#ifndef GUCEF_GEOMETRY_CTCAPSULE_H
#include "gucefGEOMETRY_CTCapsule.h"
#define GUCEF_GEOMETRY_CTCAPSULE_H
#endif /* GUCEF_GEOMETRY_CTCAPSULE_H ? */

#ifndef GUCEF_GEOMETRY_CTCIRCLE_H
#include "gucefGEOMETRY_CTCircle.h"
#define GUCEF_GEOMETRY_CTCIRCLE_H
#endif /* GUCEF_GEOMETRY_CTCIRCLE_H ? */

#ifndef GUCEF_GEOMETRY_CTINTEGERBOX3D_H
#include "gucefGEOMETRY_CTIntegerBox3D.h"
#define GUCEF_GEOMETRY_CTINTEGERBOX3D_H
#endif /* GUCEF_GEOMETRY_CTINTEGERBOX3D_H ? */

#ifndef GUCEF_GEOMETRY_CTINTEGERRECT_H
#include "gucefGEOMETRY_CTIntegerRect.h"
#define GUCEF_GEOMETRY_CTINTEGERRECT_H
#endif /* GUCEF_GEOMETRY_CTINTEGERRECT_H ? */

#ifndef GUCEF_GEOMETRY_CTLINE2D_H
#include "gucefGEOMETRY_CTLine2D.h"
#define GUCEF_GEOMETRY_CTLINE2D_H
#endif /* GUCEF_GEOMETRY_CTLINE2D_H ? */

#ifndef GUCEF_GEOMETRY_CTLINE3D_H
#include "gucefGEOMETRY_CTLine3D.h"
#define GUCEF_GEOMETRY_CTLINE3D_H
#endif /* GUCEF_GEOMETRY_CTLINE3D_H ? */

#ifndef GUCEF_GEOMETRY_CTORIENTEDRECTANGLE_H
#include "gucefGEOMETRY_CTOrientedRectangle.h"
#define GUCEF_GEOMETRY_CTORIENTEDRECTANGLE_H
#endif /* GUCEF_GEOMETRY_CTORIENTEDRECTANGLE_H ? */

#ifndef GUCEF_GEOMETRY_CTPLANE_H
#include "gucefGEOMETRY_CTPlane.h"
#define GUCEF_GEOMETRY_CTPLANE_H
#endif /* GUCEF_GEOMETRY_CTPLANE_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYGON2D_H
#include "gucefGEOMETRY_CTPolygon2D.h"
#define GUCEF_GEOMETRY_CTPOLYGON2D_H
#endif /* GUCEF_GEOMETRY_CTPOLYGON2D_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYGONWITHHOLES2D_H
#include "gucefGEOMETRY_CTPolygonWithHoles2D.h"
#define GUCEF_GEOMETRY_CTPOLYGONWITHHOLES2D_H
#endif /* GUCEF_GEOMETRY_CTPOLYGONWITHHOLES2D_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYLINE2D_H
#include "gucefGEOMETRY_CTPolyline2D.h"
#define GUCEF_GEOMETRY_CTPOLYLINE2D_H
#endif /* GUCEF_GEOMETRY_CTPOLYLINE2D_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYLINE3D_H
#include "gucefGEOMETRY_CTPolyline3D.h"
#define GUCEF_GEOMETRY_CTPOLYLINE3D_H
#endif /* GUCEF_GEOMETRY_CTPOLYLINE3D_H ? */

#ifndef GUCEF_GEOMETRY_CTRAY2D_H
#include "gucefGEOMETRY_CTRay2D.h"
#define GUCEF_GEOMETRY_CTRAY2D_H
#endif /* GUCEF_GEOMETRY_CTRAY2D_H ? */

#ifndef GUCEF_GEOMETRY_CTRAY3D_H
#include "gucefGEOMETRY_CTRay3D.h"
#define GUCEF_GEOMETRY_CTRAY3D_H
#endif /* GUCEF_GEOMETRY_CTRAY3D_H ? */

#ifndef GUCEF_GEOMETRY_CTRECTANGLE_H
#include "gucefGEOMETRY_CTRectangle.h"
#define GUCEF_GEOMETRY_CTRECTANGLE_H
#endif /* GUCEF_GEOMETRY_CTRECTANGLE_H ? */

#ifndef GUCEF_GEOMETRY_CTSEGMENT2D_H
#include "gucefGEOMETRY_CTSegment2D.h"
#define GUCEF_GEOMETRY_CTSEGMENT2D_H
#endif /* GUCEF_GEOMETRY_CTSEGMENT2D_H ? */

#ifndef GUCEF_GEOMETRY_CTSEGMENT3D_H
#include "gucefGEOMETRY_CTSegment3D.h"
#define GUCEF_GEOMETRY_CTSEGMENT3D_H
#endif /* GUCEF_GEOMETRY_CTSEGMENT3D_H ? */

#ifndef GUCEF_GEOMETRY_CTSPHERE_H
#include "gucefGEOMETRY_CTSphere.h"
#define GUCEF_GEOMETRY_CTSPHERE_H
#endif /* GUCEF_GEOMETRY_CTSPHERE_H ? */

#ifndef GUCEF_GEOMETRY_CTTRIANGLE2D_H
#include "gucefGEOMETRY_CTTriangle2D.h"
#define GUCEF_GEOMETRY_CTTRIANGLE2D_H
#endif /* GUCEF_GEOMETRY_CTTRIANGLE2D_H ? */

#ifndef GUCEF_GEOMETRY_CTTRIANGLE3D_H
#include "gucefGEOMETRY_CTTriangle3D.h"
#define GUCEF_GEOMETRY_CTTRIANGLE3D_H
#endif /* GUCEF_GEOMETRY_CTTRIANGLE3D_H ? */

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

typedef CTRay3D< Float32 > TFloat32Ray3D;
typedef CTLine3D< Float32 > TFloat32Line3D;
typedef CTSegment3D< Float32 > TFloat32Segment3D;
typedef CTPlane< Float32 > TFloat32Plane;
typedef CTSphere< Float32 > TFloat32Sphere;
typedef CTAxisAlignedBox3D< Float32 > TFloat32AxisAlignedBox3D;
typedef CTTriangle3D< Float32 > TFloat32Triangle3D;
typedef CTCapsule< Float32 > TFloat32Capsule;
typedef CTRay2D< Float32 > TFloat32Ray2D;
typedef CTLine2D< Float32 > TFloat32Line2D;
typedef CTSegment2D< Float32 > TFloat32Segment2D;
typedef CTCircle< Float32 > TFloat32Circle;
typedef CTRectangle< Float32 > TFloat32Rectangle;
typedef CTOrientedRectangle< Float32 > TFloat32OrientedRectangle;
typedef CTTriangle2D< Float32 > TFloat32Triangle2D;
typedef CTPolygon2D< Float32 > TFloat32Polygon2D;
typedef CTPolygonWithHoles2D< Float32 > TFloat32PolygonWithHoles2D;
typedef CTPolyline2D< Float32 > TFloat32Polyline2D;
typedef CTPolyline3D< Float32 > TFloat32Polyline3D;

typedef CTRay3D< Float64 > TFloat64Ray3D;
typedef CTLine3D< Float64 > TFloat64Line3D;
typedef CTSegment3D< Float64 > TFloat64Segment3D;
typedef CTPlane< Float64 > TFloat64Plane;
typedef CTSphere< Float64 > TFloat64Sphere;
typedef CTAxisAlignedBox3D< Float64 > TFloat64AxisAlignedBox3D;
typedef CTTriangle3D< Float64 > TFloat64Triangle3D;
typedef CTCapsule< Float64 > TFloat64Capsule;
typedef CTRay2D< Float64 > TFloat64Ray2D;
typedef CTLine2D< Float64 > TFloat64Line2D;
typedef CTSegment2D< Float64 > TFloat64Segment2D;
typedef CTCircle< Float64 > TFloat64Circle;
typedef CTRectangle< Float64 > TFloat64Rectangle;
typedef CTOrientedRectangle< Float64 > TFloat64OrientedRectangle;
typedef CTTriangle2D< Float64 > TFloat64Triangle2D;
typedef CTPolygon2D< Float64 > TFloat64Polygon2D;
typedef CTPolygonWithHoles2D< Float64 > TFloat64PolygonWithHoles2D;
typedef CTPolyline2D< Float64 > TFloat64Polyline2D;
typedef CTPolyline3D< Float64 > TFloat64Polyline3D;

/*
 *  The straight-line shapes of the exact Int32 kind (the closed integer box and rectangle, distinct from the half-open
 *  integer ranges below). Members that need a square root or a division (lengths, distances, centres, areas by halves)
 *  do not compile for this kind, the others do. The exact kinds are not instantiated by the module (their results do
 *  not depend on compiler flags), the user's translation unit instantiates what it uses.
 */
typedef CTSegment2D< Int32 > TInt32Segment2D;
typedef CTSegment3D< Int32 > TInt32Segment3D;
typedef CTRectangle< Int32 > TInt32Rectangle;
typedef CTAxisAlignedBox3D< Int32 > TInt32AxisAlignedBox3D;
typedef CTPolygon2D< Int32 > TInt32Polygon2D;

typedef CTIntegerRect< Int32 > TInt32IntegerRect;
typedef CTIntegerBox3D< Int32 > TInt32IntegerBox3D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_SHAPES_H ? */
