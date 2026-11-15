/*
 *  gucefWEB_TestApp: tests of the gucefWEB module, its web views
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

#ifndef GUCEF_WEBVIEWTEST_TESTWEBVIEWEVENTLOG_H
#define GUCEF_WEBVIEWTEST_TESTWEBVIEWEVENTLOG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_WEB_WEBVIEW_H
#include "gucefWEB_WebView.h"
#define GUCEF_WEB_WEBVIEW_H
#endif /* GUCEF_WEB_WEBVIEW_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEBVIEWTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Writes down the events of a view (or an engine, a relay) as they come, on whatever thread they are notified: the
 *  name of each event, a copy of its data and the thread it came on. A lock of its own guards it, so a test thread
 *  reads it while the engine thread writes.
 */
class CTestEventLog : public CORE::CObserver
{
    public:

    struct SEntry
    {
        CORE::CString name;
        CORE::CICloneable* data;
        TThreadId threadId;
    };

    CTestEventLog( void ) : CORE::CObserver(), m_lock(), m_entries(), m_watched() {}

    virtual ~CTestEventLog()
    {
        UnsubscribeAllFromObserver();
        Clear();
    }

    /** Subscribes to the events of IWebView */
    void Watch( CORE::CNotifier* view )
    {
        const CORE::CEvent* events[] =
        {
            &WEB::IWebView::LoadStartedEvent, &WEB::IWebView::LoadCommittedEvent, &WEB::IWebView::LoadFinishedEvent,
            &WEB::IWebView::LoadFailedEvent, &WEB::IWebView::TitleChangedEvent, &WEB::IWebView::UrlChangedEvent,
            &WEB::IWebView::ProgressChangedEvent, &WEB::IWebView::SecurityStateChangedEvent, &WEB::IWebView::CursorChangedEvent,
            &WEB::IWebView::FrameAvailableEvent, &WEB::IWebView::NavigationRequestedEvent, &WEB::IWebView::PopupRequestedEvent,
            &WEB::IWebView::DownloadRequestedEvent, &WEB::IWebView::PermissionRequestedEvent, &WEB::IWebView::CertificateErrorEvent,
            &WEB::IWebView::ScriptDialogEvent, &WEB::IWebView::ConsoleMessageEvent, &WEB::IWebView::ProcessTerminatedEvent,
            &WEB::IWebView::FocusLeaveRequestedEvent, &WEB::IWebView::CloseRequestedEvent, &WEB::IWebView::EngineStalledEvent,
            &WEB::IWebView::ClosedEvent
        };
        for ( UInt32 i=0; i<(UInt32)( sizeof( events ) / sizeof( events[ 0 ] ) ); ++i )
            WatchEvent( view, *events[ i ] );
    }

    void WatchEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId )
    {
        {
            MT::CScopeMutex lock( m_lock );
            m_watched.insert( eventId.GetName() );
        }
        SubscribeTo( notifier, eventId );
    }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !eventId.IsInitialized() )
            return;
        // only what is watched: the standard events of the notifiers (subscribe, unsubscribe) are not
        const CORE::CString name = eventId.GetName();
        {
            MT::CScopeMutex lock( m_lock );
            if ( m_watched.find( name ) == m_watched.end() )
                return;
        }
        SEntry entry;
        entry.name = name;
        entry.data = GUCEF_NULL != eventData ? eventData->Clone() : GUCEF_NULL;
        entry.threadId = MT::GetCurrentThreadID();
        MT::CScopeMutex lock( m_lock );
        m_entries.push_back( entry );
    }

    void Clear( void )
    {
        MT::CScopeMutex lock( m_lock );
        for ( UInt32 i=0; i<(UInt32)m_entries.size(); ++i )
            GUCEF_DELETE m_entries[ i ].data;
        m_entries.clear();
    }

    UInt32 Count( const CORE::CEvent& eventId ) const
    {
        MT::CScopeMutex lock( m_lock );
        UInt32 count = 0;
        for ( UInt32 i=0; i<(UInt32)m_entries.size(); ++i )
            if ( m_entries[ i ].name == eventId.GetName() )
                ++count;
        return count;
    }

    UInt32 Total( void ) const
    {
        MT::CScopeMutex lock( m_lock );
        return (UInt32) m_entries.size();
    }

    /** Waits up to 'timeoutMs' until the event came 'count' times */
    bool WaitFor( const CORE::CEvent& eventId, UInt32 count = 1, UInt32 timeoutMs = 5000 ) const
    {
        const UInt64 start = MT::PrecisionTickCount();
        while ( Count( eventId ) < count )
        {
            if ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) > (Float64) timeoutMs )
                return false;
            MT::PrecisionDelay( 2 );
        }
        return true;
    }

    /** The short names of the events in order ("LoadStartedEvent" ...), those of 'only' when it is not empty */
    CORE::CString::StringVector Names( const CORE::CString::StringSet& only = CORE::CString::StringSet() ) const
    {
        MT::CScopeMutex lock( m_lock );
        CORE::CString::StringVector names;
        for ( UInt32 i=0; i<(UInt32)m_entries.size(); ++i )
        {
            const CORE::CString shortName = m_entries[ i ].name.SubstrToChar( ':', false );
            if ( only.empty() || only.find( shortName ) != only.end() )
                names.push_back( shortName );
        }
        return names;
    }

    /** A copy of the data of the n-th event of the kind (0 the first), as T; false when there is no such event */
    template < class T >
    bool GetData( const CORE::CEvent& eventId, UInt32 n, T& out ) const
    {
        MT::CScopeMutex lock( m_lock );
        UInt32 seen = 0;
        for ( UInt32 i=0; i<(UInt32)m_entries.size(); ++i )
        {
            if ( m_entries[ i ].name != eventId.GetName() )
                continue;
            if ( seen++ == n )
            {
                if ( GUCEF_NULL == m_entries[ i ].data )
                    return false;
                out = static_cast< CORE::CTCloneableObj< T >* >( m_entries[ i ].data )->GetData();
                return true;
            }
        }
        return false;
    }

    /** Whether every event came on the given thread */
    bool AllOnThread( TThreadId threadId ) const
    {
        MT::CScopeMutex lock( m_lock );
        for ( UInt32 i=0; i<(UInt32)m_entries.size(); ++i )
            if ( m_entries[ i ].threadId != threadId )
                return false;
        return true;
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "GUCEF::WEBVIEWTEST::CTestEventLog";
        return classTypeName;
    }

    private:

    mutable MT::CMutex m_lock;
    GUCEF::vector< SEntry > m_entries;
    CORE::CString::StringSet m_watched;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEBVIEWTEST */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEBVIEWTEST_TESTWEBVIEWEVENTLOG_H ? */
