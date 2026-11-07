/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_GUI_CTABLECELLFORMATTER_H
#include "gucefGUI_CTableCellFormatter.h"
#define GUCEF_GUI_CTABLECELLFORMATTER_H
#endif /* GUCEF_GUI_CTABLECELLFORMATTER_H ? */

#include "gucefGUI_CTableCellParser.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
IsSpace( char c )
{
    return ' ' == c || '\t' == c || '\r' == c || '\n' == c;
}

/** The text without white space at both ends */
CString
Trim( const CString& text )
{
    const char* chars = text.C_String();
    size_t begin = 0, end = strlen( chars );
    while ( begin < end && IsSpace( chars[ begin ] ) )
        ++begin;
    while ( end > begin && IsSpace( chars[ end - 1 ] ) )
        --end;
    CString result;
    for ( size_t i=begin; i<end; ++i )
        result += chars[ i ];
    return result;
}

bool
StartsWith( const char* text, const CString& prefix )
{
    const size_t size = strlen( prefix.C_String() );
    return size > 0 && 0 == strncmp( text, prefix.C_String(), size );
}

/**
 *  The number without the prefix, the suffix and the grouping of the column: an optional sign, digits, and for
 *  a float a fraction and an exponent. Empty when the text is not built like a number
 */
CString
CleanNumber( const CString& text, const CTableViewColumn& column, bool integer, bool allowExponent )
{
    CString trimmed = Trim( text );
    const char* chars = trimmed.C_String();
    size_t begin = 0, end = strlen( chars );

    // the sign may stand before or after the prefix: -$5 and $-5
    char sign = '\0';
    if ( begin < end && ( '-' == chars[ begin ] || '+' == chars[ begin ] ) )
        sign = chars[ begin++ ];
    if ( begin < end && StartsWith( chars + begin, column.prefix ) )
        begin += strlen( column.prefix.C_String() );
    while ( begin < end && IsSpace( chars[ begin ] ) )
        ++begin;
    if ( !column.suffix.IsNULLOrEmpty() )
    {
        const size_t suffixSize = strlen( column.suffix.C_String() );
        if ( end - begin >= suffixSize && 0 == strncmp( chars + end - suffixSize, column.suffix.C_String(), suffixSize ) )
            end -= suffixSize;
    }
    while ( end > begin && IsSpace( chars[ end - 1 ] ) )
        --end;
    if ( begin < end && ( '-' == chars[ begin ] || '+' == chars[ begin ] ) )
    {
        if ( '\0' != sign )
            return CString::Empty;
        sign = chars[ begin++ ];
    }

    CString number;
    if ( '-' == sign )
        number += '-';
    bool digits = false, dot = false, exponent = false;
    for ( size_t i=begin; i<end; ++i )
    {
        const char c = chars[ i ];
        if ( c >= '0' && c <= '9' )
        {
            digits = true;
            number += c;
        }
        else
        if ( ',' == c || ' ' == c )
        {
            // grouping; only between digits
            if ( !digits || i + 1 >= end )
                return CString::Empty;
        }
        else
        if ( '.' == c && !dot && !exponent && !integer )
        {
            dot = true;
            number += c;
        }
        else
        if ( ( 'e' == c || 'E' == c ) && allowExponent && digits && !exponent && i + 1 < end )
        {
            exponent = true;
            number += 'e';
            if ( '-' == chars[ i + 1 ] || '+' == chars[ i + 1 ] )
            {
                number += chars[ i + 1 ];
                ++i;
            }
        }
        else
            return CString::Empty;
    }
    if ( !digits )
        return CString::Empty;
    return number;
}

/** Digits of an integer text up to 'limit' (inclusive); false when it does not fit */
bool
ReadMagnitude( const char* digits, UInt64 limit, UInt64& magnitude )
{
    magnitude = 0;
    for ( ; '\0' != *digits; ++digits )
    {
        const UInt64 digit = (UInt64) ( *digits - '0' );
        if ( magnitude > ( limit - digit ) / 10 )
            return false;
        magnitude = magnitude * 10 + digit;
    }
    return true;
}

