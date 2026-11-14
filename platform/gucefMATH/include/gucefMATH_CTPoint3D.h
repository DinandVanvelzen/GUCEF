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

#ifndef GUCEF_MATH_CTPOINT3D_H
#define GUCEF_MATH_CTPOINT3D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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

template < typename T > class CTPoint2D;

/*-------------------------------------------------------------------------*/

/**
 *  A position in 3D (affine point), distinct from the displacement CTVector3D:
 *  point - point gives a vector, point +/- vector gives a point. Adding two points, scaling a point and negating a
 *  point have no meaning and do not compile; the static assertion names the reason. Combinations of points are
 *  explicit: Lerp, Midpoint, Centroid, AffineCombination (weights summing to one).
 *  Layout, kinds, tolerances, text "(x, y, z)" and DataNode attributes x, y, z as CTVector3D; size 3 * sizeof( T ),
 *  interchangeable with CORE::TPoint3DF for Float32 (see gucefMATH_CoreStructInterop.h).
 *  Every member except the text and DataNode members is realtime-safe.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTPoint3D
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef CTVector3D< T > TVector;
    typedef typename CTMathComponentTraits< T >::TKernels TKernels;

    T x;
    T y;
    T z;

    /** The origin */
    GUCEF_MATH_FORCE_INLINE CTPoint3D( void )
        : x( TTraits::Zero() ), y( TTraits::Zero() ), z( TTraits::Zero() )
    {
    }

    GUCEF_MATH_FORCE_INLINE CTPoint3D( const T& xValue, const T& yValue, const T& zValue )
        : x( xValue ), y( yValue ), z( zValue )
    {
    }

    /** The point at Origin() + positionVector */
    GUCEF_MATH_FORCE_INLINE explicit CTPoint3D( const TVector& positionVector )
        : x( positionVector.x ), y( positionVector.y ), z( positionVector.z )
    {
    }

    /** Converts from another kind component-wise (section 3.4: half to even, saturating), see CTVector3D */
    template < typename U >
    explicit CTPoint3D( const CTPoint3D< U >& other )
        : x( TTraits::Zero() ), y( TTraits::Zero() ), z( TTraits::Zero() )
    {
        TMathStatus status = CTNumericConverter< U, T >::Convert( other.x, x, CORE::ROUNDINGMODE_HALF_EVEN );
        status |= CTNumericConverter< U, T >::Convert( other.y, y, CORE::ROUNDINGMODE_HALF_EVEN );
        status |= CTNumericConverter< U, T >::Convert( other.z, z, CORE::ROUNDINGMODE_HALF_EVEN );
        GUCEF_MATH_ASSERT( 0 == ( status & MATHSTATUS_INVALID_OPERAND ) );
        (void) status;
    }

    static GUCEF_MATH_FORCE_INLINE CTPoint3D Origin( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTPoint3D(); }

    /** The vector from the origin to this point */
    GUCEF_MATH_FORCE_INLINE TVector ToPositionVector( void ) const GUCEF_MATH_REALTIME_CONTEXT { return TVector( x, y, z ); }

    /*---------------------------------------------------------------------*/
    /*  Affine operators                                                    */
    /*---------------------------------------------------------------------*/

    /** The displacement from b to a */
    friend GUCEF_MATH_FORCE_INLINE TVector operator-( const CTPoint3D& a, const CTPoint3D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return TVector( TKernels::Subtract( a.x, b.x ), TKernels::Subtract( a.y, b.y ), TKernels::Subtract( a.z, b.z ) );
    }

    friend GUCEF_MATH_FORCE_INLINE CTPoint3D operator+( const CTPoint3D& p, const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTPoint3D( TKernels::Add( p.x, v.x ), TKernels::Add( p.y, v.y ), TKernels::Add( p.z, v.z ) );
    }

    friend GUCEF_MATH_FORCE_INLINE CTPoint3D operator-( const CTPoint3D& p, const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTPoint3D( TKernels::Subtract( p.x, v.x ), TKernels::Subtract( p.y, v.y ), TKernels::Subtract( p.z, v.z ) );
    }

    GUCEF_MATH_FORCE_INLINE CTPoint3D& operator+=( const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT { *this = *this + v; return *this; }
    GUCEF_MATH_FORCE_INLINE CTPoint3D& operator-=( const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT { *this = *this - v; return *this; }

    /** Exact comparison of every component */
    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTPoint3D& a, const CTPoint3D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }

    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTPoint3D& a, const CTPoint3D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /*
     *  Operations without meaning for positions. They exist only to stop the compilation with a message that names
     *  the reason (C++98 has no deleted functions); their bodies are instantiated only when used.
     */

    friend CTPoint3D operator+( const CTPoint3D& a, const CTPoint3D& /*b*/ )
    {
        GUCEF_MATH_STATIC_ASSERT( sizeof( T ) == 0, Adding_points_is_undefined_subtract_points_to_get_a_vector );
        return a;
    }

    friend CTPoint3D operator*( const T& /*factor*/, const CTPoint3D& p )
    {
        GUCEF_MATH_STATIC_ASSERT( sizeof( T ) == 0, Scaling_a_point_is_undefined_scale_a_vector );
        return p;
    }

    friend CTPoint3D operator*( const CTPoint3D& p, const T& /*factor*/ )
    {
        GUCEF_MATH_STATIC_ASSERT( sizeof( T ) == 0, Scaling_a_point_is_undefined_scale_a_vector );
        return p;
    }

    CTPoint3D operator-( void ) const
    {
        GUCEF_MATH_STATIC_ASSERT( sizeof( T ) == 0, Negating_a_point_is_undefined_negate_a_vector );
        return *this;
    }

    /*---------------------------------------------------------------------*/
    /*  Distances and combinations of points                                */
    /*---------------------------------------------------------------------*/

    T GetSquaredDistance( const CTPoint3D& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Euclidean distance (HasSqrt), without overflow of the squared distance (see CTVector3D::GetLength) */
    T GetDistance( const CTPoint3D& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** a + ( b - a ) * t, not clamped */
    static CTPoint3D Lerp( const CTPoint3D& a, const CTPoint3D& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The point halfway between a and b (HasDivision) */
    static CTPoint3D Midpoint( const CTPoint3D& a, const CTPoint3D& b ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The mean of 'count' points (HasDivision), formed relative to the first point so that large coordinates do not
     *  cost precision. No points give the origin and assert in debug builds.
     */
    static CTPoint3D Centroid( const CTPoint3D* points, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  sum( weights[ i ] * points[ i ] ) for weights that sum to one (asserted within the default tolerances in debug
     *  builds), formed as points[ 0 ] + sum( weights[ i ] * ( points[ i ] - points[ 0 ] ) ). No points give the origin.
     */
    static CTPoint3D AffineCombination( const CTPoint3D* points, const T* weights, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT;

    /*---------------------------------------------------------------------*/
    /*  Component-wise functions and queries                                */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE CTPoint3D Min( const CTPoint3D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTPoint3D( TTraits::Min( x, other.x ), TTraits::Min( y, other.y ), TTraits::Min( z, other.z ) );
    }

    GUCEF_MATH_FORCE_INLINE CTPoint3D Max( const CTPoint3D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTPoint3D( TTraits::Max( x, other.x ), TTraits::Max( y, other.y ), TTraits::Max( z, other.z ) );
    }

    /** Component by index: 0 = x, 1 = y, 2 = z. An index out of range asserts in debug builds and gives zero */
    GUCEF_MATH_FORCE_INLINE T GetComponent( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( index < 3 );
        return index < 3 ? this->*ComponentMember( index ) : TTraits::Zero();
    }

    /** Sets a component by index. An index out of range asserts in debug builds and is ignored */
    GUCEF_MATH_FORCE_INLINE void SetComponent( UInt32 index, const T& value ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( index < 3 );
        if ( index < 3 )
            this->*ComponentMember( index ) = value;
    }

    GUCEF_MATH_FORCE_INLINE bool IsFinite( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::IsFinite( x ) && TTraits::IsFinite( y ) && TTraits::IsFinite( z );
    }

    /** Every component equal within the default tolerances of the kind */
    GUCEF_MATH_FORCE_INLINE bool Equals( const CTPoint3D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return Equals( other, TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() );
    }

    /** Every component: |a - b| <= max( absolute, relative * max( |a|, |b| ) ); NaN never equals anything */
    bool Equals( const CTPoint3D& other, const T& absolute, const T& relative ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::EqualsWithin( x, other.x, absolute, relative ) &&
               TTraits::EqualsWithin( y, other.y, absolute, relative ) &&
               TTraits::EqualsWithin( z, other.z, absolute, relative );
    }

    /** Every component within 'ulps' units in the last place (floats) or raw units (fixed point) */
    bool EqualsUlp( const CTPoint3D& other, UInt32 ulps ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_STATIC_ASSERT( TTraits::IsFloatingPoint || TTraits::IsFixedPoint, EqualsUlp_requires_a_floating_or_fixed_point_kind );
        return TTraits::EqualsUlp( x, other.x, ulps ) && TTraits::EqualsUlp( y, other.y, ulps ) && TTraits::EqualsUlp( z, other.z, ulps );
    }

    /** ( x, y ): the projection onto the XY plane */
    GUCEF_MATH_FORCE_INLINE CTPoint2D< T > XY( void ) const GUCEF_MATH_REALTIME_CONTEXT { return CTPoint2D< T >( x, y ); }

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "(x, y, z)" */
    CORE::CString ToString( void ) const
    {
        const T components[ 3 ] = { x, y, z };
        return ComponentsToString( components );
    }

    /** Parses the ToString form (white space allowed); false leaves this point unchanged */
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

    /** Reads the attributes x, y, z; a missing or invalid attribute fails and leaves this point unchanged */
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

    typedef T CTPoint3D::* TComponentMember;

    static GUCEF_MATH_FORCE_INLINE TComponentMember ComponentMember( UInt32 index ) GUCEF_MATH_REALTIME_CONTEXT
    {
        static const TComponentMember members[ 3 ] = { &CTPoint3D::x, &CTPoint3D::y, &CTPoint3D::z };
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
CTPoint3D< T >::GetSquaredDistance( const CTPoint3D& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return ( other - *this ).GetSquaredLength();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTPoint3D< T >
CTPoint3D< T >::Lerp( const CTPoint3D& a, const CTPoint3D& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    return a + ( b - a ) * t;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTPoint3D< T >
CTPoint3D< T >::AffineCombination( const CTPoint3D* points, const T* weights, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_ASSERT( GUCEF_NULL != points && GUCEF_NULL != weights && count > 0 );
    if ( GUCEF_NULL == points || GUCEF_NULL == weights || 0 == count )
        return CTPoint3D();
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
GUCEF_MATH_PRECISE_INLINE CTPoint3D< T >
CTPoint3D< T >::Midpoint( const CTPoint3D& a, const CTPoint3D& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Division_requires_a_numeric_kind_with_division );
    return a + ( b - a ) * TTraits::Half();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTPoint3D< T >
CTPoint3D< T >::Centroid( const CTPoint3D* points, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Division_requires_a_numeric_kind_with_division );
    GUCEF_MATH_ASSERT( GUCEF_NULL != points && count > 0 );
    if ( GUCEF_NULL == points || 0 == count )
        return CTPoint3D();
    TVector sum;
    for ( UInt32 i=1; i<count; ++i )
        sum += points[ i ] - points[ 0 ];
    return points[ 0 ] + sum / TTraits::FromInt32( (Int32) count );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTPoint3D< T >::GetDistance( const CTPoint3D& other ) const GUCEF_MATH_REALTIME_CONTEXT
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

#ifndef GUCEF_MATH_CTPOINT2D_H
#include "gucefMATH_CTPoint2D.h"
#define GUCEF_MATH_CTPOINT2D_H
#endif /* GUCEF_MATH_CTPOINT2D_H ? */


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

#define GUCEF_MATH_CTPOINT3D_ALL_KINDS_MEMBERS( prefix, kind ) \
    prefix kind CTPoint3D< kind >::GetSquaredDistance( const CTPoint3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTPoint3D< kind > CTPoint3D< kind >::Lerp( const CTPoint3D< kind >&, const CTPoint3D< kind >&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTPoint3D< kind > CTPoint3D< kind >::AffineCombination( const CTPoint3D< kind >*, const kind*, UInt32 ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTPOINT3D_DIVISION_MEMBERS( prefix, kind ) \
    prefix CTPoint3D< kind > CTPoint3D< kind >::Midpoint( const CTPoint3D< kind >&, const CTPoint3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTPoint3D< kind > CTPoint3D< kind >::Centroid( const CTPoint3D< kind >*, UInt32 ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTPOINT3D_SQRT_MEMBERS( prefix, kind ) \
    prefix kind CTPoint3D< kind >::GetDistance( const CTPoint3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTPOINT3D_TRANSCENDENTAL_MEMBERS( prefix, kind )

#define GUCEF_MATH_CTPOINT3D_REAL_KIND( prefix, kind ) GUCEF_MATH_CTPOINT3D_ALL_KINDS_MEMBERS( prefix, kind ) GUCEF_MATH_CTPOINT3D_DIVISION_MEMBERS( prefix, kind ) GUCEF_MATH_CTPOINT3D_SQRT_MEMBERS( prefix, kind ) GUCEF_MATH_CTPOINT3D_TRANSCENDENTAL_MEMBERS( prefix, kind )
#define GUCEF_MATH_CTPOINT3D_INTEGER_KIND( prefix, kind ) GUCEF_MATH_CTPOINT3D_ALL_KINDS_MEMBERS( prefix, kind )
#define GUCEF_MATH_CTPOINT3D_EXACT_KIND( prefix, kind ) GUCEF_MATH_CTPOINT3D_ALL_KINDS_MEMBERS( prefix, kind ) GUCEF_MATH_CTPOINT3D_DIVISION_MEMBERS( prefix, kind )

/** Every predefined kind except the fractions (those are in gucefMATH_fractiontypes.h, which includes CBigInteger) */
#define GUCEF_MATH_CTPOINT3D_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTPOINT3D_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTPOINT3D_REAL_KIND( prefix, Float64 ) \
    GUCEF_MATH_CTPOINT3D_REAL_KIND( prefix, TFixed16_16 ) \
    GUCEF_MATH_CTPOINT3D_REAL_KIND( prefix, TFixed32_32 ) \
    GUCEF_MATH_CTPOINT3D_REAL_KIND( prefix, TFixed48_16 ) \
    GUCEF_MATH_CTPOINT3D_INTEGER_KIND( prefix, Int8 ) \
    GUCEF_MATH_CTPOINT3D_INTEGER_KIND( prefix, UInt8 ) \
    GUCEF_MATH_CTPOINT3D_INTEGER_KIND( prefix, Int16 ) \
    GUCEF_MATH_CTPOINT3D_INTEGER_KIND( prefix, UInt16 ) \
    GUCEF_MATH_CTPOINT3D_INTEGER_KIND( prefix, Int32 ) \
    GUCEF_MATH_CTPOINT3D_INTEGER_KIND( prefix, UInt32 ) \
    GUCEF_MATH_CTPOINT3D_INTEGER_KIND( prefix, Int64 ) \
    GUCEF_MATH_CTPOINT3D_INTEGER_KIND( prefix, UInt64 )

#define GUCEF_MATH_CTPOINT3D_FRACTION_KINDS( prefix ) \
    GUCEF_MATH_CTPOINT3D_EXACT_KIND( prefix, CORE::TInt64Fraction ) \
    GUCEF_MATH_CTPOINT3D_EXACT_KIND( prefix, TMathBigFraction )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTPOINT3D_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTPOINT3D_H ? */
