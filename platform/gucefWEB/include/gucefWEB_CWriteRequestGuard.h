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

#ifndef GUCEF_WEB_CWRITEREQUESTGUARD_H
#define GUCEF_WEB_CWRITEREQUESTGUARD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_WEB_CWRITEREQUESTGUARDSETTINGS_H
#include "gucefWEB_CWriteRequestGuardSettings.h"
#define GUCEF_WEB_CWRITEREQUESTGUARDSETTINGS_H
#endif /* GUCEF_WEB_CWRITEREQUESTGUARDSETTINGS_H ? */

#ifndef GUCEF_WEB_CWRITEREQUESTRULE_H
#include "gucefWEB_CWriteRequestRule.h"
#define GUCEF_WEB_CWRITEREQUESTRULE_H
#endif /* GUCEF_WEB_CWRITEREQUESTRULE_H ? */

#ifndef GUCEF_WEB_CWRITEREQUESTREFUSAL_H
#include "gucefWEB_CWriteRequestRefusal.h"
#define GUCEF_WEB_CWRITEREQUESTREFUSAL_H
#endif /* GUCEF_WEB_CWRITEREQUESTREFUSAL_H ? */

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

class CNavigationGuard;
class CChromeDevToolsClient;

/**
 *  Keeps a browser from sending anything but reads: every request whose method is not exactly GET, HEAD or OPTIONS,
 *  of any kind (fetch, XHR, form posts, sendBeacon and pings, the requests of frames, of tabs and windows the page
 *  opens, and every leg of a redirect), is failed before it leaves the browser unless an allow rule names it exactly
 *  (CWriteRequestRule: method, scheme, host and path or path template, query). A refused request fails in the page
 *  with a network error (Fetch.failRequest with "BlockedByClient"); it is counted, kept (the latest 64) and sent as
 *  WriteRequestRefusedEvent (TWriteRequestRefusalEventData): method, address with the path as a template and the query
 *  values redacted, resource type, frame and the reason of the miss. Never a body.
 *
 *  The guard is enforced by a CNavigationGuard, which owns the Fetch domain of the browser target:
 *  CNavigationGuard::SetWriteRequestGuard() makes it intercept every request of every target at the request stage
 *  instead of document requests only. The interception lives in the browser, outside the reach of page scripts; it
 *  ends when the connection to the browser ends, which IsEnforcing() then tells. A program that wants this guard and no
 *  navigation rules gives the navigation guard empty allow and deny lists and refuseNeverRecordUnlessAllowed false.
 *
 *  A program that may only run with the guard in force (a flow that writes) installs it before the first navigation,
 *  checks IsEnforcing() and proves the interception on the page it drives with VerifyEnforcing(), which makes the page
 *  send a POST to an address that cannot exist and passes only when the guard saw and failed it.
 *
 *  Only HTTP requests are guarded. Web sockets (Chrome does not pause their opening handshake in the Fetch domain, so
 *  neither it nor its messages are seen), WebRTC and anything else that is not an HTTP request are outside this guard.
 *  Not thread-safe: use it on the thread that drives the client's pulse generator.
 */
class GUCEF_WEB_PUBLIC_CPP CWriteRequestGuard : public CORE::CNotifier
{
    public:

    static const CORE::CEvent WriteRequestRefusedEvent;

    /** The host of the requests of VerifyEnforcing(): a reserved name that never resolves */
    static const CORE::CString ProbeHost;

    static void RegisterEvents( void );

    CWriteRequestGuard( void );

    virtual ~CWriteRequestGuard();

    /** False, keeping the rules as they were, when a rule does not parse */
    bool SetSettings( const CWriteRequestGuardSettings& settings );

    const CWriteRequestGuardSettings& GetSettings( void ) const;

    /** Exactly GET, HEAD or OPTIONS: always passed */
    static bool IsSafeMethod( const CORE::CString& method );

    /**
     *  Does the guard let this request pass? Only a check, nothing is reported.
     *  @param reason   the reason of a refusal, see CWriteRequestRefusal
     */
    bool IsAllowed( const CORE::CString& method, const CORE::CString& url, CORE::CString* reason = GUCEF_NULL ) const;

    /**
     *  Is the guard in force: given to a navigation guard that is started, whose browser accepted the interception of
     *  every request, over a connection that is still open
     */
    bool IsEnforcing( void ) const;

    /**
     *  Proves that the guard is in force for the page of a session: the page is made to send a POST (no body of
     *  interest, no credentials) to ProbeHost, and the check passes only when the guard saw that request and failed it.
     *  A page that cannot send it (its content security policy, a replaced fetch) fails the check, which is the safe
     *  side. Waits for the browser's answer by pumping the client's pulse generator.
     */
    bool VerifyEnforcing( const CORE::CString& pageSessionId, UInt32 timeoutInMs = 10000 );

    /** Requests refused, not counting those of VerifyEnforcing() */
    UInt32 GetRefusedCount( void ) const;

    /** Requests other than GET, HEAD and OPTIONS that an allow rule let pass */
    UInt32 GetAllowedWriteCount( void ) const;

    /** GET, HEAD and OPTIONS requests that passed */
    UInt32 GetPassedReadCount( void ) const;

    const TWriteRequestRefusalVector& GetRecentRefusals( void ) const;

    void ClearRecentRefusals( void );

    /** scheme://host/path-template?name={redacted}&...; the host (with the port) and the path template on request */
    static CORE::CString MakeRedactedUrl( const CORE::CString& url, CORE::CString* host = GUCEF_NULL, CORE::CString* pathTemplate = GUCEF_NULL );

    private:

    friend class CNavigationGuard;

    CWriteRequestGuard( const CWriteRequestGuard& src );                /**< not implemented */
    CWriteRequestGuard& operator=( const CWriteRequestGuard& src );     /**< not implemented */

    /** By the navigation guard that enforces this guard: who hosts it, over which client, and whether it is in force */
    void SetHost( CNavigationGuard* host, CChromeDevToolsClient* client, bool enforcing );

    /** By the hosting navigation guard, for each paused request: true to continue it, false to fail it */
    bool CheckPausedRequest( const CORE::CString& method, const CORE::CString& url, const CORE::CString& resourceType,
                             const CORE::CString& frameId, bool isMainFrame, bool redirected );

    void Report( const CORE::CString& method, const CORE::CString& url, const CORE::CString& resourceType,
                 const CORE::CString& frameId, bool isMainFrame, bool redirected, const CORE::CString& reason );

    CWriteRequestGuardSettings m_settings;
    TWriteRequestRuleVector m_allow;
    CNavigationGuard* m_host;
    CChromeDevToolsClient* m_client;
    bool m_enforcing;
    CORE::CString m_probePath;          /**< the path of the probe of a VerifyEnforcing() under way */
    bool m_probeSeen;
    UInt32 m_refusedCount;
    UInt32 m_allowedWriteCount;
    UInt32 m_passedReadCount;
    TWriteRequestRefusalVector m_recent;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWRITEREQUESTGUARD_H ? */
