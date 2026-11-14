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

#ifndef GUCEF_CORE_CBIGINTEGER_H
#define GUCEF_CORE_CBIGINTEGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdexcept>
#include <new>

#ifndef GUCEF_TYPES_H
#include "gucef_types.h"
#define GUCEF_TYPES_H
#endif /* GUCEF_TYPES_H ? */

#ifndef GUCEF_INVALID_ARG_EXCEPTION_H
#include "gucef_invalid_arg_exception.h"
#define GUCEF_INVALID_ARG_EXCEPTION_H
#endif /* GUCEF_INVALID_ARG_EXCEPTION_H ? */

#ifndef GUCEF_SFINAE_H
#include "gucef_SFINAE.h"
#define GUCEF_SFINAE_H
#endif /* GUCEF_SFINAE_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_ROUNDINGMODE_H
#include "gucefCORE_RoundingMode.h"
#define GUCEF_CORE_ROUNDINGMODE_H
#endif /* GUCEF_CORE_ROUNDINGMODE_H ? */

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

#ifndef GUCEF_CORE_CUINT128_H
#include "gucefCORE_CUInt128.h"
#define GUCEF_CORE_CUINT128_H
#endif /* GUCEF_CORE_CUINT128_H ? */

#ifndef GUCEF_CORE_CINT128_H
#include "gucefCORE_CInt128.h"
#define GUCEF_CORE_CINT128_H
#endif /* GUCEF_CORE_CINT128_H ? */

#ifndef GUCEF_CORE_CTFRACTIONINTEGEROPS_H
#include "gucefCORE_CTFractionIntegerOps.h"
#define GUCEF_CORE_CTFRACTIONINTEGEROPS_H
#endif /* GUCEF_CORE_CTFRACTIONINTEGEROPS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Limb count of the smaller operand from which multiplication switches from schoolbook to Karatsuba */
#ifndef GUCEF_CORE_BIGINTEGER_KARATSUBA_THRESHOLD
  #define GUCEF_CORE_BIGINTEGER_KARATSUBA_THRESHOLD     32
#endif

/** Result limb count from which FromString switches from Horner accumulation to divide and conquer */
#ifndef GUCEF_CORE_BIGINTEGER_FROMSTRING_DC_THRESHOLD
  #define GUCEF_CORE_BIGINTEGER_FROMSTRING_DC_THRESHOLD 64
#endif

/**
 *  Limb count from which ToString would switch to divide and conquer. 0 = never: that conversion only pays off with
 *  a sub-quadratic division, which is not part of this version (the constant exists so enabling it touches one place)
 */
#ifndef GUCEF_CORE_BIGINTEGER_TOSTRING_DC_THRESHOLD
  #define GUCEF_CORE_BIGINTEGER_TOSTRING_DC_THRESHOLD   0
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

class CBigUnsigned;
class CBigInteger;
template < typename NumeratorType, typename DenominatorType > class CTFraction;

/*-------------------------------------------------------------------------*/

/**
 *  Outcome of a big integer operation. The values are bits so the statuses of a sequence can be OR'ed together.
 *  The bits shared with EDecimalStatus have identical values, so conversions that involve CDecimal can OR a decimal
 *  status in without mapping.
 */
enum EBigIntegerStatus
{
    BIGINTEGERSTATUS_OK                 = 0x00,
    BIGINTEGERSTATUS_INEXACT            = 0x01,     /**< informational: the result was rounded */
    BIGINTEGERSTATUS_OVERFLOW           = 0x02,     /**< a narrowing conversion does not fit the target, or the result would exceed MaxLimbCount */
    BIGINTEGERSTATUS_DIVISION_BY_ZERO   = 0x04,     /**< also a modulus of 0 */
    BIGINTEGERSTATUS_INVALID_SCALE      = 0x08,     /**< a decimal scale > CDecimal::MaxScale */
    BIGINTEGERSTATUS_INVALID_FORMAT     = 0x10,     /**< text does not match the grammar */
    BIGINTEGERSTATUS_INVALID_OPERAND    = 0x20,     /**< NaN/Inf, base outside 2..36, negative exponent or modulus, ISqrt or PopCount of a negative, unknown rounding mode, no modular inverse */
    BIGINTEGERSTATUS_ROUNDING_REQUIRED  = 0x40,     /**< ROUNDINGMODE_UNNECESSARY and the result is not exact */
    BIGINTEGERSTATUS_OUT_OF_MEMORY      = 0x80      /**< the allocator failed */
};
typedef UInt32 TBigIntegerStatus;   /**< bit set of EBigIntegerStatus */

/*-------------------------------------------------------------------------*/

/** True when the status holds any bit other than the informational BIGINTEGERSTATUS_INEXACT */
inline bool BigIntegerStatusIsError( TBigIntegerStatus status ) { return 0 != ( status & ~( (TBigIntegerStatus) BIGINTEGERSTATUS_INEXACT ) ); }

/*-------------------------------------------------------------------------*/

