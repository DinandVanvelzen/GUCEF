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

#ifndef GUCEF_MATH_CTFIXEDPOINT_H
#define GUCEF_MATH_CTFIXEDPOINT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#ifndef GUCEF_CORE_CINT128_H
#include "gucefCORE_CInt128.h"
#define GUCEF_CORE_CINT128_H
#endif /* GUCEF_CORE_CINT128_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_MATH_NUMERICROUNDINGFUNCTIONS_H
#include "gucefMATH_NumericRoundingFunctions.h"
#define GUCEF_MATH_NUMERICROUNDINGFUNCTIONS_H
#endif /* GUCEF_MATH_NUMERICROUNDINGFUNCTIONS_H ? */

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
 *  Integer kernels of CTFixedPoint per storage type. Int32 storage computes in 64 bit words, Int64 storage in
 *  CInt128 / CUInt128. Every function works on magnitudes and applies the sign afterwards, so results are symmetric
 *  under negation, and uses no floating point: the results are bit-identical on every platform.
 *  The 'wrapped' outputs are the two's complement low bits of the exact rounded result; the return value tells
 *  whether that result fits the storage.
 */
template < typename TStorage >
class CTFixedPointKernels;

/*-------------------------------------------------------------------------*/

template <>
class CTFixedPointKernels< Int32 >
{
    public:

    typedef UInt32 TUnsigned;
    typedef Float32 TFloat;
    enum { StorageBits = 32 };

    /** The two's complement value of the low 32 bits */
    static GUCEF_MATH_FORCE_INLINE Int32 FromBits( UInt64 bits ) GUCEF_MATH_REALTIME_CONTEXT { UInt32 low = (UInt32) bits; Int32 value; memcpy( &value, &low, sizeof( value ) ); return value; }

    static bool Multiply( Int32 a, Int32 b, UInt32 fractionBits, TRoundingMode mode, Int32& wrapped, bool& inexact ) GUCEF_MATH_REALTIME_CONTEXT
    {
        Int64 product = (Int64) a * (Int64) b;
        bool negative = product < 0;
        UInt64 magnitude = MathMagnitude( product );
        UInt64 kept = magnitude >> fractionBits;
        TMathRemainderClass remainderClass = MathClassifyLowBits( magnitude, fractionBits, false );
        inexact = MATHREMAINDER_ZERO != remainderClass;
        if ( MathRoundingIncrementsMagnitude( mode, negative, 0 != ( kept & 1 ), remainderClass ) )
            ++kept;
        wrapped = FromBits( negative ? 0 - kept : kept );
        return kept <= ( negative ? (UInt64) 0x80000000u : (UInt64) 0x7FFFFFFFu );
    }

    /** b must not be 0 */
    static bool Divide( Int32 a, Int32 b, UInt32 fractionBits, TRoundingMode mode, Int32& wrapped, bool& inexact ) GUCEF_MATH_REALTIME_CONTEXT
    {
        bool negative = ( a < 0 ) != ( b < 0 ) && 0 != a;
        UInt64 numerator = MathMagnitude( (Int64) a ) << fractionBits;
        UInt64 divisor = MathMagnitude( (Int64) b );
        UInt64 kept = numerator / divisor;
        TMathRemainderClass remainderClass = MathClassifyRemainder( numerator % divisor, divisor );
        inexact = MATHREMAINDER_ZERO != remainderClass;
        if ( MathRoundingIncrementsMagnitude( mode, negative, 0 != ( kept & 1 ), remainderClass ) )
            ++kept;
        wrapped = FromBits( negative ? 0 - kept : kept );
        return kept <= ( negative ? (UInt64) 0x80000000u : (UInt64) 0x7FFFFFFFu );
    }

    /** Correctly rounded square root of raw * 2^fractionBits, raw >= 0 */
    static Int32 Sqrt( Int32 raw, UInt32 fractionBits ) GUCEF_MATH_REALTIME_CONTEXT
    {
        UInt64 value = (UInt64) raw << fractionBits;
        UInt64 root = 0;
        UInt64 bit = (UInt64) 1 << 62;
        while ( bit > value )
            bit >>= 2;
        while ( 0 != bit )
        {
            if ( value >= root + bit )
            {
                value -= root + bit;
                root = ( root >> 1 ) + bit;
            }
            else
            {
                root >>= 1;
            }
            bit >>= 2;
        }
        // value is now the remainder x - root^2: round to nearest (ties cannot occur for integer radicands)
        if ( value > root )
            ++root;
        return (Int32) root;
    }
};

