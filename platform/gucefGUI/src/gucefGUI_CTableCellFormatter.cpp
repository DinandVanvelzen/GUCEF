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

#include <math.h>
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

#include "gucefGUI_CTableCellFormatter.h"

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

/** The scale the largest decimal can have */
const UInt32 MaxDecimals = 18;

bool
IsDateVariant( const CORE::CVariant& value )
{
    const UInt8 id = value.GetTypeId();
    return GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == id || GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == id;
}

bool
IsTimestampVariant( const CORE::CVariant& value, CORE::TDataTableTimeUnit& unit )
{
    return CORE::CDataTableValues::TimeUnitFromTypeId( value.GetTypeId(), unit );
}

/** The digits of a non negative number with at least 'width' digits */
void
AppendNumber( CString& text, Int64 number, UInt32 width )
{
    char digits[ 24 ];
    UInt32 count = 0;
    if ( number < 0 )
        number = 0;
    do
    {
        digits[ count++ ] = (char) ( '0' + number % 10 );
        number /= 10;
    }
    while ( number > 0 && count < sizeof( digits ) );
    while ( count < width && count < sizeof( digits ) )
        digits[ count++ ] = '0';
    while ( count > 0 )
        text += digits[ --count ];
}

/** Floor division and the matching modulo, for negative tick counts */
Int64
FloorDiv( Int64 a, Int64 b )
{
    Int64 q = a / b;
    if ( ( a % b != 0 ) && ( ( a < 0 ) != ( b < 0 ) ) )
        --q;
    return q;
}

