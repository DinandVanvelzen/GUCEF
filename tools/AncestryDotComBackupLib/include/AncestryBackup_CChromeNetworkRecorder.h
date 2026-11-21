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

#include <string>
#include <vector>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_WEB_CCHROMENETWORKRECORDER_H
#include "gucefWEB_CChromeNetworkRecorder.h"
#define GUCEF_WEB_CCHROMENETWORKRECORDER_H
#endif /* GUCEF_WEB_CCHROMENETWORKRECORDER_H ? */

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
 *  Records what the web pages in the browser request into the endpoint catalog and the discovery journal. The work is
 *  done by WEB::CChromeNetworkRecorder; this adds the policy of the tool: the rules of CEndpointCatalog::MakeSettings()
 *  (example values, credentials never, DNA only when the catalog records DNA), the browser's cache switched off so every
 *  response is seen, responses saved under the discovery directory of the data layout, and the endpoints that were seen
 *  for the first time kept for the user to read.
 *
 *  Only requests to hosts that match the host filter are recorded, and only tabs that show such a host are watched.
 *  Process() has to be called regularly, on the thread that drives the pulse generator of the client.
 */
class CChromeNetworkRecorder : public WEB::CIWebObservationSink
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

    /** Starts watching the open tabs and the ones that will be opened */
    bool Start( void );

    void Stop( void );

    bool IsStarted( void ) const;

    /** Does the work that was queued up. @return the number of things that were done */
    UInt32 Process( void );

    /** The endpoints which were seen for the first time since the last call, as: METHOD host/path (type) */
    bool PopNewEndpoint( std::string& description );

    UInt32 GetRecordedRequestCount( void ) const;

    UInt32 GetSavedBodyCount( void ) const;

    UInt32 GetSessionCount( void ) const;

    /** Does a host pass the filter? */
    static bool MatchesFilter( const std::vector< std::string >& terms, const std::string& host );

    virtual void OnWebRequestObserved( const WEB::CObservedWebRequest& observation ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnUserActionObserved( const WEB::CObservedUserAction& action ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool WantsResponseShape( const WEB::CObservedWebRequest& observation ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsNewResponseShape( const WEB::CObservedWebRequest& observation, const WEB::CWebBodyShape& shape ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CChromeNetworkRecorder( const CChromeNetworkRecorder& src );
    CChromeNetworkRecorder& operator=( const CChromeNetworkRecorder& src );

    private:

    CEndpointCatalog& m_catalog;
    CDiscoveryJournal& m_journal;
    WEB::CSiteDiscoverySettings m_settings;
    WEB::CSiteDiscoveryRedactor m_redactor;
    WEB::CChromeNetworkRecorder m_recorder;
    std::vector< std::string > m_newEndpoints;
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
