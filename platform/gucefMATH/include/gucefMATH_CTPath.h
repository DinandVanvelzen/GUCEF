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

#ifndef GUCEF_MATH_CTPATH_H
#define GUCEF_MATH_CTPATH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <cmath>
#include <limits>

#ifndef GUCEF_MATH_CTCURVEPOINTTRAITS_H
#include "gucefMATH_CTCurvePointTraits.h"
#define GUCEF_MATH_CTCURVEPOINTTRAITS_H
#endif /* GUCEF_MATH_CTCURVEPOINTTRAITS_H ? */

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
 *  A baked polyline with cumulative lengths: the arc length parametrisation that games use for moving things along a
 *  curve at constant speed. A path is built from a point list or sampled from any curve, and then answers distance
 *  based questions in O( log n ) (point and tangent at a distance) or O( n ) (closest point).
 *
 *  - TPoint: see CTCurvePointTraits (Float32 / Float64, CTVector2D / 3D / 4D, CTPoint2D / 3D).
 *  - The polyline has GetPointCount() points and GetPointCount() - 1 segments; consecutive duplicate points give
 *    zero length segments which every query skips. A path with fewer than two points has length 0.
 *  - Distances are clamped to [ 0, GetLength() ]. An empty path returns the default point and a zero vector.
 *  - Queries are realtime-safe (no allocation); building and Resample() allocate (not realtime-safe).
 *  - DataNode: a child node 'point' per point (the lengths are recomputed on reading).
 */
template < typename TPoint >
class GUCEF_MATH_TEMPLATE_PUBLIC CTPath
{
    public:

    typedef CTCurvePointTraits< TPoint > TTraits;
    typedef typename TTraits::TScalar TScalar;
    typedef typename TTraits::TVector TVector;

    CTPath( void );

    void Clear( void );

    /** Appends a point and the length of the new segment */
    void AddPoint( const TPoint& point );

    /** Replaces the path with the polyline through 'count' points */
    void SetPoints( const TPoint* points, UInt32 count );

    /**
     *  Replaces the path with 'segmentCount' ( at least 1 ) equal steps in the parameter of 'curve' over its whole
     *  range; any curve of gucefMATH or a type with Evaluate, GetMinParameter and GetMaxParameter works
     */
    template < typename TCurve >
    void SetFromCurve( const TCurve& curve, UInt32 segmentCount )
    {
        Clear();
        const UInt32 count = segmentCount > 0 ? segmentCount : 1;
        const TScalar start = curve.GetMinParameter();
        const TScalar end = curve.GetMaxParameter();
        for ( UInt32 i=0; i<=count; ++i )
            AddPoint( curve.Evaluate( i == count ? end : start + ( end - start ) * ( TScalar( i ) / TScalar( count ) ) ) );
    }

    GUCEF_MATH_FORCE_INLINE UInt32 GetPointCount( void ) const GUCEF_MATH_REALTIME_CONTEXT { return (UInt32) m_points.size(); }

