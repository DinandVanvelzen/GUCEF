/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

#ifndef GUCEF_MATH_CTVBAP3D_H
#define GUCEF_MATH_CTVBAP3D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTRADIAN_H
#include "gucefMATH_CTRadian.h"
#define GUCEF_MATH_CTRADIAN_H
#endif /* GUCEF_MATH_CTRADIAN_H ? */

#ifndef GUCEF_MATH_CTMATRIX3D_H
#include "gucefMATH_CTMatrix3D.h"
#define GUCEF_MATH_CTMATRIX3D_H
#endif /* GUCEF_MATH_CTMATRIX3D_H ? */

#ifndef GUCEF_MATH_CTVECTOR3D_H
#include "gucefMATH_CTVector3D.h"
#define GUCEF_MATH_CTVECTOR3D_H
#endif /* GUCEF_MATH_CTVECTOR3D_H ? */

#ifndef GUCEF_MATH_CTDIRECTIONVECTOR3D_H
#include "gucefMATH_CTDirectionVector3D.h"
#define GUCEF_MATH_CTDIRECTIONVECTOR3D_H
#endif /* GUCEF_MATH_CTDIRECTIONVECTOR3D_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Three dimensional vector base amplitude panning over triangles of speakers (section 4.7.5 of the design; V. Pulkki,
 *  "Virtual Sound Source Positioning Using Vector Base Amplitude Panning", JAES 45(6), 1997). Also the triangulation that
 *  interpolates between measured directions (HRTF sets).
 *
 *  - Directions are in the frame of the listener, ( x right, y up, z forward ) as CTOrthonormalFrame3D::ToFrameCoordinates
 *    gives them (normalise the result); speaker positions are given as azimuth (0 ahead, positive right) and elevation
 *    (positive up) as in CTSphericalCoordinates.
 *  - Prepare triangulates the speakers as the convex hull of their unit vectors (every speaker is a vertex of that
 *    hull, because all lie on a sphere): a small dedicated routine over at most 66 points, allocation free on fixed
 *    arrays, by gift wrapping (every facet is found by turning a plane about an edge of the previous one), which is
 *    O( n^2 ) for the 2 n - 4 facets of n points. Coplanar speakers (the corners of a cube face) form one planar facet that
 *    is split into a fan from its point with the smallest coordinates, so the triangles do not depend on the order of the
 *    speakers. Only the facets that have the listener on their inner side are used: for a layout that does not surround the
 *    listener (speakers in front only) these are the facets that face away from the listener, not the ones behind them.
 *    When no speaker is clearly below the horizon (unit vector y of -0.1 or less) a virtual speaker is added at the nadir,
 *    and when none is clearly above it one at the zenith, so that the hull covers the whole sphere. Layouts whose speakers
 *    all lie in one plane get both virtual speakers, on the Y axis, or when the plane contains the Y axis (speakers in the
 *    median plane, a pair one above the other) along the normal of the plane. For every triangle (its three speaker
 *    indices in ascending order) the inverse of the matrix with the three unit vectors as columns is stored.
 *  - ComputeGains scans all triangles (at most 130) and takes the one whose smallest gain ( the gains are the inverse
 *    times the direction ) is the largest, a tie going to the smaller speaker indices: that is the triangle that contains
 *    the direction (all gains non-negative), and the result does not depend on the order of the speakers. The gains of a
 *    virtual speaker are shared equally (by amplitude, 1 / sqrt( m ) each) among the
 *    m real speakers that neighbour it in the hull, so a source straight down is spread over the lowest ring, the
 *    gains stay continuous across the triangles around a virtual speaker and a direction at a virtual speaker is not
 *    silent. The result is normalised to a sum of squares equal to the target power. A direction at a speaker gives that
 *    speaker all of the power. For a direction that no triangle contains (layouts that do not surround the listener, such
 *    as speakers in front only) the triangle with the largest smallest gain is used and its negative gains are set to 0.
 *  - Layouts: 2 to MaxSpeakers speakers; Prepare rejects (returns false and leaves the object unprepared) fewer than 2
 *    or more than MaxSpeakers speakers, a null pointer, non-finite angles, coincident speakers (closer than 1e-5), and
 *    layouts for which no usable triangle exists: a single speaker, and speakers on one line through the listener (a pair
 *    exactly opposite each other, front and back for example, with nothing else), which no virtual speaker can turn into
 *    a solid around the listener. A pair that is not opposite (one above the other, in the median plane or not) is fine.
 *  - Capacity: fixed, MaxSpeakers = 64 plus 2 virtual speakers, at most MaxTriangles = 130 triangles; no heap use at any
 *    point. The object is about 12 kilobytes (Float64), keep it in the owner rather than on a small stack.
 *  - Threading: Prepare and ComputeGains neither allocate, lock nor trace. Prepare does O( n^2 ) work (a fraction of a
 *    millisecond for 64 speakers in an optimised build) and about 20 kilobytes of stack; ComputeGains is const.
 *  - Kind: T is a floating point kind; the hull is computed in Float64 for every kind.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTVbap3D
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef CTVector3D< T > TVector;
    typedef CTDirectionVector3D< T > TDirection;

    GUCEF_MATH_STATIC_ASSERT( TTraits::IsFloatingPoint, Vbap_requires_a_floating_point_kind );

    enum
    {
        MaxSpeakers = 64,
        MaxVirtualSpeakers = 2,
        MaxPoints = MaxSpeakers + MaxVirtualSpeakers,
        MaxTriangles = 2 * MaxSpeakers + MaxVirtualSpeakers     // 2 V - 4 for the V <= 66 vertices of a hull is 128: this is the capacity
    };

    /** An unprepared panner without speakers */
    CTVbap3D( void )
        : m_count( 0 ), m_virtualCount( 0 ), m_triangleCount( 0 )
    {
        for ( UInt32 t = 0; t < (UInt32) MaxTriangles; ++t )
            m_vertex[ t ][ 0 ] = m_vertex[ t ][ 1 ] = m_vertex[ t ][ 2 ] = 0;
        for ( UInt32 v = 0; v < (UInt32) MaxVirtualSpeakers; ++v )
        {
            m_neighbourCount[ v ] = 0;
            m_neighbourWeight[ v ] = TTraits::Zero();
            for ( UInt32 n = 0; n < (UInt32) MaxSpeakers; ++n )
                m_neighbour[ v ][ n ] = 0;
        }
    }

    /**
     *  Prepares the speakers at the given azimuths and elevations (see the class comment for the layouts that are
     *  rejected). On false the object is unprepared (GetSpeakerCount() == 0).
     */
    bool Prepare( const CTRadian< T >* azimuths, const CTRadian< T >* elevations, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Forgets the layout */
    GUCEF_MATH_FORCE_INLINE void Clear( void ) GUCEF_MATH_REALTIME_CONTEXT { m_count = 0; m_virtualCount = 0; m_triangleCount = 0; }

    /** The number of speakers of the prepared layout, 0 when unprepared */
    GUCEF_MATH_FORCE_INLINE UInt32 GetSpeakerCount( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_count; }

    GUCEF_MATH_FORCE_INLINE bool IsPrepared( void ) const GUCEF_MATH_REALTIME_CONTEXT { return 0 != m_count; }

    /** The number of virtual speakers that Prepare added (0, 1 or 2): the nadir first, then the zenith */
    GUCEF_MATH_FORCE_INLINE UInt32 GetVirtualSpeakerCount( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_virtualCount; }

    /** The number of usable triangles of the hull (those that can pan: triangles through the listener position are left out) */
    GUCEF_MATH_FORCE_INLINE UInt32 GetTriangleCount( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_triangleCount; }

    /**
     *  The speaker indices of a triangle, as given to Prepare; an index of GetSpeakerCount() or more is a virtual speaker
     *  (GetSpeakerCount() is the first, the nadir when it exists). False when 'index' is not a triangle.
     */
    bool GetTriangle( UInt32 index, UInt32& a, UInt32& b, UInt32& c ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The gains of the speakers for a source in 'direction' (frame coordinates, see the class comment): writes
     *  GetSpeakerCount() values into 'gains' with the sum of their squares equal to targetPower. An unprepared object
     *  writes nothing; a target power that is not positive writes zeros.
     */
    void ComputeGains( const TDirection& direction, T* gains, const T& targetPower ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** ComputeGains for the direction of the azimuth (0 ahead, positive right) and elevation (positive up) */
    void ComputeGains( const CTRadian< T >& azimuth, const CTRadian< T >& elevation, T* gains, const T& targetPower ) const GUCEF_MATH_REALTIME_CONTEXT;

    private:

    /** The hull of the points: 0 for success, 1 when all points are coplanar, 2 for failure (capacity or no facet) */
    static UInt32 BuildHull( const CTVector3D< Float64 >* points, UInt32 pointCount, UInt32 ( *triangles )[ 3 ], UInt32& triangleCount ) GUCEF_MATH_REALTIME_CONTEXT;

    UInt32 m_count;
    UInt32 m_virtualCount;
    UInt32 m_triangleCount;
    UInt32 m_vertex[ MaxTriangles ][ 3 ];                   // speaker indices; count and above are virtual speakers
    CTMatrix3D< T > m_inverse[ MaxTriangles ];              // inverse of the matrix with the vertex unit vectors as columns
    UInt32 m_neighbourCount[ MaxVirtualSpeakers ];          // real speakers next to each virtual speaker
    UInt32 m_neighbour[ MaxVirtualSpeakers ][ MaxSpeakers ];
    T m_neighbourWeight[ MaxVirtualSpeakers ];              // 1 / sqrt( neighbour count )
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

/* The related classes used by the members, included after the class (any header of the family can be included first) */

#ifndef GUCEF_MATH_CTSPHERICALCOORDINATES_H
#include "gucefMATH_CTSphericalCoordinates.h"
#define GUCEF_MATH_CTSPHERICALCOORDINATES_H
#endif /* GUCEF_MATH_CTSPHERICALCOORDINATES_H ? */

#ifndef GUCEF_MATH_ANGLEFUNCTIONS_H
#include "gucefMATH_AngleFunctions.h"
#define GUCEF_MATH_ANGLEFUNCTIONS_H
#endif /* GUCEF_MATH_ANGLEFUNCTIONS_H ? */

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The precision-sensitive members, out of the class: with GUCEF_MATH_PRECISE_INLINE (empty by default) they are not
 *  inline, so for the predefined kinds the copy compiled in the module (without floating-point contraction) runs.
 */

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE UInt32
CTVbap3D< T >::BuildHull( const CTVector3D< Float64 >* points, UInt32 pointCount, UInt32 ( *triangles )[ 3 ], UInt32& triangleCount ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTVector3D< Float64 > TVector64;
    // points on the unit sphere: a point is on the plane of a facet when it is within this distance of it
    const Float64 planeTolerance = 1.0e-6;
    const Float64 minNormal = 1.0e-9;
    const UInt32 maxWork = 6 * (UInt32) MaxPoints;      // every directed edge of the hull is queued at most once: 2 * ( 3 V - 6 ) <= 384
    const UInt32 virtualFacet = (UInt32) MaxTriangles;  // the slot of the supporting plane that starts the wrap

    triangleCount = 0;
    if ( pointCount < 4 )
        return 1;       // fewer than four points never make a solid

    // gift wrapping: every facet is found by turning a plane about one of its edges until it meets the next point, so
    // O( F n ) = O( n^2 ) work for the F <= 2 V - 4 facets of a hull of V points, with fixed arrays. The facets of the whole
    // hull are found; those that the listener is not inside of (the origin is on their outer side) are not returned.
    TVector64 facetNormal[ MaxTriangles + 1 ];
    UInt32 workU[ 6 * MaxPoints ];
    UInt32 workV[ 6 * MaxPoints ];
    UInt32 workFacet[ 6 * MaxPoints ];
    bool done[ MaxPoints ][ MaxPoints ];                // the directed edge has a facet
    for ( UInt32 i = 0; i < pointCount; ++i )
    {
        for ( UInt32 j = 0; j < pointCount; ++j )
            done[ i ][ j ] = false;
    }

    // the start: the nearest neighbour of the first point is certainly an edge of the hull; the plane through the edge
    // that is perpendicular to the direction of its middle supports the hull
    UInt32 startV = 1;
    Float64 nearest = ( points[ 1 ] - points[ 0 ] ).Dot( points[ 1 ] - points[ 0 ] );
    for ( UInt32 i = 2; i < pointCount; ++i )
    {
        const Float64 distance = ( points[ i ] - points[ 0 ] ).Dot( points[ i ] - points[ 0 ] );
        if ( distance < nearest )
        {
            nearest = distance;
            startV = i;
        }
    }
    if ( !( points[ 0 ] + points[ startV ] ).TryNormalize( facetNormal[ virtualFacet ], minNormal ) )
        return 2;
    workU[ 0 ] = 0;
    workV[ 0 ] = startV;
    workFacet[ 0 ] = virtualFacet;
    UInt32 workCount = 1;
    UInt32 facetCount = 0;

    while ( 0 != workCount )
    {
        --workCount;
        const UInt32 u = workU[ workCount ];
        const UInt32 v = workV[ workCount ];
        if ( done[ u ][ v ] )
            continue;
        const TVector64 previousNormal = facetNormal[ workFacet[ workCount ] ];
        const TVector64& origin = points[ u ];
        const TVector64 edge = points[ v ] - origin;

        // the point that the plane turns to last: the plane through the edge and that point has all points on its inner side.
        // Points on the plane of the neighbouring facet are not candidates.
        UInt32 best = pointCount;
        TVector64 bestNormal = TVector64::Zero();
        bool changed = true;
        UInt32 passes = 0;
        while ( changed )
        {
            changed = false;
            if ( ++passes > pointCount + 2 )
                return 2;
            for ( UInt32 c = 0; c < pointCount; ++c )
            {
                if ( c == u || c == v )
                    continue;
                const TVector64 offset = points[ c ] - origin;
                if ( !( previousNormal.Dot( offset ) < -planeTolerance ) )
                    continue;
                if ( best != pointCount && !( bestNormal.Dot( offset ) > planeTolerance ) )
                    continue;
                TVector64 normal;
                if ( !edge.Cross( offset ).TryNormalize( normal, minNormal ) )
                    continue;
                best = c;
                bestNormal = normal;
                changed = true;
            }
        }
        const bool firstFacet = 0 == facetCount;
        if ( best == pointCount )
            return firstFacet ? 1 : 2;      // nothing off the plane: all points are coplanar

        // the points of the facet: those on its plane
        UInt32 members[ MaxPoints ];
        UInt32 memberCount = 0;
        for ( UInt32 m = 0; m < pointCount; ++m )
        {
            const Float64 distance = bestNormal.Dot( points[ m ] - origin );
            if ( m == u || m == v || m == best || ( distance <= planeTolerance && distance >= -planeTolerance ) )
                members[ memberCount++ ] = m;
        }
        if ( firstFacet && memberCount == pointCount )
            return 1;       // all points on one plane

        // the polygon in counter clockwise order seen from outside, starting at the point with the smallest coordinates (so
        // that the triangulation does not depend on the order of the points)
        if ( memberCount > 3 )
        {
            UInt32 apex = 0;
            for ( UInt32 m = 1; m < memberCount; ++m )
            {
                const TVector64& a = points[ members[ m ] ];
                const TVector64& b = points[ members[ apex ] ];
                if ( a.x < b.x || ( a.x == b.x && ( a.y < b.y || ( a.y == b.y && a.z < b.z ) ) ) )
                    apex = m;
            }
            TVector64 centre = TVector64::Zero();
            for ( UInt32 m = 0; m < memberCount; ++m )
                centre += points[ members[ m ] ];
            centre = centre / Float64( memberCount );
            const UInt32 apexPoint = members[ apex ];
            const TVector64 axisU = ( points[ apexPoint ] - centre ).Normalized();
            const TVector64 axisV = bestNormal.Cross( axisU );
            Float64 polygonAngle[ MaxPoints ];
            UInt32 sorted[ MaxPoints ];
            for ( UInt32 m = 0; m < memberCount; ++m )
            {
                const UInt32 member = members[ m ];
                Float64 angle = 0.0;
                if ( member != apexPoint )
                {
                    const TVector64 offset = points[ member ] - centre;
                    angle = CTNumericTraits< Float64 >::Atan2( axisV.Dot( offset ), axisU.Dot( offset ) );
                    if ( angle <= 0.0 )
                        angle += 6.283185307179586476925286766559;
                }
                UInt32 position = m;
                while ( position > 0 && angle < polygonAngle[ position - 1 ] )
                {
                    polygonAngle[ position ] = polygonAngle[ position - 1 ];
                    sorted[ position ] = sorted[ position - 1 ];
                    --position;
                }
                polygonAngle[ position ] = angle;
                sorted[ position ] = member;
            }
            for ( UInt32 m = 0; m < memberCount; ++m )
                members[ m ] = sorted[ m ];
        }
        else
        {
            // a triangle: counter clockwise about the normal
            const TVector64& p0 = points[ members[ 0 ] ];
            if ( bestNormal.Dot( ( points[ members[ 1 ] ] - p0 ).Cross( points[ members[ 2 ] ] - p0 ) ) < 0.0 )
            {
                const UInt32 swap = members[ 1 ];
                members[ 1 ] = members[ 2 ];
                members[ 2 ] = swap;
            }
        }

        if ( facetCount >= (UInt32) MaxTriangles )
            return 2;
        const UInt32 facet = facetCount++;
        facetNormal[ facet ] = bestNormal;
        for ( UInt32 m = 0; m < memberCount; ++m )
            done[ members[ m ] ][ members[ ( m + 1 ) % memberCount ] ] = true;
        for ( UInt32 m = 0; m < memberCount; ++m )
        {
            const UInt32 x = members[ m ];
            const UInt32 y = members[ ( m + 1 ) % memberCount ];
            if ( done[ y ][ x ] )
                continue;
            if ( workCount >= maxWork )
                return 2;
            workU[ workCount ] = y;
            workV[ workCount ] = x;
            workFacet[ workCount ] = facet;
            ++workCount;
        }

        // the listener has to be on the inner side of a facet that can pan; a facet in a plane through the listener is kept
        // for the neighbours of the virtual speakers and left out later
        if ( bestNormal.Dot( origin ) < -planeTolerance )
            continue;
        for ( UInt32 m = 1; m + 1 < memberCount; ++m )
        {
            if ( triangleCount >= (UInt32) MaxTriangles )
                return 2;
            triangles[ triangleCount ][ 0 ] = members[ 0 ];
            triangles[ triangleCount ][ 1 ] = members[ m ];
            triangles[ triangleCount ][ 2 ] = members[ m + 1 ];
            ++triangleCount;
        }
    }
    return 0 != triangleCount ? 0 : 2;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTVbap3D< T >::Prepare( const CTRadian< T >* azimuths, const CTRadian< T >* elevations, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    typedef CTVector3D< Float64 > TVector64;
    Clear();
    if ( GUCEF_NULL == azimuths || GUCEF_NULL == elevations || count < 2 || count > (UInt32) MaxSpeakers )
        return false;

    // the unit vectors of the speakers in the kind and in Float64 (the hull is always computed in Float64)
    TVector vectors[ MaxPoints ];
    TVector64 points[ MaxPoints ];
    bool below = false;
    bool above = false;
    for ( UInt32 i = 0; i < count; ++i )
    {
        if ( !TTraits::IsFinite( azimuths[ i ].GetValue() ) || !TTraits::IsFinite( elevations[ i ].GetValue() ) )
            return false;
        vectors[ i ] = CTSphericalCoordinates< T >( TTraits::One(), azimuths[ i ], elevations[ i ] ).ToFrameCoordinates();
        points[ i ] = TVector64( Float64( vectors[ i ].x ), Float64( vectors[ i ].y ), Float64( vectors[ i ].z ) );
        below = below || points[ i ].y < -0.1;
        above = above || points[ i ].y > 0.1;
    }
    for ( UInt32 i = 0; i < count; ++i )
    {
        for ( UInt32 j = i + 1; j < count; ++j )
        {
            if ( ( points[ i ] - points[ j ] ).GetLength() <= 1.0e-5 )
                return false;           // coincident speakers
        }
    }

    UInt32 pointCount = count;
    const bool nadirNeeded = !below;
    const bool zenithNeeded = !above;
    UInt32 triangles[ MaxTriangles ][ 3 ];
    UInt32 triangleCount = 0;
    UInt32 status = 2;
    for ( UInt32 attempt = 0; attempt < 3; ++attempt )
    {
        // 0: the virtual speakers that the layout needs; 1: both, on the Y axis (all speakers in one plane); 2: both, along the
        // normal of the plane of the layout (that plane contains the Y axis, so the Y axis does not make a solid)
        if ( 1 == attempt && nadirNeeded && zenithNeeded )
            continue;       // attempt 0 was this one
        pointCount = count;
        TVector64 axis( 0.0, 1.0, 0.0 );
        if ( 2 == attempt )
        {
            // the points are the speakers and the virtual speakers on the Y axis, all in one plane: its normal is the
            // largest cross product of the differences with the first point, with a sign that does not depend on the order
            TVector64 normal = TVector64::Zero();
            Float64 largest = 0.0;
            const UInt32 planarCount = count + 2;      // the speakers and the two virtual speakers on the Y axis of the last attempt
            for ( UInt32 j = 1; j + 1 < planarCount; ++j )
            {
                for ( UInt32 k = j + 1; k < planarCount; ++k )
                {
                    const TVector64 cross = ( points[ j ] - points[ 0 ] ).Cross( points[ k ] - points[ 0 ] );
                    const Float64 length = cross.GetLength();
                    if ( largest < length )
                    {
                        largest = length;
                        normal = cross;
                    }
                }
            }
            if ( !normal.TryNormalize( axis, 1.0e-9 ) )
                break;
            if ( axis.x < -1.0e-9 || ( axis.x <= 1.0e-9 && ( axis.y < -1.0e-9 || ( axis.y <= 1.0e-9 && axis.z < 0.0 ) ) ) )
                axis = -axis;
        }
        if ( 0 != attempt || nadirNeeded )
        {
            vectors[ pointCount ] = TVector( (T) -axis.x, (T) -axis.y, (T) -axis.z );
            points[ pointCount ] = TVector64( Float64( vectors[ pointCount ].x ), Float64( vectors[ pointCount ].y ), Float64( vectors[ pointCount ].z ) );
            ++pointCount;
        }
        if ( 0 != attempt || zenithNeeded )
        {
            vectors[ pointCount ] = TVector( (T) axis.x, (T) axis.y, (T) axis.z );
            points[ pointCount ] = TVector64( Float64( vectors[ pointCount ].x ), Float64( vectors[ pointCount ].y ), Float64( vectors[ pointCount ].z ) );
            ++pointCount;
        }
        status = BuildHull( points, pointCount, triangles, triangleCount );
        if ( 1 != status )
            break;
    }
    if ( 0 != status )
        return false;

    // the matrices of the triangles; a triangle in a plane through the listener cannot pan and is left out
    const T singularTolerance = TTraits::Epsilon() * TTraits::FromInt32( 64 );
    UInt32 usable = 0;
    for ( UInt32 t = 0; t < triangleCount; ++t )
    {
        // the vertices in ascending order: the triangle does not depend on the order in which the hull found its corners
        UInt32 corner[ 3 ] = { triangles[ t ][ 0 ], triangles[ t ][ 1 ], triangles[ t ][ 2 ] };
        for ( UInt32 i = 1; i < 3; ++i )
        {
            for ( UInt32 j = i; j > 0 && corner[ j ] < corner[ j - 1 ]; --j )
            {
                const UInt32 swap = corner[ j ];
                corner[ j ] = corner[ j - 1 ];
                corner[ j - 1 ] = swap;
            }
        }
        const CTMatrix3D< T > basis = CTMatrix3D< T >::FromColumns( vectors[ corner[ 0 ] ], vectors[ corner[ 1 ] ], vectors[ corner[ 2 ] ] );
        if ( !basis.TryInverse( m_inverse[ usable ], singularTolerance ) )
            continue;
        m_vertex[ usable ][ 0 ] = corner[ 0 ];
        m_vertex[ usable ][ 1 ] = corner[ 1 ];
        m_vertex[ usable ][ 2 ] = corner[ 2 ];
        ++usable;
    }
    if ( 0 == usable )
        return false;

    // the real speakers next to each virtual speaker (all hull triangles count, also those left out above)
    const UInt32 virtualCount = pointCount - count;
    for ( UInt32 v = 0; v < virtualCount; ++v )
    {
        bool neighbour[ MaxSpeakers ];
        for ( UInt32 i = 0; i < count; ++i )
            neighbour[ i ] = false;
        for ( UInt32 t = 0; t < triangleCount; ++t )
        {
            bool contains = false;
            for ( UInt32 c = 0; c < 3; ++c )
                contains = contains || triangles[ t ][ c ] == count + v;
            if ( !contains )
                continue;
            for ( UInt32 c = 0; c < 3; ++c )
            {
                if ( triangles[ t ][ c ] < count )
                    neighbour[ triangles[ t ][ c ] ] = true;
            }
        }
        UInt32 neighbourCount = 0;
        for ( UInt32 i = 0; i < count; ++i )
        {
            if ( neighbour[ i ] )
                m_neighbour[ v ][ neighbourCount++ ] = i;
        }
        if ( 0 == neighbourCount )
            return false;
        m_neighbourCount[ v ] = neighbourCount;
        m_neighbourWeight[ v ] = TTraits::One() / TTraits::Sqrt( TTraits::FromInt32( (Int32) neighbourCount ) );
    }

    m_count = count;
    m_virtualCount = virtualCount;
    m_triangleCount = usable;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTVbap3D< T >::GetTriangle( UInt32 index, UInt32& a, UInt32& b, UInt32& c ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( index >= m_triangleCount )
        return false;
    a = m_vertex[ index ][ 0 ];
    b = m_vertex[ index ][ 1 ];
    c = m_vertex[ index ][ 2 ];
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE void
CTVbap3D< T >::ComputeGains( const TDirection& direction, T* gains, const T& targetPower ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == m_count || GUCEF_NULL == gains )
        return;
    for ( UInt32 i = 0; i < m_count; ++i )
        gains[ i ] = TTraits::Zero();
    if ( !( TTraits::Zero() < targetPower ) )
        return;

    // the triangle whose smallest gain is the largest (a direction inside a triangle has all gains non-negative there; for a
    // direction that no triangle contains it is the least negative one); a tie goes to the smaller vertex indices, so the
    // result does not depend on the order of the triangles, which follows the order of the speakers
    const TVector d = direction.AsVector();
    UInt32 chosen = m_triangleCount;
    TVector chosenGains;
    T chosenSmallest = TTraits::Zero();
    for ( UInt32 t = 0; t < m_triangleCount; ++t )
    {
        const TVector g = m_inverse[ t ].Transform( d );
        const T smallest = TTraits::Min( g.x, TTraits::Min( g.y, g.z ) );
        if ( !TTraits::IsFinite( smallest ) )
            continue;
        bool better = chosen == m_triangleCount || chosenSmallest < smallest;
        if ( !better && !( smallest < chosenSmallest ) )
        {
            for ( UInt32 c = 0; c < 3; ++c )
            {
                if ( m_vertex[ t ][ c ] != m_vertex[ chosen ][ c ] )
                {
                    better = m_vertex[ t ][ c ] < m_vertex[ chosen ][ c ];
                    break;
                }
            }
        }
        if ( better )
        {
            chosen = t;
            chosenGains = g;
            chosenSmallest = smallest;
        }
    }

    if ( chosen != m_triangleCount )
    {
        const T triangleGains[ 3 ] = { chosenGains.x, chosenGains.y, chosenGains.z };
        for ( UInt32 c = 0; c < 3; ++c )
        {
            const T gain = triangleGains[ c ] < TTraits::Zero() ? TTraits::Zero() : triangleGains[ c ];
            const UInt32 vertex = m_vertex[ chosen ][ c ];
            if ( vertex < m_count )
            {
                gains[ vertex ] += gain;
                continue;
            }
            // a virtual speaker: its gain is shared by the real speakers next to it
            const UInt32 v = vertex - m_count;
            const T share = gain * m_neighbourWeight[ v ];
            for ( UInt32 n = 0; n < m_neighbourCount[ v ]; ++n )
                gains[ m_neighbour[ v ][ n ] ] += share;
        }
    }

    T power = TTraits::Zero();
    for ( UInt32 i = 0; i < m_count; ++i )
        power += gains[ i ] * gains[ i ];
    if ( TTraits::Zero() < power && TTraits::IsFinite( power ) )
    {
        const T scale = TTraits::Sqrt( targetPower / power );
        for ( UInt32 i = 0; i < m_count; ++i )
            gains[ i ] *= scale;
    }
    else
    {
        const T uniform = TTraits::Sqrt( targetPower / TTraits::FromInt32( (Int32) m_count ) );
        for ( UInt32 i = 0; i < m_count; ++i )
            gains[ i ] = uniform;
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE void
CTVbap3D< T >::ComputeGains( const CTRadian< T >& azimuth, const CTRadian< T >& elevation, T* gains, const T& targetPower ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const TVector v = CTSphericalCoordinates< T >( TTraits::One(), azimuth, elevation ).ToFrameCoordinates();
    TDirection direction;
    if ( !TDirection::TryCreate( v, direction ) )
    {
        // not finite angles: no direction, no gains
        if ( 0 != m_count && GUCEF_NULL != gains )
        {
            for ( UInt32 i = 0; i < m_count; ++i )
                gains[ i ] = TTraits::Zero();
        }
        return;
    }
    ComputeGains( direction, gains, targetPower );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTVbap3D< Float32 > TFloat32Vbap3D;
typedef CTVbap3D< Float64 > TFloat64Vbap3D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The precision-sensitive members of the predefined kinds are instantiated in the module and declared extern here
 *  (section 5.10 of the design), member by member. A list macro takes the prefix (GUCEF_MATH_EXTERN_TEMPLATE in
 *  headers, GUCEF_MATH_INSTANTIATE_TEMPLATE in the module source) and the kind (a name without commas).
 */

#define GUCEF_MATH_CTVBAP3D_REAL_KIND( prefix, kind ) \
    prefix UInt32 CTVbap3D< kind >::BuildHull( const CTVector3D< Float64 >*, UInt32, UInt32 ( * )[ 3 ], UInt32& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTVbap3D< kind >::Prepare( const CTRadian< kind >*, const CTRadian< kind >*, UInt32 ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTVbap3D< kind >::GetTriangle( UInt32, UInt32&, UInt32&, UInt32& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix void CTVbap3D< kind >::ComputeGains( const CTDirectionVector3D< kind >&, kind*, const kind& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix void CTVbap3D< kind >::ComputeGains( const CTRadian< kind >&, const CTRadian< kind >&, kind*, const kind& ) const GUCEF_MATH_REALTIME_CONTEXT;

/** Every predefined floating point kind */
#define GUCEF_MATH_CTVBAP3D_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTVBAP3D_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTVBAP3D_REAL_KIND( prefix, Float64 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTVBAP3D_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTVBAP3D_H ? */
