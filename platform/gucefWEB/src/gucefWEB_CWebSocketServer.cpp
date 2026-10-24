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

#ifndef GUCEF_WEB_CWEBSOCKETSERVER_H
#include "gucefWEB_CWebSocketServer.h"
#define GUCEF_WEB_CWEBSOCKETSERVER_H
#endif /* GUCEF_WEB_CWEBSOCKETSERVER_H ? */

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

const CORE::CEvent CWebSocketServer::ClientConnectedEvent = "GUCEF::WEB::CWebSocketServer::ClientConnectedEvent";
const CORE::CEvent CWebSocketServer::ClientDisconnectedEvent = "GUCEF::WEB::CWebSocketServer::ClientDisconnectedEvent";
const CORE::CEvent CWebSocketServer::TextMessageEvent = "GUCEF::WEB::CWebSocketServer::TextMessageEvent";
const CORE::CEvent CWebSocketServer::BinaryMessageEvent = "GUCEF::WEB::CWebSocketServer::BinaryMessageEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Selects the first sub protocol offered by the client which we support, the client expresses its preference by order
 *  @return false if the client offered protocols but none we support
 */
static bool
SelectSubProtocol( const CHttpRequestData& request                          ,
                   const CWebSocketServer::TStringVector& supported         ,
                   CORE::CString& selected                                  )
{
    selected.Clear();
    if ( supported.empty() )
        return true;

    CHttpRequestData::TStringMap::const_iterator offered = request.otherHeaders.find( "sec-websocket-protocol" );
    if ( offered == request.otherHeaders.end() )
        return true;    // a client that offers none can be served without a sub protocol

    CORE::CString::StringVector offeredList = (*offered).second.ParseElements( ',', false );
    for ( size_t i=0; i<offeredList.size(); ++i )
    {
        CORE::CString candidate = offeredList[ i ].Trim( true ).Trim( false );
        for ( size_t n=0; n<supported.size(); ++n )
        {
            if ( candidate == supported[ n ] )
            {
                selected = candidate;
                return true;
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CWebSocketServer::RegisterEvents( void )
{GUCEF_TRACE;

    ClientConnectedEvent.Initialize();
    ClientDisconnectedEvent.Initialize();
    TextMessageEvent.Initialize();
    BinaryMessageEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CWebSocketServer::CWebSocketServer( void )
    : CORE::CNotifier()
    , CIWebSocketUpgradeHandler()
    , m_httpServer( GUCEF_NULL )
    , m_ownsHttpServer( true )
    , m_supportedSubProtocols()
    , m_maxMessageSize( 16 * 1024 * 1024 )
    , m_connections()
    , m_lock()
{GUCEF_TRACE;

    RegisterEvents();

    CHttpServerSettings settings;
    settings.allowWebSocketUpgrades = true;
    settings.processRequestsAsync = false;
    m_httpServer = GUCEF_NEW CHTTPServer( GUCEF_NULL, &settings );
    m_httpServer->SetWebSocketUpgradeHandler( this );
}

/*-------------------------------------------------------------------------*/

CWebSocketServer::CWebSocketServer( CHTTPServer& httpServer )
    : CORE::CNotifier()
    , CIWebSocketUpgradeHandler()
    , m_httpServer( &httpServer )
    , m_ownsHttpServer( false )
    , m_supportedSubProtocols()
    , m_maxMessageSize( 16 * 1024 * 1024 )
    , m_connections()
    , m_lock()
{GUCEF_TRACE;

    RegisterEvents();

    CHttpServerSettings settings;
    m_httpServer->GetSettings( settings );
    if ( !settings.allowWebSocketUpgrades )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "WebSocketServer(" + CORE::PointerToString( this ) + "): The HTTP server does not allow web socket upgrades, enabling it" );
        settings.allowWebSocketUpgrades = true;
        m_httpServer->SetSettings( settings );
    }
    m_httpServer->SetWebSocketUpgradeHandler( this );
}

/*-------------------------------------------------------------------------*/

CWebSocketServer::~CWebSocketServer()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_httpServer )
        m_httpServer->SetWebSocketUpgradeHandler( GUCEF_NULL );

    // Connection objects may outlive us in the hands of users: cut their link to us and to the TCP connections
    TConnectionVector connections;
    GetConnections( connections );
    {
        MT::CScopeMutex lock( m_lock );
        m_connections.clear();
    }
    for ( size_t i=0; i<connections.size(); ++i )
        connections[ i ]->Detach();
    connections.clear();

    if ( GUCEF_NULL != m_httpServer )
    {
        if ( m_ownsHttpServer )
        {
            GUCEF_DELETE m_httpServer;
        }
        m_httpServer = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketServer::Listen( UInt16 port )
{GUCEF_TRACE;

    if ( !m_ownsHttpServer )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WebSocketServer(" + CORE::PointerToString( this ) + "): Listen() is only for servers which own their HTTP server, use the attached HTTP server instead" );
        return false;
    }
    return m_httpServer->ListenOnPort( port );
}

/*-------------------------------------------------------------------------*/

void
CWebSocketServer::Close( void )
{GUCEF_TRACE;

    if ( m_ownsHttpServer )
        m_httpServer->Close();
}

/*-------------------------------------------------------------------------*/

UInt16
CWebSocketServer::GetPort( void ) const
{GUCEF_TRACE;

    return m_httpServer->GetPort();
}

/*-------------------------------------------------------------------------*/

CHTTPServer&
CWebSocketServer::GetHttpServer( void )
{GUCEF_TRACE;

    return *m_httpServer;
}

/*-------------------------------------------------------------------------*/

void
CWebSocketServer::SetSupportedSubProtocols( const TStringVector& subProtocols )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_supportedSubProtocols = subProtocols;
}

/*-------------------------------------------------------------------------*/

void
CWebSocketServer::SetMaxMessageSize( UInt32 maxMessageSizeInBytes )
{GUCEF_TRACE;

    // applies to connections made from now on
    MT::CScopeMutex lock( m_lock );
    m_maxMessageSize = maxMessageSizeInBytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebSocketServer::GetMaxMessageSize( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_maxMessageSize;
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketServer::OnWebSocketUpgradeRequested( CHTTPServer& server                       ,
                                               COMCORE::CTCPServerConnection& connection ,
                                               const CHttpRequestData& request           ,
                                               CHttpResponseData& response               )
{GUCEF_TRACE;

    TStringVector supported;
    {
        MT::CScopeMutex lock( m_lock );
        supported = m_supportedSubProtocols;
    }

    CORE::CString selected;
    if ( !SelectSubProtocol( request, supported, selected ) )
    {
        response.statusCode = 400;
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "WebSocketServer(" + CORE::PointerToString( this ) + "): Refusing client " + CORE::UInt32ToString( connection.GetConnectionIndex() ) + " because none of its sub protocols are supported" );
        return false;
    }
    if ( !selected.IsNULLOrEmpty() )
        response.otherHeaders[ "Sec-WebSocket-Protocol" ] = selected;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebSocketServer::OnWebSocketConnectionEstablished( CHTTPServer& server                       ,
                                                    COMCORE::CTCPServerConnection& connection ,
                                                    const CHttpRequestData& request           )
{GUCEF_TRACE;

    SWebSocketPeerInfo peerInfo;
    peerInfo.requestUri = request.requestUri;
    if ( !request.requestUriParams.IsNULLOrEmpty() )
        peerInfo.requestUri += "?" + request.requestUriParams;
    for ( CHttpRequestData::TStringMap::const_iterator i = request.otherHeaders.begin(); i != request.otherHeaders.end(); ++i )
        peerInfo.headers[ (*i).first ] = (*i).second;

    UInt32 maxMessageSize = 0;
    TStringVector supported;
    {
        MT::CScopeMutex lock( m_lock );
        maxMessageSize = m_maxMessageSize;
        supported = m_supportedSubProtocols;
    }
    SelectSubProtocol( request, supported, peerInfo.subProtocol );

    TWebSocketConnectionPtr wsConnection = CWebSocketConnection::Create( *this, connection, peerInfo, maxMessageSize );
    {
        MT::CScopeMutex lock( m_lock );
        m_connections.insert( wsConnection );
    }

    TWebSocketServerConnectedEventData eData( wsConnection );
    NotifyObservers( ClientConnectedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketServer::OnConnectionText( const TWebSocketConnectionPtr& connection, const CORE::CString& text )
{GUCEF_TRACE;

    TWebSocketServerTextMessageEventData eData;
    eData.GetData().connection = connection;
    eData.GetData().text = text;
    return NotifyObservers( TextMessageEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketServer::OnConnectionBinary( const TWebSocketConnectionPtr& connection, const CORE::CDynamicBuffer& data )
{GUCEF_TRACE;

    TWebSocketServerBinaryMessageEventData eData;
    eData.GetData().connection = connection;
    eData.GetData().data = data;
    return NotifyObservers( BinaryMessageEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketServer::OnConnectionClosed( const TWebSocketConnectionPtr& connection, const SWebSocketCloseInfo& closeInfo )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_connections.erase( connection );
    }

    TWebSocketServerClosedEventData eData;
    eData.GetData().connection = connection;
    eData.GetData().closeInfo = closeInfo;
    return NotifyObservers( ClientDisconnectedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

void
CWebSocketServer::GetConnections( TConnectionVector& connections ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    connections.assign( m_connections.begin(), m_connections.end() );
}

/*-------------------------------------------------------------------------*/

UInt32
CWebSocketServer::GetConnectionCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_connections.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CWebSocketServer::BroadcastText( const CORE::CString& text )
{GUCEF_TRACE;

    TConnectionVector connections;
    GetConnections( connections );
    UInt32 sentCount = 0;
    for ( size_t i=0; i<connections.size(); ++i )
        if ( connections[ i ]->SendText( text ) )
            ++sentCount;
    return sentCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebSocketServer::BroadcastBinary( const void* data, UInt32 dataSize )
{GUCEF_TRACE;

    TConnectionVector connections;
    GetConnections( connections );
    UInt32 sentCount = 0;
    for ( size_t i=0; i<connections.size(); ++i )
        if ( connections[ i ]->SendBinary( data, dataSize ) )
            ++sentCount;
    return sentCount;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
