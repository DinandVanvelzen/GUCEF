/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H ? */

#ifndef GUCEF_WEB_CMCPSTATICBEARERAUTHENTICATOR_H
#include "gucefWEB_CMcpStaticBearerAuthenticator.h"
#define GUCEF_WEB_CMCPSTATICBEARERAUTHENTICATOR_H
#endif /* GUCEF_WEB_CMCPSTATICBEARERAUTHENTICATOR_H ? */

#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CCiGateway.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CCiGateway::ThreadPoolName = "gucefMcpCi.gateway";

static const UInt32 BusyPulseIntervalInMs = 20;
static const UInt32 IdlePulseIntervalInMs = 1000;
static const UInt32 StopWaitInMs = 60000;
static const UInt32 BodyTimeoutInMs = 300000;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A request on its way to a thread of the pool, with the future it completes */
class CGatewayJobData : public CORE::CICloneable
{
    public:

    CGatewayApi::CRequest request;
    CGatewayApi* api;
    CORE::CFutureResultSource source;

    CGatewayJobData( void ) : CORE::CICloneable(), request(), api( GUCEF_NULL ), source() {}
    CGatewayJobData( const CGatewayJobData& src ) : CORE::CICloneable( src ), request( src.request ), api( src.api ), source( src.source ) {}
    virtual ~CGatewayJobData() GUCEF_VIRTUAL_OVERRIDE {}

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CGatewayJobData( *this ); }
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return CloneableTypeName(); }
    static const char* CloneableTypeName( void ) { return "GUCEFMcpCi::CGatewayJobData"; }

    private:

    CGatewayJobData& operator=( const CGatewayJobData& src );
};

/*-------------------------------------------------------------------------*/

/** What a request gives back through its future: the answer */
class CGatewayJobResult : public CORE::CICloneable
{
    public:

    CGatewayApi::CResponse response;

    CGatewayJobResult( void ) : CORE::CICloneable(), response() {}
    CGatewayJobResult( const CGatewayJobResult& src ) : CORE::CICloneable( src ), response( src.response ) {}
    virtual ~CGatewayJobResult() GUCEF_VIRTUAL_OVERRIDE {}

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CGatewayJobResult( *this ); }
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return CloneableTypeName(); }
    static const char* CloneableTypeName( void ) { return "GUCEFMcpCi::CGatewayJobResult"; }

    private:

    CGatewayJobResult& operator=( const CGatewayJobResult& src );
};

/*-------------------------------------------------------------------------*/

/** Runs a request on a thread of the pool and completes its future */
class CGatewayJobTaskConsumer : public CORE::CTaskConsumer
{
    public:

    static const char* TaskType;

    CGatewayJobTaskConsumer( void ) : CORE::CTaskConsumer() {}
    virtual ~CGatewayJobTaskConsumer() GUCEF_VIRTUAL_OVERRIDE {}

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "GUCEFMcpCi::CGatewayJobTaskConsumer";
        return classTypeName;
    }

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        CGatewayJobData* data = task.IsNULL() ? GUCEF_NULL : CORE::CICloneable::CloneableAs< CGatewayJobData >( task->GetTaskData() );
        if ( GUCEF_NULL == data || GUCEF_NULL == data->api || data->source.IsCompleted() )
            return true;
        CGatewayJobResult* result = GUCEF_NEW CGatewayJobResult();
        data->api->Execute( data->request, result->response );
        data->request.body.Clear();         // a chunk is not kept longer than needed
        data->source.Complete( result, true );
        return true;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE
    {
        // a task that ended without running (a pool that stops) still ends its future
        CGatewayJobData* data = task.IsNULL() ? GUCEF_NULL : CORE::CICloneable::CloneableAs< CGatewayJobData >( task->GetTaskData() );
        if ( GUCEF_NULL != data && !data->source.IsCompleted() )
            data->source.Fail( CORE::TASKSTATUS_SETUP_FAILED, "the request did not run" );
        CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
    }

    private:

    CGatewayJobTaskConsumer( const CGatewayJobTaskConsumer& src );
    CGatewayJobTaskConsumer& operator=( const CGatewayJobTaskConsumer& src );
};

const char* CGatewayJobTaskConsumer::TaskType = "GUCEFMcpCi::GatewayJob";

typedef CORE::CTFactory< CORE::CTaskConsumer, CGatewayJobTaskConsumer, MT::CMutex > TGatewayJobTaskConsumerFactory;

/** The factory of the pool: one for the process, registered while a gateway runs */
static TGatewayJobTaskConsumerFactory g_gatewayJobFactory;
static UInt32 g_gatewayJobFactoryUsers = 0;

/*-------------------------------------------------------------------------*/

/** The router of the HTTP server: every path the gateway has is taken over, so this answers the others */
class CGatewayRouteHandler : public WEB::CIHttpServerRequestHandler
{
    public:

    CGatewayRouteHandler( void ) : WEB::CIHttpServerRequestHandler() {}
    virtual ~CGatewayRouteHandler() {}

    virtual bool OnRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        static const char body[] = "{\"error\":\"no such call\",\"code\":\"not_found\"}";
        response.statusCode = 404;
        response.contentType = "application/json; charset=utf-8";
        response.content.CopyFrom( sizeof( body ) - 1, body );
        return true;
    }

    virtual bool OnRead( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnReadMetaData( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnUpdate( const WEB::CHttpRequestData& request, bool isDeltaUpdateOnly, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnCreate( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnDelete( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool SetRouterController( WEB::CIHTTPServerRouterController* routerController ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual WEB::CIHTTPServerRouterController* GetRouterController( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }
};

typedef CORE::CTFactory< WEB::CIHttpServerRequestHandler, CGatewayRouteHandler, MT::CMutex > TGatewayRouteHandlerFactory;

static TGatewayRouteHandlerFactory g_gatewayRouteFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
HeaderOf( const WEB::CHttpRequestData& request, const char* lowerCaseName )
{
    WEB::CHttpRequestData::TStringMap::const_iterator i = request.otherHeaders.find( lowerCaseName );
    return i == request.otherHeaders.end() ? CORE::CString::Empty : (*i).second;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCiGateway::CSettings::CSettings( void )
    : listenAddress()
    , port( 8792 )
    , certificateFile()
    , privateKeyFile()
    , allowedHosts()
    , maxConnections( 64 )
    , threads( 6 )
    , maxPending( 64 )
    , maxPendingPolls( 32 )
    , maxPendingChunks( 8 )
    , requestsPerMinute( 600 )
    , requestBurst( 120 )
    , pollIntervalInMs( 5000 )
    , housekeepingIntervalInMs( 15000 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCiGateway::CPending::CPending( void )
    : request()
    , exchange()
    , future( GUCEF_NULL )
    , isPoll( false )
    , isChunk( false )
    , nextCheckInMs( 0 )
    , receivedInMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCiGateway::CCiGateway( CGatewayApi& api, CGatewayTokens& tokens, CUploadStore& uploads, const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CObserver()
    , WEB::CIHttpRequestTakeoverHandler()
    , m_api( api )
    , m_tokens( tokens )
    , m_uploads( uploads )
    , m_pulseGenerator( pulseGenerator )
    , m_settings()
    , m_rateLimiter()
    , m_server( GUCEF_NULL )
    , m_running()
    , m_waiting()
    , m_parked()
    , m_nextHousekeepingInMs( 0 )
    , m_pulseIntervalInMs( 0 )
    , m_factoryRegistered( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCiGateway::~CCiGateway()
{GUCEF_TRACE;

    Stop();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CCiGateway::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEFMcpCi::CCiGateway";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

UInt16
CCiGateway::GetPort( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_server ? m_server->GetBoundPort() : 0;
}

/*-------------------------------------------------------------------------*/

bool
CCiGateway::IsTls( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_server && m_server->IsTlsEnabled();
}

/*-------------------------------------------------------------------------*/

bool
CCiGateway::Start( const CSettings& settings, CORE::CString& problem )
{GUCEF_TRACE;

    Stop();
    m_settings = settings;
    if ( m_settings.threads < 1 )
        m_settings.threads = 1;
    if ( m_settings.maxPending < m_settings.threads )
        m_settings.maxPending = m_settings.threads;
    if ( m_settings.pollIntervalInMs < 50 )
        m_settings.pollIntervalInMs = 50;
    m_rateLimiter.SetLimits( m_settings.requestsPerMinute, m_settings.requestBurst );

    const bool tls = !m_settings.certificateFile.IsNULLOrEmpty();
    if ( tls == m_settings.privateKeyFile.IsNULLOrEmpty() )
    {
        problem = "HTTPS needs both --tls-cert and --tls-key";
        return false;
    }

    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( ThreadPoolName, true );
    if ( pool.IsNULL() )
    {
        problem = "the thread pool of the gateway cannot be made";
        return false;
    }
    pool->SetDesiredMaxTotalNrOfThreads( (Int32) m_settings.threads );
    if ( 0 == g_gatewayJobFactoryUsers++ )
        pool->RegisterTaskConsumerFactory( CGatewayJobTaskConsumer::TaskType, &g_gatewayJobFactory );
    m_factoryRegistered = true;

    WEB::CHttpServerSettings httpSettings;
    httpSettings.bindAddress = m_settings.listenAddress;
    httpSettings.maxClientConnections = m_settings.maxConnections;
    httpSettings.keepAliveConnections = true;
    httpSettings.processRequestsAsync = false;
    httpSettings.allowWebSocketUpgrades = false;
    httpSettings.maxBodyBytes = CGatewayApi::MaxJsonBodyBytes;
    httpSettings.bodyTimeoutMs = BodyTimeoutInMs;
    if ( tls )
    {
        httpSettings.tls.SetCertificateChainPath( m_settings.certificateFile );
        httpSettings.tls.SetPrivateKeyPath( m_settings.privateKeyFile );
        httpSettings.tls.AddAlpnProtocol( "http/1.1" );
    }

    m_server = GUCEF_NEW WEB::CHTTPServer( m_pulseGenerator, &g_gatewayRouteFactory, &httpSettings );
    const CORE::CString::StringVector paths = CGatewayApi::GetPaths();
    for ( size_t i=0; i<paths.size(); ++i )
    {
        // the chunk route takes the largest body; the checks of the headers hold every other method to less
        const UInt32 maxBody = paths[ i ] == "/v1/uploads" ? CUploadStore::MaxChunkBytes : CGatewayApi::MaxJsonBodyBytes;
        if ( !m_server->RegisterRequestTakeoverHandler( paths[ i ], this, maxBody ) )
        {
            problem = "the gateway cannot take over " + paths[ i ];
            Stop();
            return false;
        }
    }
    if ( !m_server->ListenOnPort( m_settings.port ) || ( tls && !m_server->IsTlsEnabled() ) )
    {
        problem = "the gateway cannot listen on " + ( m_settings.listenAddress.IsNULLOrEmpty() ? CORE::CString( "0.0.0.0" ) : m_settings.listenAddress ) +
                  ":" + CORE::ToString( m_settings.port ) + ( tls ? CORE::CString( " (or the certificate " ) + m_settings.certificateFile + " and key cannot be used)" : CORE::CString::Empty );
        Stop();
        return false;
    }

    typedef CORE::CTEventHandlerFunctor< CCiGateway > TEventCallback;
    TEventCallback pulseCallback( this, &CCiGateway::OnPulse );
    SubscribeTo( m_pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, pulseCallback );
    if ( m_settings.housekeepingIntervalInMs < 200 )
        m_settings.housekeepingIntervalInMs = 200;
    m_nextHousekeepingInMs = CCiUtils::MonotonicMs() + m_settings.housekeepingIntervalInMs;
    UpdatePulses();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: The gateway listens on " << ( tls ? "https://" : "http://" ) <<
                      ( m_settings.listenAddress.IsNULLOrEmpty() ? CORE::CString( "0.0.0.0" ) : m_settings.listenAddress ) << ":" << m_server->GetBoundPort() <<
                      " with " << m_settings.threads << " thread(s), " << m_tokens.GetMachineCount() << " machine(s)" <<
                      ( m_tokens.HasStaticToken() ? " and the static token" : "" ) );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCiGateway::ReleasePending( void )
{GUCEF_TRACE;

    // the tasks that run use the API: they are waited for, at most a while
    const UInt64 deadline = CCiUtils::MonotonicMs() + StopWaitInMs;
    for ( size_t i=0; i<m_running.size(); ++i )
    {
        CPending* pending = m_running[ i ];
        if ( GUCEF_NULL != pending->future )
        {
            while ( !pending->future->IsReady() && CCiUtils::MonotonicMs() < deadline )
                MT::ThreadDelay( 10 );
            if ( !pending->future->IsReady() )
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: Gateway: A request still runs while the gateway stops: " << pending->request.path );
        }
    }
    TPendingVector* lists[ 3 ] = { &m_running, &m_waiting, &m_parked };
    for ( int l=0; l<3; ++l )
    {
        for ( size_t i=0; i<lists[ l ]->size(); ++i )
        {
            CPending* pending = (*lists[ l ])[ i ];
            if ( GUCEF_NULL != pending->future && !pending->future->IsReady() )
                pending->future->RequestCancellation();
            GUCEF_DELETE pending->future;
            GUCEF_DELETE pending;
        }
        lists[ l ]->clear();
    }
}

/*-------------------------------------------------------------------------*/

void
CCiGateway::Stop( void )
{GUCEF_TRACE;

    if ( !m_pulseGenerator.IsNULL() )
        m_pulseGenerator->RequestStopOfPeriodicUpdates( this );
    m_pulseIntervalInMs = 0;
    UnsubscribeAllFromObserver();
    if ( GUCEF_NULL != m_server )
    {
        m_server->Close();
        GUCEF_DELETE m_server;
        m_server = GUCEF_NULL;
    }
    ReleasePending();
    if ( m_factoryRegistered )
    {
        m_factoryRegistered = false;
        if ( 0 == --g_gatewayJobFactoryUsers )
        {
            CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool( ThreadPoolName );
            if ( !pool.IsNULL() )
                pool->UnregisterTaskConsumerFactory( CGatewayJobTaskConsumer::TaskType );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CCiGateway::UpdatePulses( void )
{GUCEF_TRACE;

    // fast while something is pending, slow for the housekeeping otherwise: idle the gateway costs next to nothing
    const UInt32 wanted = ( m_running.empty() && m_waiting.empty() && m_parked.empty() ) ? IdlePulseIntervalInMs : BusyPulseIntervalInMs;
    if ( wanted == m_pulseIntervalInMs || m_pulseGenerator.IsNULL() )
        return;
    m_pulseIntervalInMs = wanted;
    m_pulseGenerator->RequestPeriodicPulses( this, wanted );
}

/*-------------------------------------------------------------------------*/

bool
CCiGateway::IsHostAllowed( const WEB::CHttpRequestData& request ) const
{GUCEF_TRACE;

    const CORE::CString host = request.GetHostName().Lowercase();
    if ( host == "localhost" || host == "127.0.0.1" || host == "::1" )
        return true;
    for ( size_t i=0; i<m_settings.allowedHosts.size(); ++i )
        if ( host == m_settings.allowedHosts[ i ].Lowercase() )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CCiGateway::CountPolls( void ) const
{GUCEF_TRACE;

    UInt32 polls = (UInt32) m_parked.size();
    for ( size_t i=0; i<m_running.size(); ++i )
        if ( m_running[ i ]->isPoll )
            ++polls;
    for ( size_t i=0; i<m_waiting.size(); ++i )
        if ( m_waiting[ i ]->isPoll )
            ++polls;
    return polls;
}

/*-------------------------------------------------------------------------*/

UInt32
CCiGateway::CountChunks( void ) const
{GUCEF_TRACE;

    UInt32 chunks = 0;
    for ( size_t i=0; i<m_running.size(); ++i )
        if ( m_running[ i ]->isChunk )
            ++chunks;
    for ( size_t i=0; i<m_waiting.size(); ++i )
        if ( m_waiting[ i ]->isChunk )
            ++chunks;
    return chunks;
}

/*-------------------------------------------------------------------------*/

void
CCiGateway::FillResponse( WEB::CHttpResponseData& response, const CGatewayApi::CResponse& answer )
{GUCEF_TRACE;

    response.statusCode = answer.status;
    response.contentType = "application/json; charset=utf-8";
    response.otherHeaders[ "Cache-Control" ] = "no-store";
    response.otherHeaders[ "X-Content-Type-Options" ] = "nosniff";
    if ( answer.body.ByteSize() > 1 )
        response.content.CopyFrom( answer.body.ByteSize() - 1, answer.body.C_String() );
}

/*-------------------------------------------------------------------------*/

void
CCiGateway::SendError( WEB::CHttpServerExchange& exchange, Int32 status, const char* code, const CORE::CString& message )
{GUCEF_TRACE;

    CGatewayApi::CResponse answer;
    CGatewayApi::SetError( answer, status, code, message );
    WEB::CHttpResponseData response;
    FillResponse( response, answer );
    if ( 503 == status )
        response.otherHeaders[ "Retry-After" ] = "2";
    exchange.SendResponse( response );
}

/*-------------------------------------------------------------------------*/

WEB::CIHttpRequestTakeoverHandler::EHeaderDecision
CCiGateway::OnHttpRequestHeaders( WEB::CHTTPServer& server                 ,
                                  const WEB::CHttpRequestData& request     ,
                                  const WEB::CHttpServerExchange& exchange ,
                                  WEB::CHttpResponseData& response         )
{GUCEF_TRACE;

    CGatewayApi::CResponse answer;
    UInt32 maxBody = 0;
    bool pathKnown = false;
    if ( !CGatewayApi::FindRoute( request.requestType, request.requestUri, maxBody, pathKnown ) )
    {
        CGatewayApi::SetError( answer, 405, "method", "the method " + request.requestType + " is not one of " + request.requestUri );
        FillResponse( response, answer );
        return TAKEOVER_REJECT;
    }
    if ( !IsHostAllowed( request ) )
    {
        CGatewayApi::SetError( answer, 421, "host", "this gateway does not serve the host name \"" + request.GetHostName() + "\" (--allowed-host)" );
        FillResponse( response, answer );
        return TAKEOVER_REJECT;
    }
    if ( WEB::CMcpStaticBearerAuthenticator::HasTokenInQuery( request.requestUriParams ) )
    {
        CGatewayApi::SetError( answer, 400, "token_in_query", "the token must not be in the query" );
        FillResponse( response, answer );
        return TAKEOVER_REJECT;
    }
    CORE::CString machineId;
    if ( !m_tokens.Authenticate( HeaderOf( request, "authorization" ), machineId ) )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "gucefMcpCi: Gateway: Refused " << request.requestType << " " << request.requestUri << " from " <<
                          exchange.GetRemoteAddress().GetFirstAddressAsString() << ": no accepted token" );
        CGatewayApi::SetError( answer, 401, "token", "an accepted bearer token is needed" );
        FillResponse( response, answer );
        response.otherHeaders[ "WWW-Authenticate" ] = "Bearer";
        return TAKEOVER_REJECT;
    }
    UInt32 retryAfter = 0;
    if ( !m_rateLimiter.Allow( machineId, CCiUtils::MonotonicMs(), retryAfter ) )
    {
        CGatewayApi::SetError( answer, 429, "rate", "too many requests from " + machineId + "; try again in " + CORE::UInt32ToString( retryAfter ) + " s" );
        FillResponse( response, answer );
        response.otherHeaders[ "Retry-After" ] = CORE::UInt32ToString( retryAfter );
        return TAKEOVER_REJECT;
    }
    const UInt64 length = CORE::StringToUInt64( HeaderOf( request, "content-length" ), 0 );
    if ( length > (UInt64) maxBody )
    {
        CGatewayApi::SetError( answer, 413, "too_large", "the body is larger than this call takes (" + CORE::UInt32ToString( maxBody ) + " bytes)" );
        FillResponse( response, answer );
        return TAKEOVER_REJECT;
    }
    return TAKEOVER_CONTINUE;
}

/*-------------------------------------------------------------------------*/

void
CCiGateway::OnHttpRequest( WEB::CHTTPServer& server             ,
                           const WEB::CHttpRequestData& request ,
                           WEB::CHttpServerExchange& exchange   )
{GUCEF_TRACE;

    CORE::CString machineId;
    if ( !m_tokens.Authenticate( HeaderOf( request, "authorization" ), machineId ) )
    {
        SendError( exchange, 401, "token", "an accepted bearer token is needed" );
        return;
    }

    // bounded: what waits or runs, and the waiting statuses
    const bool isPoll = request.requestType == "GET" && request.requestUri == "/v1/runs";
    CGatewayApi::CRequest::TParameterMap parameters;
    CGatewayApi::ParseQuery( request.requestUriParams, parameters );
    UInt32 waitSeconds = 0;
    if ( isPoll )
    {
        CGatewayApi::CRequest::TParameterMap::const_iterator wait = parameters.find( "wait" );
        if ( wait != parameters.end() )
            waitSeconds = CORE::StringToUInt32( (*wait).second, 0 );
        if ( waitSeconds > m_api.GetMaxWaitSeconds() )
            waitSeconds = m_api.GetMaxWaitSeconds();
    }
    if ( m_running.size() + m_waiting.size() >= m_settings.maxPending )
    {
        SendError( exchange, 503, "busy", "the gateway has " + CORE::UInt32ToString( m_settings.maxPending ) + " requests in hand; try again shortly" );
        return;
    }
    if ( waitSeconds > 0 && CountPolls() >= m_settings.maxPendingPolls )
    {
        SendError( exchange, 503, "polls_full", "the gateway has " + CORE::UInt32ToString( m_settings.maxPendingPolls ) + " waiting status requests; try again shortly" );
        return;
    }

    const bool isChunk = request.requestType == "PUT";
    if ( isChunk && CountChunks() >= m_settings.maxPendingChunks )
    {
        SendError( exchange, 503, "busy", "the gateway has " + CORE::UInt32ToString( m_settings.maxPendingChunks ) + " chunks in hand; try again shortly" );
        return;
    }

    CPending* pending = GUCEF_NEW CPending();
    pending->isChunk = isChunk;
    pending->request.method = request.requestType;
    pending->request.path = request.requestUri;
    pending->request.parameters = parameters;
    pending->request.body = request.content;
    pending->request.machineId = machineId;
    pending->exchange = exchange.GetSharedPtr();
    pending->receivedInMs = CCiUtils::MonotonicMs();
    if ( waitSeconds > 0 )
    {
        pending->isPoll = true;
        pending->request.pollDeadlineInMs = pending->receivedInMs + (UInt64) waitSeconds * 1000;
    }
    m_waiting.push_back( pending );
    LaunchWaiting();
    UpdatePulses();
}

/*-------------------------------------------------------------------------*/

void
CCiGateway::OnHttpExchangeClosed( WEB::CHTTPServer& server           ,
                                  WEB::CHttpServerExchange& exchange )
{GUCEF_TRACE;

    // a client that went away: its parked status is dropped, a task of it still finishes and its answer is dropped
    TPendingVector stillParked;
    for ( size_t i=0; i<m_parked.size(); ++i )
    {
        CPending* pending = m_parked[ i ];
        if ( !pending->exchange.IsNULL() && pending->exchange->GetExchangeId() == exchange.GetExchangeId() )
        {
            GUCEF_DELETE pending->future;
            GUCEF_DELETE pending;
            continue;
        }
        stillParked.push_back( pending );
    }
    m_parked = stillParked;
}

/*-------------------------------------------------------------------------*/

void
CCiGateway::LaunchWaiting( void )
{GUCEF_TRACE;

    while ( !m_waiting.empty() && m_running.size() < m_settings.threads )
    {
        CPending* next = m_waiting.front();
        m_waiting.erase( m_waiting.begin() );
        if ( next->exchange.IsNULL() || !next->exchange->IsOpen() )
        {
            GUCEF_DELETE next->future;
            GUCEF_DELETE next;
            continue;
        }
        if ( Launch( *next ) )
        {
            m_running.push_back( next );
            continue;
        }
        SendError( *next->exchange, 503, "busy", "the gateway could not start the request" );
        GUCEF_DELETE next;
    }
}

/*-------------------------------------------------------------------------*/

bool
CCiGateway::Launch( CPending& pending )
{GUCEF_TRACE;

    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool( ThreadPoolName );
    if ( pool.IsNULL() )
        return false;
    CGatewayJobData* data = GUCEF_NEW CGatewayJobData();
    data->request = pending.request;
    data->api = &m_api;
    CORE::CFutureResult future = data->source.GetFuture();
    CORE::CFutureResult queued = pool->QueueTask( CGatewayJobTaskConsumer::TaskType, data, GUCEF_NULL, true );
    if ( queued.HasNoFuture() )
    {
        GUCEF_DELETE data;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefMcpCi: Gateway: A request could not be queued on the pool " << ThreadPoolName );
        return false;
    }
    pending.request.body.Clear();       // the task has its own copy
    GUCEF_DELETE pending.future;
    pending.future = GUCEF_NEW CORE::CFutureResult( future );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCiGateway::Answer( CPending& pending, const CGatewayApi::CResponse& answer )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "gucefMcpCi: Gateway: " << pending.request.machineId << " " << pending.request.method << " " << pending.request.path <<
                      " -> " << answer.status << " in " << ( CCiUtils::MonotonicMs() - pending.receivedInMs ) << " ms" );
    if ( pending.exchange.IsNULL() || !pending.exchange->IsOpen() )
        return;
    WEB::CHttpResponseData response;
    FillResponse( response, answer );
    if ( 503 == answer.status )
        response.otherHeaders[ "Retry-After" ] = "2";
    pending.exchange->SendResponse( response );
}

/*-------------------------------------------------------------------------*/

bool
CCiGateway::Finish( CPending* pending )
{GUCEF_TRACE;

    const CGatewayJobResult* result = GUCEF_NULL;
    CORE::CTaskPtr task;
    if ( GUCEF_NULL != pending->future && pending->future->IsReady() )
    {
        task = pending->future->GetResult( 0 );
        if ( !task.IsNULL() && CORE::TASKSTATUS_FINISHED == task->GetTaskStatus() )
            result = CORE::CICloneable::CloneableAs< CGatewayJobResult >( task->GetTaskData() );
    }
    GUCEF_DELETE pending->future;
    pending->future = GUCEF_NULL;
    if ( GUCEF_NULL == result )
    {
        CGatewayApi::CResponse answer;
        CGatewayApi::SetError( answer, 503, "busy", "the gateway could not carry out the request" );
        Answer( *pending, answer );
        return true;
    }

    // a status that waits for the run: asked again after the poll interval, while the client is there
    const UInt64 now = CCiUtils::MonotonicMs();
    if ( result->response.pollAgain && pending->isPoll && now < pending->request.pollDeadlineInMs &&
         !pending->exchange.IsNULL() && pending->exchange->IsOpen() )
    {
        const UInt64 next = now + m_settings.pollIntervalInMs;
        pending->nextCheckInMs = next < pending->request.pollDeadlineInMs ? next : pending->request.pollDeadlineInMs;
        return false;
    }
    Answer( *pending, result->response );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCiGateway::OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_server )
        return;
    const UInt64 now = CCiUtils::MonotonicMs();

    if ( !m_running.empty() )
    {
        TPendingVector stillRunning;
        for ( size_t i=0; i<m_running.size(); ++i )
        {
            CPending* pending = m_running[ i ];
            if ( GUCEF_NULL != pending->future && !pending->future->IsReady() )
            {
                stillRunning.push_back( pending );
                continue;
            }
            if ( Finish( pending ) )
                GUCEF_DELETE pending;
            else
                m_parked.push_back( pending );
        }
        m_running = stillRunning;
    }

    // parked statuses whose time came are asked again, at the end of the line
    if ( !m_parked.empty() )
    {
        TPendingVector stillParked;
        for ( size_t i=0; i<m_parked.size(); ++i )
        {
            CPending* pending = m_parked[ i ];
            if ( pending->exchange.IsNULL() || !pending->exchange->IsOpen() )
            {
                GUCEF_DELETE pending;
                continue;
            }
            if ( now >= pending->nextCheckInMs )
                m_waiting.push_back( pending );
            else
                stillParked.push_back( pending );
        }
        m_parked = stillParked;
    }
    LaunchWaiting();

    if ( now >= m_nextHousekeepingInMs )
    {
        m_nextHousekeepingInMs = now + m_settings.housekeepingIntervalInMs;
        m_uploads.RemoveAbandoned( now );
        m_api.RemoveExpiredSnapshots( now );
    }
    UpdatePulses();
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
