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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <locale.h>

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#include "gucefCORE_CCsvTableText.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static inline bool
IsBlank( char c )
{
    return ' ' == c || '\t' == c;
}

/*-------------------------------------------------------------------------*/

static inline bool
IsDigit( char c )
{
    return c >= '0' && c <= '9';
}

/*-------------------------------------------------------------------------*/

static inline bool
IsNanValue( Float64 value )
{
    return value != value;
}

/*-------------------------------------------------------------------------*/

static inline bool
IsInfiniteValue( Float64 value )
{
    return value == HUGE_VAL || value == -HUGE_VAL;
}

/*-------------------------------------------------------------------------*/

static inline char
ToLowerAscii( char c )
{
    return ( c >= 'A' && c <= 'Z' ) ? (char) ( c - 'A' + 'a' ) : c;
}

/*-------------------------------------------------------------------------*/

static bool
EqualsLowerAscii( const char* text, UInt32 length, const char* lowerLiteral )
{
    UInt32 i = 0;
    for ( ; i < length && '\0' != lowerLiteral[ i ]; ++i )
        if ( ToLowerAscii( text[ i ] ) != lowerLiteral[ i ] )
            return false;
    return i == length && '\0' == lowerLiteral[ i ];
}

/*-------------------------------------------------------------------------*/

/** The length of the currency symbol the text starts with, 0 for none */
static UInt32
SymbolAtStart( const char* text, UInt32 length, const GUCEF::vector< CString >& symbols )
{
    UInt32 best = 0;
    for ( size_t i = 0; i < symbols.size(); ++i )
    {
        const UInt32 symbolLength = symbols[ i ].ByteSize() > 0 ? symbols[ i ].ByteSize() - 1 : 0;
        if ( symbolLength > 0 && symbolLength <= length && symbolLength > best && 0 == memcmp( text, symbols[ i ].C_String(), symbolLength ) )
            best = symbolLength;
    }
    return best;
}

/*-------------------------------------------------------------------------*/

static UInt32
SymbolAtEnd( const char* text, UInt32 length, const GUCEF::vector< CString >& symbols )
{
    UInt32 best = 0;
    for ( size_t i = 0; i < symbols.size(); ++i )
    {
        const UInt32 symbolLength = symbols[ i ].ByteSize() > 0 ? symbols[ i ].ByteSize() - 1 : 0;
        if ( symbolLength > 0 && symbolLength <= length && symbolLength > best && 0 == memcmp( text + length - symbolLength, symbols[ i ].C_String(), symbolLength ) )
            best = symbolLength;
    }
    return best;
}

/*-------------------------------------------------------------------------*/

/** The digits of the body as a magnitude; false on overflow of 64 bits */
static bool
DigitsToMagnitude( const char* digits, UInt32 count, UInt64& magnitude )
{
    UInt64 result = 0;
    for ( UInt32 i = 0; i < count; ++i )
    {
        const UInt32 digit = (UInt32) ( digits[ i ] - '0' );
        if ( result > ( GUCEF_UINT64MAX - digit ) / 10 )
            return false;
        result = result * 10 + digit;
    }
    magnitude = result;
    return true;
}

/*-------------------------------------------------------------------------*/

CCsvTableText::SNumberFormat::SNumberFormat( void )
    : thousandsSeparator( '\0' )
    , decimalSeparator( '.' )
    , negativeParentheses( false )
    , currencySymbols()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCsvTableText::SNumberInfo::SNumberInfo( void )
    : valid( false )
    , negative( false )
    , hasPoint( false )
    , hasExponent( false )
    , explicitPlus( false )
    , leadingZeroGuard( false )
    , special( false )
    , specialIsNan( false )
    , intDigits( 0 )
    , fracDigits( 0 )
    , intPartLength( 0 )
    , bodyLength( 0 )
{GUCEF_TRACE;

    body[ 0 ] = '\0';
}

/*-------------------------------------------------------------------------*/

