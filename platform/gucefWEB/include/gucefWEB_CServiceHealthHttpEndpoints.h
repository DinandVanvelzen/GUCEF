/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#ifndef GUCEF_WEB_CSERVICEHEALTHHTTPENDPOINTS_H
#define GUCEF_WEB_CSERVICEHEALTHHTTPENDPOINTS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CSERVICEHEALTHREGISTRY_H
#include "gucefCORE_CServiceHealthRegistry.h"
#define GUCEF_CORE_CSERVICEHEALTHREGISTRY_H
#endif /* GUCEF_CORE_CSERVICEHEALTHREGISTRY_H ? */

#ifndef GUCEF_WEB_CIHTTPREQUESTTAKEOVERHANDLER_H
#include "gucefWEB_CIHttpRequestTakeoverHandler.h"
#define GUCEF_WEB_CIHTTPREQUESTTAKEOVERHANDLER_H
#endif /* GUCEF_WEB_CIHTTPREQUESTTAKEOVERHANDLER_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CHTTPServer;

/**
 *  Serves a CORE::CServiceHealthRegistry over a CHTTPServer for the probes of a container orchestrator:
 *
 *    /health/live      liveness: the pulse thread of the registry evaluated its checks recently
 *    /health/ready     readiness: started, every readiness check passes (WARN passes), the cache is fresh
 *    /health/started   startup: the service said it finished its start
 *    /info             what the service set with CServiceHealthRegistry::SetInfo()
 *
 *  The paths can be changed or left out (an empty path is not served). Each is an exact path taken over from the
 *  router of the server (CHTTPServer::RegisterRequestTakeoverHandler()), so the endpoints work next to any router.
 *  Status 200 when the probe passes, 503 when it fails or the cached evaluation is stale; HEAD gets the status
 *  without a body; other methods get 405. The answers read the cache of the registry only, they never run a check.
 *
 *  Health body: {"status":"pass|warn|fail","stale":true,"checks":[{"name":...,"status":...,"detail":...,
 *  "since":"<ISO 8601 UTC>"}]} ("stale" only when it is). The details of the checks pass CORE::CRedactor
 *  (ForLogLine()) before they are written. Info body: a JSON object of the info values as strings.
 *
 *  Which address the server binds to is the deployment's choice (CHttpServerSettings::bindAddress); in Kubernetes
 *  the probes reach the pod address and no Service needs to expose the port.
 *  The callbacks come on the thread of the server's pulse generator; the registry may be on another one.
 */
class GUCEF_WEB_PUBLIC_CPP CServiceHealthHttpEndpoints : public CIHttpRequestTakeoverHandler
{
    public:

    static const CORE::CString DefaultLivenessPath;     /**< "/health/live" */
    static const CORE::CString DefaultReadinessPath;    /**< "/health/ready" */
    static const CORE::CString DefaultStartupPath;      /**< "/health/started" */
    static const CORE::CString DefaultInfoPath;         /**< "/info" */

    /**
     *  Whether a health body carries the checks (their names, states, redacted details and times) or only the overall
     *  state. The endpoints answer clients that are not authenticated: the checks tell such a client which dependencies
     *  the service has and how they fail. The status code is the same either way, it is what probes judge.
     */
    enum ECheckDetails
    {
        CHECKDETAILS_AUTOMATIC = 0 ,    /**< the checks only when the server listens on a loopback address (127.x.x.x) */
        CHECKDETAILS_INCLUDED      ,    /**< the checks whatever the server listens on */
        CHECKDETAILS_OMITTED            /**< never the checks */
    };

    /** On the registry of the process, CCoreGlobal::GetServiceHealthRegistry() */
    CServiceHealthHttpEndpoints( void );

    /** On the given registry, which has to outlive this object */
    explicit CServiceHealthHttpEndpoints( CORE::CServiceHealthRegistry& registry );

    /** Takes the paths back from the server it was registered with */
    virtual ~CServiceHealthHttpEndpoints();

    /** Whether the health bodies carry the checks, CHECKDETAILS_AUTOMATIC by default */
    void SetCheckDetails( ECheckDetails checkDetails );

    ECheckDetails GetCheckDetails( void ) const;

    /**
     *  Whether a server with the given bind address (CHttpServerSettings::bindAddress) gets the checks in its health
     *  bodies. Automatic: only for a dotted IPv4 loopback address; an empty address and 0.0.0.0 listen on every interface
     */
    static bool IncludesCheckDetails( ECheckDetails checkDetails, const CORE::CString& bindAddress );

    /** The paths to serve; an empty path is not served. Only before RegisterWith() */
    void SetPaths( const CORE::CString& livenessPath  ,
                   const CORE::CString& readinessPath ,
                   const CORE::CString& startupPath   ,
                   const CORE::CString& infoPath      );

    /**
     *  Takes over the paths on the server, which has to outlive this object or see Unregister() first.
     *  False when one of the paths is taken already (nothing stays registered then) or the endpoints are
     *  registered with a server already
     */
    bool RegisterWith( CHTTPServer& server );

    /** Gives the paths back to the server */
    void Unregister( void );

    bool IsRegistered( void ) const;

    /** The JSON body for a health report with its checks */
    static CORE::CString BuildHealthBody( const CORE::CServiceHealthRegistry::SHealthReport& report );

    /** The JSON body for a health report, as the endpoints write it: without "checks" when the details are left out */
    static CORE::CString BuildHealthBody( const CORE::CServiceHealthRegistry::SHealthReport& report ,
                                          bool includeCheckDetails                                );

    /** The JSON body for the info values, as the endpoints write it */
    static CORE::CString BuildInfoBody( const CORE::CValueList& info );

    virtual EHeaderDecision OnHttpRequestHeaders( CHTTPServer& server                 ,
                                                  const CHttpRequestData& request     ,
                                                  const CHttpServerExchange& exchange ,
                                                  CHttpResponseData& response         ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnHttpRequest( CHTTPServer& server             ,
                                const CHttpRequestData& request ,
                                CHttpServerExchange& exchange   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnHttpExchangeClosed( CHTTPServer& server           ,
                                       CHttpServerExchange& exchange ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CServiceHealthHttpEndpoints( const CServiceHealthHttpEndpoints& src );            /**< not implemented */
    CServiceHealthHttpEndpoints& operator=( const CServiceHealthHttpEndpoints& src ); /**< not implemented */

    void UnregisterWhileLocked( void );

    private:

    mutable MT::CMutex m_lock;
    CORE::CServiceHealthRegistry* m_registry;
    CORE::CString m_livenessPath;
    CORE::CString m_readinessPath;
    CORE::CString m_startupPath;
    CORE::CString m_infoPath;
    ECheckDetails m_checkDetails;
    CHTTPServer* m_server;
    CORE::CString::StringVector m_registeredPaths;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CSERVICEHEALTHHTTPENDPOINTS_H ? */
