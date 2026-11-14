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

#ifndef GUCEF_CORE_CUINT128_H
#define GUCEF_CORE_CUINT128_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TYPES_H
#include "gucef_types.h"
#define GUCEF_TYPES_H
#endif /* GUCEF_TYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Portable unsigned 128 bit integer made of two 64 bit words.
 *  Public, header only value class: every function is inline, nothing is exported from the module.
 *  Used by the decimal arithmetic, by CInt128 and by the exact fraction arithmetic.
 *  Everything is header only and uses C++98 constructs and unsigned 64 bit arithmetic only, where wrapping is defined.
 *  No compiler specific 128 bit type is used.
 */
class CUInt128
{
    public:

    UInt64 high;
    UInt64 low;

    CUInt128( void )
        : high( 0 )
        , low( 0 )
    {
    }

    CUInt128( UInt64 highWord, UInt64 lowWord )
        : high( highWord )
        , low( lowWord )
    {
    }

    explicit CUInt128( UInt64 value )
        : high( 0 )
        , low( value )
    {
    }

    bool IsZero( void ) const
    {
        return 0 == high && 0 == low;
    }

    bool FitsUInt64( void ) const
    {
        return 0 == high;
    }

    bool IsOdd( void ) const
    {
        return 0 != ( low & 1 );
    }

    /** -1, 0, 1 */
    static Int32 Compare( const CUInt128& a, const CUInt128& b )
    {
        if ( a.high != b.high )
            return a.high < b.high ? -1 : 1;
        if ( a.low != b.low )
            return a.low < b.low ? -1 : 1;
        return 0;
    }

    /** Wrapping addition modulo 2^128 */
    static CUInt128 Add( const CUInt128& a, const CUInt128& b )
    {
        CUInt128 r( a.high + b.high, a.low + b.low );
        if ( r.low < a.low )
            ++r.high;
        return r;
    }

    /** Addition, returns false when the sum does not fit 128 bits (out is then the wrapped sum) */
    static bool AddChecked( const CUInt128& a, const CUInt128& b, CUInt128& out )
    {
        UInt64 lowSum = a.low + b.low;
        UInt64 carry = lowSum < a.low ? 1 : 0;
        UInt64 highSum = a.high + b.high;
        bool overflow = highSum < a.high;
        UInt64 highTotal = highSum + carry;
        if ( highTotal < highSum )
            overflow = true;
        out.high = highTotal;
        out.low = lowSum;
        return !overflow;
    }

    /** Wrapping subtraction modulo 2^128 */
    static CUInt128 Subtract( const CUInt128& a, const CUInt128& b )
    {
        CUInt128 r( a.high - b.high, a.low - b.low );
        if ( a.low < b.low )
            --r.high;
        return r;
    }

    /** Two's complement negation modulo 2^128 */
    static CUInt128 Negate( const CUInt128& a )
    {
        return Subtract( CUInt128(), a );
    }

    /** Full 64x64 -> 128 bit product using 32 bit halves, four partial products and explicit carries */
    static CUInt128 Multiply( UInt64 a, UInt64 b )
    {
        const UInt64 mask32 = 0xFFFFFFFFULL;
        UInt64 a0 = a & mask32;
        UInt64 a1 = a >> 32;
        UInt64 b0 = b & mask32;
        UInt64 b1 = b >> 32;

        UInt64 p00 = a0 * b0;
        UInt64 p01 = a0 * b1;
        UInt64 p10 = a1 * b0;
        UInt64 p11 = a1 * b1;

        // at most 3 * ( 2^32 - 1 ), cannot wrap
        UInt64 middle = ( p00 >> 32 ) + ( p01 & mask32 ) + ( p10 & mask32 );

        CUInt128 r;
        r.low = ( middle << 32 ) | ( p00 & mask32 );
        r.high = p11 + ( p01 >> 32 ) + ( p10 >> 32 ) + ( middle >> 32 );
        return r;
    }

    /** 128x64 -> 192 bit product, returns false when the product does not fit 128 bits (out is then undefined) */
    static bool MultiplyChecked( const CUInt128& x, UInt64 m, CUInt128& out )
    {
        CUInt128 lowProduct = Multiply( x.low, m );
        CUInt128 highProduct = Multiply( x.high, m );
        if ( 0 != highProduct.high )
            return false;
        UInt64 highWord = lowProduct.high + highProduct.low;
        if ( highWord < lowProduct.high )
            return false;
        out.high = highWord;
        out.low = lowProduct.low;
        return true;
    }

