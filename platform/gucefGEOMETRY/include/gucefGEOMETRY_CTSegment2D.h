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

#ifndef GUCEF_GEOMETRY_CTSEGMENT2D_H
#define GUCEF_GEOMETRY_CTSEGMENT2D_H

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
 *  A line segment in 2D (the XY plane of its space) given by its two end points (section 7.2).
 *
 *  - Kind: floats (Float32, Float64 are exported); the measuring members need a square root.
 *  - Layout: start and end points, 4 * sizeof( T ), no virtual functions.
 *  - A segment of zero length is valid: its direction is Right (+X), its closest point is the point itself.
 *  - Realtime: every member except the wireframe, text and DataNode members is realtime-safe.
 *  - Text: "Segment2D(start (x, y), end (x, y))"; DataNode: child nodes start and end.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTSegment2D
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint2D< T > TPoint;
    typedef MATH::CTVector2D< T > TVector;
    typedef MATH::CTDirectionVector2D< T > TDirection;

    TPoint start;
    TPoint end;

    GUCEF_GEOMETRY_FORCE_INLINE CTSegment2D( void ) : start(), end() {}

    GUCEF_GEOMETRY_FORCE_INLINE CTSegment2D( const TPoint& startPoint, const TPoint& endPoint )
        : start( startPoint ), end( endPoint )
    {
    }

    /** end - start */
    GUCEF_GEOMETRY_FORCE_INLINE TVector GetVector( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return end - start; }

    GUCEF_GEOMETRY_FORCE_INLINE T GetLength( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( end - start ).GetLength(); }
    GUCEF_GEOMETRY_FORCE_INLINE T GetSquaredLength( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( end - start ).GetSquaredLength(); }

    /** The unit direction from start to end; false (out unchanged) for a zero length or non finite segment */
    GUCEF_GEOMETRY_FORCE_INLINE bool TryGetDirection( TDirection& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return TDirection::TryCreate( end - start, out );
    }

    /** The unit direction from start to end; Right() for a zero length segment */
    GUCEF_GEOMETRY_FORCE_INLINE TDirection GetDirection( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        TDirection d;
        TryGetDirection( d );
        return d;
    }

    /** start + ( end - start ) * t, exactly start for t <= 0 and exactly end for t >= 1 (clamped) */
    TPoint GetPoint( const T& t ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetMidpoint( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return TPoint::Midpoint( start, end ); }

    GUCEF_GEOMETRY_FORCE_INLINE CTSegment2D Reversed( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return CTSegment2D( end, start ); }

    /** The parameter t in [0, 1] of the point of the segment nearest to 'point'; 0 for a zero length segment */
    T GetClosestParameter( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    GUCEF_GEOMETRY_FORCE_INLINE TPoint ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return GetPoint( GetClosestParameter( point ) ); }
    GUCEF_GEOMETRY_FORCE_INLINE T GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( point - ClosestPoint( point ) ).GetSquaredLength(); }
    GUCEF_GEOMETRY_FORCE_INLINE T GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( point - ClosestPoint( point ) ).GetLength(); }

    /** True when the point is on the segment within 'tolerance' (a distance) */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& point, const T& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return GetDistance( point ) <= tolerance;
    }

    /** Contains with the default absolute tolerance of the kind */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return Contains( point, TTraits::DefaultAbsoluteTolerance() );
    }

    /**
     *  True when the closed segments share a point, touching end points and collinear overlap included (orientation
     *  signs of the end points; zero length segments act as points).
     */
    bool Intersects( const CTSegment2D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The axis aligned rectangle around both end points */
    GUCEF_GEOMETRY_FORCE_INLINE CTRectangle< T > GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return CTRectangle< T >( start.Min( end ), start.Max( end ) );
    }

    /** The end point furthest along 'direction' (start when both are equally far) */
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return TTraits::Zero() < ( end - start ).Dot( direction ) ? end : start;
    }

    /** Appends this segment; 'segmentsPerCircle' is not used */
    void GetWireframeLines( GUCEF::vector< CTSegment2D< T > >& lines, UInt32 /*segmentsPerCircle*/ ) const
    {
        lines.push_back( *this );
    }

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTSegment2D& a, const CTSegment2D& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return a.start == b.start && a.end == b.end; }
    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTSegment2D& a, const CTSegment2D& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !( a == b ); }

    GUCEF_GEOMETRY_FORCE_INLINE bool Equals( const CTSegment2D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return start.Equals( other.start ) && end.Equals( other.end );
    }

    /** "Segment2D(start (x, y), end (x, y))" */
    CORE::CString ToString( void ) const
    {
        return "Segment2D(start " + start.ToString() + ", end " + end.ToString() + ")";
    }

    /** Parses the ToString form; false leaves this segment unchanged */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TPoint parsedStart;
        TPoint parsedEnd;
        if ( !reader.TryLiteral( "Segment2D" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "start" ) || !reader.TryValue( parsedStart ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "end" ) || !reader.TryValue( parsedEnd ) || !reader.TryChar( ')' ) || !reader.AtEnd() )
            return false;
        start = parsedStart;
        end = parsedEnd;
        return true;
    }

    /** Writes the child nodes start and end into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        return SerializeShapeChild( node, "start", start, settings ) && SerializeShapeChild( node, "end", end, settings );
    }

    /** Reads the child nodes start and end; a missing or invalid one fails and leaves this segment unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        TPoint parsedStart;
        TPoint parsedEnd;
        if ( !DeserializeShapeChild( node, "start", parsedStart, settings ) || !DeserializeShapeChild( node, "end", parsedEnd, settings ) )
            return false;
        start = parsedStart;
        end = parsedEnd;
        return true;
    }

    private:

    /** -1, 0 or 1 */
    static GUCEF_GEOMETRY_FORCE_INLINE Int32 SignOf( const T& value ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return TTraits::Zero() < value ? 1 : ( value < TTraits::Zero() ? -1 : 0 );
    }

    /** For a point r collinear with p and q: true when r lies between them */
    static GUCEF_GEOMETRY_FORCE_INLINE bool IsWithinBox( const TPoint& p, const TPoint& q, const TPoint& r ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return p.Min( q ).x <= r.x && r.x <= p.Max( q ).x && p.Min( q ).y <= r.y && r.y <= p.Max( q ).y;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTSegment2D< T >::TPoint
CTSegment2D< T >::GetPoint( const T& t ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    /* exact end points: start + ( end - start ) * 1 does not give 'end' for end points of very different magnitude */
    if ( !( TTraits::Zero() < t ) )
        return start;
    if ( !( t < TTraits::One() ) )
        return end;
    return start + ( end - start ) * t;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTSegment2D< T >::GetClosestParameter( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TVector ab = end - start;
    T squaredLength = ab.GetSquaredLength();
    if ( !( TTraits::Zero() < squaredLength ) )
        return TTraits::Zero();
    T t = ( point - start ).Dot( ab ) / squaredLength;
    if ( !( TTraits::Zero() < t ) )
        return TTraits::Zero();
    return t < TTraits::One() ? t : TTraits::One();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTSegment2D< T >::Intersects( const CTSegment2D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const TVector ab = end - start;
    const TVector cd = other.end - other.start;
    const Int32 d1 = SignOf( ab.Cross( other.start - start ) );
    const Int32 d2 = SignOf( ab.Cross( other.end - start ) );
    const Int32 d3 = SignOf( cd.Cross( start - other.start ) );
    const Int32 d4 = SignOf( cd.Cross( end - other.start ) );

    if ( d1 * d2 < 0 && d3 * d4 < 0 )
        return true;
    if ( 0 == d1 && IsWithinBox( start, end, other.start ) )
        return true;
    if ( 0 == d2 && IsWithinBox( start, end, other.end ) )
        return true;
    if ( 0 == d3 && IsWithinBox( other.start, other.end, start ) )
        return true;
    if ( 0 == d4 && IsWithinBox( other.start, other.end, end ) )
        return true;
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTSegment2D< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTSegment2D< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTSEGMENT2D_H ? */
