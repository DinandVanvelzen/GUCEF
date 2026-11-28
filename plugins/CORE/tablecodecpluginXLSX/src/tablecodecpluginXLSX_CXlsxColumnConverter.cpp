/*
 *  tablecodecpluginXLSX: GUCEF table codec plugin for Microsoft Excel (.xlsx) workbooks
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

#include "tablecodecpluginXLSX_CXlsxColumnConverter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TABLECODECPLUGIN {
namespace XLSX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 NumberParseFlags = CORE::DECIMALPARSE_ALLOW_EXPONENT | CORE::DECIMALPARSE_ALLOW_SURROUNDING_WHITESPACE;

/*-------------------------------------------------------------------------*/

static CORE::CString
ShowText( const char* text, UInt32 length )
{
    if ( length > 40 )
        return "\"" + CORE::CString( text, 40 ) + "...\"";
    return "\"" + CORE::CString( text, length ) + "\"";
}

/*-------------------------------------------------------------------------*/

static UInt64
PowerOfTen( UInt32 exponent )
{
    UInt64 result = 1;
    for ( UInt32 i = 0; i < exponent && i < 19; ++i )
        result *= 10;
    return result;
}

/*-------------------------------------------------------------------------*/

/** Division that rounds down also for negative numbers */
static Int64
FloorDivide( Int64 value, Int64 divisor )
{
    Int64 quotient = value / divisor;
    if ( ( value % divisor ) != 0 && ( value < 0 ) != ( divisor < 0 ) )
        --quotient;
    return quotient;
}

/*-------------------------------------------------------------------------*/

