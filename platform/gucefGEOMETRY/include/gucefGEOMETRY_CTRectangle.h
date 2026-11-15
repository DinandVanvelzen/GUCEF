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

#ifndef GUCEF_GEOMETRY_CTRECTANGLE_H
#define GUCEF_GEOMETRY_CTRECTANGLE_H

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

template < typename T > class CTSegment2D;

/*-------------------------------------------------------------------------*/

/**
 *  A closed axis aligned rectangle in 2D (the XY plane of its space), given by its minimum and maximum corner (section 7.2).
 *  Converts to and from CORE::TArea2D with ToCoreArea / ToRectangle (gucefGEOMETRY_CoreStructInterop.h).
 *
 *  - Kind: floats (Float32, Float64 are exported); the members that measure need a square root.
 *  - Layout: min and max, 4 * sizeof( T ), no virtual functions.
 *  - Empty rectangle: min greater than max on some axis. The default rectangle is empty and the identity of Merged.
 *    Its distance queries give the largest value of the kind, its ClosestPoint is the query point.
 *  - Realtime: every member except the wireframe, text and DataNode members is realtime-safe.
 *  - Text: "Rectangle(min (x, y), max (x, y))"; DataNode: child nodes min and max.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTRectangle
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint2D< T > TPoint;
    typedef MATH::CTVector2D< T > TVector;

    TPoint min;
    TPoint max;

    /** The empty rectangle */
    GUCEF_GEOMETRY_FORCE_INLINE CTRectangle( void )
        : min( TTraits::Max(), TTraits::Max() )
        , max( TTraits::Lowest(), TTraits::Lowest() )
    {
    }

    GUCEF_GEOMETRY_FORCE_INLINE CTRectangle( const TPoint& minCorner, const TPoint& maxCorner )
        : min( minCorner ), max( maxCorner )
    {
    }

    /** The rectangle converted from another kind component-wise (see CTVector2D) */
    template < typename U >
    explicit CTRectangle( const CTRectangle< U >& other )
        : min( other.min ), max( other.max )
    {
    }

    static GUCEF_GEOMETRY_FORCE_INLINE CTRectangle Empty( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return CTRectangle(); }

    /** The smallest rectangle around the points; the empty rectangle for count 0 */
    static CTRectangle FromPoints( const TPoint* points, UInt32 count ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        GUCEF_GEOMETRY_ASSERT( 0 == count || GUCEF_NULL != points );
        CTRectangle rectangle;
        if ( GUCEF_NULL == points )
            return rectangle;
        for ( UInt32 i=0; i<count; ++i )
        {
            rectangle.min = rectangle.min.Min( points[ i ] );
            rectangle.max = rectangle.max.Max( points[ i ] );
        }
        return rectangle;
    }

    static GUCEF_GEOMETRY_FORCE_INLINE CTRectangle FromCentreHalfExtents( const TPoint& centre, const TVector& halfExtents ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return CTRectangle( centre - halfExtents, centre + halfExtents );
    }

    /** True when min is greater than max on some axis (a NaN component counts as empty) */
    GUCEF_GEOMETRY_FORCE_INLINE bool IsEmpty( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return !( min.x <= max.x && min.y <= max.y );
    }

    /** The centre; the origin for the empty rectangle */
    TPoint GetCentre( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return TPoint();
        return TPoint::Midpoint( min, max );
    }

    /** The size, max - min; the zero vector for the empty rectangle */
    GUCEF_GEOMETRY_FORCE_INLINE TVector GetExtents( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return IsEmpty() ? TVector() : max - min; }

    GUCEF_GEOMETRY_FORCE_INLINE TVector GetHalfExtents( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return GetExtents() * TTraits::Half(); }

    /** The corner for the bits of 'index': bit 0 selects max x, bit 1 max y (higher bits are ignored) */
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetCorner( UInt32 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return TPoint( 0 != ( index & 1 ) ? max.x : min.x, 0 != ( index & 2 ) ? max.y : min.y );
    }

    /** Width times height; 0 for the empty rectangle */
    GUCEF_GEOMETRY_FORCE_INLINE T GetArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return TTraits::Zero();
        TVector e = max - min;
        return e.x * e.y;
    }

    /** The length of the boundary; 0 for the empty rectangle */
    GUCEF_GEOMETRY_FORCE_INLINE T GetPerimeter( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return TTraits::Zero();
        TVector e = max - min;
        return TTraits::Two() * ( e.x + e.y );
    }

    /** The rectangle grown by 'margin' on every side (shrunk for a negative margin; may become empty); the empty rectangle stays empty */
    GUCEF_GEOMETRY_FORCE_INLINE CTRectangle Expanded( const T& margin ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return *this;
        TVector m( margin, margin );
        return CTRectangle( min - m, max + m );
    }

    /** The smallest rectangle that holds this rectangle and the point; the rectangle of that single point for any empty rectangle */
    GUCEF_GEOMETRY_FORCE_INLINE CTRectangle ExpandedToInclude( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return CTRectangle( point, point );
        return CTRectangle( min.Min( point ), max.Max( point ) );
    }

    /** The smallest rectangle that holds both; any empty rectangle (not only the default one) is the identity */
    GUCEF_GEOMETRY_FORCE_INLINE CTRectangle Merged( const CTRectangle& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return other;
        if ( other.IsEmpty() )
            return *this;
        return CTRectangle( min.Min( other.min ), max.Max( other.max ) );
    }

    /** The common part; the empty rectangle when they do not touch */
    GUCEF_GEOMETRY_FORCE_INLINE CTRectangle Intersection( const CTRectangle& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        CTRectangle result( min.Max( other.min ), max.Min( other.max ) );
        return result.IsEmpty() ? CTRectangle() : result;
    }

    /** True when the closed rectangles share a point; false when either is empty */
    GUCEF_GEOMETRY_FORCE_INLINE bool Intersects( const CTRectangle& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return !IsEmpty() && !other.IsEmpty() && min.x <= other.max.x && other.min.x <= max.x && min.y <= other.max.y && other.min.y <= max.y;
    }

    /** True when the point is inside or on the boundary; false for the empty rectangle */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return min.x <= point.x && point.x <= max.x && min.y <= point.y && point.y <= max.y;
    }

    /** True when 'other' lies inside this rectangle; an empty 'other' is inside every rectangle, nothing is inside the empty one */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const CTRectangle& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( other.IsEmpty() )
            return true;
        return Contains( other.min ) && Contains( other.max );
    }

    /** The point of the rectangle nearest to 'point'; 'point' itself when inside or when the rectangle is empty */
    GUCEF_GEOMETRY_FORCE_INLINE TPoint ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return point;
        return point.Max( min ).Min( max );
    }

    /** Squared distance to the point, 0 inside; the largest value of the kind for the empty rectangle */
    T GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return TTraits::Max();
        return ( point - ClosestPoint( point ) ).GetSquaredLength();
    }

    /** Distance to the point, 0 inside; the largest value of the kind for the empty rectangle */
    T GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return TTraits::Max();
        return ( point - ClosestPoint( point ) ).GetLength();
    }

    GUCEF_GEOMETRY_FORCE_INLINE CTRectangle GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return *this; }

    /** The corner furthest along 'direction'; the origin for the empty rectangle */
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return TPoint();
        return TPoint( TTraits::IsNegative( direction.x ) ? min.x : max.x, TTraits::IsNegative( direction.y ) ? min.y : max.y );
    }

    /** Appends the four edges counter-clockwise from the minimum corner; nothing for the empty rectangle. 'segmentsPerCircle' is not used */
    void GetWireframeLines( GUCEF::vector< CTSegment2D< T > >& lines, UInt32 segmentsPerCircle ) const;

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTRectangle& a, const CTRectangle& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return a.min == b.min && a.max == b.max; }
    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTRectangle& a, const CTRectangle& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !( a == b ); }

    GUCEF_GEOMETRY_FORCE_INLINE bool Equals( const CTRectangle& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return min.Equals( other.min ) && max.Equals( other.max );
    }

    /** "Rectangle(min (x, y), max (x, y))" */
    CORE::CString ToString( void ) const
    {
        return "Rectangle(min " + min.ToString() + ", max " + max.ToString() + ")";
    }

    /** Parses the ToString form; false leaves this rectangle unchanged */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TPoint parsedMin;
        TPoint parsedMax;
        if ( !reader.TryLiteral( "Rectangle" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "min" ) || !reader.TryValue( parsedMin ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "max" ) || !reader.TryValue( parsedMax ) || !reader.TryChar( ')' ) || !reader.AtEnd() )
            return false;
        min = parsedMin;
        max = parsedMax;
        return true;
    }

    /** Writes the child nodes min and max into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        return SerializeShapeChild( node, "min", min, settings ) && SerializeShapeChild( node, "max", max, settings );
    }

    /** Reads the child nodes min and max; a missing or invalid one fails and leaves this rectangle unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        TPoint parsedMin;
        TPoint parsedMax;
        if ( !DeserializeShapeChild( node, "min", parsedMin, settings ) || !DeserializeShapeChild( node, "max", parsedMax, settings ) )
            return false;
        min = parsedMin;
        max = parsedMax;
        return true;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
void
CTRectangle< T >::GetWireframeLines( GUCEF::vector< CTSegment2D< T > >& lines, UInt32 /*segmentsPerCircle*/ ) const
{
    if ( IsEmpty() )
        return;
    const TPoint corners[ 4 ] = { GetCorner( 0 ), GetCorner( 1 ), GetCorner( 3 ), GetCorner( 2 ) };
    for ( UInt32 i=0; i<4; ++i )
        lines.push_back( CTSegment2D< T >( corners[ i ], corners[ ( i + 1 ) % 4 ] ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTRectangle< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTRectangle< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*
 *  The related classes used by the members; included after the class so that each header can be included first.
 */

#ifndef GUCEF_GEOMETRY_CTSEGMENT2D_H
#include "gucefGEOMETRY_CTSegment2D.h"
#define GUCEF_GEOMETRY_CTSEGMENT2D_H
#endif /* GUCEF_GEOMETRY_CTSEGMENT2D_H ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTRECTANGLE_H ? */
