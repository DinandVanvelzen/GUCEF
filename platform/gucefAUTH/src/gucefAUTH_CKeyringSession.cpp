/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#ifndef GUCEF_AUTH_CKEYRINGOPERATIONTASKCONSUMER_H
#include "gucefAUTH_CKeyringOperationTaskConsumer.h"
#define GUCEF_AUTH_CKEYRINGOPERATIONTASKCONSUMER_H
#endif /* GUCEF_AUTH_CKEYRINGOPERATIONTASKCONSUMER_H ? */

#include "gucefAUTH_CKeyringSession.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CKeyringSession::UnlockedEvent = "GUCEF::AUTH::CKeyringSession::UnlockedEvent";
const CORE::CEvent CKeyringSession::LockedEvent = "GUCEF::AUTH::CKeyringSession::LockedEvent";
const CORE::CEvent CKeyringSession::WrongCodeEvent = "GUCEF::AUTH::CKeyringSession::WrongCodeEvent";
const CORE::CEvent CKeyringSession::RetryAllowedEvent = "GUCEF::AUTH::CKeyringSession::RetryAllowedEvent";
const CORE::CEvent CKeyringSession::CodeSetUpEvent = "GUCEF::AUTH::CKeyringSession::CodeSetUpEvent";
const CORE::CEvent CKeyringSession::CodeChangedEvent = "GUCEF::AUTH::CKeyringSession::CodeChangedEvent";
const CORE::CEvent CKeyringSession::ForgottenEvent = "GUCEF::AUTH::CKeyringSession::ForgottenEvent";
const CORE::CEvent CKeyringSession::FailedEvent = "GUCEF::AUTH::CKeyringSession::FailedEvent";

/** How often the timer looks at an operation in flight, at most */
static const UInt32 BusyCheckIntervalInMs = 25;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CKeyringSession::RegisterEvents( void )
{GUCEF_TRACE;

    UnlockedEvent.Initialize();
    LockedEvent.Initialize();
    WrongCodeEvent.Initialize();
    RetryAllowedEvent.Initialize();
    CodeSetUpEvent.Initialize();
    CodeChangedEvent.Initialize();
    ForgottenEvent.Initialize();
    FailedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CKeyringSession::CKeyringSession( void )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
    , m_lock()
    , m_store( GUCEF_NULL )
    , m_settings()
    , m_threadPool()
    , m_timer( CKeyringSettings::DefaultCheckIntervalInMs )
    , m_busy( false )
    , m_shutDown( false )
    , m_knownUnlocked( false )
    , m_failedAttempts( 0 )
    , m_retryAtMs( 0 )
    , m_retryAllowedPending( false )
    , m_lastResult( KEYRING_OK )
    , m_pendingEvents()
{GUCEF_TRACE;

    Construct();
}

/*-------------------------------------------------------------------------*/

CKeyringSession::CKeyringSession( CFileCredentialStore* store )
    : CORE::CObservingNotifier()
    , CORE::CIConfigurable()
    , m_lock()
    , m_store( GUCEF_NULL )
    , m_settings()
    , m_threadPool()
    , m_timer( CKeyringSettings::DefaultCheckIntervalInMs )
    , m_busy( false )
    , m_shutDown( false )
    , m_knownUnlocked( false )
    , m_failedAttempts( 0 )
    , m_retryAtMs( 0 )
    , m_retryAllowedPending( false )
    , m_lastResult( KEYRING_OK )
    , m_pendingEvents()
{GUCEF_TRACE;

    Construct();
    SetStore( store );
}

/*-------------------------------------------------------------------------*/

void
CKeyringSession::Construct( void )
{GUCEF_TRACE;

    RegisterEvents();
    m_timer.SetEnabled( false );
    TEventCallback timerCallback( this, &CKeyringSession::OnTimer );
    SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent, timerCallback );
    TEventCallback shutdownCallback( this, &CKeyringSession::OnApplicationShutdown );
    SubscribeTo( &CORE::CCoreGlobal::Instance()->GetApplication(), CORE::CGUCEFApplication::AppShutdownEvent, shutdownCallback );
}

