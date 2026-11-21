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

#ifndef GUCEF_VFS_CVFSFETCHSCHEDULER_H
#define GUCEF_VFS_CVFSFETCHSCHEDULER_H

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

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_VFS_CIVFSFETCHJOB_H
#include "gucefVFS_CIVfsFetchJob.h"
#define GUCEF_VFS_CIVFSFETCHJOB_H
#endif /* GUCEF_VFS_CIVFSFETCHJOB_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Runs the background jobs of the remote archives (CIVfsFetchJob) on a pool of threads of its own, within bounds, and sees
 *  to it that every accepted job ends exactly once.
 *
 *  - Dedupe: a job whose key is queued or running is merged into that job; the caller can tell it is covered, and one end
 *    follows for all of them.
 *  - Bounds: at most MaxRunning jobs are with the pool at once and at most MaxQueued wait behind them, first in first out.
 *    Beyond that a job is refused and the caller says so (a job that is not accepted never ends, so whoever asked must not
 *    wait for it).
 *  - Ending: for every accepted job CIVfsFetchJob::OnEnded() is called once, with what Run() returned, or with
 *    FILESYSTEMACCESS_CANCELLED for a job that CancelJob() took out of the queue or that was told to stop, or
 *    FILESYSTEMACCESS_QUEUEING_FAILED when the pool did not take it. The one exception is CancelAll(): the jobs of an owner
 *    that is going away end silently and nothing of them is called after CancelAll() returned.
 *  - Shutdown: the destructor does CancelAll() for every owner.
 *
 *  The pool has a name of its own (default VfsRemoteOps) and is not the pool of the asynchronous VFS calls, so that
 *  background fetches that wait for the remote cannot starve the other work. A job that waits for the remote holds one
 *  thread of the pool: the bound on the running jobs is also the bound on the blocked threads.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsFetchScheduler
{
    public:

    static const CORE::CString DefaultThreadPoolName;           /**< "VfsRemoteOps" */

    enum ESubmitResult
    {
        SUBMIT_ACCEPTED = 0,            /**< the job will run and end */
        SUBMIT_MERGED,                  /**< the same work is under way: that job covers this one */
        SUBMIT_REFUSED                  /**< over the bounds: nothing was queued and nothing will end */
    };

    struct GUCEF_VFS_PUBLIC_CPP SCounters
    {
        UInt64 accepted;
        UInt64 merged;
        UInt64 refused;
        UInt64 completed;               /**< ended with a success */
        UInt64 failed;                  /**< ended with another result than a success or a cancel */
        UInt64 cancelled;               /**< ended cancelled, or dropped by CancelAll() */
        UInt32 running;                 /**< with the pool now */
        UInt32 queued;                  /**< waiting for room with the pool now */
        SCounters( void );
    };

    CVfsFetchScheduler( void );

    explicit CVfsFetchScheduler( const CORE::CString& threadPoolName );

    ~CVfsFetchScheduler();

    /** The jobs with the pool at once (default 8, at least 1) */
    void SetMaxRunning( UInt32 maxRunning );
    UInt32 GetMaxRunning( void ) const;

    /** The jobs that wait behind them (default 256, 0 means none wait: a job beyond the running ones is refused) */
    void SetMaxQueued( UInt32 maxQueued );
    UInt32 GetMaxQueued( void ) const;

    /** The pool the jobs run on; applies to the jobs that start from now on */
    void SetThreadPoolName( const CORE::CString& threadPoolName );
    CORE::CString GetThreadPoolName( void ) const;

    /** The number of worker threads the pool gets when this scheduler makes it (default 4) */
    void SetWorkerThreads( UInt32 workerThreads );

    /** How long CancelAll() waits for jobs that are running (default 30 seconds) */
    void SetCancelWaitMs( UInt32 cancelWaitMs );

    ESubmitResult Submit( const CIVfsFetchJobPtr& job );

    /**
     *  Cancels the job with the key: a queued job is taken out and ended (OnEnded() with FILESYSTEMACCESS_CANCELLED, on the
     *  calling thread), a running one is told to stop and ends as soon as Run() returns. False when no job has the key.
     */
    bool CancelJob( const CORE::CString& key );

    /**
     *  Drops the jobs of an owner without ending them (NULL: every owner): queued ones are removed, running ones are told
     *  to stop, and the call waits for them to return, at most the cancel wait. After it returns nothing of the jobs is
     *  called. Returns the number of jobs dropped.
     */
    UInt32 CancelAll( const void* owner );

    /** Waits until no job is queued or running; false when the time is up first */
    bool WaitUntilIdle( UInt32 timeoutInMs ) const;

    UInt32 GetRunningCount( void ) const;
    UInt32 GetQueuedCount( void ) const;
    void GetCounters( SCounters& counters ) const;

    /** Whether a job with the key is queued or running */
    bool HasJob( const CORE::CString& key ) const;

    /** Called by the task consumer on the pool thread */
    void RunJob( const CORE::CString& key, const CIVfsFetchJobPtr& job );

    private:

    CVfsFetchScheduler( const CVfsFetchScheduler& src );                    /**< not implemented */
    CVfsFetchScheduler& operator=( const CVfsFetchScheduler& src );         /**< not implemented */

    struct SEntry;
    typedef CORE::CTSharedPtr< SEntry, MT::CMutex > TEntryPtr;
    typedef GUCEF::map< CORE::CString, TEntryPtr > TEntryMap;
    typedef GUCEF::map< UInt64, TEntryPtr > TQueueMap;
    typedef GUCEF::vector< TEntryPtr > TEntryVector;

    void FillStartList( TEntryVector& toStart );
    void StartEntries( TEntryVector& toStart );
    bool QueueOnPool( TEntryPtr entry );
    void FinishEntry( TEntryPtr entry, const CVfsCallResult& result, const CVfsArrivalInfo& outcome, bool ran );

    private:

    mutable MT::CMutex m_lock;
    CORE::CString m_threadPoolName;
    UInt32 m_workerThreads;
    UInt32 m_maxRunning;
    UInt32 m_maxQueued;
    UInt32 m_cancelWaitMs;
    TEntryMap m_active;                     /**< queued and running, by key */
    TQueueMap m_queue;                      /**< the queued ones in order of arrival */
    UInt64 m_nextSequence;
    UInt32 m_running;
    UInt64 m_accepted;
    UInt64 m_merged;
    UInt64 m_refused;
    UInt64 m_completed;
    UInt64 m_failed;
    UInt64 m_cancelled;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSFETCHSCHEDULER_H ? */
