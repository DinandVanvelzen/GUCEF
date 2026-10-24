/*
 *  gucefWEB: GUCEF module providing WEB application functionality
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CWEBSOCKETMESSAGEPARSER_H
#include "gucefWEB_CWebSocketMessageParser.h"
#define GUCEF_WEB_CWEBSOCKETMESSAGEPARSER_H
#endif /* GUCEF_WEB_CWEBSOCKETMESSAGEPARSER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWebSocketMessageParser::CWebSocketMessageParser( bool isServerSide, UInt32 maxMessageSize )
    : m_isServerSide( isServerSide )
    , m_maxMessageSize( maxMessageSize )
    , m_input()
    , m_fragments()
    , m_inFragmentedMessage( false )
    , m_fragmentedType( CWebSocketFrameCodec::OPCODE_TEXT )
    , m_failure( PARSE_OK )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWebSocketMessageParser::Reset( void )
{GUCEF_TRACE;

    m_input.clear();
    m_fragments.clear();
    m_inFragmentedMessage = false;
    m_failure = PARSE_OK;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebSocketMessageParser::GetMaxMessageSize( void ) const
{GUCEF_TRACE;

    return m_maxMessageSize;
}

/*-------------------------------------------------------------------------*/

void
CWebSocketMessageParser::SetMaxMessageSize( UInt32 maxMessageSize )
{GUCEF_TRACE;

    m_maxMessageSize = maxMessageSize;
}

/*-------------------------------------------------------------------------*/

UInt16
CWebSocketMessageParser::GetErrorCloseCode( void ) const
{GUCEF_TRACE;

    switch ( m_failure )
    {
        case PARSE_INVALID_PAYLOAD: return 1007;
        case PARSE_MESSAGE_TOO_BIG: return 1009;
        case PARSE_PROTOCOL_ERROR:  return 1002;
        default:                    return 1000;
    }
}

/*-------------------------------------------------------------------------*/