bool
ParseInteger( const CString& text, const CTableViewColumn& column, CORE::CVariant& value, CString& error )
{
    const CString number = CleanNumber( text, column, true, false );
    if ( number.IsNULLOrEmpty() )
    {
        // 12.0 is a whole number too
        const CString real = CleanNumber( text, column, false, false );
        const char* chars = real.C_String();
        const char* dot = strchr( chars, '.' );
        if ( real.IsNULLOrEmpty() || GUCEF_NULL == dot )
        {
            error = "Not a number";
            return false;
        }
        for ( const char* c = dot + 1; '\0' != *c; ++c )
        {
            if ( '0' != *c )
            {
                error = "Not a whole number";
                return false;
            }
        }
        return ParseInteger( CString( real.SubstrToIndex( (UInt32) ( dot - chars ), true ) ), column, value, error );
    }

    const bool negative = '-' == number.C_String()[ 0 ];
    const char* digits = number.C_String() + ( negative ? 1 : 0 );
    UInt64 low = 0, high = 0;
    bool isSigned = true;
    switch ( column.type )
    {
        case CORE::DATATABLE_TYPE_INT8:    low = 128; high = 127; break;
        case CORE::DATATABLE_TYPE_INT16:   low = 32768; high = 32767; break;
        case CORE::DATATABLE_TYPE_INT32:   low = 2147483648ULL; high = 2147483647ULL; break;
        case CORE::DATATABLE_TYPE_INT64:   low = (UInt64) 9223372036854775808ULL; high = 9223372036854775807ULL; break;
        case CORE::DATATABLE_TYPE_UINT8:   high = 255; isSigned = false; break;
        case CORE::DATATABLE_TYPE_UINT16:  high = 65535; isSigned = false; break;
        case CORE::DATATABLE_TYPE_UINT32:  high = 4294967295ULL; isSigned = false; break;
        default:                           high = 0xFFFFFFFFFFFFFFFFULL; isSigned = false; break;
    }
    UInt64 magnitude = 0;
    if ( negative && !isSigned )
    {
        error = "Must not be negative";
        return false;
    }
    if ( !ReadMagnitude( digits, negative ? low : high, magnitude ) )
    {
        error = "Out of range";
        return false;
    }
    const Int64 signedValue = negative ? (Int64) ( 0 - magnitude ) : (Int64) magnitude;
    switch ( column.type )
    {
        case CORE::DATATABLE_TYPE_INT8:    value = CORE::CVariant( (Int8) signedValue ); break;
        case CORE::DATATABLE_TYPE_INT16:   value = CORE::CVariant( (Int16) signedValue ); break;
        case CORE::DATATABLE_TYPE_INT32:   value = CORE::CVariant( (Int32) signedValue ); break;
        case CORE::DATATABLE_TYPE_INT64:   value = CORE::CVariant( (Int64) signedValue ); break;
        case CORE::DATATABLE_TYPE_UINT8:   value = CORE::CVariant( (UInt8) magnitude ); break;
        case CORE::DATATABLE_TYPE_UINT16:  value = CORE::CVariant( (UInt16) magnitude ); break;
        case CORE::DATATABLE_TYPE_UINT32:  value = CORE::CVariant( (UInt32) magnitude ); break;
        default:                           value = CORE::CVariant( (UInt64) magnitude ); break;
    }
    return true;
}

bool
ParseFloat( const CString& text, const CTableViewColumn& column, CORE::CVariant& value, CString& error )
{
    const CString number = CleanNumber( text, column, false, true );
    if ( number.IsNULLOrEmpty() )
    {
        error = "Not a number";
        return false;
    }
    const Float64 parsed = CORE::StringToDouble( number );
    if ( parsed != parsed || parsed > 1.0e308 || parsed < -1.0e308 )
    {
        error = "Out of range";
        return false;
    }
    if ( CORE::DATATABLE_TYPE_FLOAT32 == column.type )
        value = CORE::CVariant( (Float32) parsed );
    else
        value = CORE::CVariant( (Float64) parsed );
    return true;
}

bool
ParseDecimal( const CString& text, const CTableViewColumn& column, CORE::CVariant& value, CString& error )
{
    const CString number = CleanNumber( text, column, false, false );
    CORE::CDecimal decimal;
    if ( number.IsNULLOrEmpty() || CORE::DecimalStatusIsError( CORE::CDecimal::FromString( number, decimal ) ) )
    {
        error = "Not a number";
        return false;
    }
    value = CORE::CDataTableValues::Decimal( decimal );
    return true;
}

bool
ParseBoolean( const CString& text, CORE::CVariant& value, CString& error )
{
    const CString lower = Trim( text ).Lowercase();
    if ( "true" == lower || "yes" == lower || "1" == lower || "y" == lower || "x" == lower || "on" == lower )
        value = CORE::CVariant( true );
    else
    if ( "false" == lower || "no" == lower || "0" == lower || "n" == lower || "off" == lower )
        value = CORE::CVariant( false );
    else
    {
        error = "Not true or false";
        return false;
    }
    return true;
}