/** Parse flags, OR'ed together */
enum EBigIntegerParseFlags
{
    BIGINTEGERPARSE_STRICT                          = 0x00,     /**< [ "+" | "-" ] digits, nothing else */
    BIGINTEGERPARSE_ALLOW_BASE_PREFIX               = 0x01,     /**< 0x, 0b, 0o after the sign. Base 0 detects the base from the prefix (no prefix: 10), otherwise only the prefix of the given base is recognised */
    BIGINTEGERPARSE_ALLOW_SURROUNDING_WHITESPACE    = 0x02,     /**< ASCII spaces and tabs before and after the number */
    BIGINTEGERPARSE_ALLOW_DIGIT_SEPARATORS          = 0x04      /**< a single '_' between two digits */
};

/*-------------------------------------------------------------------------*/

/**
 *  Which built-in types convert implicitly to CBigInteger: every signed and unsigned integer type, but not bool and
 *  not the character types. 'value' is 1 for those types, 'isUnsigned' tells how the value is widened.
 */
template < typename T > struct TypeIsBuiltinInteger { enum { value = 0, isUnsigned = 0 }; };
template <> struct TypeIsBuiltinInteger< signed char > { enum { value = 1, isUnsigned = 0 }; };
template <> struct TypeIsBuiltinInteger< unsigned char > { enum { value = 1, isUnsigned = 1 }; };
template <> struct TypeIsBuiltinInteger< short > { enum { value = 1, isUnsigned = 0 }; };
template <> struct TypeIsBuiltinInteger< unsigned short > { enum { value = 1, isUnsigned = 1 }; };
template <> struct TypeIsBuiltinInteger< int > { enum { value = 1, isUnsigned = 0 }; };
template <> struct TypeIsBuiltinInteger< unsigned int > { enum { value = 1, isUnsigned = 1 }; };
template <> struct TypeIsBuiltinInteger< long > { enum { value = 1, isUnsigned = 0 }; };
template <> struct TypeIsBuiltinInteger< unsigned long > { enum { value = 1, isUnsigned = 1 }; };
#if defined( _MSC_VER ) || defined( __GNUC__ )
template <> struct TypeIsBuiltinInteger< long long > { enum { value = 1, isUnsigned = 0 }; };
template <> struct TypeIsBuiltinInteger< unsigned long long > { enum { value = 1, isUnsigned = 1 }; };
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Arbitrary precision signed integer.
 *
 *  Representation and rules:
 *  - Sign and magnitude. The magnitude is held as 32 bit limbs, least significant first, without leading zero limbs.
 *    Zero has no limbs and is never negative.
 *  - Values up to InlineLimbCount limbs (256 bits) live inside the object: no heap use for every Int64, CInt128 or
 *    CTFraction< Int64 > sized value. Larger values use one heap block from gucef_allocator.
 *  - Named operations return a TBigIntegerStatus and never throw. On an error status (BigIntegerStatusIsError) the
 *    result argument is left unchanged. Results may alias *this or any operand. A failed allocation is reported as
 *    BIGINTEGERSTATUS_OUT_OF_MEMORY.
 *  - The only size limit is MaxLimbCount: Pow, ShiftLeft and the multiplications check the predicted size before
 *    allocating and report BIGINTEGERSTATUS_OVERFLOW beyond it.
 *  - Operators exist for convenience. They can only fail by allocation: std::bad_alloc, std::length_error above
 *    MaxLimbCount. '/' and '%' (truncating) throw GUCEF::invalid_argument for a zero divisor. Use the named
 *    operations in numeric code.
 *  - Shifts and bit operations of negative values use two's complement semantics with infinite sign extension.
 *  - Value type, no virtuals, no global mutable state: concurrent const calls on one object are safe.
 *
 *  Complexity (n, m limbs): add, subtract, compare, shifts and bit operations O( n ); multiplication O( n m ) below
 *  KaratsubaThresholdLimbs and O( n^1.585 ) above it; division O( n m ) (Knuth algorithm D); Gcd O( n^2 );
 *  ISqrt O( M( n ) log n ); ToString O( n^2 ) except for the power of two bases (linear); FromString sub-quadratic
 *  above BaseConversionDivideConquerLimbs. Products beyond about 10^5 limbs are slow (no Toom-3 or FFT).
 */
class GUCEF_CORE_PUBLIC_CPP CBigInteger
{
    public:

    static const UInt32 InlineLimbCount = 8;                                                            /**< 256 bits inside the object */
    static const UInt32 MaxLimbCount = 0x07FFFFFF;                                                      /**< representable limit: the bit length fits UInt32 */
    static const UInt32 KaratsubaThresholdLimbs = GUCEF_CORE_BIGINTEGER_KARATSUBA_THRESHOLD;
    static const UInt32 BaseConversionDivideConquerLimbs = GUCEF_CORE_BIGINTEGER_FROMSTRING_DC_THRESHOLD;
    static const UInt32 ToStringDivideConquerLimbs = GUCEF_CORE_BIGINTEGER_TOSTRING_DC_THRESHOLD;

