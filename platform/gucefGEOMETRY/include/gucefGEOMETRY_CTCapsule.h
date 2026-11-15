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

#ifndef GUCEF_GEOMETRY_CTCAPSULE_H
#define GUCEF_GEOMETRY_CTCAPSULE_H

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
template < typename T > class CTSphere;
template < typename T > class CTSegment3D;
template < typename T > class CTAxisAlignedBox3D;

/*-------------------------------------------------------------------------*/

/**
 *  A solid capsule: a segment swept by a sphere, so a cylinder with hemispherical caps (section 7.2).
 *
 *  - Kind: floats (Float32, Float64 are exported); needs a square root.
 *  - Layout: start, end and radius, 7 * sizeof( T ), no virtual functions.
 *  - A zero length segment makes the capsule a sphere. A negative radius is not valid: it contains nothing.
 *  - Realtime: every member except the wireframe, text and DataNode members is realtime-safe.
 *  - Text: "Capsule(start (x, y, z), end (x, y, z), radius r)"; DataNode: child nodes start and end, attribute radius.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTCapsule
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

    GUCEF_GEOMETRY_FORCE_INLINE CTCapsule( void ) : start(), end( TTraits::Zero(), TTraits::One(), TTraits::Zero() ), radius( TTraits::One() ) {}

    GUCEF_GEOMETRY_FORCE_INLINE CTCapsule( const TPoint& startPoint, const TPoint& endPoint, const T& capsuleRadius )
        : start( startPoint ), end( endPoint ), radius( capsuleRadius )
    {
    }

    /** The core segment */
    GUCEF_GEOMETRY_FORCE_INLINE CTSegment3D< T > GetSegment( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The distance from the surface along the outward normal: negative inside, 0 on the surface */
    GUCEF_GEOMETRY_FORCE_INLINE T GetSignedDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** True when the point is inside or on the surface */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The nearest point of the solid capsule: the point itself when inside, otherwise on the surface */
    TPoint ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

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

    /** The point of the capsule furthest along 'direction' (the cap sphere at the end of the segment that is further along) */
    TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** pi r^2 L + 4/3 pi r^3 with L the segment length */
    T GetVolume( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** 2 pi r L + 4 pi r^2 */
    T GetSurfaceArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The axis aligned box around the capsule */
    CTAxisAlignedBox3D< T > GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  Hit of the ray with the solid capsule: the nearest of the cylinder wall (between the end points) and the two
     *  end caps. A capsule with a radius of 0 or less is a miss. A ray starting inside hits at distance 0 with RAYHIT_STARTED_INSIDE and the normal opposite to the ray.
     */
    bool RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  Appends a closed circle of 'segmentsPerCircle' segments around each end point (at least 4, rounded up to an even
     *  count), four lines along the wall, and in two perpendicular planes a half circle of half that many segments over each
     *  cap: 2 n + 4 + 4 ( n / 2 ) segments for n segments per circle.
     */
    void GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 segmentsPerCircle ) const;

    /**
     *  A capsule around the transformed capsule: both end points as points, the radius times the largest axis scale of
     *  the matrix. Exact for a similarity, a bounding capsule for non-uniform scales.
     */
    CTCapsule Transformed( const MATH::CTMatrix4D< T >& matrix ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The exact image: false (out unchanged) unless the matrix is a similarity within the default relative tolerance of the kind */
    bool TryTransformed( const MATH::CTMatrix4D< T >& matrix, CTCapsule& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTCapsule& x, const CTCapsule& y ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return x.start == y.start && x.end == y.end && x.radius == y.radius;
    }

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTCapsule& x, const CTCapsule& y ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !( x == y ); }

    GUCEF_GEOMETRY_FORCE_INLINE bool Equals( const CTCapsule& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return start.Equals( other.start ) && end.Equals( other.end ) &&
               TTraits::EqualsWithin( radius, other.radius, TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() );
    }

    /** "Capsule(start (x, y, z), end (x, y, z), radius r)" */
    CORE::CString ToString( void ) const
    {
        return "Capsule(start " + start.ToString() + ", end " + end.ToString() + ", radius " + TTraits::ToString( radius ) + ")";
    }

    /** Parses the ToString form; false leaves this capsule unchanged */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TPoint parsedStart;
        TPoint parsedEnd;
        T parsedRadius;
        if ( !reader.TryLiteral( "Capsule" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "start" ) || !reader.TryValue( parsedStart ) ||
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

    /** Reads the child nodes start and end and the attribute radius; a missing or invalid one fails and leaves this capsule unchanged */
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
GUCEF_GEOMETRY_FORCE_INLINE CTSegment3D< T >
CTCapsule< T >::GetSegment( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return CTSegment3D< T >( start, end );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE T
CTCapsule< T >::GetSignedDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return CTSegment3D< T >( start, end ).GetDistance( point ) - radius;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
CTCapsule< T >::Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return CTSegment3D< T >( start, end ).GetDistance( point ) <= radius;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTCapsule< T >::TPoint
CTCapsule< T >::ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TPoint onSegment = CTSegment3D< T >( start, end ).ClosestPoint( point );
    TVector d = point - onSegment;
    T length = d.GetLength();
    if ( length <= radius || !( TTraits::Zero() < length ) )
        return point;
    return onSegment + d * ( radius / length );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTCapsule< T >::TPoint
CTCapsule< T >::GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TPoint tip = CTSegment3D< T >( start, end ).GetSupport( direction );
    TDirection d;
    if ( !TDirection::TryCreate( direction, d ) )
        return tip;
    return tip + d * radius;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTCapsule< T >::GetVolume( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    T length = ( end - start ).GetLength();
    T r2 = radius * radius;
    T cylinder = ( r2 * TTraits::Pi() ) * length;
    T sphere = ( ( r2 * radius ) * TTraits::Pi() ) * TTraits::FromInt32( 4 ) / TTraits::FromInt32( 3 );
    return cylinder + sphere;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTCapsule< T >::GetSurfaceArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    T length = ( end - start ).GetLength();
    T wall = ( ( radius * TTraits::Pi() ) * TTraits::Two() ) * length;
    T caps = ( ( radius * radius ) * TTraits::Pi() ) * TTraits::FromInt32( 4 );
    return wall + caps;
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTAxisAlignedBox3D< T >
CTCapsule< T >::GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TVector r( radius, radius, radius );
    return CTAxisAlignedBox3D< T >( start.Min( end ) - r, start.Max( end ) + r );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTCapsule< T >::RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !ray.origin.IsFinite() || !ray.direction.AsVector().IsFinite() )
        return false;                       /* a ray that is not a ray (a NaN or infinity in it) hits nothing */
    if ( !( TTraits::Zero() < radius ) )
        return false;                       /* no surface to hit, and no defined normal, for a radius of 0 or less */
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

    const TVector axis = end - start;
    const T length = axis.GetLength();
    if ( !( TTraits::Zero() < length ) )
    {
        /* zero length: a sphere */
        return CTSphere< T >( start, radius ).RayCast( ray, maxDistance, result );
    }
    const TVector u = axis / length;

    bool found = false;
    T bestT = TTraits::Zero();
    TPoint bestCentre = start;       /* the point of the axis nearest to the hit, for the normal */

    /* the cap spheres, restricted to their outer halves */
    for ( UInt32 cap=0; cap<2; ++cap )
    {
        const TPoint& centre = 0 == cap ? start : end;
        SRayCastResult< T > sphereHit;
        if ( !CTSphere< T >( centre, radius ).RayCast( ray, maxDistance, sphereHit ) )
            continue;
        T s = ( sphereHit.point - start ).Dot( u );
        bool outer = 0 == cap ? !( TTraits::Zero() < s ) : !( s < length );
        if ( outer && ( !found || sphereHit.distance < bestT ) )
        {
            found = true;
            bestT = sphereHit.distance;
            bestCentre = centre;
        }
    }

    /* the wall: an infinite cylinder around the axis, restricted to the segment */
    const TVector m = ray.origin - start;
    const TVector dPerp = direction - u * direction.Dot( u );
    const TVector mPerp = m - u * m.Dot( u );
    const T a = dPerp.GetSquaredLength();
    if ( TTraits::Zero() < a )
    {
        const T b = mPerp.Dot( dPerp );
        const T c = mPerp.GetSquaredLength() - radius * radius;
        const T discriminant = b * b - a * c;
        if ( !TTraits::IsNegative( discriminant ) )
        {
            const T t = ( ( TTraits::Zero() - b ) - TTraits::Sqrt( discriminant ) ) / a;
            if ( !TTraits::IsNegative( t ) && !( maxDistance < t ) && ( !found || t < bestT ) )
            {
                const TPoint p = ray.origin + direction * t;
                const T s = ( p - start ).Dot( u );
                if ( !TTraits::IsNegative( s ) && s <= length )
                {
                    found = true;
                    bestT = t;
                    bestCentre = start + u * s;
                }
            }
        }
    }

    if ( !found )
        return false;
    hit.distance = bestT;
    hit.point = ray.origin + direction * bestT;
    hit.normal = ( hit.point - bestCentre ) / radius;
    result = hit;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTCapsule< T >::GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 segmentsPerCircle ) const
{
    UInt32 n = segmentsPerCircle < 4 ? 4 : segmentsPerCircle;
    if ( 0 != ( n & 1 ) )
        ++n;

    TDirection axis;
    TDirection::TryCreate( end - start, axis );
    TDirection first;
    TDirection::TryCreate( axis.AsVector().GetAnyPerpendicular(), first );
    TDirection second;
    TDirection::TryCreate( axis.AsVector().Cross( first.AsVector() ), second );
    const TVector p1 = first.AsVector() * radius;
    const TVector p2 = second.AsVector() * radius;
    const TVector pole = axis.AsVector() * radius;

    GUCEF::vector< MATH::CTVector2D< T > > ring;
    ring.reserve( n );
    for ( UInt32 i=0; i<n; ++i )
    {
        T angle = TTraits::TwoPi() * TTraits::FromInt32( (Int32) i ) / TTraits::FromInt32( (Int32) n );
        ring.push_back( MATH::CTVector2D< T >( TTraits::Cos( angle ), TTraits::Sin( angle ) ) );
    }

    /* the end circles */
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

    /* four lines along the wall */
    for ( UInt32 i=0; i<4; ++i )
    {
        const MATH::CTVector2D< T >& a = ring[ ( i * n ) / 4 ];
        TVector offset = p1 * a.x + p2 * a.y;
        lines.push_back( CTSegment3D< T >( start + offset, end + offset ) );
    }

    /* the cap arcs: from one side over the pole to the other, in the planes of p1 and p2 */
    const UInt32 half = n / 2;
    for ( UInt32 cap=0; cap<2; ++cap )
    {
        const TPoint& centre = 0 == cap ? start : end;
        const TVector poleVector = 0 == cap ? -pole : pole;
        for ( UInt32 plane=0; plane<2; ++plane )
        {
            const TVector side = 0 == plane ? p1 : p2;
            for ( UInt32 i=0; i<half; ++i )
            {
                T angleA = TTraits::Pi() * TTraits::FromInt32( (Int32) i ) / TTraits::FromInt32( (Int32) half );
                T angleB = TTraits::Pi() * TTraits::FromInt32( (Int32) ( i + 1 ) ) / TTraits::FromInt32( (Int32) half );
                lines.push_back( CTSegment3D< T >( centre + side * TTraits::Cos( angleA ) + poleVector * TTraits::Sin( angleA ),
                                                   centre + side * TTraits::Cos( angleB ) + poleVector * TTraits::Sin( angleB ) ) );
            }
        }
    }
}

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTCapsule< T >
CTCapsule< T >::Transformed( const MATH::CTMatrix4D< T >& matrix ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return CTCapsule( matrix.TransformPoint( start ), matrix.TransformPoint( end ), radius * MaxAxisScale( matrix ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTCapsule< T >::TryTransformed( const MATH::CTMatrix4D< T >& matrix, CTCapsule& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    T scale = TTraits::Zero();
    if ( !IsSimilarity( matrix, TTraits::DefaultRelativeTolerance(), scale ) )
        return false;
    out = CTCapsule( matrix.TransformPoint( start ), matrix.TransformPoint( end ), radius * scale );
    return true;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTCapsule< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTCapsule< Float64 > )

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

#ifndef GUCEF_GEOMETRY_CTSPHERE_H
#include "gucefGEOMETRY_CTSphere.h"
#define GUCEF_GEOMETRY_CTSPHERE_H
#endif /* GUCEF_GEOMETRY_CTSPHERE_H ? */

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

#endif /* GUCEF_GEOMETRY_CTCAPSULE_H ? */
