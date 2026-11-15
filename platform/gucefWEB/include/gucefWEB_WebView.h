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

/*
 *  This is the main include header of the web views of the gucefWEB library.
 *  It is part of a platform called GUCEF.
 *  All classes it brings in have to do with web views: pages of a web engine shown in an application or used
 *  offscreen, with engines as plugins
 */

#ifndef GUCEF_WEB_WEBVIEW_H
#define GUCEF_WEB_WEBVIEW_H

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

#ifdef __cplusplus

#ifndef GUCEF_WEB_WEBVIEWTYPENAMES_H
#include "gucefWEB_WebViewTypeNames.h"
#define GUCEF_WEB_WEBVIEWTYPENAMES_H
#endif /* GUCEF_WEB_WEBVIEWTYPENAMES_H ? */

#ifndef GUCEF_WEB_CWEBVIEWRECT_H
#include "gucefWEB_CWebViewRect.h"
#define GUCEF_WEB_CWEBVIEWRECT_H
#endif /* GUCEF_WEB_CWEBVIEWRECT_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCAPABILITIES_H
#include "gucefWEB_CWebViewCapabilities.h"
#define GUCEF_WEB_CWEBVIEWCAPABILITIES_H
#endif /* GUCEF_WEB_CWEBVIEWCAPABILITIES_H ? */

#ifndef GUCEF_WEB_CWEBVIEWHOSTALLOWLIST_H
#include "gucefWEB_CWebViewHostAllowList.h"
#define GUCEF_WEB_CWEBVIEWHOSTALLOWLIST_H
#endif /* GUCEF_WEB_CWEBVIEWHOSTALLOWLIST_H ? */

#ifndef GUCEF_WEB_CWEBVIEWPOLICY_H
#include "gucefWEB_CWebViewPolicy.h"
#define GUCEF_WEB_CWEBVIEWPOLICY_H
#endif /* GUCEF_WEB_CWEBVIEWPOLICY_H ? */

#ifndef GUCEF_WEB_CWEBVIEWPROFILESETTINGS_H
#include "gucefWEB_CWebViewProfileSettings.h"
#define GUCEF_WEB_CWEBVIEWPROFILESETTINGS_H
#endif /* GUCEF_WEB_CWEBVIEWPROFILESETTINGS_H ? */

#ifndef GUCEF_WEB_CWEBVIEWSETTINGS_H
#include "gucefWEB_CWebViewSettings.h"
#define GUCEF_WEB_CWEBVIEWSETTINGS_H
#endif /* GUCEF_WEB_CWEBVIEWSETTINGS_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCOOKIE_H
#include "gucefWEB_CWebViewCookie.h"
#define GUCEF_WEB_CWEBVIEWCOOKIE_H
#endif /* GUCEF_WEB_CWEBVIEWCOOKIE_H ? */

#ifndef GUCEF_WEB_CWEBVIEWINPUTEVENT_H
#include "gucefWEB_CWebViewInputEvent.h"
#define GUCEF_WEB_CWEBVIEWINPUTEVENT_H
#endif /* GUCEF_WEB_CWEBVIEWINPUTEVENT_H ? */

#ifndef GUCEF_WEB_WEBVIEWINPUTTYPES_H
#include "gucefWEB_WebViewInputTypes.h"
#define GUCEF_WEB_WEBVIEWINPUTTYPES_H
#endif /* GUCEF_WEB_WEBVIEWINPUTTYPES_H ? */

#ifndef GUCEF_WEB_CWEBVIEWKEYCODES_H
#include "gucefWEB_CWebViewKeyCodes.h"
#define GUCEF_WEB_CWEBVIEWKEYCODES_H
#endif /* GUCEF_WEB_CWEBVIEWKEYCODES_H ? */

#ifndef GUCEF_WEB_CWEBVIEWLOADERROR_H
#include "gucefWEB_CWebViewLoadError.h"
#define GUCEF_WEB_CWEBVIEWLOADERROR_H
#endif /* GUCEF_WEB_CWEBVIEWLOADERROR_H ? */

#ifndef GUCEF_WEB_CWEBVIEWNAVIGATIONREQUEST_H
#include "gucefWEB_CWebViewNavigationRequest.h"
#define GUCEF_WEB_CWEBVIEWNAVIGATIONREQUEST_H
#endif /* GUCEF_WEB_CWEBVIEWNAVIGATIONREQUEST_H ? */

#ifndef GUCEF_WEB_CWEBVIEWPOPUPREQUEST_H
#include "gucefWEB_CWebViewPopupRequest.h"
#define GUCEF_WEB_CWEBVIEWPOPUPREQUEST_H
#endif /* GUCEF_WEB_CWEBVIEWPOPUPREQUEST_H ? */

#ifndef GUCEF_WEB_CWEBVIEWDOWNLOADREQUEST_H
#include "gucefWEB_CWebViewDownloadRequest.h"
#define GUCEF_WEB_CWEBVIEWDOWNLOADREQUEST_H
#endif /* GUCEF_WEB_CWEBVIEWDOWNLOADREQUEST_H ? */

#ifndef GUCEF_WEB_CWEBVIEWPERMISSIONREQUEST_H
#include "gucefWEB_CWebViewPermissionRequest.h"
#define GUCEF_WEB_CWEBVIEWPERMISSIONREQUEST_H
#endif /* GUCEF_WEB_CWEBVIEWPERMISSIONREQUEST_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCERTIFICATEERROR_H
#include "gucefWEB_CWebViewCertificateError.h"
#define GUCEF_WEB_CWEBVIEWCERTIFICATEERROR_H
#endif /* GUCEF_WEB_CWEBVIEWCERTIFICATEERROR_H ? */

#ifndef GUCEF_WEB_CWEBVIEWSCRIPTDIALOG_H
#include "gucefWEB_CWebViewScriptDialog.h"
#define GUCEF_WEB_CWEBVIEWSCRIPTDIALOG_H
#endif /* GUCEF_WEB_CWEBVIEWSCRIPTDIALOG_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCONSOLEMESSAGE_H
#include "gucefWEB_CWebViewConsoleMessage.h"
#define GUCEF_WEB_CWEBVIEWCONSOLEMESSAGE_H
#endif /* GUCEF_WEB_CWEBVIEWCONSOLEMESSAGE_H ? */

#ifndef GUCEF_WEB_CWEBVIEWFRAMEINFO_H
#include "gucefWEB_CWebViewFrameInfo.h"
#define GUCEF_WEB_CWEBVIEWFRAMEINFO_H
#endif /* GUCEF_WEB_CWEBVIEWFRAMEINFO_H ? */

#ifndef GUCEF_WEB_CWEBVIEWFRAMEEXCHANGE_H
#include "gucefWEB_CWebViewFrameExchange.h"
#define GUCEF_WEB_CWEBVIEWFRAMEEXCHANGE_H
#endif /* GUCEF_WEB_CWEBVIEWFRAMEEXCHANGE_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCOMMAND_H
#include "gucefWEB_CWebViewCommand.h"
#define GUCEF_WEB_CWEBVIEWCOMMAND_H
#endif /* GUCEF_WEB_CWEBVIEWCOMMAND_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCOMMANDRESULT_H
#include "gucefWEB_CWebViewCommandResult.h"
#define GUCEF_WEB_CWEBVIEWCOMMANDRESULT_H
#endif /* GUCEF_WEB_CWEBVIEWCOMMANDRESULT_H ? */

#ifndef GUCEF_WEB_IWEBVIEW_H
#include "gucefWEB_IWebView.h"
#define GUCEF_WEB_IWEBVIEW_H
#endif /* GUCEF_WEB_IWEBVIEW_H ? */

#ifndef GUCEF_WEB_IWEBVIEWENGINE_H
#include "gucefWEB_IWebViewEngine.h"
#define GUCEF_WEB_IWEBVIEWENGINE_H
#endif /* GUCEF_WEB_IWEBVIEWENGINE_H ? */

