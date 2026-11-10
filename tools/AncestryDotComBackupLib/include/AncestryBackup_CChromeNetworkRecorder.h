/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#ifndef ANCESTRYBACKUP_CCHROMENETWORKRECORDER_H
#define ANCESTRYBACKUP_CCHROMENETWORKRECORDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#include "gucefWEB_CChromeDevToolsClient.h"
#define GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#endif /* GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H ? */

#ifndef ANCESTRYBACKUP_CENDPOINTCATALOG_H
#include "AncestryBackup_CEndpointCatalog.h"
#define ANCESTRYBACKUP_CENDPOINTCATALOG_H
#endif /* ANCESTRYBACKUP_CENDPOINTCATALOG_H ? */

#ifndef ANCESTRYBACKUP_CDISCOVERYJOURNAL_H
#include "AncestryBackup_CDiscoveryJournal.h"
#define ANCESTRYBACKUP_CDISCOVERYJOURNAL_H
#endif /* ANCESTRYBACKUP_CDISCOVERYJOURNAL_H ? */

#ifndef ANCESTRYBACKUP_CDATALAYOUT_H
#include "AncestryBackup_CDataLayout.h"
#define ANCESTRYBACKUP_CDATALAYOUT_H
#endif /* ANCESTRYBACKUP_CDATALAYOUT_H ? */

#include <string>
#include <vector>
#include <set>
#include <map>

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Records what the web pages in the browser request, over the DevTools protocol, into the endpoint catalog
 *  and the discovery journal. Works on a connection to the browser (not to a single page), so it sees every
 *  tab, including tabs that are opened later, and the frames and workers inside them.
 *
 *  Only requests to hosts that match the host filter are recorded, and only tabs that show such a host are
 *  watched at all. The browser is the user's own and has other tabs open, the contents of which are none of
 *  our business. A tab that goes to the site later is picked up when it does.
 *
 *  Events are only queued while they are being delivered. The work that needs commands to the browser, such
 *  as getting the body of a response, happens in Process() which has to be called regularly.
 */
class CChromeNetworkRecorder : public CORE::CObserver
{
    public:

    CChromeNetworkRecorder( WEB::CChromeDevToolsClient& client ,
                            const CDataLayout& layout          ,
                            CEndpointCatalog& catalog          ,
                            CDiscoveryJournal& journal         );

    virtual ~CChromeNetworkRecorder();

    /** Hosts are recorded when they contain one of these terms, case insensitive. No terms: record everything. */
    void SetHostFilter( const std::vector< std::string >& terms );

    /** Responses larger than this are not saved, only noted */
    void SetMaxBodyBytes( UInt64 maxBodyBytes );

    /** Subscribes to the client, and starts watching the open tabs and the ones that will be opened */
    bool Start( void );

    void Stop( void );

    bool IsStarted( void ) const;

    /** Does the work that was queued up. @return the number of things that were done */
    UInt32 Process( void );

    /** The endpoints which were seen for the first time since the last call, as: METHOD host/path */
    bool PopNewEndpoint( std::string& description );

    UInt32 GetRecordedRequestCount( void ) const;

    UInt32 GetSavedBodyCount( void ) const;

    UInt32 GetSessionCount( void ) const;

    /** Does a host pass the filter? */
    static bool MatchesFilter( const std::vector< std::string >& terms, const std::string& host );

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata /* = NULL */ ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CChromeNetworkRecorder( const CChromeNetworkRecorder& src );
    CChromeNetworkRecorder& operator=( const CChromeNetworkRecorder& src );

    struct SPendingFinish
    {
        SObservedRequest observation;
        std::string sessionId;
        std::string requestId;
    };

    typedef std::map< std::string, SPendingFinish > TInFlightMap;

    void HandleProtocolEvent( const WEB::SChromeProtocolEvent& event );
    void HandleRequestWillBeSent( const WEB::SChromeProtocolEvent& event );
    void HandleResponseReceived( const WEB::SChromeProtocolEvent& event );
    void HandleLoadingFinished( const WEB::SChromeProtocolEvent& event );
    void HandleLoadingFailed( const WEB::SChromeProtocolEvent& event );
    void HandleAttachedToTarget( const WEB::SChromeProtocolEvent& event );
    void HandleTargetCreated( const WEB::SChromeProtocolEvent& event );
    void HandleTargetInfoChanged( const WEB::SChromeProtocolEvent& event );
    bool UrlPassesFilter( const std::string& url ) const;

    bool SetupSession( const std::string& sessionId );
    bool SendOnSession( const std::string& sessionId, const CORE::CString& method, const CORE::CDataNode* params );
    void Complete( SPendingFinish& finished, bool fetchBody );
    bool ShouldSaveBody( const SObservedRequest& observation ) const;
    static std::string MakeKey( const std::string& sessionId, const std::string& requestId );

    private:

    WEB::CChromeDevToolsClient& m_client;
    CDataLayout m_layout;
    CEndpointCatalog& m_catalog;
    CDiscoveryJournal& m_journal;
    std::vector< std::string > m_hostFilter;
    UInt64 m_maxBodyBytes;
    bool m_started;

    TInFlightMap m_inFlight;
    std::vector< SPendingFinish > m_finished;               // finished loading, the body can be fetched
    std::vector< SPendingFinish > m_completedWithoutBody;   // failed, redirected: nothing to fetch
    std::vector< std::string > m_targetsToAttach;
    std::vector< std::string > m_sessionsToSetup;
    std::set< std::string > m_sessions;
    std::set< std::string > m_attachedTargets;
    std::vector< std::string > m_newEndpoints;
    UInt32 m_recordedCount;
    UInt32 m_savedBodyCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CCHROMENETWORKRECORDER_H ? */
