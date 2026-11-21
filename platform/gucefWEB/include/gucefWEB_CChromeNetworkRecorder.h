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

#ifndef GUCEF_WEB_CCHROMENETWORKRECORDER_H
#define GUCEF_WEB_CCHROMENETWORKRECORDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#include "gucefWEB_CChromeDevToolsClient.h"
#define GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#endif /* GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H ? */

#ifndef GUCEF_WEB_CIWEBOBSERVATIONSINK_H
#include "gucefWEB_CIWebObservationSink.h"
#define GUCEF_WEB_CIWEBOBSERVATIONSINK_H
#endif /* GUCEF_WEB_CIWEBOBSERVATIONSINK_H ? */

#ifndef GUCEF_WEB_CSITEDISCOVERYREDACTOR_H
#include "gucefWEB_CSiteDiscoveryRedactor.h"
#define GUCEF_WEB_CSITEDISCOVERYREDACTOR_H
#endif /* GUCEF_WEB_CSITEDISCOVERYREDACTOR_H ? */

#ifndef GUCEF_WEB_CUSERACTIONTRACKER_H
#include "gucefWEB_CUserActionTracker.h"
#define GUCEF_WEB_CUSERACTIONTRACKER_H
#endif /* GUCEF_WEB_CUSERACTIONTRACKER_H ? */

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
 *  Records what the web pages in a browser request, over the Chrome DevTools protocol, while the user browses by hand.
 *  Works on a connection to the browser itself (not to one page), so it sees every tab, also tabs opened later, and the
 *  frames and workers inside them. It only listens: it never navigates, clicks or types.
 *
 *  Only tabs that show a host passing the host filter are watched and only requests in scope are recorded (see
 *  CSiteDiscoveryRedactor): the browser is the user's own and its other tabs are none of our business. Secret header
 *  values are dropped as they arrive, never-record requests keep only that they happened, and every secret value seen
 *  is registered with the redactor so that no journal line or report can repeat it.
 *
 *  Every finished request goes to the sinks (AddSink()): typically a CSiteDiscoveryJournal and a CSiteEndpointCatalog
 *  sharing the recorder's redactor. Requests are grouped by the user action they followed, see CUserActionTracker.
 *  The response body of a data request is fetched only when a sink wants its shape or bodies are saved
 *  (saveResponseBodies, owner-only files in <body directory>/<bodiesDirectoryName>).
 *
 *  Threading: events of the client are only queued while they are delivered. Start() and Process() send commands and
 *  wait for their answers through the client's blocking members, which pump the client's pulse generator: call them on
 *  the thread that drives that pulse generator, and call Process() regularly. Not thread-safe otherwise.
 */
class GUCEF_WEB_PUBLIC_CPP CChromeNetworkRecorder : public CORE::CObserver
{
    public:

    static const CORE::CString ActionBindingName;       /**< the binding through which the optional click listener reports */
    static const CORE::CString ClickListenerWorldName;  /**< the isolated world the click listener runs in */

    explicit CChromeNetworkRecorder( CChromeDevToolsClient& client );

    virtual ~CChromeNetworkRecorder();

    /** Shares the redactor of a session; GUCEF_NULL returns to the recorder's own. Its settings are the recorder's */
    void SetRedactor( CSiteDiscoveryRedactor* redactor );

    CSiteDiscoveryRedactor& GetRedactor( void ) const;

    /** The folder saved bodies go in (a sub folder named by bodiesDirectoryName); the journal's folder, so bodyFile is relative to it */
    void SetBodyDirectory( const CORE::CString& directory );

    void AddSink( CIWebObservationSink* sink );

    void RemoveSink( CIWebObservationSink* sink );

    /** The sequence number of the next request, for example CSiteDiscoveryJournal::GetNextSequence() */
    void SetNextSequence( UInt32 sequence );

    UInt32 GetNextSequence( void ) const;

    /** Subscribes to the client, and starts watching the open tabs and the ones that will be opened. Needs a connection to the browser */
    bool Start( void );

    void Stop( void );

    bool IsStarted( void ) const;

    /** Does the work that was queued up: attaching to tabs, setting up sessions, completing requests. @return the things done */
    UInt32 Process( void );

    UInt32 GetRecordedRequestCount( void ) const;

    UInt32 GetSavedBodyCount( void ) const;

    UInt32 GetSessionCount( void ) const;

    UInt32 GetActionCount( void ) const;

    /** The script of the passive click and submit listener installed with captureClicks */
    static const CORE::CString& GetClickListenerScript( void );

    /** The disposition type of a Content-Disposition header, and the suggested file name with digit runs as {n} */
    static void ParseContentDisposition( const CORE::CString& headerValue, CORE::CString& dispositionType, CORE::CString& fileNameTemplate );

    /** A file name with its runs of digits replaced by {n} */
    static CORE::CString TemplateFileName( const CORE::CString& fileName );

    private:

    CChromeNetworkRecorder( const CChromeNetworkRecorder& src );                /**< not implemented */
    CChromeNetworkRecorder& operator=( const CChromeNetworkRecorder& src );     /**< not implemented */