    /** Point 'index'; the default point when out of range */
    TPoint GetPoint( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Length from the start up to point 'index'; 0 when out of range */
    TScalar GetDistanceOfPoint( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Total length */
    TScalar GetLength( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The point at 'distance' along the path (clamped to the path) */
    TPoint GetPointAtDistance( const TScalar& distance ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The unit direction of travel at 'distance' (the direction of the segment; at a corner the segment that
     *  starts there, at the end the last segment). The zero vector when the path has no length.
     */
    TVector GetTangentAtDistance( const TScalar& distance ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The point of the polyline nearest to 'point': stores it in 'closest' and its distance along the path in
     *  'distanceAlongPath'. False (outputs unchanged) for a path without points. Scans the segments and skips those
     *  that cannot beat the best so far by a coarse bound.
     */
    bool GetClosestPoint( const TPoint& point, TPoint& closest, TScalar& distanceAlongPath ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  Resamples into 'result' with points 'spacing' ( > 0 ) apart along the arc, ending exactly at the last point of
     *  this path: ceil( length / spacing ) + 1 points (the last step may be shorter). 'result' must not be this
     *  path. An invalid spacing or a path of fewer than two points gives a copy and true. When the result would have
     *  more than 'maxPoints' points ( default 16777216 ) nothing is done and false is returned ('result' unchanged),
     *  so a tiny spacing can neither loop for long nor exhaust memory.
     */
    bool Resample( const TScalar& spacing, CTPath& result, UInt32 maxPoints = 16777216 ) const;

    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const;
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings );

    private:

    /** The segment index with distance i <= d ( skipping zero length segments from the end ) */
    UInt32 FindSegment( const TScalar& distance ) const GUCEF_MATH_REALTIME_CONTEXT;

    GUCEF::vector< TPoint > m_points;
    GUCEF::vector< TScalar > m_distances;   /**< cumulative length at every point, first is 0 */
};

/*-------------------------------------------------------------------------*/

template < typename TPoint >
CTPath< TPoint >::CTPath( void )
    : m_points()
    , m_distances()
{
}

template < typename TPoint >
void
CTPath< TPoint >::Clear( void )
{
    m_points.clear();
    m_distances.clear();
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE void
CTPath< TPoint >::AddPoint( const TPoint& point )
{
    const TScalar distance = m_points.empty() ? TScalar( 0 ) : m_distances.back() + TTraits::Length( TTraits::Sub( point, m_points.back() ) );
    m_points.push_back( point );
    m_distances.push_back( distance );
}

template < typename TPoint >
void
CTPath< TPoint >::SetPoints( const TPoint* points, UInt32 count )
{
    Clear();
    for ( UInt32 i=0; i<count; ++i )
        AddPoint( points[ i ] );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE TPoint
CTPath< TPoint >::GetPoint( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return index < m_points.size() ? m_points[ index ] : TPoint();
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE typename CTPath< TPoint >::TScalar
CTPath< TPoint >::GetDistanceOfPoint( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return index < m_distances.size() ? m_distances[ index ] : TScalar( 0 );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE typename CTPath< TPoint >::TScalar
CTPath< TPoint >::GetLength( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return m_distances.empty() ? TScalar( 0 ) : m_distances.back();
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE UInt32
CTPath< TPoint >::FindSegment( const TScalar& distance ) const GUCEF_MATH_REALTIME_CONTEXT
{
    // largest i in [ 0, count - 2 ] with distance i <= d
    UInt32 low = 0;
    UInt32 high = (UInt32) m_points.size() - 1;
    while ( high - low > 1 )
    {
        const UInt32 mid = low + ( high - low ) / 2;
        if ( m_distances[ mid ] <= distance )
            low = mid;
        else
            high = mid;
    }
    return low;
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE TPoint
CTPath< TPoint >::GetPointAtDistance( const TScalar& distance ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( m_points.size() < 2 )
        return m_points.empty() ? TPoint() : m_points[ 0 ];
    if ( !( distance > TScalar( 0 ) ) )
        return m_points[ 0 ];
    if ( distance >= m_distances.back() )
        return m_points.back();
    const UInt32 i = FindSegment( distance );
    const TScalar length = m_distances[ i + 1 ] - m_distances[ i ];
    if ( !( length > TScalar( 0 ) ) )
        return m_points[ i ];
    return CurveLerp( m_points[ i ], m_points[ i + 1 ], ( distance - m_distances[ i ] ) / length );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE typename CTPath< TPoint >::TVector
CTPath< TPoint >::GetTangentAtDistance( const TScalar& distance ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( m_points.size() < 2 || !( m_distances.back() > TScalar( 0 ) ) )
        return TTraits::Zero();
    UInt32 i = distance >= m_distances.back() ? (UInt32) m_points.size() - 2 : ( distance > TScalar( 0 ) ? FindSegment( distance ) : 0 );
    // skip zero length segments: forward first, then backward from the end
    while ( i + 2 < m_points.size() && !( m_distances[ i + 1 ] > m_distances[ i ] ) )
        ++i;
    while ( i > 0 && !( m_distances[ i + 1 ] > m_distances[ i ] ) )
        --i;
    const TScalar length = m_distances[ i + 1 ] - m_distances[ i ];
    if ( !( length > TScalar( 0 ) ) )
        return TTraits::Zero();
    return TTraits::Scale( TTraits::Sub( m_points[ i + 1 ], m_points[ i ] ), TScalar( 1 ) / length );
}

template < typename TPoint >
GUCEF_MATH_PRECISE_INLINE bool
CTPath< TPoint >::GetClosestPoint( const TPoint& point, TPoint& closest, TScalar& distanceAlongPath ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( m_points.empty() )
        return false;
    if ( 1 == m_points.size() )
    {
        closest = m_points[ 0 ];
        distanceAlongPath = TScalar( 0 );
        return true;
    }

    TScalar bestSquared = std::numeric_limits< TScalar >::max();
    TScalar bestRoot = std::numeric_limits< TScalar >::max();     // sqrt( bestSquared ), kept in step
    TPoint bestPoint = m_points[ 0 ];
    TScalar bestDistance = TScalar( 0 );
    for ( size_t i=0; i+1<m_points.size(); ++i )
    {
        const TVector toStart = TTraits::Sub( point, m_points[ i ] );
        const TScalar segmentLength = m_distances[ i + 1 ] - m_distances[ i ];
        // coarse bound: no point of the segment is nearer than | point - start | - segment length
        const TScalar startSquared = TTraits::Dot( toStart, toStart );
        if ( bestRoot < std::numeric_limits< TScalar >::max() )
        {
            const TScalar reach = bestRoot + segmentLength;
            if ( startSquared > reach * reach )
                continue;
        }
        TScalar s = TScalar( 0 );
        if ( segmentLength > TScalar( 0 ) )
        {
            const TVector direction = TTraits::Sub( m_points[ i + 1 ], m_points[ i ] );
            s = TTraits::Dot( toStart, direction ) / TTraits::Dot( direction, direction );
            if ( s < TScalar( 0 ) ) s = TScalar( 0 );
            if ( s > TScalar( 1 ) ) s = TScalar( 1 );
        }
        const TPoint candidate = CurveLerp( m_points[ i ], m_points[ i + 1 ], s );
        const TVector offset = TTraits::Sub( point, candidate );
        const TScalar squared = TTraits::Dot( offset, offset );
        if ( squared < bestSquared )
        {
            bestSquared = squared;
            bestRoot = std::sqrt( squared );
            bestPoint = candidate;
            bestDistance = m_distances[ i ] + s * segmentLength;
        }
    }
    closest = bestPoint;
    distanceAlongPath = bestDistance;
    return true;
}

template < typename TPoint >
bool
CTPath< TPoint >::Resample( const TScalar& spacing, CTPath& result, UInt32 maxPoints ) const
{
    if ( m_points.size() < 2 || !( spacing > TScalar( 0 ) ) || !( m_distances.back() > TScalar( 0 ) ) )
    {
        if ( &result != this )
            result = *this;
        return true;
    }
    // the step count is computed in double precision so that it is exact for any Float32 path
    const Float64 stepsExact = std::ceil( (Float64) m_distances.back() / (Float64) spacing );
    if ( !( stepsExact + 1.0 <= (Float64) maxPoints ) )
        return false;
    const UInt32 steps = (UInt32) stepsExact;
    CTPath built;
    for ( UInt32 step=0; step<steps; ++step )
        built.AddPoint( GetPointAtDistance( TScalar( step ) * spacing ) );
    built.AddPoint( m_points.back() );
    result = built;
    return true;
}

template < typename TPoint >
bool
CTPath< TPoint >::Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
{
    return CurveSerializeList( node, "point", m_points, settings );
}

template < typename TPoint >
bool
CTPath< TPoint >::Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
{
    GUCEF::vector< TPoint > points;
    if ( !CurveDeserializeList( node, "point", points, settings ) )
        return false;
    Clear();
    for ( size_t i=0; i<points.size(); ++i )
        AddPoint( points[ i ] );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_MATH_CTPATH_EXTERN( kind ) GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTPath< kind > )
GUCEF_MATH_CURVE_PREDEFINED_POINTS( GUCEF_MATH_CTPATH_EXTERN )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTPATH_H ? */