/*-------------------------------------------------------------------------*/

CKeyringSession::~CKeyringSession()
{GUCEF_TRACE;

    Shutdown();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CKeyringSession::WaitForOperation( void )
{GUCEF_TRACE;

    // an operation that runs uses the store: it is let finish, it takes the time of its derivation
    for ( ;; )
    {
        {
            MT::CScopeMutex lock( m_lock );
            if ( !m_busy )
                return;
        }
        MT::ThreadDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

void
CKeyringSession::SetStore( CFileCredentialStore* store )
{GUCEF_TRACE;

    WaitForOperation();
    {
        MT::CScopeMutex lock( m_lock );
        if ( store == m_store )
            return;
        if ( GUCEF_NULL != m_store )
            m_store->LockKeyring();
        m_store = store;
        m_knownUnlocked = GUCEF_NULL != m_store && !m_store->IsKeyringLocked();
        m_failedAttempts = 0;
        m_retryAtMs = 0;
        m_retryAllowedPending = false;
        m_lastResult = KEYRING_OK;
    }
    UpdateTimer();
}

/*-------------------------------------------------------------------------*/

CFileCredentialStore*
CKeyringSession::GetStore( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_store;
}

/*-------------------------------------------------------------------------*/

void
CKeyringSession::SetSettings( const CKeyringSettings& settings )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_settings = settings;
        if ( m_settings.maxRetryDelayInMs < m_settings.retryDelayInMs )
            m_settings.maxRetryDelayInMs = m_settings.retryDelayInMs;
        if ( 0 == m_settings.checkIntervalInMs )
            m_settings.checkIntervalInMs = 1;
    }
    UpdateTimer();
}

/*-------------------------------------------------------------------------*/

CKeyringSettings
CKeyringSession::GetSettings( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CKeyringSession::SetThreadPool( CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_threadPool = threadPool;
}

/*-------------------------------------------------------------------------*/

bool
CKeyringSession::IsLocked( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return GUCEF_NULL == m_store || m_store->IsKeyringLocked();
}

/*-------------------------------------------------------------------------*/

bool
CKeyringSession::IsSetUp( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return GUCEF_NULL != m_store && m_store->IsKeyringSetUp();
}

/*-------------------------------------------------------------------------*/

bool
CKeyringSession::IsBusy( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_busy;
}

/*-------------------------------------------------------------------------*/

UInt32
CKeyringSession::GetFailedAttemptCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_failedAttempts;
}

/*-------------------------------------------------------------------------*/

UInt32
CKeyringSession::GetRetryWaitInMs( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt64 now = GetMonotonicTimeInMs();
    if ( 0 == m_retryAtMs || now >= m_retryAtMs )
        return 0;
    return (UInt32) ( m_retryAtMs - now );
}

/*-------------------------------------------------------------------------*/

TKeyringResult
CKeyringSession::GetLastResult( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lastResult;
}

/*-------------------------------------------------------------------------*/

CKeyringEventData
CKeyringSession::MakeEventDataLocked( TKeyringResult result ) const
{GUCEF_TRACE;

    CKeyringEventData data;
    data.result = result;
    data.failedAttempts = m_failedAttempts;
    UInt64 now = GetMonotonicTimeInMs();
    data.retryWaitInMs = ( 0 != m_retryAtMs && m_retryAtMs > now ) ? (UInt32) ( m_retryAtMs - now ) : 0;
    return data;
}

/*-------------------------------------------------------------------------*/

void
CKeyringSession::QueueEventLocked( const CORE::CEvent& eventId, const CKeyringEventData& data )
{GUCEF_TRACE;

    SPendingEvent pending;
    pending.eventId = eventId;
    pending.data = data;
    m_pendingEvents.push_back( pending );
}

/*-------------------------------------------------------------------------*/

void
CKeyringSession::FlushEvents( void )
{GUCEF_TRACE;

    TPendingEventVector events;
    {
        MT::CScopeMutex lock( m_lock );
        events.swap( m_pendingEvents );
    }
    for ( size_t i=0; i<events.size(); ++i )
    {
        CKeyringEventData copy( events[ i ].data );
        if ( !NotifyObservers( events[ i ].eventId, &copy ) )
            return;     // the session went away in an observer
    }
}

/*-------------------------------------------------------------------------*/

TKeyringResult
CKeyringSession::BeginOperation( bool verifiesCode )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TKeyringResult result = KEYRING_OK;
    if ( GUCEF_NULL == m_store || m_shutDown )
        result = KEYRING_FAILED;
    else
    if ( m_busy )
        result = KEYRING_BUSY;
    else
    if ( verifiesCode && 0 != m_retryAtMs && GetMonotonicTimeInMs() < m_retryAtMs )
        result = KEYRING_THROTTLED;
    if ( KEYRING_OK == result )
        m_busy = true;
    else
        m_lastResult = result;
    return result;
}

/*-------------------------------------------------------------------------*/

TKeyringResult
CKeyringSession::PerformOperation( CKeyringOperationData::EOperation operation ,
                                   const CSecretString& code                   ,
                                   const CSecretString& newCode                )
{GUCEF_TRACE;

    // m_busy is set: the store stays while the operation runs, and nothing else of the session uses it for a code
    CFileCredentialStore* store = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        store = m_store;
    }
    TKeyringResult result = KEYRING_FAILED;
    if ( GUCEF_NULL != store )
    {
        switch ( operation )
        {
            case CKeyringOperationData::OPERATION_UNLOCK: result = store->UnlockKeyring( code ); break;
            case CKeyringOperationData::OPERATION_SET_UP: result = store->SetUpKeyring( code ); break;
            case CKeyringOperationData::OPERATION_CHANGE_CODE: result = store->ChangeKeyringCode( code, newCode ); break;
        }
    }

    MT::CScopeMutex lock( m_lock );
    m_busy = false;
    m_lastResult = result;
    if ( m_shutDown && GUCEF_NULL != store )
    {
        // the application ended meanwhile: what the operation unlocked is locked again at once
        store->LockKeyring();
        m_knownUnlocked = false;
        return result;
    }
    if ( KEYRING_OK == result )
    {
        m_failedAttempts = 0;
        m_retryAtMs = 0;
        m_retryAllowedPending = false;
        if ( CKeyringOperationData::OPERATION_SET_UP == operation )
            QueueEventLocked( CodeSetUpEvent, MakeEventDataLocked( result ) );
        else
        if ( CKeyringOperationData::OPERATION_CHANGE_CODE == operation )
            QueueEventLocked( CodeChangedEvent, MakeEventDataLocked( result ) );
        if ( !m_knownUnlocked )
        {
            m_knownUnlocked = true;
            QueueEventLocked( UnlockedEvent, MakeEventDataLocked( result ) );
        }
        return result;
    }
    if ( KEYRING_WRONG_CODE == result )
    {
        ++m_failedAttempts;
        if ( m_failedAttempts > m_settings.freeAttempts && m_settings.retryDelayInMs > 0 )
        {
            UInt64 wait = m_settings.retryDelayInMs;
            for ( UInt32 i=m_settings.freeAttempts+1; i<m_failedAttempts && wait < m_settings.maxRetryDelayInMs; ++i )
                wait *= 2;
            if ( wait > m_settings.maxRetryDelayInMs )
                wait = m_settings.maxRetryDelayInMs;
            m_retryAtMs = GetMonotonicTimeInMs() + wait;
            m_retryAllowedPending = true;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "KeyringSession: A wrong code (" << m_failedAttempts << " in a row)" );
        QueueEventLocked( WrongCodeEvent, MakeEventDataLocked( result ) );
        return result;
    }
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "KeyringSession: An operation on the keyring failed: " << KeyringResultToString( result ) );
    QueueEventLocked( FailedEvent, MakeEventDataLocked( result ) );
    if ( m_knownUnlocked && GUCEF_NULL != store && store->IsKeyringLocked() )
    {
        m_knownUnlocked = false;
        CKeyringEventData data = MakeEventDataLocked( result );
        data.lockReason = store->GetKeyringLockReason();
        QueueEventLocked( LockedEvent, data );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

TKeyringResult
CKeyringSession::UnlockWithCode( const CSecretString& code )
{GUCEF_TRACE;

    TKeyringResult result = BeginOperation( true );
    if ( KEYRING_OK == result )
        result = PerformOperation( CKeyringOperationData::OPERATION_UNLOCK, code, CSecretString() );
    FlushEvents();
    UpdateTimer();
    return result;
}

/*-------------------------------------------------------------------------*/

TKeyringResult
CKeyringSession::SetUpCode( const CSecretString& code )
{GUCEF_TRACE;

    TKeyringResult result = BeginOperation( false );
    if ( KEYRING_OK == result )
        result = PerformOperation( CKeyringOperationData::OPERATION_SET_UP, code, CSecretString() );
    FlushEvents();
    UpdateTimer();
    return result;
}

/*-------------------------------------------------------------------------*/

TKeyringResult
CKeyringSession::ChangeCode( const CSecretString& currentCode, const CSecretString& newCode )
{GUCEF_TRACE;

    TKeyringResult result = BeginOperation( true );
    if ( KEYRING_OK == result )
        result = PerformOperation( CKeyringOperationData::OPERATION_CHANGE_CODE, currentCode, newCode );
    FlushEvents();
    UpdateTimer();
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CKeyringSession::QueueOperation( CKeyringOperationData::EOperation operation ,
                                 const CSecretString& code                   ,
                                 const CSecretString& newCode                )
{GUCEF_TRACE;

    CORE::CFutureResultSource source;
    TKeyringResult begin = BeginOperation( CKeyringOperationData::OPERATION_SET_UP != operation );
    if ( KEYRING_OK != begin )
    {
        // not started: the future ends in an error status at once and says why (GetResultOfFuture())
        source.Fail( KEYRING_BUSY == begin ? CORE::TASKSTATUS_TASK_WITH_TYPE_EXISTS : CORE::TASKSTATUS_SETUP_FAILED, KeyringResultToString( begin ) );
        return source.GetFuture();
    }

    CORE::ThreadPoolPtr threadPool;
    {
        MT::CScopeMutex lock( m_lock );
        threadPool = m_threadPool;
    }
    if ( threadPool.IsNULL() )
        threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();

    CKeyringOperationData* data = GUCEF_NEW CKeyringOperationData();
    data->session = this;
    data->operation = operation;
    data->code = code;
    data->newCode = newCode;
    data->source = source;

    CORE::CFutureResult queued( CORE::TASKSTATUS_QUEUEING_FAILED );
    if ( !threadPool.IsNULL() )
        queued = threadPool->QueueTask( CKeyringOperationTaskConsumer::TaskType, data, GUCEF_NULL, true );
    else
        GUCEF_DELETE data;
    if ( queued.HasNoFuture() )
    {
        {
            MT::CScopeMutex lock( m_lock );
            m_busy = false;
            m_lastResult = KEYRING_FAILED;
            QueueEventLocked( FailedEvent, MakeEventDataLocked( KEYRING_FAILED ) );
        }
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KeyringSession: The operation could not be queued on the thread pool" );
        source.Fail( CORE::TASKSTATUS_QUEUEING_FAILED, "The operation could not be queued on the thread pool" );
    }
    UpdateTimer();
    return source.GetFuture();
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CKeyringSession::UnlockWithCodeAsync( const CSecretString& code )
{GUCEF_TRACE;

    return QueueOperation( CKeyringOperationData::OPERATION_UNLOCK, code, CSecretString() );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CKeyringSession::SetUpCodeAsync( const CSecretString& code )
{GUCEF_TRACE;

    return QueueOperation( CKeyringOperationData::OPERATION_SET_UP, code, CSecretString() );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CKeyringSession::ChangeCodeAsync( const CSecretString& currentCode, const CSecretString& newCode )
{GUCEF_TRACE;

    return QueueOperation( CKeyringOperationData::OPERATION_CHANGE_CODE, currentCode, newCode );
}

/*-------------------------------------------------------------------------*/

void
CKeyringSession::RunOperation( CKeyringOperationData& data )
{GUCEF_TRACE;

    // On the pool thread: the events follow at the next tick of the timer, on the thread of its pulse generator
    TKeyringResult result = PerformOperation( data.operation, data.code, data.newCode );
    data.code.Clear();
    data.newCode.Clear();
    CKeyringEventData outcome;
    {
        MT::CScopeMutex lock( m_lock );
        outcome = MakeEventDataLocked( result );
    }
    data.source.Complete( outcome.Clone(), true );
}

/*-------------------------------------------------------------------------*/

TKeyringResult
CKeyringSession::GetResultOfFuture( const CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !future.IsReady() )
        return KEYRING_FAILED;
    CORE::CTaskPtr task = future.GetResult( 0 );
    if ( task.IsNULL() )
        return KEYRING_FAILED;
    CORE::TTaskStatus status = task->GetTaskStatus();
    if ( CORE::TASKSTATUS_FINISHED == status )
    {
        // the operation ran: its outcome is the data of the future
        const CKeyringEventData* outcome = CORE::CICloneable::CloneableAs< CKeyringEventData >( task->GetTaskData() );
        return GUCEF_NULL != outcome ? outcome->result : KEYRING_FAILED;
    }
    if ( CORE::TaskStatusIsAnError( status ) )
    {
        // refused without starting: the extra info is the name of the reason
        const CORE::CString reason = task->GetTaskStatusExtraInfo();
        const TKeyringResult refusals[] = { KEYRING_BUSY, KEYRING_THROTTLED, KEYRING_FAILED };
        for ( UInt32 i=0; i<(UInt32) ( sizeof( refusals ) / sizeof( refusals[ 0 ] ) ); ++i )
        {
            if ( reason == KeyringResultToString( refusals[ i ] ) )
                return refusals[ i ];
        }
    }
    return KEYRING_FAILED;
}

/*-------------------------------------------------------------------------*/

void
CKeyringSession::AbandonOperation( CKeyringOperationData& data )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_busy = false;
        m_lastResult = KEYRING_FAILED;
        QueueEventLocked( FailedEvent, MakeEventDataLocked( KEYRING_FAILED ) );
    }
    data.source.Cancel();
}

/*-------------------------------------------------------------------------*/

void
CKeyringSession::LockNow( void )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( GUCEF_NULL != m_store )
            m_store->LockKeyring();
        if ( m_knownUnlocked )
        {
            m_knownUnlocked = false;
            CKeyringEventData data = MakeEventDataLocked( KEYRING_OK );
            data.lockReason = m_shutDown ? KEYRINGLOCK_APPLICATION_END : KEYRINGLOCK_REQUESTED;
            QueueEventLocked( LockedEvent, data );
        }
    }
    FlushEvents();
    UpdateTimer();
}

/*-------------------------------------------------------------------------*/

bool
CKeyringSession::Forget( void )
{GUCEF_TRACE;

    WaitForOperation();
    bool forgotten = false;
    {
        MT::CScopeMutex lock( m_lock );
        if ( GUCEF_NULL == m_store || m_shutDown || CFileCredentialStore::KEYMODE_KEYRING_CODE != m_store->GetKeyMode() )
            return false;
        forgotten = m_store->Wipe();
        m_failedAttempts = 0;
        m_retryAtMs = 0;
        m_retryAllowedPending = false;
        m_lastResult = forgotten ? KEYRING_OK : KEYRING_FAILED;
        if ( m_knownUnlocked )
        {
            m_knownUnlocked = false;
            CKeyringEventData data = MakeEventDataLocked( m_lastResult );
            data.lockReason = KEYRINGLOCK_FORGOTTEN;
            QueueEventLocked( LockedEvent, data );
        }
        if ( forgotten )
            QueueEventLocked( ForgottenEvent, MakeEventDataLocked( KEYRING_OK ) );
        else
            QueueEventLocked( FailedEvent, MakeEventDataLocked( KEYRING_FAILED ) );
    }
    FlushEvents();
    UpdateTimer();
    return forgotten;
}

/*-------------------------------------------------------------------------*/

