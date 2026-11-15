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

#ifndef GUCEF_MATH_CTMATRIX4D_H
#define GUCEF_MATH_CTMATRIX4D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTMATRIX3D_H
#include "gucefMATH_CTMatrix3D.h"
#define GUCEF_MATH_CTMATRIX3D_H
#endif /* GUCEF_MATH_CTMATRIX3D_H ? */

#ifndef GUCEF_MATH_CTMATRIX4OPS_H
#include "gucefMATH_CTMatrix4Ops.h"
#define GUCEF_MATH_CTMATRIX4OPS_H
#endif /* GUCEF_MATH_CTMATRIX4OPS_H ? */

#ifndef GUCEF_MATH_CTVECTOR4D_H
#include "gucefMATH_CTVector4D.h"
#define GUCEF_MATH_CTVECTOR4D_H
#endif /* GUCEF_MATH_CTVECTOR4D_H ? */

#ifndef GUCEF_MATH_CTPOINT3D_H
#include "gucefMATH_CTPoint3D.h"
#define GUCEF_MATH_CTPOINT3D_H
#endif /* GUCEF_MATH_CTPOINT3D_H ? */

#ifndef GUCEF_MATH_SPROJECTIONCONVENTION_H
#include "gucefMATH_SProjectionConvention.h"
#define GUCEF_MATH_SPROJECTIONCONVENTION_H
#endif /* GUCEF_MATH_SPROJECTIONCONVENTION_H ? */

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

template < typename T > class CTQuaternion;
template < typename T > class CTDirectionVector3D;

/*-------------------------------------------------------------------------*/

