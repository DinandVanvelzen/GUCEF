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

#ifndef GUCEF_GEOMETRY_CTSPHERE_H
#define GUCEF_GEOMETRY_CTSPHERE_H

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
 *  A solid sphere: centre and radius (section 7.2).
 *
 *  - Kind: floats (Float32, Float64 are exported); needs a square root and, for the volume, transcendental constants.
 *  - Layout: centre point and radius, 4 * sizeof( T ), no virtual functions. The default sphere is the unit sphere at the origin.
 *  - A negative radius is not valid: it contains nothing. Distances are to the solid: 0 inside, GetSignedDistance gives
 *    the signed distance to the surface (negative inside).
 *  - Realtime: every member except the wireframe, text and DataNode members is realtime-safe.
 *  - Text: "Sphere(centre (x, y, z), radius r)"; DataNode: child node centre and the attribute radius.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTSphere
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTDirectionVector3D< T > TDirection;

    TPoint centre;
    T radius;

    GUCEF_GEOMETRY_FORCE_INLINE CTSphere( void ) : centre(), radius( TTraits::One() ) {}

    GUCEF_GEOMETRY_FORCE_INLINE CTSphere( const TPoint& centrePoint, const T& sphereRadius ) : centre( centrePoint ), radius( sphereRadius ) {}

    /** The distance from the surface along the outward normal: negative inside, 0 on the surface */
    GUCEF_GEOMETRY_FORCE_INLINE T GetSignedDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return ( point - centre ).GetLength() - radius;
    }

    /** True when the point is inside or on the surface */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return ( point - centre ).GetLength() <= radius;
    }

    /** The nearest point of the solid sphere: the point itself when inside, otherwise on the surface */
    TPoint ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The nearest point of the surface (also for points inside; the +X pole for the centre itself) */
    TPoint ClosestPointOnSurface( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Distance from the solid to the point, 0 inside */
    GUCEF_GEOMETRY_FORCE_INLINE T GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        T signedDistance = GetSignedDistance( point );
        return signedDistance < TTraits::Zero() ? TTraits::Zero() : signedDistance;
    }

    GUCEF_GEOMETRY_FORCE_INLINE T GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        T distance = GetDistance( point );
        return distance * distance;
    }

    /** The point of the sphere furthest along 'direction'; the +X pole for a zero direction */
    TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** 4/3 pi r^3 */
    T GetVolume( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** 4 pi r^2 */
    T GetSurfaceArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The axis aligned box around the sphere */
    CTAxisAlignedBox3D< T > GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** True when the solid spheres share a point */
    GUCEF_GEOMETRY_FORCE_INLINE bool Intersects( const CTSphere& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return ( other.centre - centre ).GetLength() <= radius + other.radius;
    }

    /**
     *  Hit of the ray with the solid sphere (geometric form with the closest approach, so no cancellation for rays
     *  that graze the sphere). A ray starting inside hits at distance 0 with RAYHIT_STARTED_INSIDE and the normal
     *  opposite to the ray; a sphere behind the ray or beyond maxDistance is a miss, and so is a sphere with a radius of 0 or less.
     */
    bool RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  Appends three great circles (in the XY, YZ and ZX planes), each closed with 'segmentsPerCircle' segments
     *  (at least 3).
     */
    void GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 segmentsPerCircle ) const;

    /**
     *  A sphere around the transformed sphere: the centre as a point, the radius times the largest axis scale of the
     *  matrix. Exact for a similarity (rotation, translation, uniform scale), a bounding sphere for non-uniform scales.
     */
    CTSphere Transformed( const MATH::CTMatrix4D< T >& matrix ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The exact image: false (out unchanged) unless the matrix is a similarity within the default relative tolerance of the kind */
    bool TryTransformed( const MATH::CTMatrix4D< T >& matrix, CTSphere& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTSphere& a, const CTSphere& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return a.centre == b.centre && a.radius == b.radius; }
    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTSphere& a, const CTSphere& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !( a == b ); }

    /** Centre and radius equal within the default tolerances of the kind */
    GUCEF_GEOMETRY_FORCE_INLINE bool Equals( const CTSphere& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return centre.Equals( other.centre ) && TTraits::EqualsWithin( radius, other.radius, TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() );
    }

    /** "Sphere(centre (x, y, z), radius r)" */
    CORE::CString ToString( void ) const
    {
        return "Sphere(centre " + centre.ToString() + ", radius " + TTraits::ToString( radius ) + ")";
    }

    /** Parses the ToString form; false leaves this sphere unchanged */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TPoint parsedCentre;
        T parsedRadius;
        if ( !reader.TryLiteral( "Sphere" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "centre" ) || !reader.TryValue( parsedCentre ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "radius" ) || !reader.TryScalar( parsedRadius ) || !reader.TryChar( ')' ) || !reader.AtEnd() )
            return false;
        centre = parsedCentre;
        radius = parsedRadius;
        return true;
    }

    /** Writes the child node centre and the attribute radius into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        return SerializeShapeChild( node, "centre", centre, settings ) && SerializeShapeScalar( node, "radius", radius );
    }

    /** Reads the child node centre and the attribute radius; a missing or invalid one fails and leaves this sphere unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        TPoint parsedCentre;
        T parsedRadius;
        if ( !DeserializeShapeChild( node, "centre", parsedCentre, settings ) || !DeserializeShapeScalar( node, "radius", parsedRadius ) )
            return false;
        centre = parsedCentre;
        radius = parsedRadius;
        return true;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTSphere< T >::TPoint
CTSphere< T >::ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TVector d = point - centre;
    T length = d.GetLength();
    if ( length <= radius || !( TTraits::Zero() < length ) )
        return point;
    return centre + d * ( radius / length );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTSphere< T >::TPoint
CTSphere< T >::ClosestPointOnSurface( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TVector d = point - centre;
    T length = d.GetLength();
    if ( !( TTraits::Zero() < length ) )
        return centre + TVector( radius, TTraits::Zero(), TTraits::Zero() );
    return centre + d * ( radius / length );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTSphere< T >::TPoint
CTSphere< T >::GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TDirection d;
    if ( !TDirection::TryCreate( direction, d ) )
        return centre + TVector( radius, TTraits::Zero(), TTraits::Zero() );
    return centre + d * radius;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTSphere< T >::GetVolume( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ( ( ( radius * radius ) * radius ) * TTraits::Pi() ) * TTraits::FromInt32( 4 ) / TTraits::FromInt32( 3 );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTSphere< T >::GetSurfaceArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ( ( radius * radius ) * TTraits::Pi() ) * TTraits::FromInt32( 4 );
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTAxisAlignedBox3D< T >
CTSphere< T >::GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TVector r( radius, radius, radius );
    return CTAxisAlignedBox3D< T >( centre - r, centre + r );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTSphere< T >::RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !ray.origin.IsFinite() || !ray.direction.AsVector().IsFinite() )
        return false;                       /* a ray that is not a ray (a NaN or infinity in it) hits nothing */
    if ( !( TTraits::Zero() < radius ) )
        return false;                       /* no surface to hit, and no defined normal, for a radius of 0 or less */
    const TVector direction = ray.direction.AsVector();
    const TVector oc = ray.origin - centre;
    SRayCastResult< T > hit;
    if ( oc.GetLength() <= radius )
    {
        hit.distance = TTraits::Zero();
        hit.point = ray.origin;
        hit.normal = -direction;
        hit.flags = RAYHIT_STARTED_INSIDE;
        result = hit;
        return true;
    }

    T b = oc.Dot( direction );
    if ( !TTraits::IsNegative( b ) )
        return false;                       /* moving away from a sphere that the origin is outside of */
    TVector closestApproach = oc - direction * b;
    T discriminant = radius * radius - closestApproach.GetSquaredLength();
    if ( discriminant < TTraits::Zero() )
        return false;
    T t = ( TTraits::Zero() - b ) - TTraits::Sqrt( discriminant );
    if ( t < TTraits::Zero() || maxDistance < t )
        return false;
    hit.distance = t;
    hit.point = ray.origin + direction * t;
    hit.normal = ( hit.point - centre ) / radius;
    result = hit;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTSphere< T >::GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 segmentsPerCircle ) const
{
    const UInt32 n = segmentsPerCircle < 3 ? 3 : segmentsPerCircle;
    GUCEF::vector< MATH::CTVector2D< T > > ring;
    ring.reserve( n );
    for ( UInt32 i=0; i<n; ++i )
    {
        T angle = TTraits::TwoPi() * TTraits::FromInt32( (Int32) i ) / TTraits::FromInt32( (Int32) n );
        ring.push_back( MATH::CTVector2D< T >( TTraits::Cos( angle ) * radius, TTraits::Sin( angle ) * radius ) );
    }
    lines.reserve( lines.size() + 3 * n );
    for ( UInt32 plane=0; plane<3; ++plane )
    {
        for ( UInt32 i=0; i<n; ++i )
        {
            const MATH::CTVector2D< T >& a = ring[ i ];
            const MATH::CTVector2D< T >& b = ring[ ( i + 1 ) % n ];
            TVector va;
            TVector vb;
            va.SetComponent( plane, a.x );
            va.SetComponent( ( plane + 1 ) % 3, a.y );
            vb.SetComponent( plane, b.x );
            vb.SetComponent( ( plane + 1 ) % 3, b.y );
            lines.push_back( CTSegment3D< T >( centre + va, centre + vb ) );
        }
    }
}

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTSphere< T >
CTSphere< T >::Transformed( const MATH::CTMatrix4D< T >& matrix ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return CTSphere( matrix.TransformPoint( centre ), radius * MaxAxisScale( matrix ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTSphere< T >::TryTransformed( const MATH::CTMatrix4D< T >& matrix, CTSphere& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    T scale = TTraits::Zero();
    if ( !IsSimilarity( matrix, TTraits::DefaultRelativeTolerance(), scale ) )
        return false;
    out = CTSphere( matrix.TransformPoint( centre ), radius * scale );
    return true;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTSphere< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTSphere< Float64 > )

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

#endif /* GUCEF_GEOMETRY_CTSPHERE_H ? */
