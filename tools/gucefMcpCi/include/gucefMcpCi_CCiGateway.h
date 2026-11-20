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

#ifndef GUCEF_MCPCI_CCIGATEWAY_H
#define GUCEF_MCPCI_CCIGATEWAY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_WEB_CIHTTPREQUESTTAKEOVERHANDLER_H
#include "gucefWEB_CIHttpRequestTakeoverHandler.h"
#define GUCEF_WEB_CIHTTPREQUESTTAKEOVERHANDLER_H
#endif /* GUCEF_WEB_CIHTTPREQUESTTAKEOVERHANDLER_H ? */

#ifndef GUCEF_WEB_CHTTPSERVEREXCHANGE_H
#include "gucefWEB_CHttpServerExchange.h"
#define GUCEF_WEB_CHTTPSERVEREXCHANGE_H
#endif /* GUCEF_WEB_CHTTPSERVEREXCHANGE_H ? */

#include "gucefMcpCi_CGatewayApi.h"
#include "gucefMcpCi_CGatewayTokens.h"
#include "gucefMcpCi_CRequestRateLimiter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF { namespace WEB { class CHTTPServer; }; };

namespace GUCEFMcpCi {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The gateway: the gucefWEB HTTP server of mode gateway, HTTPS with the certificate of the machine (plain HTTP only
 *  on a loopback address), on the pulse generator it is given, the application's, whose thread is the thread of every
 *  callback of the server. The exact paths of CGatewayApi are taken over (CIHttpRequestTakeoverHandler):
 *
 *    - before a body is read: the method and path, the Host (one of the allowed host names), the bearer token
 *      (CGatewayTokens), the request rate of the machine of the token and the size of the body; refused requests
 *      never make the server buffer a body
 *    - every request then runs as a task of the thread pool "gucefMcpCi.gateway" (CGatewayApi::Execute()): git, the
 *      backend and the disk are never used on the thread of the server. The futures of the tasks are looked at on the
 *      pulses of the server, and the exchange is answered when its task is done
 *    - a status with wait is asked again every poll interval until the run is finished or the wait is over, as a
 *      new short task each time: a waiting client holds no thread
 *    - bounded: at most 'threads' tasks run, 'maxPending' wait or run, 'maxPendingPolls' statuses wait,
 *      'maxPendingChunks' chunks are in hand, and the upload store bounds the uploads; beyond that the answer is 503
 *
 *  Abandoned uploads and old sent snapshots are removed on a timer.
 */
class CCiGateway : public GUCEF::CORE::CObserver ,
                   public GUCEF::WEB::CIHttpRequestTakeoverHandler
{
    public:

    class CSettings
    {
        public:

        GUCEF::CORE::CString listenAddress;
        GUCEF::UInt16 port;
        GUCEF::CORE::CString certificateFile;               /**< with the key: HTTPS */
        GUCEF::CORE::CString privateKeyFile;
        GUCEF::CORE::CString::StringVector allowedHosts;    /**< names clients use for this machine, besides localhost and 127.0.0.1 */
        GUCEF::UInt32 maxConnections;
        GUCEF::UInt32 threads;
        GUCEF::UInt32 maxPending;
        GUCEF::UInt32 maxPendingPolls;
        GUCEF::UInt32 maxPendingChunks;                 /**< chunks of uploads in hand, each up to 8 MiB of memory */
        GUCEF::UInt32 requestsPerMinute;
        GUCEF::UInt32 requestBurst;
        GUCEF::UInt32 pollIntervalInMs;
        GUCEF::UInt32 housekeepingIntervalInMs;         /**< how often abandoned uploads are looked for */

        CSettings( void );
    };

    /** The thread pool of the tasks */
    static const char* ThreadPoolName;

    /** The API, the tokens and the uploads must outlive the gateway */
    CCiGateway( CGatewayApi& api, CGatewayTokens& tokens, CUploadStore& uploads, const GUCEF::CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CCiGateway() GUCEF_VIRTUAL_OVERRIDE;

    /** @return false when it cannot serve, problem says why */
    bool Start( const CSettings& settings, GUCEF::CORE::CString& problem );

    /** Stops serving; waits for the tasks that run, at most a while */
    void Stop( void );

    /** The port it listens on, 0 when it does not */
    GUCEF::UInt16 GetPort( void ) const;

    bool IsTls( void ) const;

    virtual EHeaderDecision OnHttpRequestHeaders( GUCEF::WEB::CHTTPServer& server                 ,
                                                  const GUCEF::WEB::CHttpRequestData& request     ,
                                                  const GUCEF::WEB::CHttpServerExchange& exchange ,
                                                  GUCEF::WEB::CHttpResponseData& response         ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnHttpRequest( GUCEF::WEB::CHTTPServer& server             ,
                                const GUCEF::WEB::CHttpRequestData& request ,
                                GUCEF::WEB::CHttpServerExchange& exchange   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnHttpExchangeClosed( GUCEF::WEB::CHTTPServer& server           ,
                                       GUCEF::WEB::CHttpServerExchange& exchange ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    class CPending
    {
        public:

        CGatewayApi::CRequest request;
        GUCEF::WEB::CHttpServerExchangePtr exchange;
        GUCEF::CORE::CFutureResult* future;     /**< GUCEF_NULL while it waits for a thread */
        bool isPoll;
        bool isChunk;
        GUCEF::UInt64 nextCheckInMs;            /**< a parked poll: when it is asked again */
        GUCEF::UInt64 receivedInMs;

        CPending( void );
    };
    typedef GUCEF::vector< CPending* > TPendingVector;

    CCiGateway( const CCiGateway& src );                /**< not implemented */
    CCiGateway& operator=( const CCiGateway& src );     /**< not implemented */

    void OnPulse( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData );

    /** Starts what waits while there are threads; with false when a pending one could not start */
    void LaunchWaiting( void );

    bool Launch( CPending& pending );

    /** The answer of a task that is done; true when it was answered, false when it was parked as a poll */
    bool Finish( CPending* pending );

    void Answer( CPending& pending, const CGatewayApi::CResponse& response );

    void SendError( GUCEF::WEB::CHttpServerExchange& exchange, GUCEF::Int32 status, const char* code, const GUCEF::CORE::CString& message );

    static void FillResponse( GUCEF::WEB::CHttpResponseData& response, const CGatewayApi::CResponse& answer );

    bool IsHostAllowed( const GUCEF::WEB::CHttpRequestData& request ) const;

    GUCEF::UInt32 CountPolls( void ) const;

    GUCEF::UInt32 CountChunks( void ) const;

    void UpdatePulses( void );

    void ReleasePending( void );

    CGatewayApi& m_api;
    CGatewayTokens& m_tokens;
    CUploadStore& m_uploads;
    GUCEF::CORE::PulseGeneratorPtr m_pulseGenerator;
    CSettings m_settings;
    CRequestRateLimiter m_rateLimiter;
    GUCEF::WEB::CHTTPServer* m_server;
    TPendingVector m_running;
    TPendingVector m_waiting;
    TPendingVector m_parked;
    GUCEF::UInt64 m_nextHousekeepingInMs;
    GUCEF::UInt32 m_pulseIntervalInMs;
    bool m_factoryRegistered;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CCIGATEWAY_H ? */