CWebSocketMessageParser::EParseResult
CWebSocketMessageParser::SetError( EParseResult result )
{GUCEF_TRACE;

    m_failure = result;
    m_input.clear();
    m_fragments.clear();
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketMessageParser::IsValidUtf8( const void* data, UInt32 size )
{GUCEF_TRACE;

    const UInt8* bytes = static_cast< const UInt8* >( data );
    UInt32 i = 0;
    while ( i < size )
    {
        UInt8 b = bytes[ i ];
        UInt32 extra = 0;
        UInt32 codePoint = 0;
        UInt32 minValue = 0;
        if ( b < 0x80 )
        {
            ++i;
            continue;
        }
        else
        if ( ( b & 0xE0 ) == 0xC0 ) { extra = 1; codePoint = b & 0x1F; minValue = 0x80; }
        else
        if ( ( b & 0xF0 ) == 0xE0 ) { extra = 2; codePoint = b & 0x0F; minValue = 0x800; }
        else
        if ( ( b & 0xF8 ) == 0xF0 ) { extra = 3; codePoint = b & 0x07; minValue = 0x10000; }
        else
            return false;

        if ( i + extra >= size )
            return false;
        for ( UInt32 n=1; n<=extra; ++n )
        {
            if ( ( bytes[ i + n ] & 0xC0 ) != 0x80 )
                return false;
            codePoint = ( codePoint << 6 ) | ( bytes[ i + n ] & 0x3F );
        }
        if ( codePoint < minValue || codePoint > 0x10FFFF || ( codePoint >= 0xD800 && codePoint <= 0xDFFF ) )
            return false;
        i += extra + 1;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketMessageParser::IsValidCloseCode( UInt16 code )
{GUCEF_TRACE;

    if ( code >= 3000 && code <= 4999 )
        return true;
    return ( code >= 1000 && code <= 1003 ) || ( code >= 1007 && code <= 1011 );
}

/*-------------------------------------------------------------------------*/

CWebSocketMessageParser::EParseResult
CWebSocketMessageParser::ProcessFrame( CWebSocketFrameCodec::SFrame& frame, TMessageVector& messages )
{GUCEF_TRACE;

    // A server only accepts masked frames, a client only unmasked ones
    if ( frame.wasMasked != m_isServerSide )
        return SetError( PARSE_PROTOCOL_ERROR );

    UInt32 payloadSize = frame.payload.GetDataSize();
    const char* payload = static_cast< const char* >( frame.payload.GetConstBufferPtr() );

    if ( ( frame.opcode & 0x8 ) != 0 )
    {
        // Control frames may be interleaved between the fragments of a message
        SMessage message;
        switch ( frame.opcode )
        {
            case CWebSocketFrameCodec::OPCODE_PING:
            {
                message.type = MESSAGE_PING;
                message.payload.CopyFrom( payloadSize, payload );
                break;
            }
            case CWebSocketFrameCodec::OPCODE_PONG:
            {
                message.type = MESSAGE_PONG;
                message.payload.CopyFrom( payloadSize, payload );
                break;
            }
            default:    // close
            {
                message.type = MESSAGE_CLOSE;
                message.payload.CopyFrom( payloadSize, payload );
                if ( 1 == payloadSize )
                    return SetError( PARSE_PROTOCOL_ERROR );
                if ( payloadSize >= 2 )
                {
                    message.closeCode = (UInt16) ( ( (UInt8) payload[ 0 ] << 8 ) | (UInt8) payload[ 1 ] );
                    if ( !IsValidCloseCode( message.closeCode ) )
                        return SetError( PARSE_PROTOCOL_ERROR );
                    if ( !IsValidUtf8( payload + 2, payloadSize - 2 ) )
                        return SetError( PARSE_INVALID_PAYLOAD );
                    message.closeReason = CORE::CString( payload + 2, payloadSize - 2 );
                }
                break;
            }
        }
        messages.push_back( message );
        return PARSE_OK;
    }

    // Data frame
    if ( CWebSocketFrameCodec::OPCODE_CONTINUATION == frame.opcode )
    {
        if ( !m_inFragmentedMessage )
            return SetError( PARSE_PROTOCOL_ERROR );
    }
    else
    {
        // a new message may not start while another one is still being assembled
        if ( m_inFragmentedMessage )
            return SetError( PARSE_PROTOCOL_ERROR );
        m_fragmentedType = (CWebSocketFrameCodec::EOpcode) frame.opcode;
    }

    if ( (UInt64) m_fragments.size() + payloadSize > m_maxMessageSize )
        return SetError( PARSE_MESSAGE_TOO_BIG );
    m_fragments.append( payload, payloadSize );

    if ( !frame.fin )
    {
        m_inFragmentedMessage = true;
        return PARSE_OK;
    }

    // Message complete
    m_inFragmentedMessage = false;
    SMessage message;
    message.type = CWebSocketFrameCodec::OPCODE_TEXT == m_fragmentedType ? MESSAGE_TEXT : MESSAGE_BINARY;
    if ( MESSAGE_TEXT == message.type && !IsValidUtf8( m_fragments.data(), (UInt32) m_fragments.size() ) )
        return SetError( PARSE_INVALID_PAYLOAD );
    message.payload.CopyFrom( (UInt32) m_fragments.size(), m_fragments.data() );
    m_fragments.clear();
    messages.push_back( message );
    return PARSE_OK;
}

/*-------------------------------------------------------------------------*/

CWebSocketMessageParser::EParseResult
CWebSocketMessageParser::Feed( const void* data, UInt32 size, TMessageVector& messages )
{GUCEF_TRACE;

    if ( PARSE_OK != m_failure )
        return m_failure;

    if ( size > 0 )
        m_input.append( static_cast< const char* >( data ), size );

    size_t consumedTotal = 0;
    while ( consumedTotal < m_input.size() )
    {
        CWebSocketFrameCodec::SFrame frame;
        UInt64 bytesConsumed = 0;
        CWebSocketFrameCodec::EDecodeResult decodeResult =
            CWebSocketFrameCodec::DecodeFrame( m_input.data() + consumedTotal, m_input.size() - consumedTotal, m_maxMessageSize, frame, bytesConsumed );

        if ( CWebSocketFrameCodec::DECODE_NEED_MORE_DATA == decodeResult )
            break;
        if ( CWebSocketFrameCodec::DECODE_PROTOCOL_ERROR == decodeResult )
            return SetError( PARSE_PROTOCOL_ERROR );
        if ( CWebSocketFrameCodec::DECODE_TOO_BIG == decodeResult )
            return SetError( PARSE_MESSAGE_TOO_BIG );

        consumedTotal += (size_t) bytesConsumed;
        EParseResult result = ProcessFrame( frame, messages );
        if ( PARSE_OK != result )
            return result;
    }

    if ( consumedTotal > 0 )
        m_input.erase( 0, consumedTotal );
    return PARSE_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
