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

#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_CDECIMAL_H
#include "gucefCORE_CDecimal.h"
#define GUCEF_CORE_CDECIMAL_H
#endif /* GUCEF_CORE_CDECIMAL_H ? */

#include "gucefCORE_CCsvColumnConverter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCsvColumnConverter::CCsvColumnConverter( const SCsvReadOptions& options )
    : m_options( options )
    , m_scratch()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CString
CCsvColumnConverter::Excerpt( const char* data, UInt32 length )
{GUCEF_TRACE;

    CString result;
    const UInt32 shown = length < 40 ? length : 40;
    for ( UInt32 i = 0; i < shown; ++i )
    {
        const unsigned char c = (unsigned char) data[ i ];
        result += ( c >= 0x20 && c < 0x7F ) ? (char) c : '?';
    }
    if ( shown < length )
        result += "...";
    return result;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvColumnConverter::ValueError( const SCsvColumnConversion& column, const char* data, UInt32 length, const CString& reason,
                                 CDataTableColumnBuilder& builder, UInt64& valueErrors, CString& error )
{GUCEF_TRACE;

    if ( m_options.failOnValueError )
    {
        error = "Column \"" + column.column.name + "\": \"" + Excerpt( data, length ) + "\" " + reason;
        return DATATABLE_STATUS_TYPE_MISMATCH;
    }

    if ( !builder.AppendNull() )
    {
        error = "Column \"" + column.column.name + "\": \"" + Excerpt( data, length ) + "\" " + reason + ", and the column does not take null";
        return DATATABLE_STATUS_TYPE_MISMATCH;
    }
    ++valueErrors;
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvColumnConverter::ConvertMissing( const SCsvColumnConversion& column, CDataTableColumnBuilder& builder, CString& error )
{GUCEF_TRACE;

    if ( !builder.AppendNull() )
    {
        error = "Column \"" + column.column.name + "\" does not take null but a record has no field for it";
        return DATATABLE_STATUS_TYPE_MISMATCH;
    }
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvColumnConverter::Convert( const SCsvColumnConversion& column ,
                              const char* data                   ,
                              UInt32 length                      ,
                              bool quoted                        ,
                              CDataTableColumnBuilder& builder   ,
                              UInt64& valueErrors                ,
                              CString& error                     )
{GUCEF_TRACE;

    const bool textColumn = DATATABLE_TYPE_UTF8 == column.column.type || DATATABLE_TYPE_BINARY == column.column.type;

    if ( !quoted || !textColumn )
    {
        const bool isNull = m_options.HasOnlyEmptyNullToken() ? ( 0 == length ) : m_options.IsNullToken( data, length );
        if ( isNull )
        {
            if ( !builder.AppendNull() )
            {
                error = "Column \"" + column.column.name + "\" does not take null but a field is empty or a null token";
                return DATATABLE_STATUS_TYPE_MISMATCH;
            }
            return DATATABLE_STATUS_OK;
        }
    }

    if ( DATATABLE_TYPE_UTF8 == column.column.type )
    {
        if ( !CCsvTableText::IsAscii( data, length ) && !CCsvTableText::IsValidUtf8( data, length ) )
        {
            error = "Column \"" + column.column.name + "\" holds text that is not valid UTF-8";
            return DATATABLE_STATUS_CORRUPT_DATA;
        }
        if ( !builder.AppendUtf8( data, length ) )
        {
            error = "Column \"" + column.column.name + "\": the text could not be added";
            return builder.GetLastStatus();
        }
        return DATATABLE_STATUS_OK;
    }

    CString reason;
    if ( ConvertValue( column, data, length, builder, reason ) )
        return DATATABLE_STATUS_OK;

    if ( reason.IsNULLOrEmpty() )
    {
        // the builder refused for a reason other than the value
        if ( DATATABLE_STATUS_OUT_OF_MEMORY == builder.GetLastStatus() || DATATABLE_STATUS_LIMIT_EXCEEDED == builder.GetLastStatus() )
        {
            error = "Column \"" + column.column.name + "\": a buffer could not be grown";
            return builder.GetLastStatus();
        }
        reason = "does not fit the column";
    }
    return ValueError( column, data, length, reason, builder, valueErrors, error );
}

/*-------------------------------------------------------------------------*/

bool
CCsvColumnConverter::ConvertValue( const SCsvColumnConversion& column, const char* data, UInt32 length, CDataTableColumnBuilder& builder, CString& reason )
{GUCEF_TRACE;

    const CDataTableColumnDef& def = column.column;
    CCsvTableText::SNumberInfo number;

    switch ( def.type )
    {
        case DATATABLE_TYPE_BOOL:
        {
            bool value = false;
            if ( !CCsvTableText::ParseBool( data, length, true, value ) )
            {
                reason = "is not a boolean";
                return false;
            }
            return builder.AppendBool( value );
        }
        case DATATABLE_TYPE_INT8:
        case DATATABLE_TYPE_INT16:
        case DATATABLE_TYPE_INT32:
        case DATATABLE_TYPE_INT64:
        {
            Int64 value = 0;
            if ( !CCsvTableText::ScanNumber( data, length, m_options.numberFormat, false, number ) || !CCsvTableText::NumberToInt64( number, value ) )
            {
                reason = "is not a whole number in range";
                return false;
            }
            if ( !builder.AppendInt64( value ) )
            {
                reason = "is out of range for " + def.GetTypeText();
                return false;
            }
            return true;
        }
        case DATATABLE_TYPE_UINT8:
        case DATATABLE_TYPE_UINT16:
        case DATATABLE_TYPE_UINT32:
        case DATATABLE_TYPE_UINT64:
        {
            UInt64 value = 0;
            if ( !CCsvTableText::ScanNumber( data, length, m_options.numberFormat, false, number ) || !CCsvTableText::NumberToUInt64( number, value ) )
            {
                reason = "is not a whole number that is not negative and in range";
                return false;
            }
            if ( !builder.AppendUInt64( value ) )
            {
                reason = "is out of range for " + def.GetTypeText();
                return false;
            }
            return true;
        }
        case DATATABLE_TYPE_FLOAT32:
        {
            Float32 value = 0;
            if ( !CCsvTableText::ScanNumber( data, length, m_options.numberFormat, true, number ) || !CCsvTableText::NumberToFloat32( number, value ) )
            {
                reason = "is not a number that fits float32";
                return false;
            }
            return builder.AppendFloat64( (Float64) value );
        }
        case DATATABLE_TYPE_FLOAT64:
        {
            Float64 value = 0;
            if ( !CCsvTableText::ScanNumber( data, length, m_options.numberFormat, true, number ) || !CCsvTableText::NumberToFloat64( number, value ) )
            {
                reason = "is not a number that fits float64";
                return false;
            }
            return builder.AppendFloat64( value );
        }
        case DATATABLE_TYPE_DECIMAL:
        {
            Int64 mantissa = 0;
            UInt8 scale = 0;
            if ( !CCsvTableText::ScanNumber( data, length, m_options.numberFormat, false, number ) || !CCsvTableText::NumberToDecimal( number, mantissa, scale ) )
            {
                reason = "is not a decimal of at most 18 digits";
                return false;
            }
            bool taken = false;
            if ( scale == def.decimalScale )
            {
                taken = builder.AppendDecimalMantissa( mantissa );
            }
            else
            {
                CDecimal decimal;
                if ( DecimalStatusIsError( CDecimal::FromUnscaled( mantissa, scale, decimal ) ) )
                {
                    reason = "is not a decimal that can be held";
                    return false;
                }
                taken = builder.AppendDecimal( decimal );
            }
            if ( !taken )
            {
                reason = "does not fit " + def.GetTypeText() + " without rounding";
                return false;
            }
            return true;
        }
        case DATATABLE_TYPE_DATE:
        {
            Int32 days = 0;
            for ( size_t i = 0; i < column.dateFormats.size(); ++i )
            {
                if ( CCsvTableText::ParseDate( column.dateFormats[ i ], data, length, days ) )
                    return builder.AppendDate( days );
            }
            reason = "is not a date";
            return false;
        }
        case DATATABLE_TYPE_TIMESTAMP:
        {
            Int64 ticks = 0;
            if ( !CDataTableValues::ParseTimestamp( data, length, def.timeUnit, ticks ) )
            {
                reason = "is not an ISO 8601 date and time that fits " + def.GetTypeText();
                return false;
            }
            return builder.AppendTimestamp( ticks );
        }
        case DATATABLE_TYPE_BINARY:
        {
            if ( 0 != length % 4 )
            {
                reason = "is not base64";
                return false;
            }
            if ( m_scratch.size() < length / 4 * 3 + 1 )
                m_scratch.resize( length / 4 * 3 + 1 );
            UInt32 decoded = 0;
            if ( !CCsvTableText::Base64Decode( data, length, &m_scratch[ 0 ], decoded ) )
            {
                reason = "is not base64";
                return false;
            }
            return builder.AppendBinary( &m_scratch[ 0 ], decoded );
        }
        default:
        {
            reason = "is in a column that cannot be read";
            return false;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
