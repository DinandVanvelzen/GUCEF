/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRN_CDRNWireReader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNWireReader::CDRNWireReader( const void* data, UInt32 size )
    : m_data( static_cast< const UInt8* >( data ) )
    , m_size( GUCEF_NULL != data ? size : 0 )
    , m_pos( 0 )
    , m_failed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNWireReader::CDRNWireReader( const CORE::CDynamicBuffer& buffer, UInt32 startOffset )
    : m_data( static_cast< const UInt8* >( buffer.GetConstBufferPtr() ) )
    , m_size( buffer.GetDataSize() )
    , m_pos( startOffset <= buffer.GetDataSize() ? startOffset : buffer.GetDataSize() )
    , m_failed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDRNWireReader::Fail( void )
{GUCEF_TRACE;
    m_failed = true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNWireReader::ReadUInt8( UInt8& value )
{GUCEF_TRACE;
    if ( m_failed || m_size - m_pos < 1 )
        return Fail();
    value = m_data[ m_pos++ ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNWireReader::ReadUInt16( UInt16& value )
{GUCEF_TRACE;
    if ( m_failed || m_size - m_pos < 2 )
        return Fail();
    value = (UInt16) ( ( (UInt16) m_data[ m_pos ] << 8 ) | m_data[ m_pos+1 ] );
    m_pos += 2;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNWireReader::ReadUInt32( UInt32& value )
{GUCEF_TRACE;
    if ( m_failed || m_size - m_pos < 4 )
        return Fail();
    const UInt8* p = m_data + m_pos;
    value = ( (UInt32) p[ 0 ] << 24 ) | ( (UInt32) p[ 1 ] << 16 ) | ( (UInt32) p[ 2 ] << 8 ) | (UInt32) p[ 3 ];
    m_pos += 4;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNWireReader::ReadUInt64( UInt64& value )
{GUCEF_TRACE;
    if ( m_failed || m_size - m_pos < 8 )
        return Fail();
    const UInt8* p = m_data + m_pos;
    UInt64 v = 0;
    for ( UInt32 i=0; i<8; ++i )
        v = ( v << 8 ) | p[ i ];
    value = v;
    m_pos += 8;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNWireReader::ReadFloat32( Float32& value )
{GUCEF_TRACE;
    UInt32 bits = 0;
    if ( !ReadUInt32( bits ) )
        return false;
    memcpy( &value, &bits, 4 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNWireReader::ReadFloat64( Float64& value )
{GUCEF_TRACE;
    UInt64 bits = 0;
    if ( !ReadUInt64( bits ) )
        return false;
    memcpy( &value, &bits, 8 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNWireReader::ReadBool( bool& value )
{GUCEF_TRACE;
    UInt8 b = 0;
    if ( !ReadUInt8( b ) )
        return false;
    value = 0 != b;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNWireReader::ReadVarUInt64( UInt64& value )
{GUCEF_TRACE;
    if ( m_failed )
        return false;

    UInt64 result = 0;
    UInt32 shift = 0;
    while ( m_pos < m_size )
    {
        UInt8 b = m_data[ m_pos++ ];
        if ( shift == 63 && b > 1 )
            return Fail();                      // more than 64 bits
        result |= (UInt64) ( b & 0x7F ) << shift;
        if ( 0 == ( b & 0x80 ) )
        {
            value = result;
            return true;
        }
        shift += 7;
        if ( shift > 63 )
            return Fail();
    }
    return Fail();
}

/*-------------------------------------------------------------------------*/

bool
CDRNWireReader::ReadVarUInt32( UInt32& value )
{GUCEF_TRACE;
    if ( m_failed )
        return false;

    // fast path for the common single byte case
    if ( m_pos < m_size && m_data[ m_pos ] < 0x80 )
    {
        value = m_data[ m_pos++ ];
        return true;
    }

    UInt64 wide = 0;
    if ( !ReadVarUInt64( wide ) )
        return false;
    if ( wide > 0xFFFFFFFFUL )
        return Fail();
    value = (UInt32) wide;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNWireReader::ReadVarInt32( Int32& value )
{GUCEF_TRACE;
    UInt32 raw = 0;
    if ( !ReadVarUInt32( raw ) )
        return false;
    value = ZigZagDecode32( raw );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNWireReader::ReadVarInt64( Int64& value )
{GUCEF_TRACE;
    UInt64 raw = 0;
    if ( !ReadVarUInt64( raw ) )
        return false;
    value = ZigZagDecode64( raw );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNWireReader::ReadBytes( void* dest, UInt32 size )
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
CDRNWireReader::SkipBytes( UInt32 size )
{GUCEF_TRACE;
    if ( m_failed || m_size - m_pos < size )
        return Fail();
    m_pos += size;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNWireReader::ReadSizedBytes( const UInt8*& data, UInt32& size )
{GUCEF_TRACE;
    UInt32 len = 0;
    if ( !ReadVarUInt32( len ) )
        return false;
    if ( m_size - m_pos < len )
        return Fail();
    data = m_data + m_pos;
    size = len;
    m_pos += len;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNWireReader::ReadSizedBytes( CORE::CDynamicBuffer& dest )
{GUCEF_TRACE;
    const UInt8* data = GUCEF_NULL;
    UInt32 size = 0;
    if ( !ReadSizedBytes( data, size ) )
        return false;
    dest.SetDataSize( 0 );
    if ( size > 0 )
        dest.CopyFrom( 0, size, data );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNWireReader::ReadString( CORE::CString& str )
{GUCEF_TRACE;

    const UInt8* data = GUCEF_NULL;
    UInt32 size = 0;
    if ( !ReadSizedBytes( data, size ) )
        return false;
    if ( 0 == size )
    {
        str.Clear();
        return true;
    }
    CORE::CUtf8String utf8( reinterpret_cast< const char* >( data ), size );
    str = CORE::ToString( utf8 );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNWireReader::GetRemaining( void ) const
{GUCEF_TRACE;
    return m_size - m_pos;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNWireReader::GetPosition( void ) const
{GUCEF_TRACE;
    return m_pos;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNWireReader::GetSize( void ) const
{GUCEF_TRACE;
    return m_size;
}

/*-------------------------------------------------------------------------*/

const UInt8*
CDRNWireReader::GetCurrentPtr( void ) const
{GUCEF_TRACE;
    return m_data + m_pos;
}

/*-------------------------------------------------------------------------*/

bool
CDRNWireReader::HasFailed( void ) const
{GUCEF_TRACE;
    return m_failed;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
