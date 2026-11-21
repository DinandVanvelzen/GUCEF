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

#ifndef GUCEF_LABLINK_WEBSITE_H
#define GUCEF_LABLINK_WEBSITE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

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

#include "gucefLabLink_WebApi.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF { namespace WEB { class CHTTPServer; }; };

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CWebRouteHandlerFactory;

/**
 *  The website of the hub: a gucefWEB HTTP server (HTTPS with a certificate, plain HTTP only when that is allowed)
 *  on the pulse generator it is given, the application's, whose thread is the thread of every request. The exact
 *  paths of the site are taken over (CIHttpRequestTakeoverHandler), so a login can be answered later; the node
 *  paths with an id go through the router of the server. Password hashes run as tasks of the thread pool
 *  "gucefLabLink.web", never on the thread of the server, and at most maxConcurrentHashes at a time: the futures of
 *  the tasks are looked at on the pulses of the server, and the waiting logins are bounded.
 */
class CLabWebSite : public GUCEF::CORE::CObserver ,
                    public GUCEF::WEB::CIHttpRequestTakeoverHandler
{
    public:

    /**
     *  @param mcpDir the MCP directory of the cluster: tls/tls.crt and tls/tls.key in it are the certificate and key
     *                when none is given
     */
    CLabWebSite( const SWebSettings& settings, const CAddressRanges& allowedRanges, ILabHubView& hub, CStore& store,
                 const GUCEF::CORE::CString& mcpDir, const GUCEF::CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CLabWebSite() GUCEF_VIRTUAL_OVERRIDE;

    /** @return false when it cannot serve, problem says why */
    bool Start( GUCEF::CORE::CString& problem );

    void Stop( void );

    /** The port it listens on, 0 when it does not */
    GUCEF::UInt16 GetPort( void ) const;

    bool IsTls( void ) const;

    /** The API, for the tests */
    CLabWebApi* GetApi( void );

    /** Password hashes that run or wait */
    GUCEF::UInt32 GetPendingJobCount( void ) const;

    /** For the router of the server: a request to a path that is not taken over */
    void OnRoutedRequest( const GUCEF::WEB::CHttpRequestData& request, GUCEF::WEB::CHttpResponseData& response );

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

    struct SPending
    {
        SPasswordJob job;
        GUCEF::WEB::CHttpServerExchangePtr exchange;
        GUCEF::CORE::CFutureResult* future;     /**< NULL while it waits for its turn */

        SPending( void ) : job(), exchange(), future( GUCEF_NULL ) {}
    };
    typedef GUCEF::vector< SPending* > TPendingVector;

    CLabWebSite( const CLabWebSite& src );
    CLabWebSite& operator=( const CLabWebSite& src );

    void OnPulse( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData );
    void Enqueue( SPasswordJob& job, const GUCEF::WEB::CHttpServerExchangePtr& exchange );
    bool Launch( SPending& pending );
    void Finish( SPending& pending );
    void ReleasePending( void );

    SWebSettings m_settings;
    const CAddressRanges& m_allowedRanges;
    ILabHubView& m_hub;
    CStore& m_store;
    GUCEF::CORE::CString m_mcpDir;
    GUCEF::CORE::PulseGeneratorPtr m_pulseGenerator;
    CWebUserStore m_users;
    CLabWebApi* m_api;
    CWebRouteHandlerFactory* m_routeFactory;
    GUCEF::WEB::CHTTPServer* m_server;
    TPendingVector m_running;
    TPendingVector m_waiting;
    GUCEF::UInt64 m_nextTimerInMs;
    bool m_factoryRegistered;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_WEBSITE_H ? */
