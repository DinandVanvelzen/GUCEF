/*
 *  assetpluginUNREAL: Little endian primitive reader over a range of a resource, with a read window and a sticky error state
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

#include <stdio.h>
#include <string.h>

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H
#include "assetpluginUNREAL_CUnrealArchiveReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnrealArchiveReader::CUnrealArchiveReader( void )
    : m_access()
    , m_keepAlive()
    , m_bias( 0 )
    , m_start( 0 )
    , m_end( 0 )
    , m_hardEnd( 0 )
    , m_position( 0 )
    , m_window()
    , m_windowStart( 0 )
    , m_windowLength( 0 )
    , m_bytesFetched( 0 )
    , m_error( false )
    , m_errorText()
    , m_errorOffset( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnrealArchiveReader::~CUnrealArchiveReader()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

bool
CUnrealArchiveReader::Open( const CORE::IOAccessPtr& access, UInt64 start, UInt64 end, UInt32 windowBytes, UInt64 bias )
{GUCEF_TRACE;

    Close();
    if ( access.IsNULL() || end < start )
        return false;
    m_access = access;
    m_bias = bias;
    m_start = start;
    m_end = end;
    m_hardEnd = end;
    m_position = start;
    m_window.resize( windowBytes < 4096 ? 4096 : windowBytes );
    m_windowStart = 0;
    m_windowLength = 0;
    m_bytesFetched = 0;
    m_error = false;
    m_errorText.Clear();
    m_errorOffset = 0;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CUnrealArchiveReader::SetKeepAlive( const VFS::TBasicVfsResourcePtr& resource )
{GUCEF_TRACE;

    m_keepAlive = resource;
}

/*-------------------------------------------------------------------------*/

void
CUnrealArchiveReader::Close( void )
{GUCEF_TRACE;

    m_access.Unlink();
    m_keepAlive.Unlink();
    m_window.clear();
    m_windowLength = 0;
    m_bias = 0;
    m_start = m_end = m_hardEnd = m_position = 0;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealArchiveReader::IsOpen( void ) const
{GUCEF_TRACE;

    return !m_access.IsNULL();
}

/*-------------------------------------------------------------------------*/

UInt64
CUnrealArchiveReader::GetStart( void ) const
{GUCEF_TRACE;

    return m_start;
}

/*-------------------------------------------------------------------------*/

UInt64
CUnrealArchiveReader::GetEnd( void ) const
{GUCEF_TRACE;

    return m_end;
}

/*-------------------------------------------------------------------------*/

UInt64
CUnrealArchiveReader::GetPosition( void ) const
{GUCEF_TRACE;

    return m_position;
}

/*-------------------------------------------------------------------------*/

UInt64
CUnrealArchiveReader::GetRemaining( void ) const
{GUCEF_TRACE;

    return m_position < m_end ? m_end - m_position : 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CUnrealArchiveReader::GetBytesFetched( void ) const
{GUCEF_TRACE;

    return m_bytesFetched;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealArchiveReader::HasError( void ) const
{GUCEF_TRACE;

    return m_error;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CUnrealArchiveReader::GetError( void ) const
{GUCEF_TRACE;

    return m_errorText;
}

/*-------------------------------------------------------------------------*/

UInt64
CUnrealArchiveReader::GetErrorOffset( void ) const
{GUCEF_TRACE;

    return m_errorOffset;
}

/*-------------------------------------------------------------------------*/

void
CUnrealArchiveReader::ClearError( void )
{GUCEF_TRACE;

    m_error = false;
    m_errorText.Clear();
    m_errorOffset = 0;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealArchiveReader::SetError( const CORE::CString& reason )
{GUCEF_TRACE;

    if ( !m_error )
    {
        m_error = true;
        m_errorText = reason;
        m_errorOffset = m_position;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealArchiveReader::Seek( UInt64 position )
{GUCEF_TRACE;

    if ( m_error )
        return false;
    if ( position < m_start || position > m_end )
        return SetError( "Seek to offset " + CORE::UInt64ToString( position ) + " outside the range " + CORE::UInt64ToString( m_start ) + " to " + CORE::UInt64ToString( m_end ) );
    m_position = position;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealArchiveReader::Skip( UInt64 bytes )
{GUCEF_TRACE;

    if ( m_error )
        return false;
    if ( bytes > GetRemaining() )
        return SetError( "Skip of " + CORE::UInt64ToString( bytes ) + " bytes goes beyond the end of the data" );
    m_position += bytes;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealArchiveReader::Fill( UInt64 position )
{GUCEF_TRACE;

    UInt64 length = m_hardEnd - position;
    if ( length > m_window.size() )
        length = m_window.size();
    if ( 0 != m_access->Seek( (Int64) ( position + m_bias ), SEEK_SET ) )
        return false;
    const UInt32 got = m_access->Read( &m_window[ 0 ], 1, (UInt32) length );
    m_bytesFetched += got;
    m_windowStart = position;
    m_windowLength = got;
    return got > 0;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealArchiveReader::Read( void* dest, UInt64 size )
{GUCEF_TRACE;

    if ( m_error )
        return false;
    if ( size > GetRemaining() )
        return SetError( "Read of " + CORE::UInt64ToString( size ) + " bytes goes beyond the end of the data (" + CORE::UInt64ToString( GetRemaining() ) + " bytes remain)" );
    UInt8* out = static_cast< UInt8* >( dest );
    UInt64 left = size;
    while ( left > 0 )
    {
        const bool inWindow = m_position >= m_windowStart && m_position < m_windowStart + m_windowLength;
        if ( !inWindow )
        {
            if ( left >= m_window.size() )
            {
                // large reads go straight to the resource
                const UInt32 chunk = left > 0x40000000 ? 0x40000000 : (UInt32) left;
                if ( 0 != m_access->Seek( (Int64) ( m_position + m_bias ), SEEK_SET ) || chunk != m_access->Read( out, 1, chunk ) )
                    return SetError( "The resource could not be read at offset " + CORE::UInt64ToString( m_position ) );
                m_bytesFetched += chunk;
                m_position += chunk;
                out += chunk;
                left -= chunk;
                continue;
            }
            if ( !Fill( m_position ) || !( m_position >= m_windowStart && m_position < m_windowStart + m_windowLength ) )
                return SetError( "The resource could not be read at offset " + CORE::UInt64ToString( m_position ) );
        }
        UInt64 available = m_windowStart + m_windowLength - m_position;
        if ( available > left )
            available = left;
        memcpy( out, &m_window[ (size_t) ( m_position - m_windowStart ) ], (size_t) available );
        m_position += available;
        out += available;
        left -= available;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealArchiveReader::ReadRaw( void* dest, UInt32 size )
{GUCEF_TRACE;

    return Read( dest, size );
}

/*-------------------------------------------------------------------------*/

UInt8
CUnrealArchiveReader::ReadUInt8( void )
{GUCEF_TRACE;

    if ( !m_error && m_position < m_end && m_position >= m_windowStart && m_position < m_windowStart + m_windowLength )
        return m_window[ (size_t) ( m_position++ - m_windowStart ) ];
    UInt8 b[ 1 ] = { 0 };
    ReadRaw( b, 1 );
    return b[ 0 ];
}

/*-------------------------------------------------------------------------*/

Int8
CUnrealArchiveReader::ReadInt8( void )
{GUCEF_TRACE;

    return (Int8) ReadUInt8();
}

/*-------------------------------------------------------------------------*/

UInt16
CUnrealArchiveReader::ReadUInt16( void )
{GUCEF_TRACE;

    UInt8 b[ 2 ] = { 0, 0 };
    ReadRaw( b, 2 );
    return (UInt16) ( b[ 0 ] | ( b[ 1 ] << 8 ) );
}

/*-------------------------------------------------------------------------*/

Int16
CUnrealArchiveReader::ReadInt16( void )
{GUCEF_TRACE;

    return (Int16) ReadUInt16();
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealArchiveReader::ReadUInt32( void )
{GUCEF_TRACE;

    // the common case: all four bytes are in the window
    if ( !m_error && m_position + 4 <= m_end && m_position >= m_windowStart && m_position + 4 <= m_windowStart + m_windowLength )
    {
        const UInt8* p = &m_window[ (size_t) ( m_position - m_windowStart ) ];
        m_position += 4;
        return (UInt32) p[ 0 ] | ( (UInt32) p[ 1 ] << 8 ) | ( (UInt32) p[ 2 ] << 16 ) | ( (UInt32) p[ 3 ] << 24 );
    }
    UInt8 b[ 4 ] = { 0, 0, 0, 0 };
    ReadRaw( b, 4 );
    return (UInt32) b[ 0 ] | ( (UInt32) b[ 1 ] << 8 ) | ( (UInt32) b[ 2 ] << 16 ) | ( (UInt32) b[ 3 ] << 24 );
}

/*-------------------------------------------------------------------------*/

Int32
CUnrealArchiveReader::ReadInt32( void )
{GUCEF_TRACE;

    return (Int32) ReadUInt32();
}

/*-------------------------------------------------------------------------*/

UInt64
CUnrealArchiveReader::ReadUInt64( void )
{GUCEF_TRACE;

    const UInt64 low = ReadUInt32();
    const UInt64 high = ReadUInt32();
    return low | ( high << 32 );
}

/*-------------------------------------------------------------------------*/

Int64
CUnrealArchiveReader::ReadInt64( void )
{GUCEF_TRACE;

    return (Int64) ReadUInt64();
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealArchiveReader::ReadUInt32BE( void )
{GUCEF_TRACE;

    UInt8 b[ 4 ] = { 0, 0, 0, 0 };
    ReadRaw( b, 4 );
    return (UInt32) b[ 3 ] | ( (UInt32) b[ 2 ] << 8 ) | ( (UInt32) b[ 1 ] << 16 ) | ( (UInt32) b[ 0 ] << 24 );
}

/*-------------------------------------------------------------------------*/

UInt64
CUnrealArchiveReader::ReadUInt64BE( void )
{GUCEF_TRACE;

    const UInt64 high = ReadUInt32BE();
    const UInt64 low = ReadUInt32BE();
    return low | ( high << 32 );
}

/*-------------------------------------------------------------------------*/

Float32
CUnrealArchiveReader::ReadFloat32( void )
{GUCEF_TRACE;

    const UInt32 bits = ReadUInt32();
    Float32 value = 0;
    memcpy( &value, &bits, sizeof( value ) );
    return value;
}

/*-------------------------------------------------------------------------*/

Float64
CUnrealArchiveReader::ReadFloat64( void )
{GUCEF_TRACE;

    const UInt64 bits = ReadUInt64();
    Float64 value = 0;
    memcpy( &value, &bits, sizeof( value ) );
    return value;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealArchiveReader::ReadBool32( void )
{GUCEF_TRACE;

    const UInt32 value = ReadUInt32();
    if ( value > 1 )
        return SetError( "Boolean of 32 bits holds the value " + CORE::UInt32ToString( value ) );
    return 1 == value;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealArchiveReader::ReadBool8( void )
{GUCEF_TRACE;

    const UInt8 value = ReadUInt8();
    if ( value > 1 )
        return SetError( "Boolean of 8 bits holds the value " + CORE::UInt32ToString( value ) );
    return 1 == value;
}

/*-------------------------------------------------------------------------*/

static void
AppendUtf8( GUCEF::vector< char >& out, UInt32 codePoint )
{
    if ( codePoint < 0x80 )
        out.push_back( (char) codePoint );
    else
    if ( codePoint < 0x800 )
    {
        out.push_back( (char) ( 0xC0 | ( codePoint >> 6 ) ) );
        out.push_back( (char) ( 0x80 | ( codePoint & 0x3F ) ) );
    }
    else
    if ( codePoint < 0x10000 )
    {
        out.push_back( (char) ( 0xE0 | ( codePoint >> 12 ) ) );
        out.push_back( (char) ( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) ) );
        out.push_back( (char) ( 0x80 | ( codePoint & 0x3F ) ) );
    }
    else
    {
        out.push_back( (char) ( 0xF0 | ( codePoint >> 18 ) ) );
        out.push_back( (char) ( 0x80 | ( ( codePoint >> 12 ) & 0x3F ) ) );
        out.push_back( (char) ( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) ) );
        out.push_back( (char) ( 0x80 | ( codePoint & 0x3F ) ) );
    }
}

/*-------------------------------------------------------------------------*/

bool
CUnrealArchiveReader::ReadString( CORE::CString& text, UInt32 maxCodeUnits )
{GUCEF_TRACE;

    text.Clear();
    const Int32 length = ReadInt32();
    if ( m_error || 0 == length )
        return !m_error;

    const bool wide = length < 0;
    const UInt64 units = wide ? (UInt64) ( -(Int64) length ) : (UInt64) length;
    if ( units > maxCodeUnits )
        return SetError( "String of " + CORE::UInt64ToString( units ) + " code units exceeds the limit" );
    if ( !CheckCount( units, wide ? 2 : 1, "string" ) )
        return false;

    GUCEF::vector< char > utf8;
    utf8.reserve( (size_t) units + 1 );
    if ( wide )
    {
        GUCEF::vector< UInt8 > raw( (size_t) units * 2 );
        if ( !Read( &raw[ 0 ], units * 2 ) )
            return false;
        for ( size_t i=0; i<(size_t) units; ++i )
        {
            UInt32 unit = (UInt32) raw[ i*2 ] | ( (UInt32) raw[ i*2+1 ] << 8 );
            if ( 0 == unit && i + 1 == (size_t) units )
                break;
            if ( unit >= 0xD800 && unit < 0xDC00 && i + 1 < (size_t) units )
            {
                const UInt32 low = (UInt32) raw[ (i+1)*2 ] | ( (UInt32) raw[ (i+1)*2+1 ] << 8 );
                if ( low >= 0xDC00 && low < 0xE000 )
                {
                    unit = 0x10000 + ( ( unit - 0xD800 ) << 10 ) + ( low - 0xDC00 );
                    ++i;
                }
            }
            AppendUtf8( utf8, unit );
        }
    }
    else
    {
        GUCEF::vector< UInt8 > raw( (size_t) units );
        if ( !Read( &raw[ 0 ], units ) )
            return false;
        for ( size_t i=0; i<(size_t) units; ++i )
        {
            if ( 0 == raw[ i ] && i + 1 == (size_t) units )
                break;
            AppendUtf8( utf8, raw[ i ] );
        }
    }
    if ( !utf8.empty() )
        text.Set( &utf8[ 0 ], (UInt32) utf8.size() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealArchiveReader::ReadGuid( CUnrealGuid& guid )
{GUCEF_TRACE;

    guid.a = ReadUInt32();
    guid.b = ReadUInt32();
    guid.c = ReadUInt32();
    guid.d = ReadUInt32();
    return !m_error;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealArchiveReader::CheckCount( UInt64 count, UInt64 minElementBytes, const char* what )
{GUCEF_TRACE;

    if ( m_error )
        return false;
    const UInt64 remaining = GetRemaining();
    if ( 0 != minElementBytes && count > remaining / minElementBytes )
        return SetError( CORE::CString( "Count of " ) + CORE::UInt64ToString( count ) + " " + what + " cannot fit in the " + CORE::UInt64ToString( remaining ) + " bytes that remain" );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
CUnrealArchiveReader::NarrowEnd( UInt64 newEnd )
{GUCEF_TRACE;

    const UInt64 previous = m_end;
    if ( newEnd < m_end )
        m_end = newEnd < m_position ? m_position : newEnd;
    return previous;
}

/*-------------------------------------------------------------------------*/

void
CUnrealArchiveReader::RestoreEnd( UInt64 previousEnd )
{GUCEF_TRACE;

    m_end = previousEnd;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