void
CKeyringSession::Shutdown( void )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_shutDown = true;
    }
    WaitForOperation();
    LockNow();
    {
        MT::CScopeMutex lock( m_lock );
        m_pendingEvents.clear();
    }
    m_timer.SetEnabled( false );
}

/*-------------------------------------------------------------------------*/

void
CKeyringSession::CheckState( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( GUCEF_NULL == m_store || m_shutDown || m_busy )
        return;
    UInt64 now = GetMonotonicTimeInMs();
    bool unlocked = !m_store->IsKeyringLocked();
    if ( unlocked && m_settings.idleLockInSeconds > 0 )
    {
        UInt64 lastUse = m_store->GetKeyringLastUseInMs();
        if ( 0 != lastUse && now >= lastUse && now - lastUse >= (UInt64) m_settings.idleLockInSeconds * 1000 )
        {
            m_store->LockKeyring();
            unlocked = false;
            m_knownUnlocked = false;
            CKeyringEventData data = MakeEventDataLocked( KEYRING_OK );
            data.lockReason = KEYRINGLOCK_IDLE;
            QueueEventLocked( LockedEvent, data );
        }
    }
    if ( !unlocked && m_knownUnlocked )
    {
        // locked by the store itself: sealed with another code elsewhere, forgotten elsewhere
        m_knownUnlocked = false;
        CKeyringEventData data = MakeEventDataLocked( KEYRING_OK );
        data.lockReason = m_store->GetKeyringLockReason();
        QueueEventLocked( LockedEvent, data );
    }
    else
    if ( unlocked && !m_knownUnlocked )
    {
        m_knownUnlocked = true;
        QueueEventLocked( UnlockedEvent, MakeEventDataLocked( KEYRING_OK ) );
    }
    if ( m_retryAllowedPending && now >= m_retryAtMs )
    {
        m_retryAllowedPending = false;
        m_retryAtMs = 0;
        QueueEventLocked( RetryAllowedEvent, MakeEventDataLocked( m_lastResult ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CKeyringSession::UpdateTimer( void )
{GUCEF_TRACE;

    bool enabled = false;
    UInt32 interval = 0;
    {
        MT::CScopeMutex lock( m_lock );
        enabled = !m_shutDown && GUCEF_NULL != m_store && ( m_busy || m_knownUnlocked || m_retryAllowedPending || !m_pendingEvents.empty() );
        interval = m_settings.checkIntervalInMs;
        if ( ( m_busy || !m_pendingEvents.empty() ) && interval > BusyCheckIntervalInMs )
            interval = BusyCheckIntervalInMs;
    }
    if ( !enabled )
    {
        m_timer.SetEnabled( false );
        return;
    }
    if ( m_timer.GetInterval() != interval )
        m_timer.SetInterval( interval );
    if ( !m_timer.GetEnabled() )
        m_timer.SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

void
CKeyringSession::OnTimer( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CheckState();
    FlushEvents();
    UpdateTimer();
}

/*-------------------------------------------------------------------------*/

void
CKeyringSession::OnApplicationShutdown( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

bool
CKeyringSession::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings.SaveConfig( config );
}

/*-------------------------------------------------------------------------*/

bool
CKeyringSession::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CKeyringSettings settings = GetSettings();
    if ( !settings.LoadConfig( config ) )
        return false;
    SetSettings( settings );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CKeyringSession::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CKeyringSession";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