/** Reads 'minimum' to 'maximum' digits, false when there are fewer than 'minimum' */
bool
ReadDigits( const char*& at, UInt32 minimum, UInt32 maximum, UInt32& number )
{
    number = 0;
    UInt32 count = 0;
    while ( count < maximum && *at >= '0' && *at <= '9' )
    {
        number = number * 10 + (UInt32) ( *at - '0' );
        ++at;
        ++count;
    }
    return count >= minimum;
}

struct SDateParts
{
    UInt32 year, month, day, hour, minute, second, milliseconds;

    SDateParts( void ) : year( 1970 ), month( 1 ), day( 1 ), hour( 0 ), minute( 0 ), second( 0 ), milliseconds( 0 ) {}
};

/** The parts of the text by the pattern the formatter understands (yyyy, MM, dd, HH, mm, ss, fff and other characters as they are) */
bool
ParsePattern( const CString& text, const CString& pattern, SDateParts& parts )
{
    const char* at = text.C_String();
    const char* format = pattern.C_String();
    const size_t size = strlen( format );
    for ( size_t i=0; i<size; )
    {
        UInt32 number = 0;
        if ( 0 == strncmp( format + i, "yyyy", 4 ) )       { if ( !ReadDigits( at, 4, 4, number ) ) return false; parts.year = number; i += 4; }
        else
        if ( 0 == strncmp( format + i, "MM", 2 ) )         { if ( !ReadDigits( at, 1, 2, number ) ) return false; parts.month = number; i += 2; }
        else
        if ( 0 == strncmp( format + i, "dd", 2 ) )         { if ( !ReadDigits( at, 1, 2, number ) ) return false; parts.day = number; i += 2; }
        else
        if ( 0 == strncmp( format + i, "HH", 2 ) )         { if ( !ReadDigits( at, 1, 2, number ) ) return false; parts.hour = number; i += 2; }
        else
        if ( 0 == strncmp( format + i, "mm", 2 ) )         { if ( !ReadDigits( at, 1, 2, number ) ) return false; parts.minute = number; i += 2; }
        else
        if ( 0 == strncmp( format + i, "ss", 2 ) )         { if ( !ReadDigits( at, 1, 2, number ) ) return false; parts.second = number; i += 2; }
        else
        if ( 0 == strncmp( format + i, "fff", 3 ) )        { if ( !ReadDigits( at, 3, 3, number ) ) return false; parts.milliseconds = number; i += 3; }
        else
        {
            if ( *at != format[ i ] )
                return false;
            ++at;
            ++i;
        }
    }
    return '\0' == *at;
}

bool
ParseDateValue( const CString& text, const CTableViewColumn& column, CORE::CVariant& value, CString& error )
{
    const CString trimmed = Trim( text );
    Int32 days = 0;
    SDateParts parts;
    const CString pattern = column.dateFormat.IsNULLOrEmpty() ? CString( "yyyy-MM-dd" ) : column.dateFormat;
    if ( ParsePattern( trimmed, pattern, parts ) )
    {
        if ( !CORE::CDataTableValues::DaysFromCivil( (Int32) parts.year, parts.month, parts.day, days ) )
        {
            error = "Not a date";
            return false;
        }
    }
    else
    if ( !CORE::CDataTableValues::ParseDate( trimmed.C_String(), (UInt32) strlen( trimmed.C_String() ), days ) )
    {
        error = CString( "Not a date, use " ) + pattern;
        return false;
    }
    value = CORE::CDataTableValues::Date( days );
    return true;
}

