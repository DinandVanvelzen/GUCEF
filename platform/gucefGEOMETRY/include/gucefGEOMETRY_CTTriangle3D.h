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

#ifndef GUCEF_GEOMETRY_CTTRIANGLE3D_H
#define GUCEF_GEOMETRY_CTTRIANGLE3D_H

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

#ifndef GUCEF_GEOMETRY_SRAYCASTRESULT_H
#include "gucefGEOMETRY_SRayCastResult.h"
#define GUCEF_GEOMETRY_SRAYCASTRESULT_H
#endif /* GUCEF_GEOMETRY_SRAYCASTRESULT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {
template < typename T > class CTMatrix4D;
}
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T > class CTRay3D;
template < typename T > class CTSegment3D;
template < typename T > class CTAxisAlignedBox3D;

/*-------------------------------------------------------------------------*/

/**
 *  A triangle in 3D given by three points (section 7.2). The front side is where the vertices run counter-clockwise;
 *  the normal is ( b - a ) x ( c - a ) normalised.
 *
 *  - Kind: floats (Float32, Float64 are exported); needs a square root.
 *  - Layout: three points, 9 * sizeof( T ), no virtual functions.
 *  - Degenerate triangles (collinear or coincident vertices, see IsDegenerate) are valid values: their normal is
 *    Forward(), their area 0, their barycentric coordinates fail, and their closest point is the closest point on
 *    the three edges, so a point or a segment is handled as such.
 *  - Realtime: every member except the wireframe, text and DataNode members is realtime-safe.
 *  - Text: "Triangle3D(a (x, y, z), b (x, y, z), c (x, y, z))"; DataNode: child nodes a, b and c.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTTriangle3D
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTDirectionVector3D< T > TDirection;

    TPoint a;
    TPoint b;
    TPoint c;

    GUCEF_GEOMETRY_FORCE_INLINE CTTriangle3D( void ) : a(), b(), c() {}

    GUCEF_GEOMETRY_FORCE_INLINE CTTriangle3D( const TPoint& pointA, const TPoint& pointB, const TPoint& pointC ) : a( pointA ), b( pointB ), c( pointC ) {}

    /** The vertex for an index 0, 1 or 2 (higher indices give c) */
    GUCEF_GEOMETRY_FORCE_INLINE const TPoint& GetVertex( UInt32 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return 0 == index ? a : ( 1 == index ? b : c );
    }

    /** ( b - a ) x ( c - a ): its length is twice the area */
    GUCEF_GEOMETRY_FORCE_INLINE TVector GetNormalVector( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( b - a ).Cross( c - a ); }

    /** The unit normal; false (out unchanged) for a degenerate triangle */
    GUCEF_GEOMETRY_FORCE_INLINE bool TryGetNormal( TDirection& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return TDirection::TryCreate( GetNormalVector(), out );
    }

    /** The unit normal; Forward() for a degenerate triangle */
    GUCEF_GEOMETRY_FORCE_INLINE TDirection GetNormal( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        TDirection n;
        TryGetNormal( n );
        return n;
    }

    /** Half the length of the normal vector */
    GUCEF_GEOMETRY_FORCE_INLINE T GetArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return GetNormalVector().GetLength() * TTraits::Half(); }

    /** The mean of the three vertices */
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetCentroid( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        const TPoint points[ 3 ] = { a, b, c };
        return TPoint::Centroid( points, 3 );
    }

    /**
     *  True when twice the area is not larger than 'tolerance' times the squared length of the longest edge (a
     *  dimensionless measure of how thin the triangle is: collinear and coincident vertices always qualify).
     */
    bool IsDegenerate( const T& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** IsDegenerate with 16 times the machine epsilon of the kind as tolerance */
    GUCEF_GEOMETRY_FORCE_INLINE bool IsDegenerate( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return IsDegenerate( TTraits::Epsilon() * TTraits::FromInt32( 16 ) );
    }

    /**
     *  The barycentric coordinates ( u, v, w ) of the projection of 'point' onto the plane of the triangle, so that
     *  the projection is u * a + v * b + w * c and u + v + w = 1. False (out unchanged) for a degenerate triangle.
     */
    bool TryGetBarycentric( const TPoint& point, TVector& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The barycentric coordinates; the zero vector for a degenerate triangle */
    GUCEF_GEOMETRY_FORCE_INLINE TVector GetBarycentric( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        TVector result;
        TryGetBarycentric( point, result );
        return result;
    }

    /**
     *  The nearest point of the triangle (a surface, not a volume) by Voronoi regions (Ericson, Real-Time Collision
     *  Detection 5.1.5): vertex, edge or face region. Degenerate triangles give the nearest point on their edges.
     */
    TPoint ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    GUCEF_GEOMETRY_FORCE_INLINE T GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( point - ClosestPoint( point ) ).GetSquaredLength(); }
    GUCEF_GEOMETRY_FORCE_INLINE T GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( point - ClosestPoint( point ) ).GetLength(); }

    /** True when the point is on the triangle within 'tolerance' (a distance) */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& point, const T& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return GetDistance( point ) <= tolerance;
    }

    /** Contains with the default absolute tolerance of the kind */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return Contains( point, TTraits::DefaultAbsoluteTolerance() );
    }

    /** The axis aligned box around the three vertices */
    CTAxisAlignedBox3D< T > GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The vertex furthest along 'direction' (the first on ties) */
    TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  Moeller-Trumbore, two-sided: the normal of the result faces the ray, u and v are the barycentric weights of b
     *  and c at the hit. A ray parallel to the triangle plane and a degenerate triangle miss.
     */
    bool RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Appends the three edges; 'segmentsPerCircle' is not used */
    void GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 segmentsPerCircle ) const;

    /** The triangle moved by an affine matrix: the three vertices as points (the winding flips for a matrix with a negative determinant) */
    CTTriangle3D Transformed( const MATH::CTMatrix4D< T >& matrix ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTTriangle3D& x, const CTTriangle3D& y ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return x.a == y.a && x.b == y.b && x.c == y.c; }
    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTTriangle3D& x, const CTTriangle3D& y ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !( x == y ); }

    GUCEF_GEOMETRY_FORCE_INLINE bool Equals( const CTTriangle3D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return a.Equals( other.a ) && b.Equals( other.b ) && c.Equals( other.c );
    }

    /** "Triangle3D(a (x, y, z), b (x, y, z), c (x, y, z))" */
    CORE::CString ToString( void ) const
    {
        return "Triangle3D(a " + a.ToString() + ", b " + b.ToString() + ", c " + c.ToString() + ")";
    }

    /** Parses the ToString form; false leaves this triangle unchanged */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TPoint pa;
        TPoint pb;
        TPoint pc;
        if ( !reader.TryLiteral( "Triangle3D" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "a" ) || !reader.TryValue( pa ) ||
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
CTTriangle3D< T >::IsDegenerate( const T& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    T twiceArea = GetNormalVector().GetLength();
    T longest = TTraits::Max( TTraits::Max( ( b - a ).GetSquaredLength(), ( c - b ).GetSquaredLength() ), ( a - c ).GetSquaredLength() );
    return !( tolerance * longest < twiceArea );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTTriangle3D< T >::TryGetBarycentric( const TPoint& point, TVector& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( IsDegenerate() )
        return false;
    TVector v0 = b - a;
    TVector v1 = c - a;
    TVector v2 = point - a;
    T d00 = v0.Dot( v0 );
    T d01 = v0.Dot( v1 );
    T d11 = v1.Dot( v1 );
    T d20 = v2.Dot( v0 );
    T d21 = v2.Dot( v1 );
    T denominator = d00 * d11 - d01 * d01;
    if ( !( TTraits::Zero() < denominator ) )
        return false;
    T v = ( d11 * d20 - d01 * d21 ) / denominator;
    T w = ( d00 * d21 - d01 * d20 ) / denominator;
    out = TVector( TTraits::One() - v - w, v, w );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTTriangle3D< T >::TPoint
CTTriangle3D< T >::ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( IsDegenerate() )
    {
        TPoint best = CTSegment3D< T >( a, b ).ClosestPoint( point );
        T bestDistance = ( point - best ).GetSquaredLength();
        TPoint other = CTSegment3D< T >( b, c ).ClosestPoint( point );
        T otherDistance = ( point - other ).GetSquaredLength();
        if ( otherDistance < bestDistance )
        {
            best = other;
            bestDistance = otherDistance;
        }
        other = CTSegment3D< T >( c, a ).ClosestPoint( point );
        otherDistance = ( point - other ).GetSquaredLength();
        return otherDistance < bestDistance ? other : best;
    }

    const T zero = TTraits::Zero();
    const TVector ab = b - a;
    const TVector ac = c - a;

    /* vertex region of a */
    const TVector ap = point - a;
    const T d1 = ab.Dot( ap );
    const T d2 = ac.Dot( ap );
    if ( d1 <= zero && d2 <= zero )
        return a;

    /* vertex region of b */
    const TVector bp = point - b;
    const T d3 = ab.Dot( bp );
    const T d4 = ac.Dot( bp );
    if ( d3 >= zero && d4 <= d3 )
        return b;

    /* edge region of ab */
    const T vc = d1 * d4 - d3 * d2;
    if ( vc <= zero && d1 >= zero && d3 <= zero )
        return a + ab * ( d1 / ( d1 - d3 ) );

    /* vertex region of c */
    const TVector cp = point - c;
    const T d5 = ab.Dot( cp );
    const T d6 = ac.Dot( cp );
    if ( d6 >= zero && d5 <= d6 )
        return c;

    /* edge region of ac */
    const T vb = d5 * d2 - d1 * d6;
    if ( vb <= zero && d2 >= zero && d6 <= zero )
        return a + ac * ( d2 / ( d2 - d6 ) );

    /* edge region of bc */
    const T va = d3 * d6 - d5 * d4;
    if ( va <= zero && ( d4 - d3 ) >= zero && ( d5 - d6 ) >= zero )
        return b + ( c - b ) * ( ( d4 - d3 ) / ( ( d4 - d3 ) + ( d5 - d6 ) ) );

    /* face region */
    const T denominator = TTraits::One() / ( va + vb + vc );
    return a + ab * ( vb * denominator ) + ac * ( vc * denominator );
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTAxisAlignedBox3D< T >
CTTriangle3D< T >::GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return CTAxisAlignedBox3D< T >( a.Min( b ).Min( c ), a.Max( b ).Max( c ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
typename CTTriangle3D< T >::TPoint
CTTriangle3D< T >::GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
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
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTTriangle3D< T >::RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !ray.origin.IsFinite() || !ray.direction.AsVector().IsFinite() )
        return false;                       /* a ray that is not a ray (a NaN or infinity in it) hits nothing */
    const TVector direction = ray.direction.AsVector();
    const TVector e1 = b - a;
    const TVector e2 = c - a;
    const TVector pvec = direction.Cross( e2 );
    const T determinant = e1.Dot( pvec );
    if ( TTraits::Zero() == determinant )
        return false;
    const T inverse = TTraits::One() / determinant;
    if ( !TTraits::IsFinite( inverse ) )
        return false;
    const TVector tvec = ray.origin - a;
    const T u = tvec.Dot( pvec ) * inverse;
    if ( u < TTraits::Zero() || TTraits::One() < u )
        return false;
    const TVector qvec = tvec.Cross( e1 );
    const T v = direction.Dot( qvec ) * inverse;
    if ( v < TTraits::Zero() || TTraits::One() < u + v )
        return false;
    const T t = e2.Dot( qvec ) * inverse;
    if ( t < TTraits::Zero() || maxDistance < t )
        return false;

    SRayCastResult< T > hit;
    hit.distance = t;
    hit.point = ray.origin + direction * t;
    hit.u = u;
    hit.v = v;
    TDirection n;
    if ( !TryGetNormal( n ) )
        return false;
    hit.normal = TTraits::IsNegative( determinant ) ? -n.AsVector() : n.AsVector();
    result = hit;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTTriangle3D< T >::GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 /*segmentsPerCircle*/ ) const
{
    lines.push_back( CTSegment3D< T >( a, b ) );
    lines.push_back( CTSegment3D< T >( b, c ) );
    lines.push_back( CTSegment3D< T >( c, a ) );
}

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTTriangle3D< T >
CTTriangle3D< T >::Transformed( const MATH::CTMatrix4D< T >& matrix ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return CTTriangle3D( matrix.TransformPoint( a ), matrix.TransformPoint( b ), matrix.TransformPoint( c ) );
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTTriangle3D< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTTriangle3D< Float64 > )

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

#ifndef GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#include "gucefGEOMETRY_CTAxisAlignedBox3D.h"
#define GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#endif /* GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H ? */

#ifndef GUCEF_GEOMETRY_CTRAY3D_H
#include "gucefGEOMETRY_CTRay3D.h"
#define GUCEF_GEOMETRY_CTRAY3D_H
#endif /* GUCEF_GEOMETRY_CTRAY3D_H ? */

#ifndef GUCEF_GEOMETRY_CTSEGMENT3D_H
#include "gucefGEOMETRY_CTSegment3D.h"
#define GUCEF_GEOMETRY_CTSEGMENT3D_H
#endif /* GUCEF_GEOMETRY_CTSEGMENT3D_H ? */

#ifndef GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H
#include "gucefGEOMETRY_TransformFunctions.h"
#define GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H
#endif /* GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTTRIANGLE3D_H ? */
