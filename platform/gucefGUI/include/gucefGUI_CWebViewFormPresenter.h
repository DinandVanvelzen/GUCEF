/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CWEBVIEWFORMPRESENTER_H
#define GUCEF_GUI_CWEBVIEWFORMPRESENTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_WEB_CIWEBVIEWPRESENTER_H
#include "gucefWEB_CIWebViewPresenter.h"
#define GUCEF_WEB_CIWEBVIEWPRESENTER_H
#endif /* GUCEF_WEB_CIWEBVIEWPRESENTER_H ? */

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CGUIContext;
class CWebViewForm;

/**
 *  Shows web views on request of code without a user interface, in a CWebViewForm in a window of one GUI context: the
 *  WEB::CIWebViewPresenter of a GUI application (a sign-in on a task thread presents its view through it when it
 *  needs the person).
 *
 *  Present(), UpdatePrompt() and Dismiss() may be called from any thread: they are put in a mailbox and carried out on
 *  the FrameStartedEvent of the context (the GUI thread). Present() opens the form as a window (modal when asked) of 60 %
 *  of the context, at least the form's minimum, attaches the view and gives the page the keyboard focus. Cancel or
 *  Escape on the chrome closes it and notifies PresentationClosedEvent (on the GUI thread); Dismiss() closes it without
 *  the event. A form is never destroyed inside a notification of itself: closing happens at the next frame.
 *  The presenter registers the GUI thread as one that must not wait for a web view.
 */
class GUCEF_GUI_PUBLIC_CPP CWebViewFormPresenter : public WEB::CIWebViewPresenter
{
    public:

    explicit CWebViewFormPresenter( CGUIContext* context );

    virtual ~CWebViewFormPresenter();

    virtual bool Present( const WEB::TWebViewPtr& view, const WEB::CWebViewPresentation& presentation ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void UpdatePrompt( const CString& prompt ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Dismiss( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsPresenting( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Carries out what was asked; the presenter does it at every FrameStartedEvent of its context itself */
    void ProcessRequests( void );

    /** The form shown now, NULL when none; GUI thread */
    CWebViewForm* GetForm( void );

    /** Lets go of the context before it goes: what is shown is closed */
    void Shutdown( void );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    enum ERequestKind
    {
        REQUEST_PRESENT = 0 ,
        REQUEST_PROMPT      ,
        REQUEST_DISMISS
    };

    struct SRequest
    {
        ERequestKind kind;
        WEB::TWebViewPtr view;
        WEB::CWebViewPresentation presentation;
        CString prompt;
    };
    typedef GUCEF::vector< SRequest > TRequestVector;
    typedef CORE::CTEventHandlerFunctor< CWebViewFormPresenter > TEventCallback;

    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFormCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void Queue( const SRequest& request );
    void Open( const SRequest& request );
    void Close( bool notify );

    CWebViewFormPresenter( const CWebViewFormPresenter& src );
    CWebViewFormPresenter& operator=( const CWebViewFormPresenter& src );

    private:

    CGUIContext* m_context;
    mutable MT::CMutex m_lock;
    TRequestVector m_requests;
    TRequestVector m_processing;
    CWebViewForm* m_form;
    bool m_cancelled;
    MT::CTAtomic< UInt32 > m_presenting;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CWEBVIEWFORMPRESENTER_H ? */