    /*---------------------------------------------------------------------//
    //  construction                                                       //
    //---------------------------------------------------------------------*/

    CBigInteger( void );                                /**< 0 */
    CBigInteger( const CBigInteger& src );
    ~CBigInteger();
    CBigInteger& operator=( const CBigInteger& src );

    #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED
    CBigInteger( CBigInteger&& src ) GUCEF_NOEXCEPT;    /**< steals the heap block, the source becomes 0 */
    CBigInteger& operator=( CBigInteger&& src ) GUCEF_NOEXCEPT;
    #endif

    /**
     *  Implicit and exact from every built-in integer type (UInt64 values above INT64_MAX included), so x + 1,
     *  x == 0 and CTFraction< CBigInteger, CBigInteger >( 1, 3 ) work. Never allocates.
     */
    template < typename T >
    CBigInteger( T value, typename EnableIf< 0 != TypeIsBuiltinInteger< T >::value, int >::type = 0 );

    void Swap( CBigInteger& other ) GUCEF_NOEXCEPT;

    /** Moves a heap held value that fits InlineLimbCount back into the object, or shrinks the heap block to the size */
    TBigIntegerStatus Compact( void );

    static CBigInteger Zero( void );
    static CBigInteger One( void );

    static TBigIntegerStatus FromInt64( Int64 value, CBigInteger& result );
    static TBigIntegerStatus FromUInt64( UInt64 value, CBigInteger& result );
    static TBigIntegerStatus FromCUInt128( const CUInt128& value, CBigInteger& result );
    static TBigIntegerStatus FromCInt128( const CInt128& value, CBigInteger& result );

    /** The exact binary value of 'value' rounded to an integer with 'mode'. NaN and infinities: INVALID_OPERAND, -0.0 gives 0 */
    static TBigIntegerStatus FromFloat64( Float64 value, TRoundingMode mode, CBigInteger& result );
    static TBigIntegerStatus FromFloat32( Float32 value, TRoundingMode mode, CBigInteger& result );

    /** The decimal's value rounded to an integer with 'mode' */
    static TBigIntegerStatus FromDecimal( const CDecimal& value, TRoundingMode mode, CBigInteger& result );

    /**
     *  Parses [ "+" | "-" ] digits in 'base' (2 .. 36, digits 0-9 then a-z or A-Z). See EBigIntegerParseFlags for the
     *  options; base 0 needs BIGINTEGERPARSE_ALLOW_BASE_PREFIX. No digits, a digit >= base, a lone sign or any other
     *  character: INVALID_FORMAT. A base outside 2 .. 36: INVALID_OPERAND. Locale independent. "-0" is 0.
     */
    static TBigIntegerStatus FromString( const CString& text, UInt32 base, CBigInteger& result, UInt32 parseFlags = BIGINTEGERPARSE_STRICT );
    static TBigIntegerStatus FromChars( const char* text, UInt32 length, UInt32 base, CBigInteger& result, UInt32 parseFlags = BIGINTEGERPARSE_STRICT );

    /** Constants in code and tests: base 10, strict grammar. Throws GUCEF::invalid_argument for any error status */
    static CBigInteger Parse( const char* literal );

    /** The magnitude from big endian bytes (leading zero bytes are fine) with the sign passed separately */
    static TBigIntegerStatus FromBigEndianBytes( const void* bytes, UInt32 size, bool negative, CBigInteger& result );

    /** A two's complement big endian value of 'size' bytes (the top bit of the first byte is the sign) */
    static TBigIntegerStatus FromTwosComplementBigEndian( const void* bytes, UInt32 size, CBigInteger& result );

    /** Limb copy */
    static TBigIntegerStatus FromBigUnsigned( const CBigUnsigned& value, CBigInteger& result );

    /** The magnitude from 32 bit limbs, least significant first (leading zero limbs are dropped), and the sign */
    static TBigIntegerStatus FromLimbs( const UInt32* limbs, UInt32 limbCount, bool negative, CBigInteger& result );

    /*---------------------------------------------------------------------//
    //  queries                                                            //
    //---------------------------------------------------------------------*/

    bool IsZero( void ) const { return 0 == m_size; }
    bool IsNegative( void ) const { return m_negative; }
    bool IsPositive( void ) const { return 0 != m_size && !m_negative; }
    Int32 GetSign( void ) const { return 0 == m_size ? 0 : ( m_negative ? -1 : 1 ); }
    bool IsOdd( void ) const { return 0 != m_size && 0 != ( GetLimbs()[ 0 ] & 1 ); }
    bool IsEven( void ) const { return !IsOdd(); }
    bool IsOne( void ) const { return 1 == m_size && !m_negative && 1 == GetLimbs()[ 0 ]; }

    /** The number of 32 bit limbs of the magnitude, 0 for 0 */
    UInt32 GetLimbCount( void ) const { return m_size; }

    /** The 32 bit limbs of the magnitude, least significant first, GetLimbCount() of them (none for 0) */
    const UInt32* GetLimbs( void ) const { return 0 != m_capacity ? m_storage.heapLimbs : m_storage.inlineLimbs; }

