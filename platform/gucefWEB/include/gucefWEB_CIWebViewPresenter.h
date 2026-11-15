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

#ifndef GUCEF_WEB_CIWEBVIEWPRESENTER_H
#define GUCEF_WEB_CIWEBVIEWPRESENTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_WEB_IWEBVIEW_H
#include "gucefWEB_IWebView.h"
#define GUCEF_WEB_IWEBVIEW_H
#endif /* GUCEF_WEB_IWEBVIEW_H ? */

#ifndef GUCEF_WEB_CWEBVIEWPRESENTATION_H
#include "gucefWEB_CWebViewPresentation.h"
#define GUCEF_WEB_CWEBVIEWPRESENTATION_H
#endif /* GUCEF_WEB_CWEBVIEWPRESENTATION_H ? */

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
 *  Shows a view to a person on request of code that has no user interface (a sign-in on a task thread). A GUI
 *  implements it (GUI::CWebViewFormPresenter shows the view in a CWebViewForm in a window of a context); an application
 *  without a GUI installs none, and code that needs a person then knows embedded interaction is not available.
 *  Calls may come from any thread; the presenter carries them out on its own thread.
 */
class GUCEF_WEB_PUBLIC_CPP CIWebViewPresenter : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent PresentationClosedEvent;      /**< the person pressed Cancel or closed the window; no data. Notified on the presenter's thread */

    static void RegisterEvents( void );

    CIWebViewPresenter( void );

    virtual ~CIWebViewPresenter();

    /** Shows the view; a view shown already is replaced. False when the presenter cannot show anything (no context) */
    virtual bool Present( const TWebViewPtr& view, const CWebViewPresentation& presentation ) = 0;

    /** Changes the prompt of what is shown */
    virtual void UpdatePrompt( const CString& prompt ) = 0;

    /** Closes what is shown and lets go of the view; no PresentationClosedEvent */
    virtual void Dismiss( void ) = 0;

    /** Whether a view is shown or about to be */
    virtual bool IsPresenting( void ) const = 0;

    private:

    CIWebViewPresenter( const CIWebViewPresenter& src );                /**< not implemented */
    CIWebViewPresenter& operator=( const CIWebViewPresenter& src );     /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CIWEBVIEWPRESENTER_H ? */
