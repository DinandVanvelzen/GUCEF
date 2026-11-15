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

#ifndef GUCEF_GEOMETRY_CTEPA_H
#define GUCEF_GEOMETRY_CTEPA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_CTGJK_H
#include "gucefGEOMETRY_CTGjk.h"
#define GUCEF_GEOMETRY_CTGJK_H
#endif /* GUCEF_GEOMETRY_CTGJK_H ? */

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

/**
 *  The result of EPA: the penetration of two overlapping convex shapes A and B.
 *
 *  - status: MATHSTATUS_OK; MATHSTATUS_NOT_CONVERGED when a cap (iterations, faces, horizon edges) stopped the
 *    expansion (the values are the best estimate, a lower bound of the depth); MATHSTATUS_DEGENERATE when the
 *    Minkowski difference of the cores is flat: depth 0 (see CTEpa).
 *  - depth: the smallest translation distance that separates the shapes (>= 0), to the termination band of CTEpa
 *    (see Accuracy there).
 *  - normal: unit vector from A towards B: moving B by normal * depth separates them, on polytopes up to the band,
 *    on curved shapes up to the small excess described under Accuracy.
 *  - pointOnA, pointOnB: the deepest points: pointOnA - pointOnB = normal * depth up to rounding on polytopes; on
 *    curved shapes up to the small offset described under Accuracy in CTEpa.
 */
template < typename T >
struct SEpaResult
{
    MATH::TMathStatus status;
    T depth;
    MATH::CTVector3D< T > normal;
    MATH::CTPoint3D< T > pointOnA;
    MATH::CTPoint3D< T > pointOnB;
    UInt32 iterations;

    SEpaResult( void )
        : status( MATH::MATHSTATUS_OK ), depth( MATH::CTNumericTraits< T >::Zero() ), normal(), pointOnA(), pointOnB(), iterations( 0 )
    {
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The expanding polytope algorithm (section 7.6.3 of the design): the penetration depth, normal and witness points of
 *  the overlapping cores of two support mappings (GetSupport; the margins are not included: the penetration of the
 *  whole shapes is the core depth plus both margins along the same normal, which ComputeContact adds). It starts from
 *  the final simplex of GJK on the cores (SGjkResult::simplex of CTGjk::ComputeDistance with coreDistance 0),
 *  inflated to a tetrahedron when GJK ended on a lower dimensional simplex that touches the origin. A flat Minkowski
 *  difference of the cores (two point cores, two segment cores, coplanar flat shapes) is reported with depth 0 and
 *  MATHSTATUS_DEGENERATE: the normal of its plane, a direction perpendicular to a segment, or +X for a single point.
 *
 *  - Expansion: the face nearest to the origin comes from a binary heap keyed by distance (ties: lower face slot,
 *    then lower creation number, so the order depends only on the input); the support along its normal is added,
 *    every face that sees the new vertex is removed and the horizon is closed with new faces.
 *  - Termination: when the support does not reach beyond the face by more than max( absolute, relative * depth )
 *    (CTGeometryTolerance), or at a cap: MaxIterations = 128 vertices added, MaxFaces = 256 live faces,
 *    MaxHorizonEdges: MATHSTATUS_NOT_CONVERGED, never exceeded. A face whose support is already a vertex of the
 *    polytope (a fold left by rounding) cannot be expanded: it is set aside and the next nearest face is taken.
 *  - Accuracy: the polytope lies inside the Minkowski difference, so the distance of its nearest face is a lower
 *    bound of the exact depth and the support along that face's normal an upper bound. With band = max( absolute,
 *    relative * depth, the fixed-point floor below ), a converged result (MATHSTATUS_OK) has
 *    exact - band <= depth <= exact up to rounding at the coordinate scale (a few units of the kind). On polytopes
 *    (boxes, hulls) the expansion ends on the exact face: depth is the exact minimum translation (for two boxes the
 *    smallest SAT overlap) up to rounding, also for an edge-edge minimum. On curved shapes it converges to the band:
 *    | depth - exact | <= band (the normals of the small faces there resolve only to the rounding unit over the
 *    face size, which can lift depth above exact by a fraction of the band);
 *    a deep overlap of curved shapes can need more faces than the caps allow: MATHSTATUS_NOT_CONVERGED, with depth a
 *    lower bound only. Not a guarantee of the depth: the deepest point of a box manifold (CTSatBoxes), which prefers
 *    face axes for coherence and clips points, can differ from it by several percent.
 *  - Normal on curved shapes: when the projection of the origin falls outside the nearest face (the polytope of a
 *    curved shape is convex only up to the plane tolerance), normal and witness points come from the adjacent face
 *    within the band whose nearest point to the origin is nearest, so that the witness points lie along normal;
 *    depth stays that of the nearest face. pointOnA - pointOnB then differs from normal * depth by the distance of
 *    that nearest point from the projection (in the tests below 2e-3 of the size in Float32, 3e-6 in Float64), and
 *    as the support of that face was not checked, the translation along normal that separates the shapes can
 *    exceed depth by more than the band (in the tests below 1e-4 of the size: Float32 a few bands, Float64 below
 *    1e-8). Taking the nearest face's own normal instead would keep the band along the normal but move the witness
 *    points off normal * depth by up to 1e-2 of the size in Float32, which is worse for contact points.
 *  - Memory: fixed-capacity arrays inside the object (about 30 KB for Float64): no allocation. Keep an object for
 *    repeated queries, or use the static Penetration, which places one on the stack.
 *  - Kind: floats and fixed point (Float32, Float64 and TFixed32_32 are exported); the fixed-point form is bit-exact.
 *    Fixed point resolves the normals of small faces only to its resolution over the face size, so its expansion also
 *    stops within 2^-20 of the coordinate scale (about 1e-6 for shapes of a metre): the depth on curved shapes is
 *    accurate to that, on polytopes to the resolution.
 *  - Realtime: every member is realtime-safe.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTEpa
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef CTGeometryTolerance< T > TTolerance;
    typedef SGjkSimplex< T > TSimplex;

