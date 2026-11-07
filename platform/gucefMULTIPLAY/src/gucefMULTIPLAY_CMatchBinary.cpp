/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#include "gucefMULTIPLAY_CMatchBinary.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMatchBinaryWriter::CMatchBinaryWriter( CORE::CDynamicBuffer& target )
    : m_target( target )
{GUCEF_TRACE;

    // many small appends: the buffer grows by a percentage, not by exactly what each append needs
    m_target.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );

}

/*-------------------------------------------------------------------------*/

void
CMatchBinaryWriter::WriteUInt8( UInt8 value )
{GUCEF_TRACE;

    m_target.Append( &value, 1 );
}

/*-------------------------------------------------------------------------*/

void
CMatchBinaryWriter::WriteUInt16( UInt16 value )
{GUCEF_TRACE;

    UInt8 bytes[ 2 ] = { (UInt8)( value >> 8 ), (UInt8) value };
    m_target.Append( bytes, 2 );
}

/*-------------------------------------------------------------------------*/

void
CMatchBinaryWriter::WriteUInt32( UInt32 value )
{GUCEF_TRACE;

    UInt8 bytes[ 4 ] = { (UInt8)( value >> 24 ), (UInt8)( value >> 16 ), (UInt8)( value >> 8 ), (UInt8) value };
    m_target.Append( bytes, 4 );
}

/*-------------------------------------------------------------------------*/

void
CMatchBinaryWriter::WriteUInt64( UInt64 value )
{GUCEF_TRACE;

    WriteUInt32( (UInt32)( value >> 32 ) );
    WriteUInt32( (UInt32)( value & 0xFFFFFFFFu ) );
}

/*-------------------------------------------------------------------------*/

void
CMatchBinaryWriter::WriteInt64( Int64 value )
{GUCEF_TRACE;

    WriteUInt64( (UInt64) value );
}

/*-------------------------------------------------------------------------*/

