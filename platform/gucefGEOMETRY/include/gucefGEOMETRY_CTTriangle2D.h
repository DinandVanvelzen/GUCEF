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

#ifndef GUCEF_GEOMETRY_CTTRIANGLE2D_H
#define GUCEF_GEOMETRY_CTTRIANGLE2D_H

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
 *  A solid triangle in 2D (the XY plane of its space) given by three points (section 7.2). Either winding is valid;
 *  GetSignedArea is positive for counter-clockwise vertices.
 *
 *  - Kind: floats (Float32, Float64 are exported); needs a square root.
 *  - Layout: three points, 6 * sizeof( T ), no virtual functions.
 *  - Degenerate triangles (collinear or coincident vertices, see IsDegenerate) are valid values: they contain the
 *    points of their longest edge (within the default tolerance), their barycentric coordinates fail.
 *  - Realtime: every member except the wireframe, text and DataNode members is realtime-safe.
 *  - Text: "Triangle2D(a (x, y), b (x, y), c (x, y))"; DataNode: child nodes a, b and c.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTTriangle2D
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint2D< T > TPoint;
    typedef MATH::CTVector2D< T > TVector;

    TPoint a;
    TPoint b;
    TPoint c;

    GUCEF_GEOMETRY_FORCE_INLINE CTTriangle2D( void ) : a(), b(), c() {}

    GUCEF_GEOMETRY_FORCE_INLINE CTTriangle2D( const TPoint& pointA, const TPoint& pointB, const TPoint& pointC ) : a( pointA ), b( pointB ), c( pointC ) {}

    /** The vertex for an index 0, 1 or 2 (higher indices give c) */
    GUCEF_GEOMETRY_FORCE_INLINE const TPoint& GetVertex( UInt32 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return 0 == index ? a : ( 1 == index ? b : c );
    }

    /** Half of ( b - a ) x ( c - a ): positive for counter-clockwise vertices, negative for clockwise ones */
    GUCEF_GEOMETRY_FORCE_INLINE T GetSignedArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return ( b - a ).Cross( c - a ) * TTraits::Half();
    }

    GUCEF_GEOMETRY_FORCE_INLINE T GetArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return TTraits::Abs( GetSignedArea() ); }

    /** True for a positive signed area */
    GUCEF_GEOMETRY_FORCE_INLINE bool IsCounterClockwise( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return TTraits::Zero() < GetSignedArea(); }

    /** The mean of the three vertices */
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetCentroid( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        const TPoint points[ 3 ] = { a, b, c };
        return TPoint::Centroid( points, 3 );
    }

    /** True when twice the area is not larger than 'tolerance' times the squared length of the longest edge */
    bool IsDegenerate( const T& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** IsDegenerate with 16 times the machine epsilon of the kind as tolerance */
    GUCEF_GEOMETRY_FORCE_INLINE bool IsDegenerate( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return IsDegenerate( TTraits::Epsilon() * TTraits::FromInt32( 16 ) );
    }

    /** The barycentric coordinates ( u, v, w ) of 'point' (point = u a + v b + w c, u + v + w = 1); false (out unchanged) for a degenerate triangle */
    bool TryGetBarycentric( const TPoint& point, MATH::CTVector3D< T >& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** True when the point is inside or on the boundary, for either winding; see the class comment for degenerate triangles */
    bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The nearest point of the solid triangle: the point itself when inside, otherwise the nearest point on the edges */
    TPoint ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    GUCEF_GEOMETRY_FORCE_INLINE T GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( point - ClosestPoint( point ) ).GetSquaredLength(); }
    GUCEF_GEOMETRY_FORCE_INLINE T GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( point - ClosestPoint( point ) ).GetLength(); }

    /** The axis aligned rectangle around the three vertices */
    GUCEF_GEOMETRY_FORCE_INLINE CTRectangle< T > GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return CTRectangle< T >( a.Min( b ).Min( c ), a.Max( b ).Max( c ) );
    }

    /** The vertex furthest along 'direction' (the first on ties) */
    TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Appends the three edges a-b, b-c, c-a; 'segmentsPerCircle' is not used */
    void GetWireframeLines( GUCEF::vector< CTSegment2D< T > >& lines, UInt32 segmentsPerCircle ) const;

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTTriangle2D& x, const CTTriangle2D& y ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return x.a == y.a && x.b == y.b && x.c == y.c; }
    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTTriangle2D& x, const CTTriangle2D& y ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !( x == y ); }

    GUCEF_GEOMETRY_FORCE_INLINE bool Equals( const CTTriangle2D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return a.Equals( other.a ) && b.Equals( other.b ) && c.Equals( other.c );
    }

    /** "Triangle2D(a (x, y), b (x, y), c (x, y))" */
    CORE::CString ToString( void ) const
    {
        return "Triangle2D(a " + a.ToString() + ", b " + b.ToString() + ", c " + c.ToString() + ")";
    }

    /** Parses the ToString form; false leaves this triangle unchanged */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TPoint pa;
        TPoint pb;
        TPoint pc;
        if ( !reader.TryLiteral( "Triangle2D" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "a" ) || !reader.TryValue( pa ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "b" ) || !reader.TryValue( pb ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "c" ) || !reader.TryValue( pc ) || !reader.TryChar( ')' ) || !reader.AtEnd() )
            return false;
        a = pa;
        b = pb;
        c = pc;
        return true;
    }

    /** Writes the child nodes a, b and c into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        return SerializeShapeChild( node, "a", a, settings ) && SerializeShapeChild( node, "b", b, settings ) && SerializeShapeChild( node, "c", c, settings );
    }

    /** Reads the child nodes a, b and c; a missing or invalid one fails and leaves this triangle unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        TPoint pa;
        TPoint pb;
        TPoint pc;
        if ( !DeserializeShapeChild( node, "a", pa, settings ) || !DeserializeShapeChild( node, "b", pb, settings ) || !DeserializeShapeChild( node, "c", pc, settings ) )
            return false;
        a = pa;
        b = pb;
        c = pc;
        return true;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTTriangle2D< T >::IsDegenerate( const T& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    T twiceArea = TTraits::Abs( ( b - a ).Cross( c - a ) );
    T longest = TTraits::Max( TTraits::Max( ( b - a ).GetSquaredLength(), ( c - b ).GetSquaredLength() ), ( a - c ).GetSquaredLength() );
    return !( tolerance * longest < twiceArea );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTTriangle2D< T >::TryGetBarycentric( const TPoint& point, MATH::CTVector3D< T >& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( IsDegenerate() )
        return false;
    const T twiceArea = ( b - a ).Cross( c - a );
    const T v = ( point - a ).Cross( c - a ) / twiceArea;
    const T w = ( b - a ).Cross( point - a ) / twiceArea;
    out = MATH::CTVector3D< T >( TTraits::One() - v - w, v, w );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTTriangle2D< T >::Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( IsDegenerate() )
    {
        T best = CTSegment2D< T >( a, b ).GetDistance( point );
        T d = CTSegment2D< T >( b, c ).GetDistance( point );
        if ( d < best )
            best = d;
        d = CTSegment2D< T >( c, a ).GetDistance( point );
        if ( d < best )
            best = d;
        return best <= TTraits::DefaultAbsoluteTolerance();
    }
    const T d1 = ( b - a ).Cross( point - a );
    const T d2 = ( c - b ).Cross( point - b );
    const T d3 = ( a - c ).Cross( point - c );
    const bool hasNegative = d1 < TTraits::Zero() || d2 < TTraits::Zero() || d3 < TTraits::Zero();
    const bool hasPositive = TTraits::Zero() < d1 || TTraits::Zero() < d2 || TTraits::Zero() < d3;
    return !( hasNegative && hasPositive );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTTriangle2D< T >::TPoint
CTTriangle2D< T >::ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !IsDegenerate() && Contains( point ) )
        return point;
    TPoint best = CTSegment2D< T >( a, b ).ClosestPoint( point );
    T bestDistance = ( point - best ).GetSquaredLength();
    TPoint other = CTSegment2D< T >( b, c ).ClosestPoint( point );
    T otherDistance = ( point - other ).GetSquaredLength();
    if ( otherDistance < bestDistance )
    {
        best = other;
        bestDistance = otherDistance;
    }
    other = CTSegment2D< T >( c, a ).ClosestPoint( point );
    otherDistance = ( point - other ).GetSquaredLength();
    return otherDistance < bestDistance ? other : best;
}

/*-------------------------------------------------------------------------*/

template < typename T >
typename CTTriangle2D< T >::TPoint
CTTriangle2D< T >::GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TPoint best = a;
    T bestDot = a.ToPositionVector().Dot( direction );
    T dotB = b.ToPositionVector().Dot( direction );
    if ( bestDot < dotB )
    {
        best = b;
        bestDot = dotB;
    }
    if ( bestDot < c.ToPositionVector().Dot( direction ) )
        best = c;
    return best;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTTriangle2D< T >::GetWireframeLines( GUCEF::vector< CTSegment2D< T > >& lines, UInt32 /*segmentsPerCircle*/ ) const
{
    lines.push_back( CTSegment2D< T >( a, b ) );
    lines.push_back( CTSegment2D< T >( b, c ) );
    lines.push_back( CTSegment2D< T >( c, a ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTTriangle2D< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTTriangle2D< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTTRIANGLE2D_H ? */