    /** 10^exponent for exponent 0 .. 19 (10^19 < 2^64) */
    static UInt64 Pow10( UInt32 exponent )
    {
        static const UInt64 powersOfTen[ 20 ] =
        {
            1ULL, 10ULL, 100ULL, 1000ULL, 10000ULL, 100000ULL, 1000000ULL, 10000000ULL, 100000000ULL, 1000000000ULL,
            10000000000ULL, 100000000000ULL, 1000000000000ULL, 10000000000000ULL, 100000000000000ULL,
            1000000000000000ULL, 10000000000000000ULL, 100000000000000000ULL, 1000000000000000000ULL,
            10000000000000000000ULL
        };
        return powersOfTen[ exponent ];
    }

    /** Multiplies by 10^exponent in steps of at most 10^19, returns false when the product does not fit 128 bits */
    static bool MultiplyByPow10Checked( CUInt128& value, UInt32 exponent )
    {
        if ( value.IsZero() )
            return true;
        while ( exponent > 0 )
        {
            UInt32 step = exponent > 19 ? 19 : exponent;
            if ( !MultiplyChecked( value, Pow10( step ), value ) )
                return false;
            exponent -= step;
        }
        return true;
    }

    /** Number of leading zero bits of a 64 bit word, 64 for 0. Portable binary search */
    static UInt32 CountLeadingZeros( UInt64 value )
    {
        if ( 0 == value )
            return 64;
        UInt32 n = 0;
        if ( 0 == ( value & 0xFFFFFFFF00000000ULL ) ) { n += 32; value <<= 32; }
        if ( 0 == ( value & 0xFFFF000000000000ULL ) ) { n += 16; value <<= 16; }
        if ( 0 == ( value & 0xFF00000000000000ULL ) ) { n += 8; value <<= 8; }
        if ( 0 == ( value & 0xF000000000000000ULL ) ) { n += 4; value <<= 4; }
        if ( 0 == ( value & 0xC000000000000000ULL ) ) { n += 2; value <<= 2; }
        if ( 0 == ( value & 0x8000000000000000ULL ) ) { n += 1; }
        return n;
    }

    /** Number of leading zero bits, 128 for 0 */
    UInt32 CountLeadingZeros( void ) const
    {
        if ( 0 != high )
            return CountLeadingZeros( high );
        return 64 + CountLeadingZeros( low );
    }

    /** Number of significant bits, 0 for 0 */
    UInt32 BitLength( void ) const
    {
        return 128 - CountLeadingZeros();
    }

    /** Logical shift left, bits shifted out are lost. shift >= 128 gives 0 */
    static CUInt128 ShiftLeft( const CUInt128& a, UInt32 shift )
    {
        if ( 0 == shift )
            return a;
        if ( shift >= 128 )
            return CUInt128();
        if ( shift >= 64 )
            return CUInt128( a.low << ( shift - 64 ), 0 );
        return CUInt128( ( a.high << shift ) | ( a.low >> ( 64 - shift ) ), a.low << shift );
    }

    /** Logical shift right. shift >= 128 gives 0 */
    static CUInt128 ShiftRight( const CUInt128& a, UInt32 shift )
    {
        if ( 0 == shift )
            return a;
        if ( shift >= 128 )
            return CUInt128();
        if ( shift >= 64 )
            return CUInt128( 0, a.high >> ( shift - 64 ) );
        return CUInt128( a.high >> shift, ( a.low >> shift ) | ( a.high << ( 64 - shift ) ) );
    }