void
CMatchBinaryWriter::WriteBool( bool value )
{GUCEF_TRACE;

    WriteUInt8( value ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

void
CMatchBinaryWriter::WriteRaw( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( size > 0 && GUCEF_NULL != data )
        m_target.Append( data, size );
}

/*-------------------------------------------------------------------------*/

void
CMatchBinaryWriter::WriteBytes( const void* data, UInt32 size )
{GUCEF_TRACE;

    WriteUInt32( size );
    WriteRaw( data, size );
}

/*-------------------------------------------------------------------------*/

void
CMatchBinaryWriter::WriteString( const CORE::CString& text )
{GUCEF_TRACE;

    // the strings are UTF-8: the length that counts is the one in bytes
    WriteBytes( text.C_String(), (UInt32) strlen( text.C_String() ) );
}

/*-------------------------------------------------------------------------*/

static bool
IsValidUtf8( const UInt8* p, UInt32 size )
{GUCEF_TRACE;

    UInt32 i = 0;
    while ( i < size )
    {
        UInt8 c = p[ i ];
        if ( 0 == c )
            return false;
        if ( c < 0x80 )
        {
            ++i;
            continue;
        }
        UInt32 extra = 0;
        UInt32 minimum = 0;
        UInt32 point = 0;
        if ( c >= 0xC2 && c <= 0xDF ) { extra = 1; minimum = 0x80; point = c & 0x1F; }
        else
        if ( c >= 0xE0 && c <= 0xEF ) { extra = 2; minimum = 0x800; point = c & 0x0F; }
        else
        if ( c >= 0xF0 && c <= 0xF4 ) { extra = 3; minimum = 0x10000; point = c & 0x07; }
        else
            return false;
        if ( i + extra >= size )
            return false;
        for ( UInt32 j=1; j<=extra; ++j )
        {
            if ( ( p[ i + j ] & 0xC0 ) != 0x80 )
                return false;
            point = ( point << 6 ) | ( p[ i + j ] & 0x3F );
        }
        if ( point < minimum || point > 0x10FFFF || ( point >= 0xD800 && point <= 0xDFFF ) )
            return false;
        i += extra + 1;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CMatchBinaryReader::CMatchBinaryReader( const void* data, UInt32 size )
    : m_data( static_cast< const UInt8* >( data ) )
    , m_size( GUCEF_NULL == data ? 0 : size )
    , m_position( 0 )
    , m_ok( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const UInt8*
CMatchBinaryReader::Take( UInt32 size )
{GUCEF_TRACE;

    if ( !m_ok || size > m_size - m_position )
    {
        m_ok = false;
        return GUCEF_NULL;
    }
    const UInt8* result = m_data + m_position;
    m_position += size;
    return result;
}

/*-------------------------------------------------------------------------*/

UInt8
CMatchBinaryReader::ReadUInt8( void )
{GUCEF_TRACE;

    const UInt8* p = Take( 1 );
    return GUCEF_NULL == p ? 0 : p[ 0 ];
}

/*-------------------------------------------------------------------------*/

UInt16
CMatchBinaryReader::ReadUInt16( void )
{GUCEF_TRACE;

    const UInt8* p = Take( 2 );
    return GUCEF_NULL == p ? 0 : (UInt16)( ( p[ 0 ] << 8 ) | p[ 1 ] );
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchBinaryReader::ReadUInt32( void )
{GUCEF_TRACE;

    const UInt8* p = Take( 4 );
    if ( GUCEF_NULL == p )
        return 0;
    return ( (UInt32) p[ 0 ] << 24 ) | ( (UInt32) p[ 1 ] << 16 ) | ( (UInt32) p[ 2 ] << 8 ) | (UInt32) p[ 3 ];
}

/*-------------------------------------------------------------------------*/

UInt64
CMatchBinaryReader::ReadUInt64( void )
{GUCEF_TRACE;

    UInt64 high = ReadUInt32();
    UInt64 low = ReadUInt32();
    return ( high << 32 ) | low;
}

/*-------------------------------------------------------------------------*/

Int64
CMatchBinaryReader::ReadInt64( void )
{GUCEF_TRACE;

    return (Int64) ReadUInt64();
}

/*-------------------------------------------------------------------------*/

bool
CMatchBinaryReader::ReadBool( void )
{GUCEF_TRACE;

    UInt8 value = ReadUInt8();
    if ( value > 1 )
        m_ok = false;
    return 1 == value;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBinaryReader::ReadRaw( void* destination, UInt32 size )
{GUCEF_TRACE;

    const UInt8* p = Take( size );
    if ( GUCEF_NULL == p )
        return false;
    if ( size > 0 )
        memcpy( destination, p, size );
    return true;
}

/*-------------------------------------------------------------------------*/

const UInt8*
CMatchBinaryReader::ReadRawInPlace( UInt32 size )
{GUCEF_TRACE;

    return Take( size );
}

/*-------------------------------------------------------------------------*/

const UInt8*
CMatchBinaryReader::ReadBytesInPlace( UInt32& size, UInt32 maxLength )
{GUCEF_TRACE;

    size = ReadUInt32();
    if ( !m_ok )
    {
        size = 0;
        return GUCEF_NULL;
    }
    if ( size > maxLength )
    {
        m_ok = false;
        size = 0;
        return GUCEF_NULL;
    }
    const UInt8* p = Take( size );
    if ( GUCEF_NULL == p )
        size = 0;
    return p;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBinaryReader::ReadString( CORE::CString& text, UInt32 maxLength )
{GUCEF_TRACE;

    UInt32 size = 0;
    const UInt8* p = ReadBytesInPlace( size, maxLength );
    if ( !m_ok )
    {
        text.Clear();
        return false;
    }
    // a text is valid UTF-8 without a zero byte: nothing else is made into a string
    if ( !IsValidUtf8( p, size ) )
    {
        m_ok = false;
        text.Clear();
        return false;
    }
    text.Set( size > 0 ? reinterpret_cast< const char* >( p ) : "", size );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBinaryReader::IsOk( void ) const
{GUCEF_TRACE;

    return m_ok;
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchBinaryReader::GetRemaining( void ) const
{GUCEF_TRACE;

    return m_size - m_position;
}

/*-------------------------------------------------------------------------*/

UInt32
CMatchBinaryReader::GetPosition( void ) const
{GUCEF_TRACE;

    return m_position;
}

/*-------------------------------------------------------------------------*/

bool
CMatchBinaryReader::IsAtEndAndOk( void ) const
{GUCEF_TRACE;

    return m_ok && m_position == m_size;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
