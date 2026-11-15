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

#ifndef GUCEF_GEOMETRY_CTSATBOXES_H
#define GUCEF_GEOMETRY_CTSATBOXES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_CTCONTACTMANIFOLD_H
#include "gucefGEOMETRY_CTContactManifold.h"
#define GUCEF_GEOMETRY_CTCONTACTMANIFOLD_H
#endif /* GUCEF_GEOMETRY_CTCONTACTMANIFOLD_H ? */

#ifndef GUCEF_GEOMETRY_CTGJK_H
#include "gucefGEOMETRY_CTGjk.h"
#define GUCEF_GEOMETRY_CTGJK_H
#endif /* GUCEF_GEOMETRY_CTGJK_H ? */

#ifndef GUCEF_GEOMETRY_SEGMENTQUERIES_H
#include "gucefGEOMETRY_SegmentQueries.h"
#define GUCEF_GEOMETRY_SEGMENTQUERIES_H
#endif /* GUCEF_GEOMETRY_SEGMENTQUERIES_H ? */

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
 *  The result of the separating axis test of two boxes A and B.
 *
 *  - overlapping: no axis separates them (touching counts as overlapping).
 *  - axis: the axis of the extreme value: 0..2 the face axes of A, 3..5 the face axes of B, 6..14 the edge axes
 *    A( ( axis - 6 ) / 3 ) x B( ( axis - 6 ) % 3 ).
 *  - normal: that axis as a unit vector, oriented from A towards B.
 *  - depth: overlapping: the smallest overlap of the projections over the 15 axes (the penetration depth of the
 *    boxes); separated: minus the largest separation (a lower bound of minus the distance).
 */
template < typename T >
struct SSatBoxesResult
{
    bool overlapping;
    UInt32 axis;
    MATH::CTVector3D< T > normal;
    T depth;

