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

#ifndef ANCESTRYBACKUP_CBROWSERSESSION_H
#define ANCESTRYBACKUP_CBROWSERSESSION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#include "gucefWEB_CChromeDevToolsClient.h"
#define GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#endif /* GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H ? */

#ifndef ANCESTRYBACKUP_CCHROMEBROWSER_H
#include "AncestryBackup_CChromeBrowser.h"
#define ANCESTRYBACKUP_CCHROMEBROWSER_H
#endif /* ANCESTRYBACKUP_CCHROMEBROWSER_H ? */

#ifndef ANCESTRYBACKUP_CDATALAYOUT_H
#include "AncestryBackup_CDataLayout.h"
#define ANCESTRYBACKUP_CDATALAYOUT_H
#endif /* ANCESTRYBACKUP_CDATALAYOUT_H ? */

#ifndef ANCESTRYBACKUP_CREQUESTPOLICY_H
#include "AncestryBackup_CRequestPolicy.h"
#define ANCESTRYBACKUP_CREQUESTPOLICY_H
#endif /* ANCESTRYBACKUP_CREQUESTPOLICY_H ? */

#ifndef ANCESTRYBACKUP_CREQUESTBUDGET_H
#include "AncestryBackup_CRequestBudget.h"
#define ANCESTRYBACKUP_CREQUESTBUDGET_H
#endif /* ANCESTRYBACKUP_CREQUESTBUDGET_H ? */

#ifndef ANCESTRYBACKUP_CSETTINGS_H
#include "AncestryBackup_CSettings.h"
#define ANCESTRYBACKUP_CSETTINGS_H
#endif /* ANCESTRYBACKUP_CSETTINGS_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#include <string>

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

/** What came back from a request */
struct SFetchResult
{
    bool ok;                        /**< the request was made and the answer is a good one */
    Int32 status;                   /**< http status, 0 when no answer was received */
    std::string contentType;
    std::string body;               /**< for GetText */
    UInt64 bytes;                   /**< for GetToFile: how much was written */
    std::string error;              /**< when it is not ok: why */
    bool halted;                    /**< the site showed signs of unhappiness and the tool stopped for good, see CHaltState */

    SFetchResult( void ) : ok( false ), status( 0 ), contentType(), body(), bytes( 0 ), error(), halted( false ) {}
};

/**
 *  The way the tool talks to the site: through the user's own browser, in the tab of the site where they are logged in.
 *  The requests are the same as the ones the site's own pages make, with the same cookies and the same browser
 *  behind them. It is also what keeps the tool simple: there is no login, no cookie handling and no password.
 *
 *  Every request goes through all of these, in this order:
 *    - the halt state: when the site showed unhappiness before the tool does nothing until it is cleared
 *    - the request policy: GET only, the right tree, only known read endpoints
 *    - the request budget: a pause between requests that varies, and a maximum per run and per day
 *    - the check of the answer: an error that says to go away, or a challenge, halts the tool right there
 */
class CBrowserSession : public CORE::CObserver
{
    public:

    /** @param siteTreeId  the id Ancestry uses for the tree: a number */
    CBrowserSession( const CSettings& settings, const CDataLayout& layout, const std::string& siteTreeId );

    ~CBrowserSession();

    /** The policy and the budget can be adjusted before Open(), the defaults are the ones of the settings */
    CRequestPolicy& GetPolicy( void );

    CRequestBudget& GetBudget( void );

    /**
     *  Gets a browser to work with, connects to it and finds or opens a tab of the site.
     *  @param startUrl     the page to open when there is no tab of the site yet
     */
    bool Open( void );

    void Close( void );

    bool IsOpen( void ) const;

    /** Was a browser of the user attached to, as opposed to one that was started by the tool? */
    bool IsAttached( void ) const;

    /** GET a text resource. @param expectJson an html page in return is a sign of trouble */
    bool GetText( const std::string& url, bool expectJson, SFetchResult& result );

    /**
     *  POST a request that only reads, to the few endpoints the policy allows for it (see CRequestPolicy::AddDnaAllowed).
     *  The body is JSON made of ids.
     */
    bool PostJson( const std::string& url, const std::string& body, SFetchResult& result );

    /** GET a resource of any kind into a file, in pieces, so a large download is not held in memory */
    bool GetToFile( const std::string& url, const CORE::CString& path, SFetchResult& result );

    /**
     *  Prefer the tab whose address contains this, when there are several tabs of the site. Used by the DNA facility, whose requests
     *  are made from inside the DNA pages.
     */
    void SetPreferredTabUrlPart( const std::string& part );

    /** The POST requests of the site carry a token that the page's scripts add. Wait to see one from the tab, which reloads the page if it has to. */
    void SetNeedsPostToken( bool needed );

    /** The header the site's own requests carry and which its servers expect: it is learned from the traffic of the tab */
    const std::string& GetContextHeader( void ) const;

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata /* = NULL */ ) GUCEF_VIRTUAL_OVERRIDE;

    /** Pumps the events for this long */
    static void Pause( UInt32 delayInMs );

    private:

    CBrowserSession( const CBrowserSession& src );
    CBrowserSession& operator=( const CBrowserSession& src );

    bool Admit( const std::string& method, const std::string& url, const std::string& body, SFetchResult& result );
    void Judge( const std::string& url, SFetchResult& result, const std::string& mitigation, const std::string& bodyStart, bool expectJson );

    private:

    CSettings m_settings;
    CDataLayout m_layout;
    CRequestPolicy m_policy;
    CRequestBudget m_budget;
    CChromeBrowser m_browser;
    WEB::CChromeDevToolsClient m_client;
    CORE::CString m_sessionId;
    CORE::CString m_frameId;
    std::string m_contextHeader;
    std::string m_csrfToken;
    std::string m_preferredTabPart;
    bool m_needsPostToken;
    bool m_open;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CBROWSERSESSION_H ? */
