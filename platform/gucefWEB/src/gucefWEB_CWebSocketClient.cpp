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

#include <string.h>
#include <stdlib.h>

#ifndef GUCEF_WEB_CWEBSOCKETCLIENT_H
#include "gucefWEB_CWebSocketClient.h"
#define GUCEF_WEB_CWEBSOCKETCLIENT_H
#endif /* GUCEF_WEB_CWEBSOCKETCLIENT_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#include "gucefWEB_CWebSocketHandshake.h"
#define GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#endif /* GUCEF_WEB_CWEBSOCKETHANDSHAKE_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETFRAMECODEC_H
#include "gucefWEB_CWebSocketFrameCodec.h"
#define GUCEF_WEB_CWEBSOCKETFRAMECODEC_H
#endif /* GUCEF_WEB_CWEBSOCKETFRAMECODEC_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CWebSocketClient::ConnectedEvent = "GUCEF::WEB::CWebSocketClient::ConnectedEvent";
const CORE::CEvent CWebSocketClient::TextMessageEvent = "GUCEF::WEB::CWebSocketClient::TextMessageEvent";
const CORE::CEvent CWebSocketClient::BinaryMessageEvent = "GUCEF::WEB::CWebSocketClient::BinaryMessageEvent";
const CORE::CEvent CWebSocketClient::ClosedEvent = "GUCEF::WEB::CWebSocketClient::ClosedEvent";
const CORE::CEvent CWebSocketClient::ErrorEvent = "GUCEF::WEB::CWebSocketClient::ErrorEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static std::string
ToLowerCase( std::string str )
{
    for ( size_t i=0; i<str.size(); ++i )
        if ( str[ i ] >= 'A' && str[ i ] <= 'Z' )
            str[ i ] = (char) ( str[ i ] - 'A' + 'a' );
    return str;
}

static std::string
TrimSpaces( const std::string& str )
{
    size_t start = str.find_first_not_of( " \t" );
    if ( std::string::npos == start )
        return std::string();
    size_t end = str.find_last_not_of( " \t" );
    return str.substr( start, end - start + 1 );
}

/**
 *  Splits a ws:// url in host, port and the resource to request (path and query)
 */
