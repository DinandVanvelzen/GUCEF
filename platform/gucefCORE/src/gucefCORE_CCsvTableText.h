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

#ifndef GUCEF_CORE_CCSVTABLETEXT_H
#define GUCEF_CORE_CCSVTABLETEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_DATATABLETYPES_H
#include "gucefCORE_DataTableTypes.h"
#define GUCEF_CORE_DATATABLETYPES_H
#endif /* GUCEF_CORE_DATATABLETYPES_H ? */

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
 *  The text rules of the CSV table codec: UTF-8 validation, number scanning and exact conversion, dates, booleans,
 *  base64 and the canonical text forms of values. Stateless: every member is static and reentrant.
 *  Numbers are never converted through a binary floating point type unless the column is a float column.
 */
class CCsvTableText
{
    public:

    /** Longest number body (digits, point, exponent) that is scanned; longer text is not a number */
    static const UInt32 MaxNumberBodyLength = 126;

    /** Buffer size that FormatInt64(), FormatUInt64() and FormatDecimal() need */
    static const UInt32 MaxIntegerTextLength = 24;

    /** Buffer size that FormatFloat64() and FormatFloat32() need */
    static const UInt32 MaxFloatTextLength = 32;

    /** How numbers are written in the file */
    struct SNumberFormat
    {
        char thousandsSeparator;                    /**< 0 for none */
        char decimalSeparator;                      /**< '.' by default */
        bool negativeParentheses;                   /**< (12.34) is -12.34 */
        GUCEF::vector< CString > currencySymbols;   /**< stripped in front of or behind the digits */

        SNumberFormat( void );
    };

    /** What ScanNumber() found: the sign and the digits, normalized to body = digits [ '.' digits ] [ 'e' [ sign ] digits ] */
    struct SNumberInfo
    {
        bool valid;
        bool negative;
        bool hasPoint;                              /**< a decimal separator was present, digits behind it too */
        bool hasExponent;
        bool explicitPlus;                          /**< the text started with '+' */
        bool leadingZeroGuard;                      /**< an integer part of two or more digits that starts with 0 ("007"): text, not a number, when inferring */
        bool special;                               /**< nan, inf, infinity with an optional sign */
        bool specialIsNan;
        UInt32 intDigits;                           /**< digits of the integer part without leading zeros */
        UInt32 fracDigits;                          /**< digits behind the decimal separator, trailing zeros included */
        UInt32 intPartLength;                       /**< digits of the integer part in the body, leading zeros included */
        UInt32 bodyLength;
        char body[ MaxNumberBodyLength + 1 ];

        SNumberInfo( void );
    };

    /** A compiled date format: YYYY YY MM M DD D and literal separators */
    struct SDateFormat
    {
        enum ETokenKind { TOKEN_YEAR4, TOKEN_YEAR2, TOKEN_MONTH2, TOKEN_MONTH1, TOKEN_DAY2, TOKEN_DAY1, TOKEN_LITERAL };
        struct SToken { ETokenKind kind; char literal; };
        typedef GUCEF::vector< SToken > TTokenVector;

        CString text;
        TTokenVector tokens;
        bool isIso;                                 /**< "YYYY-MM-DD": parsed with CDataTableValues::ParseDate, which also takes signed and longer years */

        SDateFormat( void );
    };

    /** True when the bytes are well formed UTF-8: no overlong forms, no surrogates, nothing above U+10FFFF */
    static bool IsValidUtf8( const char* bytes, UInt32 byteLength );

    /** True when every byte is below 0x80 */
    static bool IsAscii( const char* bytes, UInt32 byteLength );

    /**
     *  Scans text as a number under the format. Blanks around the text are ignored. The text may hold a sign, a currency
     *  symbol and, with negativeParentheses, enclosing parentheses. The digits of the integer part may be grouped with the
     *  thousands separator, in groups of three behind a first group of one to three digits. An exponent is taken only
     *  when allowExponent is set. False, with info.valid false, when it is not a number.
     */
    static bool ScanNumber( const char* text, UInt32 length, const SNumberFormat& format, bool allowExponent, SNumberInfo& info );

    /** A whole number in Int64 range: no decimal point, no exponent */
    static bool NumberToInt64( const SNumberInfo& info, Int64& value );

    /** A whole number in UInt64 range, not negative (negative zero is zero) */
    static bool NumberToUInt64( const SNumberInfo& info, UInt64& value );

    /** The exact value as a mantissa and scale: at most 18 significant digits, scale up to 18, no exponent */
    static bool NumberToDecimal( const SNumberInfo& info, Int64& mantissa, UInt8& scale );

    /** The nearest Float64. False when the value does not fit: infinity from finite text */
    static bool NumberToFloat64( const SNumberInfo& info, Float64& value );

    /** The nearest Float32, rounded once from the text */
    static bool NumberToFloat32( const SNumberInfo& info, Float32& value );

    /** Compiles a date format. Every of year, month and day appears once and variable width tokens are not adjacent */
    static bool CompileDateFormat( const CString& format, SDateFormat& compiled, CString& error );

    /** Strict: the whole text matches the format and names a day of the calendar */
    static bool ParseDate( const SDateFormat& format, const char* text, UInt32 length, Int32& days );

    /** true / false in any case; with allowExtended also yes, no, y, n, t, f, 1, 0 */
    static bool ParseBool( const char* text, UInt32 length, bool allowExtended, bool& value );

    /** An ISO 8601 date time that carries a zone (Z or an offset), as ticks of the unit; the zone is applied */
    static bool ParseZonedTimestamp( const char* text, UInt32 length, TDataTableTimeUnit unit, Int64& ticks );

    /** A decoded base64 text needs at most length / 4 * 3 bytes in out. Padding is required; no blanks inside */
    static bool Base64Decode( const char* text, UInt32 length, char* out, UInt32& outLength );

    /** Needs ( ( length + 2 ) / 3 ) * 4 bytes in out; returns the length written */
    static UInt32 Base64Encode( const void* bytes, UInt32 length, char* out );

    static UInt32 FormatInt64( Int64 value, char* buffer );
    static UInt32 FormatUInt64( UInt64 value, char* buffer );

    /** Exactly scale fraction digits: 12.50, -0.05, 7 for scale 0 */
    static UInt32 FormatDecimal( Int64 mantissa, UInt8 scale, char* buffer );

    /** The shortest text that reads back as the same value; NaN, Infinity and -Infinity for the special values */
    static UInt32 FormatFloat64( Float64 value, char* buffer );
    static UInt32 FormatFloat32( Float32 value, char* buffer );

    private:

    CCsvTableText( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CCSVTABLETEXT_H ? */
