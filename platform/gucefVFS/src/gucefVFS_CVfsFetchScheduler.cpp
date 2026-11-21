/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSFETCHTASKDATA_H
#include "gucefVFS_CVfsFetchTaskData.h"
#define GUCEF_VFS_CVFSFETCHTASKDATA_H
#endif /* GUCEF_VFS_CVFSFETCHTASKDATA_H ? */

#ifndef GUCEF_VFS_CVFSFETCHTASKCONSUMER_H
#include "gucefVFS_CVfsFetchTaskConsumer.h"
#define GUCEF_VFS_CVFSFETCHTASKCONSUMER_H
#endif /* GUCEF_VFS_CVFSFETCHTASKCONSUMER_H ? */

#include "gucefVFS_CVfsFetchScheduler.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct CVfsFetchScheduler::SEntry
{
    CIVfsFetchJobPtr job;
    CORE::CString key;
    const void* owner;
    MT::CTAtomic< bool > cancel;
    bool inQueue;                       /**< waits in the queue for room */
    bool started;                       /**< counted in the running jobs: given to the pool, or running */
    bool accounted;                     /**< the running count was given back */
    bool discard;                       /**< CancelAll() took it: it ends silently */
    UInt64 sequence;

    SEntry( void ) : job(), key(), owner( GUCEF_NULL ), cancel( false ), inQueue( false ), started( false ), accounted( false ), discard( false ), sequence( 0 ) {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CVfsFetchScheduler::DefaultThreadPoolName = "VfsRemoteOps";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsFetchScheduler::SCounters::SCounters( void )
    : accepted( 0 )
    , merged( 0 )
    , refused( 0 )
    , completed( 0 )
    , failed( 0 )
    , cancelled( 0 )
    , running( 0 )
    , queued( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsFetchScheduler::CVfsFetchScheduler( void )
    : m_lock()
    , m_threadPoolName( DefaultThreadPoolName )
    , m_workerThreads( 4 )
    , m_maxRunning( 8 )
    , m_maxQueued( 256 )
    , m_cancelWaitMs( 30000 )
    , m_active()
    , m_queue()
    , m_nextSequence( 1 )
    , m_running( 0 )
    , m_accepted( 0 )
    , m_merged( 0 )
    , m_refused( 0 )
    , m_completed( 0 )
    , m_failed( 0 )
    , m_cancelled( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsFetchScheduler::CVfsFetchScheduler( const CORE::CString& threadPoolName )
    : m_lock()
    , m_threadPoolName( threadPoolName )
    , m_workerThreads( 4 )
    , m_maxRunning( 8 )
    , m_maxQueued( 256 )
    , m_cancelWaitMs( 30000 )
    , m_active()
    , m_queue()
    , m_nextSequence( 1 )
    , m_running( 0 )
    , m_accepted( 0 )
    , m_merged( 0 )
    , m_refused( 0 )
    , m_completed( 0 )
    , m_failed( 0 )
    , m_cancelled( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsFetchScheduler::~CVfsFetchScheduler()
{GUCEF_TRACE;

    CancelAll( GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CVfsFetchScheduler::SetMaxRunning( UInt32 maxRunning )
{GUCEF_TRACE;

    TEntryVector toStart;
    {
        MT::CScopeMutex lock( m_lock );
        m_maxRunning = maxRunning < 1 ? 1 : maxRunning;
        FillStartList( toStart );
    }
    StartEntries( toStart );
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsFetchScheduler::GetMaxRunning( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_maxRunning;
}

/*-------------------------------------------------------------------------*/

void
CVfsFetchScheduler::SetMaxQueued( UInt32 maxQueued )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_maxQueued = maxQueued;
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsFetchScheduler::GetMaxQueued( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_maxQueued;
}

/*-------------------------------------------------------------------------*/

void
CVfsFetchScheduler::SetThreadPoolName( const CORE::CString& threadPoolName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_threadPoolName = threadPoolName.IsNULLOrEmpty() ? DefaultThreadPoolName : threadPoolName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsFetchScheduler::GetThreadPoolName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_threadPoolName;
}

/*-------------------------------------------------------------------------*/

void
CVfsFetchScheduler::SetWorkerThreads( UInt32 workerThreads )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_workerThreads = workerThreads < 1 ? 1 : workerThreads;
}

/*-------------------------------------------------------------------------*/

void
CVfsFetchScheduler::SetCancelWaitMs( UInt32 cancelWaitMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_cancelWaitMs = cancelWaitMs;
}

/*-------------------------------------------------------------------------*/

void
CVfsFetchScheduler::FillStartList( TEntryVector& toStart )
{GUCEF_TRACE;

    // the lock is held by the caller
    while ( m_running < m_maxRunning && !m_queue.empty() )
    {
        TQueueMap::iterator first = m_queue.begin();
        TEntryPtr entry = first->second;
        m_queue.erase( first );
        entry->inQueue = false;
        entry->started = true;
        ++m_running;
        toStart.push_back( entry );
    }
}

/*-------------------------------------------------------------------------*/

bool
CVfsFetchScheduler::QueueOnPool( TEntryPtr entry )
{GUCEF_TRACE;

    CORE::CString poolName;
    UInt32 workerThreads = 0;
    {
        MT::CScopeMutex lock( m_lock );
        poolName = m_threadPoolName;
        workerThreads = m_workerThreads;
    }

    try
    {
        CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( poolName );
        if ( pool.IsNULL() )
            return false;
        if ( 0 == pool->GetDesiredMinNrOfWorkerThreads() )
            pool->SetDesiredMinNrOfWorkerThreads( workerThreads );

        CVfsFetchTaskData taskData( this, entry->job, entry->key );
        CORE::CFutureResult future = pool->QueueTask( CVfsFetchTaskConsumer::TaskType, &taskData );
        return future.HasAFuture();
    }
    catch ( const timeout_exception& )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VfsFetchScheduler: Failed to queue a job on the pool \"" << poolName << "\": timeout" );
    }
    catch ( const std::exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VfsFetchScheduler: Failed to queue a job on the pool \"" << poolName << "\": " << e.what() );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CVfsFetchScheduler::StartEntries( TEntryVector& toStart )
{GUCEF_TRACE;

    for ( size_t i=0; i<toStart.size(); ++i )
    {
        if ( !QueueOnPool( toStart[ i ] ) )
            FinishEntry( toStart[ i ], CVfsCallResult( CORE::FILESYSTEMACCESS_QUEUEING_FAILED, "the job could not be given to the thread pool" ), CVfsArrivalInfo(), false );
    }
    toStart.clear();
}

/*-------------------------------------------------------------------------*/

CVfsFetchScheduler::ESubmitResult
CVfsFetchScheduler::Submit( const CIVfsFetchJobPtr& job )
{GUCEF_TRACE;

    if ( job.IsNULL() )
        return SUBMIT_REFUSED;

    const CORE::CString key = job->GetKey();
    TEntryVector toStart;
    {
        MT::CScopeMutex lock( m_lock );

        if ( m_active.find( key ) != m_active.end() )
        {
            ++m_merged;
            return SUBMIT_MERGED;
        }

        if ( m_running >= m_maxRunning && m_queue.size() >= m_maxQueued )
        {
            ++m_refused;
            return SUBMIT_REFUSED;
        }

        TEntryPtr entry( GUCEF_NEW SEntry() );
        entry->job = job;
        entry->key = key;
        entry->owner = job->GetOwner();
        entry->inQueue = true;
        entry->sequence = m_nextSequence++;
        m_queue[ entry->sequence ] = entry;
        m_active[ key ] = entry;
        ++m_accepted;
        FillStartList( toStart );
    }

    StartEntries( toStart );
    return SUBMIT_ACCEPTED;
}

/*-------------------------------------------------------------------------*/

void
CVfsFetchScheduler::RunJob( const CORE::CString& key, const CIVfsFetchJobPtr& job )
{GUCEF_TRACE;

    TEntryPtr entry;
    {
        MT::CScopeMutex lock( m_lock );
        TEntryMap::iterator i = m_active.find( key );
        if ( i != m_active.end() && i->second->job.GetPointerAlways() == job.GetPointerAlways() )
            entry = i->second;
    }
    if ( entry.IsNULL() )
        return;         // dropped by CancelAll() whose wait ran out: the job is not run

    CVfsCallResult result;
    CVfsArrivalInfo outcome;
    bool ran = false;
    if ( entry->cancel.Load() )
    {
        result = CVfsCallResult( CORE::FILESYSTEMACCESS_CANCELLED, "cancelled before it ran" );
    }
    else
    {
        ran = true;
        try
        {
            result = entry->job->Run( entry->cancel, outcome );
        }
        catch ( const std::exception& e )
        {
            result = CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, CORE::CString( "exception: " ) + e.what() );
        }
        catch ( ... )
        {
            result = CVfsCallResult( CORE::FILESYSTEMACCESS_FAILED, "exception" );
        }
    }

    FinishEntry( entry, result, outcome, ran );
}

/*-------------------------------------------------------------------------*/

void
CVfsFetchScheduler::FinishEntry( TEntryPtr entry, const CVfsCallResult& result, const CVfsArrivalInfo& outcome, bool ran )
{GUCEF_TRACE;

    (void) ran;
    bool emit = false;
    {
        MT::CScopeMutex lock( m_lock );

        if ( entry->discard )
        {
            ++m_cancelled;
        }
        else
        {
            emit = true;
            if ( CORE::FILESYSTEMACCESS_CANCELLED == result.result )
                ++m_cancelled;
            else
            if ( result.IsSuccess() )
                ++m_completed;
            else
                ++m_failed;
        }
    }

    // the job stays in the active list while it reports: CancelAll() waits for that, so nothing is reported after it returns
    if ( emit )
    {
        try
        {
            entry->job->OnEnded( result, outcome );
        }
        catch ( const std::exception& e )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VfsFetchScheduler: The end of the job \"" << entry->key << "\" threw: " << e.what() );
        }
        catch ( ... )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VfsFetchScheduler: The end of the job \"" << entry->key << "\" threw" );
        }
    }

    TEntryVector toStart;
    {
        MT::CScopeMutex lock( m_lock );

        TEntryMap::iterator i = m_active.find( entry->key );
        if ( i != m_active.end() && i->second.GetPointerAlways() == entry.GetPointerAlways() )
            m_active.erase( i );
        if ( entry->inQueue )
        {
            m_queue.erase( entry->sequence );
            entry->inQueue = false;
        }
        if ( entry->started && !entry->accounted )
        {
            entry->accounted = true;
            --m_running;
        }
        FillStartList( toStart );
    }
    StartEntries( toStart );
}

/*-------------------------------------------------------------------------*/

bool
CVfsFetchScheduler::CancelJob( const CORE::CString& key )
{GUCEF_TRACE;

    TEntryPtr queuedEntry;
    {
        MT::CScopeMutex lock( m_lock );
        TEntryMap::iterator i = m_active.find( key );
        if ( i == m_active.end() )
            return false;

        TEntryPtr entry = i->second;
        entry->cancel.Store( true );
        if ( entry->inQueue )
        {
            // it never ran: out of the queue and ended here
            m_queue.erase( entry->sequence );
            entry->inQueue = false;
            queuedEntry = entry;
        }
    }

    if ( !queuedEntry.IsNULL() )
        FinishEntry( queuedEntry, CVfsCallResult( CORE::FILESYSTEMACCESS_CANCELLED, "cancelled while it waited" ), CVfsArrivalInfo(), false );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsFetchScheduler::CancelAll( const void* owner )
{GUCEF_TRACE;

    TEntryVector waitFor;
    UInt32 dropped = 0;
    UInt32 waitMs = 0;
    {
        MT::CScopeMutex lock( m_lock );
        waitMs = m_cancelWaitMs;

        TEntryMap::iterator i = m_active.begin();
        while ( i != m_active.end() )
        {
            TEntryPtr entry = i->second;
            if ( GUCEF_NULL != owner && entry->owner != owner )
            {
                ++i;
                continue;
            }

            ++dropped;
            entry->discard = true;
            entry->cancel.Store( true );
            if ( entry->inQueue )
            {
                // never started: gone at once and silent
                m_queue.erase( entry->sequence );
                entry->inQueue = false;
                ++m_cancelled;
                m_active.erase( i++ );
            }
            else
            {
                waitFor.push_back( entry );
                ++i;
            }
        }
    }

    // the jobs that are with the pool: wait for them to return
    UInt32 waited = 0;
    bool allDone = waitFor.empty();
    while ( !allDone )
    {
        {
            MT::CScopeMutex lock( m_lock );
            allDone = true;
            for ( size_t n=0; n<waitFor.size(); ++n )
            {
                TEntryMap::iterator i = m_active.find( waitFor[ n ]->key );
                if ( i != m_active.end() && i->second.GetPointerAlways() == waitFor[ n ].GetPointerAlways() )
                {
                    allDone = false;
                    break;
                }
            }
        }
        if ( allDone )
            break;
        if ( waited >= waitMs )
        {
            // they are left to end by themselves: out of the books, so that a job that never started does not run later
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VfsFetchScheduler: " << waitFor.size() << " job(s) did not return within " << waitMs << " ms of the cancel" );
            MT::CScopeMutex lock( m_lock );
            for ( size_t n=0; n<waitFor.size(); ++n )
            {
                TEntryMap::iterator i = m_active.find( waitFor[ n ]->key );
                if ( i != m_active.end() && i->second.GetPointerAlways() == waitFor[ n ].GetPointerAlways() )
                    m_active.erase( i );
                if ( waitFor[ n ]->started && !waitFor[ n ]->accounted )
                {
                    waitFor[ n ]->accounted = true;
                    --m_running;
                }
            }
            break;
        }
        MT::ThreadDelay( 1 );
        ++waited;
    }
    return dropped;
}

/*-------------------------------------------------------------------------*/

bool
CVfsFetchScheduler::WaitUntilIdle( UInt32 timeoutInMs ) const
{GUCEF_TRACE;

    UInt32 waited = 0;
    for ( ;; )
    {
        {
            MT::CScopeMutex lock( m_lock );
            if ( m_active.empty() )
                return true;
        }
        if ( waited >= timeoutInMs )
            return false;
        MT::ThreadDelay( 1 );
        ++waited;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsFetchScheduler::GetRunningCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_running;
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsFetchScheduler::GetQueuedCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_queue.size();
}

/*-------------------------------------------------------------------------*/

void
CVfsFetchScheduler::GetCounters( SCounters& counters ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    counters.accepted = m_accepted;
    counters.merged = m_merged;
    counters.refused = m_refused;
    counters.completed = m_completed;
    counters.failed = m_failed;
    counters.cancelled = m_cancelled;
    counters.running = m_running;
    counters.queued = (UInt32) m_queue.size();
}

/*-------------------------------------------------------------------------*/

bool
CVfsFetchScheduler::HasJob( const CORE::CString& key ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_active.find( key ) != m_active.end();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
