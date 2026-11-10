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

#ifndef TABLECODECPLUGIN_PARQUET_BYTEORDER_H
#include "tablecodecpluginPARQUET_ByteOrder.h"
#define TABLECODECPLUGIN_PARQUET_BYTEORDER_H
#endif /* TABLECODECPLUGIN_PARQUET_BYTEORDER_H ? */

#include "tablecodecpluginPARQUET_CThriftCompactWriter.h"

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

static inline UInt64
ZigZagEncode( Int64 value )
{
    return ( (UInt64) value << 1 ) ^ (UInt64) ( value >> 63 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CThriftCompactWriter::CThriftCompactWriter( CORE::CDynamicBuffer& output )
    : m_output( output )
    , m_structDepth( 0 )
    , m_longFormFields( 0 )
    , m_valid( true )
{GUCEF_TRACE;

    m_lastFieldId[ 0 ] = 0;

    // many small appends: amortized constant time instead of a reallocation per append
    if ( CORE::CDynamicBuffer::GROWTHSTRATEGY_EXACT == m_output.GetGrowthStrategy() )
        m_output.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::WriteByte( UInt8 value )
{
    m_output.Append( &value, 1 );
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::WriteRaw( const void* bytes, UInt32 length )
{
    if ( length > 0 )
        m_output.Append( bytes, length );
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::WriteVarint( UInt64 value )
{
    UInt8 bytes[ 10 ];
    UInt32 count = 0;
    do
    {
        UInt8 byte = (UInt8) ( value & 0x7F );
        value >>= 7;
        if ( 0 != value )
            byte |= 0x80;
        bytes[ count++ ] = byte;
    }
    while ( 0 != value );
    m_output.Append( bytes, count );
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::BeginStruct( void )
{
    if ( m_structDepth >= CThriftCompactReader::MaxSupportedDepth )
    {
        m_valid = false;
        return;
    }
    ++m_structDepth;
    m_lastFieldId[ m_structDepth ] = 0;
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::EndStruct( void )
{
    if ( 0 == m_structDepth )
    {
        m_valid = false;
        return;
    }
    WriteByte( THRIFT_COMPACT_STOP );
    --m_structDepth;
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::WriteFieldHeader( UInt8 fieldType, Int16 fieldId )
{
    const Int32 delta = (Int32) fieldId - (Int32) m_lastFieldId[ m_structDepth ];
    if ( delta > 0 && delta <= 15 && 0 == m_longFormFields )
    {
        WriteByte( (UInt8) ( ( delta << 4 ) | fieldType ) );
    }
    else
    {
        WriteByte( fieldType );
        WriteVarint( ZigZagEncode( fieldId ) );
        if ( m_longFormFields > 0 )
            --m_longFormFields;
    }
    m_lastFieldId[ m_structDepth ] = fieldId;
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::WriteBoolField( Int16 fieldId, bool value )
{
    WriteFieldHeader( value ? THRIFT_COMPACT_BOOL_TRUE : THRIFT_COMPACT_BOOL_FALSE, fieldId );
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::WriteI8Field( Int16 fieldId, Int8 value )
{
    WriteFieldHeader( THRIFT_COMPACT_I8, fieldId );
    WriteI8( value );
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::WriteI16Field( Int16 fieldId, Int16 value )
{
    WriteFieldHeader( THRIFT_COMPACT_I16, fieldId );
    WriteI16( value );
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::WriteI32Field( Int16 fieldId, Int32 value )
{
    WriteFieldHeader( THRIFT_COMPACT_I32, fieldId );
    WriteI32( value );
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::WriteI64Field( Int16 fieldId, Int64 value )
{
    WriteFieldHeader( THRIFT_COMPACT_I64, fieldId );
    WriteI64( value );
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::WriteDoubleField( Int16 fieldId, Float64 value )
{
    WriteFieldHeader( THRIFT_COMPACT_DOUBLE, fieldId );
    WriteDouble( value );
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::WriteBinaryField( Int16 fieldId, const void* bytes, UInt32 length )
{
    WriteFieldHeader( THRIFT_COMPACT_BINARY, fieldId );
    WriteBinary( bytes, length );
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::BeginStructField( Int16 fieldId )
{
    WriteFieldHeader( THRIFT_COMPACT_STRUCT, fieldId );
    BeginStruct();
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::BeginListField( Int16 fieldId, UInt8 elementType, UInt32 size )
{
    WriteFieldHeader( THRIFT_COMPACT_LIST, fieldId );
    WriteListHeader( elementType, size );
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::WriteListHeader( UInt8 elementType, UInt32 size )
{
    if ( size < 15 )
    {
        WriteByte( (UInt8) ( ( size << 4 ) | elementType ) );
    }
    else
    {
        WriteByte( (UInt8) ( 0xF0 | elementType ) );
        WriteVarint( size );
    }
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::WriteBoolElement( bool value )
{
    WriteByte( value ? 1 : 2 );
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::WriteI8( Int8 value )
{
    WriteByte( (UInt8) value );
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::WriteI16( Int16 value )
{
    WriteVarint( ZigZagEncode( value ) );
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::WriteI32( Int32 value )
{
    WriteVarint( ZigZagEncode( value ) );
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::WriteI64( Int64 value )
{
    WriteVarint( ZigZagEncode( value ) );
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::WriteDouble( Float64 value )
{
    UInt8 bytes[ 8 ];
    StoreLEFloat64( bytes, value );
    m_output.Append( bytes, 8 );
}

/*-------------------------------------------------------------------------*/

void
CThriftCompactWriter::WriteBinary( const void* bytes, UInt32 length )
{
    WriteVarint( length );
    WriteRaw( bytes, length );
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
