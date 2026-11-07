/*
 *  gucefWEB: GUCEF module providing WEB application functionality
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#define GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <vector>

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETCLIENT_H
#include "gucefWEB_CWebSocketClient.h"
#define GUCEF_WEB_CWEBSOCKETCLIENT_H
#endif /* GUCEF_WEB_CWEBSOCKETCLIENT_H ? */

#ifndef GUCEF_WEB_CJSONRPCCLIENT_H
#include "gucefWEB_CJsonRpcClient.h"
#define GUCEF_WEB_CJSONRPCCLIENT_H
#endif /* GUCEF_WEB_CJSONRPCCLIENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What /json/version reports about the browser */
struct SChromeBrowserInfo
{
    CORE::CString browser;                  /**< for example Chrome/120.0.0.0 */
    CORE::CString protocolVersion;
    CORE::CString userAgent;
    CORE::CString webSocketDebuggerUrl;     /**< connect here to talk to the browser itself instead of a page */
};

/** A target as listed by /json/list: a page, a service worker, an extension background page... */
struct SChromeTargetInfo
{
    CORE::CString id;
    CORE::CString type;
    CORE::CString title;
    CORE::CString url;
    CORE::CString webSocketDebuggerUrl;
    CORE::CString devtoolsFrontendUrl;
};

struct SChromeDiscoveryResult
{
    SChromeBrowserInfo browser;
    std::vector< SChromeTargetInfo > targets;
};

struct SChromeCookie
{
    CORE::CString name;
    CORE::CString value;
    CORE::CString domain;
    CORE::CString path;
    Float64 expires;                        /**< seconds since the epoch, -1 for a session cookie */
    bool httpOnly;
    bool secure;
    bool session;
    CORE::CString sameSite;

    SChromeCookie( void ) : name(), value(), domain(), path(), expires( -1.0 ), httpOnly( false ), secure( false ), session( true ), sameSite() {}
};

/** A protocol event: something that happened in the browser, such as Page.loadEventFired */
struct SChromeProtocolEvent
{
    CORE::CString method;
    CORE::CDataNode params;
    bool hasParams;
    CORE::CString sessionId;                /**< the session of the target it is about, empty for the target we are connected to */

    SChromeProtocolEvent( void ) : method(), params(), hasParams( false ), sessionId() {}
};

typedef CORE::CTCloneableObj< SChromeDiscoveryResult >      TChromeDiscoveryCompletedEventData;
typedef CORE::CTCloneableObj< SChromeProtocolEvent >        TChromeProtocolEventData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Client for the Chrome DevTools Protocol (CDP), the interface with which Chrome and other Chromium based
 *  browsers can be inspected and driven.
 *
 *  Start the browser with --remote-debugging-port=<port>. Discovery lists its targets through the HTTP interface
 *  of that port, connecting attaches a web socket to a target (or to the browser) and from then on commands are
 *  JSON-RPC calls, see CJsonRpcClient, while the browser's events arrive as ProtocolEvents.
 *  Everything stays on plain ws:// and http:// connections to the local debugging port.
 *
 *  Events:
 *    DiscoveryCompletedEvent  TChromeDiscoveryCompletedEventData
 *    DiscoveryFailedEvent     CORE::TCloneableString
 *    ConnectedEvent           the web socket to the target is established
 *    DisconnectedEvent        the connection ended
 *    ProtocolEvent            TChromeProtocolEventData
 *
 *  The HTTP client, the web socket and the JSON-RPC client all run on the pulse generator of this client. The blocking
 *  (...AndWait) members pump it while they wait and are refused on any thread other than the one that drives it.
 *
 *  Not thread-safe. Requires the dstorepluginJSONPARSER and codecspluginSTBRUMMEHASH plugins.
 */
class GUCEF_WEB_PUBLIC_CPP CChromeDevToolsClient : public CORE::CNotifier
{
    public:

    static const CORE::CEvent DiscoveryCompletedEvent;
    static const CORE::CEvent DiscoveryFailedEvent;
    static const CORE::CEvent ConnectedEvent;
    static const CORE::CEvent DisconnectedEvent;
    static const CORE::CEvent ProtocolEvent;

    static void RegisterEvents( void );

    public:

    /** Uses the application's (global) pulse generator */
    CChromeDevToolsClient( void );