CCsvTableText::SDateFormat::SDateFormat( void )
    : text()
    , tokens()
    , isIso( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CCsvTableText::IsAscii( const char* bytes, UInt32 byteLength )
{
    for ( UInt32 i = 0; i < byteLength; ++i )
        if ( 0 != ( (unsigned char) bytes[ i ] & 0x80 ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCsvTableText::IsValidUtf8( const char* bytes, UInt32 byteLength )
{
    const unsigned char* s = reinterpret_cast< const unsigned char* >( bytes );
    UInt32 i = 0;
    while ( i < byteLength )
    {
        const unsigned char c = s[ i ];
        if ( c < 0x80 )
        {
            ++i;
        }
        else
        if ( c >= 0xC2 && c <= 0xDF )
        {
            if ( i + 1 >= byteLength || ( s[ i + 1 ] & 0xC0 ) != 0x80 )
                return false;
            i += 2;
        }
        else
        if ( c >= 0xE0 && c <= 0xEF )
        {
            if ( i + 2 >= byteLength || ( s[ i + 1 ] & 0xC0 ) != 0x80 || ( s[ i + 2 ] & 0xC0 ) != 0x80 )
                return false;
            if ( 0xE0 == c && s[ i + 1 ] < 0xA0 )          // overlong
                return false;
            if ( 0xED == c && s[ i + 1 ] > 0x9F )          // surrogates
                return false;
            i += 3;
        }
        else
        if ( c >= 0xF0 && c <= 0xF4 )
        {
            if ( i + 3 >= byteLength || ( s[ i + 1 ] & 0xC0 ) != 0x80 || ( s[ i + 2 ] & 0xC0 ) != 0x80 || ( s[ i + 3 ] & 0xC0 ) != 0x80 )
                return false;
            if ( 0xF0 == c && s[ i + 1 ] < 0x90 )          // overlong
                return false;
            if ( 0xF4 == c && s[ i + 1 ] > 0x8F )          // above U+10FFFF
                return false;
            i += 4;
        }
        else
        {
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCsvTableText::ScanNumber( const char* text, UInt32 length, const SNumberFormat& format, bool allowExponent, SNumberInfo& info )
{
    info = SNumberInfo();
    if ( GUCEF_NULL == text )
        return false;

    UInt32 begin = 0, end = length;
    while ( begin < end && IsBlank( text[ begin ] ) ) ++begin;
    while ( end > begin && IsBlank( text[ end - 1 ] ) ) --end;
    if ( begin == end )
        return false;

    bool negative = false;
    bool parenthesized = false;
    if ( format.negativeParentheses && '(' == text[ begin ] && ')' == text[ end - 1 ] )
    {
        parenthesized = true;
        negative = true;
        ++begin;
        --end;
        while ( begin < end && IsBlank( text[ begin ] ) ) ++begin;
        while ( end > begin && IsBlank( text[ end - 1 ] ) ) --end;
        if ( begin == end )
            return false;
    }

    bool signSeen = false;
    bool currencySeen = false;
    bool explicitPlus = false;
    while ( begin < end )
    {
        const char c = text[ begin ];
        if ( !signSeen && ( '-' == c || '+' == c ) )
        {
            if ( parenthesized )
                return false;
            signSeen = true;
            negative = ( '-' == c );
            explicitPlus = ( '+' == c );
            ++begin;
            continue;
        }
        if ( !currencySeen )
        {
            const UInt32 symbolLength = SymbolAtStart( text + begin, end - begin, format.currencySymbols );
            if ( symbolLength > 0 )
            {
                currencySeen = true;
                begin += symbolLength;
                while ( begin < end && IsBlank( text[ begin ] ) ) ++begin;
                continue;
            }
        }
        break;
    }
    if ( !currencySeen )
    {
        const UInt32 symbolLength = SymbolAtEnd( text + begin, end - begin, format.currencySymbols );
        if ( symbolLength > 0 )
        {
            end -= symbolLength;
            while ( end > begin && IsBlank( text[ end - 1 ] ) ) --end;
        }
    }
    if ( begin >= end )
        return false;

    // nan, inf and infinity with the sign that was seen
    {
        const UInt32 rest = end - begin;
        if ( EqualsLowerAscii( text + begin, rest, "nan" ) )
        {
            info.valid = true; info.special = true; info.specialIsNan = true; info.negative = negative; info.explicitPlus = explicitPlus;
            return true;
        }
        if ( EqualsLowerAscii( text + begin, rest, "inf" ) || EqualsLowerAscii( text + begin, rest, "infinity" ) )
        {
            info.valid = true; info.special = true; info.specialIsNan = false; info.negative = negative; info.explicitPlus = explicitPlus;
            return true;
        }
    }

    UInt32 pos = begin;
    UInt32 bodyLength = 0;
    char* body = info.body;

    // the integer part, with grouping
    UInt32 groupLength = 0;
    bool usedSeparator = false;
    while ( pos < end )
    {
        const char c = text[ pos ];
        if ( IsDigit( c ) )
        {
            if ( bodyLength >= MaxNumberBodyLength )
                return false;
            body[ bodyLength++ ] = c;
            ++groupLength;
            ++pos;
        }
        else
        if ( '\0' != format.thousandsSeparator && c == format.thousandsSeparator )
        {
            if ( 0 == groupLength )
                return false;
            if ( !usedSeparator ? groupLength > 3 : groupLength != 3 )
                return false;
            usedSeparator = true;
            groupLength = 0;
            ++pos;
        }
        else
        {
            break;
        }
    }
    if ( usedSeparator && 3 != groupLength )
        return false;
    const UInt32 intPartLength = bodyLength;

    UInt32 fracDigits = 0;
    bool hasPoint = false;
    if ( pos < end && text[ pos ] == format.decimalSeparator )
    {
        hasPoint = true;
        if ( bodyLength >= MaxNumberBodyLength )
            return false;
        body[ bodyLength++ ] = '.';
        ++pos;
        while ( pos < end && IsDigit( text[ pos ] ) )
        {
            if ( bodyLength >= MaxNumberBodyLength )
                return false;
            body[ bodyLength++ ] = text[ pos ];
            ++fracDigits;
            ++pos;
        }
        if ( 0 == fracDigits )
            return false;
    }
    if ( 0 == intPartLength && 0 == fracDigits )
        return false;

    bool hasExponent = false;
    if ( pos < end && allowExponent && ( 'e' == text[ pos ] || 'E' == text[ pos ] ) )
    {
        hasExponent = true;
        if ( bodyLength + 2 >= MaxNumberBodyLength )
            return false;
        body[ bodyLength++ ] = 'e';
        ++pos;
        if ( pos < end && ( '+' == text[ pos ] || '-' == text[ pos ] ) )
            body[ bodyLength++ ] = text[ pos++ ];
        UInt32 exponentDigits = 0;
        while ( pos < end && IsDigit( text[ pos ] ) )
        {
            if ( bodyLength >= MaxNumberBodyLength )
                return false;
            body[ bodyLength++ ] = text[ pos++ ];
            ++exponentDigits;
        }
        if ( 0 == exponentDigits )
            return false;
    }
    if ( pos != end )
        return false;
    body[ bodyLength ] = '\0';

    UInt32 leadingZeros = 0;
    while ( leadingZeros < intPartLength && '0' == body[ leadingZeros ] ) ++leadingZeros;

    info.valid = true;
    info.negative = negative;
    info.hasPoint = hasPoint;
    info.hasExponent = hasExponent;
    info.explicitPlus = explicitPlus;
    info.leadingZeroGuard = intPartLength >= 2 && '0' == body[ 0 ];
    info.intDigits = intPartLength - leadingZeros;
    info.fracDigits = fracDigits;
    info.intPartLength = intPartLength;
    info.bodyLength = bodyLength;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCsvTableText::NumberToInt64( const SNumberInfo& info, Int64& value )
{
    if ( !info.valid || info.special || info.hasPoint || info.hasExponent || 0 == info.intPartLength )
        return false;
    UInt64 magnitude = 0;
    if ( !DigitsToMagnitude( info.body, info.intPartLength, magnitude ) )
        return false;
    if ( info.negative )
    {
        if ( magnitude > (UInt64) GUCEF_INT64MAX + 1 )
            return false;
        value = ( magnitude == (UInt64) GUCEF_INT64MAX + 1 ) ? GUCEF_INT64MIN : -(Int64) magnitude;
        return true;
    }
    if ( magnitude > (UInt64) GUCEF_INT64MAX )
        return false;
    value = (Int64) magnitude;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCsvTableText::NumberToUInt64( const SNumberInfo& info, UInt64& value )
{
    if ( !info.valid || info.special || info.hasPoint || info.hasExponent || 0 == info.intPartLength )
        return false;
    UInt64 magnitude = 0;
    if ( !DigitsToMagnitude( info.body, info.intPartLength, magnitude ) )
        return false;
    if ( info.negative && 0 != magnitude )
        return false;
    value = magnitude;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCsvTableText::NumberToDecimal( const SNumberInfo& info, Int64& mantissa, UInt8& scale )
{
    if ( !info.valid || info.special || info.hasExponent || info.fracDigits > 18 )
        return false;

    // all digits without the point; the leading zeros do not count
    char digits[ MaxNumberBodyLength + 1 ];
    UInt32 digitCount = 0;
    for ( UInt32 i = 0; i < info.bodyLength; ++i )
        if ( '.' != info.body[ i ] )
            digits[ digitCount++ ] = info.body[ i ];

    UInt32 first = 0;
    while ( first < digitCount && '0' == digits[ first ] ) ++first;
    if ( digitCount - first > 18 )
        return false;

    UInt64 magnitude = 0;
    if ( !DigitsToMagnitude( digits + first, digitCount - first, magnitude ) )
        return false;
    mantissa = info.negative ? -(Int64) magnitude : (Int64) magnitude;
    scale = (UInt8) info.fracDigits;
    return true;
}

/*-------------------------------------------------------------------------*/

/** The body with the sign and the decimal point of the locale of the C library, which strtod reads */
static bool
PrepareForStrtod( const CCsvTableText::SNumberInfo& info, char* buffer, UInt32 bufferSize )
{
    if ( info.bodyLength + 2 > bufferSize )
        return false;

    char point = '.';
    const struct lconv* conv = localeconv();
    if ( GUCEF_NULL != conv && GUCEF_NULL != conv->decimal_point && '\0' != conv->decimal_point[ 0 ] && '\0' == conv->decimal_point[ 1 ] )
        point = conv->decimal_point[ 0 ];

    UInt32 length = 0;
    if ( info.negative )
        buffer[ length++ ] = '-';
    for ( UInt32 i = 0; i < info.bodyLength; ++i )
        buffer[ length++ ] = ( '.' == info.body[ i ] ) ? point : info.body[ i ];
    buffer[ length ] = '\0';
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCsvTableText::NumberToFloat64( const SNumberInfo& info, Float64& value )
{
    if ( !info.valid )
        return false;
    if ( info.special )
    {
        if ( info.specialIsNan )
            value = info.negative ? -strtod( "nan", GUCEF_NULL ) : strtod( "nan", GUCEF_NULL );
        else
            value = info.negative ? -HUGE_VAL : HUGE_VAL;
        return true;
    }

    char buffer[ MaxNumberBodyLength + 4 ];
    if ( !PrepareForStrtod( info, buffer, sizeof( buffer ) ) )
        return false;
    char* endPtr = GUCEF_NULL;
    const Float64 result = strtod( buffer, &endPtr );
    if ( GUCEF_NULL == endPtr || '\0' != *endPtr )
        return false;
    if ( IsInfiniteValue( (Float64) result ) )
        return false;
    value = result;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCsvTableText::NumberToFloat32( const SNumberInfo& info, Float32& value )
{
    if ( !info.valid )
        return false;
    if ( info.special )
    {
        if ( info.specialIsNan )
            value = info.negative ? -strtof( "nan", GUCEF_NULL ) : strtof( "nan", GUCEF_NULL );
        else
            value = info.negative ? (Float32) -HUGE_VAL : (Float32) HUGE_VAL;
        return true;
    }

    char buffer[ MaxNumberBodyLength + 4 ];
    if ( !PrepareForStrtod( info, buffer, sizeof( buffer ) ) )
        return false;
    char* endPtr = GUCEF_NULL;
    const Float32 result = strtof( buffer, &endPtr );
    if ( GUCEF_NULL == endPtr || '\0' != *endPtr )
        return false;
    if ( IsInfiniteValue( (Float64) result ) )
        return false;
    value = result;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCsvTableText::CompileDateFormat( const CString& format, SDateFormat& compiled, CString& error )
{
    compiled = SDateFormat();
    compiled.text = format;
    const UInt32 length = format.ByteSize() > 0 ? format.ByteSize() - 1 : 0;
    const char* text = format.C_String();

    UInt32 years = 0, months = 0, days = 0;
    UInt32 i = 0;
    while ( i < length )
    {
        const char c = text[ i ];
        UInt32 run = 1;
        if ( 'Y' == c || 'M' == c || 'D' == c )
        {
            while ( i + run < length && text[ i + run ] == c ) ++run;
        }

        SDateFormat::SToken token;
        token.literal = '\0';
        if ( 'Y' == c )
        {
            if ( 4 == run ) token.kind = SDateFormat::TOKEN_YEAR4;
            else if ( 2 == run ) token.kind = SDateFormat::TOKEN_YEAR2;
            else { error = "Date format \"" + format + "\": YYYY or YY expected"; return false; }
            ++years;
        }
        else
        if ( 'M' == c )
        {
            if ( 2 == run ) token.kind = SDateFormat::TOKEN_MONTH2;
            else if ( 1 == run ) token.kind = SDateFormat::TOKEN_MONTH1;
            else { error = "Date format \"" + format + "\": MM or M expected"; return false; }
            ++months;
        }
        else
        if ( 'D' == c )
        {
            if ( 2 == run ) token.kind = SDateFormat::TOKEN_DAY2;
            else if ( 1 == run ) token.kind = SDateFormat::TOKEN_DAY1;
            else { error = "Date format \"" + format + "\": DD or D expected"; return false; }
            ++days;
        }
        else
        {
            if ( '\0' == c )
            {
                error = "Date format \"" + format + "\" has a NUL";
                return false;
            }
            token.kind = SDateFormat::TOKEN_LITERAL;
            token.literal = c;
        }
        compiled.tokens.push_back( token );
        i += run;
    }

    if ( 1 != years || 1 != months || 1 != days )
    {
        error = "Date format \"" + format + "\" needs a year, a month and a day, each once";
        return false;
    }

    // a variable width token next to another number cannot be told apart from it
    for ( size_t n = 0; n < compiled.tokens.size(); ++n )
    {
        const SDateFormat::ETokenKind kind = compiled.tokens[ n ].kind;
        if ( SDateFormat::TOKEN_MONTH1 != kind && SDateFormat::TOKEN_DAY1 != kind )
            continue;
        if ( ( n > 0 && SDateFormat::TOKEN_LITERAL != compiled.tokens[ n - 1 ].kind ) ||
             ( n + 1 < compiled.tokens.size() && SDateFormat::TOKEN_LITERAL != compiled.tokens[ n + 1 ].kind ) )
        {
            error = "Date format \"" + format + "\" has a one digit month or day next to another number";
            return false;
        }
    }

    compiled.isIso = ( format == "YYYY-MM-DD" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCsvTableText::ParseDate( const SDateFormat& format, const char* text, UInt32 length, Int32& days )
{
    if ( GUCEF_NULL == text )
        return false;
    if ( format.isIso )
        return CDataTableValues::ParseDate( text, length, days );

    UInt32 pos = 0;
    Int32 year = 0;
    UInt32 month = 0, day = 0;
    for ( size_t n = 0; n < format.tokens.size(); ++n )
    {
        const SDateFormat::SToken& token = format.tokens[ n ];
        UInt32 digits = 0, value = 0;
        switch ( token.kind )
        {
            case SDateFormat::TOKEN_LITERAL:
            {
                if ( pos >= length || text[ pos ] != token.literal )
                    return false;
                ++pos;
                continue;
            }
            case SDateFormat::TOKEN_YEAR4: digits = 4; break;
            case SDateFormat::TOKEN_YEAR2:
            case SDateFormat::TOKEN_MONTH2:
            case SDateFormat::TOKEN_DAY2: digits = 2; break;
            default: digits = 0; break;       // one or two digits
        }

        if ( 0 != digits )
        {
            if ( pos + digits > length )
                return false;
            for ( UInt32 d = 0; d < digits; ++d )
            {
                if ( !IsDigit( text[ pos + d ] ) )
                    return false;
                value = value * 10 + (UInt32) ( text[ pos + d ] - '0' );
            }
            pos += digits;
        }
        else
        {
            if ( pos >= length || !IsDigit( text[ pos ] ) )
                return false;
            value = (UInt32) ( text[ pos++ ] - '0' );
            if ( pos < length && IsDigit( text[ pos ] ) )
                value = value * 10 + (UInt32) ( text[ pos++ ] - '0' );
        }

        switch ( token.kind )
        {
            case SDateFormat::TOKEN_YEAR4: year = (Int32) value; break;
            case SDateFormat::TOKEN_YEAR2: year = value < 70 ? (Int32) ( 2000 + value ) : (Int32) ( 1900 + value ); break;
            case SDateFormat::TOKEN_MONTH2:
            case SDateFormat::TOKEN_MONTH1: month = value; break;
            default: day = value; break;
        }
    }
    if ( pos != length )
        return false;
    return CDataTableValues::DaysFromCivil( year, month, day, days );
}

/*-------------------------------------------------------------------------*/

bool
CCsvTableText::ParseBool( const char* text, UInt32 length, bool allowExtended, bool& value )
{
    if ( GUCEF_NULL == text )
        return false;
    if ( EqualsLowerAscii( text, length, "true" ) ) { value = true; return true; }
    if ( EqualsLowerAscii( text, length, "false" ) ) { value = false; return true; }
    if ( !allowExtended )
        return false;
    if ( EqualsLowerAscii( text, length, "yes" ) || EqualsLowerAscii( text, length, "y" ) ||
         EqualsLowerAscii( text, length, "t" ) || EqualsLowerAscii( text, length, "1" ) )
    {
        value = true;
        return true;
    }
    if ( EqualsLowerAscii( text, length, "no" ) || EqualsLowerAscii( text, length, "n" ) ||
         EqualsLowerAscii( text, length, "f" ) || EqualsLowerAscii( text, length, "0" ) )
    {
        value = false;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CCsvTableText::ParseZonedTimestamp( const char* text, UInt32 length, TDataTableTimeUnit unit, Int64& ticks )
{
    if ( GUCEF_NULL == text || length < 16 )
        return false;

    // a date, then a 'T' or a space and a time; the year may be signed and longer than 4 digits
    UInt32 separator = 0;
    while ( separator < length && 'T' != text[ separator ] && ' ' != text[ separator ] )
        ++separator;
    if ( separator < 8 || separator + 1 >= length )
        return false;

    bool zoned = false;
    if ( 'Z' == text[ length - 1 ] )
    {
        zoned = true;
    }
    else
    if ( length >= separator + 7 && ( '+' == text[ length - 6 ] || '-' == text[ length - 6 ] ) && ':' == text[ length - 3 ] &&
         IsDigit( text[ length - 5 ] ) && IsDigit( text[ length - 4 ] ) && IsDigit( text[ length - 2 ] ) && IsDigit( text[ length - 1 ] ) )
    {
        // an offset behind the time, not the day of a date
        zoned = ( length - 6 ) > separator;
    }
    if ( !zoned )
        return false;

    return CDataTableValues::ParseTimestamp( text, length, unit, ticks );
}

/*-------------------------------------------------------------------------*/

static inline int
Base64Value( unsigned char c )
{
    if ( c >= 'A' && c <= 'Z' ) return c - 'A';
    if ( c >= 'a' && c <= 'z' ) return c - 'a' + 26;
    if ( c >= '0' && c <= '9' ) return c - '0' + 52;
    if ( '+' == c ) return 62;
    if ( '/' == c ) return 63;
    return -1;
}

/*-------------------------------------------------------------------------*/

static const char Base64Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

bool
CCsvTableText::Base64Decode( const char* text, UInt32 length, char* out, UInt32& outLength )
{
    outLength = 0;
    if ( 0 != length % 4 )
        return false;

    UInt32 written = 0;
    for ( UInt32 i = 0; i < length; i += 4 )
    {
        const bool last = ( i + 4 == length );
        int v[ 4 ];
        int padding = 0;
        for ( int n = 0; n < 4; ++n )
        {
            const unsigned char c = (unsigned char) text[ i + n ];
            if ( '=' == c )
            {
                if ( !last || n < 2 )
                    return false;
                ++padding;
                v[ n ] = 0;
            }
            else
            {
                if ( padding > 0 )
                    return false;
                v[ n ] = Base64Value( c );
                if ( v[ n ] < 0 )
                    return false;
            }
        }
        const UInt32 triple = ( (UInt32) v[ 0 ] << 18 ) | ( (UInt32) v[ 1 ] << 12 ) | ( (UInt32) v[ 2 ] << 6 ) | (UInt32) v[ 3 ];
        out[ written++ ] = (char) ( ( triple >> 16 ) & 0xFF );
        if ( padding < 2 )
            out[ written++ ] = (char) ( ( triple >> 8 ) & 0xFF );
        if ( padding < 1 )
            out[ written++ ] = (char) ( triple & 0xFF );
    }
    outLength = written;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CCsvTableText::Base64Encode( const void* bytes, UInt32 length, char* out )
{
    const unsigned char* in = static_cast< const unsigned char* >( bytes );
    UInt32 written = 0;
    UInt32 i = 0;
    for ( ; i + 3 <= length; i += 3 )
    {
        const UInt32 triple = ( (UInt32) in[ i ] << 16 ) | ( (UInt32) in[ i + 1 ] << 8 ) | (UInt32) in[ i + 2 ];
        out[ written++ ] = Base64Alphabet[ ( triple >> 18 ) & 63 ];
        out[ written++ ] = Base64Alphabet[ ( triple >> 12 ) & 63 ];
        out[ written++ ] = Base64Alphabet[ ( triple >> 6 ) & 63 ];
        out[ written++ ] = Base64Alphabet[ triple & 63 ];
    }
    if ( length - i == 1 )
    {
        const UInt32 triple = (UInt32) in[ i ] << 16;
        out[ written++ ] = Base64Alphabet[ ( triple >> 18 ) & 63 ];
        out[ written++ ] = Base64Alphabet[ ( triple >> 12 ) & 63 ];
        out[ written++ ] = '=';
        out[ written++ ] = '=';
    }
    else
    if ( length - i == 2 )
    {
        const UInt32 triple = ( (UInt32) in[ i ] << 16 ) | ( (UInt32) in[ i + 1 ] << 8 );
        out[ written++ ] = Base64Alphabet[ ( triple >> 18 ) & 63 ];
        out[ written++ ] = Base64Alphabet[ ( triple >> 12 ) & 63 ];
        out[ written++ ] = Base64Alphabet[ ( triple >> 6 ) & 63 ];
        out[ written++ ] = '=';
    }
    return written;
}

/*-------------------------------------------------------------------------*/

UInt32
CCsvTableText::FormatUInt64( UInt64 value, char* buffer )
{
    char reversed[ 24 ];
    UInt32 count = 0;
    do
    {
        reversed[ count++ ] = (char) ( '0' + value % 10 );
        value /= 10;
    }
    while ( 0 != value );
    for ( UInt32 i = 0; i < count; ++i )
        buffer[ i ] = reversed[ count - 1 - i ];
    buffer[ count ] = '\0';
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CCsvTableText::FormatInt64( Int64 value, char* buffer )
{
    if ( value < 0 )
    {
        buffer[ 0 ] = '-';
        const UInt64 magnitude = (UInt64) 0 - (UInt64) value;
        return 1 + FormatUInt64( magnitude, buffer + 1 );
    }
    return FormatUInt64( (UInt64) value, buffer );
}

/*-------------------------------------------------------------------------*/

UInt32
CCsvTableText::FormatDecimal( Int64 mantissa, UInt8 scale, char* buffer )
{
    char digits[ 24 ];
    const UInt64 magnitude = mantissa < 0 ? (UInt64) 0 - (UInt64) mantissa : (UInt64) mantissa;
    UInt32 digitCount = FormatUInt64( magnitude, digits );

    UInt32 length = 0;
    if ( mantissa < 0 )
        buffer[ length++ ] = '-';

    if ( 0 == scale )
    {
        memcpy( buffer + length, digits, digitCount );
        length += digitCount;
    }
    else
    {
        // at least one digit in front of the point
        UInt32 leadingZeros = 0;
        if ( digitCount <= scale )
            leadingZeros = scale + 1 - digitCount;
        const UInt32 integerDigits = digitCount + leadingZeros - scale;
        UInt32 source = 0;
        for ( UInt32 i = 0; i < integerDigits; ++i )
            buffer[ length++ ] = ( i < leadingZeros ) ? '0' : digits[ source++ ];
        buffer[ length++ ] = '.';
        for ( UInt32 i = 0; i < scale; ++i )
        {
            const UInt32 position = integerDigits + i;
            buffer[ length++ ] = ( position < leadingZeros ) ? '0' : digits[ source++ ];
        }
    }
    buffer[ length ] = '\0';
    return length;
}

/*-------------------------------------------------------------------------*/

static void
FixLocalePoint( char* buffer, UInt32 length )
{
    const struct lconv* conv = localeconv();
    if ( GUCEF_NULL != conv && GUCEF_NULL != conv->decimal_point && '\0' != conv->decimal_point[ 0 ] && '.' != conv->decimal_point[ 0 ] )
    {
        for ( UInt32 i = 0; i < length; ++i )
            if ( buffer[ i ] == conv->decimal_point[ 0 ] )
                buffer[ i ] = '.';
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CCsvTableText::FormatFloat64( Float64 value, char* buffer )
{
    if ( IsNanValue( (Float64) value ) )
    {
        memcpy( buffer, "NaN", 4 );
        return 3;
    }
    if ( IsInfiniteValue( (Float64) value ) )
    {
        const char* text = value < 0 ? "-Infinity" : "Infinity";
        const UInt32 length = (UInt32) strlen( text );
        memcpy( buffer, text, length + 1 );
        return length;
    }

    // Digits are added until the text reads back as the same value; %g drops the trailing zeros, so this is the shortest form
    // in the usual case of values that were entered with at most 15 digits
    int length = 0;
    for ( int precision = 15; precision <= 17; ++precision )
    {
        length = snprintf( buffer, MaxFloatTextLength, "%.*g", precision, value );
        if ( length <= 0 || (UInt32) length >= MaxFloatTextLength )
            break;
        FixLocalePoint( buffer, (UInt32) length );
        if ( strtod( buffer, GUCEF_NULL ) == value )
            break;
    }
    return length > 0 ? (UInt32) length : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CCsvTableText::FormatFloat32( Float32 value, char* buffer )
{
    if ( IsNanValue( (Float64) value ) )
    {
        memcpy( buffer, "NaN", 4 );
        return 3;
    }
    if ( IsInfiniteValue( (Float64) value ) )
    {
        const char* text = value < 0 ? "-Infinity" : "Infinity";
        const UInt32 length = (UInt32) strlen( text );
        memcpy( buffer, text, length + 1 );
        return length;
    }

    int length = 0;
    for ( int precision = 6; precision <= 9; ++precision )
    {
        length = snprintf( buffer, MaxFloatTextLength, "%.*g", precision, (double) value );
        if ( length <= 0 || (UInt32) length >= MaxFloatTextLength )
            break;
        FixLocalePoint( buffer, (UInt32) length );
        if ( strtof( buffer, GUCEF_NULL ) == value )
            break;
    }
    return length > 0 ? (UInt32) length : 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
