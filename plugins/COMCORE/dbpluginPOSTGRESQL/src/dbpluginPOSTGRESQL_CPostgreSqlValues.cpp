/*
 *  dbpluginPOSTGRESQL: gucefCOMCORE database backend for PostgreSQL
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
#include <limits>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "dbpluginPOSTGRESQL_CPostgreSqlValues.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DBPLUGIN {
namespace POSTGRESQL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Decimal digits of an unsigned number, written backwards into the end of the buffer; the first digit */
char*
WriteDigits( UInt64 value, char* end )
{
    char* p = end;
    do
    {
        *--p = (char) ( '0' + ( value % 10 ) );
        value /= 10;
    }
    while ( 0 != value );
    return p;
}

/*-------------------------------------------------------------------------*/

UInt32
AddInt64( Int64 value, CPostgreSqlParameters& parameters )
{
    char buffer[ 32 ];
    char* end = buffer + sizeof( buffer );
    const UInt64 magnitude = value < 0 ? (UInt64) 0 - (UInt64) value : (UInt64) value;
    char* first = WriteDigits( magnitude, end );
    if ( value < 0 )
        *--first = '-';
    return parameters.AddText( first, (UInt32) ( end - first ) );
}

/*-------------------------------------------------------------------------*/

UInt32
AddUInt64( UInt64 value, CPostgreSqlParameters& parameters )
{
    char buffer[ 32 ];
    char* end = buffer + sizeof( buffer );
    char* first = WriteDigits( value, end );
    return parameters.AddText( first, (UInt32) ( end - first ) );
}

/*-------------------------------------------------------------------------*/

UInt32
AddBool( bool value, CPostgreSqlParameters& parameters )
{
    return value ? parameters.AddText( "true", 4 ) : parameters.AddText( "false", 5 );
}

/*-------------------------------------------------------------------------*/

