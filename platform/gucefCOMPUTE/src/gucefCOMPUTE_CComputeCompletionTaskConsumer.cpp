/*
 *  gucefCOMPUTE: GUCEF module providing compute: devices, buffers and kernels over pluggable backends
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

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEQUEUE_H
#include "gucefCOMPUTE_CComputeQueue.h"
#define GUCEF_COMPUTE_CCOMPUTEQUEUE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEQUEUE_H ? */

#include "gucefCOMPUTE_CComputeCompletionTaskConsumer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CComputeCompletionTaskConsumer::TaskType = "GUCEF::COMPUTE::CComputeCompletionTaskConsumer";
const CString CComputeCompletionTaskConsumer::ThreadPoolName = "ComputeCompletion";
const UInt32 CComputeCompletionTaskConsumer::WaitSliceInMs = 100;

/*-------------------------------------------------------------------------*/

CComputeCompletionTaskConsumer::CComputeCompletionTaskConsumer( CComputeQueue* queue )
    : CORE::CTaskConsumer()
    , m_queue( queue )
    , m_pendingLock()
    , m_pending()
    , m_pendingHead( 0 )
    , m_wake( 0 )
    , m_stopRequested( 0 )
    , m_loopRunning( 0 )
    , m_loopThread( 0 )
    , m_started( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeCompletionTaskConsumer::~CComputeCompletionTaskConsumer()
{GUCEF_TRACE;

    // Stop() ran before, whatever is left (a task that never started) fails here
    FailAll( COMPUTERESULT_BACKEND_ERROR, "the queue was destroyed" );
}

/*-------------------------------------------------------------------------*/

CString
CComputeCompletionTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

const CString&
CComputeCompletionTaskConsumer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CComputeCompletionTaskConsumer::Start( const TComputeCompletionTaskConsumerPtr& self )
{GUCEF_TRACE;

    if ( m_started )
        return true;

    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( ThreadPoolName );
    if ( pool.IsNULL() )
        return false;

    m_stopRequested.Store( 0, MT::MEMORDER_RELEASE );
    m_loopRunning.Store( 1, MT::MEMORDER_RELEASE );
    CORE::CFutureResult started = pool->StartTaskWithConsumer( self );
    if ( started.HasNoFuture() )
    {
        m_loopRunning.Store( 0, MT::MEMORDER_RELEASE );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ComputeCompletionTaskConsumer: No thread for the completion task on pool " << ThreadPoolName );
        return false;
    }
    m_started = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CComputeCompletionTaskConsumer::Stop( TComputeResult result )
{GUCEF_TRACE;

    if ( m_started && IsCalledFromTask() )
    {
        // the loop is further up this very stack: it sees the request when the current submission is finished
        m_stopRequested.Store( 1, MT::MEMORDER_RELEASE );
        m_started = false;
    }
    else
    if ( m_started )
    {
        m_stopRequested.Store( 1, MT::MEMORDER_RELEASE );
        m_wake.Post();

        UInt32 waitedInMs = 0;
        while ( 0 != m_loopRunning.Load( MT::MEMORDER_ACQUIRE ) )
        {
            MT::PrecisionDelay( 1 );
            if ( ++waitedInMs == 10000 )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "ComputeCompletionTaskConsumer: The completion task does not end, still waiting" );
            }
        }
        m_started = false;
    }
    FailAll( result, "the queue shut down" );
}

/*-------------------------------------------------------------------------*/

void
CComputeCompletionTaskConsumer::Enqueue( CComputePendingWork* work )
{GUCEF_TRACE;

    if ( GUCEF_NULL == work )
        return;
    {
        MT::CScopeMutex lock( m_pendingLock );
        m_pending.push_back( work );
    }
    m_wake.Post();
}

/*-------------------------------------------------------------------------*/

void
CComputeCompletionTaskConsumer::Wake( void )
{GUCEF_TRACE;

    m_wake.Post();
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeCompletionTaskConsumer::GetPendingCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_pendingLock );
    return static_cast< UInt32 >( m_pending.size() ) - m_pendingHead;
}

/*-------------------------------------------------------------------------*/

