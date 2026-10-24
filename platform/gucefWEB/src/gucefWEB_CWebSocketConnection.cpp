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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETCONNECTION_H
#include "gucefWEB_CWebSocketConnection.h"
#define GUCEF_WEB_CWEBSOCKETCONNECTION_H
#endif /* GUCEF_WEB_CWEBSOCKETCONNECTION_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETSERVER_H
#include "gucefWEB_CWebSocketServer.h"
#define GUCEF_WEB_CWEBSOCKETSERVER_H
#endif /* GUCEF_WEB_CWEBSOCKETSERVER_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETFRAMECODEC_H
#include "gucefWEB_CWebSocketFrameCodec.h"
#define GUCEF_WEB_CWEBSOCKETFRAMECODEC_H
#endif /* GUCEF_WEB_CWEBSOCKETFRAMECODEC_H ? */

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWebSocketConnection::CWebSocketConnection( void )
    : CIWebSocketConnectionHandler()
    , CORE::CTSharedObjCreator< CWebSocketConnection, MT::CMutex >( this )
    , m_server( GUCEF_NULL )
    , m_tcpConnection( GUCEF_NULL )
    , m_peerInfo()
    , m_parser( true )
    , m_closeInfo()
    , m_closeSent( false )
    , m_userData( GUCEF_NULL )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebSocketConnection::~CWebSocketConnection()
{GUCEF_TRACE;

    Detach();
}

/*-------------------------------------------------------------------------*/

TWebSocketConnectionPtr
CWebSocketConnection::Create( CWebSocketServer& server                     ,
                              COMCORE::CTCPServerConnection& tcpConnection ,
                              const SWebSocketPeerInfo& peerInfo           ,
                              UInt32 maxMessageSize                        )
{GUCEF_TRACE;

    TWebSocketConnectionPtr connection = CreateSharedObj();
    connection->Initialize( server, tcpConnection, peerInfo, maxMessageSize );
    return connection;
}

/*-------------------------------------------------------------------------*/

void
CWebSocketConnection::Initialize( CWebSocketServer& server                     ,
                                  COMCORE::CTCPServerConnection& tcpConnection ,
                                  const SWebSocketPeerInfo& peerInfo           ,
                                  UInt32 maxMessageSize                        )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_server = &server;
        m_tcpConnection = &tcpConnection;
        m_peerInfo = peerInfo;
        m_peerInfo.connectionId = tcpConnection.GetConnectionIndex();
        m_parser = CWebSocketMessageParser( true, maxMessageSize );
        m_closeInfo = SWebSocketCloseInfo();
        m_closeInfo.connectionId = m_peerInfo.connectionId;
        m_closeSent = false;
    }

    // From now on the HTTP server hands the data and the disconnect of the TCP connection to us.
    // The cast makes sure the HTTP server gets back the same type it stores
    tcpConnection.SetUserData( static_cast< CIWebSocketConnectionHandler* >( this ) );
}

/*-------------------------------------------------------------------------*/

