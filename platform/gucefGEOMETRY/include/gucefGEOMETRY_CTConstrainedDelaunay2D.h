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

#ifndef GUCEF_GEOMETRY_CTCONSTRAINEDDELAUNAY2D_H
#define GUCEF_GEOMETRY_CTCONSTRAINEDDELAUNAY2D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
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

/** The outcome of a (constrained) Delaunay triangulation call */
enum EDelaunayStatus
{
    DELAUNAY_OK                 = 0 ,   /**< done */
    DELAUNAY_DEGENERATE         = 1 ,   /**< fewer than three distinct points, or all points collinear: no triangles */
    DELAUNAY_INVALID_INPUT      = 2 ,   /**< a coordinate is not finite, a point array is missing, or a vertex index is out of range */
    DELAUNAY_FAILED             = 3     /**< an iteration cap was reached or a rounded construction could not be inserted: the triangulation stays valid, the item was not inserted */
};
typedef enum EDelaunayStatus TDelaunayStatus;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Constrained Delaunay triangulation of a 2D point set (section 7.9 of the design).
 *
 *  - Points: incremental Bowyer-Watson insertion (in Hilbert curve order for locality) with the exact InCircle of
 *    CTPredicates. The bounding triangle is symbolic: one vertex at infinity joined to every hull edge by a ghost
 *    triangle, so no large coordinates enter the predicates and the convex hull is exact (a point collinear with a hull
 *    edge is outside unless it lies strictly between its end points).
 *  - Tie rule (deterministic): when four points are cocircular the decision is the one of a symbolic perturbation of
 *    the lifted coordinate |p|^2 in which the point first in lexicographic order (x, then y) moves the most. The
 *    triangulation is therefore the unique Delaunay triangulation of the perturbed set: it depends only on the
 *    coordinates, not on the insertion order, the input order or the platform.
 *  - Constraints: edges between vertices, inserted by edge flipping (Sloan 1993): the edges crossed by the segment are
 *    flipped until the segment is an edge, then the new edges are made Delaunay again by Lawson flips that never flip a
 *    constrained edge. A constraint through a vertex is split there; a constraint that crosses an earlier one splits
 *    both at their intersection point, a new (Steiner) vertex. That construction is exact for fraction kinds and rounded
 *    to the kind otherwise (computed in Float64): when the rounded point does not lie in the two triangles of the crossed
 *    edge (integer kinds) it is inserted where it lies, outside the hull too (or the vertex at that position is used, an
 *    end point of the new segment included) and both constraints are routed through it, so each constraint is a chain
 *    within rounding of its segment; only when no insertion is possible the nearer end point of the crossed edge is
 *    used. After a crossed constraint loses its flag the Lawson flips run before the next segment, so the result stays
 *    constrained Delaunay. A rounded crossing within two rounding steps of a vertex (floating and fixed point kinds), and
 *    a vertex of the crossed edge within two rounding steps of the new segment, are used as the split vertex: segments
 *    that (nearly) overlap a rerouted constraint would otherwise cross it again and again. A repeated constraint is
 *    inserted once.
 *  - Result: every non-constrained edge is locally Delaunay (exact InCircle <= 0), triangles are counter-clockwise and
 *    tile the convex hull. GetTriangles lists them in a canonical order (each triangle rotated to start at its smallest
 *    vertex index, the list sorted), so equal inputs give bit-identical output on every platform. Exact duplicate points
 *    map to the first of them (GetVertexOfInput); vertex indices equal input indices, Steiner vertices follow.
 *  - Kinds: every kind of CTPredicates. Float32, Float64, Int32 and TFixed32_32 are exported
 *    (gucefGEOMETRY_CTConstrainedDelaunay2D.cpp).
 *  - Cost: about O( n log n ) for the points; a constraint costs the number of edges it crosses (squared in the worst
 *    case of Sloan's flipping).
 *  - Allocation: the object keeps its arrays and work arrays and reuses them; Build allocates only when the input is
 *    larger than an earlier one. Not realtime-safe; one thread at a time; not copyable.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTConstrainedDelaunay2D
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint2D< T > TPoint;
    typedef CTPredicates< T > TPredicates;
    typedef GUCEF::vector< TPoint > TPointVector;
    typedef GUCEF::vector< UInt32 > TIndexVector;

    enum { NoVertex = 0xFFFFFFFFU };

    CTConstrainedDelaunay2D( void );

    /**
     *  Triangulates 'count' points, then inserts 'constraintCount' constraint edges given as pairs of point indices in
     *  'constraints' (2 * constraintCount values). Replaces any earlier triangulation. A constraint between two equal
     *  points is ignored, and so is a repeat of an earlier constraint (the same two points, in either order);
     *  DELAUNAY_FAILED when a constraint could not be inserted (the others are).
     */
    TDelaunayStatus Build( const TPoint* points, UInt32 count, const UInt32* constraints = GUCEF_NULL, UInt32 constraintCount = 0 );

    /** Inserts the constraint edge between two vertices of the triangulation (nothing to do when it was inserted before) */
    TDelaunayStatus InsertConstraint( UInt32 a, UInt32 b );

    /** Inserts a point (outside the hull too); 'vertex' receives its vertex index, that of an equal vertex when there is one */
    TDelaunayStatus InsertPoint( const TPoint& point, UInt32& vertex );

    /** Removes everything (the arrays keep their capacity) */
    void Clear( void );

    /** The status of the last Build */
    GUCEF_GEOMETRY_FORCE_INLINE TDelaunayStatus GetStatus( void ) const { return m_status; }

    /** The vertices: the input points (duplicates included, but not referenced by triangles), then the Steiner and inserted points */
    GUCEF_GEOMETRY_FORCE_INLINE const TPointVector& GetVertices( void ) const { return m_vertices; }

    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetVertexCount( void ) const { return (UInt32) m_vertices.size(); }

    /** The number of vertices created at constraint crossings */
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetSteinerPointCount( void ) const { return m_steinerCount; }

    /** The vertex that represents input point 'index' (the first of equal points); NoVertex for an index out of range */
    UInt32 GetVertexOfInput( UInt32 index ) const;

    /** The triangles in canonical order: three vertex indices each, counter-clockwise, rotated to start at the smallest */
    const TIndexVector& GetTriangles( void ) const;

    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetTriangleCount( void ) const { return (UInt32) GetTriangles().size() / 3; }

    /** The constrained edges as sorted pairs ( a < b ), in canonical order */
    const TIndexVector& GetConstrainedEdges( void ) const;

    /** True when a and b are joined by an edge */
    bool HasEdge( UInt32 a, UInt32 b ) const;

    /** True when a and b are joined by a constrained edge */
    bool IsConstrained( UInt32 a, UInt32 b ) const;

    private:

    struct STriangle
    {
        UInt32 v[ 3 ];      /**< counter-clockwise; a ghost triangle holds the vertex at infinity in v[ 2 ] */
        UInt32 n[ 3 ];      /**< n[ k ] is across the edge opposite v[ k ] */
        UInt8 constrained;  /**< bit k: the edge opposite v[ k ] is constrained */
        bool alive;
    };

    struct SBoundary
    {
        UInt32 a;
        UInt32 b;
        UInt32 outside;
        bool constrained;
    };

    struct SEdge
    {
        UInt32 a;
        UInt32 b;
    };

    /** Orders vertex indices by their coordinates (x, then y), then by index */
    class CLexicographic
    {
        public:

        explicit CLexicographic( const TPoint* points ) : m_points( points ) {}

        bool operator()( UInt32 a, UInt32 b ) const
        {
            const TPoint& pa = m_points[ a ];
            const TPoint& pb = m_points[ b ];
            if ( pa.x < pb.x ) return true;
            if ( pb.x < pa.x ) return false;
            if ( pa.y < pb.y ) return true;
            if ( pb.y < pa.y ) return false;
            return a < b;
        }

        private:

        const TPoint* m_points;
    };

    /** A constraint as its sorted vertex pair, then its position in the input */
    struct SConstraintKey
    {
        UInt64 key;
        UInt32 index;

        bool operator<( const SConstraintKey& other ) const { return key < other.key || ( key == other.key && index < other.index ); }
    };

    static GUCEF_GEOMETRY_FORCE_INLINE UInt64 ConstraintKey( UInt32 a, UInt32 b ) { return a < b ? ( (UInt64) a << 32 ) | b : ( (UInt64) b << 32 ) | a; }

    /** A sort key: Hilbert index, then vertex index */
    struct SOrderKey
    {
        UInt32 key;
        UInt32 vertex;

        bool operator<( const SOrderKey& other ) const { return key < other.key || ( key == other.key && vertex < other.vertex ); }
    };

    struct STriangleKey
    {
        UInt32 v[ 3 ];

        bool operator<( const STriangleKey& other ) const
        {
            if ( v[ 0 ] != other.v[ 0 ] ) return v[ 0 ] < other.v[ 0 ];
            if ( v[ 1 ] != other.v[ 1 ] ) return v[ 1 ] < other.v[ 1 ];
            return v[ 2 ] < other.v[ 2 ];
        }
    };

    enum { Ghost = 0xFFFFFFFEU, NoTriangle = 0xFFFFFFFFU };

    CTConstrainedDelaunay2D( const CTConstrainedDelaunay2D& src );              /**< not implemented */
    CTConstrainedDelaunay2D& operator=( const CTConstrainedDelaunay2D& src );   /**< not implemented */

    static GUCEF_GEOMETRY_FORCE_INLINE bool SamePosition( const TPoint& a, const TPoint& b ) { return a.x == b.x && a.y == b.y; }

    static GUCEF_GEOMETRY_FORCE_INLINE bool LexLess( const TPoint& a, const TPoint& b ) { return a.x < b.x || ( a.x == b.x && a.y < b.y ); }

    /**
     *  True when the rounded construction c is at most two rounding steps of the kind from the vertex position p in each
     *  coordinate (floating point: the epsilon relative to the larger magnitude, fixed point: one raw unit) or equal to
     *  it (integer and fraction kinds)
     */
    static bool WithinRoundingStep( const TPoint& c, const TPoint& p );

    /**
     *  True when the vertex position p lies within two rounding steps of the kind (as in WithinRoundingStep, the larger
     *  of the two coordinates) of the line through a and b and projects strictly between them; 'distance' receives
     *  the distance. Always false for integer and fraction kinds. A Steiner vertex on a rerouted constraint lies that
     *  close to the segment it was rounded from.
     */
    static bool NearSegment( const TPoint& p, const TPoint& a, const TPoint& b, Float64& distance );

    /** For collinear points: true when p lies strictly between a and b */
    static bool StrictlyBetween( const TPoint& a, const TPoint& p, const TPoint& b );

    static UInt32 HilbertIndex( UInt32 x, UInt32 y );

    GUCEF_GEOMETRY_FORCE_INLINE const TPoint& V( UInt32 vertex ) const { return m_vertices[ vertex ]; }

    GUCEF_GEOMETRY_FORCE_INLINE bool IsGhost( UInt32 triangle ) const { return Ghost == m_triangles[ triangle ].v[ 2 ]; }

    /** The InCircle sign of d for the counter-clockwise a, b, c with the symbolic perturbation of the tie rule */
    Int32 PerturbedInCircle( UInt32 a, UInt32 b, UInt32 c, UInt32 d ) const;

    /** True when the circumcircle of the triangle (the outer half plane of a ghost) holds the point strictly */
    bool InConflict( UInt32 triangle, UInt32 vertex ) const;

    UInt32 NewTriangle( UInt32 a, UInt32 b, UInt32 c );

    void FreeTriangle( UInt32 triangle );

    /** The local index k of the edge from -> to (the edge opposite v[ k ]); 3 when the triangle does not hold it */
    UInt32 EdgeIndex( UInt32 triangle, UInt32 from, UInt32 to ) const;

    /** Finds the triangle holding the directed edge from -> to; false when the vertices are not joined */
    bool FindEdge( UInt32 from, UInt32 to, UInt32& triangle, UInt32& k ) const;

    /** A triangle whose closed area holds the point (a ghost in conflict when it is outside the hull); NoTriangle on failure */
    UInt32 Locate( const TPoint& point );

    /**
     *  Inserts the vertex by Bowyer-Watson from 'start' (a triangle in conflict); with 'ghostsOnly' only ghost
     *  triangles join the cavity (a point outside the hull of a constrained triangulation). False (and no change) when
     *  the cavity is not visible from the vertex.
     */
    bool InsertVertexBowyerWatson( UInt32 vertex, UInt32 start, bool ghostsOnly );

    /**
     *  Inserts the vertex that lies in the closed triangle 'start' (or in its outer half plane for a ghost) by splitting
     *  that triangle, or the edge that holds the vertex (the halves of a constrained edge stay constrained), then Lawson
     *  flips. The edges from 'constrainA' and 'constrainB' to the vertex (NoVertex for none) are constrained before the
     *  flips. False (and no change) when the vertex cannot be inserted there.
     */
    bool InsertVertexInTriangle( UInt32 vertex, UInt32 start, UInt32 constrainA, UInt32 constrainB );

    void SetNeighbour( UInt32 triangle, UInt32 from, UInt32 to, UInt32 neighbour );

    void SetConstrained( UInt32 a, UInt32 b );

    /** Flips the edge opposite v[ k ] of the triangle (the caller checked the quad is strictly convex) */
    void Flip( UInt32 triangle, UInt32 k );

    /** Lawson flips from the edges on the stack until every non-constrained edge is locally Delaunay */
    void Legalize( void );

    void PushEdge( UInt32 a, UInt32 b );

    /** The rounded intersection point of the segments a-b and c-d (they cross properly) */
    TPoint Intersection( const TPoint& a, const TPoint& b, const TPoint& c, const TPoint& d ) const;

    TDelaunayStatus InsertSegment( UInt32 u, UInt32 v );

    TDelaunayStatus Initialize( void );

    void Invalidate( void ) { m_canonicalValid = false; }

    TPointVector m_vertices;
    TIndexVector m_inputToVertex;
    GUCEF::vector< STriangle > m_triangles;
    TIndexVector m_freeTriangles;
    TIndexVector m_vertexTriangle;
    UInt32 m_lastTriangle;
    UInt32 m_walkCounter;
    UInt32 m_inputCount;
    UInt32 m_steinerCount;
    TDelaunayStatus m_status;

    TIndexVector m_stamp;
    UInt32 m_stampValue;
    TIndexVector m_stack;
    TIndexVector m_cavity;
    GUCEF::vector< SBoundary > m_boundary;
    TIndexVector m_startSlot;
    UInt32 m_ghostStartSlot;
    TIndexVector m_newTriangles;
    GUCEF::vector< SEdge > m_edgeStack;
    GUCEF::vector< SEdge > m_crossed;
    GUCEF::vector< SEdge > m_newEdges;
    GUCEF::vector< SEdge > m_pending;
    GUCEF::vector< SOrderKey > m_order;
    TIndexVector m_sorted;
    GUCEF::vector< SConstraintKey > m_constraintOrder;
    TIndexVector m_constraintSkip;
    GUCEF::vector< UInt64 > m_constraintKeys;      /**< the inserted constraints, sorted */

    mutable bool m_canonicalValid;
    mutable TIndexVector m_canonicalTriangles;
    mutable TIndexVector m_canonicalEdges;
    mutable GUCEF::vector< STriangleKey > m_keys;
};

