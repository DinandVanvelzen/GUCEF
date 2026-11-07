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

#ifndef GUCEF_CORE_CDECIMAL_H
#define GUCEF_CORE_CDECIMAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TYPES_H
#include "gucef_types.h"
#define GUCEF_TYPES_H
#endif /* GUCEF_TYPES_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_ROUNDINGMODE_H
#include "gucefCORE_RoundingMode.h"
#define GUCEF_CORE_ROUNDINGMODE_H
#endif /* GUCEF_CORE_ROUNDINGMODE_H ? */

#ifndef GUCEF_CORE_C_DECIMALDATA_H
#include "gucefCORE_c_decimaldata.h"
#define GUCEF_CORE_C_DECIMALDATA_H
#endif /* GUCEF_CORE_C_DECIMALDATA_H ? */

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

/**
 *  Outcome of a decimal operation. The values are bits: a status is a bit set of these
 *  so statuses of a sequence of operations can be OR'ed together: status |= x.Add( y, x );
 */
enum EDecimalStatus
{
    DECIMALSTATUS_OK                    = 0x00,
    DECIMALSTATUS_INEXACT               = 0x01,     /**< informational: the result was rounded */
    DECIMALSTATUS_OVERFLOW              = 0x02,     /**< the result does not fit the symmetric 64 bit unscaled range at the result scale */
    DECIMALSTATUS_DIVISION_BY_ZERO      = 0x04,
    DECIMALSTATUS_INVALID_SCALE         = 0x08,     /**< a scale > MaxScale was requested or would be needed */
    DECIMALSTATUS_INVALID_FORMAT        = 0x10,     /**< text does not match the grammar */
    DECIMALSTATUS_INVALID_OPERAND       = 0x20,     /**< NaN/Inf input, INT64_MIN unscaled value, corrupt scale byte, negative weight, unknown rounding mode */
    DECIMALSTATUS_ROUNDING_REQUIRED     = 0x40      /**< ROUNDINGMODE_UNNECESSARY or an exact-only conversion and the result is not exact */
};
typedef UInt32 TDecimalStatus;      /**< bit set of EDecimalStatus */

/*-------------------------------------------------------------------------*/

/** True when the status holds any bit other than the informational DECIMALSTATUS_INEXACT */
inline bool DecimalStatusIsError( TDecimalStatus status ) { return 0 != ( status & ~( (TDecimalStatus) DECIMALSTATUS_INEXACT ) ); }

/*-------------------------------------------------------------------------*/

/** Parse flags, OR'ed together */
enum EDecimalParseFlags
{
    DECIMALPARSE_STRICT                         = 0x00,     /**< [ "-" | "+" ] ( digits [ "." [ digits ] ] | "." digits ), nothing else */
    DECIMALPARSE_ALLOW_EXPONENT                 = 0x01,     /**< also [eE][+-]?digits, applied by moving the decimal point exactly */
    DECIMALPARSE_ALLOW_SURROUNDING_WHITESPACE   = 0x02      /**< ASCII spaces and tabs before and after the number */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Result scale and rounding mode for an operation that can lose digits.
 *  Stateless value class: a single static const context can be shared by any number of threads.
 *  There are no defaults, every policy is visible where it is defined.
 */
class GUCEF_CORE_PUBLIC_CPP CDecimalContext
{
    public:

    /** The scale is not validated here: operations report DECIMALSTATUS_INVALID_SCALE for a scale > CDecimal::MaxScale */
    CDecimalContext( UInt8 resultScale, TRoundingMode roundingMode );

    UInt8 GetResultScale( void ) const { return m_resultScale; }
    /** ROUNDINGMODE_INVALID when the context holds no rounding mode, which the operations refuse */
    TRoundingMode GetRoundingMode( void ) const { return m_roundingMode <= (UInt8) ROUNDINGMODE_UNNECESSARY ? (TRoundingMode) m_roundingMode : ROUNDINGMODE_INVALID; }

    private:

    UInt8 m_resultScale;
    UInt8 m_roundingMode;
};

/*-------------------------------------------------------------------------*/

/**
 *  Exact decimal value made of a signed 64 bit unscaled integer and a decimal scale: value = unscaled * 10^-scale
 *  Example: 0.0825 has unscaled value 825 and scale 4.
 *
 *  - The unscaled range is symmetric: -(2^63 - 1) .. 2^63 - 1. INT64_MIN is never produced and is rejected on input.
 *  - The scale is 0 .. MaxScale (18) and is preserved: arithmetic never strips trailing zeros (12.50 + 0.25 = 12.75).
 *  - There is no NaN, no infinity and no negative zero.
 *  - Every operation that can lose digits takes the result scale and rounding mode explicitly (CDecimalContext).
 *    There is no global, per type or thread local default rounding state.
 *  - Named operations return a TDecimalStatus and never throw. When the status is an error (DecimalStatusIsError)
 *    the result argument is left unchanged, otherwise it is written. 'result' may alias *this or any operand.
 *  - Intermediates are exact in 128 bit: only the final result has to fit 64 bits at its scale.
 *  - There are no arithmetic operators other than the unary minus, which cannot fail: overflow and rounding are
 *    reported by the named operations. Comparison operators are numeric (1.5 == 1.50).
 *  - No heap use, no virtuals, no global mutable state. The only allocating member is ToString().
 */
class GUCEF_CORE_PUBLIC_CPP CDecimal
{
    public:

    static const UInt8 MaxScale = GUCEF_DECIMAL_MAX_SCALE;
    static const UInt32 MaxTextLength = 21;     /**< longest canonical text: -9.223372036854775807, a buffer of MaxTextLength + 1 bytes always suffices */
    static const CDecimal Zero;

    CDecimal( void );                                   /**< 0, scale 0 */
    explicit CDecimal( Int32 integerValue );            /**< exact, scale 0, cannot fail */

    /** 0.0825 = FromUnscaled( 825, 4 ). INT64_MIN or scale > MaxScale: INVALID_OPERAND / INVALID_SCALE */
    static TDecimalStatus FromUnscaled( Int64 unscaledValue, UInt8 scale, CDecimal& result );

    /** 5 at scale 2 = 5.00 (unscaled 500), checked: INT64_MIN gives INVALID_OPERAND, a value that does not fit gives OVERFLOW */
    static TDecimalStatus FromInt64( Int64 value, UInt8 scale, CDecimal& result );

    /**
     *  Parses text, the scale is the number of digits after the decimal point (12.50 has scale 2).
     *  Strict grammar: [ "-" | "+" ] ( digits [ "." [ digits ] ] | "." digits ), see EDecimalParseFlags for the options.
     *  More than MaxScale fractional digits gives INVALID_SCALE, a magnitude above 2^63 - 1 gives OVERFLOW.
     *  Locale independent, linear in the length.
     */
    static TDecimalStatus FromString( const CString& text, CDecimal& result, UInt32 parseFlags = DECIMALPARSE_STRICT );

    /** As FromString but the result has the given scale: shorter text is zero padded, longer text is rounded once */
    static TDecimalStatus FromString( const CString& text, UInt8 scale, TRoundingMode mode, CDecimal& result, UInt32 parseFlags = DECIMALPARSE_STRICT );

    static TDecimalStatus FromChars( const char* text, UInt32 length, CDecimal& result, UInt32 parseFlags = DECIMALPARSE_STRICT );
    static TDecimalStatus FromChars( const char* text, UInt32 length, UInt8 scale, TRoundingMode mode, CDecimal& result, UInt32 parseFlags = DECIMALPARSE_STRICT );

    /** Constants in code and tests: CDecimal::Parse( "0.0825" ). Strict grammar. Throws GUCEF::invalid_argument for any error status */
    static CDecimal Parse( const char* literal );