    /** Bits of the magnitude up to and including the highest 1 bit, 0 for 0 (Python int.bit_length) */
    UInt32 GetBitLength( void ) const;

    /** The number of trailing zero bits of the magnitude (the same for -x). 0 for 0 as well, check IsZero() */
    UInt32 GetLowestSetBit( void ) const;

    /** The number of 1 bits. Negative values have infinitely many: INVALID_OPERAND */
    TBigIntegerStatus PopCount( UInt32& count ) const;

    /** The number of bytes of the magnitude, 0 for 0 */
    UInt32 GetByteLength( void ) const;

    /** Consistent with operator==: equal values have equal hashes. Independent of the storage (inline or heap) */
    UInt64 GetHash( void ) const;

    bool FitsInt64( void ) const;
    bool FitsUInt64( void ) const;
    bool FitsCUInt128( void ) const;
    bool FitsCInt128( void ) const;

    /** Diagnostic: whether the value is held inside the object */
    bool IsInline( void ) const { return 0 == m_capacity; }

    /*---------------------------------------------------------------------//
    //  arithmetic                                                         //
    //---------------------------------------------------------------------*/

    TBigIntegerStatus Add( const CBigInteger& other, CBigInteger& result ) const;
    TBigIntegerStatus Subtract( const CBigInteger& other, CBigInteger& result ) const;
    TBigIntegerStatus Multiply( const CBigInteger& other, CBigInteger& result ) const;
    TBigIntegerStatus Square( CBigInteger& result ) const;
    TBigIntegerStatus Negate( CBigInteger& result ) const;
    TBigIntegerStatus Abs( CBigInteger& result ) const;

    /** Fast paths for built-in integer operands, exact for every value of the type */
    template < typename T >
    typename EnableIf< 0 != TypeIsBuiltinInteger< T >::value, TBigIntegerStatus >::type Add( T other, CBigInteger& result ) const;
    template < typename T >
    typename EnableIf< 0 != TypeIsBuiltinInteger< T >::value, TBigIntegerStatus >::type Subtract( T other, CBigInteger& result ) const;
    template < typename T >
    typename EnableIf< 0 != TypeIsBuiltinInteger< T >::value, TBigIntegerStatus >::type Multiply( T other, CBigInteger& result ) const;

    /**
     *  Truncating division: the quotient rounds toward zero, the remainder takes the sign of the dividend,
     *  *this == quotient * divisor + remainder and |remainder| < |divisor|. 'quotient' and 'remainder' must be
     *  different objects (INVALID_OPERAND otherwise), either may alias *this or 'divisor'.
     */
    TBigIntegerStatus DivMod( const CBigInteger& divisor, CBigInteger& quotient, CBigInteger& remainder ) const;

    /** Truncating division by a built-in value (single limb fast path) */
    TBigIntegerStatus DivMod( Int64 divisor, CBigInteger& quotient, Int64& remainder ) const;

    /** Floored division: the quotient rounds toward -infinity, the remainder takes the sign of the divisor */
    TBigIntegerStatus DivModFloor( const CBigInteger& divisor, CBigInteger& quotient, CBigInteger& remainder ) const;

    /** Euclidean remainder: the result is in [ 0, |modulus| ) */
    TBigIntegerStatus Modulo( const CBigInteger& modulus, CBigInteger& result ) const;

    /**
     *  The quotient rounded with 'mode' (TOWARD_ZERO = DivMod, FLOOR = DivModFloor, HALF_EVEN, ...; UNNECESSARY
     *  reports ROUNDING_REQUIRED for an inexact quotient). The remainder is *this - quotient * divisor. INEXACT when the
     *  division was not exact.
     */
    TBigIntegerStatus DivideRounded( const CBigInteger& divisor, TRoundingMode mode, CBigInteger& quotient, CBigInteger& remainder ) const;

    /** Non-negative, Gcd( 0, 0 ) = 0 */
    TBigIntegerStatus Gcd( const CBigInteger& other, CBigInteger& result ) const;

    /** Non-negative, 0 when either operand is 0 */
    TBigIntegerStatus Lcm( const CBigInteger& other, CBigInteger& result ) const;

    /** gcd = Gcd( *this, other ) and *this * x + other * y == gcd. The four results must be different objects */
    TBigIntegerStatus ExtendedGcd( const CBigInteger& other, CBigInteger& gcd, CBigInteger& x, CBigInteger& y ) const;

    /** The inverse in [ 0, |modulus| ): *this * result mod |modulus| == 1 mod |modulus|. Not coprime: INVALID_OPERAND, modulus 0: DIVISION_BY_ZERO */
    TBigIntegerStatus ModInverse( const CBigInteger& modulus, CBigInteger& result ) const;

    /** *this ^ exponent, 0^0 = 1 */
    TBigIntegerStatus Pow( UInt32 exponent, CBigInteger& result ) const;

