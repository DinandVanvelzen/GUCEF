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

#ifndef GUCEF_GEOMETRY_CTLINE3D_H
#define GUCEF_GEOMETRY_CTLINE3D_H

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
 *  An infinite line in 3D: a point on the line and a unit direction (section 7.2).
 *
 *  - Kind: floats (Float32, Float64 are exported); needs a square root.
 *  - Layout: point and direction, 6 * sizeof( T ), no virtual functions. The default line is the Z axis.
 *  - Realtime: every member except the text and DataNode members is realtime-safe.
 *  - Text: "Line3D(point (x, y, z), direction (x, y, z))"; DataNode: child nodes point and direction.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTLine3D
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTDirectionVector3D< T > TDirection;

    TPoint point;
    TDirection direction;

    GUCEF_GEOMETRY_FORCE_INLINE CTLine3D( void ) : point(), direction() {}

    GUCEF_GEOMETRY_FORCE_INLINE CTLine3D( const TPoint& pointOnLine, const TDirection& unitDirection )
        : point( pointOnLine ), direction( unitDirection )
    {
    }

    /** The line through two points; false (out unchanged) when they coincide or are not finite */
    static bool TryFromPoints( const TPoint& a, const TPoint& b, CTLine3D& out ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        TDirection d;
        if ( !TDirection::TryCreate( b - a, d ) )
            return false;
        out = CTLine3D( a, d );
        return true;
    }

    /** point + direction * t (t is a signed distance) */
    TPoint GetPoint( const T& t ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The signed distance along the line, from 'point', of the point nearest to 'query' */
    GUCEF_GEOMETRY_FORCE_INLINE T GetClosestParameter( const TPoint& query ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return direction.Dot( query - point );
    }

    GUCEF_GEOMETRY_FORCE_INLINE TPoint ClosestPoint( const TPoint& query ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return GetPoint( GetClosestParameter( query ) ); }
    T GetSquaredDistance( const TPoint& query ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( query - ClosestPoint( query ) ).GetSquaredLength(); }
    T GetDistance( const TPoint& query ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( query - ClosestPoint( query ) ).GetLength(); }

    /** True when the point is on the line within 'tolerance' (a distance) */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& query, const T& tolerance ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return GetDistance( query ) <= tolerance;
    }

    /** Contains with the default absolute tolerance of the kind */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& query ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return Contains( query, TTraits::DefaultAbsoluteTolerance() );
    }

    /** The line moved by an affine matrix: the point as a point, the direction as a direction (renormalised) */
    CTLine3D Transformed( const MATH::CTMatrix4D< T >& matrix ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTLine3D& a, const CTLine3D& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return a.point == b.point && a.direction == b.direction; }
    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTLine3D& a, const CTLine3D& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !( a == b ); }

    GUCEF_GEOMETRY_FORCE_INLINE bool Equals( const CTLine3D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return point.Equals( other.point ) && direction.Equals( other.direction );
    }

    /** "Line3D(point (x, y, z), direction (x, y, z))" */
    CORE::CString ToString( void ) const
    {
        return "Line3D(point " + point.ToString() + ", direction " + direction.ToString() + ")";
    }

    /** Parses the ToString form; false leaves this line unchanged */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TPoint parsedPoint;
        TDirection parsedDirection;
        if ( !reader.TryLiteral( "Line3D" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "point" ) || !reader.TryValue( parsedPoint ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "direction" ) || !reader.TryValue( parsedDirection ) || !reader.TryChar( ')' ) || !reader.AtEnd() )
            return false;
        point = parsedPoint;
        direction = parsedDirection;
        return true;
    }

    /** Writes the child nodes point and direction into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        return SerializeShapeChild( node, "point", point, settings ) && SerializeShapeChild( node, "direction", direction, settings );
    }

    /** Reads the child nodes point and direction; a missing or invalid one fails and leaves this line unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        TPoint parsedPoint;
        TDirection parsedDirection;
        if ( !DeserializeShapeChild( node, "point", parsedPoint, settings ) || !DeserializeShapeChild( node, "direction", parsedDirection, settings ) )
            return false;
        point = parsedPoint;
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
GUCEF_GEOMETRY_PRECISE_INLINE typename CTLine3D< T >::TPoint
CTLine3D< T >::GetPoint( const T& t ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return point + direction * t;
}

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTLine3D< T >
CTLine3D< T >::Transformed( const MATH::CTMatrix4D< T >& matrix ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    return CTLine3D( matrix.TransformPoint( point ), matrix.TransformDirection( direction ) );
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTLine3D< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTLine3D< Float64 > )

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

#endif /* GUCEF_GEOMETRY_CTLINE3D_H ? */
