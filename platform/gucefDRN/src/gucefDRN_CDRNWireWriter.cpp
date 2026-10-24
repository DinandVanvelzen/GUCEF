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

#include "gucefDRN_CDRNWireWriter.h"

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

CDRNWireWriter::CDRNWireWriter( CORE::CDynamicBuffer& buffer, UInt32 startOffset )
    : m_buffer( &buffer )
    , m_offset( startOffset )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt8*
CDRNWireWriter::MakeRoom( UInt32 size )
{GUCEF_TRACE;
    UInt32 required = m_offset + size;
    UInt32 bufferSize = m_buffer->GetBufferSize();
    if ( required > bufferSize )
    {
        UInt32 newSize = bufferSize < 64 ? 64 : bufferSize;
        while ( newSize < required )
            newSize *= 2;
        m_buffer->SetBufferSize( newSize, false );
    }
    UInt8* dest = static_cast< UInt8* >( m_buffer->GetBufferPtr( m_offset ) );
    m_offset = required;
    if ( m_buffer->GetDataSize() < required )
        m_buffer->SetDataSize( required );
    return dest;
}

/*-------------------------------------------------------------------------*/

UInt8*
CDRNWireWriter::Reserve( UInt32 size )
{GUCEF_TRACE;

    return MakeRoom( size );
}

/*-------------------------------------------------------------------------*/

void
CDRNWireWriter::WriteUInt8( UInt8 value )
{GUCEF_TRACE;
    *MakeRoom( 1 ) = value;
}

/*-------------------------------------------------------------------------*/

void
CDRNWireWriter::WriteUInt16( UInt16 value )
{GUCEF_TRACE;
    UInt8* dest = MakeRoom( 2 );
    dest[ 0 ] = (UInt8) ( value >> 8 );
    dest[ 1 ] = (UInt8) value;
}

/*-------------------------------------------------------------------------*/

void
CDRNWireWriter::WriteUInt32( UInt32 value )
{GUCEF_TRACE;
    UInt8* dest = MakeRoom( 4 );
    dest[ 0 ] = (UInt8) ( value >> 24 );
    dest[ 1 ] = (UInt8) ( value >> 16 );
    dest[ 2 ] = (UInt8) ( value >> 8 );
    dest[ 3 ] = (UInt8) value;
}

/*-------------------------------------------------------------------------*/

void
CDRNWireWriter::WriteUInt64( UInt64 value )
{GUCEF_TRACE;
    UInt8* dest = MakeRoom( 8 );
    for ( Int32 i=7; i>=0; --i )
    {
        dest[ i ] = (UInt8) value;
        value >>= 8;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNWireWriter::WriteFloat32( Float32 value )
{GUCEF_TRACE;
    UInt32 bits = 0;
    memcpy( &bits, &value, 4 );
    WriteUInt32( bits );
}

/*-------------------------------------------------------------------------*/

void
CDRNWireWriter::WriteFloat64( Float64 value )
{GUCEF_TRACE;
    UInt64 bits = 0;
    memcpy( &bits, &value, 8 );
    WriteUInt64( bits );
}

/*-------------------------------------------------------------------------*/

void
CDRNWireWriter::WriteBool( bool value )
{GUCEF_TRACE;
    WriteUInt8( value ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNWireWriter::EncodeVarUInt64( UInt64 value, UInt8* dest )
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

void
CDRNWireWriter::WriteVarUInt32( UInt32 value )
{GUCEF_TRACE;
    if ( value < 0x80 )
    {
        *MakeRoom( 1 ) = (UInt8) value;
        return;
    }
    UInt8 tmp[ 10 ];
    UInt32 n = EncodeVarUInt64( value, tmp );
    memcpy( MakeRoom( n ), tmp, n );
}

/*-------------------------------------------------------------------------*/

void
CDRNWireWriter::WriteVarUInt64( UInt64 value )
{GUCEF_TRACE;
    if ( value < 0x80 )
    {
        *MakeRoom( 1 ) = (UInt8) value;
        return;
    }
    UInt8 tmp[ 10 ];
    UInt32 n = EncodeVarUInt64( value, tmp );
    memcpy( MakeRoom( n ), tmp, n );
}

/*-------------------------------------------------------------------------*/

void
CDRNWireWriter::WriteVarInt32( Int32 value )
{GUCEF_TRACE;
    WriteVarUInt32( ZigZagEncode32( value ) );
}

/*-------------------------------------------------------------------------*/

void
CDRNWireWriter::WriteVarInt64( Int64 value )
{GUCEF_TRACE;
    WriteVarUInt64( ZigZagEncode64( value ) );
}

/*-------------------------------------------------------------------------*/

void
CDRNWireWriter::WriteBytes( const void* data, UInt32 size )
{GUCEF_TRACE;
    if ( 0 == size )
        return;
    memcpy( MakeRoom( size ), data, size );
}

/*-------------------------------------------------------------------------*/

void
CDRNWireWriter::WriteSizedBytes( const void* data, UInt32 size )
{GUCEF_TRACE;
    WriteVarUInt32( size );
    WriteBytes( data, size );
}

/*-------------------------------------------------------------------------*/

void
CDRNWireWriter::WriteString( const CORE::CString& str )
{GUCEF_TRACE;

    CORE::CUtf8String utf8 = CORE::ToUtf8String( str );
    UInt32 byteSize = utf8.ByteSize();
    UInt32 size = byteSize > 0 ? byteSize - 1 : 0;
    WriteSizedBytes( utf8.C_String(), size );
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNWireWriter::GetOffset( void ) const
{GUCEF_TRACE;
    return m_offset;
}

/*-------------------------------------------------------------------------*/

void
CDRNWireWriter::SetOffset( UInt32 offset )
{GUCEF_TRACE;
    m_offset = offset;
}

/*-------------------------------------------------------------------------*/

CORE::CDynamicBuffer&
CDRNWireWriter::GetBuffer( void )
{GUCEF_TRACE;
    return *m_buffer;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNWireWriter::GetVarUInt32Size( UInt32 value )
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
CDRNWireWriter::GetVarUInt64Size( UInt64 value )
{GUCEF_TRACE;
    UInt32 n = 1;
    while ( value >= 0x80 )
    {
        value >>= 7;
        ++n;
    }
    return n;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
