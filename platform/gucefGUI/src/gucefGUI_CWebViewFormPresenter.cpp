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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_WEB_CWEBVIEWGLOBAL_H
#include "gucefWEB_CWebViewGlobal.h"
#define GUCEF_WEB_CWEBVIEWGLOBAL_H
#endif /* GUCEF_WEB_CWEBVIEWGLOBAL_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CWEBVIEWFORM_H
#include "gucefGUI_CWebViewForm.h"
#define GUCEF_GUI_CWEBVIEWFORM_H
#endif /* GUCEF_GUI_CWEBVIEWFORM_H ? */

#ifndef GUCEF_GUI_CWEBPAGEVIEW_H
#include "gucefGUI_CWebPageView.h"
#define GUCEF_GUI_CWEBPAGEVIEW_H
#endif /* GUCEF_GUI_CWEBPAGEVIEW_H ? */

#include "gucefGUI_CWebViewFormPresenter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWebViewFormPresenter::CWebViewFormPresenter( CGUIContext* context )
    : WEB::CIWebViewPresenter()
    , m_context( context )
    , m_lock()
    , m_requests()
    , m_processing()
    , m_form( GUCEF_NULL )
    , m_cancelled( false )
    , m_presenting( 0 )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_context )
    {
        TEventCallback callback( this, &CWebViewFormPresenter::OnFrameStarted );
        SubscribeTo( m_context, CGUIContext::FrameStartedEvent, callback );
    }
    // the presenter is made on the GUI thread, which must never wait for a web view
    WEB::CWebViewGlobal::Instance()->SetThreadMustNotWait( MT::GetCurrentThreadID(), true );
}

/*-------------------------------------------------------------------------*/

CWebViewFormPresenter::~CWebViewFormPresenter()
{GUCEF_TRACE;

    Shutdown();
    WEB::CWebViewGlobal::Instance()->SetThreadMustNotWait( MT::GetCurrentThreadID(), false );
}

/*-------------------------------------------------------------------------*/

void
CWebViewFormPresenter::Shutdown( void )
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    Close( false );
    m_context = GUCEF_NULL;
    MT::CScopeMutex lock( m_lock );
    m_requests.clear();
}

/*-------------------------------------------------------------------------*/

void
CWebViewFormPresenter::Queue( const SRequest& request )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_requests.push_back( request );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewFormPresenter::Present( const WEB::TWebViewPtr& view, const WEB::CWebViewPresentation& presentation )
{GUCEF_TRACE;

    if ( view.IsNULL() || GUCEF_NULL == m_context )
        return false;
    SRequest request;
    request.kind = REQUEST_PRESENT;
    request.view = view;
    request.presentation = presentation;
    m_presenting.Store( 1 );
    Queue( request );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebViewFormPresenter::UpdatePrompt( const CString& prompt )
{GUCEF_TRACE;

    SRequest request;
    request.kind = REQUEST_PROMPT;
    request.prompt = prompt;
    Queue( request );
}

/*-------------------------------------------------------------------------*/

void
CWebViewFormPresenter::Dismiss( void )
{GUCEF_TRACE;

    SRequest request;
    request.kind = REQUEST_DISMISS;
    m_presenting.Store( 0 );
    Queue( request );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewFormPresenter::IsPresenting( void ) const
{GUCEF_TRACE;

    return 0 != m_presenting.Load();
}

/*-------------------------------------------------------------------------*/

CWebViewForm*
CWebViewFormPresenter::GetForm( void )
{GUCEF_TRACE;

    return m_form;
}

/*-------------------------------------------------------------------------*/

void
CWebViewFormPresenter::OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ProcessRequests();
}

/*-------------------------------------------------------------------------*/

void
CWebViewFormPresenter::ProcessRequests( void )
{GUCEF_TRACE;

    // a Cancel of the last frame: closed now, outside the notification of the form
    if ( m_cancelled )
    {
        m_cancelled = false;
        Close( false );
        m_presenting.Store( 0 );
        NotifyObservers( PresentationClosedEvent );
    }
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_requests.empty() )
            return;
        m_processing.swap( m_requests );
    }
    for ( UInt32 i=0; i<(UInt32)m_processing.size(); ++i )
    {
        const SRequest& request = m_processing[ i ];
        switch ( request.kind )
        {
            case REQUEST_PRESENT: Open( request ); break;
            case REQUEST_PROMPT: if ( GUCEF_NULL != m_form ) m_form->SetPrompt( request.prompt ); break;
            default: Close( false ); break;
        }
    }
    m_processing.clear();
}

/*-------------------------------------------------------------------------*/

void
CWebViewFormPresenter::Open( const SRequest& request )
{GUCEF_TRACE;

    Close( false );
    if ( GUCEF_NULL == m_context )
        return;
    m_form = GUCEF_NEW CWebViewForm( m_context );
    m_form->ApplyPresentation( request.presentation );
    if ( !m_form->Build( request.presentation.title.IsNULLOrEmpty() ? CString( "Web page" ) : request.presentation.title ) )
    {
        GUCEF_DELETE m_form;
        m_form = GUCEF_NULL;
        m_presenting.Store( 0 );
        return;
    }
    m_form->ApplyPresentation( request.presentation );

    // 60 % of the context, at least the minimum of the form
    CWidget* root = m_form->GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        root->SetProperty( EWidgetProperty_Position, "0.2,0 0.2,0" );
        root->SetProperty( EWidgetProperty_Size, "0.6,0 0.6,0" );
    }
    if ( request.presentation.modal )
        m_form->ShowModal();
    TEventCallback callback( this, &CWebViewFormPresenter::OnFormCancelled );
    SubscribeTo( m_form, CWebViewForm::CancelledEvent, callback );
    m_form->Attach( request.view );
    CWebPageView* page = m_form->GetPageView();
    if ( GUCEF_NULL != page )
        m_context->SetFocus( page );
}

/*-------------------------------------------------------------------------*/

void
CWebViewFormPresenter::Close( bool notify )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_form )
        return;
    UnsubscribeFrom( m_form );
    m_form->Detach();
    m_form->Hide();
    GUCEF_DELETE m_form;
    m_form = GUCEF_NULL;
    if ( notify )
        NotifyObservers( PresentationClosedEvent );
}

/*-------------------------------------------------------------------------*/

void
CWebViewFormPresenter::OnFormCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_cancelled = true;
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewFormPresenter::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CWebViewFormPresenter";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