    /**
     *  Divides the 128 bit value ( numHigh, numLow ) by divisor where numHigh < divisor so the quotient fits 64 bits.
     *  One normalized 2-by-1 step with two 32 bit digit estimates and correction (Hacker's Delight divlu).
     */
    static UInt64 DivideNormalizedStep( UInt64 numHigh, UInt64 numLow, UInt64 divisor, UInt64& remainder )
    {
        const UInt64 base = 0x100000000ULL;
        const UInt64 mask32 = 0xFFFFFFFFULL;

        UInt32 s = CountLeadingZeros( divisor );
        UInt64 v = divisor << s;
        UInt64 vn1 = v >> 32;
        UInt64 vn0 = v & mask32;

        UInt64 un32 = ( numHigh << s ) | ( 0 == s ? 0 : ( numLow >> ( 64 - s ) ) );
        UInt64 un10 = numLow << s;
        UInt64 un1 = un10 >> 32;
        UInt64 un0 = un10 & mask32;

        UInt64 q1 = un32 / vn1;
        UInt64 rhat = un32 - q1 * vn1;
        while ( q1 >= base || q1 * vn0 > ( ( rhat << 32 ) | un1 ) )
        {
            --q1;
            rhat += vn1;
            if ( rhat >= base )
                break;
        }

        // the true value is below v so the wrapping arithmetic gives the exact result
        UInt64 un21 = ( un32 << 32 ) + un1 - q1 * v;

        UInt64 q0 = un21 / vn1;
        rhat = un21 - q0 * vn1;
        while ( q0 >= base || q0 * vn0 > ( ( rhat << 32 ) | un0 ) )
        {
            --q0;
            rhat += vn1;
            if ( rhat >= base )
                break;
        }

        remainder = ( ( un21 << 32 ) + un0 - q0 * v ) >> s;
        return ( q1 << 32 ) + q0;
    }

    /** n / d with remainder. d must not be 0 */
    static void DivMod( const CUInt128& n, UInt64 d, CUInt128& quotient, UInt64& remainder )
    {
        if ( n.high < d )
        {
            UInt64 r = 0;
            UInt64 q = DivideNormalizedStep( n.high, n.low, d, r );
            quotient.high = 0;
            quotient.low = q;
            remainder = r;
        }
        else
        {
            UInt64 qHigh = n.high / d;
            UInt64 rHigh = n.high % d;
            UInt64 r = 0;
            UInt64 qLow = DivideNormalizedStep( rHigh, n.low, d, r );
            quotient.high = qHigh;
            quotient.low = qLow;
            remainder = r;
        }
    }

    /** Bitwise AND */
    static CUInt128 And( const CUInt128& a, const CUInt128& b )
    {
        return CUInt128( a.high & b.high, a.low & b.low );
    }

    /** Bitwise OR */
    static CUInt128 Or( const CUInt128& a, const CUInt128& b )
    {
        return CUInt128( a.high | b.high, a.low | b.low );
    }

    /** Bitwise XOR */
    static CUInt128 Xor( const CUInt128& a, const CUInt128& b )
    {
        return CUInt128( a.high ^ b.high, a.low ^ b.low );
    }

    /** Bitwise complement */
    static CUInt128 Not( const CUInt128& a )
    {
        return CUInt128( ~a.high, ~a.low );
    }

    /** Number of trailing zero bits of a 64 bit word, 64 for 0 */
    static UInt32 CountTrailingZeros( UInt64 value )
    {
        if ( 0 == value )
            return 64;
        // isolate the lowest set bit, then count the zeros above it
        UInt64 lowestBit = value & ( 0 - value );
        return 63 - CountLeadingZeros( lowestBit );
    }

    /** Number of trailing zero bits, 128 for 0 */
    UInt32 CountTrailingZeros( void ) const
    {
        if ( 0 != low )
            return CountTrailingZeros( low );
        return 64 + CountTrailingZeros( high );
    }

    /** Comparison operators, implemented with Compare. There are deliberately no arithmetic operators */
    bool operator==( const CUInt128& other ) const { return high == other.high && low == other.low; }
    bool operator!=( const CUInt128& other ) const { return !( *this == other ); }
    bool operator<( const CUInt128& other ) const { return Compare( *this, other ) < 0; }
    bool operator<=( const CUInt128& other ) const { return Compare( *this, other ) <= 0; }
    bool operator>( const CUInt128& other ) const { return Compare( *this, other ) > 0; }
    bool operator>=( const CUInt128& other ) const { return Compare( *this, other ) >= 0; }

    /**
     *  128x128 -> 128 bit product, returns false when the product does not fit 128 bits (out is then undefined).
     *  A product with both high words non-zero is always an overflow.
     */
    static bool MultiplyChecked( const CUInt128& a, const CUInt128& b, CUInt128& out )
    {
        if ( 0 != a.high && 0 != b.high )
            return false;
        if ( 0 == b.high )
            return MultiplyChecked( a, b.low, out );
        return MultiplyChecked( b, a.low, out );
    }

