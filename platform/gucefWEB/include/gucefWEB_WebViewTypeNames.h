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

#ifndef GUCEF_WEB_WEBVIEWTYPENAMES_H
#define GUCEF_WEB_WEBVIEWTYPENAMES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

#ifndef GUCEF_WEB_WEBVIEWTYPES_H
#include "gucefWEB_WebViewTypes.h"
#define GUCEF_WEB_WEBVIEWTYPES_H
#endif /* GUCEF_WEB_WEBVIEWTYPES_H ? */

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

/*
 *  The names of the enumerations of gucefWEB_WebViewTypes.h as they are written in settings and reports.
 *  The FromString functions take the name in any case and give 'fallback' for a name they do not know.
 */

GUCEF_WEB_PUBLIC_CPP const char* WebViewModeToString( TWebViewMode value );
GUCEF_WEB_PUBLIC_CPP TWebViewMode WebViewModeFromString( const CString& name, TWebViewMode fallback );

GUCEF_WEB_PUBLIC_CPP const char* WebViewSecurityStateToString( TWebViewSecurityState value );

GUCEF_WEB_PUBLIC_CPP const char* WebViewErrorToString( TWebViewError value );

GUCEF_WEB_PUBLIC_CPP const char* WebViewDecisionToString( TWebViewDecision value );

GUCEF_WEB_PUBLIC_CPP const char* WebViewNavigationPolicyToString( TWebViewNavigationPolicy value );
GUCEF_WEB_PUBLIC_CPP TWebViewNavigationPolicy WebViewNavigationPolicyFromString( const CString& name, TWebViewNavigationPolicy fallback );

GUCEF_WEB_PUBLIC_CPP const char* WebViewPopupPolicyToString( TWebViewPopupPolicy value );
GUCEF_WEB_PUBLIC_CPP TWebViewPopupPolicy WebViewPopupPolicyFromString( const CString& name, TWebViewPopupPolicy fallback );

GUCEF_WEB_PUBLIC_CPP const char* WebViewDownloadPolicyToString( TWebViewDownloadPolicy value );
GUCEF_WEB_PUBLIC_CPP TWebViewDownloadPolicy WebViewDownloadPolicyFromString( const CString& name, TWebViewDownloadPolicy fallback );

GUCEF_WEB_PUBLIC_CPP const char* WebViewPermissionPolicyToString( TWebViewPermissionPolicy value );
GUCEF_WEB_PUBLIC_CPP TWebViewPermissionPolicy WebViewPermissionPolicyFromString( const CString& name, TWebViewPermissionPolicy fallback );

GUCEF_WEB_PUBLIC_CPP const char* WebViewCertificateErrorPolicyToString( TWebViewCertificateErrorPolicy value );
GUCEF_WEB_PUBLIC_CPP TWebViewCertificateErrorPolicy WebViewCertificateErrorPolicyFromString( const CString& name, TWebViewCertificateErrorPolicy fallback );

GUCEF_WEB_PUBLIC_CPP const char* WebViewScriptDialogPolicyToString( TWebViewScriptDialogPolicy value );
GUCEF_WEB_PUBLIC_CPP TWebViewScriptDialogPolicy WebViewScriptDialogPolicyFromString( const CString& name, TWebViewScriptDialogPolicy fallback );

GUCEF_WEB_PUBLIC_CPP const char* WebViewThirdPartyCookiesToString( TWebViewThirdPartyCookies value );
GUCEF_WEB_PUBLIC_CPP TWebViewThirdPartyCookies WebViewThirdPartyCookiesFromString( const CString& name, TWebViewThirdPartyCookies fallback );

GUCEF_WEB_PUBLIC_CPP const char* WebViewProxyModeToString( TWebViewProxyMode value );
GUCEF_WEB_PUBLIC_CPP TWebViewProxyMode WebViewProxyModeFromString( const CString& name, TWebViewProxyMode fallback );

GUCEF_WEB_PUBLIC_CPP const char* WebViewHardwareAccelerationToString( TWebViewHardwareAcceleration value );
GUCEF_WEB_PUBLIC_CPP TWebViewHardwareAcceleration WebViewHardwareAccelerationFromString( const CString& name, TWebViewHardwareAcceleration fallback );

GUCEF_WEB_PUBLIC_CPP const char* WebViewPermissionKindToString( TWebViewPermissionKind value );
GUCEF_WEB_PUBLIC_CPP const char* WebViewScriptDialogKindToString( TWebViewScriptDialogKind value );
GUCEF_WEB_PUBLIC_CPP const char* WebViewConsoleLevelToString( TWebViewConsoleLevel value );
GUCEF_WEB_PUBLIC_CPP const char* WebViewTerminationToString( TWebViewTermination value );
GUCEF_WEB_PUBLIC_CPP const char* WebViewEditCommandToString( TWebViewEditCommand value );
GUCEF_WEB_PUBLIC_CPP const char* WebViewInputKindToString( TWebViewInputKind value );
GUCEF_WEB_PUBLIC_CPP const char* WebViewSameSiteToString( TWebViewSameSite value );
GUCEF_WEB_PUBLIC_CPP TWebViewSameSite WebViewSameSiteFromString( const CString& name, TWebViewSameSite fallback );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_WEBVIEWTYPENAMES_H ? */