/*-------------------------------------------------------------------------*/

template <>
class CTFixedPointKernels< Int64 >
{
    public:

    typedef UInt64 TUnsigned;
    typedef Float64 TFloat;
    enum { StorageBits = 64 };

    static GUCEF_MATH_FORCE_INLINE Int64 FromBits( UInt64 bits ) GUCEF_MATH_REALTIME_CONTEXT { Int64 value; memcpy( &value, &bits, sizeof( value ) ); return value; }

    static bool Multiply( Int64 a, Int64 b, UInt32 fractionBits, TRoundingMode mode, Int64& wrapped, bool& inexact ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CORE::CInt128 product = CORE::CInt128::Multiply( a, b );
        bool negative = product.IsNegative();
        CORE::CUInt128 magnitude = product.GetMagnitude();
        CORE::CUInt128 kept = CORE::CUInt128::ShiftRight( magnitude, fractionBits );
        TMathRemainderClass remainderClass = MathClassifyLowBits( magnitude.low, fractionBits, false );
        inexact = MATHREMAINDER_ZERO != remainderClass;
        if ( MathRoundingIncrementsMagnitude( mode, negative, 0 != ( kept.low & 1 ), remainderClass ) )
            kept = CORE::CUInt128::Add( kept, CORE::CUInt128( (UInt64) 1 ) );
        wrapped = FromBits( negative ? 0 - kept.low : kept.low );
        return 0 == kept.high && kept.low <= ( negative ? 0x8000000000000000ULL : 0x7FFFFFFFFFFFFFFFULL );
    }

    /** b must not be 0 */
    static bool Divide( Int64 a, Int64 b, UInt32 fractionBits, TRoundingMode mode, Int64& wrapped, bool& inexact ) GUCEF_MATH_REALTIME_CONTEXT
    {
        bool negative = ( a < 0 ) != ( b < 0 ) && 0 != a;
        CORE::CUInt128 numerator = CORE::CUInt128::ShiftLeft( CORE::CUInt128( MathMagnitude( a ) ), fractionBits );
        UInt64 divisor = MathMagnitude( b );
        CORE::CUInt128 kept;
        UInt64 remainder = 0;
        CORE::CUInt128::DivMod( numerator, divisor, kept, remainder );
        TMathRemainderClass remainderClass = MathClassifyRemainder( remainder, divisor );
        inexact = MATHREMAINDER_ZERO != remainderClass;
        if ( MathRoundingIncrementsMagnitude( mode, negative, 0 != ( kept.low & 1 ), remainderClass ) )
            kept = CORE::CUInt128::Add( kept, CORE::CUInt128( (UInt64) 1 ) );
        wrapped = FromBits( negative ? 0 - kept.low : kept.low );
        return 0 == kept.high && kept.low <= ( negative ? 0x8000000000000000ULL : 0x7FFFFFFFFFFFFFFFULL );
    }

