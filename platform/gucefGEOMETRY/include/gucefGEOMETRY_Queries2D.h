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

#ifndef GUCEF_GEOMETRY_QUERIES2D_H
#define GUCEF_GEOMETRY_QUERIES2D_H

/*
 *  The 2D cells of the query matrix (section 7.5.6 of the design): segment, circle, rectangle, oriented rectangle,
 *  triangle and polygon against each other, in the plane of their space. See gucefGEOMETRY_PointQueries.h for the
 *  conventions of the facade (the 2D shapes use the same Distance, ClosestPoints and Intersects, with 2D points).
 *
 *  - Segment against segment: IntersectSegments tells the kind of the intersection (none, a point, or a collinear
 *    overlap with its two end points); Intersects is the orientation sign test of the segment (touching and collinear
 *    overlap included). The collinear and parallel decisions of IntersectSegments use the tolerance; exact predicates
 *    are the business of the predicates package.
 *  - Circle against a shape: through the nearest point of the shape to the centre, exact.
 *  - Convex shapes against each other (rectangle, oriented rectangle, triangle, convex polygon, and a segment as a
 *    degenerate one): the separating axis test over the edge normals; the distance is the smallest distance of a
 *    vertex of one to the other when they do not overlap (two disjoint convex polygons always have the nearest points
 *    at a vertex of one of them). For overlapping shapes both nearest points are a point that lies in both.
 *  - A polygon that is not convex is handled by the general test: the shapes overlap when two edges cross or one
 *    contains a vertex of the other, O(n m). Clipping, booleans and offsetting of general polygons are those of the
 *    polygon package (section 7.9).
 *  - An empty rectangle and a polygon without points have no points: they intersect nothing and have no nearest
 *    points.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_POINTQUERIES_H
#include "gucefGEOMETRY_PointQueries.h"
#define GUCEF_GEOMETRY_POINTQUERIES_H
#endif /* GUCEF_GEOMETRY_POINTQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_SHAPES_H
#include "gucefGEOMETRY_shapes.h"
#define GUCEF_GEOMETRY_SHAPES_H
#endif /* GUCEF_GEOMETRY_SHAPES_H ? */

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

/** The kind of intersection of two segments */
enum ESegmentIntersection2D
{
    SEGMENTINTERSECTION_NONE = 0 ,      /**< the segments share no point */
    SEGMENTINTERSECTION_POINT    ,      /**< the segments share one point (crossing, or an end point touching) */
    SEGMENTINTERSECTION_OVERLAP         /**< the segments are collinear and share a stretch of positive length */
};
typedef enum ESegmentIntersection2D TSegmentIntersection2D;

/** The intersection of two segments: for a point both points are equal, for an overlap they are the ends of the shared stretch */
template < typename T >
struct SSegmentIntersection2D
{
    MATH::CTPoint2D< T > first;
    MATH::CTPoint2D< T > second;
    T t;                                /**< the parameter of 'first' along the first segment, in [ 0, 1 ] */
    T u;                                /**< the parameter of 'first' along the second segment, in [ 0, 1 ] */

