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

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

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

#ifndef GUCEF_COMPUTE_ALIGNEDMEMORY_H
#include "gucefCOMPUTE_AlignedMemory.h"
#define GUCEF_COMPUTE_ALIGNEDMEMORY_H
#endif /* GUCEF_COMPUTE_ALIGNEDMEMORY_H ? */

#ifndef GUCEF_COMPUTE_CCPUWORKERTASKCONSUMER_H
#include "gucefCOMPUTE_CCpuWorkerTaskConsumer.h"
#define GUCEF_COMPUTE_CCPUWORKERTASKCONSUMER_H
#endif /* GUCEF_COMPUTE_CCPUWORKERTASKCONSUMER_H ? */

#include "gucefCOMPUTE_CCpuParallelFor.h"

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

const CString CCpuParallelFor::ThreadPoolName = "ComputeCpu";

/*-------------------------------------------------------------------------*/

CCpuParallelFor::CBody::CBody( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCpuParallelFor::CBody::~CBody()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCpuParallelFor::SParticipant::SParticipant( void )
    : done( 0 )
    , scratch( GUCEF_NULL )
    , scratchSize( 0 )
    , nextFree( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCpuParallelFor::SParticipant::~SParticipant()
{GUCEF_TRACE;

    AlignedFree( scratch );
}

/*-------------------------------------------------------------------------*/

bool
CCpuParallelFor::SParticipant::EnsureScratch( UInt32 size )
{GUCEF_TRACE;

    if ( size <= scratchSize )
        return true;
    void* grown = AlignedAllocate( size, GUCEF_COMPUTE_CPU_BUFFER_ALIGNMENT );
    if ( GUCEF_NULL == grown )
        return false;
    AlignedFree( scratch );
    scratch = grown;
    scratchSize = size;
    return true;
}

/*-------------------------------------------------------------------------*/

CCpuParallelFor::SJob::SJob( void )
    : body( GUCEF_NULL )
    , chunkCount( 0 )
    , scratchSize( 0 )
    , nextChunk( 0 )
    , failed( 0 )
    , helpers( 0 )
    , closed( false )
    , owner( GUCEF_NULL )
    , next( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCpuParallelFor::CCpuParallelFor( void )
    : CORE::CTSharedObjCreator< CCpuParallelFor, MT::CMutex >( this )
    , m_jobLock()
    , m_jobs( GUCEF_NULL )
    , m_freeParticipants( GUCEF_NULL )
    , m_allParticipants()
    , m_wake( 0 )
    , m_workerLock()
    , m_workers()
    , m_configuredWorkerCount( 0 )
    , m_workerCount( GetDefaultWorkerCount() )
    , m_shutDown( 0 )
    , m_runningWorkers( 0 )
    , m_workerStartFailed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCpuParallelFor::~CCpuParallelFor()
{GUCEF_TRACE;

    StopWorkers();
    for ( size_t i=0; i<m_allParticipants.size(); ++i )
        GUCEF_DELETE m_allParticipants[ i ];
    m_allParticipants.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CCpuParallelFor::GetDefaultWorkerCount( void )
{GUCEF_TRACE;

    const UInt32 cpus = CORE::GetLogicalCPUCount();
    return cpus > 1 ? cpus - 1 : 0;
}

/*-------------------------------------------------------------------------*/

void
CCpuParallelFor::SetWorkerCount( UInt32 workerCount )
{GUCEF_TRACE;

    GUCEF::vector< TWorkerPtr > stopping;
    {
        MT::CScopeMutex lock( m_workerLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        m_configuredWorkerCount = workerCount;
        m_workerCount.Store( 0 == workerCount ? GetDefaultWorkerCount() : workerCount, MT::MEMORDER_RELEASE );
        m_workerStartFailed = false;
        stopping.swap( m_workers );
    }
    StopWorkerSet( stopping );
}

/*-------------------------------------------------------------------------*/

UInt32
CCpuParallelFor::GetWorkerCount( void ) const
{GUCEF_TRACE;

    return m_workerCount.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

UInt32
CCpuParallelFor::GetRunningWorkerCount( void ) const
{GUCEF_TRACE;

    return m_runningWorkers.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

bool
CCpuParallelFor::EnsureWorkers( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_workerLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    const UInt32 wanted = m_workerCount.Load( MT::MEMORDER_ACQUIRE );
    if ( 0 == wanted || m_workerStartFailed || 0 != m_shutDown.Load( MT::MEMORDER_ACQUIRE ) )
        return false;
    if ( m_workers.size() >= wanted )
        return true;

    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( ThreadPoolName );
    if ( pool.IsNULL() )
    {
        m_workerStartFailed = true;
        return false;
    }

    while ( m_workers.size() < wanted )
    {
        TWorkerPtr worker( GUCEF_NEW CCpuWorkerTaskConsumer( this, static_cast< UInt32 >( m_workers.size() ) ) );
        worker->SetLoopRunning( true );
        CORE::CFutureResult started = pool->StartTaskWithConsumer( worker );
        if ( started.HasNoFuture() )
        {
            worker->SetLoopRunning( false );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CpuParallelFor: No thread for CPU worker " << m_workers.size() << " on pool " << ThreadPoolName <<
                ", running with " << m_workers.size() << " workers" );
            m_workerStartFailed = true;
            break;
        }
        m_workers.push_back( worker );
    }
    return !m_workers.empty();
}

/*-------------------------------------------------------------------------*/

void
CCpuParallelFor::StopWorkers( void )
{GUCEF_TRACE;

    GUCEF::vector< TWorkerPtr > stopping;
    {
        MT::CScopeMutex lock( m_workerLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        stopping.swap( m_workers );
    }
    StopWorkerSet( stopping );
}

/*-------------------------------------------------------------------------*/

void
CCpuParallelFor::StopWorkerSet( GUCEF::vector< TWorkerPtr >& workers )
{GUCEF_TRACE;

    // no lock is held: a worker whose chunk starts a run of its own may need the worker lock to finish
    if ( workers.empty() )
        return;
    for ( size_t i=0; i<workers.size(); ++i )
        workers[ i ]->RequestStop();
    for ( size_t i=0; i<workers.size(); ++i )
        m_wake.Post();

    const TThreadId self = MT::GetCurrentThreadID();
    for ( size_t i=0; i<workers.size(); ++i )
    {
        if ( workers[ i ]->IsRunningOn( self ) )
            continue;
        UInt32 waitedInMs = 0;
        while ( workers[ i ]->IsLoopRunning() )
        {
            MT::PrecisionDelay( 1 );
            if ( ++waitedInMs == 10000 )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "CpuParallelFor: CPU worker " << workers[ i ]->GetWorkerIndex() << " does not end, still waiting" );
            }
        }
    }
    workers.clear();
}

/*-------------------------------------------------------------------------*/

void
CCpuParallelFor::Shutdown( void )
{GUCEF_TRACE;

    m_shutDown.Store( 1, MT::MEMORDER_RELEASE );
    StopWorkers();
}

/*-------------------------------------------------------------------------*/

CCpuParallelFor::SParticipant*
CCpuParallelFor::AcquireParticipant( void )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_jobLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        if ( GUCEF_NULL != m_freeParticipants )
        {
            SParticipant* participant = m_freeParticipants;
            m_freeParticipants = participant->nextFree;
            participant->nextFree = GUCEF_NULL;
            return participant;
        }
    }

    // the pool grows to the number of concurrent callers, after that runs do not allocate
    SParticipant* participant = GUCEF_NEW SParticipant();
    MT::CScopeMutex lock( m_jobLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    m_allParticipants.push_back( participant );
    return participant;
}

/*-------------------------------------------------------------------------*/

void
CCpuParallelFor::ReleaseParticipant( SParticipant* participant )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_jobLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    participant->nextFree = m_freeParticipants;
    m_freeParticipants = participant;
}

/*-------------------------------------------------------------------------*/

void
CCpuParallelFor::RunChunks( SJob& job, void* scratch, UInt32 scratchSize )
{GUCEF_TRACE;

    for ( ;; )
    {
        const UInt32 chunk = job.nextChunk.FetchAdd( 1, MT::MEMORDER_ACQ_REL );
        if ( chunk >= job.chunkCount )
            return;
        try
        {
            job.body->ExecuteChunk( chunk, scratch, scratchSize );
        }
        catch ( ... )
        {
            job.failed.Store( 1, MT::MEMORDER_RELEASE );
        }
    }
}

/*-------------------------------------------------------------------------*/

TComputeResult
CCpuParallelFor::Run( CBody& body, UInt32 chunkCount, UInt32 scratchSizeInBytes, TComputeExecutionPolicy policy )
{GUCEF_TRACE;

    if ( 0 == chunkCount )
        return COMPUTERESULT_SUCCESS;

    SParticipant* participant = AcquireParticipant();
    if ( !participant->EnsureScratch( scratchSizeInBytes ) )
    {
        ReleaseParticipant( participant );
        return COMPUTERESULT_OUT_OF_HOST_MEMORY;
    }

    SJob job;
    job.body = &body;
    job.chunkCount = chunkCount;
    job.scratchSize = scratchSizeInBytes;
    job.owner = participant;

    const bool parallel = EXECUTIONPOLICY_CALLING_THREAD_ONLY != policy && chunkCount > 1 && EnsureWorkers();
    if ( !parallel )
    {
        RunChunks( job, participant->scratch, scratchSizeInBytes );
        ReleaseParticipant( participant );
        return 0 != job.failed.Load( MT::MEMORDER_ACQUIRE ) ? COMPUTERESULT_BACKEND_ERROR : COMPUTERESULT_SUCCESS;
    }

    {
        MT::CScopeMutex lock( m_jobLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        job.next = m_jobs;
        m_jobs = &job;
    }
    const UInt32 workers = GetWorkerCount();
    const UInt32 wakeCount = chunkCount - 1 < workers ? chunkCount - 1 : workers;
    for ( UInt32 i=0; i<wakeCount; ++i )
        m_wake.Post();

    RunChunks( job, participant->scratch, scratchSizeInBytes );

    // close the job: no new helper joins, the last helper that leaves wakes this thread
    bool mustWait = false;
    {
        MT::CScopeMutex lock( m_jobLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        SJob** link = &m_jobs;
        while ( GUCEF_NULL != *link && *link != &job )
            link = &( (*link)->next );
        if ( GUCEF_NULL != *link )
            *link = job.next;
        job.closed = true;
        mustWait = job.helpers > 0;
    }
    if ( mustWait )
        participant->done.Wait();

    ReleaseParticipant( participant );
    return 0 != job.failed.Load( MT::MEMORDER_ACQUIRE ) ? COMPUTERESULT_BACKEND_ERROR : COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

void
CCpuParallelFor::WorkerLoop( CCpuWorkerTaskConsumer& worker )
{GUCEF_TRACE;

    m_runningWorkers.FetchAdd( 1, MT::MEMORDER_ACQ_REL );

    // the worker also ends when its thread pool stops its task, as the application does when it shuts down
    while ( !worker.IsStopRequested() && !worker.IsDeactivationRequested() )
    {
        SJob* job = GUCEF_NULL;
        {
            MT::CScopeMutex lock( m_jobLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
            SJob* candidate = m_jobs;
            while ( GUCEF_NULL != candidate )
            {
                if ( candidate->nextChunk.Load( MT::MEMORDER_ACQUIRE ) < candidate->chunkCount )
                {
                    job = candidate;
                    ++job->helpers;
                    break;
                }
                candidate = candidate->next;
            }
        }

        if ( GUCEF_NULL == job )
        {
            m_wake.WaitOrTimeout( 250 );
            continue;
        }

        void* scratch = worker.GetScratch( job->scratchSize );
        if ( 0 == job->scratchSize || GUCEF_NULL != scratch )
            RunChunks( *job, scratch, job->scratchSize );

        MT::CScopeMutex lock( m_jobLock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        --job->helpers;
        if ( job->closed && 0 == job->helpers )
            job->owner->done.Post();
    }
    m_runningWorkers.FetchSub( 1, MT::MEMORDER_ACQ_REL );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
