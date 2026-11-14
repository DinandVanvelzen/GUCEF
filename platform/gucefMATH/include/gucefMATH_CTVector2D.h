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

#ifndef GUCEF_MATH_CTVECTOR2D_H
#define GUCEF_MATH_CTVECTOR2D_H

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

template < typename T > class CTVector3D;
template < typename T > class CTDirectionVector2D;

/*-------------------------------------------------------------------------*/

/**
 *  A displacement in 2D, following the pattern of CTVector3D (kinds, layout, operators, tolerances, text, DataNode).
 *  2D specifics: Cross() is the scalar z of the 3D cross product ( x y' - y x' ); GetPerpendicular() is ( -y, x ),
 *  rotated counter-clockwise; SignedAngle() is positive counter-clockwise; ToVector3D( z ) embeds the vector in 3D.
 *  Size 2 * sizeof( T ), alignment of T, interchangeable with CORE::TPoint2DF for Float32.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTVector2D
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef typename CTMathComponentTraits< T >::TKernels TKernels;
    typedef typename CTMathComponentTraits< T >::TDotWide TDotWide;

    T x;
    T y;

    /** The zero vector */
    GUCEF_MATH_FORCE_INLINE CTVector2D( void )
        : x( TTraits::Zero() ), y( TTraits::Zero() )
    {
    }

    GUCEF_MATH_FORCE_INLINE CTVector2D( const T& xValue, const T& yValue )
        : x( xValue ), y( yValue )
    {
    }

    /** Converts from another kind component-wise (section 3.4: half to even, saturating), see CTVector3D */
    template < typename U >
    explicit CTVector2D( const CTVector2D< U >& other )
        : x( TTraits::Zero() ), y( TTraits::Zero() )
    {
        TMathStatus status = CTNumericConverter< U, T >::Convert( other.x, x, CORE::ROUNDINGMODE_HALF_EVEN );
        status |= CTNumericConverter< U, T >::Convert( other.y, y, CORE::ROUNDINGMODE_HALF_EVEN );
        GUCEF_MATH_ASSERT( 0 == ( status & MATHSTATUS_INVALID_OPERAND ) );
        (void) status;
    }

    /*---------------------------------------------------------------------*/
    /*  Named constants                                                     */
    /*---------------------------------------------------------------------*/

    static GUCEF_MATH_FORCE_INLINE CTVector2D Zero( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector2D(); }
    static GUCEF_MATH_FORCE_INLINE CTVector2D One( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector2D( TTraits::One(), TTraits::One() ); }
    static GUCEF_MATH_FORCE_INLINE CTVector2D UnitX( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector2D( TTraits::One(), TTraits::Zero() ); }
    static GUCEF_MATH_FORCE_INLINE CTVector2D UnitY( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector2D( TTraits::Zero(), TTraits::One() ); }

    /** +X */
    static GUCEF_MATH_FORCE_INLINE CTVector2D Right( void ) GUCEF_MATH_REALTIME_CONTEXT { return UnitX(); }
    /** -X */
    static GUCEF_MATH_FORCE_INLINE CTVector2D Left( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector2D( ComponentNegate( TTraits::One() ), TTraits::Zero() ); }
    /** +Y */
    static GUCEF_MATH_FORCE_INLINE CTVector2D Up( void ) GUCEF_MATH_REALTIME_CONTEXT { return UnitY(); }
    /** -Y */
    static GUCEF_MATH_FORCE_INLINE CTVector2D Down( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector2D( TTraits::Zero(), ComponentNegate( TTraits::One() ) ); }

    /*---------------------------------------------------------------------*/
    /*  Arithmetic operators                                                */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE CTVector2D operator-( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector2D( ComponentNegate( x ), ComponentNegate( y ) );
    }

    GUCEF_MATH_FORCE_INLINE CTVector2D operator+( void ) const GUCEF_MATH_REALTIME_CONTEXT { return *this; }

    friend GUCEF_MATH_FORCE_INLINE CTVector2D operator+( const CTVector2D& a, const CTVector2D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector2D( TKernels::Add( a.x, b.x ), TKernels::Add( a.y, b.y ) );
    }

    friend GUCEF_MATH_FORCE_INLINE CTVector2D operator-( const CTVector2D& a, const CTVector2D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector2D( TKernels::Subtract( a.x, b.x ), TKernels::Subtract( a.y, b.y ) );
    }

    friend GUCEF_MATH_FORCE_INLINE CTVector2D operator*( const CTVector2D& v, const T& factor ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector2D( TKernels::Multiply( v.x, factor ), TKernels::Multiply( v.y, factor ) );
    }

    friend GUCEF_MATH_FORCE_INLINE CTVector2D operator*( const T& factor, const CTVector2D& v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector2D( TKernels::Multiply( factor, v.x ), TKernels::Multiply( factor, v.y ) );
    }

    /** Division by a scalar of the kind (HasDivision; integer vectors use DivideRounded) */
    friend GUCEF_MATH_FORCE_INLINE CTVector2D operator/( const CTVector2D& v, const T& divisor ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Division_requires_a_numeric_kind_with_division );
        return CTVector2D( v.x / divisor, v.y / divisor );
    }

    GUCEF_MATH_FORCE_INLINE CTVector2D& operator+=( const CTVector2D& other ) GUCEF_MATH_REALTIME_CONTEXT { *this = *this + other; return *this; }
    GUCEF_MATH_FORCE_INLINE CTVector2D& operator-=( const CTVector2D& other ) GUCEF_MATH_REALTIME_CONTEXT { *this = *this - other; return *this; }
    GUCEF_MATH_FORCE_INLINE CTVector2D& operator*=( const T& factor ) GUCEF_MATH_REALTIME_CONTEXT { *this = *this * factor; return *this; }
    GUCEF_MATH_FORCE_INLINE CTVector2D& operator/=( const T& divisor ) GUCEF_MATH_REALTIME_CONTEXT { *this = *this / divisor; return *this; }

    /** Exact comparison of every component */
    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTVector2D& a, const CTVector2D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.x == b.x && a.y == b.y;
    }

    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTVector2D& a, const CTVector2D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /*---------------------------------------------------------------------*/
    /*  Products and lengths                                                */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE CTVector2D ComponentMultiply( const CTVector2D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector2D( TKernels::Multiply( x, other.x ), TKernels::Multiply( y, other.y ) );
    }

    /** Component-wise quotient (HasDivision) */
    GUCEF_MATH_FORCE_INLINE CTVector2D ComponentDivide( const CTVector2D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Division_requires_a_numeric_kind_with_division );
        return CTVector2D( x / other.x, y / other.y );
    }

    /** The dot product in the kind (see CTVector3D::Dot) */
    T Dot( const CTVector2D& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The exact dot product of integer vectors in a wider type (see CTVector3D::DotWide) */
    TDotWide DotWide( const CTVector2D& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** x y' - y x': the z component of the 3D cross product, positive when 'other' is counter-clockwise of this */
    T Cross( const CTVector2D& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** ( -y, x ): this vector rotated a quarter turn counter-clockwise */
    GUCEF_MATH_FORCE_INLINE CTVector2D GetPerpendicular( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector2D( ComponentNegate( y ), x );
    }

    T GetSquaredLength( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Euclidean length (HasSqrt) for every vector of the format (see CTVector3D::GetLength) */
    T GetLength( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    GUCEF_MATH_FORCE_INLINE T GetManhattanLength( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TKernels::Add( TTraits::Abs( x ), TTraits::Abs( y ) );
    }

    GUCEF_MATH_FORCE_INLINE T GetChebyshevLength( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::Max( TTraits::Abs( x ), TTraits::Abs( y ) );
    }

    /*---------------------------------------------------------------------*/
    /*  Normalisation and projections                                       */
    /*---------------------------------------------------------------------*/

    /** This vector divided by its length (HasSqrt), for every vector of the format; zero and non-finite give zero */
    CTVector2D Normalized( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Normalized() when the length is finite and greater than minLength (>= 0); false leaves 'out' unchanged */
    bool TryNormalize( CTVector2D& out, const T& minLength ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The unit direction of this vector; false for the zero vector and non-finite vectors ('out' unchanged) */
    bool TryGetDirection( CTDirectionVector2D< T >& out ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTDirectionVector2D< T >::TryCreate( *this, out );
    }

    /** This vector shortened to maxLength (>= 0) when it is longer; otherwise unchanged (HasSqrt) */
    CTVector2D ClampLength( const T& maxLength ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The component of this vector along 'onto'; zero when onto is zero (HasDivision) */
    CTVector2D Project( const CTVector2D& onto ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Mirror image in the line with the given normal: v - 2 ( v . n ) n */
    CTVector2D Reflect( const CTDirectionVector2D< T >& normal ) const GUCEF_MATH_REALTIME_CONTEXT;

    /*---------------------------------------------------------------------*/
    /*  Component-wise functions                                            */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE CTVector2D Abs( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector2D( TTraits::Abs( x ), TTraits::Abs( y ) );
    }

    GUCEF_MATH_FORCE_INLINE CTVector2D Min( const CTVector2D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector2D( TTraits::Min( x, other.x ), TTraits::Min( y, other.y ) );
    }

    GUCEF_MATH_FORCE_INLINE CTVector2D Max( const CTVector2D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector2D( TTraits::Max( x, other.x ), TTraits::Max( y, other.y ) );
    }

    GUCEF_MATH_FORCE_INLINE T GetMinComponent( void ) const GUCEF_MATH_REALTIME_CONTEXT { return TTraits::Min( x, y ); }
    GUCEF_MATH_FORCE_INLINE T GetMaxComponent( void ) const GUCEF_MATH_REALTIME_CONTEXT { return TTraits::Max( x, y ); }

    /** Index ( 0 = x, 1 = y ) of the largest component; 0 on a tie */
    GUCEF_MATH_FORCE_INLINE UInt32 GetMaxComponentIndex( void ) const GUCEF_MATH_REALTIME_CONTEXT { return x < y ? 1 : 0; }

    /** Each component divided by 'divisor' and rounded with 'mode' (integer kinds), see CTVector3D::DivideRounded */
    TMathStatus DivideRounded( const T& divisor, TRoundingMode mode, CTVector2D& out ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( TTraits::IsInteger, DivideRounded_requires_an_integer_kind );
        CTVector2D result( out );
        TMathStatus status = TTraits::DivideRounded( x, divisor, mode, result.x );
        status |= TTraits::DivideRounded( y, divisor, mode, result.y );
        if ( 0 == ( status & ( MATHSTATUS_DIVISION_BY_ZERO | MATHSTATUS_ROUNDING_REQUIRED | MATHSTATUS_INVALID_OPERAND ) ) )
            out = result;
        return status;
    }

    /** Component by index: 0 = x, 1 = y. An index out of range asserts in debug builds and gives zero */
    GUCEF_MATH_FORCE_INLINE T GetComponent( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( index < 2 );
        return index < 2 ? this->*ComponentMember( index ) : TTraits::Zero();
    }

    /** Sets a component by index: 0 = x, 1 = y. An index out of range asserts in debug builds and is ignored */
    GUCEF_MATH_FORCE_INLINE void SetComponent( UInt32 index, const T& value ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( index < 2 );
        if ( index < 2 )
            this->*ComponentMember( index ) = value;
    }

    /*---------------------------------------------------------------------*/
    /*  Queries and tolerant comparison                                     */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE bool IsZero( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::Zero() == x && TTraits::Zero() == y;
    }

    GUCEF_MATH_FORCE_INLINE bool IsFinite( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::IsFinite( x ) && TTraits::IsFinite( y );
    }

    /** Every component equal within the default tolerances of the kind */
    GUCEF_MATH_FORCE_INLINE bool Equals( const CTVector2D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return Equals( other, TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() );
    }

    /** Every component: |a - b| <= max( absolute, relative * max( |a|, |b| ) ); NaN never equals anything */
    bool Equals( const CTVector2D& other, const T& absolute, const T& relative ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::EqualsWithin( x, other.x, absolute, relative ) && TTraits::EqualsWithin( y, other.y, absolute, relative );
    }

    /** Every component within 'ulps' units in the last place (floats) or raw units (fixed point) */
    bool EqualsUlp( const CTVector2D& other, UInt32 ulps ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( TTraits::IsFloatingPoint || TTraits::IsFixedPoint, EqualsUlp_requires_a_floating_or_fixed_point_kind );
        return TTraits::EqualsUlp( x, other.x, ulps ) && TTraits::EqualsUlp( y, other.y, ulps );
    }

    /*---------------------------------------------------------------------*/
    /*  Swizzles and conversion to 3D                                       */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE CTVector2D YX( void ) const GUCEF_MATH_REALTIME_CONTEXT { return CTVector2D( y, x ); }

    /** ( component I, component J ) */
    template < UInt32 I, UInt32 J >
    GUCEF_MATH_FORCE_INLINE CTVector2D Swizzle( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( I < 2 && J < 2, Swizzle_index_out_of_range );
        return CTVector2D( this->*ComponentMember( I ), this->*ComponentMember( J ) );
    }

    /** ( x, y, z ): the vector in the plane z of 3D space */
    GUCEF_MATH_FORCE_INLINE CTVector3D< T > ToVector3D( const T& z ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector3D< T >( x, y, z );
    }

    /*---------------------------------------------------------------------*/
    /*  Interpolation and angles                                            */
    /*---------------------------------------------------------------------*/

    /** a + ( b - a ) * t, not clamped */
    static CTVector2D Lerp( const CTVector2D& a, const CTVector2D& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT;

    /** current moved toward target by at most maxDistanceDelta, never past it (HasSqrt) */
    static CTVector2D MoveTowards( const CTVector2D& current, const CTVector2D& target, const T& maxDistanceDelta ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The angle between a and b in [0, pi] (see CTVector3D::Angle); 0 when a vector is zero or not finite */
    static CTRadian< T > Angle( const CTVector2D& a, const CTVector2D& b ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The rotation from 'from' to 'to' in ( -pi, pi ], positive counter-clockwise: atan2( from x to, from . to ) of the
     *  vectors divided by their largest components. 0 when a vector is zero or not finite.
     */
    static CTRadian< T > SignedAngle( const CTVector2D& from, const CTVector2D& to ) GUCEF_MATH_REALTIME_CONTEXT;

    /** This vector divided by its largest component magnitude (see CTVector3D::ScaleToUnitChebyshev) */
    bool ScaleToUnitChebyshev( CTVector2D& out ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, Scaling_requires_a_floating_or_fixed_point_kind );
        const T a[ 2 ] = { x, y };
        T scaled[ 2 ];
        if ( !TKernels::ScaleToUnitChebyshev( a, scaled ) )
            return false;
        out = CTVector2D( scaled[ 0 ], scaled[ 1 ] );
        return true;
    }

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "(x, y)" */
    CORE::CString ToString( void ) const
    {
        const T components[ 2 ] = { x, y };
        return ComponentsToString( components );
    }

    /** Parses the ToString form (white space allowed); false leaves this vector unchanged */
    bool FromString( const CORE::CString& text )
    {
        T components[ 2 ];
        if ( !ComponentsFromString( text, components ) )
            return false;
        x = components[ 0 ];
        y = components[ 1 ];
        return true;
    }

    /** Writes the attributes x, y into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ ) const
    {
        const char* const names[ 2 ] = { "x", "y" };
        const T components[ 2 ] = { x, y };
        return SerializeComponents( node, names, components );
    }

    /** Reads the attributes x, y; a missing or invalid attribute fails and leaves this vector unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ )
    {
        const char* const names[ 2 ] = { "x", "y" };
        T components[ 2 ];
        if ( !DeserializeComponents( node, names, components ) )
            return false;
        x = components[ 0 ];
        y = components[ 1 ];
        return true;
    }

    private:

    typedef T CTVector2D::* TComponentMember;

    static GUCEF_MATH_FORCE_INLINE TComponentMember ComponentMember( UInt32 index ) GUCEF_MATH_REALTIME_CONTEXT
    {
        static const TComponentMember members[ 2 ] = { &CTVector2D::x, &CTVector2D::y };
        return members[ index ];
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** a . b */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
Dot( const CTVector2D< T >& a, const CTVector2D< T >& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    return a.Dot( b );
}

/*-------------------------------------------------------------------------*/

/** x y' - y x', the scalar 2D cross product */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
Cross( const CTVector2D< T >& a, const CTVector2D< T >& b ) GUCEF_MATH_REALTIME_CONTEXT
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
CTVector2D< T >::Dot( const CTVector2D& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const T a[ 2 ] = { x, y };
    const T b[ 2 ] = { other.x, other.y };
    return TKernels::Dot( a, b );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE typename CTVector2D< T >::TDotWide
CTVector2D< T >::DotWide( const CTVector2D& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const T a[ 2 ] = { x, y };
    const T b[ 2 ] = { other.x, other.y };
    return TKernels::DotWide( a, b );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTVector2D< T >::Cross( const CTVector2D& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const T a[ 2 ] = { x, ComponentNegate( y ) };
    const T b[ 2 ] = { other.y, other.x };
    return TKernels::Dot( a, b );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTVector2D< T >::GetSquaredLength( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return Dot( *this );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector2D< T >
CTVector2D< T >::Lerp( const CTVector2D& a, const CTVector2D& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return a + ( b - a ) * t;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector2D< T >
CTVector2D< T >::Project( const CTVector2D& onto ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Division_requires_a_numeric_kind_with_division );
    T squaredLength = onto.GetSquaredLength();
    if ( TTraits::Zero() == squaredLength )
        return CTVector2D();
    return onto * ( Dot( onto ) / squaredLength );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTVector2D< T >::GetLength( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, Length_requires_a_numeric_kind_with_sqrt );
    const T a[ 2 ] = { x, y };
    return TKernels::Length( a );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector2D< T >
CTVector2D< T >::Normalized( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, Normalization_requires_a_numeric_kind_with_sqrt );
    const T a[ 2 ] = { x, y };
    T unit[ 2 ];
    if ( !TKernels::Normalize( a, unit ) )
        return CTVector2D();
    return CTVector2D( unit[ 0 ], unit[ 1 ] );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTVector2D< T >::TryNormalize( CTVector2D& out, const T& minLength ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, Normalization_requires_a_numeric_kind_with_sqrt );
    const T a[ 2 ] = { x, y };
    T length = TKernels::Length( a );
    T unit[ 2 ];
    if ( !TTraits::IsFinite( length ) || !( minLength < length ) || !TKernels::Normalize( a, unit ) )
        return false;
    out = CTVector2D( unit[ 0 ], unit[ 1 ] );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector2D< T >
CTVector2D< T >::ClampLength( const T& maxLength ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !( maxLength < GetLength() ) )
        return *this;
    return Normalized() * maxLength;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector2D< T >
CTVector2D< T >::MoveTowards( const CTVector2D& current, const CTVector2D& target, const T& maxDistanceDelta ) GUCEF_MATH_REALTIME_CONTEXT
{
    CTVector2D difference = target - current;
    if ( difference.IsZero() || ( !TTraits::IsNegative( maxDistanceDelta ) && !( maxDistanceDelta < difference.GetLength() ) ) )
        return target;
    return current + difference.Normalized() * maxDistanceDelta;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector2D< T >
CTVector2D< T >::Reflect( const CTDirectionVector2D< T >& normal ) const GUCEF_MATH_REALTIME_CONTEXT
{
    CTVector2D n = normal.AsVector();
    return *this - n * ( TTraits::Two() * Dot( n ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTRadian< T >
CTVector2D< T >::Angle( const CTVector2D& a, const CTVector2D& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CTRadian< T >( TTraits::Abs( SignedAngle( a, b ).GetValue() ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTRadian< T >
CTVector2D< T >::SignedAngle( const CTVector2D& from, const CTVector2D& to ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasTranscendentals, Angle_requires_transcendentals );
    CTVector2D scaledFrom;
    CTVector2D scaledTo;
    if ( !from.ScaleToUnitChebyshev( scaledFrom ) || !to.ScaleToUnitChebyshev( scaledTo ) )
        return CTRadian< T >();
    return CTRadian< T >( TTraits::Atan2( scaledFrom.Cross( scaledTo ), scaledFrom.Dot( scaledTo ) ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTVECTOR3D_H
#include "gucefMATH_CTVector3D.h"
#define GUCEF_MATH_CTVECTOR3D_H
#endif /* GUCEF_MATH_CTVECTOR3D_H ? */

#ifndef GUCEF_MATH_CTDIRECTIONVECTOR2D_H
#include "gucefMATH_CTDirectionVector2D.h"
#define GUCEF_MATH_CTDIRECTIONVECTOR2D_H
#endif /* GUCEF_MATH_CTDIRECTIONVECTOR2D_H ? */


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

#define GUCEF_MATH_CTVECTOR2D_ALL_KINDS_MEMBERS( prefix, kind ) \
    prefix kind CTVector2D< kind >::Dot( const CTVector2D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector2D< kind >::TDotWide CTVector2D< kind >::DotWide( const CTVector2D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix kind CTVector2D< kind >::Cross( const CTVector2D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix kind CTVector2D< kind >::GetSquaredLength( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector2D< kind > CTVector2D< kind >::Lerp( const CTVector2D< kind >&, const CTVector2D< kind >&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTVECTOR2D_DIVISION_MEMBERS( prefix, kind ) \
    prefix CTVector2D< kind > CTVector2D< kind >::Project( const CTVector2D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTVECTOR2D_SQRT_MEMBERS( prefix, kind ) \
    prefix kind CTVector2D< kind >::GetLength( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector2D< kind > CTVector2D< kind >::Normalized( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTVector2D< kind >::TryNormalize( CTVector2D< kind >&, const kind& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector2D< kind > CTVector2D< kind >::ClampLength( const kind& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector2D< kind > CTVector2D< kind >::MoveTowards( const CTVector2D< kind >&, const CTVector2D< kind >&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector2D< kind > CTVector2D< kind >::Reflect( const CTDirectionVector2D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTVECTOR2D_TRANSCENDENTAL_MEMBERS( prefix, kind ) \
    prefix CTRadian< kind > CTVector2D< kind >::Angle( const CTVector2D< kind >&, const CTVector2D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTRadian< kind > CTVector2D< kind >::SignedAngle( const CTVector2D< kind >&, const CTVector2D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTVECTOR2D_REAL_KIND( prefix, kind ) GUCEF_MATH_CTVECTOR2D_ALL_KINDS_MEMBERS( prefix, kind ) GUCEF_MATH_CTVECTOR2D_DIVISION_MEMBERS( prefix, kind ) GUCEF_MATH_CTVECTOR2D_SQRT_MEMBERS( prefix, kind ) GUCEF_MATH_CTVECTOR2D_TRANSCENDENTAL_MEMBERS( prefix, kind )
#define GUCEF_MATH_CTVECTOR2D_INTEGER_KIND( prefix, kind ) GUCEF_MATH_CTVECTOR2D_ALL_KINDS_MEMBERS( prefix, kind )
#define GUCEF_MATH_CTVECTOR2D_EXACT_KIND( prefix, kind ) GUCEF_MATH_CTVECTOR2D_ALL_KINDS_MEMBERS( prefix, kind ) GUCEF_MATH_CTVECTOR2D_DIVISION_MEMBERS( prefix, kind )

/** Every predefined kind except the fractions (those are in gucefMATH_fractiontypes.h, which includes CBigInteger) */
#define GUCEF_MATH_CTVECTOR2D_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTVECTOR2D_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTVECTOR2D_REAL_KIND( prefix, Float64 ) \
    GUCEF_MATH_CTVECTOR2D_REAL_KIND( prefix, TFixed16_16 ) \
    GUCEF_MATH_CTVECTOR2D_REAL_KIND( prefix, TFixed32_32 ) \
    GUCEF_MATH_CTVECTOR2D_REAL_KIND( prefix, TFixed48_16 ) \
    GUCEF_MATH_CTVECTOR2D_INTEGER_KIND( prefix, Int8 ) \
    GUCEF_MATH_CTVECTOR2D_INTEGER_KIND( prefix, UInt8 ) \
    GUCEF_MATH_CTVECTOR2D_INTEGER_KIND( prefix, Int16 ) \
    GUCEF_MATH_CTVECTOR2D_INTEGER_KIND( prefix, UInt16 ) \
    GUCEF_MATH_CTVECTOR2D_INTEGER_KIND( prefix, Int32 ) \
    GUCEF_MATH_CTVECTOR2D_INTEGER_KIND( prefix, UInt32 ) \
    GUCEF_MATH_CTVECTOR2D_INTEGER_KIND( prefix, Int64 ) \
    GUCEF_MATH_CTVECTOR2D_INTEGER_KIND( prefix, UInt64 )

#define GUCEF_MATH_CTVECTOR2D_FRACTION_KINDS( prefix ) \
    GUCEF_MATH_CTVECTOR2D_EXACT_KIND( prefix, CORE::TInt64Fraction ) \
    GUCEF_MATH_CTVECTOR2D_EXACT_KIND( prefix, TMathBigFraction )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTVECTOR2D_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTVECTOR2D_H ? */
