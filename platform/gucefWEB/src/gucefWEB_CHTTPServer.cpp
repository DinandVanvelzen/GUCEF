/*
 *  gucefWEB: GUCEF module providing Web application functionality
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#include "gucefWEB_CWebSocketHandshake.h"
#define GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#endif /* GUCEF_WEB_CWEBSOCKETHANDSHAKE_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_WEB_CDEFAULTHTTPSERVERROUTERCONTROLLER_H
#include "gucefWEB_CDefaultHTTPServerRouterController.h"
#define GUCEF_WEB_CDEFAULTHTTPSERVERROUTERCONTROLLER_H
#endif /* GUCEF_WEB_CDEFAULTHTTPSERVERROUTERCONTROLLER_H ? */

#ifndef GUCEF_WEB_CDEFAULTHTTPSERVERREQUESTHANDLER_H
#include "gucefWEB_CDefaultHttpServerRequestHandler.h"
#define GUCEF_WEB_CDEFAULTHTTPSERVERREQUESTHANDLER_H
#endif /* GUCEF_WEB_CDEFAULTHTTPSERVERREQUESTHANDLER_H ? */

#ifndef GUCEF_WEB_CASYNCHTTPSERVERREQUESTHANDLER_H
#include "gucefWEB_CAsyncHttpServerRequestHandler.h"
#define GUCEF_WEB_CASYNCHTTPSERVERREQUESTHANDLER_H
#endif /* GUCEF_WEB_CASYNCHTTPSERVERREQUESTHANDLER_H ? */

#include "gucefWEB_CHTTPServer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

TDefaultHttpServerRequestHandlerFactory g_defaultHttpServerRequestHandlerFactory;

const CORE::CString CHttpServerSettings::ClassTypeName = "GUCEF::WEB::CHttpServerSettings";
const CORE::CEvent CHTTPServer::WebSocketClientConnectedEvent = "GUCEF::WEB::CHTTPServer::WebSocketClientConnectedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CHttpServerSettings::CHttpServerSettings( void )
    : CORE::CIConfigurable()
    , maxClientConnections( 100 )
    , keepAliveConnections( true )
    , processRequestsAsync( false )
    , allowWebSocketUpgrades( false )
    , bindAddress()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpServerSettings::CHttpServerSettings( const CHttpServerSettings& src )
    : CORE::CIConfigurable( src )
    , maxClientConnections( src.maxClientConnections )
    , keepAliveConnections( src.keepAliveConnections )
    , processRequestsAsync( src.processRequestsAsync )
    , allowWebSocketUpgrades( src.allowWebSocketUpgrades )
    , bindAddress( src.bindAddress )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpServerSettings&
