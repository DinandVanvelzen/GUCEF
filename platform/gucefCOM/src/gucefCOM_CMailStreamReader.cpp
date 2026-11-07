/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CMIMECODEC_H
#include "gucefCOM_CMimeCodec.h"
#define GUCEF_COM_CMIMECODEC_H
#endif /* GUCEF_COM_CMIMECODEC_H ? */

#include "gucefCOM_CMailStreamReader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMailStreamReader::CMailStreamReader( void )
    : m_buffer()
    , m_start( 0 )
    , m_scan( 0 )
    , m_maxLineLength( GUCEF_COM_MAIL_DEFAULT_MAX_LINE_LENGTH )
    , m_maxBytes( GUCEF_COM_MAIL_DEFAULT_MAX_LITERAL_SIZE )
    , m_expected( 0 )
    , m_expecting( false )
    , m_expectingDot( false )
    , m_dotMax( 0 )
    , m_failed( false )
    , m_overflowReason()
{GUCEF_TRACE;

    m_buffer.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
}

/*-------------------------------------------------------------------------*/

CMailStreamReader::CMailStreamReader( UInt32 maxLineLength, UInt32 maxBytes )
    : m_buffer()
    , m_start( 0 )
    , m_scan( 0 )
    , m_maxLineLength( maxLineLength )
    , m_maxBytes( maxBytes )
    , m_expected( 0 )
    , m_expecting( false )
    , m_expectingDot( false )
    , m_dotMax( 0 )
    , m_failed( false )
    , m_overflowReason()
{GUCEF_TRACE;

    m_buffer.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
}

/*-------------------------------------------------------------------------*/

void
CMailStreamReader::SetLimits( UInt32 maxLineLength, UInt32 maxBytes )
{GUCEF_TRACE;

    m_maxLineLength = maxLineLength;
    m_maxBytes = maxBytes;
}

/*-------------------------------------------------------------------------*/

