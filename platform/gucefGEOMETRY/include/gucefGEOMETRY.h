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
 *      This is the main include header for the gucefGEOMETRY library.
 *      It is part of a framework called GUCEF.
 */

#ifndef GUCEF_GEOMETRY_H
#define GUCEF_GEOMETRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Library build configuration
 */
#ifndef GUCEF_GEOMETRY_CONFIG_H
#include "gucefGEOMETRY_config.h"
#define GUCEF_GEOMETRY_CONFIG_H
#endif /* GUCEF_GEOMETRY_CONFIG_H ? */

/*
 *      Library specific macros and generic macros
 */
#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

/*
 *      Simple types
 */
#ifndef GUCEF_GEOMETRY_ETYPES_H
#include "gucefGEOMETRY_ETypes.h"
#define GUCEF_GEOMETRY_ETYPES_H
#endif /* GUCEF_GEOMETRY_ETYPES_H ? */

#ifdef __cplusplus

#ifndef GUCEF_GEOMETRY_CMODULE_H
#include "gucefGEOMETRY_CModule.h"
#define GUCEF_GEOMETRY_CMODULE_H
#endif /* GUCEF_GEOMETRY_CMODULE_H ? */

#ifndef GUCEF_GEOMETRY_SHAPES_H
#include "gucefGEOMETRY_shapes.h"
#define GUCEF_GEOMETRY_SHAPES_H
#endif /* GUCEF_GEOMETRY_SHAPES_H ? */

#ifndef GUCEF_GEOMETRY_VOLUMESHAPES_H
#include "gucefGEOMETRY_volumeshapes.h"
#define GUCEF_GEOMETRY_VOLUMESHAPES_H
#endif /* GUCEF_GEOMETRY_VOLUMESHAPES_H ? */

#ifndef GUCEF_GEOMETRY_QUERIES_H
#include "gucefGEOMETRY_queries.h"
#define GUCEF_GEOMETRY_QUERIES_H
#endif /* GUCEF_GEOMETRY_QUERIES_H ? */

#ifndef GUCEF_GEOMETRY_CONVEXQUERIES_H
#include "gucefGEOMETRY_ConvexQueries.h"
#define GUCEF_GEOMETRY_CONVEXQUERIES_H
#endif /* GUCEF_GEOMETRY_CONVEXQUERIES_H ? */

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

#ifndef GUCEF_GEOMETRY_CTPREDICATES_H
#include "gucefGEOMETRY_CTPredicates.h"
#define GUCEF_GEOMETRY_CTPREDICATES_H
#endif /* GUCEF_GEOMETRY_CTPREDICATES_H ? */

#ifndef GUCEF_GEOMETRY_GEOSPACES_H
#include "gucefGEOMETRY_geospaces.h"
#define GUCEF_GEOMETRY_GEOSPACES_H
#endif /* GUCEF_GEOMETRY_GEOSPACES_H ? */

#ifndef GUCEF_GEOMETRY_CTOCTREE_H
#include "gucefGEOMETRY_CTOctree.h"
#define GUCEF_GEOMETRY_CTOCTREE_H
#endif /* GUCEF_GEOMETRY_CTOCTREE_H ? */

#ifndef GUCEF_GEOMETRY_CTQUADTREE_H
#include "gucefGEOMETRY_CTQuadtree.h"
#define GUCEF_GEOMETRY_CTQUADTREE_H
#endif /* GUCEF_GEOMETRY_CTQUADTREE_H ? */

#ifndef GUCEF_GEOMETRY_CTUNIFORMGRID2D_H
#include "gucefGEOMETRY_CTUniformGrid2D.h"
#define GUCEF_GEOMETRY_CTUNIFORMGRID2D_H
#endif /* GUCEF_GEOMETRY_CTUNIFORMGRID2D_H ? */

#ifndef GUCEF_GEOMETRY_CTUNIFORMGRID3D_H
#include "gucefGEOMETRY_CTUniformGrid3D.h"
#define GUCEF_GEOMETRY_CTUNIFORMGRID3D_H
#endif /* GUCEF_GEOMETRY_CTUNIFORMGRID3D_H ? */

#ifndef GUCEF_GEOMETRY_CTSPATIALHASH_H
#include "gucefGEOMETRY_CTSpatialHash.h"
#define GUCEF_GEOMETRY_CTSPATIALHASH_H
#endif /* GUCEF_GEOMETRY_CTSPATIALHASH_H ? */

#ifndef GUCEF_GEOMETRY_CTKDTREE_H
#include "gucefGEOMETRY_CTKdTree.h"
#define GUCEF_GEOMETRY_CTKDTREE_H
#endif /* GUCEF_GEOMETRY_CTKDTREE_H ? */

