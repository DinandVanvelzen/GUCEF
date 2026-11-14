/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>
#include <new>
#include <stdexcept>

#ifndef GUCEF_INVALID_ARG_EXCEPTION_H
#include "gucef_invalid_arg_exception.h"
#define GUCEF_INVALID_ARG_EXCEPTION_H
#endif /* GUCEF_INVALID_ARG_EXCEPTION_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CBIGUNSIGNED_H
#include "gucefCORE_CBigUnsigned.h"
#define GUCEF_CORE_CBIGUNSIGNED_H
#endif /* GUCEF_CORE_CBIGUNSIGNED_H ? */

#include "gucefCORE_CBigInteger.h"

#ifndef GUCEF_STL_H
#include "gucef_stl.h"
#define GUCEF_STL_H
#endif /* GUCEF_STL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef UInt32 TLimb;

/** Limbs of scratch kept on the stack before an operation allocates its scratch block */
static const UInt32 g_bigIntegerStackScratchLimbs = 96;

static const char g_bigIntegerLowerDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
static const char g_bigIntegerUpperDigits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/** Largest power of each base that fits 32 bits (index = base, 0 and 1 unused) and its exponent */
static const UInt32 g_bigIntegerChunkPower[ 37 ] =
{
    0, 0, 2147483648UL, 3486784401UL, 1073741824UL, 1220703125UL, 2176782336UL, 1977326743UL, 1073741824UL,
    3486784401UL, 1000000000UL, 2357947691UL, 429981696UL, 815730721UL, 1475789056UL, 2562890625UL, 268435456UL,
    410338673UL, 612220032UL, 893871739UL, 1280000000UL, 1801088541UL, 2494357888UL, 3404825447UL, 191102976UL,
    244140625UL, 308915776UL, 387420489UL, 481890304UL, 594823321UL, 729000000UL, 887503681UL, 1073741824UL,
    1291467969UL, 1544804416UL, 1838265625UL, 2176782336UL
};
static const UInt8 g_bigIntegerChunkDigits[ 37 ] =
{
    0, 0, 31, 20, 15, 13, 12, 11, 10, 10, 9, 9, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6
};

/** Powers of ten that fit 64 bits */
static const UInt64 g_bigIntegerPow10[ 20 ] =
{
    1ULL, 10ULL, 100ULL, 1000ULL, 10000ULL, 100000ULL, 1000000ULL, 10000000ULL, 100000000ULL, 1000000000ULL,
    10000000000ULL, 100000000000ULL, 1000000000000ULL, 10000000000000ULL, 100000000000000ULL, 1000000000000000ULL,
    10000000000000000ULL, 100000000000000000ULL, 1000000000000000000ULL, 10000000000000000000ULL
};

static const UInt64 g_bigIntegerInt64MaxMagnitude = 0x7FFFFFFFFFFFFFFFULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Where the discarded part of a rounding lies relative to one half of the last unit kept */
enum EBigRoundRelation
{
    BIGROUNDRELATION_EXACT = 0,
    BIGROUNDRELATION_BELOW_HALF,
    BIGROUNDRELATION_HALF,
    BIGROUNDRELATION_ABOVE_HALF
};

/*-------------------------------------------------------------------------*/

static inline bool
IsValidBigRoundingMode( TRoundingMode mode )
{
    return (UInt32) mode <= (UInt32) ROUNDINGMODE_UNNECESSARY;
}

/*-------------------------------------------------------------------------*/

/** Whether the magnitude is incremented (the sign is applied afterwards), same table as the decimal kernel */
static bool
BigRoundingIncrements( TRoundingMode mode, bool negative, bool keptIsOdd, EBigRoundRelation relation )
{
    if ( BIGROUNDRELATION_EXACT == relation )
        return false;

    switch ( mode )
    {
        case ROUNDINGMODE_TOWARD_ZERO:      return false;
        case ROUNDINGMODE_AWAY_FROM_ZERO:   return true;
        case ROUNDINGMODE_FLOOR:            return negative;
        case ROUNDINGMODE_CEILING:          return !negative;
        case ROUNDINGMODE_HALF_UP:          return BIGROUNDRELATION_BELOW_HALF != relation;
        case ROUNDINGMODE_HALF_DOWN:        return BIGROUNDRELATION_ABOVE_HALF == relation;
        case ROUNDINGMODE_HALF_EVEN:        return BIGROUNDRELATION_ABOVE_HALF == relation || ( BIGROUNDRELATION_HALF == relation && keptIsOdd );
        case ROUNDINGMODE_UNNECESSARY:
        default:                            return false;
    }
}

/*-------------------------------------------------------------------------*/

/** True when 'mode' rounds a value of this sign toward zero, so a result beyond the format clamps instead of overflowing */
static inline bool
BigRoundsTowardZero( TRoundingMode mode, bool negative )
{
    return ROUNDINGMODE_TOWARD_ZERO == mode || ( ROUNDINGMODE_FLOOR == mode && !negative ) || ( ROUNDINGMODE_CEILING == mode && negative );
}

/*-------------------------------------------------------------------------*/

static inline UInt32
CountLeadingZeros32( UInt32 value )
{
    if ( 0 == value )
        return 32;
    UInt32 n = 0;
    if ( value <= 0x0000FFFFUL ) { n += 16; value <<= 16; }
    if ( value <= 0x00FFFFFFUL ) { n += 8; value <<= 8; }
    if ( value <= 0x0FFFFFFFUL ) { n += 4; value <<= 4; }
    if ( value <= 0x3FFFFFFFUL ) { n += 2; value <<= 2; }
    if ( value <= 0x7FFFFFFFUL ) { n += 1; }
    return n;
}

/*-------------------------------------------------------------------------*/

static inline UInt32
CountTrailingZeros32( UInt32 value )
{
    if ( 0 == value )
        return 32;
    UInt32 n = 0;
    if ( 0 == ( value & 0x0000FFFFUL ) ) { n += 16; value >>= 16; }
    if ( 0 == ( value & 0x000000FFUL ) ) { n += 8; value >>= 8; }
    if ( 0 == ( value & 0x0000000FUL ) ) { n += 4; value >>= 4; }
    if ( 0 == ( value & 0x00000003UL ) ) { n += 2; value >>= 2; }
    if ( 0 == ( value & 0x00000001UL ) ) { n += 1; }
    return n;
}

/*-------------------------------------------------------------------------*/

static inline UInt32
PopCount32( UInt32 value )
{
    value = value - ( ( value >> 1 ) & 0x55555555UL );
    value = ( value & 0x33333333UL ) + ( ( value >> 2 ) & 0x33333333UL );
    value = ( value + ( value >> 4 ) ) & 0x0F0F0F0FUL;
    return (UInt32) ( value * 0x01010101UL ) >> 24;
}

/*-------------------------------------------------------------------------*/

static inline UInt32
BitLength64( UInt64 value )
{
    UInt32 high = (UInt32) ( value >> 32 );
    if ( 0 != high )
        return 64 - CountLeadingZeros32( high );
    return 32 - CountLeadingZeros32( (UInt32) value );
}

/*-------------------------------------------------------------------------*/

static inline UInt32
TrimmedSize( const TLimb* limbs, UInt32 size )
{
    while ( size > 0 && 0 == limbs[ size-1 ] )
        --size;
    return size;
}

/*-------------------------------------------------------------------------*/

static inline UInt32
BitLengthOf( const TLimb* limbs, UInt32 size )
{
    if ( 0 == size )
        return 0;
    return ( size - 1 ) * 32 + ( 32 - CountLeadingZeros32( limbs[ size-1 ] ) );
}

/*-------------------------------------------------------------------------*/

