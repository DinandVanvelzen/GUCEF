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

#ifndef GUCEF_GEOMETRY_CQUICKHULL3DCORE_H
#define GUCEF_GEOMETRY_CQUICKHULL3DCORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

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

/** The outcome of a quickhull build */
enum EQuickHullStatus
{
    QUICKHULL_OK            = 0 ,   /**< a convex polyhedron: vertices and polygon faces */
    QUICKHULL_EMPTY         = 1 ,   /**< no points */
    QUICKHULL_POINT         = 2 ,   /**< all points coincide (within the tolerance): one vertex */
    QUICKHULL_LINE          = 3 ,   /**< all points collinear: the two end points */
    QUICKHULL_PLANE         = 4 ,   /**< all points coplanar: the vertices of their 2D hull, no faces */
    QUICKHULL_NOT_FINITE    = 5 ,   /**< an input coordinate is not finite: nothing is built */
    QUICKHULL_FAILED        = 6     /**< the build did not converge (inconsistent topology): nothing is built */
};
typedef enum EQuickHullStatus TQuickHullStatus;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The Float64 engine of CTQuickHull3D (section 7.9 of the design): quickhull of Barber, Dobkin and Huhdanpaa with the
 *  half edge structure and the facet merging of John E. Lloyd's implementation. Points live in the vertex array,
 *  faces and half edges in pooled arrays addressed by index (no pointers), the conflict (outside) sets in two doubly
 *  linked lists threaded through the vertex array. A face is merged with a neighbour when the neighbour's centroid is
 *  not below its plane by more than the tolerance (coplanar and nearly coplanar facets become one polygon); points that
 *  are within the tolerance of every face are inside. The tolerance is 'toleranceFactor' * DBL_EPSILON times the sum of
 *  the largest absolute coordinates per axis.
 *
 *  - Output: the hull vertices (indices into the input), faces as polygons counter-clockwise seen from outside, vertices
 *    that lie in the middle of an edge removed, so the polyhedron has V - E + F = 2.
 *  - Work arrays are members and reused by the next Build (no allocation after the first build of a similar size).
 *  - Not realtime-safe; one thread at a time.
 */
class GUCEF_GEOMETRY_PUBLIC_CPP CQuickHull3DCore
{
    public:

    CQuickHull3DCore( void );
    ~CQuickHull3DCore();

    /** Builds the hull of 'count' points given as x, y, z triples; 'toleranceFactor' defaults to 3 */
    TQuickHullStatus Build( const Float64* xyz, UInt32 count, Float64 toleranceFactor = 3.0 );

    /** The tolerance of the last build */
    Float64 GetTolerance( void ) const;

    /** The hull vertices as indices of the input points (for the planar status the 2D hull, for a line its two ends) */
    const GUCEF::vector< UInt32 >& GetVertexIndices( void ) const;

    /** The faces of a QUICKHULL_OK hull: the vertex lists one after the other, as positions in GetVertexIndices() */
    const GUCEF::vector< UInt32 >& GetFaceIndices( void ) const;

    /** The number of vertices of each face */
    const GUCEF::vector< UInt32 >& GetFaceSizes( void ) const;

    private:

    struct SImpl;
    SImpl* m_impl;

    CQuickHull3DCore( const CQuickHull3DCore& src );            /**< not implemented */
    CQuickHull3DCore& operator=( const CQuickHull3DCore& src ); /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CQUICKHULL3DCORE_H ? */