/** The shortest text of a float that reads back as the same value */
CString
NaturalFloatText( Float64 number, bool single )
{
    if ( number != number )
        return "NaN";
    if ( number > 1.0e300 || number < -1.0e300 )
        return number > 0.0 ? "Inf" : "-Inf";
    if ( fabs( number ) < 1.0e15 )
    {
        for ( UInt32 scale=0; scale<=MaxDecimals; ++scale )
        {
            CORE::CDecimal decimal;
            if ( CORE::DecimalStatusIsError( CORE::CDecimal::FromFloat64( number, (UInt8) scale, CORE::ROUNDINGMODE_HALF_EVEN, decimal ) ) )
                break;
            const Float64 back = decimal.ToFloat64();
            const bool same = single ? ( (Float32) back == (Float32) number ) : ( back == number );
            if ( same )
                return decimal.ToString();
        }
    }
    CString text = CORE::DoubleToString( number );
    // the text of a big number or one with too many digits for a decimal: without trailing zeros
    if ( text.HasSubstr( "." ) > 0 )
    {
        UInt32 end = text.Length();
        while ( end > 1 && '0' == text.C_String()[ end - 1 ] )
            --end;
        if ( end > 1 && '.' == text.C_String()[ end - 1 ] )
            --end;
        text = text.SubstrToIndex( end, true );
    }
    return text;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CString
CTableCellFormatter::GroupDigits( const CString& number )
{GUCEF_TRACE;

    const char* chars = number.C_String();
    const UInt32 size = (UInt32) strlen( chars );
    UInt32 start = ( size > 0 && ( '-' == chars[ 0 ] || '+' == chars[ 0 ] ) ) ? 1 : 0;
    UInt32 end = start;
    while ( end < size && chars[ end ] >= '0' && chars[ end ] <= '9' )
        ++end;
    const UInt32 digits = end - start;
    if ( digits <= 3 )
        return number;

    CString result;
    for ( UInt32 i=0; i<start; ++i )
        result += chars[ i ];
    for ( UInt32 i=0; i<digits; ++i )
    {
        if ( i > 0 && 0 == ( digits - i ) % 3 )
            result += ',';
        result += chars[ start + i ];
    }
    for ( UInt32 i=end; i<size; ++i )
        result += chars[ i ];
    return result;
}

/*-------------------------------------------------------------------------*/

CString
CTableCellFormatter::FormatDate( Int32 days, const CString& pattern )
{GUCEF_TRACE;

    return FormatTimestamp( (Int64) days * 86400000, CORE::DATATABLE_TIMEUNIT_MILLISECONDS, pattern.IsNULLOrEmpty() ? CString( "yyyy-MM-dd" ) : pattern );
}

/*-------------------------------------------------------------------------*/

CString
CTableCellFormatter::FormatTimestamp( Int64 ticks, CORE::TDataTableTimeUnit unit, const CString& pattern )
{GUCEF_TRACE;

    const Int64 perSecond = CORE::CDataTableValues::TicksPerSecond( unit );
    const Int64 perDay = perSecond * 86400;
    const Int64 dayCount = FloorDiv( ticks, perDay );
    const Int64 inDay = ticks - dayCount * perDay;
    const Int64 secondOfDay = inDay / perSecond;
    const Int64 milliseconds = ( inDay % perSecond ) * 1000 / perSecond;
    Int32 year = 0;
    UInt32 month = 1, day = 1;
    CORE::CDataTableValues::CivilFromDays( (Int32) dayCount, year, month, day );

    const CString& format = pattern.IsNULLOrEmpty() ? CString( "yyyy-MM-dd HH:mm:ss" ) : pattern;
    const char* chars = format.C_String();
    const UInt32 size = (UInt32) strlen( chars );
    CString text;
    for ( UInt32 i=0; i<size; )
    {
        if ( 0 == strncmp( chars + i, "yyyy", 4 ) )
        {
            if ( year < 0 )
                text += '-';
            AppendNumber( text, year < 0 ? -(Int64) year : (Int64) year, 4 );
            i += 4;
        }
        else
        if ( 0 == strncmp( chars + i, "MM", 2 ) )         { AppendNumber( text, month, 2 ); i += 2; }
        else
        if ( 0 == strncmp( chars + i, "dd", 2 ) )         { AppendNumber( text, day, 2 ); i += 2; }
        else
        if ( 0 == strncmp( chars + i, "HH", 2 ) )         { AppendNumber( text, secondOfDay / 3600, 2 ); i += 2; }
        else
        if ( 0 == strncmp( chars + i, "mm", 2 ) )         { AppendNumber( text, ( secondOfDay / 60 ) % 60, 2 ); i += 2; }
        else
        if ( 0 == strncmp( chars + i, "ss", 2 ) )         { AppendNumber( text, secondOfDay % 60, 2 ); i += 2; }
        else
        if ( 0 == strncmp( chars + i, "fff", 3 ) )        { AppendNumber( text, milliseconds, 3 ); i += 3; }
        else
        {
            text += chars[ i ];
            ++i;
        }
    }
    return text;
}

/*-------------------------------------------------------------------------*/

void
CTableCellFormatter::Format( const CORE::CVariant& value, const CTableViewColumn& column, SCellContent& content )
{GUCEF_TRACE;

    content.kind = SCellContent::KIND_TEXT;
    content.text.Clear();
    content.isNumber = false;

    if ( value.IsNULL() || value.IsNIL() )
    {
        content.kind = SCellContent::KIND_NULL;
        content.text = column.nullText;
        return;
    }

    CORE::TDataTableTimeUnit unit = CORE::DATATABLE_TIMEUNIT_MILLISECONDS;
    CString text;
    bool number = false;
    if ( CORE::DATATABLE_TYPE_BOOL == column.type || value.IsBoolean() )
    {
        const bool on = value.AsBool();
        if ( CTableViewColumn::BOOLEAN_CHECK == column.booleanDisplay )
        {
            content.kind = on ? SCellContent::KIND_CHECK_ON : SCellContent::KIND_CHECK_OFF;
            content.text = on ? "true" : "false";
            return;
        }
        text = on ? "true" : "false";
    }
    else
    if ( IsDateVariant( value ) )
    {
        Int64 ticks = 0;
        if ( CORE::CDataTableValues::TicksFromVariant( value, CORE::DATATABLE_TIMEUNIT_MILLISECONDS, ticks ) )
            text = FormatDate( (Int32) FloorDiv( ticks, 86400000 ), column.dateFormat );
        else
            text = value.AsString();
    }
    else
    if ( IsTimestampVariant( value, unit ) )
    {
        Int64 ticks = 0;
        if ( CORE::CDataTableValues::TicksFromVariant( value, unit, ticks ) )
            text = FormatTimestamp( ticks, unit, column.dateFormat );
        else
            text = value.AsString();
    }
    else
    if ( value.IsDecimal() )
    {
        const CORE::CDecimal decimal = value.AsDecimal();
        if ( column.decimals >= 0 )
        {
            const UInt8 scale = (UInt8) ( (UInt32) column.decimals > MaxDecimals ? MaxDecimals : (UInt32) column.decimals );
            if ( CORE::DecimalStatusIsError( decimal.ToString( scale, CORE::ROUNDINGMODE_HALF_UP, text ) ) )
                text = decimal.ToString();
        }
        else
            text = decimal.ToString();
        number = true;
    }
    else
    if ( value.IsFloat() )
    {
        const Float64 floating = value.AsFloat64();
        const bool single = CORE::DATATABLE_TYPE_FLOAT32 == column.type || GUCEF_DATATYPE_FLOAT32 == value.GetTypeId();
        if ( column.decimals >= 0 && floating == floating && fabs( floating ) < 1.0e15 )
        {
            const UInt8 scale = (UInt8) ( (UInt32) column.decimals > MaxDecimals ? MaxDecimals : (UInt32) column.decimals );
            CORE::CDecimal decimal;
            if ( !CORE::DecimalStatusIsError( CORE::CDecimal::FromFloat64( floating, scale, CORE::ROUNDINGMODE_HALF_UP, decimal ) ) )
                text = decimal.ToString();
            else
                text = NaturalFloatText( floating, single );
        }
        else
            text = NaturalFloatText( floating, single );
        number = true;
    }
    else
    if ( value.IsUnsignedInteger() )
    {
        text = CORE::UInt64ToString( value.AsUInt64() );
        number = true;
    }
    else
    if ( value.IsInteger() )
    {
        text = CORE::Int64ToString( value.AsInt64() );
        number = true;
    }
    else
    if ( value.IsBinary() || value.IsBlob() || value.IsBsob() )
    {
        text = CString( "<" ) + CORE::UInt32ToString( value.ByteSize( false ) ) + " bytes>";
    }
    else
        text = value.AsString();

    if ( number )
    {
        if ( column.thousandsSeparator )
            text = GroupDigits( text );
        if ( !column.prefix.IsNULLOrEmpty() )
            text = column.prefix + text;
        if ( !column.suffix.IsNULLOrEmpty() )
            text += column.suffix;
    }
    content.text = text;
    content.isNumber = number;
}

/*-------------------------------------------------------------------------*/

CString
CTableCellFormatter::FormatText( const CORE::CVariant& value, const CTableViewColumn& column )
{GUCEF_TRACE;

    SCellContent content;
    Format( value, column, content );
    return content.text;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
