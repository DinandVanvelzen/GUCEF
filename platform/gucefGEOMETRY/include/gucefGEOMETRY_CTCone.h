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

#ifndef GUCEF_GEOMETRY_CTCONE_H
#define GUCEF_GEOMETRY_CTCONE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_SSHAPETEXTREADER_H
#include "gucefGEOMETRY_SShapeTextReader.h"
#define GUCEF_GEOMETRY_SSHAPETEXTREADER_H
#endif /* GUCEF_GEOMETRY_SSHAPETEXTREADER_H ? */

#ifndef GUCEF_GEOMETRY_SRAYCASTRESULT_H
#include "gucefGEOMETRY_SRayCastResult.h"
#define GUCEF_GEOMETRY_SRAYCASTRESULT_H
#endif /* GUCEF_GEOMETRY_SRAYCASTRESULT_H ? */

#ifndef GUCEF_GEOMETRY_SUPPORTFUNCTIONS_H
#include "gucefGEOMETRY_SupportFunctions.h"
#define GUCEF_GEOMETRY_SUPPORTFUNCTIONS_H
#endif /* GUCEF_GEOMETRY_SUPPORTFUNCTIONS_H ? */

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
template < typename T > class CTTriangle3D;

/*-------------------------------------------------------------------------*/

/**
 *  A solid cone with a flat base: an apex, the direction from the apex to the base, the height along that direction
 *  and the radius of the base (section 7.2).
 *
 *  - Kind: floats (Float32, Float64 are exported); needs a square root and, for the volume, transcendental constants.
 *  - Layout: apex point, axis direction, height and base radius, 8 * sizeof( T ), no virtual functions. The default
 *    cone has its apex at the origin and points up ( +Y ) with height 1 and base radius 1.
 *  - A cone is valid when its height is above zero and its base radius is not negative; an invalid cone contains
 *    nothing and is missed by rays. A base radius of 0 is a segment: it contains its axis, and rays miss it.
 *  - Realtime: every member except the wireframe, text and DataNode members is realtime-safe.
 *  - Text: "Cone(apex (x, y, z), axis (x, y, z), height h, radius r)"; DataNode: child nodes apex and axis,
 *    attributes height and radius.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTCone
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTDirectionVector3D< T > TDirection;

    TPoint apex;
    TDirection axis;
    T height;
    T baseRadius;

    GUCEF_GEOMETRY_FORCE_INLINE CTCone( void ) : apex(), axis( TDirection::Up() ), height( TTraits::One() ), baseRadius( TTraits::One() ) {}

    GUCEF_GEOMETRY_FORCE_INLINE CTCone( const TPoint& apexPoint, const TDirection& unitAxis, const T& coneHeight, const T& coneBaseRadius )
        : apex( apexPoint ), axis( unitAxis ), height( coneHeight ), baseRadius( coneBaseRadius )
    {
    }

    /** The centre of the base: apex + axis * height */
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetBaseCentre( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return apex + axis * height; }

    /** True when the height is above zero and the base radius is not negative */
    GUCEF_GEOMETRY_FORCE_INLINE bool IsValid( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return TTraits::Zero() < height && TTraits::Zero() <= baseRadius; }

    /** True when the point is inside or on the surface; false for an invalid cone */
    bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  The nearest point of the solid cone: the point itself when inside; otherwise found in the half plane through the
     *  axis and the point (the solid of revolution of the triangle apex, base centre, base rim), which makes the result
     *  exact. The apex for an invalid cone.
     */
    TPoint ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Distance from the solid to the point, 0 inside */
    T GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    T GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The apex or the point of the base rim furthest along 'direction' (the rim is replaced by the base centre for a direction along the axis); the apex for a zero direction */
    TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** pi r^2 h / 3; 0 for an invalid cone */
    T GetVolume( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** pi r ( r + sqrt( r^2 + h^2 ) ) for the lateral surface and the base; 0 for an invalid cone */
    T GetSurfaceArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The tight axis aligned box: the apex and the base circle ( centre grown by r * sqrt( 1 - u_i^2 ) along each axis ) */
    CTAxisAlignedBox3D< T > GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  Hit of the ray with the solid cone: the lateral surface as a quadric (only the entering crossings between apex
     *  and base count, so the mirrored cone behind the apex is never hit) and the flat base. A ray starting inside hits
     *  at distance 0 with RAYHIT_STARTED_INSIDE and the normal opposite to the ray. The feature index is 0 for the
     *  lateral surface and 1 for the base. Rays along a generator of the cone have no division by zero.
     */
    bool RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Appends the base circle ('segmentsPerCircle' segments, at least 3) and four lines from the apex to the rim */
    void GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 segmentsPerCircle ) const;

    /**
     *  The exact image: false (out unchanged) unless the matrix is a similarity within the default relative tolerance of
     *  the kind (a cone is not closed under other transforms, so there is no Transformed)
     */
    bool TryTransformed( const MATH::CTMatrix4D< T >& matrix, CTCone& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTCone& a, const CTCone& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return a.apex == b.apex && a.axis == b.axis && a.height == b.height && a.baseRadius == b.baseRadius;
    }
    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTCone& a, const CTCone& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !( a == b ); }

    /** Apex, axis, height and radius equal within the default tolerances of the kind */
    GUCEF_GEOMETRY_FORCE_INLINE bool Equals( const CTCone& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return apex.Equals( other.apex ) && axis.Equals( other.axis ) &&
               TTraits::EqualsWithin( height, other.height, TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() ) &&
               TTraits::EqualsWithin( baseRadius, other.baseRadius, TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() );
    }

    /** "Cone(apex (x, y, z), axis (x, y, z), height h, radius r)" */
    CORE::CString ToString( void ) const
    {
        return "Cone(apex " + apex.ToString() + ", axis " + axis.ToString() + ", height " + TTraits::ToString( height ) + ", radius " + TTraits::ToString( baseRadius ) + ")";
    }

    /** Parses the ToString form; false leaves this cone unchanged (the axis must be of unit length) */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TPoint parsedApex;
        TDirection parsedAxis;
        T parsedHeight;
        T parsedRadius;
        if ( !reader.TryLiteral( "Cone" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "apex" ) || !reader.TryValue( parsedApex ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "axis" ) || !reader.TryValue( parsedAxis ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "height" ) || !reader.TryScalar( parsedHeight ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "radius" ) || !reader.TryScalar( parsedRadius ) || !reader.TryChar( ')' ) || !reader.AtEnd() )
            return false;
        apex = parsedApex;
        axis = parsedAxis;
        height = parsedHeight;
        baseRadius = parsedRadius;
        return true;
    }

    /** Writes the child nodes apex and axis and the attributes height and radius into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        return SerializeShapeChild( node, "apex", apex, settings ) && SerializeShapeChild( node, "axis", axis, settings ) &&
               SerializeShapeScalar( node, "height", height ) && SerializeShapeScalar( node, "radius", baseRadius );
    }

    /** Reads the child nodes apex and axis and the attributes height and radius; a missing or invalid one fails and leaves this cone unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        TPoint parsedApex;
        TDirection parsedAxis;
        T parsedHeight;
        T parsedRadius;
        if ( !DeserializeShapeChild( node, "apex", parsedApex, settings ) || !DeserializeShapeChild( node, "axis", parsedAxis, settings ) ||
             !DeserializeShapeScalar( node, "height", parsedHeight ) || !DeserializeShapeScalar( node, "radius", parsedRadius ) )
            return false;
        apex = parsedApex;
        axis = parsedAxis;
        height = parsedHeight;
        baseRadius = parsedRadius;
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
CTCone< T >::Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !IsValid() )
        return false;
    const TVector u = axis.AsVector();
    const TVector w = point - apex;
    const T s = w.Dot( u );
    if ( s < TTraits::Zero() || height < s )
        return false;
    return ( w - u * s ).GetLength() * height <= baseRadius * s;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTCone< T >::TPoint
CTCone< T >::ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !IsValid() )
        return apex;
    if ( Contains( point ) )
        return point;
    const TVector u = axis.AsVector();
    const TVector w = point - apex;
    const T s = w.Dot( u );
    TVector perpendicular = w - u * s;
    T rho = perpendicular.GetLength();
    TVector radial;
    if ( TTraits::Zero() < rho )
        radial = perpendicular / rho;
    else
        radial = u.GetAnyPerpendicular().Normalized();
    const TPoint base = apex + u * height;
    /* the meridian half plane of the point: the cone is the solid of revolution of this triangle */
    return CTTriangle3D< T >( apex, base, base + radial * baseRadius ).ClosestPoint( point );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTCone< T >::GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ( point - ClosestPoint( point ) ).GetLength();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTCone< T >::GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ( point - ClosestPoint( point ) ).GetSquaredLength();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTCone< T >::TPoint