void
CWebSocketConnection::Detach( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( GUCEF_NULL != m_tcpConnection )
    {
        // never leave the connection pointing at an object which is going away
        if ( m_tcpConnection->GetUserData() == static_cast< CIWebSocketConnectionHandler* >( this ) )
            m_tcpConnection->SetUserData( GUCEF_NULL );
        m_tcpConnection = GUCEF_NULL;
    }
    m_server = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebSocketConnection::GetId( void ) const
{GUCEF_TRACE;

    return m_peerInfo.connectionId;
}

/*-------------------------------------------------------------------------*/

const SWebSocketPeerInfo&
CWebSocketConnection::GetPeerInfo( void ) const
{GUCEF_TRACE;

    return m_peerInfo;
}

/*-------------------------------------------------------------------------*/

void
CWebSocketConnection::SetUserData( void* userData )
{GUCEF_TRACE;

    m_userData = userData;
}

/*-------------------------------------------------------------------------*/

void*
CWebSocketConnection::GetUserData( void ) const
{GUCEF_TRACE;

    return m_userData;
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketConnection::IsOpen( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return GUCEF_NULL != m_tcpConnection && !m_closeSent;
}

/*-------------------------------------------------------------------------*/

void
CWebSocketConnection::SetMaxMessageSize( UInt32 maxMessageSizeInBytes )
{GUCEF_TRACE;

    // the parser belongs to the receiving thread, see the header
    m_parser.SetMaxMessageSize( maxMessageSizeInBytes );
}

/*-------------------------------------------------------------------------*/

UInt32
CWebSocketConnection::GetMaxMessageSize( void ) const
{GUCEF_TRACE;

    return m_parser.GetMaxMessageSize();
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketConnection::SendFrame( UInt8 opcode, const void* data, UInt32 dataSize )
{GUCEF_TRACE;

    CORE::CDynamicBuffer frame;
    if ( !CWebSocketFrameCodec::EncodeFrame( true, opcode, data, dataSize, false, frame ) )
        return false;

    // The lock keeps frames from different threads from interleaving and the connection from ending mid send
    MT::CScopeMutex lock( m_lock );
    if ( GUCEF_NULL == m_tcpConnection || m_closeSent )
        return false;
    return m_tcpConnection->Send( frame.GetConstBufferPtr(), frame.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketConnection::SendText( const CORE::CString& text )
{GUCEF_TRACE;

    return SendFrame( CWebSocketFrameCodec::OPCODE_TEXT, text.C_String(), GetWebSocketPayloadSize( text ) );
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketConnection::SendBinary( const void* data, UInt32 dataSize )
{GUCEF_TRACE;

    return SendFrame( CWebSocketFrameCodec::OPCODE_BINARY, data, dataSize );
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketConnection::SendPing( const CORE::CString& payload )
{GUCEF_TRACE;

    return SendFrame( CWebSocketFrameCodec::OPCODE_PING, payload.C_String(), GetWebSocketPayloadSize( payload ) );
}

/*-------------------------------------------------------------------------*/

void
CWebSocketConnection::SendCloseAndDrop( UInt16 sendCode, const SWebSocketCloseInfo& reportedCloseInfo )
{GUCEF_TRACE;

    CORE::CDynamicBuffer frame;
    CWebSocketFrameCodec::EncodeCloseFrame( sendCode, reportedCloseInfo.closedByPeer ? CORE::CString() : reportedCloseInfo.closeReason, false, frame );

    COMCORE::CTCPServerConnection* tcpConnection = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        if ( GUCEF_NULL == m_tcpConnection || m_closeSent )
            return;

        m_closeSent = true;
        m_closeInfo = reportedCloseInfo;
        m_closeInfo.connectionId = m_peerInfo.connectionId;
        tcpConnection = m_tcpConnection;
        tcpConnection->Send( frame.GetConstBufferPtr(), frame.GetDataSize() );
    }

    // We do not wait for the peer to answer our close frame: the connection is dropped right away.
    // This must not be done with our lock held as closing reports the disconnect to us right away
    tcpConnection->Close();
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketConnection::Close( UInt16 closeCode, const CORE::CString& reason )
{GUCEF_TRACE;

    if ( !IsOpen() )
        return false;

    SWebSocketCloseInfo closeInfo;
    closeInfo.closeCode = closeCode;
    closeInfo.closeReason = reason;
    closeInfo.closedByPeer = false;
    SendCloseAndDrop( closeCode, closeInfo );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebSocketConnection::OnWebSocketData( const void* data, UInt32 dataSize )
{GUCEF_TRACE;

    // The parser is only ever used from the receiving thread
    CWebSocketMessageParser::TMessageVector messages;
    CWebSocketMessageParser::EParseResult parseResult = m_parser.Feed( data, dataSize, messages );

    TWebSocketConnectionPtr self = CreateSharedPtr();
    CWebSocketServer* server = m_server;

    for ( size_t n=0; n<messages.size(); ++n )
    {
        CWebSocketMessageParser::SMessage& message = messages[ n ];
        switch ( message.type )
        {
            case CWebSocketMessageParser::MESSAGE_TEXT:
            {
                // false means the server was deleted as a result of the notification: do not touch anything anymore
                if ( GUCEF_NULL != server && !server->OnConnectionText( self, CORE::CString( static_cast< const char* >( message.payload.GetConstBufferPtr() ), message.payload.GetDataSize() ) ) )
                    return;
                break;
            }
            case CWebSocketMessageParser::MESSAGE_BINARY:
            {
                if ( GUCEF_NULL != server && !server->OnConnectionBinary( self, message.payload ) )
                    return;
                break;
            }
            case CWebSocketMessageParser::MESSAGE_PING:
            {
                SendFrame( CWebSocketFrameCodec::OPCODE_PONG, message.payload.GetConstBufferPtr(), message.payload.GetDataSize() );
                break;
            }
            case CWebSocketMessageParser::MESSAGE_PONG:
            {
                break;
            }
            case CWebSocketMessageParser::MESSAGE_CLOSE:
            {
                // answer the close with the same code, the closing handshake is then complete
                // while the application gets to see what the peer told us
                SWebSocketCloseInfo closeInfo;
                closeInfo.closeCode = message.closeCode;
                closeInfo.closeReason = message.closeReason;
                closeInfo.closedByPeer = true;
                SendCloseAndDrop( 1005 == message.closeCode ? (UInt16) WEBSOCKET_CLOSE_NORMAL : message.closeCode, closeInfo );
                return;
            }
        }
    }

    if ( CWebSocketMessageParser::PARSE_OK != parseResult )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "WebSocketConnection(" + CORE::PointerToString( this ) + "): Protocol violation by client " + CORE::UInt32ToString( m_peerInfo.connectionId ) + ", closing connection" );

        SWebSocketCloseInfo closeInfo;
        closeInfo.closeCode = m_parser.GetErrorCloseCode();
        closeInfo.closedByPeer = false;
        SendCloseAndDrop( closeInfo.closeCode, closeInfo );
    }
}

/*-------------------------------------------------------------------------*/

void
CWebSocketConnection::OnWebSocketClosed( void )
{GUCEF_TRACE;

    CWebSocketServer* server = GUCEF_NULL;
    SWebSocketCloseInfo closeInfo;
    {
        MT::CScopeMutex lock( m_lock );
        if ( GUCEF_NULL == m_tcpConnection )
            return;     // already reported

        // The connection object is re-used for other clients after this: we are done with it
        m_tcpConnection = GUCEF_NULL;
        server = m_server;
        closeInfo = m_closeInfo;
    }

    if ( GUCEF_NULL != server )
        server->OnConnectionClosed( CreateSharedPtr(), closeInfo );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