    /**
     *  WARNING: a double holds a binary approximation. This rounds the exact binary value of 'value' to 'scale' digits,
     *  e.g. FromFloat64( 2.675, 2, HALF_EVEN ) = 2.67 because the double is 2.67499999...
     *  Use only at boundaries where a float is all there is (third party APIs, legacy files). NaN/Inf: INVALID_OPERAND
     */
    static TDecimalStatus FromFloat64( Float64 value, UInt8 scale, TRoundingMode mode, CDecimal& result );

    /**
     *  The decimal with the smallest scale that converts back to exactly 'value' (0.0825 -> 0.0825).
     *  INVALID_SCALE if there is none with a scale <= MaxScale. INEXACT when the decimal differs from the exact binary value
     */
    static TDecimalStatus FromFloat64Shortest( Float64 value, CDecimal& result );

    /** As FromFloat64Shortest with the round trip check done in Float32 */
    static TDecimalStatus FromFloat32Shortest( Float32 value, CDecimal& result );

    Int64 GetUnscaledValue( void ) const { return m_unscaled; }
    UInt8 GetScale( void ) const { return m_scale; }
    UInt8 GetDigitCount( void ) const;                  /**< decimal digits of |unscaled|, 1 for 0 */
    bool FitsPrecision( UInt8 precision ) const;        /**< |unscaled| <= 10^precision - 1 */

    bool IsZero( void ) const { return 0 == m_unscaled; }
    bool IsNegative( void ) const { return m_unscaled < 0; }
    bool IsPositive( void ) const { return m_unscaled > 0; }
    Int32 GetSign( void ) const { return m_unscaled < 0 ? -1 : ( m_unscaled > 0 ? 1 : 0 ); }
    bool IsIntegral( void ) const;                      /**< no non-zero fractional digits */

    CDecimal Negate( void ) const;                      /**< cannot fail (symmetric range) */
    CDecimal Abs( void ) const;                         /**< cannot fail */
    CDecimal Normalized( void ) const;                  /**< trailing zeros stripped: 2.2000 -> 2.2, 30.00 -> 30 */

    /** Canonical text: '-' when negative, the integer digits (at least 0), for scale > 0 a '.' and exactly 'scale' digits. Round trips through FromString */
    CString ToString( void ) const;

    /** Text at 'displayScale' digits: rounds with 'mode' or zero pads */
    TDecimalStatus ToString( UInt8 displayScale, TRoundingMode mode, CString& text ) const;

    /** Canonical text into buffer, null terminated, no allocation. Returns the length, 0 if the buffer is too small (MaxTextLength + 1 bytes always suffice) */
    UInt32 ToChars( char* buffer, UInt32 bufferSize ) const;

    /** Nearest double, ties to even */
    Float64 ToFloat64( void ) const;

    /** Integer value rounded with 'mode' */
    TDecimalStatus ToInt64( TRoundingMode mode, Int64& result ) const;

    /** Exact sum, result scale = max( scale, other scale ) */
    TDecimalStatus Add( const CDecimal& other, CDecimal& result ) const;
    /** Sum aligned and added in 128 bit, rounded once to the context */
    TDecimalStatus Add( const CDecimal& other, const CDecimalContext& ctx, CDecimal& result ) const;
    /** Exact difference, result scale = max( scale, other scale ) */
    TDecimalStatus Subtract( const CDecimal& other, CDecimal& result ) const;
    TDecimalStatus Subtract( const CDecimal& other, const CDecimalContext& ctx, CDecimal& result ) const;
    /** Exact product, result scale = scale + other scale; INVALID_SCALE if that exceeds MaxScale */
    TDecimalStatus Multiply( const CDecimal& other, CDecimal& result ) const;
    /** Exact 128 bit product, rounded once to the context */
    TDecimalStatus Multiply( const CDecimal& other, const CDecimalContext& ctx, CDecimal& result ) const;
    /** Exact product with an integer, result scale = scale */
    TDecimalStatus Multiply( Int64 factor, CDecimal& result ) const;
    /** Quotient at the context scale, rounded once. Division by zero (also 0 / 0): DIVISION_BY_ZERO */
    TDecimalStatus Divide( const CDecimal& divisor, const CDecimalContext& ctx, CDecimal& result ) const;
    TDecimalStatus Divide( Int64 divisor, const CDecimalContext& ctx, CDecimal& result ) const;
    /** this * factor / divisor with one rounding and no intermediate overflow (128 bit product, exact division chain) */
    TDecimalStatus MulDiv( const CDecimal& factor, const CDecimal& divisor, const CDecimalContext& ctx, CDecimal& result ) const;
    /** this * percent / 100, one rounding */
    TDecimalStatus PercentOf( const CDecimal& percent, const CDecimalContext& ctx, CDecimal& result ) const;
    /** The value at 'newScale': zero padded (checked) or rounded with 'mode' */
    TDecimalStatus Rescale( UInt8 newScale, TRoundingMode mode, CDecimal& result ) const;
    /** Exact division by 10^places by raising the scale. INVALID_SCALE when the scale would exceed MaxScale */
    TDecimalStatus MovePointLeft( UInt8 places, CDecimal& result ) const;
    /** Exact multiplication by 10^places by lowering the scale, then the unscaled value. OVERFLOW when it does not fit */
    TDecimalStatus MovePointRight( UInt8 places, CDecimal& result ) const;

    /** -1, 0, 1, numeric across scales */
    Int32 Compare( const CDecimal& other ) const;

    /** Same unscaled value and same scale: 1.5 is not identical to 1.50 */
    bool IsIdentical( const CDecimal& other ) const { return m_unscaled == other.m_unscaled && m_scale == other.m_scale; }

    /** Consistent with operator==: equal values have equal hashes (the normalized form is hashed) */
    UInt64 GetHash( void ) const;

    /** Exact sum, result scale = the largest scale. Accumulates in 128 bit and narrows once */
    static TDecimalStatus Sum( const CDecimal* values, UInt32 count, CDecimal& result );
    /** Sum accumulated in 128 bit and rounded once to the context */
    static TDecimalStatus Sum( const CDecimal* values, UInt32 count, const CDecimalContext& ctx, CDecimal& result );

    /**
     *  Splits 'total' over 'count' parts in proportion to 'weights' (>= 0, sum > 0) at total's scale so that the parts sum
     *  to total exactly: largest remainder method on exact remainders, ties to the lowest index.
     *  100.00 over weights 1,1,1 -> 33.34, 33.33, 33.33
     *  The weights aligned to their largest scale have to sum within 64 bits (OVERFLOW otherwise).
     *  'parts' must hold 'count' elements and must not overlap 'weights'; it is only written on success.
     *  Negative weight: INVALID_OPERAND. All weights zero (or count 0): DIVISION_BY_ZERO.
     */
    static TDecimalStatus Allocate( const CDecimal& total, const CDecimal* weights, UInt32 count, CDecimal* parts );

    /** Ordering that keeps representations apart: numeric, then scale (1.5 before 1.50) */
    struct GUCEF_CORE_PUBLIC_CPP RepresentationLess
    {
        bool operator()( const CDecimal& a, const CDecimal& b ) const;
    };

    bool operator==( const CDecimal& other ) const { return 0 == Compare( other ); }     /**< numeric */
    bool operator!=( const CDecimal& other ) const { return 0 != Compare( other ); }
    bool operator<( const CDecimal& other ) const { return Compare( other ) < 0; }
    bool operator<=( const CDecimal& other ) const { return Compare( other ) <= 0; }
    bool operator>( const CDecimal& other ) const { return Compare( other ) > 0; }
    bool operator>=( const CDecimal& other ) const { return Compare( other ) >= 0; }

    CDecimal operator-( void ) const { return Negate(); }      /**< cannot fail */

    private:

    friend class CDecimalAccumulator;

    CDecimal( Int64 unscaledValue, UInt8 scale, bool );     /**< unchecked, internal */

