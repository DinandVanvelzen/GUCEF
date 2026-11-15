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

#ifndef GUCEF_GEOMETRY_CTGEOMETRY3D_H
#define GUCEF_GEOMETRY_CTGEOMETRY3D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_CTQUICKHULL3D_H
#include "gucefGEOMETRY_CTQuickHull3D.h"
#define GUCEF_GEOMETRY_CTQUICKHULL3D_H
#endif /* GUCEF_GEOMETRY_CTQUICKHULL3D_H ? */

#ifndef GUCEF_GEOMETRY_TRIANGLEQUERIES_H
#include "gucefGEOMETRY_TriangleQueries.h"
#define GUCEF_GEOMETRY_TRIANGLEQUERIES_H
#endif /* GUCEF_GEOMETRY_TRIANGLEQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_SEGMENTQUERIES_H
#include "gucefGEOMETRY_SegmentQueries.h"
#define GUCEF_GEOMETRY_SEGMENTQUERIES_H
#endif /* GUCEF_GEOMETRY_SEGMENTQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_POLYLINESIMPLIFICATION_H
#include "gucefGEOMETRY_PolylineSimplification.h"
#define GUCEF_GEOMETRY_POLYLINESIMPLIFICATION_H
#endif /* GUCEF_GEOMETRY_POLYLINESIMPLIFICATION_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The 3D helpers in one place (section 7.9 of the design): convex hull, closest points and the triangle and segment
 *  helpers, as static entry points over the shapes and queries. The hull uses a temporary CTQuickHull3D (repeated work
 *  should keep one). Kinds: those of the underlying functions (the hull Float32 and Float64; the closest-point helpers
 *  the kinds of the shapes). The closest-point and triangle helpers are realtime-safe where their shapes are; the hull
 *  is not.
 */
template < typename T >
class CTGeometry3D
{
    public:

    typedef T TValue;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTDirectionVector3D< T > TDirection;
    typedef CTSegment3D< T > TSegment;
    typedef CTTriangle3D< T > TTriangle;

    /** The convex hull of 'count' points by quickhull (CTQuickHull3D); 'hull' receives vertices and, for status OK, faces */
    static TQuickHullStatus ConvexHull( const TPoint* points, UInt32 count, CTConvexHull3D< T >& hull )
    {
        CTQuickHull3D< T > engine;
        const TQuickHullStatus status = engine.Build( points, count );
        hull = engine.GetHull();
        return status;
    }

    /** The nearest point of the triangle (Voronoi regions) */
    static TPoint ClosestPointOnTriangle( const TTriangle& triangle, const TPoint& point ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return triangle.ClosestPoint( point ); }

    /** The nearest point of the segment */
    static TPoint ClosestPointOnSegment( const TSegment& segment, const TPoint& point ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return segment.ClosestPoint( point ); }

    /** The nearest points of two segments */
    static void ClosestPoints( const TSegment& a, const TSegment& b, TPoint& onA, TPoint& onB ) GUCEF_GEOMETRY_REALTIME_CONTEXT { GEOMETRY::ClosestPoints( a, b, onA, onB ); }

    /** The nearest points of a segment and a triangle */
    static void ClosestPoints( const TSegment& segment, const TTriangle& triangle, TPoint& onSegment, TPoint& onTriangle ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        GEOMETRY::ClosestPoints( segment, triangle, onSegment, onTriangle );
    }

    /** The nearest points of two triangles */
    static void ClosestPoints( const TTriangle& a, const TTriangle& b, TPoint& onA, TPoint& onB ) GUCEF_GEOMETRY_REALTIME_CONTEXT { GEOMETRY::ClosestPoints( a, b, onA, onB ); }

    /** The area of a triangle */
    static T TriangleArea( const TTriangle& triangle ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return triangle.GetArea(); }

    /** The unit normal ( b - a ) x ( c - a ) of a triangle; false for a degenerate triangle */
    static bool TryGetTriangleNormal( const TTriangle& triangle, TDirection& normal ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return triangle.TryGetNormal( normal ); }

    /** The barycentric coordinates of a point (projected onto the plane of the triangle); false for a degenerate triangle */
    static bool TryGetBarycentric( const TTriangle& triangle, const TPoint& point, TVector& weights ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return triangle.TryGetBarycentric( point, weights ); }

    /** True when the segment and the triangle meet */
    static bool SegmentIntersectsTriangle( const TSegment& segment, const TTriangle& triangle ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return Intersects( segment, triangle ); }

    /** True when the two segments meet */
    static bool SegmentsIntersect( const TSegment& a, const TSegment& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return Intersects( a, b ); }

    /** The parameter where the segment crosses the plane; false when it does not */
    static bool IntersectSegmentWithPlane( const TSegment& segment, const CTPlane< T >& plane, T& t ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return IntersectSegmentPlane( segment, plane, t ); }

    /** Douglas-Peucker on a 3D polyline (PolylineSimplification) */
    static void Simplify( const CTPolyline3D< T >& in, const T& epsilon, CTPolyline3D< T >& out ) { SimplifyPolyline( in, epsilon, out ); }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTGeometry3D< Float32 > TFloat32Geometry3D;
typedef CTGeometry3D< Float64 > TFloat64Geometry3D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTGEOMETRY3D_H ? */