    /** *this ^ exponent mod modulus in [ 0, modulus ). Negative exponent or modulus: INVALID_OPERAND, modulus 0: DIVISION_BY_ZERO */
    TBigIntegerStatus ModPow( const CBigInteger& exponent, const CBigInteger& modulus, CBigInteger& result ) const;

    /** floor( sqrt( *this ) ). Negative: INVALID_OPERAND */
    TBigIntegerStatus ISqrt( CBigInteger& result ) const;

    /** -1, 0, 1 */
    Int32 Compare( const CBigInteger& other ) const;
    Int32 CompareMagnitude( const CBigInteger& other ) const;

    template < typename T >
    typename EnableIf< 0 != TypeIsBuiltinInteger< T >::value, Int32 >::type Compare( T other ) const;

    /*---------------------------------------------------------------------//
    //  shifts and bits (two's complement semantics for negative values)   //
    //---------------------------------------------------------------------*/

    /** *this * 2^bits. OVERFLOW above MaxLimbCount */
    TBigIntegerStatus ShiftLeft( UInt32 bits, CBigInteger& result ) const;

    /** Arithmetic shift: floor( *this / 2^bits ), -5 >> 1 == -3, -1 >> k == -1 */
    TBigIntegerStatus ShiftRight( UInt32 bits, CBigInteger& result ) const;

    /** The magnitude shifted, the sign kept: truncation toward zero, -5 >> 1 == -2 */
    TBigIntegerStatus ShiftRightMagnitude( UInt32 bits, CBigInteger& result ) const;

    TBigIntegerStatus And( const CBigInteger& other, CBigInteger& result ) const;
    TBigIntegerStatus Or( const CBigInteger& other, CBigInteger& result ) const;
    TBigIntegerStatus Xor( const CBigInteger& other, CBigInteger& result ) const;

    /** ~x == -x - 1 */
    TBigIntegerStatus Not( CBigInteger& result ) const;

    /** Bit 'index' of the two's complement form: negative values extend with ones */
    bool TestBit( UInt32 index ) const;
    TBigIntegerStatus SetBit( UInt32 index );
    TBigIntegerStatus ClearBit( UInt32 index );
    TBigIntegerStatus FlipBit( UInt32 index );

    /*---------------------------------------------------------------------//
    //  text                                                               //
    //---------------------------------------------------------------------*/

    /** '-' when negative, then the digits in 'base' (2 .. 36), lower case. Empty for an invalid base */
    CString ToString( UInt32 base = 10 ) const;
    CString ToString( UInt32 base, bool upperCase ) const;
    TBigIntegerStatus ToString( UInt32 base, bool upperCase, CString& text ) const;

    /** The text into 'buffer', null terminated, without allocating the text. Returns the length, 0 (nothing written) when the buffer is too small or the base is invalid */
    UInt32 ToChars( char* buffer, UInt32 bufferSize, UInt32 base = 10, bool upperCase = false ) const;

    /** A buffer size that always suffices for ToChars in 'base' (sign and terminating null included), 0 for an invalid base */
    UInt32 GetTextLengthUpperBound( UInt32 base = 10 ) const;

    /*---------------------------------------------------------------------//
    //  conversions                                                        //
    //---------------------------------------------------------------------*/

    /** OVERFLOW when the value does not fit, the output is then unchanged. INT64_MIN round trips */
    TBigIntegerStatus ToInt64( Int64& value ) const;
    TBigIntegerStatus ToUInt64( UInt64& value ) const;
    TBigIntegerStatus ToCUInt128( CUInt128& value ) const;
    TBigIntegerStatus ToCInt128( CInt128& value ) const;

    /**
     *  The value rounded once to a double with 'mode'. INEXACT when rounded. Beyond the largest finite double after
     *  rounding: the modes that round toward zero for the sign (TOWARD_ZERO, FLOOR for positive, CEILING for negative)
     *  give the largest finite value with INEXACT, the others OVERFLOW
     */
    TBigIntegerStatus ToFloat64( TRoundingMode mode, Float64& value ) const;

    /** Nearest, ties to even, +/-infinity beyond the range */
    Float64 ToFloat64( void ) const;

    /** As ToFloat64 for the float format, rounded once (no double rounding through Float64) */
    TBigIntegerStatus ToFloat32( TRoundingMode mode, Float32& value ) const;
    Float32 ToFloat32( void ) const;

    /** The decimal *this at 'scale': *this * 10^scale must fit the symmetric Int64 range (OVERFLOW). scale > 18: INVALID_SCALE */
    TBigIntegerStatus ToDecimal( UInt8 scale, CDecimal& value ) const;

    /** Limb copy, non-negative values only (INVALID_OPERAND for a negative) */
    TBigIntegerStatus ToBigUnsigned( CBigUnsigned& value ) const;

    /** The magnitude as 'size' big endian bytes, zero padded in front. OVERFLOW (nothing written) when it needs more bytes */
    TBigIntegerStatus ToBigEndianBytes( void* buffer, UInt32 size ) const;

    /** The two's complement form in 'size' big endian bytes. OVERFLOW (nothing written) when it does not fit */
    TBigIntegerStatus ToTwosComplementBigEndian( void* buffer, UInt32 size ) const;