CTCone< T >::GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return GetConeSupport( apex, GetBaseCentre(), axis.AsVector(), baseRadius, direction );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTCone< T >::GetVolume( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !IsValid() )
        return TTraits::Zero();
    return ( ( ( baseRadius * baseRadius ) * TTraits::Pi() ) * height ) / TTraits::FromInt32( 3 );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTCone< T >::GetSurfaceArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !IsValid() )
        return TTraits::Zero();
    const T slant = TTraits::Sqrt( baseRadius * baseRadius + height * height );
    return ( baseRadius * TTraits::Pi() ) * ( baseRadius + slant );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTAxisAlignedBox3D< T >
CTCone< T >::GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const TVector u = axis.AsVector();
    const T one = TTraits::One();
    const T zero = TTraits::Zero();
    const T fx = one - u.x * u.x;
    const T fy = one - u.y * u.y;
    const T fz = one - u.z * u.z;
    const TVector e( baseRadius * TTraits::Sqrt( zero < fx ? fx : zero ), baseRadius * TTraits::Sqrt( zero < fy ? fy : zero ), baseRadius * TTraits::Sqrt( zero < fz ? fz : zero ) );
    const TPoint base = apex + u * height;
    return CTAxisAlignedBox3D< T >( apex.Min( base - e ), apex.Max( base + e ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTCone< T >::RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !ray.origin.IsFinite() || !ray.direction.AsVector().IsFinite() )
        return false;                       /* a ray that is not a ray (a NaN or infinity in it) hits nothing */
    if ( !apex.IsFinite() || !axis.AsVector().IsFinite() )
        return false;
    if ( !( TTraits::Zero() < height ) || !( TTraits::Zero() < baseRadius ) )
        return false;
    const TVector direction = ray.direction.AsVector();
    SRayCastResult< T > hit;

    if ( Contains( ray.origin ) )
    {
        hit.distance = TTraits::Zero();
        hit.point = ray.origin;
        hit.normal = -direction;
        hit.flags = RAYHIT_STARTED_INSIDE;
        result = hit;
        return true;
    }

    const TVector u = axis.AsVector();
    const TVector w = ray.origin - apex;
    const T wAlong = w.Dot( u );
    const T dAlong = direction.Dot( u );
    const T ratio = baseRadius / height;
    const T factor = TTraits::One() + ratio * ratio;       /* 1 + tan^2 of the half angle */

    bool found = false;
    T bestT = TTraits::Zero();
    TVector bestNormal;
    Int32 bestFeature = -1;

    /*
     *  the lateral surface: |V|^2 - factor ( V . u )^2 = 0 along V = w + t d, the entering crossings with the axial
     *  coordinate in [ 0, h ]. The quadric is solved in Float64 whatever the kind: it has differences of large products
     *  that make the roots of rays grazing the surface lose half the digits of a Float32.
     */
    const Float64 factor64 = (Float64) factor;
    const Float64 axisLength = sqrt( ( (Float64) u.x * (Float64) u.x + (Float64) u.y * (Float64) u.y ) + (Float64) u.z * (Float64) u.z );
    const Float64 ux = (Float64) u.x / axisLength;
    const Float64 uy = (Float64) u.y / axisLength;
    const Float64 uz = (Float64) u.z / axisLength;
    const Float64 rayLength = sqrt( ( (Float64) direction.x * (Float64) direction.x + (Float64) direction.y * (Float64) direction.y ) + (Float64) direction.z * (Float64) direction.z );
    const Float64 dx = (Float64) direction.x / rayLength;
    const Float64 dy = (Float64) direction.y / rayLength;
    const Float64 dz = (Float64) direction.z / rayLength;
    const Float64 wx = (Float64) w.x;
    const Float64 wy = (Float64) w.y;
    const Float64 wz = (Float64) w.z;
    const Float64 dAlong64 = ( dx * ux + dy * uy ) + dz * uz;
    const Float64 wAlong64 = ( wx * ux + wy * uy ) + wz * uz;
    const Float64 dw = ( dx * wx + dy * wy ) + dz * wz;
    const Float64 ww = ( wx * wx + wy * wy ) + wz * wz;
    const Float64 a = 1.0 - factor64 * ( dAlong64 * dAlong64 );
    const Float64 b = dw - factor64 * ( dAlong64 * wAlong64 );
    const Float64 c = ww - factor64 * ( wAlong64 * wAlong64 );
    Float64 roots[ 2 ];
    UInt32 rootCount = 0;
    const Float64 scale = fabs( a ) > fabs( b ) ? ( fabs( a ) > fabs( c ) ? fabs( a ) : fabs( c ) ) : ( fabs( b ) > fabs( c ) ? fabs( b ) : fabs( c ) );
    if ( 0.0 < scale )
    {
        if ( fabs( a ) <= scale * 1.0e-14 )
        {
            if ( 0.0 != b )
                roots[ rootCount++ ] = ( 0.0 - c ) / ( b * 2.0 );
        }
        else
        {
            const Float64 discriminant = b * b - a * c;
            if ( discriminant >= 0.0 )
            {
                const Float64 root = sqrt( discriminant );
                /* the numerically stable pair: q avoids the cancellation of -b +- root */
                const Float64 q = b < 0.0 ? ( 0.0 - b ) + root : ( 0.0 - b ) - root;
                roots[ rootCount++ ] = q / a;
                if ( 0.0 != q )
                    roots[ rootCount++ ] = c / q;
            }
        }
    }
    for ( UInt32 i=0; i<rootCount; ++i )
    {
        const T t = (T) roots[ i ];
        if ( TTraits::IsNegative( t ) || maxDistance < t || ( found && !( t < bestT ) ) )
            continue;
        if ( !( a * roots[ i ] + b < 0.0 ) )
            continue;                       /* the quadric grows along the ray: leaving the lateral surface, not entering */
        const T s = wAlong + dAlong * t;
        if ( TTraits::IsNegative( s ) || height < s )
            continue;
        /* the normal is the gradient of the quadric at the hit, evaluated in Float64 from the root as well */
        const Float64 root = roots[ i ];
        const Float64 s64 = wAlong64 + dAlong64 * root;
        const Float64 vx = wx + dx * root - ux * ( factor64 * s64 );
        const Float64 vy = wy + dy * root - uy * ( factor64 * s64 );
        const Float64 vz = wz + dz * root - uz * ( factor64 * s64 );
        const Float64 normalLength = sqrt( ( vx * vx + vy * vy ) + vz * vz );
        TVector normal = -u;
        if ( 0.0 < normalLength )
            normal = TVector( (T) ( vx / normalLength ), (T) ( vy / normalLength ), (T) ( vz / normalLength ) );
        found = true;
        bestT = t;
        bestNormal = normal;
        bestFeature = 0;
    }

    /* the base: entered when moving against the axis direction */
    if ( dAlong < TTraits::Zero() )
    {
        const T t = ( height - wAlong ) / dAlong;
        if ( !TTraits::IsNegative( t ) && !( maxDistance < t ) && ( !found || t < bestT ) )
        {
            const TVector radial = ( w + direction * t ) - u * height;
            if ( radial.GetLength() <= baseRadius )
            {
                found = true;
                bestT = t;
                bestNormal = u;
                bestFeature = 1;
            }
        }
    }

    /*
     *  a ray through the apex whose direction lies inside the cone enters the solid there: the quadric has a double root
     *  at the apex, which the entering test above does not take for a crossing, and the ray may leave through the base
     */
    {
        const Float64 tApex = 0.0 - dw;
        const Float64 missSquared = ww - dw * dw;
        if ( 0.0 <= tApex && tApex <= (Float64) maxDistance && missSquared <= 1.0e-14 * ww && dAlong64 * dAlong64 * factor64 >= 1.0 - 1.0e-12 && 0.0 < dAlong64 )
        {
            const T t = (T) tApex;
            if ( !found || t < bestT )
            {
                found = true;
                bestT = t;
                bestNormal = -u;
                bestFeature = 0;
            }
        }
    }

    if ( !found )
        return false;
    hit.distance = bestT;
    hit.point = ray.origin + direction * bestT;
    hit.normal = bestNormal;
    hit.featureIndex = bestFeature;
    result = hit;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTCone< T >::GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 segmentsPerCircle ) const
{
    const UInt32 n = segmentsPerCircle < 3 ? 3 : segmentsPerCircle;
    TDirection first;
    TDirection::TryCreate( axis.AsVector().GetAnyPerpendicular(), first );
    TDirection second;
    TDirection::TryCreate( axis.AsVector().Cross( first.AsVector() ), second );
    const TVector p1 = first.AsVector() * baseRadius;
    const TVector p2 = second.AsVector() * baseRadius;
    const TPoint base = GetBaseCentre();

    GUCEF::vector< MATH::CTVector2D< T > > ring;
    ring.reserve( n );
    for ( UInt32 i=0; i<n; ++i )
    {
        T angle = TTraits::TwoPi() * TTraits::FromInt32( (Int32) i ) / TTraits::FromInt32( (Int32) n );
        ring.push_back( MATH::CTVector2D< T >( TTraits::Cos( angle ), TTraits::Sin( angle ) ) );
    }
    for ( UInt32 i=0; i<n; ++i )
    {
        const MATH::CTVector2D< T >& a = ring[ i ];
        const MATH::CTVector2D< T >& b = ring[ ( i + 1 ) % n ];
        lines.push_back( CTSegment3D< T >( base + p1 * a.x + p2 * a.y, base + p1 * b.x + p2 * b.y ) );
    }
    for ( UInt32 i=0; i<4; ++i )
    {
        const MATH::CTVector2D< T >& a = ring[ ( i * n ) / 4 ];
        lines.push_back( CTSegment3D< T >( apex, base + p1 * a.x + p2 * a.y ) );
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTCone< T >::TryTransformed( const MATH::CTMatrix4D< T >& matrix, CTCone& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    T scale = TTraits::Zero();
    if ( !IsSimilarity( matrix, TTraits::DefaultRelativeTolerance(), scale ) )
        return false;
    TDirection movedAxis;
    if ( !TDirection::TryCreate( matrix.TransformVector( axis.AsVector() ), movedAxis ) )
        return false;
    out = CTCone( matrix.TransformPoint( apex ), movedAxis, height * scale, baseRadius * scale );
    return true;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTCone< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTCone< Float64 > )

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

#ifndef GUCEF_GEOMETRY_CTSEGMENT3D_H
#include "gucefGEOMETRY_CTSegment3D.h"
#define GUCEF_GEOMETRY_CTSEGMENT3D_H
#endif /* GUCEF_GEOMETRY_CTSEGMENT3D_H ? */

#ifndef GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#include "gucefGEOMETRY_CTAxisAlignedBox3D.h"
#define GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#endif /* GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H ? */

#ifndef GUCEF_GEOMETRY_CTRAY3D_H
#include "gucefGEOMETRY_CTRay3D.h"
#define GUCEF_GEOMETRY_CTRAY3D_H
#endif /* GUCEF_GEOMETRY_CTRAY3D_H ? */

#ifndef GUCEF_GEOMETRY_CTTRIANGLE3D_H
#include "gucefGEOMETRY_CTTriangle3D.h"
#define GUCEF_GEOMETRY_CTTRIANGLE3D_H
#endif /* GUCEF_GEOMETRY_CTTRIANGLE3D_H ? */

#ifndef GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H
#include "gucefGEOMETRY_TransformFunctions.h"
#define GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H
#endif /* GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTCONE_H ? */
