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

#ifndef GUCEF_MATH_CTSPHERICALCOORDINATES_H
#define GUCEF_MATH_CTSPHERICALCOORDINATES_H

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

template < typename T > class CTVector3D;

/*-------------------------------------------------------------------------*/

/**
 *  Spherical coordinates in the terms of a frame (section 4.7.1 of the design), the convention of gucefAUDIO: the
 *  azimuth is 0 along the forward axis of the frame and positive toward its right, the elevation is positive toward its up
 *  axis. The frame coordinates of a position are ( x, y, z ) = ( right, up, forward ); CTOrthonormalFrame3D produces them
 *  from world coordinates.
 *
 *  - Kind: T is a floating point kind.
 *  - Layout: radius, azimuth and elevation, public, no padding, no virtual functions: 3 * sizeof( T ).
 *  - Formulas: azimuth = atan2( x, z ) in ( -pi, pi ], elevation = atan2( y, sqrt( x^2 + z^2 ) ) in [ -pi/2, pi/2 ]. A
 *    source straight ahead has azimuth 0, to the right pi/2, behind pi; straight up elevation is pi/2.
 *  - Degenerate input: the zero position converts to radius 0, azimuth 0 and elevation 0; on the vertical axis (x = z = 0)
 *    the azimuth is 0. A non-finite position converts to non-finite values.
 *  - The mathematical spherical convention of spherical harmonics code (polar angle from the +Z axis, azimuth
 *    counter-clockwise from +X toward +Y in the same x, y, z coordinates) is offered by ToMathematical /
 *    FromMathematical.
 *  - Realtime: every member except the text and DataNode members is realtime-safe.
 *  - Text "(radius, azimuth, elevation)" with the angles in radians; DataNode: attributes radius, azimuth, elevation.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTSphericalCoordinates
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef typename CTMathComponentTraits< T >::TKernels TKernels;

    GUCEF_MATH_STATIC_ASSERT( TTraits::IsFloatingPoint, Spherical_coordinates_require_a_floating_point_kind );

    T radius;
    CTRadian< T > azimuth;
    CTRadian< T > elevation;

    /** Radius 0, both angles 0 */
    GUCEF_MATH_FORCE_INLINE CTSphericalCoordinates( void )
        : radius( TTraits::Zero() ), azimuth(), elevation()
    {
    }

    GUCEF_MATH_FORCE_INLINE CTSphericalCoordinates( const T& radiusValue, const CTRadian< T >& azimuthValue, const CTRadian< T >& elevationValue )
        : radius( radiusValue ), azimuth( azimuthValue ), elevation( elevationValue )
    {
    }

    /** The spherical coordinates of the frame coordinates ( x right, y up, z forward ) */
    static CTSphericalCoordinates FromFrameCoordinates( const T& x, const T& y, const T& z ) GUCEF_MATH_REALTIME_CONTEXT;

    /** FromFrameCoordinates( v.x, v.y, v.z ) */
    static CTSphericalCoordinates FromFrameCoordinates( const CTVector3D< T >& v ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The elevation of the frame coordinates ( x right, y up, z forward ): atan2( y, sqrt( x^2 + z^2 ) ), also for
     *  coordinates whose squares leave the range of the kind; 0 for the zero vector
     */
    static CTRadian< T > GetElevationOfFrameCoordinates( const T& x, const T& y, const T& z ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The frame coordinates ( x right, y up, z forward ): ( r cos( e ) sin( a ), r sin( e ), r cos( e ) cos( a ) ) */
    CTVector3D< T > ToFrameCoordinates( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The mathematical convention of the same position: 'theta' is the polar angle from the +Z axis in [ 0, pi ] and
     *  'phi' the angle of ( x, y ) counter-clockwise from +X in ( -pi, pi ]. Both are 0 for the zero position.
     */
    void ToMathematical( CTRadian< T >& theta, CTRadian< T >& phi ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The spherical coordinates of the position with the mathematical angles theta (from +Z) and phi (from +X toward +Y) */
    static CTSphericalCoordinates FromMathematical( const T& radiusValue, const CTRadian< T >& theta, const CTRadian< T >& phi ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Exact comparison of radius and both angles */
    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTSphericalCoordinates& a, const CTSphericalCoordinates& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.radius == b.radius && a.azimuth == b.azimuth && a.elevation == b.elevation;
    }

    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTSphericalCoordinates& a, const CTSphericalCoordinates& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "(radius, azimuth, elevation)" with the angles in radians */
    CORE::CString ToString( void ) const
    {
        const T components[ 3 ] = { radius, azimuth.GetValue(), elevation.GetValue() };
        return ComponentsToString( components );
    }

    /** Parses the ToString form (white space allowed); false leaves this value unchanged */
    bool FromString( const CORE::CString& text )
    {
        T components[ 3 ];
        if ( !ComponentsFromString( text, components ) )
            return false;
        radius = components[ 0 ];
        azimuth = CTRadian< T >( components[ 1 ] );
        elevation = CTRadian< T >( components[ 2 ] );
        return true;
    }

    /** Writes the attributes radius, azimuth and elevation into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ ) const
    {
        const char* const names[ 3 ] = { "radius", "azimuth", "elevation" };
        const T components[ 3 ] = { radius, azimuth.GetValue(), elevation.GetValue() };
        return SerializeComponents( node, names, components );
    }

    /** Reads the attributes radius, azimuth and elevation; a missing or invalid attribute fails and leaves this value unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ )
    {
        const char* const names[ 3 ] = { "radius", "azimuth", "elevation" };
        T components[ 3 ];
        if ( !DeserializeComponents( node, names, components ) )
            return false;
        radius = components[ 0 ];
        azimuth = CTRadian< T >( components[ 1 ] );
        elevation = CTRadian< T >( components[ 2 ] );
        return true;
    }

    private:

    /** The coordinates as they are, or divided by their largest magnitude when their squares leave the range of the kind (the angles do not change) */
    static void GetAngleOperands( const T ( &coordinates )[ 3 ], T ( &operands )[ 3 ] ) GUCEF_MATH_REALTIME_CONTEXT;
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

#ifndef GUCEF_MATH_CTVECTOR3D_H
#include "gucefMATH_CTVector3D.h"
#define GUCEF_MATH_CTVECTOR3D_H
#endif /* GUCEF_MATH_CTVECTOR3D_H ? */

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
GUCEF_MATH_PRECISE_INLINE void
CTSphericalCoordinates< T >::GetAngleOperands( const T ( &coordinates )[ 3 ], T ( &operands )[ 3 ] ) GUCEF_MATH_REALTIME_CONTEXT
{
    operands[ 0 ] = coordinates[ 0 ];
    operands[ 1 ] = coordinates[ 1 ];
    operands[ 2 ] = coordinates[ 2 ];
    const T squaredLength = coordinates[ 0 ] * coordinates[ 0 ] + coordinates[ 1 ] * coordinates[ 1 ] + coordinates[ 2 ] * coordinates[ 2 ];
    if ( TKernels::IsSafeSquaredLength( squaredLength ) )
        return;
    // scaling by the largest magnitude keeps the angles and cannot overflow or underflow; zero and non-finite input stay as they are
    T scaled[ 3 ];
    if ( TKernels::ScaleToUnitChebyshev( coordinates, scaled ) )
    {
        operands[ 0 ] = scaled[ 0 ];
        operands[ 1 ] = scaled[ 1 ];
        operands[ 2 ] = scaled[ 2 ];
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTRadian< T >
CTSphericalCoordinates< T >::GetElevationOfFrameCoordinates( const T& x, const T& y, const T& z ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( CTVector3D< T >( x, y, z ).IsZero() )
        return CTRadian< T >();
    const T coordinates[ 3 ] = { x, y, z };
    T operands[ 3 ];
    GetAngleOperands( coordinates, operands );
    return Atan2( operands[ 1 ], CTVector2D< T >( operands[ 0 ], operands[ 2 ] ).GetLength() );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTSphericalCoordinates< T >
CTSphericalCoordinates< T >::FromFrameCoordinates( const T& x, const T& y, const T& z ) GUCEF_MATH_REALTIME_CONTEXT
{
    const CTVector3D< T > v( x, y, z );
    if ( v.IsZero() )
        return CTSphericalCoordinates();
    const T coordinates[ 3 ] = { x, y, z };
    T operands[ 3 ];
    GetAngleOperands( coordinates, operands );
    const T horizontal = CTVector2D< T >( operands[ 0 ], operands[ 2 ] ).GetLength();
    return CTSphericalCoordinates( v.GetLength(), horizontal == TTraits::Zero() ? CTRadian< T >() : Atan2( operands[ 0 ], operands[ 2 ] ), Atan2( operands[ 1 ], horizontal ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTSphericalCoordinates< T >
CTSphericalCoordinates< T >::FromFrameCoordinates( const CTVector3D< T >& v ) GUCEF_MATH_REALTIME_CONTEXT
{
    return FromFrameCoordinates( v.x, v.y, v.z );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
CTSphericalCoordinates< T >::ToFrameCoordinates( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const T horizontal = radius * Cos( elevation );
    return CTVector3D< T >( horizontal * Sin( azimuth ), radius * Sin( elevation ), horizontal * Cos( azimuth ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE void
CTSphericalCoordinates< T >::ToMathematical( CTRadian< T >& theta, CTRadian< T >& phi ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const CTVector3D< T > v = ToFrameCoordinates();
    if ( v.IsZero() )
    {
        theta = CTRadian< T >();
        phi = CTRadian< T >();
        return;
    }
    const T coordinates[ 3 ] = { v.x, v.y, v.z };
    T operands[ 3 ];
    GetAngleOperands( coordinates, operands );
    theta = Atan2( CTVector2D< T >( operands[ 0 ], operands[ 1 ] ).GetLength(), operands[ 2 ] );
    phi = operands[ 0 ] == TTraits::Zero() && operands[ 1 ] == TTraits::Zero() ? CTRadian< T >() : Atan2( operands[ 1 ], operands[ 0 ] );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTSphericalCoordinates< T >
CTSphericalCoordinates< T >::FromMathematical( const T& radiusValue, const CTRadian< T >& theta, const CTRadian< T >& phi ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T planar = radiusValue * Sin( theta );
    return FromFrameCoordinates( planar * Cos( phi ), planar * Sin( phi ), radiusValue * Cos( theta ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTSphericalCoordinates< Float32 > TFloat32SphericalCoordinates;
typedef CTSphericalCoordinates< Float64 > TFloat64SphericalCoordinates;

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

#define GUCEF_MATH_CTSPHERICALCOORDINATES_REAL_KIND( prefix, kind ) \
    prefix void CTSphericalCoordinates< kind >::GetAngleOperands( const kind ( & )[ 3 ], kind ( & )[ 3 ] ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTRadian< kind > CTSphericalCoordinates< kind >::GetElevationOfFrameCoordinates( const kind&, const kind&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTSphericalCoordinates< kind > CTSphericalCoordinates< kind >::FromFrameCoordinates( const kind&, const kind&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTSphericalCoordinates< kind > CTSphericalCoordinates< kind >::FromFrameCoordinates( const CTVector3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector3D< kind > CTSphericalCoordinates< kind >::ToFrameCoordinates( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix void CTSphericalCoordinates< kind >::ToMathematical( CTRadian< kind >&, CTRadian< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTSphericalCoordinates< kind > CTSphericalCoordinates< kind >::FromMathematical( const kind&, const CTRadian< kind >&, const CTRadian< kind >& ) GUCEF_MATH_REALTIME_CONTEXT;

/** Every predefined floating point kind */
#define GUCEF_MATH_CTSPHERICALCOORDINATES_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTSPHERICALCOORDINATES_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTSPHERICALCOORDINATES_REAL_KIND( prefix, Float64 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTSPHERICALCOORDINATES_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTSPHERICALCOORDINATES_H ? */
