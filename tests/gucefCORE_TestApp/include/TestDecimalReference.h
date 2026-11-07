/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_TESTAPP_TESTDECIMALREFERENCE_H
#define GUCEF_CORE_TESTAPP_TESTDECIMALREFERENCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Test-only reference arithmetic for the decimal and 128 bit tests.
 *  A signed big integer in base 10^9 limbs with schoolbook operations and a decimal digit long division.
 *  It uses a different radix and different algorithms from the binary 128 bit code under test, so the two are
 *  unlikely to share a bug. It is slow and simple on purpose.
 */
namespace TestDecimalReference {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CRefInt
{
    public:

    static const UInt32 Base = 1000000000UL;

    bool negative;
    GUCEF::vector< UInt32 > limbs;      /**< little endian base 10^9 limbs, no leading zero limbs, empty for 0 */

    CRefInt( void )
        : negative( false )
        , limbs()
    {
    }

    bool IsZero( void ) const
    {
        return limbs.empty();
    }

    void Trim( void )
    {
        while ( !limbs.empty() && 0 == limbs.back() )
            limbs.pop_back();
        if ( limbs.empty() )
            negative = false;
    }

    static CRefInt FromUInt64( UInt64 value )
    {
        CRefInt r;
        while ( 0 != value )
        {
            r.limbs.push_back( (UInt32) ( value % Base ) );
            value /= Base;
        }
        return r;
    }

    static CRefInt FromInt64( Int64 value )
    {
        // magnitude through decimal text so no signed overflow is possible for INT64_MIN either
        bool isNegative = value < 0;
        UInt64 magnitude = 0;
        if ( isNegative )
        {
            magnitude = (UInt64) ( -( value + 1 ) ) + 1;
        }
        else
        {
            magnitude = (UInt64) value;
        }
        CRefInt r = FromUInt64( magnitude );
        r.negative = isNegative && !r.IsZero();
        return r;
    }

    /** 2^bits */
    static CRefInt PowerOfTwo( UInt32 bits )
    {
        CRefInt r = FromUInt64( 1 );
        for ( UInt32 i=0; i<bits; ++i )
            r = MultiplySmall( r, 2 );
        return r;
    }

    /** 10^exponent */
    static CRefInt PowerOfTen( UInt32 exponent )
    {
        CRefInt r = FromUInt64( 1 );
        for ( UInt32 i=0; i<exponent; ++i )
            r = MultiplySmall( r, 10 );
        return r;
    }

    /** From the unsigned 128 bit value high * 2^64 + low */
    static CRefInt FromWords( UInt64 high, UInt64 low )
    {
        CRefInt h = FromUInt64( high );
        CRefInt shifted = Multiply( h, PowerOfTwo( 64 ) );
        return Add( shifted, FromUInt64( low ) );
    }

    static Int32 CompareMagnitude( const CRefInt& a, const CRefInt& b )
    {
        if ( a.limbs.size() != b.limbs.size() )
            return a.limbs.size() < b.limbs.size() ? -1 : 1;
        for ( size_t i=a.limbs.size(); i>0; --i )
        {
            if ( a.limbs[ i-1 ] != b.limbs[ i-1 ] )
                return a.limbs[ i-1 ] < b.limbs[ i-1 ] ? -1 : 1;
        }
        return 0;
    }

    static Int32 Compare( const CRefInt& a, const CRefInt& b )
    {
        if ( a.negative != b.negative )
            return a.negative ? -1 : 1;
        Int32 cmp = CompareMagnitude( a, b );
        return a.negative ? -cmp : cmp;
    }

    static CRefInt AddMagnitudes( const CRefInt& a, const CRefInt& b )
    {
        CRefInt r;
        size_t n = a.limbs.size() > b.limbs.size() ? a.limbs.size() : b.limbs.size();
        UInt32 carry = 0;
        for ( size_t i=0; i<n; ++i )
        {
            UInt32 sum = carry;
            if ( i < a.limbs.size() ) sum += a.limbs[ i ];
            if ( i < b.limbs.size() ) sum += b.limbs[ i ];
            carry = sum >= Base ? 1 : 0;
            r.limbs.push_back( sum - carry * Base );
        }
        if ( 0 != carry )
            r.limbs.push_back( carry );
        r.Trim();
        return r;
    }

    /** |a| - |b| for |a| >= |b| */
    static CRefInt SubtractMagnitudes( const CRefInt& a, const CRefInt& b )
    {
        CRefInt r;
        Int32 borrow = 0;
        for ( size_t i=0; i<a.limbs.size(); ++i )
        {
            Int64 d = (Int64) a.limbs[ i ] - borrow - ( i < b.limbs.size() ? (Int64) b.limbs[ i ] : 0 );
            borrow = 0;
            if ( d < 0 )
            {
                d += Base;
                borrow = 1;
            }
            r.limbs.push_back( (UInt32) d );
        }
        r.Trim();
        return r;
    }

    static CRefInt Add( const CRefInt& a, const CRefInt& b )
    {
        if ( a.negative == b.negative )
        {
            CRefInt r = AddMagnitudes( a, b );
            r.negative = a.negative && !r.IsZero();
            return r;
        }
        if ( CompareMagnitude( a, b ) >= 0 )
        {
            CRefInt r = SubtractMagnitudes( a, b );
            r.negative = a.negative && !r.IsZero();
            return r;
        }
        CRefInt r = SubtractMagnitudes( b, a );
        r.negative = b.negative && !r.IsZero();
        return r;
    }

    static CRefInt Negated( const CRefInt& a )
    {
        CRefInt r = a;
        r.negative = !a.negative && !a.IsZero();
        return r;
    }

    static CRefInt Subtract( const CRefInt& a, const CRefInt& b )
    {
        return Add( a, Negated( b ) );
    }

    static CRefInt Abs( const CRefInt& a )
    {
        CRefInt r = a;
        r.negative = false;
        return r;
    }

    static CRefInt MultiplySmall( const CRefInt& a, UInt32 m )
    {
        CRefInt r;
        UInt64 carry = 0;
        for ( size_t i=0; i<a.limbs.size(); ++i )
        {
            UInt64 p = (UInt64) a.limbs[ i ] * m + carry;
            r.limbs.push_back( (UInt32) ( p % Base ) );
            carry = p / Base;
        }
        while ( 0 != carry )
        {
            r.limbs.push_back( (UInt32) ( carry % Base ) );
            carry /= Base;
        }
        r.negative = a.negative;
        r.Trim();
        return r;
    }

    static CRefInt Multiply( const CRefInt& a, const CRefInt& b )
    {
        CRefInt r;
        if ( a.IsZero() || b.IsZero() )
            return r;
        GUCEF::vector< UInt64 > acc( a.limbs.size() + b.limbs.size() + 1, 0 );
        for ( size_t i=0; i<a.limbs.size(); ++i )
        {
            UInt64 carry = 0;
            for ( size_t j=0; j<b.limbs.size(); ++j )
            {
                UInt64 cur = acc[ i+j ] + (UInt64) a.limbs[ i ] * b.limbs[ j ] + carry;
                acc[ i+j ] = cur % Base;
                carry = cur / Base;
            }
            size_t k = i + b.limbs.size();
            while ( 0 != carry )
            {
                UInt64 cur = acc[ k ] + carry;
                acc[ k ] = cur % Base;
                carry = cur / Base;
                ++k;
            }
        }
        for ( size_t i=0; i<acc.size(); ++i )
            r.limbs.push_back( (UInt32) acc[ i ] );
        r.negative = a.negative != b.negative;
        r.Trim();
        return r;
    }

    /** Decimal digits of the magnitude, most significant first, "0" for 0 */
    GUCEF::vector< char > Digits( void ) const
    {
        GUCEF::vector< char > digits;
        if ( limbs.empty() )
        {
            digits.push_back( '0' );
            return digits;
        }
        for ( size_t i=limbs.size(); i>0; --i )
        {
            UInt32 limb = limbs[ i-1 ];
            char chunk[ 9 ];
            for ( int d=8; d>=0; --d )
            {
                chunk[ d ] = (char) ( '0' + limb % 10 );
                limb /= 10;
            }
            int start = 0;
            if ( i == limbs.size() )
            {
                while ( start < 8 && '0' == chunk[ start ] )
                    ++start;
            }
            for ( int d=start; d<9; ++d )
                digits.push_back( chunk[ d ] );
        }
        return digits;
    }

    /** Magnitude from decimal digits */
    static CRefInt FromDigits( const char* digits, UInt32 count )
    {
        CRefInt r;
        for ( UInt32 i=0; i<count; ++i )
        {
            r = MultiplySmall( r, 10 );
            r = AddMagnitudes( r, FromUInt64( (UInt64) ( digits[ i ] - '0' ) ) );
        }
        return r;
    }

    /** Magnitudes: n = q * d + r with 0 <= r < d, decimal digit long division. d must not be 0 */
    static void DivMod( const CRefInt& n, const CRefInt& d, CRefInt& quotient, CRefInt& remainder )
    {
        CRefInt divisor = Abs( d );
        CRefInt multiples[ 10 ];
        for ( UInt32 k=0; k<10; ++k )
            multiples[ k ] = MultiplySmall( divisor, k );

        GUCEF::vector< char > digits = Abs( n ).Digits();
        GUCEF::vector< char > qDigits;
        CRefInt r;
        for ( size_t i=0; i<digits.size(); ++i )
        {
            r = MultiplySmall( r, 10 );
            r = AddMagnitudes( r, FromUInt64( (UInt64) ( digits[ i ] - '0' ) ) );
            UInt32 k = 9;
            while ( k > 0 && CompareMagnitude( multiples[ k ], r ) > 0 )
                --k;
            r = SubtractMagnitudes( r, multiples[ k ] );
            qDigits.push_back( (char) ( '0' + k ) );
        }
        quotient = FromDigits( &qDigits[ 0 ], (UInt32) qDigits.size() );
        remainder = r;
    }

    /** The value as Int64 when it fits the symmetric decimal range -(2^63 - 1) .. 2^63 - 1 */
    bool ToInt64( Int64& value ) const
    {
        static const CRefInt maxMagnitude = FromUInt64( 9223372036854775807ULL );
        if ( CompareMagnitude( *this, maxMagnitude ) > 0 )
            return false;
        UInt64 magnitude = 0;
        for ( size_t i=limbs.size(); i>0; --i )
            magnitude = magnitude * Base + limbs[ i-1 ];
        value = negative ? -(Int64) magnitude : (Int64) magnitude;
        return true;
    }

    /** The value as UInt64 words when the magnitude fits 128 bit */
    bool ToWords( UInt64& high, UInt64& low ) const
    {
        static const CRefInt two64 = PowerOfTwo( 64 );
        CRefInt q, r;
        DivMod( *this, two64, q, r );
        if ( CompareMagnitude( q, two64 ) >= 0 )
            return false;
        UInt64 h = 0;
        for ( size_t i=q.limbs.size(); i>0; --i )
            h = h * Base + q.limbs[ i-1 ];
        UInt64 l = 0;
        for ( size_t i=r.limbs.size(); i>0; --i )
            l = l * Base + r.limbs[ i-1 ];
        high = h;
        low = l;
        return true;
    }
};

/*-------------------------------------------------------------------------*/

/** Expected outcome of an operation computed by the reference */
struct SRefOutcome
{
    CORE::TDecimalStatus status;
    Int64 unscaled;
    UInt8 scale;
};

/*-------------------------------------------------------------------------*/

/**
 *  round( n / d ) for the signed exact value n / d (d > 0) per the rounding mode table, written independently from the library:
 *  the discarded fraction r / d is compared with one half as 2r against d.
 */
inline CRefInt
RoundedQuotient( const CRefInt& n, const CRefInt& d, CORE::TRoundingMode mode, bool& inexact, bool& roundingRequired )
{
    CRefInt q, r;
    CRefInt::DivMod( n, d, q, r );
    bool isNegative = n.negative;
    inexact = !r.IsZero();
    roundingRequired = false;
    if ( inexact )
    {
        Int32 vsHalf = CRefInt::CompareMagnitude( CRefInt::MultiplySmall( r, 2 ), CRefInt::Abs( d ) );
        bool up = false;
        switch ( mode )
        {
            case CORE::ROUNDINGMODE_HALF_EVEN:
            {
                bool odd = !q.IsZero() && 0 != ( q.limbs[ 0 ] % 2 );
                up = vsHalf > 0 || ( 0 == vsHalf && odd );
                break;
            }
            case CORE::ROUNDINGMODE_HALF_UP: up = vsHalf >= 0; break;
            case CORE::ROUNDINGMODE_HALF_DOWN: up = vsHalf > 0; break;
            case CORE::ROUNDINGMODE_TOWARD_ZERO: up = false; break;
            case CORE::ROUNDINGMODE_AWAY_FROM_ZERO: up = true; break;
            case CORE::ROUNDINGMODE_FLOOR: up = isNegative; break;
            case CORE::ROUNDINGMODE_CEILING: up = !isNegative; break;
            default: roundingRequired = true; break;
        }
        if ( up )
            q = CRefInt::AddMagnitudes( q, CRefInt::FromUInt64( 1 ) );
    }
    q.negative = isNegative && !q.IsZero();
    return q;
}

/*-------------------------------------------------------------------------*/

/** The expected result of rounding the signed exact value n / d to 'scale' */
inline SRefOutcome
ExpectRounded( const CRefInt& n, const CRefInt& d, CORE::TRoundingMode mode, UInt8 scale )
{
    SRefOutcome outcome;
    outcome.status = CORE::DECIMALSTATUS_OK;
    outcome.unscaled = 0;
    outcome.scale = scale;
    if ( scale > CORE::CDecimal::MaxScale )
    {
        outcome.status = CORE::DECIMALSTATUS_INVALID_SCALE;
        return outcome;
    }
    // a truncated quotient that does not fit overflows in every mode
    CRefInt truncated, remainder;
    CRefInt::DivMod( n, d, truncated, remainder );
    Int64 unused = 0;
    if ( !truncated.ToInt64( unused ) )
    {
        outcome.status = CORE::DECIMALSTATUS_OVERFLOW;
        return outcome;
    }
    bool inexact = false;
    bool roundingRequired = false;
    CRefInt q = RoundedQuotient( n, d, mode, inexact, roundingRequired );
    if ( roundingRequired )
    {
        outcome.status = CORE::DECIMALSTATUS_ROUNDING_REQUIRED;
        return outcome;
    }
    if ( !q.ToInt64( outcome.unscaled ) )
    {
        outcome.status = CORE::DECIMALSTATUS_OVERFLOW;
        return outcome;
    }
    outcome.status = inexact ? CORE::DECIMALSTATUS_INEXACT : CORE::DECIMALSTATUS_OK;
    return outcome;
}

/*-------------------------------------------------------------------------*/

/** n * 10^exponent / 1 for exponent >= 0, n / 10^-exponent otherwise: the numerator and denominator of a scaled value */
inline void
ScaledFraction( const CRefInt& value, Int32 exponent, CRefInt& n, CRefInt& d )
{
    if ( exponent >= 0 )
    {
        n = CRefInt::Multiply( value, CRefInt::PowerOfTen( (UInt32) exponent ) );
        d = CRefInt::FromUInt64( 1 );
    }
    else
    {
        n = value;
        d = CRefInt::PowerOfTen( (UInt32) -exponent );
    }
}

/*-------------------------------------------------------------------------*/

inline CRefInt
UnscaledOf( const CORE::CDecimal& value )
{
    return CRefInt::FromInt64( value.GetUnscaledValue() );
}

/*-------------------------------------------------------------------------*/

/**
 *  Reference parser for the strict grammar [ "-" | "+" ] ( digits [ "." [ digits ] ] | "." digits ), written without
 *  sharing any code with the library parser. Gives the status and the value it expects.
 */
inline SRefOutcome
ReferenceParse( const char* text, UInt32 length )
{
    SRefOutcome outcome;
    outcome.status = CORE::DECIMALSTATUS_OK;
    outcome.unscaled = 0;
    outcome.scale = 0;

    UInt32 pos = 0;
    bool isNegative = false;
    if ( pos < length && ( text[ pos ] == '+' || text[ pos ] == '-' ) )
    {
        isNegative = text[ pos ] == '-';
        ++pos;
    }
    GUCEF::vector< char > digits;
    UInt32 fractionDigits = 0;
    bool seenPoint = false;
    bool anyDigit = false;
    for ( ; pos<length; ++pos )
    {
        char c = text[ pos ];
        if ( c >= '0' && c <= '9' )
        {
            digits.push_back( c );
            anyDigit = true;
            if ( seenPoint )
                ++fractionDigits;
        }
        else
        if ( c == '.' && !seenPoint )
        {
            seenPoint = true;
        }
        else
        {
            outcome.status = CORE::DECIMALSTATUS_INVALID_FORMAT;
            return outcome;
        }
    }
    if ( !anyDigit )
    {
        outcome.status = CORE::DECIMALSTATUS_INVALID_FORMAT;
        return outcome;
    }
    if ( fractionDigits > CORE::CDecimal::MaxScale )
    {
        outcome.status = CORE::DECIMALSTATUS_INVALID_SCALE;
        return outcome;
    }
    CRefInt magnitude = CRefInt::FromDigits( &digits[ 0 ], (UInt32) digits.size() );
    magnitude.negative = isNegative && !magnitude.IsZero();
    if ( !magnitude.ToInt64( outcome.unscaled ) )
    {
        outcome.status = CORE::DECIMALSTATUS_OVERFLOW;
        outcome.unscaled = 0;
        return outcome;
    }
    outcome.scale = (UInt8) fractionDigits;
    return outcome;
}

/*-------------------------------------------------------------------------*/

/** splitmix64: small seeded generator so every run with the same seed checks the same values */
class CSplitMix64
{
    public:

    explicit CSplitMix64( UInt64 seed )
        : m_state( seed )
    {
    }

    UInt64 Next( void )
    {
        UInt64 z = ( m_state += 0x9E3779B97F4A7C15ULL );
        z = ( z ^ ( z >> 30 ) ) * 0xBF58476D1CE4E5B9ULL;
        z = ( z ^ ( z >> 27 ) ) * 0x94D049BB133111EBULL;
        return z ^ ( z >> 31 );
    }

    /** 0 .. bound-1 */
    UInt64 Below( UInt64 bound )
    {
        return 0 == bound ? 0 : Next() % bound;
    }

    private:

    UInt64 m_state;
};

/*-------------------------------------------------------------------------*/

}; /* namespace TestDecimalReference */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTDECIMALREFERENCE_H ? */