bool
CComputeCompletionTaskConsumer::IsRunning( void ) const
{GUCEF_TRACE;

    return 0 != m_loopRunning.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

bool
CComputeCompletionTaskConsumer::IsCalledFromTask( void ) const
{GUCEF_TRACE;

    return 0 != m_loopRunning.Load( MT::MEMORDER_ACQUIRE ) && MT::GetCurrentThreadID() == m_loopThread.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

CComputePendingWork*
CComputeCompletionTaskConsumer::PeekOldest( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_pendingLock );
    if ( m_pendingHead < m_pending.size() )
        return m_pending[ m_pendingHead ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CComputeCompletionTaskConsumer::FinishOldest( TComputeResult result, const CString& errorText )
{GUCEF_TRACE;

    CComputePendingWork* work = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_pendingLock );
        if ( m_pendingHead >= m_pending.size() )
            return;
        work = m_pending[ m_pendingHead ];
        m_pending[ m_pendingHead ] = GUCEF_NULL;
        ++m_pendingHead;
        if ( m_pendingHead == m_pending.size() )
        {
            // keeps the capacity, so steady use does not allocate
            m_pending.clear();
            m_pendingHead = 0;
        }
    }

    // outside the lock: completing a future may notify observers, the references are released here (deferred release)
    work->Finish( result, errorText );
    GUCEF_DELETE work;
}

/*-------------------------------------------------------------------------*/

void
CComputeCompletionTaskConsumer::FailAll( TComputeResult result, const CString& errorText )
{GUCEF_TRACE;

    GUCEF::vector< CComputePendingWork* > failed;
    {
        MT::CScopeMutex lock( m_pendingLock );
        for ( size_t i=m_pendingHead; i<m_pending.size(); ++i )
            failed.push_back( m_pending[ i ] );
        m_pending.clear();
        m_pendingHead = 0;
    }
    for ( size_t i=0; i<failed.size(); ++i )
    {
        failed[ i ]->Finish( result, errorText );
        GUCEF_DELETE failed[ i ];
    }
}

/*-------------------------------------------------------------------------*/

void
CComputeCompletionTaskConsumer::RunLoop( void )
{GUCEF_TRACE;

    // the loop also ends when the thread pool stops the task, as the application does when it shuts down
    while ( 0 == m_stopRequested.Load( MT::MEMORDER_ACQUIRE ) && !IsDeactivationRequested() )
    {
        CComputePendingWork* work = PeekOldest();
        if ( GUCEF_NULL == work )
        {
            m_wake.WaitOrTimeout( WaitSliceInMs );
            continue;
        }

        CString errorText;
        TComputeResult result = COMPUTERESULT_BACKEND_ERROR;
        try
        {
            result = m_queue->ProcessPendingWork( *work, WaitSliceInMs, errorText );
        }
        catch ( ... )
        {
            errorText = "an exception was raised while the work ran";
        }

        if ( COMPUTERESULT_TIMEOUT == result )
            continue;
        if ( COMPUTERESULT_DEVICE_LOST == result )
        {
            FinishOldest( result, errorText );
            FailAll( COMPUTERESULT_DEVICE_LOST, "the device was lost" );
            continue;
        }
        FinishOldest( result, errorText );
    }

    // Stopped by its thread pool rather than by its queue (the application shuts down): the queue still exists, it
    // closes so new work is refused. Nothing that is pending stays behind: it fails, which also releases its
    // references, so the device and the queue can go
    if ( 0 == m_stopRequested.Load( MT::MEMORDER_ACQUIRE ) )
    {
        m_queue->CloseFromCompletionTask( COMPUTERESULT_BACKEND_ERROR );
        FailAll( COMPUTERESULT_BACKEND_ERROR, "the completion task was stopped by its thread pool" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CComputeCompletionTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    // One cycle runs the loop until the queue stops it: the queue is not touched after this returns, which is what Stop() waits for
    m_loopThread.Store( MT::GetCurrentThreadID(), MT::MEMORDER_RELEASE );
    RunLoop();
    m_loopRunning.Store( 0, MT::MEMORDER_RELEASE );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CComputeCompletionTaskConsumer::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    // also when the cycle never ran, for example when the pool shut down first
    m_loopRunning.Store( 0, MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
