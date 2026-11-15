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

#ifndef GUCEF_GEOMETRY_CTCYLINDER_H
#define GUCEF_GEOMETRY_CTCYLINDER_H

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

/*-------------------------------------------------------------------------*/

/**
 *  A solid cylinder with flat caps: an axis segment and a radius (section 7.2).
 *
 *  - Kind: floats (Float32, Float64 are exported); needs a square root and, for the volume, transcendental constants.
 *  - Layout: start, end and radius, 7 * sizeof( T ), no virtual functions. The default cylinder runs from the origin
 *    to ( 0, 1, 0 ) with radius 1.
 *  - A cylinder is valid when its axis has a length above zero and its radius is not negative; an invalid cylinder
 *    contains nothing and is missed by rays. A radius of 0 is a segment: it contains its axis, and rays miss it.
 *  - Realtime: every member except the wireframe, text and DataNode members is realtime-safe.
 *  - Text: "Cylinder(start (x, y, z), end (x, y, z), radius r)"; DataNode: child nodes start and end, attribute radius.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTCylinder
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTDirectionVector3D< T > TDirection;

    TPoint start;
    TPoint end;
    T radius;

    GUCEF_GEOMETRY_FORCE_INLINE CTCylinder( void ) : start(), end( TTraits::Zero(), TTraits::One(), TTraits::Zero() ), radius( TTraits::One() ) {}

    GUCEF_GEOMETRY_FORCE_INLINE CTCylinder( const TPoint& startPoint, const TPoint& endPoint, const T& cylinderRadius )
        : start( startPoint ), end( endPoint ), radius( cylinderRadius )
    {
    }

    /** The axis segment */
    CTSegment3D< T > GetAxis( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The length of the axis */
    GUCEF_GEOMETRY_FORCE_INLINE T GetLength( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( end - start ).GetLength(); }

    /** True when the axis has a length above zero and the radius is not negative */
    bool IsValid( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** True when the point is inside or on the surface; false for an invalid cylinder */
    bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The nearest point of the solid cylinder: the point itself when inside; the start for an invalid cylinder */
    TPoint ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Distance from the solid to the point, 0 inside */
    T GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    T GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The point of the solid furthest along 'direction': on the cap rim for a direction off the axis, the cap centre on the axis; the start end for a zero direction */
    TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** pi r^2 times the length; 0 for an invalid cylinder */
    T GetVolume( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** 2 pi r ( r + length ) for the wall and both caps; 0 for an invalid cylinder */
    T GetSurfaceArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The tight axis aligned box: the end points grown by r * sqrt( 1 - u_i^2 ) along each axis, u the unit axis */
    CTAxisAlignedBox3D< T > GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  Hit of the ray with the solid cylinder: wall (stable quadratic) and the two flat caps. A ray starting inside
     *  hits at distance 0 with RAYHIT_STARTED_INSIDE and the normal opposite to the ray. The feature index is 0 for the
     *  wall, 1 for the start cap and 2 for the end cap. Rays parallel to the axis or to a cap have no division by zero.
     */
    bool RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Appends both cap circles ('segmentsPerCircle' segments each, at least 3) and four lines along the wall */
    void GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 segmentsPerCircle ) const;

    /**
     *  The exact image: false (out unchanged) unless the matrix is a similarity within the default relative tolerance of
     *  the kind (a cylinder is not closed under other transforms, so there is no Transformed)
     */
    bool TryTransformed( const MATH::CTMatrix4D< T >& matrix, CTCylinder& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTCylinder& a, const CTCylinder& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return a.start == b.start && a.end == b.end && a.radius == b.radius; }
    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTCylinder& a, const CTCylinder& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !( a == b ); }

    /** Axis end points and radius equal within the default tolerances of the kind */
    GUCEF_GEOMETRY_FORCE_INLINE bool Equals( const CTCylinder& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return start.Equals( other.start ) && end.Equals( other.end ) && TTraits::EqualsWithin( radius, other.radius, TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() );
    }

    /** "Cylinder(start (x, y, z), end (x, y, z), radius r)" */
    CORE::CString ToString( void ) const
    {
        return "Cylinder(start " + start.ToString() + ", end " + end.ToString() + ", radius " + TTraits::ToString( radius ) + ")";
    }

    /** Parses the ToString form; false leaves this cylinder unchanged */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TPoint parsedStart;
        TPoint parsedEnd;
        T parsedRadius;
        if ( !reader.TryLiteral( "Cylinder" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "start" ) || !reader.TryValue( parsedStart ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "end" ) || !reader.TryValue( parsedEnd ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "radius" ) || !reader.TryScalar( parsedRadius ) || !reader.TryChar( ')' ) || !reader.AtEnd() )
            return false;
        start = parsedStart;
        end = parsedEnd;
        radius = parsedRadius;
        return true;
    }

    /** Writes the child nodes start and end and the attribute radius into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        return SerializeShapeChild( node, "start", start, settings ) && SerializeShapeChild( node, "end", end, settings ) && SerializeShapeScalar( node, "radius", radius );
    }

    /** Reads the child nodes start and end and the attribute radius; a missing or invalid one fails and leaves this cylinder unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        TPoint parsedStart;
        TPoint parsedEnd;
        T parsedRadius;
        if ( !DeserializeShapeChild( node, "start", parsedStart, settings ) || !DeserializeShapeChild( node, "end", parsedEnd, settings ) ||
             !DeserializeShapeScalar( node, "radius", parsedRadius ) )
            return false;
        start = parsedStart;
        end = parsedEnd;
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
CTSegment3D< T >
CTCylinder< T >::GetAxis( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return CTSegment3D< T >( start, end );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTCylinder< T >::IsValid( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return TTraits::Zero() < ( end - start ).GetSquaredLength() && TTraits::Zero() <= radius;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTCylinder< T >::Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const TVector axis = end - start;
    const T length = axis.GetLength();
    if ( !( TTraits::Zero() < length ) || !( TTraits::Zero() <= radius ) )
        return false;
    const TVector u = axis / length;
    const TVector w = point - start;
    const T s = w.Dot( u );
    if ( s < TTraits::Zero() || length < s )
        return false;
    return ( w - u * s ).GetLength() <= radius;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTCylinder< T >::TPoint
CTCylinder< T >::ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const TVector axis = end - start;
    const T length = axis.GetLength();
    if ( !( TTraits::Zero() < length ) || !( TTraits::Zero() <= radius ) )
        return start;
    const TVector u = axis / length;
    const TVector w = point - start;
    const T s = w.Dot( u );
    const TVector perpendicular = w - u * s;
    const T rho = perpendicular.GetLength();
    if ( !( s < TTraits::Zero() ) && !( length < s ) && rho <= radius )
        return point;
    const T clampedS = s < TTraits::Zero() ? TTraits::Zero() : ( length < s ? length : s );
    TPoint result = start + u * clampedS;
    if ( TTraits::Zero() < rho )
        result = result + perpendicular * ( ( radius < rho ? radius : rho ) / rho );
    return result;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTCylinder< T >::GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ( point - ClosestPoint( point ) ).GetLength();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTCylinder< T >::GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ( point - ClosestPoint( point ) ).GetSquaredLength();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTCylinder< T >::TPoint
CTCylinder< T >::GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TVector unit;
    if ( !( end - start ).TryNormalize( unit, TTraits::Zero() ) )
        return start;
    return GetCylinderSupport( start, end, unit, radius, direction );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTCylinder< T >::GetVolume( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !IsValid() )
        return TTraits::Zero();
    return ( ( radius * radius ) * TTraits::Pi() ) * GetLength();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTCylinder< T >::GetSurfaceArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !IsValid() )
        return TTraits::Zero();
    return ( ( radius * TTraits::TwoPi() ) * ( radius + GetLength() ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTAxisAlignedBox3D< T >
CTCylinder< T >::GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const TVector axis = end - start;
    const T length = axis.GetLength();
    TVector e( radius, radius, radius );
    if ( TTraits::Zero() < length )
    {
        const TVector u = axis / length;
        const T one = TTraits::One();
        const T zero = TTraits::Zero();
        const T fx = one - u.x * u.x;
        const T fy = one - u.y * u.y;
        const T fz = one - u.z * u.z;
        e = TVector( radius * TTraits::Sqrt( zero < fx ? fx : zero ), radius * TTraits::Sqrt( zero < fy ? fy : zero ), radius * TTraits::Sqrt( zero < fz ? fz : zero ) );
    }
    return CTAxisAlignedBox3D< T >( start.Min( end ) - e, start.Max( end ) + e );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTCylinder< T >::RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !ray.origin.IsFinite() || !ray.direction.AsVector().IsFinite() )
        return false;                       /* a ray that is not a ray (a NaN or infinity in it) hits nothing */
    if ( !start.IsFinite() || !end.IsFinite() )
        return false;
    const TVector axis = end - start;
    const T length = axis.GetLength();
    if ( !( TTraits::Zero() < length ) || !( TTraits::Zero() < radius ) )
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

    const TVector u = axis / length;
    const TVector w = ray.origin - start;
    const T wAlong = w.Dot( u );
    const T dAlong = direction.Dot( u );
    bool found = false;
    T bestT = TTraits::Zero();
    TVector bestNormal;
    Int32 bestFeature = -1;

    /* the wall: the infinite cylinder around the axis, restricted to the axis segment */
    const TVector dPerp = direction - u * dAlong;
    const TVector wPerp = w - u * wAlong;
    const T a = dPerp.GetSquaredLength();
    if ( TTraits::Zero() < a )
    {
        const T b = wPerp.Dot( dPerp );
        const T c = wPerp.GetSquaredLength() - radius * radius;
        const T discriminant = b * b - a * c;
        if ( !TTraits::IsNegative( discriminant ) )
        {
            const T t = ( ( TTraits::Zero() - b ) - TTraits::Sqrt( discriminant ) ) / a;
            if ( !TTraits::IsNegative( t ) && !( maxDistance < t ) )
            {
                const T s = wAlong + dAlong * t;
                if ( !TTraits::IsNegative( s ) && s <= length )
                {
                    found = true;
                    bestT = t;
                    bestNormal = ( wPerp + dPerp * t ) / radius;
                    bestFeature = 0;
                }
            }
        }
    }

    /* the caps: entered only when moving along the inward axis direction of the cap */
    if ( TTraits::Zero() != dAlong )
    {
        for ( UInt32 cap=0; cap<2; ++cap )
        {
            const bool entering = 0 == cap ? TTraits::Zero() < dAlong : dAlong < TTraits::Zero();
            if ( !entering )
                continue;
            const T t = ( ( 0 == cap ? TTraits::Zero() : length ) - wAlong ) / dAlong;
            if ( TTraits::IsNegative( t ) || maxDistance < t || ( found && !( t < bestT ) ) )
                continue;
            const TVector radial = wPerp + dPerp * t;
            if ( radial.GetLength() <= radius )
            {
                found = true;
                bestT = t;
                bestNormal = 0 == cap ? -u : u;
                bestFeature = (Int32) cap + 1;
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
CTCylinder< T >::GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 segmentsPerCircle ) const
{
    const UInt32 n = segmentsPerCircle < 3 ? 3 : segmentsPerCircle;
    TDirection axis;
    if ( !TDirection::TryCreate( end - start, axis ) )
        return;
    TDirection first;
    TDirection::TryCreate( axis.AsVector().GetAnyPerpendicular(), first );
    TDirection second;
    TDirection::TryCreate( axis.AsVector().Cross( first.AsVector() ), second );
    const TVector p1 = first.AsVector() * radius;
    const TVector p2 = second.AsVector() * radius;

    GUCEF::vector< MATH::CTVector2D< T > > ring;
    ring.reserve( n );
    for ( UInt32 i=0; i<n; ++i )
    {
        T angle = TTraits::TwoPi() * TTraits::FromInt32( (Int32) i ) / TTraits::FromInt32( (Int32) n );
        ring.push_back( MATH::CTVector2D< T >( TTraits::Cos( angle ), TTraits::Sin( angle ) ) );
    }
    for ( UInt32 cap=0; cap<2; ++cap )
    {
        const TPoint& centre = 0 == cap ? start : end;
        for ( UInt32 i=0; i<n; ++i )
        {
            const MATH::CTVector2D< T >& a = ring[ i ];
            const MATH::CTVector2D< T >& b = ring[ ( i + 1 ) % n ];
            lines.push_back( CTSegment3D< T >( centre + p1 * a.x + p2 * a.y, centre + p1 * b.x + p2 * b.y ) );
        }
    }
    for ( UInt32 i=0; i<4; ++i )
    {
        const MATH::CTVector2D< T >& a = ring[ ( i * n ) / 4 ];
        const TVector offset = p1 * a.x + p2 * a.y;
        lines.push_back( CTSegment3D< T >( start + offset, end + offset ) );
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTCylinder< T >::TryTransformed( const MATH::CTMatrix4D< T >& matrix, CTCylinder& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    T scale = TTraits::Zero();
    if ( !IsSimilarity( matrix, TTraits::DefaultRelativeTolerance(), scale ) )
        return false;
    out = CTCylinder( matrix.TransformPoint( start ), matrix.TransformPoint( end ), radius * scale );
    return true;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTCylinder< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTCylinder< Float64 > )

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

#ifndef GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H
#include "gucefGEOMETRY_TransformFunctions.h"
#define GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H
#endif /* GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTCYLINDER_H ? */
