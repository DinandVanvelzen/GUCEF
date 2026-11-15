/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_WEB_CWEBVIEWPOLICY_H
#define GUCEF_WEB_CWEBVIEWPOLICY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CWEBVIEWHOSTALLOWLIST_H
#include "gucefWEB_CWebViewHostAllowList.h"
#define GUCEF_WEB_CWEBVIEWHOSTALLOWLIST_H
#endif /* GUCEF_WEB_CWEBVIEWHOSTALLOWLIST_H ? */

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
 *  What a view may do and what it refuses, applied by the engine on its own thread at once. Only what the policy marks
 *  'ask' is deferred to the owner of the view (IWebView::ResolveDecision()). Refused navigations are reported
 *  (IWebView::NavigationRequestedEvent with the decision, a load failure for a main frame), never silently dropped.
 *  Nothing in it is secret.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewPolicy : public CORE::CIConfigurable
{
    public:

    TWebViewNavigationPolicy navigation;            /**< any host, or the hosts of allowList */
    CWebViewHostAllowList allowList;                /**< the hosts of main frames (and subframes) with WEBVIEWNAVIGATION_ALLOWLIST */
    CWebViewHostAllowList subframeHosts;            /**< hosts allowed in frames in addition (challenge providers) */
    TWebViewPopupPolicy popups;
    TWebViewDownloadPolicy downloads;
    TWebViewPermissionPolicy permissions;
    TWebViewCertificateErrorPolicy certificateErrors;
    TWebViewScriptDialogPolicy scriptDialogs;
    bool defaultWorldScripts;                       /**< scripts of the application may run in the world of the page (off: isolated worlds only) */

    CWebViewPolicy( void );

    virtual ~CWebViewPolicy();

    /**
     *  Makes the policy consistent; with 'forSignIn' also safe for a sign-in profile: certificate errors are refused
     *  (asking is not allowed there) and scripts stay out of the page's world. True when nothing had to change.
     */
    bool Sanitize( bool forSignIn );

    /**
     *  Whether a navigation may happen. about:blank always; data: only when the application asked for it (byApplication)
     *  and only in a main frame; http and https by the navigation policy, a subframe also when subframeHosts allows it;
     *  everything else is refused.
     */
    TWebViewDecision DecideNavigation( const CString& url, bool mainFrame, bool byApplication ) const;

    /** A popup: refused, opened in the same view (when the address may be navigated to) or asked */
    TWebViewDecision DecidePopup( const CString& url ) const;

    TWebViewDecision DecideDownload( void ) const;

    TWebViewDecision DecidePermission( TWebViewPermissionKind kind ) const;

    /** Never WEBVIEWDECISION_ALLOW */
    TWebViewDecision DecideCertificateError( void ) const;

    /** Shown dialogs are deferred to the owner (WEBVIEWDECISION_ASK), refused ones answered with cancel */
    TWebViewDecision DecideScriptDialog( void ) const;

    /** Whether a script of the application may run in the world named (empty: the page's own world) */
    bool IsScriptWorldAllowed( const CString& world ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Values that are not there keep what they were; names that are not known keep the value too */
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWPOLICY_H ? */
