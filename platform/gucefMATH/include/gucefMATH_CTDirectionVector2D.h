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

#ifndef GUCEF_MATH_CTDIRECTIONVECTOR2D_H
#define GUCEF_MATH_CTDIRECTIONVECTOR2D_H

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

/**
 *  A unit direction in 2D, following CTDirectionVector3D (private data, invariant, implicit conversion to the vector,
 *  directions only from normalisation or FromNormalized, no sum or difference of directions).
 *  2D specifics: the default direction and the result for a zero or non-finite input is Right() (+X, angle 0);
 *  FromAngle / GetAngle convert from and to the counter-clockwise angle from +X; GetPerpendicular() turns a quarter
 *  turn counter-clockwise; Cross() is the scalar sine of the signed angle.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTDirectionVector2D
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef CTVector2D< T > TVector;

    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, Direction_vectors_require_a_numeric_kind_with_sqrt );

    /** Right(): ( 1, 0 ) */
    GUCEF_MATH_FORCE_INLINE CTDirectionVector2D( void )
        : m_x( TTraits::One() ), m_y( TTraits::Zero() )
    {
    }

    /** The direction of v (see CTDirectionVector3D); a zero or non-finite v gives Right() and asserts in debug builds */
    explicit CTDirectionVector2D( const TVector& v )
        : m_x( TTraits::One() ), m_y( TTraits::Zero() )
    {
        bool created = TryCreate( v, *this );
        GUCEF_MATH_ASSERT( created );
        (void) created;
    }

    /** The direction of v; false for a zero or non-finite v, 'out' unchanged */
    static bool TryCreate( const TVector& v, CTDirectionVector2D& out ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Adopts a vector that is already of unit length, without normalising; the invariant is asserted in debug builds */
    static GUCEF_MATH_FORCE_INLINE CTDirectionVector2D FromNormalized( const TVector& unit ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( IsUnitLength( unit ) );
        return CTDirectionVector2D( unit.x, unit.y );
    }

    /** ( cos angle, sin angle ): the direction at 'angle' counter-clockwise from +X (HasTranscendentals) */
    static CTDirectionVector2D FromAngle( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT;

    /** True when the length of v is 1 within GetUnitLengthTolerance() */
    static bool IsUnitLength( const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The tolerance of the invariant: 4 times the default relative tolerance (floats), 2 raw units (fixed point) */
    static GUCEF_MATH_FORCE_INLINE T GetUnitLengthTolerance( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::IsFloatingPoint ? TTraits::DefaultRelativeTolerance() * TTraits::FromInt32( 4 ) : TTraits::Epsilon() * TTraits::FromInt32( 2 );
    }

    /*---------------------------------------------------------------------*/
    /*  Named constants                                                     */
    /*---------------------------------------------------------------------*/

    static GUCEF_MATH_FORCE_INLINE CTDirectionVector2D UnitX( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTDirectionVector2D( TTraits::One(), TTraits::Zero() ); }
    static GUCEF_MATH_FORCE_INLINE CTDirectionVector2D UnitY( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTDirectionVector2D( TTraits::Zero(), TTraits::One() ); }

    /** +X */
    static GUCEF_MATH_FORCE_INLINE CTDirectionVector2D Right( void ) GUCEF_MATH_REALTIME_CONTEXT { return UnitX(); }
    /** -X */
    static GUCEF_MATH_FORCE_INLINE CTDirectionVector2D Left( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTDirectionVector2D( ComponentNegate( TTraits::One() ), TTraits::Zero() ); }
    /** +Y */
    static GUCEF_MATH_FORCE_INLINE CTDirectionVector2D Up( void ) GUCEF_MATH_REALTIME_CONTEXT { return UnitY(); }
    /** -Y */
    static GUCEF_MATH_FORCE_INLINE CTDirectionVector2D Down( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTDirectionVector2D( TTraits::Zero(), ComponentNegate( TTraits::One() ) ); }

    /*---------------------------------------------------------------------*/
    /*  Access                                                              */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE const T& GetX( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_x; }
    GUCEF_MATH_FORCE_INLINE const T& GetY( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_y; }

    GUCEF_MATH_FORCE_INLINE TVector AsVector( void ) const GUCEF_MATH_REALTIME_CONTEXT { return TVector( m_x, m_y ); }

    /** A direction is a vector (the converse is never implicit) */
    GUCEF_MATH_FORCE_INLINE operator TVector( void ) const GUCEF_MATH_REALTIME_CONTEXT { return AsVector(); }

    /** The counter-clockwise angle from +X in ( -pi, pi ] (HasTranscendentals) */
    CTRadian< T > GetAngle( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /*---------------------------------------------------------------------*/
    /*  Operators                                                           */
    /*---------------------------------------------------------------------*/

    /** The opposite direction */
    GUCEF_MATH_FORCE_INLINE CTDirectionVector2D operator-( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTDirectionVector2D( ComponentNegate( m_x ), ComponentNegate( m_y ) );
    }

    /** The vector of length 'factor' along this direction */
    friend GUCEF_MATH_FORCE_INLINE TVector operator*( const CTDirectionVector2D& d, const T& factor ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return TVector( d.m_x * factor, d.m_y * factor );
    }

    friend GUCEF_MATH_FORCE_INLINE TVector operator*( const T& factor, const CTDirectionVector2D& d ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return TVector( factor * d.m_x, factor * d.m_y );
    }

    /** Exact comparison of every component */
    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTDirectionVector2D& a, const CTDirectionVector2D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.m_x == b.m_x && a.m_y == b.m_y;
    }

    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTDirectionVector2D& a, const CTDirectionVector2D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /* The sum and difference of two directions are not directions (see CTDirectionVector3D) */

    friend CTDirectionVector2D operator+( const CTDirectionVector2D& a, const CTDirectionVector2D& /*b*/ )
    {
        GUCEF_MATH_STATIC_ASSERT( sizeof( T ) == 0, Adding_directions_is_undefined_use_AsVector );
        return a;
    }

    friend CTDirectionVector2D operator-( const CTDirectionVector2D& a, const CTDirectionVector2D& /*b*/ )
    {
        GUCEF_MATH_STATIC_ASSERT( sizeof( T ) == 0, Subtracting_directions_is_undefined_use_AsVector );
        return a;
    }

    /*---------------------------------------------------------------------*/
    /*  Products, angles and interpolation                                  */
    /*---------------------------------------------------------------------*/

    /** The dot product with a vector or a direction */
    T Dot( const TVector& v ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** x v.y - y v.x: for two directions the sine of the signed angle */
    T Cross( const TVector& v ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** This direction turned a quarter turn counter-clockwise: ( -y, x ) */
    GUCEF_MATH_FORCE_INLINE CTDirectionVector2D GetPerpendicular( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTDirectionVector2D( ComponentNegate( m_y ), m_x );
    }

    /** Every component equal within the default tolerances of the kind */
    GUCEF_MATH_FORCE_INLINE bool Equals( const CTDirectionVector2D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return AsVector().Equals( other.AsVector() );
    }

    /** The angle between a and b in [0, pi] */
    static CTRadian< T > Angle( const CTDirectionVector2D& a, const CTDirectionVector2D& b ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The rotation from 'from' to 'to' in ( -pi, pi ], positive counter-clockwise */
    static CTRadian< T > SignedAngle( const CTDirectionVector2D& from, const CTDirectionVector2D& to ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The direction at fraction t of the shorter arc from a to b with constant angular speed (HasTranscendentals);
     *  opposite directions turn counter-clockwise. The result is renormalised, so the invariant holds for every t.
     */
    static CTDirectionVector2D Slerp( const CTDirectionVector2D& a, const CTDirectionVector2D& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT;

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "(x, y)" */
    CORE::CString ToString( void ) const
    {
        return AsVector().ToString();
    }

    /** Parses the ToString form; false (this direction unchanged) for malformed text or a vector that is not unit length */
    bool FromString( const CORE::CString& text )
    {
        TVector v;
        if ( !v.FromString( text ) || !IsUnitLength( v ) )
            return false;
        m_x = v.x;
        m_y = v.y;
        return true;
    }

    /** Writes the attributes x, y into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        return AsVector().Serialize( node, settings );
    }

    /** Reads the attributes x, y; fails (this direction unchanged) when one is missing or the vector is not unit length */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        TVector v;
        if ( !v.Deserialize( node, settings ) || !IsUnitLength( v ) )
            return false;
        m_x = v.x;
        m_y = v.y;
        return true;
    }

    private:

    GUCEF_MATH_FORCE_INLINE CTDirectionVector2D( const T& xValue, const T& yValue )
        : m_x( xValue ), m_y( yValue )
    {
    }

    T m_x;
    T m_y;
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
GUCEF_MATH_PRECISE_INLINE bool
CTDirectionVector2D< T >::TryCreate( const TVector& v, CTDirectionVector2D& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !v.IsFinite() || v.IsZero() )
        return false;
    TVector unit = v.Normalized();
    if ( unit.IsZero() )
        return false;
    out.m_x = unit.x;
    out.m_y = unit.y;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTDirectionVector2D< T >
CTDirectionVector2D< T >::FromAngle( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasTranscendentals, Angle_requires_transcendentals );
    CTDirectionVector2D result;
    TryCreate( TVector( TTraits::Cos( angle.GetValue() ), TTraits::Sin( angle.GetValue() ) ), result );
    return result;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTDirectionVector2D< T >::IsUnitLength( const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !v.IsFinite() )
        return false;
    return TTraits::Abs( v.GetLength() - TTraits::One() ) <= GetUnitLengthTolerance();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTRadian< T >
CTDirectionVector2D< T >::GetAngle( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasTranscendentals, Angle_requires_transcendentals );
    return CTRadian< T >( TTraits::Atan2( m_y, m_x ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTDirectionVector2D< T >::Dot( const TVector& v ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return AsVector().Dot( v );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTDirectionVector2D< T >::Cross( const TVector& v ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return m_x * v.y - m_y * v.x;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTDirectionVector2D< T >
CTDirectionVector2D< T >::Slerp( const CTDirectionVector2D& a, const CTDirectionVector2D& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasTranscendentals, Slerp_requires_transcendentals );
    T angle = TTraits::Atan2( a.Cross( b.AsVector() ), a.Dot( b.AsVector() ) ) * t;
    TVector va = a.AsVector();
    TVector perpendicular = a.GetPerpendicular().AsVector();
    CTDirectionVector2D result = a;
    TryCreate( va * TTraits::Cos( angle ) + perpendicular * TTraits::Sin( angle ), result );
    return result;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTRadian< T >
CTDirectionVector2D< T >::Angle( const CTDirectionVector2D& a, const CTDirectionVector2D& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    return TVector::Angle( a.AsVector(), b.AsVector() );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTRadian< T >
CTDirectionVector2D< T >::SignedAngle( const CTDirectionVector2D& from, const CTDirectionVector2D& to ) GUCEF_MATH_REALTIME_CONTEXT
{
    return TVector::SignedAngle( from.AsVector(), to.AsVector() );
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
 *  The precision-sensitive members of the predefined kinds are instantiated in the module and declared extern here
 *  (section 5.10 of the design), member by member: a whole-class instantiation would also instantiate the members
 *  that a kind does not support, whose static assertions must stay compile errors in user code. The groups:
 *  ALL_KINDS for every kind, DIVISION for kinds with division, SQRT for kinds with a square root,
 *  TRANSCENDENTAL for kinds with transcendental functions. A list macro takes the prefix (GUCEF_MATH_EXTERN_TEMPLATE
 *  in headers, GUCEF_MATH_INSTANTIATE_TEMPLATE in the module source) and the kind (a name without commas).
 */

#define GUCEF_MATH_CTDIRECTIONVECTOR2D_ALL_KINDS_MEMBERS( prefix, kind ) \
    prefix bool CTDirectionVector2D< kind >::TryCreate( const CTVector2D< kind >&, CTDirectionVector2D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTDirectionVector2D< kind > CTDirectionVector2D< kind >::FromAngle( const CTRadian< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTDirectionVector2D< kind >::IsUnitLength( const CTVector2D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTRadian< kind > CTDirectionVector2D< kind >::GetAngle( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix kind CTDirectionVector2D< kind >::Dot( const CTVector2D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix kind CTDirectionVector2D< kind >::Cross( const CTVector2D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTDirectionVector2D< kind > CTDirectionVector2D< kind >::Slerp( const CTDirectionVector2D< kind >&, const CTDirectionVector2D< kind >&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTRadian< kind > CTDirectionVector2D< kind >::Angle( const CTDirectionVector2D< kind >&, const CTDirectionVector2D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTRadian< kind > CTDirectionVector2D< kind >::SignedAngle( const CTDirectionVector2D< kind >&, const CTDirectionVector2D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTDIRECTIONVECTOR2D_REAL_KIND( prefix, kind ) GUCEF_MATH_CTDIRECTIONVECTOR2D_ALL_KINDS_MEMBERS( prefix, kind )

/** Every predefined kind except the fractions (those are in gucefMATH_fractiontypes.h, which includes CBigInteger) */
#define GUCEF_MATH_CTDIRECTIONVECTOR2D_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTDIRECTIONVECTOR2D_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTDIRECTIONVECTOR2D_REAL_KIND( prefix, Float64 ) \
    GUCEF_MATH_CTDIRECTIONVECTOR2D_REAL_KIND( prefix, TFixed16_16 ) \
    GUCEF_MATH_CTDIRECTIONVECTOR2D_REAL_KIND( prefix, TFixed32_32 ) \
    GUCEF_MATH_CTDIRECTIONVECTOR2D_REAL_KIND( prefix, TFixed48_16 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTDIRECTIONVECTOR2D_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTDIRECTIONVECTOR2D_H ? */
