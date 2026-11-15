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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefWEB_WebViewTypeNames.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* ModeNames[] = { "offscreen", "nativeWindow" };
const char* SecurityNames[] = { "none", "secure", "insecure", "certificateError" };
const char* ErrorNames[] = { "none", "network", "refusedByPolicy", "certificate", "cancelled", "processEnded", "shuttingDown",
                             "engineStalled", "notSupported", "viewClosed", "script", "resultTooLarge", "invalidArgument", "other" };
const char* DecisionNames[] = { "allow", "refuse", "ask" };
const char* NavigationNames[] = { "any", "allowList" };
const char* PopupNames[] = { "refuse", "sameView", "ask" };
const char* DownloadNames[] = { "refuse", "ask" };
const char* PermissionPolicyNames[] = { "denyAll", "ask" };
const char* CertificateNames[] = { "refuse", "ask" };
const char* DialogPolicyNames[] = { "show", "refuse" };
const char* ThirdPartyNames[] = { "block", "allow" };
const char* ProxyNames[] = { "none", "system", "explicit" };
const char* AccelerationNames[] = { "never", "auto" };
const char* PermissionKindNames[] = { "camera", "microphone", "geolocation", "notifications", "clipboard", "other" };
const char* DialogKindNames[] = { "alert", "confirm", "prompt", "beforeUnload" };
const char* ConsoleNames[] = { "debug", "info", "warning", "error" };
const char* TerminationNames[] = { "crash", "outOfMemory", "killed" };
const char* EditNames[] = { "cut", "copy", "paste", "selectAll", "undo", "redo" };
const char* InputNames[] = { "none", "pointerMove", "pointerDown", "pointerUp", "pointerLeave", "wheel", "keyDown", "keyUp", "text", "focusIn", "focusOut" };
const char* SameSiteNames[] = { "unset", "none", "lax", "strict" };

/** The name of a value, "unknown" for one outside the table */
const char*
NameOf( const char** names, UInt32 count, Int32 value )
{
    return ( value >= 0 && (UInt32) value < count ) ? names[ value ] : "unknown";
}

/** The value of a name in any case, the fallback for an unknown name */
Int32
ValueOf( const char** names, UInt32 count, const CString& name, Int32 fallback )
{
    for ( UInt32 i=0; i<count; ++i )
        if ( name.Equals( names[ i ], false ) )
            return (Int32) i;
    return fallback;
}

#define GUCEF_WEB_COUNT( table ) ( (UInt32) ( sizeof( table ) / sizeof( table[ 0 ] ) ) )

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* WebViewModeToString( TWebViewMode value ) {GUCEF_TRACE; return NameOf( ModeNames, GUCEF_WEB_COUNT( ModeNames ), value ); }
TWebViewMode WebViewModeFromString( const CString& name, TWebViewMode fallback ) {GUCEF_TRACE; return (TWebViewMode) ValueOf( ModeNames, GUCEF_WEB_COUNT( ModeNames ), name, fallback ); }

const char* WebViewSecurityStateToString( TWebViewSecurityState value ) {GUCEF_TRACE; return NameOf( SecurityNames, GUCEF_WEB_COUNT( SecurityNames ), value ); }

const char* WebViewErrorToString( TWebViewError value ) {GUCEF_TRACE; return NameOf( ErrorNames, GUCEF_WEB_COUNT( ErrorNames ), value ); }

const char* WebViewDecisionToString( TWebViewDecision value ) {GUCEF_TRACE; return NameOf( DecisionNames, GUCEF_WEB_COUNT( DecisionNames ), value ); }

const char* WebViewNavigationPolicyToString( TWebViewNavigationPolicy value ) {GUCEF_TRACE; return NameOf( NavigationNames, GUCEF_WEB_COUNT( NavigationNames ), value ); }
TWebViewNavigationPolicy WebViewNavigationPolicyFromString( const CString& name, TWebViewNavigationPolicy fallback ) {GUCEF_TRACE; return (TWebViewNavigationPolicy) ValueOf( NavigationNames, GUCEF_WEB_COUNT( NavigationNames ), name, fallback ); }

const char* WebViewPopupPolicyToString( TWebViewPopupPolicy value ) {GUCEF_TRACE; return NameOf( PopupNames, GUCEF_WEB_COUNT( PopupNames ), value ); }
TWebViewPopupPolicy WebViewPopupPolicyFromString( const CString& name, TWebViewPopupPolicy fallback ) {GUCEF_TRACE; return (TWebViewPopupPolicy) ValueOf( PopupNames, GUCEF_WEB_COUNT( PopupNames ), name, fallback ); }

const char* WebViewDownloadPolicyToString( TWebViewDownloadPolicy value ) {GUCEF_TRACE; return NameOf( DownloadNames, GUCEF_WEB_COUNT( DownloadNames ), value ); }
TWebViewDownloadPolicy WebViewDownloadPolicyFromString( const CString& name, TWebViewDownloadPolicy fallback ) {GUCEF_TRACE; return (TWebViewDownloadPolicy) ValueOf( DownloadNames, GUCEF_WEB_COUNT( DownloadNames ), name, fallback ); }