    /** 128x128 -> low 128 bits of the product (wrapping) */
    static CUInt128 Multiply128Low( const CUInt128& a, const CUInt128& b )
    {
        CUInt128 r = Multiply( a.low, b.low );
        r.high += a.high * b.low + a.low * b.high;
        return r;
    }

    /**
     *  n / d with remainder for a 128 bit divisor. d must not be 0.
     *  A divisor that fits 64 bits forwards to the 128 by 64 path. Otherwise the quotient fits 64 bits:
     *  Knuth algorithm D with two 64 bit digits, estimate from the top words and correct.
     */
    static void DivMod( const CUInt128& n, const CUInt128& d, CUInt128& quotient, CUInt128& remainder )
    {
        if ( 0 == d.high )
        {
            UInt64 r = 0;
            CUInt128 q;
            DivMod( n, d.low, q, r );
            quotient = q;
            remainder = CUInt128( r );
            return;
        }
        if ( Compare( n, d ) < 0 )
        {
            CUInt128 r = n;
            quotient = CUInt128();
            remainder = r;
            return;
        }

        // normalize so that the top bit of the divisor is set, the quotient then fits 64 bits
        UInt32 s = CountLeadingZeros( d.high );
        CUInt128 v = ShiftLeft( d, s );
        // numerator needs 192 bits after the shift: the overflow word is the bits shifted out of the top
        UInt64 un2 = 0 == s ? 0 : ( n.high >> ( 64 - s ) );
        CUInt128 un = ShiftLeft( n, s );

        // estimate the quotient from the top two words of the numerator (un2:un.high) and v.high
        UInt64 qhat = 0;
        UInt64 rhat = 0;
        bool rhatOverflow = false;
        if ( un2 >= v.high )
        {
            // 2^64 estimate: clamp to 2^64 - 1 as Knuth does
            qhat = ~0ULL;
            // rhat = ( un2:un.high ) - qhat * v.high, with un2 == v.high here
            CUInt128 t = Multiply( qhat, v.high );
            CUInt128 top( un2, un.high );
            CUInt128 diff = Subtract( top, t );
            if ( 0 != diff.high )
                rhatOverflow = true;
            rhat = diff.low;
        }
        else
        {
            UInt64 r = 0;
            qhat = DivideNormalizedStep( un2, un.high, v.high, r );
            rhat = r;
        }

        // refine: while qhat * v.low > ( rhat : un.low ) decrement
        while ( !rhatOverflow )
        {
            CUInt128 prod = Multiply( qhat, v.low );
            CUInt128 rhs( rhat, un.low );
            if ( Compare( prod, rhs ) <= 0 )
                break;
            --qhat;
            UInt64 sum = rhat + v.high;
            if ( sum < rhat )
                rhatOverflow = true;
            rhat = sum;
        }

        // multiply and subtract: numerator ( 192 bits ) - qhat * v ( up to 192 bits )
        CUInt128 lowProd = Multiply( qhat, v.low );
        CUInt128 highProd = Multiply( qhat, v.high );
        // qhat * v = highProd << 64 + lowProd, as three words w2:w1:w0
        UInt64 w0 = lowProd.low;
        UInt64 w1 = lowProd.high + highProd.low;
        UInt64 w2 = highProd.high + ( w1 < lowProd.high ? 1 : 0 );
        // numerator words un2:un.high:un.low minus w2:w1:w0
        UInt64 d0 = un.low - w0;
        UInt64 borrow = un.low < w0 ? 1 : 0;
        UInt64 d1 = un.high - w1 - borrow;
        borrow = ( un.high < w1 || ( un.high == w1 && 0 != borrow ) ) ? 1 : 0;
        UInt64 d2 = un2 - w2 - borrow;
        bool negative = un2 < w2 || ( un2 == w2 && 0 != borrow );
        CUInt128 rem( d1, d0 );
        if ( negative || 0 != d2 )
        {
            // qhat was one too large: add the divisor back
            --qhat;
            rem = Add( rem, v );
        }
        quotient = CUInt128( qhat );
        remainder = ShiftRight( rem, s );
    }

};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CUINT128_H ? */
