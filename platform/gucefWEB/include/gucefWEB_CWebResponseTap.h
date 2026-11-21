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

#ifndef GUCEF_WEB_CWEBRESPONSETAP_H
#define GUCEF_WEB_CWEBRESPONSETAP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#include "gucefWEB_CChromeDevToolsClient.h"
#define GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#endif /* GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H ? */

#ifndef GUCEF_WEB_CIWEBRESPONSESINK_H
#include "gucefWEB_CIWebResponseSink.h"
#define GUCEF_WEB_CIWEBRESPONSESINK_H
#endif /* GUCEF_WEB_CIWEBRESPONSESINK_H ? */

#ifndef GUCEF_WEB_CSITEDISCOVERYREDACTOR_H
#include "gucefWEB_CSiteDiscoveryRedactor.h"
#define GUCEF_WEB_CSITEDISCOVERYREDACTOR_H
#endif /* GUCEF_WEB_CSITEDISCOVERYREDACTOR_H ? */

#ifndef GUCEF_WEB_CURLPATHTEMPLATE_H
#include "gucefWEB_CUrlPathTemplate.h"
#define GUCEF_WEB_CURLPATHTEMPLATE_H
#endif /* GUCEF_WEB_CURLPATHTEMPLATE_H ? */

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

/**
 *  Hands the bodies of responses the page itself requested to a caller, in memory, for the endpoints the caller names
 *  (gap G5 of the investment design, site discovery design section 12.4). It never sends a request to the site: it
 *  reads, with Network.getResponseBody, the answer the browser already received.
 *
 *  A tap (AddTap()) names a method, a host glob pattern and a glob pattern on the path template (CUrlPathTemplate: no
 *  query, identifiers as {n}, {guid}, ...). Responses of never-record addresses (sign-in, payment) are never matched and
 *  their bodies never fetched. A body larger than the tap's maxBodyBytes is not delivered.
 *
 *  The tap watches the tabs that pass its page host filter with sessions of its own (network events only, frames and
 *  workers through auto-attach), so it works with a CChromeNetworkRecorder on the same client and without one.
 *
 *  Non-blocking: WaitForResponse() arms a wait, Process() delivers bodies and ends waits, GetWaitState() reads it.
 *  Events: ResponseDeliveredEvent and WaitEndedEvent with the tap id (CORE::TCloneableUInt32), never the body.
 *  Process() and Start() run on the thread that drives the client's pulse generator. Not thread-safe otherwise.
 */