#ifndef GUCEF_GEOMETRY_FRUSTUMCULLING_H
#include "gucefGEOMETRY_FrustumCulling.h"
#define GUCEF_GEOMETRY_FRUSTUMCULLING_H
#endif /* GUCEF_GEOMETRY_FRUSTUMCULLING_H ? */

#ifndef GUCEF_GEOMETRY_CTGRIDTRAVERSAL2D_H
#include "gucefGEOMETRY_CTGridTraversal2D.h"
#define GUCEF_GEOMETRY_CTGRIDTRAVERSAL2D_H
#endif /* GUCEF_GEOMETRY_CTGRIDTRAVERSAL2D_H ? */

#ifndef GUCEF_GEOMETRY_CTGRIDTRAVERSAL3D_H
#include "gucefGEOMETRY_CTGridTraversal3D.h"
#define GUCEF_GEOMETRY_CTGRIDTRAVERSAL3D_H
#endif /* GUCEF_GEOMETRY_CTGRIDTRAVERSAL3D_H ? */

#ifndef GUCEF_GEOMETRY_CTBRESENHAMLINE2D_H
#include "gucefGEOMETRY_CTBresenhamLine2D.h"
#define GUCEF_GEOMETRY_CTBRESENHAMLINE2D_H
#endif /* GUCEF_GEOMETRY_CTBRESENHAMLINE2D_H ? */

#ifndef GUCEF_GEOMETRY_CTBRESENHAMLINE3D_H
#include "gucefGEOMETRY_CTBresenhamLine3D.h"
#define GUCEF_GEOMETRY_CTBRESENHAMLINE3D_H
#endif /* GUCEF_GEOMETRY_CTBRESENHAMLINE3D_H ? */

#ifndef GUCEF_GEOMETRY_CTHEIGHTFIELD_H
#include "gucefGEOMETRY_CTHeightField.h"
#define GUCEF_GEOMETRY_CTHEIGHTFIELD_H
#endif /* GUCEF_GEOMETRY_CTHEIGHTFIELD_H ? */

#ifndef GUCEF_GEOMETRY_POINTINPOLYGON_H
#include "gucefGEOMETRY_PointInPolygon.h"
#define GUCEF_GEOMETRY_POINTINPOLYGON_H
#endif /* GUCEF_GEOMETRY_POINTINPOLYGON_H ? */

#ifndef GUCEF_GEOMETRY_CONVEXHULL2D_H
#include "gucefGEOMETRY_ConvexHull2D.h"
#define GUCEF_GEOMETRY_CONVEXHULL2D_H
#endif /* GUCEF_GEOMETRY_CONVEXHULL2D_H ? */

#ifndef GUCEF_GEOMETRY_CTQUICKHULL3D_H
#include "gucefGEOMETRY_CTQuickHull3D.h"
#define GUCEF_GEOMETRY_CTQUICKHULL3D_H
#endif /* GUCEF_GEOMETRY_CTQUICKHULL3D_H ? */

#ifndef GUCEF_GEOMETRY_POLYLINESIMPLIFICATION_H
#include "gucefGEOMETRY_PolylineSimplification.h"
#define GUCEF_GEOMETRY_POLYLINESIMPLIFICATION_H
#endif /* GUCEF_GEOMETRY_POLYLINESIMPLIFICATION_H ? */

#ifndef GUCEF_GEOMETRY_CTTRIANGLEMESH_H
#include "gucefGEOMETRY_CTTriangleMesh.h"
#define GUCEF_GEOMETRY_CTTRIANGLEMESH_H
#endif /* GUCEF_GEOMETRY_CTTRIANGLEMESH_H ? */

#ifndef GUCEF_GEOMETRY_CTMESHBVH_H
#include "gucefGEOMETRY_CTMeshBvh.h"
#define GUCEF_GEOMETRY_CTMESHBVH_H
#endif /* GUCEF_GEOMETRY_CTMESHBVH_H ? */

#ifndef GUCEF_GEOMETRY_MESHQUERIES_H
#include "gucefGEOMETRY_MeshQueries.h"
#define GUCEF_GEOMETRY_MESHQUERIES_H
#endif /* GUCEF_GEOMETRY_MESHQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_CTDYNAMICAABBTREE_H
#include "gucefGEOMETRY_CTDynamicAabbTree.h"
#define GUCEF_GEOMETRY_CTDYNAMICAABBTREE_H
#endif /* GUCEF_GEOMETRY_CTDYNAMICAABBTREE_H ? */

#ifndef GUCEF_GEOMETRY_SGEORAYHIT_H
#include "gucefGEOMETRY_SGeoRayHit.h"
#define GUCEF_GEOMETRY_SGEORAYHIT_H
#endif /* GUCEF_GEOMETRY_SGEORAYHIT_H ? */

