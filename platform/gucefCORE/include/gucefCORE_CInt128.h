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

#ifndef GUCEF_CORE_CINT128_H
#define GUCEF_CORE_CINT128_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TYPES_H
#include "gucef_types.h"
#define GUCEF_TYPES_H
#endif /* GUCEF_TYPES_H ? */

#ifndef GUCEF_CORE_CUINT128_H
#include "gucefCORE_CUInt128.h"
#define GUCEF_CORE_CUINT128_H
#endif /* GUCEF_CORE_CUINT128_H ? */

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
 *  Portable signed 128 bit integer: the two's complement pattern in two 64 bit words (same layout as CUInt128).
 *  Range -2^127 .. 2^127 - 1. Header only, C++98, no heap, no exceptions, no arithmetic operators.
 *  All arithmetic is done on unsigned words where wrapping is defined: there is no signed overflow and no
 *  right shift of a negative value on a built in type anywhere. Checked variants return false on overflow and
 *  leave the output as the wrapped value.
 */
class CInt128
{
    public:

    UInt64 high;
    UInt64 low;

    CInt128( void )
        : high( 0 )
        , low( 0 )
    {
    }

    /** Raw two's complement pattern */
    CInt128( UInt64 highWord, UInt64 lowWord )
        : high( highWord )
        , low( lowWord )
    {
    }

    /** Sign extends */
    explicit CInt128( Int64 value )
        : high( value < 0 ? ~0ULL : 0ULL )
        , low( (UInt64) value )
    {
    }

    static CInt128 FromUInt64( UInt64 value )
    {
        return CInt128( 0, value );
    }

    /** false when the value is above 2^127 - 1 */
    static bool FromCUInt128Checked( const CUInt128& value, CInt128& out )
    {
        out = CInt128( value.high, value.low );
        return 0 == ( value.high >> 63 );
    }

    /** false when the magnitude exceeds 2^127 for a negative value, or 2^127 - 1 for a positive one */
    static bool FromSignMagnitude( bool negative, const CUInt128& magnitude, CInt128& out )
    {
        if ( negative )
        {
            CUInt128 n = CUInt128::Negate( magnitude );
            out = CInt128( n.high, n.low );
            // the magnitude is at most 2^127: the negation then has the top bit set, or is 0 for a 0 magnitude
            return magnitude.IsZero() || 0 != ( n.high >> 63 );
        }
        out = CInt128( magnitude.high, magnitude.low );
        return 0 == ( magnitude.high >> 63 );
    }

    bool IsZero( void ) const
    {
        return 0 == high && 0 == low;
    }

    bool IsNegative( void ) const
    {
        return 0 != ( high >> 63 );
    }

    /** -1, 0, 1 */
    Int32 GetSign( void ) const
    {
        if ( IsNegative() )
            return -1;
        return IsZero() ? 0 : 1;
    }

    bool IsOdd( void ) const
    {
        return 0 != ( low & 1 );
    }

    bool FitsInt64( void ) const
    {
        // the high word must be the sign extension of the low word
        return high == ( 0 != ( low >> 63 ) ? ~0ULL : 0ULL );
    }

    /** true when 0 <= value <= 2^64 - 1 */
    bool FitsUInt64( void ) const
    {
        return 0 == high;
    }

    /** Magnitude, valid for the minimum: 2^127 */
    CUInt128 GetMagnitude( void ) const
    {
        CUInt128 raw( high, low );
        return IsNegative() ? CUInt128::Negate( raw ) : raw;
    }

    /** Number of significant bits of the magnitude */
    UInt32 BitLength( void ) const
    {
        return GetMagnitude().BitLength();
    }

    CUInt128 ToCUInt128Raw( void ) const
    {
        return CUInt128( high, low );
    }

    /** -1, 0, 1 signed comparison */
    static Int32 Compare( const CInt128& a, const CInt128& b )
    {
        bool an = a.IsNegative();
        bool bn = b.IsNegative();
        if ( an != bn )
            return an ? -1 : 1;
        // same sign: the unsigned order of the two's complement pattern is the signed order
        return CUInt128::Compare( CUInt128( a.high, a.low ), CUInt128( b.high, b.low ) );
    }

    bool operator==( const CInt128& other ) const { return high == other.high && low == other.low; }
    bool operator!=( const CInt128& other ) const { return !( *this == other ); }
    bool operator<( const CInt128& other ) const { return Compare( *this, other ) < 0; }
    bool operator<=( const CInt128& other ) const { return Compare( *this, other ) <= 0; }
    bool operator>( const CInt128& other ) const { return Compare( *this, other ) > 0; }
    bool operator>=( const CInt128& other ) const { return Compare( *this, other ) >= 0; }

