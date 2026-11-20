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

#ifndef GUCEF_WEB_CNAVIGATIONGUARD_H
#define GUCEF_WEB_CNAVIGATIONGUARD_H

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

#ifndef GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#include "gucefWEB_CChromeDevToolsClient.h"
#define GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#endif /* GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H ? */

#ifndef GUCEF_WEB_CNAVIGATIONGUARDSETTINGS_H
#include "gucefWEB_CNavigationGuardSettings.h"
#define GUCEF_WEB_CNAVIGATIONGUARDSETTINGS_H
#endif /* GUCEF_WEB_CNAVIGATIONGUARDSETTINGS_H ? */

#ifndef GUCEF_WEB_CNAVIGATIONRULE_H
#include "gucefWEB_CNavigationRule.h"
#define GUCEF_WEB_CNAVIGATIONRULE_H
#endif /* GUCEF_WEB_CNAVIGATIONRULE_H ? */

#ifndef GUCEF_WEB_CNAVIGATIONREFUSAL_H
#include "gucefWEB_CNavigationRefusal.h"
#define GUCEF_WEB_CNAVIGATIONREFUSAL_H
#endif /* GUCEF_WEB_CNAVIGATIONREFUSAL_H ? */

#ifndef GUCEF_WEB_CWRITEREQUESTGUARD_H
#include "gucefWEB_CWriteRequestGuard.h"
#define GUCEF_WEB_CWRITEREQUESTGUARD_H
#endif /* GUCEF_WEB_CWRITEREQUESTGUARD_H ? */

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
 *  Keeps the tabs and frames of a browser on the addresses a program allows (see CNavigationRule and
 *  CNavigationGuardSettings): a document request outside the allow rules, inside a deny rule, of a scheme other than
 *  http(s), or of a sign-in or payment address that no allow rule names, is refused before it leaves the browser.
 *
 *  Start() enables the Fetch domain of the browser target for document requests: every navigation of every tab and
 *  frame (links, form posts, scripts, Page.navigate sent by anyone, every leg of a redirect) waits in the browser until
 *  the guard answers, which it does from the event itself: Fetch.continueRequest, or Fetch.failRequest with "Aborted",
 *  which keeps the page where it was. Navigations within a document (history.pushState) make no request: an address
 *  outside the rules that a tab shows is reported with source "committed" and blocked false.
 *
 *  With a CWriteRequestGuard given (SetWriteRequestGuard()) the guard intercepts every request of every tab, frame and
 *  worker instead of document requests only, and lets the write request guard decide first: a request other than GET,
 *  HEAD or OPTIONS that no write rule names is failed with "BlockedByClient" and reported by that guard; what passes it
 *  and is a document request is then checked against the navigation rules; every other request continues.
 *
 *  Every refusal is counted, kept (the latest 64) and sent as NavigationRefusedEvent (TNavigationRefusalEventData).
 *  The guard owns the Fetch domain of the browser target while it is started. Navigations wait while the thread that
 *  drives the client's pulse generator does not pump it. Not thread-safe: use it on that thread.
 */
class GUCEF_WEB_PUBLIC_CPP CNavigationGuard : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent NavigationRefusedEvent;

    static void RegisterEvents( void );

    CNavigationGuard( void );

    virtual ~CNavigationGuard();

    /** False, keeping the rules as they were, when a rule does not parse */
    bool SetSettings( const CNavigationGuardSettings& settings );

    const CNavigationGuardSettings& GetSettings( void ) const;

    /**
     *  Does the guard allow this address? Only a check, nothing is reported.
     *  @param isMainFrame  a frame's document passes when guardSubframes is off
     *  @param reason       the reason of a refusal, see CNavigationRefusal
     */
    bool IsAllowed( const CORE::CString& url, bool isMainFrame = true, CORE::CString* reason = GUCEF_NULL ) const;

    /** Starts guarding the browser over a client connected to the browser itself */
    bool Start( CChromeDevToolsClient& browserClient );

    void Stop( void );

    bool IsStarted( void ) const;

    /**
     *  For a wrapper of Page.navigate, such as CDevToolsPage::Navigate(): true when the address is allowed, otherwise
     *  the refusal is reported with source "navigate" and the caller sends nothing
     */
    bool CheckNavigate( const CORE::CString& url );

    /**
     *  Enforces a write request guard (see CWriteRequestGuard), or none with GUCEF_NULL. Before Start() it only takes
     *  effect at the start; on a started guard the browser is told at once, and false (keeping the guard as it was) when
     *  the browser does not accept the change. A write request guard serves one navigation guard at a time.
     */
    bool SetWriteRequestGuard( CWriteRequestGuard* writeGuard );

    CWriteRequestGuard* GetWriteRequestGuard( void ) const;

    UInt32 GetRefusedCount( void ) const;

    UInt32 GetAllowedCount( void ) const;

    const TNavigationRefusalVector& GetRecentRefusals( void ) const;

    void ClearRecentRefusals( void );

    /** The template of an address as a refusal names it: scheme://host/path-template, the scheme only when it is not http(s) */
    static CORE::CString MakeUrlTemplate( const CORE::CString& url, CORE::CString* host = GUCEF_NULL );

    private:

    friend class CWriteRequestGuard;

    CNavigationGuard( const CNavigationGuard& src );                /**< not implemented */
    CNavigationGuard& operator=( const CNavigationGuard& src );     /**< not implemented */

    typedef CORE::CTEventHandlerFunctor< CNavigationGuard > TEventCallback;
    typedef GUCEF::map< CORE::CString, CORE::CString >      TStringMap;

    void OnProtocolEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnClientDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void HandleRequestPaused( const SChromeProtocolEvent& event );
    void HandleTargetInfo( const SChromeProtocolEvent& event );
    void Report( const CORE::CString& url, const CORE::CString& frameId, bool isMainFrame, const CORE::CString& reason, const CORE::CString& source, bool blocked );
    bool IsNeverRecord( const CORE::CString& hostName, const CORE::CString& hostAndPort, const CORE::CString& path ) const;
    bool EnableInterception( bool everyRequest, bool wait );
    void ReleaseWriteRequestGuard( CWriteRequestGuard* writeGuard );

    CNavigationGuardSettings m_settings;
    TNavigationRuleVector m_allow;
    TNavigationRuleVector m_deny;
    CChromeDevToolsClient* m_client;
    CORE::CString::StringSet m_pageTargets;     /**< target ids of tabs, which are the frame ids of their main frames */
    TStringMap m_reportedCommits;               /**< target id to the address last reported as committed */
    UInt32 m_refusedCount;
    UInt32 m_allowedCount;
    TNavigationRefusalVector m_recent;
    CWriteRequestGuard* m_writeGuard;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CNAVIGATIONGUARD_H ? */
