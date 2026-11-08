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

#include "gucefCORE_CBinaryWireReader.h"

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

CBinaryWireReader::CBinaryWireReader( const void* data, UInt32 size )
    : m_data( static_cast< const UInt8* >( data ) )
    , m_size( GUCEF_NULL != data ? size : 0 )
    , m_pos( 0 )
    , m_failed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CBinaryWireReader::CBinaryWireReader( const CDynamicBuffer& buffer, UInt32 startOffset )
    : m_data( static_cast< const UInt8* >( buffer.GetConstBufferPtr() ) )
    , m_size( buffer.GetDataSize() )
    , m_pos( startOffset <= buffer.GetDataSize() ? startOffset : buffer.GetDataSize() )
    , m_failed( startOffset > buffer.GetDataSize() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::Fail( void )
{GUCEF_TRACE;

    m_failed = true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadUInt8( UInt8& value )
{GUCEF_TRACE;

    if ( m_failed || m_size - m_pos < 1 )
        return Fail();
    value = m_data[ m_pos++ ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadBool( bool& value )
{GUCEF_TRACE;

    UInt8 byte = 0;
    if ( !ReadUInt8( byte ) )
        return false;
    if ( byte > 1 )
        return Fail();
    value = 1 == byte;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadFixed32( UInt32& value )
{GUCEF_TRACE;

    if ( m_failed || m_size - m_pos < 4 )
        return Fail();
    const UInt8* src = m_data + m_pos;
    value = (UInt32) src[ 0 ] | ( (UInt32) src[ 1 ] << 8 ) | ( (UInt32) src[ 2 ] << 16 ) | ( (UInt32) src[ 3 ] << 24 );
    m_pos += 4;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadFixed64( UInt64& value )
{GUCEF_TRACE;

    if ( m_failed || m_size - m_pos < 8 )
        return Fail();
    const UInt8* src = m_data + m_pos;
    UInt64 result = 0;
    for ( Int32 i=7; i>=0; --i )
        result = ( result << 8 ) | src[ i ];
    value = result;
    m_pos += 8;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadUInt64BigEndian( UInt64& value )
{GUCEF_TRACE;

    if ( m_failed || m_size - m_pos < 8 )
        return Fail();
    const UInt8* src = m_data + m_pos;
    UInt64 result = 0;
    for ( UInt32 i=0; i<8; ++i )
        result = ( result << 8 ) | src[ i ];
    value = result;
    m_pos += 8;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadFloat32( Float32& value )
{GUCEF_TRACE;

    UInt32 bits = 0;
    if ( !ReadFixed32( bits ) )
        return false;
    memcpy( &value, &bits, 4 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadFloat64( Float64& value )
{GUCEF_TRACE;

    UInt64 bits = 0;
    if ( !ReadFixed64( bits ) )
        return false;
    memcpy( &value, &bits, 8 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadVarUInt64( UInt64& value )
{GUCEF_TRACE;

    if ( m_failed )
        return false;

    UInt64 result = 0;
    UInt32 shift = 0;
    UInt32 pos = m_pos;
    for ( UInt32 i=0; i<10; ++i )
    {
        if ( pos >= m_size )
            return Fail();
        UInt8 byte = m_data[ pos++ ];
        if ( 9 == i && byte > 1 )
            return Fail();      // the 10th byte may only carry the 64th bit
        result |= (UInt64) ( byte & 0x7F ) << shift;
        if ( 0 == ( byte & 0x80 ) )
        {
            m_pos = pos;
            value = result;
            return true;
        }
        shift += 7;
    }
    return Fail();
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadVarUInt32( UInt32& value )
{GUCEF_TRACE;

    UInt64 wide = 0;
    if ( !ReadVarUInt64( wide ) )
        return false;
    if ( wide > 0xFFFFFFFFULL )
        return Fail();
    value = (UInt32) wide;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadVarInt32( Int32& value )
{GUCEF_TRACE;

    UInt32 raw = 0;
    if ( !ReadVarUInt32( raw ) )
        return false;
    value = ZigZagDecode32( raw );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadVarInt64( Int64& value )
{GUCEF_TRACE;

    UInt64 raw = 0;
    if ( !ReadVarUInt64( raw ) )
        return false;
    value = ZigZagDecode64( raw );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadBytes( void* dest, UInt32 size )
{GUCEF_TRACE;

    if ( m_failed || m_size - m_pos < size )
        return Fail();
    if ( size > 0 )
        memcpy( dest, m_data + m_pos, size );
    m_pos += size;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::SkipBytes( UInt32 size )
{GUCEF_TRACE;

    if ( m_failed || m_size - m_pos < size )
        return Fail();
    m_pos += size;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadSizedBytes( const UInt8*& data, UInt32& size )
{GUCEF_TRACE;

    UInt32 length = 0;
    if ( !ReadVarUInt32( length ) )
        return false;
    if ( m_size - m_pos < length )
        return Fail();
    data = m_data + m_pos;
    size = length;
    m_pos += length;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadSizedBytes( CDynamicBuffer& dest )
{GUCEF_TRACE;

    const UInt8* data = GUCEF_NULL;
    UInt32 size = 0;
    if ( !ReadSizedBytes( data, size ) )
        return false;
    dest.Clear( true );
    if ( size > 0 )
        dest.CopyFrom( size, data );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::IsValidUtf8( const void* data, UInt32 size )
{GUCEF_TRACE;

    const UInt8* s = static_cast< const UInt8* >( data );
    UInt32 i = 0;
    while ( i < size )
    {
        UInt8 c = s[ i ];
        if ( c < 0x80 )
        {
            ++i;
            continue;
        }
        UInt32 needed = 0;
        UInt32 codePoint = 0;
        UInt32 minimum = 0;
        if ( 0xC0 == ( c & 0xE0 ) )      { needed = 1; codePoint = c & 0x1F; minimum = 0x80; }
        else if ( 0xE0 == ( c & 0xF0 ) ) { needed = 2; codePoint = c & 0x0F; minimum = 0x800; }
        else if ( 0xF0 == ( c & 0xF8 ) ) { needed = 3; codePoint = c & 0x07; minimum = 0x10000; }
        else
            return false;
        if ( size - i - 1 < needed )
            return false;
        for ( UInt32 n=1; n<=needed; ++n )
        {
            UInt8 cc = s[ i + n ];
            if ( 0x80 != ( cc & 0xC0 ) )
                return false;
            codePoint = ( codePoint << 6 ) | ( cc & 0x3F );
        }
        if ( codePoint < minimum || codePoint > 0x10FFFF || ( codePoint >= 0xD800 && codePoint <= 0xDFFF ) )
            return false;
        i += needed + 1;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadUtf8( const char*& utf8, UInt32& byteSize )
{GUCEF_TRACE;

    const UInt8* data = GUCEF_NULL;
    UInt32 size = 0;
    if ( !ReadSizedBytes( data, size ) )
        return false;
    if ( !IsValidUtf8( data, size ) )
        return Fail();
    utf8 = reinterpret_cast< const char* >( data );
    byteSize = size;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadString( CString& str )
{GUCEF_TRACE;

    const char* utf8 = GUCEF_NULL;
    UInt32 size = 0;
    if ( !ReadUtf8( utf8, size ) )
        return false;
    if ( 0 == size )
    {
        str.Clear();
        return true;
    }
    CUtf8String text( utf8, size );
    str = ToString( text );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadDecimal( CDecimal& value )
{GUCEF_TRACE;

    Int64 unscaled = 0;
    UInt32 scale = 0;
    if ( !ReadVarInt64( unscaled ) || !ReadVarUInt32( scale ) )
        return false;
    if ( scale > CDecimal::MaxScale )
        return Fail();
    if ( DECIMALSTATUS_OK != CDecimal::FromUnscaled( unscaled, (UInt8) scale, value ) )
        return Fail();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadDate( Int32& daysSinceEpoch )
{GUCEF_TRACE;

    return ReadVarInt32( daysSinceEpoch );
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadTimestamp( Int64& microsSinceEpoch )
{GUCEF_TRACE;

    return ReadVarInt64( microsSinceEpoch );
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadDateTime( CDateTime& value )
{GUCEF_TRACE;

    Int64 utcMs = 0;
    Int32 offsetMins = 0;
    if ( !ReadVarInt64( utcMs ) || !ReadVarInt32( offsetMins ) )
        return false;
    if ( offsetMins < -1440 || offsetMins > 1440 )
        return Fail();
    // The fields of the date time are local time: the UTC instant moved by the offset
    Int64 offsetMs = (Int64) offsetMins * 60000;
    if ( ( offsetMs > 0 && utcMs > GUCEF_INT64MAX - offsetMs ) || ( offsetMs < 0 && utcMs < GUCEF_INT64MIN - offsetMs ) )
        return Fail();
    CDateTime local;
    if ( !CDataTableValues::DateTimeFromTicks( utcMs + offsetMs, DATATABLE_TIMEUNIT_MILLISECONDS, local ) )
        return Fail();
    local.SetTimeZoneUTCOffsetInMins( (Int16) offsetMins );
    value = local;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::ReadVariant( CVariant& value, bool linkBinary )
{GUCEF_TRACE;

    UInt8 typeId = 0;
    if ( !ReadUInt8( typeId ) )
        return false;

    switch ( typeId )
    {
        case GUCEF_DATATYPE_BOOLEAN:
        {
            UInt64 v = 0;
            if ( !ReadVarUInt64( v ) || v > 1 )
                return Fail();
            value = ( 1 == v );
            return true;
        }
        case GUCEF_DATATYPE_UINT8:
        {
            UInt64 v = 0;
            if ( !ReadVarUInt64( v ) || v > 0xFF )
                return Fail();
            value = (UInt8) v;
            return true;
        }
        case GUCEF_DATATYPE_LE_UINT16:
        case GUCEF_DATATYPE_BE_UINT16:
        {
            UInt64 v = 0;
            if ( !ReadVarUInt64( v ) || v > 0xFFFF )
                return Fail();
            value = (UInt16) v;
            return true;
        }
        case GUCEF_DATATYPE_LE_UINT32:
        case GUCEF_DATATYPE_BE_UINT32:
        {
            UInt32 v = 0;
            if ( !ReadVarUInt32( v ) )
                return false;
            value = v;
            return true;
        }
        case GUCEF_DATATYPE_LE_UINT64:
        case GUCEF_DATATYPE_BE_UINT64:
        {
            UInt64 v = 0;
            if ( !ReadVarUInt64( v ) )
                return false;
            value = v;
            return true;
        }
        case GUCEF_DATATYPE_INT8:
        {
            Int64 v = 0;
            if ( !ReadVarInt64( v ) || v < -128 || v > 127 )
                return Fail();
            value = (Int8) v;
            return true;
        }
        case GUCEF_DATATYPE_LE_INT16:
        case GUCEF_DATATYPE_BE_INT16:
        {
            Int64 v = 0;
            if ( !ReadVarInt64( v ) || v < -32768 || v > 32767 )
                return Fail();
            value = (Int16) v;
            return true;
        }
        case GUCEF_DATATYPE_LE_INT32:
        case GUCEF_DATATYPE_BE_INT32:
        {
            Int32 v = 0;
            if ( !ReadVarInt32( v ) )
                return false;
            value = v;
            return true;
        }
        case GUCEF_DATATYPE_LE_INT64:
        case GUCEF_DATATYPE_BE_INT64:
        {
            Int64 v = 0;
            if ( !ReadVarInt64( v ) )
                return false;
            value = v;
            return true;
        }
        case GUCEF_DATATYPE_LE_FLOAT32:
        case GUCEF_DATATYPE_BE_FLOAT32:
        {
            Float32 v = 0;
            if ( !ReadFloat32( v ) )
                return false;
            value = v;
            return true;
        }
        case GUCEF_DATATYPE_LE_FLOAT64:
        case GUCEF_DATATYPE_BE_FLOAT64:
        {
            Float64 v = 0;
            if ( !ReadFloat64( v ) )
                return false;
            value = v;
            return true;
        }
        case GUCEF_DATATYPE_ASCII_STRING:
        case GUCEF_DATATYPE_UTF8_STRING:
        case GUCEF_DATATYPE_UTF16_LE_STRING:
        case GUCEF_DATATYPE_UTF16_BE_STRING:
        case GUCEF_DATATYPE_UTF32_LE_STRING:
        case GUCEF_DATATYPE_UTF32_BE_STRING:
        {
            const char* utf8 = GUCEF_NULL;
            UInt32 size = 0;
            if ( !ReadUtf8( utf8, size ) )
                return false;
            value = CUtf8String( utf8, size );
            return true;
        }
        case GUCEF_DATATYPE_BINARY_BLOB:
        case GUCEF_DATATYPE_BINARY_BSOB:
        {
            const UInt8* data = GUCEF_NULL;
            UInt32 size = 0;
            if ( !ReadSizedBytes( data, size ) )
                return false;
            if ( linkBinary )
                value.LinkTo( data, size, GUCEF_DATATYPE_BINARY_BLOB );
            else
                value.Set( data, size, GUCEF_DATATYPE_BINARY_BLOB );
            return true;
        }
        case GUCEF_DATATYPE_INT64_UINT8_LE_DECIMAL:
        case GUCEF_DATATYPE_INT64_UINT8_BE_DECIMAL:
        {
            CDecimal decimal;
            if ( !ReadDecimal( decimal ) )
                return false;
            value = decimal;
            return true;
        }
        case GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        case GUCEF_DATATYPE_BE_DATE_IN_DAYS_SINCE_UNIX_EPOCH:
        {
            Int32 days = 0;
            if ( !ReadDate( days ) )
                return false;
            value = CDataTableValues::Date( days );
            return true;
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
            Int64 ticks = 0;
            if ( !ReadVarInt64( ticks ) )
                return false;

            // Written in little endian form; give the native form of the same unit
            UInt8 nativeTypeId = typeId;
            switch ( typeId )
            {
                case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH:
                case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH: nativeTypeId = GUCEF_DATATYPE_TIMESTAMP_IN_MICROS_SINCE_UNIX_EPOCH; break;
                case GUCEF_DATATYPE_LE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH:
                case GUCEF_DATATYPE_BE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH: nativeTypeId = GUCEF_DATATYPE_TIMESTAMP_IN_MS_SINCE_UNIX_EPOCH; break;
                case GUCEF_DATATYPE_LE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH:
                case GUCEF_DATATYPE_BE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH: nativeTypeId = GUCEF_DATATYPE_TIMESTAMP_IN_NANOS_SINCE_UNIX_EPOCH; break;
                default: nativeTypeId = GUCEF_DATATYPE_TIMESTAMP_IN_SECS_SINCE_UNIX_EPOCH; break;
            }
            value.Set( &ticks, sizeof( ticks ), nativeTypeId );
            return true;
        }
        case GUCEF_DATATYPE_NIL:
        case GUCEF_DATATYPE_NULL:
        case GUCEF_DATATYPE_VOID:
        {
            value.Clear();
            if ( GUCEF_DATATYPE_VOID == typeId )
                value.SetVoidType();
            else
                value.OverrideTypeId( typeId );
            return true;
        }
        case GUCEF_DATATYPE_CONST_DATA_MEMORY_ADDRESS:
        case GUCEF_DATATYPE_DATA_MEMORY_ADDRESS:
        case GUCEF_DATATYPE_FUNCTION_MEMORY_ADDRESS:
        {
            return Fail();
        }
        default:
        {
            const char* utf8 = GUCEF_NULL;
            UInt32 size = 0;
            if ( !ReadUtf8( utf8, size ) )
                return false;
            CVariant parsed( typeId, ToString( CUtf8String( utf8, size ) ) );
            if ( typeId != parsed.GetTypeId() )
                return Fail();
            value = parsed;
            return true;
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CBinaryWireReader::GetRemaining( void ) const
{GUCEF_TRACE;

    return m_size - m_pos;
}

/*-------------------------------------------------------------------------*/

UInt32
CBinaryWireReader::GetPosition( void ) const
{GUCEF_TRACE;

    return m_pos;
}

/*-------------------------------------------------------------------------*/

UInt32
CBinaryWireReader::GetSize( void ) const
{GUCEF_TRACE;

    return m_size;
}

/*-------------------------------------------------------------------------*/

const UInt8*
CBinaryWireReader::GetCurrentPtr( void ) const
{GUCEF_TRACE;

    return m_data + m_pos;
}

/*-------------------------------------------------------------------------*/

const UInt8*
CBinaryWireReader::GetData( void ) const
{GUCEF_TRACE;

    return m_data;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::HasFailed( void ) const
{GUCEF_TRACE;

    return m_failed;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryWireReader::IsAtEnd( void ) const
{GUCEF_TRACE;

    return m_pos >= m_size;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