    typedef CORE::CTEventHandlerFunctor< CChromeNetworkRecorder > TEventCallback;

    typedef GUCEF::map< CORE::CString, CORE::CString >      TStringMap;

    struct SPending
    {
        CObservedWebRequest observation;
        CORE::CString sessionId;
        CORE::CString requestId;
        bool needsPostData;

        SPending( void ) : observation(), sessionId(), requestId(), needsPostData( false ) {}
    };

    /** What Network.requestWillBeSentExtraInfo and Network.responseReceivedExtraInfo told, which can come before the request they belong to */
    struct SExtraInfo
    {
        CORE::CString::StringVector cookiesSent;
        CORE::CString::StringVector cookiesSet;
        TStringMap headerNames;     /**< request header names with the value the redactor kept (not recorded for secret headers) */

        SExtraInfo( void ) : cookiesSent(), cookiesSet(), headerNames() {}
    };

    typedef GUCEF::map< CORE::CString, SExtraInfo >         TExtraInfoMap;
    typedef GUCEF::map< CORE::CString, UInt32 >             TCountMap;
    typedef GUCEF::map< CORE::CString, SPending >           TPendingMap;
    typedef GUCEF::vector< SPending >                       TPendingVector;
    typedef GUCEF::vector< CIWebObservationSink* >          TSinkVector;

    void OnProtocolEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void HandleRequestWillBeSent( const SChromeProtocolEvent& event );
    void HandleResponseReceived( const SChromeProtocolEvent& event );
    void HandleLoadingFinished( const SChromeProtocolEvent& event );
    void HandleLoadingFailed( const SChromeProtocolEvent& event );
    void HandleRequestWillBeSentExtraInfo( const SChromeProtocolEvent& event );
    void HandleResponseReceivedExtraInfo( const SChromeProtocolEvent& event );
    void HandleDownloadWillBegin( const SChromeProtocolEvent& event );
    void HandleNavigatedWithinDocument( const SChromeProtocolEvent& event );
    void HandleBindingCalled( const SChromeProtocolEvent& event );
    void HandleAttachedToTarget( const SChromeProtocolEvent& event );
    void HandleDetachedFromTarget( const SChromeProtocolEvent& event );
    void HandleTargetCreated( const SChromeProtocolEvent& event );
    void HandleTargetInfoChanged( const SChromeProtocolEvent& event );

    bool UrlPassesFilter( const CORE::CString& url ) const;
    void StartAction( const CORE::CString& kind, const CORE::CString& label, const CORE::CString& page );
    void EmitAction( const CObservedUserAction& action );
    bool SetupSession( const CORE::CString& sessionId, bool isPage );
    bool InstallClickListener( const CORE::CString& sessionId );
    CORE::CString FindPageOfSession( const CORE::CString& sessionId ) const;
    bool SendOnSession( const CORE::CString& sessionId, const CORE::CString& method, const CORE::CDataNode* params, CORE::CDataNode* result = GUCEF_NULL );
    void DescribeRequestBody( CObservedWebRequest& observation ) const;
    void Complete( SPending& pending, bool fetchBody );
    SPending* FindPending( const CORE::CString& key );
    void ApplyExtraInfo( CObservedWebRequest& observation, const SExtraInfo& extra ) const;
    void NoteCookiesSet( CObservedWebRequest& observation, const CORE::CString& setCookieHeader ) const;
    bool SaveBody( CObservedWebRequest& observation, const CORE::CString& body );
    static CORE::CString MakeKey( const CORE::CString& sessionId, const CORE::CString& requestId );
    static CORE::CString PageOf( const CORE::CString& url );
    UInt64 NowInMs( void ) const;

    private:

    CChromeDevToolsClient& m_client;
    CSiteDiscoveryRedactor m_ownRedactor;
    CSiteDiscoveryRedactor* m_redactor;
    CORE::CString m_bodyDirectory;
    TSinkVector m_sinks;
    CUserActionTracker m_tracker;
    bool m_started;
    UInt32 m_nextSequence;
    UInt64 m_startTicks;

    TPendingMap m_inFlight;
    TPendingVector m_finished;                  /**< finished loading, the body can be fetched */
    TPendingVector m_completedWithoutBody;      /**< failed, redirected: nothing to fetch */
    TExtraInfoMap m_extraInfo;                  /**< extra info of requests not seen yet, bounded */
    TCountMap m_shapeProbes;                    /**< per endpoint the responses fetched to look for a new shape */
    CORE::CString::StringVector m_targetsToAttach;
    TStringMap m_sessionsToSetup;               /**< session id to target type */
    CORE::CString::StringSet m_sessions;
    CORE::CString::StringSet m_attachedTargets;
    TStringMap m_sessionTargets;                /**< session id to target id, the id of the main frame of a page */
    TStringMap m_sessionPages;                  /**< session id to the page it shows, as scheme://host/path-template */
    TStringMap m_sessionParents;                /**< session id of a frame or worker to the session it was attached through */
    UInt32 m_recordedCount;
    UInt32 m_savedBodyCount;
    bool m_bodyDirectoryReady;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CCHROMENETWORKRECORDER_H ? */