bool
ParseTimestampValue( const CString& text, const CTableViewColumn& column, const CORE::CVariant* current, CORE::CVariant& value, CString& error )
{
    CORE::TDataTableTimeUnit unit = CORE::DATATABLE_TIMEUNIT_MILLISECONDS;
    if ( GUCEF_NULL != current )
        CORE::CDataTableValues::TimeUnitFromTypeId( current->GetTypeId(), unit );
    const CString trimmed = Trim( text );
    const CString pattern = column.dateFormat.IsNULLOrEmpty() ? CString( "yyyy-MM-dd HH:mm:ss" ) : column.dateFormat;
    Int64 ticks = 0;
    SDateParts parts;
    if ( ParsePattern( trimmed, pattern, parts ) )
    {
        Int32 days = 0;
        if ( !CORE::CDataTableValues::DaysFromCivil( (Int32) parts.year, parts.month, parts.day, days ) || parts.hour > 23 || parts.minute > 59 || parts.second > 59 )
        {
            error = "Not a date and time";
            return false;
        }
        const Int64 perSecond = CORE::CDataTableValues::TicksPerSecond( unit );
        const Int64 seconds = (Int64) days * 86400 + (Int64) parts.hour * 3600 + (Int64) parts.minute * 60 + (Int64) parts.second;
        ticks = seconds * perSecond + (Int64) parts.milliseconds * ( perSecond / 1000 );
    }
    else
    if ( !CORE::CDataTableValues::ParseTimestamp( trimmed.C_String(), (UInt32) strlen( trimmed.C_String() ), unit, ticks ) )
    {
        error = CString( "Not a date and time, use " ) + pattern;
        return false;
    }
    value = CORE::CDataTableValues::Timestamp( ticks, unit );
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CTableCellParser::IsEditableType( const CTableViewColumn& column )
{GUCEF_TRACE;

    return CORE::DATATABLE_TYPE_BINARY != column.type && CORE::DATATABLE_TYPE_UNSUPPORTED != column.type;
}

/*-------------------------------------------------------------------------*/

CString
CTableCellParser::GetTypeHint( const CTableViewColumn& column )
{GUCEF_TRACE;

    switch ( column.type )
    {
        case CORE::DATATABLE_TYPE_BOOL:         return "true or false";
        case CORE::DATATABLE_TYPE_DATE:         return column.dateFormat.IsNULLOrEmpty() ? CString( "yyyy-MM-dd" ) : column.dateFormat;
        case CORE::DATATABLE_TYPE_TIMESTAMP:    return column.dateFormat.IsNULLOrEmpty() ? CString( "yyyy-MM-dd HH:mm:ss" ) : column.dateFormat;
        case CORE::DATATABLE_TYPE_UTF8:         return "text";
        default:
            return column.IsNumeric() ? CString( "number" ) : CString( "text" );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CVariant
CTableCellParser::EmptyValue( const CTableViewColumn& column )
{GUCEF_TRACE;

    if ( CORE::DATATABLE_TYPE_UTF8 == column.type )
        return CORE::CVariant( CString::Empty );
    CORE::CVariant empty;
    empty.Set( GUCEF_NULL, 0, GUCEF_DATATYPE_NULL, false );
    return empty;
}

/*-------------------------------------------------------------------------*/

CString
CTableCellParser::FormatEditText( const CORE::CVariant& value, const CTableViewColumn& column )
{GUCEF_TRACE;

    if ( value.IsNULL() || value.IsNIL() )
        return CString::Empty;
    CTableViewColumn plain = column;
    plain.thousandsSeparator = false;
    plain.prefix.Clear();
    plain.suffix.Clear();
    plain.decimals = -1;
    plain.booleanDisplay = CTableViewColumn::BOOLEAN_TEXT;
    return CTableCellFormatter::FormatText( value, plain );
}

/*-------------------------------------------------------------------------*/

bool
CTableCellParser::Parse( const CString& text, const CTableViewColumn& column, const CORE::CVariant* current, CORE::CVariant& value, CString& error )
{GUCEF_TRACE;

    error.Clear();
    if ( CORE::DATATABLE_TYPE_UTF8 == column.type )
    {
        value = CORE::CVariant( text );
        return true;
    }
    if ( !IsEditableType( column ) )
    {
        error = "This kind of value cannot be edited";
        return false;
    }
    if ( Trim( text ).IsNULLOrEmpty() )
    {
        value = EmptyValue( column );
        return true;
    }
    switch ( column.type )
    {
        case CORE::DATATABLE_TYPE_BOOL:         return ParseBoolean( text, value, error );
        case CORE::DATATABLE_TYPE_INT8:
        case CORE::DATATABLE_TYPE_INT16:
        case CORE::DATATABLE_TYPE_INT32:
        case CORE::DATATABLE_TYPE_INT64:
        case CORE::DATATABLE_TYPE_UINT8:
        case CORE::DATATABLE_TYPE_UINT16:
        case CORE::DATATABLE_TYPE_UINT32:
        case CORE::DATATABLE_TYPE_UINT64:       return ParseInteger( text, column, value, error );
        case CORE::DATATABLE_TYPE_FLOAT32:
        case CORE::DATATABLE_TYPE_FLOAT64:      return ParseFloat( text, column, value, error );
        case CORE::DATATABLE_TYPE_DECIMAL:      return ParseDecimal( text, column, value, error );
        case CORE::DATATABLE_TYPE_DATE:         return ParseDateValue( text, column, value, error );
        case CORE::DATATABLE_TYPE_TIMESTAMP:    return ParseTimestampValue( text, column, current, value, error );
        default:
            break;
    }
    error = "This kind of value cannot be edited";
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
