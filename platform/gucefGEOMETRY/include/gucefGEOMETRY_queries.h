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

#ifndef GUCEF_GEOMETRY_QUERIES_H
#define GUCEF_GEOMETRY_QUERIES_H

/*
 *  All query headers of the first query package (section 7.5 of the design) and the volume shapes they need: the
 *  facade functions Distance, ClosestPoints, Intersects and Contains over the pairs of shapes, and the ray cast
 *  facade. The pairs that need GJK and EPA are added by the convex query header of the GJK package.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_VOLUMESHAPES_H
#include "gucefGEOMETRY_volumeshapes.h"
#define GUCEF_GEOMETRY_VOLUMESHAPES_H
#endif /* GUCEF_GEOMETRY_VOLUMESHAPES_H ? */

#ifndef GUCEF_GEOMETRY_POINTQUERIES_H
#include "gucefGEOMETRY_PointQueries.h"
#define GUCEF_GEOMETRY_POINTQUERIES_H
#endif /* GUCEF_GEOMETRY_POINTQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_SEGMENTQUERIES_H
#include "gucefGEOMETRY_SegmentQueries.h"
#define GUCEF_GEOMETRY_SEGMENTQUERIES_H
#endif /* GUCEF_GEOMETRY_SEGMENTQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_RAYQUERIES_H
#include "gucefGEOMETRY_RayQueries.h"
#define GUCEF_GEOMETRY_RAYQUERIES_H
#endif /* GUCEF_GEOMETRY_RAYQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_PLANEQUERIES_H
#include "gucefGEOMETRY_PlaneQueries.h"
#define GUCEF_GEOMETRY_PLANEQUERIES_H
#endif /* GUCEF_GEOMETRY_PLANEQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_TRIANGLEQUERIES_H
#include "gucefGEOMETRY_TriangleQueries.h"
#define GUCEF_GEOMETRY_TRIANGLEQUERIES_H
#endif /* GUCEF_GEOMETRY_TRIANGLEQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_SPHEREQUERIES_H
#include "gucefGEOMETRY_SphereQueries.h"
#define GUCEF_GEOMETRY_SPHEREQUERIES_H
#endif /* GUCEF_GEOMETRY_SPHEREQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_CAPSULEQUERIES_H
#include "gucefGEOMETRY_CapsuleQueries.h"
#define GUCEF_GEOMETRY_CAPSULEQUERIES_H
#endif /* GUCEF_GEOMETRY_CAPSULEQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_AXISALIGNEDBOXQUERIES_H
#include "gucefGEOMETRY_AxisAlignedBoxQueries.h"
#define GUCEF_GEOMETRY_AXISALIGNEDBOXQUERIES_H
#endif /* GUCEF_GEOMETRY_AXISALIGNEDBOXQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_ORIENTEDBOXQUERIES_H
#include "gucefGEOMETRY_OrientedBoxQueries.h"
#define GUCEF_GEOMETRY_ORIENTEDBOXQUERIES_H
#endif /* GUCEF_GEOMETRY_ORIENTEDBOXQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_VOLUMEQUERIES_H
#include "gucefGEOMETRY_VolumeQueries.h"
#define GUCEF_GEOMETRY_VOLUMEQUERIES_H
#endif /* GUCEF_GEOMETRY_VOLUMEQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_QUERIES2D_H
#include "gucefGEOMETRY_Queries2D.h"
#define GUCEF_GEOMETRY_QUERIES2D_H
#endif /* GUCEF_GEOMETRY_QUERIES2D_H ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_QUERIES_H ? */
