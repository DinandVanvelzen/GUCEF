/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H ? */

#ifndef GUCEF_WEB_CMCPSTATICBEARERAUTHENTICATOR_H
#include "gucefWEB_CMcpStaticBearerAuthenticator.h"
#define GUCEF_WEB_CMCPSTATICBEARERAUTHENTICATOR_H
#endif /* GUCEF_WEB_CMCPSTATICBEARERAUTHENTICATOR_H ? */

#include "gucefLabLink_PasswordHash.h"
#include "gucefLabLink_WebSite.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The pool the password hashes run in: of its own, so a burst of logins never holds up the calls of the nodes */
static const char* WebThreadPoolName = "gucefLabLink.web";

/** The largest body of a request: the forms of the site are small */
static const UInt32 MaxRequestBodyBytes = 4096;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A password job on its way to a thread of the pool, with the future it completes */
class CPasswordJobData : public CORE::CICloneable
{
    public:

    SPasswordJob job;
    CORE::CFutureResultSource source;

    CPasswordJobData( void ) : CORE::CICloneable(), job(), source() {}
    CPasswordJobData( const CPasswordJobData& src ) : CORE::CICloneable( src ), job( src.job ), source( src.source ) {}
    virtual ~CPasswordJobData() GUCEF_VIRTUAL_OVERRIDE { CPasswordHash::Wipe( job.password ); }

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CPasswordJobData( *this ); }
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return CloneableTypeName(); }
    static const char* CloneableTypeName( void ) { return "GUCEFLabLink::CPasswordJobData"; }

    private:

    CPasswordJobData& operator=( const CPasswordJobData& src );
};

/*-------------------------------------------------------------------------*/

/** What a password job gives back through its future: the job after it ran, without the password */
class CPasswordJobResult : public CORE::CICloneable
{
    public:

    SPasswordJob job;

    CPasswordJobResult( void ) : CORE::CICloneable(), job() {}
    CPasswordJobResult( const CPasswordJobResult& src ) : CORE::CICloneable( src ), job( src.job ) {}
    virtual ~CPasswordJobResult() GUCEF_VIRTUAL_OVERRIDE {}

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CPasswordJobResult( *this ); }
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return CloneableTypeName(); }
    static const char* CloneableTypeName( void ) { return "GUCEFLabLink::CPasswordJobResult"; }

    private:

    CPasswordJobResult& operator=( const CPasswordJobResult& src );
};

/*-------------------------------------------------------------------------*/

/** Runs a password job on a thread of the pool and completes its future */
class CPasswordJobTaskConsumer : public CORE::CTaskConsumer
{
    public:

    static const char* TaskType;

    CPasswordJobTaskConsumer( void ) : CORE::CTaskConsumer() {}
    virtual ~CPasswordJobTaskConsumer() GUCEF_VIRTUAL_OVERRIDE {}

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "GUCEFLabLink::CPasswordJobTaskConsumer";
        return classTypeName;
    }

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        CPasswordJobData* data = task.IsNULL() ? GUCEF_NULL : CORE::CICloneable::CloneableAs< CPasswordJobData >( task->GetTaskData() );
        if ( GUCEF_NULL == data || data->source.IsCompleted() )
            return true;
        CLabWebApi::RunPasswordJob( data->job );
        CPasswordJobResult* result = GUCEF_NEW CPasswordJobResult();
        result->job = data->job;
        data->source.Complete( result, true );
        return true;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE
    {
        // a task that ended without running (a pool that stops) still ends its future, and its password goes
        CPasswordJobData* data = task.IsNULL() ? GUCEF_NULL : CORE::CICloneable::CloneableAs< CPasswordJobData >( task->GetTaskData() );
        if ( GUCEF_NULL != data )
        {
            CPasswordHash::Wipe( data->job.password );
            if ( !data->source.IsCompleted() )
                data->source.Fail( CORE::TASKSTATUS_SETUP_FAILED, "the password job did not run" );
        }
        CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
    }

    private:

    CPasswordJobTaskConsumer( const CPasswordJobTaskConsumer& src );
    CPasswordJobTaskConsumer& operator=( const CPasswordJobTaskConsumer& src );
};

const char* CPasswordJobTaskConsumer::TaskType = "GUCEFLabLink::PasswordJob";

typedef CORE::CTFactory< CORE::CTaskConsumer, CPasswordJobTaskConsumer, MT::CMutex > TPasswordJobTaskConsumerFactory;

/** The factory of the pool: one for the process, registered while a site runs */
static TPasswordJobTaskConsumerFactory g_passwordJobFactory;
static UInt32 g_passwordJobFactoryUsers = 0;

/*-------------------------------------------------------------------------*/

/** The router of the HTTP server: the node paths with an id, and the paths the site does not have */
class CWebRouteHandler : public WEB::CIHttpServerRequestHandler
{
    public:

    CWebRouteHandler( void ) : WEB::CIHttpServerRequestHandler(), m_site( GUCEF_NULL ) {}
    virtual ~CWebRouteHandler() {}

    void SetSite( CLabWebSite* site ) { m_site = site; }

    virtual bool OnRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL == m_site )
        {
            response.statusCode = 503;
            return true;
        }
        m_site->OnRoutedRequest( request, response );
        return true;
    }

    virtual bool OnRead( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnReadMetaData( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnUpdate( const WEB::CHttpRequestData& request, bool isDeltaUpdateOnly, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnCreate( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnDelete( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool SetRouterController( WEB::CIHTTPServerRouterController* routerController ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual WEB::CIHTTPServerRouterController* GetRouterController( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    private:

    CLabWebSite* m_site;
};

/*-------------------------------------------------------------------------*/

/** Makes the router of the server, bound to the site */
class CWebRouteHandlerFactory : public CORE::CTFactory< WEB::CIHttpServerRequestHandler, CWebRouteHandler, MT::CMutex >
{
    public:

    typedef CORE::CTFactory< WEB::CIHttpServerRequestHandler, CWebRouteHandler, MT::CMutex > TBase;

    explicit CWebRouteHandlerFactory( CLabWebSite* site ) : TBase(), m_site( site ) {}
    virtual ~CWebRouteHandlerFactory() {}

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        TProductPtr product = TBase::Create();
        if ( !product.IsNULL() )
            static_cast< CWebRouteHandler* >( product.GetPointerAlways() )->SetSite( m_site );
        return product;
    }

    private:

    CLabWebSite* m_site;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLabWebSite::CLabWebSite( const SWebSettings& settings, const CAddressRanges& allowedRanges, ILabHubView& hub, CStore& store,
                          const CORE::CString& mcpDir, const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CObserver()
    , WEB::CIHttpRequestTakeoverHandler()
    , m_settings( settings )
    , m_allowedRanges( allowedRanges )
    , m_hub( hub )
    , m_store( store )
    , m_mcpDir( mcpDir )
    , m_pulseGenerator( pulseGenerator )
    , m_users( store )
    , m_api( GUCEF_NULL )
    , m_routeFactory( GUCEF_NULL )
    , m_server( GUCEF_NULL )
    , m_running()
    , m_waiting()
    , m_nextTimerInMs( 0 )
    , m_factoryRegistered( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLabWebSite::~CLabWebSite()
{GUCEF_TRACE;

    Stop();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLabWebSite::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEFLabLink::CLabWebSite";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

UInt16
CLabWebSite::GetPort( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_server ? m_server->GetBoundPort() : 0;
}

/*-------------------------------------------------------------------------*/

bool
CLabWebSite::IsTls( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_server && m_server->IsTlsEnabled();
}

/*-------------------------------------------------------------------------*/

CLabWebApi*
CLabWebSite::GetApi( void )
{GUCEF_TRACE;

    return m_api;
}

/*-------------------------------------------------------------------------*/

UInt32
CLabWebSite::GetPendingJobCount( void ) const
{GUCEF_TRACE;

    return (UInt32) ( m_running.size() + m_waiting.size() );
}

/*-------------------------------------------------------------------------*/

bool
CLabWebSite::Start( CORE::CString& problem )
{GUCEF_TRACE;

    Stop();

    // TLS with the certificate given, or the one the MCP services of this node have
    CORE::CString certificate = m_settings.certificateFile;
    CORE::CString key = m_settings.privateKeyFile;
    if ( certificate.IsNULLOrEmpty() && key.IsNULLOrEmpty() && !m_mcpDir.IsNULLOrEmpty() )
    {
        const CORE::CString defaultCertificate = CORE::CombinePath( m_mcpDir, "tls/tls.crt" );
        const CORE::CString defaultKey = CORE::CombinePath( m_mcpDir, "tls/tls.key" );
        if ( CORE::FileExists( defaultCertificate ) && CORE::FileExists( defaultKey ) )
        {
            certificate = defaultCertificate;
            key = defaultKey;
        }
    }
    if ( certificate.IsNULLOrEmpty() != key.IsNULLOrEmpty() )
    {
        problem = "the website needs both --web-cert and --web-key";
        return false;
    }
    const bool tls = !certificate.IsNULLOrEmpty();
    if ( !tls && !m_settings.allowPlainHttp )
    {
        problem = "the website needs a certificate and key (--web-cert, --web-key, or tls/tls.crt and tls/tls.key in " + m_mcpDir +
                  "), or --web-allow-plain-http to serve without TLS";
        return false;
    }

    // the bearer token: read, or made for the owner only when its file is not there yet
    CORE::CString token;
    if ( !m_settings.tokenFile.IsNULLOrEmpty() )
    {
        WEB::CMcpStaticBearerAuthenticator tokenReader;
        tokenReader.SetTokenFile( m_settings.tokenFile );
        if ( !tokenReader.Prepare( problem ) )
        {
            problem = "the token of the website: " + problem;
            return false;
        }
        token = tokenReader.GetToken();
    }

    if ( !m_users.Load( problem ) )
    {
        problem = "the users of the website cannot be read: " + problem;
        return false;
    }
    if ( !CPasswordHash::IsArgon2idAvailable() )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Web: the argon2id codec is not loaded; new passwords are hashed with PBKDF2-HMAC-SHA256" );

    m_api = GUCEF_NEW CLabWebApi( m_settings, m_allowedRanges, m_hub, m_users, tls, token );
    CPasswordHash::Wipe( token );

    // the pool of the hashes
    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( WebThreadPoolName, true );
    if ( pool.IsNULL() )
    {
        problem = "the thread pool of the website cannot be made";
        Stop();
        return false;
    }
    pool->SetDesiredMaxTotalNrOfThreads( (Int32) ( m_settings.maxConcurrentHashes < 1 ? 1 : m_settings.maxConcurrentHashes ) );
    if ( 0 == g_passwordJobFactoryUsers++ )
        pool->RegisterTaskConsumerFactory( CPasswordJobTaskConsumer::TaskType, &g_passwordJobFactory );
    m_factoryRegistered = true;

    WEB::CHttpServerSettings httpSettings;
    httpSettings.bindAddress = m_settings.listenAddress;
    httpSettings.maxClientConnections = m_settings.maxConnections;
    httpSettings.keepAliveConnections = true;
    httpSettings.processRequestsAsync = false;
    httpSettings.allowWebSocketUpgrades = false;
    httpSettings.maxBodyBytes = MaxRequestBodyBytes;
    if ( tls )
    {
        httpSettings.tls.SetCertificateChainPath( certificate );
        httpSettings.tls.SetPrivateKeyPath( key );
        httpSettings.tls.AddAlpnProtocol( "http/1.1" );
    }

    m_routeFactory = GUCEF_NEW CWebRouteHandlerFactory( this );
    m_server = GUCEF_NEW WEB::CHTTPServer( m_pulseGenerator, m_routeFactory, &httpSettings );
    static const char* exactPaths[] = { "/", "/index.html", "/app.js", "/app.css", "/api/v1/login", "/api/v1/logout", "/api/v1/session",
                                        "/api/v1/nodes", "/api/v1/endpoints", "/api/v1/users", "/api/v1/users/add", "/api/v1/users/disable",
                                        "/api/v1/users/enable" };
    for ( size_t i=0; i<sizeof( exactPaths ) / sizeof( exactPaths[ 0 ] ); ++i )
    {
        if ( !m_server->RegisterRequestTakeoverHandler( exactPaths[ i ], this, MaxRequestBodyBytes ) )
        {
            problem = CORE::CString( "the website cannot take over " ) + exactPaths[ i ];
            Stop();
            return false;
        }
    }
    if ( !m_server->ListenOnPort( m_settings.port ) || ( tls && !m_server->IsTlsEnabled() ) )
    {
        problem = "the website cannot listen on " + ( m_settings.listenAddress.IsNULLOrEmpty() ? CORE::CString( "0.0.0.0" ) : m_settings.listenAddress ) +
                  ":" + CORE::ToString( m_settings.port ) + ( tls ? CORE::CString( " (or the certificate " ) + certificate + " and key cannot be used)" : CORE::CString::Empty );
        Stop();
        return false;
    }
    m_api->SetListening( m_settings.listenAddress, m_server->GetBoundPort() );

    typedef CORE::CTEventHandlerFunctor< CLabWebSite > TEventCallback;
    TEventCallback pulseCallback( this, &CLabWebSite::OnPulse );
    SubscribeTo( m_pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, pulseCallback );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: The website listens on " << ( tls ? "https://" : "http://" ) <<
                      ( m_settings.listenAddress.IsNULLOrEmpty() ? CORE::CString( "0.0.0.0" ) : m_settings.listenAddress ) << ":" << m_server->GetBoundPort() <<
                      " for " << m_allowedRanges.Describe() << ", " << m_users.GetCount() << " user(s)" << ( m_settings.tokenFile.IsNULLOrEmpty() ? "" : " and the token" ) );
    if ( !tls )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: The website is served over plain HTTP (--web-allow-plain-http): passwords cross the network unencrypted" );
    if ( 0 == m_users.GetCount() && m_settings.tokenFile.IsNULLOrEmpty() )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Nobody can log in to the website yet: add a user with gucefLabLink --add-user NAME --role admin" );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLabWebSite::ReleasePending( void )
{GUCEF_TRACE;

    TPendingVector* lists[ 2 ] = { &m_running, &m_waiting };
    for ( int l=0; l<2; ++l )
    {
        for ( size_t i=0; i<lists[ l ]->size(); ++i )
        {
            SPending* pending = (*lists[ l ])[ i ];
            CPasswordHash::Wipe( pending->job.password );
            if ( GUCEF_NULL != pending->future )
                pending->future->RequestCancellation();
            GUCEF_DELETE pending->future;
            GUCEF_DELETE pending;
        }
        lists[ l ]->clear();
    }
}

/*-------------------------------------------------------------------------*/

void
CLabWebSite::Stop( void )
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    if ( GUCEF_NULL != m_server )
    {
        m_server->Close();
        GUCEF_DELETE m_server;
        m_server = GUCEF_NULL;
    }
    GUCEF_DELETE m_routeFactory;
    m_routeFactory = GUCEF_NULL;
    ReleasePending();
    GUCEF_DELETE m_api;
    m_api = GUCEF_NULL;
    if ( m_factoryRegistered )
    {
        m_factoryRegistered = false;
        if ( 0 == --g_passwordJobFactoryUsers )
        {
            CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool( WebThreadPoolName );
            if ( !pool.IsNULL() )
                pool->UnregisterTaskConsumerFactory( CPasswordJobTaskConsumer::TaskType );
        }
    }
}

/*-------------------------------------------------------------------------*/

WEB::CIHttpRequestTakeoverHandler::EHeaderDecision
CLabWebSite::OnHttpRequestHeaders( WEB::CHTTPServer& server                 ,
                                   const WEB::CHttpRequestData& request     ,
                                   const WEB::CHttpServerExchange& exchange ,
                                   WEB::CHttpResponseData& response         )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_api )
    {
        response.statusCode = 503;
        return TAKEOVER_REJECT;
    }
    const CORE::CString peer = exchange.GetRemoteAddress().GetFirstAddressAsString();
    return m_api->CheckHead( request, &peer, NowInMs(), response ) ? TAKEOVER_CONTINUE : TAKEOVER_REJECT;
}

/*-------------------------------------------------------------------------*/

void
CLabWebSite::OnHttpRequest( WEB::CHTTPServer& server             ,
                            const WEB::CHttpRequestData& request ,
                            WEB::CHttpServerExchange& exchange   )
{GUCEF_TRACE;

    WEB::CHttpResponseData response;
    if ( GUCEF_NULL == m_api )
    {
        response.statusCode = 503;
        exchange.SendResponse( response );
        return;
    }
    const CORE::CString peer = exchange.GetRemoteAddress().GetFirstAddressAsString();
    SPasswordJob job;
    if ( m_api->Handle( request, &peer, NowInMs(), response, job ) )
    {
        exchange.SendResponse( response );
        return;
    }
    Enqueue( job, exchange.GetSharedPtr() );
}

/*-------------------------------------------------------------------------*/

void
CLabWebSite::OnHttpExchangeClosed( WEB::CHTTPServer& server           ,
                                   WEB::CHttpServerExchange& exchange )
{GUCEF_TRACE;

    // a job of the exchange still finishes; its answer has nowhere to go and is dropped then
}

/*-------------------------------------------------------------------------*/

void
CLabWebSite::OnRoutedRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_api )
    {
        response.statusCode = 503;
        return;
    }
    // the router is not told the address of the client: the paths it gets need a session or the token
    SPasswordJob job;
    if ( !m_api->Handle( request, GUCEF_NULL, NowInMs(), response, job ) )
        m_api->RefusePasswordJob( job, response );
}

/*-------------------------------------------------------------------------*/

void
CLabWebSite::Enqueue( SPasswordJob& job, const WEB::CHttpServerExchangePtr& exchange )
{GUCEF_TRACE;

    if ( m_waiting.size() >= m_settings.maxQueuedHashes )
    {
        WEB::CHttpResponseData response;
        m_api->RefusePasswordJob( job, response );
        WEB::CHttpServerExchangePtr target = exchange;
        if ( !target.IsNULL() )
            target->SendResponse( response );
        return;
    }
    SPending* pending = GUCEF_NEW SPending();
    pending->job = job;
    pending->exchange = exchange;
    CPasswordHash::Wipe( job.password );
    m_waiting.push_back( pending );

    // the next one starts as soon as there is room
    while ( !m_waiting.empty() && m_running.size() < m_settings.maxConcurrentHashes )
    {
        SPending* next = m_waiting.front();
        m_waiting.erase( m_waiting.begin() );
        if ( Launch( *next ) )
            m_running.push_back( next );
        else
        {
            Finish( *next );
            GUCEF_DELETE next;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CLabWebSite::Launch( SPending& pending )
{GUCEF_TRACE;

    // a login whose client went away is not worth the work
    if ( SPasswordJob::JOB_LOGIN == pending.job.kind && ( pending.exchange.IsNULL() || !pending.exchange->IsOpen() ) )
    {
        CPasswordHash::Wipe( pending.job.password );
        return false;
    }
    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool( WebThreadPoolName );
    if ( pool.IsNULL() )
        return false;
    CPasswordJobData* data = GUCEF_NEW CPasswordJobData();
    data->job = pending.job;
    CPasswordHash::Wipe( pending.job.password );
    CORE::CFutureResult future = data->source.GetFuture();
    CORE::CFutureResult queued = pool->QueueTask( CPasswordJobTaskConsumer::TaskType, data, GUCEF_NULL, true );
    if ( queued.HasNoFuture() )
    {
        GUCEF_DELETE data;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: Web: a password job could not be queued on the pool " << WebThreadPoolName );
        return false;
    }
    pending.future = GUCEF_NEW CORE::CFutureResult( future );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLabWebSite::Finish( SPending& pending )
{GUCEF_TRACE;

    WEB::CHttpResponseData response;
    const CPasswordJobResult* result = GUCEF_NULL;
    CORE::CTaskPtr task;
    if ( GUCEF_NULL != pending.future && pending.future->IsReady() )
    {
        task = pending.future->GetResult( 0 );
        if ( !task.IsNULL() && CORE::TASKSTATUS_FINISHED == task->GetTaskStatus() )
            result = CORE::CICloneable::CloneableAs< CPasswordJobResult >( task->GetTaskData() );
    }
    if ( GUCEF_NULL != result )
    {
        SPasswordJob job = result->job;
        m_api->CompletePasswordJob( job, NowInMs(), response );
    }
    else
    {
        m_api->RefusePasswordJob( pending.job, response );
    }
    if ( !pending.exchange.IsNULL() && pending.exchange->IsOpen() )
        pending.exchange->SendResponse( response );
}

/*-------------------------------------------------------------------------*/

void
CLabWebSite::OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_api )
        return;
    if ( !m_running.empty() )
    {
        TPendingVector stillRunning;
        for ( size_t i=0; i<m_running.size(); ++i )
        {
            SPending* pending = m_running[ i ];
            if ( GUCEF_NULL != pending->future && !pending->future->IsReady() )
            {
                stillRunning.push_back( pending );
                continue;
            }
            Finish( *pending );
            GUCEF_DELETE pending->future;
            GUCEF_DELETE pending;
        }
        m_running = stillRunning;
        while ( !m_waiting.empty() && m_running.size() < m_settings.maxConcurrentHashes )
        {
            SPending* next = m_waiting.front();
            m_waiting.erase( m_waiting.begin() );
            if ( Launch( *next ) )
                m_running.push_back( next );
            else
            {
                Finish( *next );
                GUCEF_DELETE next;
            }
        }
    }

    const UInt64 now = NowInMs();
    if ( now >= m_nextTimerInMs )
    {
        m_nextTimerInMs = now + 1000;
        m_api->OnTimer( now );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
