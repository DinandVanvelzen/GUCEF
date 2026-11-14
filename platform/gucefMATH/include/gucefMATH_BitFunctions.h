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

#ifndef GUCEF_MATH_BITFUNCTIONS_H
#define GUCEF_MATH_BITFUNCTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Bit and integer functions for UInt32 and UInt64.
 *
 *  Compiler builtins are used where the compiler has them, every function has a portable fallback of identical
 *  behaviour. All functions are realtime safe: no allocation, no locks, no loops of unbounded length.
 *  Degenerate inputs are defined: the result for zero is stated per function.
 */

/*-------------------------------------------------------------------------*/

/** Number of set bits */
GUCEF_MATH_FORCE_INLINE UInt32
CountBits( UInt32 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    #if defined( __GNUC__ ) || defined( __clang__ )
    return (UInt32) __builtin_popcount( (unsigned int) value );
    #else
    value = value - ( ( value >> 1 ) & 0x55555555U );
    value = ( value & 0x33333333U ) + ( ( value >> 2 ) & 0x33333333U );
    value = ( value + ( value >> 4 ) ) & 0x0F0F0F0FU;
    return ( value * 0x01010101U ) >> 24;
    #endif
}

/*-------------------------------------------------------------------------*/

/** Number of set bits */
GUCEF_MATH_FORCE_INLINE UInt32
CountBits( UInt64 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    #if defined( __GNUC__ ) || defined( __clang__ )
    return (UInt32) __builtin_popcountll( (unsigned long long) value );
    #else
    value = value - ( ( value >> 1 ) & 0x5555555555555555ULL );
    value = ( value & 0x3333333333333333ULL ) + ( ( value >> 2 ) & 0x3333333333333333ULL );
    value = ( value + ( value >> 4 ) ) & 0x0F0F0F0F0F0F0F0FULL;
    return (UInt32) ( ( value * 0x0101010101010101ULL ) >> 56 );
    #endif
}

/*-------------------------------------------------------------------------*/

/** Number of leading zero bits, 32 for the value 0 */
GUCEF_MATH_FORCE_INLINE UInt32
CountLeadingZeros( UInt32 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == value )
        return 32;
    #if defined( __GNUC__ ) || defined( __clang__ )
    return (UInt32) __builtin_clz( (unsigned int) value );
    #else
    UInt32 n = 0;
    if ( ( value & 0xFFFF0000U ) == 0 ) { n += 16; value <<= 16; }
    if ( ( value & 0xFF000000U ) == 0 ) { n += 8;  value <<= 8;  }
    if ( ( value & 0xF0000000U ) == 0 ) { n += 4;  value <<= 4;  }
    if ( ( value & 0xC0000000U ) == 0 ) { n += 2;  value <<= 2;  }
    if ( ( value & 0x80000000U ) == 0 ) { n += 1; }
    return n;
    #endif
}

/*-------------------------------------------------------------------------*/

/** Number of leading zero bits, 64 for the value 0 */
GUCEF_MATH_FORCE_INLINE UInt32
CountLeadingZeros( UInt64 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == value )
        return 64;
    #if defined( __GNUC__ ) || defined( __clang__ )
    return (UInt32) __builtin_clzll( (unsigned long long) value );
    #else
    UInt32 high = (UInt32) ( value >> 32 );
    if ( 0 != high )
        return CountLeadingZeros( high );
    return 32 + CountLeadingZeros( (UInt32) value );
    #endif
}

/*-------------------------------------------------------------------------*/

/** Number of trailing zero bits, 32 for the value 0 */
GUCEF_MATH_FORCE_INLINE UInt32
CountTrailingZeros( UInt32 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == value )
        return 32;
    #if defined( __GNUC__ ) || defined( __clang__ )
    return (UInt32) __builtin_ctz( (unsigned int) value );
    #else
    return CountBits( (UInt32) ( ( value & ( ~value + 1U ) ) - 1U ) );
    #endif
}

/*-------------------------------------------------------------------------*/