    SSatBoxesResult( void ) : overlapping( false ), axis( 0 ), normal(), depth( MATH::CTNumericTraits< T >::Zero() ) {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Box-box separating axis test and single-shot contact manifold (sections 7.5.1 and 7.6.4 of the design).
 *
 *  - Axes: the 3 + 3 face normals and the 9 edge cross products (Gottschalk); an edge pair whose cross product is
 *    shorter than the angular tolerance (parallel edges) is skipped: its face axes decide.
 *  - Contact: face axes are preferred over edge axes unless an edge axis is clearly shallower: its overlap is below
 *    the face overlap minus 0.05 of its magnitude minus the slop (the larger of the length tolerance and 1/1024 of the
 *    smallest half extent of the two boxes); faces of A are preferred over faces of B in the same way with 0.02. Edge
 *    pairs closer to parallel than sin( angle ) = 1/1024 (about 1e-3) are not offered for contact generation (their
 *    cross product is mostly rounding noise); they still count for the separation test. A face contact clips the
 *    incident face of the other box against the four side planes of the reference face (Sutherland-Hodgman, up to 8
 *    points) and keeps the points within the speculative distance (plus the length tolerance) of the reference face;
 *    an edge contact gives the nearest points of the two edges. Reduce( maxPoints ) then keeps the deepest point and
 *    the largest area.
 *  - Separated boxes: when the largest separation over the axes exceeds the speculative distance (0 by default), one
 *    point: the nearest points from GJK, with a negative depth. Within the speculative distance the contact is built
 *    as above with negative depths (speculative contacts) and ComputeContact returns false.
 *  - Feature ids name the features, not the position in the clipping output, so a point keeps its id while the boxes
 *    slide. Face contacts: bits 0..5 the point: an incident face corner k (0..3, in the order -u-v, +u-v, +u+v,
 *    -u+v of the two other incident axes), 4 + 4 * incident edge e + side plane s for an incident edge e (from corner e
 *    to corner e + 1) cut by side plane s of the reference face (0..3: +ru, -ru, +rv, -rv of the two other reference
 *    axes), 20 + 4 * s + t for the reference face corner of side planes s < t; bits 6..8 the reference face
 *    (2 * axis + 1 for the negative side), bit 9 set when B holds the reference face, bits 10..12 the incident face.
 *    Edge contacts: 0x2000 | ( 3 * edge axis of A + edge axis of B ); separated beyond the speculative distance and
 *    fallback points: 0x4000.
 *  - Kind: floats and fixed point (Float32, Float64 and TFixed32_32 are exported); bit-exact in fixed point.
 *  - Realtime: every member is realtime-safe and allocation-free.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTSatBoxes
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef CTGeometryTolerance< T > TTolerance;
    typedef CTOrientedBox3D< T > TBox;

    /** The separating axis test; true when the boxes overlap or touch (also false for an invalid box) */
    static bool Test( const TBox& a, const TBox& b, const TTolerance& tolerance, SSatBoxesResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  The contact manifold of the boxes, reduced to at most 'maxPoints' points (1 .. 8). True when they touch or
     *  overlap; false for separated boxes (one point with the separation as negative depth) and for an invalid box (no
     *  points).
     */
    static bool ComputeContact( const TBox& a, const TBox& b, const TTolerance& tolerance, UInt32 maxPoints, CTContactManifold< T >& manifold ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  As ComputeContact, with speculative contacts: points of the face (or edge) contact up to 'speculativeDistance'
     *  apart are kept with a negative depth, also for boxes that are separated by at most that distance (the result is
     *  false for those). A negative speculativeDistance counts as 0.
     */
    static bool ComputeContact( const TBox& a, const TBox& b, const TTolerance& tolerance, const T& speculativeDistance, UInt32 maxPoints, CTContactManifold< T >& manifold ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    private:

    /** Overlap of the projections of the boxes on the unit axis (negative: separation) */
    static T AxisOverlap( const TVector ( &axesA )[ 3 ], const TVector& halfA, const TVector ( &axesB )[ 3 ], const TVector& halfB, const TVector& between, const TVector& axis ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The best axis for contact generation: faces preferred over edges, faces of A over faces of B */
    static void SelectContactAxis( const T ( &overlaps )[ 15 ], const bool ( &usable )[ 15 ], const T& slop, UInt32& axis ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** True when 'candidate' is below 'current' by more than 'fraction' of the magnitude of 'current' plus 'slop' */
    static bool ClearlyBelow( const T& candidate, const T& current, const T& fraction, const T& slop ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  Sutherland-Hodgman against one plane: keeps the part with normal . p <= offset; at most count + 1 points.
     *  ids and edges carry the feature id of each point and the feature of the edge from it to the next point (an
     *  incident edge 0..3 or a side plane 4..7); a cut by side plane 'plane' gets the id of that edge and plane.
     */
    static UInt32 ClipPolygon( const TPoint* in, const UInt32* ids, const UInt32* edges, UInt32 count, const TVector& normal, const T& offset, UInt32 plane,
                               TPoint* out, UInt32* outIds, UInt32* outEdges ) GUCEF_GEOMETRY_REALTIME_CONTEXT;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTSatBoxes< T >::AxisOverlap( const TVector ( &axesA )[ 3 ], const TVector& halfA, const TVector ( &axesB )[ 3 ], const TVector& halfB, const TVector& between, const TVector& axis ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const TVector projectionsA( TTraits::Abs( axesA[ 0 ].Dot( axis ) ), TTraits::Abs( axesA[ 1 ].Dot( axis ) ), TTraits::Abs( axesA[ 2 ].Dot( axis ) ) );
    const TVector projectionsB( TTraits::Abs( axesB[ 0 ].Dot( axis ) ), TTraits::Abs( axesB[ 1 ].Dot( axis ) ), TTraits::Abs( axesB[ 2 ].Dot( axis ) ) );
    const T radiusA = halfA.Dot( projectionsA );
    const T radiusB = halfB.Dot( projectionsB );
    return ( radiusA + radiusB ) - TTraits::Abs( between.Dot( axis ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTSatBoxes< T >::Test( const TBox& a, const TBox& b, const TTolerance& tolerance, SSatBoxesResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    result = SSatBoxesResult< T >();
    if ( !a.IsValid() || !b.IsValid() )
        return false;
    TVector axesA[ 3 ];
    TVector axesB[ 3 ];
    a.GetAxes( axesA );
    b.GetAxes( axesB );
    const TVector between = b.centre - a.centre;

    bool first = true;
    bool overlapping = true;
    T extreme = TTraits::Zero();
    UInt32 extremeAxis = 0;
    TVector extremeNormal;
    for ( UInt32 index=0; index<15; ++index )
    {
        TVector axis;
        if ( index < 3 )
            axis = axesA[ index ];
        else if ( index < 6 )
            axis = axesB[ index - 3 ];
        else if ( !axesA[ ( index - 6 ) / 3 ].Cross( axesB[ ( index - 6 ) % 3 ] ).TryNormalize( axis, tolerance.angular ) )
            continue;
        const T overlap = AxisOverlap( axesA, a.halfExtents, axesB, b.halfExtents, between, axis );
        const bool separating = TTraits::IsNegative( overlap );
        /* overlapping: the smallest overlap; separated: the largest separation */
        bool take = first;
        if ( !first )
        {
            if ( separating && overlapping )
                take = true;
            else if ( separating )
                take = overlap < extreme;
            else if ( overlapping )
                take = overlap < extreme;
        }
        if ( separating )
            overlapping = false;
        if ( take )
        {
            first = false;
            extreme = overlap;
            extremeAxis = index;
            extremeNormal = TTraits::IsNegative( between.Dot( axis ) ) ? -axis : axis;
        }
    }
    result.overlapping = overlapping;
    result.axis = extremeAxis;
    result.normal = extremeNormal;
    result.depth = extreme;
    return overlapping;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTSatBoxes< T >::ClearlyBelow( const T& candidate, const T& current, const T& fraction, const T& slop ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return candidate < current - ( fraction * TTraits::Abs( current ) + slop );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTSatBoxes< T >::SelectContactAxis( const T ( &overlaps )[ 15 ], const bool ( &usable )[ 15 ], const T& slop, UInt32& axis ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const T faceFraction = TTraits::FromInt32( 2 ) / TTraits::FromInt32( 100 );
    const T edgeFraction = TTraits::FromInt32( 5 ) / TTraits::FromInt32( 100 );
    UInt32 bestA = 0;
    for ( UInt32 i=1; i<3; ++i )
        if ( overlaps[ i ] < overlaps[ bestA ] )
            bestA = i;
    UInt32 bestB = 3;
    for ( UInt32 i=4; i<6; ++i )
        if ( overlaps[ i ] < overlaps[ bestB ] )
            bestB = i;
    UInt32 best = bestA;
    if ( ClearlyBelow( overlaps[ bestB ], overlaps[ bestA ], faceFraction, slop ) )
        best = bestB;
    UInt32 bestEdge = 15;
    for ( UInt32 i=6; i<15; ++i )
        if ( usable[ i ] && ( 15 == bestEdge || overlaps[ i ] < overlaps[ bestEdge ] ) )
            bestEdge = i;
    if ( bestEdge < 15 && ClearlyBelow( overlaps[ bestEdge ], overlaps[ best ], edgeFraction, slop ) )
        best = bestEdge;
    axis = best;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE UInt32
CTSatBoxes< T >::ClipPolygon( const TPoint* in, const UInt32* ids, const UInt32* edges, UInt32 count, const TVector& normal, const T& offset, UInt32 plane,
                              TPoint* out, UInt32* outIds, UInt32* outEdges ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    UInt32 written = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const UInt32 next = ( i + 1 ) % count;
        const TPoint& p = in[ i ];
        const TPoint& q = in[ next ];
        const T dp = normal.Dot( p.ToPositionVector() ) - offset;
        const T dq = normal.Dot( q.ToPositionVector() ) - offset;
        const bool pInside = !( TTraits::Zero() < dp );
        const bool qInside = !( TTraits::Zero() < dq );
        if ( pInside )
        {
            out[ written ] = p;
            outIds[ written ] = ids[ i ];
            outEdges[ written ] = edges[ i ];
            ++written;
        }
        if ( pInside != qInside )
        {
            const T t = dp / ( dp - dq );
            const UInt32 edge = edges[ i ];
            out[ written ] = p + ( q - p ) * t;
            if ( edge < 4 )
            {
                outIds[ written ] = 4 + 4 * edge + plane;
            }
            else
            {
                const UInt32 other = edge - 4;
                outIds[ written ] = other < plane ? 20 + 4 * other + plane : 20 + 4 * plane + other;
            }
            /* leaving: the polygon continues along the plane; entering: along the edge that was cut */
            outEdges[ written ] = pInside ? 4 + plane : edge;
            ++written;
        }
    }
    return written;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTSatBoxes< T >::ComputeContact( const TBox& a, const TBox& b, const TTolerance& tolerance, UInt32 maxPoints, CTContactManifold< T >& manifold ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ComputeContact( a, b, tolerance, TTraits::Zero(), maxPoints, manifold );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTSatBoxes< T >::ComputeContact( const TBox& a, const TBox& b, const TTolerance& tolerance, const T& speculativeDistance, UInt32 maxPoints, CTContactManifold< T >& manifold ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    manifold.Clear();
    if ( !a.IsValid() || !b.IsValid() )
        return false;

    TVector axesA[ 3 ];
    TVector axesB[ 3 ];
    a.GetAxes( axesA );
    b.GetAxes( axesB );
    const TVector between = b.centre - a.centre;
    const T scale = TTraits::Max( GetCoordinateScale( a.centre ), GetCoordinateScale( b.centre ) ) +
                    TTraits::Max( a.halfExtents.GetChebyshevLength(), b.halfExtents.GetChebyshevLength() );
    const T lengthTolerance = tolerance.GetLengthTolerance( scale );
    const T speculative = TTraits::IsNegative( speculativeDistance ) ? TTraits::Zero() : speculativeDistance;
    const T smallestHalf = TTraits::Min( TTraits::Min( TTraits::Min( a.halfExtents.x, a.halfExtents.y ), TTraits::Min( a.halfExtents.z, b.halfExtents.x ) ),
                                         TTraits::Min( b.halfExtents.y, b.halfExtents.z ) );
    const T slop = TTraits::Max( lengthTolerance, smallestHalf / TTraits::FromInt32( 1024 ) );
    /* edge pairs closer to parallel than this squared sine are not used for contact generation */
    const T minimumEdgeSine = TTraits::FromInt32( 1 ) / TTraits::FromInt32( 1024 );
    const T minimumEdgeSquaredSine = minimumEdgeSine * minimumEdgeSine;

    T overlaps[ 15 ];
    bool usable[ 15 ];
    TVector axes[ 15 ];
    bool separated = false;
    bool beyondSpeculative = false;
    for ( UInt32 index=0; index<15; ++index )
    {
        usable[ index ] = true;
        overlaps[ index ] = TTraits::Max();
        if ( index < 3 )
        {
            axes[ index ] = axesA[ index ];
        }
        else if ( index < 6 )
        {
            axes[ index ] = axesB[ index - 3 ];
        }
        else
        {
            const TVector cross = axesA[ ( index - 6 ) / 3 ].Cross( axesB[ ( index - 6 ) % 3 ] );
            if ( !cross.TryNormalize( axes[ index ], tolerance.angular ) )
            {
                usable[ index ] = false;
                continue;
            }
            usable[ index ] = !( cross.GetSquaredLength() < minimumEdgeSquaredSine );
        }
        overlaps[ index ] = AxisOverlap( axesA, a.halfExtents, axesB, b.halfExtents, between, axes[ index ] );
        if ( TTraits::IsNegative( overlaps[ index ] + lengthTolerance ) )
            separated = true;
        if ( TTraits::IsNegative( overlaps[ index ] + lengthTolerance + speculative ) )
            beyondSpeculative = true;
    }

    if ( beyondSpeculative )
    {
        SGjkResult< T > gjk;
        manifold.status = CTGjk< T >::ComputeDistance( CTShapeSupport< TBox >( a ), CTShapeSupport< TBox >( b ), tolerance, GUCEF_NULL, gjk );
        manifold.normal = gjk.normal;
        manifold.AddPoint( gjk.pointOnA, gjk.pointOnB, MATH::ComponentNegate( gjk.distance ), 0x4000 );
        return false;
    }

    UInt32 axis = 0;
    SelectContactAxis( overlaps, usable, slop, axis );
    const TVector normal = TTraits::IsNegative( between.Dot( axes[ axis ] ) ) ? -axes[ axis ] : axes[ axis ];
    manifold.normal = normal;

    if ( axis >= 6 )
    {
        /* edge against edge: the supporting edges along the normal, their nearest points */
        const UInt32 i = ( axis - 6 ) / 3;
        const UInt32 j = ( axis - 6 ) % 3;
        const T halfA[ 3 ] = { a.halfExtents.x, a.halfExtents.y, a.halfExtents.z };
        const T halfB[ 3 ] = { b.halfExtents.x, b.halfExtents.y, b.halfExtents.z };
        TPoint edgeCentreA = a.centre;
        TPoint edgeCentreB = b.centre;
        for ( UInt32 k=0; k<3; ++k )
        {
            if ( k != i )
                edgeCentreA = edgeCentreA + axesA[ k ] * ( TTraits::IsNegative( normal.Dot( axesA[ k ] ) ) ? MATH::ComponentNegate( halfA[ k ] ) : halfA[ k ] );
            if ( k != j )
                edgeCentreB = edgeCentreB + axesB[ k ] * ( TTraits::IsNegative( normal.Dot( axesB[ k ] ) ) ? halfB[ k ] : MATH::ComponentNegate( halfB[ k ] ) );
        }
        const CTSegment3D< T > edgeA( edgeCentreA - axesA[ i ] * halfA[ i ], edgeCentreA + axesA[ i ] * halfA[ i ] );
        const CTSegment3D< T > edgeB( edgeCentreB - axesB[ j ] * halfB[ j ], edgeCentreB + axesB[ j ] * halfB[ j ] );
        TPoint onA;
        TPoint onB;
        ClosestPoints( edgeA, edgeB, onA, onB );
        manifold.AddPoint( onA, onB, overlaps[ axis ], 0x2000 | ( 3 * i + j ) );
        return !separated;
    }

    /* face contact: reference face on the box of the axis, incident face on the other box */
    const bool referenceIsB = axis >= 3;
    const TBox& reference = referenceIsB ? b : a;
    const TBox& incident = referenceIsB ? a : b;
    const TVector* referenceAxes = referenceIsB ? axesB : axesA;
    const TVector* incidentAxes = referenceIsB ? axesA : axesB;
    const UInt32 referenceAxis = referenceIsB ? axis - 3 : axis;
    const TVector referenceNormal = referenceIsB ? -normal : normal;
    const T referenceHalf[ 3 ] = { reference.halfExtents.x, reference.halfExtents.y, reference.halfExtents.z };
    const T incidentHalf[ 3 ] = { incident.halfExtents.x, incident.halfExtents.y, incident.halfExtents.z };

    UInt32 incidentAxis = 0;
    T incidentDot = incidentAxes[ 0 ].Dot( referenceNormal );
    for ( UInt32 k=1; k<3; ++k )
    {
        const T d = incidentAxes[ k ].Dot( referenceNormal );
        if ( TTraits::Abs( incidentDot ) < TTraits::Abs( d ) )
        {
            incidentDot = d;
            incidentAxis = k;
        }
    }
    /* the incident face faces against the reference normal */
    const bool incidentNegative = !TTraits::IsNegative( incidentDot );
    const TVector incidentFaceNormal = incidentNegative ? -incidentAxes[ incidentAxis ] : incidentAxes[ incidentAxis ];
    const UInt32 u = ( incidentAxis + 1 ) % 3;
    const UInt32 v = ( incidentAxis + 2 ) % 3;
    const TPoint incidentCentre = incident.centre + incidentFaceNormal * incidentHalf[ incidentAxis ];
    const TVector eu = incidentAxes[ u ] * incidentHalf[ u ];
    const TVector ev = incidentAxes[ v ] * incidentHalf[ v ];
    TPoint polygon[ 8 ];
    TPoint clipped[ 8 ];
    UInt32 ids[ 8 ];
    UInt32 clippedIds[ 8 ];
    UInt32 edges[ 8 ];
    UInt32 clippedEdges[ 8 ];
    polygon[ 0 ] = incidentCentre - eu - ev;
    polygon[ 1 ] = incidentCentre + eu - ev;
    polygon[ 2 ] = incidentCentre + eu + ev;
    polygon[ 3 ] = incidentCentre - eu + ev;
    for ( UInt32 k=0; k<4; ++k )
    {
        ids[ k ] = k;
        edges[ k ] = k;
    }
    UInt32 count = 4;

    const UInt32 ru = ( referenceAxis + 1 ) % 3;
    const UInt32 rv = ( referenceAxis + 2 ) % 3;
    const TVector sideNormals[ 4 ] = { referenceAxes[ ru ], -referenceAxes[ ru ], referenceAxes[ rv ], -referenceAxes[ rv ] };
    const T sideHalf[ 4 ] = { referenceHalf[ ru ], referenceHalf[ ru ], referenceHalf[ rv ], referenceHalf[ rv ] };
    for ( UInt32 s=0; s<4 && count > 0; ++s )
    {
        const T offset = sideNormals[ s ].Dot( reference.centre.ToPositionVector() ) + sideHalf[ s ];
        count = ClipPolygon( polygon, ids, edges, count, sideNormals[ s ], offset, s, clipped, clippedIds, clippedEdges );
        for ( UInt32 k=0; k<count; ++k )
        {
            polygon[ k ] = clipped[ k ];
            ids[ k ] = clippedIds[ k ];
            edges[ k ] = clippedEdges[ k ];
        }
    }

    const TPoint referenceFaceCentre = reference.centre + referenceNormal * referenceHalf[ referenceAxis ];
    const UInt32 referenceFace = 2 * referenceAxis + ( TTraits::IsNegative( referenceNormal.Dot( referenceAxes[ referenceAxis ] ) ) ? 1 : 0 );
    const UInt32 incidentFace = 2 * incidentAxis + ( incidentNegative ? 1 : 0 );
    const T keepLimit = lengthTolerance + speculative;
    for ( UInt32 k=0; k<count; ++k )
    {
        const T separation = referenceNormal.Dot( polygon[ k ] - referenceFaceCentre );
        if ( keepLimit < separation )
            continue;
        const TPoint onReference = polygon[ k ] - referenceNormal * separation;
        const UInt32 feature = ids[ k ] | ( referenceFace << 6 ) | ( referenceIsB ? 0x200u : 0u ) | ( incidentFace << 10 );
        if ( referenceIsB )
            manifold.AddPoint( polygon[ k ], onReference, MATH::ComponentNegate( separation ), feature );
        else
            manifold.AddPoint( onReference, polygon[ k ], MATH::ComponentNegate( separation ), feature );
    }
    if ( 0 == manifold.GetPointCount() )
    {
        /* rounding removed every clipped point (a touching corner): the nearest points of the boxes */
        SGjkResult< T > gjk;
        manifold.status = CTGjk< T >::ComputeDistance( CTShapeSupport< TBox >( a ), CTShapeSupport< TBox >( b ), tolerance, GUCEF_NULL, gjk );
        manifold.AddPoint( gjk.pointOnA, gjk.pointOnB, separated ? MATH::ComponentNegate( gjk.distance ) : overlaps[ axis ], 0x4000 );
    }
    manifold.Reduce( maxPoints );
    return !separated;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTSatBoxes< Float32 > TFloat32SatBoxes;
typedef CTSatBoxes< Float64 > TFloat64SatBoxes;
typedef CTSatBoxes< MATH::TFixed32_32 > TFixed32_32SatBoxes;

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTSatBoxes< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTSatBoxes< Float64 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTSatBoxes< MATH::TFixed32_32 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTSATBOXES_H ? */
