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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCOMMANDRESULT_H
#include "gucefWEB_CWebViewCommandResult.h"
#define GUCEF_WEB_CWEBVIEWCOMMANDRESULT_H
#endif /* GUCEF_WEB_CWEBVIEWCOMMANDRESULT_H ? */

#ifndef GUCEF_WEB_CWEBVIEWENGINEBASE_H
#include "gucefWEB_CWebViewEngineBase.h"
#define GUCEF_WEB_CWEBVIEWENGINEBASE_H
#endif /* GUCEF_WEB_CWEBVIEWENGINEBASE_H ? */

#ifndef GUCEF_WEB_CWEBVIEWGLOBAL_H
#include "gucefWEB_CWebViewGlobal.h"
#define GUCEF_WEB_CWEBVIEWGLOBAL_H
#endif /* GUCEF_WEB_CWEBVIEWGLOBAL_H ? */

#include "gucefWEB_CWebViewEngineHost.h"

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

const CString CWebViewEngineHost::TaskType = "WebViewEngineHost";

/*-------------------------------------------------------------------------*/

namespace {

void
FailCommand( CWebViewCommand& command, TWebViewError error, const CString& message )
{
    CORE::CFutureResultSource* completion = command.GetCompletion();
    if ( GUCEF_NULL != completion )
        completion->Complete( GUCEF_NEW CWebViewCommandResult( error, message ), true );
}

}

/*-------------------------------------------------------------------------*/

CWebViewEngineHost::CWebViewEngineHost( CWebViewEngineBase* engine, CIWebViewEngineLoop* loop, UInt32 idleWakeMs )
    : CORE::CTaskConsumer()
    , CORE::CTSharedPtrCreator< CWebViewEngineHost, MT::CMutex >( this )
    , m_engine( engine )
    , m_loop( loop )
    , m_idleWakeMs( idleWakeMs )
    , m_mail()
    , m_started( 0 )
    , m_running( 0 )
    , m_stopping( 0 )
    , m_stalled( 0 )
    , m_busySinceMs( 0 )
    , m_firstUnservedMs( 0 )
    , m_executed( 0 )
    , m_threadId( 0 )
    , m_executingLock()
    , m_executing( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewEngineHost::~CWebViewEngineHost()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    FailQueued( WEBVIEWERROR_SHUTTING_DOWN, "The web view engine is shutting down" );
}

/*-------------------------------------------------------------------------*/