/** Number of trailing zero bits, 64 for the value 0 */
GUCEF_MATH_FORCE_INLINE UInt32
CountTrailingZeros( UInt64 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( 0 == value )
        return 64;
    #if defined( __GNUC__ ) || defined( __clang__ )
    return (UInt32) __builtin_ctzll( (unsigned long long) value );
    #else
    return CountBits( (UInt64) ( ( value & ( ~value + 1ULL ) ) - 1ULL ) );
    #endif
}

/*-------------------------------------------------------------------------*/

/** True when exactly one bit is set. Zero is not a power of two */
GUCEF_MATH_FORCE_INLINE bool
IsPowerOfTwo( UInt32 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    return 0 != value && 0 == ( value & ( value - 1U ) );
}

/*-------------------------------------------------------------------------*/

/** True when exactly one bit is set. Zero is not a power of two */
GUCEF_MATH_FORCE_INLINE bool
IsPowerOfTwo( UInt64 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    return 0 != value && 0 == ( value & ( value - 1ULL ) );
}

/*-------------------------------------------------------------------------*/

/** Floor of the base 2 logarithm. Defined as 0 for the input 0 */
GUCEF_MATH_FORCE_INLINE UInt32
Log2Floor( UInt32 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    return 0 == value ? 0 : 31 - CountLeadingZeros( value );
}

/*-------------------------------------------------------------------------*/

/** Floor of the base 2 logarithm. Defined as 0 for the input 0 */
GUCEF_MATH_FORCE_INLINE UInt32
Log2Floor( UInt64 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    return 0 == value ? 0 : 63 - CountLeadingZeros( value );
}

/*-------------------------------------------------------------------------*/

/** Ceiling of the base 2 logarithm. Defined as 0 for the inputs 0 and 1 */
GUCEF_MATH_FORCE_INLINE UInt32
Log2Ceil( UInt32 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    return value <= 1 ? 0 : 32 - CountLeadingZeros( (UInt32) ( value - 1U ) );
}

/*-------------------------------------------------------------------------*/

/** Ceiling of the base 2 logarithm. Defined as 0 for the inputs 0 and 1 */
GUCEF_MATH_FORCE_INLINE UInt32
Log2Ceil( UInt64 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    return value <= 1 ? 0 : 64 - CountLeadingZeros( (UInt64) ( value - 1ULL ) );
}

/*-------------------------------------------------------------------------*/

/** Smallest power of two that is >= value. 0 maps to 1. Returns 0 when the result is not representable */
GUCEF_MATH_FORCE_INLINE UInt32
NextPowerOfTwo( UInt32 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( value <= 1 )
        return 1;
    if ( value > 0x80000000U )
        return 0;
    return 1U << Log2Ceil( value );
}

/*-------------------------------------------------------------------------*/

/** Smallest power of two that is >= value. 0 maps to 1. Returns 0 when the result is not representable */
GUCEF_MATH_FORCE_INLINE UInt64
NextPowerOfTwo( UInt64 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( value <= 1 )
        return 1;
    if ( value > 0x8000000000000000ULL )
        return 0;
    return 1ULL << Log2Ceil( value );
}

/*-------------------------------------------------------------------------*/

/** Largest power of two that is <= value. 0 maps to 0 */
GUCEF_MATH_FORCE_INLINE UInt32
PreviousPowerOfTwo( UInt32 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    return 0 == value ? 0 : 1U << Log2Floor( value );
}

/*-------------------------------------------------------------------------*/

/** Largest power of two that is <= value. 0 maps to 0 */
GUCEF_MATH_FORCE_INLINE UInt64
PreviousPowerOfTwo( UInt64 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    return 0 == value ? 0 : 1ULL << Log2Floor( value );
}

/*-------------------------------------------------------------------------*/

/** Rotate the bits left, the count is taken modulo the bit width */
GUCEF_MATH_FORCE_INLINE UInt32
RotateLeft( UInt32 value, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    count &= 31U;
    return 0 == count ? value : ( value << count ) | ( value >> ( 32U - count ) );
}

/*-------------------------------------------------------------------------*/

/** Rotate the bits left, the count is taken modulo the bit width */
GUCEF_MATH_FORCE_INLINE UInt64
RotateLeft( UInt64 value, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    count &= 63U;
    return 0 == count ? value : ( value << count ) | ( value >> ( 64U - count ) );
}

/*-------------------------------------------------------------------------*/

/** Rotate the bits right, the count is taken modulo the bit width */
GUCEF_MATH_FORCE_INLINE UInt32
RotateRight( UInt32 value, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    count &= 31U;
    return 0 == count ? value : ( value >> count ) | ( value << ( 32U - count ) );
}

/*-------------------------------------------------------------------------*/

/** Rotate the bits right, the count is taken modulo the bit width */
GUCEF_MATH_FORCE_INLINE UInt64
RotateRight( UInt64 value, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    count &= 63U;
    return 0 == count ? value : ( value >> count ) | ( value << ( 64U - count ) );
}

/*-------------------------------------------------------------------------*/

/** Reverse the order of all bits */
GUCEF_MATH_FORCE_INLINE UInt32
ReverseBits( UInt32 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    value = ( ( value >> 1 ) & 0x55555555U ) | ( ( value & 0x55555555U ) << 1 );
    value = ( ( value >> 2 ) & 0x33333333U ) | ( ( value & 0x33333333U ) << 2 );
    value = ( ( value >> 4 ) & 0x0F0F0F0FU ) | ( ( value & 0x0F0F0F0FU ) << 4 );
    value = ( ( value >> 8 ) & 0x00FF00FFU ) | ( ( value & 0x00FF00FFU ) << 8 );
    return ( value >> 16 ) | ( value << 16 );
}

/*-------------------------------------------------------------------------*/

/** Reverse the order of all bits */
GUCEF_MATH_FORCE_INLINE UInt64
ReverseBits( UInt64 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    value = ( ( value >> 1 ) & 0x5555555555555555ULL ) | ( ( value & 0x5555555555555555ULL ) << 1 );
    value = ( ( value >> 2 ) & 0x3333333333333333ULL ) | ( ( value & 0x3333333333333333ULL ) << 2 );
    value = ( ( value >> 4 ) & 0x0F0F0F0F0F0F0F0FULL ) | ( ( value & 0x0F0F0F0F0F0F0F0FULL ) << 4 );
    value = ( ( value >> 8 ) & 0x00FF00FF00FF00FFULL ) | ( ( value & 0x00FF00FF00FF00FFULL ) << 8 );
    value = ( ( value >> 16 ) & 0x0000FFFF0000FFFFULL ) | ( ( value & 0x0000FFFF0000FFFFULL ) << 16 );
    return ( value >> 32 ) | ( value << 32 );
}

/*-------------------------------------------------------------------------*/

/** Round up to a multiple of a power of two alignment. The result wraps around when it is not representable */
GUCEF_MATH_FORCE_INLINE UInt32
AlignUp( UInt32 value, UInt32 alignment ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_ASSERT( IsPowerOfTwo( alignment ) );
    return ( value + ( alignment - 1U ) ) & ~( alignment - 1U );
}

/*-------------------------------------------------------------------------*/

/** Round up to a multiple of a power of two alignment. The result wraps around when it is not representable */
GUCEF_MATH_FORCE_INLINE UInt64
AlignUp( UInt64 value, UInt64 alignment ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_ASSERT( IsPowerOfTwo( alignment ) );
    return ( value + ( alignment - 1ULL ) ) & ~( alignment - 1ULL );
}

/*-------------------------------------------------------------------------*/

/** Round down to a multiple of a power of two alignment */
GUCEF_MATH_FORCE_INLINE UInt32
AlignDown( UInt32 value, UInt32 alignment ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_ASSERT( IsPowerOfTwo( alignment ) );
    return value & ~( alignment - 1U );
}

/*-------------------------------------------------------------------------*/

/** Round down to a multiple of a power of two alignment */
GUCEF_MATH_FORCE_INLINE UInt64
AlignDown( UInt64 value, UInt64 alignment ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_ASSERT( IsPowerOfTwo( alignment ) );
    return value & ~( alignment - 1ULL );
}

/*-------------------------------------------------------------------------*/