CHttpServerSettings::operator=( const CHttpServerSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        maxClientConnections = src.maxClientConnections;
        keepAliveConnections = src.keepAliveConnections;
        processRequestsAsync = src.processRequestsAsync;
        allowWebSocketUpgrades = src.allowWebSocketUpgrades;
        bindAddress = src.bindAddress;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CHttpServerSettings::~CHttpServerSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CHttpServerSettings::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    totalSuccess = cfg.SetAttribute( "maxClientConnections", maxClientConnections ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "keepAliveConnections", keepAliveConnections ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "processRequestsAsync", processRequestsAsync ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "allowWebSocketUpgrades", allowWebSocketUpgrades ) && totalSuccess;
    if ( !bindAddress.IsNULLOrEmpty() )
        totalSuccess = cfg.SetAttribute( "bindAddress", bindAddress ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CHttpServerSettings::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    maxClientConnections = cfg.GetAttributeValueOrChildValueByName( "maxClientConnections" ).AsUInt32( maxClientConnections, true );
    keepAliveConnections = cfg.GetAttributeValueOrChildValueByName( "keepAliveConnections" ).AsBool( keepAliveConnections, true );
    processRequestsAsync = cfg.GetAttributeValueOrChildValueByName( "processRequestsAsync" ).AsBool( processRequestsAsync, true );
    allowWebSocketUpgrades = cfg.GetAttributeValueOrChildValueByName( "allowWebSocketUpgrades" ).AsBool( allowWebSocketUpgrades, true );
    bindAddress = cfg.GetAttributeValueOrChildValueByName( "bindAddress" ).AsString( bindAddress, true );

    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CHttpServerSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

CHTTPServer::CHTTPServer( THttpServerRequestHandlerFactory* requestHandlerFactory /* = GUCEF_NULL */ ,
                          const CHttpServerSettings* settings /* = GUCEF_NULL */                     )
    : CORE::CObservingNotifier()
    , m_tcpServerSocket( false, GUCEF_NULL != settings ? settings->maxClientConnections : 100 )
    , m_requestHandler()
    , m_requestHandlerFactory( requestHandlerFactory )
    , m_settings()
    , m_webSocketHandler( GUCEF_NULL )
    , m_lock()
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_requestHandlerFactory )
    {
        m_requestHandlerFactory = &g_defaultHttpServerRequestHandlerFactory;
    }
    m_requestHandler = m_requestHandlerFactory->Create();

    if ( GUCEF_NULL != settings )
    {
        m_settings = *settings;
        ApplyBindAddress();
    }

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

CHTTPServer::CHTTPServer( const CORE::PulseGeneratorPtr& pulsGenerator                               ,
                          THttpServerRequestHandlerFactory* requestHandlerFactory /* = GUCEF_NULL */ ,
                          const CHttpServerSettings* settings /* = GUCEF_NULL */                     )
    : CORE::CObservingNotifier()
    , m_tcpServerSocket( pulsGenerator, false, GUCEF_NULL != settings ? settings->maxClientConnections : 100 )
    , m_requestHandler()
    , m_requestHandlerFactory( requestHandlerFactory )
    , m_settings()
    , m_webSocketHandler( GUCEF_NULL )
    , m_lock()
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_requestHandlerFactory )
    {
        m_requestHandlerFactory = &g_defaultHttpServerRequestHandlerFactory;
    }
    m_requestHandler = m_requestHandlerFactory->Create();

    if ( GUCEF_NULL != settings )
    {
        m_settings = *settings;
        ApplyBindAddress();
    }

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

CHTTPServer::~CHTTPServer()
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    SignalUpcomingObserverDestruction();
    m_requestHandler.Unlink();
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::RegisterEvents( void )
{GUCEF_TRACE;

    WebSocketClientConnectedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::RegisterEventHandlers( void )
{GUCEF_TRACE;

    RegisterEvents();

    // Register event handlers
    TEventCallback callback( this, &CHTTPServer::OnClientConnectedEvent );
    SubscribeTo( &m_tcpServerSocket                              ,
                 COMCORE::CTCPServerSocket::ClientConnectedEvent ,
                 callback                                        );
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::SetSettings( const CHttpServerSettings& settings )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_settings = settings;
    }
    ApplyBindAddress();
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::ApplyBindAddress( void )
{GUCEF_TRACE;

    CORE::CString bindAddress;
    {
        MT::CScopeMutex lock( m_lock );
        bindAddress = m_settings.bindAddress;
    }

    // a socket that is active listens again on the new address, an address that is not understood stops the listening
    if ( !m_tcpServerSocket.SetBindAddress( bindAddress ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CHTTPServer: Unable to use the bind address \"" << bindAddress << "\"" );
    }
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::GetSettings( CHttpServerSettings& settings )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    settings = m_settings;
}

/*-------------------------------------------------------------------------*/

THttpServerRequestHandlerFactory::TProductPtr
CHTTPServer::GetRequestHandler( void ) const
{GUCEF_TRACE;

    return m_requestHandler;
}

/*-------------------------------------------------------------------------*/

bool
CHTTPServer::IsActive( void ) const
{GUCEF_TRACE;

    return m_tcpServerSocket.IsActive();
}

/*-------------------------------------------------------------------------*/

bool
CHTTPServer::ListenOnPort( const UInt16 port )
{GUCEF_TRACE;

    return m_tcpServerSocket.ListenOnPort( port );
}

/*-------------------------------------------------------------------------*/

bool
CHTTPServer::Listen( void )
{GUCEF_TRACE;

    return m_tcpServerSocket.Listen();
}

/*-------------------------------------------------------------------------*/

UInt16
CHTTPServer::GetBoundPort( void ) const
{GUCEF_TRACE;

    return m_tcpServerSocket.GetBoundPort();
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::Close( void )
{GUCEF_TRACE;

    m_tcpServerSocket.Close();
}

/*-------------------------------------------------------------------------*/

bool
CHTTPServer::SetPort( UInt16 port )
{GUCEF_TRACE;

    return m_tcpServerSocket.SetPort( port );
}

/*-------------------------------------------------------------------------*/

UInt16
CHTTPServer::GetPort( void ) const
{GUCEF_TRACE;

    return m_tcpServerSocket.GetPort();
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::OnClientConnectedEvent( CORE::CNotifier* notifier                 ,
                                     const CORE::CEvent& eventid               ,
                                     CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    const COMCORE::CTCPServerSocket::TClientConnectedEventData* eData = static_cast< COMCORE::CTCPServerSocket::TClientConnectedEventData* >( eventdata );
    const COMCORE::CTCPServerSocket::TConnectionInfo& storage = eData->GetData();

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" + CORE::PointerToString( this ) + "): Client connected" );

    // Subscribe to the connection
    TEventCallback callback( this, &CHTTPServer::OnClientDataReceivedEvent );
    SubscribeTo( storage.connection                               ,
                 COMCORE::CTCPServerConnection::DataRecievedEvent ,
                 callback                                         );

    TEventCallback disconnectCallback( this, &CHTTPServer::OnClientDisconnectedEvent );
    SubscribeTo( storage.connection                               ,
                 COMCORE::CTCPServerConnection::DisconnectedEvent ,
                 disconnectCallback                               );
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::OnClientDisconnectedEvent( CORE::CNotifier* notifier                 ,
                                        const CORE::CEvent& eventid               ,
                                        CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    // Tell the web socket handler of the connection, if it has one, that its connection ended
    COMCORE::CTCPServerConnection* connection = static_cast< COMCORE::CTCPServerConnection* >( notifier );
    CIWebSocketConnectionHandler* handler = static_cast< CIWebSocketConnectionHandler* >( connection->GetUserData() );
    if ( GUCEF_NULL != handler )
    {
        connection->SetUserData( GUCEF_NULL );
        handler->OnWebSocketClosed();
    }
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::OnClientDataReceivedEvent( CORE::CNotifier* notifier                 ,
                                        const CORE::CEvent& eventid               ,
                                        CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    const COMCORE::CTCPServerConnection::TDataRecievedEventData* eData = static_cast< COMCORE::CTCPServerConnection::TDataRecievedEventData* >( eventdata );
    const CORE::CDynamicBuffer& receivedData = eData->GetData();
    COMCORE::CTCPServerConnection* connection = static_cast< COMCORE::CTCPServerConnection* >( notifier );

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" + CORE::PointerToString( this ) + "): " + CORE::UInt32ToString( receivedData.GetDataSize() ) + " Bytes received from client " + connection->GetRemoteHostName() );

    // Connections which were upgraded to the web socket protocol no longer speak HTTP,
    // the handler they were handed to is linked on the connection itself
    CIWebSocketConnectionHandler* webSocketHandler = static_cast< CIWebSocketConnectionHandler* >( connection->GetUserData() );
    if ( GUCEF_NULL != webSocketHandler )
    {
        webSocketHandler->OnWebSocketData( receivedData.GetConstBufferPtr(), receivedData.GetDataSize() );
        return;
    }

    if ( m_settings.processRequestsAsync )
    {
        // Use a worker thread to actually process the request
        ASyncProcessReceivedData( connection, receivedData );
    }
    else
    {
        // Syncronously process the request in a blocking manner
        CORE::CDynamicBuffer responseBuffer;
        if ( !SyncProcessReceivedData( connection, receivedData, responseBuffer ) )
        {
            // A connection which was just upgraded to a web socket has already been answered and must stay open
            return;
        }

        // Send the reponse back to the client
        connection->Send( responseBuffer );
        if ( !m_settings.keepAliveConnections )
            connection->Close();
    }
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::OnNotify( CORE::CNotifier* notifier                 ,
                       const CORE::CEvent& eventid               ,
                       CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

/**
 *  Optional whitespace in a header field: spaces and horizontal tabs.
 *  The only whitespace that is not part of the value of a field is the whitespace around it.
 */
static bool
IsOptionalWhitespace( char c )
{GUCEF_TRACE;

    return ' ' == c || '\t' == c;
}

/*-------------------------------------------------------------------------*/

/**
 *  Drops the optional whitespace at the start and at the end of the text, whatever is inside is left as it is
 */
static CString
TrimOptionalWhitespace( const CString& text )
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() )
        return CString();

    const char* start = text.C_String();
    const char* end = start + ( text.ByteSize() - 1 );
    while ( start < end && IsOptionalWhitespace( *start ) )
        ++start;
    while ( end > start && IsOptionalWhitespace( *( end-1 ) ) )
        --end;
    return CString( start, (UInt32) ( end - start ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  Splits the value of a header field that holds a list on its commas. Commas inside quoted strings belong to the
 *  element they are in. The optional whitespace around each element is dropped and empty elements are left out.
 */
static void
SplitListValue( const CString& value        ,
                CString::StringVector& list )
{GUCEF_TRACE;

    if ( value.IsNULLOrEmpty() )
        return;

    const char* text = value.C_String();
    const UInt32 byteSize = value.ByteSize() - 1;
    bool inQuotes = false;
    UInt32 elementStart = 0;
    for ( UInt32 i=0; i<=byteSize; ++i )
    {
        // the end of the value ends the last element
        bool isSeparator = ( i == byteSize );
        if ( !isSeparator )
        {
            const char c = text[ i ];
            if ( inQuotes )
            {
                if ( '\\' == c && i+1 < byteSize )
                    ++i;    // an escaped character in a quoted string
                else
                if ( '"' == c )
                    inQuotes = false;
            }
            else
            if ( '"' == c )
                inQuotes = true;
            else
            if ( ',' == c )
                isSeparator = true;
        }

        if ( isSeparator )
        {
            CString element = TrimOptionalWhitespace( CString( text+elementStart, i-elementStart ) );
            if ( !element.IsNULLOrEmpty() )
                list.push_back( element );
            elementStart = i+1;
        }
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  Whether the value of a header field that holds a list has the token as one of its elements.
 *  Tokens are matched without regard to case.
 */
static bool
ListHasToken( const CString& value ,
              const CString& token )
{GUCEF_TRACE;

    CString::StringVector elements;
    SplitListValue( value, elements );
    for ( CString::StringVector::size_type i=0; i<elements.size(); ++i )
    {
        if ( elements[ i ].Equals( token, false ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/**
 *  Splits a header field in its name and its value. The value is what follows the first colon without the optional
 *  whitespace around it. Spaces and tabs inside the value are part of it.
 *  @return false if the line is not a header field because it has no colon
 */
static bool
SplitHeaderField( const CString& field ,
                  CString& name        ,
                  CString& value       )
{GUCEF_TRACE;

    if ( field.IsNULLOrEmpty() )
        return false;

    const char* text = field.C_String();
    const UInt32 byteSize = field.ByteSize() - 1;
    const char* colon = (const char*) memchr( text, ':', byteSize );
    if ( GUCEF_NULL == colon )
        return false;

    const UInt32 nameSize = (UInt32) ( colon - text );
    name = TrimOptionalWhitespace( CString( text, nameSize ) );
    value = TrimOptionalWhitespace( CString( colon+1, byteSize - nameSize - 1 ) );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::ExtractCommaSeparatedValues( const CString& stringToExtractFrom ,
                                          TStringVector& list                ) const
{GUCEF_TRACE;

    // Create a list and make sure we trim each element
    SplitListValue( stringToExtractFrom, list );
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::StripItems( TStringVector& list ,
                         char stripDelim     ) const
{GUCEF_TRACE;

    TStringVector::iterator i = list.begin();
    while ( i != list.end() )
    {
        (*i) = TrimOptionalWhitespace( (*i).SubstrToChar( stripDelim, true ) );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::SetWebSocketUpgradeHandler( CIWebSocketUpgradeHandler* handler )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_webSocketHandler = handler;
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::OnWebSocketUpgrade( COMCORE::CTCPServerConnection* connection ,
                                 const CHttpRequestData& requestData       )
{GUCEF_TRACE;

    if ( !m_settings.allowWebSocketUpgrades )
    {
        CHttpResponseData response;
        response.statusCode = 403; // Forbidden

        CORE::CDynamicBuffer responseBuffer;
        response.Serialize( responseBuffer );
        connection->Send( responseBuffer );
        connection->Close();

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "HttpServer(" + CORE::ToString( this ) + "):OnWebSocketUpgrade: Client attempted websocket upgrade. This feature is disabled" );
        return;
    }

    CHttpRequestData::TStringMap::const_iterator versionHeader = requestData.otherHeaders.find( "sec-websocket-version" );
    CHttpRequestData::TStringMap::const_iterator connectionHeader = requestData.otherHeaders.find( "connection" );
    CHttpRequestData::TStringMap::const_iterator sanityCheckKeyHeader = requestData.otherHeaders.find( "sec-websocket-key" );

    // In the case of a websocket upgrade request perform additional sanity check
    // We want a well formed request
    if ( versionHeader == requestData.otherHeaders.end()       ||
         connectionHeader == requestData.otherHeaders.end()    ||
         sanityCheckKeyHeader == requestData.otherHeaders.end() )
    {
        CHttpResponseData response;
        response.statusCode = 400; // Bad Request - missing headers

        CORE::CDynamicBuffer responseBuffer;
        response.Serialize( responseBuffer );
        connection->Send( responseBuffer );
        connection->Close();

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "HttpServer(" + CORE::ToString( this ) + "):OnWebSocketUpgrade: Client attempted websocket upgrade without all the mandatory headers" );
        return;
    }

    // The Connection field is a list, browsers send for example: keep-alive, Upgrade
    if ( !ListHasToken( (*connectionHeader).second, "Upgrade" ) )
    {
        CHttpResponseData response;
        response.statusCode = 400; // Bad Request - malformed, mandatory partner header

        CORE::CDynamicBuffer responseBuffer;
        response.Serialize( responseBuffer );
        connection->Send( responseBuffer );
        connection->Close();

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "HttpServer(" + CORE::ToString( this ) + "):OnWebSocketUpgrade: Client attempted websocket upgrade without the connection upgrade header" );
        return;
    }

    // We only support version 13
    if ( "13" != (*versionHeader).second )
    {
        CHttpResponseData response;
        response.statusCode = 426; // Upgrade Required
        response.location = requestData.requestUri;
        response.otherHeaders[ "Sec-WebSocket-Version" ] = "13";

        CORE::CDynamicBuffer responseBuffer;
        response.Serialize( responseBuffer );
        connection->Send( responseBuffer );
        connection->Close();

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "HttpServer(" + CORE::ToString( this ) + "):OnWebSocketUpgrade: Client attempting to connect with unsupported websocket protocol version: " + (*versionHeader).second );
        return;
    }

    // Since websockets are permitted per settings and SHA1 support is mandatory as such
    // we will consider it a bad server setup if we cannot get the needed codec
    CORE::CString acceptKey;
    if ( !CWebSocketHandshake::ComputeAcceptKey( (*sanityCheckKeyHeader).second, acceptKey ) )
    {
        CHttpResponseData response;
        response.statusCode = 500; // Internal Server Error

        CORE::CDynamicBuffer responseBuffer;
        response.Serialize( responseBuffer );
        connection->Send( responseBuffer );
        connection->Close();

        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpServer(" + CORE::ToString( this ) + "):OnWebSocketUpgrade: Cannot perform websocket upgrade: No SHA1 codec is available" );
        return;
    }

    // We are accepting the web socket upgrade
    CHttpResponseData response;
    response.statusCode = 101; // Switching Protocols
    response.otherHeaders[ "Upgrade" ] = "websocket";
    response.otherHeaders[ "Connection" ] = "Upgrade";
    response.otherHeaders[ "Sec-WebSocket-Accept" ] = acceptKey;

    CIWebSocketUpgradeHandler* handler = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        handler = m_webSocketHandler;
    }

    if ( GUCEF_NULL != handler && !handler->OnWebSocketUpgradeRequested( *this, *connection, requestData, response ) )
    {
        // The handler refused the upgrade, make sure that is clear to the client
        if ( response.statusCode < 400 )
            response.statusCode = 403;
        response.otherHeaders.erase( "Sec-WebSocket-Accept" );

        CORE::CDynamicBuffer responseBuffer;
        response.Serialize( responseBuffer );
        connection->Send( responseBuffer );
        connection->Close();

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "HttpServer(" + CORE::ToString( this ) + "):OnWebSocketUpgrade: The web socket upgrade handler refused the upgrade" );
        return;
    }

    CORE::CDynamicBuffer responseBuffer;
    response.Serialize( responseBuffer );
    connection->Send( responseBuffer );

    TWebSocketClientConnectedEventData eData( &requestData );
    if ( !NotifyObservers( WebSocketClientConnectedEvent, &eData ) )
        return;     // we were deleted as a result of the notification

    if ( GUCEF_NULL != handler )
    {
        // The handler takes the connection over: it links its CIWebSocketConnectionHandler to the connection
        // after which we no longer treat data from this connection as HTTP
        handler->OnWebSocketConnectionEstablished( *this, *connection, requestData );
    }

}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::ASyncProcessReceivedData( COMCORE::CTCPServerConnection* connection ,
                                       const CORE::CDynamicBuffer& inputBuffer   )
{GUCEF_TRACE;

    CAsyncHttpRequestData requestData;
    if ( !ParseRequest( inputBuffer, requestData ) )
        return;

    if ( requestData.isWebSocketUpgrade )
    {
        OnWebSocketUpgrade( connection, requestData );
        return;
    }

    requestData.clientConnectionId = connection->GetConnectionIndex();
    requestData.remoteClientAddress = connection->GetRemoteHostAddress().GetFirstIPv4Address();
    requestData.keepConnectionsAlive = m_settings.keepAliveConnections;
    requestData.httpServer = this;

    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    CORE::CFutureResult futureResult = taskManager.GetThreadPool()->QueueTask( CAsyncHttpServerRequestHandler::TaskType, &requestData, GUCEF_NULL, &AsObserver() ); 
    if ( futureResult.HasAFuture() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" + CORE::PointerToString( this ) + "): Queued task to ASync handle the request processing" );
    }
    else
    {
        // Since we are doing async processing the failure to queue a processing task would leave a
        // client in limbo up to their timeout. To prevent this we will handle this scenario here
        // directly

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" + CORE::PointerToString( this ) + "): Unable to ASync queue the request processing (QueueTask failed). As such will respond with 503" );

        CHttpResponseData returnData;
        returnData.statusCode = 503;   // Send 'Server too busy' because this is per configured capacity constraints

        CORE::CDynamicBuffer outputBuffer;
        returnData.Serialize( outputBuffer );

        connection->Send( outputBuffer );
        connection->Close();
    }
}

/*-------------------------------------------------------------------------*/

bool
CHTTPServer::SyncProcessReceivedData( COMCORE::CTCPServerConnection* connection ,
                                      const CORE::CDynamicBuffer& inputBuffer   ,
                                      CORE::CDynamicBuffer& outputBuffer        )
{GUCEF_TRACE;

    try
    {
        CHttpRequestData requestData;
        if ( !ParseRequest( inputBuffer, requestData ) )
            return true;

        if ( requestData.isWebSocketUpgrade )
        {
            OnWebSocketUpgrade( connection, requestData );

            // whether or not the upgrade was accepted, the answer was already sent and the connection must be left as is
            return false;
        }

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" + CORE::PointerToString( this ) + "): About to process request of type: " + requestData.requestType );

        CHttpResponseData response;
        m_requestHandler->OnRequest( requestData, response );
        response.Serialize( outputBuffer );

        connection->Send( outputBuffer );
        if ( !m_settings.keepAliveConnections )
            connection->Close();
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" + CORE::PointerToString( this ) + "): Exception sync processing received data: " + CORE::CString( e.what() ) );

        try
        {
            // Inform the client we ran into an internal server error so they are not just sitting there waiting for a response
            CHttpResponseData returnData;
            returnData.statusCode = 500;
            CORE::CDynamicBuffer outputBuffer;
            returnData.Serialize( outputBuffer );

            connection->Send( outputBuffer );

            if ( !m_settings.keepAliveConnections )
            {
                try
                {
                    connection->Close();
                }
                catch ( const std::exception& e )
                {
                    GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" + CORE::PointerToString( this ) + "): Exception closing connection: " + CORE::CString( e.what() ) );
                }
            }
        }
        catch ( const std::exception& e )
        {
            GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" + CORE::PointerToString( this ) + "): Exception sending internal server error response: " + CORE::CString( e.what() ) );
        }
    }

    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CHTTPServer::ParseHeaderFields( const char* bufferPtr       ,
                                const UInt32 bufferSize     ,
                                TStringVector& headerFields ) const
{GUCEF_TRACE;

    try
    {
        UInt32 startIndex = 0;
        UInt32 headerSize = 0;

        // According to the RFC lines are separated using '\r\n'
        for ( UInt32 i=0; i<bufferSize; ++i )
        {
            // Check for the end of line delimiter, cariage return first
            if ( bufferPtr[ i ] == '\r' )
            {
                if ( i+1 < bufferSize )
                {
                    // Check for line feed
                    if ( bufferPtr[ i+1 ] == '\n' )
                    {
                        ++i;
                        if ( i+1 < bufferSize )
                        {
                            // Add the segment to our list
                            headerFields.push_back( CString( bufferPtr+startIndex, i-1-startIndex ) );
                            startIndex = i+1;

                            // Check for empty line which is the end of header delimiter
                            // We start with cariage return
                            if ( bufferPtr[ i+1 ] == '\r' )
                            {
                                ++i;
                                if ( i+1 < bufferSize )
                                {
                                    // Check for line feed
                                    if ( bufferPtr[ i+1 ] == '\n' )
                                    {
                                        // Proper end of header delimiter found, we can stop
                                        headerSize = i+2;
                                        break;
                                    }

                                    // If we get here:
                                    // Some HTTP1.0 implementations add an extra '/r' after a '/r/n' so we have to be robust
                                    // and tolerate this condition
                                }
                                else
                                {
                                    // Not a well formatted HTTP header
                                    headerSize = i;
                                    break;
                                }
                            }
                        }
                        else
                        {
                            // Not a well formatted HTTP header
                            headerSize = i;
                            break;
                        }
                    }
                }
                else
                {
                    // Not a well formatted HTTP header
                    headerSize = i;
                    break;
                }
            }
        }
        return headerSize;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" + CORE::PointerToString( this ) + "): Exception parsing HTTP request header fields: " + CORE::CString( e.what() ) );
        return 0;
    }
}

/*-------------------------------------------------------------------------*/

bool
CHTTPServer::ParseRequest( const CORE::CDynamicBuffer& inputBuffer ,
                           CHttpRequestData& request               )
{GUCEF_TRACE;

    try
    {
        if ( inputBuffer.GetDataSize() == 0 )
        {
            // Invalid input
            return false;
        }

        // Parse all the HTTP Header fields out of the buffer
        TStringVector headerFields;
        UInt32 headerSize = ParseHeaderFields( static_cast< const char* >( inputBuffer.GetConstBufferPtr() ) ,
                                               inputBuffer.GetDataSize()                                     ,
                                               headerFields                                                  );

        // Sanity check on the parsed result
        if ( headerSize == 0 || headerFields.size() == 0 )
        {
            return false;
        }

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" + CORE::PointerToString( this ) + "): Finished parsing request header" );

        CString temp = headerFields.front().CompactRepeatingChar( ' ' );
        headerFields.erase( headerFields.begin() );

        // Parse the request type from the first line
        request.requestType = temp.SubstrToChar( ' ', true );
        temp = temp.CutChars( request.requestType.Length()+1, true );

        // Parse the request URI
        request.requestUri = temp.SubstrToChar( ' ', true );
        request.requestUriParams = request.requestUri.SubstrToChar( '?', false, true );
        temp = temp.CutChars( request.requestUri.Length()+1, true );
        if ( !request.requestUriParams.IsNULLOrEmpty() )
            request.requestUri = request.requestUri.CutChars( request.requestUriParams.Length()+1, false, 0 );

        // Parse the request protocol and its version
        request.requestProtocol = temp.SubstrToChar( '/', true );
        request.requestProtocolVersion = temp.CutChars( request.requestProtocol.Length()+1, true );

        // A line that starts with whitespace continues the header field before it (an obsolete line folding).
        // We replace the fold with a single space and treat the field as one line
        TStringVector unfoldedFields;
        for ( TStringVector::const_iterator f = headerFields.begin(); f != headerFields.end(); ++f )
        {
            if ( !unfoldedFields.empty() && !(*f).IsNULLOrEmpty() && IsOptionalWhitespace( (*f).C_String()[ 0 ] ) )
                unfoldedFields.back() = TrimOptionalWhitespace( unfoldedFields.back() ) + " " + TrimOptionalWhitespace( *f );
            else
                unfoldedFields.push_back( *f );
        }

        // Parse all the subsequent HTTP header fields
        TStringVector::iterator i = unfoldedFields.begin();
        while ( i != unfoldedFields.end() )
        {
            // Parse the header name and header value out of the header field
            // Only the whitespace around the name and the value is dropped, the name is lowercased for easy comparison
            CString headerName;
            CString headerValue;
            if ( !SplitHeaderField( (*i), headerName, headerValue ) )
            {
                // not a header field
                ++i;
                continue;
            }
            headerName = headerName.Lowercase();

            // Now that we have formatted the header name + value we can use them
            if ( headerName == "accept" )
            {
                ExtractCommaSeparatedValues( headerValue                     ,
                                             request.resourceRepresentations );
                StripItems( request.resourceRepresentations, ';' );
            }
            else
            if ( headerName == "accept-encoding" )
            {
                ExtractCommaSeparatedValues( headerValue                     ,
                                             request.encodingRepresentations );
                StripItems( request.encodingRepresentations, ';' );
            }
            else
            if ( headerName == "content-type" )
            {
                request.contentRepresentation = headerValue;
            }
            else
            if ( headerName == "if-match" )
            {
                ExtractCommaSeparatedValues( headerValue              ,
                                             request.resourceVersions );
            }
            else
            if ( headerName == "cookie" )
            {
                request.transactionID = headerValue;
            }
            else
            if ( headerName == "host" )
            {
                request.requestHost = headerValue;
            }
            else
            if ( headerName == "upgrade" )
            {
                request.isWebSocketUpgrade = ListHasToken( headerValue, "websocket" );
            }
            else
            {
                request.otherHeaders[ headerName ] = headerValue;
            }
            ++i;
        }

        // Set the content as a sub-segment of our data buffer
        if ( inputBuffer.GetDataSize() - headerSize > 0 )
        {
            request.content.LinkTo( inputBuffer.GetConstBufferPtr( headerSize ) ,
                                    inputBuffer.GetDataSize() - headerSize      );
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" + CORE::PointerToString( this ) + "): Exception parsing request: " + CORE::CString( e.what() ) );
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CHTTPServer::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CHTTPServer::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

CIHTTPServerRouterController*
CHTTPServer::GetRouterController( void ) const
{GUCEF_TRACE;

    if ( !m_requestHandler.IsNULL() )
    {
        return m_requestHandler->GetRouterController();
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

THttpServerRequestHandlerFactory*
CHTTPServer::GetRequestHandlerFactory( void ) const
{GUCEF_TRACE;

    return m_requestHandlerFactory;
}

/*-------------------------------------------------------------------------*/

bool
CHTTPServer::SendResponseASync( UInt32 connectionId                  ,
                                const CORE::CDynamicBuffer& response ,
                                const COMCORE::CIPv4Address& remoteIP  )
{GUCEF_TRACE;

    return m_tcpServerSocket.SendToConnection( connectionId                 ,
                                               response.GetConstBufferPtr() ,
                                               response.GetDataSize()       ,
                                               &remoteIP                    );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
