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

#include "gucefWEB_CWebViewEventRelay.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The events of a view that are relayed (the standard events of the notifier are not) */
const CORE::CEvent* const* RelayedEvents( UInt32& count )
{
    static const CORE::CEvent* events[] =
    {
        &IWebView::LoadStartedEvent, &IWebView::LoadCommittedEvent, &IWebView::LoadFinishedEvent, &IWebView::LoadFailedEvent,
        &IWebView::TitleChangedEvent, &IWebView::UrlChangedEvent, &IWebView::ProgressChangedEvent, &IWebView::SecurityStateChangedEvent,
        &IWebView::CursorChangedEvent, &IWebView::FrameAvailableEvent, &IWebView::NavigationRequestedEvent, &IWebView::PopupRequestedEvent,
        &IWebView::DownloadRequestedEvent, &IWebView::PermissionRequestedEvent, &IWebView::CertificateErrorEvent, &IWebView::ScriptDialogEvent,
        &IWebView::ConsoleMessageEvent, &IWebView::ProcessTerminatedEvent, &IWebView::FocusLeaveRequestedEvent, &IWebView::CloseRequestedEvent,
        &IWebView::EngineStalledEvent, &IWebView::ClosedEvent
    };
    count = (UInt32) ( sizeof( events ) / sizeof( events[ 0 ] ) );
    return events;
}

}

/*-------------------------------------------------------------------------*/

CWebViewEventRelay::CMailObserver::CMailObserver( MT::CTMailBox< CORE::CEvent >& mail )
    : CORE::CObserver()
    , m_mail( mail )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewEventRelay::CMailObserver::~CMailObserver()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CWebViewEventRelay::CMailObserver::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    UInt32 count = 0;
    const CORE::CEvent* const* events = RelayedEvents( count );
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( *events[ i ] == eventId )
        {
            m_mail.AddMail( eventId, eventData );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewEventRelay::CMailObserver::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::WEB::CWebViewEventRelay::CMailObserver";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CWebViewEventRelay::CWebViewEventRelay( void )
    : CORE::CNotifier()
    , m_mail()
    , m_observer( m_mail )
    , m_view()
{GUCEF_TRACE;

    IWebView::RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CWebViewEventRelay::~CWebViewEventRelay()
{GUCEF_TRACE;

    Detach();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEventRelay::Attach( const TWebViewPtr& view )
{GUCEF_TRACE;

    Detach();
    if ( view.IsNULL() )
        return false;
    m_view = view;
    m_mail.SetAcceptsNewMail( true );
    UInt32 count = 0;
    const CORE::CEvent* const* events = RelayedEvents( count );
    for ( UInt32 i=0; i<count; ++i )
        m_observer.SubscribeTo( m_view.GetPointerAlways(), *events[ i ] );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebViewEventRelay::Detach( void )
{GUCEF_TRACE;

    m_observer.UnsubscribeAllFromObserver();
    m_mail.Clear();
    m_view.Unlink();
}

/*-------------------------------------------------------------------------*/

TWebViewPtr
CWebViewEventRelay::GetView( void ) const
{GUCEF_TRACE;

    return m_view;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewEventRelay::Relay( UInt32 maxEvents )
{GUCEF_TRACE;

    UInt32 relayed = 0;
    CORE::CEvent eventId;
    CORE::CICloneable* eventData = GUCEF_NULL;
    while ( ( 0 == maxEvents || relayed < maxEvents ) && m_mail.GetMail( eventId, &eventData ) )
    {
        ++relayed;
        NotifyObservers( eventId, eventData );
        GUCEF_DELETE eventData;
        eventData = GUCEF_NULL;
        // an observer may have detached the view: what is left is not for it any more
        if ( m_view.IsNULL() )
            break;
    }
    return relayed;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEventRelay::HasEvents( void ) const
{GUCEF_TRACE;

    return m_mail.HasMail();
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewEventRelay::GetQueuedCount( void ) const
{GUCEF_TRACE;

    return m_mail.AmountOfMail();
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewEventRelay::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::WEB::CWebViewEventRelay";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