class GUCEF_WEB_PUBLIC_CPP CWebResponseTap : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent ResponseDeliveredEvent;
    static const CORE::CEvent WaitEndedEvent;

    static void RegisterEvents( void );

    static const UInt32 MaxBodyBytesCeiling;        /**< 64 MiB: the largest maxBodyBytes of a tap */

    enum EWaitState
    {
        WAIT_NONE       ,   /**< no wait armed, or an unknown tap */
        WAIT_PENDING    ,
        WAIT_RECEIVED   ,   /**< a body was delivered to the tap's sink */
        WAIT_TIMED_OUT  ,
        WAIT_TOO_LARGE      /**< a matching response came, larger than the tap's maxBodyBytes */
    };

    /** @param browserClient a client connected to the browser itself, which must outlive this object */
    explicit CWebResponseTap( CChromeDevToolsClient& browserClient );

    virtual ~CWebResponseTap();

    /**
     *  @param method               GET, POST, ...; empty or "*" for any
     *  @param hostPattern          glob on the host name, or on host:port when the pattern holds a colon; case insensitive
     *  @param pathTemplatePattern  glob on the path template, case sensitive: '*' within one segment, '**' across segments,
     *                              braces literal; for example /api/accounts/{n}/positions, or the glob ** for every path
     *  @param sink                 receives the bodies, must outlive the tap or be removed first
     *  @param maxBodyBytes         larger bodies are not delivered; at most MaxBodyBytesCeiling
     *  @return the tap id, 0 when the arguments are not usable
     */
    UInt32 AddTap( const CORE::CString& method              ,
                   const CORE::CString& hostPattern         ,
                   const CORE::CString& pathTemplatePattern ,
                   CIWebResponseSink* sink                  ,
                   UInt32 maxBodyBytes                      );

    bool RemoveTap( UInt32 tapId );

    UInt32 GetTapCount( void ) const;

    /** Which tabs are watched: the host must contain one of the terms, case insensitive; none: every tab */
    void SetPageHostFilter( const CORE::CString::StringVector& terms );

    /** The never-record rules (neverRecordHosts, neverRecordPathPatterns) of these settings; by default the defaults */
    void SetNeverRecordRules( const CSiteDiscoverySettings& settings );

    /** Starts watching the tabs, over a client connected to the browser itself */
    bool Start( void );

    void Stop( void );

    bool IsStarted( void ) const;

    /** Attaches to tabs, delivers bodies, ends waits. @return the things done */
    UInt32 Process( void );

    /**
     *  Arms a wait that ends with the first delivery to the tap after the previous wait of that tap ended (a body that
     *  came before this call counts), with a matching response that is too large, or with the timeout
     */
    bool WaitForResponse( UInt32 tapId, UInt32 timeoutInMs );

    EWaitState GetWaitState( UInt32 tapId ) const;

    UInt32 GetDeliveredCount( UInt32 tapId ) const;

    UInt32 GetSessionCount( void ) const;

    /** The matching rule of a tap, for a request method and a parsed address */
    static bool IsMatch( const CORE::CString& methodPattern, const CORE::CString& hostPattern, const CORE::CString& pathTemplatePattern,
                         const CORE::CString& method, const CUrlPathTemplate& parsed );

    private:

    CWebResponseTap( const CWebResponseTap& src );                  /**< not implemented */
    CWebResponseTap& operator=( const CWebResponseTap& src );       /**< not implemented */

    typedef CORE::CTEventHandlerFunctor< CWebResponseTap > TEventCallback;
    typedef GUCEF::map< CORE::CString, CORE::CString >     TStringMap;
    typedef GUCEF::vector< UInt32 >                         TUInt32Vector;

    struct STap
    {
        UInt32 id;
        CORE::CString method;
        CORE::CString hostPattern;
        CORE::CString pathPattern;
        CIWebResponseSink* sink;
        UInt32 maxBodyBytes;
        UInt32 delivered;
        UInt32 tooLarge;
        UInt32 deliveredSeen;       /**< delivered when the previous wait ended */
        UInt32 tooLargeSeen;
        EWaitState waitState;
        UInt64 waitDeadline;

        STap( void );
    };

    struct SPending
    {
        CORE::CString sessionId;
        CORE::CString requestId;
        CORE::CString method;
        CORE::CString host;
        CORE::CString pathTemplate;
        Int32 status;
        CORE::CString mimeType;
        UInt32 sequence;
        UInt64 encodedDataLength;
        TUInt32Vector tapIds;

        SPending( void );
    };

    typedef GUCEF::map< UInt32, STap >              TTapMap;
    typedef GUCEF::map< CORE::CString, SPending >   TPendingMap;
    typedef GUCEF::vector< SPending >               TPendingVector;

    void OnProtocolEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void HandleRequestWillBeSent( const SChromeProtocolEvent& event );
    void HandleResponseReceived( const SChromeProtocolEvent& event );
    void HandleLoadingFinished( const SChromeProtocolEvent& event );
    void HandleTarget( const SChromeProtocolEvent& event );
    bool PagePassesFilter( const CORE::CString& url ) const;
    bool SetupSession( const CORE::CString& sessionId );
    void Deliver( SPending& pending );
    void EndWait( STap& tap, EWaitState state );
    UInt64 NowInMs( void ) const;

    CChromeDevToolsClient& m_client;
    CSiteDiscoveryRedactor m_neverRecord;
    CORE::CString::StringVector m_pageHostFilter;
    TTapMap m_taps;
    UInt32 m_nextTapId;
    UInt32 m_nextSequence;
    bool m_started;
    TPendingMap m_inFlight;
    TPendingVector m_ready;
    CORE::CString::StringVector m_targetsToAttach;
    CORE::CString::StringSet m_attachedTargets;
    CORE::CString::StringSet m_sessions;
    TStringMap m_sessionsToSetup;
    TStringMap m_sessionTargets;
    CORE::CDynamicBuffer m_body;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBRESPONSETAP_H ? */
