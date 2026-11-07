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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOM_CMcpStreamChannel.h"

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

CMcpStreamChannel::CMcpStreamChannel( const CMcpPrincipal& principal, CIMcpLineSink* sink )
    : CMcpChannel( CHANNEL_STREAM, principal )
    , m_sink( sink )
    , m_partialLine()
    , m_skippingLine( false )
    , m_skippedBytes( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpStreamChannel::~CMcpStreamChannel()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpStreamChannel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CMcpStreamChannel";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CMcpStreamChannel::SetSink( CIMcpLineSink* sink )
{GUCEF_TRACE;

    m_sink = sink;
}

/*-------------------------------------------------------------------------*/

UInt32
CMcpStreamChannel::GetPendingBytes( void ) const
{GUCEF_TRACE;

    return m_partialLine.GetDataSize();
}

/*-------------------------------------------------------------------------*/

bool
CMcpStreamChannel::SendMessageText( const CORE::CString& messageText )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_sink )
        return false;

    // the framing depends on it: a message with a line end in it would be two messages to the peer
    const UInt32 size = ( messageText.ByteSize() > 0 ? messageText.ByteSize() - 1 : 0 );
    if ( GUCEF_NULL != ::memchr( messageText.C_String(), '\n', size ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpStreamChannel(" << GetChannelId() << "): A message with a line end in it was not sent" );
        return false;
    }
    return m_sink->SendMcpLine( messageText );
}

/*-------------------------------------------------------------------------*/

bool
CMcpStreamChannel::OnLineReceived( const CORE::CString& line )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpStreamChannel::ProcessIncomingLine( const CORE::CString& line )
{GUCEF_TRACE;

    if ( IsClosed() )
        return false;

    // a '\r' in front of the line end was part of the line end
    // sizes in bytes: CString::Length() counts characters in a UTF-8 build
    const UInt32 lineBytes = ( line.ByteSize() > 0 ? line.ByteSize() - 1 : 0 );
    UInt32 size = lineBytes;
    if ( size > 0 && '\r' == line.C_String()[ size-1 ] )
        --size;
    if ( 0 == size )
        return true;
    return ProcessIncomingMessage( size == lineBytes ? line : CORE::CString( line.C_String(), size ) );
}

/*-------------------------------------------------------------------------*/

void
CMcpStreamChannel::DeliverLine( const char* text, UInt32 size )
{GUCEF_TRACE;

    if ( size > 0 && '\r' == text[ size-1 ] )
        --size;
    if ( 0 == size )
        return;
    CORE::CString line( text, size );
    if ( !OnLineReceived( line ) )
        return;
    ProcessIncomingMessage( line );
}

/*-------------------------------------------------------------------------*/

void
CMcpStreamChannel::ProcessIncomingBytes( const void* data, UInt32 size )
{GUCEF_TRACE;

    const char* bytes = static_cast< const char* >( data );
    const UInt32 maxLineBytes = GetMaxMessageBytes();
    UInt32 position = 0;

    while ( position < size && !IsClosed() )
    {
        const char* lineEnd = static_cast< const char* >( ::memchr( bytes + position, '\n', size - position ) );

        if ( m_skippingLine )
        {
            // the rest of a line that was too long is dropped up to its end
            if ( GUCEF_NULL == lineEnd )
            {
                m_skippedBytes += size - position;
                return;
            }
            m_skippedBytes += (UInt64) ( lineEnd - ( bytes + position ) );
            position = (UInt32) ( lineEnd - bytes ) + 1;
            m_skippingLine = false;
            UInt64 skipped = m_skippedBytes;
            m_skippedBytes = 0;
            ProcessMessageTooLarge( skipped > GUCEF_UINT32MAX ? GUCEF_UINT32MAX : (UInt32) skipped );
            continue;
        }

        const UInt32 available = GUCEF_NULL == lineEnd ? size - position : (UInt32) ( lineEnd - ( bytes + position ) );
        const UInt64 lineSize = (UInt64) m_partialLine.GetDataSize() + available;

        if ( lineSize > (UInt64) maxLineBytes + 1 )         // + 1 for a '\r' that is part of the line end
        {
            m_skippedBytes = lineSize;
            m_partialLine.Clear( false );
            if ( GUCEF_NULL == lineEnd )
            {
                m_skippingLine = true;
                return;
            }
            position = (UInt32) ( lineEnd - bytes ) + 1;
            UInt64 skipped = m_skippedBytes;
            m_skippedBytes = 0;
            ProcessMessageTooLarge( skipped > GUCEF_UINT32MAX ? GUCEF_UINT32MAX : (UInt32) skipped );
            continue;
        }

        if ( GUCEF_NULL == lineEnd )
        {
            // the line goes on in the next bytes
            m_partialLine.Append( bytes + position, available );
            return;
        }

        if ( m_partialLine.GetDataSize() > 0 )
        {
            m_partialLine.Append( bytes + position, available );
            DeliverLine( static_cast< const char* >( m_partialLine.GetConstBufferPtr() ), m_partialLine.GetDataSize() );
            m_partialLine.Clear();
        }
        else
        {
            // the common case: the whole line is in these bytes
            DeliverLine( bytes + position, available );
        }
        position = (UInt32) ( lineEnd - bytes ) + 1;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