    /** The correctly rounded n / d with one rounding (no double rounding of Float64( n ) / Float64( d )), subnormals included */
    static TBigIntegerStatus RatioToFloat64( const CBigInteger& numerator, const CBigInteger& denominator, TRoundingMode mode, Float64& value );
    static TBigIntegerStatus RatioToFloat32( const CBigInteger& numerator, const CBigInteger& denominator, TRoundingMode mode, Float32& value );

    /** n / d rounded once to the context's scale and rounding mode, statuses as CDecimal::Divide (DECIMALSTATUS_* bits) */
    static TDecimalStatus RatioToDecimal( const CBigInteger& numerator, const CBigInteger& denominator, const CDecimalContext& context, CDecimal& value );

    /*---------------------------------------------------------------------//
    //  operators                                                          //
    //---------------------------------------------------------------------*/

    CBigInteger operator-( void ) const;
    CBigInteger operator~( void ) const;

    CBigInteger& operator+=( const CBigInteger& other );
    CBigInteger& operator-=( const CBigInteger& other );
    CBigInteger& operator*=( const CBigInteger& other );
    CBigInteger& operator/=( const CBigInteger& other );     /**< truncating, GUCEF::invalid_argument for 0 */
    CBigInteger& operator%=( const CBigInteger& other );     /**< sign of the dividend, GUCEF::invalid_argument for 0 */
    CBigInteger& operator&=( const CBigInteger& other );
    CBigInteger& operator|=( const CBigInteger& other );
    CBigInteger& operator^=( const CBigInteger& other );
    CBigInteger& operator<<=( UInt32 bits );
    CBigInteger& operator>>=( UInt32 bits );                 /**< arithmetic */

    CBigInteger& operator++( void );
    CBigInteger& operator--( void );
    CBigInteger operator++( int );
    CBigInteger operator--( int );

    friend CBigInteger operator+( CBigInteger lhs, const CBigInteger& rhs ) { lhs += rhs; return lhs; }
    friend CBigInteger operator-( CBigInteger lhs, const CBigInteger& rhs ) { lhs -= rhs; return lhs; }
    friend CBigInteger operator*( CBigInteger lhs, const CBigInteger& rhs ) { lhs *= rhs; return lhs; }
    friend CBigInteger operator/( CBigInteger lhs, const CBigInteger& rhs ) { lhs /= rhs; return lhs; }
    friend CBigInteger operator%( CBigInteger lhs, const CBigInteger& rhs ) { lhs %= rhs; return lhs; }
    friend CBigInteger operator&( CBigInteger lhs, const CBigInteger& rhs ) { lhs &= rhs; return lhs; }
    friend CBigInteger operator|( CBigInteger lhs, const CBigInteger& rhs ) { lhs |= rhs; return lhs; }
    friend CBigInteger operator^( CBigInteger lhs, const CBigInteger& rhs ) { lhs ^= rhs; return lhs; }
    friend CBigInteger operator<<( CBigInteger lhs, UInt32 bits ) { lhs <<= bits; return lhs; }
    friend CBigInteger operator>>( CBigInteger lhs, UInt32 bits ) { lhs >>= bits; return lhs; }

    friend bool operator==( const CBigInteger& lhs, const CBigInteger& rhs ) { return 0 == lhs.Compare( rhs ); }
    friend bool operator!=( const CBigInteger& lhs, const CBigInteger& rhs ) { return 0 != lhs.Compare( rhs ); }
    friend bool operator<( const CBigInteger& lhs, const CBigInteger& rhs ) { return lhs.Compare( rhs ) < 0; }
    friend bool operator<=( const CBigInteger& lhs, const CBigInteger& rhs ) { return lhs.Compare( rhs ) <= 0; }
    friend bool operator>( const CBigInteger& lhs, const CBigInteger& rhs ) { return lhs.Compare( rhs ) > 0; }
    friend bool operator>=( const CBigInteger& lhs, const CBigInteger& rhs ) { return lhs.Compare( rhs ) >= 0; }

    /** Throws for an error status of a named operation, as the operators do: std::bad_alloc, std::length_error, GUCEF::invalid_argument */
    static void ThrowOnError( TBigIntegerStatus status );

    private:

    typedef UInt32 TLimb;

    template < bool B > struct TBoolTag {};

    void InitFromUInt64( UInt64 value );
    void InitFromInt64( Int64 value );
    template < typename T > void InitFromBuiltin( T value, TBoolTag< true > ) { InitFromUInt64( (UInt64) value ); }
    template < typename T > void InitFromBuiltin( T value, TBoolTag< false > ) { InitFromInt64( (Int64) value ); }

    TLimb* MutableLimbs( void ) { return 0 != m_capacity ? m_storage.heapLimbs : m_storage.inlineLimbs; }
    UInt32 GetCapacity( void ) const { return 0 != m_capacity ? m_capacity : InlineLimbCount; }

    /** Drops leading zero limbs and clears the sign of 0 */
    void Trim( void );

