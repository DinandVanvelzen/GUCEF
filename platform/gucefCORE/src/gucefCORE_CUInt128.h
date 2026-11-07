/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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
 *  Internal helper of the gucefCORE decimal arithmetic: it is not exported and not part of the public API.
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