UInt32
AddFloat64( Float64 value, CPostgreSqlParameters& parameters )
{
    return parameters.AddText( CPostgreSqlValues::Float64ToText( value ) );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CPostgreSqlValues::MantissaToDecimalText( Int64 mantissa, UInt8 scale )
{GUCEF_TRACE;

    char buffer[ 64 ];
    char* end = buffer + sizeof( buffer );
    const UInt64 magnitude = mantissa < 0 ? (UInt64) 0 - (UInt64) mantissa : (UInt64) mantissa;
    char digits[ 32 ];
    char* digitsEnd = digits + sizeof( digits );
    char* first = WriteDigits( magnitude, digitsEnd );
    UInt32 count = (UInt32) ( digitsEnd - first );

    char* p = end;
    UInt32 written = 0;
    // the fraction, zero padded to the scale
    for ( UInt32 i=0; i<scale; ++i )
    {
        *--p = written < count ? digitsEnd[ -1 - (Int32) written ] : '0';
        ++written;
    }
    if ( scale > 0 )
        *--p = '.';
    // the integer part, at least one digit
    if ( written >= count )
        *--p = '0';
    while ( written < count )
    {
        *--p = digitsEnd[ -1 - (Int32) written ];
        ++written;
    }
    if ( mantissa < 0 )
        *--p = '-';
    return CORE::CString( p, (UInt32) ( end - p ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPostgreSqlValues::Float64ToText( Float64 value )
{GUCEF_TRACE;

    if ( value != value )
        return "NaN";
    if ( value > 0 && value * 0.5 == value )
        return "Infinity";
    if ( value < 0 && value * 0.5 == value )
        return "-Infinity";
    return CORE::Float64ToRoundTripString( value );
}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlValues::ParseUInt64( const char* text, UInt32 byteLength, UInt64& value )
{GUCEF_TRACE;

    if ( GUCEF_NULL == text || 0 == byteLength || byteLength > 20 )
        return false;
    UInt64 number = 0;
    for ( UInt32 i=0; i<byteLength; ++i )
    {
        const char c = text[ i ];
        if ( c < '0' || c > '9' )
            return false;
        const UInt64 digit = (UInt64) ( c - '0' );
        if ( number > ( GUCEF_UINT64MAX - digit ) / 10 )
            return false;
        number = number * 10 + digit;
    }
    value = number;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlValues::ParseInt64( const char* text, UInt32 byteLength, Int64& value )
{GUCEF_TRACE;

    if ( GUCEF_NULL == text || 0 == byteLength )
        return false;
    const bool negative = '-' == text[ 0 ];
    UInt64 magnitude = 0;
    if ( !ParseUInt64( negative ? text + 1 : text, negative ? byteLength - 1 : byteLength, magnitude ) )
        return false;
    if ( negative )
    {
        if ( magnitude > (UInt64) GUCEF_INT64MAX + 1 )
            return false;
        value = (Int64) ( (UInt64) 0 - magnitude );
        return true;
    }
    if ( magnitude > (UInt64) GUCEF_INT64MAX )
        return false;
    value = (Int64) magnitude;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlValues::ParseFloat64( const char* text, UInt32 byteLength, Float64& value )
{GUCEF_TRACE;

    if ( GUCEF_NULL == text || 0 == byteLength )
        return false;
    if ( 3 == byteLength && 0 == memcmp( text, "NaN", 3 ) )
    {
        value = std::numeric_limits< Float64 >::quiet_NaN();
        return true;
    }
    if ( 8 == byteLength && 0 == memcmp( text, "Infinity", 8 ) )
    {
        value = std::numeric_limits< Float64 >::infinity();
        return true;
    }
    if ( 9 == byteLength && 0 == memcmp( text, "-Infinity", 9 ) )
    {
        value = -std::numeric_limits< Float64 >::infinity();
        return true;
    }
    return CORE::TryParseFloat64Invariant( CORE::CString( text, byteLength ), value );
}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlValues::DecodeHex( const char* text, UInt32 byteLength, GUCEF::vector< UInt8 >& bytes )
{GUCEF_TRACE;

    bytes.clear();
    if ( 0 != ( byteLength & 1 ) )
        return false;
    bytes.reserve( byteLength / 2 );
    for ( UInt32 i=0; i<byteLength; i+=2 )
    {
        Int32 nibbles[ 2 ];
        for ( UInt32 n=0; n<2; ++n )
        {
            const char c = text[ i+n ];
            if ( c >= '0' && c <= '9' )
                nibbles[ n ] = c - '0';
            else
            if ( c >= 'a' && c <= 'f' )
                nibbles[ n ] = c - 'a' + 10;
            else
            if ( c >= 'A' && c <= 'F' )
                nibbles[ n ] = c - 'A' + 10;
            else
                return false;
        }
        bytes.push_back( (UInt8) ( ( nibbles[ 0 ] << 4 ) | nibbles[ 1 ] ) );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CPostgreSqlValues::AddCellParameter( const CORE::CDataTableColumnDef& column, const CORE::CDataTableColumnBatch& cells, UInt32 row, CPostgreSqlParameters& parameters )
{GUCEF_TRACE;

    if ( row >= cells.GetRowCount() )
        return 0;
    if ( !cells.IsValid( row ) )
        return parameters.AddNull();

    switch ( column.type )
    {
        case CORE::DATATABLE_TYPE_BOOL:
        {
            const UInt8* bits = cells.GetBoolBits();
            return GUCEF_NULL == bits ? 0 : AddBool( 0 != ( bits[ row >> 3 ] & ( 1u << ( row & 7u ) ) ), parameters );
        }
        case CORE::DATATABLE_TYPE_INT8:
        {
            const Int8* values = cells.GetInt8Data();
            return GUCEF_NULL == values ? 0 : AddInt64( values[ row ], parameters );
        }
        case CORE::DATATABLE_TYPE_INT16:
        {
            const Int16* values = cells.GetInt16Data();
            return GUCEF_NULL == values ? 0 : AddInt64( values[ row ], parameters );
        }
        case CORE::DATATABLE_TYPE_INT32:
        case CORE::DATATABLE_TYPE_DATE:
        {
            const Int32* values = cells.GetInt32Data();
            return GUCEF_NULL == values ? 0 : AddInt64( values[ row ], parameters );
        }
        case CORE::DATATABLE_TYPE_INT64:
        case CORE::DATATABLE_TYPE_TIMESTAMP:
        {
            const Int64* values = cells.GetInt64Data();
            return GUCEF_NULL == values ? 0 : AddInt64( values[ row ], parameters );
        }
        case CORE::DATATABLE_TYPE_DECIMAL:
        {
            const Int64* values = cells.GetInt64Data();
            return GUCEF_NULL == values ? 0 : parameters.AddText( MantissaToDecimalText( values[ row ], column.decimalScale ) );
        }
        case CORE::DATATABLE_TYPE_UINT8:
        {
            const UInt8* values = cells.GetUInt8Data();
            return GUCEF_NULL == values ? 0 : AddUInt64( values[ row ], parameters );
        }
        case CORE::DATATABLE_TYPE_UINT16:
        {
            const UInt16* values = cells.GetUInt16Data();
            return GUCEF_NULL == values ? 0 : AddUInt64( values[ row ], parameters );
        }
        case CORE::DATATABLE_TYPE_UINT32:
        {
            const UInt32* values = cells.GetUInt32Data();
            return GUCEF_NULL == values ? 0 : AddUInt64( values[ row ], parameters );
        }
        case CORE::DATATABLE_TYPE_UINT64:
        {
            const UInt64* values = cells.GetUInt64Data();
            return GUCEF_NULL == values ? 0 : AddUInt64( values[ row ], parameters );
        }
        case CORE::DATATABLE_TYPE_FLOAT32:
        {
            const Float32* values = cells.GetFloat32Data();
            return GUCEF_NULL == values ? 0 : AddFloat64( (Float64) values[ row ], parameters );
        }
        case CORE::DATATABLE_TYPE_FLOAT64:
        {
            const Float64* values = cells.GetFloat64Data();
            return GUCEF_NULL == values ? 0 : AddFloat64( values[ row ], parameters );
        }
        case CORE::DATATABLE_TYPE_UTF8:
        {
            UInt32 length = 0;
            const char* bytes = cells.GetVariableValue( row, length );
            return GUCEF_NULL == bytes ? 0 : parameters.AddText( bytes, length );
        }
        case CORE::DATATABLE_TYPE_BINARY:
        {
            UInt32 length = 0;
            const char* bytes = cells.GetVariableValue( row, length );
            return GUCEF_NULL == bytes ? 0 : parameters.AddBinary( bytes, length );
        }
        default:
            return 0;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CPostgreSqlValues::AddLiteralParameter( const CORE::CDataTableColumnDef& column, const CORE::CVariant& literal, CPostgreSqlParameters& parameters )
{GUCEF_TRACE;

    if ( literal.IsNULL() || !literal.IsInitialized() )
        return parameters.AddNull();

    switch ( column.type )
    {
        case CORE::DATATABLE_TYPE_BOOL:
            return AddBool( literal.AsBool(), parameters );
        case CORE::DATATABLE_TYPE_INT8:
        case CORE::DATATABLE_TYPE_INT16:
        case CORE::DATATABLE_TYPE_INT32:
        case CORE::DATATABLE_TYPE_INT64:
        case CORE::DATATABLE_TYPE_DATE:
        case CORE::DATATABLE_TYPE_TIMESTAMP:
            return AddInt64( literal.AsInt64(), parameters );
        case CORE::DATATABLE_TYPE_DECIMAL:
            return parameters.AddText( MantissaToDecimalText( literal.AsInt64(), column.decimalScale ) );
        case CORE::DATATABLE_TYPE_UINT8:
        case CORE::DATATABLE_TYPE_UINT16:
        case CORE::DATATABLE_TYPE_UINT32:
        case CORE::DATATABLE_TYPE_UINT64:
            return AddUInt64( literal.AsUInt64(), parameters );
        case CORE::DATATABLE_TYPE_FLOAT32:
        case CORE::DATATABLE_TYPE_FLOAT64:
            return AddFloat64( literal.AsFloat64(), parameters );
        case CORE::DATATABLE_TYPE_UTF8:
        {
            const char* bytes = literal.AsCharPtr();
            const UInt32 length = GUCEF_NULL == bytes ? 0 : literal.ByteSize( false );
            return parameters.AddText( GUCEF_NULL == bytes ? "" : bytes, length );
        }
        case CORE::DATATABLE_TYPE_BINARY:
        {
            const void* bytes = literal.AsVoidPtr();
            return parameters.AddBinary( bytes, GUCEF_NULL == bytes ? 0 : literal.ByteSize( false ) );
        }
        default:
            return 0;
    }
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlValues::AppendResultValue( const CORE::CDataTableColumnDef& column, const char* text, UInt32 byteLength, bool isNull, CORE::CDataTableColumnBuilder& target )
{GUCEF_TRACE;

    if ( isNull )
        return target.AppendNull() ? CORE::DATATABLE_STATUS_OK : target.GetLastStatus();

    bool appended = false;
    switch ( column.type )
    {
        case CORE::DATATABLE_TYPE_BOOL:
        {
            if ( 1 != byteLength || ( 't' != text[ 0 ] && 'f' != text[ 0 ] ) )
                return CORE::DATATABLE_STATUS_CORRUPT_DATA;
            appended = target.AppendBool( 't' == text[ 0 ] );
            break;
        }
        case CORE::DATATABLE_TYPE_INT8:
        case CORE::DATATABLE_TYPE_INT16:
        case CORE::DATATABLE_TYPE_INT32:
        case CORE::DATATABLE_TYPE_INT64:
        {
            Int64 value = 0;
            if ( !ParseInt64( text, byteLength, value ) )
                return CORE::DATATABLE_STATUS_CORRUPT_DATA;
            appended = target.AppendInt64( value );
            break;
        }
        case CORE::DATATABLE_TYPE_UINT8:
        case CORE::DATATABLE_TYPE_UINT16:
        case CORE::DATATABLE_TYPE_UINT32:
        case CORE::DATATABLE_TYPE_UINT64:
        {
            UInt64 value = 0;
            if ( !ParseUInt64( text, byteLength, value ) )
                return CORE::DATATABLE_STATUS_TYPE_MISMATCH;
            appended = target.AppendUInt64( value );
            break;
        }
        case CORE::DATATABLE_TYPE_FLOAT32:
        {
            Float64 value = 0.0;
            if ( !ParseFloat64( text, byteLength, value ) )
                return CORE::DATATABLE_STATUS_CORRUPT_DATA;
            // the shortest text of a real read as a double is rounded to the real it came from
            appended = target.AppendFloat64( (Float64) (Float32) value );
            break;
        }
        case CORE::DATATABLE_TYPE_FLOAT64:
        {
            Float64 value = 0.0;
            if ( !ParseFloat64( text, byteLength, value ) )
                return CORE::DATATABLE_STATUS_CORRUPT_DATA;
            appended = target.AppendFloat64( value );
            break;
        }
        case CORE::DATATABLE_TYPE_DECIMAL:
        {
            Int64 mantissa = 0;
            if ( !ParseInt64( text, byteLength, mantissa ) )
                return CORE::DATATABLE_STATUS_TYPE_MISMATCH;
            appended = target.AppendDecimalMantissa( mantissa );
            break;
        }
        case CORE::DATATABLE_TYPE_DATE:
        {
            Int64 days = 0;
            if ( !ParseInt64( text, byteLength, days ) || days < GUCEF_INT32MIN || days > GUCEF_INT32MAX )
                return CORE::DATATABLE_STATUS_TYPE_MISMATCH;
            appended = target.AppendDate( (Int32) days );
            break;
        }
        case CORE::DATATABLE_TYPE_TIMESTAMP:
        {
            Int64 ticks = 0;
            if ( !ParseInt64( text, byteLength, ticks ) )
                return CORE::DATATABLE_STATUS_TYPE_MISMATCH;
            appended = target.AppendTimestamp( ticks );
            break;
        }
        case CORE::DATATABLE_TYPE_UTF8:
        {
            appended = target.AppendUtf8( text, byteLength );
            break;
        }
        case CORE::DATATABLE_TYPE_BINARY:
        {
            GUCEF::vector< UInt8 > bytes;
            if ( !DecodeHex( text, byteLength, bytes ) )
                return CORE::DATATABLE_STATUS_CORRUPT_DATA;
            appended = target.AppendBinary( bytes.empty() ? GUCEF_NULL : &bytes[ 0 ], (UInt32) bytes.size() );
            break;
        }
        default:
            return CORE::DATATABLE_STATUS_UNSUPPORTED;
    }
    return appended ? CORE::DATATABLE_STATUS_OK : target.GetLastStatus();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace POSTGRESQL */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
