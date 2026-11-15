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

#ifndef GUCEF_MATH_CTQUATERNION_H
#define GUCEF_MATH_CTQUATERNION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTNUMERICCONVERTER_H
#include "gucefMATH_CTNumericConverter.h"
#define GUCEF_MATH_CTNUMERICCONVERTER_H
#endif /* GUCEF_MATH_CTNUMERICCONVERTER_H ? */

#ifndef GUCEF_MATH_CTRADIAN_H
#include "gucefMATH_CTRadian.h"
#define GUCEF_MATH_CTRADIAN_H
#endif /* GUCEF_MATH_CTRADIAN_H ? */

#ifndef GUCEF_MATH_COMPONENTFUNCTIONS_H
#include "gucefMATH_ComponentFunctions.h"
#define GUCEF_MATH_COMPONENTFUNCTIONS_H
#endif /* GUCEF_MATH_COMPONENTFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTQUATERNIONOPS_H
#include "gucefMATH_CTQuaternionOps.h"
#define GUCEF_MATH_CTQUATERNIONOPS_H
#endif /* GUCEF_MATH_CTQUATERNIONOPS_H ? */

#ifndef GUCEF_MATH_CTVECTOR3D_H
#include "gucefMATH_CTVector3D.h"
#define GUCEF_MATH_CTVECTOR3D_H
#endif /* GUCEF_MATH_CTVECTOR3D_H ? */

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

template < typename T > class CTMatrix3D;
template < typename T > class CTMatrix4D;
template < typename T > class CTEulerAngles;
template < typename T > class CTAxisAngle;
template < typename T > class CTDirectionVector3D;
class CCoordinateSystem;

/*-------------------------------------------------------------------------*/