    SSegmentIntersection2D( void ) : first(), second(), t( MATH::CTNumericTraits< T >::Zero() ), u( MATH::CTNumericTraits< T >::Zero() ) {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The point against a 2D shape: true when the shape contains the point */
template < typename T, typename TShape >
GUCEF_GEOMETRY_FORCE_INLINE bool
Intersects( const MATH::CTPoint2D< T >& point, const TShape& shape ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return shape.Contains( point );
}

template < typename TShape, typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
Intersects( const TShape& shape, const MATH::CTPoint2D< T >& point ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return shape.Contains( point );
}

/** The distance from the point to the solid 2D shape, 0 inside */
template < typename T, typename TShape >
GUCEF_GEOMETRY_FORCE_INLINE T
Distance( const MATH::CTPoint2D< T >& point, const TShape& shape ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return shape.GetDistance( point );
}

template < typename TShape, typename T >
GUCEF_GEOMETRY_FORCE_INLINE T
Distance( const TShape& shape, const MATH::CTPoint2D< T >& point ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return shape.GetDistance( point );
}

/** The nearest point of the 2D shape to the point; 'onPoint' is the point itself */
template < typename T, typename TShape >
GUCEF_GEOMETRY_FORCE_INLINE bool
ClosestPoints( const MATH::CTPoint2D< T >& point, const TShape& shape, MATH::CTPoint2D< T >& onPoint, MATH::CTPoint2D< T >& onShape ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    onPoint = point;
    onShape = shape.ClosestPoint( point );
    return true;
}

template < typename TShape, typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
ClosestPoints( const TShape& shape, const MATH::CTPoint2D< T >& point, MATH::CTPoint2D< T >& onShape, MATH::CTPoint2D< T >& onPoint ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    onPoint = point;
    onShape = shape.ClosestPoint( point );
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  The intersection of two segments: crossing and touching give a point, collinear segments with a shared stretch give
 *  an overlap (a shared end point alone is a point). Segments are parallel when the sine of their angle is within the
 *  angular tolerance, and collinear when in addition the distance between their lines is within the length tolerance
 *  for the magnitude of the coordinates. A zero-length segment is a point.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE TSegmentIntersection2D
IntersectSegments( const CTSegment2D< T >& a, const CTSegment2D< T >& b, SSegmentIntersection2D< T >& result, const CTGeometryTolerance< T >& tolerance ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTVector2D< T > TVector;
    typedef MATH::CTPoint2D< T > TPoint;
    const T zero = TTraits::Zero();
    const T one = TTraits::One();
    const TVector r = a.end - a.start;
    const TVector s = b.end - b.start;
    const TVector qp = b.start - a.start;
    const T rLength = r.GetLength();
    const T sLength = s.GetLength();
    const T scale = TTraits::Max( TTraits::Max( TTraits::Abs( a.start.x ), TTraits::Abs( a.start.y ) ), TTraits::Max( TTraits::Max( TTraits::Abs( a.end.x ), TTraits::Abs( a.end.y ) ),
                    TTraits::Max( TTraits::Max( TTraits::Abs( b.start.x ), TTraits::Abs( b.start.y ) ), TTraits::Max( TTraits::Abs( b.end.x ), TTraits::Abs( b.end.y ) ) ) ) );
    const T lengthTolerance = tolerance.GetLengthTolerance( scale );

    SSegmentIntersection2D< T > found;
    const T rxs = r.Cross( s );

    if ( zero < rLength && zero < sLength && tolerance.angular * ( rLength * sLength ) < TTraits::Abs( rxs ) )
    {
        /* not parallel: the lines cross once, the parameters tell whether within both segments */
        const T t = qp.Cross( s ) / rxs;
        const T u = qp.Cross( r ) / rxs;
        const T tMargin = lengthTolerance / rLength;
        const T uMargin = lengthTolerance / sLength;
        if ( t < MATH::ComponentNegate( tMargin ) || one + tMargin < t || u < MATH::ComponentNegate( uMargin ) || one + uMargin < u )
            return SEGMENTINTERSECTION_NONE;
        found.t = TTraits::Max( zero, TTraits::Min( one, t ) );
        found.u = TTraits::Max( zero, TTraits::Min( one, u ) );
        found.first = a.GetPoint( found.t );
        found.second = found.first;
        result = found;
        return SEGMENTINTERSECTION_POINT;
    }

    /* parallel, or one of them is a point: the distance between the lines decides */
    if ( !( zero < rLength ) && !( zero < sLength ) )
    {
        if ( ( b.start - a.start ).GetLength() > lengthTolerance )
            return SEGMENTINTERSECTION_NONE;
        found.first = a.start;
        found.second = a.start;
        result = found;
        return SEGMENTINTERSECTION_POINT;
    }
    if ( !( zero < rLength ) || !( zero < sLength ) )
    {
        /* a point against a segment */
        const bool aIsPoint = !( zero < rLength );
        const CTSegment2D< T >& segment = aIsPoint ? b : a;
        const TPoint& point = aIsPoint ? a.start : b.start;
        if ( segment.GetDistance( point ) > lengthTolerance )
            return SEGMENTINTERSECTION_NONE;
        found.first = point;
        found.second = point;
        found.t = aIsPoint ? zero : segment.GetClosestParameter( point );
        found.u = aIsPoint ? segment.GetClosestParameter( point ) : zero;
        result = found;
        return SEGMENTINTERSECTION_POINT;
    }
    if ( lengthTolerance * rLength < TTraits::Abs( qp.Cross( r ) ) )
        return SEGMENTINTERSECTION_NONE;                    /* parallel and apart */

    /* collinear: the interval of the second segment along the first */
    const T rr = r.Dot( r );
    T t0 = qp.Dot( r ) / rr;
    T t1 = t0 + s.Dot( r ) / rr;
    const bool reversed = t1 < t0;
    const T low = reversed ? t1 : t0;
    const T high = reversed ? t0 : t1;
    const T tMargin = lengthTolerance / rLength;
    const T from = TTraits::Max( zero, low );
    const T to = TTraits::Min( one, high );
    if ( to + tMargin < from )
        return SEGMENTINTERSECTION_NONE;
    const T fromClamped = TTraits::Min( from, to );
    found.t = fromClamped;
    found.first = a.GetPoint( fromClamped );
    found.second = a.GetPoint( to );
    found.u = b.GetClosestParameter( found.first );
    result = found;
    return ( to - from ) * rLength > lengthTolerance ? SEGMENTINTERSECTION_OVERLAP : SEGMENTINTERSECTION_POINT;
}

template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE TSegmentIntersection2D
IntersectSegments( const CTSegment2D< T >& a, const CTSegment2D< T >& b, SSegmentIntersection2D< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return IntersectSegments( a, b, result, CTGeometryTolerance< T >() );
}

/*-------------------------------------------------------------------------*/

/** The nearest points of two segments (Ericson 5.1.9 in the plane), also for zero-length and parallel segments */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClosestPoints( const CTSegment2D< T >& a, const CTSegment2D< T >& b, MATH::CTPoint2D< T >& onA, MATH::CTPoint2D< T >& onB ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTVector2D< T > TVector;
    const T zero = TTraits::Zero();
    const T one = TTraits::One();
    const TVector d1 = a.end - a.start;
    const TVector d2 = b.end - b.start;
    const TVector r = a.start - b.start;
    const T aa = d1.Dot( d1 );
    const T e = d2.Dot( d2 );
    const T f = d2.Dot( r );
    T s = zero;
    T t = zero;
    if ( !( zero < aa ) && !( zero < e ) )
    {
        /* both are points */
    }
    else if ( !( zero < aa ) )
    {
        t = f / e;
        t = t < zero ? zero : ( one < t ? one : t );
    }
    else
    {
        const T c = d1.Dot( r );
        if ( !( zero < e ) )
        {
            s = MATH::ComponentNegate( c ) / aa;
            s = s < zero ? zero : ( one < s ? one : s );
        }
        else
        {
            const T bb = d1.Dot( d2 );
            const T denominator = aa * e - bb * bb;
            if ( zero < denominator )
            {
                s = ( bb * f - c * e ) / denominator;
                s = s < zero ? zero : ( one < s ? one : s );
            }
            t = ( bb * s + f ) / e;
            if ( t < zero )
            {
                t = zero;
                s = MATH::ComponentNegate( c ) / aa;
                s = s < zero ? zero : ( one < s ? one : s );
            }
            else if ( one < t )
            {
                t = one;
                s = ( bb - c ) / aa;
                s = s < zero ? zero : ( one < s ? one : s );
            }
        }
    }
    onA = a.start + d1 * s;
    onB = b.start + d2 * t;
    return true;
}

/** True when the closed segments share a point (the orientation sign test of the segment) */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
Intersects( const CTSegment2D< T >& a, const CTSegment2D< T >& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return a.Intersects( b );
}

/*-------------------------------------------------------------------------*/

/**
 *  The nearest points of two shapes that are a core inflated by a radius, in the plane (circles, and with radius 0 a
 *  shape whose nearest point is given): as the 3D ClosestPointsOfRoundCores. The result is the distance, 0 when they
 *  touch or overlap, then both outputs are the same point that lies in both shapes.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
ClosestPointsOfRoundCores2D( const MATH::CTPoint2D< T >& coreA, const T& radiusA, const MATH::CTPoint2D< T >& coreB, const T& radiusB,
                             MATH::CTPoint2D< T >& onA, MATH::CTPoint2D< T >& onB ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTVector2D< T > TVector;
    const TVector between = coreB - coreA;
    const T distance = between.GetLength();
    const T gap = distance - ( radiusA + radiusB );
    if ( TTraits::Zero() < gap )
    {
        const TVector direction = between / distance;
        onA = coreA + direction * radiusA;
        onB = coreB - direction * radiusB;
        return gap;
    }
    MATH::CTPoint2D< T > witness = coreA;
    if ( !( TTraits::Zero() < radiusB ) )
        witness = coreB;
    else if ( TTraits::Zero() < radiusA && TTraits::Zero() < distance )
    {
        /* along the line of the cores the first shape covers [ -rA, rA ] and the second [ d - rB, d + rB ]: the middle of the overlap */
        const T low = TTraits::Max( MATH::ComponentNegate( radiusA ), distance - radiusB );
        const T high = TTraits::Min( radiusA, distance + radiusB );
        witness = coreA + ( between / distance ) * ( ( low + high ) * TTraits::Half() );
    }
    onA = witness;
    onB = witness;
    return TTraits::Zero();
}

/*-------------------------------------------------------------------------*/

/** The nearest points of a segment and a circle; false for a negative radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClosestPoints( const CTSegment2D< T >& segment, const CTCircle< T >& circle, MATH::CTPoint2D< T >& onSegment, MATH::CTPoint2D< T >& onCircle ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( TTraits::IsNegative( circle.radius ) )
        return false;
    ClosestPointsOfRoundCores2D( segment.ClosestPoint( circle.centre ), TTraits::Zero(), circle.centre, circle.radius, onSegment, onCircle );
    return true;
}

/** The nearest points of two circles; false for a negative radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
ClosestPoints( const CTCircle< T >& a, const CTCircle< T >& b, MATH::CTPoint2D< T >& onA, MATH::CTPoint2D< T >& onB ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( TTraits::IsNegative( a.radius ) || TTraits::IsNegative( b.radius ) )
        return false;
    ClosestPointsOfRoundCores2D( a.centre, a.radius, b.centre, b.radius, onA, onB );
    return true;
}

/** True when the segment shares a point with the circle (a solid disc); false for a negative radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTSegment2D< T >& segment, const CTCircle< T >& circle ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( MATH::CTNumericTraits< T >::IsNegative( circle.radius ) )
        return false;
    return segment.GetSquaredDistance( circle.centre ) <= circle.radius * circle.radius;
}

/** True when the discs share a point; false for a negative radius */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
Intersects( const CTCircle< T >& a, const CTCircle< T >& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( MATH::CTNumericTraits< T >::IsNegative( a.radius ) || MATH::CTNumericTraits< T >::IsNegative( b.radius ) )
        return false;
    const T reach = a.radius + b.radius;
    return ( b.centre - a.centre ).GetSquaredLength() <= reach * reach;
}

GUCEF_GEOMETRY_SWAPPED_INTERSECTS( CTSegment2D, CTCircle )

/** The nearest points of a circle and a segment (the other order of the arguments) */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
ClosestPoints( const CTCircle< T >& circle, const CTSegment2D< T >& segment, MATH::CTPoint2D< T >& onCircle, MATH::CTPoint2D< T >& onSegment ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ClosestPoints( segment, circle, onSegment, onCircle );
}

/*-------------------------------------------------------------------------*/

/**
 *  The vertices of a convex shape (or of a polygon) in order, as the ring that the polygon cells work on: a segment is
 *  a ring of two points, a rectangle, an oriented rectangle a ring of four, a triangle a ring of three, and a
 *  polygon is its own ring. 'points' is set to 'storage' or to the polygon, 'count' is 0 for a shape without points
 *  (an empty rectangle), 'convex' says whether the separating axis test applies.
 */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE void
FillRing2D( const CTSegment2D< T >& segment, MATH::CTPoint2D< T > ( &storage )[ 4 ], const MATH::CTPoint2D< T >*& points, UInt32& count, bool& convex ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    storage[ 0 ] = segment.start;
    storage[ 1 ] = segment.end;
    points = storage;
    count = 2;
    convex = true;
}

template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE void
FillRing2D( const CTTriangle2D< T >& triangle, MATH::CTPoint2D< T > ( &storage )[ 4 ], const MATH::CTPoint2D< T >*& points, UInt32& count, bool& convex ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    storage[ 0 ] = triangle.a;
    storage[ 1 ] = triangle.b;
    storage[ 2 ] = triangle.c;
    points = storage;
    count = 3;
    convex = true;
}

template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE void
FillRing2D( const CTRectangle< T >& rectangle, MATH::CTPoint2D< T > ( &storage )[ 4 ], const MATH::CTPoint2D< T >*& points, UInt32& count, bool& convex ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    points = storage;
    convex = true;
    if ( rectangle.IsEmpty() )
    {
        count = 0;
        return;
    }
    storage[ 0 ] = rectangle.GetCorner( 0 );
    storage[ 1 ] = rectangle.GetCorner( 1 );
    storage[ 2 ] = rectangle.GetCorner( 3 );
    storage[ 3 ] = rectangle.GetCorner( 2 );
    count = 4;
}

template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE void
FillRing2D( const CTOrientedRectangle< T >& rectangle, MATH::CTPoint2D< T > ( &storage )[ 4 ], const MATH::CTPoint2D< T >*& points, UInt32& count, bool& convex ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    points = storage;
    convex = true;
    if ( MATH::CTNumericTraits< T >::IsNegative( rectangle.halfExtents.x ) || MATH::CTNumericTraits< T >::IsNegative( rectangle.halfExtents.y ) )
    {
        count = 0;
        return;
    }
    rectangle.GetCorners( storage );
    count = 4;
}

template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE void
FillRing2D( const CTPolygon2D< T >& polygon, MATH::CTPoint2D< T > ( &/*storage*/ )[ 4 ], const MATH::CTPoint2D< T >*& points, UInt32& count, bool& convex ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    count = polygon.GetPointCount();
    points = count > 0 ? &polygon.points[ 0 ] : GUCEF_NULL;
    convex = count < 4 || polygon.IsConvex();
}

/*-------------------------------------------------------------------------*/

/**
 *  True when two rings (see FillRing2D) share a point: the separating axis test over the edge normals when both are
 *  convex, otherwise the general test (two edges cross, or a vertex of one is inside the other). Touching counts.
 *  A ring without points intersects nothing.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
RingsOverlap2D( const MATH::CTPoint2D< T >* a, UInt32 countA, bool convexA, const MATH::CTPoint2D< T >* b, UInt32 countB, bool convexB ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint2D< T > TPoint;
    if ( 0 == countA || 0 == countB )
        return false;

    if ( convexA && convexB && countA >= 2 && countB >= 2 )
    {
        for ( UInt32 side=0; side<2; ++side )
        {
            const TPoint* edges = 0 == side ? a : b;
            const UInt32 edgeCount = 0 == side ? countA : countB;
            for ( UInt32 i=0; i<edgeCount; ++i )
            {
                const MATH::CTVector2D< T > edge = edges[ ( i + 1 ) % edgeCount ] - edges[ i ];
                const MATH::CTVector2D< T > axis( edge.y, MATH::ComponentNegate( edge.x ) );
                T minA = TTraits::Max();
                T maxA = TTraits::Lowest();
                T minB = TTraits::Max();
                T maxB = TTraits::Lowest();
                for ( UInt32 j=0; j<countA; ++j )
                {
                    const T d = a[ j ].ToPositionVector().Dot( axis );
                    minA = TTraits::Min( minA, d );
                    maxA = TTraits::Max( maxA, d );
                }
                for ( UInt32 j=0; j<countB; ++j )
                {
                    const T d = b[ j ].ToPositionVector().Dot( axis );
                    minB = TTraits::Min( minB, d );
                    maxB = TTraits::Max( maxB, d );
                }
                if ( maxA < minB || maxB < minA )
                    return false;
            }
        }
        return true;
    }

    /* the general test: any pair of edges crossing, or a vertex of one ring inside the other */
    for ( UInt32 i=0; i<countA; ++i )
    {
        const CTSegment2D< T > edgeA( a[ i ], a[ ( i + 1 ) % countA ] );
        for ( UInt32 j=0; j<countB; ++j )
        {
            if ( edgeA.Intersects( CTSegment2D< T >( b[ j ], b[ ( j + 1 ) % countB ] ) ) )
                return true;
        }
    }
    for ( UInt32 side=0; side<2; ++side )
    {
        const TPoint* points = 0 == side ? a : b;
        const UInt32 pointCount = 0 == side ? countA : countB;
        const TPoint* ring = 0 == side ? b : a;
        const UInt32 ringCount = 0 == side ? countB : countA;
        if ( ringCount < 3 )
            continue;
        /* crossing number of a ray towards +X */
        for ( UInt32 i=0; i<pointCount; ++i )
        {
            bool inside = false;
            for ( UInt32 j=0, k=ringCount-1; j<ringCount; k=j++ )
            {
                if ( ( ring[ j ].y > points[ i ].y ) != ( ring[ k ].y > points[ i ].y ) &&
                     points[ i ].x < ( ring[ k ].x - ring[ j ].x ) * ( points[ i ].y - ring[ j ].y ) / ( ring[ k ].y - ring[ j ].y ) + ring[ j ].x )
                    inside = !inside;
            }
            if ( inside )
                return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/**
 *  The nearest points of two rings (see FillRing2D): for rings that do not overlap the smallest distance of a vertex
 *  of one ring to an edge of the other; for rings that overlap one point that lies in both (a vertex of one inside the
 *  other, else the crossing of two edges). False for a ring without points.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
RingsClosestPoints2D( const MATH::CTPoint2D< T >* a, UInt32 countA, bool convexA, const MATH::CTPoint2D< T >* b, UInt32 countB, bool convexB,
                      MATH::CTPoint2D< T >& onA, MATH::CTPoint2D< T >& onB ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint2D< T > TPoint;
    if ( 0 == countA || 0 == countB )
        return false;

    if ( RingsOverlap2D( a, countA, convexA, b, countB, convexB ) )
    {
        /* a witness: a vertex of one ring that lies in the other (at distance 0 from it), else a crossing of two edges */
        for ( UInt32 side=0; side<2; ++side )
        {
            const TPoint* points = 0 == side ? a : b;
            const UInt32 pointCount = 0 == side ? countA : countB;
            const TPoint* ring = 0 == side ? b : a;
            const UInt32 ringCount = 0 == side ? countB : countA;
            for ( UInt32 i=0; i<pointCount; ++i )
            {
                if ( RingsOverlap2D( &points[ i ], 1, true, ring, ringCount, 0 == side ? convexB : convexA ) )
                {
                    onA = points[ i ];
                    onB = points[ i ];
                    return true;
                }
            }
        }
        for ( UInt32 i=0; i<countA; ++i )
        {
            const CTSegment2D< T > edgeA( a[ i ], a[ ( i + 1 ) % countA ] );
            for ( UInt32 j=0; j<countB; ++j )
            {
                SSegmentIntersection2D< T > crossing;
                if ( SEGMENTINTERSECTION_NONE != IntersectSegments( edgeA, CTSegment2D< T >( b[ j ], b[ ( j + 1 ) % countB ] ), crossing ) )
                {
                    onA = crossing.first;
                    onB = crossing.first;
                    return true;
                }
            }
        }
        onA = a[ 0 ];
        onB = a[ 0 ];
        return true;
    }

    T best = TTraits::Max();
    for ( UInt32 side=0; side<2; ++side )
    {
        const TPoint* points = 0 == side ? a : b;
        const UInt32 pointCount = 0 == side ? countA : countB;
        const TPoint* ring = 0 == side ? b : a;
        const UInt32 ringCount = 0 == side ? countB : countA;
        for ( UInt32 i=0; i<pointCount; ++i )
        {
            for ( UInt32 j=0; j<ringCount; ++j )
            {
                const CTSegment2D< T > edge( ring[ j ], ring[ ( j + 1 ) % ringCount ] );
                const TPoint onEdge = edge.ClosestPoint( points[ i ] );
                const T d = ( onEdge - points[ i ] ).GetSquaredLength();
                if ( d < best )
                {
                    best = d;
                    onA = 0 == side ? points[ i ] : onEdge;
                    onB = 0 == side ? onEdge : points[ i ];
                }
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Defines Intersects and ClosestPoints for the pair of shape classes (TA, TB) through their rings, and the other order
 *  of the arguments; TA and TB are different class templates of one parameter, from segment, rectangle, oriented
 *  rectangle, triangle and polygon.
 */
#define GUCEF_GEOMETRY_RING_PAIR_QUERIES( TA, TB )                                                                        \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_FORCE_INLINE bool                                                                                      \
    Intersects( const TA< T >& a, const TB< T >& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT                                      \
    {                                                                                                                     \
        MATH::CTPoint2D< T > storageA[ 4 ];                                                                               \
        MATH::CTPoint2D< T > storageB[ 4 ];                                                                               \
        const MATH::CTPoint2D< T >* pointsA = GUCEF_NULL;                                                                 \
        const MATH::CTPoint2D< T >* pointsB = GUCEF_NULL;                                                                 \
        UInt32 countA = 0;                                                                                                \
        UInt32 countB = 0;                                                                                                \
        bool convexA = true;                                                                                              \
        bool convexB = true;                                                                                              \
        FillRing2D( a, storageA, pointsA, countA, convexA );                                                              \
        FillRing2D( b, storageB, pointsB, countB, convexB );                                                              \
        return RingsOverlap2D( pointsA, countA, convexA, pointsB, countB, convexB );                                      \
    }                                                                                                                     \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_FORCE_INLINE bool                                                                                      \
    ClosestPoints( const TA< T >& a, const TB< T >& b, MATH::CTPoint2D< T >& onA, MATH::CTPoint2D< T >& onB ) GUCEF_GEOMETRY_REALTIME_CONTEXT \
    {                                                                                                                     \
        MATH::CTPoint2D< T > storageA[ 4 ];                                                                               \
        MATH::CTPoint2D< T > storageB[ 4 ];                                                                               \
        const MATH::CTPoint2D< T >* pointsA = GUCEF_NULL;                                                                 \
        const MATH::CTPoint2D< T >* pointsB = GUCEF_NULL;                                                                 \
        UInt32 countA = 0;                                                                                                \
        UInt32 countB = 0;                                                                                                \
        bool convexA = true;                                                                                              \
        bool convexB = true;                                                                                              \
        FillRing2D( a, storageA, pointsA, countA, convexA );                                                              \
        FillRing2D( b, storageB, pointsB, countB, convexB );                                                              \
        return RingsClosestPoints2D( pointsA, countA, convexA, pointsB, countB, convexB, onA, onB );                      \
    }

#define GUCEF_GEOMETRY_RING_PAIR_QUERIES_SWAPPED( TA, TB )                                                                \
    GUCEF_GEOMETRY_RING_PAIR_QUERIES( TA, TB )                                                                            \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_FORCE_INLINE bool                                                                                      \
    Intersects( const TB< T >& b, const TA< T >& a ) GUCEF_GEOMETRY_REALTIME_CONTEXT                                      \
    {                                                                                                                     \
        return Intersects( a, b );                                                                                        \
    }                                                                                                                     \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_FORCE_INLINE bool                                                                                      \
    ClosestPoints( const TB< T >& b, const TA< T >& a, MATH::CTPoint2D< T >& onB, MATH::CTPoint2D< T >& onA ) GUCEF_GEOMETRY_REALTIME_CONTEXT \
    {                                                                                                                     \
        return ClosestPoints( a, b, onA, onB );                                                                           \
    }

GUCEF_GEOMETRY_RING_PAIR_QUERIES_SWAPPED( CTSegment2D, CTRectangle )
GUCEF_GEOMETRY_RING_PAIR_QUERIES_SWAPPED( CTSegment2D, CTOrientedRectangle )
GUCEF_GEOMETRY_RING_PAIR_QUERIES_SWAPPED( CTSegment2D, CTTriangle2D )
GUCEF_GEOMETRY_RING_PAIR_QUERIES_SWAPPED( CTSegment2D, CTPolygon2D )
GUCEF_GEOMETRY_RING_PAIR_QUERIES( CTRectangle, CTRectangle )
GUCEF_GEOMETRY_RING_PAIR_QUERIES_SWAPPED( CTRectangle, CTOrientedRectangle )
GUCEF_GEOMETRY_RING_PAIR_QUERIES_SWAPPED( CTRectangle, CTTriangle2D )
GUCEF_GEOMETRY_RING_PAIR_QUERIES_SWAPPED( CTRectangle, CTPolygon2D )
GUCEF_GEOMETRY_RING_PAIR_QUERIES( CTOrientedRectangle, CTOrientedRectangle )
GUCEF_GEOMETRY_RING_PAIR_QUERIES_SWAPPED( CTOrientedRectangle, CTTriangle2D )
GUCEF_GEOMETRY_RING_PAIR_QUERIES_SWAPPED( CTOrientedRectangle, CTPolygon2D )
GUCEF_GEOMETRY_RING_PAIR_QUERIES( CTTriangle2D, CTTriangle2D )
GUCEF_GEOMETRY_RING_PAIR_QUERIES_SWAPPED( CTTriangle2D, CTPolygon2D )
GUCEF_GEOMETRY_RING_PAIR_QUERIES( CTPolygon2D, CTPolygon2D )

/*-------------------------------------------------------------------------*/

/**
 *  Defines Intersects and ClosestPoints of a circle against a shape that has the 2D members ClosestPoint and
 *  GetDistance (rectangle, oriented rectangle, triangle, polygon): through the nearest point of the shape to the
 *  centre, in both orders of the arguments. A negative radius has no points.
 */
#define GUCEF_GEOMETRY_CIRCLE_PAIR_QUERIES( TShape )                                                                      \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_FORCE_INLINE bool                                                                                      \
    Intersects( const CTCircle< T >& circle, const TShape< T >& shape ) GUCEF_GEOMETRY_REALTIME_CONTEXT                   \
    {                                                                                                                     \
        if ( MATH::CTNumericTraits< T >::IsNegative( circle.radius ) )                                                    \
            return false;                                                                                                 \
        return shape.GetDistance( circle.centre ) <= circle.radius;                                                       \
    }                                                                                                                     \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_FORCE_INLINE bool                                                                                      \
    Intersects( const TShape< T >& shape, const CTCircle< T >& circle ) GUCEF_GEOMETRY_REALTIME_CONTEXT                   \
    {                                                                                                                     \
        return Intersects( circle, shape );                                                                               \
    }                                                                                                                     \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_PRECISE_INLINE bool                                                                                    \
    ClosestPoints( const CTCircle< T >& circle, const TShape< T >& shape, MATH::CTPoint2D< T >& onCircle, MATH::CTPoint2D< T >& onShape ) GUCEF_GEOMETRY_REALTIME_CONTEXT \
    {                                                                                                                     \
        if ( MATH::CTNumericTraits< T >::IsNegative( circle.radius ) )                                                    \
            return false;                                                                                                 \
        ClosestPointsOfRoundCores2D( circle.centre, circle.radius, shape.ClosestPoint( circle.centre ), MATH::CTNumericTraits< T >::Zero(), onCircle, onShape ); \
        return true;                                                                                                      \
    }                                                                                                                     \
    template < typename T >                                                                                               \
    GUCEF_GEOMETRY_FORCE_INLINE bool                                                                                      \
    ClosestPoints( const TShape< T >& shape, const CTCircle< T >& circle, MATH::CTPoint2D< T >& onShape, MATH::CTPoint2D< T >& onCircle ) GUCEF_GEOMETRY_REALTIME_CONTEXT \
    {                                                                                                                     \
        return ClosestPoints( circle, shape, onCircle, onShape );                                                         \
    }

GUCEF_GEOMETRY_CIRCLE_PAIR_QUERIES( CTRectangle )
GUCEF_GEOMETRY_CIRCLE_PAIR_QUERIES( CTOrientedRectangle )
GUCEF_GEOMETRY_CIRCLE_PAIR_QUERIES( CTTriangle2D )
GUCEF_GEOMETRY_CIRCLE_PAIR_QUERIES( CTPolygon2D )

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* For Float32 and Float64 the module's instantiations (gucefGEOMETRY_QueryInstantiations.cpp) are used, section 5.10 */

#define GUCEF_GEOMETRY_QUERIES2D_KIND( prefix, kind ) \
    prefix TSegmentIntersection2D IntersectSegments< kind >( const CTSegment2D< kind >&, const CTSegment2D< kind >&, SSegmentIntersection2D< kind >&, const CTGeometryTolerance< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ClosestPoints< kind >( const CTSegment2D< kind >&, const CTSegment2D< kind >&, MATH::CTPoint2D< kind >&, MATH::CTPoint2D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix kind ClosestPointsOfRoundCores2D< kind >( const MATH::CTPoint2D< kind >&, const kind&, const MATH::CTPoint2D< kind >&, const kind&, MATH::CTPoint2D< kind >&, MATH::CTPoint2D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ClosestPoints< kind >( const CTSegment2D< kind >&, const CTCircle< kind >&, MATH::CTPoint2D< kind >&, MATH::CTPoint2D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool ClosestPoints< kind >( const CTCircle< kind >&, const CTCircle< kind >&, MATH::CTPoint2D< kind >&, MATH::CTPoint2D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTSegment2D< kind >&, const CTCircle< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool Intersects< kind >( const CTCircle< kind >&, const CTCircle< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool RingsOverlap2D< kind >( const MATH::CTPoint2D< kind >*, UInt32, bool, const MATH::CTPoint2D< kind >*, UInt32, bool ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool RingsClosestPoints2D< kind >( const MATH::CTPoint2D< kind >*, UInt32, bool, const MATH::CTPoint2D< kind >*, UInt32, bool, MATH::CTPoint2D< kind >&, MATH::CTPoint2D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_QUERIES2D_PREDEFINED_KINDS( prefix ) \
    GUCEF_GEOMETRY_QUERIES2D_KIND( prefix, Float32 ) \
    GUCEF_GEOMETRY_QUERIES2D_KIND( prefix, Float64 )

#ifdef GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED
GUCEF_GEOMETRY_QUERIES2D_PREDEFINED_KINDS( GUCEF_GEOMETRY_EXTERN_TEMPLATE )
#endif /* GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_QUERIES2D_H ? */