/**
 *  A 4x4 matrix: affine and projective maps of space (section 4.5.1 of the design).
 *
 *  - Kind: T is any numeric kind of CTNumericTraits. Inversion and the orthographic and off-centre projections need
 *    division; rotation builders and Perspective need transcendentals; quaternions, decomposition, LookAt and direction
 *    transforms need a square root.
 *  - Layout: column-major T m_data[ 16 ] (element (r, c) at c * 4 + r; the translation of an affine matrix at 12, 13,
 *    14), private; access through operator()( row, column ), GetColumn, GetRow, GetData (column-major, as GLSL, Vulkan
 *    and glTF expect), ToRowMajor (for row-major APIs such as Ogre). Size 16 * sizeof( T ) (64 for Float32), alignment
 *    that of T, no over-alignment.
 *  - Conventions (section 4.1): column vectors, v' = M v, A then B is B * A; points transform with w = 1, vectors with
 *    w = 0, directions renormalised, normals with the inverse transpose. Projections take a mandatory
 *    SProjectionConvention.
 *  - Products: CTMatrix4Ops (canonical order; SIMD specialisations reproduce it bit for bit).
 *  - Realtime: every member except the text and DataNode members is realtime-safe.
 *  - Text "[[m00, m01, m02, m03], ...]" (row order); DataNode attributes m00 ... m33.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTMatrix4D
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef typename CTMathComponentTraits< T >::TKernels TKernels;
    typedef CTMatrix4Ops< T > TOps;
    typedef CTVector3D< T > TVector3;
    typedef CTVector4D< T > TVector4;
    typedef CTPoint3D< T > TPoint;

    /** The identity */
    GUCEF_MATH_FORCE_INLINE CTMatrix4D( void )
    {
        for ( UInt32 i=0; i<16; ++i )
            m_data[ i ] = ( 0 == i % 5 ) ? TTraits::One() : TTraits::Zero();
    }

    /** Converts every element with CTNumericConverter (half to even, saturating; NaN into an exact kind gives 0) */
    template < typename U >
    explicit CTMatrix4D( const CTMatrix4D< U >& other )
    {
        TMathStatus status = MATHSTATUS_OK;
        for ( UInt32 i=0; i<16; ++i )
        {
            m_data[ i ] = TTraits::Zero();
            status |= CTNumericConverter< U, T >::Convert( other.GetData()[ i ], m_data[ i ], CORE::ROUNDINGMODE_HALF_EVEN );
        }
        GUCEF_MATH_ASSERT( 0 == ( status & MATHSTATUS_INVALID_OPERAND ) );
        (void) status;
    }

    /** The 3x3 matrix in the upper-left block, the rest of the identity */
    explicit CTMatrix4D( const CTMatrix3D< T >& linear )
    {
        *this = CTMatrix4D();
        for ( UInt32 c=0; c<3; ++c )
            for ( UInt32 r=0; r<3; ++r )
                m_data[ c * 4 + r ] = linear( r, c );
    }

    static GUCEF_MATH_FORCE_INLINE CTMatrix4D Identity( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTMatrix4D(); }

    static GUCEF_MATH_FORCE_INLINE CTMatrix4D Zero( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix4D m;
        for ( UInt32 i=0; i<16; ++i )
            m.m_data[ i ] = TTraits::Zero();
        return m;
    }

    static GUCEF_MATH_FORCE_INLINE CTMatrix4D FromColumns( const TVector4& c0, const TVector4& c1, const TVector4& c2, const TVector4& c3 ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix4D m;
        m.SetColumn( 0, c0 );
        m.SetColumn( 1, c1 );
        m.SetColumn( 2, c2 );
        m.SetColumn( 3, c3 );
        return m;
    }

    /** From 16 values in column-major order (the GetData order) */
    static GUCEF_MATH_FORCE_INLINE CTMatrix4D FromColumnMajor( const T* values16 ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix4D m;
        for ( UInt32 i=0; i<16; ++i )
            m.m_data[ i ] = values16[ i ];
        return m;
    }

    /** From 16 values in row-major order (m00, m01, m02, m03, m10, ...) */
    static GUCEF_MATH_FORCE_INLINE CTMatrix4D FromRowMajor( const T* values16 ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix4D m;
        for ( UInt32 r=0; r<4; ++r )
            for ( UInt32 c=0; c<4; ++c )
                m.m_data[ c * 4 + r ] = values16[ r * 4 + c ];
        return m;
    }

    /*---------------------------------------------------------------------*/
    /*  Builders                                                            */
    /*---------------------------------------------------------------------*/

    /** Translation by t: the elements 12, 13, 14 of GetData() */
    static GUCEF_MATH_FORCE_INLINE CTMatrix4D Translation( const TVector3& t ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix4D m;
        m.m_data[ 12 ] = t.x;
        m.m_data[ 13 ] = t.y;
        m.m_data[ 14 ] = t.z;
        return m;
    }

    /** diag( s.x, s.y, s.z, 1 ) */
    static GUCEF_MATH_FORCE_INLINE CTMatrix4D Scale( const TVector3& s ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix4D m;
        m.m_data[ 0 ] = s.x;
        m.m_data[ 5 ] = s.y;
        m.m_data[ 10 ] = s.z;
        return m;
    }

    /** Rotation about +X by the right-hand rule (HasTranscendentals): RotationX( 90 deg ) * UnitY == UnitZ */
    static CTMatrix4D RotationX( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Rotation about +Y by the right-hand rule: RotationY( 90 deg ) * UnitX == -UnitZ */
    static CTMatrix4D RotationY( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Rotation about +Z by the right-hand rule: RotationZ( 90 deg ) * UnitX == UnitY */
    static CTMatrix4D RotationZ( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Rotation about 'axis' by the right-hand rule (see CTMatrix3D::RotationAxisAngle) */
    static CTMatrix4D RotationAxisAngle( const CTDirectionVector3D< T >& axis, const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The rotation of q (HasSqrt; see CTMatrix3D::FromQuaternion) */
    static CTMatrix4D FromQuaternion( const CTQuaternion< T >& q ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Translation * Rotation * Scale: scales first, then rotates, then translates (HasSqrt) */
    static CTMatrix4D FromTRS( const TVector3& t, const CTQuaternion< T >& r, const TVector3& s ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The view matrix of a camera at 'eye' looking at 'target' with 'up' as the up hint (HasSqrt). Right-handed views
     *  look down -Z (the GUCEF default), left-handed views down +Z. A target equal to the eye looks along -Z (right) or
     *  +Z (left); an up hint parallel to the view direction is replaced by a perpendicular.
     */
    static CTMatrix4D LookAt( const TPoint& eye, const TPoint& target, const TVector3& up, THandedness handedness ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  Perspective projection with a vertical field of view (HasTranscendentals). The near plane maps to
     *  convention.GetNearDepth() and the far plane to GetFarDepth() (with infiniteFar the far plane is at infinity and
     *  farZ is ignored); nearZ and farZ are positive distances in front of the camera.
     */
    static CTMatrix4D Perspective( const CTRadian< T >& fovY, const T& aspect, const T& nearZ, const T& farZ, const SProjectionConvention& convention ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Perspective projection of the frustum through the near-plane rectangle [left, right] x [bottom, top] (HasDivision) */
    static CTMatrix4D PerspectiveOffCenter( const T& left, const T& right, const T& bottom, const T& top, const T& nearZ, const T& farZ, const SProjectionConvention& convention ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Orthographic projection of the box [left, right] x [bottom, top] x [near, far] (HasDivision; infiniteFar is ignored) */
    static CTMatrix4D Orthographic( const T& left, const T& right, const T& bottom, const T& top, const T& nearZ, const T& farZ, const SProjectionConvention& convention ) GUCEF_MATH_REALTIME_CONTEXT;

    /*---------------------------------------------------------------------*/
    /*  Access                                                              */
    /*---------------------------------------------------------------------*/

    /** Element (row, column); an index out of range asserts in debug builds and is clamped */
    GUCEF_MATH_FORCE_INLINE T operator()( UInt32 row, UInt32 column ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( row < 4 && column < 4 );
        return m_data[ Index( row, column ) ];
    }

    GUCEF_MATH_FORCE_INLINE T& operator()( UInt32 row, UInt32 column ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( row < 4 && column < 4 );
        return m_data[ Index( row, column ) ];
    }

    GUCEF_MATH_FORCE_INLINE TVector4 GetColumn( UInt32 c ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TVector4( ( *this )( 0, c ), ( *this )( 1, c ), ( *this )( 2, c ), ( *this )( 3, c ) );
    }

    GUCEF_MATH_FORCE_INLINE void SetColumn( UInt32 c, const TVector4& v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        ( *this )( 0, c ) = v.x;
        ( *this )( 1, c ) = v.y;
        ( *this )( 2, c ) = v.z;
        ( *this )( 3, c ) = v.w;
    }

    GUCEF_MATH_FORCE_INLINE TVector4 GetRow( UInt32 r ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TVector4( ( *this )( r, 0 ), ( *this )( r, 1 ), ( *this )( r, 2 ), ( *this )( r, 3 ) );
    }

    /** The 16 elements in column-major order */
    GUCEF_MATH_FORCE_INLINE const T* GetData( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_data; }

    /** Writes the 16 elements in row-major order */
    GUCEF_MATH_FORCE_INLINE void ToRowMajor( T* out16 ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 r=0; r<4; ++r )
            for ( UInt32 c=0; c<4; ++c )
                out16[ r * 4 + c ] = m_data[ c * 4 + r ];
    }

    /** The translation column ( m03, m13, m23 ) */
    GUCEF_MATH_FORCE_INLINE TVector3 GetTranslation( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TVector3( m_data[ 12 ], m_data[ 13 ], m_data[ 14 ] );
    }

    GUCEF_MATH_FORCE_INLINE void SetTranslation( const TVector3& t ) GUCEF_MATH_REALTIME_CONTEXT
    {
        m_data[ 12 ] = t.x;
        m_data[ 13 ] = t.y;
        m_data[ 14 ] = t.z;
    }

    /** The upper-left 3x3 block (the linear part of an affine matrix) */
    GUCEF_MATH_FORCE_INLINE CTMatrix3D< T > GetLinear( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        const T values[ 9 ] = { m_data[ 0 ], m_data[ 1 ], m_data[ 2 ], m_data[ 4 ], m_data[ 5 ], m_data[ 6 ], m_data[ 8 ], m_data[ 9 ], m_data[ 10 ] };
        return CTMatrix3D< T >::FromColumnMajor( values );
    }

    /*---------------------------------------------------------------------*/
    /*  Algebra                                                             */
    /*---------------------------------------------------------------------*/

    friend GUCEF_MATH_FORCE_INLINE CTMatrix4D operator+( const CTMatrix4D& a, const CTMatrix4D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix4D m;
        for ( UInt32 i=0; i<16; ++i )
            m.m_data[ i ] = TKernels::Add( a.m_data[ i ], b.m_data[ i ] );
        return m;
    }

    friend GUCEF_MATH_FORCE_INLINE CTMatrix4D operator-( const CTMatrix4D& a, const CTMatrix4D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix4D m;
        for ( UInt32 i=0; i<16; ++i )
            m.m_data[ i ] = TKernels::Subtract( a.m_data[ i ], b.m_data[ i ] );
        return m;
    }

    GUCEF_MATH_FORCE_INLINE CTMatrix4D operator-( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix4D m;
        for ( UInt32 i=0; i<16; ++i )
            m.m_data[ i ] = TKernels::Negate( m_data[ i ] );
        return m;
    }

    friend GUCEF_MATH_FORCE_INLINE CTMatrix4D operator*( const CTMatrix4D& a, const T& factor ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix4D m;
        for ( UInt32 i=0; i<16; ++i )
            m.m_data[ i ] = TKernels::Multiply( a.m_data[ i ], factor );
        return m;
    }

    friend GUCEF_MATH_FORCE_INLINE CTMatrix4D operator*( const T& factor, const CTMatrix4D& a ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a * factor;
    }

    /** a * b: applies b first */
    friend GUCEF_MATH_FORCE_INLINE CTMatrix4D operator*( const CTMatrix4D& a, const CTMatrix4D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.Multiply( b );
    }

    friend GUCEF_MATH_FORCE_INLINE TVector4 operator*( const CTMatrix4D& m, const TVector4& v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return m.Transform( v );
    }

    /** Affine map of a point (w = 1, no division by w) */
    friend GUCEF_MATH_FORCE_INLINE TPoint operator*( const CTMatrix4D& m, const TPoint& p ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return m.TransformPoint( p );
    }

    /** Map of a displacement (w = 0: the translation does not apply) */
    friend GUCEF_MATH_FORCE_INLINE TVector3 operator*( const CTMatrix4D& m, const TVector3& v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return m.TransformVector( v );
    }

    GUCEF_MATH_FORCE_INLINE CTMatrix4D& operator*=( const CTMatrix4D& other ) GUCEF_MATH_REALTIME_CONTEXT { *this = Multiply( other ); return *this; }

    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTMatrix4D& a, const CTMatrix4D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 i=0; i<16; ++i )
            if ( !( a.m_data[ i ] == b.m_data[ i ] ) )
                return false;
        return true;
    }

    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTMatrix4D& a, const CTMatrix4D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /** this * other (other applied first) in the canonical order of CTMatrix4Ops */
    CTMatrix4D Multiply( const CTMatrix4D& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** this * v in the canonical order of CTMatrix4Ops */
    TVector4 Transform( const TVector4& v ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The affine map of p: rows 0 to 2 applied to ( x, y, z, 1 ); the last row is not used (no division) */
    TPoint TransformPoint( const TPoint& p ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The map of a displacement: rows 0 to 2 applied to ( x, y, z, 0 ) */
    TVector3 TransformVector( const TVector3& v ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The projective map of p: ( x, y, z ) / w of this * ( x, y, z, 1 ) (HasDivision); w == 0 gives the undivided x, y, z */
    TPoint TransformPointProjective( const TPoint& p ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The projective map of p with its homogeneous w (HasDivision): 'w' receives w of this * ( x, y, z, 1 ), so callers
     *  can detect points behind a camera (w < 0 for a perspective projection). False, 'out' unchanged, when w is not
     *  finite or |w| <= epsilon of the kind * the largest of |x|, |y|, |z|, |w| (the point maps to infinity).
     */
    bool TryTransformPointProjective( const TPoint& p, TPoint& out, T& w ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The direction of the linear part applied to d (HasSqrt); d itself when the result is zero or not finite */
    CTDirectionVector3D< T > TransformDirection( const CTDirectionVector3D< T >& d ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The normal n under the inverse transpose of the linear part, renormalised (HasSqrt; see CTMatrix3D) */
    CTDirectionVector3D< T > TransformNormal( const CTDirectionVector3D< T >& n ) const GUCEF_MATH_REALTIME_CONTEXT;

    GUCEF_MATH_FORCE_INLINE CTMatrix4D Transposed( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix4D m;
        TOps::Transpose( m_data, m.m_data );
        return m;
    }

    /** The determinant by the 2x2 minors of the first two and last two rows (Laplace expansion) */
    T GetDeterminant( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The inverse (HasDivision): the adjugate from the 2x2 minors divided by the determinant, of the equilibrated
     *  matrix E = Dr M Dc: each row, then each column, is scaled by the power of two that brings its largest magnitude
     *  into [0.5, 1) (exact; it keeps fixed point precise at small and large scales and for unequal row or column
     *  scales, such as a small linear part next to a large translation). False, 'out' unchanged, for a zero row or
     *  column, or when det( E ) is not finite or |det( E )| <= singularTolerance (>= 0): a relative test, so the
     *  outcome does not depend on the scale of the world or of single rows and columns.
     *  Fixed point: the adjugate and the determinant are exact in 128 bits (CTMathExactMatrixInverse), every element is
     *  rounded once. The equilibration keeps 30 significant bits per value: raw magnitudes above 2^30 (Q16.16 and Q48.16
     *  values of 16384 and more, Q32.32 values of 0.25 and more) are rounded first, a relative input error up to 2^-31
     *  that the condition number amplifies (Q32.32 rotations and translations: errors of 10^4 to 10^6 ULP). Without
     *  such rounding (Q16.16 below 16384) the result is within half a unit in the last place of the exact inverse.
     */
    bool TryInverse( CTMatrix4D& out, const T& singularTolerance ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The inverse (HasDivision); the zero matrix and a debug assertion when the determinant is zero */
    CTMatrix4D Inverse( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The inverse of an affine matrix (IsAffine; HasDivision): the 3x3 inverse of the linear part and the translation
     *  -L^-1 t. The zero matrix and a debug assertion when the linear part is singular.
     */
    CTMatrix4D InverseAffine( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The inverse of a rotation plus translation (no scale): the transposed rotation and -R^T t; no division */
    CTMatrix4D InverseRigid( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  Splits an affine matrix into translation, rotation and scale with FromTRS( t, r, s ) == *this (HasSqrt). A
     *  negative determinant is placed on the x scale (s.x < 0). False, outputs unchanged, for a matrix that is not
     *  affine (perspective), has a zero scale, or has shear (the scaled columns are not orthogonal within
     *  4 * sqrt( epsilon ) of the kind).
     */
    bool Decompose( TVector3& t, CTQuaternion< T >& r, TVector3& s ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** True when the last row is exactly ( 0, 0, 0, 1 ) */
    GUCEF_MATH_FORCE_INLINE bool IsAffine( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::Zero() == m_data[ 3 ] && TTraits::Zero() == m_data[ 7 ] && TTraits::Zero() == m_data[ 11 ] && TTraits::One() == m_data[ 15 ];
    }

    GUCEF_MATH_FORCE_INLINE bool IsIdentity( void ) const GUCEF_MATH_REALTIME_CONTEXT { return *this == CTMatrix4D(); }

    GUCEF_MATH_FORCE_INLINE bool IsFinite( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 i=0; i<16; ++i )
            if ( !TTraits::IsFinite( m_data[ i ] ) )
                return false;
        return true;
    }

    /** True when the three column lengths of the linear part differ by at most tolerance * the largest (HasSqrt) */
    bool HasUniformScale( const T& tolerance ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Every element equal within the default tolerances of the kind */
    GUCEF_MATH_FORCE_INLINE bool Equals( const CTMatrix4D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return Equals( other, TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() );
    }

    /** Every element: |a - b| <= max( absolute, relative * max( |a|, |b| ) ) */
    bool Equals( const CTMatrix4D& other, const T& absolute, const T& relative ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 i=0; i<16; ++i )
            if ( !TTraits::EqualsWithin( m_data[ i ], other.m_data[ i ], absolute, relative ) )
                return false;
        return true;
    }

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "[[m00, m01, m02, m03], [m10, ...], [m20, ...], [m30, ...]]" */
    CORE::CString ToString( void ) const
    {
        T rows[ 16 ];
        ToRowMajor( rows );
        return CTMatrixComponentsCodec< T, 4, 4 >::ToString( rows );
    }

    /** Parses the ToString form; false leaves this matrix unchanged */
    bool FromString( const CORE::CString& text )
    {
        T rows[ 16 ];
        if ( !CTMatrixComponentsCodec< T, 4, 4 >::FromString( text, rows ) )
            return false;
        *this = FromRowMajor( rows );
        return true;
    }

    /** Writes the attributes m00 ... m33 */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ ) const
    {
        T rows[ 16 ];
        ToRowMajor( rows );
        return CTMatrixComponentsCodec< T, 4, 4 >::Serialize( node, rows );
    }

    /** Reads the attributes m00 ... m33; a missing or invalid attribute fails and leaves this matrix unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ )
    {
        T rows[ 16 ];
        if ( !CTMatrixComponentsCodec< T, 4, 4 >::Deserialize( node, rows ) )
            return false;
        *this = FromRowMajor( rows );
        return true;
    }

    private:

    static GUCEF_MATH_FORCE_INLINE UInt32 Index( UInt32 row, UInt32 column ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return ( column < 4 ? column : 3 ) * 4 + ( row < 4 ? row : 3 );
    }

    /** a0 b0 + a1 b1 + a2 b2 with the given signs folded into a, one dot product of the kind */
    static GUCEF_MATH_FORCE_INLINE T Dot3( const T& a0, const T& b0, const T& a1, const T& b1, const T& a2, const T& b2 ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const T a[ 3 ] = { a0, a1, a2 };
        const T b[ 3 ] = { b0, b1, b2 };
        return TKernels::Dot( a, b );
    }

    /** a0 b0 - a1 b1, one dot product of the kind */
    static GUCEF_MATH_FORCE_INLINE T Minor2( const T& a0, const T& b0, const T& a1, const T& b1 ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const T a[ 2 ] = { a0, ComponentNegate( a1 ) };
        const T b[ 2 ] = { b0, b1 };
        return TKernels::Dot( a, b );
    }

    /** The 2x2 minors of rows 0, 1 (s) and rows 2, 3 (c) */
    void GetMinors( T ( &s )[ 6 ], T ( &c )[ 6 ] ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        const CTMatrix4D& m = *this;
        s[ 0 ] = Minor2( m( 0, 0 ), m( 1, 1 ), m( 1, 0 ), m( 0, 1 ) );
        s[ 1 ] = Minor2( m( 0, 0 ), m( 1, 2 ), m( 1, 0 ), m( 0, 2 ) );
        s[ 2 ] = Minor2( m( 0, 0 ), m( 1, 3 ), m( 1, 0 ), m( 0, 3 ) );
        s[ 3 ] = Minor2( m( 0, 1 ), m( 1, 2 ), m( 1, 1 ), m( 0, 2 ) );
        s[ 4 ] = Minor2( m( 0, 1 ), m( 1, 3 ), m( 1, 1 ), m( 0, 3 ) );
        s[ 5 ] = Minor2( m( 0, 2 ), m( 1, 3 ), m( 1, 2 ), m( 0, 3 ) );
        c[ 5 ] = Minor2( m( 2, 2 ), m( 3, 3 ), m( 3, 2 ), m( 2, 3 ) );
        c[ 4 ] = Minor2( m( 2, 1 ), m( 3, 3 ), m( 3, 1 ), m( 2, 3 ) );
        c[ 3 ] = Minor2( m( 2, 1 ), m( 3, 2 ), m( 3, 1 ), m( 2, 2 ) );
        c[ 2 ] = Minor2( m( 2, 0 ), m( 3, 3 ), m( 3, 0 ), m( 2, 3 ) );
        c[ 1 ] = Minor2( m( 2, 0 ), m( 3, 2 ), m( 3, 0 ), m( 2, 2 ) );
        c[ 0 ] = Minor2( m( 2, 0 ), m( 3, 1 ), m( 3, 0 ), m( 2, 1 ) );
    }

    /** The determinant from the minors: s0 c5 - s1 c4 + s2 c3 + s3 c2 - s4 c1 + s5 c0 */
    static GUCEF_MATH_FORCE_INLINE T DeterminantFromMinors( const T ( &s )[ 6 ], const T ( &c )[ 6 ] ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const T a[ 6 ] = { s[ 0 ], ComponentNegate( s[ 1 ] ), s[ 2 ], s[ 3 ], ComponentNegate( s[ 4 ] ), s[ 5 ] };
        const T b[ 6 ] = { c[ 5 ], c[ 4 ], c[ 3 ], c[ 2 ], c[ 1 ], c[ 0 ] };
        return TKernels::Dot( a, b );
    }

    /** Fills the depth row (row 2) and the w row (row 3) of a perspective projection */
    static void SetPerspectiveDepth( CTMatrix4D& m, const T& nearZ, const T& farZ, const SProjectionConvention& convention ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const T viewSign = HANDEDNESS_LEFT == convention.handedness ? TTraits::One() : ComponentNegate( TTraits::One() );
        const T nearDepth = TTraits::FromInt32( convention.GetNearDepth() );
        const T farDepth = TTraits::FromInt32( convention.GetFarDepth() );
        T a;
        T b;
        if ( convention.infiniteFar )
        {
            // depth = a + b / d for the view distance d: d = n gives the near depth, d -> infinity the far depth
            a = farDepth;
            b = ( nearDepth - farDepth ) * nearZ;
        }
        else
        {
            const T range = farZ - nearZ;
            a = ( farDepth * farZ - nearDepth * nearZ ) / range;
            b = ( ( nearDepth - farDepth ) * nearZ ) * ( farZ / range );
        }
        m( 2, 0 ) = TTraits::Zero();
        m( 2, 1 ) = TTraits::Zero();
        m( 2, 2 ) = a * viewSign;
        m( 2, 3 ) = b;
        m( 3, 0 ) = TTraits::Zero();
        m( 3, 1 ) = TTraits::Zero();
        m( 3, 2 ) = viewSign;
        m( 3, 3 ) = TTraits::Zero();
    }

    T m_data[ 16 ];
};

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
GUCEF_MATH_PRECISE_INLINE CTMatrix4D< T >
CTMatrix4D< T >::RotationX( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTMatrix4D( CTMatrix3D< T >::RotationX( angle ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix4D< T >
CTMatrix4D< T >::RotationY( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTMatrix4D( CTMatrix3D< T >::RotationY( angle ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix4D< T >
CTMatrix4D< T >::RotationZ( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTMatrix4D( CTMatrix3D< T >::RotationZ( angle ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix4D< T >
CTMatrix4D< T >::RotationAxisAngle( const CTDirectionVector3D< T >& axis, const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTMatrix4D( CTMatrix3D< T >::RotationAxisAngle( axis, angle ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix4D< T >
CTMatrix4D< T >::FromQuaternion( const CTQuaternion< T >& q ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTMatrix4D( CTMatrix3D< T >::FromQuaternion( q ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix4D< T >
CTMatrix4D< T >::FromTRS( const TVector3& t, const CTQuaternion< T >& r, const TVector3& s ) GUCEF_MATH_REALTIME_CONTEXT
{
    const CTMatrix3D< T > rotation = CTMatrix3D< T >::FromQuaternion( r );
    CTMatrix4D m;
    const T scale[ 3 ] = { s.x, s.y, s.z };
    for ( UInt32 c=0; c<3; ++c )
        for ( UInt32 row=0; row<3; ++row )
            m.m_data[ c * 4 + row ] = TKernels::Multiply( rotation( row, c ), scale[ c ] );
    m.SetTranslation( t );
    return m;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix4D< T >
CTMatrix4D< T >::LookAt( const TPoint& eye, const TPoint& target, const TVector3& up, THandedness handedness ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, LookAt_requires_a_numeric_kind_with_sqrt );
    const bool leftHanded = HANDEDNESS_LEFT == handedness;
    TVector3 forward;
    if ( !( target - eye ).TryNormalize( forward, TTraits::Zero() ) )
        forward = leftHanded ? TVector3::Back() : TVector3::Forward();
    TVector3 right;
    const TVector3 side = leftHanded ? up.Cross( forward ) : forward.Cross( up );
    if ( !side.TryNormalize( right, TTraits::Zero() ) )
        right = forward.GetAnyPerpendicular().Normalized();
    const TVector3 trueUp = leftHanded ? forward.Cross( right ) : right.Cross( forward );
    // the rows of the view rotation: right, up and the view axis (+Z of the view looks backward in right-handed views)
    const TVector3 back = leftHanded ? forward : -forward;
    const TVector3 position = eye.ToPositionVector();
    CTMatrix4D m;
    const TVector3 rows[ 3 ] = { right, trueUp, back };
    for ( UInt32 r=0; r<3; ++r )
    {
        m( r, 0 ) = rows[ r ].x;
        m( r, 1 ) = rows[ r ].y;
        m( r, 2 ) = rows[ r ].z;
        m( r, 3 ) = ComponentNegate( rows[ r ].Dot( position ) );
    }
    return m;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix4D< T >
CTMatrix4D< T >::Perspective( const CTRadian< T >& fovY, const T& aspect, const T& nearZ, const T& farZ, const SProjectionConvention& convention ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasTranscendentals, Perspective_requires_transcendentals );
    const T half = fovY.GetValue() * TTraits::Half();
    const T cotangent = TTraits::Cos( half ) / TTraits::Sin( half );
    CTMatrix4D m = Zero();
    m( 0, 0 ) = cotangent / aspect;
    m( 1, 1 ) = convention.flipY ? ComponentNegate( cotangent ) : cotangent;
    SetPerspectiveDepth( m, nearZ, farZ, convention );
    return m;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix4D< T >
CTMatrix4D< T >::PerspectiveOffCenter( const T& left, const T& right, const T& bottom, const T& top, const T& nearZ, const T& farZ, const SProjectionConvention& convention ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Projection_requires_a_numeric_kind_with_division );
    const T width = right - left;
    const T height = top - bottom;
    const T twoNear = TTraits::Two() * nearZ;
    // x_clip = 2 n x / w - ( r + l ) / w * d with the view distance d = viewSign * z
    const bool leftHanded = HANDEDNESS_LEFT == convention.handedness;
    const T xOffset = ( right + left ) / width;
    const T yOffset = ( top + bottom ) / height;
    CTMatrix4D m = Zero();
    m( 0, 0 ) = twoNear / width;
    m( 0, 2 ) = leftHanded ? ComponentNegate( xOffset ) : xOffset;
    m( 1, 1 ) = twoNear / height;
    m( 1, 2 ) = leftHanded ? ComponentNegate( yOffset ) : yOffset;
    if ( convention.flipY )
    {
        m( 1, 1 ) = ComponentNegate( m( 1, 1 ) );
        m( 1, 2 ) = ComponentNegate( m( 1, 2 ) );
    }
    SetPerspectiveDepth( m, nearZ, farZ, convention );
    return m;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix4D< T >
CTMatrix4D< T >::Orthographic( const T& left, const T& right, const T& bottom, const T& top, const T& nearZ, const T& farZ, const SProjectionConvention& convention ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Projection_requires_a_numeric_kind_with_division );
    const T width = right - left;
    const T height = top - bottom;
    const T depth = farZ - nearZ;
    const T viewSign = HANDEDNESS_LEFT == convention.handedness ? TTraits::One() : ComponentNegate( TTraits::One() );
    const T nearDepth = TTraits::FromInt32( convention.GetNearDepth() );
    const T farDepth = TTraits::FromInt32( convention.GetFarDepth() );
    const T depthScale = ( farDepth - nearDepth ) / depth;
    CTMatrix4D m;
    m( 0, 0 ) = TTraits::Two() / width;
    m( 0, 3 ) = ComponentNegate( ( right + left ) / width );
    m( 1, 1 ) = TTraits::Two() / height;
    m( 1, 3 ) = ComponentNegate( ( top + bottom ) / height );
    if ( convention.flipY )
    {
        m( 1, 1 ) = ComponentNegate( m( 1, 1 ) );
        m( 1, 3 ) = ComponentNegate( m( 1, 3 ) );
    }
    // depth = near depth + ( far depth - near depth ) ( d - n ) / ( f - n ) for the view distance d = viewSign * z
    m( 2, 2 ) = depthScale * viewSign;
    m( 2, 3 ) = nearDepth - depthScale * nearZ;
    return m;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix4D< T >
CTMatrix4D< T >::Multiply( const CTMatrix4D& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    CTMatrix4D m;
    TOps::Multiply( m_data, other.m_data, m.m_data );
    return m;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE typename CTMatrix4D< T >::TVector4
CTMatrix4D< T >::Transform( const TVector4& v ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const T in[ 4 ] = { v.x, v.y, v.z, v.w };
    T out[ 4 ];
    TOps::Transform( m_data, in, out );
    return TVector4( out[ 0 ], out[ 1 ], out[ 2 ], out[ 3 ] );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE typename CTMatrix4D< T >::TPoint
CTMatrix4D< T >::TransformPoint( const TPoint& p ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const T in[ 4 ] = { p.x, p.y, p.z, TTraits::One() };
    T out[ 4 ];
    TOps::Transform( m_data, in, out );
    return TPoint( out[ 0 ], out[ 1 ], out[ 2 ] );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE typename CTMatrix4D< T >::TVector3
CTMatrix4D< T >::TransformVector( const TVector3& v ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const T in[ 4 ] = { v.x, v.y, v.z, TTraits::Zero() };
    T out[ 4 ];
    TOps::Transform( m_data, in, out );
    return TVector3( out[ 0 ], out[ 1 ], out[ 2 ] );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE typename CTMatrix4D< T >::TPoint
CTMatrix4D< T >::TransformPointProjective( const TPoint& p ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Projective_transform_requires_a_numeric_kind_with_division );
    const T in[ 4 ] = { p.x, p.y, p.z, TTraits::One() };
    T out[ 4 ];
    TOps::Transform( m_data, in, out );
    if ( TTraits::Zero() == out[ 3 ] )
        return TPoint( out[ 0 ], out[ 1 ], out[ 2 ] );
    return TPoint( out[ 0 ] / out[ 3 ], out[ 1 ] / out[ 3 ], out[ 2 ] / out[ 3 ] );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTMatrix4D< T >::TryTransformPointProjective( const TPoint& p, TPoint& out, T& w ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Projective_transform_requires_a_numeric_kind_with_division );
    const T in[ 4 ] = { p.x, p.y, p.z, TTraits::One() };
    T h[ 4 ];
    TOps::Transform( m_data, in, h );
    w = h[ 3 ];
    T largest = TTraits::Abs( h[ 3 ] );
    for ( UInt32 i=0; i<3; ++i )
        largest = TTraits::Max( largest, TTraits::Abs( h[ i ] ) );
    if ( !TTraits::IsFinite( h[ 3 ] ) || !( TTraits::Epsilon() * largest < TTraits::Abs( h[ 3 ] ) ) )
        return false;
    out = TPoint( h[ 0 ] / h[ 3 ], h[ 1 ] / h[ 3 ], h[ 2 ] / h[ 3 ] );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTDirectionVector3D< T >
CTMatrix4D< T >::TransformDirection( const CTDirectionVector3D< T >& d ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return GetLinear().TransformDirection( d );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTDirectionVector3D< T >
CTMatrix4D< T >::TransformNormal( const CTDirectionVector3D< T >& n ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return GetLinear().TransformNormal( n );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTMatrix4D< T >::GetDeterminant( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    T s[ 6 ];
    T c[ 6 ];
    GetMinors( s, c );
    return DeterminantFromMinors( s, c );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTMatrix4D< T >::TryInverse( CTMatrix4D& out, const T& singularTolerance ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Inverse_requires_a_numeric_kind_with_division );
    typedef CTMathExactMatrixInverse< T, CTMathComponentKind< T >::Value > TExactInverse;
    if ( TExactInverse::IsAvailable )
    {
        // fixed point: exact 128 bit adjugate and determinant, one rounding per element
        T values[ 16 ];
        if ( !TExactInverse::TryInverse( m_data, 4, singularTolerance, values ) )
            return false;
        out = FromColumnMajor( values );
        return true;
    }
    typedef CTMathBinaryScaling< T, CTMathComponentKind< T >::Value > TScaling;
    // rows equilibrated by powers of two
    Int32 exponent[ 4 ];
    CTMatrix4D a;
    for ( UInt32 r=0; r<4; ++r )
    {
        T largest = TTraits::Abs( m_data[ r ] );
        for ( UInt32 col=1; col<4; ++col )
            largest = TTraits::Max( largest, TTraits::Abs( m_data[ col * 4 + r ] ) );
        if ( !( TTraits::Zero() < largest ) )
            return false;
        exponent[ r ] = TScaling::NormalizingExponent( largest );
        for ( UInt32 col=0; col<4; ++col )
            a.m_data[ col * 4 + r ] = TScaling::Scale( m_data[ col * 4 + r ], exponent[ r ] );
    }
    // then the columns: E = Dr M Dc, M^-1 = Dc E^-1 Dr
    Int32 columnExponent[ 4 ];
    for ( UInt32 col=0; col<4; ++col )
    {
        T largest = TTraits::Abs( a.m_data[ col * 4 ] );
        for ( UInt32 r=1; r<4; ++r )
            largest = TTraits::Max( largest, TTraits::Abs( a.m_data[ col * 4 + r ] ) );
        if ( !( TTraits::Zero() < largest ) )
            return false;
        columnExponent[ col ] = TScaling::NormalizingExponent( largest );
        for ( UInt32 r=0; r<4; ++r )
            a.m_data[ col * 4 + r ] = TScaling::Scale( a.m_data[ col * 4 + r ], columnExponent[ col ] );
    }
    T s[ 6 ];
    T c[ 6 ];
    a.GetMinors( s, c );
    const T determinant = DeterminantFromMinors( s, c );
    if ( !TTraits::IsFinite( determinant ) || !( singularTolerance < TTraits::Abs( determinant ) ) )
        return false;

    T b[ 4 ][ 4 ];    // b[ row ][ column ] of the adjugate
    b[ 0 ][ 0 ] = Dot3( a( 1, 1 ), c[ 5 ], ComponentNegate( a( 1, 2 ) ), c[ 4 ], a( 1, 3 ), c[ 3 ] );
    b[ 0 ][ 1 ] = Dot3( ComponentNegate( a( 0, 1 ) ), c[ 5 ], a( 0, 2 ), c[ 4 ], ComponentNegate( a( 0, 3 ) ), c[ 3 ] );
    b[ 0 ][ 2 ] = Dot3( a( 3, 1 ), s[ 5 ], ComponentNegate( a( 3, 2 ) ), s[ 4 ], a( 3, 3 ), s[ 3 ] );
    b[ 0 ][ 3 ] = Dot3( ComponentNegate( a( 2, 1 ) ), s[ 5 ], a( 2, 2 ), s[ 4 ], ComponentNegate( a( 2, 3 ) ), s[ 3 ] );
    b[ 1 ][ 0 ] = Dot3( ComponentNegate( a( 1, 0 ) ), c[ 5 ], a( 1, 2 ), c[ 2 ], ComponentNegate( a( 1, 3 ) ), c[ 1 ] );
    b[ 1 ][ 1 ] = Dot3( a( 0, 0 ), c[ 5 ], ComponentNegate( a( 0, 2 ) ), c[ 2 ], a( 0, 3 ), c[ 1 ] );
    b[ 1 ][ 2 ] = Dot3( ComponentNegate( a( 3, 0 ) ), s[ 5 ], a( 3, 2 ), s[ 2 ], ComponentNegate( a( 3, 3 ) ), s[ 1 ] );
    b[ 1 ][ 3 ] = Dot3( a( 2, 0 ), s[ 5 ], ComponentNegate( a( 2, 2 ) ), s[ 2 ], a( 2, 3 ), s[ 1 ] );
    b[ 2 ][ 0 ] = Dot3( a( 1, 0 ), c[ 4 ], ComponentNegate( a( 1, 1 ) ), c[ 2 ], a( 1, 3 ), c[ 0 ] );
    b[ 2 ][ 1 ] = Dot3( ComponentNegate( a( 0, 0 ) ), c[ 4 ], a( 0, 1 ), c[ 2 ], ComponentNegate( a( 0, 3 ) ), c[ 0 ] );
    b[ 2 ][ 2 ] = Dot3( a( 3, 0 ), s[ 4 ], ComponentNegate( a( 3, 1 ) ), s[ 2 ], a( 3, 3 ), s[ 0 ] );
    b[ 2 ][ 3 ] = Dot3( ComponentNegate( a( 2, 0 ) ), s[ 4 ], a( 2, 1 ), s[ 2 ], ComponentNegate( a( 2, 3 ) ), s[ 0 ] );
    b[ 3 ][ 0 ] = Dot3( ComponentNegate( a( 1, 0 ) ), c[ 3 ], a( 1, 1 ), c[ 1 ], ComponentNegate( a( 1, 2 ) ), c[ 0 ] );
    b[ 3 ][ 1 ] = Dot3( a( 0, 0 ), c[ 3 ], ComponentNegate( a( 0, 1 ) ), c[ 1 ], a( 0, 2 ), c[ 0 ] );
    b[ 3 ][ 2 ] = Dot3( ComponentNegate( a( 3, 0 ) ), s[ 3 ], a( 3, 1 ), s[ 1 ], ComponentNegate( a( 3, 2 ) ), s[ 0 ] );
    b[ 3 ][ 3 ] = Dot3( a( 2, 0 ), s[ 3 ], ComponentNegate( a( 2, 1 ) ), s[ 1 ], a( 2, 2 ), s[ 0 ] );

    CTMatrix4D m;
    for ( UInt32 r=0; r<4; ++r )
        for ( UInt32 col=0; col<4; ++col )
            m( r, col ) = TScaling::Scale( b[ r ][ col ] / determinant, columnExponent[ r ] + exponent[ col ] );
    out = m;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix4D< T >
CTMatrix4D< T >::Inverse( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    CTMatrix4D m = Zero();
    bool invertible = TryInverse( m, TTraits::Zero() );
    GUCEF_MATH_ASSERT( invertible );
    (void) invertible;
    return m;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix4D< T >
CTMatrix4D< T >::InverseAffine( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_ASSERT( IsAffine() );
    CTMatrix3D< T > linearInverse = CTMatrix3D< T >::Zero();
    if ( !GetLinear().TryInverse( linearInverse, TTraits::Zero() ) )
    {
        GUCEF_MATH_ASSERT( false );
        return Zero();
    }
    CTMatrix4D m( linearInverse );
    m.SetTranslation( -linearInverse.Transform( GetTranslation() ) );
    return m;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix4D< T >
CTMatrix4D< T >::InverseRigid( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const CTMatrix3D< T > rotationInverse = GetLinear().Transposed();
    CTMatrix4D m( rotationInverse );
    m.SetTranslation( -rotationInverse.Transform( GetTranslation() ) );
    return m;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTMatrix4D< T >::HasUniformScale( const T& tolerance ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, Scale_queries_require_a_numeric_kind_with_sqrt );
    const CTMatrix3D< T > linear = GetLinear();
    const T l0 = linear.GetColumn( 0 ).GetLength();
    const T l1 = linear.GetColumn( 1 ).GetLength();
    const T l2 = linear.GetColumn( 2 ).GetLength();
    const T largest = TTraits::Max( TTraits::Max( l0, l1 ), l2 );
    const T allowed = tolerance * largest;
    return !( allowed < largest - l0 ) && !( allowed < largest - l1 ) && !( allowed < largest - l2 );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTMatrix4D< T >::Decompose( TVector3& t, CTQuaternion< T >& r, TVector3& s ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, Decompose_requires_a_numeric_kind_with_sqrt );
    if ( !IsAffine() || !IsFinite() )
        return false;
    const CTMatrix3D< T > linear = GetLinear();
    TVector3 columns[ 3 ] = { linear.GetColumn( 0 ), linear.GetColumn( 1 ), linear.GetColumn( 2 ) };
    T scale[ 3 ];
    TVector3 units[ 3 ];
    for ( UInt32 i=0; i<3; ++i )
    {
        scale[ i ] = columns[ i ].GetLength();
        if ( !( TTraits::Zero() < scale[ i ] ) )
            return false;
        units[ i ] = columns[ i ] / scale[ i ];
    }
    if ( TTraits::IsNegative( linear.GetDeterminant() ) )
    {
        scale[ 0 ] = ComponentNegate( scale[ 0 ] );
        units[ 0 ] = -units[ 0 ];
    }
    const T tolerance = TTraits::Sqrt( TTraits::Epsilon() ) * TTraits::FromInt32( 4 );
    if ( tolerance < TTraits::Abs( units[ 0 ].Dot( units[ 1 ] ) ) ||
         tolerance < TTraits::Abs( units[ 0 ].Dot( units[ 2 ] ) ) ||
         tolerance < TTraits::Abs( units[ 1 ].Dot( units[ 2 ] ) ) )
        return false;
    r = CTMatrix3D< T >::FromColumns( units[ 0 ], units[ 1 ], units[ 2 ] ).ToQuaternion();
    s = TVector3( scale[ 0 ], scale[ 1 ], scale[ 2 ] );
    t = GetTranslation();
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

/* The related classes used by the quaternion and direction members, included after the class (any header first) */

#ifndef GUCEF_MATH_CTQUATERNION_H
#include "gucefMATH_CTQuaternion.h"
#define GUCEF_MATH_CTQUATERNION_H
#endif /* GUCEF_MATH_CTQUATERNION_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The precision-sensitive members of the predefined kinds, member by member (section 5.10 of the design; see
 *  gucefMATH_CTVector3D.h for the scheme): ALL_KINDS, DIVISION, SQRT and TRANSCENDENTAL groups.
 */

#define GUCEF_MATH_CTMATRIX4D_ALL_KINDS_MEMBERS( prefix, kind ) \
    prefix CTMatrix4D< kind > CTMatrix4D< kind >::Multiply( const CTMatrix4D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector4D< kind > CTMatrix4D< kind >::Transform( const CTVector4D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTPoint3D< kind > CTMatrix4D< kind >::TransformPoint( const CTPoint3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector3D< kind > CTMatrix4D< kind >::TransformVector( const CTVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix kind CTMatrix4D< kind >::GetDeterminant( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix4D< kind > CTMatrix4D< kind >::InverseRigid( void ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTMATRIX4D_DIVISION_MEMBERS( prefix, kind ) \
    prefix bool CTMatrix4D< kind >::TryInverse( CTMatrix4D< kind >&, const kind& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix4D< kind > CTMatrix4D< kind >::Inverse( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix4D< kind > CTMatrix4D< kind >::InverseAffine( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTPoint3D< kind > CTMatrix4D< kind >::TransformPointProjective( const CTPoint3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTMatrix4D< kind >::TryTransformPointProjective( const CTPoint3D< kind >&, CTPoint3D< kind >&, kind& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix4D< kind > CTMatrix4D< kind >::PerspectiveOffCenter( const kind&, const kind&, const kind&, const kind&, const kind&, const kind&, const SProjectionConvention& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix4D< kind > CTMatrix4D< kind >::Orthographic( const kind&, const kind&, const kind&, const kind&, const kind&, const kind&, const SProjectionConvention& ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTMATRIX4D_SQRT_MEMBERS( prefix, kind ) \
    prefix CTMatrix4D< kind > CTMatrix4D< kind >::FromQuaternion( const CTQuaternion< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix4D< kind > CTMatrix4D< kind >::FromTRS( const CTVector3D< kind >&, const CTQuaternion< kind >&, const CTVector3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix4D< kind > CTMatrix4D< kind >::LookAt( const CTPoint3D< kind >&, const CTPoint3D< kind >&, const CTVector3D< kind >&, THandedness ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTDirectionVector3D< kind > CTMatrix4D< kind >::TransformDirection( const CTDirectionVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTDirectionVector3D< kind > CTMatrix4D< kind >::TransformNormal( const CTDirectionVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTMatrix4D< kind >::HasUniformScale( const kind& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTMatrix4D< kind >::Decompose( CTVector3D< kind >&, CTQuaternion< kind >&, CTVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTMATRIX4D_TRANSCENDENTAL_MEMBERS( prefix, kind ) \
    prefix CTMatrix4D< kind > CTMatrix4D< kind >::RotationX( const CTRadian< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix4D< kind > CTMatrix4D< kind >::RotationY( const CTRadian< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix4D< kind > CTMatrix4D< kind >::RotationZ( const CTRadian< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix4D< kind > CTMatrix4D< kind >::RotationAxisAngle( const CTDirectionVector3D< kind >&, const CTRadian< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix4D< kind > CTMatrix4D< kind >::Perspective( const CTRadian< kind >&, const kind&, const kind&, const kind&, const SProjectionConvention& ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTMATRIX4D_REAL_KIND( prefix, kind ) GUCEF_MATH_CTMATRIX4D_ALL_KINDS_MEMBERS( prefix, kind ) GUCEF_MATH_CTMATRIX4D_DIVISION_MEMBERS( prefix, kind ) GUCEF_MATH_CTMATRIX4D_SQRT_MEMBERS( prefix, kind ) GUCEF_MATH_CTMATRIX4D_TRANSCENDENTAL_MEMBERS( prefix, kind )
#define GUCEF_MATH_CTMATRIX4D_INTEGER_KIND( prefix, kind ) GUCEF_MATH_CTMATRIX4D_ALL_KINDS_MEMBERS( prefix, kind )
#define GUCEF_MATH_CTMATRIX4D_EXACT_KIND( prefix, kind ) GUCEF_MATH_CTMATRIX4D_ALL_KINDS_MEMBERS( prefix, kind ) GUCEF_MATH_CTMATRIX4D_DIVISION_MEMBERS( prefix, kind )

/** Every predefined kind except the fractions (those are in gucefMATH_fractiontypes.h, which includes CBigInteger) */
#define GUCEF_MATH_CTMATRIX4D_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTMATRIX4D_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTMATRIX4D_REAL_KIND( prefix, Float64 ) \
    GUCEF_MATH_CTMATRIX4D_REAL_KIND( prefix, TFixed16_16 ) \
    GUCEF_MATH_CTMATRIX4D_REAL_KIND( prefix, TFixed32_32 ) \
    GUCEF_MATH_CTMATRIX4D_REAL_KIND( prefix, TFixed48_16 ) \
    GUCEF_MATH_CTMATRIX4D_INTEGER_KIND( prefix, Int8 ) \
    GUCEF_MATH_CTMATRIX4D_INTEGER_KIND( prefix, UInt8 ) \
    GUCEF_MATH_CTMATRIX4D_INTEGER_KIND( prefix, Int16 ) \
    GUCEF_MATH_CTMATRIX4D_INTEGER_KIND( prefix, UInt16 ) \
    GUCEF_MATH_CTMATRIX4D_INTEGER_KIND( prefix, Int32 ) \
    GUCEF_MATH_CTMATRIX4D_INTEGER_KIND( prefix, UInt32 ) \
    GUCEF_MATH_CTMATRIX4D_INTEGER_KIND( prefix, Int64 ) \
    GUCEF_MATH_CTMATRIX4D_INTEGER_KIND( prefix, UInt64 )

#define GUCEF_MATH_CTMATRIX4D_FRACTION_KINDS( prefix ) \
    GUCEF_MATH_CTMATRIX4D_EXACT_KIND( prefix, CORE::TInt64Fraction ) \
    GUCEF_MATH_CTMATRIX4D_EXACT_KIND( prefix, TMathBigFraction )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTMATRIX4D_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTMATRIX4D_H ? */
