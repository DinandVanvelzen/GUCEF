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

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "gucefGUI_CActivityService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CActivityService::OperationShownEvent = "GUCEF::GUI::CActivityService::OperationShownEvent";
const CORE::CEvent CActivityService::OperationChangedEvent = "GUCEF::GUI::CActivityService::OperationChangedEvent";
const CORE::CEvent CActivityService::OperationEndedEvent = "GUCEF::GUI::CActivityService::OperationEndedEvent";
const CORE::CEvent CActivityService::CancelRequestedEvent = "GUCEF::GUI::CActivityService::CancelRequestedEvent";
const CORE::CEvent CActivityService::ActivityChangedEvent = "GUCEF::GUI::CActivityService::ActivityChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

UInt64
PrecisionClockInMs( void* userData )
{GUCEF_TRACE;

    const UInt64 resolution = MT::PrecisionTimerResolution();
    return 0 == resolution ? 0 : MT::PrecisionTickCount() * 1000 / resolution;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CActivityService::RegisterEvents( void )
{GUCEF_TRACE;

    OperationShownEvent.Initialize();
    OperationChangedEvent.Initialize();
    OperationEndedEvent.Initialize();
    CancelRequestedEvent.Initialize();
    ActivityChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CActivityService::SRecord::SRecord( void )
    : info()
    , ended( false )
    , changedQueued( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CActivityService::CActivityService( void )
    : CORE::CObservingNotifier()
    , m_lock()
    , m_records()
    , m_notices()
    , m_nextId( 1 )
    , m_thresholdInMs( (UInt32) DefaultThresholdInMs )
    , m_clock( &PrecisionClockInMs )
    , m_clockData( GUCEF_NULL )
    , m_context( GUCEF_NULL )
    , m_busyCursorEnabled( true )
    , m_cursorSet( false )
    , m_lastShownCount( 0 )
    , m_stallThresholdInMs( (UInt32) DefaultStallThresholdInMs )
    , m_stallCount( 0 )
    , m_inUpdate( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CActivityService::~CActivityService()
{GUCEF_TRACE;

    Detach();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CActivityService::SetThresholdInMs( UInt32 thresholdInMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_thresholdInMs = thresholdInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CActivityService::GetThresholdInMs( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_thresholdInMs;
}

/*-------------------------------------------------------------------------*/

void
CActivityService::SetClock( TClockFunction clock, void* userData )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_clock = GUCEF_NULL != clock ? clock : &PrecisionClockInMs;
    m_clockData = GUCEF_NULL != clock ? userData : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt64
CActivityService::Now( void ) const
{GUCEF_TRACE;

    // the lock is held by the caller where the clock may be replaced at the same time; a clock function itself takes no lock of ours
    return m_clock( m_clockData );
}

/*-------------------------------------------------------------------------*/

void
CActivityService::SetBusyCursorEnabled( bool enabled )
{GUCEF_TRACE;

    m_busyCursorEnabled = enabled;
    UpdateCursor();
}

/*-------------------------------------------------------------------------*/

bool
CActivityService::IsBusyCursorEnabled( void ) const
{GUCEF_TRACE;

    return m_busyCursorEnabled;
}

/*-------------------------------------------------------------------------*/

bool
CActivityService::IsBusyCursorShown( void ) const
{GUCEF_TRACE;

    return m_cursorSet;
}

/*-------------------------------------------------------------------------*/

void
CActivityService::SetStallThresholdInMs( UInt32 thresholdInMs )
{GUCEF_TRACE;

    m_stallThresholdInMs = thresholdInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CActivityService::GetStallThresholdInMs( void ) const
{GUCEF_TRACE;

    return m_stallThresholdInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CActivityService::GetStallCount( void ) const
{GUCEF_TRACE;

    return m_stallCount;
}

/*-------------------------------------------------------------------------*/

bool
CActivityService::Attach( CGUIContext* context )
{GUCEF_TRACE;

    Detach();
    if ( GUCEF_NULL == context )
        return false;
    m_context = context;
    TEventCallback frameCallback( this, &CActivityService::OnFrameStarted );
    SubscribeTo( context, CGUIContext::FrameStartedEvent, frameCallback );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CActivityService::Detach( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_context )
        return;
    if ( m_cursorSet )
    {
        // the busy cursor goes whatever is still going on
        m_context->ClearSystemCursorShapeOverride();
        m_cursorSet = false;
    }
    UnsubscribeFrom( m_context );
    m_context = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CGUIContext*
CActivityService::GetContext( void ) const
{GUCEF_TRACE;

    return m_context;
}

/*-------------------------------------------------------------------------*/

CActivityService::SRecord*
CActivityService::Find( UInt32 id )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_records.size(); ++i )
        if ( m_records[ i ].info.id == id && !m_records[ i ].ended )
            return &m_records[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CActivityService::SRecord*
CActivityService::Find( UInt32 id ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_records.size(); ++i )
        if ( m_records[ i ].info.id == id && !m_records[ i ].ended )
            return &m_records[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CActivityService::QueueNotice( ENoticeType type, UInt32 id )
{GUCEF_TRACE;

    SNotice notice;
    notice.type = type;
    notice.id = id;
    m_notices.push_back( notice );
}

/*-------------------------------------------------------------------------*/

void
CActivityService::QueueChanged( SRecord& record )
{GUCEF_TRACE;

    // only what the user sees is told about, and once per Update()
    if ( record.info.shown && !record.changedQueued )
    {
        record.changedQueued = true;
        QueueNotice( NOTICE_CHANGED, record.info.id );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CActivityService::Begin( const CORE::CString& label, UInt32 expectedDurationInMs, bool determinate, bool cancellable )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    SRecord record;
    record.info.id = m_nextId++;
    if ( 0 == m_nextId )
        m_nextId = 1;
    record.info.label = label;
    record.info.expectedDurationInMs = expectedDurationInMs;
    record.info.determinate = determinate;
    record.info.cancellable = cancellable;
    record.info.startedAtInMs = Now();
    m_records.push_back( record );
    return record.info.id;
}

/*-------------------------------------------------------------------------*/

void
CActivityService::SetProgress( UInt32 id, Float32 fraction )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    SRecord* record = Find( id );
    if ( GUCEF_NULL == record || !record->info.determinate )
        return;
    if ( fraction < 0.0f )
        fraction = 0.0f;
    else if ( fraction > 1.0f )
        fraction = 1.0f;
    if ( fraction == record->info.progress )
        return;
    record->info.progress = fraction;
    QueueChanged( *record );
}

/*-------------------------------------------------------------------------*/

void
CActivityService::SetLabel( UInt32 id, const CORE::CString& label )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    SRecord* record = Find( id );
    if ( GUCEF_NULL == record || record->info.label == label )
        return;
    record->info.label = label;
    QueueChanged( *record );
}

/*-------------------------------------------------------------------------*/

void
CActivityService::End( UInt32 id )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_records.size(); ++i )
    {
        SRecord& record = m_records[ i ];
        if ( record.info.id != id || record.ended )
            continue;
        if ( record.info.shown )
        {
            // kept until the next Update() has said so
            record.ended = true;
            QueueNotice( NOTICE_ENDED, id );
        }
        else
        {
            // never seen by the user: it is as if it never was
            m_records.erase( m_records.begin() + i );
        }
        return;
    }
}

/*-------------------------------------------------------------------------*/

bool
CActivityService::RequestCancel( UInt32 id )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    SRecord* record = Find( id );
    if ( GUCEF_NULL == record || !record->info.cancellable )
        return false;
    if ( !record->info.cancelRequested )
    {
        record->info.cancelRequested = true;
        QueueNotice( NOTICE_CANCEL, id );
        QueueChanged( *record );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CActivityService::IsCancelRequested( UInt32 id ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    const SRecord* record = Find( id );
    return GUCEF_NULL != record && record->info.cancelRequested;
}

/*-------------------------------------------------------------------------*/

bool
CActivityService::IsActive( UInt32 id ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return GUCEF_NULL != Find( id );
}

/*-------------------------------------------------------------------------*/

UInt32
CActivityService::GetShownCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt32 count = 0;
    for ( size_t i=0; i<m_records.size(); ++i )
        if ( m_records[ i ].info.shown && !m_records[ i ].ended )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CActivityService::GetActiveCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt32 count = 0;
    for ( size_t i=0; i<m_records.size(); ++i )
        if ( !m_records[ i ].ended )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

void
CActivityService::GetShownOperations( TActivityInfoVector& operations ) const
{GUCEF_TRACE;

    operations.clear();
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_records.size(); ++i )
        if ( m_records[ i ].info.shown && !m_records[ i ].ended )
            operations.push_back( m_records[ i ].info );
}

/*-------------------------------------------------------------------------*/

bool
CActivityService::GetOperation( UInt32 id, CActivityInfo& info ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    const SRecord* record = Find( id );
    if ( GUCEF_NULL == record )
        return false;
    info = record->info;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CActivityService::UpdateCursor( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_context )
        return;
    const bool wanted = m_busyCursorEnabled && m_lastShownCount > 0;
    if ( wanted && !m_cursorSet )
    {
        // an override: a driver sets the shape of the context itself, every frame, from what is under the pointer
        m_context->SetSystemCursorShapeOverride( CURSORSHAPE_PROGRESS );
        m_cursorSet = true;
    }
    else
    if ( !wanted && m_cursorSet )
    {
        m_context->ClearSystemCursorShapeOverride();
        m_cursorSet = false;
    }
}

/*-------------------------------------------------------------------------*/

void
CActivityService::Update( void )
{GUCEF_TRACE;

    if ( m_inUpdate )
        return;
    m_inUpdate = true;

    TNoticeVector notices;
    UInt32 shownCount = 0;
    {
        MT::CScopeMutex lock( m_lock );
        const UInt64 now = Now();
        for ( size_t i=0; i<m_records.size(); ++i )
        {
            SRecord& record = m_records[ i ];
            if ( record.ended )
                continue;
            if ( !record.info.shown )
            {
                // the threshold is passed, or the operation said that it will take longer than that
                const bool longRunning = record.info.expectedDurationInMs > m_thresholdInMs;
                const bool overdue = now >= record.info.startedAtInMs && now - record.info.startedAtInMs >= m_thresholdInMs;
                if ( longRunning || overdue )
                {
                    record.info.shown = true;
                    QueueNotice( NOTICE_SHOWN, record.info.id );
                }
            }
            record.changedQueued = false;
        }
        notices.swap( m_notices );
        // what ended is told below, and then it is gone
        for ( size_t i=m_records.size(); i>0; --i )
            if ( m_records[ i-1 ].ended )
                m_records.erase( m_records.begin() + ( i-1 ) );
        for ( size_t i=0; i<m_records.size(); ++i )
            if ( m_records[ i ].info.shown )
                ++shownCount;
    }

    for ( size_t i=0; i<notices.size(); ++i )
    {
        CORE::TCloneableUInt32 eventData( notices[ i ].id );
        switch ( notices[ i ].type )
        {
            case NOTICE_SHOWN:      NotifyObservers( OperationShownEvent, &eventData ); break;
            case NOTICE_CHANGED:    NotifyObservers( OperationChangedEvent, &eventData ); break;
            case NOTICE_ENDED:      NotifyObservers( OperationEndedEvent, &eventData ); break;
            case NOTICE_CANCEL:     NotifyObservers( CancelRequestedEvent, &eventData ); break;
        }
    }
    m_lastShownCount = shownCount;
    UpdateCursor();
    if ( !notices.empty() )
        NotifyObservers( ActivityChangedEvent );

    m_inUpdate = false;
}

/*-------------------------------------------------------------------------*/

void
CActivityService::OnFrameStarted( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    // the watchdog: the frame before this one took too long, so the interface thread was blocked for that time
    CORE::TCloneableUInt32* frameTime = static_cast< CORE::TCloneableUInt32* >( eventData );
    if ( GUCEF_NULL != frameTime && frameTime->GetData() > m_stallThresholdInMs )
    {
        ++m_stallCount;
        TActivityInfoVector running;
        GetShownOperations( running );
        CORE::CString labels;
        for ( size_t i=0; i<running.size(); ++i )
        {
            if ( i > 0 )
                labels += ", ";
            labels += running[ i ].label;
        }
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "CActivityService: The interface thread stalled for " << frameTime->GetData() << " ms in one frame (limit " <<
                           m_stallThresholdInMs << " ms); operations running: " << ( labels.IsNULLOrEmpty() ? CORE::CString( "none" ) : labels ) );
    }
    #endif /* GUCEF_DEBUG_MODE ? */

    Update();
}

/*-------------------------------------------------------------------------*/

void
CActivityService::OnNotify( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CORE::CNotifier::DestructionEvent == eventId && GUCEF_NULL != notifier && notifier == m_context )
    {
        // nothing to put back on a context that is gone
        m_context = GUCEF_NULL;
        m_cursorSet = false;
        return;
    }
    CORE::CObservingNotifier::OnNotify( notifier, eventId, eventData );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