/*-------------------------------------------------------------------------*/

/**
 *  The construction of a crossing point: exact in the kind for fraction kinds, otherwise in Float64 and rounded to the
 *  kind (ties to even)
 */
template < typename T, int Exact = MATH::CTNumericTraits< T >::IsExact && !MATH::CTNumericTraits< T >::IsInteger ? 1 : 0 >
class CTDelaunayIntersection
{
    public:

    static MATH::CTPoint2D< T > Compute( const MATH::CTPoint2D< T >& a, const MATH::CTPoint2D< T >& b, const MATH::CTPoint2D< T >& c, const MATH::CTPoint2D< T >& d )
    {
        typedef MATH::CTNumericTraits< T > TTraits;
        const Float64 ax = TTraits::ToFloat64( a.x ), ay = TTraits::ToFloat64( a.y );
        const Float64 bx = TTraits::ToFloat64( b.x ), by = TTraits::ToFloat64( b.y );
        const Float64 cx = TTraits::ToFloat64( c.x ), cy = TTraits::ToFloat64( c.y );
        const Float64 dx = TTraits::ToFloat64( d.x ), dy = TTraits::ToFloat64( d.y );
        const Float64 rx = bx - ax, ry = by - ay, sx = dx - cx, sy = dy - cy;
        const Float64 denominator = rx * sy - ry * sx;
        Float64 t = 0.0 != denominator ? ( ( cx - ax ) * sy - ( cy - ay ) * sx ) / denominator : 0.5;
        t = t < 0.0 ? 0.0 : ( t > 1.0 ? 1.0 : t );
        MATH::CTPoint2D< T > result;
        TTraits::FromFloat64( ax + t * rx, CORE::ROUNDINGMODE_HALF_EVEN, result.x );
        TTraits::FromFloat64( ay + t * ry, CORE::ROUNDINGMODE_HALF_EVEN, result.y );
        return result;
    }
};

template < typename T >
class CTDelaunayIntersection< T, 1 >
{
    public:

    static MATH::CTPoint2D< T > Compute( const MATH::CTPoint2D< T >& a, const MATH::CTPoint2D< T >& b, const MATH::CTPoint2D< T >& c, const MATH::CTPoint2D< T >& d )
    {
        const T rx = b.x - a.x, ry = b.y - a.y, sx = d.x - c.x, sy = d.y - c.y;
        const T denominator = rx * sy - ry * sx;
        const T t = ( ( c.x - a.x ) * sy - ( c.y - a.y ) * sx ) / denominator;
        return MATH::CTPoint2D< T >( a.x + t * rx, a.y + t * ry );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
CTConstrainedDelaunay2D< T >::CTConstrainedDelaunay2D( void )
    : m_vertices()
    , m_inputToVertex()
    , m_triangles()
    , m_freeTriangles()
    , m_vertexTriangle()
    , m_lastTriangle( NoTriangle )
    , m_walkCounter( 0 )
    , m_inputCount( 0 )
    , m_steinerCount( 0 )
    , m_status( DELAUNAY_DEGENERATE )
    , m_stamp()
    , m_stampValue( 0 )
    , m_stack()
    , m_cavity()
    , m_boundary()
    , m_startSlot()
    , m_ghostStartSlot( NoTriangle )
    , m_newTriangles()
    , m_edgeStack()
    , m_crossed()
    , m_newEdges()
    , m_pending()
    , m_order()
    , m_sorted()
    , m_constraintOrder()
    , m_constraintSkip()
    , m_constraintKeys()
    , m_canonicalValid( false )
    , m_canonicalTriangles()
    , m_canonicalEdges()
    , m_keys()
{
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTConstrainedDelaunay2D< T >::Clear( void )
{
    m_vertices.clear();
    m_inputToVertex.clear();
    m_triangles.clear();
    m_freeTriangles.clear();
    m_vertexTriangle.clear();
    m_stamp.clear();
    m_startSlot.clear();
    m_constraintKeys.clear();
    m_lastTriangle = NoTriangle;
    m_walkCounter = 0;
    m_inputCount = 0;
    m_steinerCount = 0;
    m_stampValue = 0;
    m_status = DELAUNAY_DEGENERATE;
    Invalidate();
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTConstrainedDelaunay2D< T >::StrictlyBetween( const TPoint& a, const TPoint& p, const TPoint& b )
{
    if ( SamePosition( p, a ) || SamePosition( p, b ) )
        return false;
    return !( p.x < a.x && p.x < b.x ) && !( a.x < p.x && b.x < p.x ) && !( p.y < a.y && p.y < b.y ) && !( a.y < p.y && b.y < p.y );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTConstrainedDelaunay2D< T >::WithinRoundingStep( const TPoint& c, const TPoint& p )
{
    if ( SamePosition( c, p ) )
        return true;
    if ( TTraits::IsInteger || ( !TTraits::IsFloatingPoint && !TTraits::IsFixedPoint ) )
        return false;
    const Float64 cx = TTraits::ToFloat64( c.x ), cy = TTraits::ToFloat64( c.y );
    const Float64 px = TTraits::ToFloat64( p.x ), py = TTraits::ToFloat64( p.y );
    const Float64 epsilon = 2.0 * TTraits::ToFloat64( TTraits::Epsilon() );
    Float64 stepX = epsilon, stepY = epsilon;
    if ( TTraits::IsFloatingPoint )
    {
        stepX = epsilon * ( fabs( cx ) > fabs( px ) ? fabs( cx ) : fabs( px ) );
        stepY = epsilon * ( fabs( cy ) > fabs( py ) ? fabs( cy ) : fabs( py ) );
    }
    return fabs( cx - px ) <= stepX && fabs( cy - py ) <= stepY;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTConstrainedDelaunay2D< T >::NearSegment( const TPoint& p, const TPoint& a, const TPoint& b, Float64& distance )
{
    distance = 0.0;
    if ( TTraits::IsInteger || ( !TTraits::IsFloatingPoint && !TTraits::IsFixedPoint ) )
        return false;
    const Float64 px = TTraits::ToFloat64( p.x ), py = TTraits::ToFloat64( p.y );
    const Float64 ax = TTraits::ToFloat64( a.x ), ay = TTraits::ToFloat64( a.y );
    const Float64 dx = TTraits::ToFloat64( b.x ) - ax, dy = TTraits::ToFloat64( b.y ) - ay;
    const Float64 length2 = dx * dx + dy * dy;
    if ( !( length2 > 0.0 ) )
        return false;
    const Float64 along = ( px - ax ) * dx + ( py - ay ) * dy;
    if ( !( along > 0.0 ) || !( along < length2 ) )
        return false;
    distance = fabs( ( px - ax ) * dy - ( py - ay ) * dx ) / sqrt( length2 );
    const Float64 epsilon = 2.0 * TTraits::ToFloat64( TTraits::Epsilon() );
    Float64 step = epsilon;
    if ( TTraits::IsFloatingPoint )
        step = epsilon * ( fabs( px ) > fabs( py ) ? fabs( px ) : fabs( py ) );
    return distance <= step;
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTConstrainedDelaunay2D< T >::HilbertIndex( UInt32 x, UInt32 y )
{
    // the classic conversion of a cell of a 2^16 grid to its distance along the Hilbert curve
    UInt32 d = 0;
    for ( UInt32 s=1u << 15; s>0; s>>=1 )
    {
        const UInt32 rx = ( x & s ) ? 1u : 0u;
        const UInt32 ry = ( y & s ) ? 1u : 0u;
        d += s * s * ( ( 3u * rx ) ^ ry );
        if ( 0 == ry )
        {
            if ( 1 == rx )
            {
                x = 0xFFFFu - x;
                y = 0xFFFFu - y;
            }
            const UInt32 swap = x;
            x = y;
            y = swap;
        }
    }
    return d;
}

/*-------------------------------------------------------------------------*/

template < typename T >
Int32
CTConstrainedDelaunay2D< T >::PerturbedInCircle( UInt32 a, UInt32 b, UInt32 c, UInt32 d ) const
{
    const Int32 sign = TPredicates::InCircle( V( a ), V( b ), V( c ), V( d ) );
    if ( 0 != sign )
        return sign;
    /*
     *  Cocircular: the lift of each point moves by eps^rank, the lexicographically first point the most. The determinant
     *  is linear in each lift with the cofactors +Orient( b, c, d ), -Orient( a, c, d ), +Orient( a, b, d ), -Orient( a, b, c )
     *  for a, b, c, d; the term of the first point decides (three distinct points of a circle are never collinear).
     */
    UInt32 order[ 4 ] = { a, b, c, d };
    UInt32 position[ 4 ] = { 0, 1, 2, 3 };
    for ( UInt32 i=1; i<4; ++i )
        for ( UInt32 j=i; j>0 && LexLess( V( order[ j ] ), V( order[ j - 1 ] ) ); --j )
        {
            const UInt32 swapVertex = order[ j ]; order[ j ] = order[ j - 1 ]; order[ j - 1 ] = swapVertex;
            const UInt32 swapPosition = position[ j ]; position[ j ] = position[ j - 1 ]; position[ j - 1 ] = swapPosition;
        }
    for ( UInt32 i=0; i<4; ++i )
    {
        Int32 cofactor = 0;
        switch ( position[ i ] )
        {
            case 0: cofactor = TPredicates::Orient2D( V( b ), V( c ), V( d ) ); break;
            case 1: cofactor = -TPredicates::Orient2D( V( a ), V( c ), V( d ) ); break;
            case 2: cofactor = TPredicates::Orient2D( V( a ), V( b ), V( d ) ); break;
            default: cofactor = -TPredicates::Orient2D( V( a ), V( b ), V( c ) ); break;
        }
        if ( 0 != cofactor )
            return cofactor;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTConstrainedDelaunay2D< T >::InConflict( UInt32 triangle, UInt32 vertex ) const
{
    const STriangle& t = m_triangles[ triangle ];
    if ( Ghost == t.v[ 2 ] )
    {
        const Int32 side = TPredicates::Orient2D( V( t.v[ 0 ] ), V( t.v[ 1 ] ), V( vertex ) );
        if ( 0 != side )
            return side > 0;
        return StrictlyBetween( V( t.v[ 0 ] ), V( vertex ), V( t.v[ 1 ] ) );
    }
    return PerturbedInCircle( t.v[ 0 ], t.v[ 1 ], t.v[ 2 ], vertex ) > 0;
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTConstrainedDelaunay2D< T >::NewTriangle( UInt32 a, UInt32 b, UInt32 c )
{
    UInt32 index;
    if ( !m_freeTriangles.empty() )
    {
        index = m_freeTriangles.back();
        m_freeTriangles.pop_back();
    }
    else
    {
        index = (UInt32) m_triangles.size();
        m_triangles.push_back( STriangle() );
        m_stamp.push_back( 0 );
    }
    STriangle& t = m_triangles[ index ];
    // the vertex at infinity always in v[ 2 ]
    if ( Ghost == a ) { a = b; b = c; c = Ghost; }
    else
    if ( Ghost == b ) { b = a; a = c; c = Ghost; }
    t.v[ 0 ] = a;
    t.v[ 1 ] = b;
    t.v[ 2 ] = c;
    t.n[ 0 ] = t.n[ 1 ] = t.n[ 2 ] = NoTriangle;
    t.constrained = 0;
    t.alive = true;
    if ( Ghost != a ) m_vertexTriangle[ a ] = index;
    if ( Ghost != b ) m_vertexTriangle[ b ] = index;
    if ( Ghost != c ) m_vertexTriangle[ c ] = index;
    return index;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTConstrainedDelaunay2D< T >::FreeTriangle( UInt32 triangle )
{
    m_triangles[ triangle ].alive = false;
    m_freeTriangles.push_back( triangle );
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTConstrainedDelaunay2D< T >::EdgeIndex( UInt32 triangle, UInt32 from, UInt32 to ) const
{
    const STriangle& t = m_triangles[ triangle ];
    for ( UInt32 k=0; k<3; ++k )
        if ( t.v[ ( k + 1 ) % 3 ] == from && t.v[ ( k + 2 ) % 3 ] == to )
            return k;
    return 3;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTConstrainedDelaunay2D< T >::SetNeighbour( UInt32 triangle, UInt32 from, UInt32 to, UInt32 neighbour )
{
    const UInt32 k = EdgeIndex( triangle, from, to );
    GUCEF_GEOMETRY_ASSERT( k < 3 );
    if ( k < 3 )
        m_triangles[ triangle ].n[ k ] = neighbour;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTConstrainedDelaunay2D< T >::FindEdge( UInt32 from, UInt32 to, UInt32& triangle, UInt32& k ) const
{
    if ( from >= m_vertexTriangle.size() || NoTriangle == m_vertexTriangle[ from ] )
        return false;
    const UInt32 start = m_vertexTriangle[ from ];
    UInt32 t = start;
    const size_t cap = m_triangles.size() + 1;
    for ( size_t step=0; step<cap; ++step )
    {
        const STriangle& tri = m_triangles[ t ];
        UInt32 i = 0;
        while ( i < 3 && tri.v[ i ] != from )
            ++i;
        if ( 3 == i )
            return false;
        // the edge from -> v[ i + 1 ] is opposite v[ i + 2 ]
        if ( tri.v[ ( i + 1 ) % 3 ] == to )
        {
            triangle = t;
            k = ( i + 2 ) % 3;
            return true;
        }
        // counter-clockwise around 'from': across the edge v[ i + 2 ] -> from, opposite v[ i + 1 ]
        t = tri.n[ ( i + 1 ) % 3 ];
        if ( t == start || NoTriangle == t )
            return false;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTConstrainedDelaunay2D< T >::HasEdge( UInt32 a, UInt32 b ) const
{
    UInt32 t = 0, k = 0;
    return a != b && FindEdge( a, b, t, k );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTConstrainedDelaunay2D< T >::IsConstrained( UInt32 a, UInt32 b ) const
{
    UInt32 t = 0, k = 0;
    return a != b && FindEdge( a, b, t, k ) && 0 != ( m_triangles[ t ].constrained & ( 1u << k ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTConstrainedDelaunay2D< T >::SetConstrained( UInt32 a, UInt32 b )
{
    UInt32 t = 0, k = 0;
    if ( FindEdge( a, b, t, k ) )
        m_triangles[ t ].constrained |= (UInt8) ( 1u << k );
    if ( FindEdge( b, a, t, k ) )
        m_triangles[ t ].constrained |= (UInt8) ( 1u << k );
    Invalidate();
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTConstrainedDelaunay2D< T >::Locate( const TPoint& point )
{
    if ( m_triangles.empty() )
        return NoTriangle;
    UInt32 t = m_lastTriangle;
    if ( NoTriangle == t || t >= m_triangles.size() || !m_triangles[ t ].alive )
    {
        t = 0;
        while ( t < m_triangles.size() && !m_triangles[ t ].alive )
            ++t;
        if ( t == m_triangles.size() )
            return NoTriangle;
    }
    const size_t cap = 4 * m_triangles.size() + 64;
    for ( size_t step=0; step<cap; ++step )
    {
        const STriangle& tri = m_triangles[ t ];
        if ( Ghost == tri.v[ 2 ] )
        {
            const Int32 side = TPredicates::Orient2D( V( tri.v[ 0 ] ), V( tri.v[ 1 ] ), point );
            if ( side > 0 || ( 0 == side && StrictlyBetween( V( tri.v[ 0 ] ), point, V( tri.v[ 1 ] ) ) ) )
                return t;
            t = tri.n[ 2 ];
            continue;
        }
        // a deterministic rotation of the first edge tested avoids cycling in triangulations that are not Delaunay
        const UInt32 rotation = ( m_walkCounter++ ) % 3;
        bool moved = false;
        for ( UInt32 j=0; j<3 && !moved; ++j )
        {
            const UInt32 k = ( rotation + j ) % 3;
            if ( TPredicates::Orient2D( V( tri.v[ ( k + 1 ) % 3 ] ), V( tri.v[ ( k + 2 ) % 3 ] ), point ) < 0 )
            {
                t = tri.n[ k ];
                moved = true;
            }
        }
        if ( !moved )
            return t;
    }
    // the walk did not end: scan
    for ( UInt32 i=0; i<(UInt32) m_triangles.size(); ++i )
    {
        const STriangle& tri = m_triangles[ i ];
        if ( !tri.alive )
            continue;
        if ( Ghost == tri.v[ 2 ] )
        {
            const Int32 side = TPredicates::Orient2D( V( tri.v[ 0 ] ), V( tri.v[ 1 ] ), point );
            if ( side > 0 || ( 0 == side && StrictlyBetween( V( tri.v[ 0 ] ), point, V( tri.v[ 1 ] ) ) ) )
                return i;
            continue;
        }
        if ( TPredicates::Orient2D( V( tri.v[ 0 ] ), V( tri.v[ 1 ] ), point ) >= 0 && TPredicates::Orient2D( V( tri.v[ 1 ] ), V( tri.v[ 2 ] ), point ) >= 0 &&
             TPredicates::Orient2D( V( tri.v[ 2 ] ), V( tri.v[ 0 ] ), point ) >= 0 )
            return i;
    }
    return NoTriangle;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTConstrainedDelaunay2D< T >::InsertVertexBowyerWatson( UInt32 vertex, UInt32 start, bool ghostsOnly )
{
    m_stampValue += 2;
    if ( m_stampValue < 2 )
    {
        // the stamps wrapped: start again
        std::fill( m_stamp.begin(), m_stamp.end(), 0u );
        m_stampValue = 2;
    }
    const UInt32 inside = m_stampValue;
    const UInt32 outside = m_stampValue + 1;
    m_cavity.clear();
    m_stack.clear();
    m_boundary.clear();
    m_cavity.push_back( start );
    m_stack.push_back( start );
    m_stamp[ start ] = inside;
    while ( !m_stack.empty() )
    {
        const UInt32 t = m_stack.back();
        m_stack.pop_back();
        for ( UInt32 k=0; k<3; ++k )
        {
            const STriangle& tri = m_triangles[ t ];
            const UInt32 nb = tri.n[ k ];
            if ( m_stamp[ nb ] == inside )
                continue;
            if ( m_stamp[ nb ] != outside )
            {
                if ( ( !ghostsOnly || IsGhost( nb ) ) && InConflict( nb, vertex ) )
                {
                    m_stamp[ nb ] = inside;
                    m_cavity.push_back( nb );
                    m_stack.push_back( nb );
                    continue;
                }
                m_stamp[ nb ] = outside;
            }
            SBoundary edge;
            edge.a = tri.v[ ( k + 1 ) % 3 ];
            edge.b = tri.v[ ( k + 2 ) % 3 ];
            edge.outside = nb;
            edge.constrained = 0 != ( tri.constrained & ( 1u << k ) );
            m_boundary.push_back( edge );
        }
    }
    // the cavity must be star shaped from the vertex
    for ( size_t i=0; i<m_boundary.size(); ++i )
    {
        const SBoundary& e = m_boundary[ i ];
        if ( Ghost != e.a && Ghost != e.b && TPredicates::Orient2D( V( e.a ), V( e.b ), V( vertex ) ) <= 0 )
        {
            for ( size_t c=0; c<m_cavity.size(); ++c )
                m_stamp[ m_cavity[ c ] ] = 0;
            return false;
        }
    }
    for ( size_t c=0; c<m_cavity.size(); ++c )
        FreeTriangle( m_cavity[ c ] );
    if ( m_startSlot.size() < m_vertices.size() )
        m_startSlot.resize( m_vertices.size(), NoTriangle );
    m_newTriangles.clear();
    for ( size_t i=0; i<m_boundary.size(); ++i )
    {
        const SBoundary& e = m_boundary[ i ];
        const UInt32 t = NewTriangle( e.a, e.b, vertex );
        m_stamp[ t ] = 0;
        STriangle& tri = m_triangles[ t ];
        const UInt32 k = EdgeIndex( t, e.a, e.b );
        tri.n[ k ] = e.outside;
        if ( e.constrained )
            tri.constrained |= (UInt8) ( 1u << k );
        SetNeighbour( e.outside, e.b, e.a, t );
        if ( Ghost == e.a ) m_ghostStartSlot = t; else m_startSlot[ e.a ] = t;
        m_newTriangles.push_back( t );
    }
    for ( size_t i=0; i<m_boundary.size(); ++i )
    {
        const SBoundary& e = m_boundary[ i ];
        const UInt32 t = m_newTriangles[ i ];
        const UInt32 next = Ghost == e.b ? m_ghostStartSlot : m_startSlot[ e.b ];
        SetNeighbour( t, e.b, vertex, next );
        SetNeighbour( next, vertex, e.b, t );
    }
    for ( size_t i=0; i<m_boundary.size(); ++i )
    {
        const SBoundary& e = m_boundary[ i ];
        if ( Ghost == e.a ) m_ghostStartSlot = NoTriangle; else m_startSlot[ e.a ] = NoTriangle;
    }
    m_lastTriangle = m_newTriangles.back();
    m_vertexTriangle[ vertex ] = m_newTriangles.back();
    Invalidate();
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTConstrainedDelaunay2D< T >::InsertVertexInTriangle( UInt32 vertex, UInt32 start, UInt32 constrainA, UInt32 constrainB )
{
    const TPoint& p = V( vertex );
    if ( IsGhost( start ) )
    {
        // outside the hull: the visible hull edges become triangles with the vertex
        if ( !InsertVertexBowyerWatson( vertex, start, true ) )
            return false;
        for ( size_t i=0; i<m_boundary.size(); ++i )
            if ( Ghost != m_boundary[ i ].a && Ghost != m_boundary[ i ].b )
                PushEdge( m_boundary[ i ].a, m_boundary[ i ].b );
    }
    else
    {
        const STriangle t0 = m_triangles[ start ];
        UInt32 edge = 3;
        for ( UInt32 k=0; k<3; ++k )
        {
            if ( SamePosition( V( t0.v[ k ] ), p ) )
                return false;
            const Int32 side = TPredicates::Orient2D( V( t0.v[ ( k + 1 ) % 3 ] ), V( t0.v[ ( k + 2 ) % 3 ] ), p );
            if ( side < 0 )
                return false;
            if ( 0 == side )
                edge = k;
        }
        if ( 3 == edge )
        {
            // strictly inside: three triangles
            const UInt32 a = t0.v[ 0 ], b = t0.v[ 1 ], c = t0.v[ 2 ];
            FreeTriangle( start );
            const UInt32 tab = NewTriangle( a, b, vertex );
            const UInt32 tbc = NewTriangle( b, c, vertex );
            const UInt32 tca = NewTriangle( c, a, vertex );
            const UInt32 corner[ 3 ] = { tbc, tca, tab };     // the new triangle on the edge opposite v[ k ]
            for ( UInt32 k=0; k<3; ++k )
            {
                const UInt32 x = t0.v[ ( k + 1 ) % 3 ], y = t0.v[ ( k + 2 ) % 3 ];
                const UInt32 t = corner[ k ];
                SetNeighbour( t, x, y, t0.n[ k ] );
                SetNeighbour( t0.n[ k ], y, x, t );
                if ( 0 != ( t0.constrained & ( 1u << k ) ) )
                    m_triangles[ t ].constrained |= (UInt8) ( 1u << EdgeIndex( t, x, y ) );
            }
            SetNeighbour( tab, b, vertex, tbc ); SetNeighbour( tbc, vertex, b, tab );
            SetNeighbour( tbc, c, vertex, tca ); SetNeighbour( tca, vertex, c, tbc );
            SetNeighbour( tca, a, vertex, tab ); SetNeighbour( tab, vertex, a, tca );
            PushEdge( a, b );
            PushEdge( b, c );
            PushEdge( c, a );
        }
        else
        {
            // on the edge x -> y: four triangles (two of them ghosts on the hull)
            const UInt32 x = t0.v[ ( edge + 1 ) % 3 ], y = t0.v[ ( edge + 2 ) % 3 ], c = t0.v[ edge ];
            const UInt32 o = t0.n[ edge ];
            const STriangle t1 = m_triangles[ o ];
            const UInt32 kk = EdgeIndex( o, y, x );
            const UInt32 d = t1.v[ kk ];
            const bool constrained = 0 != ( t0.constrained & ( 1u << edge ) );
            const UInt32 nYC = t0.n[ ( edge + 1 ) % 3 ], nCX = t0.n[ ( edge + 2 ) % 3 ];
            const bool cYC = 0 != ( t0.constrained & ( 1u << ( ( edge + 1 ) % 3 ) ) ), cCX = 0 != ( t0.constrained & ( 1u << ( ( edge + 2 ) % 3 ) ) );
            const UInt32 nXD = t1.n[ ( kk + 1 ) % 3 ], nDY = t1.n[ ( kk + 2 ) % 3 ];
            const bool cXD = 0 != ( t1.constrained & ( 1u << ( ( kk + 1 ) % 3 ) ) ), cDY = 0 != ( t1.constrained & ( 1u << ( ( kk + 2 ) % 3 ) ) );
            FreeTriangle( start );
            FreeTriangle( o );
            const UInt32 A = NewTriangle( x, vertex, c );
            const UInt32 B = NewTriangle( vertex, y, c );
            const UInt32 C = NewTriangle( y, vertex, d );
            const UInt32 D = NewTriangle( vertex, x, d );
            SetNeighbour( A, x, vertex, D ); SetNeighbour( D, vertex, x, A );
            SetNeighbour( A, vertex, c, B ); SetNeighbour( B, c, vertex, A );
            SetNeighbour( B, vertex, y, C ); SetNeighbour( C, y, vertex, B );
            SetNeighbour( C, vertex, d, D ); SetNeighbour( D, d, vertex, C );
            SetNeighbour( A, c, x, nCX ); SetNeighbour( nCX, x, c, A );
            SetNeighbour( B, y, c, nYC ); SetNeighbour( nYC, c, y, B );
            SetNeighbour( C, d, y, nDY ); SetNeighbour( nDY, y, d, C );
            SetNeighbour( D, x, d, nXD ); SetNeighbour( nXD, d, x, D );
            if ( cCX ) m_triangles[ A ].constrained |= (UInt8) ( 1u << EdgeIndex( A, c, x ) );
            if ( cYC ) m_triangles[ B ].constrained |= (UInt8) ( 1u << EdgeIndex( B, y, c ) );
            if ( cDY ) m_triangles[ C ].constrained |= (UInt8) ( 1u << EdgeIndex( C, d, y ) );
            if ( cXD ) m_triangles[ D ].constrained |= (UInt8) ( 1u << EdgeIndex( D, x, d ) );
            if ( constrained )
            {
                SetConstrained( x, vertex );
                SetConstrained( vertex, y );
            }
            PushEdge( y, c );
            PushEdge( c, x );
            PushEdge( x, d );
            PushEdge( d, y );
        }
        m_lastTriangle = m_vertexTriangle[ vertex ];
    }
    if ( NoVertex != constrainA )
        SetConstrained( constrainA, vertex );
    if ( NoVertex != constrainB )
        SetConstrained( vertex, constrainB );
    Legalize();
    Invalidate();
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTConstrainedDelaunay2D< T >::Flip( UInt32 triangle, UInt32 k )
{
    // t1 = ( x, y, p1 ) with the edge x -> y opposite p1, t2 = ( y, x, p2 )
    const UInt32 t1 = triangle;
    STriangle& a = m_triangles[ t1 ];
    const UInt32 p1 = a.v[ k ];
    const UInt32 x = a.v[ ( k + 1 ) % 3 ];
    const UInt32 y = a.v[ ( k + 2 ) % 3 ];
    const UInt32 t2 = a.n[ k ];
    STriangle& b = m_triangles[ t2 ];
    const UInt32 kk = EdgeIndex( t2, y, x );
    const UInt32 p2 = b.v[ kk ];
    // the outer neighbours and their constraint flags
    const UInt32 nYP1 = a.n[ ( k + 1 ) % 3 ];      // across y -> p1 (opposite x)
    const UInt32 nP1X = a.n[ ( k + 2 ) % 3 ];      // across p1 -> x (opposite y)
    const bool cYP1 = 0 != ( a.constrained & ( 1u << ( ( k + 1 ) % 3 ) ) );
    const bool cP1X = 0 != ( a.constrained & ( 1u << ( ( k + 2 ) % 3 ) ) );
    const UInt32 nXP2 = b.n[ ( kk + 1 ) % 3 ];     // across x -> p2 (opposite y)
    const UInt32 nP2Y = b.n[ ( kk + 2 ) % 3 ];     // across p2 -> y (opposite x)
    const bool cXP2 = 0 != ( b.constrained & ( 1u << ( ( kk + 1 ) % 3 ) ) );
    const bool cP2Y = 0 != ( b.constrained & ( 1u << ( ( kk + 2 ) % 3 ) ) );
    // t1 = ( p1, x, p2 ), t2 = ( p2, y, p1 )
    a.v[ 0 ] = p1; a.v[ 1 ] = x; a.v[ 2 ] = p2;
    a.n[ 0 ] = nXP2; a.n[ 1 ] = t2; a.n[ 2 ] = nP1X;
    a.constrained = (UInt8) ( ( cXP2 ? 1u : 0u ) | ( cP1X ? 4u : 0u ) );
    b.v[ 0 ] = p2; b.v[ 1 ] = y; b.v[ 2 ] = p1;
    b.n[ 0 ] = nYP1; b.n[ 1 ] = t1; b.n[ 2 ] = nP2Y;
    b.constrained = (UInt8) ( ( cYP1 ? 1u : 0u ) | ( cP2Y ? 4u : 0u ) );
    SetNeighbour( nXP2, p2, x, t1 );
    SetNeighbour( nYP1, p1, y, t2 );
    m_vertexTriangle[ x ] = t1;
    m_vertexTriangle[ p1 ] = t1;
    m_vertexTriangle[ y ] = t2;
    m_vertexTriangle[ p2 ] = t2;
    m_lastTriangle = t1;
    Invalidate();
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTConstrainedDelaunay2D< T >::PushEdge( UInt32 a, UInt32 b )
{
    SEdge e;
    e.a = a;
    e.b = b;
    m_edgeStack.push_back( e );
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTConstrainedDelaunay2D< T >::Legalize( void )
{
    size_t flips = 0;
    const size_t cap = 64 * m_triangles.size() + 1024;
    while ( !m_edgeStack.empty() && flips < cap )
    {
        const SEdge e = m_edgeStack.back();
        m_edgeStack.pop_back();
        if ( Ghost == e.a || Ghost == e.b )
            continue;
        UInt32 t = 0, k = 0;
        if ( !FindEdge( e.a, e.b, t, k ) )
            continue;
        const STriangle& tri = m_triangles[ t ];
        if ( 0 != ( tri.constrained & ( 1u << k ) ) || Ghost == tri.v[ 2 ] )
            continue;
        const UInt32 other = tri.n[ k ];
        if ( IsGhost( other ) )
            continue;
        const UInt32 kk = EdgeIndex( other, e.b, e.a );
        const UInt32 p2 = m_triangles[ other ].v[ kk ];
        if ( PerturbedInCircle( tri.v[ 0 ], tri.v[ 1 ], tri.v[ 2 ], p2 ) <= 0 )
            continue;
        const UInt32 p1 = tri.v[ k ];
        Flip( t, k );
        ++flips;
        PushEdge( e.a, p2 );
        PushEdge( p2, e.b );
        PushEdge( e.b, p1 );
        PushEdge( p1, e.a );
    }
    m_edgeStack.clear();
}

/*-------------------------------------------------------------------------*/

template < typename T >
typename CTConstrainedDelaunay2D< T >::TPoint
CTConstrainedDelaunay2D< T >::Intersection( const TPoint& a, const TPoint& b, const TPoint& c, const TPoint& d ) const
{
    return CTDelaunayIntersection< T >::Compute( a, b, c, d );
}

/*-------------------------------------------------------------------------*/

template < typename T >
TDelaunayStatus
CTConstrainedDelaunay2D< T >::Initialize( void )
{
    // unique vertices: sorted by coordinates, equal points map to the first
    const UInt32 n = (UInt32) m_vertices.size();
    m_sorted.resize( n );
    for ( UInt32 i=0; i<n; ++i )
        m_sorted[ i ] = i;
    std::sort( m_sorted.begin(), m_sorted.end(), CLexicographic( &m_vertices[ 0 ] ) );
    m_inputToVertex.resize( n );
    UInt32 unique = 0;
    for ( UInt32 i=0; i<n; ++i )
    {
        const UInt32 v = m_sorted[ i ];
        if ( i > 0 && SamePosition( V( v ), V( m_sorted[ i - 1 ] ) ) )
            m_inputToVertex[ v ] = m_inputToVertex[ m_sorted[ i - 1 ] ];
        else
        {
            m_inputToVertex[ v ] = v;
            m_sorted[ unique++ ] = v;
        }
    }
    m_sorted.resize( unique );
    if ( unique < 3 )
        return DELAUNAY_DEGENERATE;

    // insertion order along a Hilbert curve over the bounding box (only the speed depends on it)
    Float64 minX = TTraits::ToFloat64( V( m_sorted[ 0 ] ).x ), maxX = minX;
    Float64 minY = TTraits::ToFloat64( V( m_sorted[ 0 ] ).y ), maxY = minY;
    for ( UInt32 i=1; i<unique; ++i )
    {
        const Float64 x = TTraits::ToFloat64( V( m_sorted[ i ] ).x );
        const Float64 y = TTraits::ToFloat64( V( m_sorted[ i ] ).y );
        minX = x < minX ? x : minX; maxX = x > maxX ? x : maxX;
        minY = y < minY ? y : minY; maxY = y > maxY ? y : maxY;
    }
    const Float64 scaleX = maxX > minX ? 65535.0 / ( maxX - minX ) : 0.0;
    const Float64 scaleY = maxY > minY ? 65535.0 / ( maxY - minY ) : 0.0;
    m_order.resize( unique );
    for ( UInt32 i=0; i<unique; ++i )
    {
        const Float64 fx = floor( ( TTraits::ToFloat64( V( m_sorted[ i ] ).x ) - minX ) * scaleX );
        const Float64 fy = floor( ( TTraits::ToFloat64( V( m_sorted[ i ] ).y ) - minY ) * scaleY );
        const UInt32 gx = fx < 0.0 ? 0u : ( fx > 65535.0 ? 65535u : (UInt32) fx );
        const UInt32 gy = fy < 0.0 ? 0u : ( fy > 65535.0 ? 65535u : (UInt32) fy );
        m_order[ i ].key = HilbertIndex( gx, gy );
        m_order[ i ].vertex = m_sorted[ i ];
    }
    std::sort( m_order.begin(), m_order.end() );

    // the first triangle: the first two points and the first point not collinear with them
    const UInt32 a = m_order[ 0 ].vertex;
    const UInt32 b = m_order[ 1 ].vertex;
    UInt32 third = 2;
    while ( third < unique && 0 == TPredicates::Orient2D( V( a ), V( b ), V( m_order[ third ].vertex ) ) )
        ++third;
    if ( third == unique )
        return DELAUNAY_DEGENERATE;
    UInt32 c = m_order[ third ].vertex;
    UInt32 p = a, q = b;
    if ( TPredicates::Orient2D( V( a ), V( b ), V( c ) ) < 0 )
    {
        p = b;
        q = a;
    }
    const UInt32 real = NewTriangle( p, q, c );
    const UInt32 g0 = NewTriangle( q, p, Ghost );
    const UInt32 g1 = NewTriangle( c, q, Ghost );
    const UInt32 g2 = NewTriangle( p, c, Ghost );
    SetNeighbour( real, p, q, g0 ); SetNeighbour( g0, q, p, real );
    SetNeighbour( real, q, c, g1 ); SetNeighbour( g1, c, q, real );
    SetNeighbour( real, c, p, g2 ); SetNeighbour( g2, p, c, real );
    SetNeighbour( g0, p, Ghost, g2 ); SetNeighbour( g2, Ghost, p, g0 );
    SetNeighbour( g1, q, Ghost, g0 ); SetNeighbour( g0, Ghost, q, g1 );
    SetNeighbour( g2, c, Ghost, g1 ); SetNeighbour( g1, Ghost, c, g2 );
    m_lastTriangle = real;

    for ( UInt32 i=2; i<unique; ++i )
    {
        if ( i == third )
            continue;
        const UInt32 v = m_order[ i ].vertex;
        const UInt32 start = Locate( V( v ) );
        if ( NoTriangle == start || !InsertVertexBowyerWatson( v, start, false ) )
            return DELAUNAY_FAILED;
    }
    return DELAUNAY_OK;
}

/*-------------------------------------------------------------------------*/

template < typename T >
TDelaunayStatus
CTConstrainedDelaunay2D< T >::InsertSegment( UInt32 u0, UInt32 v0 )
{
    m_pending.clear();
    SEdge first;
    first.a = u0;
    first.b = v0;
    m_pending.push_back( first );
    const size_t cap = 16 * m_triangles.size() + 1024;
    size_t work = 0;
    while ( !m_pending.empty() )
    {
        if ( ++work > cap )
            return DELAUNAY_FAILED;
        const SEdge segment = m_pending.back();
        m_pending.pop_back();
        const UInt32 u = segment.a;
        const UInt32 v = segment.b;
        if ( u == v )
            continue;
        if ( HasEdge( u, v ) )
        {
            SetConstrained( u, v );
            continue;
        }

        // the triangle at u whose angle holds the direction to v, or a vertex on the segment
        UInt32 t = m_vertexTriangle[ u ];
        UInt32 right = NoVertex, left = NoVertex, onSegment = NoVertex;
        const size_t fanCap = m_triangles.size() + 1;
        for ( size_t step=0; step<fanCap; ++step )
        {
            const STriangle& tri = m_triangles[ t ];
            UInt32 i = 0;
            while ( tri.v[ i ] != u )
                ++i;
            const UInt32 a = tri.v[ ( i + 1 ) % 3 ];
            const UInt32 b = tri.v[ ( i + 2 ) % 3 ];
            if ( Ghost != a && Ghost != b )
            {
                const Int32 oa = TPredicates::Orient2D( V( u ), V( a ), V( v ) );
                const Int32 ob = TPredicates::Orient2D( V( u ), V( b ), V( v ) );
                if ( 0 == oa && StrictlyBetween( V( u ), V( a ), V( v ) ) )
                {
                    onSegment = a;
                    break;
                }
                if ( 0 == ob && StrictlyBetween( V( u ), V( b ), V( v ) ) )
                {
                    onSegment = b;
                    break;
                }
                if ( oa > 0 && ob < 0 )
                {
                    right = a;
                    left = b;
                    break;
                }
            }
            t = tri.n[ ( i + 1 ) % 3 ];
        }
        if ( NoVertex != onSegment )
        {
            SEdge e;
            e.a = onSegment; e.b = v; m_pending.push_back( e );
            e.a = u; e.b = onSegment; m_pending.push_back( e );
            continue;
        }
        if ( NoVertex == right )
            return DELAUNAY_FAILED;

        // walk along the segment collecting the crossed edges ( left, right )
        m_crossed.clear();
        bool split = false;
        UInt32 current = t;
        for ( size_t step=0; step<fanCap; ++step )
        {
            const UInt32 k = EdgeIndex( current, right, left );
            const STriangle& tri = m_triangles[ current ];
            if ( 0 != ( tri.constrained & ( 1u << k ) ) )
            {
                // a constraint crosses: split both at the intersection point
                const TPoint crossing = Intersection( V( u ), V( v ), V( left ), V( right ) );
                UInt32 w = NoVertex;
                /*
                 *  Within two rounding steps of an end point of the crossed edge: the new segment runs through that
                 *  vertex. This also ends the sequence in which a segment that (nearly) overlaps a rerouted constraint
                 *  crosses the chain again and again a step or two away from the previous Steiner point.
                 */
                Float64 leftDistance = 0.0, rightDistance = 0.0;
                const bool leftNear = NearSegment( V( left ), V( u ), V( v ), leftDistance );
                const bool rightNear = NearSegment( V( right ), V( u ), V( v ), rightDistance );
                if ( WithinRoundingStep( crossing, V( left ) ) ) w = left;
                else if ( WithinRoundingStep( crossing, V( right ) ) ) w = right;
                // an end point of the crossed edge within rounding of the segment (a Steiner vertex of an overlapping chain)
                else if ( leftNear && ( !rightNear || leftDistance <= rightDistance ) ) w = left;
                else if ( rightNear ) w = right;
                else
                {
                    // the point lies in one of the two triangles of the crossed edge, or (rounded) elsewhere
                    const UInt32 at = Locate( crossing );
                    const UInt32 across = tri.n[ k ];
                    UInt32 et = 0, ek = 0;
                    FindEdge( left, right, et, ek );
                    m_triangles[ et ].constrained &= (UInt8) ~( 1u << ek );
                    FindEdge( right, left, et, ek );
                    m_triangles[ et ].constrained &= (UInt8) ~( 1u << ek );
                    PushEdge( left, right );
                    bool reroute = true;
                    // rounded onto an end point of the new segment: the crossed constraint runs through that end point
                    if ( WithinRoundingStep( crossing, V( u ) ) )
                        w = u;
                    else
                    if ( WithinRoundingStep( crossing, V( v ) ) )
                        w = v;
                    else
                    if ( NoTriangle != at )
                    {
                        // rounded outside the hull (a ghost): inserted there, the hull grows by the point
                        for ( UInt32 c=0; c<3; ++c )
                            if ( Ghost != m_triangles[ at ].v[ c ] && SamePosition( crossing, V( m_triangles[ at ].v[ c ] ) ) )
                                w = m_triangles[ at ].v[ c ];
                        if ( NoVertex == w )
                        {
                            const bool onCrossed = at == current || at == across;
                            w = (UInt32) m_vertices.size();
                            m_vertices.push_back( crossing );
                            m_vertexTriangle.push_back( NoTriangle );
                            m_inputToVertex.push_back( w );
                            if ( InsertVertexInTriangle( w, at, onCrossed ? left : (UInt32) NoVertex, onCrossed ? right : (UInt32) NoVertex ) )
                            {
                                ++m_steinerCount;
                                reroute = !onCrossed;
                            }
                            else
                            {
                                m_vertices.pop_back();
                                m_vertexTriangle.pop_back();
                                m_inputToVertex.pop_back();
                                w = NoVertex;
                            }
                        }
                    }
                    if ( NoVertex == w )
                        SetConstrained( left, right );
                    else
                    if ( reroute )
                    {
                        // the crossed constraint runs through the rounded point as well
                        SEdge e;
                        e.a = left; e.b = w; m_pending.push_back( e );
                        e.a = w; e.b = right; m_pending.push_back( e );
                    }
                    /*
                     *  Without its constraint flag the edge ( left, right ) may no longer be locally Delaunay, and the
                     *  vertices it hid may now see into the triangles beyond it. The insertion of the pending segments
                     *  (Sloan's flips) needs a constrained Delaunay triangulation to start from, so the Lawson flips
                     *  run now and not after the next segment.
                     */
                    Legalize();
                }
                if ( NoVertex == w )
                {
                    // the rounded point does not fit: route through the nearer end point of the crossed edge
                    const Float64 lx = TTraits::ToFloat64( V( left ).x ) - TTraits::ToFloat64( crossing.x ), ly = TTraits::ToFloat64( V( left ).y ) - TTraits::ToFloat64( crossing.y );
                    const Float64 rx = TTraits::ToFloat64( V( right ).x ) - TTraits::ToFloat64( crossing.x ), ry = TTraits::ToFloat64( V( right ).y ) - TTraits::ToFloat64( crossing.y );
                    w = lx * lx + ly * ly <= rx * rx + ry * ry ? left : right;
                }
                SEdge e;
                e.a = w; e.b = v; m_pending.push_back( e );
                e.a = u; e.b = w; m_pending.push_back( e );
                split = true;
                break;
            }
            SEdge crossed;
            crossed.a = left;
            crossed.b = right;
            m_crossed.push_back( crossed );
            const UInt32 next = tri.n[ k ];
            const UInt32 kk = EdgeIndex( next, left, right );
            const UInt32 w = m_triangles[ next ].v[ kk ];
            if ( w == v )
                break;
            if ( Ghost == w )
                return DELAUNAY_FAILED;
            const Int32 side = TPredicates::Orient2D( V( u ), V( v ), V( w ) );
            if ( 0 == side )
            {
                SEdge e;
                e.a = w; e.b = v; m_pending.push_back( e );
                e.a = u; e.b = w; m_pending.push_back( e );
                split = true;
                break;
            }
            if ( side > 0 )
                left = w;
            else
                right = w;
            current = next;
        }
        if ( split )
            continue;

        // Sloan: flip the crossed edges whose quad is strictly convex until none crosses the segment
        m_newEdges.clear();
        size_t head = 0;
        size_t flipCap = 4 * m_crossed.size() * m_crossed.size() + 64;
        while ( head < m_crossed.size() )
        {
            if ( 0 == flipCap-- )
                return DELAUNAY_FAILED;
            const SEdge e = m_crossed[ head++ ];
            UInt32 et = 0, ek = 0;
            if ( !FindEdge( e.a, e.b, et, ek ) )
                return DELAUNAY_FAILED;
            const UInt32 p1 = m_triangles[ et ].v[ ek ];
            const UInt32 other = m_triangles[ et ].n[ ek ];
            const UInt32 p2 = m_triangles[ other ].v[ EdgeIndex( other, e.b, e.a ) ];
            const Int32 s1 = TPredicates::Orient2D( V( p1 ), V( p2 ), V( e.a ) );
            const Int32 s2 = TPredicates::Orient2D( V( p1 ), V( p2 ), V( e.b ) );
            if ( s1 * s2 >= 0 )
            {
                m_crossed.push_back( e );
                continue;
            }
            Flip( et, ek );
            SEdge flipped;
            flipped.a = p1;
            flipped.b = p2;
            const Int32 o1 = TPredicates::Orient2D( V( u ), V( v ), V( p1 ) );
            const Int32 o2 = TPredicates::Orient2D( V( u ), V( v ), V( p2 ) );
            if ( o1 * o2 < 0 )
                m_crossed.push_back( flipped );
            else
                m_newEdges.push_back( flipped );
        }
        SetConstrained( u, v );
        for ( size_t i=0; i<m_newEdges.size(); ++i )
            PushEdge( m_newEdges[ i ].a, m_newEdges[ i ].b );
        Legalize();
    }
    return DELAUNAY_OK;
}

/*-------------------------------------------------------------------------*/

template < typename T >
TDelaunayStatus
CTConstrainedDelaunay2D< T >::Build( const TPoint* points, UInt32 count, const UInt32* constraints, UInt32 constraintCount )
{
    Clear();
    if ( ( GUCEF_NULL == points && count > 0 ) || ( GUCEF_NULL == constraints && constraintCount > 0 ) )
        return m_status = DELAUNAY_INVALID_INPUT;
    for ( UInt32 i=0; i<count; ++i )
        if ( !TTraits::IsFinite( points[ i ].x ) || !TTraits::IsFinite( points[ i ].y ) )
            return m_status = DELAUNAY_INVALID_INPUT;
    for ( UInt32 i=0; i<2*constraintCount; ++i )
        if ( constraints[ i ] >= count )
            return m_status = DELAUNAY_INVALID_INPUT;
    m_inputCount = count;
    m_vertices.assign( points, points + count );
    m_vertexTriangle.assign( count, NoTriangle );
    m_triangles.reserve( 2 * (size_t) count + 8 );
    m_stamp.reserve( 2 * (size_t) count + 8 );
    if ( 0 == count )
        return m_status = DELAUNAY_DEGENERATE;
    m_status = Initialize();
    if ( DELAUNAY_OK != m_status )
    {
        if ( DELAUNAY_DEGENERATE == m_status )
        {
            m_triangles.clear();
            m_freeTriangles.clear();
            m_vertexTriangle.assign( count, NoTriangle );
        }
        return m_status;
    }
    /*
     *  A repeated constraint is inserted once: when the first copy was rerouted through a rounded crossing, the second
     *  would cross that chain again and again a rounding step away
     */
    m_constraintOrder.resize( constraintCount );
    for ( UInt32 c=0; c<constraintCount; ++c )
    {
        m_constraintOrder[ c ].key = ConstraintKey( m_inputToVertex[ constraints[ 2 * c ] ], m_inputToVertex[ constraints[ 2 * c + 1 ] ] );
        m_constraintOrder[ c ].index = c;
    }
    std::sort( m_constraintOrder.begin(), m_constraintOrder.end() );
    m_constraintSkip.assign( constraintCount, 0u );
    for ( UInt32 i=0; i<constraintCount; ++i )
    {
        if ( i > 0 && m_constraintOrder[ i ].key == m_constraintOrder[ i - 1 ].key )
            m_constraintSkip[ m_constraintOrder[ i ].index ] = 1u;
        else
            m_constraintKeys.push_back( m_constraintOrder[ i ].key );
    }
    for ( UInt32 c=0; c<constraintCount; ++c )
    {
        if ( 0 != m_constraintSkip[ c ] )
            continue;
        const UInt32 a = m_inputToVertex[ constraints[ 2 * c ] ];
        const UInt32 b = m_inputToVertex[ constraints[ 2 * c + 1 ] ];
        if ( DELAUNAY_OK != InsertSegment( a, b ) )
            m_status = DELAUNAY_FAILED;
    }
    Invalidate();
    return m_status;
}

/*-------------------------------------------------------------------------*/

template < typename T >
TDelaunayStatus
CTConstrainedDelaunay2D< T >::InsertConstraint( UInt32 a, UInt32 b )
{
    if ( a >= m_vertices.size() || b >= m_vertices.size() )
        return DELAUNAY_INVALID_INPUT;
    if ( m_triangles.empty() )
        return DELAUNAY_DEGENERATE;
    a = a < m_inputToVertex.size() ? m_inputToVertex[ a ] : a;
    b = b < m_inputToVertex.size() ? m_inputToVertex[ b ] : b;
    const UInt64 key = ConstraintKey( a, b );
    GUCEF::vector< UInt64 >::iterator at = std::lower_bound( m_constraintKeys.begin(), m_constraintKeys.end(), key );
    if ( at != m_constraintKeys.end() && *at == key )
        return DELAUNAY_OK;
    m_constraintKeys.insert( at, key );
    Invalidate();
    return InsertSegment( a, b );
}

/*-------------------------------------------------------------------------*/

template < typename T >
TDelaunayStatus
CTConstrainedDelaunay2D< T >::InsertPoint( const TPoint& point, UInt32& vertex )
{
    vertex = NoVertex;
    if ( !TTraits::IsFinite( point.x ) || !TTraits::IsFinite( point.y ) )
        return DELAUNAY_INVALID_INPUT;
    if ( m_triangles.empty() )
        return DELAUNAY_DEGENERATE;
    const UInt32 start = Locate( point );
    if ( NoTriangle == start )
        return DELAUNAY_FAILED;
    // an equal vertex
    const STriangle& tri = m_triangles[ start ];
    for ( UInt32 k=0; k<3; ++k )
    {
        if ( Ghost != tri.v[ k ] && SamePosition( V( tri.v[ k ] ), point ) )
        {
            vertex = tri.v[ k ];
            return DELAUNAY_OK;
        }
    }
    const UInt32 index = (UInt32) m_vertices.size();
    m_vertices.push_back( point );
    m_vertexTriangle.push_back( NoTriangle );
    m_inputToVertex.push_back( index );
    if ( !InsertVertexInTriangle( index, start, NoVertex, NoVertex ) )
    {
        m_vertices.pop_back();
        m_vertexTriangle.pop_back();
        m_inputToVertex.pop_back();
        return DELAUNAY_FAILED;
    }
    vertex = index;
    return DELAUNAY_OK;
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTConstrainedDelaunay2D< T >::GetVertexOfInput( UInt32 index ) const
{
    return index < m_inputToVertex.size() ? m_inputToVertex[ index ] : (UInt32) NoVertex;
}

/*-------------------------------------------------------------------------*/

template < typename T >
const typename CTConstrainedDelaunay2D< T >::TIndexVector&
CTConstrainedDelaunay2D< T >::GetTriangles( void ) const
{
    if ( m_canonicalValid )
        return m_canonicalTriangles;
    m_keys.clear();
    m_canonicalEdges.clear();
    for ( size_t i=0; i<m_triangles.size(); ++i )
    {
        const STriangle& t = m_triangles[ i ];
        if ( !t.alive || Ghost == t.v[ 2 ] )
            continue;
        UInt32 first = 0;
        if ( t.v[ 1 ] < t.v[ first ] ) first = 1;
        if ( t.v[ 2 ] < t.v[ first ] ) first = 2;
        STriangleKey key;
        for ( UInt32 k=0; k<3; ++k )
            key.v[ k ] = t.v[ ( first + k ) % 3 ];
        m_keys.push_back( key );
        for ( UInt32 k=0; k<3; ++k )
        {
            const UInt32 a = t.v[ ( k + 1 ) % 3 ];
            const UInt32 b = t.v[ ( k + 2 ) % 3 ];
            // each constrained edge once: from the side where a < b, or from the only real side on the hull
            if ( 0 != ( t.constrained & ( 1u << k ) ) && ( a < b || IsGhost( t.n[ k ] ) ) )
            {
                m_canonicalEdges.push_back( a < b ? a : b );
                m_canonicalEdges.push_back( a < b ? b : a );
            }
        }
    }
    std::sort( m_keys.begin(), m_keys.end() );
    m_canonicalTriangles.resize( 3 * m_keys.size() );
    for ( size_t i=0; i<m_keys.size(); ++i )
        for ( UInt32 k=0; k<3; ++k )
            m_canonicalTriangles[ 3 * i + k ] = m_keys[ i ].v[ k ];
    // the edge pairs sorted
    m_keys.clear();
    for ( size_t i=0; i<m_canonicalEdges.size(); i+=2 )
    {
        STriangleKey key;
        key.v[ 0 ] = m_canonicalEdges[ i ];
        key.v[ 1 ] = m_canonicalEdges[ i + 1 ];
        key.v[ 2 ] = 0;
        m_keys.push_back( key );
    }
    std::sort( m_keys.begin(), m_keys.end() );
    m_canonicalEdges.clear();
    for ( size_t i=0; i<m_keys.size(); ++i )
    {
        if ( i > 0 && m_keys[ i ].v[ 0 ] == m_keys[ i - 1 ].v[ 0 ] && m_keys[ i ].v[ 1 ] == m_keys[ i - 1 ].v[ 1 ] )
            continue;
        m_canonicalEdges.push_back( m_keys[ i ].v[ 0 ] );
        m_canonicalEdges.push_back( m_keys[ i ].v[ 1 ] );
    }
    m_canonicalValid = true;
    return m_canonicalTriangles;
}

/*-------------------------------------------------------------------------*/

template < typename T >
const typename CTConstrainedDelaunay2D< T >::TIndexVector&
CTConstrainedDelaunay2D< T >::GetConstrainedEdges( void ) const
{
    GetTriangles();
    return m_canonicalEdges;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTConstrainedDelaunay2D< Float32 > TFloat32ConstrainedDelaunay2D;
typedef CTConstrainedDelaunay2D< Float64 > TFloat64ConstrainedDelaunay2D;
typedef CTConstrainedDelaunay2D< Int32 > TInt32ConstrainedDelaunay2D;
typedef CTConstrainedDelaunay2D< MATH::TFixed32_32 > TFixed32_32ConstrainedDelaunay2D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTConstrainedDelaunay2D< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTConstrainedDelaunay2D< Float64 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTConstrainedDelaunay2D< Int32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTConstrainedDelaunay2D< MATH::TFixed32_32 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTCONSTRAINEDDELAUNAY2D_H ? */