/** Spreads the low 32 bits of the value to the even bit positions */
GUCEF_MATH_FORCE_INLINE UInt64
MortonSpreadBits2D( UInt32 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    UInt64 x = value;
    x = ( x | ( x << 16 ) ) & 0x0000FFFF0000FFFFULL;
    x = ( x | ( x << 8 ) )  & 0x00FF00FF00FF00FFULL;
    x = ( x | ( x << 4 ) )  & 0x0F0F0F0F0F0F0F0FULL;
    x = ( x | ( x << 2 ) )  & 0x3333333333333333ULL;
    x = ( x | ( x << 1 ) )  & 0x5555555555555555ULL;
    return x;
}

/*-------------------------------------------------------------------------*/

/** Inverse of MortonSpreadBits2D: gathers the even bit positions */
GUCEF_MATH_FORCE_INLINE UInt32
MortonGatherBits2D( UInt64 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    UInt64 x = value & 0x5555555555555555ULL;
    x = ( x | ( x >> 1 ) )  & 0x3333333333333333ULL;
    x = ( x | ( x >> 2 ) )  & 0x0F0F0F0F0F0F0F0FULL;
    x = ( x | ( x >> 4 ) )  & 0x00FF00FF00FF00FFULL;
    x = ( x | ( x >> 8 ) )  & 0x0000FFFF0000FFFFULL;
    x = ( x | ( x >> 16 ) ) & 0x00000000FFFFFFFFULL;
    return (UInt32) x;
}

/*-------------------------------------------------------------------------*/

/** Spreads the low 21 bits of the value to every third bit position */
GUCEF_MATH_FORCE_INLINE UInt64
MortonSpreadBits3D( UInt32 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    UInt64 x = value & 0x1FFFFFULL;
    x = ( x | ( x << 32 ) ) & 0x001F00000000FFFFULL;
    x = ( x | ( x << 16 ) ) & 0x001F0000FF0000FFULL;
    x = ( x | ( x << 8 ) )  & 0x100F00F00F00F00FULL;
    x = ( x | ( x << 4 ) )  & 0x10C30C30C30C30C3ULL;
    x = ( x | ( x << 2 ) )  & 0x1249249249249249ULL;
    return x;
}

/*-------------------------------------------------------------------------*/

/** Inverse of MortonSpreadBits3D: gathers every third bit position */
GUCEF_MATH_FORCE_INLINE UInt32
MortonGatherBits3D( UInt64 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    UInt64 x = value & 0x1249249249249249ULL;
    x = ( x | ( x >> 2 ) )  & 0x10C30C30C30C30C3ULL;
    x = ( x | ( x >> 4 ) )  & 0x100F00F00F00F00FULL;
    x = ( x | ( x >> 8 ) )  & 0x001F0000FF0000FFULL;
    x = ( x | ( x >> 16 ) ) & 0x001F00000000FFFFULL;
    x = ( x | ( x >> 32 ) ) & 0x00000000001FFFFFULL;
    return (UInt32) x;
}

/*-------------------------------------------------------------------------*/

/** Morton (Z-order) code of a 2D cell: bit i of x goes to bit 2i, bit i of y to bit 2i+1 */
GUCEF_MATH_FORCE_INLINE UInt64
MortonEncode2D( UInt32 x, UInt32 y ) GUCEF_MATH_REALTIME_CONTEXT
{
    return MortonSpreadBits2D( x ) | ( MortonSpreadBits2D( y ) << 1 );
}

/*-------------------------------------------------------------------------*/

/** Inverse of MortonEncode2D */
GUCEF_MATH_FORCE_INLINE void
MortonDecode2D( UInt64 code, UInt32& x, UInt32& y ) GUCEF_MATH_REALTIME_CONTEXT
{
    x = MortonGatherBits2D( code );
    y = MortonGatherBits2D( code >> 1 );
}

/*-------------------------------------------------------------------------*/

/** Morton code of a 3D cell with 21 bits per coordinate (higher bits are ignored): x to bit 3i, y to 3i+1, z to 3i+2 */
GUCEF_MATH_FORCE_INLINE UInt64
MortonEncode3D( UInt32 x, UInt32 y, UInt32 z ) GUCEF_MATH_REALTIME_CONTEXT
{
    return MortonSpreadBits3D( x ) | ( MortonSpreadBits3D( y ) << 1 ) | ( MortonSpreadBits3D( z ) << 2 );
}

