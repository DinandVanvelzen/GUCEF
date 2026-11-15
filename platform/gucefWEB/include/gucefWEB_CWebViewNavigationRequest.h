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

#ifndef GUCEF_WEB_CWEBVIEWNAVIGATIONREQUEST_H
#define GUCEF_WEB_CWEBVIEWNAVIGATIONREQUEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A navigation and what the policy decided about it: IWebView::NavigationRequestedEvent.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewNavigationRequest
{
    public:

    CString url;
    bool mainFrame;                 /**< false for a frame in the page */
    bool userGesture;               /**< caused by a click or a key of the person */
    bool byApplication;             /**< asked with IWebView::Navigate() */
    TWebViewDecision decision;      /**< what the policy decided; WEBVIEWDECISION_ASK when the owner answers */
    UInt32 decisionId;              /**< the id to answer with IWebView::ResolveDecision(); 0 when not deferred */

    CWebViewNavigationRequest( void );
};

/** The data of the event: a copy of the CWebViewNavigationRequest */
typedef CORE::CTCloneableObj< CWebViewNavigationRequest > TWebViewNavigationRequestEventData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWNAVIGATIONREQUEST_H ? */
