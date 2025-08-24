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
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHttpServerSettings::CHttpServerSettings( const CHttpServerSettings& src )
    : CORE::CIConfigurable( src )
    , maxClientConnections( src.maxClientConnections )
    , keepAliveConnections( src.keepAliveConnections )
    , processRequestsAsync( src.processRequestsAsync )
    , allowWebSocketUpgrades( src.allowWebSocketUpgrades )
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
    , m_sha1Codec()
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
    , m_sha1Codec()
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
CHTTPServer::RegisterEventHandlers( void )
{GUCEF_TRACE;

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

    MT::CScopeMutex lock( m_lock );
    m_settings = settings;
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

    if ( m_settings.processRequestsAsync )
    {
        // Use a worker thread to actually process the request
        ASyncProcessReceivedData( connection, receivedData );
    }
    else
    {
        // Syncronously process the request in a blocking manner
        CORE::CDynamicBuffer responseBuffer;
        SyncProcessReceivedData( connection, receivedData, responseBuffer );

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

void
CHTTPServer::ExtractCommaSeparatedValues( const CString& stringToExtractFrom ,
                                          TStringVector& list                ) const
{GUCEF_TRACE;

    // Create a list and make sure we trim each element
    TStringVector elements = stringToExtractFrom.ParseElements( ',' );
    TStringVector::iterator i = elements.begin();
    while ( i != elements.end() )
    {
        list.push_back( (*i).RemoveChar( ' ' ) );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::StripItems( TStringVector& list ,
                         char stripDelim     ) const
{GUCEF_TRACE;

    TStringVector::iterator i = list.begin();
    while ( i != list.end() )
    {
        (*i) = (*i).SubstrToChar( stripDelim, true );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CHTTPServer::InitSha1CodecLink( void )
{GUCEF_TRACE;

    if ( m_sha1Codec.IsNULL() )
    {
        CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();

        // First obtain the registery specific to compression codecs
        CORE::CCodecRegistry::TCodecFamilyRegistryPtr codecFamilyRegistry;
        if ( codecRegistry.TryLookup( CORE::CoreCodecTypes::CryptographicHashCodec, codecFamilyRegistry, false ) && codecFamilyRegistry )
        {
            // Hook up sha1 codec if we have it which per the spec is needed for the web socket handshake
            CORE::CCodecRegistry::TICodecPtr codec;
            if ( !codecFamilyRegistry->TryLookup( "sha1", m_sha1Codec, false ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "HttpServer(" + CORE::ToString( this ) + "):OnWebSocketUpgrade: No SHA1 codec is available" );
            }
            else
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "HttpServer(" + CORE::ToString( this ) + "):OnWebSocketUpgrade: Initialized link to SHA1 codec" );
                return true;
            }
        }
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CHTTPServer::GenerateWebSocketResponseKey( const CORE::CString& clientKey )
{GUCEF_TRACE;

    if ( !m_sha1Codec.IsNULL() )
    {
        CORE::CString sanityCheckKey = clientKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        CORE::CDynamicBuffer inBuffer; inBuffer.LinkTo( sanityCheckKey );
        CORE::CDynamicBufferAccess inBufferAccess( inBuffer );

        CORE::CDynamicBuffer outBuffer( true );
        CORE::CDynamicBufferAccess outBufferAccess( outBuffer );

        if ( m_sha1Codec->Encode( inBufferAccess, outBufferAccess ) )
        {
            return CORE::Base64Encode( outBuffer.GetBufferPtr(), outBuffer.GetDataSize() );
        }
    }
    return CORE::CString::Empty;
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

    if ( "Upgrade" != (*connectionHeader).second )
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
    if ( !InitSha1CodecLink() )
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

    // We are accepting the web socket upgrade. Notify as such
    CHttpResponseData response;
    response.statusCode = 101; // Switching Protocols
    response.otherHeaders[ "Upgrade" ] = "websocket";
    response.otherHeaders[ "Connection" ] = "Upgrade";
    response.otherHeaders[ "Sec-WebSocket-Accept" ] = GenerateWebSocketResponseKey( (*sanityCheckKeyHeader).second );

    CORE::CDynamicBuffer responseBuffer;
    response.Serialize( responseBuffer );
    connection->Send( responseBuffer );

    TWebSocketClientConnectedEventData eData( &requestData );
    NotifyObservers( WebSocketClientConnectedEvent, &eData );

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

void
CHTTPServer::SyncProcessReceivedData( COMCORE::CTCPServerConnection* connection ,
                                      const CORE::CDynamicBuffer& inputBuffer   ,
                                      CORE::CDynamicBuffer& outputBuffer        )
{GUCEF_TRACE;

    try
    {
        CHttpRequestData requestData;
        if ( !ParseRequest( inputBuffer, requestData ) )
            return;

        if ( requestData.isWebSocketUpgrade )
        {
            OnWebSocketUpgrade( connection, requestData );
            return;
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

        // Parse all the subsequent HTTP header fields
        TStringVector::iterator i = headerFields.begin();
        while ( i != headerFields.end() )
        {
            // Parse the header name and header value out of the header field
            CString& headerField = (*i);
            CString headerName = headerField.SubstrToChar( ':', true );
            CString headerValue( headerField.C_String() + headerName.Length(), headerField.Length() - headerName.Length() );

            // Remove additional spaces and lowercase the headername for easy comparison
            headerName = headerName.RemoveChar( ' ' ).Lowercase();
            headerValue = headerValue.RemoveChar( ' ' );

            //  Remove the ':' prefix
            headerValue = headerValue.CutChars( 1, true );

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
                request.isWebSocketUpgrade = "websocket" == headerValue;
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