/*-------------------------------------------------------------------------*/

/** Inverse of MortonEncode3D */
GUCEF_MATH_FORCE_INLINE void
MortonDecode3D( UInt64 code, UInt32& x, UInt32& y, UInt32& z ) GUCEF_MATH_REALTIME_CONTEXT
{
    x = MortonGatherBits3D( code );
    y = MortonGatherBits3D( code >> 1 );
    z = MortonGatherBits3D( code >> 2 );
}

/*-------------------------------------------------------------------------*/

/**
 *  Distance along the Hilbert curve of the cell (x, y) in a grid of 2^order x 2^order cells (order 0 to 32;
 *  larger orders are treated as 32). Coordinates are taken modulo the grid size. Consecutive distances are
 *  neighbouring cells.
 */
GUCEF_MATH_FORCE_INLINE UInt64
HilbertEncode2D( UInt32 order, UInt32 x, UInt32 y ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( order > 32 )
        order = 32;
    UInt64 n = 1ULL << order;
    UInt64 cx = x & ( n - 1ULL );
    UInt64 cy = y & ( n - 1ULL );
    UInt64 d = 0;
    for ( UInt64 s = n >> 1; s > 0; s >>= 1 )
    {
        UInt64 rx = ( cx & s ) > 0 ? 1 : 0;
        UInt64 ry = ( cy & s ) > 0 ? 1 : 0;
        d += s * s * ( ( 3ULL * rx ) ^ ry );
        if ( 0 == ry )
        {
            if ( 1 == rx )
            {
                cx = n - 1ULL - cx;
                cy = n - 1ULL - cy;
            }
            UInt64 t = cx; cx = cy; cy = t;
        }
    }
    return d;
}

/*-------------------------------------------------------------------------*/

/** Inverse of HilbertEncode2D */
GUCEF_MATH_FORCE_INLINE void
HilbertDecode2D( UInt32 order, UInt64 distance, UInt32& x, UInt32& y ) GUCEF_MATH_REALTIME_CONTEXT
{
    if ( order > 32 )
        order = 32;
    UInt64 n = 1ULL << order;
    UInt64 t = distance;
    UInt64 cx = 0;
    UInt64 cy = 0;
    for ( UInt64 s = 1; s < n; s <<= 1 )
    {
        UInt64 rx = 1ULL & ( t >> 1 );
        UInt64 ry = 1ULL & ( t ^ rx );
        if ( 0 == ry )
        {
            if ( 1 == rx )
            {
                cx = s - 1ULL - cx;
                cy = s - 1ULL - cy;
            }
            UInt64 tmp = cx; cx = cy; cy = tmp;
        }
        cx += s * rx;
        cy += s * ry;
        t >>= 2;
    }
    x = (UInt32) cx;
    y = (UInt32) cy;
}

/*-------------------------------------------------------------------------*/

/** 32 bit integer hash with good avalanche (the lowbias32 finaliser of C. Wellons). Maps 0 to 0 */
GUCEF_MATH_FORCE_INLINE UInt32
HashUInt32( UInt32 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    value ^= value >> 16;
    value *= 0x7FEB352DU;
    value ^= value >> 15;
    value *= 0x846CA68BU;
    value ^= value >> 16;
    return value;
}

/*-------------------------------------------------------------------------*/

/** 64 bit integer hash: the SplitMix64 finaliser. Maps 0 to 0 */
GUCEF_MATH_FORCE_INLINE UInt64
HashUInt64( UInt64 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    value = ( value ^ ( value >> 30 ) ) * 0xBF58476D1CE4E5B9ULL;
    value = ( value ^ ( value >> 27 ) ) * 0x94D049BB133111EBULL;
    return value ^ ( value >> 31 );
}

/*-------------------------------------------------------------------------*/

/** Combines a running hash with a further value, order dependent */
GUCEF_MATH_FORCE_INLINE UInt64
HashCombine( UInt64 seed, UInt64 value ) GUCEF_MATH_REALTIME_CONTEXT
{
    return seed ^ ( HashUInt64( value ) + 0x9E3779B97F4A7C15ULL + ( seed << 6 ) + ( seed >> 2 ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_BITFUNCTIONS_H ? */
