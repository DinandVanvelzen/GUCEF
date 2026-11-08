/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCORE_CBinaryWireWriter.h"

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

CBinaryWireWriter::CBinaryWireWriter( CDynamicBuffer& buffer, UInt32 startOffset )
    : m_buffer( &buffer )
    , m_offset( startOffset )
    , m_maxSize( 0 )
    , m_failed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt8*
CBinaryWireWriter::MakeRoom( UInt32 size )
{GUCEF_TRACE;

    if ( m_failed )
        return GUCEF_NULL;

    // 64 bit: the end of the write and the doubled size may pass the 32 bit range of the buffer
    const UInt64 limit = 0 != m_maxSize ? (UInt64) m_maxSize : (UInt64) GUCEF_UINT32MAX;
    const UInt64 required = (UInt64) m_offset + size;
    if ( required > limit )
    {
        m_failed = true;
        return GUCEF_NULL;
    }
    UInt32 bufferSize = m_buffer->GetBufferSize();
    if ( required > bufferSize )
    {
        UInt64 newSize = bufferSize < 64 ? 64 : bufferSize;
        while ( newSize < required )
            newSize *= 2;
        if ( newSize > limit )
            newSize = limit;
        if ( !m_buffer->SetBufferSize( (UInt32) newSize, false ) || m_buffer->GetBufferSize() < required )
        {
            m_failed = true;
            return GUCEF_NULL;
        }
    }
    UInt8* dest = static_cast< UInt8* >( m_buffer->GetBufferPtr( m_offset ) );
    m_offset = (UInt32) required;
    if ( m_buffer->GetDataSize() < m_offset )
        m_buffer->SetDataSize( m_offset );
    return dest;
}

/*-------------------------------------------------------------------------*/