    /** round( sign * magnitude * 10^exponent10 ) at resultScale, for exact 128 bit intermediates held elsewhere */
    static TDecimalStatus RoundScaled( bool negative, UInt64 magnitudeHigh, UInt64 magnitudeLow, Int32 exponent10, TRoundingMode mode, UInt8 resultScale, CDecimal& result );

    TDecimalStatus AddAligned( const CDecimal& other, bool subtract, CDecimal& result ) const;
    Int32 CompareAligned( const CDecimal& other ) const;

    Int64 m_unscaled;
    UInt8 m_scale;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      INLINE IMPLEMENTATION                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

inline
CDecimal::CDecimal( void )
    : m_unscaled( 0 )
    , m_scale( 0 )
{
}

/*-------------------------------------------------------------------------*/

inline
CDecimal::CDecimal( Int32 integerValue )
    : m_unscaled( integerValue )
    , m_scale( 0 )
{
}

/*-------------------------------------------------------------------------*/

inline
CDecimal::CDecimal( Int64 unscaledValue, UInt8 scale, bool )
    : m_unscaled( unscaledValue )
    , m_scale( scale )
{
}

/*-------------------------------------------------------------------------*/

inline CDecimal
CDecimal::Negate( void ) const
{
    return CDecimal( -m_unscaled, m_scale, true );
}

/*-------------------------------------------------------------------------*/

inline CDecimal
CDecimal::Abs( void ) const
{
    return CDecimal( m_unscaled < 0 ? -m_unscaled : m_unscaled, m_scale, true );
}

/*-------------------------------------------------------------------------*/

inline Int32
CDecimal::Compare( const CDecimal& other ) const
{
    if ( m_scale == other.m_scale )
        return m_unscaled < other.m_unscaled ? -1 : ( m_unscaled > other.m_unscaled ? 1 : 0 );
    return CompareAligned( other );
}

/*-------------------------------------------------------------------------*/

inline TDecimalStatus
CDecimal::Add( const CDecimal& other, CDecimal& result ) const
{
    if ( m_scale == other.m_scale )
    {
        // unsigned add: wrapping is defined, the overflow test is done on the wrapped result
        Int64 sum = (Int64) ( (UInt64) m_unscaled + (UInt64) other.m_unscaled );
        if ( ( ( m_unscaled ^ sum ) & ( other.m_unscaled ^ sum ) ) < 0 || GUCEF_INT64MIN == sum )
            return DECIMALSTATUS_OVERFLOW;
        result.m_scale = m_scale;
        result.m_unscaled = sum;
        return DECIMALSTATUS_OK;
    }
    return AddAligned( other, false, result );
}

/*-------------------------------------------------------------------------*/

inline TDecimalStatus
CDecimal::Subtract( const CDecimal& other, CDecimal& result ) const
{
    if ( m_scale == other.m_scale )
    {
        Int64 difference = (Int64) ( (UInt64) m_unscaled - (UInt64) other.m_unscaled );
        if ( ( ( m_unscaled ^ other.m_unscaled ) & ( m_unscaled ^ difference ) ) < 0 || GUCEF_INT64MIN == difference )
            return DECIMALSTATUS_OVERFLOW;
        result.m_scale = m_scale;
        result.m_unscaled = difference;
        return DECIMALSTATUS_OK;
    }
    return AddAligned( other, true, result );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */

/*-------------------------------------------------------------------------*/

template <> inline UInt8 TryToGetGucefTypeIdForTType( const CORE::CDecimal* dummyValue ) { return GUCEF_DATATYPE_INT64_UINT8_DECIMAL; }

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_STL_HAVE_CXX11_FEATURES
namespace std {

template <>
struct hash< GUCEF::CORE::CDecimal >
{
    size_t operator()( const GUCEF::CORE::CDecimal& value ) const { return (size_t) value.GetHash(); }
};

} /* namespace std */
#endif /* GUCEF_STL_HAVE_CXX11_FEATURES ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDECIMAL_H ? */
