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

#ifndef GUCEF_MATH_CTPOINT2D_H
#define GUCEF_MATH_CTPOINT2D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTVECTOR2D_H
#include "gucefMATH_CTVector2D.h"
#define GUCEF_MATH_CTVECTOR2D_H
#endif /* GUCEF_MATH_CTVECTOR2D_H ? */

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

template < typename T > class CTPoint2D;

/*-------------------------------------------------------------------------*/

/**
 *  A position in 2D (affine point), distinct from the displacement CTVector2D:
 *  point - point gives a vector, point +/- vector gives a point. Adding two points, scaling a point and negating a
 *  point have no meaning and do not compile; the static assertion names the reason. Combinations of points are
 *  explicit: Lerp, Midpoint, Centroid, AffineCombination (weights summing to one).
 *  Layout, kinds, tolerances, text "(x, y)" and DataNode attributes x, y as CTVector2D; size 2 * sizeof( T ),
 *  interchangeable with CORE::TPoint2DF for Float32 (see gucefMATH_CoreStructInterop.h).
 *  Every member except the text and DataNode members is realtime-safe.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTPoint2D
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef CTVector2D< T > TVector;
    typedef typename CTMathComponentTraits< T >::TKernels TKernels;

    T x;
    T y;

    /** The origin */
    GUCEF_MATH_FORCE_INLINE CTPoint2D( void )
        : x( TTraits::Zero() ), y( TTraits::Zero() )
    {
    }

    GUCEF_MATH_FORCE_INLINE CTPoint2D( const T& xValue, const T& yValue )
        : x( xValue ), y( yValue )
    {
    }

    /** The point at Origin() + positionVector */
    GUCEF_MATH_FORCE_INLINE explicit CTPoint2D( const TVector& positionVector )
        : x( positionVector.x ), y( positionVector.y )
    {
    }

    /** Converts from another kind component-wise (section 3.4: half to even, saturating), see CTVector2D */
    template < typename U >
    explicit CTPoint2D( const CTPoint2D< U >& other )
        : x( TTraits::Zero() ), y( TTraits::Zero() )
    {
        TMathStatus status = CTNumericConverter< U, T >::Convert( other.x, x, CORE::ROUNDINGMODE_HALF_EVEN );
        status |= CTNumericConverter< U, T >::Convert( other.y, y, CORE::ROUNDINGMODE_HALF_EVEN );
        GUCEF_MATH_ASSERT( 0 == ( status & MATHSTATUS_INVALID_OPERAND ) );
        (void) status;
    }

    static GUCEF_MATH_FORCE_INLINE CTPoint2D Origin( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTPoint2D(); }

    /** The vector from the origin to this point */
    GUCEF_MATH_FORCE_INLINE TVector ToPositionVector( void ) const GUCEF_MATH_REALTIME_CONTEXT { return TVector( x, y ); }

    /*---------------------------------------------------------------------*/
    /*  Affine operators                                                    */
    /*---------------------------------------------------------------------*/

    /** The displacement from b to a */
    friend GUCEF_MATH_FORCE_INLINE TVector operator-( const CTPoint2D& a, const CTPoint2D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return TVector( TKernels::Subtract( a.x, b.x ), TKernels::Subtract( a.y, b.y ) );
    }

    friend GUCEF_MATH_FORCE_INLINE CTPoint2D operator+( const CTPoint2D& p, const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTPoint2D( TKernels::Add( p.x, v.x ), TKernels::Add( p.y, v.y ) );
    }

    friend GUCEF_MATH_FORCE_INLINE CTPoint2D operator-( const CTPoint2D& p, const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTPoint2D( TKernels::Subtract( p.x, v.x ), TKernels::Subtract( p.y, v.y ) );
    }

    GUCEF_MATH_FORCE_INLINE CTPoint2D& operator+=( const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT { *this = *this + v; return *this; }
    GUCEF_MATH_FORCE_INLINE CTPoint2D& operator-=( const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT { *this = *this - v; return *this; }

    /** Exact comparison of every component */
    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTPoint2D& a, const CTPoint2D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.x == b.x && a.y == b.y;
    }

    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTPoint2D& a, const CTPoint2D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /*
     *  Operations without meaning for positions. They exist only to stop the compilation with a message that names
     *  the reason (C++98 has no deleted functions); their bodies are instantiated only when used.
     */

    friend CTPoint2D operator+( const CTPoint2D& a, const CTPoint2D& /*b*/ )
    {
        GUCEF_MATH_STATIC_ASSERT( sizeof( T ) == 0, Adding_points_is_undefined_subtract_points_to_get_a_vector );
        return a;
    }

    friend CTPoint2D operator*( const T& /*factor*/, const CTPoint2D& p )
    {
        GUCEF_MATH_STATIC_ASSERT( sizeof( T ) == 0, Scaling_a_point_is_undefined_scale_a_vector );
        return p;
    }

    friend CTPoint2D operator*( const CTPoint2D& p, const T& /*factor*/ )
    {
        GUCEF_MATH_STATIC_ASSERT( sizeof( T ) == 0, Scaling_a_point_is_undefined_scale_a_vector );
        return p;
    }

    CTPoint2D operator-( void ) const
    {
        GUCEF_MATH_STATIC_ASSERT( sizeof( T ) == 0, Negating_a_point_is_undefined_negate_a_vector );
        return *this;
    }

    /*---------------------------------------------------------------------*/
    /*  Distances and combinations of points                                */
    /*---------------------------------------------------------------------*/

    T GetSquaredDistance( const CTPoint2D& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Euclidean distance (HasSqrt), without overflow of the squared distance (see CTVector2D::GetLength) */
    T GetDistance( const CTPoint2D& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** a + ( b - a ) * t, not clamped */
    static CTPoint2D Lerp( const CTPoint2D& a, const CTPoint2D& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The point halfway between a and b (HasDivision) */
    static CTPoint2D Midpoint( const CTPoint2D& a, const CTPoint2D& b ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The mean of 'count' points (HasDivision), formed relative to the first point so that large coordinates do not
     *  cost precision. No points give the origin and assert in debug builds.
     */
    static CTPoint2D Centroid( const CTPoint2D* points, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  sum( weights[ i ] * points[ i ] ) for weights that sum to one (asserted within the default tolerances in debug
     *  builds), formed as points[ 0 ] + sum( weights[ i ] * ( points[ i ] - points[ 0 ] ) ). No points give the origin.
     */
    static CTPoint2D AffineCombination( const CTPoint2D* points, const T* weights, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT;

    /*---------------------------------------------------------------------*/
    /*  Component-wise functions and queries                                */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE CTPoint2D Min( const CTPoint2D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTPoint2D( TTraits::Min( x, other.x ), TTraits::Min( y, other.y ) );
    }

    GUCEF_MATH_FORCE_INLINE CTPoint2D Max( const CTPoint2D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTPoint2D( TTraits::Max( x, other.x ), TTraits::Max( y, other.y ) );
    }

    /** Component by index: 0 = x, 1 = y. An index out of range asserts in debug builds and gives zero */
    GUCEF_MATH_FORCE_INLINE T GetComponent( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( index < 2 );
        return index < 2 ? this->*ComponentMember( index ) : TTraits::Zero();
    }

    /** Sets a component by index. An index out of range asserts in debug builds and is ignored */
    GUCEF_MATH_FORCE_INLINE void SetComponent( UInt32 index, const T& value ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( index < 2 );
        if ( index < 2 )
            this->*ComponentMember( index ) = value;
    }

    GUCEF_MATH_FORCE_INLINE bool IsFinite( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::IsFinite( x ) && TTraits::IsFinite( y );
    }

    /** Every component equal within the default tolerances of the kind */
    GUCEF_MATH_FORCE_INLINE bool Equals( const CTPoint2D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return Equals( other, TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() );
    }

    /** Every component: |a - b| <= max( absolute, relative * max( |a|, |b| ) ); NaN never equals anything */
    bool Equals( const CTPoint2D& other, const T& absolute, const T& relative ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::EqualsWithin( x, other.x, absolute, relative ) &&
               TTraits::EqualsWithin( y, other.y, absolute, relative );
    }

    /** Every component within 'ulps' units in the last place (floats) or raw units (fixed point) */
    bool EqualsUlp( const CTPoint2D& other, UInt32 ulps ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( TTraits::IsFloatingPoint || TTraits::IsFixedPoint, EqualsUlp_requires_a_floating_or_fixed_point_kind );
        return TTraits::EqualsUlp( x, other.x, ulps ) && TTraits::EqualsUlp( y, other.y, ulps );
    }

    /** ( x, y, z ): the point in the plane z of 3D space */
    GUCEF_MATH_FORCE_INLINE CTPoint3D< T > ToPoint3D( const T& z ) const GUCEF_MATH_REALTIME_CONTEXT { return CTPoint3D< T >( x, y, z ); }

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "(x, y)" */
    CORE::CString ToString( void ) const
    {
        const T components[ 2 ] = { x, y };
        return ComponentsToString( components );
    }

    /** Parses the ToString form (white space allowed); false leaves this point unchanged */
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

    /** Reads the attributes x, y; a missing or invalid attribute fails and leaves this point unchanged */
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

    typedef T CTPoint2D::* TComponentMember;

    static GUCEF_MATH_FORCE_INLINE TComponentMember ComponentMember( UInt32 index ) GUCEF_MATH_REALTIME_CONTEXT
    {
        static const TComponentMember members[ 2 ] = { &CTPoint2D::x, &CTPoint2D::y };
        return members[ index ];
    }
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

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTPoint2D< T >::GetSquaredDistance( const CTPoint2D& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return ( other - *this ).GetSquaredLength();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTPoint2D< T >
CTPoint2D< T >::Lerp( const CTPoint2D& a, const CTPoint2D& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return a + ( b - a ) * t;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTPoint2D< T >
CTPoint2D< T >::AffineCombination( const CTPoint2D* points, const T* weights, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_ASSERT( GUCEF_NULL != points && GUCEF_NULL != weights && count > 0 );
    if ( GUCEF_NULL == points || GUCEF_NULL == weights || 0 == count )
        return CTPoint2D();
    T weightSum = weights[ 0 ];
    TVector sum;
    for ( UInt32 i=1; i<count; ++i )
    {
        sum += ( points[ i ] - points[ 0 ] ) * weights[ i ];
        weightSum = weightSum + weights[ i ];
    }
    GUCEF_MATH_ASSERT( TTraits::EqualsWithin( weightSum, TTraits::One(), TTraits::DefaultAbsoluteTolerance() * TTraits::FromInt32( (Int32) count ), TTraits::DefaultRelativeTolerance() ) );
    (void) weightSum;
    return points[ 0 ] + sum;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTPoint2D< T >
CTPoint2D< T >::Midpoint( const CTPoint2D& a, const CTPoint2D& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Division_requires_a_numeric_kind_with_division );
    return a + ( b - a ) * TTraits::Half();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTPoint2D< T >
CTPoint2D< T >::Centroid( const CTPoint2D* points, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Division_requires_a_numeric_kind_with_division );
    GUCEF_MATH_ASSERT( GUCEF_NULL != points && count > 0 );
    if ( GUCEF_NULL == points || 0 == count )
        return CTPoint2D();
    TVector sum;
    for ( UInt32 i=1; i<count; ++i )
        sum += points[ i ] - points[ 0 ];
    return points[ 0 ] + sum / TTraits::FromInt32( (Int32) count );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTPoint2D< T >::GetDistance( const CTPoint2D& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return ( other - *this ).GetLength();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

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

#define GUCEF_MATH_CTPOINT2D_ALL_KINDS_MEMBERS( prefix, kind ) \
    prefix kind CTPoint2D< kind >::GetSquaredDistance( const CTPoint2D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTPoint2D< kind > CTPoint2D< kind >::Lerp( const CTPoint2D< kind >&, const CTPoint2D< kind >&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTPoint2D< kind > CTPoint2D< kind >::AffineCombination( const CTPoint2D< kind >*, const kind*, UInt32 ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTPOINT2D_DIVISION_MEMBERS( prefix, kind ) \
    prefix CTPoint2D< kind > CTPoint2D< kind >::Midpoint( const CTPoint2D< kind >&, const CTPoint2D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTPoint2D< kind > CTPoint2D< kind >::Centroid( const CTPoint2D< kind >*, UInt32 ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTPOINT2D_SQRT_MEMBERS( prefix, kind ) \
    prefix kind CTPoint2D< kind >::GetDistance( const CTPoint2D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTPOINT2D_TRANSCENDENTAL_MEMBERS( prefix, kind )

#define GUCEF_MATH_CTPOINT2D_REAL_KIND( prefix, kind ) GUCEF_MATH_CTPOINT2D_ALL_KINDS_MEMBERS( prefix, kind ) GUCEF_MATH_CTPOINT2D_DIVISION_MEMBERS( prefix, kind ) GUCEF_MATH_CTPOINT2D_SQRT_MEMBERS( prefix, kind ) GUCEF_MATH_CTPOINT2D_TRANSCENDENTAL_MEMBERS( prefix, kind )
#define GUCEF_MATH_CTPOINT2D_INTEGER_KIND( prefix, kind ) GUCEF_MATH_CTPOINT2D_ALL_KINDS_MEMBERS( prefix, kind )
#define GUCEF_MATH_CTPOINT2D_EXACT_KIND( prefix, kind ) GUCEF_MATH_CTPOINT2D_ALL_KINDS_MEMBERS( prefix, kind ) GUCEF_MATH_CTPOINT2D_DIVISION_MEMBERS( prefix, kind )

/** Every predefined kind except the fractions (those are in gucefMATH_fractiontypes.h, which includes CBigInteger) */
#define GUCEF_MATH_CTPOINT2D_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTPOINT2D_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTPOINT2D_REAL_KIND( prefix, Float64 ) \
    GUCEF_MATH_CTPOINT2D_REAL_KIND( prefix, TFixed16_16 ) \
    GUCEF_MATH_CTPOINT2D_REAL_KIND( prefix, TFixed32_32 ) \
    GUCEF_MATH_CTPOINT2D_REAL_KIND( prefix, TFixed48_16 ) \
    GUCEF_MATH_CTPOINT2D_INTEGER_KIND( prefix, Int8 ) \
    GUCEF_MATH_CTPOINT2D_INTEGER_KIND( prefix, UInt8 ) \
    GUCEF_MATH_CTPOINT2D_INTEGER_KIND( prefix, Int16 ) \
    GUCEF_MATH_CTPOINT2D_INTEGER_KIND( prefix, UInt16 ) \
    GUCEF_MATH_CTPOINT2D_INTEGER_KIND( prefix, Int32 ) \
    GUCEF_MATH_CTPOINT2D_INTEGER_KIND( prefix, UInt32 ) \
    GUCEF_MATH_CTPOINT2D_INTEGER_KIND( prefix, Int64 ) \
    GUCEF_MATH_CTPOINT2D_INTEGER_KIND( prefix, UInt64 )

#define GUCEF_MATH_CTPOINT2D_FRACTION_KINDS( prefix ) \
    GUCEF_MATH_CTPOINT2D_EXACT_KIND( prefix, CORE::TInt64Fraction ) \
    GUCEF_MATH_CTPOINT2D_EXACT_KIND( prefix, TMathBigFraction )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTPOINT2D_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTPOINT2D_H ? */
