/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_COM_NETWORKBYTEORDER_H
#define GUCEF_COM_NETWORKBYTEORDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"      /* often used gucefCOM macros */
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Appends values in network byte order (big endian) to a buffer, the way the wire formats of the protocols here
 *  want them. The writes are byte by byte so the host byte order does not matter.
 */
class CNetworkByteWriter
{
    public:

    explicit CNetworkByteWriter( CORE::CDynamicBuffer& buffer )
        : m_buffer( buffer )
    {
    }

    void WriteUInt8( UInt8 value )
    {
        m_buffer.Append( &value, 1, true );
    }

    void WriteUInt16( UInt16 value )
    {
        UInt8 bytes[ 2 ] = { (UInt8)( value >> 8 ), (UInt8)( value ) };
        m_buffer.Append( bytes, 2, true );
    }

    void WriteUInt32( UInt32 value )
    {
        UInt8 bytes[ 4 ] = { (UInt8)( value >> 24 ), (UInt8)( value >> 16 ), (UInt8)( value >> 8 ), (UInt8)( value ) };
        m_buffer.Append( bytes, 4, true );
    }

    void WriteUInt64( UInt64 value )
    {
        WriteUInt32( (UInt32)( value >> 32 ) );
        WriteUInt32( (UInt32)( value ) );
    }

    void WriteBytes( const void* data, UInt32 size )
    {
        if ( size > 0 )
            m_buffer.Append( data, size, true );
    }

    /** Writes the bytes of the string without a terminator */
    void WriteStringBytes( const CORE::CString& str )
    {
        UInt32 size = GetStringByteLength( str );
        if ( size > 0 )
            m_buffer.Append( str.C_String(), size, true );
    }

    /** Overwrites a 16 bit value written earlier, for example a length that is only known later */
    void PatchUInt16( UInt32 offset, UInt16 value )
    {
        m_buffer.SetUByte( offset, (UInt8)( value >> 8 ) );
        m_buffer.SetUByte( offset + 1, (UInt8)( value ) );
    }

    UInt32 GetSize( void ) const
    {
        return m_buffer.GetDataSize();
    }

    /** The number of bytes of the string without its terminator */
    static UInt32 GetStringByteLength( const CORE::CString& str )
    {
        UInt32 byteSize = str.ByteSize();
        return byteSize > 0 ? byteSize - 1 : 0;
    }

    private:

    CORE::CDynamicBuffer& m_buffer;
};

/*-------------------------------------------------------------------------*/

/**
 *  Reads values in network byte order (big endian) from a block of memory. Every read is checked against the end
 *  of the data: once a read runs past it the reader is in the failed state and all further reads fail too.
 */
class CNetworkByteReader
{
    public:

    CNetworkByteReader( const void* data, UInt32 size )
        : m_data( static_cast< const UInt8* >( data ) )
        , m_size( GUCEF_NULL != data ? size : 0 )
        , m_offset( 0 )
        , m_failed( false )
    {
    }

    bool ReadUInt8( UInt8& value )
    {
        if ( !Require( 1 ) )
            return false;
        value = m_data[ m_offset ];
        ++m_offset;
        return true;
    }

    bool ReadUInt16( UInt16& value )
    {
        if ( !Require( 2 ) )
            return false;
        value = (UInt16)( ( m_data[ m_offset ] << 8 ) | m_data[ m_offset+1 ] );
        m_offset += 2;
        return true;
    }

    bool ReadUInt32( UInt32& value )
    {
        if ( !Require( 4 ) )
            return false;
        value = ( (UInt32) m_data[ m_offset ] << 24 ) | ( (UInt32) m_data[ m_offset+1 ] << 16 ) |
                ( (UInt32) m_data[ m_offset+2 ] << 8 ) | (UInt32) m_data[ m_offset+3 ];
        m_offset += 4;
        return true;
    }

    bool ReadUInt64( UInt64& value )
    {
        UInt32 high = 0;
        UInt32 low = 0;
        if ( !ReadUInt32( high ) || !ReadUInt32( low ) )
            return false;
        value = ( (UInt64) high << 32 ) | low;
        return true;
    }

    bool ReadBytes( void* dest, UInt32 size )
    {
        if ( !Require( size ) )
            return false;
        if ( size > 0 )
            memcpy( dest, m_data + m_offset, size );
        m_offset += size;
        return true;
    }

    /** Reads the given number of bytes as a UTF-8 string */
    bool ReadString( UInt32 size, CORE::CString& str )
    {
        if ( !Require( size ) )
            return false;
        if ( size > 0 )
            str.Set( reinterpret_cast< const char* >( m_data + m_offset ), size );
        else
            str.Clear();
        m_offset += size;
        return true;
    }

    bool Skip( UInt32 size )
    {
        if ( !Require( size ) )
            return false;
        m_offset += size;
        return true;
    }

    bool Seek( UInt32 offset )
    {
        if ( m_failed || offset > m_size )
        {
            m_failed = true;
            return false;
        }
        m_offset = offset;
        return true;
    }

    UInt32 GetOffset( void ) const { return m_offset; }
    UInt32 GetSize( void ) const { return m_size; }
    UInt32 GetRemaining( void ) const { return m_size - m_offset; }
    bool HasFailed( void ) const { return m_failed; }
    const UInt8* GetData( void ) const { return m_data; }

    private:

    bool Require( UInt32 size )
    {
        if ( m_failed || size > m_size - m_offset )
        {
            m_failed = true;
            return false;
        }
        return true;
    }

    const UInt8* m_data;
    UInt32 m_size;
    UInt32 m_offset;
    bool m_failed;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_NETWORKBYTEORDER_H ? */