#ifndef GUCEF_GEOMETRY_CTGEORAYFILTER_H
#include "gucefGEOMETRY_CTGeoRayFilter.h"
#define GUCEF_GEOMETRY_CTGEORAYFILTER_H
#endif /* GUCEF_GEOMETRY_CTGEORAYFILTER_H ? */

#ifndef GUCEF_GEOMETRY_CTGEOTRIANGLEMESH_H
#include "gucefGEOMETRY_CTGeoTriangleMesh.h"
#define GUCEF_GEOMETRY_CTGEOTRIANGLEMESH_H
#endif /* GUCEF_GEOMETRY_CTGEOTRIANGLEMESH_H ? */

#ifndef GUCEF_GEOMETRY_CTRAYCASTER_H
#include "gucefGEOMETRY_CTRayCaster.h"
#define GUCEF_GEOMETRY_CTRAYCASTER_H
#endif /* GUCEF_GEOMETRY_CTRAYCASTER_H ? */

#ifndef GUCEF_GEOMETRY_CTAUDIOOCCLUSIONESTIMATOR_H
#include "gucefGEOMETRY_CTAudioOcclusionEstimator.h"
#define GUCEF_GEOMETRY_CTAUDIOOCCLUSIONESTIMATOR_H
#endif /* GUCEF_GEOMETRY_CTAUDIOOCCLUSIONESTIMATOR_H ? */

#ifndef GUCEF_GEOMETRY_CTIMAGESOURCEMODEL_H
#include "gucefGEOMETRY_CTImageSourceModel.h"
#define GUCEF_GEOMETRY_CTIMAGESOURCEMODEL_H
#endif /* GUCEF_GEOMETRY_CTIMAGESOURCEMODEL_H ? */

#ifndef GUCEF_GEOMETRY_POLYGONTRIANGULATION_H
#include "gucefGEOMETRY_PolygonTriangulation.h"
#define GUCEF_GEOMETRY_POLYGONTRIANGULATION_H
#endif /* GUCEF_GEOMETRY_POLYGONTRIANGULATION_H ? */

#ifndef GUCEF_GEOMETRY_CTCONSTRAINEDDELAUNAY2D_H
#include "gucefGEOMETRY_CTConstrainedDelaunay2D.h"
#define GUCEF_GEOMETRY_CTCONSTRAINEDDELAUNAY2D_H
#endif /* GUCEF_GEOMETRY_CTCONSTRAINEDDELAUNAY2D_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYGONCLIPPER_H
#include "gucefGEOMETRY_CTPolygonClipper.h"
#define GUCEF_GEOMETRY_CTPOLYGONCLIPPER_H
#endif /* GUCEF_GEOMETRY_CTPOLYGONCLIPPER_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYGONOFFSETTER_H
#include "gucefGEOMETRY_CTPolygonOffsetter.h"
#define GUCEF_GEOMETRY_CTPOLYGONOFFSETTER_H
#endif /* GUCEF_GEOMETRY_CTPOLYGONOFFSETTER_H ? */

#ifndef GUCEF_GEOMETRY_CONVEXDECOMPOSITION2D_H
#include "gucefGEOMETRY_ConvexDecomposition2D.h"
#define GUCEF_GEOMETRY_CONVEXDECOMPOSITION2D_H
#endif /* GUCEF_GEOMETRY_CONVEXDECOMPOSITION2D_H ? */

#ifndef GUCEF_GEOMETRY_CTGEOMETRY2D_H
#include "gucefGEOMETRY_CTGeometry2D.h"
#define GUCEF_GEOMETRY_CTGEOMETRY2D_H
#endif /* GUCEF_GEOMETRY_CTGEOMETRY2D_H ? */

#ifndef GUCEF_GEOMETRY_CTGEOMETRY3D_H
#include "gucefGEOMETRY_CTGeometry3D.h"
#define GUCEF_GEOMETRY_CTGEOMETRY3D_H
#endif /* GUCEF_GEOMETRY_CTGEOMETRY3D_H ? */

#ifndef GUCEF_GEOMETRY_BATCHFUNCTIONS_H
#include "gucefGEOMETRY_BatchFunctions.h"
#define GUCEF_GEOMETRY_BATCHFUNCTIONS_H
#endif /* GUCEF_GEOMETRY_BATCHFUNCTIONS_H ? */

#ifndef GUCEF_GEOMETRY_CORESTRUCTINTEROP_H
#include "gucefGEOMETRY_CoreStructInterop.h"
#define GUCEF_GEOMETRY_CORESTRUCTINTEROP_H
#endif /* GUCEF_GEOMETRY_CORESTRUCTINTEROP_H ? */

#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_H ? */
