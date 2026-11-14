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

#ifndef GUCEF_MATH_CTVECTOR4D_H
#define GUCEF_MATH_CTVECTOR4D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTNUMERICCONVERTER_H
#include "gucefMATH_CTNumericConverter.h"
#define GUCEF_MATH_CTNUMERICCONVERTER_H
#endif /* GUCEF_MATH_CTNUMERICCONVERTER_H ? */

#ifndef GUCEF_MATH_FIXEDPOINT_H
#include "gucefMATH_fixedpoint.h"
#define GUCEF_MATH_FIXEDPOINT_H
#endif /* GUCEF_MATH_FIXEDPOINT_H ? */

#ifndef GUCEF_MATH_COMPONENTFUNCTIONS_H
#include "gucefMATH_ComponentFunctions.h"
#define GUCEF_MATH_COMPONENTFUNCTIONS_H
#endif /* GUCEF_MATH_COMPONENTFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTVECTOR4OPS_H
#include "gucefMATH_CTVector4Ops.h"
#define GUCEF_MATH_CTVECTOR4OPS_H
#endif /* GUCEF_MATH_CTVECTOR4OPS_H ? */

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
template < typename T > class CTVector3D;
template < typename T > class CTPoint3D;

/*-------------------------------------------------------------------------*/

/**
 *  A 4 component vector (x, y, z, w): homogeneous coordinates and generic 4 lane data. Follows the pattern of
 *  CTVector3D; the operations that SIMD backends may accelerate (+, -, scalar *, Dot, Min, Max, Normalized) go through
 *  CTVector4Ops, whose scalar implementation defines the canonical evaluation order (Dot = ( px + pz ) + ( py + pw )).
 *  FromPoint( p ) has w = 1, FromVector( v ) has w = 0, TryGetPoint divides by w.
 *  Size 4 * sizeof( T ) (16 bytes for Float32), alignment of T: no over-alignment, arrays stay contiguous.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTVector4D
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef CTVector4Ops< T > TOps;
    typedef typename CTMathComponentTraits< T >::TKernels TKernels;
    typedef typename CTMathComponentTraits< T >::TDotWide TDotWide;

    T x;
    T y;
    T z;
    T w;

    /** The zero vector */
    GUCEF_MATH_FORCE_INLINE CTVector4D( void )
        : x( TTraits::Zero() ), y( TTraits::Zero() ), z( TTraits::Zero() ), w( TTraits::Zero() )
    {
    }

    GUCEF_MATH_FORCE_INLINE CTVector4D( const T& xValue, const T& yValue, const T& zValue, const T& wValue )
        : x( xValue ), y( yValue ), z( zValue ), w( wValue )
    {
    }

    /** Converts from another kind component-wise (section 3.4: half to even, saturating), see CTVector3D */
    template < typename U >
    explicit CTVector4D( const CTVector4D< U >& other )
        : x( TTraits::Zero() ), y( TTraits::Zero() ), z( TTraits::Zero() ), w( TTraits::Zero() )
    {
        TMathStatus status = CTNumericConverter< U, T >::Convert( other.x, x, CORE::ROUNDINGMODE_HALF_EVEN );
        status |= CTNumericConverter< U, T >::Convert( other.y, y, CORE::ROUNDINGMODE_HALF_EVEN );
        status |= CTNumericConverter< U, T >::Convert( other.z, z, CORE::ROUNDINGMODE_HALF_EVEN );
        status |= CTNumericConverter< U, T >::Convert( other.w, w, CORE::ROUNDINGMODE_HALF_EVEN );
        GUCEF_MATH_ASSERT( 0 == ( status & MATHSTATUS_INVALID_OPERAND ) );
        (void) status;
    }

    /*---------------------------------------------------------------------*/
    /*  Named constants and homogeneous factories                           */
    /*---------------------------------------------------------------------*/

    static GUCEF_MATH_FORCE_INLINE CTVector4D Zero( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector4D(); }
    static GUCEF_MATH_FORCE_INLINE CTVector4D One( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector4D( TTraits::One(), TTraits::One(), TTraits::One(), TTraits::One() ); }
    static GUCEF_MATH_FORCE_INLINE CTVector4D UnitX( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector4D( TTraits::One(), TTraits::Zero(), TTraits::Zero(), TTraits::Zero() ); }
    static GUCEF_MATH_FORCE_INLINE CTVector4D UnitY( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector4D( TTraits::Zero(), TTraits::One(), TTraits::Zero(), TTraits::Zero() ); }
    static GUCEF_MATH_FORCE_INLINE CTVector4D UnitZ( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector4D( TTraits::Zero(), TTraits::Zero(), TTraits::One(), TTraits::Zero() ); }
    static GUCEF_MATH_FORCE_INLINE CTVector4D UnitW( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTVector4D( TTraits::Zero(), TTraits::Zero(), TTraits::Zero(), TTraits::One() ); }

    /** ( p.x, p.y, p.z, 1 ): a position, translated by an affine matrix */
    static GUCEF_MATH_FORCE_INLINE CTVector4D FromPoint( const CTPoint3D< T >& point ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector4D( point.x, point.y, point.z, TTraits::One() );
    }

    /** ( v.x, v.y, v.z, 0 ): a displacement, not translated by an affine matrix */
    static GUCEF_MATH_FORCE_INLINE CTVector4D FromVector( const CTVector3D< T >& vector ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector4D( vector.x, vector.y, vector.z, TTraits::Zero() );
    }

    /** The point ( x / w, y / w, z / w ) (HasDivision); false when w is zero, 'out' unchanged */
    bool TryGetPoint( CTPoint3D< T >& out ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Division_requires_a_numeric_kind_with_division );
        if ( TTraits::Zero() == w )
            return false;
        out = CTPoint3D< T >( x / w, y / w, z / w );
        return true;
    }

    /*---------------------------------------------------------------------*/
    /*  Arithmetic operators (through CTVector4Ops)                         */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE CTVector4D operator-( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        CTVector4D result;
        TOps::Negate( *this, result );
        return result;
    }

    GUCEF_MATH_FORCE_INLINE CTVector4D operator+( void ) const GUCEF_MATH_REALTIME_CONTEXT { return *this; }

    friend GUCEF_MATH_FORCE_INLINE CTVector4D operator+( const CTVector4D& a, const CTVector4D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTVector4D result;
        TOps::Add( a, b, result );
        return result;
    }

    friend GUCEF_MATH_FORCE_INLINE CTVector4D operator-( const CTVector4D& a, const CTVector4D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTVector4D result;
        TOps::Subtract( a, b, result );
        return result;
    }

    friend GUCEF_MATH_FORCE_INLINE CTVector4D operator*( const CTVector4D& v, const T& factor ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTVector4D result;
        TOps::Multiply( v, factor, result );
        return result;
    }

    friend GUCEF_MATH_FORCE_INLINE CTVector4D operator*( const T& factor, const CTVector4D& v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector4D( TKernels::Multiply( factor, v.x ), TKernels::Multiply( factor, v.y ), TKernels::Multiply( factor, v.z ), TKernels::Multiply( factor, v.w ) );
    }

    /** Division by a scalar of the kind (HasDivision) */
    friend GUCEF_MATH_FORCE_INLINE CTVector4D operator/( const CTVector4D& v, const T& divisor ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Division_requires_a_numeric_kind_with_division );
        return CTVector4D( v.x / divisor, v.y / divisor, v.z / divisor, v.w / divisor );
    }

    GUCEF_MATH_FORCE_INLINE CTVector4D& operator+=( const CTVector4D& other ) GUCEF_MATH_REALTIME_CONTEXT { TOps::Add( *this, other, *this ); return *this; }
    GUCEF_MATH_FORCE_INLINE CTVector4D& operator-=( const CTVector4D& other ) GUCEF_MATH_REALTIME_CONTEXT { TOps::Subtract( *this, other, *this ); return *this; }
    GUCEF_MATH_FORCE_INLINE CTVector4D& operator*=( const T& factor ) GUCEF_MATH_REALTIME_CONTEXT { TOps::Multiply( *this, factor, *this ); return *this; }
    GUCEF_MATH_FORCE_INLINE CTVector4D& operator/=( const T& divisor ) GUCEF_MATH_REALTIME_CONTEXT { *this = *this / divisor; return *this; }

    /** Exact comparison of every component */
    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTVector4D& a, const CTVector4D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
    }

    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTVector4D& a, const CTVector4D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /*---------------------------------------------------------------------*/
    /*  Products and lengths                                                */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE CTVector4D ComponentMultiply( const CTVector4D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector4D( TKernels::Multiply( x, other.x ), TKernels::Multiply( y, other.y ), TKernels::Multiply( z, other.z ), TKernels::Multiply( w, other.w ) );
    }

    /** Component-wise quotient (HasDivision) */
    GUCEF_MATH_FORCE_INLINE CTVector4D ComponentDivide( const CTVector4D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Division_requires_a_numeric_kind_with_division );
        return CTVector4D( x / other.x, y / other.y, z / other.z, w / other.w );
    }

    /** ( x x' + z z' ) + ( y y' + w w' ): the canonical order of CTVector4Ops */
    T Dot( const CTVector4D& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The exact dot product of integer vectors in a wider type (see CTVector3D::DotWide) */
    TDotWide DotWide( const CTVector4D& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    T GetSquaredLength( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Euclidean length (HasSqrt) for every vector of the format (see CTVector3D::GetLength) */
    T GetLength( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    GUCEF_MATH_FORCE_INLINE T GetManhattanLength( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TKernels::Add( TKernels::Add( TTraits::Abs( x ), TTraits::Abs( y ) ), TKernels::Add( TTraits::Abs( z ), TTraits::Abs( w ) ) );
    }

    GUCEF_MATH_FORCE_INLINE T GetChebyshevLength( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::Max( TTraits::Max( TTraits::Abs( x ), TTraits::Abs( y ) ), TTraits::Max( TTraits::Abs( z ), TTraits::Abs( w ) ) );
    }

    /** This vector divided by its length (HasSqrt) for every vector of the format; zero and non-finite give zero */
    CTVector4D Normalized( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Normalized() when the length is finite and greater than minLength (>= 0); false leaves 'out' unchanged */
    bool TryNormalize( CTVector4D& out, const T& minLength ) const GUCEF_MATH_REALTIME_CONTEXT;

    /*---------------------------------------------------------------------*/
    /*  Component-wise functions                                            */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE CTVector4D Abs( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTVector4D( TTraits::Abs( x ), TTraits::Abs( y ), TTraits::Abs( z ), TTraits::Abs( w ) );
    }

    GUCEF_MATH_FORCE_INLINE CTVector4D Min( const CTVector4D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        CTVector4D result;
        TOps::Min( *this, other, result );
        return result;
    }

    GUCEF_MATH_FORCE_INLINE CTVector4D Max( const CTVector4D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        CTVector4D result;
        TOps::Max( *this, other, result );
        return result;
    }

    GUCEF_MATH_FORCE_INLINE T GetMinComponent( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::Min( TTraits::Min( x, y ), TTraits::Min( z, w ) );
    }

    GUCEF_MATH_FORCE_INLINE T GetMaxComponent( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::Max( TTraits::Max( x, y ), TTraits::Max( z, w ) );
    }

    /** Component by index: 0 = x, 1 = y, 2 = z, 3 = w. An index out of range asserts in debug builds and gives zero */
    GUCEF_MATH_FORCE_INLINE T GetComponent( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( index < 4 );
        return index < 4 ? this->*ComponentMember( index ) : TTraits::Zero();
    }

    /** Sets a component by index. An index out of range asserts in debug builds and is ignored */
    GUCEF_MATH_FORCE_INLINE void SetComponent( UInt32 index, const T& value ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( index < 4 );
        if ( index < 4 )
            this->*ComponentMember( index ) = value;
    }

    /*---------------------------------------------------------------------*/
    /*  Queries and tolerant comparison                                     */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE bool IsZero( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::Zero() == x && TTraits::Zero() == y && TTraits::Zero() == z && TTraits::Zero() == w;
    }

    GUCEF_MATH_FORCE_INLINE bool IsFinite( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::IsFinite( x ) && TTraits::IsFinite( y ) && TTraits::IsFinite( z ) && TTraits::IsFinite( w );
    }

    /** Every component equal within the default tolerances of the kind */
    GUCEF_MATH_FORCE_INLINE bool Equals( const CTVector4D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return Equals( other, TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() );
    }

    /** Every component: |a - b| <= max( absolute, relative * max( |a|, |b| ) ); NaN never equals anything */
    bool Equals( const CTVector4D& other, const T& absolute, const T& relative ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::EqualsWithin( x, other.x, absolute, relative ) &&
               TTraits::EqualsWithin( y, other.y, absolute, relative ) &&
               TTraits::EqualsWithin( z, other.z, absolute, relative ) &&
               TTraits::EqualsWithin( w, other.w, absolute, relative );
    }

    /** Every component within 'ulps' units in the last place (floats) or raw units (fixed point) */
    bool EqualsUlp( const CTVector4D& other, UInt32 ulps ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( TTraits::IsFloatingPoint || TTraits::IsFixedPoint, EqualsUlp_requires_a_floating_or_fixed_point_kind );
        return TTraits::EqualsUlp( x, other.x, ulps ) && TTraits::EqualsUlp( y, other.y, ulps ) &&
               TTraits::EqualsUlp( z, other.z, ulps ) && TTraits::EqualsUlp( w, other.w, ulps );
    }

    /*---------------------------------------------------------------------*/
    /*  Swizzles (by value)                                                 */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE CTVector2D< T > XY( void ) const GUCEF_MATH_REALTIME_CONTEXT { return CTVector2D< T >( x, y ); }
    GUCEF_MATH_FORCE_INLINE CTVector3D< T > XYZ( void ) const GUCEF_MATH_REALTIME_CONTEXT { return CTVector3D< T >( x, y, z ); }

    /** ( component I, J, K, L ), for example Swizzle< 3, 2, 1, 0 >() is ( w, z, y, x ) */
    template < UInt32 I, UInt32 J, UInt32 K, UInt32 L >
    GUCEF_MATH_FORCE_INLINE CTVector4D Swizzle( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( I < 4 && J < 4 && K < 4 && L < 4, Swizzle_index_out_of_range );
        return CTVector4D( this->*ComponentMember( I ), this->*ComponentMember( J ), this->*ComponentMember( K ), this->*ComponentMember( L ) );
    }

    /*---------------------------------------------------------------------*/
    /*  Interpolation                                                       */
    /*---------------------------------------------------------------------*/

    /** a + ( b - a ) * t, not clamped */
    static CTVector4D Lerp( const CTVector4D& a, const CTVector4D& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT;

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "(x, y, z, w)" */
    CORE::CString ToString( void ) const
    {
        const T components[ 4 ] = { x, y, z, w };
        return ComponentsToString( components );
    }

    /** Parses the ToString form (white space allowed); false leaves this vector unchanged */
    bool FromString( const CORE::CString& text )
    {
        T components[ 4 ];
        if ( !ComponentsFromString( text, components ) )
            return false;
        x = components[ 0 ];
        y = components[ 1 ];
        z = components[ 2 ];
        w = components[ 3 ];
        return true;
    }

    /** Writes the attributes x, y, z, w into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ ) const
    {
        const char* const names[ 4 ] = { "x", "y", "z", "w" };
        const T components[ 4 ] = { x, y, z, w };
        return SerializeComponents( node, names, components );
    }

    /** Reads the attributes x, y, z, w; a missing or invalid attribute fails and leaves this vector unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ )
    {
        const char* const names[ 4 ] = { "x", "y", "z", "w" };
        T components[ 4 ];
        if ( !DeserializeComponents( node, names, components ) )
            return false;
        x = components[ 0 ];
        y = components[ 1 ];
        z = components[ 2 ];
        w = components[ 3 ];
        return true;
    }

    private:

    typedef T CTVector4D::* TComponentMember;

    static GUCEF_MATH_FORCE_INLINE TComponentMember ComponentMember( UInt32 index ) GUCEF_MATH_REALTIME_CONTEXT
    {
        static const TComponentMember members[ 4 ] = { &CTVector4D::x, &CTVector4D::y, &CTVector4D::z, &CTVector4D::w };
        return members[ index ];
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** a . b in the canonical order of CTVector4Ops */
template < typename T >
GUCEF_MATH_FORCE_INLINE T
Dot( const CTVector4D< T >& a, const CTVector4D< T >& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    return a.Dot( b );
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
CTVector4D< T >::Dot( const CTVector4D& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return TOps::Dot( *this, other );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE typename CTVector4D< T >::TDotWide
CTVector4D< T >::DotWide( const CTVector4D& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const T a[ 4 ] = { x, y, z, w };
    const T b[ 4 ] = { other.x, other.y, other.z, other.w };
    return TKernels::DotWide( a, b );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTVector4D< T >::GetSquaredLength( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return Dot( *this );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector4D< T >
CTVector4D< T >::Lerp( const CTVector4D& a, const CTVector4D& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return a + ( b - a ) * t;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTVector4D< T >::GetLength( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, Length_requires_a_numeric_kind_with_sqrt );
    const T a[ 4 ] = { x, y, z, w };
    return TKernels::Length( a );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector4D< T >
CTVector4D< T >::Normalized( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, Normalization_requires_a_numeric_kind_with_sqrt );
    CTVector4D result;
    TOps::Normalize( *this, result );
    return result;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTVector4D< T >::TryNormalize( CTVector4D& out, const T& minLength ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, Normalization_requires_a_numeric_kind_with_sqrt );
    T length = GetLength();
    if ( !TTraits::IsFinite( length ) || !( minLength < length ) )
        return false;
    CTVector4D unit;
    TOps::Normalize( *this, unit );
    if ( unit.IsZero() )
        return false;
    out = unit;
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

#ifndef GUCEF_MATH_CTVECTOR3D_H
#include "gucefMATH_CTVector3D.h"
#define GUCEF_MATH_CTVECTOR3D_H
#endif /* GUCEF_MATH_CTVECTOR3D_H ? */

#ifndef GUCEF_MATH_CTPOINT3D_H
#include "gucefMATH_CTPoint3D.h"
#define GUCEF_MATH_CTPOINT3D_H
#endif /* GUCEF_MATH_CTPOINT3D_H ? */


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

#define GUCEF_MATH_CTVECTOR4D_ALL_KINDS_MEMBERS( prefix, kind ) \
    prefix kind CTVector4D< kind >::Dot( const CTVector4D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector4D< kind >::TDotWide CTVector4D< kind >::DotWide( const CTVector4D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix kind CTVector4D< kind >::GetSquaredLength( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector4D< kind > CTVector4D< kind >::Lerp( const CTVector4D< kind >&, const CTVector4D< kind >&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTVECTOR4D_DIVISION_MEMBERS( prefix, kind )

#define GUCEF_MATH_CTVECTOR4D_SQRT_MEMBERS( prefix, kind ) \
    prefix kind CTVector4D< kind >::GetLength( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector4D< kind > CTVector4D< kind >::Normalized( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTVector4D< kind >::TryNormalize( CTVector4D< kind >&, const kind& ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTVECTOR4D_TRANSCENDENTAL_MEMBERS( prefix, kind )

#define GUCEF_MATH_CTVECTOR4D_REAL_KIND( prefix, kind ) GUCEF_MATH_CTVECTOR4D_ALL_KINDS_MEMBERS( prefix, kind ) GUCEF_MATH_CTVECTOR4D_DIVISION_MEMBERS( prefix, kind ) GUCEF_MATH_CTVECTOR4D_SQRT_MEMBERS( prefix, kind ) GUCEF_MATH_CTVECTOR4D_TRANSCENDENTAL_MEMBERS( prefix, kind )
#define GUCEF_MATH_CTVECTOR4D_INTEGER_KIND( prefix, kind ) GUCEF_MATH_CTVECTOR4D_ALL_KINDS_MEMBERS( prefix, kind )
#define GUCEF_MATH_CTVECTOR4D_EXACT_KIND( prefix, kind ) GUCEF_MATH_CTVECTOR4D_ALL_KINDS_MEMBERS( prefix, kind ) GUCEF_MATH_CTVECTOR4D_DIVISION_MEMBERS( prefix, kind )

/** Every predefined kind except the fractions (those are in gucefMATH_fractiontypes.h, which includes CBigInteger) */
#define GUCEF_MATH_CTVECTOR4D_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTVECTOR4D_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTVECTOR4D_REAL_KIND( prefix, Float64 ) \
    GUCEF_MATH_CTVECTOR4D_REAL_KIND( prefix, TFixed16_16 ) \
    GUCEF_MATH_CTVECTOR4D_REAL_KIND( prefix, TFixed32_32 ) \
    GUCEF_MATH_CTVECTOR4D_REAL_KIND( prefix, TFixed48_16 ) \
    GUCEF_MATH_CTVECTOR4D_INTEGER_KIND( prefix, Int8 ) \
    GUCEF_MATH_CTVECTOR4D_INTEGER_KIND( prefix, UInt8 ) \
    GUCEF_MATH_CTVECTOR4D_INTEGER_KIND( prefix, Int16 ) \
    GUCEF_MATH_CTVECTOR4D_INTEGER_KIND( prefix, UInt16 ) \
    GUCEF_MATH_CTVECTOR4D_INTEGER_KIND( prefix, Int32 ) \
    GUCEF_MATH_CTVECTOR4D_INTEGER_KIND( prefix, UInt32 ) \
    GUCEF_MATH_CTVECTOR4D_INTEGER_KIND( prefix, Int64 ) \
    GUCEF_MATH_CTVECTOR4D_INTEGER_KIND( prefix, UInt64 )

#define GUCEF_MATH_CTVECTOR4D_FRACTION_KINDS( prefix ) \
    GUCEF_MATH_CTVECTOR4D_EXACT_KIND( prefix, CORE::TInt64Fraction ) \
    GUCEF_MATH_CTVECTOR4D_EXACT_KIND( prefix, TMathBigFraction )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTVECTOR4D_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTVECTOR4D_H ? */
