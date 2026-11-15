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

#ifndef GUCEF_GEOMETRY_CTRAY3D_H
#define GUCEF_GEOMETRY_CTRAY3D_H

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
namespace MATH {
template < typename T > class CTMatrix4D;
}
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A ray in 3D: an origin and a unit direction, extending from the origin along the direction only (section 7.2).
 *
 *  - Kind: floats (Float32, Float64 are exported); needs a square root.
 *  - Layout: origin point and direction, 6 * sizeof( T ), no virtual functions. Nothing is cached: queries that need
 *    the inverse direction compute it.
 *  - The default ray starts at the origin and points forward (-Z).
 *  - Realtime: every member except the text and DataNode members is realtime-safe.
 *  - Text: "Ray3D(origin (x, y, z), direction (x, y, z))"; DataNode: child nodes origin and direction.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTRay3D
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTDirectionVector3D< T > TDirection;

    TPoint origin;
    TDirection direction;

    GUCEF_GEOMETRY_FORCE_INLINE CTRay3D( void ) : origin(), direction() {}

    GUCEF_GEOMETRY_FORCE_INLINE CTRay3D( const TPoint& originPoint, const TDirection& unitDirection )
        : origin( originPoint ), direction( unitDirection )
    {
    }

    /** The ray from 'from' through 'to'; false (out unchanged) when the points coincide or are not finite */
    static bool TryFromPoints( const TPoint& from, const TPoint& to, CTRay3D& out ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        TDirection d;
        if ( !TDirection::TryCreate( to - from, d ) )
            return false;
        out = CTRay3D( from, d );
        return true;
    }

    /** origin + direction * t (t is a distance; negative t is behind the origin) */
    TPoint GetPoint( const T& t ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The distance along the ray of the point nearest to 'point': the projection, 0 when it falls behind the origin */
    T GetClosestParameter( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        T t = direction.Dot( point - origin );
        return t < TTraits::Zero() ? TTraits::Zero() : t;
    }

    /** The point of the ray nearest to 'point' */
    TPoint ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return GetPoint( GetClosestParameter( point ) ); }

    T GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( point - ClosestPoint( point ) ).GetSquaredLength(); }
    T GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( point - ClosestPoint( point ) ).GetLength(); }

    /** True when the point is on the ray within 'tolerance' (a distance) */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& point, const T& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return GetDistance( point ) <= tolerance;
    }

    /** Contains with the default absolute tolerance of the kind */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return Contains( point, TTraits::DefaultAbsoluteTolerance() );
    }

    /** The ray moved by an affine matrix: the origin as a point, the direction as a direction (renormalised, so a non-uniform scale gives the direction of the transformed line); distances along the ray scale with the matrix */
    CTRay3D Transformed( const MATH::CTMatrix4D< T >& matrix ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTRay3D& a, const CTRay3D& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return a.origin == b.origin && a.direction == b.direction; }
    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTRay3D& a, const CTRay3D& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !( a == b ); }

    /** Origin and direction equal within the default tolerances of the kind */
    GUCEF_GEOMETRY_FORCE_INLINE bool Equals( const CTRay3D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return origin.Equals( other.origin ) && direction.Equals( other.direction );
    }

    /** "Ray3D(origin (x, y, z), direction (x, y, z))" */
    CORE::CString ToString( void ) const
    {
        return "Ray3D(origin " + origin.ToString() + ", direction " + direction.ToString() + ")";
    }

    /** Parses the ToString form; false leaves this ray unchanged (the direction must be of unit length) */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TPoint parsedOrigin;
        TDirection parsedDirection;
        if ( !reader.TryLiteral( "Ray3D" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "origin" ) || !reader.TryValue( parsedOrigin ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "direction" ) || !reader.TryValue( parsedDirection ) || !reader.TryChar( ')' ) || !reader.AtEnd() )
            return false;
        origin = parsedOrigin;
        direction = parsedDirection;
        return true;
    }

    /** Writes the child nodes origin and direction into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        return SerializeShapeChild( node, "origin", origin, settings ) && SerializeShapeChild( node, "direction", direction, settings );
    }

    /** Reads the child nodes origin and direction; a missing or invalid one fails and leaves this ray unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        TPoint parsedOrigin;
        TDirection parsedDirection;
        if ( !DeserializeShapeChild( node, "origin", parsedOrigin, settings ) || !DeserializeShapeChild( node, "direction", parsedDirection, settings ) )
            return false;
        origin = parsedOrigin;
        direction = parsedDirection;
        return true;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTRay3D< T >::TPoint
CTRay3D< T >::GetPoint( const T& t ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return origin + direction * t;
}

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTRay3D< T >
CTRay3D< T >::Transformed( const MATH::CTMatrix4D< T >& matrix ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return CTRay3D( matrix.TransformPoint( origin ), matrix.TransformDirection( direction ) );
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTRay3D< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTRay3D< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

#ifndef GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H
#include "gucefGEOMETRY_TransformFunctions.h"
#define GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H
#endif /* GUCEF_GEOMETRY_TRANSFORMFUNCTIONS_H ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTRAY3D_H ? */