UInt64
CWebViewEngineHost::NowMs( void )
{GUCEF_TRACE;

    return (UInt64) MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineHost::Start( const CString& threadPoolName )
{GUCEF_TRACE;

    if ( 0 != m_started.Exchange( 1 ) )
        return true;
    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( threadPoolName, true );
    if ( pool.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebViewEngineHost: The thread pool \"" << threadPoolName << "\" is not available" );
        m_started.Store( 0 );
        return false;
    }
    CORE::CFutureResult future = pool->StartTaskWithConsumer( CreateSharedPtr() );
    if ( future.HasNoFuture() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebViewEngineHost: The engine thread could not be started on the pool \"" << threadPoolName << "\"" );
        m_started.Store( 0 );
        return false;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "WebViewEngineHost: Started the engine thread on the pool \"" << threadPoolName << "\"" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineHost::Post( const CWebViewCommand& command )
{GUCEF_TRACE;

    if ( 0 != m_stopping.Load() || 0 != m_stalled.Load() )
        return false;
    CWebViewCommand posted( command );
    posted.postedAtMs = NowMs();
    UInt64 none = 0;
    m_firstUnservedMs.CompareExchangeStrong( none, posted.postedAtMs );
    if ( !m_mail.AddMail( posted ) )
        return false;
    if ( GUCEF_NULL != m_loop )
        m_loop->Wake();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebViewEngineHost::Wake( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_loop )
        m_loop->Wake();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineHost::Stop( UInt32 timeoutMs )
{GUCEF_TRACE;

    m_stopping.Store( 1 );
    Wake();
    if ( 0 == m_started.Load() )
        return true;
    if ( IsEngineThread() )
        return false;
    RequestTaskToStop( false );
    Wake();
    const bool ended = WaitForTaskToFinish( (Int32) ( timeoutMs > 0x7FFFFFFF ? 0x7FFFFFFF : timeoutMs ) );
    if ( !ended )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebViewEngineHost: The engine thread did not end within " << timeoutMs << " ms" );
    return ended;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineHost::IsStarted( void ) const
{GUCEF_TRACE;

    return 0 != m_started.Load();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineHost::IsRunning( void ) const
{GUCEF_TRACE;

    return 0 != m_running.Load();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineHost::IsStopping( void ) const
{GUCEF_TRACE;

    return 0 != m_stopping.Load();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineHost::IsEngineThread( void ) const
{GUCEF_TRACE;

    const TThreadId id = m_threadId.Load();
    return 0 != id && MT::GetCurrentThreadID() == id;
}

/*-------------------------------------------------------------------------*/

TThreadId
CWebViewEngineHost::GetEngineThreadId( void ) const
{GUCEF_TRACE;

    return m_threadId.Load();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineHost::CheckWatchdog( UInt32 watchdogMs, bool& newlyStalled )
{GUCEF_TRACE;

    newlyStalled = false;
    if ( 0 != m_stalled.Load() )
        return true;
    if ( 0 == watchdogMs || 0 == m_started.Load() || 0 != m_stopping.Load() )
        return false;
    const UInt64 now = NowMs();
    const UInt64 busySince = m_busySinceMs.Load();
    const UInt64 firstUnserved = m_firstUnservedMs.Load();
    const bool stalled = ( 0 != busySince && now > busySince && now - busySince > watchdogMs ) ||
                         ( 0 != firstUnserved && now > firstUnserved && now - firstUnserved > watchdogMs );
    if ( !stalled )
        return false;
    if ( 0 != m_stalled.Exchange( 1 ) )
        return true;

    newlyStalled = true;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebViewEngineHost: The engine thread did not answer within " << watchdogMs << " ms: its pending commands fail" );
    FailQueued( WEBVIEWERROR_ENGINE_STALLED, "The web view engine did not answer in time" );
    MT::CScopeMutex lock( m_executingLock );
    if ( GUCEF_NULL != m_executing )
        m_executing->Complete( GUCEF_NEW CWebViewCommandResult( WEBVIEWERROR_ENGINE_STALLED, "The web view engine did not answer in time" ), true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineHost::IsStalled( void ) const
{GUCEF_TRACE;

    return 0 != m_stalled.Load();
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewEngineHost::GetPendingCommandCount( void ) const
{GUCEF_TRACE;

    return m_mail.AmountOfMail();
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewEngineHost::GetExecutedCommandCount( void ) const
{GUCEF_TRACE;

    return m_executed.Load();
}

/*-------------------------------------------------------------------------*/

void
CWebViewEngineHost::FailQueued( TWebViewError error, const CString& message )
{GUCEF_TRACE;

    CWebViewCommand command;
    while ( m_mail.GetMail( command, GUCEF_NULL ) )
        FailCommand( command, error, message );
}

/*-------------------------------------------------------------------------*/

void
CWebViewEngineHost::ProcessMail( void )
{GUCEF_TRACE;

    // a command posted from now on sets the time again: what is in the mailbox now is taken below
    m_firstUnservedMs.Store( 0 );
    CWebViewCommand command;
    while ( 0 == m_stalled.Load() && m_mail.GetMail( command, GUCEF_NULL ) )
    {
        {
            MT::CScopeMutex lock( m_executingLock );
            m_executing = command.GetCompletion();
        }
        m_busySinceMs.Store( NowMs() );
        if ( GUCEF_NULL != m_engine )
            m_engine->ExecuteCommand( command );
        else
            FailCommand( command, WEBVIEWERROR_SHUTTING_DOWN, "The web view engine is gone" );
        m_busySinceMs.Store( 0 );
        {
            MT::CScopeMutex lock( m_executingLock );
            m_executing = GUCEF_NULL;
        }
        ++m_executed;
    }
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineHost::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    m_threadId.Store( MT::GetCurrentThreadID() );
    CWebViewGlobal::Instance()->SetThreadMustNotWait( MT::GetCurrentThreadID(), true );
    if ( GUCEF_NULL == m_loop || !m_loop->OnLoopStart() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebViewEngineHost: The loop of the engine could not start on the engine thread" );
        return false;
    }
    if ( GUCEF_NULL != m_engine && !m_engine->OnEngineThreadStarted() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebViewEngineHost: The engine could not start on the engine thread" );
        m_loop->OnLoopStop();
        return false;
    }
    m_running.Store( 1 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineHost::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    // the thread stays in here, sleeping in the loop of the engine, until it is asked to stop
    for ( ;; )
    {
        if ( 0 != m_stopping.Load() || IsDeactivationRequested() )
            return true;
        ProcessMail();
        UInt32 wait = CIWebViewEngineLoop::WaitForever;
        if ( GUCEF_NULL != m_engine )
        {
            const UInt64 now = NowMs();
            m_engine->OnEngineTick( now );
            wait = m_engine->GetTickWaitMs( now );
        }
        if ( 0 != m_idleWakeMs && m_idleWakeMs < wait )
            wait = m_idleWakeMs;
        if ( 0 != m_stopping.Load() || IsDeactivationRequested() )
            return true;
        if ( m_mail.HasMail() && 0 == m_stalled.Load() )
            continue;
        m_loop->RunOnce( wait );
    }
}

/*-------------------------------------------------------------------------*/

void
CWebViewEngineHost::OnTaskEnding( CORE::CTaskPtr task, bool willBeForced )
{GUCEF_TRACE;

    m_stopping.Store( 1 );
    FailQueued( WEBVIEWERROR_SHUTTING_DOWN, "The web view engine is shutting down" );
    if ( GUCEF_NULL != m_engine )
        m_engine->OnEngineThreadStopping();
    if ( GUCEF_NULL != m_loop )
        m_loop->OnLoopStop();
}

/*-------------------------------------------------------------------------*/

void
CWebViewEngineHost::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    m_running.Store( 0 );
    CWebViewGlobal::Instance()->SetThreadMustNotWait( m_threadId.Load(), false );
}

/*-------------------------------------------------------------------------*/

CString
CWebViewEngineHost::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewEngineHost::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::WEB::CWebViewEngineHost";
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
