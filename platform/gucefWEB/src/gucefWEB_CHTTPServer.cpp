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

#include <string.h>

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

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#include "gucefCORE_CIMonotonicClock.h"
#define GUCEF_CORE_CIMONOTONICCLOCK_H
#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */

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

/** Exchanges are numbered for the whole process: an id is never used twice, also not by two servers */
static MT::CMutex g_exchangeIdLock;
static UInt64 g_nextExchangeId = 1;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the server knows of one connection while HTTP is spoken on it. One per connection index: the connection
 *  objects of the server socket are reused, and so are these. Reset when a client connects and when it goes.
 */
class CHttpServerConnectionState
{
    public:

    enum EPhase
    {
        PHASE_IDLE          ,   /**< between requests: nothing of the next one has arrived */
        PHASE_HEADERS       ,   /**< the header block of a request is coming in */
        PHASE_BODY          ,   /**< the header block is in, the body is coming in */
        PHASE_DISPATCHED    ,   /**< the request is with a handler; what comes in waits behind it */
        PHASE_CLOSING       ,   /**< the connection closes once what was sent is out; what comes in is dropped */
        PHASE_WEBSOCKET         /**< upgraded: the bytes go to the web socket handler of the connection */
    };

    COMCORE::CTCPServerConnection* connection;
    UInt32 generation;                  /**< changes at every reset: work that saw another generation stops */
    bool inUse;
    EPhase phase;
    CORE::CDynamicBuffer buffer;        /**< what arrived and was not taken yet: a request that is not complete, pipelined requests */
    UInt32 headerScanPos;               /**< how far the search for the end of the header block got, from the start of the request */
    UInt32 bodySize;
    bool keepAlive;                     /**< what the request asks for; the settings may still say no */
    UInt64 deadlineInMs;                /**< 0: none */
    bool processing;                    /**< inside ProcessConnectionData() */
    bool resumePending;                 /**< pipelined bytes wait to be processed on the next pulse */
    bool closeRequested;                /**< close on the next pulse, without waiting for what is queued */
    bool closeWhenFlushed;              /**< close once the send queue of the connection is empty */
    UInt64 closeDeadlineInMs;
    bool hadRequest;                    /**< a whole request came in on the connection: it is no longer a mere connect */
    UInt64 phaseSinceInMs;              /**< when the connection became idle or a request began to come in: who waits longest */
    CHttpRequestData request;
    CIHttpRequestTakeoverHandler* takeoverHandler;
    CORE::CString takeoverPath;
    CHttpServerExchangePtr exchange;

    CHttpServerConnectionState( void )
        : connection( GUCEF_NULL )
        , generation( 0 )
        , inUse( false )
        , phase( PHASE_IDLE )
        , buffer()
        , headerScanPos( 0 )
        , bodySize( 0 )
        , keepAlive( true )
        , deadlineInMs( 0 )
        , processing( false )
        , resumePending( false )
        , closeRequested( false )
        , closeWhenFlushed( false )
        , closeDeadlineInMs( 0 )
        , hadRequest( false )
        , phaseSinceInMs( 0 )
        , request()
        , takeoverHandler( GUCEF_NULL )
        , takeoverPath()
        , exchange()
    {GUCEF_TRACE;

    }

    /** Forgets the request in progress, keeps the connection */
    void ResetRequest( void )
    {GUCEF_TRACE;

        headerScanPos = 0;
        bodySize = 0;
        keepAlive = true;
        request = CHttpRequestData();
        takeoverHandler = GUCEF_NULL;
        takeoverPath.Clear();
    }
};

/*-------------------------------------------------------------------------*/

/** What the parsing of a header block found out, besides the request itself */
struct CHTTPServer::SRequestHeadInfo
{
    UInt32 fieldCount;
    UInt32 hostCount;
    UInt32 contentTypeCount;
    bool hasContentLength;
    bool contentLengthValid;
    UInt64 contentLength;
    bool hasTransferEncoding;
    bool hasExpect;
    CString expect;
    bool keepAlive;
    bool requestLineValid;

    SRequestHeadInfo( void )
        : fieldCount( 0 )
        , hostCount( 0 )
        , contentTypeCount( 0 )
        , hasContentLength( false )
        , contentLengthValid( true )
        , contentLength( 0 )
        , hasTransferEncoding( false )
        , hasExpect( false )
        , expect()
        , keepAlive( true )
        , requestLineValid( false )
    {}
};

/*-------------------------------------------------------------------------*/

/** The settings a read is processed with, taken under the lock once per read */
struct CHTTPServer::SLimits
{
    UInt32 maxClientConnections;
    UInt32 maxHeaderBytes;
    UInt32 maxHeaderCount;
    UInt32 maxBodyBytes;
    UInt32 headerTimeoutMs;
    UInt32 bodyTimeoutMs;
    UInt32 idleTimeoutMs;
    UInt32 maxStreamBacklogBytes;
    bool keepAliveConnections;
    bool processRequestsAsync;
};

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
    , maxHeaderBytes( DefaultMaxHeaderBytes )
    , maxHeaderCount( DefaultMaxHeaderCount )
    , maxBodyBytes( DefaultMaxBodyBytes )
    , headerTimeoutMs( DefaultHeaderTimeoutMs )
    , bodyTimeoutMs( DefaultBodyTimeoutMs )
    , idleTimeoutMs( DefaultIdleTimeoutMs )
    , maxStreamBacklogBytes( DefaultMaxStreamBacklogBytes )
    , tls()
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
    , maxHeaderBytes( src.maxHeaderBytes )
    , maxHeaderCount( src.maxHeaderCount )
    , maxBodyBytes( src.maxBodyBytes )
    , headerTimeoutMs( src.headerTimeoutMs )
    , bodyTimeoutMs( src.bodyTimeoutMs )
    , idleTimeoutMs( src.idleTimeoutMs )
    , maxStreamBacklogBytes( src.maxStreamBacklogBytes )
    , tls( src.tls )
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
        maxHeaderBytes = src.maxHeaderBytes;
        maxHeaderCount = src.maxHeaderCount;
        maxBodyBytes = src.maxBodyBytes;
        headerTimeoutMs = src.headerTimeoutMs;
        bodyTimeoutMs = src.bodyTimeoutMs;
        idleTimeoutMs = src.idleTimeoutMs;
        maxStreamBacklogBytes = src.maxStreamBacklogBytes;
        tls = src.tls;
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
    totalSuccess = cfg.SetAttribute( "maxHeaderBytes", maxHeaderBytes ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxHeaderCount", maxHeaderCount ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxBodyBytes", maxBodyBytes ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "headerTimeoutMs", headerTimeoutMs ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "bodyTimeoutMs", bodyTimeoutMs ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "idleTimeoutMs", idleTimeoutMs ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxStreamBacklogBytes", maxStreamBacklogBytes ) && totalSuccess;
    if ( tls.IsEnabled() )
        totalSuccess = tls.SaveConfig( cfg ) && totalSuccess;
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
    maxHeaderBytes = cfg.GetAttributeValueOrChildValueByName( "maxHeaderBytes" ).AsUInt32( maxHeaderBytes, true );
    maxHeaderCount = cfg.GetAttributeValueOrChildValueByName( "maxHeaderCount" ).AsUInt32( maxHeaderCount, true );
    maxBodyBytes = cfg.GetAttributeValueOrChildValueByName( "maxBodyBytes" ).AsUInt32( maxBodyBytes, true );
    headerTimeoutMs = cfg.GetAttributeValueOrChildValueByName( "headerTimeoutMs" ).AsUInt32( headerTimeoutMs, true );
    bodyTimeoutMs = cfg.GetAttributeValueOrChildValueByName( "bodyTimeoutMs" ).AsUInt32( bodyTimeoutMs, true );
    idleTimeoutMs = cfg.GetAttributeValueOrChildValueByName( "idleTimeoutMs" ).AsUInt32( idleTimeoutMs, true );
    maxStreamBacklogBytes = cfg.GetAttributeValueOrChildValueByName( "maxStreamBacklogBytes" ).AsUInt32( maxStreamBacklogBytes, true );
    tls.LoadConfig( cfg );

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
    , m_pulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() )
    , m_tcpServerSocket( m_pulseGenerator, false, ( GUCEF_NULL != settings ? settings->maxClientConnections : 100 ) + 1 )
    , m_requestHandler()
    , m_requestHandlerFactory( requestHandlerFactory )
    , m_settings()
    , m_webSocketHandler( GUCEF_NULL )
    , m_takeoverRoutes()
    , m_connectionStates()
    , m_hasPulseWork( false )
    , m_nextDeadlineCheckInMs( 0 )
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
        ApplyTls();
    }

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

CHTTPServer::CHTTPServer( const CORE::PulseGeneratorPtr& pulsGenerator                               ,
                          THttpServerRequestHandlerFactory* requestHandlerFactory /* = GUCEF_NULL */ ,
                          const CHttpServerSettings* settings /* = GUCEF_NULL */                     )
    : CORE::CObservingNotifier()
    , m_pulseGenerator( pulsGenerator.IsNULL() ? CORE::CCoreGlobal::Instance()->GetPulseGenerator() : pulsGenerator )
    , m_tcpServerSocket( m_pulseGenerator, false, ( GUCEF_NULL != settings ? settings->maxClientConnections : 100 ) + 1 )
    , m_requestHandler()
    , m_requestHandlerFactory( requestHandlerFactory )
    , m_settings()
    , m_webSocketHandler( GUCEF_NULL )
    , m_takeoverRoutes()
    , m_connectionStates()
    , m_hasPulseWork( false )
    , m_nextDeadlineCheckInMs( 0 )
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
        ApplyTls();
    }

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

CHTTPServer::~CHTTPServer()
{GUCEF_TRACE;

    // The subscriptions go first: no event of what the server observes (the pulse generator, the server socket, the
    // connections) starts after them. Telling those notifiers takes their notification locks under the server's
    // notification lock, which is its lock (CObservingNotifier); their events take the server's lock under theirs. The
    // two never meet: the server is destroyed on the thread that pulses its pulse generator, where its events run
    SignalUpcomingObserverDestruction();
    UnsubscribeAllFromObserver();

    MT::CScopeMutex lock( m_lock );

    // exchanges that handlers still hold must not reach back into a server that is gone
    for ( TConnectionStateVector::iterator i = m_connectionStates.begin(); i != m_connectionStates.end(); ++i )
    {
        CHttpServerConnectionState* state = (*i);
        if ( !state->exchange.IsNULL() )
        {
            state->exchange->m_server = GUCEF_NULL;
            if ( CHttpServerExchange::EXCHANGE_ENDED != state->exchange->m_state )
                state->exchange->m_state = CHttpServerExchange::EXCHANGE_CLOSED;
            state->exchange.Unlink();
        }
        GUCEF_DELETE state;
    }
    m_connectionStates.clear();
    m_takeoverRoutes.clear();
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

    // the socket tells of every connection that ends, also those that end in a socket error, which the
    // connection itself does not report as a disconnect
    TEventCallback disconnectCallback( this, &CHTTPServer::OnClientDisconnectedEvent );
    SubscribeTo( &m_tcpServerSocket                                 ,
                 COMCORE::CTCPServerSocket::ClientDisconnectedEvent ,
                 disconnectCallback                                 );

    TEventCallback pulseCallback( this, &CHTTPServer::OnPulse );
    SubscribeTo( m_pulseGenerator.GetPointerAlways() ,
                 CORE::CPulseGenerator::PulseEvent   ,
                 pulseCallback                       );
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
    ApplyTls();
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::ApplyTls( void )
{GUCEF_TRACE;

    COMCORE::CTlsServerSettings tls;
    {
        MT::CScopeMutex lock( m_lock );
        tls = m_settings.tls;
    }

    // takes effect at the next listen; TLS that cannot be used makes that listen fail
    if ( !m_tcpServerSocket.SetTlsSettings( tls ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CHTTPServer: HTTPS cannot be served with the certificate \"" << tls.GetCertificateChainPath() << "\" and the key \"" << tls.GetPrivateKeyPath() << "\"" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CHTTPServer::IsTlsEnabled( void ) const
{GUCEF_TRACE;

    return m_tcpServerSocket.IsTlsEnabled();
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

void
CHTTPServer::GetLimits( SLimits& limits ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    limits.maxClientConnections = m_settings.maxClientConnections;
    limits.maxHeaderBytes = m_settings.maxHeaderBytes > 0 ? m_settings.maxHeaderBytes : (UInt32) CHttpServerSettings::DefaultMaxHeaderBytes;
    limits.maxHeaderCount = m_settings.maxHeaderCount > 0 ? m_settings.maxHeaderCount : (UInt32) CHttpServerSettings::DefaultMaxHeaderCount;
    limits.maxBodyBytes = m_settings.maxBodyBytes;
    limits.headerTimeoutMs = m_settings.headerTimeoutMs;
    limits.bodyTimeoutMs = m_settings.bodyTimeoutMs;
    limits.idleTimeoutMs = m_settings.idleTimeoutMs;
    limits.maxStreamBacklogBytes = m_settings.maxStreamBacklogBytes > 0 ? m_settings.maxStreamBacklogBytes : (UInt32) CHttpServerSettings::DefaultMaxStreamBacklogBytes;
    limits.keepAliveConnections = m_settings.keepAliveConnections;
    limits.processRequestsAsync = m_settings.processRequestsAsync;
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

CORE::PulseGeneratorPtr
CHTTPServer::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

UInt64
CHTTPServer::NowInMs( void ) const
{GUCEF_TRACE;

    return CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs();
}

/*-------------------------------------------------------------------------*/

UInt32
CHTTPServer::GetOpenExchangeCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( TConnectionStateVector::const_iterator i = m_connectionStates.begin(); i != m_connectionStates.end(); ++i )
    {
        if ( !(*i)->exchange.IsNULL() )
            ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CHTTPServer::GetTrackedConnectionCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( TConnectionStateVector::const_iterator i = m_connectionStates.begin(); i != m_connectionStates.end(); ++i )
    {
        if ( (*i)->inUse && CHttpServerConnectionState::PHASE_WEBSOCKET != (*i)->phase )
            ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CHTTPServer::GetBufferedRequestBytes( void ) const
{GUCEF_TRACE;

    UInt32 total = 0;
    for ( TConnectionStateVector::const_iterator i = m_connectionStates.begin(); i != m_connectionStates.end(); ++i )
        total += (*i)->buffer.GetDataSize();
    return total;
}

/*-------------------------------------------------------------------------*/

CHttpServerConnectionState*
CHTTPServer::GetConnectionState( UInt32 connectionIndex ) const
{GUCEF_TRACE;

    if ( connectionIndex < m_connectionStates.size() )
        return m_connectionStates[ connectionIndex ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CHttpServerConnectionState*
CHTTPServer::GetExchangeState( const CHttpServerExchange& exchange ) const
{GUCEF_TRACE;

    // an exchange only ever reaches the connection it was made for: the state must still hold this very exchange
    CHttpServerConnectionState* state = GetConnectionState( exchange.m_connectionIndex );
    if ( GUCEF_NULL == state || !state->inUse || state->exchange.GetPointerAlways() != &exchange )
        return GUCEF_NULL;
    if ( GUCEF_NULL == state->connection || !state->connection->IsActive() )
        return GUCEF_NULL;
    return state;
}

/*-------------------------------------------------------------------------*/

bool
CHTTPServer::RegisterRequestTakeoverHandler( const CORE::CString& exactPath           ,
                                             CIHttpRequestTakeoverHandler* handler    ,
                                             UInt32 maxBodyBytes                      )
{GUCEF_TRACE;

    if ( exactPath.IsNULLOrEmpty() || GUCEF_NULL == handler )
        return false;

    MT::CScopeMutex lock( m_lock );
    if ( m_takeoverRoutes.find( exactPath ) != m_takeoverRoutes.end() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" << CORE::PointerToString( this ) << "): The path \"" << exactPath << "\" has a takeover handler already" );
        return false;
    }
    STakeoverRoute& route = m_takeoverRoutes[ exactPath ];
    route.handler = handler;
    route.maxBodyBytes = maxBodyBytes;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHTTPServer::UnregisterRequestTakeoverHandler( const CORE::CString& exactPath )
{GUCEF_TRACE;

    CIHttpRequestTakeoverHandler* handler = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        TTakeoverRouteMap::iterator i = m_takeoverRoutes.find( exactPath );
        if ( i == m_takeoverRoutes.end() )
            return false;
        handler = (*i).second.handler;
        m_takeoverRoutes.erase( i );
    }

    // what the handler has of this route ends now, without a word to the handler that asked for it
    GUCEF::vector< COMCORE::CTCPServerConnection* > toClose;
    for ( TConnectionStateVector::iterator i = m_connectionStates.begin(); i != m_connectionStates.end(); ++i )
    {
        CHttpServerConnectionState* state = (*i);
        if ( !state->inUse || state->takeoverHandler != handler || state->takeoverPath != exactPath )
            continue;
        if ( !state->exchange.IsNULL() )
        {
            if ( CHttpServerExchange::EXCHANGE_ENDED != state->exchange->m_state )
                state->exchange->m_state = CHttpServerExchange::EXCHANGE_CLOSED;
            state->exchange->m_server = GUCEF_NULL;
            state->exchange.Unlink();
        }
        state->takeoverHandler = GUCEF_NULL;
        if ( GUCEF_NULL != state->connection )
            toClose.push_back( state->connection );
    }
    for ( size_t i=0; i<toClose.size(); ++i )
        toClose[ i ]->Close();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::OnClientConnectedEvent( CORE::CNotifier* notifier                 ,
                                     const CORE::CEvent& eventid               ,
                                     CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    const COMCORE::CTCPServerSocket::TClientConnectedEventData* eData = static_cast< COMCORE::CTCPServerSocket::TClientConnectedEventData* >( eventdata );
    const COMCORE::CTCPServerSocket::TConnectionInfo& storage = eData->GetData();

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" << CORE::PointerToString( this ) << "): Client connected" );

    const UInt32 index = storage.connection->GetConnectionIndex();
    while ( m_connectionStates.size() <= index )
        m_connectionStates.push_back( GUCEF_NEW CHttpServerConnectionState() );
    CHttpServerConnectionState& state = *m_connectionStates[ index ];
    ResetConnectionState( state );
    state.inUse = true;
    state.connection = storage.connection;

    SLimits limits;
    GetLimits( limits );
    // until a whole request header came in, the time from the accept on is that of a header (RFC 9112 9.3 leaves the
    // limits to the server): a connection that sends nothing is no idle client and holds no slot for a minute
    const UInt64 now = NowInMs();
    state.phaseSinceInMs = now;
    // the shorter of the header and the idle timeout: a new connection is idle and waits for a header at once
    UInt32 firstTimeoutMs = limits.headerTimeoutMs > 0 ? limits.headerTimeoutMs : limits.idleTimeoutMs;
    if ( limits.idleTimeoutMs > 0 && limits.idleTimeoutMs < firstTimeoutMs )
        firstTimeoutMs = limits.idleTimeoutMs;
    if ( firstTimeoutMs > 0 )
        state.deadlineInMs = now + firstTimeoutMs;

    // the socket takes one connection more than allowed, so that a new client is always accepted; beyond the limit
    // the connection that waits longest without a request in progress makes room, which is the new one itself when
    // every other connection is busy
    UInt32 inUse = 0;
    CHttpServerConnectionState* evict = GUCEF_NULL;
    for ( size_t i=0; i<m_connectionStates.size(); ++i )
    {
        CHttpServerConnectionState& other = *m_connectionStates[ i ];
        if ( !other.inUse || GUCEF_NULL == other.connection )
            continue;
        ++inUse;
        const bool evictable = ( CHttpServerConnectionState::PHASE_IDLE == other.phase || CHttpServerConnectionState::PHASE_HEADERS == other.phase ) &&
                               !other.processing && !other.closeRequested && !other.closeWhenFlushed && other.exchange.IsNULL();
        if ( evictable && &other != &state && ( GUCEF_NULL == evict || other.phaseSinceInMs < evict->phaseSinceInMs ) )
            evict = &other;
    }
    if ( limits.maxClientConnections > 0 && inUse > limits.maxClientConnections )
    {
        if ( GUCEF_NULL == evict )
            evict = &state;
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" << CORE::PointerToString( this ) << "): " << inUse << " connections, more than the "
                         << limits.maxClientConnections << " allowed: the connection that waited longest without a request is closed" );
        evict->closeRequested = true;
        m_hasPulseWork = true;
    }

    SubscribeToNewConnection( storage.connection );
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::SubscribeToNewConnection( COMCORE::CTCPServerConnection* connection )
{GUCEF_TRACE;

    TEventCallback callback( this, &CHTTPServer::OnClientDataReceivedEvent );
    SubscribeTo( connection                                       ,
                 COMCORE::CTCPServerConnection::DataRecievedEvent ,
                 callback                                         );
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::ResetConnectionState( CHttpServerConnectionState& state )
{GUCEF_TRACE;

    ++state.generation;
    state.inUse = false;
    state.connection = GUCEF_NULL;
    state.phase = CHttpServerConnectionState::PHASE_IDLE;
    state.hadRequest = false;
    state.phaseSinceInMs = 0;
    state.buffer.Clear( false );                // the memory goes too: a connection that is gone keeps nothing
    state.deadlineInMs = 0;
    state.processing = false;
    state.resumePending = false;
    state.closeRequested = false;
    state.closeWhenFlushed = false;
    state.closeDeadlineInMs = 0;
    state.ResetRequest();
    state.exchange.Unlink();
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::OnClientDisconnectedEvent( CORE::CNotifier* notifier                 ,
                                        const CORE::CEvent& eventid               ,
                                        CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    const COMCORE::CTCPServerSocket::TClientDisconnectedEventData* eData = static_cast< COMCORE::CTCPServerSocket::TClientDisconnectedEventData* >( eventdata );
    COMCORE::CTCPServerConnection* connection = eData->GetData().connectionInfo.connection;
    if ( GUCEF_NULL == connection )
        return;

    // Tell the web socket handler of the connection, if it has one, that its connection ended
    CIWebSocketConnectionHandler* handler = static_cast< CIWebSocketConnectionHandler* >( connection->GetUserData() );
    if ( GUCEF_NULL != handler )
    {
        connection->SetUserData( GUCEF_NULL );
        handler->OnWebSocketClosed();
    }

    CHttpServerConnectionState* state = GetConnectionState( connection->GetConnectionIndex() );
    if ( GUCEF_NULL == state || !state->inUse || state->connection != connection )
        return;

    // an exchange the handler did not end is closed under it: the handler hears of it, once
    CHttpServerExchangePtr exchange = state->exchange;
    CIHttpRequestTakeoverHandler* takeoverHandler = state->takeoverHandler;
    ResetConnectionState( *state );

    if ( !exchange.IsNULL() )
    {
        const CHttpServerExchange::EState before = exchange->m_state;
        exchange->m_server = GUCEF_NULL;
        if ( CHttpServerExchange::EXCHANGE_ENDED != before )
        {
            exchange->m_state = CHttpServerExchange::EXCHANGE_CLOSED;
            if ( GUCEF_NULL != takeoverHandler && ( CHttpServerExchange::EXCHANGE_OPEN == before || CHttpServerExchange::EXCHANGE_STREAMING == before ) )
                takeoverHandler->OnHttpExchangeClosed( *this, *exchange );
        }
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

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CHTTPServer(" << CORE::PointerToString( this ) << "): " << receivedData.GetDataSize() << " Bytes received from client " << connection->GetRemoteHostName() );

    // Connections which were upgraded to the web socket protocol no longer speak HTTP,
    // the handler they were handed to is linked on the connection itself
    CIWebSocketConnectionHandler* webSocketHandler = static_cast< CIWebSocketConnectionHandler* >( connection->GetUserData() );
    if ( GUCEF_NULL != webSocketHandler )
    {
        webSocketHandler->OnWebSocketData( receivedData.GetConstBufferPtr(), receivedData.GetDataSize() );
        return;
    }

    CHttpServerConnectionState* state = GetConnectionState( connection->GetConnectionIndex() );
    if ( GUCEF_NULL == state || !state->inUse || state->connection != connection || 0 == receivedData.GetDataSize() )
        return;

    ProcessConnectionData( *state, static_cast< const char* >( receivedData.GetConstBufferPtr() ), receivedData.GetDataSize() );
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

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "HttpServer(" << CORE::PointerToString( this ) << "):OnWebSocketUpgrade: Client attempted websocket upgrade. This feature is disabled" );
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

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "HttpServer(" << CORE::PointerToString( this ) << "):OnWebSocketUpgrade: Client attempted websocket upgrade without all the mandatory headers" );
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

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "HttpServer(" << CORE::PointerToString( this ) << "):OnWebSocketUpgrade: Client attempted websocket upgrade without the connection upgrade header" );
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

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "HttpServer(" << CORE::PointerToString( this ) << "):OnWebSocketUpgrade: Client attempting to connect with unsupported websocket protocol version: " << (*versionHeader).second );
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

        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpServer(" << CORE::PointerToString( this ) << "):OnWebSocketUpgrade: Cannot perform websocket upgrade: No SHA1 codec is available" );
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

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "HttpServer(" << CORE::PointerToString( this ) << "):OnWebSocketUpgrade: The web socket upgrade handler refused the upgrade" );
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


/** Finds the empty line that ends a header block: the offset just past it, 0 when it is not there yet */
static UInt32
FindHeaderBlockEnd( const char* data, UInt32 size, UInt32 scanFrom )
{GUCEF_TRACE;

    for ( UInt32 i=scanFrom; i+3<size; ++i )
    {
        if ( '\r' == data[ i ] && '\n' == data[ i+1 ] && '\r' == data[ i+2 ] && '\n' == data[ i+3 ] )
            return i + 4;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::ProcessConnectionData( CHttpServerConnectionState& state, const char* data, UInt32 dataSize )
{GUCEF_TRACE;

    if ( state.processing )
    {
        // a handler that answered from inside a callback made the server resume: the loop below goes on by itself
        if ( dataSize > 0 )
            state.buffer.Append( data, dataSize );
        return;
    }

    SLimits limits;
    GetLimits( limits );

    if ( CHttpServerConnectionState::PHASE_CLOSING == state.phase )
        return;

    // What is held for a connection is bounded: the request coming in, and what waits behind the one being answered,
    // up to what one more request may hold
    const UInt64 bound = (UInt64) limits.maxHeaderBytes + (UInt64) limits.maxBodyBytes + 65536;
    if ( (UInt64) state.buffer.GetDataSize() + dataSize > bound )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" << CORE::PointerToString( this ) << "): A client sent more than the server holds for a connection, the connection is closed" );
        state.connection->Close();
        return;
    }
    if ( CHttpServerConnectionState::PHASE_DISPATCHED == state.phase )
    {
        if ( dataSize > 0 )
            state.buffer.Append( data, dataSize );
        return;
    }

    // The common case, a whole request in one read with nothing held from before, is parsed where it lies
    const char* view = GUCEF_NULL;
    UInt32 viewSize = 0;
    const bool inPlace = ( 0 == state.buffer.GetDataSize() );
    if ( inPlace )
    {
        view = data;
        viewSize = dataSize;
    }
    else
    {
        if ( dataSize > 0 )
            state.buffer.Append( data, dataSize );
        view = static_cast< const char* >( state.buffer.GetConstBufferPtr() );
        viewSize = state.buffer.GetDataSize();
    }
    if ( GUCEF_NULL == view || 0 == viewSize )
        return;

    const UInt32 generation = state.generation;
    state.processing = true;
    UInt32 offset = 0;
    while ( offset < viewSize && generation == state.generation )
    {
        if ( CHttpServerConnectionState::PHASE_IDLE == state.phase )
        {
            // Empty lines in front of a request line are ignored (RFC 9112, 2.2)
            while ( offset < viewSize && ( '\r' == view[ offset ] || '\n' == view[ offset ] ) )
                ++offset;
            if ( offset >= viewSize )
                break;

            // a client that sends requests without reading the responses waits until they went out
            if ( state.connection->GetBufferedSendDataSize() >= limits.maxStreamBacklogBytes )
            {
                state.resumePending = true;
                m_hasPulseWork = true;     // the periodic pulses of the socket look again, no pulse is forced
                break;
            }
            state.ResetRequest();
            state.phase = CHttpServerConnectionState::PHASE_HEADERS;
            if ( state.hadRequest || 0 == state.deadlineInMs )
            {
                // the first request of a connection keeps the deadline that counts from the accept
                state.phaseSinceInMs = NowInMs();
                state.deadlineInMs = limits.headerTimeoutMs > 0 ? state.phaseSinceInMs + limits.headerTimeoutMs : 0;
            }
        }

        if ( CHttpServerConnectionState::PHASE_HEADERS == state.phase )
        {
            const UInt32 available = viewSize - offset;
            const UInt32 scanLimit = available < limits.maxHeaderBytes ? available : limits.maxHeaderBytes;
            const UInt32 scanFrom = state.headerScanPos > 3 ? state.headerScanPos - 3 : 0;
            const UInt32 headSize = FindHeaderBlockEnd( view + offset, scanLimit, scanFrom );
            if ( 0 == headSize )
            {
                if ( available >= limits.maxHeaderBytes )
                {
                    RefuseRequest( state, 431, "the header block is larger than allowed" );
                    break;
                }
                // the search goes on where it stopped when more comes in: a trickle costs O(n), not O(n^2)
                state.headerScanPos = available;
                break;
            }
            if ( !ProcessRequestHead( state, view + offset, headSize, limits ) )
                break;
            offset += headSize;
            if ( generation != state.generation )
                break;
        }

        if ( CHttpServerConnectionState::PHASE_BODY == state.phase )
        {
            if ( viewSize - offset < state.bodySize )
                break;

            if ( state.bodySize > 0 )
                state.request.content.LinkTo( view + offset, state.bodySize );
            offset += state.bodySize;
            state.phase = CHttpServerConnectionState::PHASE_DISPATCHED;
            state.hadRequest = true;
            state.deadlineInMs = 0;

            DispatchRequest( state, view + offset, viewSize - offset );
            if ( generation != state.generation )
                break;      // the connection went: what was viewed may be gone with it
            state.request.content.Clear( false );
            if ( CHttpServerConnectionState::PHASE_WEBSOCKET == state.phase )
            {
                // what came behind the upgrade went to the web socket handler
                offset = viewSize;
                break;
            }
            continue;
        }

        // with its handler or closing: the rest waits, or is dropped
        break;
    }

    if ( generation != state.generation )
        return;
    state.processing = false;

    if ( CHttpServerConnectionState::PHASE_CLOSING == state.phase || CHttpServerConnectionState::PHASE_WEBSOCKET == state.phase )
    {
        state.buffer.Clear( false );
        return;
    }

    // keep what was not taken: the rest of a request, or requests that wait for the one being answered
    if ( inPlace )
    {
        if ( offset < viewSize )
            state.buffer.CopyFrom( viewSize - offset, view + offset );
    }
    else
    {
        if ( offset >= state.buffer.GetDataSize() )
            state.buffer.Clear( false );
        else
        if ( offset > 0 )
            state.buffer.Downshift( offset );
    }

    // requests that wait until the responses before them went out
    if ( CHttpServerConnectionState::PHASE_IDLE == state.phase && state.buffer.GetDataSize() > 0 )
    {
        state.resumePending = true;
        m_hasPulseWork = true;
    }
}

/*-------------------------------------------------------------------------*/

bool
CHTTPServer::ProcessRequestHead( CHttpServerConnectionState& state, const char* head, UInt32 headSize, const SLimits& limits )
{GUCEF_TRACE;

    SRequestHeadInfo info;
    if ( !ParseRequestHead( head, headSize, state.request, info ) || !info.requestLineValid )
    {
        RefuseRequest( state, 400, "the request line or a header field is malformed" );
        return false;
    }
    if ( info.fieldCount > limits.maxHeaderCount )
    {
        RefuseRequest( state, 431, "the request has more header fields than allowed" );
        return false;
    }
    if ( info.hostCount > 1 || info.contentTypeCount > 1 )
    {
        RefuseRequest( state, 400, "a header field that can only be there once is there more than once" );
        return false;
    }
    if ( info.hasTransferEncoding )
    {
        // chunked request bodies are not supported, and a Transfer-Encoding next to a Content-Length is how requests are smuggled
        RefuseRequest( state, 501, "request bodies with a Transfer-Encoding are not supported" );
        return false;
    }
    if ( !info.contentLengthValid )
    {
        RefuseRequest( state, 400, "the Content-Length is not one number" );
        return false;
    }
    if ( info.hasExpect && !info.expect.Equals( "100-continue", false ) )
    {
        RefuseRequest( state, 417, "the expectation is not supported" );
        return false;
    }
    state.keepAlive = info.keepAlive;

    // the takeover route of the exact path, if there is one
    UInt32 bodyLimit = limits.maxBodyBytes;
    {
        MT::CScopeMutex lock( m_lock );
        TTakeoverRouteMap::const_iterator route = m_takeoverRoutes.find( state.request.requestUri );
        if ( route != m_takeoverRoutes.end() )
        {
            state.takeoverHandler = (*route).second.handler;
            state.takeoverPath = (*route).first;
            if ( (*route).second.maxBodyBytes > 0 )
                bodyLimit = (*route).second.maxBodyBytes;
        }
    }

    if ( GUCEF_NULL != state.takeoverHandler )
    {
        UInt64 exchangeId = 0;
        {
            MT::CScopeMutex lock( g_exchangeIdLock );
            exchangeId = g_nextExchangeId++;
        }
        CHttpServerExchange* exchange = GUCEF_NEW CHttpServerExchange( this, state.connection->GetConnectionIndex(), exchangeId, state.connection->GetRemoteHostAddress() );
        state.exchange = exchange->GetSharedPtr();

        // the checks of the handler come before the body is read: who is refused cannot make us buffer a body
        CHttpResponseData refusal;
        const UInt32 generation = state.generation;
        CIHttpRequestTakeoverHandler::EHeaderDecision decision = state.takeoverHandler->OnHttpRequestHeaders( *this, state.request, *exchange, refusal );
        if ( generation != state.generation )
            return false;
        if ( CIHttpRequestTakeoverHandler::TAKEOVER_REJECT == decision )
        {
            if ( refusal.statusCode < 400 )
                refusal.statusCode = 400;
            state.exchange->m_state = CHttpServerExchange::EXCHANGE_ENDED;
            state.exchange->m_server = GUCEF_NULL;
            state.exchange.Unlink();
            SendRefusal( state, refusal );
            return false;
        }
    }

    if ( info.contentLength > (UInt64) bodyLimit )
    {
        RefuseRequest( state, 413, "the body is larger than allowed" );
        return false;
    }
    state.bodySize = (UInt32) info.contentLength;

    // a client that waits for a go ahead gets it now that the checks passed
    if ( info.hasExpect && state.bodySize > 0 )
    {
        static const char continueResponse[] = "HTTP/1.1 100 Continue\r\n\r\n";
        state.connection->Send( continueResponse, (UInt32) sizeof( continueResponse ) - 1 );
    }

    state.phase = CHttpServerConnectionState::PHASE_BODY;
    state.deadlineInMs = ( state.bodySize > 0 && limits.bodyTimeoutMs > 0 ) ? NowInMs() + limits.bodyTimeoutMs : 0;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::DispatchRequest( CHttpServerConnectionState& state, const char* rest, UInt32 restSize )
{GUCEF_TRACE;

    if ( GUCEF_NULL != state.takeoverHandler && !state.exchange.IsNULL() )
    {
        CHttpServerExchangePtr exchange = state.exchange;
        exchange->m_state = CHttpServerExchange::EXCHANGE_OPEN;
        state.takeoverHandler->OnHttpRequest( *this, state.request, *exchange );
        return;
    }

    if ( state.request.isWebSocketUpgrade )
    {
        const UInt32 generation = state.generation;
        COMCORE::CTCPServerConnection* connection = state.connection;
        OnWebSocketUpgrade( connection, state.request );
        if ( generation != state.generation )
            return;

        CIWebSocketConnectionHandler* handler = static_cast< CIWebSocketConnectionHandler* >( connection->GetUserData() );
        if ( GUCEF_NULL == handler )
        {
            // nobody took the connection over: it carries no HTTP after the 101 either
            CloseAfterFlush( state );
            return;
        }
        state.phase = CHttpServerConnectionState::PHASE_WEBSOCKET;
        state.deadlineInMs = 0;

        // bytes the client sent right behind its upgrade request belong to the web socket
        if ( restSize > 0 )
            handler->OnWebSocketData( rest, restSize );
        return;
    }

    SLimits limits;
    GetLimits( limits );
    if ( limits.processRequestsAsync )
        ASyncProcessRouterRequest( state );
    else
        SyncProcessRouterRequest( state );
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::RefuseRequest( CHttpServerConnectionState& state, int statusCode, const CORE::CString& reason )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" << CORE::PointerToString( this ) << "): Refused a request with " << statusCode << ": " << reason );

    if ( !state.exchange.IsNULL() )
    {
        state.exchange->m_state = CHttpServerExchange::EXCHANGE_ENDED;
        state.exchange->m_server = GUCEF_NULL;
        state.exchange.Unlink();
    }

    CHttpResponseData response;
    response.statusCode = statusCode;
    SendRefusal( state, response );
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::SendRefusal( CHttpServerConnectionState& state, CHttpResponseData& response )
{GUCEF_TRACE;

    CORE::CDynamicBuffer responseBuffer;
    if ( !response.Serialize( responseBuffer, false ) )
    {
        CHttpResponseData plain;
        plain.statusCode = 500;
        plain.Serialize( responseBuffer, false );
    }
    state.connection->Send( responseBuffer );
    CloseAfterFlush( state );
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::CloseAfterFlush( CHttpServerConnectionState& state )
{GUCEF_TRACE;

    state.phase = CHttpServerConnectionState::PHASE_CLOSING;
    state.deadlineInMs = 0;
    if ( GUCEF_NULL == state.connection )
        return;

    // A close drops what the socket did not take yet, so the close waits for the send queue to drain
    if ( 0 == state.connection->GetBufferedSendDataSize() )
    {
        state.connection->Close();
        return;
    }
    SLimits limits;
    GetLimits( limits );
    state.closeWhenFlushed = true;
    state.closeDeadlineInMs = NowInMs() + ( limits.bodyTimeoutMs > 0 ? limits.bodyTimeoutMs : (UInt32) CHttpServerSettings::DefaultBodyTimeoutMs );
    RequestPulseWork();
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::RequestPulseWork( void )
{GUCEF_TRACE;

    // the periodic pulses the socket asks for while it listens do the work. Not RequestImmediatePulse(): without a
    // driver it pulses at once, from inside the call that asked, which is what the deferral is there to avoid
    m_hasPulseWork = true;
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::SyncProcessRouterRequest( CHttpServerConnectionState& state )
{GUCEF_TRACE;

    SLimits limits;
    GetLimits( limits );
    const bool keepAlive = limits.keepAliveConnections && state.keepAlive;

    CHttpResponseData response;
    try
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" << CORE::PointerToString( this ) << "): About to process request of type: " << state.request.requestType );
        m_requestHandler->OnRequest( state.request, response );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" << CORE::PointerToString( this ) << "): Exception sync processing a request: " << e.what() );

        // Inform the client we ran into an internal server error so they are not just sitting there waiting for a response
        response = CHttpResponseData();
        response.statusCode = 500;
    }

    const bool stays = keepAlive && response.keepConnectionsAlive;
    CORE::CDynamicBuffer responseBuffer;
    if ( !response.Serialize( responseBuffer, stays ) )
    {
        CHttpResponseData plain;
        plain.statusCode = 500;
        plain.Serialize( responseBuffer, false );
        state.connection->Send( responseBuffer );
        CloseAfterFlush( state );
        return;
    }

    // The response goes out once: before this was reworked it went out a second time on keep-alive connections
    state.connection->Send( responseBuffer );
    if ( !stays )
    {
        CloseAfterFlush( state );
        return;
    }
    state.phase = CHttpServerConnectionState::PHASE_IDLE;
    state.phaseSinceInMs = NowInMs();
    state.deadlineInMs = limits.idleTimeoutMs > 0 ? NowInMs() + limits.idleTimeoutMs : 0;
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::ASyncProcessRouterRequest( CHttpServerConnectionState& state )
{GUCEF_TRACE;

    SLimits limits;
    GetLimits( limits );

    // a copy for the task: the content is copied with it, it no longer points into what was received
    CAsyncHttpRequestData requestData;
    static_cast< CHttpRequestData& >( requestData ) = state.request;
    requestData.clientConnectionId = state.connection->GetConnectionIndex();
    requestData.remoteClientAddress = state.connection->GetRemoteHostAddress().GetFirstIPv4Address();
    requestData.keepConnectionsAlive = limits.keepAliveConnections && state.keepAlive;
    requestData.httpServer = this;

    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    CORE::CFutureResult futureResult = taskManager.GetThreadPool()->QueueTask( CAsyncHttpServerRequestHandler::TaskType, &requestData, GUCEF_NULL, &AsObserver() );
    if ( futureResult.HasAFuture() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" << CORE::PointerToString( this ) << "): Queued task to ASync handle the request processing" );

        // the task answers on its own; the next request of the connection is taken in meanwhile
        state.phase = CHttpServerConnectionState::PHASE_IDLE;
        state.phaseSinceInMs = NowInMs();
        state.deadlineInMs = limits.idleTimeoutMs > 0 ? NowInMs() + limits.idleTimeoutMs : 0;
        return;
    }

    // Since we are doing async processing the failure to queue a processing task would leave a
    // client in limbo up to their timeout. To prevent this we will handle this scenario here directly
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" << CORE::PointerToString( this ) << "): Unable to ASync queue the request processing (QueueTask failed). As such will respond with 503" );

    CHttpResponseData returnData;
    returnData.statusCode = 503;   // Send 'Server too busy' because this is per configured capacity constraints
    SendRefusal( state, returnData );
}

/*-------------------------------------------------------------------------*/

bool
CHTTPServer::OnExchangeSendResponse( CHttpServerExchange& exchange, const CHttpResponseData& response )
{GUCEF_TRACE;

    CHttpServerConnectionState* state = GetExchangeState( exchange );
    if ( GUCEF_NULL == state )
        return false;

    SLimits limits;
    GetLimits( limits );
    const bool stays = limits.keepAliveConnections && state->keepAlive && response.keepConnectionsAlive;

    CORE::CDynamicBuffer responseBuffer;
    bool sendOk = response.Serialize( responseBuffer, stays );
    if ( !sendOk )
    {
        CHttpResponseData plain;
        plain.statusCode = 500;
        plain.Serialize( responseBuffer, false );
    }

    exchange.m_state = CHttpServerExchange::EXCHANGE_ENDED;
    exchange.m_server = GUCEF_NULL;
    state->exchange.Unlink();
    state->takeoverHandler = GUCEF_NULL;

    sendOk = state->connection->Send( responseBuffer ) && sendOk;
    if ( !stays || !sendOk )
    {
        CloseAfterFlush( *state );
        return sendOk;
    }

    state->phase = CHttpServerConnectionState::PHASE_IDLE;
    state->phaseSinceInMs = NowInMs();
    state->deadlineInMs = limits.idleTimeoutMs > 0 ? NowInMs() + limits.idleTimeoutMs : 0;

    // the next request that waited behind this one; the processing loop takes it when the answer came from inside it
    if ( !state->processing && state->buffer.GetDataSize() > 0 )
    {
        state->resumePending = true;
        RequestPulseWork();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHTTPServer::OnExchangeBeginStream( CHttpServerExchange& exchange, const CHttpResponseData& head )
{GUCEF_TRACE;

    CHttpServerConnectionState* state = GetExchangeState( exchange );
    if ( GUCEF_NULL == state )
        return false;

    CORE::CDynamicBuffer headBuffer;
    if ( !head.SerializeStreamHead( headBuffer ) )
        return false;
    if ( !state->connection->Send( headBuffer ) )
        return false;
    exchange.m_state = CHttpServerExchange::EXCHANGE_STREAMING;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHTTPServer::OnExchangeStreamData( CHttpServerExchange& exchange, const void* data, UInt32 size )
{GUCEF_TRACE;

    CHttpServerConnectionState* state = GetExchangeState( exchange );
    if ( GUCEF_NULL == state )
        return false;

    SLimits limits;
    GetLimits( limits );
    const UInt64 backlog = (UInt64) state->connection->GetBufferedSendDataSize() + size;
    if ( backlog > (UInt64) limits.maxStreamBacklogBytes )
    {
        // a client that does not read: the server gives up on it on the next pulse and the handler is told then,
        // not from within this call of the handler
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" << CORE::PointerToString( this ) << "): A stream fell more than " << limits.maxStreamBacklogBytes << " bytes behind, the client is too slow and the connection is closed" );
        exchange.m_aborted = true;
        state->closeRequested = true;
        RequestPulseWork();
        return false;
    }
    return state->connection->Send( data, size );
}

/*-------------------------------------------------------------------------*/

bool
CHTTPServer::OnExchangeEndStream( CHttpServerExchange& exchange )
{GUCEF_TRACE;

    CHttpServerConnectionState* state = GetExchangeState( exchange );
    if ( GUCEF_NULL == state )
        return false;

    exchange.m_state = CHttpServerExchange::EXCHANGE_ENDED;
    exchange.m_server = GUCEF_NULL;
    state->exchange.Unlink();
    state->takeoverHandler = GUCEF_NULL;
    CloseAfterFlush( *state );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CHTTPServer::GetExchangeBacklogBytes( const CHttpServerExchange& exchange ) const
{GUCEF_TRACE;

    CHttpServerConnectionState* state = GetExchangeState( exchange );
    if ( GUCEF_NULL == state )
        return 0;
    return state->connection->GetBufferedSendDataSize();
}

/*-------------------------------------------------------------------------*/

void
CHTTPServer::OnPulse( CORE::CNotifier* notifier    ,
                      const CORE::CEvent& eventId  ,
                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_connectionStates.empty() )
        return;
    const UInt64 now = NowInMs();
    const bool checkDeadlines = now >= m_nextDeadlineCheckInMs;
    if ( !m_hasPulseWork && !checkDeadlines )
        return;

    SLimits limits;
    GetLimits( limits );
    if ( checkDeadlines )
    {
        // deadlines are looked at once a second, or more often when a timeout is short
        UInt32 interval = 1000;
        const UInt32 timeouts[ 3 ] = { limits.headerTimeoutMs, limits.bodyTimeoutMs, limits.idleTimeoutMs };
        for ( int t=0; t<3; ++t )
            if ( timeouts[ t ] > 0 && timeouts[ t ] / 4 < interval )
                interval = timeouts[ t ] / 4 < 10 ? 10 : timeouts[ t ] / 4;
        m_nextDeadlineCheckInMs = now + interval;
    }

    m_hasPulseWork = false;
    for ( size_t i=0; i<m_connectionStates.size(); ++i )
    {
        CHttpServerConnectionState& state = *m_connectionStates[ i ];
        if ( !state.inUse || GUCEF_NULL == state.connection )
            continue;

        if ( state.closeRequested )
        {
            state.closeRequested = false;
            state.connection->Close();
            continue;
        }
        if ( state.closeWhenFlushed )
        {
            if ( 0 == state.connection->GetBufferedSendDataSize() || now >= state.closeDeadlineInMs )
            {
                state.closeWhenFlushed = false;
                state.connection->Close();
            }
            else
            {
                m_hasPulseWork = true;
            }
            continue;
        }
        if ( state.resumePending )
        {
            state.resumePending = false;
            if ( CHttpServerConnectionState::PHASE_IDLE == state.phase && state.buffer.GetDataSize() > 0 )
                ProcessConnectionData( state, GUCEF_NULL, 0 );
            if ( state.resumePending )
                m_hasPulseWork = true;
            continue;
        }

        if ( checkDeadlines && 0 != state.deadlineInMs && now >= state.deadlineInMs )
        {
            state.deadlineInMs = 0;
            if ( CHttpServerConnectionState::PHASE_HEADERS == state.phase || CHttpServerConnectionState::PHASE_BODY == state.phase )
            {
                RefuseRequest( state, 408, "the request did not come in within the time allowed" );
            }
            else
            if ( CHttpServerConnectionState::PHASE_IDLE == state.phase )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" << CORE::PointerToString( this ) << "): A connection without a request was idle too long and is closed" );
                state.connection->Close();
            }
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
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" << CORE::PointerToString( this ) << "): Exception parsing HTTP request header fields: " << e.what() );
        return 0;
    }
}

/*-------------------------------------------------------------------------*/

/** Whether the text is a token (RFC 9110 5.6.2), the syntax of a method */
static bool
IsToken( const CString& text )
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() )
        return false;
    const char* chars = text.C_String();
    const UInt32 size = text.ByteSize() - 1;
    for ( UInt32 i=0; i<size; ++i )
    {
        const unsigned char c = (unsigned char) chars[ i ];
        if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) )
            continue;
        if ( GUCEF_NULL == memchr( "!#$%&'*+-.^_`|~", (int) c, 15 ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Whether the byte may be in a field name: a tchar of RFC 9110 5.6.2 */
static bool
IsTokenChar( unsigned char c )
{GUCEF_TRACE;

    if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) )
        return true;
    return GUCEF_NULL != memchr( "!#$%&'*+-.^_`|~", (int) c, 15 );
}

/*-------------------------------------------------------------------------*/

/**
 *  Checks the syntax of a header block that ends with an empty line, before anything of it is used, as RFC 9112
 *  asks of a server that may sit behind a proxy (request smuggling):
 *  - every line ends with CR LF; a CR or a LF on its own is refused (2.2);
 *  - the request line holds no control characters;
 *  - no field line starts with whitespace: that is an obs-fold, or whitespace in front of the first field (2.2, 5.2);
 *  - a field line is a name that is a token, directly followed by a colon (5.1: no whitespace before the colon);
 *  - a field value holds visible characters, obs-text, spaces and tabs only (RFC 9110 5.5): no NUL, no control.
 *  @return false when the block has to be refused with 400
 */
static bool
IsWellFormedHeaderBlock( const char* head, UInt32 headSize )
{GUCEF_TRACE;

    const unsigned char* bytes = reinterpret_cast< const unsigned char* >( head );
    UInt32 lineStart = 0;
    bool requestLine = true;
    for ( UInt32 i=0; i<headSize; ++i )
    {
        const unsigned char c = bytes[ i ];
        if ( '\n' == c )
            return false;                               // a LF that does not follow a CR
        if ( '\r' != c )
            continue;
        if ( i+1 >= headSize || '\n' != bytes[ i+1 ] )
            return false;                               // a CR that is not followed by a LF

        const UInt32 lineSize = i - lineStart;
        const unsigned char* line = bytes + lineStart;
        if ( requestLine )
        {
            for ( UInt32 n=0; n<lineSize; ++n )
                if ( line[ n ] < 0x20 || 0x7F == line[ n ] )
                    return false;
            requestLine = false;
        }
        else
        if ( lineSize > 0 )
        {
            UInt32 n = 0;
            while ( n < lineSize && IsTokenChar( line[ n ] ) )
                ++n;
            if ( 0 == n || n >= lineSize || ':' != line[ n ] )
                return false;                           // no name, a character that is no tchar, or no colon right after the name
            for ( ++n; n<lineSize; ++n )
            {
                const unsigned char v = line[ n ];
                if ( ( v < 0x20 && '\t' != v ) || 0x7F == v )
                    return false;
            }
        }
        else
        {
            // the empty line that ends the block
            return true;
        }
        ++i;
        lineStart = i + 1;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/**
 *  The value of a Content-Length field: one number of digits. Fields that came more than once were joined with
 *  commas, as for any list; they are accepted when every element is the same number (RFC 9110 8.6)
 */
static bool
ParseContentLength( const CString& value, UInt64& length )
{GUCEF_TRACE;

    CString::StringVector elements;
    SplitListValue( value, elements );
    if ( elements.empty() )
        return false;
    for ( size_t e=0; e<elements.size(); ++e )
    {
        const CString& element = elements[ e ];
        const char* digits = element.C_String();
        const UInt32 size = element.ByteSize() - 1;
        if ( 0 == size || size > 19 )
            return false;
        UInt64 number = 0;
        for ( UInt32 i=0; i<size; ++i )
        {
            if ( digits[ i ] < '0' || digits[ i ] > '9' )
                return false;
            number = number * 10 + (UInt64) ( digits[ i ] - '0' );
        }
        if ( 0 == e )
            length = number;
        else
        if ( number != length )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHTTPServer::ParseRequestHead( const char* head, UInt32 headSize, CHttpRequestData& request, SRequestHeadInfo& info ) const
{GUCEF_TRACE;

    try
    {
        if ( GUCEF_NULL == head || 0 == headSize )
            return false;
        if ( !IsWellFormedHeaderBlock( head, headSize ) )
            return false;

        // Parse all the HTTP Header fields out of the header block
        TStringVector headerFields;
        UInt32 parsedSize = ParseHeaderFields( head, headSize, headerFields );

        // Sanity check on the parsed result
        if ( parsedSize == 0 || headerFields.size() == 0 )
            return false;

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

        info.requestLineValid = IsToken( request.requestType ) && !request.requestUri.IsNULLOrEmpty() &&
                                "HTTP" == request.requestProtocol && ( "1.1" == request.requestProtocolVersion || "1.0" == request.requestProtocolVersion );

        // HTTP/1.1 keeps the connection unless asked not to, HTTP/1.0 only when asked to
        info.keepAlive = "1.1" == request.requestProtocolVersion;
        request.keepConnectionsAlive = info.keepAlive;

        // IsWellFormedHeaderBlock() refused folded lines (obs-fold), so every line is one field
        info.fieldCount = (UInt32) headerFields.size();

        // Parse all the subsequent HTTP header fields
        TStringVector::iterator i = headerFields.begin();
        while ( i != headerFields.end() )
        {
            // Parse the header name and header value out of the header field
            // Only the whitespace around the name and the value is dropped, the name is lowercased for easy comparison
            CString headerName;
            CString headerValue;
            if ( !SplitHeaderField( (*i), headerName, headerValue ) )
            {
                // not a header field: skipped, as it always was
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
                ++info.contentTypeCount;
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
                ++info.hostCount;
            }
            else
            if ( headerName == "upgrade" )
            {
                request.isWebSocketUpgrade = ListHasToken( headerValue, "websocket" );
            }
            else
            {
                // a field that came more than once is one field with the values joined by commas (RFC 9110 5.3),
                // so that no value is lost and none hides another
                CHttpRequestData::TStringMap::iterator existing = request.otherHeaders.find( headerName );
                if ( existing == request.otherHeaders.end() )
                    request.otherHeaders[ headerName ] = headerValue;
                else
                    (*existing).second += ", " + headerValue;
            }
            ++i;
        }

        CHttpRequestData::TStringMap::const_iterator field = request.otherHeaders.find( "content-length" );
        if ( field != request.otherHeaders.end() )
        {
            info.hasContentLength = true;
            info.contentLengthValid = ParseContentLength( (*field).second, info.contentLength );
        }
        info.hasTransferEncoding = request.otherHeaders.end() != request.otherHeaders.find( "transfer-encoding" );
        field = request.otherHeaders.find( "expect" );
        if ( field != request.otherHeaders.end() )
        {
            info.hasExpect = true;
            info.expect = (*field).second;
        }
        field = request.otherHeaders.find( "connection" );
        if ( field != request.otherHeaders.end() )
        {
            if ( ListHasToken( (*field).second, "close" ) )
                info.keepAlive = false;
            else
            if ( ListHasToken( (*field).second, "keep-alive" ) )
                info.keepAlive = true;
            request.keepConnectionsAlive = info.keepAlive;
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPServer(" << CORE::PointerToString( this ) << "): Exception parsing request: " << e.what() );
        return false;
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "CHTTPServer(" << CORE::PointerToString( this ) << "): Finished parsing request header" );
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