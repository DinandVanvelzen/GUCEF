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

#ifndef GUCEF_GEOMETRY_CTPLANE_H
#define GUCEF_GEOMETRY_CTPLANE_H

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

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

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
template < typename T > class CTLine3D;

/*-------------------------------------------------------------------------*/

/**
 *  An infinite plane in 3D: a unit normal n and a signed offset d such that n . p = d for the points p of the plane
 *  (section 7.2). The front side is the side the normal points to; the signed distance of a point is positive there.
 *
 *  - Kind: floats (Float32, Float64 are exported); needs a square root.
 *  - Layout: normal and offset, 4 * sizeof( T ), no virtual functions. The default plane is y = 0 with the normal +Y.
 *  - Parallel lines and planes: a direction pair whose cross product is shorter than 16 times the machine epsilon of
 *    the kind (about 2e-6 for Float32) is treated as parallel and reports no intersection.
 *  - Realtime: every member except the text and DataNode members is realtime-safe.
 *  - Text: "Plane(normal (x, y, z), offset d)"; DataNode: child node normal and the attribute offset.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTPlane
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTDirectionVector3D< T > TDirection;

    TDirection normal;
    T offset;

    /** The plane y = 0, front side +Y */
    GUCEF_GEOMETRY_FORCE_INLINE CTPlane( void ) : normal( TDirection::Up() ), offset( TTraits::Zero() ) {}

    /** The plane n . p = offset */
    GUCEF_GEOMETRY_FORCE_INLINE CTPlane( const TDirection& unitNormal, const T& signedOffset ) : normal( unitNormal ), offset( signedOffset ) {}

    /** The plane through 'point' with the given normal */
    static CTPlane FromPointNormal( const TPoint& point, const TDirection& unitNormal ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return CTPlane( unitNormal, unitNormal.Dot( point.ToPositionVector() ) );
    }

    /**
     *  The plane through three points; the normal is ( p1 - p0 ) x ( p2 - p0 ), so the points run counter-clockwise
     *  when seen from the front. False (out unchanged) when the points are collinear or coincide.
     */
    static bool TryFromPoints( const TPoint& p0, const TPoint& p1, const TPoint& p2, CTPlane& out ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        TDirection n;
        if ( !TDirection::TryCreate( ( p1 - p0 ).Cross( p2 - p0 ), n ) )
            return false;
        out = FromPointNormal( p0, n );
        return true;
    }

    /**
     *  For points known to be independent. Collinear points give the default plane (y = 0) and assert in debug builds:
     *  use TryFromPoints when the points may be degenerate.
     */
    static GUCEF_GEOMETRY_FORCE_INLINE CTPlane FromPoints( const TPoint& p0, const TPoint& p1, const TPoint& p2 ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        CTPlane plane;
        const bool created = TryFromPoints( p0, p1, p2, plane );
        GUCEF_GEOMETRY_ASSERT( created );
        (void) created;
        return plane;
    }

    /** The same plane with the front on the other side */
    GUCEF_GEOMETRY_FORCE_INLINE CTPlane Flipped( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return CTPlane( -normal, MATH::ComponentNegate( offset ) );
    }

    /** n . p - d: positive in front, negative behind, 0 on the plane */
    GUCEF_GEOMETRY_FORCE_INLINE T GetSignedDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return normal.Dot( point.ToPositionVector() ) - offset;
    }

    /** The orthogonal projection of the point onto the plane */
    TPoint Project( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The mirror image of the point in the plane (an image source, section 7.11) */
    TPoint Mirror( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The reflection of a vector at the plane (the plane through the origin with this normal) */
    TVector Mirror( const TVector& vector ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The reflection of a direction at the plane, again a unit direction */
    TDirection Mirror( const TDirection& d ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    GUCEF_GEOMETRY_FORCE_INLINE TPoint ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return Project( point ); }
    GUCEF_GEOMETRY_FORCE_INLINE T GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return TTraits::Abs( GetSignedDistance( point ) ); }
    GUCEF_GEOMETRY_FORCE_INLINE T GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        T distance = GetSignedDistance( point );
        return distance * distance;
    }

    /** True when the point is on the plane within 'tolerance' (a distance) */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& point, const T& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return GetDistance( point ) <= tolerance;
    }

    /** Contains with the default absolute tolerance of the kind */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return Contains( point, TTraits::DefaultAbsoluteTolerance() );
    }

    /** The smallest |sin| of the angle between two directions that is not treated as parallel */
    static GUCEF_GEOMETRY_FORCE_INLINE T GetParallelTolerance( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return TTraits::Epsilon() * TTraits::FromInt32( 16 );
    }

    /** The point where the line meets the plane; false (out unchanged) when the line is parallel to the plane */
    bool IntersectLine( const CTLine3D< T >& line, TPoint& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The line where two planes meet; false (out unchanged) when the planes are parallel (equal planes included) */
    bool IntersectPlane( const CTPlane& other, CTLine3D< T >& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The point common to three planes; false (out unchanged) when their normals are not independent */
    static bool IntersectPlanes( const CTPlane& a, const CTPlane& b, const CTPlane& c, TPoint& out ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  Hit of the ray with the plane, two-sided: the normal of the result faces the ray. A ray parallel to the plane
     *  misses (also when it lies in the plane); a ray starting on the plane hits at distance 0.
     */
    bool RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  The plane moved by an affine matrix: the normal by the inverse transpose of the linear part (renormalised), the
     *  offset from the image of a point of the plane; the front side stays the front side, also for reflections.
     *  False (out unchanged) when the linear part is singular.
     */
    bool TryTransformed( const MATH::CTMatrix4D< T >& matrix, CTPlane& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** TryTransformed; this plane unchanged (and an assertion in debug builds) for a singular matrix */
    CTPlane Transformed( const MATH::CTMatrix4D< T >& matrix ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTPlane& a, const CTPlane& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return a.normal == b.normal && a.offset == b.offset; }
    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTPlane& a, const CTPlane& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !( a == b ); }

    /** Normal and offset equal within the default tolerances of the kind */
    GUCEF_GEOMETRY_FORCE_INLINE bool Equals( const CTPlane& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return normal.Equals( other.normal ) && TTraits::EqualsWithin( offset, other.offset, TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() );
    }

    /** "Plane(normal (x, y, z), offset d)" */
    CORE::CString ToString( void ) const
    {
        return "Plane(normal " + normal.ToString() + ", offset " + TTraits::ToString( offset ) + ")";
    }

    /** Parses the ToString form; false leaves this plane unchanged (the normal must be of unit length) */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TDirection parsedNormal;
        T parsedOffset;
        if ( !reader.TryLiteral( "Plane" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "normal" ) || !reader.TryValue( parsedNormal ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "offset" ) || !reader.TryScalar( parsedOffset ) || !reader.TryChar( ')' ) || !reader.AtEnd() )
            return false;
        normal = parsedNormal;
        offset = parsedOffset;
        return true;
    }

    /** Writes the child node normal and the attribute offset into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        return SerializeShapeChild( node, "normal", normal, settings ) && SerializeShapeScalar( node, "offset", offset );
    }

    /** Reads the child node normal and the attribute offset; a missing or invalid one fails and leaves this plane unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        TDirection parsedNormal;
        T parsedOffset;
        if ( !DeserializeShapeChild( node, "normal", parsedNormal, settings ) || !DeserializeShapeScalar( node, "offset", parsedOffset ) )
            return false;
        normal = parsedNormal;
        offset = parsedOffset;
        return true;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTPlane< T >::TPoint
CTPlane< T >::Project( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return point - normal * GetSignedDistance( point );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTPlane< T >::TPoint
CTPlane< T >::Mirror( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return point - normal * ( TTraits::Two() * GetSignedDistance( point ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTPlane< T >::TVector
CTPlane< T >::Mirror( const TVector& vector ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return vector.Reflect( normal );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTPlane< T >::TDirection
CTPlane< T >::Mirror( const TDirection& d ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TDirection result;
    if ( !TDirection::TryCreate( d.AsVector().Reflect( normal ), result ) )
        return d;
    return result;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTPlane< T >::IntersectLine( const CTLine3D< T >& line, TPoint& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    T denominator = normal.Dot( line.direction.AsVector() );
    if ( !( GetParallelTolerance() < TTraits::Abs( denominator ) ) )
        return false;
    T t = ( TTraits::Zero() - GetSignedDistance( line.point ) ) / denominator;
    out = line.GetPoint( t );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTPlane< T >::IntersectPlane( const CTPlane& other, CTLine3D< T >& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TVector n1 = normal.AsVector();
    TVector n2 = other.normal.AsVector();
    TVector crossProduct = n1.Cross( n2 );
    T crossLength = crossProduct.GetLength();
    if ( !( GetParallelTolerance() < crossLength ) )
        return false;
    TDirection lineDirection;
    if ( !TDirection::TryCreate( crossProduct, lineDirection ) )
        return false;

    /* the point of the line nearest to the origin: a * n1 + b * n2 with n1 . p = d1 and n2 . p = d2 */
    T c = n1.Dot( n2 );
    T determinant = crossProduct.GetSquaredLength();
    T a = ( offset - c * other.offset ) / determinant;
    T b = ( other.offset - c * offset ) / determinant;
    out = CTLine3D< T >( TPoint( n1 * a + n2 * b ), lineDirection );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTPlane< T >::IntersectPlanes( const CTPlane& a, const CTPlane& b, const CTPlane& c, TPoint& out ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TVector n1 = a.normal.AsVector();
    TVector n2 = b.normal.AsVector();
    TVector n3 = c.normal.AsVector();
    TVector n2xn3 = n2.Cross( n3 );
    T denominator = n1.Dot( n2xn3 );
    if ( !( GetParallelTolerance() < TTraits::Abs( denominator ) ) )
        return false;
    TVector sum = n2xn3 * a.offset + n3.Cross( n1 ) * b.offset + n1.Cross( n2 ) * c.offset;
    out = TPoint( sum / denominator );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTPlane< T >::RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !ray.origin.IsFinite() || !ray.direction.AsVector().IsFinite() )
        return false;                       /* a ray that is not a ray (a NaN or infinity in it) hits nothing */
    T denominator = normal.Dot( ray.direction.AsVector() );
    if ( !( GetParallelTolerance() < TTraits::Abs( denominator ) ) )
        return false;
    T t = ( TTraits::Zero() - GetSignedDistance( ray.origin ) ) / denominator;
    if ( t < TTraits::Zero() || maxDistance < t )
        return false;
    SRayCastResult< T > hit;
    hit.distance = t;
    hit.point = ray.GetPoint( t );
    hit.normal = TTraits::IsNegative( denominator ) ? normal.AsVector() : -normal.AsVector();
    result = hit;
    return true;
}

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTPlane< T >::TryTransformed( const MATH::CTMatrix4D< T >& matrix, CTPlane& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    MATH::CTMatrix3D< T > inverse = MATH::CTMatrix3D< T >::Zero();
    if ( !matrix.GetLinear().TryInverse( inverse, TTraits::Zero() ) )
        return false;
    TDirection n;
    if ( !TDirection::TryCreate( inverse.Transposed().Transform( normal.AsVector() ), n ) )
        return false;
    const TPoint moved = matrix.TransformPoint( TPoint( normal.AsVector() * offset ) );
    out = CTPlane( n, n.Dot( moved.ToPositionVector() ) );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTPlane< T >
CTPlane< T >::Transformed( const MATH::CTMatrix4D< T >& matrix ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    CTPlane result;
    const bool transformed = TryTransformed( matrix, result );
    GUCEF_GEOMETRY_ASSERT( transformed );
    return transformed ? result : *this;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPlane< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPlane< Float64 > )

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

#ifndef GUCEF_GEOMETRY_CTRAY3D_H
#include "gucefGEOMETRY_CTRay3D.h"
#define GUCEF_GEOMETRY_CTRAY3D_H
#endif /* GUCEF_GEOMETRY_CTRAY3D_H ? */

#ifndef GUCEF_GEOMETRY_CTLINE3D_H
#include "gucefGEOMETRY_CTLine3D.h"
#define GUCEF_GEOMETRY_CTLINE3D_H
#endif /* GUCEF_GEOMETRY_CTLINE3D_H ? */

#ifndef GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H
#include "gucefGEOMETRY_TransformFunctions.h"
#define GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H
#endif /* GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTPLANE_H ? */