    /** At least 'limbCount' limbs of storage, the content is not kept. Throws std::bad_alloc */
    void ReserveDiscard( UInt32 limbCount );

    /** At least 'limbCount' limbs of storage, the content is kept; 'geometric' grows by doubling. Throws std::bad_alloc */
    void ReserveKeep( UInt32 limbCount, bool geometric );

    /** Copies the magnitude limbs and the sign, trims. Throws std::bad_alloc */
    void AssignLimbs( const TLimb* limbs, UInt32 count, bool negative );

    void ReleaseHeap( void );

    static TLimb* AllocateLimbs( UInt32 limbCount );
    static void FreeLimbs( TLimb* limbs, UInt32 limbCount );

    /** Where a result of up to 'limbCount' limbs is written: this object's own storage when it is large enough and may be
     *  written while the operands are read, otherwise 'temporary' (CommitOutput swaps it in). Throws std::bad_alloc */
    TLimb* PrepareOutput( UInt32 limbCount, bool mustUseTemporary, bool geometric, CBigInteger& temporary );
    void CommitOutput( const TLimb* written, UInt32 size, bool negative, CBigInteger& temporary );
    void SetMagnitude64( UInt64 magnitude, bool negative );
    void SetZero( void );

    static TBigIntegerStatus AddSignedImpl( const TLimb* a, UInt32 an, bool aNegative, const TLimb* b, UInt32 bn, bool bNegative, bool aliased, CBigInteger& result );
    static TBigIntegerStatus MultiplyImpl( const TLimb* a, UInt32 an, const TLimb* b, UInt32 bn, bool negative, bool aliased, CBigInteger& result );
    TBigIntegerStatus AddInt64( Int64 other, CBigInteger& result ) const;
    TBigIntegerStatus AddUInt64( UInt64 other, bool subtract, CBigInteger& result ) const;
    TBigIntegerStatus MultiplyInt64( Int64 other, CBigInteger& result ) const;
    TBigIntegerStatus MultiplyUInt64( UInt64 other, CBigInteger& result ) const;
    Int32 CompareInt64( Int64 other ) const;
    Int32 CompareUInt64( UInt64 other ) const;
    TBigIntegerStatus DivModTruncating( const CBigInteger& divisor, CBigInteger* quotient, CBigInteger* remainder ) const;
    TBigIntegerStatus BitwiseOperation( const CBigInteger& other, UInt32 operation, CBigInteger& result ) const;
    TBigIntegerStatus ToFloatFormat( TRoundingMode mode, UInt32 precisionBits, Int32 minExponent, Int32 maxExponent, Float64& value ) const;

    union UStorage
    {
        TLimb inlineLimbs[ InlineLimbCount ];
        TLimb* heapLimbs;
    };

    UStorage m_storage;
    UInt32 m_size;          /**< limbs in use */
    UInt32 m_capacity;      /**< heap limbs, 0 when the value is held in m_storage.inlineLimbs */
    bool m_negative;
};

/*-------------------------------------------------------------------------*/

/** CTFraction over big integers: exact rational numbers */
typedef CTFraction< CBigInteger, CBigInteger > TBigIntegerFraction;

/*-------------------------------------------------------------------------//
//                                                                         //
//      INLINE IMPLEMENTATION                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

inline
CBigInteger::CBigInteger( void )
    : m_size( 0 )
    , m_capacity( 0 )
    , m_negative( false )
{
}

/*-------------------------------------------------------------------------*/

template < typename T >
inline
CBigInteger::CBigInteger( T value, typename EnableIf< 0 != TypeIsBuiltinInteger< T >::value, int >::type )
    : m_size( 0 )
    , m_capacity( 0 )
    , m_negative( false )
{
    InitFromBuiltin( value, TBoolTag< 0 != TypeIsBuiltinInteger< T >::isUnsigned >() );
}

/*-------------------------------------------------------------------------*/

inline void
CBigInteger::InitFromUInt64( UInt64 value )
{
    m_storage.inlineLimbs[ 0 ] = (TLimb) value;
    m_storage.inlineLimbs[ 1 ] = (TLimb) ( value >> 32 );
    m_size = 0 != m_storage.inlineLimbs[ 1 ] ? 2 : ( 0 != m_storage.inlineLimbs[ 0 ] ? 1 : 0 );
    m_negative = false;
}

/*-------------------------------------------------------------------------*/

inline void
CBigInteger::InitFromInt64( Int64 value )
{
    // the magnitude in unsigned arithmetic: no overflow for INT64_MIN
    InitFromUInt64( value < 0 ? 0 - (UInt64) value : (UInt64) value );
    m_negative = value < 0;
}

/*-------------------------------------------------------------------------*/

template < typename T >
inline typename EnableIf< 0 != TypeIsBuiltinInteger< T >::value, TBigIntegerStatus >::type
CBigInteger::Add( T other, CBigInteger& result ) const
{
    if ( 0 != TypeIsBuiltinInteger< T >::isUnsigned )
        return AddUInt64( (UInt64) other, false, result );
    return AddInt64( (Int64) other, result );
}