/** -1, 0, 1 for magnitudes without leading zero limbs */
static Int32
CompareMagnitudes( const TLimb* a, UInt32 an, const TLimb* b, UInt32 bn )
{
    if ( an != bn )
        return an < bn ? -1 : 1;
    for ( UInt32 i=an; i>0; --i )
    {
        if ( a[ i-1 ] != b[ i-1 ] )
            return a[ i-1 ] < b[ i-1 ] ? -1 : 1;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

/** r = a + b for an >= bn. r holds an + 1 limbs and may alias a or b (same index). Returns the used size */
static UInt32
AddMagnitudes( const TLimb* a, UInt32 an, const TLimb* b, UInt32 bn, TLimb* r )
{
    UInt64 carry = 0;
    UInt32 i = 0;
    for ( ; i<bn; ++i )
    {
        carry += (UInt64) a[ i ] + b[ i ];
        r[ i ] = (TLimb) carry;
        carry >>= 32;
    }
    for ( ; i<an; ++i )
    {
        carry += a[ i ];
        r[ i ] = (TLimb) carry;
        carry >>= 32;
    }
    if ( 0 != carry )
    {
        r[ an ] = (TLimb) carry;
        return an + 1;
    }
    return an;
}

/*-------------------------------------------------------------------------*/

/** Whether a + b (an >= bn) carries out of limb an - 1, decided from the top limbs down without adding */
static bool
AddCarriesOut( const TLimb* a, UInt32 an, const TLimb* b, UInt32 bn )
{
    for ( UInt32 i=an; i>0; --i )
    {
        UInt64 sum = (UInt64) a[ i-1 ] + ( i-1 < bn ? b[ i-1 ] : 0 );
        if ( sum > 0xFFFFFFFFULL )
            return true;
        if ( sum < 0xFFFFFFFFULL )
            return false;
        // all ones: a carry from below propagates through, look further down
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** r = a - b for a >= b (magnitudes, an >= bn). r holds an limbs and may alias a or b (same index). Returns the trimmed size */
static UInt32
SubtractMagnitudes( const TLimb* a, UInt32 an, const TLimb* b, UInt32 bn, TLimb* r )
{
    UInt64 borrow = 0;
    UInt32 i = 0;
    for ( ; i<bn; ++i )
    {
        // a negative difference wraps to a value with the top bit set: |difference| <= 2^32
        UInt64 d = (UInt64) a[ i ] - b[ i ] - borrow;
        r[ i ] = (TLimb) d;
        borrow = d >> 63;
    }
    for ( ; i<an; ++i )
    {
        UInt64 d = (UInt64) a[ i ] - borrow;
        r[ i ] = (TLimb) d;
        borrow = d >> 63;
    }
    return TrimmedSize( r, an );
}

/*-------------------------------------------------------------------------*/

/** r[ 0 .. rn ) += b[ 0 .. bn ) for rn >= bn, the carry propagated through r. Returns the carry out of r */
static TLimb
AddInto( TLimb* r, UInt32 rn, const TLimb* b, UInt32 bn )
{
    UInt64 carry = 0;
    UInt32 i = 0;
    for ( ; i<bn; ++i )
    {
        carry += (UInt64) r[ i ] + b[ i ];
        r[ i ] = (TLimb) carry;
        carry >>= 32;
    }
    for ( ; 0 != carry && i<rn; ++i )
    {
        carry += r[ i ];
        r[ i ] = (TLimb) carry;
        carry >>= 32;
    }
    return (TLimb) carry;
}

/*-------------------------------------------------------------------------*/

/** r[ 0 .. rn ) -= b[ 0 .. bn ) for rn >= bn. Returns the borrow out of r */
static TLimb
SubtractInto( TLimb* r, UInt32 rn, const TLimb* b, UInt32 bn )
{
    UInt64 borrow = 0;
    UInt32 i = 0;
    for ( ; i<bn; ++i )
    {
        UInt64 d = (UInt64) r[ i ] - b[ i ] - borrow;
        r[ i ] = (TLimb) d;
        borrow = d >> 63;
    }
    for ( ; 0 != borrow && i<rn; ++i )
    {
        UInt64 d = (UInt64) r[ i ] - borrow;
        r[ i ] = (TLimb) d;
        borrow = d >> 63;
    }
    return (TLimb) borrow;
}

/*-------------------------------------------------------------------------*/

/** r[ 0 .. an ) += a * m, returns the carry limb. Cannot overflow: a*m + r + carry < 2^64 */
static inline TLimb
MultiplyAddLimb( TLimb* r, const TLimb* a, UInt32 an, TLimb m )
{
    UInt64 carry = 0;
    for ( UInt32 i=0; i<an; ++i )
    {
        carry += (UInt64) a[ i ] * m + r[ i ];
        r[ i ] = (TLimb) carry;
        carry >>= 32;
    }
    return (TLimb) carry;
}

/*-------------------------------------------------------------------------*/

/** r = a * m (an limbs), returns the carry limb. r may alias a */
static inline TLimb
MultiplyLimb( TLimb* r, const TLimb* a, UInt32 an, TLimb m )
{
    UInt64 carry = 0;
    for ( UInt32 i=0; i<an; ++i )
    {
        carry += (UInt64) a[ i ] * m;
        r[ i ] = (TLimb) carry;
        carry >>= 32;
    }
    return (TLimb) carry;
}

/*-------------------------------------------------------------------------*/

/** q = a / d (an limbs), returns the remainder. q may alias a */
static inline TLimb
DivideByLimb( TLimb* q, const TLimb* a, UInt32 an, TLimb d )
{
    UInt64 remainder = 0;
    for ( UInt32 i=an; i>0; --i )
    {
        UInt64 current = ( remainder << 32 ) | a[ i-1 ];
        q[ i-1 ] = (TLimb) ( current / d );
        remainder = current % d;
    }
    return (TLimb) remainder;
}

/*-------------------------------------------------------------------------*/

/** a mod d without a quotient */
static inline TLimb
RemainderByLimb( const TLimb* a, UInt32 an, TLimb d )
{
    UInt64 remainder = 0;
    for ( UInt32 i=an; i>0; --i )
        remainder = ( ( remainder << 32 ) | a[ i-1 ] ) % d;
    return (TLimb) remainder;
}

/*-------------------------------------------------------------------------*/

/** Schoolbook product, r holds an + bn limbs and does not overlap a or b */
static void
MultiplySchoolbook( TLimb* r, const TLimb* a, UInt32 an, const TLimb* b, UInt32 bn )
{
    memset( r, 0, ( an + bn ) * sizeof( TLimb ) );
    for ( UInt32 i=0; i<bn; ++i )
        r[ i + an ] = MultiplyAddLimb( r + i, a, an, b[ i ] );
}

/*-------------------------------------------------------------------------*/

/** Schoolbook square: the off-diagonal products once, doubled, plus the squares. r holds 2n limbs, no overlap with a */
static void
SquareSchoolbook( TLimb* r, const TLimb* a, UInt32 n )
{
    memset( r, 0, 2 * n * sizeof( TLimb ) );
    for ( UInt32 i=0; i+1<n; ++i )
        r[ i + n ] = MultiplyAddLimb( r + 2*i + 1, a + i + 1, n - i - 1, a[ i ] );

    // double
    TLimb carryBit = 0;
    for ( UInt32 i=0; i<2*n; ++i )
    {
        TLimb limb = r[ i ];
        r[ i ] = ( limb << 1 ) | carryBit;
        carryBit = limb >> 31;
    }

    // add the squares on the diagonal
    UInt64 carry = 0;
    for ( UInt32 i=0; i<n; ++i )
    {
        UInt64 square = (UInt64) a[ i ] * a[ i ];
        carry += (UInt64) r[ 2*i ] + (TLimb) square;
        r[ 2*i ] = (TLimb) carry;
        carry >>= 32;
        carry += (UInt64) r[ 2*i + 1 ] + ( square >> 32 );
        r[ 2*i + 1 ] = (TLimb) carry;
        carry >>= 32;
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  Scratch limbs the Karatsuba recursion needs for operands of up to n limbs: 4( h + 1 ) per level for the sums and
 *  the middle product, with h = ceil( n / 2 ). Monotone in n, and an upper bound for the squaring recursion as well.
 */
static UInt64
KaratsubaScratchLimbs( UInt64 n )
{
    UInt64 total = 0;
    while ( n >= CBigInteger::KaratsubaThresholdLimbs )
    {
        UInt64 h = ( n + 1 ) / 2;
        total += 4 * ( h + 1 );
        n = h + 1;
    }
    return total;
}

/*-------------------------------------------------------------------------*/

/** Scratch for MultiplyRecursive of an by bn limbs: the chunk product of the unbalanced split plus the recursion */
static UInt64
MultiplyScratchLimbs( UInt32 an, UInt32 bn )
{
    UInt32 larger = an > bn ? an : bn;
    UInt32 smaller = an > bn ? bn : an;
    if ( smaller < CBigInteger::KaratsubaThresholdLimbs )
        return 0;
    return 2 * (UInt64) smaller + KaratsubaScratchLimbs( larger );
}

/*-------------------------------------------------------------------------*/

/**
 *  r = a * b, r holds an + bn limbs and overlaps neither operand. Schoolbook below the threshold, otherwise Karatsuba
 *  with a balanced split at half the larger operand; operands of very different sizes are cut into chunks of the
 *  smaller size and the partial products are added. 'scratch' holds MultiplyScratchLimbs( an, bn ) limbs.
 */
static void
MultiplyRecursive( TLimb* r, const TLimb* a, UInt32 an, const TLimb* b, UInt32 bn, TLimb* scratch )
{
    if ( an < bn )
    {
        const TLimb* t = a; a = b; b = t;
        UInt32 tn = an; an = bn; bn = tn;
    }
    if ( 0 == bn )
    {
        memset( r, 0, an * sizeof( TLimb ) );
        return;
    }
    if ( bn < CBigInteger::KaratsubaThresholdLimbs )
    {
        MultiplySchoolbook( r, a, an, b, bn );
        return;
    }

    UInt32 h = ( an + 1 ) / 2;
    if ( bn <= h )
    {
        // unbalanced: chunks of bn limbs of a times b
        memset( r, 0, ( an + bn ) * sizeof( TLimb ) );
        TLimb* product = scratch;
        TLimb* rest = scratch + 2 * bn;
        for ( UInt32 i=0; i<an; i+=bn )
        {
            UInt32 length = an - i < bn ? an - i : bn;
            MultiplyRecursive( product, a + i, length, b, bn, rest );
            AddInto( r + i, an + bn - i, product, length + bn );
        }
        return;
    }

    // a = a1 B^h + a0, b = b1 B^h + b0
    UInt32 a1n = an - h;
    UInt32 b1n = bn - h;
    MultiplyRecursive( r, a, h, b, h, scratch );                        // z0 in r[ 0 .. 2h )
    MultiplyRecursive( r + 2*h, a + h, a1n, b + h, b1n, scratch );      // z2 in r[ 2h .. an + bn )

    TLimb* sumA = scratch;                  // h + 1
    TLimb* sumB = scratch + h + 1;          // h + 1
    TLimb* middle = scratch + 2*h + 2;      // 2h + 2
    TLimb* next = scratch + 4*h + 4;
    if ( AddMagnitudes( a, h, a + h, a1n, sumA ) == h )
        sumA[ h ] = 0;
    if ( AddMagnitudes( b, h, b + h, b1n, sumB ) == h )
        sumB[ h ] = 0;
    MultiplyRecursive( middle, sumA, h + 1, sumB, h + 1, next );
    SubtractInto( middle, 2*h + 2, r, 2*h );
    SubtractInto( middle, 2*h + 2, r + 2*h, a1n + b1n );
    UInt32 middleSize = TrimmedSize( middle, 2*h + 2 );
    AddInto( r + h, an + bn - h, middle, middleSize );
}

/*-------------------------------------------------------------------------*/

/** r = a^2, r holds 2n limbs and does not overlap a. 'scratch' holds KaratsubaScratchLimbs( n ) limbs */
static void
SquareRecursive( TLimb* r, const TLimb* a, UInt32 n, TLimb* scratch )
{
    if ( n < CBigInteger::KaratsubaThresholdLimbs )
    {
        SquareSchoolbook( r, a, n );
        return;
    }

    UInt32 h = ( n + 1 ) / 2;
    UInt32 a1n = n - h;
    SquareRecursive( r, a, h, scratch );                    // z0
    SquareRecursive( r + 2*h, a + h, a1n, scratch );        // z2

    TLimb* sum = scratch;                   // h + 1
    TLimb* middle = scratch + h + 1;        // 2h + 2
    TLimb* next = scratch + 3*h + 3;
    if ( AddMagnitudes( a, h, a + h, a1n, sum ) == h )
        sum[ h ] = 0;
    SquareRecursive( middle, sum, h + 1, next );
    SubtractInto( middle, 2*h + 2, r, 2*h );
    SubtractInto( middle, 2*h + 2, r + 2*h, 2*a1n );
    UInt32 middleSize = TrimmedSize( middle, 2*h + 2 );
    AddInto( r + h, 2*n - h, middle, middleSize );
}

/*-------------------------------------------------------------------------*/

/**
 *  Knuth algorithm D (TAOCP 4.3.1): u (un limbs) divided by v (vn >= 2 limbs, top limb non-zero), un >= vn.
 *  q receives un - vn + 1 limbs (may be NULL), r receives vn limbs (may be NULL). 'work' holds un + vn + 1 limbs.
 *  None of the outputs may overlap u, v or work.
 */
static void
DivideKnuth( const TLimb* u, UInt32 un, const TLimb* v, UInt32 vn, TLimb* q, TLimb* r, TLimb* work )
{
    TLimb* vNorm = work;
    TLimb* uNorm = work + vn;

    // D1: normalise so the top bit of the divisor is set
    UInt32 shift = CountLeadingZeros32( v[ vn-1 ] );
    if ( shift > 0 )
    {
        for ( UInt32 i=vn-1; i>0; --i )
            vNorm[ i ] = ( v[ i ] << shift ) | ( v[ i-1 ] >> ( 32 - shift ) );
        vNorm[ 0 ] = v[ 0 ] << shift;
        uNorm[ un ] = u[ un-1 ] >> ( 32 - shift );
        for ( UInt32 i=un-1; i>0; --i )
            uNorm[ i ] = ( u[ i ] << shift ) | ( u[ i-1 ] >> ( 32 - shift ) );
        uNorm[ 0 ] = u[ 0 ] << shift;
    }
    else
    {
        memcpy( vNorm, v, vn * sizeof( TLimb ) );
        memcpy( uNorm, u, un * sizeof( TLimb ) );
        uNorm[ un ] = 0;
    }

    const UInt64 base = 0x100000000ULL;
    const UInt64 vTop = vNorm[ vn-1 ];
    const UInt64 vNext = vNorm[ vn-2 ];

    for ( UInt32 jj=un-vn+1; jj>0; --jj )
    {
        UInt32 j = jj - 1;

        // D3: estimate the quotient digit from the top two limbs, then correct it (at most twice)
        UInt64 numerator = ( (UInt64) uNorm[ j+vn ] << 32 ) | uNorm[ j+vn-1 ];
        UInt64 qHat = numerator / vTop;
        UInt64 rHat = numerator - qHat * vTop;
        while ( qHat >= base || qHat * vNext > ( ( rHat << 32 ) | uNorm[ j+vn-2 ] ) )
        {
            --qHat;
            rHat += vTop;
            if ( rHat >= base )
                break;
        }

        // D4: multiply and subtract
        UInt64 carry = 0;
        UInt64 borrow = 0;
        for ( UInt32 i=0; i<vn; ++i )
        {
            UInt64 product = qHat * vNorm[ i ] + carry;
            carry = product >> 32;
            UInt64 difference = (UInt64) uNorm[ i+j ] - ( product & 0xFFFFFFFFULL ) - borrow;
            uNorm[ i+j ] = (TLimb) difference;
            borrow = difference >> 63;
        }
        UInt64 top = (UInt64) uNorm[ j+vn ] - carry - borrow;
        uNorm[ j+vn ] = (TLimb) top;

        // D5 / D6: the estimate was one too large, add the divisor back
        if ( 0 != ( top >> 63 ) )
        {
            --qHat;
            UInt64 sum = 0;
            for ( UInt32 i=0; i<vn; ++i )
            {
                sum += (UInt64) uNorm[ i+j ] + vNorm[ i ];
                uNorm[ i+j ] = (TLimb) sum;
                sum >>= 32;
            }
            uNorm[ j+vn ] = (TLimb) ( uNorm[ j+vn ] + (TLimb) sum );
        }

        if ( GUCEF_NULL != q )
            q[ j ] = (TLimb) qHat;
    }

    // D8: de-normalise the remainder
    if ( GUCEF_NULL != r )
    {
        if ( shift > 0 )
        {
            for ( UInt32 i=0; i+1<vn; ++i )
                r[ i ] = ( uNorm[ i ] >> shift ) | ( uNorm[ i+1 ] << ( 32 - shift ) );
            r[ vn-1 ] = uNorm[ vn-1 ] >> shift;
        }
        else
        {
            memcpy( r, uNorm, vn * sizeof( TLimb ) );
        }
    }
}

/*-------------------------------------------------------------------------*/

/** Bits [ position, position + width ) of the magnitude, width <= 32, bits beyond the value are 0 */
static inline UInt32
ExtractBits( const TLimb* limbs, UInt32 size, UInt64 position, UInt32 width )
{
    UInt64 limbIndex = position / 32;
    UInt32 bitIndex = (UInt32) ( position % 32 );
    UInt64 window = 0;
    if ( limbIndex < size )
        window = limbs[ limbIndex ];
    if ( limbIndex + 1 < size )
        window |= (UInt64) limbs[ limbIndex + 1 ] << 32;
    window >>= bitIndex;
    return (UInt32) ( window & ( ( 1ULL << width ) - 1 ) );
}

/*-------------------------------------------------------------------------*/

/** log2 of a power of two base, 0 for other bases */
static inline UInt32
PowerOfTwoBaseBits( UInt32 base )
{
    switch ( base )
    {
        case 2: return 1;
        case 4: return 2;
        case 8: return 3;
        case 16: return 4;
        case 32: return 5;
        default: return 0;
    }
}

/*-------------------------------------------------------------------------*/

/** floor( log2( base ) ) for base >= 2 */
static inline UInt32
FloorLog2( UInt32 base )
{
    return 31 - CountLeadingZeros32( base );
}

/*-------------------------------------------------------------------------*/

static inline Int32
DigitValue( char c )
{
    if ( c >= '0' && c <= '9' )
        return c - '0';
    if ( c >= 'a' && c <= 'z' )
        return c - 'a' + 10;
    if ( c >= 'A' && c <= 'Z' )
        return c - 'A' + 10;
    return -1;
}

/*-------------------------------------------------------------------------*/

/**
 *  Limb scratch of an operation: on the stack up to g_bigIntegerStackScratchLimbs limbs, otherwise one block from the
 *  allocator. Throws std::bad_alloc.
 */
class CBigIntegerScratch
{
    public:

    explicit CBigIntegerScratch( UInt64 limbCount )
        : m_heap( GUCEF_NULL )
        , m_count( 0 )
    {
        if ( limbCount > g_bigIntegerStackScratchLimbs )
        {
            if ( limbCount > 0xFFFFFFFFULL )
                throw std::bad_alloc();
            gucef_allocator< TLimb > allocator;
            m_heap = allocator.allocate( (size_t) limbCount );
            if ( GUCEF_NULL == m_heap )
                throw std::bad_alloc();
            m_count = (UInt32) limbCount;
        }
    }

    ~CBigIntegerScratch()
    {
        if ( GUCEF_NULL != m_heap )
        {
            gucef_allocator< TLimb > allocator;
            allocator.deallocate( m_heap, m_count );
        }
    }

    TLimb* Get( void ) { return GUCEF_NULL != m_heap ? m_heap : m_stack; }

    private:

    CBigIntegerScratch( const CBigIntegerScratch& );
    CBigIntegerScratch& operator=( const CBigIntegerScratch& );

    TLimb m_stack[ g_bigIntegerStackScratchLimbs ];
    TLimb* m_heap;
    UInt32 m_count;
};

/*-------------------------------------------------------------------------*/

/** Classifies a remainder r of a division by d (both magnitudes) against d / 2 */
static EBigRoundRelation
ClassifyBigRemainder( const CBigInteger& remainder, const CBigInteger& divisor )
{
    if ( remainder.IsZero() )
        return BIGROUNDRELATION_EXACT;

    // 2r against d, done as r against d - r so nothing is shifted
    CBigInteger rest;
    CBigInteger::ThrowOnError( divisor.Abs( rest ) );
    CBigInteger magnitude;
    CBigInteger::ThrowOnError( remainder.Abs( magnitude ) );
    CBigInteger::ThrowOnError( rest.Subtract( magnitude, rest ) );
    Int32 cmp = magnitude.Compare( rest );
    if ( cmp < 0 )
        return BIGROUNDRELATION_BELOW_HALF;
    if ( cmp > 0 )
        return BIGROUNDRELATION_ABOVE_HALF;
    return BIGROUNDRELATION_HALF;
}

/*-------------------------------------------------------------------------*/

/**
 *  Rounds the positive value ( q + sticky ) * 2^exponent2 (q != 0; when sticky, q has more than 'precisionBits' bits)
 *  once to a binary format with 'precisionBits' significand bits, normal exponents minExponent .. maxExponent and
 *  subnormals below. The result is written as a double (exact for the float format as well).
 */
static TBigIntegerStatus
RoundToBinaryFormat( bool negative                  ,
                     UInt64 q                       ,
                     Int64 exponent2                ,
                     bool sticky                    ,
                     TRoundingMode mode             ,
                     UInt32 precisionBits           ,
                     Int32 minExponent              ,
                     Int32 maxExponent              ,
                     Float64& value                 )
{
    Int64 qBits = (Int64) BitLength64( q );
    Int64 leadingExponent = qBits - 1 + exponent2;
    Int64 precision = (Int64) precisionBits;
    if ( leadingExponent < minExponent )
        precision -= (Int64) minExponent - leadingExponent;     // subnormal: fewer bits, may be <= 0
    Int64 drop = qBits - precision;

    UInt64 kept = q;
    EBigRoundRelation relation = sticky ? BIGROUNDRELATION_BELOW_HALF : BIGROUNDRELATION_EXACT;
    if ( drop > 0 )
    {
        if ( drop > 64 )
        {
            kept = 0;
            relation = BIGROUNDRELATION_BELOW_HALF;
        }
        else
        {
            UInt64 discarded = 64 == drop ? q : ( q & ( ( 1ULL << drop ) - 1 ) );
            UInt64 half = 1ULL << ( drop - 1 );
            kept = 64 == drop ? 0 : ( q >> drop );
            if ( discarded < half )
                relation = ( 0 == discarded && !sticky ) ? BIGROUNDRELATION_EXACT : BIGROUNDRELATION_BELOW_HALF;
            else
            if ( discarded > half )
                relation = BIGROUNDRELATION_ABOVE_HALF;
            else
                relation = sticky ? BIGROUNDRELATION_ABOVE_HALF : BIGROUNDRELATION_HALF;
        }
    }
    else
    {
        drop = 0;
    }

    TBigIntegerStatus status = BIGINTEGERSTATUS_OK;
    if ( BIGROUNDRELATION_EXACT != relation )
    {
        if ( ROUNDINGMODE_UNNECESSARY == mode )
            return BIGINTEGERSTATUS_ROUNDING_REQUIRED;
        status = BIGINTEGERSTATUS_INEXACT;
        if ( BigRoundingIncrements( mode, negative, 0 != ( kept & 1 ), relation ) )
            ++kept;
    }

    Int64 resultExponent2 = exponent2 + drop;
    if ( 0 == kept )
    {
        value = negative ? -0.0 : 0.0;
        return status;
    }

    Int64 resultLeading = (Int64) BitLength64( kept ) - 1 + resultExponent2;
    if ( resultLeading > maxExponent )
    {
        if ( !BigRoundsTowardZero( mode, negative ) )
            return BIGINTEGERSTATUS_OVERFLOW;
        // the largest finite value of the format
        Float64 largest = ldexp( (Float64) ( ( 1ULL << precisionBits ) - 1 ), maxExponent + 1 - (Int32) precisionBits );
        value = negative ? -largest : largest;
        return BIGINTEGERSTATUS_INEXACT;
    }

    Float64 magnitude = ldexp( (Float64) kept, (int) resultExponent2 );
    value = negative ? -magnitude : magnitude;
    return status;
}

/*-------------------------------------------------------------------------*/

/** n / d rounded once to a binary format, see RoundToBinaryFormat */
static TBigIntegerStatus
RatioToBinaryFormat( const CBigInteger& numerator   ,
                     const CBigInteger& denominator ,
                     TRoundingMode mode             ,
                     UInt32 precisionBits           ,
                     Int32 minExponent              ,
                     Int32 maxExponent              ,
                     Float64& value                 )
{
    if ( denominator.IsZero() )
        return BIGINTEGERSTATUS_DIVISION_BY_ZERO;
    if ( !IsValidBigRoundingMode( mode ) )
        return BIGINTEGERSTATUS_INVALID_OPERAND;
    if ( numerator.IsZero() )
    {
        value = 0.0;
        return BIGINTEGERSTATUS_OK;
    }

    try
    {
        bool negative = numerator.IsNegative() != denominator.IsNegative();
        CBigInteger n;
        CBigInteger d;
        CBigInteger::ThrowOnError( numerator.Abs( n ) );
        CBigInteger::ThrowOnError( denominator.Abs( d ) );

        // scale so the integer quotient has precision + 3 or precision + 4 bits
        Int64 shift = (Int64) precisionBits + 3 - ( (Int64) n.GetBitLength() - (Int64) d.GetBitLength() );
        if ( shift > 0 )
        {
            if ( shift > 0xFFFFFFFFLL )
                return BIGINTEGERSTATUS_OUT_OF_MEMORY;
            CBigInteger::ThrowOnError( n.ShiftLeft( (UInt32) shift, n ) );
        }
        else
        if ( shift < 0 )
        {
            if ( -shift > 0xFFFFFFFFLL )
                return BIGINTEGERSTATUS_OUT_OF_MEMORY;
            CBigInteger::ThrowOnError( d.ShiftLeft( (UInt32) -shift, d ) );
        }

        CBigInteger quotient;
        CBigInteger remainder;
        CBigInteger::ThrowOnError( n.DivMod( d, quotient, remainder ) );
        UInt64 q = 0;
        CBigInteger::ThrowOnError( quotient.ToUInt64( q ) );
        return RoundToBinaryFormat( negative, q, -shift, !remainder.IsZero(), mode, precisionBits, minExponent, maxExponent, value );
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
    catch ( const std::length_error& )
    {
        return BIGINTEGERSTATUS_OVERFLOW;
    }
}

/*-------------------------------------------------------------------------*/

static inline bool
IsFiniteBigFloat64( Float64 value )
{
    // false for NaN and both infinities, without relying on C99 / C++11 classification functions
    return ( value - value ) == ( value - value ) && value == value;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CBigInteger::CBigInteger( const CBigInteger& src )
    : m_size( 0 )
    , m_capacity( 0 )
    , m_negative( false )
{GUCEF_TRACE;

    AssignLimbs( src.GetLimbs(), src.m_size, src.m_negative );
}

/*-------------------------------------------------------------------------*/

CBigInteger::~CBigInteger()
{GUCEF_TRACE;

    ReleaseHeap();
}

/*-------------------------------------------------------------------------*/

CBigInteger&
CBigInteger::operator=( const CBigInteger& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        if ( GetCapacity() >= src.m_size )
        {
            if ( src.m_size > 0 )
                memcpy( MutableLimbs(), src.GetLimbs(), src.m_size * sizeof( TLimb ) );
            m_size = src.m_size;
            m_negative = src.m_negative;
        }
        else
        {
            CBigInteger copy( src );
            Swap( copy );
        }
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

CBigInteger::CBigInteger( CBigInteger&& src ) GUCEF_NOEXCEPT
    : m_storage( src.m_storage )
    , m_size( src.m_size )
    , m_capacity( src.m_capacity )
    , m_negative( src.m_negative )
{GUCEF_TRACE;

    src.m_capacity = 0;
    src.m_size = 0;
    src.m_negative = false;
}

/*-------------------------------------------------------------------------*/

CBigInteger&
CBigInteger::operator=( CBigInteger&& src ) GUCEF_NOEXCEPT
{GUCEF_TRACE;

    if ( this != &src )
    {
        ReleaseHeap();
        m_storage = src.m_storage;
        m_size = src.m_size;
        m_capacity = src.m_capacity;
        m_negative = src.m_negative;
        src.m_capacity = 0;
        src.m_size = 0;
        src.m_negative = false;
    }
    return *this;
}

#endif /* GUCEF_MOVE_SEMANTICS_SUPPORTED ? */

/*-------------------------------------------------------------------------*/

void
CBigInteger::Swap( CBigInteger& other ) GUCEF_NOEXCEPT
{
    UStorage storage = m_storage;
    m_storage = other.m_storage;
    other.m_storage = storage;

    UInt32 size = m_size;
    m_size = other.m_size;
    other.m_size = size;

    UInt32 capacity = m_capacity;
    m_capacity = other.m_capacity;
    other.m_capacity = capacity;

    bool negative = m_negative;
    m_negative = other.m_negative;
    other.m_negative = negative;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::Compact( void )
{GUCEF_TRACE;

    if ( 0 == m_capacity || m_capacity == m_size )
        return BIGINTEGERSTATUS_OK;
    if ( m_size <= InlineLimbCount )
    {
        TLimb* heap = m_storage.heapLimbs;
        UInt32 capacity = m_capacity;
        if ( m_size > 0 )
            memcpy( m_storage.inlineLimbs, heap, m_size * sizeof( TLimb ) );
        m_capacity = 0;
        FreeLimbs( heap, capacity );
        return BIGINTEGERSTATUS_OK;
    }
    try
    {
        CBigInteger copy( *this );
        Swap( copy );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
}

/*-------------------------------------------------------------------------*/

CBigInteger::TLimb*
CBigInteger::AllocateLimbs( UInt32 limbCount )
{
    if ( limbCount > MaxLimbCount + 2 )
        throw std::bad_alloc();
    gucef_allocator< TLimb > allocator;
    TLimb* limbs = allocator.allocate( limbCount );
    if ( GUCEF_NULL == limbs )
        throw std::bad_alloc();
    return limbs;
}

/*-------------------------------------------------------------------------*/

void
CBigInteger::FreeLimbs( TLimb* limbs, UInt32 limbCount )
{
    gucef_allocator< TLimb > allocator;
    allocator.deallocate( limbs, limbCount );
}

/*-------------------------------------------------------------------------*/

void
CBigInteger::ReleaseHeap( void )
{
    if ( 0 != m_capacity )
    {
        FreeLimbs( m_storage.heapLimbs, m_capacity );
        m_capacity = 0;
    }
}

/*-------------------------------------------------------------------------*/

void
CBigInteger::ReserveDiscard( UInt32 limbCount )
{
    if ( limbCount <= GetCapacity() )
        return;
    TLimb* limbs = AllocateLimbs( limbCount );
    ReleaseHeap();
    m_storage.heapLimbs = limbs;
    m_capacity = limbCount;
    m_size = 0;
    m_negative = false;
}

/*-------------------------------------------------------------------------*/

void
CBigInteger::ReserveKeep( UInt32 limbCount, bool geometric )
{
    UInt32 capacity = GetCapacity();
    if ( limbCount <= capacity )
        return;
    UInt32 newCapacity = limbCount;
    if ( geometric && capacity <= ( MaxLimbCount + 2 ) / 2 && 2 * capacity > newCapacity )
        newCapacity = 2 * capacity;
    TLimb* limbs = AllocateLimbs( newCapacity );
    if ( m_size > 0 )
        memcpy( limbs, GetLimbs(), m_size * sizeof( TLimb ) );
    ReleaseHeap();
    m_storage.heapLimbs = limbs;
    m_capacity = newCapacity;
}

/*-------------------------------------------------------------------------*/

void
CBigInteger::AssignLimbs( const TLimb* limbs, UInt32 count, bool negative )
{
    count = TrimmedSize( limbs, count );
    ReserveDiscard( count );
    if ( count > 0 )
        memcpy( MutableLimbs(), limbs, count * sizeof( TLimb ) );
    m_size = count;
    m_negative = negative && count > 0;
}

/*-------------------------------------------------------------------------*/

void
CBigInteger::Trim( void )
{
    m_size = TrimmedSize( GetLimbs(), m_size );
    if ( 0 == m_size )
        m_negative = false;
}

/*-------------------------------------------------------------------------*/

void
CBigInteger::SetZero( void )
{
    m_size = 0;
    m_negative = false;
}

/*-------------------------------------------------------------------------*/

void
CBigInteger::SetMagnitude64( UInt64 magnitude, bool negative )
{
    // every storage holds at least InlineLimbCount >= 2 limbs
    TLimb* limbs = MutableLimbs();
    limbs[ 0 ] = (TLimb) magnitude;
    limbs[ 1 ] = (TLimb) ( magnitude >> 32 );
    m_size = 0 != limbs[ 1 ] ? 2 : ( 0 != limbs[ 0 ] ? 1 : 0 );
    m_negative = negative && 0 != m_size;
}

/*-------------------------------------------------------------------------*/

CBigInteger::TLimb*
CBigInteger::PrepareOutput( UInt32 limbCount, bool mustUseTemporary, bool geometric, CBigInteger& temporary )
{
    if ( !mustUseTemporary && GetCapacity() >= limbCount )
        return MutableLimbs();

    UInt32 capacity = limbCount;
    if ( geometric )
    {
        UInt32 current = GetCapacity();
        if ( current <= ( MaxLimbCount + 2 ) / 2 && 2 * current > capacity )
            capacity = 2 * current;
    }
    temporary.ReserveDiscard( capacity );
    return temporary.MutableLimbs();
}

/*-------------------------------------------------------------------------*/

void
CBigInteger::CommitOutput( const TLimb* written, UInt32 size, bool negative, CBigInteger& temporary )
{
    if ( written != GetLimbs() )
        Swap( temporary );
    m_size = size;
    m_negative = negative;
    Trim();
}

/*-------------------------------------------------------------------------*/

CBigInteger
CBigInteger::Zero( void )
{
    return CBigInteger();
}

/*-------------------------------------------------------------------------*/

CBigInteger
CBigInteger::One( void )
{
    return CBigInteger( 1 );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::FromInt64( Int64 value, CBigInteger& result )
{GUCEF_TRACE;

    result.SetMagnitude64( value < 0 ? 0 - (UInt64) value : (UInt64) value, value < 0 );
    return BIGINTEGERSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::FromUInt64( UInt64 value, CBigInteger& result )
{GUCEF_TRACE;

    result.SetMagnitude64( value, false );
    return BIGINTEGERSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::FromCUInt128( const CUInt128& value, CBigInteger& result )
{GUCEF_TRACE;

    TLimb limbs[ 4 ];
    limbs[ 0 ] = (TLimb) value.low;
    limbs[ 1 ] = (TLimb) ( value.low >> 32 );
    limbs[ 2 ] = (TLimb) value.high;
    limbs[ 3 ] = (TLimb) ( value.high >> 32 );
    return FromLimbs( limbs, 4, false, result );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::FromCInt128( const CInt128& value, CBigInteger& result )
{GUCEF_TRACE;

    CUInt128 magnitude = value.GetMagnitude();
    TLimb limbs[ 4 ];
    limbs[ 0 ] = (TLimb) magnitude.low;
    limbs[ 1 ] = (TLimb) ( magnitude.low >> 32 );
    limbs[ 2 ] = (TLimb) magnitude.high;
    limbs[ 3 ] = (TLimb) ( magnitude.high >> 32 );
    return FromLimbs( limbs, 4, value.IsNegative(), result );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::FromLimbs( const UInt32* limbs, UInt32 limbCount, bool negative, CBigInteger& result )
{GUCEF_TRACE;

    if ( GUCEF_NULL == limbs && limbCount > 0 )
        return BIGINTEGERSTATUS_INVALID_OPERAND;
    limbCount = GUCEF_NULL != limbs ? TrimmedSize( limbs, limbCount ) : 0;
    if ( limbCount > MaxLimbCount )
        return BIGINTEGERSTATUS_OVERFLOW;
    try
    {
        CBigInteger temporary;
        TLimb* out = result.PrepareOutput( limbCount, false, false, temporary );
        if ( limbCount > 0 && out != limbs )
            memmove( out, limbs, limbCount * sizeof( TLimb ) );
        result.CommitOutput( out, limbCount, negative, temporary );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::FromBigUnsigned( const CBigUnsigned& value, CBigInteger& result )
{GUCEF_TRACE;

    return FromLimbs( value.GetLimbs(), value.GetLimbCount(), false, result );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ToBigUnsigned( CBigUnsigned& value ) const
{GUCEF_TRACE;

    if ( m_negative )
        return BIGINTEGERSTATUS_INVALID_OPERAND;
    try
    {
        value.SetFromLimbs( GetLimbs(), m_size );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::FromBigEndianBytes( const void* bytes, UInt32 size, bool negative, CBigInteger& result )
{GUCEF_TRACE;

    if ( GUCEF_NULL == bytes && size > 0 )
        return BIGINTEGERSTATUS_INVALID_OPERAND;
    const UInt8* data = static_cast< const UInt8* >( bytes );
    while ( size > 0 && 0 == *data )
    {
        ++data;
        --size;
    }
    UInt32 limbCount = ( size + 3 ) / 4;
    if ( limbCount > MaxLimbCount )
        return BIGINTEGERSTATUS_OVERFLOW;
    try
    {
        CBigInteger temporary;
        TLimb* out = result.PrepareOutput( limbCount, true, false, temporary );
        for ( UInt32 i=0; i<limbCount; ++i )
            out[ i ] = 0;
        for ( UInt32 i=0; i<size; ++i )
        {
            UInt32 position = size - 1 - i;        // byte significance
            out[ position / 4 ] |= (TLimb) data[ i ] << ( 8 * ( position % 4 ) );
        }
        result.CommitOutput( out, limbCount, negative, temporary );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::FromTwosComplementBigEndian( const void* bytes, UInt32 size, CBigInteger& result )
{GUCEF_TRACE;

    if ( GUCEF_NULL == bytes && size > 0 )
        return BIGINTEGERSTATUS_INVALID_OPERAND;
    if ( 0 == size )
    {
        result.SetZero();
        return BIGINTEGERSTATUS_OK;
    }
    const UInt8* data = static_cast< const UInt8* >( bytes );
    if ( 0 == ( data[ 0 ] & 0x80 ) )
        return FromBigEndianBytes( bytes, size, false, result );

    // negative: magnitude = ~pattern + 1
    try
    {
        CBigInteger pattern;
        UInt32 limbCount = ( size + 3 ) / 4;
        if ( limbCount > MaxLimbCount )
            return BIGINTEGERSTATUS_OVERFLOW;
        pattern.ReserveDiscard( limbCount );
        TLimb* limbs = pattern.MutableLimbs();
        for ( UInt32 i=0; i<limbCount; ++i )
            limbs[ i ] = 0;
        for ( UInt32 i=0; i<size; ++i )
        {
            UInt32 position = size - 1 - i;
            limbs[ position / 4 ] |= (TLimb) (UInt8) ~data[ i ] << ( 8 * ( position % 4 ) );
        }
        pattern.m_size = limbCount;
        pattern.Trim();
        CBigInteger magnitude;
        TBigIntegerStatus status = pattern.Add( 1, magnitude );
        if ( BigIntegerStatusIsError( status ) )
            return status;
        magnitude.m_negative = !magnitude.IsZero();
        result.Swap( magnitude );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ToBigEndianBytes( void* buffer, UInt32 size ) const
{GUCEF_TRACE;

    UInt32 needed = GetByteLength();
    if ( needed > size || ( GUCEF_NULL == buffer && size > 0 ) )
        return BIGINTEGERSTATUS_OVERFLOW;
    UInt8* out = static_cast< UInt8* >( buffer );
    const TLimb* limbs = GetLimbs();
    for ( UInt32 i=0; i<size; ++i )
    {
        UInt32 position = size - 1 - i;
        UInt32 limbIndex = position / 4;
        out[ i ] = limbIndex < m_size ? (UInt8) ( limbs[ limbIndex ] >> ( 8 * ( position % 4 ) ) ) : 0;
    }
    return BIGINTEGERSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ToTwosComplementBigEndian( void* buffer, UInt32 size ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == buffer && size > 0 )
        return BIGINTEGERSTATUS_OVERFLOW;
    if ( 0 == size )
        return IsZero() ? BIGINTEGERSTATUS_OK : BIGINTEGERSTATUS_OVERFLOW;

    // fits when -2^( 8 size - 1 ) <= x < 2^( 8 size - 1 )
    UInt64 bits = (UInt64) size * 8;
    UInt32 bitLength = GetBitLength();
    if ( !m_negative )
    {
        if ( (UInt64) bitLength > bits - 1 )
            return BIGINTEGERSTATUS_OVERFLOW;
        return ToBigEndianBytes( buffer, size );
    }
    // a negative value fits when its magnitude is at most 2^( bits - 1 )
    if ( (UInt64) bitLength > bits || ( (UInt64) bitLength == bits && GetLowestSetBit() != bitLength - 1 ) )
        return BIGINTEGERSTATUS_OVERFLOW;

    // ~( magnitude - 1 ), byte by byte from the least significant end
    UInt8* out = static_cast< UInt8* >( buffer );
    const TLimb* limbs = GetLimbs();
    UInt32 borrow = 1;
    for ( UInt32 i=0; i<size; ++i )
    {
        UInt32 position = i;
        UInt32 limbIndex = position / 4;
        UInt32 byteValue = limbIndex < m_size ? (UInt8) ( limbs[ limbIndex ] >> ( 8 * ( position % 4 ) ) ) : 0;
        UInt32 difference = byteValue - borrow;
        borrow = byteValue < borrow ? 1 : 0;
        out[ size - 1 - i ] = (UInt8) ~(UInt8) difference;
    }
    return BIGINTEGERSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

UInt32
CBigInteger::GetBitLength( void ) const
{
    return BitLengthOf( GetLimbs(), m_size );
}

/*-------------------------------------------------------------------------*/

UInt32
CBigInteger::GetByteLength( void ) const
{
    return ( GetBitLength() + 7 ) / 8;
}

/*-------------------------------------------------------------------------*/

UInt32
CBigInteger::GetLowestSetBit( void ) const
{
    const TLimb* limbs = GetLimbs();
    for ( UInt32 i=0; i<m_size; ++i )
    {
        if ( 0 != limbs[ i ] )
            return i * 32 + CountTrailingZeros32( limbs[ i ] );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::PopCount( UInt32& count ) const
{GUCEF_TRACE;

    if ( m_negative )
        return BIGINTEGERSTATUS_INVALID_OPERAND;
    const TLimb* limbs = GetLimbs();
    UInt32 total = 0;
    for ( UInt32 i=0; i<m_size; ++i )
        total += PopCount32( limbs[ i ] );
    count = total;
    return BIGINTEGERSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

UInt64
CBigInteger::GetHash( void ) const
{GUCEF_TRACE;

    // splitmix64 finalizer over the canonical limbs: equal values have equal limbs and signs
    UInt64 h = m_negative ? 0x6A09E667F3BCC909ULL : 0xBB67AE8584CAA73BULL;
    const TLimb* limbs = GetLimbs();
    for ( UInt32 i=0; i<m_size; ++i )
    {
        UInt64 z = h + 0x9E3779B97F4A7C15ULL + limbs[ i ];
        z = ( z ^ ( z >> 30 ) ) * 0xBF58476D1CE4E5B9ULL;
        z = ( z ^ ( z >> 27 ) ) * 0x94D049BB133111EBULL;
        h = z ^ ( z >> 31 );
    }
    return h ^ m_size;
}

/*-------------------------------------------------------------------------*/

bool
CBigInteger::FitsUInt64( void ) const
{
    return !m_negative && m_size <= 2;
}

/*-------------------------------------------------------------------------*/

bool
CBigInteger::FitsInt64( void ) const
{
    if ( m_size > 2 )
        return false;
    UInt64 magnitude = 0;
    const TLimb* limbs = GetLimbs();
    if ( m_size > 0 ) magnitude = limbs[ 0 ];
    if ( m_size > 1 ) magnitude |= (UInt64) limbs[ 1 ] << 32;
    return magnitude <= g_bigIntegerInt64MaxMagnitude + ( m_negative ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

bool
CBigInteger::FitsCUInt128( void ) const
{
    return !m_negative && m_size <= 4;
}

/*-------------------------------------------------------------------------*/

bool
CBigInteger::FitsCInt128( void ) const
{
    UInt32 bits = GetBitLength();
    if ( bits <= 127 )
        return true;
    // -2^127 is the only value of 128 bits that fits
    return m_negative && 128 == bits && GetLowestSetBit() == 127;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ToInt64( Int64& value ) const
{GUCEF_TRACE;

    if ( !FitsInt64() )
        return BIGINTEGERSTATUS_OVERFLOW;
    UInt64 magnitude = 0;
    const TLimb* limbs = GetLimbs();
    if ( m_size > 0 ) magnitude = limbs[ 0 ];
    if ( m_size > 1 ) magnitude |= (UInt64) limbs[ 1 ] << 32;
    value = m_negative ? (Int64) ( 0 - magnitude ) : (Int64) magnitude;
    return BIGINTEGERSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ToUInt64( UInt64& value ) const
{GUCEF_TRACE;

    if ( !FitsUInt64() )
        return BIGINTEGERSTATUS_OVERFLOW;
    UInt64 magnitude = 0;
    const TLimb* limbs = GetLimbs();
    if ( m_size > 0 ) magnitude = limbs[ 0 ];
    if ( m_size > 1 ) magnitude |= (UInt64) limbs[ 1 ] << 32;
    value = magnitude;
    return BIGINTEGERSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ToCUInt128( CUInt128& value ) const
{GUCEF_TRACE;

    if ( !FitsCUInt128() )
        return BIGINTEGERSTATUS_OVERFLOW;
    TLimb limbs[ 4 ] = { 0, 0, 0, 0 };
    if ( m_size > 0 )
        memcpy( limbs, GetLimbs(), m_size * sizeof( TLimb ) );
    value = CUInt128( ( (UInt64) limbs[ 3 ] << 32 ) | limbs[ 2 ], ( (UInt64) limbs[ 1 ] << 32 ) | limbs[ 0 ] );
    return BIGINTEGERSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ToCInt128( CInt128& value ) const
{GUCEF_TRACE;

    if ( !FitsCInt128() )
        return BIGINTEGERSTATUS_OVERFLOW;
    TLimb limbs[ 4 ] = { 0, 0, 0, 0 };
    if ( m_size > 0 )
        memcpy( limbs, GetLimbs(), m_size * sizeof( TLimb ) );
    CUInt128 magnitude( ( (UInt64) limbs[ 3 ] << 32 ) | limbs[ 2 ], ( (UInt64) limbs[ 1 ] << 32 ) | limbs[ 0 ] );
    CInt128 out;
    if ( !CInt128::FromSignMagnitude( m_negative, magnitude, out ) )
        return BIGINTEGERSTATUS_OVERFLOW;
    value = out;
    return BIGINTEGERSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

Int32
CBigInteger::Compare( const CBigInteger& other ) const
{
    if ( m_negative != other.m_negative )
        return m_negative ? -1 : 1;
    Int32 cmp = CompareMagnitudes( GetLimbs(), m_size, other.GetLimbs(), other.m_size );
    return m_negative ? -cmp : cmp;
}

/*-------------------------------------------------------------------------*/

Int32
CBigInteger::CompareMagnitude( const CBigInteger& other ) const
{
    return CompareMagnitudes( GetLimbs(), m_size, other.GetLimbs(), other.m_size );
}

/*-------------------------------------------------------------------------*/

Int32
CBigInteger::CompareUInt64( UInt64 other ) const
{
    if ( m_negative )
        return -1;
    if ( m_size > 2 )
        return 1;
    UInt64 magnitude = 0;
    const TLimb* limbs = GetLimbs();
    if ( m_size > 0 ) magnitude = limbs[ 0 ];
    if ( m_size > 1 ) magnitude |= (UInt64) limbs[ 1 ] << 32;
    return magnitude < other ? -1 : ( magnitude > other ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

Int32
CBigInteger::CompareInt64( Int64 other ) const
{
    if ( other >= 0 )
        return CompareUInt64( (UInt64) other );
    if ( !m_negative )
        return 1;
    if ( m_size > 2 )
        return -1;
    UInt64 magnitude = 0;
    const TLimb* limbs = GetLimbs();
    if ( m_size > 0 ) magnitude = limbs[ 0 ];
    if ( m_size > 1 ) magnitude |= (UInt64) limbs[ 1 ] << 32;
    UInt64 otherMagnitude = 0 - (UInt64) other;
    return magnitude > otherMagnitude ? -1 : ( magnitude < otherMagnitude ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::AddSignedImpl( const TLimb* a, UInt32 an, bool aNegative, const TLimb* b, UInt32 bn, bool bNegative, bool aliased, CBigInteger& result )
{
    try
    {
        CBigInteger temporary;
        if ( aNegative == bNegative )
        {
            if ( an < bn )
            {
                const TLimb* t = a; a = b; b = t;
                UInt32 tn = an; an = bn; bn = tn;
            }
            UInt32 needed = an + ( AddCarriesOut( a, an, b, bn ) ? 1 : 0 );
            if ( needed > MaxLimbCount )
                return BIGINTEGERSTATUS_OVERFLOW;
            TLimb* out = result.PrepareOutput( needed, false, aliased, temporary );
            UInt32 size = AddMagnitudes( a, an, b, bn, out );
            result.CommitOutput( out, size, aNegative, temporary );
            return BIGINTEGERSTATUS_OK;
        }

        Int32 cmp = CompareMagnitudes( a, an, b, bn );
        if ( 0 == cmp )
        {
            result.SetZero();
            return BIGINTEGERSTATUS_OK;
        }
        bool negative = aNegative;
        if ( cmp < 0 )
        {
            const TLimb* t = a; a = b; b = t;
            UInt32 tn = an; an = bn; bn = tn;
            negative = bNegative;
        }
        TLimb* out = result.PrepareOutput( an, false, aliased, temporary );
        UInt32 size = SubtractMagnitudes( a, an, b, bn, out );
        result.CommitOutput( out, size, negative, temporary );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::Add( const CBigInteger& other, CBigInteger& result ) const
{GUCEF_TRACE;

    return AddSignedImpl( GetLimbs(), m_size, m_negative, other.GetLimbs(), other.m_size, other.m_negative, &result == this || &result == &other, result );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::Subtract( const CBigInteger& other, CBigInteger& result ) const
{GUCEF_TRACE;

    return AddSignedImpl( GetLimbs(), m_size, m_negative, other.GetLimbs(), other.m_size, !other.m_negative && 0 != other.m_size, &result == this || &result == &other, result );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::AddUInt64( UInt64 other, bool subtract, CBigInteger& result ) const
{GUCEF_TRACE;

    TLimb limbs[ 2 ];
    limbs[ 0 ] = (TLimb) other;
    limbs[ 1 ] = (TLimb) ( other >> 32 );
    UInt32 size = 0 != limbs[ 1 ] ? 2 : ( 0 != limbs[ 0 ] ? 1 : 0 );
    return AddSignedImpl( GetLimbs(), m_size, m_negative, limbs, size, subtract && 0 != size, &result == this, result );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::AddInt64( Int64 other, CBigInteger& result ) const
{GUCEF_TRACE;

    if ( other < 0 )
        return AddUInt64( 0 - (UInt64) other, true, result );
    return AddUInt64( (UInt64) other, false, result );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::MultiplyImpl( const TLimb* a, UInt32 an, const TLimb* b, UInt32 bn, bool negative, bool aliased, CBigInteger& result )
{
    if ( 0 == an || 0 == bn )
    {
        result.SetZero();
        return BIGINTEGERSTATUS_OK;
    }
    UInt64 bits = (UInt64) BitLengthOf( a, an ) + BitLengthOf( b, bn );
    if ( bits > (UInt64) MaxLimbCount * 32 )
        return BIGINTEGERSTATUS_OVERFLOW;

    try
    {
        // the product has bits or bits - 1 bits: storage for the upper bound only, so a product that fits the inline
        // limbs stays there even when the operands' limb counts add up to one more
        UInt32 needed = (UInt32) ( ( bits + 31 ) / 32 );
        CBigInteger temporary;
        if ( 1 == bn || 1 == an )
        {
            // single limb factor: the in place product reads each limb before writing it
            const TLimb* x = 1 == bn ? a : b;
            UInt32 xn = 1 == bn ? an : bn;
            TLimb m = 1 == bn ? b[ 0 ] : a[ 0 ];
            TLimb* out = result.PrepareOutput( needed, false, false, temporary );
            TLimb carry = MultiplyLimb( out, x, xn, m );
            if ( needed > xn )
                out[ xn ] = carry;
            result.CommitOutput( out, needed, negative, temporary );
            return BIGINTEGERSTATUS_OK;
        }

        bool square = a == b && an == bn;
        UInt64 scratchLimbs = square ? KaratsubaScratchLimbs( an ) : MultiplyScratchLimbs( an, bn );
        bool viaBuffer = needed < an + bn;      // the kernels write an + bn limbs
        CBigIntegerScratch scratch( scratchLimbs + ( viaBuffer ? an + bn : 0 ) );
        TLimb* out = result.PrepareOutput( needed, aliased, false, temporary );
        TLimb* product = viaBuffer ? scratch.Get() + scratchLimbs : out;
        if ( square )
            SquareRecursive( product, a, an, scratch.Get() );
        else
            MultiplyRecursive( product, a, an, b, bn, scratch.Get() );
        if ( viaBuffer )
            memcpy( out, product, needed * sizeof( TLimb ) );
        result.CommitOutput( out, needed, negative, temporary );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::Multiply( const CBigInteger& other, CBigInteger& result ) const
{GUCEF_TRACE;

    return MultiplyImpl( GetLimbs(), m_size, other.GetLimbs(), other.m_size, m_negative != other.m_negative, &result == this || &result == &other, result );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::Square( CBigInteger& result ) const
{GUCEF_TRACE;

    return MultiplyImpl( GetLimbs(), m_size, GetLimbs(), m_size, false, &result == this, result );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::MultiplyUInt64( UInt64 other, CBigInteger& result ) const
{GUCEF_TRACE;

    TLimb limbs[ 2 ];
    limbs[ 0 ] = (TLimb) other;
    limbs[ 1 ] = (TLimb) ( other >> 32 );
    UInt32 size = 0 != limbs[ 1 ] ? 2 : ( 0 != limbs[ 0 ] ? 1 : 0 );
    return MultiplyImpl( GetLimbs(), m_size, limbs, size, m_negative, &result == this, result );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::MultiplyInt64( Int64 other, CBigInteger& result ) const
{GUCEF_TRACE;

    UInt64 magnitude = other < 0 ? 0 - (UInt64) other : (UInt64) other;
    TLimb limbs[ 2 ];
    limbs[ 0 ] = (TLimb) magnitude;
    limbs[ 1 ] = (TLimb) ( magnitude >> 32 );
    UInt32 size = 0 != limbs[ 1 ] ? 2 : ( 0 != limbs[ 0 ] ? 1 : 0 );
    return MultiplyImpl( GetLimbs(), m_size, limbs, size, m_negative != ( other < 0 ), &result == this, result );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::Negate( CBigInteger& result ) const
{GUCEF_TRACE;

    if ( &result != this )
    {
        try
        {
            result = *this;
        }
        catch ( const std::bad_alloc& )
        {
            return BIGINTEGERSTATUS_OUT_OF_MEMORY;
        }
    }
    result.m_negative = !m_negative && 0 != m_size;
    return BIGINTEGERSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::Abs( CBigInteger& result ) const
{GUCEF_TRACE;

    if ( &result != this )
    {
        try
        {
            result = *this;
        }
        catch ( const std::bad_alloc& )
        {
            return BIGINTEGERSTATUS_OUT_OF_MEMORY;
        }
    }
    result.m_negative = false;
    return BIGINTEGERSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::DivModTruncating( const CBigInteger& divisor, CBigInteger* quotient, CBigInteger* remainder ) const
{
    if ( divisor.IsZero() )
        return BIGINTEGERSTATUS_DIVISION_BY_ZERO;
    if ( GUCEF_NULL != quotient && quotient == remainder )
        return BIGINTEGERSTATUS_INVALID_OPERAND;

    try
    {
        bool quotientNegative = m_negative != divisor.m_negative;
        bool remainderNegative = m_negative;
        const TLimb* u = GetLimbs();
        const TLimb* v = divisor.GetLimbs();
        UInt32 un = m_size;
        UInt32 vn = divisor.m_size;

        CBigInteger q;
        CBigInteger r;
        if ( CompareMagnitudes( u, un, v, vn ) < 0 )
        {
            // |n| < |d|: q = 0, r = n
            if ( GUCEF_NULL != remainder )
                r.AssignLimbs( u, un, remainderNegative );
        }
        else
        if ( 1 == vn )
        {
            if ( GUCEF_NULL != quotient )
            {
                q.ReserveDiscard( un );
                TLimb rest = DivideByLimb( q.MutableLimbs(), u, un, v[ 0 ] );
                q.m_size = un;
                q.m_negative = quotientNegative;
                q.Trim();
                r.SetMagnitude64( rest, remainderNegative );
            }
            else
            {
                r.SetMagnitude64( RemainderByLimb( u, un, v[ 0 ] ), remainderNegative );
            }
        }
        else
        {
            CBigIntegerScratch work( (UInt64) un + vn + 1 );
            TLimb* qLimbs = GUCEF_NULL;
            if ( GUCEF_NULL != quotient )
            {
                q.ReserveDiscard( un - vn + 1 );
                qLimbs = q.MutableLimbs();
            }
            r.ReserveDiscard( vn );
            DivideKnuth( u, un, v, vn, qLimbs, r.MutableLimbs(), work.Get() );
            if ( GUCEF_NULL != quotient )
            {
                q.m_size = un - vn + 1;
                q.m_negative = quotientNegative;
                q.Trim();
            }
            r.m_size = vn;
            r.m_negative = remainderNegative;
            r.Trim();
        }

        if ( GUCEF_NULL != quotient )
            quotient->Swap( q );
        if ( GUCEF_NULL != remainder )
            remainder->Swap( r );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::DivMod( const CBigInteger& divisor, CBigInteger& quotient, CBigInteger& remainder ) const
{GUCEF_TRACE;

    return DivModTruncating( divisor, &quotient, &remainder );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::DivMod( Int64 divisor, CBigInteger& quotient, Int64& remainder ) const
{GUCEF_TRACE;

    if ( 0 == divisor )
        return BIGINTEGERSTATUS_DIVISION_BY_ZERO;

    UInt64 magnitude = divisor < 0 ? 0 - (UInt64) divisor : (UInt64) divisor;
    if ( magnitude <= 0xFFFFFFFFULL )
    {
        try
        {
            CBigInteger temporary;
            TLimb* out = quotient.PrepareOutput( m_size, false, false, temporary );
            TLimb rest = DivideByLimb( out, GetLimbs(), m_size, (TLimb) magnitude );
            bool dividendNegative = m_negative;
            quotient.CommitOutput( out, m_size, dividendNegative != ( divisor < 0 ), temporary );
            remainder = dividendNegative ? -(Int64) rest : (Int64) rest;
            return BIGINTEGERSTATUS_OK;
        }
        catch ( const std::bad_alloc& )
        {
            return BIGINTEGERSTATUS_OUT_OF_MEMORY;
        }
    }

    CBigInteger d( divisor );
    CBigInteger q;
    CBigInteger r;
    TBigIntegerStatus status = DivModTruncating( d, &q, &r );
    if ( BigIntegerStatusIsError( status ) )
        return status;
    Int64 rest = 0;
    r.ToInt64( rest );      // |r| < |divisor| <= 2^63
    quotient.Swap( q );
    remainder = rest;
    return status;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::DivModFloor( const CBigInteger& divisor, CBigInteger& quotient, CBigInteger& remainder ) const
{GUCEF_TRACE;

    if ( &quotient == &remainder )
        return BIGINTEGERSTATUS_INVALID_OPERAND;
    try
    {
        CBigInteger d( divisor );
        CBigInteger q;
        CBigInteger r;
        TBigIntegerStatus status = DivModTruncating( d, &q, &r );
        if ( BigIntegerStatusIsError( status ) )
            return status;
        if ( !r.IsZero() && r.m_negative != d.m_negative )
        {
            ThrowOnError( q.Subtract( 1, q ) );
            ThrowOnError( r.Add( d, r ) );
        }
        quotient.Swap( q );
        remainder.Swap( r );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
    catch ( const std::length_error& )
    {
        return BIGINTEGERSTATUS_OVERFLOW;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::Modulo( const CBigInteger& modulus, CBigInteger& result ) const
{GUCEF_TRACE;

    try
    {
        CBigInteger r;
        TBigIntegerStatus status = DivModTruncating( modulus, GUCEF_NULL, &r );
        if ( BigIntegerStatusIsError( status ) )
            return status;
        if ( r.m_negative )
        {
            CBigInteger m;
            ThrowOnError( modulus.Abs( m ) );
            ThrowOnError( r.Add( m, r ) );
        }
        result.Swap( r );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
    catch ( const std::length_error& )
    {
        return BIGINTEGERSTATUS_OVERFLOW;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::DivideRounded( const CBigInteger& divisor, TRoundingMode mode, CBigInteger& quotient, CBigInteger& remainder ) const
{GUCEF_TRACE;

    if ( !IsValidBigRoundingMode( mode ) )
        return BIGINTEGERSTATUS_INVALID_OPERAND;
    if ( &quotient == &remainder )
        return BIGINTEGERSTATUS_INVALID_OPERAND;
    try
    {
        CBigInteger d( divisor );
        CBigInteger q;
        CBigInteger r;
        TBigIntegerStatus status = DivModTruncating( d, &q, &r );
        if ( BigIntegerStatusIsError( status ) )
            return status;

        EBigRoundRelation relation = ClassifyBigRemainder( r, d );
        if ( BIGROUNDRELATION_EXACT != relation )
        {
            if ( ROUNDINGMODE_UNNECESSARY == mode )
                return BIGINTEGERSTATUS_ROUNDING_REQUIRED;
            status = BIGINTEGERSTATUS_INEXACT;
            bool negative = m_negative != d.m_negative;
            if ( BigRoundingIncrements( mode, negative, q.IsOdd(), relation ) )
            {
                if ( negative )
                {
                    // n = ( q - 1 ) d + ( r + d )
                    ThrowOnError( q.Subtract( 1, q ) );
                    ThrowOnError( r.Add( d, r ) );
                }
                else
                {
                    ThrowOnError( q.Add( 1, q ) );
                    ThrowOnError( r.Subtract( d, r ) );
                }
            }
        }
        quotient.Swap( q );
        remainder.Swap( r );
        return status;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
    catch ( const std::length_error& )
    {
        return BIGINTEGERSTATUS_OVERFLOW;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::Gcd( const CBigInteger& other, CBigInteger& result ) const
{GUCEF_TRACE;

    try
    {
        CBigInteger a;
        CBigInteger b;
        ThrowOnError( Abs( a ) );
        ThrowOnError( other.Abs( b ) );
        if ( a.IsZero() )
        {
            result.Swap( b );
            return BIGINTEGERSTATUS_OK;
        }
        if ( b.IsZero() )
        {
            result.Swap( a );
            return BIGINTEGERSTATUS_OK;
        }

        // gcd( 2^i a', 2^j b' ) = 2^min( i, j ) gcd( a', b' ) for odd a', b'
        UInt32 aZeros = a.GetLowestSetBit();
        UInt32 bZeros = b.GetLowestSetBit();
        UInt32 commonZeros = aZeros < bZeros ? aZeros : bZeros;
        ThrowOnError( a.ShiftRightMagnitude( aZeros, a ) );
        ThrowOnError( b.ShiftRightMagnitude( bZeros, b ) );

        // Euclid on big values until both fit 64 bits
        CBigInteger r;
        while ( !b.IsZero() && !( a.FitsUInt64() && b.FitsUInt64() ) )
        {
            ThrowOnError( a.DivModTruncating( b, GUCEF_NULL, &r ) );
            a.Swap( b );
            b.Swap( r );
        }
        if ( !b.IsZero() )
        {
            UInt64 x = 0;
            UInt64 y = 0;
            a.ToUInt64( x );
            b.ToUInt64( y );
            while ( 0 != y )
            {
                UInt64 t = x % y;
                x = y;
                y = t;
            }
            a.SetMagnitude64( x, false );
        }
        ThrowOnError( a.ShiftLeft( commonZeros, a ) );
        result.Swap( a );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
    catch ( const std::length_error& )
    {
        return BIGINTEGERSTATUS_OVERFLOW;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::Lcm( const CBigInteger& other, CBigInteger& result ) const
{GUCEF_TRACE;

    if ( IsZero() || other.IsZero() )
    {
        result.SetZero();
        return BIGINTEGERSTATUS_OK;
    }
    try
    {
        CBigInteger g;
        ThrowOnError( Gcd( other, g ) );
        CBigInteger a;
        ThrowOnError( Abs( a ) );
        CBigInteger q;
        CBigInteger r;
        ThrowOnError( a.DivModTruncating( g, &q, &r ) );
        CBigInteger b;
        ThrowOnError( other.Abs( b ) );
        TBigIntegerStatus status = q.Multiply( b, q );
        if ( BigIntegerStatusIsError( status ) )
            return status;
        result.Swap( q );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
    catch ( const std::length_error& )
    {
        return BIGINTEGERSTATUS_OVERFLOW;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ExtendedGcd( const CBigInteger& other, CBigInteger& gcd, CBigInteger& x, CBigInteger& y ) const
{GUCEF_TRACE;

    if ( &gcd == &x || &gcd == &y || &x == &y )
        return BIGINTEGERSTATUS_INVALID_OPERAND;
    try
    {
        bool aNegative = m_negative;
        bool bNegative = other.m_negative;
        CBigInteger oldR;
        CBigInteger r;
        ThrowOnError( Abs( oldR ) );
        ThrowOnError( other.Abs( r ) );
        CBigInteger oldS( 1 );
        CBigInteger s;
        CBigInteger oldT;
        CBigInteger t( 1 );
        CBigInteger q;
        CBigInteger rest;
        CBigInteger product;
        while ( !r.IsZero() )
        {
            ThrowOnError( oldR.DivModTruncating( r, &q, &rest ) );
            oldR.Swap( r );
            r.Swap( rest );

            ThrowOnError( q.Multiply( s, product ) );
            ThrowOnError( oldS.Subtract( product, oldS ) );
            oldS.Swap( s );

            ThrowOnError( q.Multiply( t, product ) );
            ThrowOnError( oldT.Subtract( product, oldT ) );
            oldT.Swap( t );
        }
        if ( aNegative )
            ThrowOnError( oldS.Negate( oldS ) );
        if ( bNegative )
            ThrowOnError( oldT.Negate( oldT ) );
        gcd.Swap( oldR );
        x.Swap( oldS );
        y.Swap( oldT );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
    catch ( const std::length_error& )
    {
        return BIGINTEGERSTATUS_OVERFLOW;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ModInverse( const CBigInteger& modulus, CBigInteger& result ) const
{GUCEF_TRACE;

    if ( modulus.IsZero() )
        return BIGINTEGERSTATUS_DIVISION_BY_ZERO;
    try
    {
        CBigInteger m;
        ThrowOnError( modulus.Abs( m ) );
        CBigInteger a;
        ThrowOnError( Modulo( m, a ) );
        CBigInteger g;
        CBigInteger x;
        CBigInteger y;
        ThrowOnError( a.ExtendedGcd( m, g, x, y ) );
        if ( !g.IsOne() )
            return BIGINTEGERSTATUS_INVALID_OPERAND;
        ThrowOnError( x.Modulo( m, x ) );
        result.Swap( x );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
    catch ( const std::length_error& )
    {
        return BIGINTEGERSTATUS_OVERFLOW;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::Pow( UInt32 exponent, CBigInteger& result ) const
{GUCEF_TRACE;

    if ( 0 == exponent )
    {
        result.SetMagnitude64( 1, false );
        return BIGINTEGERSTATUS_OK;
    }
    bool negative = m_negative && 0 != ( exponent & 1 );
    if ( IsZero() )
    {
        result.SetZero();
        return BIGINTEGERSTATUS_OK;
    }
    if ( 1 == m_size && 1 == GetLimbs()[ 0 ] )
    {
        result.SetMagnitude64( 1, negative );
        return BIGINTEGERSTATUS_OK;
    }

    // ( bits - 1 ) * e < bitlength of the power <= bits * e
    UInt64 bits = GetBitLength();
    if ( ( bits - 1 ) * (UInt64) exponent >= (UInt64) MaxLimbCount * 32 )
        return BIGINTEGERSTATUS_OVERFLOW;

    try
    {
        CBigInteger base;
        ThrowOnError( Abs( base ) );
        CBigInteger power( 1 );

        // a power of two base is a shift
        if ( base.GetLowestSetBit() == bits - 1 )
        {
            ThrowOnError( power.ShiftLeft( (UInt32) ( ( bits - 1 ) * exponent ), power ) );
        }
        else
        {
            // left to right binary
            UInt32 topBit = 31 - CountLeadingZeros32( exponent );
            ThrowOnError( base.Abs( power ) );
            for ( UInt32 i=topBit; i>0; --i )
            {
                ThrowOnError( power.Square( power ) );
                if ( 0 != ( exponent & ( 1UL << ( i-1 ) ) ) )
                    ThrowOnError( power.Multiply( base, power ) );
            }
        }
        power.m_negative = negative && !power.IsZero();
        result.Swap( power );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
    catch ( const std::length_error& )
    {
        return BIGINTEGERSTATUS_OVERFLOW;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ModPow( const CBigInteger& exponent, const CBigInteger& modulus, CBigInteger& result ) const
{GUCEF_TRACE;

    if ( modulus.IsZero() )
        return BIGINTEGERSTATUS_DIVISION_BY_ZERO;
    if ( modulus.m_negative || exponent.m_negative )
        return BIGINTEGERSTATUS_INVALID_OPERAND;
    try
    {
        if ( modulus.IsOne() )
        {
            result.SetZero();
            return BIGINTEGERSTATUS_OK;
        }

        // the 16 residues base^0 .. base^15 of the 4 bit fixed window
        CBigInteger table[ 16 ];
        table[ 0 ] = CBigInteger( 1 );
        ThrowOnError( Modulo( modulus, table[ 1 ] ) );
        for ( UInt32 i=2; i<16; ++i )
        {
            ThrowOnError( table[ i-1 ].Multiply( table[ 1 ], table[ i ] ) );
            ThrowOnError( table[ i ].DivModTruncating( modulus, GUCEF_NULL, &table[ i ] ) );
        }

        CBigInteger accumulator( 1 );
        UInt32 bits = exponent.GetBitLength();
        UInt32 windows = ( bits + 3 ) / 4;
        const TLimb* e = exponent.GetLimbs();
        UInt32 en = exponent.m_size;
        for ( UInt32 w=windows; w>0; --w )
        {
            if ( w != windows )
            {
                for ( UInt32 k=0; k<4; ++k )
                {
                    ThrowOnError( accumulator.Square( accumulator ) );
                    ThrowOnError( accumulator.DivModTruncating( modulus, GUCEF_NULL, &accumulator ) );
                }
            }
            UInt32 digit = ExtractBits( e, en, (UInt64) ( w-1 ) * 4, 4 );
            if ( 0 != digit )
            {
                ThrowOnError( accumulator.Multiply( table[ digit ], accumulator ) );
                ThrowOnError( accumulator.DivModTruncating( modulus, GUCEF_NULL, &accumulator ) );
            }
        }
        result.Swap( accumulator );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
    catch ( const std::length_error& )
    {
        return BIGINTEGERSTATUS_OVERFLOW;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ISqrt( CBigInteger& result ) const
{GUCEF_TRACE;

    if ( m_negative )
        return BIGINTEGERSTATUS_INVALID_OPERAND;
    if ( IsZero() )
    {
        result.SetZero();
        return BIGINTEGERSTATUS_OK;
    }
    try
    {
        // Newton from 2^ceil( bits / 2 ) >= sqrt( n ): decreasing until the first non-decreasing step
        UInt32 bits = GetBitLength();
        CBigInteger x( 1 );
        ThrowOnError( x.ShiftLeft( ( bits + 1 ) / 2, x ) );
        CBigInteger y;
        CBigInteger q;
        for ( ;; )
        {
            ThrowOnError( DivModTruncating( x, &q, GUCEF_NULL ) );
            ThrowOnError( q.Add( x, y ) );
            ThrowOnError( y.ShiftRightMagnitude( 1, y ) );
            if ( y.Compare( x ) >= 0 )
                break;
            x.Swap( y );
        }
        result.Swap( x );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
    catch ( const std::length_error& )
    {
        return BIGINTEGERSTATUS_OVERFLOW;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ShiftLeft( UInt32 bits, CBigInteger& result ) const
{GUCEF_TRACE;

    if ( IsZero() )
    {
        result.SetZero();
        return BIGINTEGERSTATUS_OK;
    }
    if ( (UInt64) GetBitLength() + bits > (UInt64) MaxLimbCount * 32 )
        return BIGINTEGERSTATUS_OVERFLOW;

    UInt32 limbShift = bits / 32;
    UInt32 bitShift = bits % 32;
    UInt32 n = m_size;
    UInt32 needed = (UInt32) ( ( (UInt64) GetBitLength() + bits + 31 ) / 32 );     // n + limbShift or one more
    bool negative = m_negative;
    try
    {
        CBigInteger temporary;
        const TLimb* a = GetLimbs();
        TLimb* out = result.PrepareOutput( needed, false, false, temporary );

        // from the top down, so an in place shift reads every limb before it is overwritten
        if ( 0 == bitShift )
        {
            for ( UInt32 i=n; i>0; --i )
                out[ i-1 + limbShift ] = a[ i-1 ];
        }
        else
        {
            if ( needed > n + limbShift )
                out[ n + limbShift ] = a[ n-1 ] >> ( 32 - bitShift );
            for ( UInt32 i=n-1; i>0; --i )
                out[ i + limbShift ] = ( a[ i ] << bitShift ) | ( a[ i-1 ] >> ( 32 - bitShift ) );
            out[ limbShift ] = a[ 0 ] << bitShift;
        }
        for ( UInt32 i=0; i<limbShift; ++i )
            out[ i ] = 0;
        result.CommitOutput( out, needed, negative, temporary );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ShiftRightMagnitude( UInt32 bits, CBigInteger& result ) const
{GUCEF_TRACE;

    UInt32 limbShift = bits / 32;
    UInt32 bitShift = bits % 32;
    if ( limbShift >= m_size )
    {
        result.SetZero();
        return BIGINTEGERSTATUS_OK;
    }
    UInt32 n = m_size - limbShift;
    try
    {
        CBigInteger temporary;
        TLimb* out = result.PrepareOutput( n, false, false, temporary );
        const TLimb* a = GetLimbs() + limbShift;

        // from the bottom up, so an in place shift reads every limb before it is overwritten
        if ( 0 == bitShift )
        {
            for ( UInt32 i=0; i<n; ++i )
                out[ i ] = a[ i ];
        }
        else
        {
            for ( UInt32 i=0; i+1<n; ++i )
                out[ i ] = ( a[ i ] >> bitShift ) | ( a[ i+1 ] << ( 32 - bitShift ) );
            out[ n-1 ] = a[ n-1 ] >> bitShift;
        }
        result.CommitOutput( out, n, m_negative, temporary );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ShiftRight( UInt32 bits, CBigInteger& result ) const
{GUCEF_TRACE;

    if ( !m_negative )
        return ShiftRightMagnitude( bits, result );

    // floor( -m / 2^k ) = -( ( m >> k ) + ( any bit shifted out ? 1 : 0 ) )
    bool bitsDropped = GetLowestSetBit() < bits;
    try
    {
        CBigInteger shifted;
        ThrowOnError( ShiftRightMagnitude( bits, shifted ) );
        if ( bitsDropped )
        {
            shifted.m_negative = false;
            ThrowOnError( shifted.Add( 1, shifted ) );
        }
        shifted.m_negative = !shifted.IsZero();
        result.Swap( shifted );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
    catch ( const std::length_error& )
    {
        return BIGINTEGERSTATUS_OVERFLOW;
    }
}

/*-------------------------------------------------------------------------*/

/** Two's complement form of a sign / magnitude value in n limbs (n > magnitude size) */
static void
ToTwosComplementLimbs( const TLimb* magnitude, UInt32 size, bool negative, TLimb* out, UInt32 n )
{
    for ( UInt32 i=0; i<n; ++i )
        out[ i ] = i < size ? magnitude[ i ] : 0;
    if ( negative )
    {
        // ~( m - 1 )
        UInt64 borrow = 1;
        for ( UInt32 i=0; i<n; ++i )
        {
            UInt64 d = (UInt64) out[ i ] - borrow;
            out[ i ] = ~(TLimb) d;
            borrow = d >> 63;
        }
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::BitwiseOperation( const CBigInteger& other, UInt32 operation, CBigInteger& result ) const
{
    // operation: 0 = and, 1 = or, 2 = xor
    UInt32 n = ( m_size > other.m_size ? m_size : other.m_size ) + 1;
    if ( n > MaxLimbCount + 1 )
        return BIGINTEGERSTATUS_OVERFLOW;
    try
    {
        CBigIntegerScratch scratch( 2 * (UInt64) n );
        TLimb* a = scratch.Get();
        TLimb* b = a + n;
        ToTwosComplementLimbs( GetLimbs(), m_size, m_negative, a, n );
        ToTwosComplementLimbs( other.GetLimbs(), other.m_size, other.m_negative, b, n );
        for ( UInt32 i=0; i<n; ++i )
        {
            switch ( operation )
            {
                case 0: a[ i ] &= b[ i ]; break;
                case 1: a[ i ] |= b[ i ]; break;
                default: a[ i ] ^= b[ i ]; break;
            }
        }
        bool negative = 0 != ( a[ n-1 ] >> 31 );
        if ( negative )
        {
            // magnitude = ~pattern + 1
            UInt64 carry = 1;
            for ( UInt32 i=0; i<n; ++i )
            {
                carry += (TLimb) ~a[ i ];
                a[ i ] = (TLimb) carry;
                carry >>= 32;
            }
        }
        UInt32 size = TrimmedSize( a, n );
        CBigInteger temporary;
        TLimb* out = result.PrepareOutput( size, false, false, temporary );
        if ( size > 0 )
            memcpy( out, a, size * sizeof( TLimb ) );
        result.CommitOutput( out, size, negative, temporary );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::And( const CBigInteger& other, CBigInteger& result ) const
{GUCEF_TRACE;

    return BitwiseOperation( other, 0, result );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::Or( const CBigInteger& other, CBigInteger& result ) const
{GUCEF_TRACE;

    return BitwiseOperation( other, 1, result );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::Xor( const CBigInteger& other, CBigInteger& result ) const
{GUCEF_TRACE;

    return BitwiseOperation( other, 2, result );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::Not( CBigInteger& result ) const
{GUCEF_TRACE;

    // ~x = -x - 1 = -( x + 1 )
    CBigInteger sum;
    TBigIntegerStatus status = Add( 1, sum );
    if ( BigIntegerStatusIsError( status ) )
        return status;
    sum.m_negative = !sum.m_negative && !sum.IsZero();
    result.Swap( sum );
    return BIGINTEGERSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

bool
CBigInteger::TestBit( UInt32 index ) const
{
    UInt32 limbIndex = index / 32;
    bool magnitudeBit = limbIndex < m_size && 0 != ( ( GetLimbs()[ limbIndex ] >> ( index % 32 ) ) & 1 );
    if ( !m_negative )
        return magnitudeBit;

    // -m: zeros below the lowest set bit t of m, a one at t, the inverted bits of m above t
    UInt32 lowest = GetLowestSetBit();
    if ( index < lowest )
        return false;
    if ( index == lowest )
        return true;
    return !magnitudeBit;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::SetBit( UInt32 index )
{GUCEF_TRACE;

    if ( TestBit( index ) )
        return BIGINTEGERSTATUS_OK;
    UInt32 limbIndex = index / 32;
    if ( limbIndex >= MaxLimbCount )
        return BIGINTEGERSTATUS_OVERFLOW;
    try
    {
        if ( !m_negative )
        {
            ReserveKeep( limbIndex + 1, false );
            TLimb* limbs = MutableLimbs();
            for ( UInt32 i=m_size; i<=limbIndex; ++i )
                limbs[ i ] = 0;
            if ( m_size <= limbIndex )
                m_size = limbIndex + 1;
            limbs[ limbIndex ] |= (TLimb) 1 << ( index % 32 );
            return BIGINTEGERSTATUS_OK;
        }
        CBigInteger mask( 1 );
        ThrowOnError( mask.ShiftLeft( index, mask ) );
        return Or( mask, *this );
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
    catch ( const std::length_error& )
    {
        return BIGINTEGERSTATUS_OVERFLOW;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ClearBit( UInt32 index )
{GUCEF_TRACE;

    if ( !TestBit( index ) )
        return BIGINTEGERSTATUS_OK;
    if ( !m_negative )
    {
        MutableLimbs()[ index / 32 ] &= ~( (TLimb) 1 << ( index % 32 ) );
        Trim();
        return BIGINTEGERSTATUS_OK;
    }
    try
    {
        CBigInteger mask( 1 );
        ThrowOnError( mask.ShiftLeft( index, mask ) );
        ThrowOnError( mask.Not( mask ) );
        return And( mask, *this );
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
    catch ( const std::length_error& )
    {
        return BIGINTEGERSTATUS_OVERFLOW;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::FlipBit( UInt32 index )
{GUCEF_TRACE;

    return TestBit( index ) ? ClearBit( index ) : SetBit( index );
}

/*-------------------------------------------------------------------------*/

UInt32
CBigInteger::GetTextLengthUpperBound( UInt32 base ) const
{
    if ( base < 2 || base > 36 )
        return 0;
    UInt64 digits = (UInt64) GetBitLength() / FloorLog2( base ) + 1;
    UInt64 total = digits + 2;      // sign and terminating null
    return total > 0xFFFFFFFFULL ? 0xFFFFFFFFUL : (UInt32) total;
}

/*-------------------------------------------------------------------------*/

UInt32
CBigInteger::ToChars( char* buffer, UInt32 bufferSize, UInt32 base, bool upperCase ) const
{GUCEF_TRACE;

    if ( base < 2 || base > 36 || GUCEF_NULL == buffer )
        return 0;
    const char* alphabet = upperCase ? g_bigIntegerUpperDigits : g_bigIntegerLowerDigits;

    if ( IsZero() )
    {
        if ( bufferSize < 2 )
            return 0;
        buffer[ 0 ] = '0';
        buffer[ 1 ] = '\0';
        return 1;
    }

    const TLimb* limbs = GetLimbs();
    UInt32 sign = m_negative ? 1 : 0;
    UInt32 bitsPerDigit = PowerOfTwoBaseBits( base );
    if ( 0 != bitsPerDigit )
    {
        // regrouping of bits, linear
        UInt64 bitLength = GetBitLength();
        UInt64 digits = ( bitLength + bitsPerDigit - 1 ) / bitsPerDigit;
        UInt64 length = sign + digits;
        if ( length + 1 > bufferSize )
            return 0;
        if ( 0 != sign )
            buffer[ 0 ] = '-';
        for ( UInt64 d=0; d<digits; ++d )
            buffer[ length - 1 - d ] = alphabet[ ExtractBits( limbs, m_size, d * bitsPerDigit, bitsPerDigit ) ];
        buffer[ length ] = '\0';
        return (UInt32) length;
    }

    // repeated division by the largest power of the base that fits a limb, chunks are collected low to high first
    UInt32 chunkPower = g_bigIntegerChunkPower[ base ];
    UInt32 chunkDigits = g_bigIntegerChunkDigits[ base ];
    UInt32 maxChunks = 2 * m_size + 2;
    try
    {
        CBigIntegerScratch scratch( (UInt64) m_size + maxChunks );
        TLimb* magnitude = scratch.Get();
        TLimb* chunks = magnitude + m_size;
        memcpy( magnitude, limbs, m_size * sizeof( TLimb ) );
        UInt32 n = m_size;
        UInt32 chunkCount = 0;
        while ( n > 0 )
        {
            chunks[ chunkCount++ ] = DivideByLimb( magnitude, magnitude, n, chunkPower );
            n = TrimmedSize( magnitude, n );
        }

        UInt32 topDigits = 0;
        for ( UInt32 top=chunks[ chunkCount-1 ]; 0 != top; top /= base )
            ++topDigits;
        UInt64 length = sign + (UInt64) ( chunkCount - 1 ) * chunkDigits + topDigits;
        if ( length + 1 > bufferSize )
            return 0;

        char* write = buffer + length;
        *write = '\0';
        for ( UInt32 c=0; c<chunkCount; ++c )
        {
            UInt32 chunk = chunks[ c ];
            UInt32 count = c + 1 < chunkCount ? chunkDigits : topDigits;
            for ( UInt32 d=0; d<count; ++d )
            {
                *--write = alphabet[ chunk % base ];
                chunk /= base;
            }
        }
        if ( 0 != sign )
            buffer[ 0 ] = '-';
        return (UInt32) length;
    }
    catch ( const std::bad_alloc& )
    {
        return 0;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ToString( UInt32 base, bool upperCase, CString& text ) const
{GUCEF_TRACE;

    if ( base < 2 || base > 36 )
        return BIGINTEGERSTATUS_INVALID_OPERAND;
    try
    {
        UInt32 bound = GetTextLengthUpperBound( base );
        char stackBuffer[ 128 ];
        GUCEF::vector< char > heapBuffer;
        char* buffer = stackBuffer;
        if ( bound > (UInt32) sizeof( stackBuffer ) )
        {
            heapBuffer.resize( bound );
            buffer = &heapBuffer[ 0 ];
        }
        UInt32 length = ToChars( buffer, bound, base, upperCase );
        if ( 0 == length )
            return BIGINTEGERSTATUS_OUT_OF_MEMORY;

        #if ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_ASCII_STRING ) || ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_UTF8_STRING )
        text = CString( buffer, length );
        #else
        text = CORE::ToString( CUtf8String( buffer, length ) );
        #endif
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
}

/*-------------------------------------------------------------------------*/

CString
CBigInteger::ToString( UInt32 base ) const
{GUCEF_TRACE;

    CString text;
    ToString( base, false, text );
    return text;
}

/*-------------------------------------------------------------------------*/

CString
CBigInteger::ToString( UInt32 base, bool upperCase ) const
{GUCEF_TRACE;

    CString text;
    ToString( base, upperCase, text );
    return text;
}

/*-------------------------------------------------------------------------*/

/**
 *  Divide and conquer conversion of 'count' digit values (most significant first, each < base) into a magnitude:
 *  value = high * base^lowCount + low. 'powers[ k ]' holds base^( chunkDigits * 2^k ) for the split sizes used.
 *  Below the threshold the digits are accumulated with Horner by chunks.
 */
static void
DigitsToMagnitude( const UInt8* digits, UInt32 count, UInt32 base, CBigInteger& result )
{
    UInt32 chunkDigits = g_bigIntegerChunkDigits[ base ];
    UInt32 chunkPower = g_bigIntegerChunkPower[ base ];

    // Horner by chunks: the size of the result is about count / chunkDigits limbs
    UInt64 estimatedLimbs = (UInt64) count / chunkDigits + 1;
    if ( estimatedLimbs < CBigInteger::BaseConversionDivideConquerLimbs || count <= 2 * chunkDigits )
    {
        CBigInteger value;
        UInt32 first = count % chunkDigits;
        if ( 0 == first )
            first = chunkDigits;
        UInt32 position = 0;
        UInt32 take = first;
        while ( position < count )
        {
            UInt32 chunk = 0;
            UInt32 multiplier = 1;
            for ( UInt32 i=0; i<take; ++i )
            {
                chunk = chunk * base + digits[ position + i ];
                multiplier *= base;
            }
            position += take;
            CBigInteger::ThrowOnError( value.Multiply( (UInt32) ( take == chunkDigits ? chunkPower : multiplier ), value ) );
            CBigInteger::ThrowOnError( value.Add( chunk, value ) );
            take = chunkDigits;
        }
        result.Swap( value );
        return;
    }

    // split so the low part is a whole number of chunks: high * base^lowCount + low
    UInt32 lowChunks = ( count / chunkDigits ) / 2;
    UInt32 lowCount = lowChunks * chunkDigits;
    CBigInteger high;
    CBigInteger low;
    DigitsToMagnitude( digits, count - lowCount, base, high );
    DigitsToMagnitude( digits + count - lowCount, lowCount, base, low );
    CBigInteger scale( chunkPower );
    CBigInteger::ThrowOnError( scale.Pow( lowChunks, scale ) );
    CBigInteger::ThrowOnError( high.Multiply( scale, high ) );
    CBigInteger::ThrowOnError( high.Add( low, high ) );
    result.Swap( high );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::FromChars( const char* text, UInt32 length, UInt32 base, CBigInteger& result, UInt32 parseFlags )
{GUCEF_TRACE;

    bool allowPrefix = 0 != ( parseFlags & BIGINTEGERPARSE_ALLOW_BASE_PREFIX );
    if ( ( base < 2 && !( 0 == base && allowPrefix ) ) || base > 36 )
        return BIGINTEGERSTATUS_INVALID_OPERAND;
    if ( GUCEF_NULL == text && length > 0 )
        return BIGINTEGERSTATUS_INVALID_FORMAT;

    UInt32 begin = 0;
    UInt32 end = length;
    if ( 0 != ( parseFlags & BIGINTEGERPARSE_ALLOW_SURROUNDING_WHITESPACE ) )
    {
        while ( begin < end && ( ' ' == text[ begin ] || '\t' == text[ begin ] ) )
            ++begin;
        while ( end > begin && ( ' ' == text[ end-1 ] || '\t' == text[ end-1 ] ) )
            --end;
    }

    bool negative = false;
    if ( begin < end && ( '+' == text[ begin ] || '-' == text[ begin ] ) )
    {
        negative = '-' == text[ begin ];
        ++begin;
    }

    if ( allowPrefix && begin + 1 < end && '0' == text[ begin ] )
    {
        char p = text[ begin+1 ];
        UInt32 prefixBase = 0;
        if ( 'x' == p || 'X' == p ) prefixBase = 16;
        else if ( 'b' == p || 'B' == p ) prefixBase = 2;
        else if ( 'o' == p || 'O' == p ) prefixBase = 8;
        if ( 0 != prefixBase && ( 0 == base || prefixBase == base ) )
        {
            base = prefixBase;
            begin += 2;
        }
    }
    if ( 0 == base )
        base = 10;

    // validate and collect the digit values
    bool allowSeparators = 0 != ( parseFlags & BIGINTEGERPARSE_ALLOW_DIGIT_SEPARATORS );
    UInt32 digitCount = 0;
    for ( UInt32 i=begin; i<end; ++i )
    {
        char c = text[ i ];
        if ( '_' == c && allowSeparators )
        {
            // a single separator between two digits
            if ( i == begin || i+1 >= end || '_' == text[ i+1 ] )
                return BIGINTEGERSTATUS_INVALID_FORMAT;
            continue;
        }
        Int32 value = DigitValue( c );
        if ( value < 0 || (UInt32) value >= base )
            return BIGINTEGERSTATUS_INVALID_FORMAT;
        ++digitCount;
    }
    if ( 0 == digitCount )
        return BIGINTEGERSTATUS_INVALID_FORMAT;
    if ( (UInt64) digitCount * FloorLog2( base ) > (UInt64) MaxLimbCount * 32 )
        return BIGINTEGERSTATUS_OVERFLOW;

    try
    {
        // skip leading zeros, they do not change the value
        UInt32 bitsPerDigit = PowerOfTwoBaseBits( base );
        CBigInteger value;
        if ( 0 != bitsPerDigit )
        {
            UInt64 bits = (UInt64) digitCount * bitsPerDigit;
            UInt32 limbCount = (UInt32) ( ( bits + 31 ) / 32 );
            value.ReserveDiscard( limbCount );
            TLimb* limbs = value.MutableLimbs();
            memset( limbs, 0, limbCount * sizeof( TLimb ) );
            UInt64 position = 0;
            for ( UInt32 i=end; i>begin; --i )
            {
                char c = text[ i-1 ];
                if ( '_' == c )
                    continue;
                UInt64 digit = (UInt64) DigitValue( c );
                UInt64 limbIndex = position / 32;
                UInt32 bitIndex = (UInt32) ( position % 32 );
                UInt64 shifted = digit << bitIndex;
                limbs[ limbIndex ] |= (TLimb) shifted;
                if ( 0 != ( shifted >> 32 ) )
                    limbs[ limbIndex + 1 ] |= (TLimb) ( shifted >> 32 );
                position += bitsPerDigit;
            }
            value.m_size = limbCount;
            value.Trim();
        }
        else
        {
            UInt8 stackDigits[ 256 ];
            GUCEF::vector< UInt8 > heapDigits;
            UInt8* digits = stackDigits;
            if ( digitCount > (UInt32) sizeof( stackDigits ) )
            {
                heapDigits.resize( digitCount );
                digits = &heapDigits[ 0 ];
            }
            UInt32 d = 0;
            for ( UInt32 i=begin; i<end; ++i )
            {
                if ( '_' != text[ i ] )
                    digits[ d++ ] = (UInt8) DigitValue( text[ i ] );
            }
            UInt32 first = 0;
            while ( first + 1 < digitCount && 0 == digits[ first ] )
                ++first;
            DigitsToMagnitude( &digits[ first ], digitCount - first, base, value );
        }
        value.m_negative = negative && !value.IsZero();
        result.Swap( value );
        return BIGINTEGERSTATUS_OK;
    }
    catch ( const std::bad_alloc& )
    {
        return BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
    catch ( const std::length_error& )
    {
        return BIGINTEGERSTATUS_OVERFLOW;
    }
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::FromString( const CString& text, UInt32 base, CBigInteger& result, UInt32 parseFlags )
{GUCEF_TRACE;

    #if ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_ASCII_STRING ) || ( GUCEF_DEFAULT_STRING_FORMAT == GUCEF_DATATYPE_UTF8_STRING )
    const CString& narrow = text;
    #else
    CUtf8String narrow = ToUtf8String( text );
    #endif

    UInt32 byteSize = narrow.ByteSize();
    const char* chars = narrow.C_String();
    if ( GUCEF_NULL == chars || 0 == byteSize )
        return FromChars( "", 0, base, result, parseFlags );
    if ( '\0' == chars[ byteSize-1 ] )
        --byteSize;
    return FromChars( chars, byteSize, base, result, parseFlags );
}

/*-------------------------------------------------------------------------*/

CBigInteger
CBigInteger::Parse( const char* literal )
{GUCEF_TRACE;

    CBigInteger result;
    UInt32 length = GUCEF_NULL != literal ? (UInt32) strlen( literal ) : 0;
    TBigIntegerStatus status = FromChars( literal, length, 10, result, BIGINTEGERPARSE_STRICT );
    if ( BIGINTEGERSTATUS_OK != status )
        throw GUCEF::invalid_argument( __FILE__, __LINE__, "CBigInteger::Parse: the literal is not a valid integer in the strict grammar" );
    return result;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ToFloatFormat( TRoundingMode mode, UInt32 precisionBits, Int32 minExponent, Int32 maxExponent, Float64& value ) const
{
    if ( !IsValidBigRoundingMode( mode ) )
        return BIGINTEGERSTATUS_INVALID_OPERAND;
    if ( IsZero() )
    {
        value = 0.0;
        return BIGINTEGERSTATUS_OK;
    }

    // the top 64 bits and a sticky bit for the rest
    const TLimb* limbs = GetLimbs();
    UInt32 bitLength = GetBitLength();
    UInt64 q = 0;
    Int64 exponent2 = 0;
    bool sticky = false;
    if ( bitLength <= 64 )
    {
        q = limbs[ 0 ];
        if ( m_size > 1 )
            q |= (UInt64) limbs[ 1 ] << 32;
    }
    else
    {
        UInt64 low = (UInt64) bitLength - 64;
        q = ( (UInt64) ExtractBits( limbs, m_size, low + 32, 32 ) << 32 ) | ExtractBits( limbs, m_size, low, 32 );
        exponent2 = (Int64) low;
        UInt64 lowLimbs = low / 32;
        for ( UInt64 i=0; i<lowLimbs && !sticky; ++i )
            sticky = 0 != limbs[ i ];
        if ( !sticky && 0 != ( low % 32 ) )
            sticky = 0 != ( limbs[ lowLimbs ] & ( ( 1UL << ( low % 32 ) ) - 1 ) );
    }
    return RoundToBinaryFormat( m_negative, q, exponent2, sticky, mode, precisionBits, minExponent, maxExponent, value );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ToFloat64( TRoundingMode mode, Float64& value ) const
{GUCEF_TRACE;

    Float64 out = 0.0;
    TBigIntegerStatus status = ToFloatFormat( mode, 53, -1022, 1023, out );
    if ( !BigIntegerStatusIsError( status ) )
        value = out;
    return status;
}

/*-------------------------------------------------------------------------*/

Float64
CBigInteger::ToFloat64( void ) const
{GUCEF_TRACE;

    Float64 out = 0.0;
    if ( BIGINTEGERSTATUS_OVERFLOW == ToFloatFormat( ROUNDINGMODE_HALF_EVEN, 53, -1022, 1023, out ) )
        return m_negative ? -HUGE_VAL : HUGE_VAL;
    return out;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ToFloat32( TRoundingMode mode, Float32& value ) const
{GUCEF_TRACE;

    Float64 out = 0.0;
    TBigIntegerStatus status = ToFloatFormat( mode, 24, -126, 127, out );
    if ( !BigIntegerStatusIsError( status ) )
        value = (Float32) out;     // exact: the value has 24 significant bits and is in range
    return status;
}

/*-------------------------------------------------------------------------*/

Float32
CBigInteger::ToFloat32( void ) const
{GUCEF_TRACE;

    Float64 out = 0.0;
    if ( BIGINTEGERSTATUS_OVERFLOW == ToFloatFormat( ROUNDINGMODE_HALF_EVEN, 24, -126, 127, out ) )
        return (Float32) ( m_negative ? -HUGE_VAL : HUGE_VAL );
    return (Float32) out;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::RatioToFloat64( const CBigInteger& numerator, const CBigInteger& denominator, TRoundingMode mode, Float64& value )
{GUCEF_TRACE;

    Float64 out = 0.0;
    TBigIntegerStatus status = RatioToBinaryFormat( numerator, denominator, mode, 53, -1022, 1023, out );
    if ( !BigIntegerStatusIsError( status ) )
        value = out;
    return status;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::RatioToFloat32( const CBigInteger& numerator, const CBigInteger& denominator, TRoundingMode mode, Float32& value )
{GUCEF_TRACE;

    Float64 out = 0.0;
    TBigIntegerStatus status = RatioToBinaryFormat( numerator, denominator, mode, 24, -126, 127, out );
    if ( !BigIntegerStatusIsError( status ) )
        value = (Float32) out;
    return status;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::FromFloat64( Float64 value, TRoundingMode mode, CBigInteger& result )
{GUCEF_TRACE;

    if ( !IsFiniteBigFloat64( value ) || !IsValidBigRoundingMode( mode ) )
        return BIGINTEGERSTATUS_INVALID_OPERAND;
    if ( 0.0 == value )
    {
        result.SetZero();
        return BIGINTEGERSTATUS_OK;
    }

    bool negative = value < 0.0;
    int exponent = 0;
    Float64 fraction = frexp( negative ? -value : value, &exponent );     // value = fraction * 2^exponent, fraction in [ 0.5, 1 )
    UInt64 mantissa = (UInt64) ldexp( fraction, 53 );                       // exact, 53 significant bits
    Int32 exponent2 = exponent - 53;

    if ( exponent2 >= 0 )
    {
        CBigInteger magnitude( mantissa );
        TBigIntegerStatus status = magnitude.ShiftLeft( (UInt32) exponent2, magnitude );
        if ( BigIntegerStatusIsError( status ) )
            return status;
        magnitude.m_negative = negative;
        result.Swap( magnitude );
        return BIGINTEGERSTATUS_OK;
    }

    UInt32 shift = (UInt32) -exponent2;
    UInt64 kept = 0;
    EBigRoundRelation relation = BIGROUNDRELATION_BELOW_HALF;
    if ( shift <= 53 )
    {
        kept = mantissa >> shift;
        UInt64 discarded = mantissa & ( ( 1ULL << shift ) - 1 );
        UInt64 half = 1ULL << ( shift - 1 );
        if ( 0 == discarded )
            relation = BIGROUNDRELATION_EXACT;
        else
            relation = discarded < half ? BIGROUNDRELATION_BELOW_HALF : ( discarded > half ? BIGROUNDRELATION_ABOVE_HALF : BIGROUNDRELATION_HALF );
    }

    TBigIntegerStatus status = BIGINTEGERSTATUS_OK;
    if ( BIGROUNDRELATION_EXACT != relation )
    {
        if ( ROUNDINGMODE_UNNECESSARY == mode )
            return BIGINTEGERSTATUS_ROUNDING_REQUIRED;
        status = BIGINTEGERSTATUS_INEXACT;
        if ( BigRoundingIncrements( mode, negative, 0 != ( kept & 1 ), relation ) )
            ++kept;
    }
    result.SetMagnitude64( kept, negative );
    return status;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::FromFloat32( Float32 value, TRoundingMode mode, CBigInteger& result )
{GUCEF_TRACE;

    // widening to double is exact
    return FromFloat64( (Float64) value, mode, result );
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::FromDecimal( const CDecimal& value, TRoundingMode mode, CBigInteger& result )
{GUCEF_TRACE;

    if ( !IsValidBigRoundingMode( mode ) )
        return BIGINTEGERSTATUS_INVALID_OPERAND;
    Int64 unscaled = value.GetUnscaledValue();
    UInt8 scale = value.GetScale();
    if ( scale > CDecimal::MaxScale )
        return BIGINTEGERSTATUS_INVALID_SCALE;

    bool negative = unscaled < 0;
    UInt64 magnitude = negative ? 0 - (UInt64) unscaled : (UInt64) unscaled;
    UInt64 divisor = g_bigIntegerPow10[ scale ];
    UInt64 kept = magnitude / divisor;
    UInt64 discarded = magnitude % divisor;

    TBigIntegerStatus status = BIGINTEGERSTATUS_OK;
    if ( 0 != discarded )
    {
        if ( ROUNDINGMODE_UNNECESSARY == mode )
            return BIGINTEGERSTATUS_ROUNDING_REQUIRED;
        status = BIGINTEGERSTATUS_INEXACT;
        UInt64 rest = divisor - discarded;
        EBigRoundRelation relation = discarded < rest ? BIGROUNDRELATION_BELOW_HALF : ( discarded > rest ? BIGROUNDRELATION_ABOVE_HALF : BIGROUNDRELATION_HALF );
        if ( BigRoundingIncrements( mode, negative, 0 != ( kept & 1 ), relation ) )
            ++kept;
    }
    result.SetMagnitude64( kept, negative );
    return status;
}

/*-------------------------------------------------------------------------*/

TBigIntegerStatus
CBigInteger::ToDecimal( UInt8 scale, CDecimal& value ) const
{GUCEF_TRACE;

    if ( scale > CDecimal::MaxScale )
        return BIGINTEGERSTATUS_INVALID_SCALE;
    if ( m_size > 2 )
        return BIGINTEGERSTATUS_OVERFLOW;
    UInt64 magnitude = 0;
    const TLimb* limbs = GetLimbs();
    if ( m_size > 0 ) magnitude = limbs[ 0 ];
    if ( m_size > 1 ) magnitude |= (UInt64) limbs[ 1 ] << 32;
    CUInt128 scaled( magnitude );
    if ( !CUInt128::MultiplyByPow10Checked( scaled, scale ) || !scaled.FitsUInt64() || scaled.low > g_bigIntegerInt64MaxMagnitude )
        return BIGINTEGERSTATUS_OVERFLOW;
    Int64 unscaled = m_negative ? -(Int64) scaled.low : (Int64) scaled.low;
    return (TBigIntegerStatus) CDecimal::FromUnscaled( unscaled, scale, value );
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
CBigInteger::RatioToDecimal( const CBigInteger& numerator, const CBigInteger& denominator, const CDecimalContext& context, CDecimal& value )
{GUCEF_TRACE;

    if ( denominator.IsZero() )
        return DECIMALSTATUS_DIVISION_BY_ZERO;
    UInt8 scale = context.GetResultScale();
    TRoundingMode mode = context.GetRoundingMode();
    if ( scale > CDecimal::MaxScale )
        return DECIMALSTATUS_INVALID_SCALE;
    if ( !IsValidBigRoundingMode( mode ) )
        return DECIMALSTATUS_INVALID_OPERAND;

    try
    {
        bool negative = numerator.IsNegative() != denominator.IsNegative();
        CBigInteger n;
        CBigInteger d;
        CBigInteger::ThrowOnError( numerator.Abs( n ) );
        CBigInteger::ThrowOnError( denominator.Abs( d ) );
        CBigInteger::ThrowOnError( n.Multiply( g_bigIntegerPow10[ scale ], n ) );
        CBigInteger q;
        CBigInteger r;
        CBigInteger::ThrowOnError( n.DivMod( d, q, r ) );

        // a truncated quotient that does not fit overflows in every mode
        if ( q.CompareUInt64( g_bigIntegerInt64MaxMagnitude ) > 0 )
            return DECIMALSTATUS_OVERFLOW;
        UInt64 kept = 0;
        q.ToUInt64( kept );

        TDecimalStatus status = DECIMALSTATUS_OK;
        EBigRoundRelation relation = ClassifyBigRemainder( r, d );
        if ( BIGROUNDRELATION_EXACT != relation )
        {
            if ( ROUNDINGMODE_UNNECESSARY == mode )
                return DECIMALSTATUS_ROUNDING_REQUIRED;
            status = DECIMALSTATUS_INEXACT;
            if ( BigRoundingIncrements( mode, negative, 0 != ( kept & 1 ), relation ) )
                ++kept;
            if ( kept > g_bigIntegerInt64MaxMagnitude )
                return DECIMALSTATUS_OVERFLOW;
        }
        Int64 unscaled = negative ? -(Int64) kept : (Int64) kept;
        return status | CDecimal::FromUnscaled( unscaled, scale, value );
    }
    catch ( const std::bad_alloc& )
    {
        return (TDecimalStatus) BIGINTEGERSTATUS_OUT_OF_MEMORY;
    }
    catch ( const std::length_error& )
    {
        return DECIMALSTATUS_OVERFLOW;
    }
}

/*-------------------------------------------------------------------------*/

void
CBigInteger::ThrowOnError( TBigIntegerStatus status )
{
    if ( !BigIntegerStatusIsError( status ) )
        return;
    if ( 0 != ( status & BIGINTEGERSTATUS_OUT_OF_MEMORY ) )
        throw std::bad_alloc();
    if ( 0 != ( status & BIGINTEGERSTATUS_OVERFLOW ) )
        throw std::length_error( "CBigInteger: the result exceeds the representable size" );
    if ( 0 != ( status & BIGINTEGERSTATUS_DIVISION_BY_ZERO ) )
        throw GUCEF::invalid_argument( __FILE__, __LINE__, "CBigInteger: division by zero" );
    throw GUCEF::invalid_argument( __FILE__, __LINE__, "CBigInteger: invalid operand" );
}

/*-------------------------------------------------------------------------*/

CBigInteger
CBigInteger::operator-( void ) const
{GUCEF_TRACE;

    CBigInteger result( *this );
    result.m_negative = !m_negative && 0 != m_size;
    return result;
}

/*-------------------------------------------------------------------------*/

CBigInteger
CBigInteger::operator~( void ) const
{GUCEF_TRACE;

    CBigInteger result;
    ThrowOnError( Not( result ) );
    return result;
}

/*-------------------------------------------------------------------------*/

CBigInteger& CBigInteger::operator+=( const CBigInteger& other ) { ThrowOnError( Add( other, *this ) ); return *this; }
CBigInteger& CBigInteger::operator-=( const CBigInteger& other ) { ThrowOnError( Subtract( other, *this ) ); return *this; }
CBigInteger& CBigInteger::operator*=( const CBigInteger& other ) { ThrowOnError( Multiply( other, *this ) ); return *this; }
CBigInteger& CBigInteger::operator&=( const CBigInteger& other ) { ThrowOnError( And( other, *this ) ); return *this; }
CBigInteger& CBigInteger::operator|=( const CBigInteger& other ) { ThrowOnError( Or( other, *this ) ); return *this; }
CBigInteger& CBigInteger::operator^=( const CBigInteger& other ) { ThrowOnError( Xor( other, *this ) ); return *this; }
CBigInteger& CBigInteger::operator<<=( UInt32 bits ) { ThrowOnError( ShiftLeft( bits, *this ) ); return *this; }
CBigInteger& CBigInteger::operator>>=( UInt32 bits ) { ThrowOnError( ShiftRight( bits, *this ) ); return *this; }
CBigInteger& CBigInteger::operator++( void ) { ThrowOnError( Add( 1, *this ) ); return *this; }
CBigInteger& CBigInteger::operator--( void ) { ThrowOnError( Subtract( 1, *this ) ); return *this; }

/*-------------------------------------------------------------------------*/

CBigInteger&
CBigInteger::operator/=( const CBigInteger& other )
{GUCEF_TRACE;

    CBigInteger quotient;
    ThrowOnError( DivModTruncating( other, &quotient, GUCEF_NULL ) );
    Swap( quotient );
    return *this;
}

/*-------------------------------------------------------------------------*/

CBigInteger&
CBigInteger::operator%=( const CBigInteger& other )
{GUCEF_TRACE;

    CBigInteger remainder;
    ThrowOnError( DivModTruncating( other, GUCEF_NULL, &remainder ) );
    Swap( remainder );
    return *this;
}

/*-------------------------------------------------------------------------*/

CBigInteger
CBigInteger::operator++( int )
{
    CBigInteger previous( *this );
    ++( *this );
    return previous;
}

/*-------------------------------------------------------------------------*/

CBigInteger
CBigInteger::operator--( int )
{
    CBigInteger previous( *this );
    --( *this );
    return previous;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      FRACTION INTEGER OPERATIONS                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTFractionIntegerOps< CBigInteger, CBigInteger > TBigFractionOps;

/*-------------------------------------------------------------------------*/

/** a / b for an exact division (b divides a) */
static void
ExactQuotient( const CBigInteger& a, const CBigInteger& b, CBigInteger& result )
{
    if ( b.IsOne() )
    {
        if ( &result != &a )
            result = a;
        return;
    }
    CBigInteger quotient;
    CBigInteger remainder;
    CBigInteger::ThrowOnError( a.DivMod( b, quotient, remainder ) );
    result.Swap( quotient );
}

/*-------------------------------------------------------------------------*/

bool
TBigFractionOps::Normalize( CBigInteger& n, CBigInteger& d )
{GUCEF_TRACE;

    if ( d.IsNegative() )
    {
        CBigInteger::ThrowOnError( n.Negate( n ) );
        CBigInteger::ThrowOnError( d.Negate( d ) );
    }
    CBigInteger g;
    CBigInteger::ThrowOnError( n.Gcd( d, g ) );
    if ( !g.IsOne() && !g.IsZero() )
    {
        ExactQuotient( n, g, n );
        ExactQuotient( d, g, d );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
TBigFractionOps::Add( CBigInteger& n, CBigInteger& d, const CBigInteger& otherN, const CBigInteger& otherD )
{GUCEF_TRACE;

    // Knuth 4.5.1: with g = gcd( d1, d2 ), t = n1 ( d2 / g ) + n2 ( d1 / g ), g2 = gcd( t, g ):
    // n1/d1 + n2/d2 = ( t / g2 ) / ( ( d1 / g ) ( d2 / g2 ) ) in lowest terms
    CBigInteger g;
    CBigInteger::ThrowOnError( d.Gcd( otherD, g ) );
    CBigInteger d1g;
    CBigInteger d2g;
    ExactQuotient( d, g, d1g );
    ExactQuotient( otherD, g, d2g );
    CBigInteger t;
    CBigInteger product;
    CBigInteger::ThrowOnError( n.Multiply( d2g, t ) );
    CBigInteger::ThrowOnError( otherN.Multiply( d1g, product ) );
    CBigInteger::ThrowOnError( t.Add( product, t ) );
    if ( t.IsZero() )
    {
        n = CBigInteger();
        d = CBigInteger( 1 );
        return true;
    }
    CBigInteger g2;
    CBigInteger::ThrowOnError( t.Gcd( g, g2 ) );
    CBigInteger newN;
    CBigInteger d2g2;
    ExactQuotient( t, g2, newN );
    ExactQuotient( otherD, g2, d2g2 );
    CBigInteger newD;
    CBigInteger::ThrowOnError( d1g.Multiply( d2g2, newD ) );
    n.Swap( newN );
    d.Swap( newD );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
TBigFractionOps::Subtract( CBigInteger& n, CBigInteger& d, const CBigInteger& otherN, const CBigInteger& otherD )
{GUCEF_TRACE;

    CBigInteger negated;
    CBigInteger::ThrowOnError( otherN.Negate( negated ) );
    return Add( n, d, negated, otherD );
}

/*-------------------------------------------------------------------------*/

bool
TBigFractionOps::Multiply( CBigInteger& n, CBigInteger& d, const CBigInteger& otherN, const CBigInteger& otherD )
{GUCEF_TRACE;

    // ( n1 / g1 )( n2 / g2 ) / ( ( d1 / g2 )( d2 / g1 ) ) with g1 = gcd( n1, d2 ), g2 = gcd( n2, d1 )
    CBigInteger g1;
    CBigInteger g2;
    CBigInteger::ThrowOnError( n.Gcd( otherD, g1 ) );
    CBigInteger::ThrowOnError( otherN.Gcd( d, g2 ) );
    if ( g1.IsZero() || g2.IsZero() )
    {
        // a zero numerator: the product is 0/1
        n = CBigInteger();
        d = CBigInteger( 1 );
        return true;
    }
    CBigInteger a;
    CBigInteger b;
    CBigInteger c;
    CBigInteger e;
    ExactQuotient( n, g1, a );
    ExactQuotient( otherN, g2, b );
    ExactQuotient( d, g2, c );
    ExactQuotient( otherD, g1, e );
    CBigInteger newN;
    CBigInteger newD;
    CBigInteger::ThrowOnError( a.Multiply( b, newN ) );
    CBigInteger::ThrowOnError( c.Multiply( e, newD ) );
    if ( newN.IsZero() )
        newD = CBigInteger( 1 );
    n.Swap( newN );
    d.Swap( newD );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
TBigFractionOps::Divide( CBigInteger& n, CBigInteger& d, const CBigInteger& otherN, const CBigInteger& otherD )
{GUCEF_TRACE;

    // multiply by the reciprocal with a positive denominator
    CBigInteger reciprocalN( otherD );
    CBigInteger reciprocalD;
    CBigInteger::ThrowOnError( otherN.Abs( reciprocalD ) );
    if ( otherN.IsNegative() )
        CBigInteger::ThrowOnError( reciprocalN.Negate( reciprocalN ) );
    return Multiply( n, d, reciprocalN, reciprocalD );
}

/*-------------------------------------------------------------------------*/

bool
TBigFractionOps::AddScalar( CBigInteger& n, CBigInteger& d, CBigInteger value )
{GUCEF_TRACE;

    // gcd( n + v d, d ) = gcd( n, d ) = 1: no reduction needed
    CBigInteger::ThrowOnError( value.Multiply( d, value ) );
    CBigInteger::ThrowOnError( n.Add( value, n ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
TBigFractionOps::SubtractScalar( CBigInteger& n, CBigInteger& d, CBigInteger value )
{GUCEF_TRACE;

    CBigInteger::ThrowOnError( value.Multiply( d, value ) );
    CBigInteger::ThrowOnError( n.Subtract( value, n ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
TBigFractionOps::MultiplyScalar( CBigInteger& n, CBigInteger& d, CBigInteger value )
{GUCEF_TRACE;

    return Multiply( n, d, value, CBigInteger( 1 ) );
}

/*-------------------------------------------------------------------------*/

bool
TBigFractionOps::DivideScalar( CBigInteger& n, CBigInteger& d, CBigInteger value )
{GUCEF_TRACE;

    return Divide( n, d, value, CBigInteger( 1 ) );
}

/*-------------------------------------------------------------------------*/

bool
TBigFractionOps::FromMixed( CBigInteger& n, CBigInteger& d, CBigInteger whole, const CBigInteger& fractionN, const CBigInteger& fractionD )
{GUCEF_TRACE;

    CBigInteger newN;
    CBigInteger::ThrowOnError( whole.Multiply( fractionD, newN ) );
    CBigInteger::ThrowOnError( newN.Add( fractionN, newN ) );
    CBigInteger newD( fractionD );
    Normalize( newN, newD );
    n.Swap( newN );
    d.Swap( newD );
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
TBigFractionOps::Compare( const CBigInteger& n1, const CBigInteger& d1, const CBigInteger& n2, const CBigInteger& d2 )
{GUCEF_TRACE;

    // the denominators are positive: the signs of the numerators decide unless they agree
    Int32 s1 = n1.GetSign();
    Int32 s2 = n2.GetSign();
    if ( s1 != s2 )
        return s1 < s2 ? -1 : 1;
    if ( 0 == s1 )
        return 0;
    CBigInteger left;
    CBigInteger right;
    CBigInteger::ThrowOnError( n1.Multiply( d2, left ) );
    CBigInteger::ThrowOnError( n2.Multiply( d1, right ) );
    return left.Compare( right );
}

/*-------------------------------------------------------------------------*/

Float32
TBigFractionOps::ToFloat32( const CBigInteger& n, const CBigInteger& d )
{GUCEF_TRACE;

    Float32 value = 0.0f;
    TBigIntegerStatus status = CBigInteger::RatioToFloat32( n, d, ROUNDINGMODE_HALF_EVEN, value );
    if ( 0 != ( status & BIGINTEGERSTATUS_OVERFLOW ) )
        return (Float32) ( n.IsNegative() != d.IsNegative() ? -HUGE_VAL : HUGE_VAL );
    CBigInteger::ThrowOnError( status );
    return value;
}

/*-------------------------------------------------------------------------*/

Float64
TBigFractionOps::ToFloat64( const CBigInteger& n, const CBigInteger& d )
{GUCEF_TRACE;

    Float64 value = 0.0;
    TBigIntegerStatus status = CBigInteger::RatioToFloat64( n, d, ROUNDINGMODE_HALF_EVEN, value );
    if ( 0 != ( status & BIGINTEGERSTATUS_OVERFLOW ) )
        return n.IsNegative() != d.IsNegative() ? -HUGE_VAL : HUGE_VAL;
    CBigInteger::ThrowOnError( status );
    return value;
}

/*-------------------------------------------------------------------------*/

CString
TBigFractionOps::ToString( const CBigInteger& n, const CBigInteger& d )
{GUCEF_TRACE;

    return n.ToString( 10 ) + "/" + d.ToString( 10 );
}

/*-------------------------------------------------------------------------*/

bool
TBigFractionOps::FromString( const CString& numeratorText, const CString& denominatorText, CBigInteger& n, CBigInteger& d )
{GUCEF_TRACE;

    CBigInteger numerator;
    CBigInteger denominator;
    if ( BIGINTEGERSTATUS_OK != CBigInteger::FromString( numeratorText, 10, numerator ) )
        return false;
    if ( BIGINTEGERSTATUS_OK != CBigInteger::FromString( denominatorText, 10, denominator ) )
        return false;
    if ( denominator.IsZero() )
        return false;
    n.Swap( numerator );
    d.Swap( denominator );
    return true;
}

/*-------------------------------------------------------------------------*/

TDecimalStatus
TBigFractionOps::ToDecimal( const CBigInteger& n, const CBigInteger& d, const CDecimalContext& context, CDecimal& result )
{GUCEF_TRACE;

    return CBigInteger::RatioToDecimal( n, d, context, result );
}

/*-------------------------------------------------------------------------*/

bool
TBigFractionOps::FromDecimal( const CDecimal& value, CBigInteger& n, CBigInteger& d )
{GUCEF_TRACE;

    CBigInteger numerator( value.GetUnscaledValue() );
    CBigInteger denominator( g_bigIntegerPow10[ value.GetScale() <= CDecimal::MaxScale ? value.GetScale() : 0 ] );
    n.Swap( numerator );
    d.Swap( denominator );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