UInt8*
CBinaryWireWriter::Reserve( UInt32 size )
{GUCEF_TRACE;

    return MakeRoom( size );
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::WriteUInt8( UInt8 value )
{GUCEF_TRACE;

    UInt8* dest = MakeRoom( 1 );
    if ( GUCEF_NULL != dest )
        *dest = value;
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::WriteBool( bool value )
{GUCEF_TRACE;

    UInt8* dest = MakeRoom( 1 );
    if ( GUCEF_NULL != dest )
        *dest = value ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::WriteFixed32( UInt32 value )
{GUCEF_TRACE;

    UInt8* dest = MakeRoom( 4 );
    if ( GUCEF_NULL == dest )
        return;
    dest[ 0 ] = (UInt8) value;
    dest[ 1 ] = (UInt8) ( value >> 8 );
    dest[ 2 ] = (UInt8) ( value >> 16 );
    dest[ 3 ] = (UInt8) ( value >> 24 );
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::WriteFixed64( UInt64 value )
{GUCEF_TRACE;

    UInt8* dest = MakeRoom( 8 );
    if ( GUCEF_NULL == dest )
        return;
    for ( UInt32 i=0; i<8; ++i )
    {
        dest[ i ] = (UInt8) value;
        value >>= 8;
    }
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::WriteUInt64BigEndian( UInt64 value )
{GUCEF_TRACE;

    UInt8* dest = MakeRoom( 8 );
    if ( GUCEF_NULL == dest )
        return;
    for ( Int32 i=7; i>=0; --i )
    {
        dest[ i ] = (UInt8) value;
        value >>= 8;
    }
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::WriteFloat32( Float32 value )
{GUCEF_TRACE;

    UInt32 bits = 0;
    memcpy( &bits, &value, 4 );
    WriteFixed32( bits );
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::WriteFloat64( Float64 value )
{GUCEF_TRACE;

    UInt64 bits = 0;
    memcpy( &bits, &value, 8 );
    WriteFixed64( bits );
}

/*-------------------------------------------------------------------------*/

UInt32
CBinaryWireWriter::EncodeVarUInt64( UInt64 value, UInt8* dest )
{GUCEF_TRACE;

    UInt32 n = 0;
    while ( value >= 0x80 )
    {
        dest[ n++ ] = (UInt8) ( value | 0x80 );
        value >>= 7;
    }
    dest[ n++ ] = (UInt8) value;
    return n;
}

/*-------------------------------------------------------------------------*/

UInt32
CBinaryWireWriter::GetVarUInt32Size( UInt32 value )
{GUCEF_TRACE;

    UInt32 n = 1;
    while ( value >= 0x80 )
    {
        value >>= 7;
        ++n;
    }
    return n;
}

/*-------------------------------------------------------------------------*/

UInt32
CBinaryWireWriter::GetVarUInt64Size( UInt64 value )
{GUCEF_TRACE;

    UInt32 n = 1;
    while ( value >= 0x80 )
    {
        value >>= 7;
        ++n;
    }
    return n;
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::WriteVarUInt32( UInt32 value )
{GUCEF_TRACE;

    WriteVarUInt64( value );
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::WriteVarUInt64( UInt64 value )
{GUCEF_TRACE;

    if ( value < 0x80 )
    {
        UInt8* dest = MakeRoom( 1 );
        if ( GUCEF_NULL != dest )
            *dest = (UInt8) value;
        return;
    }
    UInt8 tmp[ MaxVarIntSize ];
    UInt32 n = EncodeVarUInt64( value, tmp );
    UInt8* dest = MakeRoom( n );
    if ( GUCEF_NULL != dest )
        memcpy( dest, tmp, n );
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::WriteVarInt32( Int32 value )
{GUCEF_TRACE;

    WriteVarUInt64( ZigZagEncode32( value ) );
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::WriteVarInt64( Int64 value )
{GUCEF_TRACE;

    WriteVarUInt64( ZigZagEncode64( value ) );
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::WriteBytes( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( 0 == size )
        return;
    UInt8* dest = MakeRoom( size );
    if ( GUCEF_NULL != dest )
        memcpy( dest, data, size );
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::WriteSizedBytes( const void* data, UInt32 size )
{GUCEF_TRACE;

    WriteVarUInt32( size );
    WriteBytes( data, size );
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::WriteSizedBytes( const CDynamicBuffer& data )
{GUCEF_TRACE;

    WriteSizedBytes( data.GetConstBufferPtr(), data.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::WriteString( const CString& str )
{GUCEF_TRACE;

    CUtf8String utf8 = ToUtf8String( str );
    UInt32 byteSize = utf8.ByteSize();
    UInt32 size = byteSize > 0 ? byteSize - 1 : 0;
    WriteSizedBytes( utf8.C_String(), size );
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::WriteUtf8( const char* utf8, UInt32 byteSize )
{GUCEF_TRACE;

    WriteSizedBytes( utf8, byteSize );
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::WriteDecimal( const CDecimal& value )
{GUCEF_TRACE;

    WriteVarInt64( value.GetUnscaledValue() );
    WriteVarUInt32( value.GetScale() );
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::WriteDate( Int32 daysSinceEpoch )
{GUCEF_TRACE;

    WriteVarInt32( daysSinceEpoch );
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::WriteTimestamp( Int64 microsSinceEpoch )
{GUCEF_TRACE;

    WriteVarInt64( microsSinceEpoch );
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireWriter::WriteDateTime( const CDateTime& value )
{GUCEF_TRACE;

    Int64 utcMs = 0;
    if ( !CDataTableValues::TicksFromDateTime( value, DATATABLE_TIMEUNIT_MILLISECONDS, utcMs ) )
        return false;
    WriteVarInt64( utcMs );
    WriteVarInt32( value.GetTimeZoneUTCOffsetInMins() );
    return !m_failed;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireWriter::WriteVariant( const CVariant& value )
{GUCEF_TRACE;

    UInt8 typeId = value.GetTypeId();
    switch ( typeId )
    {
        case GUCEF_DATATYPE_BOOLEAN:
        {
            WriteUInt8( typeId );
            WriteVarUInt64( value.AsBool() ? 1 : 0 );
            return !m_failed;
        }
        case GUCEF_DATATYPE_UINT8:
        {
            WriteUInt8( typeId );
            WriteVarUInt64( value.AsUInt8() );
            return !m_failed;
        }
        case GUCEF_DATATYPE_LE_UINT16:
        case GUCEF_DATATYPE_BE_UINT16:
        {
            WriteUInt8( GUCEF_DATATYPE_LE_UINT16 );
            WriteVarUInt64( value.AsUInt16() );
            return !m_failed;
        }
        case GUCEF_DATATYPE_LE_UINT32:
        case GUCEF_DATATYPE_BE_UINT32:
        {
            WriteUInt8( GUCEF_DATATYPE_LE_UINT32 );
            WriteVarUInt64( value.AsUInt32() );
            return !m_failed;
        }
        case GUCEF_DATATYPE_LE_UINT64:
        case GUCEF_DATATYPE_BE_UINT64:
        {
            WriteUInt8( GUCEF_DATATYPE_LE_UINT64 );
            WriteVarUInt64( value.AsUInt64() );
            return !m_failed;
        }
        case GUCEF_DATATYPE_INT8:
        {
            WriteUInt8( typeId );
            WriteVarInt64( value.AsInt8() );
            return !m_failed;
        }
        case GUCEF_DATATYPE_LE_INT16:
        case GUCEF_DATATYPE_BE_INT16:
        {
            WriteUInt8( GUCEF_DATATYPE_LE_INT16 );
            WriteVarInt64( value.AsInt16() );
            return !m_failed;
        }
        case GUCEF_DATATYPE_LE_INT32:
        case GUCEF_DATATYPE_BE_INT32:
        {
            WriteUInt8( GUCEF_DATATYPE_LE_INT32 );
            WriteVarInt64( value.AsInt32() );
            return !m_failed;
        }
        case GUCEF_DATATYPE_LE_INT64:
        case GUCEF_DATATYPE_BE_INT64:
        {
            WriteUInt8( GUCEF_DATATYPE_LE_INT64 );
            WriteVarInt64( value.AsInt64() );
            return !m_failed;
        }
        case GUCEF_DATATYPE_LE_FLOAT32:
        case GUCEF_DATATYPE_BE_FLOAT32:
        {
            WriteUInt8( GUCEF_DATATYPE_LE_FLOAT32 );
            WriteFloat32( value.AsFloat32() );
            return !m_failed;
        }
        case GUCEF_DATATYPE_LE_FLOAT64:
        case GUCEF_DATATYPE_BE_FLOAT64:
        {
            WriteUInt8( GUCEF_DATATYPE_LE_FLOAT64 );
            WriteFloat64( value.AsFloat64() );
            return !m_failed;
        }
        case GUCEF_DATATYPE_ASCII_STRING:
        case GUCEF_DATATYPE_UTF8_STRING:
        case GUCEF_DATATYPE_UTF16_LE_STRING:
        case GUCEF_DATATYPE_UTF16_BE_STRING:
        case GUCEF_DATATYPE_UTF32_LE_STRING:
        case GUCEF_DATATYPE_UTF32_BE_STRING:
        {
            CUtf8String utf8 = value.AsUtf8String();
            WriteUInt8( GUCEF_DATATYPE_UTF8_STRING );
            UInt32 byteSize = utf8.ByteSize();
            WriteSizedBytes( utf8.C_String(), byteSize > 0 ? byteSize - 1 : 0 );
            return !m_failed;
        }
        case GUCEF_DATATYPE_BINARY_BLOB:
        case GUCEF_DATATYPE_BINARY_BSOB:
        {
            WriteUInt8( GUCEF_DATATYPE_BINARY_BLOB );
            WriteSizedBytes( value.AsVoidPtr(), value.ByteSize( false ) );
            return !m_failed;
        }
        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
        {
            CDecimal decimal;
            if ( DECIMALSTATUS_OK != value.GetDecimal( decimal ) )
                return false;
            WriteUInt8( GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL );
            WriteDecimal( decimal );
            return !m_failed;
        }
        case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        {
            WriteUInt8( GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH );
            WriteDate( value.AsInt32() );
            return !m_failed;
        }
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH:
        {
            // Each unit keeps its own id so the receiver gets the unit it was given; the ticks are signed
            UInt8 leTypeId = typeId;
            if ( GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH == typeId ) leTypeId = GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH;
            else if ( GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH == typeId ) leTypeId = GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH;
            else if ( GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH == typeId ) leTypeId = GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH;
            else if ( GUCEF_DATATYPE_BE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH == typeId ) leTypeId = GUCEF_DATATYPE_LE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH;

            CVariant native( value );
            native.ToNativeEndian();
            WriteUInt8( leTypeId );
            WriteVarInt64( native.CStyleAccess()->union_data.int64_data );
            return !m_failed;
        }
        case GUCEF_DATATYPE_NIL:
        case GUCEF_DATATYPE_NULL:
        case GUCEF_DATATYPE_VOID:
        {
            WriteUInt8( typeId );
            return !m_failed;
        }
        case GUCEF_DATATYPE_CONST_DATA_MEMORY_ADDRESS:
        case GUCEF_DATATYPE_DATA_MEMORY_ADDRESS:
        case GUCEF_DATATYPE_FUNCTION_MEMORY_ADDRESS:
        {
            // Addresses mean nothing in another process
            return false;
        }
        default:
        {
            // Every other id travels in its string form, which the reader turns back into the same type
            CUtf8String utf8 = ToUtf8String( value.AsString() );
            WriteUInt8( typeId );
            UInt32 byteSize = utf8.ByteSize();
            WriteSizedBytes( utf8.C_String(), byteSize > 0 ? byteSize - 1 : 0 );
            return !m_failed;
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CBinaryWireWriter::GetOffset( void ) const
{GUCEF_TRACE;

    return m_offset;
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::SetOffset( UInt32 offset )
{GUCEF_TRACE;

    m_offset = offset;
}

/*-------------------------------------------------------------------------*/

CDynamicBuffer&
CBinaryWireWriter::GetBuffer( void )
{GUCEF_TRACE;

    return *m_buffer;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireWriter::HasFailed( void ) const
{GUCEF_TRACE;

    return m_failed;
}

/*-------------------------------------------------------------------------*/

void
CBinaryWireWriter::SetMaxSize( UInt32 maxBytes )
{GUCEF_TRACE;

    m_maxSize = maxBytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CBinaryWireWriter::GetMaxSize( void ) const
{GUCEF_TRACE;

    return m_maxSize;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