/*-------------------------------------------------------------------------*/

template < typename T >
inline typename EnableIf< 0 != TypeIsBuiltinInteger< T >::value, TBigIntegerStatus >::type
CBigInteger::Subtract( T other, CBigInteger& result ) const
{
    if ( 0 != TypeIsBuiltinInteger< T >::isUnsigned )
        return AddUInt64( (UInt64) other, true, result );
    Int64 value = (Int64) other;
    if ( value < 0 )
        return AddUInt64( 0 - (UInt64) value, false, result );
    return AddUInt64( (UInt64) value, true, result );
}

/*-------------------------------------------------------------------------*/

template < typename T >
inline typename EnableIf< 0 != TypeIsBuiltinInteger< T >::value, TBigIntegerStatus >::type
CBigInteger::Multiply( T other, CBigInteger& result ) const
{
    if ( 0 != TypeIsBuiltinInteger< T >::isUnsigned )
        return MultiplyUInt64( (UInt64) other, result );
    return MultiplyInt64( (Int64) other, result );
}

/*-------------------------------------------------------------------------*/

template < typename T >
inline typename EnableIf< 0 != TypeIsBuiltinInteger< T >::value, Int32 >::type
CBigInteger::Compare( T other ) const
{
    if ( 0 != TypeIsBuiltinInteger< T >::isUnsigned )
        return CompareUInt64( (UInt64) other );
    return CompareInt64( (Int64) other );
}

/*-------------------------------------------------------------------------*/

/** Decimal text, for lookup by generic code such as CTFraction::ToString */
inline CString ToString( const CBigInteger& value ) { return value.ToString( 10 ); }

/*-------------------------------------------------------------------------*/

/**
 *  CBigInteger / CBigInteger specialization of the CTFraction integer operations.
 *  Arithmetic with the reduced-denominator formulas of Knuth 4.5.1 (the gcd of the denominators first, which keeps
 *  intermediates small and gives the same lowest terms result), comparison by cross products after a sign check,
 *  correctly rounded float conversion. The functions never report an overflow; a failed allocation throws
 *  std::bad_alloc. Lives here so it is visible wherever the type is complete.
 */
template <>
class GUCEF_CORE_PUBLIC_CPP CTFractionIntegerOps< CBigInteger, CBigInteger >
{
    public:

    static bool Normalize( CBigInteger& n, CBigInteger& d );
    static bool Add( CBigInteger& n, CBigInteger& d, const CBigInteger& otherN, const CBigInteger& otherD );
    static bool Subtract( CBigInteger& n, CBigInteger& d, const CBigInteger& otherN, const CBigInteger& otherD );
    static bool Multiply( CBigInteger& n, CBigInteger& d, const CBigInteger& otherN, const CBigInteger& otherD );
    /** otherN must not be 0 */
    static bool Divide( CBigInteger& n, CBigInteger& d, const CBigInteger& otherN, const CBigInteger& otherD );
    static bool AddScalar( CBigInteger& n, CBigInteger& d, CBigInteger value );
    static bool SubtractScalar( CBigInteger& n, CBigInteger& d, CBigInteger value );
    static bool MultiplyScalar( CBigInteger& n, CBigInteger& d, CBigInteger value );
    /** value must not be 0 */
    static bool DivideScalar( CBigInteger& n, CBigInteger& d, CBigInteger value );
    static bool FromMixed( CBigInteger& n, CBigInteger& d, CBigInteger whole, const CBigInteger& fractionN, const CBigInteger& fractionD );
    static Int32 Compare( const CBigInteger& n1, const CBigInteger& d1, const CBigInteger& n2, const CBigInteger& d2 );
    /** Correctly rounded, nearest even */
    static Float32 ToFloat32( const CBigInteger& n, const CBigInteger& d );
    static Float64 ToFloat64( const CBigInteger& n, const CBigInteger& d );
    /** "n/d" in decimal */
    static CString ToString( const CBigInteger& n, const CBigInteger& d );
    /** Strict decimal parsing of both parts; false (outputs unchanged) for malformed text or a zero denominator */
    static bool FromString( const CString& numeratorText, const CString& denominatorText, CBigInteger& n, CBigInteger& d );
    /** n / d rounded once to the context */
    static TDecimalStatus ToDecimal( const CBigInteger& n, const CBigInteger& d, const CDecimalContext& context, CDecimal& result );
    /** Exact: unscaled / 10^scale (not yet in lowest terms), always true */
    static bool FromDecimal( const CDecimal& value, CBigInteger& n, CBigInteger& d );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_STL_HAVE_CXX11_FEATURES
namespace std {

template <>
struct hash< GUCEF::CORE::CBigInteger >
{
    size_t operator()( const GUCEF::CORE::CBigInteger& value ) const { return (size_t) value.GetHash(); }
};

} /* namespace std */
#endif /* GUCEF_STL_HAVE_CXX11_FEATURES ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CBIGINTEGER_H ? */
