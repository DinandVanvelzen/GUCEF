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

#ifndef GUCEF_GEOMETRY_VOLUMESHAPES_H
#define GUCEF_GEOMETRY_VOLUMESHAPES_H

/*
 *  The typedef family of the convex volume shapes (oriented box, cylinder, cone, frustum and plane-bounded volume), as
 *  gucefGEOMETRY_shapes.h is for the shapes of the first value shape package. Float32 and Float64 are the exported
 *  kinds (section 5.10 of the design); other kinds instantiate the headers in the code of the user.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_SHAPES_H
#include "gucefGEOMETRY_shapes.h"
#define GUCEF_GEOMETRY_SHAPES_H
#endif /* GUCEF_GEOMETRY_SHAPES_H ? */

#ifndef GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H
#include "gucefGEOMETRY_CTGeometryTolerance.h"
#define GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H
#endif /* GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H ? */

#ifndef GUCEF_GEOMETRY_CTORIENTEDBOX3D_H
#include "gucefGEOMETRY_CTOrientedBox3D.h"
#define GUCEF_GEOMETRY_CTORIENTEDBOX3D_H
#endif /* GUCEF_GEOMETRY_CTORIENTEDBOX3D_H ? */

#ifndef GUCEF_GEOMETRY_CTCYLINDER_H
#include "gucefGEOMETRY_CTCylinder.h"
#define GUCEF_GEOMETRY_CTCYLINDER_H
#endif /* GUCEF_GEOMETRY_CTCYLINDER_H ? */

#ifndef GUCEF_GEOMETRY_CTCONE_H
#include "gucefGEOMETRY_CTCone.h"
#define GUCEF_GEOMETRY_CTCONE_H
#endif /* GUCEF_GEOMETRY_CTCONE_H ? */

#ifndef GUCEF_GEOMETRY_CTFRUSTUM_H
#include "gucefGEOMETRY_CTFrustum.h"
#define GUCEF_GEOMETRY_CTFRUSTUM_H
#endif /* GUCEF_GEOMETRY_CTFRUSTUM_H ? */

#ifndef GUCEF_GEOMETRY_CTPLANEBOUNDEDVOLUME_H
#include "gucefGEOMETRY_CTPlaneBoundedVolume.h"
#define GUCEF_GEOMETRY_CTPLANEBOUNDEDVOLUME_H
#endif /* GUCEF_GEOMETRY_CTPLANEBOUNDEDVOLUME_H ? */

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

typedef CTOrientedBox3D< Float32 > TFloat32OrientedBox3D;
typedef CTCylinder< Float32 > TFloat32Cylinder;
typedef CTCone< Float32 > TFloat32Cone;
typedef CTFrustum< Float32 > TFloat32Frustum;
typedef CTPlaneBoundedVolume< Float32 > TFloat32PlaneBoundedVolume;

typedef CTOrientedBox3D< Float64 > TFloat64OrientedBox3D;
typedef CTCylinder< Float64 > TFloat64Cylinder;
typedef CTCone< Float64 > TFloat64Cone;
typedef CTFrustum< Float64 > TFloat64Frustum;
typedef CTPlaneBoundedVolume< Float64 > TFloat64PlaneBoundedVolume;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_VOLUMESHAPES_H ? */