#ifndef GUCEF_WEB_CIWEBVIEWENGINELOOP_H
#include "gucefWEB_CIWebViewEngineLoop.h"
#define GUCEF_WEB_CIWEBVIEWENGINELOOP_H
#endif /* GUCEF_WEB_CIWEBVIEWENGINELOOP_H ? */

#ifndef GUCEF_WEB_CWEBVIEWSEMAPHOREENGINELOOP_H
#include "gucefWEB_CWebViewSemaphoreEngineLoop.h"
#define GUCEF_WEB_CWEBVIEWSEMAPHOREENGINELOOP_H
#endif /* GUCEF_WEB_CWEBVIEWSEMAPHOREENGINELOOP_H ? */

#ifndef GUCEF_WEB_CWEBVIEWENGINEHOST_H
#include "gucefWEB_CWebViewEngineHost.h"
#define GUCEF_WEB_CWEBVIEWENGINEHOST_H
#endif /* GUCEF_WEB_CWEBVIEWENGINEHOST_H ? */

#ifndef GUCEF_WEB_CWEBVIEWBASE_H
#include "gucefWEB_CWebViewBase.h"
#define GUCEF_WEB_CWEBVIEWBASE_H
#endif /* GUCEF_WEB_CWEBVIEWBASE_H ? */

#ifndef GUCEF_WEB_CWEBVIEWENGINEBASE_H
#include "gucefWEB_CWebViewEngineBase.h"
#define GUCEF_WEB_CWEBVIEWENGINEBASE_H
#endif /* GUCEF_WEB_CWEBVIEWENGINEBASE_H ? */

#ifndef GUCEF_WEB_CWEBVIEWENGINEREGISTRY_H
#include "gucefWEB_CWebViewEngineRegistry.h"
#define GUCEF_WEB_CWEBVIEWENGINEREGISTRY_H
#endif /* GUCEF_WEB_CWEBVIEWENGINEREGISTRY_H ? */

#ifndef GUCEF_WEB_CWEBVIEWENGINESELECTION_H
#include "gucefWEB_CWebViewEngineSelection.h"
#define GUCEF_WEB_CWEBVIEWENGINESELECTION_H
#endif /* GUCEF_WEB_CWEBVIEWENGINESELECTION_H ? */

#ifndef GUCEF_WEB_CWEBVIEWGLOBALSETTINGS_H
#include "gucefWEB_CWebViewGlobalSettings.h"
#define GUCEF_WEB_CWEBVIEWGLOBALSETTINGS_H
#endif /* GUCEF_WEB_CWEBVIEWGLOBALSETTINGS_H ? */

#ifndef GUCEF_WEB_CWEBVIEWGLOBAL_H
#include "gucefWEB_CWebViewGlobal.h"
#define GUCEF_WEB_CWEBVIEWGLOBAL_H
#endif /* GUCEF_WEB_CWEBVIEWGLOBAL_H ? */

#ifndef GUCEF_WEB_CWEBVIEWEVENTRELAY_H
#include "gucefWEB_CWebViewEventRelay.h"
#define GUCEF_WEB_CWEBVIEWEVENTRELAY_H
#endif /* GUCEF_WEB_CWEBVIEWEVENTRELAY_H ? */

#ifndef GUCEF_WEB_CWEBVIEWPRESENTATION_H
#include "gucefWEB_CWebViewPresentation.h"
#define GUCEF_WEB_CWEBVIEWPRESENTATION_H
#endif /* GUCEF_WEB_CWEBVIEWPRESENTATION_H ? */

#ifndef GUCEF_WEB_CIWEBVIEWPRESENTER_H
#include "gucefWEB_CIWebViewPresenter.h"
#define GUCEF_WEB_CIWEBVIEWPRESENTER_H
#endif /* GUCEF_WEB_CIWEBVIEWPRESENTER_H ? */

#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_WEBVIEW_H ? */
