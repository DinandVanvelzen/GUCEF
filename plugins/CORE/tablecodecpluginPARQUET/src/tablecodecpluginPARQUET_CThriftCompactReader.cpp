/*
 *  tablecodecpluginPARQUET: GUCEF table codec plugin for Apache Parquet files
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

#ifndef TABLECODECPLUGIN_PARQUET_BYTEORDER_H
#include "tablecodecpluginPARQUET_ByteOrder.h"
#define TABLECODECPLUGIN_PARQUET_BYTEORDER_H
#endif /* TABLECODECPLUGIN_PARQUET_BYTEORDER_H ? */

#include "tablecodecpluginPARQUET_CThriftCompactReader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TABLECODECPLUGIN {
namespace PARQUET {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static inline Int64
ZigZagDecode( UInt64 value )
{
    return (Int64) ( value >> 1 ) ^ -(Int64) ( value & 1 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CThriftCompactReader::CThriftCompactReader( const UInt8* data, UInt32 size, UInt32 maxDepth )
    : m_data( data )
    , m_size( GUCEF_NULL != data ? size : 0 )
    , m_position( 0 )
    , m_maxDepth( maxDepth > MaxSupportedDepth ? MaxSupportedDepth : maxDepth )
    , m_depth( 0 )
    , m_structDepth( 0 )
    , m_status( CORE::DATATABLE_STATUS_OK )
    , m_error( GUCEF_NULL )
    , m_errorPosition( 0 )
{GUCEF_TRACE;

    m_lastFieldId[ 0 ] = 0;
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::Fail( CORE::TDataTableStatus status, const char* message )
{GUCEF_TRACE;

    if ( !HasFailed() )
    {
        m_status = CORE::DataTableStatusIsError( status ) ? status : CORE::DATATABLE_STATUS_CORRUPT_DATA;
        m_error = message;
        m_errorPosition = m_position;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CThriftCompactReader::GetErrorMessage( void ) const
{GUCEF_TRACE;

    if ( !HasFailed() )
        return CORE::CString::Empty;
    return CORE::CString( GUCEF_NULL != m_error ? m_error : "Thrift compact data is not valid" ) + " (Thrift byte " + CORE::ToString( m_errorPosition ) + ")";
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::Enter( void )
{
    if ( HasFailed() )
        return false;
    if ( m_depth >= m_maxDepth )
        return Fail( CORE::DATATABLE_STATUS_LIMIT_EXCEEDED, "Thrift values are nested deeper than the limit" );
    ++m_depth;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::Leave( void )
{
    if ( HasFailed() )
        return false;
    if ( 0 == m_depth )
        return Fail( CORE::DATATABLE_STATUS_FAILED, "Thrift nesting left more often than entered" );
    --m_depth;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::BeginStruct( void )
{
    if ( !Enter() )
        return false;
    ++m_structDepth;
    m_lastFieldId[ m_structDepth ] = 0;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::EndStruct( void )
{
    if ( 0 == m_structDepth )
        return Fail( CORE::DATATABLE_STATUS_FAILED, "Thrift struct left more often than entered" );
    if ( !Leave() )
        return false;
    --m_structDepth;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::BeginContainer( void )
{
    return Enter();
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::EndContainer( void )
{
    return Leave();
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::ReadVarint( UInt64& value )
{
    value = 0;
    if ( HasFailed() )
        return false;
    UInt32 shift = 0;
    for ( UInt32 i=0; i<10; ++i )
    {
        if ( m_position >= m_size )
            return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift varint runs past the end of the data" );
        const UInt8 byte = m_data[ m_position++ ];
        if ( 9 == i && byte > 1 )
            return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift varint is larger than 64 bits" );
        value |= (UInt64) ( byte & 0x7F ) << shift;
        if ( 0 == ( byte & 0x80 ) )
            return true;
        shift += 7;
    }
    return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift varint is longer than 10 bytes" );
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::ReadFieldHeader( UInt8& fieldType, Int16& fieldId )
{
    fieldType = THRIFT_COMPACT_STOP;
    fieldId = 0;
    if ( HasFailed() )
        return false;
    if ( 0 == m_structDepth )
        return Fail( CORE::DATATABLE_STATUS_FAILED, "Thrift field read outside a struct" );
    if ( m_position >= m_size )
        return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift struct runs past the end of the data" );

    const UInt8 header = m_data[ m_position++ ];
    const UInt8 type = (UInt8) ( header & 0x0F );
    if ( THRIFT_COMPACT_STOP == type )
    {
        if ( 0 != header )
            return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift stop field with a field id" );
        return true;
    }
    if ( type > THRIFT_COMPACT_UUID )
        return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift field of an unknown type" );

    const UInt8 delta = (UInt8) ( header >> 4 );
    Int32 id = 0;
    if ( 0 == delta )
    {
        Int16 longId = 0;
        if ( !ReadI16( longId ) )
            return false;
        id = longId;
    }
    else
    {
        id = (Int32) m_lastFieldId[ m_structDepth ] + delta;
        if ( id > 32767 )
            return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift field id beyond the i16 range" );
    }
    fieldType = type;
    fieldId = (Int16) id;
    m_lastFieldId[ m_structDepth ] = (Int16) id;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::ReadBoolField( UInt8 fieldType, bool& value )
{
    value = false;
    if ( HasFailed() )
        return false;
    if ( THRIFT_COMPACT_BOOL_TRUE == fieldType ) { value = true; return true; }
    if ( THRIFT_COMPACT_BOOL_FALSE == fieldType ) { value = false; return true; }
    return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift field is not a bool" );
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::ReadBoolElement( bool& value )
{
    value = false;
    if ( HasFailed() )
        return false;
    if ( m_position >= m_size )
        return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift bool runs past the end of the data" );
    const UInt8 byte = m_data[ m_position++ ];
    if ( 1 == byte ) { value = true; return true; }
    if ( 2 == byte || 0 == byte ) { value = false; return true; }
    return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift bool element is neither 1 nor 2" );
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::ReadI8( Int8& value )
{
    value = 0;
    if ( HasFailed() )
        return false;
    if ( m_position >= m_size )
        return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift i8 runs past the end of the data" );
    value = (Int8) m_data[ m_position++ ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::ReadI16( Int16& value )
{
    value = 0;
    UInt64 raw = 0;
    if ( !ReadVarint( raw ) )
        return false;
    if ( raw > 0xFFFF )
        return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift i16 beyond its range" );
    value = (Int16) ZigZagDecode( raw );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::ReadI32( Int32& value )
{
    value = 0;
    UInt64 raw = 0;
    if ( !ReadVarint( raw ) )
        return false;
    if ( raw > 0xFFFFFFFFULL )
        return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift i32 beyond its range" );
    value = (Int32) ZigZagDecode( raw );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::ReadI64( Int64& value )
{
    value = 0;
    UInt64 raw = 0;
    if ( !ReadVarint( raw ) )
        return false;
    value = ZigZagDecode( raw );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::ReadDouble( Float64& value )
{
    value = 0.0;
    if ( HasFailed() )
        return false;
    if ( m_size - m_position < 8 )
        return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift double runs past the end of the data" );
    value = LoadLEFloat64( m_data + m_position );
    m_position += 8;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::ReadBinary( const UInt8*& bytes, UInt32& length )
{
    bytes = GUCEF_NULL;
    length = 0;
    UInt64 raw = 0;
    if ( !ReadVarint( raw ) )
        return false;
    if ( raw > (UInt64) ( m_size - m_position ) )
        return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift binary is longer than the data left" );
    length = (UInt32) raw;
    bytes = m_data + m_position;
    m_position += length;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CThriftCompactReader::MinimumElementBytes( UInt8 type )
{
    switch ( type )
    {
        case THRIFT_COMPACT_DOUBLE: return 8;
        case THRIFT_COMPACT_UUID: return 16;
        default: return 1;
    }
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::ReadListHeader( UInt8& elementType, UInt32& size )
{
    elementType = 0;
    size = 0;
    if ( HasFailed() )
        return false;
    if ( m_position >= m_size )
        return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift list runs past the end of the data" );
    const UInt8 header = m_data[ m_position++ ];
    elementType = (UInt8) ( header & 0x0F );
    UInt64 count = header >> 4;
    if ( 15 == count )
    {
        if ( !ReadVarint( count ) )
            return false;
    }
    if ( elementType < THRIFT_COMPACT_BOOL_TRUE || elementType > THRIFT_COMPACT_UUID )
    {
        // an empty list may carry any element type, a writer has no element to describe
        if ( 0 != count )
            return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift list of an unknown element type" );
    }
    if ( count > (UInt64) ( m_size - m_position ) / MinimumElementBytes( elementType ) )
        return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift list has more elements than the data left can hold" );
    size = (UInt32) count;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::ReadMapHeader( UInt8& keyType, UInt8& valueType, UInt32& size )
{
    keyType = 0;
    valueType = 0;
    size = 0;
    UInt64 count = 0;
    if ( !ReadVarint( count ) )
        return false;
    if ( 0 == count )
        return true;
    if ( m_position >= m_size )
        return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift map runs past the end of the data" );
    const UInt8 types = m_data[ m_position++ ];
    keyType = (UInt8) ( types >> 4 );
    valueType = (UInt8) ( types & 0x0F );
    if ( keyType < THRIFT_COMPACT_BOOL_TRUE || keyType > THRIFT_COMPACT_UUID ||
         valueType < THRIFT_COMPACT_BOOL_TRUE || valueType > THRIFT_COMPACT_UUID )
        return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift map of an unknown key or value type" );
    const UInt64 pairBytes = (UInt64) MinimumElementBytes( keyType ) + MinimumElementBytes( valueType );
    if ( count > (UInt64) ( m_size - m_position ) / pairBytes )
        return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift map has more entries than the data left can hold" );
    size = (UInt32) count;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::SkipElements( UInt8 elementType, UInt32 count )
{
    // every element takes at least one byte, so the loop ends with the data at the latest
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( !Skip( elementType ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::Skip( UInt8 type )
{
    if ( HasFailed() )
        return false;
    switch ( type )
    {
        case THRIFT_COMPACT_BOOL_TRUE:
        case THRIFT_COMPACT_BOOL_FALSE:
        {
            // as an element: one byte
            bool value = false;
            return ReadBoolElement( value );
        }
        case THRIFT_COMPACT_I8:
        {
            Int8 value = 0;
            return ReadI8( value );
        }
        case THRIFT_COMPACT_I16:
        case THRIFT_COMPACT_I32:
        case THRIFT_COMPACT_I64:
        {
            UInt64 value = 0;
            return ReadVarint( value );
        }
        case THRIFT_COMPACT_DOUBLE:
        {
            Float64 value = 0;
            return ReadDouble( value );
        }
        case THRIFT_COMPACT_BINARY:
        {
            const UInt8* bytes = GUCEF_NULL;
            UInt32 length = 0;
            return ReadBinary( bytes, length );
        }
        case THRIFT_COMPACT_UUID:
        {
            if ( m_size - m_position < 16 )
                return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift uuid runs past the end of the data" );
            m_position += 16;
            return true;
        }
        case THRIFT_COMPACT_LIST:
        case THRIFT_COMPACT_SET:
        {
            UInt8 elementType = 0;
            UInt32 count = 0;
            if ( !ReadListHeader( elementType, count ) || !BeginContainer() )
                return false;
            if ( !SkipElements( elementType, count ) )
                return false;
            return EndContainer();
        }
        case THRIFT_COMPACT_MAP:
        {
            UInt8 keyType = 0, valueType = 0;
            UInt32 count = 0;
            if ( !ReadMapHeader( keyType, valueType, count ) || !BeginContainer() )
                return false;
            for ( UInt32 i=0; i<count; ++i )
            {
                if ( !Skip( keyType ) || !Skip( valueType ) )
                    return false;
            }
            return EndContainer();
        }
        case THRIFT_COMPACT_STRUCT:
        {
            if ( !BeginStruct() )
                return false;
            for ( ;; )
            {
                UInt8 fieldType = 0;
                Int16 fieldId = 0;
                if ( !ReadFieldHeader( fieldType, fieldId ) )
                    return false;
                if ( THRIFT_COMPACT_STOP == fieldType )
                    break;
                if ( !SkipField( fieldType ) )
                    return false;
            }
            return EndStruct();
        }
        default:
        {
            return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "Thrift value of an unknown type" );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CThriftCompactReader::SkipField( UInt8 fieldType )
{
    if ( THRIFT_COMPACT_BOOL_TRUE == fieldType || THRIFT_COMPACT_BOOL_FALSE == fieldType )
        return !HasFailed();
    return Skip( fieldType );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PARQUET */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
