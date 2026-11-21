/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H

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
#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */
#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */
#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOB_H
#include "gucefASSETSTORE_CAssetStoreDownloadJob.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOB_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOB_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADSETTINGS_H
#include "gucefASSETSTORE_CAssetStoreDownloadSettings.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADSETTINGS_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADSETTINGS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOBEVENTDATA_H
#include "gucefASSETSTORE_CAssetStoreDownloadJobEventData.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOBEVENTDATA_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOBEVENTDATA_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#include "gucefASSETSTORE_CIAssetStoreProvider.h"
#define GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H ? */
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CAssetStoreRegistry;
class CAssetStoreDownloadTaskConsumer;
class CAssetStoreTransferContext;

/**
 *  Downloads of store items in the background (section 4.4 of the design).
 *
 *  A job is one item and format of a store, or chosen files of it, into a destination folder of the VFS. Jobs wait in
 *  a queue and run as tasks on a thread pool of the task manager (CAssetStoreDownloadSettings::threadPoolName), at most
 *  maxConcurrentJobs at once and at most maxConcurrentJobsPerProvider of one store. A job asks its store for a plan
 *  right before its transfers start, then fetches every file into "<name>.part" with the gucefWEB HTTP client
 *  (or the transfer source of the store), resuming with an HTTP range when the file is there in part, checks the
 *  size and the digest (SHA-1 or SHA-256) and renames the file.
 *
 *  Failures: network errors, 5xx and 429 are retried with exponential back-off and jitter (429 and 503 wait for
 *  Retry-After when given), up to maxAttempts per file; an expired address renews the plan once; 401 renews the
 *  session of the store once and then fails with ASSETSTORE_STATUS_SESSION_EXPIRED ("sign in again"). Names that
 *  could leave the destination are refused before anything is written (CAssetStoreSafeNames).
 *
 *  Pause keeps the ".part" files, so Resume() continues where the job was; Cancel() removes them. The queue is saved
 *  as JSON in the VFS file of the settings on every change and LoadQueue() takes it back after a restart: jobs that were
 *  running are queued again and resume from their ".part" files. Start() begins running queued jobs, Stop() pauses the
 *  running ones (queued again, not paused) and waits for their tasks.
 *
 *  Events are raised on the thread of the pulse generator of the manager: JobAddedEvent, JobStateChangedEvent,
 *  JobProgressEvent (throttled to one per progressIntervalInMs per job), JobFinishedEvent (done), JobFailedEvent
 *  (failed; the job holds the status and the reason) and JobRemovedEvent, each with a CAssetStoreDownloadJobEventData.
 *  Observers subscribe per event with CTEventHandlerFunctor. Thread safe.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreDownloadManager : public CORE::CTSGNotifier ,
                                                               public CORE::CIConfigurable
{
    public:

    static const CORE::CEvent JobAddedEvent;
    static const CORE::CEvent JobStateChangedEvent;
    static const CORE::CEvent JobProgressEvent;
    static const CORE::CEvent JobFinishedEvent;
    static const CORE::CEvent JobFailedEvent;
    static const CORE::CEvent JobRemovedEvent;

    static void RegisterEvents( void );

    /** Events reach observers on the thread of the global pulse generator; the stores of the global registry are used */
    CAssetStoreDownloadManager( void );

    /** Events reach observers on the thread of the given pulse generator */
    explicit CAssetStoreDownloadManager( CORE::PulseGeneratorPtr pulseGenerator );

    virtual ~CAssetStoreDownloadManager() GUCEF_VIRTUAL_OVERRIDE;

    void SetSettings( const CAssetStoreDownloadSettings& settings );
    CAssetStoreDownloadSettings GetSettings( void ) const;

    /** The stores the jobs refer to by id; GUCEF_NULL for the registry of CAssetStoreGlobal (the default). Not owned */
    void SetRegistry( CAssetStoreRegistry* registry );

    /** Begins running queued jobs */
    bool Start( void );

    /** Stops: running jobs stop their transfers, keep their ".part" files and are queued again; waits for their tasks */
    void Stop( void );

    bool IsStarted( void ) const;

    /**
     *  Adds a job to the queue: the format of the item of the store, the files with the given ids (every file of the
     *  format when none are given), into the VFS folder 'destination'. 'forIngest' marks a job for the primary store.
     *  NOT_FOUND for a store that is not registered, INVALID_ARGUMENT for an empty item or destination, IO_ERROR for a
     *  destination that no writeable mount covers.
     */
    TAssetStoreStatus Enqueue( const CORE::CString& providerId                ,
                               const CORE::CString& itemId                    ,
                               const CORE::CString& formatId                  ,
                               const CORE::CString::StringVector& fileIds     ,
                               const CORE::CString& destination               ,
                               bool forIngest                                 ,
                               CORE::CString& jobId                           ,
                               const CORE::CString& itemTitle = CORE::CString::Empty );

    /**
     *  Adds a job to the queue as the other form does, from the members of a job: providerId, itemId, itemTitle, formatId,
     *  fileIds, destination, forIngest and the members of the library sync (expectedRevision, syncJobId), which the job keeps
     *  in the queue file and gives to the ingest
     */
    TAssetStoreStatus Enqueue( const CAssetStoreDownloadJob& request, CORE::CString& jobId );

    /** A queued job waits; a running job stops its transfer and keeps its ".part" files */
    TAssetStoreStatus Pause( const CORE::CString& jobId );

    /** A paused job is queued again */
    TAssetStoreStatus Resume( const CORE::CString& jobId );

    /** The job ends as cancelled; its ".part" files are removed, complete files are kept */
    TAssetStoreStatus Cancel( const CORE::CString& jobId );

    /** A failed or cancelled job is queued again, with new attempts */
    TAssetStoreStatus Retry( const CORE::CString& jobId );

    /** Takes a job that is done, failed or cancelled out of the queue */
    TAssetStoreStatus Remove( const CORE::CString& jobId );

    bool GetJob( const CORE::CString& jobId, CAssetStoreDownloadJob& job ) const;

    /** Every job in the order of the queue */
    void GetJobs( TAssetStoreDownloadJobVector& jobs ) const;

    /**
     *  Blocks the calling thread until the job is done, failed, cancelled or paused, or the timeout passed; it does not
     *  pump a pulse generator. True when the job reached such a state
     */
    bool WaitForJob( const CORE::CString& jobId, UInt32 timeoutInMs ) const;

    UInt32 GetRunningJobCount( void ) const;

    /** The most jobs that ran at once, overall and of one store, since the last reset */
    UInt32 GetPeakRunningJobCount( void ) const;
    UInt32 GetPeakRunningJobCountOfProvider( void ) const;
    void ResetPeakRunningJobCounts( void );

    /** Writes the queue to the queue file of the settings; true without a queue file */
    bool SaveQueue( void );

    /**
     *  Reads the queue file of the settings and adds its jobs that the queue does not have; jobs that were planning,
     *  running or verifying are queued again. False when the file exists and cannot be read
     */
    bool LoadQueue( void );

    /** The settings, as a "downloads" object */
    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CAssetStoreDownloadTaskConsumer;
    friend class CAssetStoreTransferContext;

    struct SJobEntry
    {
        CAssetStoreDownloadJob job;
        bool taskActive;
        bool cancelRequested;
        bool pauseRequested;
        UInt64 lastProgressTicks;
        UInt64 lastProgressBytes;
        UInt64 bytesPerSecond;
    };
    typedef GUCEF::map< CORE::CString, SJobEntry* > TJobMap;

    struct SJobRun
    {
        CAssetStoreDownloadJob job;
        TAssetStoreProviderPtr provider;
        CAssetStoreDownloadPlan plan;
        bool planRenewed;
        bool sessionRenewed;
    };

    void Initialize( void );
    void Schedule( void );
    /** Marks a queued job as launching (under the lock of the manager) */
    void MarkJobLaunching( SJobEntry& entry );

    /** Queues the task of a job marked launching, without the lock of the manager: making a pool or a worker registers with the pulse generator, whose thread may wait for that lock. Puts the job back in the queue when it cannot be queued */
    bool QueueJobTask( const CORE::CString& jobId );

    void RunJob( const CORE::CString& jobId );
    void OnJobTaskEnded( const CORE::CString& jobId );
    void OnJobException( const CORE::CString& jobId );
    bool IsJobStopRequested( const CORE::CString& jobId ) const;
    void OnTransferProgress( const CORE::CString& jobId, UInt32 fileIndex, UInt64 bytesInFile );

    TAssetStoreStatus PlanJob( SJobRun& run, CORE::CString& message );
    TAssetStoreStatus RunFile( SJobRun& run, UInt32 fileIndex, CORE::CString& message );
    TAssetStoreStatus RunHttpTransfer( SJobRun& run, const CAssetStoreTransfer& transfer, CAssetStoreTransferContext& context, CORE::CString& message );
    TAssetStoreStatus VerifyFile( SJobRun& run, UInt32 fileIndex, const CAssetStoreTransfer& transfer, CORE::CString& message );
    TAssetStoreStatus StopStatusOf( const CORE::CString& jobId ) const;
    bool SleepUnlessStopped( const CORE::CString& jobId, UInt32 delayInMs ) const;
    UInt32 GetBackOffDelayInMs( UInt32 attempt, UInt32 retryAfterInMs ) const;

    void SetJobState( const CORE::CString& jobId, TAssetDownloadJobState state );
    void EndJob( const CORE::CString& jobId, TAssetDownloadJobState state, TAssetStoreStatus status, const CORE::CString& message );
    void RemovePartFiles( const CAssetStoreDownloadJob& job ) const;
    void NotifyJobEvent( const CORE::CEvent& eventId, const CAssetStoreDownloadJob& job );
    void SaveQueueIfAutomatic( void );
    TAssetStoreProviderPtr FindProvider( const CORE::CString& providerId ) const;
    UInt32 CountActiveTasks( const CORE::CString& providerId ) const;

    static CORE::CString GetPartFilePath( const CAssetStoreDownloadJob& job, const CAssetStoreJobFile& file );

    CAssetStoreDownloadManager( const CAssetStoreDownloadManager& src );              /**< not implemented */
    CAssetStoreDownloadManager& operator=( const CAssetStoreDownloadManager& src );   /**< not implemented */

    private:

    CAssetStoreDownloadSettings m_settings;
    CAssetStoreRegistry* m_registry;
    TJobMap m_jobs;
    UInt64 m_nextSequence;
    bool m_started;
    bool m_stopping;
    UInt32 m_activeTasks;
    UInt32 m_peakRunning;
    UInt32 m_peakRunningOfProvider;
    mutable MT::CMutex m_lock;
    MT::CMutex m_saveLock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H ? */