static bool
ParseWebSocketUrl( const std::string& url, std::string& host, UInt16& port, std::string& resource )
{
    const std::string scheme = "ws://";
    if ( ToLowerCase( url.substr( 0, scheme.size() ) ) != scheme )
        return false;

    std::string rest = url.substr( scheme.size() );
    size_t resourceStart = rest.find_first_of( "/?" );
    std::string authority = rest.substr( 0, resourceStart );
    resource = std::string::npos == resourceStart ? "/" : rest.substr( resourceStart );
    if ( '/' != resource[ 0 ] )
        resource = "/" + resource;      // ws://host?query

    port = 80;
    size_t colon = authority.rfind( ':' );
    if ( std::string::npos != colon && authority.find( ']' ) == std::string::npos )
    {
        host = authority.substr( 0, colon );
        std::string portStr = authority.substr( colon + 1 );
        if ( portStr.empty() || portStr.find_first_not_of( "0123456789" ) != std::string::npos || portStr.size() > 5 )
            return false;
        int portValue = atoi( portStr.c_str() );
        if ( portValue < 1 || portValue > 65535 )
            return false;
        port = (UInt16) portValue;
    }
    else
    {
        host = authority;
    }
    return !host.empty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CWebSocketClient::RegisterEvents( void )
{GUCEF_TRACE;

    ConnectedEvent.Initialize();
    TextMessageEvent.Initialize();
    BinaryMessageEvent.Initialize();
    ClosedEvent.Initialize();
    ErrorEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CWebSocketClient::CWebSocketClient( void )
    : CORE::CObservingNotifier()
    , m_socket( false )
    , m_state( STATE_DISCONNECTED )
    , m_host()
    , m_port( 80 )
    , m_resource()
    , m_clientKey()
    , m_offeredSubProtocols()
    , m_subProtocol()
    , m_handshakeBuffer()
    , m_upgradeRequest()
    , m_parser( false )
    , m_maxMessageSize( 16 * 1024 * 1024 )
    , m_closeInfo()
    , m_closedReported( true )
    , m_suppressDisconnectReport( false )
    , m_peerInfo()
{GUCEF_TRACE;

    RegisterEvents();
    SubscribeTo( &m_socket );
}

/*-------------------------------------------------------------------------*/

CWebSocketClient::CWebSocketClient( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CObservingNotifier()
    , m_socket( pulseGenerator, false )
    , m_state( STATE_DISCONNECTED )
    , m_host()
    , m_port( 80 )
    , m_resource()
    , m_clientKey()
    , m_offeredSubProtocols()
    , m_subProtocol()
    , m_handshakeBuffer()
    , m_upgradeRequest()
    , m_parser( false )
    , m_maxMessageSize( 16 * 1024 * 1024 )
    , m_closeInfo()
    , m_closedReported( true )
    , m_suppressDisconnectReport( false )
    , m_peerInfo()
{GUCEF_TRACE;

    RegisterEvents();
    SubscribeTo( &m_socket );
}

/*-------------------------------------------------------------------------*/

CWebSocketClient::~CWebSocketClient()
{GUCEF_TRACE;

    // no events for an object that is going away
    m_closedReported = true;
    m_socket.Close();
}

/*-------------------------------------------------------------------------*/

CWebSocketClient::EState
CWebSocketClient::GetState( void ) const
{GUCEF_TRACE;

    return m_state;
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketClient::IsConnected( void ) const
{GUCEF_TRACE;

    return STATE_OPEN == m_state;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWebSocketClient::GetSubProtocol( void ) const
{GUCEF_TRACE;

    return m_subProtocol;
}

/*-------------------------------------------------------------------------*/

void
CWebSocketClient::SetMaxMessageSize( UInt32 maxMessageSizeInBytes )
{GUCEF_TRACE;

    m_maxMessageSize = maxMessageSizeInBytes;
    if ( STATE_DISCONNECTED == m_state )
        m_parser = CWebSocketMessageParser( false, m_maxMessageSize );
    else
        m_parser.SetMaxMessageSize( m_maxMessageSize );     // from now on, the connection stays as it is
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketClient::Connect( const CORE::CString& url         ,
                           const TStringVector& subProtocols ,
                           const TStringMap& extraHeaders    )
{GUCEF_TRACE;

    if ( STATE_DISCONNECTED != m_state )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WebSocketClient(" + CORE::PointerToString( this ) + "): Connect() called while already connected or connecting" );
        return false;
    }

    std::string host;
    std::string resource;
    UInt16 port = 80;
    if ( !ParseWebSocketUrl( url.STL_String(), host, port, resource ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WebSocketClient(" + CORE::PointerToString( this ) + "): Unsupported or invalid url \"" + url + "\", only ws:// is supported" );
        return false;
    }

    if ( !WEB::CWebSocketHandshake::GenerateClientKey( m_clientKey ) )
        return false;
    if ( !WEB::CWebSocketHandshake::IsAvailable() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WebSocketClient(" + CORE::PointerToString( this ) + "): The sha1-raw codec is not available, is codecspluginSTBRUMMEHASH loaded?" );
        return false;
    }

    m_host = host;
    m_port = port;
    m_resource = resource;
    m_offeredSubProtocols = subProtocols;
    m_subProtocol.Clear();
    m_handshakeBuffer.clear();
    m_parser = CWebSocketMessageParser( false, m_maxMessageSize );
    m_closeInfo = SWebSocketCloseInfo();
    m_peerInfo = SWebSocketPeerInfo();
    m_closedReported = false;

    std::string request = "GET " + resource + " HTTP/1.1\r\n";
    request += "Host: " + host + ( 80 == port ? std::string() : ":" + CORE::UInt32ToString( port ).STL_String() ) + "\r\n";
    request += "Upgrade: websocket\r\nConnection: Upgrade\r\n";
    request += "Sec-WebSocket-Key: " + m_clientKey.STL_String() + "\r\n";
    request += "Sec-WebSocket-Version: " + WEB::CWebSocketHandshake::ProtocolVersion.STL_String() + "\r\n";
    if ( !subProtocols.empty() )
    {
        std::string list;
        for ( size_t i=0; i<subProtocols.size(); ++i )
            list += ( 0 == i ? "" : ", " ) + subProtocols[ i ].STL_String();
        request += "Sec-WebSocket-Protocol: " + list + "\r\n";
    }
    for ( TStringMap::const_iterator i = extraHeaders.begin(); i != extraHeaders.end(); ++i )
        request += (*i).first.STL_String() + ": " + (*i).second.STL_String() + "\r\n";
    request += "\r\n";
    m_upgradeRequest = request;

    m_state = STATE_CONNECTING;
    if ( !m_socket.ConnectTo( CORE::CString( host ), port ) )
    {
        m_state = STATE_DISCONNECTED;
        m_closedReported = true;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WebSocketClient(" + CORE::PointerToString( this ) + "): Failed to start connecting to " + CORE::CString( host ) );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebSocketClient::OnNotify( CORE::CNotifier* notifier                 ,
                            const CORE::CEvent& eventid               ,
                            CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( notifier != &m_socket )
        return;

    if ( eventid == COMCORE::CTCPClientSocket::ConnectedEvent )
    {
        OnSocketConnected();
    }
    else
    if ( eventid == COMCORE::CTCPClientSocket::DisconnectedEvent )
    {
        OnSocketDisconnected();
    }
    else
    if ( eventid == COMCORE::CTCPClientSocket::SocketErrorEvent )
    {
        FailAndClose( "TCP socket error while connected to " + m_host );
    }
    else
    if ( eventid == COMCORE::CTCPClientSocket::DataRecievedEvent )
    {
        COMCORE::CTCPClientSocket::TDataRecievedEventData* eData = static_cast< COMCORE::CTCPClientSocket::TDataRecievedEventData* >( eventdata );
        OnSocketData( eData->GetData() );
    }
}

/*-------------------------------------------------------------------------*/

void
CWebSocketClient::OnSocketConnected( void )
{GUCEF_TRACE;

    if ( STATE_CONNECTING != m_state )
        return;

    m_state = STATE_HANDSHAKING;
    if ( !m_socket.Send( m_upgradeRequest.data(), (UInt32) m_upgradeRequest.size() ) )
        FailAndClose( "Failed to send the web socket upgrade request" );
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketClient::FailAndClose( const CORE::CString& description )
{GUCEF_TRACE;

    if ( STATE_DISCONNECTED == m_state && m_closedReported )
        return true;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "WebSocketClient(" + CORE::PointerToString( this ) + "): " + description );

    CORE::TCloneableString eData( description );
    if ( !NotifyObservers( ErrorEvent, &eData ) )
        return false;

    return CloseSocketAndReport();
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketClient::CloseSocketAndReport( void )
{GUCEF_TRACE;

    // The socket reports its own disconnect which we would turn into the close report: avoid doing that
    // from within the Close() call so the application only hears about it once and at a moment we are done
    m_suppressDisconnectReport = true;
    m_socket.Close();
    m_suppressDisconnectReport = false;
    return ReportClosed();
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketClient::ReportClosed( void )
{GUCEF_TRACE;

    m_state = STATE_DISCONNECTED;
    if ( m_closedReported )
        return true;
    m_closedReported = true;

    TWebSocketClosedEventData eData( m_closeInfo );
    return NotifyObservers( ClosedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

void
CWebSocketClient::OnSocketDisconnected( void )
{GUCEF_TRACE;

    if ( m_suppressDisconnectReport )
        return;

    if ( !m_closedReported && ( STATE_CONNECTING == m_state || STATE_HANDSHAKING == m_state ) )
    {
        CORE::TCloneableString eData( "The connection was lost before the web socket handshake completed" );
        if ( !NotifyObservers( ErrorEvent, &eData ) )
            return;
    }
    ReportClosed();
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketClient::ProcessHandshakeResponse( void )
{GUCEF_TRACE;

    size_t headEnd = m_handshakeBuffer.find( "\r\n\r\n" );
    if ( std::string::npos == headEnd )
    {
        if ( m_handshakeBuffer.size() > 16 * 1024 )
            return FailAndClose( "The server response to the upgrade request is too large" );
        return true;    // need more data
    }

    std::string head = m_handshakeBuffer.substr( 0, headEnd );
    std::string leftover = m_handshakeBuffer.substr( headEnd + 4 );
    m_handshakeBuffer.clear();

    // status line
    size_t lineEnd = head.find( "\r\n" );
    std::string statusLine = head.substr( 0, lineEnd );
    std::string headers = std::string::npos == lineEnd ? std::string() : head.substr( lineEnd + 2 );
    if ( statusLine.size() < 12 || 0 != statusLine.compare( 0, 9, "HTTP/1.1 " ) || 0 != statusLine.compare( 9, 3, "101" ) )
    {
        return FailAndClose( "The server did not accept the web socket upgrade: " + CORE::CString( statusLine ) );
    }

    std::map< std::string, std::string > fields;
    size_t pos = 0;
    while ( pos < headers.size() )
    {
        size_t end = headers.find( "\r\n", pos );
        std::string line = headers.substr( pos, std::string::npos == end ? std::string::npos : end - pos );
        pos = std::string::npos == end ? headers.size() : end + 2;
        size_t colon = line.find( ':' );
        if ( std::string::npos != colon )
            fields[ ToLowerCase( TrimSpaces( line.substr( 0, colon ) ) ) ] = TrimSpaces( line.substr( colon + 1 ) );
    }

    if ( ToLowerCase( fields[ "upgrade" ] ) != "websocket" || ToLowerCase( fields[ "connection" ] ).find( "upgrade" ) == std::string::npos )
    {
        return FailAndClose( "The upgrade response lacks the Upgrade: websocket and Connection: Upgrade headers" );
    }

    CORE::CString expectedAccept;
    if ( !WEB::CWebSocketHandshake::ComputeAcceptKey( m_clientKey, expectedAccept ) )
    {
        return FailAndClose( "Unable to validate the server handshake response" );
    }
    if ( fields[ "sec-websocket-accept" ] != expectedAccept.STL_String() )
    {
        return FailAndClose( "The Sec-WebSocket-Accept value of the server is not correct" );
    }

    // We never offer extensions so the server may not select any
    if ( fields.find( "sec-websocket-extensions" ) != fields.end() )
    {
        return FailAndClose( "The server selected a web socket extension which was not offered" );
    }

    std::map< std::string, std::string >::const_iterator selected = fields.find( "sec-websocket-protocol" );
    if ( selected != fields.end() )
    {
        bool wasOffered = false;
        for ( size_t i=0; i<m_offeredSubProtocols.size(); ++i )
            wasOffered = wasOffered || m_offeredSubProtocols[ i ].STL_String() == (*selected).second;
        if ( !wasOffered )
        {
            return FailAndClose( "The server selected a sub protocol which was not offered: " + CORE::CString( (*selected).second ) );
        }
        m_subProtocol = (*selected).second;
    }

    // Established
    m_state = STATE_OPEN;
    m_peerInfo.connectionId = 0;
    m_peerInfo.requestUri = m_resource;
    m_peerInfo.subProtocol = m_subProtocol;
    for ( std::map< std::string, std::string >::const_iterator i = fields.begin(); i != fields.end(); ++i )
        m_peerInfo.headers[ (*i).first ] = (*i).second;

    TWebSocketConnectedEventData eData( m_peerInfo );
    if ( !NotifyObservers( ConnectedEvent, &eData ) )
        return false;

    // the server may already have sent web socket data right behind its response
    if ( !leftover.empty() )
    {
        CORE::CDynamicBuffer extra;
        extra.CopyFrom( (UInt32) leftover.size(), leftover.data() );
        return OnSocketData( extra );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketClient::OnSocketData( const CORE::CDynamicBuffer& data )
{GUCEF_TRACE;

    if ( STATE_HANDSHAKING == m_state )
    {
        m_handshakeBuffer.append( static_cast< const char* >( data.GetConstBufferPtr() ), data.GetDataSize() );
        return ProcessHandshakeResponse();
    }
    if ( STATE_OPEN != m_state && STATE_CLOSING != m_state )
        return true;

    CWebSocketMessageParser::TMessageVector messages;
    CWebSocketMessageParser::EParseResult result = m_parser.Feed( data.GetConstBufferPtr(), data.GetDataSize(), messages );
    if ( !ProcessMessages( messages ) )
        return false;

    if ( CWebSocketMessageParser::PARSE_OK != result && STATE_DISCONNECTED != m_state )
    {
        // tell the server why we are leaving
        UInt16 errorCode = m_parser.GetErrorCloseCode();
        CORE::CDynamicBuffer frame;
        if ( CWebSocketFrameCodec::EncodeCloseFrame( errorCode, CORE::CString(), true, frame ) )
            m_socket.Send( frame.GetConstBufferPtr(), frame.GetDataSize() );
        m_closeInfo.closeCode = errorCode;
        m_closeInfo.closedByPeer = false;
        return FailAndClose( "Protocol violation by the server" );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketClient::ProcessMessages( const CWebSocketMessageParser::TMessageVector& messages )
{GUCEF_TRACE;

    for ( size_t n=0; n<messages.size(); ++n )
    {
        const CWebSocketMessageParser::SMessage& message = messages[ n ];
        switch ( message.type )
        {
            case CWebSocketMessageParser::MESSAGE_TEXT:
            {
                TWebSocketTextMessageEventData eData;
                eData.GetData().text = CORE::CString( static_cast< const char* >( message.payload.GetConstBufferPtr() ), message.payload.GetDataSize() );
                if ( !NotifyObservers( TextMessageEvent, &eData ) )
                    return false;
                break;
            }
            case CWebSocketMessageParser::MESSAGE_BINARY:
            {
                TWebSocketBinaryMessageEventData eData;
                eData.GetData().data = message.payload;
                if ( !NotifyObservers( BinaryMessageEvent, &eData ) )
                    return false;
                break;
            }
            case CWebSocketMessageParser::MESSAGE_PING:
            {
                SendFrame( true, CWebSocketFrameCodec::OPCODE_PONG, message.payload.GetConstBufferPtr(), message.payload.GetDataSize() );
                break;
            }
            case CWebSocketMessageParser::MESSAGE_PONG:
            {
                break;
            }
            case CWebSocketMessageParser::MESSAGE_CLOSE:
            {
                if ( STATE_OPEN == m_state )
                {
                    // the server started the closing handshake: answer it and we are done
                    m_closeInfo.closeCode = message.closeCode;
                    m_closeInfo.closeReason = message.closeReason;
                    m_closeInfo.closedByPeer = true;
                    CORE::CDynamicBuffer frame;
                    if ( CWebSocketFrameCodec::EncodeCloseFrame( 1005 == message.closeCode ? (UInt16) WEBSOCKET_CLOSE_NORMAL : message.closeCode, CORE::CString(), true, frame ) )
                        m_socket.Send( frame.GetConstBufferPtr(), frame.GetDataSize() );
                }
                // else we started it and this is the answer, the close info is what we sent
                return CloseSocketAndReport();
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketClient::SendFrame( bool fin, UInt8 opcode, const void* data, UInt32 dataSize )
{GUCEF_TRACE;

    // control frames such as the answer to a ping are also valid while we wait for the close answer
    if ( STATE_OPEN != m_state && !( STATE_CLOSING == m_state && ( opcode & 0x8 ) != 0 ) )
        return false;

    CORE::CDynamicBuffer frame;
    if ( !CWebSocketFrameCodec::EncodeFrame( fin, opcode, data, dataSize, true, frame ) )
        return false;
    return m_socket.Send( frame.GetConstBufferPtr(), frame.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketClient::SendText( const CORE::CString& text )
{GUCEF_TRACE;

    return SendFrame( true, CWebSocketFrameCodec::OPCODE_TEXT, text.C_String(), GetWebSocketPayloadSize( text ) );
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketClient::SendBinary( const void* data, UInt32 dataSize )
{GUCEF_TRACE;

    return SendFrame( true, CWebSocketFrameCodec::OPCODE_BINARY, data, dataSize );
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketClient::SendPing( const CORE::CString& payload )
{GUCEF_TRACE;

    return SendFrame( true, CWebSocketFrameCodec::OPCODE_PING, payload.C_String(), GetWebSocketPayloadSize( payload ) );
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketClient::Close( UInt16 closeCode, const CORE::CString& reason )
{GUCEF_TRACE;

    if ( STATE_OPEN != m_state )
        return false;

    CORE::CDynamicBuffer frame;
    if ( !CWebSocketFrameCodec::EncodeCloseFrame( closeCode, reason, true, frame ) )
        return false;

    m_closeInfo.closeCode = closeCode;
    m_closeInfo.closeReason = reason;
    m_closeInfo.closedByPeer = false;
    m_state = STATE_CLOSING;
    return m_socket.Send( frame.GetConstBufferPtr(), frame.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

void
CWebSocketClient::Disconnect( void )
{GUCEF_TRACE;

    CloseSocketAndReport();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