    enum
    {
        MaxIterations = 128,
        MaxFaces = 256,
        MaxVertices = MaxIterations + 4,
        MaxHorizonEdges = 3 * MaxFaces,
        MaxHeapEntries = 2 * MaxFaces
    };

    CTEpa( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  The penetration of the cores of A and B, from 'simplex': the final simplex of CTGjk::ComputeDistance for the same
     *  pair when the cores overlap. Returns result.status.
     */
    template < typename TA, typename TB >
    MATH::TMathStatus ComputePenetration( const TA& a, const TB& b, const TSimplex& simplex, const TTolerance& tolerance, SEpaResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  ComputePenetration with a work object on the stack: about 40 KB for Float64 and TFixed32_32, 28 KB for Float32.
     *  Threads with small stacks use the overload with a workspace.
     */
    template < typename TA, typename TB >
    static MATH::TMathStatus Penetration( const TA& a, const TB& b, const TSimplex& simplex, const TTolerance& tolerance, SEpaResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        CTEpa epa;
        return epa.ComputePenetration( a, b, simplex, tolerance, result );
    }

    /** ComputePenetration in a caller-provided work object (kept between queries, for example one per thread) */
    template < typename TA, typename TB >
    static MATH::TMathStatus Penetration( const TA& a, const TB& b, const TSimplex& simplex, const TTolerance& tolerance, CTEpa& workspace, SEpaResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return workspace.ComputePenetration( a, b, simplex, tolerance, result );
    }

    private:

    struct SFace
    {
        UInt16 v[ 3 ];
        UInt16 adjacent[ 3 ];   /**< the face across edge i, which runs from v[ i ] to v[ ( i + 1 ) % 3 ] */
        UInt8 adjacentEdge[ 3 ]; /**< the index of that edge in the adjacent face */
        bool alive;
        bool valid;             /**< the normal is defined (the face is not degenerate) */
        bool selectable;        /**< offered as the nearest face (false for a fold that cannot be expanded) */
        UInt32 pass;            /**< the expansion that removed the face */
        UInt32 serial;          /**< creation number, for the heap entries and the tie-break */
        TVector normal;
        T distance;
    };

    struct SHeapEntry
    {
        T distance;
        UInt32 face;
        UInt32 serial;
    };

    /** A face on the horizon and the index of its edge that borders the removed region */
    struct SHorizonEdge
    {
        UInt16 face;
        UInt8 edge;
    };

    /** A frame of the walk over the removed region: the face, the edge it was entered by (3 for the start) and the next edge to visit */
    struct SWalkFrame
    {
        UInt16 face;
        UInt8 entry;
        UInt8 step;
    };

    template < typename TA, typename TB >
    bool AddVertex( const TA& a, const TB& b, const TVector& direction ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** A new face over vertices i, j, k (counter-clockwise from outside); MaxFaces when no slot is free. Adjacency is set by the caller */
    UInt32 AddFace( UInt16 i, UInt16 j, UInt16 k ) GUCEF_GEOMETRY_REALTIME_CONTEXT;
    void RemoveFace( UInt32 face ) GUCEF_GEOMETRY_REALTIME_CONTEXT;
    void Link( UInt32 face, UInt32 edge, UInt32 other, UInt32 otherEdge ) GUCEF_GEOMETRY_REALTIME_CONTEXT;
    bool PopNearestFace( UInt32& face ) GUCEF_GEOMETRY_REALTIME_CONTEXT;
    void PushHeap( const SHeapEntry& entry ) GUCEF_GEOMETRY_REALTIME_CONTEXT;
    void RebuildHeap( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT;
    bool BuildTetrahedron( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  Removes the faces that see vertex 'added', walking from 'face' over the adjacency so that the removed region is
     *  connected, and closes the hole with faces from its horizon to the vertex. False when a cap or an inconsistent
     *  horizon stopped it.
     */
    bool Expand( UInt32 face, UInt16 added, const T& planeTolerance ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    void FinishResult( UInt32 face, MATH::TMathStatus status, const TTolerance& tolerance, SEpaResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;
    void SolveFace( UInt32 face, TSimplex& triangle, TVector& v ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;
    static bool HeapLess( const SHeapEntry& x, const SHeapEntry& y ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The vertex of the polytope that is furthest from the given line or plane, used to inflate degenerate simplices */
    template < typename TA, typename TB >
    bool Inflate( const TA& a, const TB& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    TVector m_w[ MaxVertices ];
    TPoint m_a[ MaxVertices ];
    TPoint m_b[ MaxVertices ];
    UInt32 m_vertexCount;

    SFace m_faces[ MaxFaces ];
    UInt16 m_freeFaces[ MaxFaces ];
    UInt32 m_freeCount;
    UInt32 m_liveFaces;
    UInt32 m_serial;

    SHeapEntry m_heap[ MaxHeapEntries ];
    UInt32 m_heapSize;

    UInt32 m_pass;
    T m_absoluteTolerance;
    T m_resolution;                             /**< the fixed-point floor of the face distances (0 for floats), see ComputePenetration */
    SHorizonEdge m_horizon[ MaxHorizonEdges ];
    SWalkFrame m_walk[ MaxFaces ];
    UInt16 m_removed[ MaxFaces ];
    UInt16 m_newFaceAt[ MaxVertices ];          /**< per vertex: the new face of this expansion whose first vertex it is */
    UInt32 m_newFacePass[ MaxVertices ];        /**< the expansion m_newFaceAt was written in */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
CTEpa< T >::CTEpa( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    : m_vertexCount( 0 ), m_freeCount( 0 ), m_liveFaces( 0 ), m_serial( 0 ), m_heapSize( 0 ), m_pass( 0 ), m_absoluteTolerance( TTraits::Zero() ), m_resolution( TTraits::Zero() )
{
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTEpa< T >::HeapLess( const SHeapEntry& x, const SHeapEntry& y ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( x.distance < y.distance )
        return true;
    if ( y.distance < x.distance )
        return false;
    return x.serial < y.serial;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTEpa< T >::PushHeap( const SHeapEntry& entry ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( m_heapSize >= (UInt32) MaxHeapEntries )
        RebuildHeap();
    if ( m_heapSize >= (UInt32) MaxHeapEntries )
        return;
    UInt32 i = m_heapSize++;
    m_heap[ i ] = entry;
    while ( i > 0 )
    {
        const UInt32 parent = ( i - 1 ) / 2;
        if ( !HeapLess( m_heap[ i ], m_heap[ parent ] ) )
            break;
        const SHeapEntry swap = m_heap[ i ];
        m_heap[ i ] = m_heap[ parent ];
        m_heap[ parent ] = swap;
        i = parent;
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTEpa< T >::RebuildHeap( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    /* drop the entries of removed faces: at most MaxFaces live entries remain */
    m_heapSize = 0;
    for ( UInt32 f=0; f<(UInt32) MaxFaces; ++f )
    {
        if ( !m_faces[ f ].alive || !m_faces[ f ].valid || !m_faces[ f ].selectable )
            continue;
        SHeapEntry entry;
        entry.distance = m_faces[ f ].distance;
        entry.face = f;
        entry.serial = m_faces[ f ].serial;
        UInt32 i = m_heapSize++;
        m_heap[ i ] = entry;
        while ( i > 0 )
        {
            const UInt32 parent = ( i - 1 ) / 2;
            if ( !HeapLess( m_heap[ i ], m_heap[ parent ] ) )
                break;
            const SHeapEntry swap = m_heap[ i ];
            m_heap[ i ] = m_heap[ parent ];
            m_heap[ parent ] = swap;
            i = parent;
        }
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTEpa< T >::PopNearestFace( UInt32& face ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    while ( m_heapSize > 0 )
    {
        const SHeapEntry top = m_heap[ 0 ];
        m_heap[ 0 ] = m_heap[ --m_heapSize ];
        UInt32 i = 0;
        for ( ;; )
        {
            const UInt32 left = 2 * i + 1;
            const UInt32 right = left + 1;
            UInt32 smallest = i;
            if ( left < m_heapSize && HeapLess( m_heap[ left ], m_heap[ smallest ] ) )
                smallest = left;
            if ( right < m_heapSize && HeapLess( m_heap[ right ], m_heap[ smallest ] ) )
                smallest = right;
            if ( smallest == i )
                break;
            const SHeapEntry swap = m_heap[ i ];
            m_heap[ i ] = m_heap[ smallest ];
            m_heap[ smallest ] = swap;
            i = smallest;
        }
        const SFace& f = m_faces[ top.face ];
        if ( f.alive && f.serial == top.serial )
        {
            face = top.face;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE UInt32
CTEpa< T >::AddFace( UInt16 i, UInt16 j, UInt16 k ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( 0 == m_freeCount )
        return (UInt32) MaxFaces;
    const UInt32 slot = m_freeFaces[ --m_freeCount ];
    SFace& f = m_faces[ slot ];
    f.v[ 0 ] = i;
    f.v[ 1 ] = j;
    f.v[ 2 ] = k;
    for ( UInt32 e=0; e<3; ++e )
    {
        f.adjacent[ e ] = (UInt16) slot;
        f.adjacentEdge[ e ] = (UInt8) e;
    }
    f.alive = true;
    f.pass = 0;
    f.serial = ++m_serial;
    const TVector e1 = m_w[ j ] - m_w[ i ];
    const TVector e2 = m_w[ k ] - m_w[ i ];
    f.valid = false;
    f.selectable = true;
    f.normal = TVector();
    f.distance = TTraits::Max();
    const T edgeScale = TTraits::Max( e1.GetChebyshevLength(), e2.GetChebyshevLength() );
    if ( TTraits::Zero() < edgeScale )
    {
        /* the cross product of the edges scaled to their largest component keeps small faces accurate */
        const TVector s1 = e1 / edgeScale;
        const TVector s2 = e2 / edgeScale;
        TVector unit;
        if ( s1.Cross( s2 ).TryNormalize( unit, TTraits::Zero() ) )
        {
            f.valid = true;
            f.normal = unit;
            T distance = unit.Dot( m_w[ i ] );
            if ( TTraits::IsNegative( distance ) )
            {
                /*
                 *  the origin outside a face: beyond the tolerance this is a fold left by rounding, which must not
                 *  become the answer (it stays in the polytope but is not offered); within it the origin is on the face
                 */
                if ( TTraits::IsNegative( distance + m_absoluteTolerance ) )
                    f.selectable = false;
                distance = TTraits::Zero();
            }
            f.distance = distance;
        }
    }
    ++m_liveFaces;
    if ( f.valid && f.selectable )
    {
        SHeapEntry entry;
        entry.distance = f.distance;
        entry.face = slot;
        entry.serial = f.serial;
        PushHeap( entry );
    }
    return slot;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTEpa< T >::Link( UInt32 face, UInt32 edge, UInt32 other, UInt32 otherEdge ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    m_faces[ face ].adjacent[ edge ] = (UInt16) other;
    m_faces[ face ].adjacentEdge[ edge ] = (UInt8) otherEdge;
    m_faces[ other ].adjacent[ otherEdge ] = (UInt16) face;
    m_faces[ other ].adjacentEdge[ otherEdge ] = (UInt8) edge;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTEpa< T >::RemoveFace( UInt32 face ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !m_faces[ face ].alive )
        return;
    m_faces[ face ].alive = false;
    m_freeFaces[ m_freeCount++ ] = (UInt16) face;
    --m_liveFaces;
}

/*-------------------------------------------------------------------------*/

template < typename T >
template < typename TA, typename TB >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTEpa< T >::AddVertex( const TA& a, const TB& b, const TVector& direction ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( m_vertexCount >= (UInt32) MaxVertices )
        return false;
    const TPoint pa = a.GetSupport( direction );
    const TPoint pb = b.GetSupport( -direction );
    m_a[ m_vertexCount ] = pa;
    m_b[ m_vertexCount ] = pb;
    m_w[ m_vertexCount ] = pa - pb;
    ++m_vertexCount;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
template < typename TA, typename TB >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTEpa< T >::Inflate( const TA& a, const TB& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const T zero = TTraits::Zero();
    const T one = TTraits::One();
    if ( 1 == m_vertexCount )
    {
        /* a point: the furthest support along the six axes */
        const TVector axes[ 6 ] = { TVector( one, zero, zero ), TVector( -one, zero, zero ), TVector( zero, one, zero ),
                                    TVector( zero, -one, zero ), TVector( zero, zero, one ), TVector( zero, zero, -one ) };
        UInt32 best = 6;
        T bestDistance = zero;
        for ( UInt32 i=0; i<6; ++i )
        {
            const TVector w = a.GetSupport( axes[ i ] ) - b.GetSupport( -axes[ i ] );
            const T d = CTGjk< T >::GetMagnitude( w - m_w[ 0 ] );
            if ( bestDistance < d )
            {
                bestDistance = d;
                best = i;
            }
        }
        if ( 6 == best || !AddVertex( a, b, axes[ best ] ) )
            return false;
    }
    if ( 2 == m_vertexCount )
    {
        /* a segment: the support furthest from its line, along four directions around it */
        const TVector d = m_w[ 1 ] - m_w[ 0 ];
        const TVector e = d.GetAnyPerpendicular();
        const TVector f = d.Cross( e );
        const TVector se = CTSearchDirectionScaler< T >::Scale( e );
        const TVector sf = CTSearchDirectionScaler< T >::Scale( f );
        const TVector directions[ 4 ] = { se, -se, sf, -sf };
        TVector scaledD;
        if ( !d.ScaleToUnitChebyshev( scaledD ) )
            return false;
        UInt32 best = 4;
        T bestDistance = zero;
        for ( UInt32 i=0; i<4; ++i )
        {
            const TVector w = a.GetSupport( directions[ i ] ) - b.GetSupport( -directions[ i ] );
            const T distance = CTGjk< T >::GetMagnitude( scaledD.Cross( w - m_w[ 0 ] ) );
            if ( bestDistance < distance )
            {
                bestDistance = distance;
                best = i;
            }
        }
        if ( 4 == best || !AddVertex( a, b, directions[ best ] ) )
            return false;
    }
    if ( 3 == m_vertexCount )
    {
        /* a triangle: the support furthest from its plane, on either side */
        const TVector n = ( m_w[ 1 ] - m_w[ 0 ] ).Cross( m_w[ 2 ] - m_w[ 0 ] );
        TVector unit;
        if ( !n.ScaleToUnitChebyshev( unit ) )
            return false;
        const TVector up = a.GetSupport( unit ) - b.GetSupport( -unit );
        const TVector down = a.GetSupport( -unit ) - b.GetSupport( unit );
        const T upDistance = TTraits::Abs( unit.Dot( up - m_w[ 0 ] ) );
        const T downDistance = TTraits::Abs( unit.Dot( down - m_w[ 0 ] ) );
        const TVector chosen = downDistance < upDistance ? unit : -unit;
        if ( !( TTraits::Zero() < TTraits::Max( upDistance, downDistance ) ) )
            return false;
        if ( !AddVertex( a, b, chosen ) )
            return false;
    }
    return 4 == m_vertexCount;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTEpa< T >::BuildTetrahedron( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const TVector e1 = m_w[ 1 ] - m_w[ 0 ];
    const TVector e2 = m_w[ 2 ] - m_w[ 0 ];
    const TVector e3 = m_w[ 3 ] - m_w[ 0 ];
    const T scale = TTraits::Max( TTraits::Max( e1.GetChebyshevLength(), e2.GetChebyshevLength() ), e3.GetChebyshevLength() );
    if ( !( TTraits::Zero() < scale ) )
        return false;
    const T volume = ( e1 / scale ).Dot( ( e2 / scale ).Cross( e3 / scale ) );
    if ( TTraits::Zero() == volume )
        return false;
    /* outward faces: counter-clockwise seen from outside */
    static const UInt16 negative[ 4 ][ 3 ] = { { 0, 1, 2 }, { 0, 3, 1 }, { 0, 2, 3 }, { 1, 3, 2 } };
    static const UInt16 positive[ 4 ][ 3 ] = { { 0, 2, 1 }, { 0, 1, 3 }, { 0, 3, 2 }, { 1, 2, 3 } };
    const UInt16 ( *faces )[ 3 ] = TTraits::IsNegative( volume ) ? negative : positive;
    UInt32 slots[ 4 ];
    for ( UInt32 f=0; f<4; ++f )
    {
        slots[ f ] = AddFace( faces[ f ][ 0 ], faces[ f ][ 1 ], faces[ f ][ 2 ] );
        if ( slots[ f ] >= (UInt32) MaxFaces )
            return false;
    }
    /* adjacency: the face that has the reversed edge */
    for ( UInt32 f=0; f<4; ++f )
        for ( UInt32 e=0; e<3; ++e )
            for ( UInt32 g=0; g<4; ++g )
                for ( UInt32 h=0; h<3; ++h )
                    if ( faces[ g ][ h ] == faces[ f ][ ( e + 1 ) % 3 ] && faces[ g ][ ( h + 1 ) % 3 ] == faces[ f ][ e ] )
                        Link( slots[ f ], e, slots[ g ], h );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTEpa< T >::Expand( UInt32 face, UInt16 added, const T& planeTolerance ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    ++m_pass;
    UInt32 removedCount = 0;
    UInt32 horizonCount = 0;
    UInt32 depth = 0;
    m_faces[ face ].pass = m_pass;
    m_removed[ removedCount++ ] = (UInt16) face;
    m_walk[ depth ].face = (UInt16) face;
    m_walk[ depth ].entry = 3;
    m_walk[ depth ].step = 0;
    ++depth;

    /* depth first over the faces that see the vertex; the edges of each face in order, so the horizon comes out as a loop */
    while ( depth > 0 )
    {
        SWalkFrame& frame = m_walk[ depth - 1 ];
        const UInt32 steps = 3 == frame.entry ? 3 : 2;
        if ( frame.step >= steps )
        {
            --depth;
            continue;
        }
        const UInt32 edge = 3 == frame.entry ? frame.step : ( frame.entry + 1 + frame.step ) % 3;
        ++frame.step;
        const SFace& current = m_faces[ frame.face ];
        const UInt32 next = current.adjacent[ edge ];
        const UInt32 nextEdge = current.adjacentEdge[ edge ];
        SFace& neighbour = m_faces[ next ];
        if ( neighbour.pass == m_pass )
            continue;
        bool visible = !neighbour.valid;
        if ( neighbour.valid )
            visible = !TTraits::IsNegative( neighbour.normal.Dot( m_w[ added ] - m_w[ neighbour.v[ 0 ] ] ) + planeTolerance );
        if ( !visible )
        {
            if ( horizonCount >= (UInt32) MaxHorizonEdges )
                return false;
            m_horizon[ horizonCount ].face = (UInt16) next;
            m_horizon[ horizonCount ].edge = (UInt8) nextEdge;
            ++horizonCount;
            continue;
        }
        if ( depth >= (UInt32) MaxFaces || removedCount >= (UInt32) MaxFaces )
            return false;
        neighbour.pass = m_pass;
        m_removed[ removedCount++ ] = (UInt16) next;
        m_walk[ depth ].face = (UInt16) next;
        m_walk[ depth ].entry = (UInt8) nextEdge;
        m_walk[ depth ].step = 0;
        ++depth;
    }

    for ( UInt32 r=0; r<removedCount; ++r )
        RemoveFace( m_removed[ r ] );
    if ( m_freeCount < horizonCount || horizonCount < 3 )
        return false;

    /* the new faces ( b, a, added ) over each horizon edge a -> b of the faces that stay */
    UInt32 created[ MaxHorizonEdges ];
    for ( UInt32 h=0; h<horizonCount; ++h )
    {
        const SFace& outside = m_faces[ m_horizon[ h ].face ];
        const UInt16 from = outside.v[ ( m_horizon[ h ].edge + 1 ) % 3 ];
        const UInt16 to = outside.v[ m_horizon[ h ].edge ];
        const UInt32 slot = AddFace( from, to, added );
        if ( slot >= (UInt32) MaxFaces )
            return false;
        Link( slot, 0, m_horizon[ h ].face, m_horizon[ h ].edge );
        created[ h ] = slot;
        if ( m_newFacePass[ from ] == m_pass )
            return false;    /* a vertex twice on the horizon: not a simple loop */
        m_newFacePass[ from ] = m_pass;
        m_newFaceAt[ from ] = (UInt16) slot;
    }
    /* edge 1 of a new face runs from its second vertex to the new vertex; the new face that starts there has it reversed as edge 2 */
    for ( UInt32 h=0; h<horizonCount; ++h )
    {
        const UInt16 to = m_faces[ created[ h ] ].v[ 1 ];
        if ( m_newFacePass[ to ] != m_pass )
            return false;
        Link( created[ h ], 1, m_newFaceAt[ to ], 2 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTEpa< T >::SolveFace( UInt32 face, TSimplex& triangle, TVector& v ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const SFace& f = m_faces[ face ];
    for ( UInt32 i=0; i<3; ++i )
    {
        triangle.w[ i ] = m_w[ f.v[ i ] ];
        triangle.a[ i ] = m_a[ f.v[ i ] ];
        triangle.b[ i ] = m_b[ f.v[ i ] ];
    }
    triangle.count = 3;
    CTGjk< T >::SolveSimplex( triangle, v );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTEpa< T >::FinishResult( UInt32 face, MATH::TMathStatus status, const TTolerance& tolerance, SEpaResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TSimplex triangle;
    TVector v;
    SolveFace( face, triangle, v );
    UInt32 chosen = face;
    if ( triangle.count < 3 )
    {
        /*
         *  The projection of the origin lies outside this face: a flat part of the polytope is split into several
         *  faces at the same distance. Take the face of that part whose nearest point to the origin is nearest, so
         *  that the witness points are the points of A and B over the projection.
         */
        const T limit = m_faces[ face ].distance + TTraits::Max( TTraits::Max( tolerance.absolute, tolerance.relative * m_faces[ face ].distance ), m_resolution );
        T best = CTGjk< T >::GetMagnitude( v );
        for ( UInt32 f=0; f<(UInt32) MaxFaces; ++f )
        {
            if ( f == face || !m_faces[ f ].alive || !m_faces[ f ].valid || !m_faces[ f ].selectable || limit < m_faces[ f ].distance )
                continue;
            TSimplex candidate;
            TVector candidateV;
            SolveFace( f, candidate, candidateV );
            const T m = CTGjk< T >::GetMagnitude( candidateV );
            if ( m < best )
            {
                best = m;
                chosen = f;
                triangle = candidate;
            }
        }
    }
    CTGjk< T >::GetClosestPoints( triangle, result.pointOnA, result.pointOnB );
    result.status = status;
    result.normal = m_faces[ chosen ].normal;
    result.depth = m_faces[ face ].distance;
}

/*-------------------------------------------------------------------------*/

template < typename T >
template < typename TA, typename TB >
GUCEF_GEOMETRY_PRECISE_INLINE MATH::TMathStatus
CTEpa< T >::ComputePenetration( const TA& a, const TB& b, const TSimplex& simplex, const TTolerance& tolerance, SEpaResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    result = SEpaResult< T >();
    m_vertexCount = 0;
    m_heapSize = 0;
    m_liveFaces = 0;
    m_serial = 0;
    m_freeCount = 0;
    m_pass = 0;
    m_absoluteTolerance = tolerance.absolute;
    for ( UInt32 f=0; f<(UInt32) MaxFaces; ++f )
    {
        m_faces[ f ].alive = false;
        m_faces[ f ].pass = 0;
        m_freeFaces[ m_freeCount++ ] = (UInt16) ( MaxFaces - 1 - f );
    }
    for ( UInt32 v=0; v<(UInt32) MaxVertices; ++v )
        m_newFacePass[ v ] = 0;

    for ( UInt32 i=0; i<simplex.count && i<4; ++i )
    {
        m_w[ i ] = simplex.w[ i ];
        m_a[ i ] = simplex.a[ i ];
        m_b[ i ] = simplex.b[ i ];
    }
    m_vertexCount = simplex.count < 4 ? simplex.count : 4;
    if ( 0 == m_vertexCount )
        AddVertex( a, b, TVector::UnitX() );

    bool built = false;
    if ( 4 == m_vertexCount )
        built = BuildTetrahedron();
    if ( !built )
    {
        /* a lower dimensional simplex (touching shapes) or a flat tetrahedron: inflate */
        if ( m_vertexCount > 3 )
            m_vertexCount = 3;
        if ( 3 == m_vertexCount )
        {
            const TVector n = ( m_w[ 1 ] - m_w[ 0 ] ).Cross( m_w[ 2 ] - m_w[ 0 ] );
            TVector unit;
            if ( !n.ScaleToUnitChebyshev( unit ) )
                m_vertexCount = 2;
        }
        if ( 2 == m_vertexCount && m_w[ 1 ] == m_w[ 0 ] )
            m_vertexCount = 1;
        if ( !Inflate( a, b ) || !BuildTetrahedron() )
        {
            /* the Minkowski difference has no volume: flat shapes in one plane */
            result.status = MATH::MATHSTATUS_DEGENERATE;
            result.depth = TTraits::Zero();
            TVector n;
            if ( m_vertexCount >= 3 )
                ( m_w[ 1 ] - m_w[ 0 ] ).Cross( m_w[ 2 ] - m_w[ 0 ] ).TryNormalize( n, TTraits::Zero() );
            if ( n.IsZero() && 2 == m_vertexCount )
                ( m_w[ 1 ] - m_w[ 0 ] ).GetAnyPerpendicular().TryNormalize( n, TTraits::Zero() );
            if ( n.IsZero() )
                n = TVector::UnitX();
            result.normal = n;
            TSimplex s;
            for ( UInt32 i=0; i<m_vertexCount && i<3; ++i )
            {
                s.w[ i ] = m_w[ i ];
                s.a[ i ] = m_a[ i ];
                s.b[ i ] = m_b[ i ];
            }
            s.count = m_vertexCount < 3 ? m_vertexCount : 3;
            TVector v;
            CTGjk< T >::SolveSimplex( s, v );
            CTGjk< T >::GetClosestPoints( s, result.pointOnA, result.pointOnB );
            return result.status;
        }
    }

    /*
     *  Fixed point resolves the normals of small faces only to its resolution divided by the face size: on curved
     *  shapes the expansion cannot get closer than about 2^-20 of the coordinates, which is its floor
     */
    T scale = TTraits::One();
    for ( UInt32 i=0; i<m_vertexCount; ++i )
        scale = TTraits::Max( scale, m_w[ i ].GetChebyshevLength() );
    const T resolution = TTraits::IsFloatingPoint ? TTraits::Zero() : TTraits::Epsilon() * TTraits::FromInt32( 4096 ) * scale;
    m_resolution = resolution;
    SEpaResult< T > fallback;
    bool hasFallback = false;
    UInt32 iterations = 0;
    for ( ;; )
    {
        UInt32 face = 0;
        if ( !PopNearestFace( face ) )
        {
            /* no face left to expand: the last estimate, else the nearest face of the polytope as it stands */
            if ( hasFallback )
            {
                result = fallback;
            }
            else
            {
                UInt32 nearest = (UInt32) MaxFaces;
                for ( UInt32 f=0; f<(UInt32) MaxFaces; ++f )
                    if ( m_faces[ f ].alive && m_faces[ f ].valid && ( nearest == (UInt32) MaxFaces || m_faces[ f ].distance < m_faces[ nearest ].distance ) )
                        nearest = f;
                if ( nearest < (UInt32) MaxFaces )
                    FinishResult( nearest, MATH::MATHSTATUS_NOT_CONVERGED, tolerance, result );
            }
            result.status = MATH::MATHSTATUS_NOT_CONVERGED;
            result.iterations = iterations;
            return result.status;
        }
        if ( iterations >= (UInt32) MaxIterations )
        {
            FinishResult( face, MATH::MATHSTATUS_NOT_CONVERGED, tolerance, result );
            result.iterations = iterations;
            return result.status;
        }
        ++iterations;

        const SFace current = m_faces[ face ];
        if ( !AddVertex( a, b, current.normal ) )
        {
            FinishResult( face, MATH::MATHSTATUS_NOT_CONVERGED, tolerance, result );
            result.iterations = iterations;
            return result.status;
        }
        const UInt16 added = (UInt16) ( m_vertexCount - 1 );
        const T reach = current.normal.Dot( m_w[ added ] );
        const T allowed = TTraits::Max( TTraits::Max( tolerance.absolute, tolerance.relative * current.distance ), resolution );
        if ( !( allowed < reach - current.distance ) )
        {
            --m_vertexCount;
            FinishResult( face, MATH::MATHSTATUS_OK, tolerance, result );
            result.iterations = iterations;
            return result.status;
        }
        bool repeated = false;
        for ( UInt32 i=0; i<added; ++i )
            repeated = repeated || m_w[ i ] == m_w[ added ];
        if ( repeated )
        {
            /*
             *  The support beyond this face is a vertex of the polytope already: rounding left the face slightly
             *  inside the hull of the vertices (a fold). It cannot be expanded and is no answer: it stays in the
             *  polytope but is no longer offered as the nearest face.
             */
            --m_vertexCount;
            m_faces[ face ].selectable = false;
            continue;
        }

        /* faces within the plane tolerance of the new vertex count as seeing it: coplanar regions merge instead of folding */
        const T planeTolerance = TTraits::Max( tolerance.absolute, tolerance.relative * current.distance );
        FinishResult( face, MATH::MATHSTATUS_NOT_CONVERGED, tolerance, fallback );
        hasFallback = true;
        if ( !Expand( face, added, planeTolerance ) )
        {
            result = fallback;
            result.iterations = iterations;
            return result.status;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTEpa< Float32 > TFloat32Epa;
typedef CTEpa< Float64 > TFloat64Epa;
typedef CTEpa< MATH::TFixed32_32 > TFixed32_32Epa;

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* the non-template members; the member templates over the support mappings come with the query functions */
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTEpa< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTEpa< Float64 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTEpa< MATH::TFixed32_32 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTEPA_H ? */
