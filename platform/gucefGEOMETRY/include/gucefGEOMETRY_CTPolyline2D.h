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

#ifndef GUCEF_GEOMETRY_CTPOLYLINE2D_H
#define GUCEF_GEOMETRY_CTPOLYLINE2D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  An open sequence of points in 2D (section 7.2): the segments join consecutive points, the last point does not
 *  connect to the first.
 *
 *  - Kind: floats (Float32, Float64 are exported); needs a square root.
 *  - Layout: the points are a public GUCEF::vector (no invariant); the polyline owns its storage, so copying copies
 *    the points.
 *  - Fewer than two points have no segments: length 0; one point is a valid polyline whose closest point is that point,
 *    no points give the query point back.
 *  - Realtime: the const queries are realtime-safe; AddPoint, the wireframe, text and DataNode members are not.
 *  - Text: "Polyline2D((x, y), (x, y), ...)"; DataNode: child node points with one child node point per vertex.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTPolyline2D
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint2D< T > TPoint;
    typedef MATH::CTVector2D< T > TVector;
    typedef GUCEF::vector< TPoint > TPointVector;

    TPointVector points;

    CTPolyline2D( void ) : points() {}

    /** The polyline through 'count' points */
    CTPolyline2D( const TPoint* linePoints, UInt32 count ) : points()
    {
        GUCEF_GEOMETRY_ASSERT( 0 == count || GUCEF_NULL != linePoints );
        if ( GUCEF_NULL != linePoints )
            points.assign( linePoints, linePoints + count );
    }

    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetPointCount( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return (UInt32) points.size(); }

    /** The number of segments: one less than the number of points, 0 for fewer than two */
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetSegmentCount( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return points.size() < 2 ? 0 : (UInt32) points.size() - 1; }

    /** The vertex at 'index' (below GetPointCount) */
    GUCEF_GEOMETRY_FORCE_INLINE const TPoint& GetPoint( UInt32 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        GUCEF_GEOMETRY_ASSERT( index < points.size() );
        return points[ index ];
    }

    /** The segment from vertex 'index' to the next (index below GetSegmentCount) */
    GUCEF_GEOMETRY_FORCE_INLINE CTSegment2D< T > GetSegment( UInt32 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        GUCEF_GEOMETRY_ASSERT( index + 1 < points.size() );
        return CTSegment2D< T >( points[ index ], points[ index + 1 ] );
    }

    void AddPoint( const TPoint& point ) { points.push_back( point ); }

    void Clear( void ) { points.clear(); }

    /** The sum of the segment lengths */
    T GetLength( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The point at 'distance' along the polyline from its first point, clamped to the first and last point; the origin without points */
    TPoint GetPointAtDistance( const T& distance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The nearest point of the polyline (the first on ties); the vertex of a one point polyline, the query point without points */
    TPoint ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    GUCEF_GEOMETRY_FORCE_INLINE T GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( point - ClosestPoint( point ) ).GetSquaredLength(); }
    GUCEF_GEOMETRY_FORCE_INLINE T GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( point - ClosestPoint( point ) ).GetLength(); }

    /** True when the point is on the polyline within 'tolerance' (a distance); false without points */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& point, const T& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return !points.empty() && GetDistance( point ) <= tolerance;
    }

    /** Contains with the default absolute tolerance of the kind */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return Contains( point, TTraits::DefaultAbsoluteTolerance() );
    }

    /** The axis aligned bounds of the points; empty without points */
    CTRectangle< T > GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The vertex furthest along 'direction' (the first on ties); the origin without points */
    TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Appends the segments; 'segmentsPerCircle' is not used */
    void GetWireframeLines( GUCEF::vector< CTSegment2D< T > >& lines, UInt32 segmentsPerCircle ) const;

    friend bool operator==( const CTPolyline2D& a, const CTPolyline2D& b ) { return a.points == b.points; }
    friend bool operator!=( const CTPolyline2D& a, const CTPolyline2D& b ) { return !( a == b ); }

    /** Same number of points, each equal within the default tolerances of the kind */
    bool Equals( const CTPolyline2D& other ) const
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

    /** "Polyline2D((x, y), (x, y), ...)" */
    CORE::CString ToString( void ) const
    {
        return ShapePointsToString( "Polyline2D", points );
    }

    /** Parses the ToString form; false leaves this polyline unchanged */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TPointVector parsed;
        if ( !reader.TryLiteral( "Polyline2D" ) || !reader.TryPointList( parsed ) || !reader.AtEnd() )
            return false;
        points.swap( parsed );
        return true;
    }

    /** Writes the child node points (one child node point per vertex) into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        return SerializeShapePoints( node, "points", points, settings );
    }

    /** Reads the child node points; a missing or invalid one fails and leaves this polyline unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        return DeserializeShapePoints( node, "points", points, settings );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTPolyline2D< T >::GetLength( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    T sum = TTraits::Zero();
    for ( UInt32 i=1; i<(UInt32) points.size(); ++i )
        sum = sum + ( points[ i ] - points[ i - 1 ] ).GetLength();
    return sum;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTPolyline2D< T >::TPoint
CTPolyline2D< T >::GetPointAtDistance( const T& distance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( points.empty() )
        return TPoint();
    T remaining = distance;
    if ( !( TTraits::Zero() < remaining ) )
        return points[ 0 ];
    for ( UInt32 i=1; i<(UInt32) points.size(); ++i )
    {
        const T length = ( points[ i ] - points[ i - 1 ] ).GetLength();
        if ( remaining <= length )
        {
            if ( !( remaining < length ) )
                return points[ i ];
            return points[ i - 1 ] + ( points[ i ] - points[ i - 1 ] ) * ( remaining / length );
        }
        remaining = remaining - length;
    }
    return points[ points.size() - 1 ];
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTPolyline2D< T >::TPoint
CTPolyline2D< T >::ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( points.empty() )
        return point;
    TPoint best = points[ 0 ];
    T bestDistance = ( point - best ).GetSquaredLength();
    for ( UInt32 i=1; i<(UInt32) points.size(); ++i )
    {
        TPoint candidate = CTSegment2D< T >( points[ i - 1 ], points[ i ] ).ClosestPoint( point );
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
CTRectangle< T >
CTPolyline2D< T >::GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( points.empty() )
        return CTRectangle< T >();
    return CTRectangle< T >::FromPoints( &points[ 0 ], (UInt32) points.size() );
}

/*-------------------------------------------------------------------------*/

template < typename T >
typename CTPolyline2D< T >::TPoint
CTPolyline2D< T >::GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( points.empty() )
        return TPoint();
    UInt32 best = 0;
    T bestDot = points[ 0 ].ToPositionVector().Dot( direction );
    for ( UInt32 i=1; i<(UInt32) points.size(); ++i )
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
CTPolyline2D< T >::GetWireframeLines( GUCEF::vector< CTSegment2D< T > >& lines, UInt32 /*segmentsPerCircle*/ ) const
{
    for ( UInt32 i=1; i<(UInt32) points.size(); ++i )
        lines.push_back( CTSegment2D< T >( points[ i - 1 ], points[ i ] ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolyline2D< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolyline2D< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTPOLYLINE2D_H ? */
