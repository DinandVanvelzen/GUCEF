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

#ifndef GUCEF_GEOMETRY_CTGEOMETRY2D_H
#define GUCEF_GEOMETRY_CTGEOMETRY2D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_CONVEXHULL2D_H
#include "gucefGEOMETRY_ConvexHull2D.h"
#define GUCEF_GEOMETRY_CONVEXHULL2D_H
#endif /* GUCEF_GEOMETRY_CONVEXHULL2D_H ? */

#ifndef GUCEF_GEOMETRY_POINTINPOLYGON_H
#include "gucefGEOMETRY_PointInPolygon.h"
#define GUCEF_GEOMETRY_POINTINPOLYGON_H
#endif /* GUCEF_GEOMETRY_POINTINPOLYGON_H ? */

#ifndef GUCEF_GEOMETRY_POLYLINESIMPLIFICATION_H
#include "gucefGEOMETRY_PolylineSimplification.h"
#define GUCEF_GEOMETRY_POLYLINESIMPLIFICATION_H
#endif /* GUCEF_GEOMETRY_POLYLINESIMPLIFICATION_H ? */

#ifndef GUCEF_GEOMETRY_POLYGONTRIANGULATION_H
#include "gucefGEOMETRY_PolygonTriangulation.h"
#define GUCEF_GEOMETRY_POLYGONTRIANGULATION_H
#endif /* GUCEF_GEOMETRY_POLYGONTRIANGULATION_H ? */

#ifndef GUCEF_GEOMETRY_CONVEXDECOMPOSITION2D_H
#include "gucefGEOMETRY_ConvexDecomposition2D.h"
#define GUCEF_GEOMETRY_CONVEXDECOMPOSITION2D_H
#endif /* GUCEF_GEOMETRY_CONVEXDECOMPOSITION2D_H ? */

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

#ifndef GUCEF_GEOMETRY_QUERIES2D_H
#include "gucefGEOMETRY_Queries2D.h"
#define GUCEF_GEOMETRY_QUERIES2D_H
#endif /* GUCEF_GEOMETRY_QUERIES2D_H ? */

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
 *  The 2D algorithms in one place (section 7.9 of the design): static entry points over the class based API for the
 *  common questions (hull, point in polygon, triangulation, Delaunay, convex pieces, simplification, booleans,
 *  offsetting, strokes, segment intersection). Each call uses a temporary engine and so allocates its work arrays;
 *  repeated work should keep the engine classes (CTPolygonTriangulator, CTConstrainedDelaunay2D, CTPolygonClipper,
 *  CTPolygonOffsetter) instead. The members exist for every kind their engine supports (booleans, offsets and strokes:
 *  Float32, Float64, Int32, Int64, TFixed32_32; the rest every kind of CTPredicates). Not realtime-safe.
 */
template < typename T >
class CTGeometry2D
{
    public:

    typedef T TValue;
    typedef MATH::CTPoint2D< T > TPoint;
    typedef GUCEF::vector< TPoint > TPointVector;
    typedef GUCEF::vector< UInt32 > TIndexVector;
    typedef CTPolygon2D< T > TPolygon;
    typedef CTPolygonWithHoles2D< T > TPolygonWithHoles;
    typedef GUCEF::vector< TPolygonWithHoles > TPolygonsWithHoles;

    /** The convex hull, counter-clockwise (ConvexHull2D); the number of hull points */
    static UInt32 ConvexHull( const TPoint* points, UInt32 count, TPointVector& hull, bool keepCollinear = false )
    {
        return ConvexHull2D( points, count, hull, keepCollinear );
    }

    /** Inside, outside or on the boundary of a polygon (exact winding number) */
    static TPointInPolygon ClassifyPoint( const TPolygon& polygon, const TPoint& point ) { return ClassifyPointInPolygon( polygon, point ); }

    static TPointInPolygon ClassifyPoint( const TPolygonWithHoles& polygon, const TPoint& point ) { return ClassifyPointInPolygon( polygon, point ); }

    /** True when the point is inside or on the boundary */
    static bool Contains( const TPolygonWithHoles& polygon, const TPoint& point ) { return POINTINPOLYGON_OUTSIDE != ClassifyPointInPolygon( polygon, point ); }

    /** Ear clipping: three point indices per counter-clockwise triangle (CTPolygonTriangulator) */
    static TTriangulationStatus Triangulate( const TPolygon& polygon, TIndexVector& triangles ) { return TriangulatePolygon( polygon, triangles ); }

