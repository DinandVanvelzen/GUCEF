/*
 *  gucefGEOMETRY_TestApp: GUCEF test application for the Geometry module
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

#ifndef GUCEF_GEOMETRY_TESTAPP_TESTGEOPOLYGONQUERIES_H
#define GUCEF_GEOMETRY_TESTAPP_TESTGEOPOLYGONQUERIES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the pair queries of non-convex polygons hosted on a plane (the cached triangulation): L-shaped, star and holed
 *  polygons against spheres, boxes, capsules and polygons under rotated and scaled parents, compared with brute force
 *  over the world-space triangles; points in a hole; the cache cleared when the shape is set; the plane pair
 */
void
PerformGeoPolygonQueriesTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_TESTAPP_TESTGEOPOLYGONQUERIES_H ? */
