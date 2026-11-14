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

#ifndef GUCEF_MATH_CTVECTOR3D_H
#define GUCEF_MATH_CTVECTOR3D_H

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
template < typename T > class CTVector4D;
template < typename T > class CTDirectionVector3D;

/*-------------------------------------------------------------------------*/

/**
 *  A displacement in 3D (column vector): the pattern that every vector class of gucefMATH follows.
 *
 *  - Kind: T is any numeric kind of CTNumericTraits (floats, fixed point, integers, fractions). An operation that the
 *    kind cannot support does not compile; its static assertion names the reason (lengths and normalisation need a
 *    square root, division needs a kind with division, angles need transcendental functions).
 *  - Layout: exactly the three components x, y, z, public, no padding, no virtual functions: the size is 3 * sizeof( T )
 *    and the alignment that of T, so arrays are contiguous and interchangeable with CORE::TPoint3DF for Float32.
 *  - Affine types: a position is a CTPoint3D, a unit direction a CTDirectionVector3D; point - point gives a vector.
 *  - Directions per the GUCEF default convention: right-handed, +X right, +Y up, -Z forward.
 *  - Arithmetic per kind through CTMathComponentKernels: integer components wrap as the built-in unsigned types do
 *    (no undefined overflow), DotWide is exact; fixed point dot products, lengths and normalisation accumulate the
 *    raw products exactly in 128 bits and round once; float lengths and normalisation scale by the largest component
 *    when the squared length would overflow or underflow, so neither happens.
 *  - Equality: == and != are exact; Equals uses tolerances (the defaults of the kind or given ones) per component.
 *  - Realtime: every member except the text and DataNode members is realtime-safe (no allocation, lock or tracing).
 *  - Text: "(x, y, z)", locale independent, exact round trip; DataNode: attributes x, y, z (see ComponentFunctions).
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTVector3D
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef typename CTMathComponentTraits< T >::TKernels TKernels;
    typedef typename CTMathComponentTraits< T >::TDotWide TDotWide;

    T x;
    T y;
    T z;

    /** The zero vector */
    GUCEF_MATH_FORCE_INLINE CTVector3D( void )
        : x( TTraits::Zero() ), y( TTraits::Zero() ), z( TTraits::Zero() )
    {
    }

    GUCEF_MATH_FORCE_INLINE CTVector3D( const T& xValue, const T& yValue, const T& zValue )
        : x( xValue ), y( yValue ), z( zValue )
    {
    }

    /**
     *  Converts from another kind component-wise with CTNumericConverter, rounding half to even and saturating
     *  (section 3.4 of the design). NaN or infinity into a kind without them gives 0 and asserts in debug builds.
     */
    template < typename U >
    explicit CTVector3D( const CTVector3D< U >& other )
        : x( TTraits::Zero() ), y( TTraits::Zero() ), z( TTraits::Zero() )
    {
        TMathStatus status = CTNumericConverter< U, T >::Convert( other.x, x, CORE::ROUNDINGMODE_HALF_EVEN );
        status |= CTNumericConverter< U, T >::Convert( other.y, y, CORE::ROUNDINGMODE_HALF_EVEN );
        status |= CTNumericConverter< U, T >::Convert( other.z, z, CORE::ROUNDINGMODE_HALF_EVEN );
        GUCEF_MATH_ASSERT( 0 == ( status & MATHSTATUS_INVALID_OPERAND ) );
        (void) status;
    }

    /*---------------------------------------------------------------------*/
    /*  Named constants (directions per the GUCEF default convention)       */
    /*---------------------------------------------------------------------*/

    static GUCEF_MATH_FORCE_INLINE CTVector3D Zero( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector3D(); }
    static GUCEF_MATH_FORCE_INLINE CTVector3D One( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector3D( TTraits::One(), TTraits::One(), TTraits::One() ); }
    static GUCEF_MATH_FORCE_INLINE CTVector3D UnitX( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector3D( TTraits::One(), TTraits::Zero(), TTraits::Zero() ); }
    static GUCEF_MATH_FORCE_INLINE CTVector3D UnitY( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector3D( TTraits::Zero(), TTraits::One(), TTraits::Zero() ); }
    static GUCEF_MATH_FORCE_INLINE CTVector3D UnitZ( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector3D( TTraits::Zero(), TTraits::Zero(), TTraits::One() ); }

    /** +X */
    static GUCEF_MATH_FORCE_INLINE CTVector3D Right( void ) GUCEF_MATH_REALTIME_CONTEXT { return UnitX(); }
    /** -X */
    static GUCEF_MATH_FORCE_INLINE CTVector3D Left( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector3D( ComponentNegate( TTraits::One() ), TTraits::Zero(), TTraits::Zero() ); }
    /** +Y */
    static GUCEF_MATH_FORCE_INLINE CTVector3D Up( void ) GUCEF_MATH_REALTIME_CONTEXT { return UnitY(); }
    /** -Y */
    static GUCEF_MATH_FORCE_INLINE CTVector3D Down( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector3D( TTraits::Zero(), ComponentNegate( TTraits::One() ), TTraits::Zero() ); }
    /** -Z */
    static GUCEF_MATH_FORCE_INLINE CTVector3D Forward( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector3D( TTraits::Zero(), TTraits::Zero(), ComponentNegate( TTraits::One() ) ); }
    /** +Z */
    static GUCEF_MATH_FORCE_INLINE CTVector3D Back( void ) GUCEF_MATH_REALTIME_CONTEXT { return UnitZ(); }

    /*---------------------------------------------------------------------*/
    /*  Arithmetic operators                                                */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE CTVector3D operator-( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector3D( ComponentNegate( x ), ComponentNegate( y ), ComponentNegate( z ) );
    }

    GUCEF_MATH_FORCE_INLINE CTVector3D operator+( void ) const GUCEF_MATH_REALTIME_CONTEXT { return *this; }

    friend GUCEF_MATH_FORCE_INLINE CTVector3D operator+( const CTVector3D& a, const CTVector3D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector3D( TKernels::Add( a.x, b.x ), TKernels::Add( a.y, b.y ), TKernels::Add( a.z, b.z ) );
    }

    friend GUCEF_MATH_FORCE_INLINE CTVector3D operator-( const CTVector3D& a, const CTVector3D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector3D( TKernels::Subtract( a.x, b.x ), TKernels::Subtract( a.y, b.y ), TKernels::Subtract( a.z, b.z ) );
    }

    friend GUCEF_MATH_FORCE_INLINE CTVector3D operator*( const CTVector3D& v, const T& factor ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector3D( TKernels::Multiply( v.x, factor ), TKernels::Multiply( v.y, factor ), TKernels::Multiply( v.z, factor ) );
    }

    friend GUCEF_MATH_FORCE_INLINE CTVector3D operator*( const T& factor, const CTVector3D& v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector3D( TKernels::Multiply( factor, v.x ), TKernels::Multiply( factor, v.y ), TKernels::Multiply( factor, v.z ) );
    }

    /** Division by a scalar of the kind (HasDivision; integer vectors use DivideRounded) */
    friend GUCEF_MATH_FORCE_INLINE CTVector3D operator/( const CTVector3D& v, const T& divisor ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Division_requires_a_numeric_kind_with_division );
        return CTVector3D( v.x / divisor, v.y / divisor, v.z / divisor );
    }

    GUCEF_MATH_FORCE_INLINE CTVector3D& operator+=( const CTVector3D& other ) GUCEF_MATH_REALTIME_CONTEXT { *this = *this + other; return *this; }
    GUCEF_MATH_FORCE_INLINE CTVector3D& operator-=( const CTVector3D& other ) GUCEF_MATH_REALTIME_CONTEXT { *this = *this - other; return *this; }
    GUCEF_MATH_FORCE_INLINE CTVector3D& operator*=( const T& factor ) GUCEF_MATH_REALTIME_CONTEXT { *this = *this * factor; return *this; }
    GUCEF_MATH_FORCE_INLINE CTVector3D& operator/=( const T& divisor ) GUCEF_MATH_REALTIME_CONTEXT { *this = *this / divisor; return *this; }

    /** Exact comparison of every component (NaN is unequal to everything, -0 equals +0) */
    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTVector3D& a, const CTVector3D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }

    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTVector3D& a, const CTVector3D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /*---------------------------------------------------------------------*/
    /*  Products and lengths                                                */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE CTVector3D ComponentMultiply( const CTVector3D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector3D( TKernels::Multiply( x, other.x ), TKernels::Multiply( y, other.y ), TKernels::Multiply( z, other.z ) );
    }

    /** Component-wise quotient (HasDivision) */
    GUCEF_MATH_FORCE_INLINE CTVector3D ComponentDivide( const CTVector3D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Division_requires_a_numeric_kind_with_division );
        return CTVector3D( x / other.x, y / other.y, z / other.z );
    }

    /**
     *  The dot product in the kind: floats ( x x' + y y' ) + z z'; integers wrap as their type does (see DotWide);
     *  fixed point is the exact sum of the products rounded once and saturated; fractions are exact
     */
    T Dot( const CTVector3D& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The exact dot product of integer vectors in a wider type (CTMathIntegerComponentTypes): Int64 for 16 bit,
     *  CInt128 / CUInt128 for 32 and 64 bit components. Equals Dot() for the other kinds.
     */
    TDotWide DotWide( const CTVector3D& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  Right-handed cross product: UnitX().Cross( UnitY() ) == UnitZ(). Each component is a two term dot product of the
     *  kind, so fixed point rounds once per component.
     */
    CTVector3D Cross( const CTVector3D& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    T GetSquaredLength( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  Euclidean length (HasSqrt) for every vector of the format: floats scale by the largest component when the
     *  squared length leaves the safe range; fixed point is correctly rounded from the exact sum of squares (and
     *  saturates only when the length itself is not representable).
     */
    T GetLength( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** |x| + |y| + |z| */
    GUCEF_MATH_FORCE_INLINE T GetManhattanLength( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TKernels::Add( TKernels::Add( TTraits::Abs( x ), TTraits::Abs( y ) ), TTraits::Abs( z ) );
    }

    /** max( |x|, |y|, |z| ) */
    GUCEF_MATH_FORCE_INLINE T GetChebyshevLength( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::Max( TTraits::Max( TTraits::Abs( x ), TTraits::Abs( y ) ), TTraits::Abs( z ) );
    }

    /*---------------------------------------------------------------------*/
    /*  Normalisation and projections                                       */
    /*---------------------------------------------------------------------*/

    /**
     *  This vector divided by its length (HasSqrt), for every vector of the format (see GetLength; very long and very
     *  short vectors are first divided by their largest component). The zero vector and non-finite vectors give the
     *  zero vector.
     */
    CTVector3D Normalized( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Normalized() when the length is finite and greater than minLength (>= 0); false leaves 'out' unchanged */
    bool TryNormalize( CTVector3D& out, const T& minLength ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The unit direction of this vector; false for the zero vector and non-finite vectors ('out' unchanged) */
    bool TryGetDirection( CTDirectionVector3D< T >& out ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTDirectionVector3D< T >::TryCreate( *this, out );
    }

    /** This vector shortened to maxLength (>= 0) when it is longer; otherwise unchanged (HasSqrt) */
    CTVector3D ClampLength( const T& maxLength ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The component of this vector along 'onto': onto * ( dot / |onto|^2 ); zero when onto is zero (HasDivision) */
    CTVector3D Project( const CTVector3D& onto ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** This vector minus its component along the plane normal */
    CTVector3D ProjectOnPlane( const CTDirectionVector3D< T >& planeNormal ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Mirror image in the plane with the given normal: v - 2 ( v . n ) n */
    CTVector3D Reflect( const CTDirectionVector3D< T >& normal ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  A vector perpendicular to this one, not normalised: the cross product with the axis of the smallest absolute
     *  component, so it is never close to zero for a non-zero input. Zero for the zero vector.
     */
    CTVector3D GetAnyPerpendicular( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        T ax = TTraits::Abs( x );
        T ay = TTraits::Abs( y );
        T az = TTraits::Abs( z );
        if ( !( ay < ax ) && !( az < ax ) )
            return CTVector3D( TTraits::Zero(), z, ComponentNegate( y ) );    // this x UnitX
        if ( !( az < ay ) )
            return CTVector3D( ComponentNegate( z ), TTraits::Zero(), x );    // this x UnitY
        return CTVector3D( y, ComponentNegate( x ), TTraits::Zero() );        // this x UnitZ
    }

    /*---------------------------------------------------------------------*/
    /*  Component-wise functions                                            */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE CTVector3D Abs( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector3D( TTraits::Abs( x ), TTraits::Abs( y ), TTraits::Abs( z ) );
    }

    GUCEF_MATH_FORCE_INLINE CTVector3D Min( const CTVector3D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector3D( TTraits::Min( x, other.x ), TTraits::Min( y, other.y ), TTraits::Min( z, other.z ) );
    }

    GUCEF_MATH_FORCE_INLINE CTVector3D Max( const CTVector3D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector3D( TTraits::Max( x, other.x ), TTraits::Max( y, other.y ), TTraits::Max( z, other.z ) );
    }

    GUCEF_MATH_FORCE_INLINE T GetMinComponent( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::Min( TTraits::Min( x, y ), z );
    }

    GUCEF_MATH_FORCE_INLINE T GetMaxComponent( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::Max( TTraits::Max( x, y ), z );
    }

    /** Index ( 0 = x, 1 = y, 2 = z ) of the largest component; the lowest index on ties */
    GUCEF_MATH_FORCE_INLINE UInt32 GetMaxComponentIndex( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !( x < y ) && !( x < z ) )
            return 0;
        return !( y < z ) ? 1 : 2;
    }

    /**
     *  Each component divided by 'divisor' and rounded with 'mode' (integer kinds; the exact quotient is rounded once).
     *  Returns the OR of the component statuses: DIVISION_BY_ZERO, ROUNDING_REQUIRED and INVALID_OPERAND leave 'out'
     *  unchanged, OVERFLOW stores the saturated result.
     */
    TMathStatus DivideRounded( const T& divisor, TRoundingMode mode, CTVector3D& out ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( TTraits::IsInteger, DivideRounded_requires_an_integer_kind );
        CTVector3D result( out );
        TMathStatus status = TTraits::DivideRounded( x, divisor, mode, result.x );
        status |= TTraits::DivideRounded( y, divisor, mode, result.y );
        status |= TTraits::DivideRounded( z, divisor, mode, result.z );
        if ( 0 == ( status & ( MATHSTATUS_DIVISION_BY_ZERO | MATHSTATUS_ROUNDING_REQUIRED | MATHSTATUS_INVALID_OPERAND ) ) )
            out = result;
        return status;
    }

    /** Component by index: 0 = x, 1 = y, 2 = z. An index out of range asserts in debug builds and gives zero */
    GUCEF_MATH_FORCE_INLINE T GetComponent( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( index < 3 );
        return index < 3 ? this->*ComponentMember( index ) : TTraits::Zero();
    }

    /** Sets a component by index: 0 = x, 1 = y, 2 = z. An index out of range asserts in debug builds and is ignored */
    GUCEF_MATH_FORCE_INLINE void SetComponent( UInt32 index, const T& value ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( index < 3 );
        if ( index < 3 )
            this->*ComponentMember( index ) = value;
    }

    /*---------------------------------------------------------------------*/
    /*  Queries and tolerant comparison                                     */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE bool IsZero( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::Zero() == x && TTraits::Zero() == y && TTraits::Zero() == z;
    }

    /** No component is NaN or infinite (always true for kinds without them) */
    GUCEF_MATH_FORCE_INLINE bool IsFinite( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::IsFinite( x ) && TTraits::IsFinite( y ) && TTraits::IsFinite( z );
    }

    /** Every component equal within the default tolerances of the kind (exact for integers and fractions) */
    GUCEF_MATH_FORCE_INLINE bool Equals( const CTVector3D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return Equals( other, TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() );
    }

    /** Every component: |a - b| <= max( absolute, relative * max( |a|, |b| ) ); NaN never equals anything */
    bool Equals( const CTVector3D& other, const T& absolute, const T& relative ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::EqualsWithin( x, other.x, absolute, relative ) &&
               TTraits::EqualsWithin( y, other.y, absolute, relative ) &&
               TTraits::EqualsWithin( z, other.z, absolute, relative );
    }

    /** Every component within 'ulps' units in the last place (floats) or raw units (fixed point) */
    bool EqualsUlp( const CTVector3D& other, UInt32 ulps ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( TTraits::IsFloatingPoint || TTraits::IsFixedPoint, EqualsUlp_requires_a_floating_or_fixed_point_kind );
        return TTraits::EqualsUlp( x, other.x, ulps ) && TTraits::EqualsUlp( y, other.y, ulps ) && TTraits::EqualsUlp( z, other.z, ulps );
    }

    /*---------------------------------------------------------------------*/
    /*  Swizzles (by value)                                                 */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE CTVector2D< T > XY( void ) const GUCEF_MATH_REALTIME_CONTEXT { return CTVector2D< T >( x, y ); }
    GUCEF_MATH_FORCE_INLINE CTVector2D< T > XZ( void ) const GUCEF_MATH_REALTIME_CONTEXT { return CTVector2D< T >( x, z ); }
    GUCEF_MATH_FORCE_INLINE CTVector2D< T > YZ( void ) const GUCEF_MATH_REALTIME_CONTEXT { return CTVector2D< T >( y, z ); }

    /** ( x, y, z, 0 ): the homogeneous form of a direction or displacement */
    GUCEF_MATH_FORCE_INLINE CTVector4D< T > XYZ0( void ) const GUCEF_MATH_REALTIME_CONTEXT { return CTVector4D< T >( x, y, z, TTraits::Zero() ); }

    /** ( x, y, z, 1 ) */
    GUCEF_MATH_FORCE_INLINE CTVector4D< T > XYZ1( void ) const GUCEF_MATH_REALTIME_CONTEXT { return CTVector4D< T >( x, y, z, TTraits::One() ); }

    /** ( component I, component J, component K ), for example Swizzle< 2, 1, 0 >() is ( z, y, x ) */
    template < UInt32 I, UInt32 J, UInt32 K >
    GUCEF_MATH_FORCE_INLINE CTVector3D Swizzle( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( I < 3 && J < 3 && K < 3, Swizzle_index_out_of_range );
        return CTVector3D( this->*ComponentMember( I ), this->*ComponentMember( J ), this->*ComponentMember( K ) );
    }

    /*---------------------------------------------------------------------*/
    /*  Interpolation and angles                                            */
    /*---------------------------------------------------------------------*/

    /** a + ( b - a ) * t, not clamped: t outside [0, 1] extrapolates */
    static CTVector3D Lerp( const CTVector3D& a, const CTVector3D& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT;

    /** current moved toward target by at most maxDistanceDelta, never past it (HasSqrt) */
    static CTVector3D MoveTowards( const CTVector3D& current, const CTVector3D& target, const T& maxDistanceDelta ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The angle between a and b in [0, pi], as atan2( |a x b|, a . b ) of the vectors divided by their largest
     *  components (the angle does not change, and no product overflows); accurate for small angles. 0 when a vector
     *  is zero or not finite.
     */
    static CTRadian< T > Angle( const CTVector3D& a, const CTVector3D& b ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  Angle( from, to ) with the sign of the rotation about 'axis' by the right-hand rule: positive when
     *  ( from x to ) . axis >= 0. SignedAngle( UnitX, UnitY, +Z ) is +pi/2.
     */
    static CTRadian< T > SignedAngle( const CTVector3D& from, const CTVector3D& to, const CTDirectionVector3D< T >& axis ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  This vector divided by its largest component magnitude (floats and fixed point): every component in [-1, 1].
     *  The direction is unchanged and products of the result cannot overflow. False (out unchanged) for the zero
     *  vector and non-finite vectors.
     */
    bool ScaleToUnitChebyshev( CTVector3D& out ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, Scaling_requires_a_floating_or_fixed_point_kind );
        const T a[ 3 ] = { x, y, z };
        T scaled[ 3 ];
        if ( !TKernels::ScaleToUnitChebyshev( a, scaled ) )
            return false;
        out = CTVector3D( scaled[ 0 ], scaled[ 1 ], scaled[ 2 ] );
        return true;
    }

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "(x, y, z)" */
    CORE::CString ToString( void ) const
    {
        const T components[ 3 ] = { x, y, z };
        return ComponentsToString( components );
    }

    /** Parses the ToString form (white space allowed); false leaves this vector unchanged */
    bool FromString( const CORE::CString& text )
    {
        T components[ 3 ];
        if ( !ComponentsFromString( text, components ) )
            return false;
        x = components[ 0 ];
        y = components[ 1 ];
        z = components[ 2 ];
        return true;
    }

    /** Writes the attributes x, y, z into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ ) const
    {
        const char* const names[ 3 ] = { "x", "y", "z" };
        const T components[ 3 ] = { x, y, z };
        return SerializeComponents( node, names, components );
    }

    /** Reads the attributes x, y, z; a missing or invalid attribute fails and leaves this vector unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ )
    {
        const char* const names[ 3 ] = { "x", "y", "z" };
        T components[ 3 ];
        if ( !DeserializeComponents( node, names, components ) )
            return false;
        x = components[ 0 ];
        y = components[ 1 ];
        z = components[ 2 ];
        return true;
    }

    private:

    typedef T CTVector3D::* TComponentMember;

    /** The member for an index below 3 (constant table, no initialisation at run time) */
    static GUCEF_MATH_FORCE_INLINE TComponentMember ComponentMember( UInt32 index ) GUCEF_MATH_REALTIME_CONTEXT
    {
        static const TComponentMember members[ 3 ] = { &CTVector3D::x, &CTVector3D::y, &CTVector3D::z };
        return members[ index ];
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
Dot( const CTVector3D< T >& a, const CTVector3D< T >& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    return a.Dot( b );
}

/*-------------------------------------------------------------------------*/

/** a x b (right-handed), as a.Cross( b ) */
template < typename T >
GUCEF_MATH_FORCE_INLINE CTVector3D< T >
Cross( const CTVector3D< T >& a, const CTVector3D< T >& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    return a.Cross( b );
}

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
GUCEF_MATH_PRECISE_INLINE T
CTVector3D< T >::Dot( const CTVector3D& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const T a[ 3 ] = { x, y, z };
    const T b[ 3 ] = { other.x, other.y, other.z };
    return TKernels::Dot( a, b );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE typename CTVector3D< T >::TDotWide
CTVector3D< T >::DotWide( const CTVector3D& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const T a[ 3 ] = { x, y, z };
    const T b[ 3 ] = { other.x, other.y, other.z };
    return TKernels::DotWide( a, b );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
CTVector3D< T >::Cross( const CTVector3D& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const T ax[ 2 ] = { y, ComponentNegate( z ) };
    const T bx[ 2 ] = { other.z, other.y };
    const T ay[ 2 ] = { z, ComponentNegate( x ) };
    const T by[ 2 ] = { other.x, other.z };
    const T az[ 2 ] = { x, ComponentNegate( y ) };
    const T bz[ 2 ] = { other.y, other.x };
    return CTVector3D( TKernels::Dot( ax, bx ), TKernels::Dot( ay, by ), TKernels::Dot( az, bz ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTVector3D< T >::GetSquaredLength( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return Dot( *this );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
CTVector3D< T >::Lerp( const CTVector3D& a, const CTVector3D& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return a + ( b - a ) * t;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
CTVector3D< T >::Project( const CTVector3D& onto ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Division_requires_a_numeric_kind_with_division );
    T squaredLength = onto.GetSquaredLength();
    if ( TTraits::Zero() == squaredLength )
        return CTVector3D();
    return onto * ( Dot( onto ) / squaredLength );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTVector3D< T >::GetLength( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, Length_requires_a_numeric_kind_with_sqrt );
    const T a[ 3 ] = { x, y, z };
    return TKernels::Length( a );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
CTVector3D< T >::Normalized( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, Normalization_requires_a_numeric_kind_with_sqrt );
    const T a[ 3 ] = { x, y, z };
    T unit[ 3 ];
    if ( !TKernels::Normalize( a, unit ) )
        return CTVector3D();
    return CTVector3D( unit[ 0 ], unit[ 1 ], unit[ 2 ] );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTVector3D< T >::TryNormalize( CTVector3D& out, const T& minLength ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, Normalization_requires_a_numeric_kind_with_sqrt );
    const T a[ 3 ] = { x, y, z };
    T length = TKernels::Length( a );
    T unit[ 3 ];
    if ( !TTraits::IsFinite( length ) || !( minLength < length ) || !TKernels::Normalize( a, unit ) )
        return false;
    out = CTVector3D( unit[ 0 ], unit[ 1 ], unit[ 2 ] );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
CTVector3D< T >::ClampLength( const T& maxLength ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !( maxLength < GetLength() ) )
        return *this;
    return Normalized() * maxLength;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
CTVector3D< T >::MoveTowards( const CTVector3D& current, const CTVector3D& target, const T& maxDistanceDelta ) GUCEF_MATH_REALTIME_CONTEXT
{
    CTVector3D difference = target - current;
    if ( difference.IsZero() || ( !TTraits::IsNegative( maxDistanceDelta ) && !( maxDistanceDelta < difference.GetLength() ) ) )
        return target;
    return current + difference.Normalized() * maxDistanceDelta;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
CTVector3D< T >::ProjectOnPlane( const CTDirectionVector3D< T >& planeNormal ) const GUCEF_MATH_REALTIME_CONTEXT
{
    CTVector3D n = planeNormal.AsVector();
    return *this - n * Dot( n );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
CTVector3D< T >::Reflect( const CTDirectionVector3D< T >& normal ) const GUCEF_MATH_REALTIME_CONTEXT
{
    CTVector3D n = normal.AsVector();
    return *this - n * ( TTraits::Two() * Dot( n ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTRadian< T >
CTVector3D< T >::Angle( const CTVector3D& a, const CTVector3D& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasTranscendentals, Angle_requires_transcendentals );
    CTVector3D scaledA;
    CTVector3D scaledB;
    if ( !a.ScaleToUnitChebyshev( scaledA ) || !b.ScaleToUnitChebyshev( scaledB ) )
        return CTRadian< T >();
    return CTRadian< T >( TTraits::Atan2( scaledA.Cross( scaledB ).GetLength(), scaledA.Dot( scaledB ) ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTRadian< T >
CTVector3D< T >::SignedAngle( const CTVector3D& from, const CTVector3D& to, const CTDirectionVector3D< T >& axis ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasTranscendentals, Angle_requires_transcendentals );
    CTVector3D scaledFrom;
    CTVector3D scaledTo;
    if ( !from.ScaleToUnitChebyshev( scaledFrom ) || !to.ScaleToUnitChebyshev( scaledTo ) )
        return CTRadian< T >();
    CTVector3D cross = scaledFrom.Cross( scaledTo );
    CTRadian< T > angle( TTraits::Atan2( cross.GetLength(), scaledFrom.Dot( scaledTo ) ) );
    if ( TTraits::IsNegative( cross.Dot( axis.AsVector() ) ) )
        return -angle;
    return angle;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

/*
 *  The related classes used by the swizzles and the direction members; included after the class so that each header
 *  can be included first.
 */

#ifndef GUCEF_MATH_CTVECTOR2D_H
#include "gucefMATH_CTVector2D.h"
#define GUCEF_MATH_CTVECTOR2D_H
#endif /* GUCEF_MATH_CTVECTOR2D_H ? */

#ifndef GUCEF_MATH_CTVECTOR4D_H
#include "gucefMATH_CTVector4D.h"
#define GUCEF_MATH_CTVECTOR4D_H
#endif /* GUCEF_MATH_CTVECTOR4D_H ? */

#ifndef GUCEF_MATH_CTDIRECTIONVECTOR3D_H
#include "gucefMATH_CTDirectionVector3D.h"
#define GUCEF_MATH_CTDIRECTIONVECTOR3D_H
#endif /* GUCEF_MATH_CTDIRECTIONVECTOR3D_H ? */


/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The precision-sensitive members of the predefined kinds are instantiated in the module and declared extern here
 *  (section 5.10 of the design), member by member: a whole-class instantiation would also instantiate the members
 *  that a kind does not support, whose static assertions must stay compile errors in user code. The groups:
 *  ALL_KINDS for every kind, DIVISION for kinds with division, SQRT for kinds with a square root,
 *  TRANSCENDENTAL for kinds with transcendental functions. A list macro takes the prefix (GUCEF_MATH_EXTERN_TEMPLATE
 *  in headers, GUCEF_MATH_INSTANTIATE_TEMPLATE in the module source) and the kind (a name without commas).
 */

#define GUCEF_MATH_CTVECTOR3D_ALL_KINDS_MEMBERS( prefix, kind ) \
    prefix kind CTVector3D< kind >::Dot( const CTVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector3D< kind >::TDotWide CTVector3D< kind >::DotWide( const CTVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector3D< kind > CTVector3D< kind >::Cross( const CTVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix kind CTVector3D< kind >::GetSquaredLength( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector3D< kind > CTVector3D< kind >::Lerp( const CTVector3D< kind >&, const CTVector3D< kind >&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTVECTOR3D_DIVISION_MEMBERS( prefix, kind ) \
    prefix CTVector3D< kind > CTVector3D< kind >::Project( const CTVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTVECTOR3D_SQRT_MEMBERS( prefix, kind ) \
    prefix kind CTVector3D< kind >::GetLength( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector3D< kind > CTVector3D< kind >::Normalized( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTVector3D< kind >::TryNormalize( CTVector3D< kind >&, const kind& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector3D< kind > CTVector3D< kind >::ClampLength( const kind& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector3D< kind > CTVector3D< kind >::MoveTowards( const CTVector3D< kind >&, const CTVector3D< kind >&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector3D< kind > CTVector3D< kind >::ProjectOnPlane( const CTDirectionVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector3D< kind > CTVector3D< kind >::Reflect( const CTDirectionVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTVECTOR3D_TRANSCENDENTAL_MEMBERS( prefix, kind ) \
    prefix CTRadian< kind > CTVector3D< kind >::Angle( const CTVector3D< kind >&, const CTVector3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTRadian< kind > CTVector3D< kind >::SignedAngle( const CTVector3D< kind >&, const CTVector3D< kind >&, const CTDirectionVector3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTVECTOR3D_REAL_KIND( prefix, kind ) GUCEF_MATH_CTVECTOR3D_ALL_KINDS_MEMBERS( prefix, kind ) GUCEF_MATH_CTVECTOR3D_DIVISION_MEMBERS( prefix, kind ) GUCEF_MATH_CTVECTOR3D_SQRT_MEMBERS( prefix, kind ) GUCEF_MATH_CTVECTOR3D_TRANSCENDENTAL_MEMBERS( prefix, kind )
#define GUCEF_MATH_CTVECTOR3D_INTEGER_KIND( prefix, kind ) GUCEF_MATH_CTVECTOR3D_ALL_KINDS_MEMBERS( prefix, kind )
#define GUCEF_MATH_CTVECTOR3D_EXACT_KIND( prefix, kind ) GUCEF_MATH_CTVECTOR3D_ALL_KINDS_MEMBERS( prefix, kind ) GUCEF_MATH_CTVECTOR3D_DIVISION_MEMBERS( prefix, kind )

/** Every predefined kind except the fractions (those are in gucefMATH_fractiontypes.h, which includes CBigInteger) */
#define GUCEF_MATH_CTVECTOR3D_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTVECTOR3D_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTVECTOR3D_REAL_KIND( prefix, Float64 ) \
    GUCEF_MATH_CTVECTOR3D_REAL_KIND( prefix, TFixed16_16 ) \
    GUCEF_MATH_CTVECTOR3D_REAL_KIND( prefix, TFixed32_32 ) \
    GUCEF_MATH_CTVECTOR3D_REAL_KIND( prefix, TFixed48_16 ) \
    GUCEF_MATH_CTVECTOR3D_INTEGER_KIND( prefix, Int8 ) \
    GUCEF_MATH_CTVECTOR3D_INTEGER_KIND( prefix, UInt8 ) \
    GUCEF_MATH_CTVECTOR3D_INTEGER_KIND( prefix, Int16 ) \
    GUCEF_MATH_CTVECTOR3D_INTEGER_KIND( prefix, UInt16 ) \
    GUCEF_MATH_CTVECTOR3D_INTEGER_KIND( prefix, Int32 ) \
    GUCEF_MATH_CTVECTOR3D_INTEGER_KIND( prefix, UInt32 ) \
    GUCEF_MATH_CTVECTOR3D_INTEGER_KIND( prefix, Int64 ) \
    GUCEF_MATH_CTVECTOR3D_INTEGER_KIND( prefix, UInt64 )

#define GUCEF_MATH_CTVECTOR3D_FRACTION_KINDS( prefix ) \
    GUCEF_MATH_CTVECTOR3D_EXACT_KIND( prefix, CORE::TInt64Fraction ) \
    GUCEF_MATH_CTVECTOR3D_EXACT_KIND( prefix, TMathBigFraction )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTVECTOR3D_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTVECTOR3D_H ? */