static bool
TextEqualsIgnoreCase( const char* text, UInt32 length, const char* expected )
{
    if ( strlen( expected ) != length )
        return false;
    for ( UInt32 i = 0; i < length; ++i )
    {
        char c = text[ i ];
        if ( c >= 'A' && c <= 'Z' )
            c = (char) ( c - 'A' + 'a' );
        if ( c != expected[ i ] )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

SXlsxNumberParts::SXlsxNumberParts( void )
    : negative( false )
    , whole( 0 )
    , fractionCount( 0 )
    , fractionIsZero( true )
{GUCEF_TRACE;

    memset( fraction, '0', sizeof( fraction ) );
}

/*-------------------------------------------------------------------------*/

CXlsxColumnConverter::CXlsxColumnConverter( const SXlsxReadOptions& options, bool date1904 )
    : m_options( options )
    , m_date1904( date1904 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CXlsxColumnConverter::SplitNumber( const char* text, UInt32 length, SXlsxNumberParts& parts )
{GUCEF_TRACE;

    parts = SXlsxNumberParts();
    while ( length > 0 && ( ' ' == text[ 0 ] || '\t' == text[ 0 ] ) ) { ++text; --length; }
    while ( length > 0 && ( ' ' == text[ length - 1 ] || '\t' == text[ length - 1 ] ) ) --length;
    if ( 0 == length )
        return false;

    bool hasExponent = false;
    for ( UInt32 i = 0; i < length; ++i )
        if ( 'e' == text[ i ] || 'E' == text[ i ] )
            hasExponent = true;

    if ( hasExponent )
    {
        CORE::CDecimal value;
        if ( CORE::DecimalStatusIsError( CORE::CDecimal::FromChars( text, length, value, NumberParseFlags ) ) )
            return false;
        parts.negative = value.IsNegative();
        const Int64 unscaled = value.GetUnscaledValue();
        const UInt64 magnitude = unscaled < 0 ? (UInt64) ( -unscaled ) : (UInt64) unscaled;
        const UInt32 scale = value.GetScale();
        const UInt64 divisor = PowerOfTen( scale );
        parts.whole = magnitude / divisor;
        UInt64 remainder = magnitude % divisor;
        parts.fractionIsZero = 0 == remainder;
        parts.fractionCount = scale < 15 ? scale : 15;
        char digits[ 24 ];
        for ( UInt32 i = scale; i > 0; --i )
        {
            digits[ i - 1 ] = (char) ( '0' + remainder % 10 );
            remainder /= 10;
        }
        for ( UInt32 i = 0; i < parts.fractionCount; ++i )
            parts.fraction[ i ] = digits[ i ];
        return true;
    }

    UInt32 pos = 0;
    if ( '-' == text[ pos ] || '+' == text[ pos ] )
    {
        parts.negative = '-' == text[ pos ];
        ++pos;
    }
    UInt32 wholeDigits = 0;
    for ( ; pos < length && text[ pos ] >= '0' && text[ pos ] <= '9'; ++pos, ++wholeDigits )
    {
        const UInt64 digit = (UInt64) ( text[ pos ] - '0' );
        if ( parts.whole > ( GUCEF_UINT64MAX - digit ) / 10 )
            return false;
        parts.whole = parts.whole * 10 + digit;
    }
    UInt32 fractionDigits = 0;
    if ( pos < length && '.' == text[ pos ] )
    {
        ++pos;
        for ( ; pos < length && text[ pos ] >= '0' && text[ pos ] <= '9'; ++pos, ++fractionDigits )
        {
            if ( fractionDigits < 15 )
            {
                parts.fraction[ fractionDigits ] = text[ pos ];
                ++parts.fractionCount;
            }
            if ( '0' != text[ pos ] )
                parts.fractionIsZero = false;
        }
    }
    return pos == length && ( wholeDigits > 0 || fractionDigits > 0 );
}

/*-------------------------------------------------------------------------*/

bool
CXlsxColumnConverter::ConvertText( const CORE::CDataTableColumnDef& column, const SXlsxCell& cell, CORE::CDataTableColumnBuilder& builder, CORE::CString& reason ) const
{GUCEF_TRACE;

    switch ( cell.kind )
    {
        case XLSX_CELL_STRING:
        case XLSX_CELL_ISODATE:
            return builder.AppendUtf8( cell.text, cell.length );
        case XLSX_CELL_BOOL:
            return builder.AppendString( cell.boolValue ? "true" : "false" );
        case XLSX_CELL_NUMBER:
        {
            if ( XLSX_NUMBERKIND_DATE == cell.numberKind || XLSX_NUMBERKIND_DATETIME == cell.numberKind )
            {
                SXlsxNumberParts parts;
                Int32 days = 0;
                if ( SplitNumber( cell.text, cell.length, parts ) && !parts.negative && parts.whole < 10000000 && CXlsxDates::SerialToDays( (Int64) parts.whole, m_date1904, days ) )
                {
                    Int64 micros = parts.fractionIsZero ? 0 : CXlsxDates::FractionToMicros( parts.fraction, parts.fractionCount );
                    if ( micros >= CXlsxDates::MicrosPerDay )
                    {
                        micros = 0;
                        ++days;
                    }
                    return builder.AppendString( CXlsxDates::ToIsoText( days, micros ) );
                }
            }
            return builder.AppendUtf8( cell.text, cell.length );
        }
        default:
            break;
    }
    reason = "the cell has no text";
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CXlsxColumnConverter::ConvertDate( const CORE::CDataTableColumnDef& column, const SXlsxCell& cell, CORE::CDataTableColumnBuilder& builder, CORE::CString& reason ) const
{GUCEF_TRACE;

    Int32 days = 0;
    Int64 micros = 0;

    if ( XLSX_CELL_NUMBER == cell.kind )
    {
        SXlsxNumberParts parts;
        if ( !SplitNumber( cell.text, cell.length, parts ) )
        {
            reason = "the text " + ShowText( cell.text, cell.length ) + " is no number";
            return false;
        }
        if ( parts.negative || parts.whole > 10000000 || !CXlsxDates::SerialToDays( (Int64) parts.whole, m_date1904, days ) )
        {
            reason = "the number " + ShowText( cell.text, cell.length ) + " is no date in the " + ( m_date1904 ? "1904" : "1900" ) + " date system";
            return false;
        }
        micros = parts.fractionIsZero ? 0 : CXlsxDates::FractionToMicros( parts.fraction, parts.fractionCount );
        if ( micros >= CXlsxDates::MicrosPerDay )
        {
            micros = 0;
            ++days;
            if ( days > CXlsxDates::MaxDays )
            {
                reason = "the number " + ShowText( cell.text, cell.length ) + " is past the year 9999";
                return false;
            }
        }
    }
    else
    if ( XLSX_CELL_ISODATE == cell.kind || XLSX_CELL_STRING == cell.kind )
    {
        bool hasTime = false;
        if ( !CXlsxDates::ParseIso( cell.text, cell.length, days, micros, hasTime ) )
        {
            reason = "the text " + ShowText( cell.text, cell.length ) + " is no date of the form yyyy-mm-dd or yyyy-mm-ddThh:mm:ss";
            return false;
        }
    }
    else
    {
        reason = "the cell holds no date";
        return false;
    }

    if ( CORE::DATATABLE_TYPE_DATE == column.type )
    {
        if ( 0 != micros )
        {
            reason = "the value " + ShowText( cell.text, cell.length ) + " has a time of day, a date column needs whole days";
            return false;
        }
        if ( !builder.AppendDate( days ) )
        {
            reason = "the date does not fit the column";
            return false;
        }
        return true;
    }

    const Int64 totalMicros = (Int64) days * CXlsxDates::MicrosPerDay + micros;
    Int64 ticks = totalMicros;
    if ( CORE::DATATABLE_TIMEUNIT_MILLISECONDS == column.timeUnit )
        ticks = FloorDivide( totalMicros + 500, 1000 );
    else
    if ( CORE::DATATABLE_TIMEUNIT_NANOSECONDS == column.timeUnit )
    {
        if ( totalMicros > GUCEF_INT64MAX / 1000 || totalMicros < GUCEF_INT64MIN / 1000 )
        {
            reason = "the time " + ShowText( cell.text, cell.length ) + " does not fit a timestamp in nanoseconds";
            return false;
        }
        ticks = totalMicros * 1000;
    }
    if ( !builder.AppendTimestamp( ticks ) )
    {
        reason = "the time does not fit the column";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CXlsxColumnConverter::ConvertNumber( const CORE::CDataTableColumnDef& column, const SXlsxCell& cell, CORE::CDataTableColumnBuilder& builder, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( XLSX_CELL_NUMBER != cell.kind && XLSX_CELL_STRING != cell.kind )
    {
        reason = "the cell holds no number";
        return false;
    }

    const char* text = cell.text;
    const UInt32 length = cell.length;

    switch ( column.type )
    {
        case CORE::DATATABLE_TYPE_DECIMAL:
        {
            CORE::CDecimal value;
            CORE::TDecimalStatus status = 0;
            if ( m_options.roundDecimals )
                status = CORE::CDecimal::FromChars( text, length, column.decimalScale, m_options.decimalRounding, value, NumberParseFlags );
            else
                status = CORE::CDecimal::FromChars( text, length, value, NumberParseFlags );
            if ( CORE::DecimalStatusIsError( status ) )
            {
                if ( 0 != ( status & CORE::DECIMALSTATUS_INVALID_FORMAT ) )
                    reason = "the text " + ShowText( text, length ) + " is no number";
                else
                    reason = "the number " + ShowText( text, length ) + " has more digits than a decimal holds (18)";
                return false;
            }
            if ( !builder.AppendDecimal( value ) )
            {
                reason = "the number " + ShowText( text, length ) + " does not fit " + column.GetTypeText() + " (more digits than the column holds" +
                         ( m_options.roundDecimals ? "" : ", decimalRounding is none" ) + ")";
                return false;
            }
            return true;
        }
        case CORE::DATATABLE_TYPE_FLOAT32:
        case CORE::DATATABLE_TYPE_FLOAT64:
        {
            CORE::CDecimal value;
            Float64 result = 0;
            if ( !CORE::DecimalStatusIsError( CORE::CDecimal::FromChars( text, length, value, NumberParseFlags ) ) )
                result = value.ToFloat64();
            else
            {
                SXlsxNumberParts parts;
                if ( !SplitNumber( text, length, parts ) )
                {
                    reason = "the text " + ShowText( text, length ) + " is no number";
                    return false;
                }
                result = CORE::StringToDouble( CORE::CString( text, length ), 0 );
            }
            if ( !builder.AppendFloat64( result ) )
            {
                reason = "the number " + ShowText( text, length ) + " does not fit " + column.GetTypeText() + " exactly";
                return false;
            }
            return true;
        }
        default:
            break;
    }

    // the integer columns
    CORE::CDecimal value;
    const CORE::TDecimalStatus status = CORE::CDecimal::FromChars( text, length, value, NumberParseFlags );
    if ( CORE::DecimalStatusIsError( status ) )
    {
        // 19 or 20 digits: a UInt64 holds what a decimal does not
        SXlsxNumberParts parts;
        if ( CORE::DATATABLE_TYPE_UINT64 == column.type && SplitNumber( text, length, parts ) && !parts.negative && parts.fractionIsZero && 0 != ( status & CORE::DECIMALSTATUS_OVERFLOW ) )
        {
            if ( !builder.AppendUInt64( parts.whole ) )
            {
                reason = "the number " + ShowText( text, length ) + " does not fit " + column.GetTypeText();
                return false;
            }
            return true;
        }
        reason = "the text " + ShowText( text, length ) + " is no whole number that fits an integer";
        return false;
    }
    Int64 integer = 0;
    if ( !value.IsIntegral() || CORE::DECIMALSTATUS_OK != value.ToInt64( CORE::ROUNDINGMODE_UNNECESSARY, integer ) )
    {
        reason = "the number " + ShowText( text, length ) + " is not a whole number";
        return false;
    }
    bool appended = false;
    if ( CORE::DataTableLogicalTypeIsUnsigned( column.type ) )
    {
        if ( integer < 0 )
        {
            reason = "the number " + ShowText( text, length ) + " is negative, the column is " + column.GetTypeText();
            return false;
        }
        appended = builder.AppendUInt64( (UInt64) integer );
    }
    else
        appended = builder.AppendInt64( integer );
    if ( !appended )
    {
        reason = "the number " + ShowText( text, length ) + " does not fit " + column.GetTypeText();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CXlsxColumnConverter::ConvertValue( const CORE::CDataTableColumnDef& column, const SXlsxCell& cell, CORE::CDataTableColumnBuilder& builder, CORE::CString& reason ) const
{GUCEF_TRACE;

    if ( XLSX_CELL_ERROR == cell.kind )
    {
        reason = "the cell holds the error value " + ShowText( cell.text, cell.length );
        return false;
    }
    if ( XLSX_CELL_UNCACHED_FORMULA == cell.kind )
    {
        reason = "the cell is a formula without a cached value: the workbook was never calculated";
        return false;
    }
    if ( XLSX_CELL_STRING == cell.kind && m_options.IsNullToken( cell.text, cell.length ) )
        return builder.AppendNull();

    switch ( column.type )
    {
        case CORE::DATATABLE_TYPE_UTF8:
        {
            return ConvertText( column, cell, builder, reason );
        }
        case CORE::DATATABLE_TYPE_BOOL:
        {
            if ( XLSX_CELL_BOOL == cell.kind )
                return builder.AppendBool( cell.boolValue );
            if ( XLSX_CELL_STRING == cell.kind || XLSX_CELL_NUMBER == cell.kind )
            {
                if ( TextEqualsIgnoreCase( cell.text, cell.length, "true" ) || TextEqualsIgnoreCase( cell.text, cell.length, "1" ) )
                    return builder.AppendBool( true );
                if ( TextEqualsIgnoreCase( cell.text, cell.length, "false" ) || TextEqualsIgnoreCase( cell.text, cell.length, "0" ) )
                    return builder.AppendBool( false );
            }
            reason = "the cell holds no boolean";
            return false;
        }
        case CORE::DATATABLE_TYPE_DATE:
        case CORE::DATATABLE_TYPE_TIMESTAMP:
        {
            return ConvertDate( column, cell, builder, reason );
        }
        case CORE::DATATABLE_TYPE_UNSUPPORTED:
        case CORE::DATATABLE_TYPE_BINARY:
        {
            reason = "the column type cannot be read from a worksheet";
            return false;
        }
        default:
            break;
    }
    return ConvertNumber( column, cell, builder, reason );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxColumnConverter::Convert( const CORE::CDataTableColumnDef& column, const SXlsxCell& cell, CORE::CDataTableColumnBuilder& builder,
                               UInt64& valueErrors, CORE::CString& error ) const
{GUCEF_TRACE;

    CORE::CString reason;
    if ( ConvertValue( column, cell, builder, reason ) )
        return CORE::DATATABLE_STATUS_OK;

    if ( reason.IsNULLOrEmpty() )
        reason = "the value does not fit the column";

    if ( !m_options.failOnValueError && builder.AppendNull() )
    {
        ++valueErrors;
        return CORE::DATATABLE_STATUS_OK;
    }
    error = reason;
    return CORE::DATATABLE_STATUS_TYPE_MISMATCH;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxColumnConverter::ConvertBlank( const CORE::CDataTableColumnDef& column, CORE::CDataTableColumnBuilder& builder, CORE::CString& error ) const
{GUCEF_TRACE;

    if ( builder.AppendNull() )
        return CORE::DATATABLE_STATUS_OK;
    error = "the cell is empty but the column is not nullable";
    return CORE::DATATABLE_STATUS_TYPE_MISMATCH;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace XLSX */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