    /** Wrapping addition modulo 2^128 */
    static CInt128 Add( const CInt128& a, const CInt128& b )
    {
        CUInt128 r = CUInt128::Add( CUInt128( a.high, a.low ), CUInt128( b.high, b.low ) );
        return CInt128( r.high, r.low );
    }

    /** Wrapping subtraction modulo 2^128 */
    static CInt128 Subtract( const CInt128& a, const CInt128& b )
    {
        CUInt128 r = CUInt128::Subtract( CUInt128( a.high, a.low ), CUInt128( b.high, b.low ) );
        return CInt128( r.high, r.low );
    }

    /** Wrapping negation: the minimum maps to itself */
    static CInt128 Negate( const CInt128& a )
    {
        CUInt128 r = CUInt128::Negate( CUInt128( a.high, a.low ) );
        return CInt128( r.high, r.low );
    }

    static bool AddChecked( const CInt128& a, const CInt128& b, CInt128& out )
    {
        CInt128 r = Add( a, b );
        // overflow when both operands have the same sign and the result has another
        bool overflow = a.IsNegative() == b.IsNegative() && r.IsNegative() != a.IsNegative();
        out = r;
        return !overflow;
    }

    static bool SubtractChecked( const CInt128& a, const CInt128& b, CInt128& out )
    {
        CInt128 r = Subtract( a, b );
        bool overflow = a.IsNegative() != b.IsNegative() && r.IsNegative() != a.IsNegative();
        out = r;
        return !overflow;
    }

    /** false for the minimum, where the output is the wrapped value (the minimum again) */
    static bool NegateChecked( const CInt128& a, CInt128& out )
    {
        out = Negate( a );
        return !( a.IsNegative() && out.IsNegative() );
    }

    /** Exact signed 64 x 64 -> 128 bit product, cannot overflow: |a * b| <= 2^126 */
    static CInt128 Multiply( Int64 a, Int64 b )
    {
        bool negative = ( a < 0 ) != ( b < 0 );
        UInt64 ma = a < 0 ? 0 - (UInt64) a : (UInt64) a;
        UInt64 mb = b < 0 ? 0 - (UInt64) b : (UInt64) b;
        CUInt128 magnitude = CUInt128::Multiply( ma, mb );
        CInt128 r;
        FromSignMagnitude( negative, magnitude, r );
        return r;
    }

    static bool MultiplyChecked( const CInt128& a, const CInt128& b, CInt128& out )
    {
        bool negative = a.IsNegative() != b.IsNegative();
        CUInt128 product;
        if ( !CUInt128::MultiplyChecked( a.GetMagnitude(), b.GetMagnitude(), product ) )
        {
            CUInt128 wrapped = CUInt128::Multiply128Low( a.GetMagnitude(), b.GetMagnitude() );
            out = CInt128( wrapped.high, wrapped.low );
            if ( negative )
                out = Negate( out );
            return false;
        }
        return FromSignMagnitude( negative, product, out );
    }

    static bool MultiplyChecked( const CInt128& a, Int64 b, CInt128& out )
    {
        return MultiplyChecked( a, CInt128( b ), out );
    }

    /**
     *  Truncating division (toward zero, the remainder takes the sign of n, the C99 rule).
     *  Returns false for division by zero and for the single overflow min / -1; the outputs are then unchanged.
     */
    static bool DivMod( const CInt128& n, const CInt128& d, CInt128& quotient, CInt128& remainder )
    {
        if ( d.IsZero() )
            return false;
        bool negativeQuotient = n.IsNegative() != d.IsNegative();
        CUInt128 q;
        CUInt128 r;
        CUInt128::DivMod( n.GetMagnitude(), d.GetMagnitude(), q, r );
        CInt128 outQ;
        CInt128 outR;
        if ( !FromSignMagnitude( negativeQuotient, q, outQ ) )
            return false;
        // |r| < |d| <= 2^127 so the remainder always fits ( 2^127 only for the zero remainder case of min )
        FromSignMagnitude( n.IsNegative(), r, outR );
        quotient = outQ;
        remainder = outR;
        return true;
    }

    static bool DivMod( const CInt128& n, Int64 d, CInt128& quotient, Int64& remainder )
    {
        CInt128 r;
        CInt128 q;
        if ( !DivMod( n, CInt128( d ), q, r ) )
            return false;
        quotient = q;
        remainder = (Int64) r.low;
        return true;
    }

