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

#ifndef GUCEF_GEOMETRY_CTPOLYGON2D_H
#define GUCEF_GEOMETRY_CTPOLYGON2D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_SSHAPETEXTREADER_H
#include "gucefGEOMETRY_SShapeTextReader.h"
#define GUCEF_GEOMETRY_SSHAPETEXTREADER_H
#endif /* GUCEF_GEOMETRY_SSHAPETEXTREADER_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#ifndef GUCEF_GEOMETRY_CTRECTANGLE_H
#include "gucefGEOMETRY_CTRectangle.h"
#define GUCEF_GEOMETRY_CTRECTANGLE_H
#endif /* GUCEF_GEOMETRY_CTRECTANGLE_H ? */

#ifndef GUCEF_GEOMETRY_CTSEGMENT2D_H
#include "gucefGEOMETRY_CTSegment2D.h"
#define GUCEF_GEOMETRY_CTSEGMENT2D_H
#endif /* GUCEF_GEOMETRY_CTSEGMENT2D_H ? */

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
 *  One closed ring of points in 2D (the XY plane of its space); the last point connects to the first, the closing
 *  point is not repeated (section 7.2). Counter-clockwise rings have a positive signed area.
 *
 *  - Kind: floats (Float32, Float64 are exported); needs a square root.
 *  - Layout: the points are a public GUCEF::vector (a ring has no invariant); the polygon owns its storage, so
 *    copying copies the points. Queries do not allocate; building (AddPoint), the wireframe, text and DataNode
 *    members do.
 *  - Fewer than three points is a valid value but not a polygon: the area is 0, Contains is false, IsConvex and
 *    IsSimple are false, the closest point is that of the points or the segment.
 *  - Contains uses the winding number (non zero is inside), so a self-intersecting ring contains the regions that it
 *    winds around; points on the boundary are inside (exact test).
 *  - Realtime: the const queries are realtime-safe; AddPoint, the wireframe, text and DataNode members are not.
 *  - Text: "Polygon2D((x, y), (x, y), ...)"; DataNode: child node points with one child node point per vertex.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTPolygon2D
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint2D< T > TPoint;
    typedef MATH::CTVector2D< T > TVector;
    typedef GUCEF::vector< TPoint > TPointVector;

    TPointVector points;

    CTPolygon2D( void ) : points() {}

    /** The ring through 'count' points */
    CTPolygon2D( const TPoint* ringPoints, UInt32 count ) : points()
    {
        GUCEF_GEOMETRY_ASSERT( 0 == count || GUCEF_NULL != ringPoints );
        if ( GUCEF_NULL != ringPoints )
            points.assign( ringPoints, ringPoints + count );
    }

    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetPointCount( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return (UInt32) points.size(); }

    /** The vertex at 'index' (below GetPointCount) */
    GUCEF_GEOMETRY_FORCE_INLINE const TPoint& GetPoint( UInt32 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        GUCEF_GEOMETRY_ASSERT( index < points.size() );
        return points[ index ];
    }

    void AddPoint( const TPoint& point ) { points.push_back( point ); }

    void Clear( void ) { points.clear(); }

    /** The edge from vertex 'index' to the next vertex (the first one after the last); a zero segment when there are no points */
    CTSegment2D< T > GetEdge( UInt32 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The shoelace area: positive for counter-clockwise rings, negative for clockwise ones, 0 for fewer than 3 points */
    T GetSignedArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    GUCEF_GEOMETRY_FORCE_INLINE T GetArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return TTraits::Abs( GetSignedArea() ); }

    GUCEF_GEOMETRY_FORCE_INLINE bool IsCounterClockwise( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return TTraits::Zero() < GetSignedArea(); }

    /** The length of the closed boundary; the length of the segment for two points, 0 for fewer */
    T GetPerimeter( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The area centroid; the mean of the points when the area is 0, the origin without points */
    TPoint GetCentroid( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  True for a ring of at least three points whose turns all have the same sign (collinear points allowed, not all
     *  collinear) and that winds around once: star shapes and self-intersecting rings are not convex.
     */
    bool IsConvex( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  True for a ring of at least three points with no edge of zero length, no edge that folds back over its
     *  neighbour and no two edges that touch or cross apart from neighbours sharing their vertex (O(n^2)).
     */
    bool IsSimple( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Reverses the order of the points (flips the winding) */
    void Reverse( void ) { std::reverse( points.begin(), points.end() ); }

    /** True when the point lies exactly on an edge (collinear and between the end points) */
    bool IsOnBoundary( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** True when the point is inside or on the boundary; false for fewer than 3 points */
    bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The nearest point of the boundary (the vertex of a single point ring, the point itself for no points) */
    TPoint GetClosestBoundaryPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The nearest point of the solid ring: the point itself when inside, otherwise on the boundary */
    TPoint ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    GUCEF_GEOMETRY_FORCE_INLINE T GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( point - ClosestPoint( point ) ).GetSquaredLength(); }
    GUCEF_GEOMETRY_FORCE_INLINE T GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( point - ClosestPoint( point ) ).GetLength(); }

    /** The axis aligned rectangle around the points; empty without points */
    CTRectangle< T > GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The vertex furthest along 'direction' (the first on ties; the support point of the ring and of its hull alike); the origin without points */
    TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Appends the edges (closed for 3 or more points, the single segment for two); 'segmentsPerCircle' is not used */
    void GetWireframeLines( GUCEF::vector< CTSegment2D< T > >& lines, UInt32 segmentsPerCircle ) const;

    friend bool operator==( const CTPolygon2D& a, const CTPolygon2D& b ) { return a.points == b.points; }
    friend bool operator!=( const CTPolygon2D& a, const CTPolygon2D& b ) { return !( a == b ); }

    /** Same number of points, each equal within the default tolerances of the kind */
    bool Equals( const CTPolygon2D& other ) const
    {
        if ( points.size() != other.points.size() )
            return false;
        for ( UInt32 i=0; i<(UInt32) points.size(); ++i )
        {
            if ( !points[ i ].Equals( other.points[ i ] ) )
                return false;
        }
        return true;
    }

    /** "Polygon2D((x, y), (x, y), ...)" */
    CORE::CString ToString( void ) const
    {
        return ShapePointsToString( "Polygon2D", points );
    }

    /** Parses the ToString form; false leaves this polygon unchanged */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TPointVector parsed;
        if ( !reader.TryLiteral( "Polygon2D" ) || !reader.TryPointList( parsed ) || !reader.AtEnd() )
            return false;
        points.swap( parsed );
        return true;
    }

    /** Writes the child node points (one child node point per vertex) into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        return SerializeShapePoints( node, "points", points, settings );
    }

    /** Reads the child node points; a missing or invalid one fails and leaves this polygon unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        return DeserializeShapePoints( node, "points", points, settings );
    }

    private:

    static GUCEF_GEOMETRY_FORCE_INLINE Int32 SignOf( const T& value ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return TTraits::Zero() < value ? 1 : ( value < TTraits::Zero() ? -1 : 0 );
    }

    /** The number of sign changes of one component of the edge vectors, going around the ring (zero components skipped) */
    UInt32 CountEdgeSignChanges( UInt32 axis ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
CTSegment2D< T >
CTPolygon2D< T >::GetEdge( UInt32 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const UInt32 n = (UInt32) points.size();
    if ( 0 == n )
        return CTSegment2D< T >();
    return CTSegment2D< T >( points[ index % n ], points[ ( index + 1 ) % n ] );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTPolygon2D< T >::GetSignedArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const UInt32 n = (UInt32) points.size();
    if ( n < 3 )
        return TTraits::Zero();
    /* the shoelace sum relative to the first vertex keeps the terms small for polygons far from the origin */
    T sum = TTraits::Zero();
    for ( UInt32 i=1; i+1<n; ++i )
        sum = sum + ( points[ i ] - points[ 0 ] ).Cross( points[ i + 1 ] - points[ 0 ] );
    return sum * TTraits::Half();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTPolygon2D< T >::GetPerimeter( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const UInt32 n = (UInt32) points.size();
    if ( n < 2 )
        return TTraits::Zero();
    T sum = TTraits::Zero();
    const UInt32 edges = 2 == n ? 1 : n;
    for ( UInt32 i=0; i<edges; ++i )
        sum = sum + ( points[ ( i + 1 ) % n ] - points[ i ] ).GetLength();
    return sum;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTPolygon2D< T >::TPoint
CTPolygon2D< T >::GetCentroid( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const UInt32 n = (UInt32) points.size();
    if ( 0 == n )
        return TPoint();
    if ( n >= 3 )
    {
        /* triangle fan from the first vertex: the centroid of each triangle weighted by its twice signed area */
        T twiceArea = TTraits::Zero();
        TVector weighted;
        for ( UInt32 i=1; i+1<n; ++i )
        {
            TVector r1 = points[ i ] - points[ 0 ];
            TVector r2 = points[ i + 1 ] - points[ 0 ];
            T cross = r1.Cross( r2 );
            twiceArea = twiceArea + cross;
            weighted = weighted + ( r1 + r2 ) * cross;
        }
        if ( TTraits::Zero() != twiceArea )
            return points[ 0 ] + weighted / ( twiceArea * TTraits::FromInt32( 3 ) );
    }
    return TPoint::Centroid( &points[ 0 ], n );
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTPolygon2D< T >::CountEdgeSignChanges( UInt32 axis ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const UInt32 n = (UInt32) points.size();
    Int32 first = 0;
    Int32 previous = 0;
    UInt32 changes = 0;
    for ( UInt32 i=0; i<n; ++i )
    {
        const Int32 s = SignOf( ( points[ ( i + 1 ) % n ] - points[ i ] ).GetComponent( axis ) );
        if ( 0 == s )
            continue;
        if ( 0 == first )
            first = s;
        else if ( s != previous )
            ++changes;
        previous = s;
    }
    if ( 0 != first && previous != first )
        ++changes;
    return changes;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTPolygon2D< T >::IsConvex( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const UInt32 n = (UInt32) points.size();
    if ( n < 3 )
        return false;
    Int32 turn = 0;
    for ( UInt32 i=0; i<n; ++i )
    {
        const TVector e1 = points[ ( i + 1 ) % n ] - points[ i ];
        const TVector e2 = points[ ( i + 2 ) % n ] - points[ ( i + 1 ) % n ];
        const Int32 s = SignOf( e1.Cross( e2 ) );
        if ( 0 == s )
            continue;
        if ( 0 != turn && turn != s )
            return false;
        turn = s;
    }
    return 0 != turn && CountEdgeSignChanges( 0 ) <= 2 && CountEdgeSignChanges( 1 ) <= 2;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTPolygon2D< T >::IsSimple( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const UInt32 n = (UInt32) points.size();
    if ( n < 3 )
        return false;
    for ( UInt32 i=0; i<n; ++i )
    {
        const TVector e1 = points[ ( i + 1 ) % n ] - points[ i ];
        const TVector e2 = points[ ( i + 2 ) % n ] - points[ ( i + 1 ) % n ];
        if ( e1.IsZero() )
            return false;
        if ( TTraits::Zero() == e1.Cross( e2 ) && e1.Dot( e2 ) < TTraits::Zero() )
            return false;
    }
    for ( UInt32 i=0; i<n; ++i )
    {
        const CTSegment2D< T > edgeI( points[ i ], points[ ( i + 1 ) % n ] );
        for ( UInt32 j=i+2; j<n; ++j )
        {
            if ( 0 == i && n - 1 == j )
                continue;       /* neighbours across the closing edge */
            if ( edgeI.Intersects( CTSegment2D< T >( points[ j ], points[ ( j + 1 ) % n ] ) ) )
                return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTPolygon2D< T >::IsOnBoundary( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const UInt32 n = (UInt32) points.size();
    if ( 0 == n )
        return false;
    if ( 1 == n )
        return point == points[ 0 ];
    const UInt32 edges = 2 == n ? 1 : n;
    for ( UInt32 i=0; i<edges; ++i )
    {
        const TPoint& p = points[ i ];
        const TPoint& q = points[ ( i + 1 ) % n ];
        if ( TTraits::Zero() == ( q - p ).Cross( point - p ) &&
             p.Min( q ).x <= point.x && point.x <= p.Max( q ).x && p.Min( q ).y <= point.y && point.y <= p.Max( q ).y )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTPolygon2D< T >::Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const UInt32 n = (UInt32) points.size();
    if ( n < 3 )
        return false;
    if ( IsOnBoundary( point ) )
        return true;
    /* winding number (Sunday): upward edges with the point on their left count up, downward edges with it on their right count down */
    Int32 winding = 0;
    for ( UInt32 i=0; i<n; ++i )
    {
        const TPoint& p = points[ i ];
        const TPoint& q = points[ ( i + 1 ) % n ];
        if ( p.y <= point.y )
        {
            if ( point.y < q.y && TTraits::Zero() < ( q - p ).Cross( point - p ) )
                ++winding;
        }
        else if ( q.y <= point.y && ( q - p ).Cross( point - p ) < TTraits::Zero() )
        {
            --winding;
        }
    }
    return 0 != winding;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTPolygon2D< T >::TPoint
CTPolygon2D< T >::GetClosestBoundaryPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const UInt32 n = (UInt32) points.size();
    if ( 0 == n )
        return point;
    if ( 1 == n )
        return points[ 0 ];
    const UInt32 edges = 2 == n ? 1 : n;
    TPoint best = CTSegment2D< T >( points[ 0 ], points[ 1 % n ] ).ClosestPoint( point );
    T bestDistance = ( point - best ).GetSquaredLength();
    for ( UInt32 i=1; i<edges; ++i )
    {
        TPoint candidate = CTSegment2D< T >( points[ i ], points[ ( i + 1 ) % n ] ).ClosestPoint( point );
        T distance = ( point - candidate ).GetSquaredLength();
        if ( distance < bestDistance )
        {
            best = candidate;
            bestDistance = distance;
        }
    }
    return best;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTPolygon2D< T >::TPoint
CTPolygon2D< T >::ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( Contains( point ) )
        return point;
    return GetClosestBoundaryPoint( point );
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTRectangle< T >
CTPolygon2D< T >::GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( points.empty() )
        return CTRectangle< T >();
    return CTRectangle< T >::FromPoints( &points[ 0 ], (UInt32) points.size() );
}

/*-------------------------------------------------------------------------*/

template < typename T >
typename CTPolygon2D< T >::TPoint
CTPolygon2D< T >::GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const UInt32 n = (UInt32) points.size();
    if ( 0 == n )
        return TPoint();
    UInt32 best = 0;
    T bestDot = points[ 0 ].ToPositionVector().Dot( direction );
    for ( UInt32 i=1; i<n; ++i )
    {
        T d = points[ i ].ToPositionVector().Dot( direction );
        if ( bestDot < d )
        {
            bestDot = d;
            best = i;
        }
    }
    return points[ best ];
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTPolygon2D< T >::GetWireframeLines( GUCEF::vector< CTSegment2D< T > >& lines, UInt32 /*segmentsPerCircle*/ ) const
{
    const UInt32 n = (UInt32) points.size();
    if ( n < 2 )
        return;
    const UInt32 edges = 2 == n ? 1 : n;
    for ( UInt32 i=0; i<edges; ++i )
        lines.push_back( CTSegment2D< T >( points[ i ], points[ ( i + 1 ) % n ] ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolygon2D< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolygon2D< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTPOLYGON2D_H ? */
