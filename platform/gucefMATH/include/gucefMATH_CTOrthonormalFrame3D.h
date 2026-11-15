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

#ifndef GUCEF_MATH_CTORTHONORMALFRAME3D_H
#define GUCEF_MATH_CTORTHONORMALFRAME3D_H

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

#ifndef GUCEF_MATH_CTVECTOR3D_H
#include "gucefMATH_CTVector3D.h"
#define GUCEF_MATH_CTVECTOR3D_H
#endif /* GUCEF_MATH_CTVECTOR3D_H ? */

#ifndef GUCEF_MATH_CTDIRECTIONVECTOR3D_H
#include "gucefMATH_CTDirectionVector3D.h"
#define GUCEF_MATH_CTDIRECTIONVECTOR3D_H
#endif /* GUCEF_MATH_CTDIRECTIONVECTOR3D_H ? */

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

class CCoordinateSystem;
template < typename T > class CTQuaternion;
template < typename T > class CTSphericalCoordinates;

/*-------------------------------------------------------------------------*/

/**
 *  An orthonormal frame in 3D with a right, an up and a forward axis (section 4.7.2 of the design): the frame of a
 *  listener, a camera or an emitter. It converts world vectors into the frame coordinates ( x right, y up, z forward )
 *  that CTSphericalCoordinates and the panners of gucefAUDIO use, and answers the azimuth and elevation of a source.
 *
 *  - Kind: T is a floating point kind.
 *  - Layout: three CTDirectionVector3D, nine values of T, no virtual functions. The axes are unit length and mutually
 *    orthogonal to the tolerance of the kind; every constructor establishes that.
 *  - Handedness: the frame is right-handed when right x up == -forward (the GUCEF default, forward = -Z) and left-handed
 *    when right x up == +forward (forward = +Z); FromForwardUp builds the right-handed form and has an overload with a
 *    handedness, FromQuaternion takes the handedness of the coordinate system.
 *  - Azimuth: 0 along the forward axis, positive toward the right axis, in ( -pi, pi ]; elevation: positive toward the up
 *    axis, in [ -pi/2, pi/2 ]. A vector without a horizontal part has azimuth 0, the zero vector azimuth 0 and elevation 0.
 *  - Degenerate input (forward and up parallel, a zero or non-finite vector): FromForwardUp gives the identity frame of
 *    the default convention and asserts in debug builds; TryFromForwardUp reports it with false.
 *  - Realtime: every member except the text and DataNode members is realtime-safe.
 *  - Text "(rx, ry, rz, ux, uy, uz, fx, fy, fz)" (right, up and forward axis); DataNode attributes rx, ry, rz, ux, uy, uz,
 *    fx, fy, fz; reading checks that the axes are orthonormal.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTOrthonormalFrame3D
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef CTVector3D< T > TVector;
    typedef CTDirectionVector3D< T > TDirection;

    GUCEF_MATH_STATIC_ASSERT( TTraits::IsFloatingPoint, Orthonormal_frames_require_a_floating_point_kind );

    /** The identity frame of the default convention: right +X, up +Y, forward -Z */
    GUCEF_MATH_FORCE_INLINE CTOrthonormalFrame3D( void )
        : m_right( TDirection::Right() ), m_up( TDirection::Up() ), m_forward( TDirection::Forward() )
    {
    }

    /**
     *  The right-handed frame looking along 'forward' with 'up' as the up hint: right = normalise( forward x up ), up' =
     *  right x forward. Neither vector needs to be normalised and they need not be orthogonal; the forward axis is
     *  kept exactly. Degenerate input gives the identity frame and asserts in debug builds (see TryFromForwardUp).
     */
    static CTOrthonormalFrame3D FromForwardUp( const TVector& forward, const TVector& up ) GUCEF_MATH_REALTIME_CONTEXT;

    /** FromForwardUp for a view of the given handedness: left-handed frames have right = normalise( up x forward ) and up' = forward x right */
    static CTOrthonormalFrame3D FromForwardUp( const TVector& forward, const TVector& up, THandedness handedness ) GUCEF_MATH_REALTIME_CONTEXT;

    /** FromForwardUp that reports degenerate input: false and 'out' unchanged for parallel (or zero, or non-finite) vectors */
    static bool TryFromForwardUp( const TVector& forward, const TVector& up, CTOrthonormalFrame3D& out ) GUCEF_MATH_REALTIME_CONTEXT;

    /** TryFromForwardUp for a view of the given handedness */
    static bool TryFromForwardUp( const TVector& forward, const TVector& up, THandedness handedness, CTOrthonormalFrame3D& out ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The frame of an object with the given orientation in 'system': the axes of the system (right, up, forward) rotated by
     *  the unit quaternion. For the default system, FromQuaternion( q ) equals FromForwardUp( q.GetForward(), q.GetUp() ).
     */
    static CTOrthonormalFrame3D FromQuaternion( const CTQuaternion< T >& orientation, const CCoordinateSystem& system ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The unrotated frame of 'system': its right, up and forward axes */
    static CTOrthonormalFrame3D Identity( const CCoordinateSystem& system ) GUCEF_MATH_REALTIME_CONTEXT;

    GUCEF_MATH_FORCE_INLINE const TDirection& GetRight( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_right; }
    GUCEF_MATH_FORCE_INLINE const TDirection& GetUp( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_up; }
    GUCEF_MATH_FORCE_INLINE const TDirection& GetForward( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_forward; }

    /** The frame coordinates of a world vector: ( world . right, world . up, world . forward ) */
    TVector ToFrameCoordinates( const TVector& world ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The world vector of frame coordinates: right x + up y + forward z (the inverse of ToFrameCoordinates) */
    TVector FromFrameCoordinates( const TVector& local ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** atan2( x, z ) of the frame coordinates of 'world': 0 ahead, positive to the right, pi behind; 0 without a horizontal part */
    CTRadian< T > GetAzimuth( const TVector& world ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** atan2( y, sqrt( x^2 + z^2 ) ) of the frame coordinates of 'world': positive upward; 0 for the zero vector */
    CTRadian< T > GetElevation( const TVector& world ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The radius, azimuth and elevation of 'world' in this frame */
    CTSphericalCoordinates< T > ToSpherical( const TVector& world ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Every component of the three axes equal within the default tolerances of the kind */
    bool Equals( const CTOrthonormalFrame3D& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** True when the axes are unit length and mutually orthogonal within the tolerance of the kind (the invariant) */
    static bool IsOrthonormal( const TVector& right, const TVector& up, const TVector& forward ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Exact comparison of every component */
    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTOrthonormalFrame3D& a, const CTOrthonormalFrame3D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.m_right == b.m_right && a.m_up == b.m_up && a.m_forward == b.m_forward;
    }

    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTOrthonormalFrame3D& a, const CTOrthonormalFrame3D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "(rx, ry, rz, ux, uy, uz, fx, fy, fz)" */
    CORE::CString ToString( void ) const
    {
        T components[ 9 ];
        GetComponents( components );
        return ComponentsToString( components );
    }

    /** Parses the ToString form (white space allowed); false (this frame unchanged) for malformed text or axes that are not orthonormal */
    bool FromString( const CORE::CString& text )
    {
        T components[ 9 ];
        if ( !ComponentsFromString( text, components ) )
            return false;
        return SetComponents( components );
    }

    /** Writes the attributes rx, ry, rz, ux, uy, uz, fx, fy and fz into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ ) const
    {
        const char* const names[ 9 ] = { "rx", "ry", "rz", "ux", "uy", "uz", "fx", "fy", "fz" };
        T components[ 9 ];
        GetComponents( components );
        return SerializeComponents( node, names, components );
    }

    /** Reads the attributes of Serialize; a missing or invalid attribute, or axes that are not orthonormal, fail and leave this frame unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ )
    {
        const char* const names[ 9 ] = { "rx", "ry", "rz", "ux", "uy", "uz", "fx", "fy", "fz" };
        T components[ 9 ];
        if ( !DeserializeComponents( node, names, components ) )
            return false;
        return SetComponents( components );
    }

    private:

    GUCEF_MATH_FORCE_INLINE CTOrthonormalFrame3D( const TDirection& right, const TDirection& up, const TDirection& forward )
        : m_right( right ), m_up( up ), m_forward( forward )
    {
    }

    void GetComponents( T ( &components )[ 9 ] ) const
    {
        components[ 0 ] = m_right.GetX(); components[ 1 ] = m_right.GetY(); components[ 2 ] = m_right.GetZ();
        components[ 3 ] = m_up.GetX(); components[ 4 ] = m_up.GetY(); components[ 5 ] = m_up.GetZ();
        components[ 6 ] = m_forward.GetX(); components[ 7 ] = m_forward.GetY(); components[ 8 ] = m_forward.GetZ();
    }

    bool SetComponents( const T ( &components )[ 9 ] )
    {
        const TVector right( components[ 0 ], components[ 1 ], components[ 2 ] );
        const TVector up( components[ 3 ], components[ 4 ], components[ 5 ] );
        const TVector forward( components[ 6 ], components[ 7 ], components[ 8 ] );
        if ( !IsOrthonormal( right, up, forward ) )
            return false;
        m_right = TDirection::FromNormalized( right );
        m_up = TDirection::FromNormalized( up );
        m_forward = TDirection::FromNormalized( forward );
        return true;
    }

    TDirection m_right;
    TDirection m_up;
    TDirection m_forward;
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

#ifndef GUCEF_MATH_CCOORDINATESYSTEM_H
#include "gucefMATH_CCoordinateSystem.h"
#define GUCEF_MATH_CCOORDINATESYSTEM_H
#endif /* GUCEF_MATH_CCOORDINATESYSTEM_H ? */

#ifndef GUCEF_MATH_CTQUATERNION_H
#include "gucefMATH_CTQuaternion.h"
#define GUCEF_MATH_CTQUATERNION_H
#endif /* GUCEF_MATH_CTQUATERNION_H ? */

#ifndef GUCEF_MATH_CTSPHERICALCOORDINATES_H
#include "gucefMATH_CTSphericalCoordinates.h"
#define GUCEF_MATH_CTSPHERICALCOORDINATES_H
#endif /* GUCEF_MATH_CTSPHERICALCOORDINATES_H ? */

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
GUCEF_MATH_PRECISE_INLINE bool
CTOrthonormalFrame3D< T >::TryFromForwardUp( const TVector& forward, const TVector& up, THandedness handedness, CTOrthonormalFrame3D& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    TVector f;
    TVector u;
    if ( !forward.TryNormalize( f, TTraits::Zero() ) || !up.TryNormalize( u, TTraits::Zero() ) )
        return false;

    // the cross product of two unit vectors has the length of the sine of their angle
    const bool rightHanded = HANDEDNESS_LEFT != handedness;
    TVector r;
    if ( !( rightHanded ? f.Cross( u ) : u.Cross( f ) ).TryNormalize( r, TTraits::Epsilon() * TTraits::FromInt32( 16 ) ) )
        return false;

    // a second pass removes the part of r along f that rounding leaves when forward and up are nearly parallel
    if ( !( r - f * f.Dot( r ) ).TryNormalize( r, TTraits::Zero() ) )
        return false;

    const TVector newUp = rightHanded ? r.Cross( f ) : f.Cross( r );
    out = CTOrthonormalFrame3D( TDirection( r ), TDirection( newUp ), TDirection( f ) );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTOrthonormalFrame3D< T >::TryFromForwardUp( const TVector& forward, const TVector& up, CTOrthonormalFrame3D& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    return TryFromForwardUp( forward, up, HANDEDNESS_RIGHT, out );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTOrthonormalFrame3D< T >
CTOrthonormalFrame3D< T >::FromForwardUp( const TVector& forward, const TVector& up, THandedness handedness ) GUCEF_MATH_REALTIME_CONTEXT
{
    CTOrthonormalFrame3D frame;
    bool created = TryFromForwardUp( forward, up, handedness, frame );
    GUCEF_MATH_ASSERT( created );
    (void) created;
    return frame;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTOrthonormalFrame3D< T >
CTOrthonormalFrame3D< T >::FromForwardUp( const TVector& forward, const TVector& up ) GUCEF_MATH_REALTIME_CONTEXT
{
    return FromForwardUp( forward, up, HANDEDNESS_RIGHT );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTOrthonormalFrame3D< T >
CTOrthonormalFrame3D< T >::FromQuaternion( const CTQuaternion< T >& orientation, const CCoordinateSystem& system ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTOrthonormalFrame3D( orientation.GetRight( system ), orientation.GetUp( system ), orientation.GetForward( system ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTOrthonormalFrame3D< T >
CTOrthonormalFrame3D< T >::Identity( const CCoordinateSystem& system ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTOrthonormalFrame3D( TDirection::FromNormalized( system.GetRight< T >() ), TDirection::FromNormalized( system.GetUp< T >() ),
                                 TDirection::FromNormalized( system.GetForward< T >() ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE typename CTOrthonormalFrame3D< T >::TVector
CTOrthonormalFrame3D< T >::ToFrameCoordinates( const TVector& world ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return TVector( m_right.Dot( world ), m_up.Dot( world ), m_forward.Dot( world ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE typename CTOrthonormalFrame3D< T >::TVector
CTOrthonormalFrame3D< T >::FromFrameCoordinates( const TVector& local ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return m_right.AsVector() * local.x + m_up.AsVector() * local.y + m_forward.AsVector() * local.z;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTRadian< T >
CTOrthonormalFrame3D< T >::GetAzimuth( const TVector& world ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const TVector local = ToFrameCoordinates( world );
    if ( local.x == TTraits::Zero() && local.z == TTraits::Zero() )
        return CTRadian< T >();
    return Atan2( local.x, local.z );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTRadian< T >
CTOrthonormalFrame3D< T >::GetElevation( const TVector& world ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const TVector local = ToFrameCoordinates( world );
    return CTSphericalCoordinates< T >::GetElevationOfFrameCoordinates( local.x, local.y, local.z );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTSphericalCoordinates< T >
CTOrthonormalFrame3D< T >::ToSpherical( const TVector& world ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return CTSphericalCoordinates< T >::FromFrameCoordinates( ToFrameCoordinates( world ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTOrthonormalFrame3D< T >::Equals( const CTOrthonormalFrame3D& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return m_right.Equals( other.m_right ) && m_up.Equals( other.m_up ) && m_forward.Equals( other.m_forward );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTOrthonormalFrame3D< T >::IsOrthonormal( const TVector& right, const TVector& up, const TVector& forward ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !TDirection::IsUnitLength( right ) || !TDirection::IsUnitLength( up ) || !TDirection::IsUnitLength( forward ) )
        return false;
    const T tolerance = TDirection::GetUnitLengthTolerance() * TTraits::FromInt32( 4 );
    return TTraits::Abs( right.Dot( up ) ) <= tolerance && TTraits::Abs( right.Dot( forward ) ) <= tolerance && TTraits::Abs( up.Dot( forward ) ) <= tolerance;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTOrthonormalFrame3D< Float32 > TFloat32OrthonormalFrame3D;
typedef CTOrthonormalFrame3D< Float64 > TFloat64OrthonormalFrame3D;

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

#define GUCEF_MATH_CTORTHONORMALFRAME3D_REAL_KIND( prefix, kind ) \
    prefix bool CTOrthonormalFrame3D< kind >::TryFromForwardUp( const CTVector3D< kind >&, const CTVector3D< kind >&, THandedness, CTOrthonormalFrame3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTOrthonormalFrame3D< kind >::TryFromForwardUp( const CTVector3D< kind >&, const CTVector3D< kind >&, CTOrthonormalFrame3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTOrthonormalFrame3D< kind > CTOrthonormalFrame3D< kind >::FromForwardUp( const CTVector3D< kind >&, const CTVector3D< kind >&, THandedness ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTOrthonormalFrame3D< kind > CTOrthonormalFrame3D< kind >::FromForwardUp( const CTVector3D< kind >&, const CTVector3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTOrthonormalFrame3D< kind > CTOrthonormalFrame3D< kind >::FromQuaternion( const CTQuaternion< kind >&, const CCoordinateSystem& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTOrthonormalFrame3D< kind > CTOrthonormalFrame3D< kind >::Identity( const CCoordinateSystem& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector3D< kind > CTOrthonormalFrame3D< kind >::ToFrameCoordinates( const CTVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector3D< kind > CTOrthonormalFrame3D< kind >::FromFrameCoordinates( const CTVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTRadian< kind > CTOrthonormalFrame3D< kind >::GetAzimuth( const CTVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTRadian< kind > CTOrthonormalFrame3D< kind >::GetElevation( const CTVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTSphericalCoordinates< kind > CTOrthonormalFrame3D< kind >::ToSpherical( const CTVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTOrthonormalFrame3D< kind >::Equals( const CTOrthonormalFrame3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTOrthonormalFrame3D< kind >::IsOrthonormal( const CTVector3D< kind >&, const CTVector3D< kind >&, const CTVector3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT;

/** Every predefined floating point kind */
#define GUCEF_MATH_CTORTHONORMALFRAME3D_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTORTHONORMALFRAME3D_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTORTHONORMALFRAME3D_REAL_KIND( prefix, Float64 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTORTHONORMALFRAME3D_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTORTHONORMALFRAME3D_H ? */
