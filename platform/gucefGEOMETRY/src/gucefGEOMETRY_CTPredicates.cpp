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

/*
 *  The Float64 filters and the exact stages of the geometric predicates (section 7.10 of the design). The filters use
 *  the static error bounds of Shewchuk ("Adaptive Precision Floating-Point Arithmetic and Fast Robust Geometric
 *  Predicates", 1997) on the permanent of the same expression tree, for inputs whose coordinate differences lie in
 *  [2^-200, 2^200] (so no product underflows or overflows); the exact stages evaluate the determinants on
 *  CORE::CBigInteger for floats, and on a fixed-width 384-bit integer on the stack for the integer InCircle and
 *  InSphere (coordinate differences of integer and fixed-point kinds need at most 65 bits, the determinants at most
 *  4 * 65 + 4 and 5 * 65 + 7 bits).
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#ifndef GUCEF_CORE_CBIGINTEGER_H
#include "gucefCORE_CBigInteger.h"
#define GUCEF_CORE_CBIGINTEGER_H
#endif /* GUCEF_CORE_CBIGINTEGER_H ? */

#ifndef GUCEF_GEOMETRY_CTPREDICATES_H
#include "gucefGEOMETRY_CTPredicates.h"
#define GUCEF_GEOMETRY_CTPREDICATES_H
#endif /* GUCEF_GEOMETRY_CTPREDICATES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Shewchuk's epsilon: half a unit in the last place of 1.0 in Float64, 2^-53 */
const Float64 PredicateEpsilon = 1.1102230246251565404236316680908203125e-16;

/** Below this permanent a product may have lost bits to underflow: the filter defers to the exact stage */
const Float64 PredicateSmallestPermanent = 1.0e-270;

/** The range of nonzero coordinate differences the filters accept: 2^-200 and 2^200 */
const Float64 PredicateSmallestDifference = 6.2230152778611417071440640537801242405902521687211671331011166147896988340353834411839448231257136169569665895551224821247160434722900390625e-61;
const Float64 PredicateLargestDifference = 1.606938044258990275541962092341162602522202993782792835301376e60;

/*-------------------------------------------------------------------------*/

/**
 *  True when every coordinate difference is 0 or has a magnitude in [2^-200, 2^200]: products of up to five of them
 *  then neither underflow nor overflow, which the static error bounds assume
 */