    /** Correctly rounded square root of raw * 2^fractionBits, raw >= 0; restoring method on 128 bits */
    static Int64 Sqrt( Int64 raw, UInt32 fractionBits ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CORE::CUInt128 value = CORE::CUInt128::ShiftLeft( CORE::CUInt128( (UInt64) raw ), fractionBits );
        CORE::CUInt128 root;
        UInt32 bitIndex = 126;
        while ( bitIndex > 0 && CORE::CUInt128::Compare( CORE::CUInt128::ShiftLeft( CORE::CUInt128( (UInt64) 1 ), bitIndex ), value ) > 0 )
            bitIndex -= 2;
        CORE::CUInt128 bit = CORE::CUInt128::ShiftLeft( CORE::CUInt128( (UInt64) 1 ), bitIndex );
        if ( CORE::CUInt128::Compare( bit, value ) > 0 )
            bit = CORE::CUInt128();
        while ( !bit.IsZero() )
        {
            CORE::CUInt128 trial = CORE::CUInt128::Add( root, bit );
            if ( CORE::CUInt128::Compare( value, trial ) >= 0 )
            {
                value = CORE::CUInt128::Subtract( value, trial );
                root = CORE::CUInt128::Add( CORE::CUInt128::ShiftRight( root, 1 ), bit );
            }
            else
            {
                root = CORE::CUInt128::ShiftRight( root, 1 );
            }
            bit = CORE::CUInt128::ShiftRight( bit, 2 );
        }
        if ( CORE::CUInt128::Compare( value, root ) > 0 )
            root = CORE::CUInt128::Add( root, CORE::CUInt128( (UInt64) 1 ) );
        return (Int64) root.low;
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  The integer types that CTFixedPoint accepts implicitly: integers of at most 32 bits whose every value is an Int32.
 *  The primary template rejects everything else (floats, Int64, UInt32, UInt64) through the static assertion of the
 *  constructor; ToInt32 still compiles for them so that only that assertion is reported.
 */
template < typename T > class CTFixedPointImplicitSource
{
    public:
    enum { IsAllowed = 0 };
    static GUCEF_MATH_FORCE_INLINE Int32 ToInt32( const T& /*value*/ ) GUCEF_MATH_REALTIME_CONTEXT { return 0; }
};
template < typename T > class CTFixedPointImplicitSourceAllowed
{
    public:
    enum { IsAllowed = 1 };
    static GUCEF_MATH_FORCE_INLINE Int32 ToInt32( const T& value ) GUCEF_MATH_REALTIME_CONTEXT { return (Int32) value; }
};
template <> class CTFixedPointImplicitSource< Int8 > : public CTFixedPointImplicitSourceAllowed< Int8 > {};
template <> class CTFixedPointImplicitSource< Int16 > : public CTFixedPointImplicitSourceAllowed< Int16 > {};
template <> class CTFixedPointImplicitSource< Int32 > : public CTFixedPointImplicitSourceAllowed< Int32 > {};
template <> class CTFixedPointImplicitSource< UInt8 > : public CTFixedPointImplicitSourceAllowed< UInt8 > {};
template <> class CTFixedPointImplicitSource< UInt16 > : public CTFixedPointImplicitSourceAllowed< UInt16 > {};

/*-------------------------------------------------------------------------*/

/**
 *  Binary fixed point number: value = raw / 2^FractionBits, stored in exactly sizeof( TStorage ) bytes.
 *  TStorage is Int32 or Int64; FractionBits is less than the storage width minus one.
 *
 *  Arithmetic (no floating point anywhere, bit-identical on every platform):
 *  - + and - wrap in two's complement (debug builds assert on overflow).
 *  - * forms the exact product in a wider type and rounds it to nearest, ties away from zero, on the magnitude, so
 *    ( -a ) * b == -( a * b ). / rounds the exact quotient the same way. Results that do not fit wrap.
 *  - Division by zero gives the saturated value with the sign of the dividend (0 for 0 / 0) and asserts in debug.
 *  - The named operations Add, Subtract, Multiply, Divide take a rounding mode and report OVERFLOW (the output is
 *    then saturated), DIVISION_BY_ZERO (saturated as above) and INEXACT.
 *
 *  Construction from integers of at most 32 bits is implicit and exact (so x * 2 and x + 1 read naturally; out of range
 *  asserts in debug and wraps), from other integers and from floats only explicitly (nearest-even, saturating) or through FromFloat64 with a mode.
 *  Square root, trigonometric and exponential functions are in gucefMATH_FixedPointFunctions.h.
 */
template < typename TStorage, UInt32 FractionBits >
class GUCEF_MATH_TEMPLATE_PUBLIC CTFixedPoint
{
    public:

    typedef TStorage TStorageType;
    typedef CTFixedPointKernels< TStorage > TKernels;

    enum
    {
        FRACTION_BITS = FractionBits,
        STORAGE_BITS = TKernels::StorageBits
    };

    GUCEF_MATH_FORCE_INLINE CTFixedPoint( void ) GUCEF_MATH_REALTIME_CONTEXT
        : m_raw( 0 )
    {
    }

    /**
     *  Implicit construction from an integer of at most 32 bits (Int8, Int16, Int32, UInt8, UInt16), so that x * 2 and
     *  x + 1 read naturally. Exact for values within range; out of range wraps and asserts in debug builds.
     *  Every other type is a compile error (static assertion
     *  CTFixedPoint_implicit_construction_requires_an_integer_of_at_most_32_bits): floats convert only through the
     *  explicit Float32 / Float64 constructors or FromFloat64, wider integers through FromInt64 of the traits, so
     *  x * 0.5 and TFixed32 y = 2.75 do not compile instead of truncating.
     */
    template < typename TInteger >
    GUCEF_MATH_FORCE_INLINE CTFixedPoint( TInteger value ) GUCEF_MATH_REALTIME_CONTEXT
        : m_raw( TKernels::FromBits( (UInt64) (Int64) CTFixedPointImplicitSource< TInteger >::ToInt32( value ) << FractionBits ) )
    {
        GUCEF_MATH_STATIC_ASSERT( CTFixedPointImplicitSource< TInteger >::IsAllowed, CTFixedPoint_implicit_construction_requires_an_integer_of_at_most_32_bits );
        GUCEF_MATH_STATIC_ASSERT( sizeof( TStorage ) == 4 || sizeof( TStorage ) == 8, CTFixedPoint_storage_must_be_Int32_or_Int64 );
        GUCEF_MATH_STATIC_ASSERT( FractionBits < (UInt32) TKernels::StorageBits - 1, CTFixedPoint_fraction_bits_must_be_less_than_storage_bits_minus_one );
        GUCEF_MATH_ASSERT( (Int64) value >= 0 ? (UInt64) (Int64) value < ( (UInt64) 1 << ( STORAGE_BITS - 1 - FractionBits ) )
                                              : MathMagnitude( (Int64) value ) <= ( (UInt64) 1 << ( STORAGE_BITS - 1 - FractionBits ) ) );
    }

    /** Nearest representable value (ties to even); saturates out of range; NaN gives 0 */
    explicit CTFixedPoint( Float32 value ) GUCEF_MATH_REALTIME_CONTEXT
        : m_raw( 0 )
    {
        FromFloat64( (Float64) value, CORE::ROUNDINGMODE_HALF_EVEN, *this );
    }

    /** Nearest representable value (ties to even); saturates out of range; NaN gives 0 */
    explicit CTFixedPoint( Float64 value ) GUCEF_MATH_REALTIME_CONTEXT
        : m_raw( 0 )
    {
        FromFloat64( value, CORE::ROUNDINGMODE_HALF_EVEN, *this );
    }

    static GUCEF_MATH_FORCE_INLINE CTFixedPoint FromRaw( TStorage raw ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTFixedPoint result;
        result.m_raw = raw;
        return result;
    }

    GUCEF_MATH_FORCE_INLINE TStorage GetRaw( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return m_raw;
    }

    static GUCEF_MATH_FORCE_INLINE CTFixedPoint Lowest( void ) GUCEF_MATH_REALTIME_CONTEXT { return FromRaw( (TStorage) MathApplySign( true, (UInt64) 1 << ( STORAGE_BITS - 1 ) ) ); }
    static GUCEF_MATH_FORCE_INLINE CTFixedPoint Highest( void ) GUCEF_MATH_REALTIME_CONTEXT { return FromRaw( (TStorage) ( ( (UInt64) 1 << ( STORAGE_BITS - 1 ) ) - 1 ) ); }

    /** Exact value as parts: raw * 2^-FractionBits */
    GUCEF_MATH_FORCE_INLINE TNumericValueParts ToParts( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return MathMakeValueParts( m_raw < 0, MathMagnitude( (Int64) m_raw ), -(Int32) FractionBits, 1 );
    }

    /**
     *  One rounding of an exact value with 'mode'. OVERFLOW saturates, NaN / infinity give 0 with INVALID_OPERAND,
     *  ROUNDINGMODE_UNNECESSARY with an inexact result (ROUNDING_REQUIRED) and an invalid mode leave 'out' unchanged.
     */
    static TMathStatus FromParts( const TNumericValueParts& parts, TRoundingMode mode, CTFixedPoint& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !MathIsValidRoundingMode( mode ) )
            return MATHSTATUS_INVALID_OPERAND;
        Int64 raw = 0;
        TMathStatus status = MathRoundPartsToSigned( parts, (Int32) FractionBits, STORAGE_BITS, mode, raw );
        if ( 0 == ( status & MATHSTATUS_ROUNDING_REQUIRED ) )
            out.m_raw = (TStorage) raw;
        return status;
    }

    static TMathStatus FromFloat64( Float64 value, TRoundingMode mode, CTFixedPoint& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return FromParts( MathFloat64ToParts( value ), mode, out );
    }

    /** raw * 2^-FractionBits: exact for Int32 storage, nearest (ties to even) for Int64 storage */
    GUCEF_MATH_FORCE_INLINE Float64 ToFloat64( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return ldexp( (Float64) m_raw, -(int) FractionBits );
    }

    /** Correctly rounded to nearest (ties to even) */
    Float32 ToFloat32( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        Float32 result = 0.0f;
        MathRoundPartsToFloat( ToParts(), CORE::ROUNDINGMODE_HALF_EVEN, result );
        return result;
    }

    /*---- operators ----*/

    GUCEF_MATH_FORCE_INLINE CTFixedPoint operator-( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return FromRaw( TKernels::FromBits( (UInt64) 0 - (UInt64) (Int64) m_raw ) );
    }

    GUCEF_MATH_FORCE_INLINE CTFixedPoint operator+( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return *this;
    }

    friend GUCEF_MATH_FORCE_INLINE CTFixedPoint operator+( const CTFixedPoint& a, const CTFixedPoint& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTFixedPoint result = FromRaw( TKernels::FromBits( (UInt64) (Int64) a.m_raw + (UInt64) (Int64) b.m_raw ) );
        GUCEF_MATH_ASSERT( FitsStorage( (Int64) a.m_raw + (Int64) b.m_raw ) || STORAGE_BITS == 64 );
        GUCEF_MATH_ASSERT( STORAGE_BITS != 64 || !( ( a.m_raw < 0 ) == ( b.m_raw < 0 ) && ( result.m_raw < 0 ) != ( a.m_raw < 0 ) ) );
        return result;
    }

    friend GUCEF_MATH_FORCE_INLINE CTFixedPoint operator-( const CTFixedPoint& a, const CTFixedPoint& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTFixedPoint result = FromRaw( TKernels::FromBits( (UInt64) (Int64) a.m_raw - (UInt64) (Int64) b.m_raw ) );
        GUCEF_MATH_ASSERT( FitsStorage( (Int64) a.m_raw - (Int64) b.m_raw ) || STORAGE_BITS == 64 );
        GUCEF_MATH_ASSERT( STORAGE_BITS != 64 || !( ( a.m_raw < 0 ) != ( b.m_raw < 0 ) && ( result.m_raw < 0 ) != ( a.m_raw < 0 ) ) );
        return result;
    }

    /** Exact product rounded to nearest, ties away from zero; wraps when the result does not fit */
    friend GUCEF_MATH_FORCE_INLINE CTFixedPoint operator*( const CTFixedPoint& a, const CTFixedPoint& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        TStorage wrapped = 0;
        bool inexact = false;
        TKernels::Multiply( a.m_raw, b.m_raw, FractionBits, CORE::ROUNDINGMODE_HALF_UP, wrapped, inexact );
        return FromRaw( wrapped );
    }

    /** Exact quotient rounded to nearest, ties away from zero; wraps when the result does not fit; see the class for 0 */
    friend inline CTFixedPoint operator/( const CTFixedPoint& a, const CTFixedPoint& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( 0 != b.m_raw );
        if ( 0 == b.m_raw )
            return DivisionByZeroResult( a );
        TStorage wrapped = 0;
        bool inexact = false;
        TKernels::Divide( a.m_raw, b.m_raw, FractionBits, CORE::ROUNDINGMODE_HALF_UP, wrapped, inexact );
        return FromRaw( wrapped );
    }

    GUCEF_MATH_FORCE_INLINE CTFixedPoint& operator+=( const CTFixedPoint& other ) GUCEF_MATH_REALTIME_CONTEXT { *this = *this + other; return *this; }
    GUCEF_MATH_FORCE_INLINE CTFixedPoint& operator-=( const CTFixedPoint& other ) GUCEF_MATH_REALTIME_CONTEXT { *this = *this - other; return *this; }
    GUCEF_MATH_FORCE_INLINE CTFixedPoint& operator*=( const CTFixedPoint& other ) GUCEF_MATH_REALTIME_CONTEXT { *this = *this * other; return *this; }
    inline CTFixedPoint& operator/=( const CTFixedPoint& other ) GUCEF_MATH_REALTIME_CONTEXT { *this = *this / other; return *this; }

    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTFixedPoint& a, const CTFixedPoint& b ) GUCEF_MATH_REALTIME_CONTEXT { return a.m_raw == b.m_raw; }
    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTFixedPoint& a, const CTFixedPoint& b ) GUCEF_MATH_REALTIME_CONTEXT { return a.m_raw != b.m_raw; }
    friend GUCEF_MATH_FORCE_INLINE bool operator<( const CTFixedPoint& a, const CTFixedPoint& b ) GUCEF_MATH_REALTIME_CONTEXT { return a.m_raw < b.m_raw; }
    friend GUCEF_MATH_FORCE_INLINE bool operator<=( const CTFixedPoint& a, const CTFixedPoint& b ) GUCEF_MATH_REALTIME_CONTEXT { return a.m_raw <= b.m_raw; }
    friend GUCEF_MATH_FORCE_INLINE bool operator>( const CTFixedPoint& a, const CTFixedPoint& b ) GUCEF_MATH_REALTIME_CONTEXT { return a.m_raw > b.m_raw; }
    friend GUCEF_MATH_FORCE_INLINE bool operator>=( const CTFixedPoint& a, const CTFixedPoint& b ) GUCEF_MATH_REALTIME_CONTEXT { return a.m_raw >= b.m_raw; }

    /*---- named operations with status ----*/

    /** Exact sum; OVERFLOW saturates. 'mode' is accepted for symmetry (the sum is never rounded) */
    static TMathStatus Add( const CTFixedPoint& a, const CTFixedPoint& b, TRoundingMode mode, CTFixedPoint& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return AddExact( (Int64) a.m_raw, (Int64) b.m_raw, false, mode, out );
    }

    static TMathStatus Subtract( const CTFixedPoint& a, const CTFixedPoint& b, TRoundingMode mode, CTFixedPoint& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return AddExact( (Int64) a.m_raw, (Int64) b.m_raw, true, mode, out );
    }

    /** Exact product rounded once with 'mode'; OVERFLOW saturates; ROUNDING_REQUIRED leaves 'out' unchanged */
    static TMathStatus Multiply( const CTFixedPoint& a, const CTFixedPoint& b, TRoundingMode mode, CTFixedPoint& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !MathIsValidRoundingMode( mode ) )
            return MATHSTATUS_INVALID_OPERAND;
        TStorage wrapped = 0;
        bool inexact = false;
        bool fits = TKernels::Multiply( a.m_raw, b.m_raw, FractionBits, mode, wrapped, inexact );
        return Finish( fits, ( a.m_raw < 0 ) != ( b.m_raw < 0 ), wrapped, inexact, mode, out );
    }

    /** Exact quotient rounded once with 'mode'; DIVISION_BY_ZERO and OVERFLOW saturate */
    static TMathStatus Divide( const CTFixedPoint& a, const CTFixedPoint& b, TRoundingMode mode, CTFixedPoint& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !MathIsValidRoundingMode( mode ) )
            return MATHSTATUS_INVALID_OPERAND;
        if ( 0 == b.m_raw )
        {
            out = DivisionByZeroResult( a );
            return MATHSTATUS_DIVISION_BY_ZERO;
        }
        TStorage wrapped = 0;
        bool inexact = false;
        bool fits = TKernels::Divide( a.m_raw, b.m_raw, FractionBits, mode, wrapped, inexact );
        return Finish( fits, ( a.m_raw < 0 ) != ( b.m_raw < 0 ), wrapped, inexact, mode, out );
    }

    /** Multiply with ROUNDINGMODE_HALF_UP, the rounding of operator* */
    static GUCEF_MATH_FORCE_INLINE TMathStatus MultiplyChecked( const CTFixedPoint& a, const CTFixedPoint& b, CTFixedPoint& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return Multiply( a, b, CORE::ROUNDINGMODE_HALF_UP, out );
    }

    /** Divide with ROUNDINGMODE_HALF_UP, the rounding of operator/ */
    static GUCEF_MATH_FORCE_INLINE TMathStatus DivideChecked( const CTFixedPoint& a, const CTFixedPoint& b, CTFixedPoint& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return Divide( a, b, CORE::ROUNDINGMODE_HALF_UP, out );
    }

    /** a * b as operator* but saturating instead of wrapping */
    static GUCEF_MATH_FORCE_INLINE CTFixedPoint MultiplySaturated( const CTFixedPoint& a, const CTFixedPoint& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTFixedPoint result;
        Multiply( a, b, CORE::ROUNDINGMODE_HALF_UP, result );
        return result;
    }

    /** a / b as operator/ but saturating instead of wrapping; division by zero saturates without an assertion */
    static GUCEF_MATH_FORCE_INLINE CTFixedPoint DivideSaturated( const CTFixedPoint& a, const CTFixedPoint& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTFixedPoint result;
        Divide( a, b, CORE::ROUNDINGMODE_HALF_UP, result );
        return result;
    }

    /*---- text ----*/

    /** The exact decimal value (2^-F has a finite decimal expansion), without trailing zeros: "-1.5", "3", "0.0000152587890625" */
    CORE::CString ToString( void ) const
    {
        char buffer[ 96 ];
        UInt32 length = 0;
        UInt64 magnitude = MathMagnitude( (Int64) m_raw );
        if ( m_raw < 0 )
            buffer[ length++ ] = '-';
        UInt64 whole = FractionBits >= 64 ? 0 : ( magnitude >> FractionBits );
        UInt64 fraction = magnitude & ( ( (UInt64) 1 << FractionBits ) - 1 );

        char digits[ 24 ];
        UInt32 digitCount = 0;
        do
        {
            digits[ digitCount++ ] = (char) ( '0' + ( whole % 10 ) );
            whole /= 10;
        }
        while ( 0 != whole );
        while ( digitCount > 0 )
            buffer[ length++ ] = digits[ --digitCount ];

        if ( 0 != fraction )
        {
            buffer[ length++ ] = '.';
            while ( 0 != fraction && length < sizeof( buffer ) - 1 )
            {
                CORE::CUInt128 product = CORE::CUInt128::Multiply( fraction, 10 );
                buffer[ length++ ] = (char) ( '0' + CORE::CUInt128::ShiftRight( product, FractionBits ).low );
                fraction = product.low & ( ( (UInt64) 1 << FractionBits ) - 1 );
            }
        }
        buffer[ length ] = '\0';
        return CORE::CString( buffer );
    }

    /**
     *  Parses "[+|-]digits[.digits]" (locale independent, no exponent) and rounds the exact decimal value to nearest,
     *  ties to even. Returns false for malformed text and for a value out of range; the object is then unchanged.
     *  Exact round trip with ToString.
     */
    bool FromString( const CORE::CString& text )
    {
        const char* data = text.C_String();
        UInt32 length = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
        if ( GUCEF_NULL == data || 0 == length )
            return false;
        UInt32 i = 0;
        bool negative = false;
        if ( '+' == data[ 0 ] || '-' == data[ 0 ] )
        {
            negative = '-' == data[ 0 ];
            ++i;
        }
        UInt64 whole = 0;
        UInt32 wholeDigits = 0;
        for ( ; i<length && data[ i ] >= '0' && data[ i ] <= '9'; ++i, ++wholeDigits )
        {
            UInt64 digit = (UInt64) ( data[ i ] - '0' );
            if ( whole > ( ~(UInt64) 0 - digit ) / 10 )
                return false;
            whole = whole * 10 + digit;
        }

        // the fraction digits are converted to binary exactly by repeated doubling of the decimal digit string
        char fractionDigits[ 80 ];
        UInt32 fractionCount = 0;
        if ( i < length && '.' == data[ i ] )
        {
            ++i;
            for ( ; i<length && data[ i ] >= '0' && data[ i ] <= '9'; ++i )
            {
                if ( fractionCount >= sizeof( fractionDigits ) )
                    return false;
                fractionDigits[ fractionCount++ ] = (char) ( data[ i ] - '0' );
            }
            if ( 0 == fractionCount )
                return false;
        }
        if ( i != length || ( 0 == wholeDigits && 0 == fractionCount ) )
            return false;

        UInt64 fractionBitsValue = 0;
        for ( UInt32 bit=0; bit<FractionBits + 1; ++bit )
        {
            UInt32 carry = 0;
            for ( UInt32 d=fractionCount; d>0; --d )
            {
                UInt32 doubled = (UInt32) fractionDigits[ d - 1 ] * 2 + carry;
                fractionDigits[ d - 1 ] = (char) ( doubled % 10 );
                carry = doubled / 10;
            }
            fractionBitsValue = ( fractionBitsValue << 1 ) | carry;
        }
        bool sticky = false;
        for ( UInt32 d=0; d<fractionCount; ++d )
            if ( 0 != fractionDigits[ d ] )
                sticky = true;

        // fractionBitsValue holds FractionBits + 1 bits: the last one is the half bit
        bool halfBit = 0 != ( fractionBitsValue & 1 );
        UInt64 fractionRaw = fractionBitsValue >> 1;
        TMathRemainderClass remainderClass = !halfBit ? ( sticky ? MATHREMAINDER_BELOW_HALF : MATHREMAINDER_ZERO )
                                                      : ( sticky ? MATHREMAINDER_ABOVE_HALF : MATHREMAINDER_HALF );
        if ( FractionBits > 0 && whole > ( ~(UInt64) 0 >> FractionBits ) )
            return false;
        const UInt64 limit = negative ? (UInt64) 1 << ( STORAGE_BITS - 1 ) : ( (UInt64) 1 << ( STORAGE_BITS - 1 ) ) - 1;
        UInt64 magnitude = ( whole << FractionBits ) | fractionRaw;
        if ( magnitude > limit )
            return false;           // checked before the increment, which could otherwise wrap 2^64 - 1 to 0
        if ( MathRoundingIncrementsMagnitude( CORE::ROUNDINGMODE_HALF_EVEN, negative, 0 != ( magnitude & 1 ), remainderClass ) )
            ++magnitude;
        if ( magnitude > limit )
            return false;
        m_raw = (TStorage) MathApplySign( negative, magnitude );
        return true;
    }

    private:

    static GUCEF_MATH_FORCE_INLINE bool FitsStorage( Int64 value ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return STORAGE_BITS == 64 || ( value >= -(Int64) 0x80000000LL && value <= (Int64) 0x7FFFFFFFLL );
    }

    static GUCEF_MATH_FORCE_INLINE CTFixedPoint DivisionByZeroResult( const CTFixedPoint& dividend ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( 0 == dividend.m_raw )
            return CTFixedPoint();
        return dividend.m_raw < 0 ? Lowest() : Highest();
    }

    static TMathStatus Finish( bool fits, bool negative, TStorage wrapped, bool inexact, TRoundingMode mode, CTFixedPoint& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !fits )
        {
            out = negative ? Lowest() : Highest();
            return MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT;
        }
        if ( inexact && CORE::ROUNDINGMODE_UNNECESSARY == mode )
            return MATHSTATUS_ROUNDING_REQUIRED;
        out.m_raw = wrapped;
        return inexact ? MATHSTATUS_INEXACT : MATHSTATUS_OK;
    }

    static TMathStatus AddExact( Int64 a, Int64 b, bool subtract, TRoundingMode mode, CTFixedPoint& out ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !MathIsValidRoundingMode( mode ) )
            return MATHSTATUS_INVALID_OPERAND;
        UInt64 bits = subtract ? (UInt64) a - (UInt64) b : (UInt64) a + (UInt64) b;
        bool overflow = false;
        if ( STORAGE_BITS == 64 )
        {
            // the true result has the sign of a whenever the wrapped one is wrong
            bool resultNegative = 0 != ( bits >> 63 );
            bool sameSigns = ( a < 0 ) == ( b < 0 );
            overflow = ( subtract ? !sameSigns : sameSigns ) && resultNegative != ( a < 0 );
        }
        else
        {
            overflow = !FitsStorage( subtract ? a - b : a + b );
        }
        if ( overflow )
        {
            out = a < 0 ? Lowest() : Highest();
            if ( STORAGE_BITS != 64 )
                out = ( subtract ? a - b : a + b ) < 0 ? Lowest() : Highest();
            return MATHSTATUS_OVERFLOW | MATHSTATUS_INEXACT;
        }
        out.m_raw = TKernels::FromBits( bits );
        return MATHSTATUS_OK;
    }

    TStorage m_raw;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* The predefined formats are instantiated in the module (gucefMATH_CTFixedPoint.cpp), section 5.10 */
#define GUCEF_MATH_FIXEDPOINT_COMMA ,
GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTFixedPoint< Int32 GUCEF_MATH_FIXEDPOINT_COMMA 16 > )
GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTFixedPoint< Int64 GUCEF_MATH_FIXEDPOINT_COMMA 32 > )
GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTFixedPoint< Int64 GUCEF_MATH_FIXEDPOINT_COMMA 16 > )
#undef GUCEF_MATH_FIXEDPOINT_COMMA

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTFIXEDPOINT_H ? */
