/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

#ifndef GUCEF_MATH_CTPOLARCOORDINATES_H
#define GUCEF_MATH_CTPOLARCOORDINATES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTRADIAN_H
#include "gucefMATH_CTRadian.h"
#define GUCEF_MATH_CTRADIAN_H
#endif /* GUCEF_MATH_CTRADIAN_H ? */

#ifndef GUCEF_MATH_COMPONENTFUNCTIONS_H
#include "gucefMATH_ComponentFunctions.h"
#define GUCEF_MATH_COMPONENTFUNCTIONS_H
#endif /* GUCEF_MATH_COMPONENTFUNCTIONS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T > class CTVector2D;

/*-------------------------------------------------------------------------*/

/**
 *  Polar coordinates in the plane (section 4.7.1 of the design): a radius and an angle counter-clockwise from +X.
 *
 *  - Kind: T is a floating point kind (the conversions need a square root and the inverse tangent).
 *  - Layout: the radius followed by the angle, public, no padding, no virtual functions: 2 * sizeof( T ).
 *  - Conventions: the angle is measured counter-clockwise from +X, so FromCartesian( 0, 1 ) has the angle pi / 2; the
 *    angle of a vector is in ( -pi, pi ]. The radius is not restricted: a negative radius is a valid value that
 *    ToCartesian maps to the opposite point. Equality is exact and does not identify different angles of one point.
 *  - Degenerate input: the zero vector converts to radius 0 and angle 0; a non-finite vector converts to non-finite
 *    values.
 *  - Realtime: every member except the text and DataNode members is realtime-safe.
 *  - Text "(radius, angle)" with the angle in radians; DataNode: attributes radius and angle (radians).
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTPolarCoordinates
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;

    GUCEF_MATH_STATIC_ASSERT( TTraits::IsFloatingPoint, Polar_coordinates_require_a_floating_point_kind );

    T radius;
    CTRadian< T > angle;

    /** Radius 0, angle 0 */
    GUCEF_MATH_FORCE_INLINE CTPolarCoordinates( void )
        : radius( TTraits::Zero() ), angle()
    {
    }

    GUCEF_MATH_FORCE_INLINE CTPolarCoordinates( const T& radiusValue, const CTRadian< T >& angleValue )
        : radius( radiusValue ), angle( angleValue )
    {
    }

    /** The polar coordinates of the vector: the length and atan2( y, x ); the zero vector gives ( 0, 0 ) */
    static CTPolarCoordinates FromCartesian( const CTVector2D< T >& v ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The vector ( radius cos( angle ), radius sin( angle ) ) */
    CTVector2D< T > ToCartesian( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Exact comparison of radius and angle */
    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTPolarCoordinates& a, const CTPolarCoordinates& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.radius == b.radius && a.angle == b.angle;
    }

    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTPolarCoordinates& a, const CTPolarCoordinates& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "(radius, angle)" with the angle in radians */
    CORE::CString ToString( void ) const
    {
        const T components[ 2 ] = { radius, angle.GetValue() };
        return ComponentsToString( components );
    }

    /** Parses the ToString form (white space allowed); false leaves this value unchanged */
    bool FromString( const CORE::CString& text )
    {
        T components[ 2 ];
        if ( !ComponentsFromString( text, components ) )
            return false;
        radius = components[ 0 ];
        angle = CTRadian< T >( components[ 1 ] );
        return true;
    }

    /** Writes the attributes radius and angle into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ ) const
    {
        const char* const names[ 2 ] = { "radius", "angle" };
        const T components[ 2 ] = { radius, angle.GetValue() };
        return SerializeComponents( node, names, components );
    }

    /** Reads the attributes radius and angle; a missing or invalid attribute fails and leaves this value unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ )
    {
        const char* const names[ 2 ] = { "radius", "angle" };
        T components[ 2 ];
        if ( !DeserializeComponents( node, names, components ) )
            return false;
        radius = components[ 0 ];
        angle = CTRadian< T >( components[ 1 ] );
        return true;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

/* The related classes used by the members, included after the class (any header of the family can be included first) */

#ifndef GUCEF_MATH_CTVECTOR2D_H
#include "gucefMATH_CTVector2D.h"
#define GUCEF_MATH_CTVECTOR2D_H
#endif /* GUCEF_MATH_CTVECTOR2D_H ? */

#ifndef GUCEF_MATH_ANGLEFUNCTIONS_H
#include "gucefMATH_AngleFunctions.h"
#define GUCEF_MATH_ANGLEFUNCTIONS_H
#endif /* GUCEF_MATH_ANGLEFUNCTIONS_H ? */

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The precision-sensitive members, out of the class: with GUCEF_MATH_PRECISE_INLINE (empty by default) they are not
 *  inline, so for the predefined kinds the copy compiled in the module (without floating-point contraction) runs.
 */

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTPolarCoordinates< T >
CTPolarCoordinates< T >::FromCartesian( const CTVector2D< T >& v ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( v.IsZero() )
        return CTPolarCoordinates();
    return CTPolarCoordinates( v.GetLength(), Atan2( v.y, v.x ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector2D< T >
CTPolarCoordinates< T >::ToCartesian( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return CTVector2D< T >( radius * Cos( angle ), radius * Sin( angle ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTPolarCoordinates< Float32 > TFloat32PolarCoordinates;
typedef CTPolarCoordinates< Float64 > TFloat64PolarCoordinates;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The precision-sensitive members of the predefined kinds are instantiated in the module and declared extern here
 *  (section 5.10 of the design), member by member. A list macro takes the prefix (GUCEF_MATH_EXTERN_TEMPLATE in
 *  headers, GUCEF_MATH_INSTANTIATE_TEMPLATE in the module source) and the kind (a name without commas).
 */

#define GUCEF_MATH_CTPOLARCOORDINATES_REAL_KIND( prefix, kind ) \
    prefix CTPolarCoordinates< kind > CTPolarCoordinates< kind >::FromCartesian( const CTVector2D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector2D< kind > CTPolarCoordinates< kind >::ToCartesian( void ) const GUCEF_MATH_REALTIME_CONTEXT;

/** Every predefined floating point kind */
#define GUCEF_MATH_CTPOLARCOORDINATES_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTPOLARCOORDINATES_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTPOLARCOORDINATES_REAL_KIND( prefix, Float64 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTPOLARCOORDINATES_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTPOLARCOORDINATES_H ? */