bool
DifferencesInFilterRange( const Float64* differences, UInt32 count )
{
    for ( UInt32 i=0; i<count; ++i )
    {
        const Float64 magnitude = fabs( differences[ i ] );
        if ( 0.0 != magnitude && ( magnitude < PredicateSmallestDifference || magnitude > PredicateLargestDifference ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  A signed integer of up to 384 bits on the stack (sign and magnitude in 32-bit limbs, least significant first): the
 *  exact stage of the integer InCircle and InSphere without heap use. The callers keep every value within 384 bits.
 */
class CPredicateWide
{
    public:

    enum { Limbs = 12 };

    CPredicateWide( void )
        : m_size( 0 ), m_negative( false )
    {
        for ( UInt32 i=0; i<(UInt32) Limbs; ++i )
            m_limbs[ i ] = 0;
    }

    /** a - b for sign and magnitude coordinates (at most 65 bits) */
    static CPredicateWide Difference( const SPredicateInteger& a, const SPredicateInteger& b )
    {
        CPredicateWide result;
        UInt64 low = 0;
        UInt32 carry = 0;
        bool negative = false;
        if ( a.negative == b.negative )
        {
            if ( a.magnitude >= b.magnitude )
            {
                low = a.magnitude - b.magnitude;
                negative = a.negative;
            }
            else
            {
                low = b.magnitude - a.magnitude;
                negative = !a.negative;
            }
        }
        else
        {
            low = a.magnitude + b.magnitude;
            carry = low < a.magnitude ? 1 : 0;
            negative = a.negative;
        }
        result.m_limbs[ 0 ] = (UInt32) low;
        result.m_limbs[ 1 ] = (UInt32) ( low >> 32 );
        result.m_limbs[ 2 ] = carry;
        result.m_size = 3;
        result.Trim();
        result.m_negative = negative && 0 != result.m_size;
        return result;
    }

    static CPredicateWide Multiply( const CPredicateWide& x, const CPredicateWide& y )
    {
        CPredicateWide result;
        if ( 0 == x.m_size || 0 == y.m_size )
            return result;
        GUCEF_GEOMETRY_ASSERT( x.m_size + y.m_size <= (UInt32) Limbs );
        for ( UInt32 i=0; i<x.m_size; ++i )
        {
            UInt64 carry = 0;
            for ( UInt32 j=0; j<y.m_size && i+j<(UInt32) Limbs; ++j )
            {
                const UInt64 t = (UInt64) x.m_limbs[ i ] * (UInt64) y.m_limbs[ j ] + (UInt64) result.m_limbs[ i + j ] + carry;
                result.m_limbs[ i + j ] = (UInt32) t;
                carry = t >> 32;
            }
            if ( i + y.m_size < (UInt32) Limbs )
                result.m_limbs[ i + y.m_size ] = (UInt32) carry;
        }
        result.m_size = x.m_size + y.m_size < (UInt32) Limbs ? x.m_size + y.m_size : (UInt32) Limbs;
        result.Trim();
        result.m_negative = ( x.m_negative != y.m_negative ) && 0 != result.m_size;
        return result;
    }

    static CPredicateWide Add( const CPredicateWide& x, const CPredicateWide& y )
    {
        if ( x.m_negative == y.m_negative )
        {
            CPredicateWide result = AddMagnitudes( x, y );
            result.m_negative = x.m_negative && 0 != result.m_size;
            return result;
        }
        if ( CompareMagnitudes( x, y ) >= 0 )
        {
            CPredicateWide result = SubtractMagnitudes( x, y );
            result.m_negative = x.m_negative && 0 != result.m_size;
            return result;
        }
        CPredicateWide result = SubtractMagnitudes( y, x );
        result.m_negative = y.m_negative && 0 != result.m_size;
        return result;
    }

    static CPredicateWide Subtract( const CPredicateWide& x, const CPredicateWide& y )
    {
        CPredicateWide negated = y;
        negated.m_negative = !y.m_negative && 0 != y.m_size;
        return Add( x, negated );
    }

    Int32 GetSign( void ) const
    {
        if ( 0 == m_size )
            return 0;
        return m_negative ? -1 : 1;
    }

    private:

    void Trim( void )
    {
        while ( m_size > 0 && 0 == m_limbs[ m_size - 1 ] )
            --m_size;
    }

    static int CompareMagnitudes( const CPredicateWide& x, const CPredicateWide& y )
    {
        if ( x.m_size != y.m_size )
            return x.m_size < y.m_size ? -1 : 1;
        for ( UInt32 i=x.m_size; i>0; --i )
            if ( x.m_limbs[ i - 1 ] != y.m_limbs[ i - 1 ] )
                return x.m_limbs[ i - 1 ] < y.m_limbs[ i - 1 ] ? -1 : 1;
        return 0;
    }

    static CPredicateWide AddMagnitudes( const CPredicateWide& x, const CPredicateWide& y )
    {
        CPredicateWide result;
        const UInt32 size = x.m_size > y.m_size ? x.m_size : y.m_size;
        UInt64 carry = 0;
        for ( UInt32 i=0; i<size; ++i )
        {
            const UInt64 t = (UInt64) x.m_limbs[ i ] + (UInt64) y.m_limbs[ i ] + carry;
            result.m_limbs[ i ] = (UInt32) t;
            carry = t >> 32;
        }
        result.m_size = size;
        if ( 0 != carry )
        {
            GUCEF_GEOMETRY_ASSERT( size < (UInt32) Limbs );
            if ( size < (UInt32) Limbs )
            {
                result.m_limbs[ size ] = (UInt32) carry;
                result.m_size = size + 1;
            }
        }
        return result;
    }

    /** |x| - |y| for |x| >= |y| */
    static CPredicateWide SubtractMagnitudes( const CPredicateWide& x, const CPredicateWide& y )
    {
        CPredicateWide result;
        UInt64 borrow = 0;
        for ( UInt32 i=0; i<x.m_size; ++i )
        {
            const UInt64 subtrahend = (UInt64) y.m_limbs[ i ] + borrow;
            const UInt64 minuend = (UInt64) x.m_limbs[ i ];
            if ( minuend >= subtrahend )
            {
                result.m_limbs[ i ] = (UInt32) ( minuend - subtrahend );
                borrow = 0;
            }
            else
            {
                result.m_limbs[ i ] = (UInt32) ( ( minuend + ( (UInt64) 1 << 32 ) ) - subtrahend );
                borrow = 1;
            }
        }
        result.m_size = x.m_size;
        result.Trim();
        return result;
    }

    UInt32 m_limbs[ Limbs ];
    UInt32 m_size;
    bool m_negative;
};

/*-------------------------------------------------------------------------*/

Int32
InCircleWide( const SPredicateInteger* v )
{
    typedef CPredicateWide W;
    const W adx = W::Difference( v[ 0 ], v[ 6 ] ), ady = W::Difference( v[ 1 ], v[ 7 ] );
    const W bdx = W::Difference( v[ 2 ], v[ 6 ] ), bdy = W::Difference( v[ 3 ], v[ 7 ] );
    const W cdx = W::Difference( v[ 4 ], v[ 6 ] ), cdy = W::Difference( v[ 5 ], v[ 7 ] );
    const W alift = W::Add( W::Multiply( adx, adx ), W::Multiply( ady, ady ) );
    const W blift = W::Add( W::Multiply( bdx, bdx ), W::Multiply( bdy, bdy ) );
    const W clift = W::Add( W::Multiply( cdx, cdx ), W::Multiply( cdy, cdy ) );
    const W bc = W::Subtract( W::Multiply( bdx, cdy ), W::Multiply( cdx, bdy ) );
    const W ca = W::Subtract( W::Multiply( cdx, ady ), W::Multiply( adx, cdy ) );
    const W ab = W::Subtract( W::Multiply( adx, bdy ), W::Multiply( bdx, ady ) );
    const W det = W::Add( W::Add( W::Multiply( alift, bc ), W::Multiply( blift, ca ) ), W::Multiply( clift, ab ) );
    return det.GetSign();
}

/*-------------------------------------------------------------------------*/

/** As InSphereExact on the stack integer */
Int32
InSphereWide( const SPredicateInteger* v )
{
    typedef CPredicateWide W;
    W r[ 4 ][ 3 ];
    W lift[ 4 ];
    for ( UInt32 i=0; i<4; ++i )
    {
        for ( UInt32 k=0; k<3; ++k )
            r[ i ][ k ] = W::Difference( v[ 3 * i + k ], v[ 12 + k ] );
        lift[ i ] = W::Add( W::Add( W::Multiply( r[ i ][ 0 ], r[ i ][ 0 ] ), W::Multiply( r[ i ][ 1 ], r[ i ][ 1 ] ) ), W::Multiply( r[ i ][ 2 ], r[ i ][ 2 ] ) );
    }
    const UInt32 rows[ 4 ][ 3 ] = { { 1, 2, 3 }, { 0, 2, 3 }, { 0, 1, 3 }, { 0, 1, 2 } };
    W det;
    for ( UInt32 i=0; i<4; ++i )
    {
        const W* p = r[ rows[ i ][ 0 ] ];
        const W* q = r[ rows[ i ][ 1 ] ];
        const W* s = r[ rows[ i ][ 2 ] ];
        const W m0 = W::Subtract( W::Multiply( q[ 1 ], s[ 2 ] ), W::Multiply( q[ 2 ], s[ 1 ] ) );
        const W m1 = W::Subtract( W::Multiply( q[ 2 ], s[ 0 ] ), W::Multiply( q[ 0 ], s[ 2 ] ) );
        const W m2 = W::Subtract( W::Multiply( q[ 0 ], s[ 1 ] ), W::Multiply( q[ 1 ], s[ 0 ] ) );
        const W minor = W::Add( W::Add( W::Multiply( p[ 0 ], m0 ), W::Multiply( p[ 1 ], m1 ) ), W::Multiply( p[ 2 ], m2 ) );
        const W term = W::Multiply( lift[ i ], minor );
        det = ( 0 == i % 2 ) ? W::Subtract( det, term ) : W::Add( det, term );
    }
    return -det.GetSign();
}

/*-------------------------------------------------------------------------*/

bool
IsFiniteFloat64( Float64 x )
{
    return x == x && x - x == 0.0;
}

/*-------------------------------------------------------------------------*/

/** The sign once |det| exceeds bound * permanent; false when undecided */
bool
DecideFiltered( Float64 det, Float64 permanent, Float64 boundFactor, Int32& sign )
{
    if ( !IsFiniteFloat64( det ) || !IsFiniteFloat64( permanent ) || permanent < PredicateSmallestPermanent )
        return false;
    const Float64 bound = boundFactor * permanent;
    if ( det > bound )
    {
        sign = 1;
        return true;
    }
    if ( -det > bound )
    {
        sign = -1;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** The values as exact integers scaled by one common power of two (2^-shift); false for a non-finite value */
bool
ToCommonScaleIntegers( const Float64* values, UInt32 count, CORE::CBigInteger* out )
{
    int smallest = 0;
    bool any = false;
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( !IsFiniteFloat64( values[ i ] ) )
            return false;
        if ( 0.0 == values[ i ] )
            continue;
        int exponent = 0;
        frexp( values[ i ], &exponent );
        /* value = mantissa * 2^( exponent - 53 ) with an integer mantissa below 2^53 */
        const int low = exponent - 53;
        if ( !any || low < smallest )
            smallest = low;
        any = true;
    }
    for ( UInt32 i=0; i<count; ++i )
    {
        out[ i ] = CORE::CBigInteger::Zero();
        if ( 0.0 == values[ i ] )
            continue;
        int exponent = 0;
        const Float64 fraction = frexp( values[ i ], &exponent );
        const Int64 mantissa = (Int64) ldexp( fraction, 53 );
        CORE::CBigInteger::FromInt64( mantissa, out[ i ] );
        out[ i ] <<= (UInt32) ( ( exponent - 53 ) - smallest );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
ToIntegers( const SPredicateInteger* values, UInt32 count, CORE::CBigInteger* out )
{
    for ( UInt32 i=0; i<count; ++i )
    {
        CORE::CBigInteger::FromUInt64( values[ i ].magnitude, out[ i ] );
        if ( values[ i ].negative )
            out[ i ] = -out[ i ];
    }
}

/*-------------------------------------------------------------------------*/

Int32
Orient2DExact( const CORE::CBigInteger* v )
{
    const CORE::CBigInteger det = ( v[ 0 ] - v[ 4 ] ) * ( v[ 3 ] - v[ 5 ] ) - ( v[ 1 ] - v[ 5 ] ) * ( v[ 2 ] - v[ 4 ] );
    return det.GetSign();
}

/*-------------------------------------------------------------------------*/

/** det[ b - a, c - a, d - a ] (positive: d in front of the plane through a, b, c) */
Int32
Orient3DExact( const CORE::CBigInteger* v )
{
    const CORE::CBigInteger ux = v[ 3 ] - v[ 0 ], uy = v[ 4 ] - v[ 1 ], uz = v[ 5 ] - v[ 2 ];
    const CORE::CBigInteger wx = v[ 6 ] - v[ 0 ], wy = v[ 7 ] - v[ 1 ], wz = v[ 8 ] - v[ 2 ];
    const CORE::CBigInteger tx = v[ 9 ] - v[ 0 ], ty = v[ 10 ] - v[ 1 ], tz = v[ 11 ] - v[ 2 ];
    const CORE::CBigInteger det = ux * ( wy * tz - wz * ty ) + uy * ( wz * tx - wx * tz ) + uz * ( wx * ty - wy * tx );
    return det.GetSign();
}

/*-------------------------------------------------------------------------*/

Int32
InCircleExact( const CORE::CBigInteger* v )
{
    const CORE::CBigInteger adx = v[ 0 ] - v[ 6 ], ady = v[ 1 ] - v[ 7 ];
    const CORE::CBigInteger bdx = v[ 2 ] - v[ 6 ], bdy = v[ 3 ] - v[ 7 ];
    const CORE::CBigInteger cdx = v[ 4 ] - v[ 6 ], cdy = v[ 5 ] - v[ 7 ];
    const CORE::CBigInteger alift = adx * adx + ady * ady;
    const CORE::CBigInteger blift = bdx * bdx + bdy * bdy;
    const CORE::CBigInteger clift = cdx * cdx + cdy * cdy;
    const CORE::CBigInteger det = alift * ( bdx * cdy - cdx * bdy ) + blift * ( cdx * ady - adx * cdy ) + clift * ( adx * bdy - bdx * ady );
    return det.GetSign();
}

/*-------------------------------------------------------------------------*/

/** Minus the 4x4 determinant of the rows ( p - e, |p - e|^2 ) for p = a, b, c, d (positive: e inside for Orient3D > 0) */
Int32
InSphereExact( const CORE::CBigInteger* v )
{
    CORE::CBigInteger r[ 4 ][ 3 ];
    CORE::CBigInteger lift[ 4 ];
    for ( UInt32 i=0; i<4; ++i )
    {
        for ( UInt32 k=0; k<3; ++k )
            r[ i ][ k ] = v[ 3 * i + k ] - v[ 12 + k ];
        lift[ i ] = r[ i ][ 0 ] * r[ i ][ 0 ] + r[ i ][ 1 ] * r[ i ][ 1 ] + r[ i ][ 2 ] * r[ i ][ 2 ];
    }
    /* minor of the rows p, q, s: p . ( q x s ) */
    const UInt32 rows[ 4 ][ 3 ] = { { 1, 2, 3 }, { 0, 2, 3 }, { 0, 1, 3 }, { 0, 1, 2 } };
    CORE::CBigInteger det = CORE::CBigInteger::Zero();
    for ( UInt32 i=0; i<4; ++i )
    {
        const CORE::CBigInteger* p = r[ rows[ i ][ 0 ] ];
        const CORE::CBigInteger* q = r[ rows[ i ][ 1 ] ];
        const CORE::CBigInteger* s = r[ rows[ i ][ 2 ] ];
        const CORE::CBigInteger minor = p[ 0 ] * ( q[ 1 ] * s[ 2 ] - q[ 2 ] * s[ 1 ] ) + p[ 1 ] * ( q[ 2 ] * s[ 0 ] - q[ 0 ] * s[ 2 ] ) + p[ 2 ] * ( q[ 0 ] * s[ 1 ] - q[ 1 ] * s[ 0 ] );
        /* expansion along the lift column: ( -1 )^( i + 3 ) */
        if ( 0 == i % 2 )
            det -= lift[ i ] * minor;
        else
            det += lift[ i ] * minor;
    }
    return -det.GetSign();
}

}  /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
PredicateFilterOrient2D( const Float64 ( &c )[ 6 ], Int32& sign )
{
    const Float64 differences[ 4 ] = { c[ 0 ] - c[ 4 ], c[ 3 ] - c[ 5 ], c[ 1 ] - c[ 5 ], c[ 2 ] - c[ 4 ] };
    if ( !DifferencesInFilterRange( differences, 4 ) )
        return false;
    const Float64 left = differences[ 0 ] * differences[ 1 ];
    const Float64 right = differences[ 2 ] * differences[ 3 ];
    const Float64 det = left - right;
    const Float64 permanent = fabs( left ) + fabs( right );
    if ( 0.0 == permanent && IsFiniteFloat64( det ) )
    {
        /* a factor of each product is exactly zero only when coordinates coincide: the exact stage decides cheaply */
        return false;
    }
    return DecideFiltered( det, permanent, ( 3.0 + 16.0 * PredicateEpsilon ) * PredicateEpsilon, sign );
}

/*-------------------------------------------------------------------------*/

bool
PredicateFilterOrient3D( const Float64 ( &c )[ 12 ], Int32& sign )
{
    /* Shewchuk's orient3d with d as the reference point; it has the opposite sign of det[ b - a, c - a, d - a ] */
    const Float64 adx = c[ 0 ] - c[ 9 ], ady = c[ 1 ] - c[ 10 ], adz = c[ 2 ] - c[ 11 ];
    const Float64 bdx = c[ 3 ] - c[ 9 ], bdy = c[ 4 ] - c[ 10 ], bdz = c[ 5 ] - c[ 11 ];
    const Float64 cdx = c[ 6 ] - c[ 9 ], cdy = c[ 7 ] - c[ 10 ], cdz = c[ 8 ] - c[ 11 ];
    const Float64 differences[ 9 ] = { adx, ady, adz, bdx, bdy, bdz, cdx, cdy, cdz };
    if ( !DifferencesInFilterRange( differences, 9 ) )
        return false;
    const Float64 bdxcdy = bdx * cdy, cdxbdy = cdx * bdy;
    const Float64 cdxady = cdx * ady, adxcdy = adx * cdy;
    const Float64 adxbdy = adx * bdy, bdxady = bdx * ady;
    const Float64 det = adz * ( bdxcdy - cdxbdy ) + bdz * ( cdxady - adxcdy ) + cdz * ( adxbdy - bdxady );
    const Float64 permanent = ( fabs( bdxcdy ) + fabs( cdxbdy ) ) * fabs( adz ) + ( fabs( cdxady ) + fabs( adxcdy ) ) * fabs( bdz ) + ( fabs( adxbdy ) + fabs( bdxady ) ) * fabs( cdz );
    Int32 shewchukSign = 0;
    if ( !DecideFiltered( det, permanent, ( 7.0 + 56.0 * PredicateEpsilon ) * PredicateEpsilon, shewchukSign ) )
        return false;
    sign = -shewchukSign;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
PredicateFilterInCircle( const Float64 ( &c )[ 8 ], Int32& sign )
{
    const Float64 adx = c[ 0 ] - c[ 6 ], ady = c[ 1 ] - c[ 7 ];
    const Float64 bdx = c[ 2 ] - c[ 6 ], bdy = c[ 3 ] - c[ 7 ];
    const Float64 cdx = c[ 4 ] - c[ 6 ], cdy = c[ 5 ] - c[ 7 ];
    const Float64 differences[ 6 ] = { adx, ady, bdx, bdy, cdx, cdy };
    if ( !DifferencesInFilterRange( differences, 6 ) )
        return false;
    const Float64 bdxcdy = bdx * cdy, cdxbdy = cdx * bdy;
    const Float64 alift = adx * adx + ady * ady;
    const Float64 cdxady = cdx * ady, adxcdy = adx * cdy;
    const Float64 blift = bdx * bdx + bdy * bdy;
    const Float64 adxbdy = adx * bdy, bdxady = bdx * ady;
    const Float64 clift = cdx * cdx + cdy * cdy;
    const Float64 det = alift * ( bdxcdy - cdxbdy ) + blift * ( cdxady - adxcdy ) + clift * ( adxbdy - bdxady );
    const Float64 permanent = ( fabs( bdxcdy ) + fabs( cdxbdy ) ) * alift + ( fabs( cdxady ) + fabs( adxcdy ) ) * blift + ( fabs( adxbdy ) + fabs( bdxady ) ) * clift;
    return DecideFiltered( det, permanent, ( 10.0 + 96.0 * PredicateEpsilon ) * PredicateEpsilon, sign );
}

/*-------------------------------------------------------------------------*/

bool
PredicateFilterInSphere( const Float64 ( &c )[ 15 ], Int32& sign )
{
    /* Shewchuk's insphere (positive inside for his orientation); negated for the orientation of Orient3D */
    const Float64 aex = c[ 0 ] - c[ 12 ], aey = c[ 1 ] - c[ 13 ], aez = c[ 2 ] - c[ 14 ];
    const Float64 bex = c[ 3 ] - c[ 12 ], bey = c[ 4 ] - c[ 13 ], bez = c[ 5 ] - c[ 14 ];
    const Float64 cex = c[ 6 ] - c[ 12 ], cey = c[ 7 ] - c[ 13 ], cez = c[ 8 ] - c[ 14 ];
    const Float64 dex = c[ 9 ] - c[ 12 ], dey = c[ 10 ] - c[ 13 ], dez = c[ 11 ] - c[ 14 ];
    const Float64 differences[ 12 ] = { aex, aey, aez, bex, bey, bez, cex, cey, cez, dex, dey, dez };
    if ( !DifferencesInFilterRange( differences, 12 ) )
        return false;

    const Float64 aexbey = aex * bey, bexaey = bex * aey;
    const Float64 bexcey = bex * cey, cexbey = cex * bey;
    const Float64 cexdey = cex * dey, dexcey = dex * cey;
    const Float64 dexaey = dex * aey, aexdey = aex * dey;
    const Float64 aexcey = aex * cey, cexaey = cex * aey;
    const Float64 bexdey = bex * dey, dexbey = dex * bey;
    const Float64 ab = aexbey - bexaey, bc = bexcey - cexbey, cd = cexdey - dexcey;
    const Float64 da = dexaey - aexdey, ac = aexcey - cexaey, bd = bexdey - dexbey;

    const Float64 abc = aez * bc - bez * ac + cez * ab;
    const Float64 bcd = bez * cd - cez * bd + dez * bc;
    const Float64 cda = cez * da + dez * ac + aez * cd;
    const Float64 dab = dez * ab + aez * bd + bez * da;

    const Float64 alift = aex * aex + aey * aey + aez * aez;
    const Float64 blift = bex * bex + bey * bey + bez * bez;
    const Float64 clift = cex * cex + cey * cey + cez * cez;
    const Float64 dlift = dex * dex + dey * dey + dez * dez;

    const Float64 det = ( dlift * abc - clift * dab ) + ( blift * cda - alift * bcd );

    const Float64 pab = fabs( aexbey ) + fabs( bexaey ), pbc = fabs( bexcey ) + fabs( cexbey ), pcd = fabs( cexdey ) + fabs( dexcey );
    const Float64 pda = fabs( dexaey ) + fabs( aexdey ), pac = fabs( aexcey ) + fabs( cexaey ), pbd = fabs( bexdey ) + fabs( dexbey );
    const Float64 pabc = fabs( aez ) * pbc + fabs( bez ) * pac + fabs( cez ) * pab;
    const Float64 pbcd = fabs( bez ) * pcd + fabs( cez ) * pbd + fabs( dez ) * pbc;
    const Float64 pcda = fabs( cez ) * pda + fabs( dez ) * pac + fabs( aez ) * pcd;
    const Float64 pdab = fabs( dez ) * pab + fabs( aez ) * pbd + fabs( bez ) * pda;
    const Float64 permanent = ( dlift * pabc + clift * pdab ) + ( blift * pcda + alift * pbcd );

    Int32 shewchukSign = 0;
    if ( !DecideFiltered( det, permanent, ( 16.0 + 224.0 * PredicateEpsilon ) * PredicateEpsilon, shewchukSign ) )
        return false;
    sign = -shewchukSign;
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
PredicateExactOrient2D( const Float64 ( &coordinates )[ 6 ] )
{
    CORE::CBigInteger v[ 6 ];
    if ( !ToCommonScaleIntegers( coordinates, 6, v ) )
    {
        GUCEF_GEOMETRY_ASSERT( false );
        return 0;
    }
    return Orient2DExact( v );
}

/*-------------------------------------------------------------------------*/

Int32
PredicateExactOrient3D( const Float64 ( &coordinates )[ 12 ] )
{
    CORE::CBigInteger v[ 12 ];
    if ( !ToCommonScaleIntegers( coordinates, 12, v ) )
    {
        GUCEF_GEOMETRY_ASSERT( false );
        return 0;
    }
    return Orient3DExact( v );
}

/*-------------------------------------------------------------------------*/

Int32
PredicateExactInCircle( const Float64 ( &coordinates )[ 8 ] )
{
    CORE::CBigInteger v[ 8 ];
    if ( !ToCommonScaleIntegers( coordinates, 8, v ) )
    {
        GUCEF_GEOMETRY_ASSERT( false );
        return 0;
    }
    return InCircleExact( v );
}

/*-------------------------------------------------------------------------*/

Int32
PredicateExactInSphere( const Float64 ( &coordinates )[ 15 ] )
{
    CORE::CBigInteger v[ 15 ];
    if ( !ToCommonScaleIntegers( coordinates, 15, v ) )
    {
        GUCEF_GEOMETRY_ASSERT( false );
        return 0;
    }
    return InSphereExact( v );
}

/*-------------------------------------------------------------------------*/

Int32
PredicateExactOrient2D( const SPredicateInteger ( &coordinates )[ 6 ] )
{
    CORE::CBigInteger v[ 6 ];
    ToIntegers( coordinates, 6, v );
    return Orient2DExact( v );
}

/*-------------------------------------------------------------------------*/

Int32
PredicateExactOrient3D( const SPredicateInteger ( &coordinates )[ 12 ] )
{
    CORE::CBigInteger v[ 12 ];
    ToIntegers( coordinates, 12, v );
    return Orient3DExact( v );
}

/*-------------------------------------------------------------------------*/

Int32
PredicateExactInCircle( const SPredicateInteger ( &coordinates )[ 8 ] )
{
    /* differences of at most 65 bits: the determinant needs at most 264 bits, within the 384-bit stack integer */
    return InCircleWide( coordinates );
}

/*-------------------------------------------------------------------------*/

Int32
PredicateExactInSphere( const SPredicateInteger ( &coordinates )[ 15 ] )
{
    /* differences of at most 65 bits: the determinant needs at most 332 bits, within the 384-bit stack integer */
    return InSphereWide( coordinates );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