    /** Floor division: the quotient is rounded toward negative infinity, the remainder takes the sign of d */
    static bool DivModFloor( const CInt128& n, const CInt128& d, CInt128& quotient, CInt128& remainder )
    {
        CInt128 q;
        CInt128 r;
        if ( !DivMod( n, d, q, r ) )
            return false;
        if ( !r.IsZero() && r.IsNegative() != d.IsNegative() )
        {
            CInt128 one( (Int64) 1 );
            // q - 1 cannot overflow here: a non exact negative quotient is above the minimum
            q = Subtract( q, one );
            r = Add( r, d );
        }
        quotient = q;
        remainder = r;
        return true;
    }

    /** Left shift, bits shifted out are lost. shift >= 128 gives 0 */
    static CInt128 ShiftLeft( const CInt128& a, UInt32 shift )
    {
        CUInt128 r = CUInt128::ShiftLeft( CUInt128( a.high, a.low ), shift );
        return CInt128( r.high, r.low );
    }

    /** Logical right shift of the bit pattern. shift >= 128 gives 0 */
    static CInt128 ShiftRightLogical( const CInt128& a, UInt32 shift )
    {
        CUInt128 r = CUInt128::ShiftRight( CUInt128( a.high, a.low ), shift );
        return CInt128( r.high, r.low );
    }

    /** Arithmetic right shift (floor, sign extends). shift >= 128 gives 0 or -1 */
    static CInt128 ShiftRightArithmetic( const CInt128& a, UInt32 shift )
    {
        if ( !a.IsNegative() )
            return ShiftRightLogical( a, shift );
        if ( shift >= 128 )
            return CInt128( ~0ULL, ~0ULL );
        // complement, logical shift, complement: sign fill without a built in signed shift
        CUInt128 inverted = CUInt128::Not( CUInt128( a.high, a.low ) );
        CUInt128 shifted = CUInt128::ShiftRight( inverted, shift );
        CUInt128 r = CUInt128::Not( shifted );
        return CInt128( r.high, r.low );
    }

    static CInt128 And( const CInt128& a, const CInt128& b ) { return CInt128( a.high & b.high, a.low & b.low ); }
    static CInt128 Or( const CInt128& a, const CInt128& b ) { return CInt128( a.high | b.high, a.low | b.low ); }
    static CInt128 Xor( const CInt128& a, const CInt128& b ) { return CInt128( a.high ^ b.high, a.low ^ b.low ); }
    static CInt128 Not( const CInt128& a ) { return CInt128( ~a.high, ~a.low ); }

    bool ToInt64Checked( Int64& out ) const
    {
        out = (Int64) low;
        return FitsInt64();
    }

    bool ToUInt64Checked( UInt64& out ) const
    {
        out = low;
        return FitsUInt64();
    }

    bool ToCUInt128Checked( CUInt128& out ) const
    {
        out = CUInt128( high, low );
        return !IsNegative();
    }

    /** Nearest Float64, ties to even */
    Float64 ToFloat64( void ) const
    {
        CUInt128 m = GetMagnitude();
        Float64 result = 0.0;
        UInt32 bits = m.BitLength();
        if ( bits <= 64 )
        {
            result = UInt64ToFloat64( m.low );
        }
        else
        {
            // keep the top 64 bits and a sticky bit for the rest, then round once through the 64 bit conversion
            UInt32 shift = bits - 64;
            CUInt128 top = CUInt128::ShiftRight( m, shift );
            CUInt128 back = CUInt128::ShiftLeft( top, shift );
            UInt64 mantissa = top.low;
            if ( !( back == m ) )
                mantissa |= 1;     // sticky: the lowest of the 64 bits is below the 53 bit rounding point
            result = UInt64ToFloat64( mantissa ) * PowerOfTwo( shift );
        }
        return IsNegative() ? -result : result;
    }

    /** 10^exponent for exponent 0 .. 19, shared with CUInt128 */
    static UInt64 Pow10( UInt32 exponent )
    {
        return CUInt128::Pow10( exponent );
    }

    private:

    /** Exact 2^n for n in 0 .. 127 built by repeated squaring free multiplication */
    static Float64 PowerOfTwo( UInt32 n )
    {
        Float64 result = 1.0;
        while ( n >= 32 )
        {
            result *= 4294967296.0;
            n -= 32;
        }
        return result * (Float64) ( 1ULL << n );
    }

    /** Correctly rounded (ties to even by the hardware) conversion that does not depend on a signed intermediate */
    static Float64 UInt64ToFloat64( UInt64 value )
    {
        // two exact 32 bit halves: the high half scaled is exact and the single final addition rounds once
        // only when the value exceeds 53 bits; that is the one rounding of the conversion
        UInt64 highHalf = value >> 32;
        UInt64 lowHalf = value & 0xFFFFFFFFULL;
        return (Float64) highHalf * 4294967296.0 + (Float64) lowHalf;
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

#endif /* GUCEF_CORE_CINT128_H ? */