    /** As Triangulate; the indices count the outer points first, then those of each hole */
    static TTriangulationStatus Triangulate( const TPolygonWithHoles& polygon, TIndexVector& triangles ) { return TriangulatePolygon( polygon, triangles ); }

    /**
     *  The (constrained) Delaunay triangulation of a point set: three point indices per triangle in canonical order;
     *  constraints as index pairs (CTConstrainedDelaunay2D). Crossing constraints create Steiner points, whose indices
     *  follow the input points: 'vertices' receives all vertices when given.
     */
    static TDelaunayStatus Delaunay( const TPoint* points, UInt32 count, TIndexVector& triangles, const UInt32* constraints = GUCEF_NULL,
                                     UInt32 constraintCount = 0, TPointVector* vertices = GUCEF_NULL )
    {
        CTConstrainedDelaunay2D< T > cdt;
        const TDelaunayStatus status = cdt.Build( points, count, constraints, constraintCount );
        triangles = cdt.GetTriangles();
        if ( GUCEF_NULL != vertices )
            *vertices = cdt.GetVertices();
        return status;
    }

    /** Convex pieces by Hertel-Mehlhorn (DecomposeIntoConvex) */
    static TTriangulationStatus DecomposeIntoConvex( const TPolygon& polygon, GUCEF::vector< TPolygon >& pieces ) { return GEOMETRY::DecomposeIntoConvex( polygon, pieces ); }

    static TTriangulationStatus DecomposeIntoConvex( const TPolygonWithHoles& polygon, GUCEF::vector< TPolygon >& pieces )
    {
        CTConvexDecomposition2DWorkspace< T > workspace;
        return GEOMETRY::DecomposeIntoConvex( polygon, pieces, workspace );
    }

    /** Douglas-Peucker on an open polyline and on a closed ring (PolylineSimplification) */
    static void Simplify( const CTPolyline2D< T >& in, const T& epsilon, CTPolyline2D< T >& out ) { SimplifyPolyline( in, epsilon, out ); }

    static void Simplify( const TPolygon& in, const T& epsilon, TPolygon& out ) { SimplifyPolygon( in, epsilon, out ); }

    /** A boolean operation on two polygons with holes (CTPolygonClipper, fill rule POSITIVE on normalised rings) */
    static TPolygonClipperStatus Boolean( const TPolygonWithHoles& a, const TPolygonWithHoles& b, TPolygonBooleanOperation operation, TPolygonsWithHoles& result )
    {
        return PolygonBoolean( a, b, operation, result );
    }

    static TPolygonClipperStatus Union( const TPolygonWithHoles& a, const TPolygonWithHoles& b, TPolygonsWithHoles& result ) { return PolygonBoolean( a, b, POLYGONBOOLEAN_UNION, result ); }

    static TPolygonClipperStatus Intersection( const TPolygonWithHoles& a, const TPolygonWithHoles& b, TPolygonsWithHoles& result ) { return PolygonBoolean( a, b, POLYGONBOOLEAN_INTERSECTION, result ); }

    static TPolygonClipperStatus Difference( const TPolygonWithHoles& a, const TPolygonWithHoles& b, TPolygonsWithHoles& result ) { return PolygonBoolean( a, b, POLYGONBOOLEAN_DIFFERENCE, result ); }

    static TPolygonClipperStatus Xor( const TPolygonWithHoles& a, const TPolygonWithHoles& b, TPolygonsWithHoles& result ) { return PolygonBoolean( a, b, POLYGONBOOLEAN_XOR, result ); }

    /** Offsets a polygon with holes (CTPolygonOffsetter) */
    static TPolygonClipperStatus Offset( const TPolygonWithHoles& polygon, const T& delta, TPolygonJoinType join, TPolygonsWithHoles& result )
    {
        return OffsetPolygon( polygon, delta, join, result );
    }

    /** Strokes a polyline at distance |halfWidth| (CTPolygonOffsetter) */
    static TPolygonClipperStatus Stroke( const CTPolyline2D< T >& polyline, const T& halfWidth, TPolygonJoinType join, TPolylineEndType ends, TPolygonsWithHoles& result )
    {
        return StrokePolyline( polyline, halfWidth, join, ends, result );
    }

    /** The intersection of two segments (Queries2D): none, a point or a collinear overlap */
    static TSegmentIntersection2D IntersectSegments( const CTSegment2D< T >& a, const CTSegment2D< T >& b, SSegmentIntersection2D< T >& result )
    {
        return GEOMETRY::IntersectSegments( a, b, result );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTGeometry2D< Float32 > TFloat32Geometry2D;
typedef CTGeometry2D< Float64 > TFloat64Geometry2D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTGEOMETRY2D_H ? */
