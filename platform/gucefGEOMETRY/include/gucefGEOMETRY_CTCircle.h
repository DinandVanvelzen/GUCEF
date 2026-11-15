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

#ifndef GUCEF_GEOMETRY_CTCIRCLE_H
#define GUCEF_GEOMETRY_CTCIRCLE_H

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
 *  A solid circle (disc) in 2D (the XY plane of its space): centre and radius (section 7.2).
 *
 *  - Kind: floats (Float32, Float64 are exported); needs a square root and transcendental constants.
 *  - Layout: centre point and radius, 3 * sizeof( T ), no virtual functions. The default circle is the unit circle at the origin.
 *  - A negative radius is not valid: it contains nothing. Distances are to the solid: 0 inside.
 *  - Realtime: every member except the wireframe, text and DataNode members is realtime-safe.
 *  - Text: "Circle(centre (x, y), radius r)"; DataNode: child node centre and the attribute radius.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTCircle
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint2D< T > TPoint;
    typedef MATH::CTVector2D< T > TVector;
    typedef MATH::CTDirectionVector2D< T > TDirection;

    TPoint centre;
    T radius;

    GUCEF_GEOMETRY_FORCE_INLINE CTCircle( void ) : centre(), radius( TTraits::One() ) {}

    GUCEF_GEOMETRY_FORCE_INLINE CTCircle( const TPoint& centrePoint, const T& circleRadius ) : centre( centrePoint ), radius( circleRadius ) {}

    /** The distance from the boundary along the outward normal: negative inside, 0 on the boundary */
    GUCEF_GEOMETRY_FORCE_INLINE T GetSignedDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return ( point - centre ).GetLength() - radius;
    }

    /** True when the point is inside or on the boundary */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return ( point - centre ).GetLength() <= radius;
    }

    /** The nearest point of the disc: the point itself when inside, otherwise on the boundary */
    TPoint ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

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

    /** The point of the circle furthest along 'direction'; the +X point for a zero direction */
    TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** pi r^2 */
    T GetArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** 2 pi r */
    T GetCircumference( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The axis aligned rectangle around the circle */
    GUCEF_GEOMETRY_FORCE_INLINE CTRectangle< T > GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        TVector r( radius, radius );
        return CTRectangle< T >( centre - r, centre + r );
    }

    /** True when the discs share a point */
    GUCEF_GEOMETRY_FORCE_INLINE bool Intersects( const CTCircle& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return ( other.centre - centre ).GetLength() <= radius + other.radius;
    }

    /** Appends the boundary as a closed polygon of 'segmentsPerCircle' segments (at least 3), counter-clockwise from the +X point */
    void GetWireframeLines( GUCEF::vector< CTSegment2D< T > >& lines, UInt32 segmentsPerCircle ) const;

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTCircle& a, const CTCircle& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return a.centre == b.centre && a.radius == b.radius; }
    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTCircle& a, const CTCircle& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !( a == b ); }

    GUCEF_GEOMETRY_FORCE_INLINE bool Equals( const CTCircle& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return centre.Equals( other.centre ) && TTraits::EqualsWithin( radius, other.radius, TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() );
    }

    /** "Circle(centre (x, y), radius r)" */
    CORE::CString ToString( void ) const
    {
        return "Circle(centre " + centre.ToString() + ", radius " + TTraits::ToString( radius ) + ")";
    }

    /** Parses the ToString form; false leaves this circle unchanged */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TPoint parsedCentre;
        T parsedRadius;
        if ( !reader.TryLiteral( "Circle" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "centre" ) || !reader.TryValue( parsedCentre ) ||
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

    /** Reads the child node centre and the attribute radius; a missing or invalid one fails and leaves this circle unchanged */
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
GUCEF_GEOMETRY_PRECISE_INLINE typename CTCircle< T >::TPoint
CTCircle< T >::ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TVector d = point - centre;
    T length = d.GetLength();
    if ( length <= radius || !( TTraits::Zero() < length ) )
        return point;
    return centre + d * ( radius / length );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTCircle< T >::TPoint
CTCircle< T >::GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TDirection d;
    if ( !TDirection::TryCreate( direction, d ) )
        return centre + TVector( radius, TTraits::Zero() );
    return centre + d * radius;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTCircle< T >::GetArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ( radius * radius ) * TTraits::Pi();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTCircle< T >::GetCircumference( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return ( TTraits::TwoPi() * radius );
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTCircle< T >::GetWireframeLines( GUCEF::vector< CTSegment2D< T > >& lines, UInt32 segmentsPerCircle ) const
{
    const UInt32 n = segmentsPerCircle < 3 ? 3 : segmentsPerCircle;
    GUCEF::vector< TPoint > ring;
    ring.reserve( n );
    for ( UInt32 i=0; i<n; ++i )
    {
        T angle = TTraits::TwoPi() * TTraits::FromInt32( (Int32) i ) / TTraits::FromInt32( (Int32) n );
        ring.push_back( centre + TVector( TTraits::Cos( angle ) * radius, TTraits::Sin( angle ) * radius ) );
    }
    lines.reserve( lines.size() + n );
    for ( UInt32 i=0; i<n; ++i )
        lines.push_back( CTSegment2D< T >( ring[ i ], ring[ ( i + 1 ) % n ] ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTCircle< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTCircle< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTCIRCLE_H ? */