const char* WebViewPermissionPolicyToString( TWebViewPermissionPolicy value ) {GUCEF_TRACE; return NameOf( PermissionPolicyNames, GUCEF_WEB_COUNT( PermissionPolicyNames ), value ); }
TWebViewPermissionPolicy WebViewPermissionPolicyFromString( const CString& name, TWebViewPermissionPolicy fallback ) {GUCEF_TRACE; return (TWebViewPermissionPolicy) ValueOf( PermissionPolicyNames, GUCEF_WEB_COUNT( PermissionPolicyNames ), name, fallback ); }

const char* WebViewCertificateErrorPolicyToString( TWebViewCertificateErrorPolicy value ) {GUCEF_TRACE; return NameOf( CertificateNames, GUCEF_WEB_COUNT( CertificateNames ), value ); }
TWebViewCertificateErrorPolicy WebViewCertificateErrorPolicyFromString( const CString& name, TWebViewCertificateErrorPolicy fallback ) {GUCEF_TRACE; return (TWebViewCertificateErrorPolicy) ValueOf( CertificateNames, GUCEF_WEB_COUNT( CertificateNames ), name, fallback ); }

const char* WebViewScriptDialogPolicyToString( TWebViewScriptDialogPolicy value ) {GUCEF_TRACE; return NameOf( DialogPolicyNames, GUCEF_WEB_COUNT( DialogPolicyNames ), value ); }
TWebViewScriptDialogPolicy WebViewScriptDialogPolicyFromString( const CString& name, TWebViewScriptDialogPolicy fallback ) {GUCEF_TRACE; return (TWebViewScriptDialogPolicy) ValueOf( DialogPolicyNames, GUCEF_WEB_COUNT( DialogPolicyNames ), name, fallback ); }

const char* WebViewThirdPartyCookiesToString( TWebViewThirdPartyCookies value ) {GUCEF_TRACE; return NameOf( ThirdPartyNames, GUCEF_WEB_COUNT( ThirdPartyNames ), value ); }
TWebViewThirdPartyCookies WebViewThirdPartyCookiesFromString( const CString& name, TWebViewThirdPartyCookies fallback ) {GUCEF_TRACE; return (TWebViewThirdPartyCookies) ValueOf( ThirdPartyNames, GUCEF_WEB_COUNT( ThirdPartyNames ), name, fallback ); }

const char* WebViewProxyModeToString( TWebViewProxyMode value ) {GUCEF_TRACE; return NameOf( ProxyNames, GUCEF_WEB_COUNT( ProxyNames ), value ); }
TWebViewProxyMode WebViewProxyModeFromString( const CString& name, TWebViewProxyMode fallback ) {GUCEF_TRACE; return (TWebViewProxyMode) ValueOf( ProxyNames, GUCEF_WEB_COUNT( ProxyNames ), name, fallback ); }

const char* WebViewHardwareAccelerationToString( TWebViewHardwareAcceleration value ) {GUCEF_TRACE; return NameOf( AccelerationNames, GUCEF_WEB_COUNT( AccelerationNames ), value ); }
TWebViewHardwareAcceleration WebViewHardwareAccelerationFromString( const CString& name, TWebViewHardwareAcceleration fallback ) {GUCEF_TRACE; return (TWebViewHardwareAcceleration) ValueOf( AccelerationNames, GUCEF_WEB_COUNT( AccelerationNames ), name, fallback ); }

const char* WebViewPermissionKindToString( TWebViewPermissionKind value ) {GUCEF_TRACE; return NameOf( PermissionKindNames, GUCEF_WEB_COUNT( PermissionKindNames ), value ); }
const char* WebViewScriptDialogKindToString( TWebViewScriptDialogKind value ) {GUCEF_TRACE; return NameOf( DialogKindNames, GUCEF_WEB_COUNT( DialogKindNames ), value ); }
const char* WebViewConsoleLevelToString( TWebViewConsoleLevel value ) {GUCEF_TRACE; return NameOf( ConsoleNames, GUCEF_WEB_COUNT( ConsoleNames ), value ); }
const char* WebViewTerminationToString( TWebViewTermination value ) {GUCEF_TRACE; return NameOf( TerminationNames, GUCEF_WEB_COUNT( TerminationNames ), value ); }
const char* WebViewEditCommandToString( TWebViewEditCommand value ) {GUCEF_TRACE; return NameOf( EditNames, GUCEF_WEB_COUNT( EditNames ), value ); }
const char* WebViewInputKindToString( TWebViewInputKind value ) {GUCEF_TRACE; return NameOf( InputNames, GUCEF_WEB_COUNT( InputNames ), value ); }
const char* WebViewSameSiteToString( TWebViewSameSite value ) {GUCEF_TRACE; return NameOf( SameSiteNames, GUCEF_WEB_COUNT( SameSiteNames ), value ); }
TWebViewSameSite WebViewSameSiteFromString( const CString& name, TWebViewSameSite fallback ) {GUCEF_TRACE; return (TWebViewSameSite) ValueOf( SameSiteNames, GUCEF_WEB_COUNT( SameSiteNames ), name, fallback ); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