void
CMailStreamReader::Reset( void )
{GUCEF_TRACE;

    m_buffer.SetDataSize( 0 );
    m_start = 0;
    m_scan = 0;
    m_expected = 0;
    m_expecting = false;
    m_expectingDot = false;
    m_dotMax = 0;
    m_failed = false;
    m_overflowReason.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CMailStreamReader::Feed( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( m_failed )
        return false;
    if ( 0 == size || GUCEF_NULL == data )
        return true;
    m_buffer.Append( data, size, true );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMailStreamReader::Compact( void )
{GUCEF_TRACE;

    if ( m_start == m_buffer.GetDataSize() )
    {
        m_buffer.SetDataSize( 0 );
        m_scan = 0;
        m_start = 0;
    }
    else
    if ( m_start >= 65536 && m_start * 2 >= m_buffer.GetDataSize() )
    {
        m_buffer.Downshift( m_start );
        m_scan -= m_start;
        m_start = 0;
    }
}

/*-------------------------------------------------------------------------*/

CMailStreamReader::EItem
CMailStreamReader::Next( CORE::CString& line, CORE::CDynamicBuffer& bytes )
{GUCEF_TRACE;

    if ( m_failed )
        return ITEM_OVERFLOW;

    const UInt32 size = m_buffer.GetDataSize();
    const char* data = static_cast< const char* >( m_buffer.GetConstBufferPtr() );

    if ( m_expecting )
    {
        if ( size - m_start < m_expected )
            return ITEM_NONE;
        bytes.Clear();
        if ( m_expected > 0 )
            bytes.CopyFrom( m_expected, data + m_start );
        m_start += m_expected;
        if ( m_scan < m_start )
            m_scan = m_start;
        m_expecting = false;
        m_expected = 0;
        Compact();
        return ITEM_BYTES;
    }

    if ( m_expectingDot )
    {
        // the end is a line of one dot: at the start of the block, or behind a line feed
        UInt32 i = m_scan < m_start ? m_start : m_scan;
        for ( ; i < size; ++i )
        {
            const bool atLineStart = i == m_start || '\n' == data[ i - 1 ];
            if ( !atLineStart || '.' != data[ i ] )
                continue;
            if ( i + 1 >= size )
                break;                      // not enough bytes to tell yet
            UInt32 after = 0;
            if ( '\n' == data[ i + 1 ] )
                after = i + 2;
            else
            if ( '\r' == data[ i + 1 ] )
            {
                if ( i + 2 >= size )
                    break;
                if ( '\n' == data[ i + 2 ] )
                    after = i + 3;
            }
            if ( 0 != after )
            {
                bytes.Clear();
                if ( i > m_start )
                    bytes.CopyFrom( i - m_start, data + m_start );
                m_start = after;
                m_scan = after;
                m_expectingDot = false;
                Compact();
                return ITEM_BYTES;
            }
        }
        m_scan = i;
        if ( size - m_start > m_dotMax + 4 )
        {
            m_failed = true;
            m_overflowReason = "The server sent a message of more than " + CORE::UInt32ToString( m_dotMax ) + " bytes";
            return ITEM_OVERFLOW;
        }
        return ITEM_NONE;
    }

    if ( m_scan < m_start )
        m_scan = m_start;
    const char* end = data + size;
    const char* lineFeed = GUCEF_NULL;
    if ( m_scan < size )
        lineFeed = static_cast< const char* >( memchr( data + m_scan, '\n', size - m_scan ) );

    if ( GUCEF_NULL == lineFeed )
    {
        m_scan = size;
        // a line that has no end yet must not be longer than the maximum (a CR may be the start of its end)
        if ( size - m_start > m_maxLineLength + 1 )
        {
            m_failed = true;
            m_overflowReason = "The server sent a line longer than " + CORE::UInt32ToString( m_maxLineLength ) + " bytes without ending it";
            return ITEM_OVERFLOW;
        }
        return ITEM_NONE;
    }

    UInt32 lineEnd = (UInt32)( lineFeed - data );
    const UInt32 next = lineEnd + 1;
    if ( lineEnd > m_start && '\r' == data[ lineEnd - 1 ] )
        --lineEnd;
    if ( lineEnd - m_start > m_maxLineLength )
    {
        m_failed = true;
        m_overflowReason = "The server sent a line longer than " + CORE::UInt32ToString( m_maxLineLength ) + " bytes";
        return ITEM_OVERFLOW;
    }

    line = CMimeCodec::SanitizeUtf8( data + m_start, lineEnd - m_start );
    m_start = next;
    m_scan = next;
    (void) end;
    Compact();
    return ITEM_LINE;
}

/*-------------------------------------------------------------------------*/

bool
CMailStreamReader::ExpectBytes( UInt32 count )
{GUCEF_TRACE;

    if ( m_failed )
        return false;
    if ( count > m_maxBytes )
    {
        m_failed = true;
        m_overflowReason = "The server announced a block of " + CORE::UInt32ToString( count ) + " bytes, more than the limit of " + CORE::UInt32ToString( m_maxBytes );
        return false;
    }
    m_expected = count;
    m_expecting = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMailStreamReader::ExpectDotTerminatedBytes( UInt32 maxBytes )
{GUCEF_TRACE;

    m_expectingDot = true;
    m_dotMax = maxBytes;
    if ( m_scan > m_start )
        m_scan = m_start;
}

/*-------------------------------------------------------------------------*/

bool
CMailStreamReader::IsExpectingBytes( void ) const
{GUCEF_TRACE;

    return m_expecting || m_expectingDot;
}

/*-------------------------------------------------------------------------*/

UInt32
CMailStreamReader::GetBufferedSize( void ) const
{GUCEF_TRACE;

    return m_buffer.GetDataSize() - m_start;
}

/*-------------------------------------------------------------------------*/

bool
CMailStreamReader::HasBufferedData( void ) const
{GUCEF_TRACE;

    return m_buffer.GetDataSize() > m_start;
}

/*-------------------------------------------------------------------------*/

bool
CMailStreamReader::HasFailed( void ) const
{GUCEF_TRACE;

    return m_failed;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMailStreamReader::GetOverflowReason( void ) const
{GUCEF_TRACE;

    return m_overflowReason;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