/**
 *  A rotation quaternion (section 4.5.3 of the design).
 *
 *  - Kind: T needs a square root and transcendental functions (floats and fixed point); the class does not compile for
 *    other kinds.
 *  - Layout: the public components x, y, z, w in this storage order (as glTF, DirectXMath, Unity, Unreal, Jolt and
 *    Eigen), no padding: size 4 * sizeof( T ), alignment that of T.
 *  - Conventions (section 4.1): Hamilton (i j = k), active rotation v' = q v q*; there is no four-scalar constructor,
 *    build with FromXYZW or FromWXYZ. q = qB * qA applies qA first, and ToRotationMatrix3D( qB * qA ) equals
 *    ToRotationMatrix3D( qB ) * ToRotationMatrix3D( qA ). q and -q are the same rotation.
 *  - Arithmetic through CTQuaternionOps (canonical order; fixed point rounds once per dot product).
 *  - Rotation members assume a unit quaternion except where stated; Normalized() restores the length.
 *  - Equality: == exact per component; EqualsRotation compares rotations (q and -q are equal).
 *  - Realtime: every member except the text and DataNode members is realtime-safe.
 *  - Text "(x, y, z, w)" in storage order; DataNode attributes x, y, z, w.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTQuaternion
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef typename CTMathComponentTraits< T >::TKernels TKernels;
    typedef CTQuaternionOps< T > TOps;
    typedef CTVector3D< T > TVector;

    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt && TTraits::HasTranscendentals, Quaternions_require_a_numeric_kind_with_sqrt_and_transcendentals );

    T x;
    T y;
    T z;
    T w;

    /** The identity ( 0, 0, 0, 1 ) */
    GUCEF_MATH_FORCE_INLINE CTQuaternion( void )
        : x( TTraits::Zero() ), y( TTraits::Zero() ), z( TTraits::Zero() ), w( TTraits::One() )
    {
    }

    /** Converts every component with CTNumericConverter (half to even, saturating) */
    template < typename U >
    explicit CTQuaternion( const CTQuaternion< U >& other )
        : x( TTraits::Zero() ), y( TTraits::Zero() ), z( TTraits::Zero() ), w( TTraits::One() )
    {
        TMathStatus status = CTNumericConverter< U, T >::Convert( other.x, x, CORE::ROUNDINGMODE_HALF_EVEN );
        status |= CTNumericConverter< U, T >::Convert( other.y, y, CORE::ROUNDINGMODE_HALF_EVEN );
        status |= CTNumericConverter< U, T >::Convert( other.z, z, CORE::ROUNDINGMODE_HALF_EVEN );
        status |= CTNumericConverter< U, T >::Convert( other.w, w, CORE::ROUNDINGMODE_HALF_EVEN );
        GUCEF_MATH_ASSERT( 0 == ( status & MATHSTATUS_INVALID_OPERAND ) );
        (void) status;
    }

    static GUCEF_MATH_FORCE_INLINE CTQuaternion Identity( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTQuaternion(); }

    /** The components in storage order (glTF, Unity, Unreal, DirectXMath) */
    static GUCEF_MATH_FORCE_INLINE CTQuaternion FromXYZW( const T& xValue, const T& yValue, const T& zValue, const T& wValue ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTQuaternion q;
        q.x = xValue;
        q.y = yValue;
        q.z = zValue;
        q.w = wValue;
        return q;
    }

    /** The components with w first (the constructor order of Ogre and GLM) */
    static GUCEF_MATH_FORCE_INLINE CTQuaternion FromWXYZ( const T& wValue, const T& xValue, const T& yValue, const T& zValue ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return FromXYZW( xValue, yValue, zValue, wValue );
    }

    /** Rotation by 'angle' about 'axis' by the right-hand rule: ( axis sin( angle / 2 ), cos( angle / 2 ) ) */
    static CTQuaternion FromAxisAngle( const CTDirectionVector3D< T >& axis, const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT;

    static CTQuaternion FromAxisAngle( const CTAxisAngle< T >& axisAngle ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The rotation of Euler angles in their order and frame */
    static CTQuaternion FromEuler( const CTEulerAngles< T >& euler ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Shepperd's method; m must be a rotation (orthonormal, determinant 1); the result is normalised */
    static CTQuaternion FromRotationMatrix( const CTMatrix3D< T >& m ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The shortest-arc rotation that maps 'from' onto 'to'. Antiparallel input gives 180 degrees about an axis
     *  perpendicular to 'from' (chosen deterministically from the smallest component of 'from').
     */
    static CTQuaternion FromTo( const CTDirectionVector3D< T >& from, const CTDirectionVector3D< T >& to ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The rotation that turns the forward axis of 'system' onto 'forward' and its up axis toward 'up' (made orthogonal
     *  to 'forward'). LookRotation( Forward(), Up(), GucefDefault() ) is the identity. An up hint parallel to 'forward'
     *  is replaced by a perpendicular.
     */
    static CTQuaternion LookRotation( const CTDirectionVector3D< T >& forward, const TVector& up, const CCoordinateSystem& system ) GUCEF_MATH_REALTIME_CONTEXT;

    /*---------------------------------------------------------------------*/
    /*  Operators                                                           */
    /*---------------------------------------------------------------------*/

    /** a * b: the composition applying b first (Hamilton product) */
    friend GUCEF_MATH_FORCE_INLINE CTQuaternion operator*( const CTQuaternion& a, const CTQuaternion& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return Multiply( a, b );
    }

    GUCEF_MATH_FORCE_INLINE CTQuaternion& operator*=( const CTQuaternion& other ) GUCEF_MATH_REALTIME_CONTEXT { *this = Multiply( *this, other ); return *this; }

    /** q.Rotate( v ) */
    friend GUCEF_MATH_FORCE_INLINE TVector operator*( const CTQuaternion& q, const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return q.Rotate( v );
    }

    /** The negated components: the same rotation */
    GUCEF_MATH_FORCE_INLINE CTQuaternion operator-( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return FromXYZW( ComponentNegate( x ), ComponentNegate( y ), ComponentNegate( z ), ComponentNegate( w ) );
    }

    /** Exact comparison of the components (q and -q compare unequal; see EqualsRotation) */
    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTQuaternion& a, const CTQuaternion& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
    }

    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTQuaternion& a, const CTQuaternion& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /** The Hamilton product a b (b applied first) in the canonical order of CTQuaternionOps */
    static CTQuaternion Multiply( const CTQuaternion& a, const CTQuaternion& b ) GUCEF_MATH_REALTIME_CONTEXT;

    /*---------------------------------------------------------------------*/
    /*  Rotation of vectors                                                 */
    /*---------------------------------------------------------------------*/

    /** v rotated (unit q): t = 2 ( q.xyz x v ), v + w t + q.xyz x t */
    TVector Rotate( const TVector& v ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** d rotated and renormalised (unit q) */
    CTDirectionVector3D< T > Rotate( const CTDirectionVector3D< T >& d ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The forward axis of 'system' rotated by this quaternion */
    CTDirectionVector3D< T > GetForward( const CCoordinateSystem& system ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The up axis of 'system' rotated by this quaternion */
    CTDirectionVector3D< T > GetUp( const CCoordinateSystem& system ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The right axis of 'system' rotated by this quaternion */
    CTDirectionVector3D< T > GetRight( const CCoordinateSystem& system ) const GUCEF_MATH_REALTIME_CONTEXT;

    /*---------------------------------------------------------------------*/
    /*  Algebra                                                             */
    /*---------------------------------------------------------------------*/

    /** ( -x, -y, -z, w ): the inverse rotation of a unit quaternion */
    GUCEF_MATH_FORCE_INLINE CTQuaternion Conjugate( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return FromXYZW( ComponentNegate( x ), ComponentNegate( y ), ComponentNegate( z ), w );
    }

    /** The conjugate divided by the squared length (any non-zero q); the zero quaternion gives zero */
    CTQuaternion Inverse( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** This quaternion divided by its length (Chebyshev pre-scaling as the vectors); zero gives the identity */
    CTQuaternion Normalized( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The 4 component dot product in the canonical order */
    T Dot( const CTQuaternion& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    T GetLength( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The logarithm: ( v / |v| atan2( |v|, w ), ln |q| ) for q = ( v, w ); a unit quaternion gives ( axis angle / 2, 0 ).
     *  The zero vector part gives ( 0, 0, 0, ln |w| ); the zero quaternion gives zero.
     */
    CTQuaternion Log( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The exponential: e^w ( v / |v| sin |v|, cos |v| ); the inverse of Log */
    CTQuaternion Exp( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The angle of the rotation from this to 'other' in [0, pi] (unit quaternions; q and -q give 0) */
    CTRadian< T > GetAngleTo( const CTQuaternion& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Axis and angle in [0, pi] (unit q); the identity gives axis +X and angle 0 */
    void ToAxisAngle( CTDirectionVector3D< T >& axis, CTRadian< T >& angle ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The rotation matrix (see CTMatrix3D::FromQuaternion: q need not be of unit length) */
    CTMatrix3D< T > ToRotationMatrix3D( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    CTMatrix4D< T > ToRotationMatrix4D( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Euler angles of this rotation in the given order and frame (see CTEulerAngles::FromQuaternion) */
    CTEulerAngles< T > ToEuler( TEulerOrder order, TEulerFrame frame ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** True when the rotations differ by at most 'tolerance' (q and -q are equal rotations) */
    bool EqualsRotation( const CTQuaternion& other, const CTRadian< T >& tolerance ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Every component equal within the default tolerances of the kind (q and -q differ; see EqualsRotation) */
    GUCEF_MATH_FORCE_INLINE bool Equals( const CTQuaternion& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return Equals( other, TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() );
    }

    bool Equals( const CTQuaternion& other, const T& absolute, const T& relative ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::EqualsWithin( x, other.x, absolute, relative ) && TTraits::EqualsWithin( y, other.y, absolute, relative ) &&
               TTraits::EqualsWithin( z, other.z, absolute, relative ) && TTraits::EqualsWithin( w, other.w, absolute, relative );
    }

    GUCEF_MATH_FORCE_INLINE bool IsFinite( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::IsFinite( x ) && TTraits::IsFinite( y ) && TTraits::IsFinite( z ) && TTraits::IsFinite( w );
    }

    /** The vector part ( x, y, z ) */
    GUCEF_MATH_FORCE_INLINE TVector GetVectorPart( void ) const GUCEF_MATH_REALTIME_CONTEXT { return TVector( x, y, z ); }

    /*---------------------------------------------------------------------*/
    /*  Interpolation                                                       */
    /*---------------------------------------------------------------------*/

    /**
     *  Spherical interpolation along the shorter arc (b is negated when a . b < 0), renormalised; nearly parallel
     *  inputs use Nlerp. t = 0 gives a exactly and t = 1 gives b (or -b when the shorter arc needed it) exactly.
     */
    static CTQuaternion Slerp( const CTQuaternion& a, const CTQuaternion& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  Spherical interpolation without choosing the shorter arc; exact end points. For opposite inputs (b = -a: the
     *  same rotation, the long way is a full turn about an undefined axis) the arc runs through the quaternion
     *  ( -a.y, a.x, -a.w, a.z ) orthogonal to a, a deterministic choice.
     */
    static CTQuaternion SlerpLongPath( const CTQuaternion& a, const CTQuaternion& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Normalised linear interpolation along the shorter arc */
    static CTQuaternion Nlerp( const CTQuaternion& a, const CTQuaternion& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  Spherical cubic interpolation between q0 and q1 with the inner control points a0, a1 (from SquadTangent):
     *  SlerpLongPath( SlerpLongPath( q0, q1', t ), SlerpLongPath( a0', a1', t ), 2 t ( 1 - t ) ) with q1', a0', a1' the
     *  signs of q1, a0, a1 that put them on the hemisphere of q0 (and a1' on that of q1'), so the segment takes the
     *  short arc whatever signs the keys have. Passes through q0 at t = 0 and returns q1 at t = 1.
     */
    static CTQuaternion Squad( const CTQuaternion& q0, const CTQuaternion& a0, const CTQuaternion& a1, const CTQuaternion& q1, const T& t ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The Squad control point of 'current': current exp( -( log( c* next ) + log( c* previous ) ) / 4 ) */
    static CTQuaternion SquadTangent( const CTQuaternion& previous, const CTQuaternion& current, const CTQuaternion& next ) GUCEF_MATH_REALTIME_CONTEXT;

    /** 'from' rotated toward 'to' by at most maxDelta (>= 0) along the shorter arc; 'to' when closer than that */
    static CTQuaternion RotateTowards( const CTQuaternion& from, const CTQuaternion& to, const CTRadian< T >& maxDelta ) GUCEF_MATH_REALTIME_CONTEXT;

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "(x, y, z, w)" in storage order */
    CORE::CString ToString( void ) const
    {
        const T components[ 4 ] = { x, y, z, w };
        return ComponentsToString( components );
    }

    /** Parses the ToString form; false leaves this quaternion unchanged */
    bool FromString( const CORE::CString& text )
    {
        T components[ 4 ];
        if ( !ComponentsFromString( text, components ) )
            return false;
        *this = FromXYZW( components[ 0 ], components[ 1 ], components[ 2 ], components[ 3 ] );
        return true;
    }

    /** Writes the attributes x, y, z, w */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ ) const
    {
        const char* const names[ 4 ] = { "x", "y", "z", "w" };
        const T components[ 4 ] = { x, y, z, w };
        return SerializeComponents( node, names, components );
    }

    /** Reads the attributes x, y, z, w; a missing or invalid attribute fails and leaves this quaternion unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ )
    {
        const char* const names[ 4 ] = { "x", "y", "z", "w" };
        T components[ 4 ];
        if ( !DeserializeComponents( node, names, components ) )
            return false;
        *this = FromXYZW( components[ 0 ], components[ 1 ], components[ 2 ], components[ 3 ] );
        return true;
    }

    private:

    GUCEF_MATH_FORCE_INLINE void ToArray( T ( &out )[ 4 ] ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        out[ 0 ] = x; out[ 1 ] = y; out[ 2 ] = z; out[ 3 ] = w;
    }

    static GUCEF_MATH_FORCE_INLINE CTQuaternion FromArray( const T ( &in )[ 4 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return FromXYZW( in[ 0 ], in[ 1 ], in[ 2 ], in[ 3 ] );
    }

    /** a wa + b wb, normalised, component-wise as 2 term dot products of the kind */
    static CTQuaternion Blend( const CTQuaternion& a, const T& wa, const CTQuaternion& b, const T& wb ) GUCEF_MATH_REALTIME_CONTEXT
    {
        T out[ 4 ];
        const T ca[ 4 ] = { a.x, a.y, a.z, a.w };
        const T cb[ 4 ] = { b.x, b.y, b.z, b.w };
        for ( UInt32 i=0; i<4; ++i )
        {
            const T l[ 2 ] = { ca[ i ], cb[ i ] };
            const T r[ 2 ] = { wa, wb };
            out[ i ] = TKernels::Dot( l, r );
        }
        return FromArray( out ).Normalized();
    }

    /** The shortest arc from unit f to unit t with f . t >= 0: ( f x h, f . h ) with h the normalised f + t (|f + t| >= sqrt( 2 )) */
    static CTQuaternion HalfArc( const TVector& f, const TVector& t ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const TVector h = ( f + t ).Normalized();
        const TVector axis = f.Cross( h );
        return FromXYZW( axis.x, axis.y, axis.z, f.Dot( h ) ).Normalized();
    }

    /** Slerp without the choice of arc; exact end points */
    static CTQuaternion SlerpArc( const CTQuaternion& a, const CTQuaternion& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( TTraits::Zero() == t )
            return a;
        if ( TTraits::One() == t )
            return b;
        const T cosine = TTraits::Min( TTraits::Max( a.Dot( b ), ComponentNegate( TTraits::One() ) ), TTraits::One() );
        // the angle from the components orthogonal and parallel to a: accurate for small angles
        const CTQuaternion orthogonal = FromXYZW( b.x - a.x * cosine, b.y - a.y * cosine, b.z - a.z * cosine, b.w - a.w * cosine );
        const T sine = orthogonal.GetLength();
        const T theta = TTraits::Atan2( sine, cosine );
        if ( !( TTraits::Sqrt( TTraits::Epsilon() ) < sine ) )
        {
            // nearly parallel: sin( theta ) too small to divide by, the linear blend is accurate
            if ( !TTraits::IsNegative( cosine ) )
                return Blend( a, TTraits::One() - t, b, t );
            // nearly opposite (b ~ -a, the long way round a full turn): the arc through the quaternion p orthogonal to
            // a, chosen deterministically, as a cos( pi t ) + p sin( pi t )
            const CTQuaternion p = FromXYZW( ComponentNegate( a.y ), a.x, ComponentNegate( a.w ), a.z );
            const T angle = TTraits::Pi() * t;
            return Blend( a, TTraits::Cos( angle ), p, TTraits::Sin( angle ) );
        }
        const T wa = TTraits::Sin( ( TTraits::One() - t ) * theta ) / sine;
        const T wb = TTraits::Sin( t * theta ) / sine;
        return Blend( a, wa, b, wb );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** a . b, as a.Dot( b ) */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
Dot( const CTQuaternion< T >& a, const CTQuaternion< T >& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    return a.Dot( b );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

/* The related classes, included after the class (any header of the family can be included first) */

#ifndef GUCEF_MATH_CTDIRECTIONVECTOR3D_H
#include "gucefMATH_CTDirectionVector3D.h"
#define GUCEF_MATH_CTDIRECTIONVECTOR3D_H
#endif /* GUCEF_MATH_CTDIRECTIONVECTOR3D_H ? */

#ifndef GUCEF_MATH_CTMATRIX3D_H
#include "gucefMATH_CTMatrix3D.h"
#define GUCEF_MATH_CTMATRIX3D_H
#endif /* GUCEF_MATH_CTMATRIX3D_H ? */

#ifndef GUCEF_MATH_CTMATRIX4D_H
#include "gucefMATH_CTMatrix4D.h"
#define GUCEF_MATH_CTMATRIX4D_H
#endif /* GUCEF_MATH_CTMATRIX4D_H ? */

#ifndef GUCEF_MATH_CTEULERANGLES_H
#include "gucefMATH_CTEulerAngles.h"
#define GUCEF_MATH_CTEULERANGLES_H
#endif /* GUCEF_MATH_CTEULERANGLES_H ? */

#ifndef GUCEF_MATH_CTAXISANGLE_H
#include "gucefMATH_CTAxisAngle.h"
#define GUCEF_MATH_CTAXISANGLE_H
#endif /* GUCEF_MATH_CTAXISANGLE_H ? */

#ifndef GUCEF_MATH_CCOORDINATESYSTEM_H
#include "gucefMATH_CCoordinateSystem.h"
#define GUCEF_MATH_CCOORDINATESYSTEM_H
#endif /* GUCEF_MATH_CCOORDINATESYSTEM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

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

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTQuaternion< T >::FromAxisAngle( const CTDirectionVector3D< T >& axis, const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T half = angle.GetValue() * TTraits::Half();
    const T s = TTraits::Sin( half );
    const TVector a = axis.AsVector();
    return FromXYZW( a.x * s, a.y * s, a.z * s, TTraits::Cos( half ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTQuaternion< T >::FromAxisAngle( const CTAxisAngle< T >& axisAngle ) GUCEF_MATH_REALTIME_CONTEXT
{
    return FromAxisAngle( axisAngle.axis, axisAngle.angle );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTQuaternion< T >::FromEuler( const CTEulerAngles< T >& euler ) GUCEF_MATH_REALTIME_CONTEXT
{
    return euler.ToQuaternion();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTQuaternion< T >::FromRotationMatrix( const CTMatrix3D< T >& m ) GUCEF_MATH_REALTIME_CONTEXT
{
    return m.ToQuaternion();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTQuaternion< T >::FromTo( const CTDirectionVector3D< T >& from, const CTDirectionVector3D< T >& to ) GUCEF_MATH_REALTIME_CONTEXT
{
    const TVector f = from.AsVector();
    const TVector t = to.AsVector();
    if ( !TTraits::IsNegative( f.Dot( t ) ) )
        return HalfArc( f, t );
    // obtuse: 180 degrees about an axis exactly perpendicular to f (f -> -f), then the acute arc -f -> t. The half-way
    // direction of nearly opposite unit vectors is ill-conditioned (their lengths differ by rounding); this is not.
    TVector axis;
    if ( !f.Cross( t ).TryNormalize( axis, TTraits::Epsilon() * TTraits::FromInt32( 16 ) ) )
        axis = f.GetAnyPerpendicular();
    axis = ( axis - f * f.Dot( axis ) ).Normalized();
    const CTQuaternion halfTurn = FromXYZW( axis.x, axis.y, axis.z, TTraits::Zero() );
    return Multiply( HalfArc( -f, t ), halfTurn ).Normalized();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTQuaternion< T >::LookRotation( const CTDirectionVector3D< T >& forward, const TVector& up, const CCoordinateSystem& system ) GUCEF_MATH_REALTIME_CONTEXT
{
    const TVector f = forward.AsVector();
    TVector u;
    if ( !( up - f * f.Dot( up ) ).TryNormalize( u, TTraits::Zero() ) )
        u = f.GetAnyPerpendicular().Normalized();
    // the system's axes as vectors and the orientation sign of its basis ( right = sign * up x forward )
    const TVector systemRight = system.GetRight< T >();
    const TVector systemUp = system.GetUp< T >();
    const TVector systemForward = system.GetForward< T >();
    const bool positive = !TTraits::IsNegative( systemRight.Dot( systemUp.Cross( systemForward ) ) );
    const TVector r = positive ? u.Cross( f ) : f.Cross( u );
    // R = [ r u f ] [ Rs Us Fs ]^T maps the system's axes onto the target axes
    const CTMatrix3D< T > target = CTMatrix3D< T >::FromColumns( r, u, f );
    const CTMatrix3D< T > source = CTMatrix3D< T >::FromColumns( systemRight, systemUp, systemForward );
    return ( target * source.Transposed() ).ToQuaternion();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTQuaternion< T >::Multiply( const CTQuaternion& a, const CTQuaternion& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    T ca[ 4 ];
    T cb[ 4 ];
    a.ToArray( ca );
    b.ToArray( cb );
    T out[ 4 ];
    TOps::Multiply( ca, cb, out );
    return FromArray( out );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE typename CTQuaternion< T >::TVector
CTQuaternion< T >::Rotate( const TVector& v ) const GUCEF_MATH_REALTIME_CONTEXT
{
    T q[ 4 ];
    ToArray( q );
    const T in[ 3 ] = { v.x, v.y, v.z };
    T out[ 3 ];
    TOps::Rotate( q, in, out );
    return TVector( out[ 0 ], out[ 1 ], out[ 2 ] );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTDirectionVector3D< T >
CTQuaternion< T >::Rotate( const CTDirectionVector3D< T >& d ) const GUCEF_MATH_REALTIME_CONTEXT
{
    CTDirectionVector3D< T > result( d );
    if ( !CTDirectionVector3D< T >::TryCreate( Rotate( d.AsVector() ), result ) )
        return d;
    return result;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTDirectionVector3D< T >
CTQuaternion< T >::GetForward( const CCoordinateSystem& system ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return Rotate( CTDirectionVector3D< T >::FromNormalized( system.GetForward< T >() ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTDirectionVector3D< T >
CTQuaternion< T >::GetUp( const CCoordinateSystem& system ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return Rotate( CTDirectionVector3D< T >::FromNormalized( system.GetUp< T >() ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTDirectionVector3D< T >
CTQuaternion< T >::GetRight( const CCoordinateSystem& system ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return Rotate( CTDirectionVector3D< T >::FromNormalized( system.GetRight< T >() ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTQuaternion< T >::Inverse( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const T squaredLength = Dot( *this );
    if ( !( TTraits::Zero() < squaredLength ) )
        return FromXYZW( TTraits::Zero(), TTraits::Zero(), TTraits::Zero(), TTraits::Zero() );
    const CTQuaternion c = Conjugate();
    return FromXYZW( c.x / squaredLength, c.y / squaredLength, c.z / squaredLength, c.w / squaredLength );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTQuaternion< T >::Normalized( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    T in[ 4 ];
    ToArray( in );
    T out[ 4 ];
    if ( !TOps::Normalize( in, out ) )
        return CTQuaternion();
    return FromArray( out );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTQuaternion< T >::Dot( const CTQuaternion& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    T a[ 4 ];
    T b[ 4 ];
    ToArray( a );
    other.ToArray( b );
    return TOps::Dot( a, b );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTQuaternion< T >::GetLength( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    T a[ 4 ];
    ToArray( a );
    return TKernels::Length( a );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTQuaternion< T >::Log( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const T length = GetLength();
    if ( !( TTraits::Zero() < length ) )
        return FromXYZW( TTraits::Zero(), TTraits::Zero(), TTraits::Zero(), TTraits::Zero() );
    const TVector v = GetVectorPart();
    const T vectorLength = v.GetLength();
    const T logLength = TTraits::Log( length );
    if ( !( TTraits::Zero() < vectorLength ) )
        return FromXYZW( TTraits::Zero(), TTraits::Zero(), TTraits::Zero(), logLength );
    const T factor = TTraits::Atan2( vectorLength, w ) / vectorLength;
    return FromXYZW( v.x * factor, v.y * factor, v.z * factor, logLength );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTQuaternion< T >::Exp( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const TVector v = GetVectorPart();
    const T angle = v.GetLength();
    const T scale = TTraits::Zero() == w ? TTraits::One() : TTraits::Exp( w );
    if ( !( TTraits::Zero() < angle ) )
        return FromXYZW( TTraits::Zero(), TTraits::Zero(), TTraits::Zero(), scale );
    const T factor = scale * TTraits::Sin( angle ) / angle;
    return FromXYZW( v.x * factor, v.y * factor, v.z * factor, scale * TTraits::Cos( angle ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTRadian< T >
CTQuaternion< T >::GetAngleTo( const CTQuaternion& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const CTQuaternion difference = Multiply( Conjugate(), other );
    return CTRadian< T >( TTraits::Two() * TTraits::Atan2( difference.GetVectorPart().GetLength(), TTraits::Abs( difference.w ) ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE void
CTQuaternion< T >::ToAxisAngle( CTDirectionVector3D< T >& axis, CTRadian< T >& angle ) const GUCEF_MATH_REALTIME_CONTEXT
{
    // the representative with w >= 0 has its angle in [0, pi]
    const CTQuaternion q = TTraits::IsNegative( w ) ? -( *this ) : *this;
    const TVector v = q.GetVectorPart();
    const T sine = v.GetLength();
    CTDirectionVector3D< T > direction = CTDirectionVector3D< T >::UnitX();
    if ( !( TTraits::Zero() < sine ) || !CTDirectionVector3D< T >::TryCreate( v, direction ) )
    {
        axis = CTDirectionVector3D< T >::UnitX();
        angle = CTRadian< T >();
        return;
    }
    axis = direction;
    angle = CTRadian< T >( TTraits::Two() * TTraits::Atan2( sine, q.w ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix3D< T >
CTQuaternion< T >::ToRotationMatrix3D( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return CTMatrix3D< T >::FromQuaternion( *this );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix4D< T >
CTQuaternion< T >::ToRotationMatrix4D( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return CTMatrix4D< T >::FromQuaternion( *this );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTEulerAngles< T >
CTQuaternion< T >::ToEuler( TEulerOrder order, TEulerFrame frame ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return CTEulerAngles< T >::FromQuaternion( *this, order, frame );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTQuaternion< T >::EqualsRotation( const CTQuaternion& other, const CTRadian< T >& tolerance ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return !( tolerance < GetAngleTo( other ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTQuaternion< T >::Slerp( const CTQuaternion& a, const CTQuaternion& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return SlerpArc( a, TTraits::IsNegative( a.Dot( b ) ) ? -b : b, t );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTQuaternion< T >::SlerpLongPath( const CTQuaternion& a, const CTQuaternion& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return SlerpArc( a, b, t );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTQuaternion< T >::Nlerp( const CTQuaternion& a, const CTQuaternion& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    const CTQuaternion target = TTraits::IsNegative( a.Dot( b ) ) ? -b : b;
    return Blend( a, TTraits::One() - t, target, t );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTQuaternion< T >::Squad( const CTQuaternion& q0, const CTQuaternion& a0, const CTQuaternion& a1, const CTQuaternion& q1, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    // all four on one hemisphere ( q0's ), so the outer interpolation takes the short arc between the keys and the
    // control points stay with their keys whatever signs the caller's quaternions have
    const CTQuaternion end = TTraits::IsNegative( q0.Dot( q1 ) ) ? -q1 : q1;
    const CTQuaternion control0 = TTraits::IsNegative( q0.Dot( a0 ) ) ? -a0 : a0;
    const CTQuaternion control1 = TTraits::IsNegative( end.Dot( a1 ) ) ? -a1 : a1;
    if ( TTraits::One() == t )
        return q1;
    const T blend = TTraits::Two() * t * ( TTraits::One() - t );
    return SlerpArc( SlerpArc( q0, end, t ), SlerpArc( control0, control1, t ), blend );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTQuaternion< T >::SquadTangent( const CTQuaternion& previous, const CTQuaternion& current, const CTQuaternion& next ) GUCEF_MATH_REALTIME_CONTEXT
{
    const CTQuaternion inverse = current.Conjugate();
    // neighbours on the same hemisphere as 'current', so the logarithms take the short way
    const CTQuaternion p = TTraits::IsNegative( current.Dot( previous ) ) ? -previous : previous;
    const CTQuaternion n = TTraits::IsNegative( current.Dot( next ) ) ? -next : next;
    const CTQuaternion logNext = Multiply( inverse, n ).Log();
    const CTQuaternion logPrevious = Multiply( inverse, p ).Log();
    const T quarter = ComponentNegate( TTraits::Half() * TTraits::Half() );
    const CTQuaternion sum = FromXYZW( ( logNext.x + logPrevious.x ) * quarter, ( logNext.y + logPrevious.y ) * quarter,
                                       ( logNext.z + logPrevious.z ) * quarter, ( logNext.w + logPrevious.w ) * quarter );
    return Multiply( current, sum.Exp() ).Normalized();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTQuaternion< T >::RotateTowards( const CTQuaternion& from, const CTQuaternion& to, const CTRadian< T >& maxDelta ) GUCEF_MATH_REALTIME_CONTEXT
{
    const T angle = from.GetAngleTo( to ).GetValue();
    if ( !( maxDelta.GetValue() < angle ) )
        return to;
    if ( !( TTraits::Zero() < maxDelta.GetValue() ) )
        return from;
    return Slerp( from, to, maxDelta.GetValue() / angle );
}

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
 *  The precision-sensitive members of the predefined kinds, member by member (section 5.10 of the design; see
 *  gucefMATH_CTVector3D.h for the scheme). Quaternions exist only for the kinds with a square root and
 *  transcendentals, so there is one group.
 */

#define GUCEF_MATH_CTQUATERNION_REAL_KIND( prefix, kind ) \
    prefix CTQuaternion< kind > CTQuaternion< kind >::FromAxisAngle( const CTDirectionVector3D< kind >&, const CTRadian< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTQuaternion< kind > CTQuaternion< kind >::FromAxisAngle( const CTAxisAngle< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTQuaternion< kind > CTQuaternion< kind >::FromEuler( const CTEulerAngles< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTQuaternion< kind > CTQuaternion< kind >::FromRotationMatrix( const CTMatrix3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTQuaternion< kind > CTQuaternion< kind >::FromTo( const CTDirectionVector3D< kind >&, const CTDirectionVector3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTQuaternion< kind > CTQuaternion< kind >::LookRotation( const CTDirectionVector3D< kind >&, const CTVector3D< kind >&, const CCoordinateSystem& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTQuaternion< kind > CTQuaternion< kind >::Multiply( const CTQuaternion< kind >&, const CTQuaternion< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector3D< kind > CTQuaternion< kind >::Rotate( const CTVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTDirectionVector3D< kind > CTQuaternion< kind >::Rotate( const CTDirectionVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTDirectionVector3D< kind > CTQuaternion< kind >::GetForward( const CCoordinateSystem& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTDirectionVector3D< kind > CTQuaternion< kind >::GetUp( const CCoordinateSystem& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTDirectionVector3D< kind > CTQuaternion< kind >::GetRight( const CCoordinateSystem& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTQuaternion< kind > CTQuaternion< kind >::Inverse( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTQuaternion< kind > CTQuaternion< kind >::Normalized( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix kind CTQuaternion< kind >::Dot( const CTQuaternion< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix kind CTQuaternion< kind >::GetLength( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTQuaternion< kind > CTQuaternion< kind >::Log( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTQuaternion< kind > CTQuaternion< kind >::Exp( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTRadian< kind > CTQuaternion< kind >::GetAngleTo( const CTQuaternion< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix void CTQuaternion< kind >::ToAxisAngle( CTDirectionVector3D< kind >&, CTRadian< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix3D< kind > CTQuaternion< kind >::ToRotationMatrix3D( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix4D< kind > CTQuaternion< kind >::ToRotationMatrix4D( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTEulerAngles< kind > CTQuaternion< kind >::ToEuler( TEulerOrder, TEulerFrame ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTQuaternion< kind >::EqualsRotation( const CTQuaternion< kind >&, const CTRadian< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTQuaternion< kind > CTQuaternion< kind >::Slerp( const CTQuaternion< kind >&, const CTQuaternion< kind >&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTQuaternion< kind > CTQuaternion< kind >::SlerpLongPath( const CTQuaternion< kind >&, const CTQuaternion< kind >&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTQuaternion< kind > CTQuaternion< kind >::Nlerp( const CTQuaternion< kind >&, const CTQuaternion< kind >&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTQuaternion< kind > CTQuaternion< kind >::Squad( const CTQuaternion< kind >&, const CTQuaternion< kind >&, const CTQuaternion< kind >&, const CTQuaternion< kind >&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTQuaternion< kind > CTQuaternion< kind >::SquadTangent( const CTQuaternion< kind >&, const CTQuaternion< kind >&, const CTQuaternion< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTQuaternion< kind > CTQuaternion< kind >::RotateTowards( const CTQuaternion< kind >&, const CTQuaternion< kind >&, const CTRadian< kind >& ) GUCEF_MATH_REALTIME_CONTEXT;

/** Every predefined kind with a square root and transcendentals */
#define GUCEF_MATH_CTQUATERNION_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTQUATERNION_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTQUATERNION_REAL_KIND( prefix, Float64 ) \
    GUCEF_MATH_CTQUATERNION_REAL_KIND( prefix, TFixed16_16 ) \
    GUCEF_MATH_CTQUATERNION_REAL_KIND( prefix, TFixed32_32 ) \
    GUCEF_MATH_CTQUATERNION_REAL_KIND( prefix, TFixed48_16 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTQUATERNION_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTQUATERNION_H ? */
