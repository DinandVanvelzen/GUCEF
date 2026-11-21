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

#ifndef GUCEF_GEOMETRY_POLYGONTRIANGULATION_H
#define GUCEF_GEOMETRY_POLYGONTRIANGULATION_H

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

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_FIXEDPOINT_H
#include "gucefMATH_fixedpoint.h"
#define GUCEF_MATH_FIXEDPOINT_H
#endif /* GUCEF_MATH_FIXEDPOINT_H ? */

#ifndef GUCEF_GEOMETRY_CTPREDICATES_H
#include "gucefGEOMETRY_CTPredicates.h"
#define GUCEF_GEOMETRY_CTPREDICATES_H
#endif /* GUCEF_GEOMETRY_CTPREDICATES_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYGON2D_H
#include "gucefGEOMETRY_CTPolygon2D.h"
#define GUCEF_GEOMETRY_CTPOLYGON2D_H
#endif /* GUCEF_GEOMETRY_CTPOLYGON2D_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYGONWITHHOLES2D_H
#include "gucefGEOMETRY_CTPolygonWithHoles2D.h"
#define GUCEF_GEOMETRY_CTPOLYGONWITHHOLES2D_H
#endif /* GUCEF_GEOMETRY_CTPOLYGONWITHHOLES2D_H ? */

#ifndef GUCEF_GEOMETRY_CTTRIANGLE2D_H
#include "gucefGEOMETRY_CTTriangle2D.h"
#define GUCEF_GEOMETRY_CTTRIANGLE2D_H
#endif /* GUCEF_GEOMETRY_CTTRIANGLE2D_H ? */

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

/** The outcome of a polygon triangulation */
enum ETriangulationStatus
{
    TRIANGULATION_OK            = 0 ,   /**< every ear and every hole bridge passed the exact tests */
    TRIANGULATION_DEGENERATE    = 1 ,   /**< the outer ring has fewer than three distinct points or no area: no triangles */
    TRIANGULATION_NOT_SIMPLE    = 2 ,   /**< a ring is not simple, or a hole is not inside the outer ring: the triangles are a best effort (see the class comment) */
    TRIANGULATION_INVALID_INPUT = 3     /**< a coordinate is not finite or a point array is missing: no triangles */
};
typedef enum ETriangulationStatus TTriangulationStatus;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Triangulation of simple polygons with holes by ear clipping (section 7.9 of the design), with the exact orientation
 *  predicate of CTPredicates for every decision, so the result does not depend on rounding.
 *
 *  - Input: an outer ring and hole rings, each a closed ring of points (the closing point not repeated), in either
 *    orientation (rings are normalised: the outer ring counter-clockwise, holes clockwise). Consecutive duplicate points
 *    are merged (the first index is kept); collinear points are kept as vertices.
 *  - Holes are joined to the outer ring by bridges in the order of their rightmost point (Eberly, "Triangulation by ear
 *    clipping"): from the rightmost hole point M the nearest vertex V of the merged ring is chosen whose segment to M
 *    leaves both vertices into the region and touches no edge of any ring (exact tests, nearest first by Float64
 *    distance); the ring then runs V, M, the hole, M, V.
 *  - An ear is a strictly convex vertex whose triangle contains no other vertex of the ring (closed test against the
 *    vertices that are not strictly convex; a copy of a corner made by a bridge or a pinch point blocks only when one of
 *    its edges starts into the triangle).
 *    Triangles are counter-clockwise with positive area.
 *  - Result: indices into the points, three per triangle. For a simple polygon of n distinct points with h holes of
 *    n_i points that lie strictly inside it, the status is OK and the count is n + sum( n_i ) + 2 h - 2: the triangles
 *    tile the polygon exactly (no overlap, total area equal to the polygon area). A hole that shares a vertex with the
 *    merged ring (the outer ring and the holes joined before it), or without a valid bridge has a vertex on one of its
 *    edges, is joined at that point instead when the hole lies inside the region there: a bridge of zero length, the
 *    ring passes the point twice (an edge with the point inside gets a vertex there), so the count is lower by 2 (by
 *    1 on an edge) per such hole and the status stays OK. Holes that touch along an edge, or overlap, are not simple
 *    input.
 *  - Degenerate input: a fold back of zero width (a spike) is removed without a triangle; when no ear exists (a
 *    self-intersecting or touching ring) the first strictly convex vertex is clipped anyway; a hole without a valid
 *    bridge is left out; the status is then NOT_SIMPLE. Rings and holes of fewer than three distinct points, or without
 *    area, are left out.
 *  - Kinds: every kind of CTPredicates (floats, integers, fixed point, fractions); Float64 is used only to order bridge
 *    candidates. Float32, Float64, Int32 and TFixed32_32 are exported (gucefGEOMETRY_PolygonTriangulation.cpp).
 *  - Cost: O(n r) for n ring vertices and r vertices that are not strictly convex, plus O(n) per bridge candidate.
 *  - Allocation: the object keeps its work arrays and reuses them; a call allocates only when the input is larger than
 *    any earlier one, plus the growth of the output vector. Not realtime-safe; one thread at a time; not copyable.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTPolygonTriangulator
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint2D< T > TPoint;
    typedef CTPredicates< T > TPredicates;
    typedef GUCEF::vector< UInt32 > TIndexVector;

    CTPolygonTriangulator( void );

    /** Triangulates the ring of 'count' points; 'triangles' receives three point indices per triangle */
    TTriangulationStatus Triangulate( const TPoint* ring, UInt32 count, TIndexVector& triangles );

    /**
     *  Triangulates a ring with holes. 'points' holds the outer ring ('outerCount' points) followed by the holes, hole i
     *  having holeCounts[ i ] points; the indices of 'triangles' are into 'points'.
     */
    TTriangulationStatus Triangulate( const TPoint* points, UInt32 outerCount, const UInt32* holeCounts, UInt32 holeCount, TIndexVector& triangles );

    /** As the ring form, for a CTPolygon2D */
    TTriangulationStatus Triangulate( const CTPolygon2D< T >& polygon, TIndexVector& triangles );

    /** As the ring-with-holes form; the indices count the outer points first, then the points of each hole in order */
    TTriangulationStatus Triangulate( const CTPolygonWithHoles2D< T >& polygon, TIndexVector& triangles );

    /** The number of triangles of the last call */
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetTriangleCount( void ) const { return m_triangleCount; }

    private:

    struct SNode
    {
        UInt32 point;       /**< index into the input points */
        UInt32 prev;
        UInt32 next;
    };

    /** Orders bridge candidates by distance, then by node index */
    struct SCandidate
    {
        Float64 distance;
        UInt32 node;

        bool operator<( const SCandidate& other ) const
        {
            if ( distance < other.distance ) return true;
            if ( other.distance < distance ) return false;
            return node < other.node;
        }
    };

    /** A hole ring: its first node and its rightmost node */
    struct SHole
    {
        UInt32 first;
        UInt32 rightmost;
        UInt32 index;

        bool alive;
    };

    /** Orders holes by their rightmost point (x then y descending), then by hole index */
    class CHoleOrder
    {
        public:

        CHoleOrder( const TPoint* points, const SNode* nodes ) : m_points( points ), m_nodes( nodes ) {}

        bool operator()( const SHole& a, const SHole& b ) const
        {
            const TPoint& pa = m_points[ m_nodes[ a.rightmost ].point ];
            const TPoint& pb = m_points[ m_nodes[ b.rightmost ].point ];
            if ( pb.x < pa.x ) return true;
            if ( pa.x < pb.x ) return false;
            if ( pb.y < pa.y ) return true;
            if ( pa.y < pb.y ) return false;
            return a.index < b.index;
        }

        private:

        const TPoint* m_points;
        const SNode* m_nodes;
    };

    /** Orders nodes by the position of their point (x, then y), then by node index */
    class CNodePositionOrder
    {
        public:

        CNodePositionOrder( const TPoint* points, const SNode* nodes ) : m_points( points ), m_nodes( nodes ) {}

        bool operator()( UInt32 a, UInt32 b ) const
        {
            const TPoint& pa = m_points[ m_nodes[ a ].point ];
            const TPoint& pb = m_points[ m_nodes[ b ].point ];
            if ( pa.x < pb.x ) return true;
            if ( pb.x < pa.x ) return false;
            if ( pa.y < pb.y ) return true;
            if ( pb.y < pa.y ) return false;
            return a < b;
        }

        private:

        const TPoint* m_points;
        const SNode* m_nodes;
    };

    enum { NoNode = 0xFFFFFFFFU };

    CTPolygonTriangulator( const CTPolygonTriangulator& src );              /**< not implemented */
    CTPolygonTriangulator& operator=( const CTPolygonTriangulator& src );   /**< not implemented */

    GUCEF_GEOMETRY_FORCE_INLINE const TPoint& P( UInt32 node ) const { return m_points[ m_nodes[ node ].point ]; }

    static GUCEF_GEOMETRY_FORCE_INLINE bool SamePosition( const TPoint& a, const TPoint& b ) { return a.x == b.x && a.y == b.y; }

    static GUCEF_GEOMETRY_FORCE_INLINE bool LexLess( const TPoint& a, const TPoint& b ) { return a.x < b.x || ( a.x == b.x && a.y < b.y ); }

    /** For collinear points: true when v lies on the closed segment p-n */
    static GUCEF_GEOMETRY_FORCE_INLINE bool Between( const TPoint& p, const TPoint& v, const TPoint& n )
    {
        return !( v.x < p.x && v.x < n.x ) && !( p.x < v.x && n.x < v.x ) && !( v.y < p.y && v.y < n.y ) && !( p.y < v.y && n.y < v.y );
    }

    /** Appends the ring of 'count' points starting at point 'firstPoint' as linked nodes without consecutive duplicates; the first node or NoNode for fewer than three nodes */
    UInt32 BuildRing( UInt32 firstPoint, UInt32 count, UInt32& nodeCount );

    /** +1 counter-clockwise, -1 clockwise, 0 without area: the turn at the lowest vertex, or the sign of the Float64 area when that turn is 0 */
    Int32 RingOrientation( UInt32 first ) const;

    void ReverseRing( UInt32 first );

    /** True when X lies strictly inside the region wedge at V (the region is left of P -> V -> N) */
    bool InCone( const TPoint& p, const TPoint& v, const TPoint& n, const TPoint& x ) const;

    /** True when the closed segments a-b and c-d have a point in common */
    static bool SegmentsTouch( const TPoint& a, const TPoint& b, const TPoint& c, const TPoint& d );

    /** True when the segment v-m crosses or touches an edge of the ring from 'first' (edges at v or m only when they overlap the segment) */
    bool RingBlocks( UInt32 first, const TPoint& v, const TPoint& m ) const;

    /** Joins the hole to the merged ring through a bridge; false when no valid bridge exists */
    bool BridgeHole( UInt32 holeSlot );

    /**
     *  Joins a hole that touches the merged ring (a hole vertex at a vertex of the ring or on one of its edges) at that
     *  point: a bridge of zero length, so the ring runs through the point twice. False when the hole touches nowhere
     *  in a way that keeps both corners at the point inside the region.
     */
    bool PinchHole( UInt32 holeSlot );

    /** Splices the hole node h into the merged ring at the node v of the same position when both new corners are valid */
    bool SpliceAt( UInt32 v, UInt32 h );

    /** Joins the hole at a vertex it shares with the merged ring (found through the position order of the input nodes) */
    bool PinchHoleAtVertex( UInt32 holeSlot );

    void ClassifyNode( UInt32 node );

    /** For a node at the position of a corner of the counter-clockwise triangle a, b, c: true when an edge of the node starts into the open angle of the triangle at that corner */
    bool CopyEntersTriangle( UInt32 node, const TPoint& a, const TPoint& b, const TPoint& c ) const;

    bool IsEar( UInt32 node ) const;

    void Unlink( UInt32 node );

    TTriangulationStatus ClipEars( TIndexVector& triangles );

    const TPoint* m_points;
    GUCEF::vector< SNode > m_nodes;
    GUCEF::vector< SHole > m_holes;
    GUCEF::vector< SCandidate > m_candidates;
    TIndexVector m_positionOrder;   /**< the input nodes sorted by position */
    TIndexVector m_ringFirst;       /**< per input node: the first node of its ring */
    GUCEF::vector< UInt8 > m_merged;    /**< per input node: 1 when its ring is part of the merged ring */
    GUCEF::vector< UInt32 > m_reflex;           /**< the nodes that are not strictly convex */
    GUCEF::vector< UInt32 > m_reflexSlot;       /**< per node its position in m_reflex or NoNode */
    GUCEF::vector< TPoint > m_concatenated;     /**< the rings of a CTPolygonWithHoles2D in one array */
    GUCEF::vector< UInt32 > m_holeCounts;
    UInt32 m_outerFirst;
    UInt32 m_ringSize;
    UInt32 m_triangleCount;
    bool m_simple;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
CTPolygonTriangulator< T >::CTPolygonTriangulator( void )
    : m_points( GUCEF_NULL )
    , m_nodes()
    , m_holes()
    , m_candidates()
    , m_positionOrder()
    , m_ringFirst()
    , m_merged()
    , m_reflex()
    , m_reflexSlot()
    , m_concatenated()
    , m_holeCounts()
    , m_outerFirst( NoNode )
    , m_ringSize( 0 )
    , m_triangleCount( 0 )
    , m_simple( true )
{
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTPolygonTriangulator< T >::BuildRing( UInt32 firstPoint, UInt32 count, UInt32& nodeCount )
{
    nodeCount = 0;
    const UInt32 base = (UInt32) m_nodes.size();
    for ( UInt32 i=0; i<count; ++i )
    {
        const UInt32 point = firstPoint + i;
        if ( nodeCount > 0 && SamePosition( m_points[ point ], m_points[ m_nodes.back().point ] ) )
            continue;
        SNode node;
        node.point = point;
        node.prev = NoNode;
        node.next = NoNode;
        m_nodes.push_back( node );
        ++nodeCount;
    }
    // the closing duplicate
    while ( nodeCount > 1 && SamePosition( m_points[ m_nodes.back().point ], m_points[ m_nodes[ base ].point ] ) )
    {
        m_nodes.pop_back();
        --nodeCount;
    }
    if ( nodeCount < 3 )
    {
        m_nodes.resize( base );
        nodeCount = 0;
        return NoNode;
    }
    for ( UInt32 i=0; i<nodeCount; ++i )
    {
        m_nodes[ base + i ].prev = base + ( i + nodeCount - 1 ) % nodeCount;
        m_nodes[ base + i ].next = base + ( i + 1 ) % nodeCount;
    }
    return base;
}

/*-------------------------------------------------------------------------*/

template < typename T >
Int32
CTPolygonTriangulator< T >::RingOrientation( UInt32 first ) const
{
    UInt32 lowest = first;
    UInt32 node = m_nodes[ first ].next;
    while ( node != first )
    {
        if ( LexLess( P( node ), P( lowest ) ) )
            lowest = node;
        node = m_nodes[ node ].next;
    }
    const Int32 turn = TPredicates::Orient2D( P( m_nodes[ lowest ].prev ), P( lowest ), P( m_nodes[ lowest ].next ) );
    if ( 0 != turn )
        return turn;
    // a spike at the lowest vertex: the sign of the area decides (only reached for rings that are not simple)
    Float64 area = 0.0;
    const TPoint& origin = P( first );
    node = m_nodes[ first ].next;
    while ( m_nodes[ node ].next != first )
    {
        const UInt32 after = m_nodes[ node ].next;
        const Float64 ax = TTraits::ToFloat64( P( node ).x ) - TTraits::ToFloat64( origin.x );
        const Float64 ay = TTraits::ToFloat64( P( node ).y ) - TTraits::ToFloat64( origin.y );
        const Float64 bx = TTraits::ToFloat64( P( after ).x ) - TTraits::ToFloat64( origin.x );
        const Float64 by = TTraits::ToFloat64( P( after ).y ) - TTraits::ToFloat64( origin.y );
        area += ax * by - ay * bx;
        node = after;
    }
    return area > 0.0 ? 1 : ( area < 0.0 ? -1 : 0 );
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTPolygonTriangulator< T >::ReverseRing( UInt32 first )
{
    UInt32 node = first;
    do
    {
        SNode& n = m_nodes[ node ];
        const UInt32 next = n.next;
        n.next = n.prev;
        n.prev = next;
        node = next;
    }
    while ( node != first );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTPolygonTriangulator< T >::InCone( const TPoint& p, const TPoint& v, const TPoint& n, const TPoint& x ) const
{
    const Int32 turn = TPredicates::Orient2D( p, v, n );
    const Int32 a = TPredicates::Orient2D( p, v, x );
    const Int32 b = TPredicates::Orient2D( v, n, x );
    if ( turn > 0 )
        return a > 0 && b > 0;
    if ( turn < 0 )
        return a > 0 || b > 0;
    // straight on: the half plane to the left; a spike has no inside
    return Between( p, v, n ) && a > 0;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTPolygonTriangulator< T >::SegmentsTouch( const TPoint& a, const TPoint& b, const TPoint& c, const TPoint& d )
{
    const Int32 o1 = TPredicates::Orient2D( a, b, c );
    const Int32 o2 = TPredicates::Orient2D( a, b, d );
    if ( o1 * o2 > 0 )
        return false;
    const Int32 o3 = TPredicates::Orient2D( c, d, a );
    const Int32 o4 = TPredicates::Orient2D( c, d, b );
    if ( o3 * o4 > 0 )
        return false;
    if ( 0 != o1 || 0 != o2 || 0 != o3 || 0 != o4 )
        return true;
    // collinear: the coordinate ranges must overlap
    const T& abMinX = a.x < b.x ? a.x : b.x;
    const T& abMaxX = a.x < b.x ? b.x : a.x;
    const T& abMinY = a.y < b.y ? a.y : b.y;
    const T& abMaxY = a.y < b.y ? b.y : a.y;
    const T& cdMinX = c.x < d.x ? c.x : d.x;
    const T& cdMaxX = c.x < d.x ? d.x : c.x;
    const T& cdMinY = c.y < d.y ? c.y : d.y;
    const T& cdMaxY = c.y < d.y ? d.y : c.y;
    return !( abMaxX < cdMinX || cdMaxX < abMinX || abMaxY < cdMinY || cdMaxY < abMinY );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTPolygonTriangulator< T >::RingBlocks( UInt32 first, const TPoint& v, const TPoint& m ) const
{
    UInt32 node = first;
    do
    {
        const UInt32 next = m_nodes[ node ].next;
        const TPoint& a = P( node );
        const TPoint& b = P( next );
        const bool aShared = SamePosition( a, v ) || SamePosition( a, m );
        const bool bShared = SamePosition( b, v ) || SamePosition( b, m );
        if ( aShared && bShared )
        {
            // an edge between the two end points (or a zero edge) blocks only when it is the segment itself
            if ( !SamePosition( a, b ) )
                return true;
        }
        else
        if ( aShared || bShared )
        {
            // an edge at an end point blocks when it runs along the segment
            const TPoint& shared = aShared ? a : b;
            const TPoint& other = aShared ? b : a;
            if ( 0 == TPredicates::Orient2D( v, m, other ) && SegmentsTouch( v, m, other, other ) )
                return true;
            const TPoint& farPoint = SamePosition( shared, v ) ? m : v;
            if ( 0 == TPredicates::Orient2D( shared, other, farPoint ) && SegmentsTouch( shared, other, farPoint, farPoint ) )
                return true;
        }
        else
        if ( SegmentsTouch( v, m, a, b ) )
            return true;
        node = next;
    }
    while ( node != first );
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTPolygonTriangulator< T >::BridgeHole( UInt32 holeSlot )
{
    const SHole& hole = m_holes[ holeSlot ];
    const UInt32 mNode = hole.rightmost;
    const TPoint& m = P( mNode );
    const TPoint& mPrev = P( m_nodes[ mNode ].prev );
    const TPoint& mNext = P( m_nodes[ mNode ].next );
    const Float64 mx = TTraits::ToFloat64( m.x );
    const Float64 my = TTraits::ToFloat64( m.y );

    m_candidates.clear();
    UInt32 node = m_outerFirst;
    do
    {
        SCandidate candidate;
        const Float64 dx = TTraits::ToFloat64( P( node ).x ) - mx;
        const Float64 dy = TTraits::ToFloat64( P( node ).y ) - my;
        candidate.distance = dx * dx + dy * dy;
        candidate.node = node;
        m_candidates.push_back( candidate );
        node = m_nodes[ node ].next;
    }
    while ( node != m_outerFirst );
    std::sort( m_candidates.begin(), m_candidates.end() );

    for ( size_t c=0; c<m_candidates.size(); ++c )
    {
        const UInt32 vNode = m_candidates[ c ].node;
        const TPoint& v = P( vNode );
        if ( SamePosition( v, m ) )
            continue;
        if ( !InCone( P( m_nodes[ vNode ].prev ), v, P( m_nodes[ vNode ].next ), m ) )
            continue;
        if ( !InCone( mPrev, m, mNext, v ) )
            continue;
        if ( RingBlocks( m_outerFirst, v, m ) )
            continue;
        bool blocked = false;
        for ( size_t h=holeSlot; h<m_holes.size() && !blocked; ++h )
            if ( m_holes[ h ].alive )
                blocked = RingBlocks( m_holes[ h ].first, v, m );
        if ( blocked )
            continue;

        // V, M, the hole, M', V', then the successor of V
        const UInt32 vNext = m_nodes[ vNode ].next;
        const UInt32 mPrevNode = m_nodes[ mNode ].prev;
        const UInt32 m2 = (UInt32) m_nodes.size();
        const UInt32 v2 = m2 + 1;
        SNode copy;
        copy.point = m_nodes[ mNode ].point;
        copy.prev = mPrevNode;
        copy.next = v2;
        m_nodes.push_back( copy );
        copy.point = m_nodes[ vNode ].point;
        copy.prev = m2;
        copy.next = vNext;
        m_nodes.push_back( copy );
        m_nodes[ vNode ].next = mNode;
        m_nodes[ mNode ].prev = vNode;
        m_nodes[ mPrevNode ].next = m2;
        m_nodes[ vNext ].prev = v2;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTPolygonTriangulator< T >::SpliceAt( UInt32 v, UInt32 h )
{
    /*
     *  The region wedge at v runs counter-clockwise from b to a; the hole (clockwise) excludes the wedge from p to q
     *  inside it. The ring a, v, q, ..., p, h, b keeps the wedges b to p (at h) and q to a (at v).
     */
    const UInt32 a = m_nodes[ v ].prev, b = m_nodes[ v ].next;
    const UInt32 p = m_nodes[ h ].prev, q = m_nodes[ h ].next;
    if ( !InCone( P( a ), P( v ), P( b ), P( p ) ) || !InCone( P( a ), P( v ), P( b ), P( q ) ) || InCone( P( p ), P( h ), P( b ), P( q ) ) )
        return false;
    m_nodes[ v ].next = q;
    m_nodes[ q ].prev = v;
    m_nodes[ h ].next = b;
    m_nodes[ b ].prev = h;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTPolygonTriangulator< T >::PinchHoleAtVertex( UInt32 holeSlot )
{
    const UInt32 holeFirst = m_holes[ holeSlot ].first;
    const UInt32 inputNodes = (UInt32) m_ringFirst.size();
    const CNodePositionOrder order( m_points, &m_nodes[ 0 ] );
    UInt32 h = holeFirst;
    do
    {
        if ( h < inputNodes )
        {
            // the input nodes at the position of h: a range of the position order around h
            const TIndexVector::const_iterator at = std::lower_bound( m_positionOrder.begin(), m_positionOrder.end(), h, order );
            TIndexVector::const_iterator begin = at;
            while ( begin != m_positionOrder.begin() && SamePosition( P( *( begin - 1 ) ), P( h ) ) )
                --begin;
            for ( TIndexVector::const_iterator i=begin; i!=m_positionOrder.end() && SamePosition( P( *i ), P( h ) ); ++i )
            {
                const UInt32 v = *i;
                if ( v != h && m_ringFirst[ v ] != holeFirst && 0 != m_merged[ m_ringFirst[ v ] ] && SpliceAt( v, h ) )
                    return true;
            }
        }
        h = m_nodes[ h ].next;
    }
    while ( h != holeFirst );
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTPolygonTriangulator< T >::PinchHole( UInt32 holeSlot )
{
    const UInt32 holeFirst = m_holes[ holeSlot ].first;
    UInt32 h = holeFirst;
    do
    {
        const TPoint hp = P( h );
        UInt32 v = m_outerFirst;
        do
        {
            const UInt32 vNext = m_nodes[ v ].next;
            if ( SamePosition( P( v ), hp ) )
            {
                if ( SpliceAt( v, h ) )
                    return true;
            }
            else
            if ( !SamePosition( P( vNext ), hp ) && 0 == TPredicates::Orient2D( P( v ), P( vNext ), hp ) && Between( P( v ), hp, P( vNext ) ) )
            {
                // on the edge v -> vNext: a vertex there (a copy of the hole point), removed again when the splice fails
                const UInt32 w = (UInt32) m_nodes.size();
                SNode split;
                split.point = m_nodes[ h ].point;
                split.prev = v;
                split.next = vNext;
                m_nodes.push_back( split );
                m_nodes[ v ].next = w;
                m_nodes[ vNext ].prev = w;
                if ( SpliceAt( w, h ) )
                    return true;
                m_nodes[ v ].next = vNext;
                m_nodes[ vNext ].prev = v;
                m_nodes.pop_back();
            }
            v = vNext;
        }
        while ( v != m_outerFirst );
        h = m_nodes[ h ].next;
    }
    while ( h != holeFirst );
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTPolygonTriangulator< T >::ClassifyNode( UInt32 node )
{
    const bool convex = TPredicates::Orient2D( P( m_nodes[ node ].prev ), P( node ), P( m_nodes[ node ].next ) ) > 0;
    const UInt32 slot = m_reflexSlot[ node ];
    if ( convex && NoNode != slot )
    {
        const UInt32 last = m_reflex.back();
        m_reflex[ slot ] = last;
        m_reflexSlot[ last ] = slot;
        m_reflex.pop_back();
        m_reflexSlot[ node ] = NoNode;
    }
    else
    if ( !convex && NoNode == slot )
    {
        m_reflexSlot[ node ] = (UInt32) m_reflex.size();
        m_reflex.push_back( node );
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTPolygonTriangulator< T >::CopyEntersTriangle( UInt32 node, const TPoint& a, const TPoint& b, const TPoint& c ) const
{
    const TPoint& p = P( node );
    // the corner and its angle from 'first' counter-clockwise to 'second'
    const TPoint& first = SamePosition( p, a ) ? b : ( SamePosition( p, b ) ? c : a );
    const TPoint& second = SamePosition( p, a ) ? c : ( SamePosition( p, b ) ? a : b );
    const TPoint& q1 = P( m_nodes[ node ].prev );
    const TPoint& q2 = P( m_nodes[ node ].next );
    return ( TPredicates::Orient2D( p, first, q1 ) > 0 && TPredicates::Orient2D( p, second, q1 ) < 0 ) ||
           ( TPredicates::Orient2D( p, first, q2 ) > 0 && TPredicates::Orient2D( p, second, q2 ) < 0 );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTPolygonTriangulator< T >::IsEar( UInt32 node ) const
{
    const UInt32 prev = m_nodes[ node ].prev;
    const UInt32 next = m_nodes[ node ].next;
    const TPoint& a = P( prev );
    const TPoint& b = P( node );
    const TPoint& c = P( next );
    if ( TPredicates::Orient2D( a, b, c ) <= 0 )
        return false;
    const T& minX = a.x < b.x ? ( a.x < c.x ? a.x : c.x ) : ( b.x < c.x ? b.x : c.x );
    const T& maxX = a.x < b.x ? ( b.x < c.x ? c.x : b.x ) : ( a.x < c.x ? c.x : a.x );
    const T& minY = a.y < b.y ? ( a.y < c.y ? a.y : c.y ) : ( b.y < c.y ? b.y : c.y );
    const T& maxY = a.y < b.y ? ( b.y < c.y ? c.y : b.y ) : ( a.y < c.y ? c.y : a.y );
    for ( size_t i=0; i<m_reflex.size(); ++i )
    {
        const UInt32 r = m_reflex[ i ];
        if ( r == prev || r == next )
            continue;
        const TPoint& p = P( r );
        if ( p.x < minX || maxX < p.x || p.y < minY || maxY < p.y )
            continue;
        if ( SamePosition( p, a ) || SamePosition( p, b ) || SamePosition( p, c ) )
        {
            // a copy of a corner (bridge or pinch): it blocks when one of its edges leaves into the triangle
            if ( CopyEntersTriangle( r, a, b, c ) )
                return false;
            continue;
        }
        if ( TPredicates::Orient2D( a, b, p ) >= 0 && TPredicates::Orient2D( b, c, p ) >= 0 && TPredicates::Orient2D( c, a, p ) >= 0 )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTPolygonTriangulator< T >::Unlink( UInt32 node )
{
    const UInt32 prev = m_nodes[ node ].prev;
    const UInt32 next = m_nodes[ node ].next;
    m_nodes[ prev ].next = next;
    m_nodes[ next ].prev = prev;
    const UInt32 slot = m_reflexSlot[ node ];
    if ( NoNode != slot )
    {
        const UInt32 last = m_reflex.back();
        m_reflex[ slot ] = last;
        m_reflexSlot[ last ] = slot;
        m_reflex.pop_back();
        m_reflexSlot[ node ] = NoNode;
    }
    if ( m_outerFirst == node )
        m_outerFirst = next;
    --m_ringSize;
}

/*-------------------------------------------------------------------------*/

template < typename T >
TTriangulationStatus
CTPolygonTriangulator< T >::ClipEars( TIndexVector& triangles )
{
    m_reflex.clear();
    m_reflexSlot.assign( m_nodes.size(), NoNode );
    m_ringSize = 0;
    UInt32 node = m_outerFirst;
    do
    {
        ++m_ringSize;
        ClassifyNode( node );
        node = m_nodes[ node ].next;
    }
    while ( node != m_outerFirst );

    triangles.reserve( triangles.size() + 3 * (size_t) m_ringSize );
    node = m_outerFirst;
    UInt32 stalled = 0;
    while ( m_ringSize > 3 )
    {
        UInt32 prev = m_nodes[ node ].prev;
        UInt32 next = m_nodes[ node ].next;
        const TPoint& a = P( prev );
        const TPoint& b = P( node );
        const TPoint& c = P( next );
        if ( 0 == TPredicates::Orient2D( a, b, c ) && ( SamePosition( a, c ) || !Between( a, b, c ) ) )
        {
            // a fold back of zero width (only in rings that are not simple): removed without a triangle, with the duplicate it leaves
            m_simple = false;
            Unlink( node );
            if ( m_ringSize > 3 && SamePosition( P( prev ), P( next ) ) )
            {
                const UInt32 after = m_nodes[ next ].next;
                Unlink( next );
                next = after;
            }
            ClassifyNode( prev );
            ClassifyNode( next );
            node = prev;
            stalled = 0;
            continue;
        }
        if ( IsEar( node ) )
        {
            triangles.push_back( m_nodes[ prev ].point );
            triangles.push_back( m_nodes[ node ].point );
            triangles.push_back( m_nodes[ next ].point );
            ++m_triangleCount;
            Unlink( node );
            ClassifyNode( prev );
            ClassifyNode( next );
            node = next;
            stalled = 0;
            continue;
        }
        node = next;
        if ( ++stalled < m_ringSize )
            continue;

        // no ear in a full turn: the ring is not simple; the first strictly convex vertex is clipped anyway
        m_simple = false;
        stalled = 0;
        UInt32 convex = NoNode;
        UInt32 probe = m_outerFirst;
        do
        {
            if ( TPredicates::Orient2D( P( m_nodes[ probe ].prev ), P( probe ), P( m_nodes[ probe ].next ) ) > 0 )
                convex = probe;
            probe = m_nodes[ probe ].next;
        }
        while ( probe != m_outerFirst && NoNode == convex );
        if ( NoNode == convex )
            return TRIANGULATION_NOT_SIMPLE;
        prev = m_nodes[ convex ].prev;
        next = m_nodes[ convex ].next;
        triangles.push_back( m_nodes[ prev ].point );
        triangles.push_back( m_nodes[ convex ].point );
        triangles.push_back( m_nodes[ next ].point );
        ++m_triangleCount;
        Unlink( convex );
        ClassifyNode( prev );
        ClassifyNode( next );
        node = next;
    }
    const UInt32 prev = m_nodes[ m_outerFirst ].prev;
    const UInt32 next = m_nodes[ m_outerFirst ].next;
    if ( TPredicates::Orient2D( P( prev ), P( m_outerFirst ), P( next ) ) > 0 )
    {
        triangles.push_back( m_nodes[ prev ].point );
        triangles.push_back( m_nodes[ m_outerFirst ].point );
        triangles.push_back( m_nodes[ next ].point );
        ++m_triangleCount;
    }
    else
        m_simple = false;
    return m_simple ? TRIANGULATION_OK : TRIANGULATION_NOT_SIMPLE;
}

/*-------------------------------------------------------------------------*/

template < typename T >
TTriangulationStatus
CTPolygonTriangulator< T >::Triangulate( const TPoint* points, UInt32 outerCount, const UInt32* holeCounts, UInt32 holeCount, TIndexVector& triangles )
{
    triangles.clear();
    m_triangleCount = 0;
    m_simple = true;
    m_nodes.clear();
    m_holes.clear();
    m_outerFirst = NoNode;
    m_points = points;
    if ( ( GUCEF_NULL == points && outerCount > 0 ) || ( GUCEF_NULL == holeCounts && holeCount > 0 ) )
        return TRIANGULATION_INVALID_INPUT;
    UInt32 total = outerCount;
    for ( UInt32 h=0; h<holeCount; ++h )
        total += holeCounts[ h ];
    if ( total > 0 && GUCEF_NULL == points )
        return TRIANGULATION_INVALID_INPUT;
    for ( UInt32 i=0; i<total; ++i )
        if ( !TTraits::IsFinite( points[ i ].x ) || !TTraits::IsFinite( points[ i ].y ) )
            return TRIANGULATION_INVALID_INPUT;

    m_nodes.reserve( (size_t) total + 2 * (size_t) holeCount );
    UInt32 outerNodes = 0;
    m_outerFirst = BuildRing( 0, outerCount, outerNodes );
    if ( NoNode == m_outerFirst )
        return TRIANGULATION_DEGENERATE;
    const Int32 orientation = RingOrientation( m_outerFirst );
    if ( 0 == orientation )
        return TRIANGULATION_DEGENERATE;
    if ( orientation < 0 )
        ReverseRing( m_outerFirst );

    UInt32 firstPoint = outerCount;
    for ( UInt32 h=0; h<holeCount; ++h )
    {
        UInt32 holeNodes = 0;
        const UInt32 first = BuildRing( firstPoint, holeCounts[ h ], holeNodes );
        firstPoint += holeCounts[ h ];
        if ( NoNode == first )
            continue;
        const Int32 holeOrientation = RingOrientation( first );
        if ( 0 == holeOrientation )
        {
            m_nodes.resize( first );
            continue;
        }
        if ( holeOrientation > 0 )
            ReverseRing( first );
        SHole hole;
        hole.first = first;
        hole.rightmost = first;
        hole.index = h;
        hole.alive = true;
        for ( UInt32 node=first+1; node<first+holeNodes; ++node )
            if ( LexLess( P( hole.rightmost ), P( node ) ) )
                hole.rightmost = node;
        m_holes.push_back( hole );
    }
    if ( !m_holes.empty() )
    {
        // the input nodes by position and by ring, for holes that share a vertex with the merged ring
        const UInt32 inputNodes = (UInt32) m_nodes.size();
        m_positionOrder.resize( inputNodes );
        m_ringFirst.resize( inputNodes );
        m_merged.assign( inputNodes, 0 );
        for ( UInt32 i=0; i<inputNodes; ++i )
            m_positionOrder[ i ] = i;
        std::sort( m_positionOrder.begin(), m_positionOrder.end(), CNodePositionOrder( m_points, &m_nodes[ 0 ] ) );
        for ( UInt32 i=0; i<outerNodes; ++i )
            m_ringFirst[ m_outerFirst + i ] = m_outerFirst;
        m_merged[ m_outerFirst ] = 1;
        for ( size_t h=0; h<m_holes.size(); ++h )
        {
            UInt32 node = m_holes[ h ].first;
            do
            {
                m_ringFirst[ node ] = m_holes[ h ].first;
                node = m_nodes[ node ].next;
            }
            while ( node != m_holes[ h ].first );
        }
        std::sort( m_holes.begin(), m_holes.end(), CHoleOrder( m_points, &m_nodes[ 0 ] ) );
        for ( size_t h=0; h<m_holes.size(); ++h )
        {
            // a shared vertex first (a bridge elsewhere would leave the touching point as a pinch the ear clipping cannot resolve)
            if ( PinchHoleAtVertex( (UInt32) h ) || BridgeHole( (UInt32) h ) || PinchHole( (UInt32) h ) )
                m_merged[ m_holes[ h ].first ] = 1;
            else
                m_simple = false;
            m_holes[ h ].alive = false;
        }
    }
    return ClipEars( triangles );
}

/*-------------------------------------------------------------------------*/

template < typename T >
TTriangulationStatus
CTPolygonTriangulator< T >::Triangulate( const TPoint* ring, UInt32 count, TIndexVector& triangles )
{
    return Triangulate( ring, count, GUCEF_NULL, 0, triangles );
}

/*-------------------------------------------------------------------------*/

template < typename T >
TTriangulationStatus
CTPolygonTriangulator< T >::Triangulate( const CTPolygon2D< T >& polygon, TIndexVector& triangles )
{
    return Triangulate( polygon.points.empty() ? static_cast< const TPoint* >( GUCEF_NULL ) : &polygon.points[ 0 ], (UInt32) polygon.points.size(), GUCEF_NULL, 0, triangles );
}

/*-------------------------------------------------------------------------*/

template < typename T >
TTriangulationStatus
CTPolygonTriangulator< T >::Triangulate( const CTPolygonWithHoles2D< T >& polygon, TIndexVector& triangles )
{
    m_concatenated.clear();
    m_holeCounts.clear();
    m_concatenated.insert( m_concatenated.end(), polygon.outer.points.begin(), polygon.outer.points.end() );
    for ( size_t h=0; h<polygon.holes.size(); ++h )
    {
        m_concatenated.insert( m_concatenated.end(), polygon.holes[ h ].points.begin(), polygon.holes[ h ].points.end() );
        m_holeCounts.push_back( (UInt32) polygon.holes[ h ].points.size() );
    }
    return Triangulate( m_concatenated.empty() ? static_cast< const TPoint* >( GUCEF_NULL ) : &m_concatenated[ 0 ], (UInt32) polygon.outer.points.size(),
                        m_holeCounts.empty() ? static_cast< const UInt32* >( GUCEF_NULL ) : &m_holeCounts[ 0 ], (UInt32) m_holeCounts.size(), triangles );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Ear clipping of a polygon (section 7.9 of the design): three point indices per counter-clockwise triangle, see
 *  CTPolygonTriangulator. Allocates its work arrays per call; repeated calls should keep a CTPolygonTriangulator.
 */
template < typename T >
TTriangulationStatus
TriangulatePolygon( const CTPolygon2D< T >& polygon, GUCEF::vector< UInt32 >& triangles )
{
    CTPolygonTriangulator< T > triangulator;
    return triangulator.Triangulate( polygon, triangles );
}

/*-------------------------------------------------------------------------*/

/** As TriangulatePolygon, with holes: the indices count the outer points first, then the points of each hole */
template < typename T >
TTriangulationStatus
TriangulatePolygon( const CTPolygonWithHoles2D< T >& polygon, GUCEF::vector< UInt32 >& triangles )
{
    CTPolygonTriangulator< T > triangulator;
    return triangulator.Triangulate( polygon, triangles );
}

/*-------------------------------------------------------------------------*/

/** As TriangulatePolygon, for a ring of 'count' points */
template < typename T >
TTriangulationStatus
TriangulatePolygon( const MATH::CTPoint2D< T >* ring, UInt32 count, GUCEF::vector< UInt32 >& triangles )
{
    CTPolygonTriangulator< T > triangulator;
    return triangulator.Triangulate( ring, count, triangles );
}

/*-------------------------------------------------------------------------*/

/**
 *  The triangulation of a polygon with holes as triangles (counter-clockwise, convex: usable as support shapes, for
 *  example for the cached triangulation of a plane-hosted polygon, section 7.4.5), with the given triangulator
 */
template < typename T >
TTriangulationStatus
TriangulatePolygon( const CTPolygonWithHoles2D< T >& polygon, GUCEF::vector< CTTriangle2D< T > >& triangles, CTPolygonTriangulator< T >& triangulator, GUCEF::vector< UInt32 >& indices )
{
    const TTriangulationStatus status = triangulator.Triangulate( polygon, indices );
    triangles.clear();
    triangles.reserve( indices.size() / 3 );
    const UInt32 outerCount = (UInt32) polygon.outer.points.size();
    for ( size_t t=0; t+2<indices.size(); t+=3 )
    {
        MATH::CTPoint2D< T > corners[ 3 ];
        for ( UInt32 k=0; k<3; ++k )
        {
            // the index counts the outer points, then the points of each hole
            UInt32 index = indices[ t + k ];
            if ( index < outerCount )
            {
                corners[ k ] = polygon.outer.points[ index ];
                continue;
            }
            index -= outerCount;
            size_t h = 0;
            while ( index >= polygon.holes[ h ].points.size() )
                index -= (UInt32) polygon.holes[ h++ ].points.size();
            corners[ k ] = polygon.holes[ h ].points[ index ];
        }
        triangles.push_back( CTTriangle2D< T >( corners[ 0 ], corners[ 1 ], corners[ 2 ] ) );
    }
    return status;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTPolygonTriangulator< Float32 > TFloat32PolygonTriangulator;
typedef CTPolygonTriangulator< Float64 > TFloat64PolygonTriangulator;
typedef CTPolygonTriangulator< Int32 > TInt32PolygonTriangulator;
typedef CTPolygonTriangulator< MATH::TFixed32_32 > TFixed32_32PolygonTriangulator;

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolygonTriangulator< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolygonTriangulator< Float64 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolygonTriangulator< Int32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolygonTriangulator< MATH::TFixed32_32 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_POLYGONTRIANGULATION_H ? */
