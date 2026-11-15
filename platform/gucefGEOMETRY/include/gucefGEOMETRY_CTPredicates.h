/*
 *  gucefGEOMETRY: GUCEF module providing geometry: shapes, spaces and spatial queries
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

#ifndef GUCEF_GEOMETRY_CTPREDICATES_H
#define GUCEF_GEOMETRY_CTPREDICATES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_CORE_CINT128_H
#include "gucefCORE_CInt128.h"
#define GUCEF_CORE_CINT128_H
#endif /* GUCEF_CORE_CINT128_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_FIXEDPOINT_H
#include "gucefMATH_fixedpoint.h"
#define GUCEF_MATH_FIXEDPOINT_H
#endif /* GUCEF_MATH_FIXEDPOINT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** An exact integer coordinate for the exact stage of the predicates: sign and magnitude (every integer kind and every fixed-point raw value fits) */
struct SPredicateInteger
{
    UInt64 magnitude;
    bool negative;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The exact stages of the predicates (gucefGEOMETRY_CTPredicates.cpp), on CORE::CBigInteger: no allocation while
 *  the values fit 256 bits. The Float64 forms scale every input to the smallest binary exponent among them (every
 *  float is a dyadic rational, so the integers are exact). Non-finite inputs give 0. Coordinates in the order of
 *  the points, x before y before z: Orient2D a, b, c; Orient3D a, b, c, d; InCircle a, b, c, d; InSphere a, b, c, d, e.
 *  The signs follow CTPredicates.
 */
GUCEF_GEOMETRY_PUBLIC_CPP Int32 PredicateExactOrient2D( const Float64 ( &coordinates )[ 6 ] );
GUCEF_GEOMETRY_PUBLIC_CPP Int32 PredicateExactOrient3D( const Float64 ( &coordinates )[ 12 ] );
GUCEF_GEOMETRY_PUBLIC_CPP Int32 PredicateExactInCircle( const Float64 ( &coordinates )[ 8 ] );
GUCEF_GEOMETRY_PUBLIC_CPP Int32 PredicateExactInSphere( const Float64 ( &coordinates )[ 15 ] );
GUCEF_GEOMETRY_PUBLIC_CPP Int32 PredicateExactOrient2D( const SPredicateInteger ( &coordinates )[ 6 ] );
GUCEF_GEOMETRY_PUBLIC_CPP Int32 PredicateExactOrient3D( const SPredicateInteger ( &coordinates )[ 12 ] );
GUCEF_GEOMETRY_PUBLIC_CPP Int32 PredicateExactInCircle( const SPredicateInteger ( &coordinates )[ 8 ] );
GUCEF_GEOMETRY_PUBLIC_CPP Int32 PredicateExactInSphere( const SPredicateInteger ( &coordinates )[ 15 ] );

/*-------------------------------------------------------------------------*/

/*
 *  The Float64 filters (stage 1, Shewchuk's static error bounds over the permanent of the same expression): true with
 *  the exact sign when the rounded determinant is further from 0 than its error bound; false when the exact stage is
 *  needed (near-degenerate input, non-finite intermediate values, or magnitudes where products could underflow).
 */
GUCEF_GEOMETRY_PUBLIC_CPP bool PredicateFilterOrient2D( const Float64 ( &coordinates )[ 6 ], Int32& sign );
GUCEF_GEOMETRY_PUBLIC_CPP bool PredicateFilterOrient3D( const Float64 ( &coordinates )[ 12 ], Int32& sign );
GUCEF_GEOMETRY_PUBLIC_CPP bool PredicateFilterInCircle( const Float64 ( &coordinates )[ 8 ], Int32& sign );
GUCEF_GEOMETRY_PUBLIC_CPP bool PredicateFilterInSphere( const Float64 ( &coordinates )[ 15 ], Int32& sign );

/*-------------------------------------------------------------------------*/

/** The exact integer of a coordinate of an integer or fixed-point kind (the raw value: the scale is common to all inputs) */
template < typename T >
class CTPredicateInteger
{
    public:
    static GUCEF_GEOMETRY_FORCE_INLINE SPredicateInteger Convert( const T& value )
    {
        SPredicateInteger result;
        result.negative = value < T( 0 );
        result.magnitude = result.negative ? (UInt64) 0 - (UInt64) value : (UInt64) value;
        return result;
    }
};

template < typename TStorage, UInt32 FractionBits >
class CTPredicateInteger< MATH::CTFixedPoint< TStorage, FractionBits > >
{
    public:
    static GUCEF_GEOMETRY_FORCE_INLINE SPredicateInteger Convert( const MATH::CTFixedPoint< TStorage, FractionBits >& value )
    {
        const Int64 raw = (Int64) value.GetRaw();
        SPredicateInteger result;
        result.negative = raw < 0;
        result.magnitude = result.negative ? (UInt64) 0 - (UInt64) raw : (UInt64) raw;
        return result;
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  The evaluation strategy of a kind: 0 floats (filter, then exact), 1 integers and fixed point (exact integer
 *  arithmetic), 2 exact kinds (direct evaluation).
 */
template < typename T, int Strategy = MATH::CTNumericTraits< T >::IsFloatingPoint ? 0 : ( ( MATH::CTNumericTraits< T >::IsFixedPoint || MATH::CTNumericTraits< T >::IsInteger ) ? 1 : 2 ) >
class CTPredicateEvaluator;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Exact geometric predicates (section 7.10 of the design): the exact sign (-1, 0, +1) of the classic determinants,
 *  whatever the rounding of the kind.
 *
 *  - Orient2D( a, b, c ): +1 when a, b, c run counter-clockwise, -1 clockwise, 0 collinear.
 *  - Orient3D( a, b, c, d ): +1 when d lies in front of the plane through a, b, c (the side of the normal
 *    ( b - a ) x ( c - a ), the convention of CTPlane::TryFromPoints), -1 behind, 0 coplanar.
 *  - InCircle( a, b, c, d ): for a, b, c counter-clockwise, +1 when d lies inside their circle, -1 outside, 0 on it
 *    (the sign flips for clockwise a, b, c).
 *  - InSphere( a, b, c, d, e ): for Orient3D( a, b, c, d ) > 0, +1 when e lies inside their sphere, -1 outside, 0 on
 *    it (the sign flips for the other orientation).
 *
 *  - Float kinds (Float32 is widened exactly): stage 1 evaluates in Float64 with Shewchuk's static error bound when
 *    every nonzero coordinate difference lies in [2^-200, 2^200] (so no product underflows or overflows); when the
 *    result is not further from 0 than the bound, or a difference is outside that range, stage 2 evaluates exactly
 *    with CBigInteger on the inputs scaled to a common power of two. Non-finite inputs give 0 (and assert in debug
 *    builds).
 *  - Fixed point and integer kinds: exact integer arithmetic on the raw values: CInt128 for Orient2D while
 *    |raw| < 2^61 and for Orient3D while the coordinate differences are below 2^41 (every 32 bit kind, and fixed
 *    point within about 512 units in Q32.32), CBigInteger otherwise; InCircle and InSphere on a fixed-width 384-bit
 *    integer on the stack, which holds their determinants for every input of these kinds.
 *  - Fraction kinds: direct exact evaluation in the kind.
 *  - Realtime: the filters, the CInt128 forms and the integer InCircle and InSphere are; the CBigInteger stage
 *    allocates beyond 256 bits.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTPredicates
{
    public:

    typedef T TValue;
    typedef MATH::CTPoint2D< T > TPoint2D;
    typedef MATH::CTPoint3D< T > TPoint3D;

    static GUCEF_GEOMETRY_FORCE_INLINE Int32 Orient2D( const TPoint2D& a, const TPoint2D& b, const TPoint2D& c )
    {
        return CTPredicateEvaluator< T >::Orient2D( a, b, c );
    }

    static GUCEF_GEOMETRY_FORCE_INLINE Int32 Orient3D( const TPoint3D& a, const TPoint3D& b, const TPoint3D& c, const TPoint3D& d )
    {
        return CTPredicateEvaluator< T >::Orient3D( a, b, c, d );
    }

    static GUCEF_GEOMETRY_FORCE_INLINE Int32 InCircle( const TPoint2D& a, const TPoint2D& b, const TPoint2D& c, const TPoint2D& d )
    {
        return CTPredicateEvaluator< T >::InCircle( a, b, c, d );
    }

    static GUCEF_GEOMETRY_FORCE_INLINE Int32 InSphere( const TPoint3D& a, const TPoint3D& b, const TPoint3D& c, const TPoint3D& d, const TPoint3D& e )
    {
        return CTPredicateEvaluator< T >::InSphere( a, b, c, d, e );
    }
};

/*-------------------------------------------------------------------------*/

/** Float kinds: the Float64 filter, then the exact stage */
template < typename T >
class CTPredicateEvaluator< T, 0 >
{
    public:

    static Int32 Orient2D( const MATH::CTPoint2D< T >& a, const MATH::CTPoint2D< T >& b, const MATH::CTPoint2D< T >& c )
    {
        const Float64 v[ 6 ] = { (Float64) a.x, (Float64) a.y, (Float64) b.x, (Float64) b.y, (Float64) c.x, (Float64) c.y };
        Int32 sign = 0;
        if ( PredicateFilterOrient2D( v, sign ) )
            return sign;
        return PredicateExactOrient2D( v );
    }

    static Int32 Orient3D( const MATH::CTPoint3D< T >& a, const MATH::CTPoint3D< T >& b, const MATH::CTPoint3D< T >& c, const MATH::CTPoint3D< T >& d )
    {
        const Float64 v[ 12 ] = { (Float64) a.x, (Float64) a.y, (Float64) a.z, (Float64) b.x, (Float64) b.y, (Float64) b.z,
                                  (Float64) c.x, (Float64) c.y, (Float64) c.z, (Float64) d.x, (Float64) d.y, (Float64) d.z };
        Int32 sign = 0;
        if ( PredicateFilterOrient3D( v, sign ) )
            return sign;
        return PredicateExactOrient3D( v );
    }

    static Int32 InCircle( const MATH::CTPoint2D< T >& a, const MATH::CTPoint2D< T >& b, const MATH::CTPoint2D< T >& c, const MATH::CTPoint2D< T >& d )
    {
        const Float64 v[ 8 ] = { (Float64) a.x, (Float64) a.y, (Float64) b.x, (Float64) b.y, (Float64) c.x, (Float64) c.y, (Float64) d.x, (Float64) d.y };
        Int32 sign = 0;
        if ( PredicateFilterInCircle( v, sign ) )
            return sign;
        return PredicateExactInCircle( v );
    }

    static Int32 InSphere( const MATH::CTPoint3D< T >& a, const MATH::CTPoint3D< T >& b, const MATH::CTPoint3D< T >& c, const MATH::CTPoint3D< T >& d, const MATH::CTPoint3D< T >& e )
    {
        const Float64 v[ 15 ] = { (Float64) a.x, (Float64) a.y, (Float64) a.z, (Float64) b.x, (Float64) b.y, (Float64) b.z, (Float64) c.x, (Float64) c.y, (Float64) c.z,
                                  (Float64) d.x, (Float64) d.y, (Float64) d.z, (Float64) e.x, (Float64) e.y, (Float64) e.z };
        Int32 sign = 0;
        if ( PredicateFilterInSphere( v, sign ) )
            return sign;
        return PredicateExactInSphere( v );
    }
};

/*-------------------------------------------------------------------------*/

/** Integer and fixed-point kinds: exact integer arithmetic on the raw values */
template < typename T >
class CTPredicateEvaluator< T, 1 >
{
    public:

    static Int32 Orient2D( const MATH::CTPoint2D< T >& a, const MATH::CTPoint2D< T >& b, const MATH::CTPoint2D< T >& c )
    {
        const SPredicateInteger v[ 6 ] = { Convert( a.x ), Convert( a.y ), Convert( b.x ), Convert( b.y ), Convert( c.x ), Convert( c.y ) };
        if ( AllBelow( v, 6, (UInt64) 1 << 61 ) )
        {
            const Int64 ax = ToInt64( v[ 0 ] ), ay = ToInt64( v[ 1 ] ), bx = ToInt64( v[ 2 ] ), by = ToInt64( v[ 3 ] ), cx = ToInt64( v[ 4 ] ), cy = ToInt64( v[ 5 ] );
            const CORE::CInt128 left = CORE::CInt128::Multiply( ax - cx, by - cy );
            const CORE::CInt128 right = CORE::CInt128::Multiply( ay - cy, bx - cx );
            return CORE::CInt128::Subtract( left, right ).GetSign();
        }
        return PredicateExactOrient2D( v );
    }

    static Int32 Orient3D( const MATH::CTPoint3D< T >& a, const MATH::CTPoint3D< T >& b, const MATH::CTPoint3D< T >& c, const MATH::CTPoint3D< T >& d )
    {
        const SPredicateInteger v[ 12 ] = { Convert( a.x ), Convert( a.y ), Convert( a.z ), Convert( b.x ), Convert( b.y ), Convert( b.z ),
                                            Convert( c.x ), Convert( c.y ), Convert( c.z ), Convert( d.x ), Convert( d.y ), Convert( d.z ) };
        if ( AllBelow( v, 12, (UInt64) 1 << 62 ) )
        {
            /* coordinates below 2^62: the differences fit Int64 */
            const Int64 ax = ToInt64( v[ 0 ] ), ay = ToInt64( v[ 1 ] ), az = ToInt64( v[ 2 ] );
            const Int64 ux = ToInt64( v[ 3 ] ) - ax, uy = ToInt64( v[ 4 ] ) - ay, uz = ToInt64( v[ 5 ] ) - az;
            const Int64 wx = ToInt64( v[ 6 ] ) - ax, wy = ToInt64( v[ 7 ] ) - ay, wz = ToInt64( v[ 8 ] ) - az;
            const Int64 tx = ToInt64( v[ 9 ] ) - ax, ty = ToInt64( v[ 10 ] ) - ay, tz = ToInt64( v[ 11 ] ) - az;
            const Int64 differences[ 9 ] = { ux, uy, uz, wx, wy, wz, tx, ty, tz };
            if ( !DifferencesBelow( differences, 9, (Int64) 1 << 41 ) )
                return PredicateExactOrient3D( v );
            /*
             *  det[ b - a, c - a, d - a ] with differences below 2^41: the 2x2 minors below 2^83, every product with the
             *  third difference below 2^124, the sum below 2^126
             */
            const CORE::CInt128 m1 = CORE::CInt128::Subtract( CORE::CInt128::Multiply( wy, tz ), CORE::CInt128::Multiply( wz, ty ) );
            const CORE::CInt128 m2 = CORE::CInt128::Subtract( CORE::CInt128::Multiply( wz, tx ), CORE::CInt128::Multiply( wx, tz ) );
            const CORE::CInt128 m3 = CORE::CInt128::Subtract( CORE::CInt128::Multiply( wx, ty ), CORE::CInt128::Multiply( wy, tx ) );
            CORE::CInt128 p1;
            CORE::CInt128 p2;
            CORE::CInt128 p3;
            CORE::CInt128::MultiplyChecked( m1, ux, p1 );
            CORE::CInt128::MultiplyChecked( m2, uy, p2 );
            CORE::CInt128::MultiplyChecked( m3, uz, p3 );
            return CORE::CInt128::Add( CORE::CInt128::Add( p1, p2 ), p3 ).GetSign();
        }
        return PredicateExactOrient3D( v );
    }

    static Int32 InCircle( const MATH::CTPoint2D< T >& a, const MATH::CTPoint2D< T >& b, const MATH::CTPoint2D< T >& c, const MATH::CTPoint2D< T >& d )
    {
        const SPredicateInteger v[ 8 ] = { Convert( a.x ), Convert( a.y ), Convert( b.x ), Convert( b.y ), Convert( c.x ), Convert( c.y ), Convert( d.x ), Convert( d.y ) };
        return PredicateExactInCircle( v );
    }

    static Int32 InSphere( const MATH::CTPoint3D< T >& a, const MATH::CTPoint3D< T >& b, const MATH::CTPoint3D< T >& c, const MATH::CTPoint3D< T >& d, const MATH::CTPoint3D< T >& e )
    {
        const SPredicateInteger v[ 15 ] = { Convert( a.x ), Convert( a.y ), Convert( a.z ), Convert( b.x ), Convert( b.y ), Convert( b.z ), Convert( c.x ), Convert( c.y ), Convert( c.z ),
                                            Convert( d.x ), Convert( d.y ), Convert( d.z ), Convert( e.x ), Convert( e.y ), Convert( e.z ) };
        return PredicateExactInSphere( v );
    }

    private:

    static GUCEF_GEOMETRY_FORCE_INLINE SPredicateInteger Convert( const T& value ) { return CTPredicateInteger< T >::Convert( value ); }

    static GUCEF_GEOMETRY_FORCE_INLINE bool AllBelow( const SPredicateInteger* v, UInt32 count, UInt64 limit )
    {
        for ( UInt32 i=0; i<count; ++i )
            if ( v[ i ].magnitude >= limit )
                return false;
        return true;
    }

    /** True when every difference has a magnitude below 'limit' */
    static GUCEF_GEOMETRY_FORCE_INLINE bool DifferencesBelow( const Int64* differences, UInt32 count, Int64 limit )
    {
        for ( UInt32 i=0; i<count; ++i )
            if ( differences[ i ] >= limit || differences[ i ] <= -limit )
                return false;
        return true;
    }

    /** A magnitude below 2^63 as Int64 */
    static GUCEF_GEOMETRY_FORCE_INLINE Int64 ToInt64( const SPredicateInteger& v )
    {
        return v.negative ? -(Int64) v.magnitude : (Int64) v.magnitude;
    }
};

/*-------------------------------------------------------------------------*/

/** Exact kinds (fractions): direct evaluation in the kind */
template < typename T >
class CTPredicateEvaluator< T, 2 >
{
    public:

    static Int32 Orient2D( const MATH::CTPoint2D< T >& a, const MATH::CTPoint2D< T >& b, const MATH::CTPoint2D< T >& c )
    {
        return Sign( ( a.x - c.x ) * ( b.y - c.y ) - ( a.y - c.y ) * ( b.x - c.x ) );
    }

    static Int32 Orient3D( const MATH::CTPoint3D< T >& a, const MATH::CTPoint3D< T >& b, const MATH::CTPoint3D< T >& c, const MATH::CTPoint3D< T >& d )
    {
        return Sign( ( b - a ).Dot( ( c - a ).Cross( d - a ) ) );
    }

    static Int32 InCircle( const MATH::CTPoint2D< T >& a, const MATH::CTPoint2D< T >& b, const MATH::CTPoint2D< T >& c, const MATH::CTPoint2D< T >& d )
    {
        const T adx = a.x - d.x, ady = a.y - d.y, bdx = b.x - d.x, bdy = b.y - d.y, cdx = c.x - d.x, cdy = c.y - d.y;
        const T alift = adx * adx + ady * ady;
        const T blift = bdx * bdx + bdy * bdy;
        const T clift = cdx * cdx + cdy * cdy;
        return Sign( alift * ( bdx * cdy - cdx * bdy ) + blift * ( cdx * ady - adx * cdy ) + clift * ( adx * bdy - bdx * ady ) );
    }

    static Int32 InSphere( const MATH::CTPoint3D< T >& a, const MATH::CTPoint3D< T >& b, const MATH::CTPoint3D< T >& c, const MATH::CTPoint3D< T >& d, const MATH::CTPoint3D< T >& e )
    {
        /* rows ( p - e, |p - e|^2 ) for p = a, b, c, d; the sign negated so that inside is positive for Orient3D > 0 */
        const MATH::CTVector3D< T > rows[ 4 ] = { a - e, b - e, c - e, d - e };
        T lift[ 4 ];
        for ( UInt32 i=0; i<4; ++i )
            lift[ i ] = rows[ i ].Dot( rows[ i ] );
        T det = MATH::CTNumericTraits< T >::Zero();
        for ( UInt32 i=0; i<4; ++i )
        {
            const MATH::CTVector3D< T >& p = rows[ ( i + 1 ) % 4 ];
            const MATH::CTVector3D< T >& q = rows[ ( i + 2 ) % 4 ];
            const MATH::CTVector3D< T >& r = rows[ ( i + 3 ) % 4 ];
            const T minor = p.Dot( q.Cross( r ) );
            /* cofactor expansion along the lift column: sign ( -1 )^( i + 3 ) for row i, with the cyclic minor sign */
            const T term = lift[ i ] * minor;
            det = ( 0 == i % 2 ) ? det - term : det + term;
        }
        return -Sign( det );
    }

    private:

    static GUCEF_GEOMETRY_FORCE_INLINE Int32 Sign( const T& value )
    {
        const T zero = MATH::CTNumericTraits< T >::Zero();
        return zero < value ? 1 : ( value < zero ? -1 : 0 );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTPREDICATES_H ? */