    /** Uses the given pulse generator for its connections and for waiting */
    explicit CChromeDevToolsClient( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CChromeDevToolsClient();

    CORE::PulseGeneratorPtr GetPulseGenerator( void ) const;

    /**
     *  Asks the browser listening on the given port for its version information and targets.
     *  Completes with DiscoveryCompletedEvent or DiscoveryFailedEvent
     */
    bool DiscoverTargets( const CORE::CString& host = "127.0.0.1", UInt16 port = 9222 );

    /** The blocking version of DiscoverTargets(), pumps the pulse generator of this client while it waits */
    bool DiscoverTargetsAndWait( const CORE::CString& host, UInt16 port, SChromeDiscoveryResult& result, UInt32 timeoutInMs = 10000 );

    /**
     *  Connects to the web socket debugger url of a target or of the browser, as given by discovery
     *  The connection is established when ConnectedEvent is sent
     */
    bool Connect( const CORE::CString& webSocketDebuggerUrl );

    /** The blocking version of Connect(), pumps the pulse generator of this client while it waits */
    bool ConnectAndWait( const CORE::CString& webSocketDebuggerUrl, UInt32 timeoutInMs = 10000 );

    void Disconnect( void );

    bool IsConnected( void ) const;

    /**
     *  Directs commands to a target which was attached to with AttachToTarget() over the connection
     *  to the browser. An empty id directs them to the target we are connected to.
     */
    void SetSessionId( const CORE::CString& sessionId );

    /** The JSON-RPC client commands go through, for access to anything this class does not wrap */
    CJsonRpcClient& GetRpcClient( void );

    /** Sends a command, the answer is delivered as a response event of the JSON-RPC client */
    bool SendCommand( const CORE::CString& method, const CORE::CDataNode* params, UInt64& commandId );

    /**
     *  Sends a command and waits for its result.
     *  @return true if the browser executed the command. If it did not the failure is described by 'error'
     */
    bool SendCommandAndWait( const CORE::CString& method   ,
                             const CORE::CDataNode* params ,
                             CORE::CDataNode& result       ,
                             COM::SJsonRpcError* error = GUCEF_NULL ,
                             UInt32 timeoutInMs = 30000    );

    // Commonly needed commands

    bool NetworkEnable( void );

    /** All cookies of the browser including those of other domains and HttpOnly ones, which is the way to get at a login session */
    bool GetAllCookies( std::vector< SChromeCookie >& cookies );

    /** The body of a response the browser received, requires NetworkEnable() before the request was made */
    bool GetResponseBody( const CORE::CString& requestId, CORE::CString& body, bool& isBase64Encoded );

    /**
     *  Evaluates JavaScript in the page and returns its result by value
     *  @param awaitPromise     wait for a promise the expression results in
     *  @param exceptionText    receives a description if the script threw
     */
    bool RuntimeEvaluate( const CORE::CString& expression ,
                          bool awaitPromise               ,
                          CORE::CDataNode& result         ,
                          CORE::CString* exceptionText = GUCEF_NULL );

    bool PageNavigate( const CORE::CString& url, CORE::CString* frameId = GUCEF_NULL );

    /** Opens a new tab, to be used over a connection to the browser */
    bool TargetCreate( const CORE::CString& url, CORE::CString& targetId );

    /** Attaches to a target over the connection to the browser so its commands can be sent with SetSessionId() */
    bool AttachToTarget( const CORE::CString& targetId, CORE::CString& sessionId );

    private:

    CChromeDevToolsClient( const CChromeDevToolsClient& src );
    CChromeDevToolsClient& operator=( const CChromeDevToolsClient& src );

    class CEventSink : public CORE::CObserver
    {
        public:
        explicit CEventSink( CChromeDevToolsClient& client ) : CORE::CObserver(), m_client( client ) {}

        /** Subscribes to the events of the clients of the owner that it reports on, one handler per notifier */
        void RegisterEventHandlers( void );

        private:

        typedef CORE::CTEventHandlerFunctor< CEventSink > TEventCallback;

        void OnHttpNotification( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData );

        void OnWebSocketNotification( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData );

        void OnRpcNotificationEvent( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData );

        CChromeDevToolsClient& m_client;
    };

    void OnHttpEvent( const CORE::CEvent& eventid, CORE::CICloneable* eventdata );
    void OnWebSocketEvent( const CORE::CEvent& eventid, CORE::CICloneable* eventdata );
    void OnRpcNotification( const COM::SJsonRpcNotification& notification );
    bool RequestNextDiscoveryDocument( void );
    void FinishDiscovery( bool success, const CORE::CString& failureReason );
    bool IsCallingThreadDriving( const char* operation ) const;
    void PumpPulses( UInt32 milliseconds );

    enum EDiscoveryPhase
    {
        DISCOVERY_IDLE        ,
        DISCOVERY_VERSION     ,
        DISCOVERY_TARGETS
    };

    private:

    CORE::PulseGeneratorPtr m_pulseGenerator;
    CHTTPClient m_httpClient;
    CWebSocketClient m_webSocketClient;
    CJsonRpcClient m_rpcClient;
    CEventSink m_eventSink;
    EDiscoveryPhase m_discoveryPhase;
    CORE::CString m_discoveryHost;
    UInt16 m_discoveryPort;
    std::string m_discoveryBody;
    SChromeDiscoveryResult m_discoveryResult;
    bool m_discoveryDone;
    bool m_discoverySucceeded;
    bool m_waitingForDiscovery;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H ? */
