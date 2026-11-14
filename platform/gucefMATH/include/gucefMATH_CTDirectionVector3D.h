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

#ifndef GUCEF_MATH_CTDIRECTIONVECTOR3D_H
#define GUCEF_MATH_CTDIRECTIONVECTOR3D_H

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

/**
 *  A unit direction in 3D. Invariant: the length is 1 within GetUnitLengthTolerance() (4 times the default relative
 *  tolerance for floats, 2 raw units for fixed point), so the data is private and every constructor establishes it.
 *  The kind needs a square root (floats and fixed point).
 *
 *  - A direction is a vector: it converts implicitly to CTVector3D (AsVector()); a vector becomes a direction only
 *    explicitly, by normalising (the constructor, TryCreate) or by promising unit length (FromNormalized).
 *  - Operations that keep the invariant return directions (unary minus, Slerp); the others return vectors
 *    (scaling, cross products). Adding or subtracting two directions does not compile: use AsVector() for that.
 *  - The default direction is Forward() (-Z); a zero or non-finite input to the constructor gives Forward() too.
 *  - Text "(x, y, z)" and DataNode attributes x, y, z; reading checks the invariant.
 *  Every member except the text and DataNode members is realtime-safe.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTDirectionVector3D
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef CTVector3D< T > TVector;

    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt, Direction_vectors_require_a_numeric_kind_with_sqrt );

    /** Forward(): ( 0, 0, -1 ) */
    GUCEF_MATH_FORCE_INLINE CTDirectionVector3D( void )
        : m_x( TTraits::Zero() ), m_y( TTraits::Zero() ), m_z( ComponentNegate( TTraits::One() ) )
    {
    }

    /**
     *  The direction of v: v normalised by CTVector3D::Normalized (which scales by its largest component, so very short and
     *  very long vectors normalise accurately, also in fixed point). A zero or non-finite v gives Forward() and asserts in debug builds;
     *  use TryCreate when the input may be degenerate.
     */
    explicit CTDirectionVector3D( const TVector& v )
        : m_x( TTraits::Zero() ), m_y( TTraits::Zero() ), m_z( ComponentNegate( TTraits::One() ) )
    {
        bool created = TryCreate( v, *this );
        GUCEF_MATH_ASSERT( created );
        (void) created;
    }

    /** The direction of v; false for a zero or non-finite v, 'out' unchanged */
    static bool TryCreate( const TVector& v, CTDirectionVector3D& out ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Adopts a vector that is already of unit length, without normalising; the invariant is asserted in debug builds */
    static GUCEF_MATH_FORCE_INLINE CTDirectionVector3D FromNormalized( const TVector& unit ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( IsUnitLength( unit ) );
        return CTDirectionVector3D( unit.x, unit.y, unit.z );
    }

    /** True when the length of v is 1 within GetUnitLengthTolerance() */
    static bool IsUnitLength( const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The tolerance of the invariant: 4 times the default relative tolerance (floats), 2 raw units (fixed point) */
    static GUCEF_MATH_FORCE_INLINE T GetUnitLengthTolerance( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::IsFloatingPoint ? TTraits::DefaultRelativeTolerance() * TTraits::FromInt32( 4 ) : TTraits::Epsilon() * TTraits::FromInt32( 2 );
    }

    /*---------------------------------------------------------------------*/
    /*  Named constants (GUCEF default convention)                          */
    /*---------------------------------------------------------------------*/

    static GUCEF_MATH_FORCE_INLINE CTDirectionVector3D UnitX( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTDirectionVector3D( TTraits::One(), TTraits::Zero(), TTraits::Zero() ); }
    static GUCEF_MATH_FORCE_INLINE CTDirectionVector3D UnitY( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTDirectionVector3D( TTraits::Zero(), TTraits::One(), TTraits::Zero() ); }
    static GUCEF_MATH_FORCE_INLINE CTDirectionVector3D UnitZ( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTDirectionVector3D( TTraits::Zero(), TTraits::Zero(), TTraits::One() ); }

    /** +X */
    static GUCEF_MATH_FORCE_INLINE CTDirectionVector3D Right( void ) GUCEF_MATH_REALTIME_CONTEXT { return UnitX(); }
    /** -X */
    static GUCEF_MATH_FORCE_INLINE CTDirectionVector3D Left( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTDirectionVector3D( ComponentNegate( TTraits::One() ), TTraits::Zero(), TTraits::Zero() ); }
    /** +Y */
    static GUCEF_MATH_FORCE_INLINE CTDirectionVector3D Up( void ) GUCEF_MATH_REALTIME_CONTEXT { return UnitY(); }
    /** -Y */
    static GUCEF_MATH_FORCE_INLINE CTDirectionVector3D Down( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTDirectionVector3D( TTraits::Zero(), ComponentNegate( TTraits::One() ), TTraits::Zero() ); }
    /** -Z */
    static GUCEF_MATH_FORCE_INLINE CTDirectionVector3D Forward( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTDirectionVector3D( TTraits::Zero(), TTraits::Zero(), ComponentNegate( TTraits::One() ) ); }
    /** +Z */
    static GUCEF_MATH_FORCE_INLINE CTDirectionVector3D Back( void ) GUCEF_MATH_REALTIME_CONTEXT { return UnitZ(); }

    /*---------------------------------------------------------------------*/
    /*  Access                                                              */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE const T& GetX( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_x; }
    GUCEF_MATH_FORCE_INLINE const T& GetY( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_y; }
    GUCEF_MATH_FORCE_INLINE const T& GetZ( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_z; }

    GUCEF_MATH_FORCE_INLINE TVector AsVector( void ) const GUCEF_MATH_REALTIME_CONTEXT { return TVector( m_x, m_y, m_z ); }

    /** A direction is a vector (the converse is never implicit) */
    GUCEF_MATH_FORCE_INLINE operator TVector( void ) const GUCEF_MATH_REALTIME_CONTEXT { return AsVector(); }

    /*---------------------------------------------------------------------*/
    /*  Operators                                                           */
    /*---------------------------------------------------------------------*/

    /** The opposite direction */
    GUCEF_MATH_FORCE_INLINE CTDirectionVector3D operator-( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTDirectionVector3D( ComponentNegate( m_x ), ComponentNegate( m_y ), ComponentNegate( m_z ) );
    }

    /** The vector of length 'factor' along this direction */
    friend GUCEF_MATH_FORCE_INLINE TVector operator*( const CTDirectionVector3D& d, const T& factor ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return TVector( d.m_x * factor, d.m_y * factor, d.m_z * factor );
    }

    friend GUCEF_MATH_FORCE_INLINE TVector operator*( const T& factor, const CTDirectionVector3D& d ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return TVector( factor * d.m_x, factor * d.m_y, factor * d.m_z );
    }

    /** Exact comparison of every component */
    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTDirectionVector3D& a, const CTDirectionVector3D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.m_x == b.m_x && a.m_y == b.m_y && a.m_z == b.m_z;
    }

    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTDirectionVector3D& a, const CTDirectionVector3D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /*
     *  The sum and difference of two directions are not directions. These exist only to stop the compilation with a
     *  message that names the reason; their bodies are instantiated only when used.
     */

    friend CTDirectionVector3D operator+( const CTDirectionVector3D& a, const CTDirectionVector3D& /*b*/ )
    {
        GUCEF_MATH_STATIC_ASSERT( sizeof( T ) == 0, Adding_directions_is_undefined_use_AsVector );
        return a;
    }

    friend CTDirectionVector3D operator-( const CTDirectionVector3D& a, const CTDirectionVector3D& /*b*/ )
    {
        GUCEF_MATH_STATIC_ASSERT( sizeof( T ) == 0, Subtracting_directions_is_undefined_use_AsVector );
        return a;
    }

    /*---------------------------------------------------------------------*/
    /*  Products, angles and interpolation                                  */
    /*---------------------------------------------------------------------*/

    /** The dot product with a vector or (through the conversion) a direction: the cosine of the angle for two directions */
    T Dot( const TVector& v ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The cross product with a vector or a direction; a vector, of length sin( angle ) for two directions */
    TVector Cross( const TVector& v ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Every component equal within the default tolerances of the kind */
    GUCEF_MATH_FORCE_INLINE bool Equals( const CTDirectionVector3D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return AsVector().Equals( other.AsVector() );
    }

    /** The angle between a and b in [0, pi] */
    static CTRadian< T > Angle( const CTDirectionVector3D& a, const CTDirectionVector3D& b ) GUCEF_MATH_REALTIME_CONTEXT;

    /** The angle from 'from' to 'to', signed by the right-hand rule about 'axis' (see CTVector3D::SignedAngle) */
    static CTRadian< T > SignedAngle( const CTDirectionVector3D& from, const CTDirectionVector3D& to, const CTDirectionVector3D& axis ) GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  Spherical linear interpolation: the direction at fraction t of the great-circle arc from a to b, with constant
     *  angular speed (HasTranscendentals). Formed as a cos( t theta ) + p sin( t theta ) with p the unit direction of b
     *  orthogonal to a; nearly equal directions interpolate linearly, opposite directions turn about
     *  a.GetAnyPerpendicular(). The result is renormalised, so the invariant holds for every t.
     */
    static CTDirectionVector3D Slerp( const CTDirectionVector3D& a, const CTDirectionVector3D& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT;

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "(x, y, z)" */
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
        SetFromUnit( v );
        return true;
    }

    /** Writes the attributes x, y, z into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        return AsVector().Serialize( node, settings );
    }

    /** Reads the attributes x, y, z; fails (this direction unchanged) when one is missing or the vector is not unit length */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        TVector v;
        if ( !v.Deserialize( node, settings ) || !IsUnitLength( v ) )
            return false;
        SetFromUnit( v );
        return true;
    }

    private:

    GUCEF_MATH_FORCE_INLINE CTDirectionVector3D( const T& xValue, const T& yValue, const T& zValue )
        : m_x( xValue ), m_y( yValue ), m_z( zValue )
    {
    }

    GUCEF_MATH_FORCE_INLINE void SetFromUnit( const TVector& unit )
    {
        m_x = unit.x;
        m_y = unit.y;
        m_z = unit.z;
    }

    T m_x;
    T m_y;
    T m_z;
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
CTDirectionVector3D< T >::TryCreate( const TVector& v, CTDirectionVector3D& out ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !v.IsFinite() || v.IsZero() )
        return false;
    TVector unit = v.Normalized();
    if ( unit.IsZero() )
        return false;
    out.m_x = unit.x;
    out.m_y = unit.y;
    out.m_z = unit.z;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTDirectionVector3D< T >::IsUnitLength( const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( !v.IsFinite() )
        return false;
    return TTraits::Abs( v.GetLength() - TTraits::One() ) <= GetUnitLengthTolerance();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTDirectionVector3D< T >::Dot( const TVector& v ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return AsVector().Dot( v );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE typename CTDirectionVector3D< T >::TVector
CTDirectionVector3D< T >::Cross( const TVector& v ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return AsVector().Cross( v );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTDirectionVector3D< T >
CTDirectionVector3D< T >::Slerp( const CTDirectionVector3D& a, const CTDirectionVector3D& b, const T& t ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasTranscendentals, Slerp_requires_transcendentals );
    TVector va = a.AsVector();
    TVector vb = b.AsVector();
    T cosine = va.Dot( vb );
    TVector orthogonal = vb - va * cosine;
    T sine = orthogonal.GetLength();
    if ( sine <= TTraits::Sqrt( TTraits::Epsilon() ) )
    {
        if ( !TTraits::IsNegative( cosine ) )
        {
            CTDirectionVector3D result = a;
            TryCreate( TVector::Lerp( va, vb, t ), result );
            return result;
        }
        orthogonal = va.GetAnyPerpendicular();
        sine = TTraits::Zero();
        cosine = ComponentNegate( TTraits::One() );
    }
    T theta = TTraits::Atan2( sine, cosine );
    TVector p = orthogonal.Normalized();
    T angle = theta * t;
    CTDirectionVector3D result = a;
    TryCreate( va * TTraits::Cos( angle ) + p * TTraits::Sin( angle ), result );
    return result;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTRadian< T >
CTDirectionVector3D< T >::Angle( const CTDirectionVector3D& a, const CTDirectionVector3D& b ) GUCEF_MATH_REALTIME_CONTEXT
{
    return TVector::Angle( a.AsVector(), b.AsVector() );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTRadian< T >
CTDirectionVector3D< T >::SignedAngle( const CTDirectionVector3D& from, const CTDirectionVector3D& to, const CTDirectionVector3D& axis ) GUCEF_MATH_REALTIME_CONTEXT
{
    return TVector::SignedAngle( from.AsVector(), to.AsVector(), axis );
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

#define GUCEF_MATH_CTDIRECTIONVECTOR3D_ALL_KINDS_MEMBERS( prefix, kind ) \
    prefix bool CTDirectionVector3D< kind >::TryCreate( const CTVector3D< kind >&, CTDirectionVector3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix bool CTDirectionVector3D< kind >::IsUnitLength( const CTVector3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix kind CTDirectionVector3D< kind >::Dot( const CTVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector3D< kind > CTDirectionVector3D< kind >::Cross( const CTVector3D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTDirectionVector3D< kind > CTDirectionVector3D< kind >::Slerp( const CTDirectionVector3D< kind >&, const CTDirectionVector3D< kind >&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTRadian< kind > CTDirectionVector3D< kind >::Angle( const CTDirectionVector3D< kind >&, const CTDirectionVector3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTRadian< kind > CTDirectionVector3D< kind >::SignedAngle( const CTDirectionVector3D< kind >&, const CTDirectionVector3D< kind >&, const CTDirectionVector3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTDIRECTIONVECTOR3D_REAL_KIND( prefix, kind ) GUCEF_MATH_CTDIRECTIONVECTOR3D_ALL_KINDS_MEMBERS( prefix, kind )

/** Every predefined kind except the fractions (those are in gucefMATH_fractiontypes.h, which includes CBigInteger) */
#define GUCEF_MATH_CTDIRECTIONVECTOR3D_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTDIRECTIONVECTOR3D_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTDIRECTIONVECTOR3D_REAL_KIND( prefix, Float64 ) \
    GUCEF_MATH_CTDIRECTIONVECTOR3D_REAL_KIND( prefix, TFixed16_16 ) \
    GUCEF_MATH_CTDIRECTIONVECTOR3D_REAL_KIND( prefix, TFixed32_32 ) \
    GUCEF_MATH_CTDIRECTIONVECTOR3D_REAL_KIND( prefix, TFixed48_16 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTDIRECTIONVECTOR3D_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTDIRECTIONVECTOR3D_H ? */
