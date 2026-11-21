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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */
#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */
#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */
#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */
#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */
#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */
#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */
#ifndef GUCEF_WEB_CHTTPLOGREDACTION_H
#include "gucefWEB_CHttpLogRedaction.h"
#define GUCEF_WEB_CHTTPLOGREDACTION_H
#endif /* GUCEF_WEB_CHTTPLOGREDACTION_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#include "gucefASSETSTORE_CAssetStoreRegistry.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H
#include "gucefASSETSTORE_CAssetStoreSafeNames.h"
#define GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETDIGEST_H
#include "gucefASSETSTORE_CAssetDigest.h"
#define GUCEF_ASSETSTORE_CASSETDIGEST_H
#endif /* GUCEF_ASSETSTORE_CASSETDIGEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORETRANSFERCONTEXT_H
#include "gucefASSETSTORE_CAssetStoreTransferContext.h"
#define GUCEF_ASSETSTORE_CASSETSTORETRANSFERCONTEXT_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORETRANSFERCONTEXT_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORETRANSFERSINK_H
#include "gucefASSETSTORE_CAssetStoreTransferSink.h"
#define GUCEF_ASSETSTORE_CASSETSTORETRANSFERSINK_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORETRANSFERSINK_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETSTORETRANSFERSOURCE_H
#include "gucefASSETSTORE_CIAssetStoreTransferSource.h"
#define GUCEF_ASSETSTORE_CIASSETSTORETRANSFERSOURCE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORETRANSFERSOURCE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADTASKCONSUMER_H
#include "gucefASSETSTORE_CAssetStoreDownloadTaskConsumer.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADTASKCONSUMER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADTASKCONSUMER_H ? */
#include "gucefASSETSTORE_CAssetStoreDownloadManager.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CAssetStoreDownloadManager::JobAddedEvent = "GUCEF::ASSETSTORE::CAssetStoreDownloadManager::JobAddedEvent";
const CORE::CEvent CAssetStoreDownloadManager::JobStateChangedEvent = "GUCEF::ASSETSTORE::CAssetStoreDownloadManager::JobStateChangedEvent";
const CORE::CEvent CAssetStoreDownloadManager::JobProgressEvent = "GUCEF::ASSETSTORE::CAssetStoreDownloadManager::JobProgressEvent";
const CORE::CEvent CAssetStoreDownloadManager::JobFinishedEvent = "GUCEF::ASSETSTORE::CAssetStoreDownloadManager::JobFinishedEvent";
const CORE::CEvent CAssetStoreDownloadManager::JobFailedEvent = "GUCEF::ASSETSTORE::CAssetStoreDownloadManager::JobFailedEvent";
const CORE::CEvent CAssetStoreDownloadManager::JobRemovedEvent = "GUCEF::ASSETSTORE::CAssetStoreDownloadManager::JobRemovedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The bytes of the text without the terminating zero, for every string format of CString */
inline UInt32
ByteLengthOf( const CORE::CString& text )
{
    const UInt32 size = text.ByteSize();
    return size > 0 ? size - 1 : 0;
}

Int64
NowUtcMs( void )
{GUCEF_TRACE;

    return (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

/** The value of a response header, the name compared without regard for case */
CORE::CString
FindResponseHeader( const WEB::CHttpResponseData& response, const CORE::CString& name )
{GUCEF_TRACE;

    WEB::CHttpResponseData::TStringMap::const_iterator i = response.otherHeaders.begin();
    while ( i != response.otherHeaders.end() )
    {
        if ( (*i).first.Equals( name, false ) )
            return (*i).second;
        ++i;
    }
    return CORE::CString::Empty;
}

/** Retry-After in seconds, as milliseconds; 0 when absent or a date (the back-off is used then) */
UInt32
ParseRetryAfterInMs( const WEB::CHttpResponseData& response )
{GUCEF_TRACE;

    const CORE::CString value = FindResponseHeader( response, "Retry-After" ).Trim( true ).Trim( false );
    if ( value.IsNULLOrEmpty() )
        return 0;
    const char* text = value.C_String();
    for ( UInt32 i=0; i<ByteLengthOf( value ); ++i )
        if ( text[ i ] < '0' || text[ i ] > '9' )
            return 0;
    const UInt64 seconds = CORE::StringToUInt64( value );
    return seconds > 86400 ? 86400u * 1000u : (UInt32) ( seconds * 1000 );
}

/** The folder of a VFS path */
CORE::CString
ParentOfVfsPath( const CORE::CString& path )
{GUCEF_TRACE;

    const Int32 slash = path.HasChar( '/', false );
    return slash > 0 ? path.SubstrToIndex( (UInt32) slash, true ) : CORE::CString::Empty;
}

bool
IsRetryable( TAssetStoreStatus status )
{GUCEF_TRACE;

    return ASSETSTORE_STATUS_NETWORK_ERROR == status || ASSETSTORE_STATUS_SERVER_ERROR == status || ASSETSTORE_STATUS_RATE_LIMITED == status;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CAssetStoreDownloadManager::RegisterEvents( void )
{GUCEF_TRACE;

    JobAddedEvent.Initialize();
    JobStateChangedEvent.Initialize();
    JobProgressEvent.Initialize();
    JobFinishedEvent.Initialize();
    JobFailedEvent.Initialize();
    JobRemovedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAssetStoreDownloadManager::CAssetStoreDownloadManager( void )
    : CORE::CTSGNotifier()
    , CORE::CIConfigurable()
    , m_settings()
    , m_registry( GUCEF_NULL )
    , m_jobs()
    , m_nextSequence( 1 )
    , m_started( false )
    , m_stopping( false )
    , m_activeTasks( 0 )
    , m_peakRunning( 0 )
    , m_peakRunningOfProvider( 0 )
    , m_lock()
    , m_saveLock()
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

CAssetStoreDownloadManager::CAssetStoreDownloadManager( CORE::PulseGeneratorPtr pulseGenerator )
    : CORE::CTSGNotifier( pulseGenerator )
    , CORE::CIConfigurable()
    , m_settings()
    , m_registry( GUCEF_NULL )
    , m_jobs()
    , m_nextSequence( 1 )
    , m_started( false )
    , m_stopping( false )
    , m_activeTasks( 0 )
    , m_peakRunning( 0 )
    , m_peakRunningOfProvider( 0 )
    , m_lock()
    , m_saveLock()
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreDownloadManager::Initialize( void )
{GUCEF_TRACE;

    // the global registers the task type of the jobs
    CAssetStoreGlobal::Instance();
    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CAssetStoreDownloadManager::~CAssetStoreDownloadManager()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    Stop();
    MT::CScopeMutex lock( m_lock );
    TJobMap::iterator i = m_jobs.begin();
    while ( i != m_jobs.end() )
    {
        GUCEF_DELETE (*i).second;
        ++i;
    }
    m_jobs.clear();
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreDownloadManager::SetSettings( const CAssetStoreDownloadSettings& settings )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_settings = settings;
        if ( m_settings.threadPoolName.IsNULLOrEmpty() )
            m_settings.threadPoolName = CAssetStoreDownloadSettings::DefaultThreadPoolName;
        if ( 0 == m_settings.maxAttempts )
            m_settings.maxAttempts = 1;
    }
    Schedule();
}

/*-------------------------------------------------------------------------*/

CAssetStoreDownloadSettings
CAssetStoreDownloadManager::GetSettings( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreDownloadManager::SetRegistry( CAssetStoreRegistry* registry )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_registry = registry;
}

/*-------------------------------------------------------------------------*/

TAssetStoreProviderPtr
CAssetStoreDownloadManager::FindProvider( const CORE::CString& providerId ) const
{GUCEF_TRACE;

    CAssetStoreRegistry* registry = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        registry = m_registry;
    }
    if ( GUCEF_NULL == registry )
        registry = &CAssetStoreGlobal::Instance()->GetRegistry();
    TAssetStoreProviderPtr provider;
    registry->TryGetProvider( providerId, provider );
    return provider;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreDownloadManager::Start( void )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_started = true;
        m_stopping = false;
    }
    Schedule();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreDownloadManager::Stop( void )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_started && 0 == m_activeTasks )
            return;
        m_started = false;
        m_stopping = true;
    }

    // the tasks see the stop in their transfers and queue their jobs again
    for ( ;; )
    {
        {
            MT::CScopeMutex lock( m_lock );
            if ( 0 == m_activeTasks )
                break;
        }
        MT::ThreadDelay( 5 );
    }
    {
        MT::CScopeMutex lock( m_lock );
        m_stopping = false;
    }
    SaveQueue();
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreDownloadManager::IsStarted( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_started;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreDownloadManager::CountActiveTasks( const CORE::CString& providerId ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    TJobMap::const_iterator i = m_jobs.begin();
    while ( i != m_jobs.end() )
    {
        const SJobEntry* entry = (*i).second;
        if ( entry->taskActive && ( providerId.IsNULLOrEmpty() || entry->job.providerId == providerId ) )
            ++count;
        ++i;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreDownloadManager::Schedule( void )
{GUCEF_TRACE;

    GUCEF::vector< CAssetStoreDownloadJob > started;
    CORE::CString::StringVector launching;
    {
        MT::CScopeMutex lock( m_lock );
        if ( !m_started || m_stopping )
            return;

        // the queued jobs in queue order
        GUCEF::vector< std::pair< UInt64, SJobEntry* > > queued;
        TJobMap::iterator i = m_jobs.begin();
        while ( i != m_jobs.end() )
        {
            SJobEntry* entry = (*i).second;
            if ( ASSETDOWNLOAD_STATE_QUEUED == entry->job.state && !entry->taskActive )
                queued.push_back( std::pair< UInt64, SJobEntry* >( entry->job.sequence, entry ) );
            ++i;
        }
        std::sort( queued.begin(), queued.end() );

        for ( size_t q=0; q<queued.size(); ++q )
        {
            SJobEntry& entry = *queued[ q ].second;
            if ( m_settings.maxConcurrentJobs > 0 && CountActiveTasks( CORE::CString::Empty ) >= m_settings.maxConcurrentJobs )
                break;
            if ( m_settings.maxConcurrentJobsPerProvider > 0 && CountActiveTasks( entry.job.providerId ) >= m_settings.maxConcurrentJobsPerProvider )
                continue;
            MarkJobLaunching( entry );
            launching.push_back( entry.job.jobId );
            started.push_back( entry.job );

            const UInt32 running = CountActiveTasks( CORE::CString::Empty );
            const UInt32 runningOfProvider = CountActiveTasks( entry.job.providerId );
            if ( running > m_peakRunning )
                m_peakRunning = running;
            if ( runningOfProvider > m_peakRunningOfProvider )
                m_peakRunningOfProvider = runningOfProvider;
        }
    }
    // the tasks go to the pool outside the lock (a new pool or worker registers with the pulse generator)
    for ( size_t l=0; l<launching.size(); ++l )
    {
        if ( !QueueJobTask( launching[ l ] ) )
        {
            for ( size_t s=0; s<started.size(); ++s )
                if ( started[ s ].jobId == launching[ l ] )
                {
                    started.erase( started.begin() + s );
                    break;
                }
        }
    }
    for ( size_t s=0; s<started.size(); ++s )
        NotifyJobEvent( JobStateChangedEvent, started[ s ] );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreDownloadManager::MarkJobLaunching( SJobEntry& entry )
{GUCEF_TRACE;

    entry.taskActive = true;
    entry.cancelRequested = false;
    entry.pauseRequested = false;
    entry.job.state = ASSETDOWNLOAD_STATE_PLANNING;
    entry.job.status = ASSETSTORE_STATUS_OK;
    entry.job.message.Clear();
    entry.job.updatedAtMs = NowUtcMs();
    ++m_activeTasks;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreDownloadManager::QueueJobTask( const CORE::CString& jobId )
{GUCEF_TRACE;

    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    CORE::ThreadPoolPtr threadPool = taskManager.GetOrCreateThreadPool( m_settings.threadPoolName );
    CAssetStoreDownloadTaskData* data = GUCEF_NULL;
    if ( !threadPool.IsNULL() )
    {
        // a pool has one worker unless asked for more: the jobs run side by side
        const UInt32 workers = 0 == m_settings.maxConcurrentJobs ? 16 : m_settings.maxConcurrentJobs;
        if ( threadPool->GetDesiredMinNrOfWorkerThreads() < workers )
            threadPool->SetDesiredMinNrOfWorkerThreads( workers );
        data = GUCEF_NEW CAssetStoreDownloadTaskData();
        data->manager = this;
        data->jobId = jobId;
        CORE::CFutureResult queued = threadPool->QueueTask( CAssetStoreDownloadTaskConsumer::TaskType, data, GUCEF_NULL, true );
        if ( !queued.HasNoFuture() )
            return true;
    }

    GUCEF_DELETE data;
    {
        MT::CScopeMutex lock( m_lock );
        TJobMap::iterator i = m_jobs.find( jobId );
        if ( i != m_jobs.end() && (*i).second->taskActive )
        {
            SJobEntry& entry = *(*i).second;
            entry.taskActive = false;
            entry.job.state = ASSETDOWNLOAD_STATE_QUEUED;
            if ( m_activeTasks > 0 )
                --m_activeTasks;
        }
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreDownloadManager: The task of job " << jobId << " could not be queued" );
    return false;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreDownloadManager::Enqueue( const CORE::CString& providerId                ,
                                     const CORE::CString& itemId                    ,
                                     const CORE::CString& formatId                  ,
                                     const CORE::CString::StringVector& fileIds     ,
                                     const CORE::CString& destination               ,
                                     bool forIngest                                 ,
                                     CORE::CString& jobId                           ,
                                     const CORE::CString& itemTitle                 )
{GUCEF_TRACE;

    CAssetStoreDownloadJob request;
    request.providerId = providerId;
    request.itemId = itemId;
    request.itemTitle = itemTitle;
    request.formatId = formatId;
    request.fileIds = fileIds;
    request.destination = destination;
    request.forIngest = forIngest;
    return Enqueue( request, jobId );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreDownloadManager::Enqueue( const CAssetStoreDownloadJob& request, CORE::CString& jobId )
{GUCEF_TRACE;

    const CORE::CString& providerId = request.providerId;
    const CORE::CString& itemId = request.itemId;
    const CORE::CString& destination = request.destination;
    if ( itemId.IsNULLOrEmpty() || destination.IsNULLOrEmpty() )
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    if ( FindProvider( providerId ).IsNULL() )
        return ASSETSTORE_STATUS_NOT_FOUND;
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().IsWriteable( destination ) )
        return ASSETSTORE_STATUS_IO_ERROR;

    SJobEntry* entry = GUCEF_NEW SJobEntry();
    entry->taskActive = false;
    entry->cancelRequested = false;
    entry->pauseRequested = false;
    entry->lastProgressTicks = 0;
    entry->lastProgressBytes = 0;
    entry->bytesPerSecond = 0;
    CAssetStoreDownloadJob& job = entry->job;
    job.providerId = providerId.Lowercase();
    job.itemId = itemId;
    job.itemTitle = request.itemTitle;
    job.formatId = request.formatId;
    job.fileIds = request.fileIds;
    job.destination = destination.ReplaceChar( '\\', '/' );
    job.forIngest = request.forIngest;
    job.expectedRevision = request.expectedRevision;
    job.syncJobId = request.syncJobId;
    job.state = ASSETDOWNLOAD_STATE_QUEUED;
    job.createdAtMs = NowUtcMs();
    job.updatedAtMs = job.createdAtMs;
    {
        MT::CScopeMutex lock( m_lock );
        job.sequence = m_nextSequence++;
        job.jobId = "dl" + CORE::ToString( (UInt64) job.createdAtMs ) + "-" + CORE::ToString( job.sequence );
        m_jobs[ job.jobId ] = entry;
    }
    jobId = job.jobId;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreDownloadManager: Queued job " << job.jobId << ": item " << itemId << " of store " << job.providerId );

    CAssetStoreDownloadJob snapshot;
    GetJob( jobId, snapshot );
    NotifyJobEvent( JobAddedEvent, snapshot );
    SaveQueueIfAutomatic();
    Schedule();
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreDownloadManager::Pause( const CORE::CString& jobId )
{GUCEF_TRACE;

    CAssetStoreDownloadJob snapshot;
    {
        MT::CScopeMutex lock( m_lock );
        TJobMap::iterator i = m_jobs.find( jobId );
        if ( i == m_jobs.end() )
            return ASSETSTORE_STATUS_NOT_FOUND;
        SJobEntry& entry = *(*i).second;
        if ( AssetDownloadJobStateIsFinal( entry.job.state ) )
            return ASSETSTORE_STATUS_INVALID_ARGUMENT;
        if ( entry.taskActive )
        {
            entry.pauseRequested = true;      // the task ends the job as paused
            return ASSETSTORE_STATUS_OK;
        }
        if ( ASSETDOWNLOAD_STATE_PAUSED == entry.job.state )
            return ASSETSTORE_STATUS_OK;
        entry.job.state = ASSETDOWNLOAD_STATE_PAUSED;
        entry.job.updatedAtMs = NowUtcMs();
        snapshot = entry.job;
    }
    NotifyJobEvent( JobStateChangedEvent, snapshot );
    SaveQueueIfAutomatic();
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreDownloadManager::Resume( const CORE::CString& jobId )
{GUCEF_TRACE;

    CAssetStoreDownloadJob snapshot;
    {
        MT::CScopeMutex lock( m_lock );
        TJobMap::iterator i = m_jobs.find( jobId );
        if ( i == m_jobs.end() )
            return ASSETSTORE_STATUS_NOT_FOUND;
        SJobEntry& entry = *(*i).second;
        if ( entry.taskActive )
        {
            entry.pauseRequested = false;
            return ASSETSTORE_STATUS_OK;
        }
        if ( ASSETDOWNLOAD_STATE_PAUSED != entry.job.state )
            return ASSETSTORE_STATUS_INVALID_ARGUMENT;
        entry.job.state = ASSETDOWNLOAD_STATE_QUEUED;
        entry.job.updatedAtMs = NowUtcMs();
        snapshot = entry.job;
    }
    NotifyJobEvent( JobStateChangedEvent, snapshot );
    SaveQueueIfAutomatic();
    Schedule();
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreDownloadManager::Cancel( const CORE::CString& jobId )
{GUCEF_TRACE;

    CAssetStoreDownloadJob snapshot;
    {
        MT::CScopeMutex lock( m_lock );
        TJobMap::iterator i = m_jobs.find( jobId );
        if ( i == m_jobs.end() )
            return ASSETSTORE_STATUS_NOT_FOUND;
        SJobEntry& entry = *(*i).second;
        if ( AssetDownloadJobStateIsFinal( entry.job.state ) )
            return ASSETSTORE_STATUS_INVALID_ARGUMENT;
        if ( entry.taskActive )
        {
            entry.cancelRequested = true;     // the task ends the job as cancelled
            return ASSETSTORE_STATUS_OK;
        }
        entry.job.state = ASSETDOWNLOAD_STATE_CANCELLED;
        entry.job.status = ASSETSTORE_STATUS_CANCELLED;
        entry.job.updatedAtMs = NowUtcMs();
        snapshot = entry.job;
    }
    RemovePartFiles( snapshot );
    NotifyJobEvent( JobStateChangedEvent, snapshot );
    SaveQueueIfAutomatic();
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreDownloadManager::Retry( const CORE::CString& jobId )
{GUCEF_TRACE;

    CAssetStoreDownloadJob snapshot;
    {
        MT::CScopeMutex lock( m_lock );
        TJobMap::iterator i = m_jobs.find( jobId );
        if ( i == m_jobs.end() )
            return ASSETSTORE_STATUS_NOT_FOUND;
        SJobEntry& entry = *(*i).second;
        if ( entry.taskActive || ( ASSETDOWNLOAD_STATE_FAILED != entry.job.state && ASSETDOWNLOAD_STATE_CANCELLED != entry.job.state ) )
            return ASSETSTORE_STATUS_INVALID_ARGUMENT;
        entry.job.state = ASSETDOWNLOAD_STATE_QUEUED;
        entry.job.status = ASSETSTORE_STATUS_OK;
        entry.job.message.Clear();
        entry.job.attempts = 0;
        entry.job.updatedAtMs = NowUtcMs();
        snapshot = entry.job;
    }
    NotifyJobEvent( JobStateChangedEvent, snapshot );
    SaveQueueIfAutomatic();
    Schedule();
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreDownloadManager::Remove( const CORE::CString& jobId )
{GUCEF_TRACE;

    CAssetStoreDownloadJob snapshot;
    {
        MT::CScopeMutex lock( m_lock );
        TJobMap::iterator i = m_jobs.find( jobId );
        if ( i == m_jobs.end() )
            return ASSETSTORE_STATUS_NOT_FOUND;
        SJobEntry* entry = (*i).second;
        if ( entry->taskActive || !AssetDownloadJobStateIsFinal( entry->job.state ) )
            return ASSETSTORE_STATUS_INVALID_ARGUMENT;
        snapshot = entry->job;
        m_jobs.erase( i );
        GUCEF_DELETE entry;
    }
    NotifyJobEvent( JobRemovedEvent, snapshot );
    SaveQueueIfAutomatic();
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreDownloadManager::GetJob( const CORE::CString& jobId, CAssetStoreDownloadJob& job ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TJobMap::const_iterator i = m_jobs.find( jobId );
    if ( i == m_jobs.end() )
        return false;
    job = (*i).second->job;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreDownloadManager::GetJobs( TAssetStoreDownloadJobVector& jobs ) const
{GUCEF_TRACE;

    jobs.clear();
    GUCEF::vector< std::pair< UInt64, CORE::CString > > order;
    MT::CScopeMutex lock( m_lock );
    TJobMap::const_iterator i = m_jobs.begin();
    while ( i != m_jobs.end() )
    {
        order.push_back( std::pair< UInt64, CORE::CString >( (*i).second->job.sequence, (*i).first ) );
        ++i;
    }
    std::sort( order.begin(), order.end() );
    for ( size_t n=0; n<order.size(); ++n )
        jobs.push_back( m_jobs.find( order[ n ].second )->second->job );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreDownloadManager::WaitForJob( const CORE::CString& jobId, UInt32 timeoutInMs ) const
{GUCEF_TRACE;

    const UInt64 resolution = MT::PrecisionTimerResolution();
    const UInt64 start = MT::PrecisionTickCount();
    for ( ;; )
    {
        {
            MT::CScopeMutex lock( m_lock );
            TJobMap::const_iterator i = m_jobs.find( jobId );
            if ( i == m_jobs.end() )
                return false;
            const SJobEntry& entry = *(*i).second;
            if ( !entry.taskActive && ( AssetDownloadJobStateIsFinal( entry.job.state ) || ASSETDOWNLOAD_STATE_PAUSED == entry.job.state ) )
                return true;
        }
        if ( resolution > 0 && ( MT::PrecisionTickCount() - start ) * 1000 / resolution >= timeoutInMs )
            return false;
        MT::ThreadDelay( 2 );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreDownloadManager::GetRunningJobCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return CountActiveTasks( CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreDownloadManager::GetPeakRunningJobCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_peakRunning;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreDownloadManager::GetPeakRunningJobCountOfProvider( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_peakRunningOfProvider;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreDownloadManager::ResetPeakRunningJobCounts( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_peakRunning = 0;
    m_peakRunningOfProvider = 0;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreDownloadManager::GetPartFilePath( const CAssetStoreDownloadJob& job, const CAssetStoreJobFile& file )
{GUCEF_TRACE;

    return job.GetFilePath( file ) + ".part";
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreDownloadManager::IsJobStopRequested( const CORE::CString& jobId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_stopping )
        return true;
    TJobMap::const_iterator i = m_jobs.find( jobId );
    if ( i == m_jobs.end() )
        return true;
    return (*i).second->cancelRequested || (*i).second->pauseRequested;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreDownloadManager::StopStatusOf( const CORE::CString& jobId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TJobMap::const_iterator i = m_jobs.find( jobId );
    if ( i == m_jobs.end() || (*i).second->cancelRequested )
        return ASSETSTORE_STATUS_CANCELLED;
    return ASSETSTORE_STATUS_PAUSED;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreDownloadManager::SleepUnlessStopped( const CORE::CString& jobId, UInt32 delayInMs ) const
{GUCEF_TRACE;

    UInt32 slept = 0;
    while ( slept < delayInMs )
    {
        if ( IsJobStopRequested( jobId ) )
            return false;
        const UInt32 slice = ( delayInMs - slept ) > 10 ? 10 : ( delayInMs - slept );
        MT::ThreadDelay( slice );
        slept += slice;
    }
    return !IsJobStopRequested( jobId );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreDownloadManager::GetBackOffDelayInMs( UInt32 attempt, UInt32 retryAfterInMs ) const
{GUCEF_TRACE;

    UInt32 baseDelay = 0;
    UInt32 maxDelay = 0;
    {
        MT::CScopeMutex lock( m_lock );
        baseDelay = m_settings.retryBaseDelayInMs;
        maxDelay = m_settings.retryMaxDelayInMs;
    }
    if ( retryAfterInMs > 0 )
        return retryAfterInMs > maxDelay && maxDelay > 0 ? maxDelay : retryAfterInMs;

    // exponential with jitter: a random delay between half of the step and the step
    UInt64 step = baseDelay;
    for ( UInt32 a=1; a<attempt && step < maxDelay; ++a )
        step *= 2;
    if ( maxDelay > 0 && step > maxDelay )
        step = maxDelay;
    const UInt32 half = (UInt32) ( step / 2 );
    return half + CORE::CRandom::GetRandomUInt32( 0, (UInt32) step - half );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreDownloadManager::OnTransferProgress( const CORE::CString& jobId, UInt32 fileIndex, UInt64 bytesInFile )
{GUCEF_TRACE;

    CAssetStoreDownloadJobEventData eventData;
    {
        MT::CScopeMutex lock( m_lock );
        TJobMap::iterator i = m_jobs.find( jobId );
        if ( i == m_jobs.end() )
            return;
        SJobEntry& entry = *(*i).second;
        if ( fileIndex >= entry.job.files.size() )
            return;
        entry.job.files[ fileIndex ].bytesDone = bytesInFile;

        const UInt64 resolution = MT::PrecisionTimerResolution();
        const UInt64 now = MT::PrecisionTickCount();
        const UInt64 bytesDone = entry.job.GetBytesDone();
        if ( 0 == resolution )
            return;
        if ( 0 == entry.lastProgressTicks )
        {
            entry.lastProgressTicks = now;
            entry.lastProgressBytes = bytesDone;
            return;
        }
        const UInt64 elapsedMs = ( now - entry.lastProgressTicks ) * 1000 / resolution;
        if ( elapsedMs < m_settings.progressIntervalInMs || 0 == elapsedMs )
            return;
        const UInt64 delta = bytesDone >= entry.lastProgressBytes ? bytesDone - entry.lastProgressBytes : 0;
        const UInt64 rate = delta * 1000 / elapsedMs;
        entry.bytesPerSecond = 0 == entry.bytesPerSecond ? rate : ( entry.bytesPerSecond + rate ) / 2;
        entry.lastProgressTicks = now;
        entry.lastProgressBytes = bytesDone;

        eventData.job = entry.job;
        eventData.bytesPerSecond = entry.bytesPerSecond;
        const UInt64 total = entry.job.GetTotalBytes();
        eventData.secondsLeft = ( entry.bytesPerSecond > 0 && total > bytesDone ) ? ( total - bytesDone ) / entry.bytesPerSecond : 0;
    }
    NotifyObserversFromThread( JobProgressEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreDownloadManager::NotifyJobEvent( const CORE::CEvent& eventId, const CAssetStoreDownloadJob& job )
{GUCEF_TRACE;

    CAssetStoreDownloadJobEventData eventData( job );
    NotifyObserversFromThread( eventId, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreDownloadManager::SetJobState( const CORE::CString& jobId, TAssetDownloadJobState state )
{GUCEF_TRACE;

    CAssetStoreDownloadJob snapshot;
    {
        MT::CScopeMutex lock( m_lock );
        TJobMap::iterator i = m_jobs.find( jobId );
        if ( i == m_jobs.end() || state == (*i).second->job.state )
            return;
        (*i).second->job.state = state;
        (*i).second->job.updatedAtMs = NowUtcMs();
        snapshot = (*i).second->job;
    }
    NotifyJobEvent( JobStateChangedEvent, snapshot );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreDownloadManager::EndJob( const CORE::CString& jobId, TAssetDownloadJobState state, TAssetStoreStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    CAssetStoreDownloadJob snapshot;
    {
        MT::CScopeMutex lock( m_lock );
        TJobMap::iterator i = m_jobs.find( jobId );
        if ( i == m_jobs.end() )
            return;
        SJobEntry& entry = *(*i).second;
        entry.job.state = state;
        entry.job.status = status;
        entry.job.message = message;
        entry.job.updatedAtMs = NowUtcMs();
        entry.cancelRequested = false;
        entry.pauseRequested = false;
        entry.lastProgressTicks = 0;
        entry.bytesPerSecond = 0;
        snapshot = entry.job;
    }
    if ( ASSETDOWNLOAD_STATE_CANCELLED == state )
        RemovePartFiles( snapshot );

    if ( ASSETDOWNLOAD_STATE_FAILED == state )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreDownloadManager: Job " << jobId << " failed (" << AssetStoreStatusToString( status ) << "): " << message );
    }
    else
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreDownloadManager: Job " << jobId << " is " << AssetDownloadJobStateToString( state ) );
    }
    SaveQueueIfAutomatic();
    NotifyJobEvent( JobStateChangedEvent, snapshot );
    if ( ASSETDOWNLOAD_STATE_DONE == state )
        NotifyJobEvent( JobFinishedEvent, snapshot );
    else
    if ( ASSETDOWNLOAD_STATE_FAILED == state )
        NotifyJobEvent( JobFailedEvent, snapshot );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreDownloadManager::RemovePartFiles( const CAssetStoreDownloadJob& job ) const
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    for ( size_t f=0; f<job.files.size(); ++f )
    {
        if ( job.files[ f ].done || !CAssetStoreSafeNames::IsSafeRelativeName( job.files[ f ].name ) )
            continue;
        const CORE::CString partPath = GetPartFilePath( job, job.files[ f ] );
        if ( vfs.FileExists( partPath ) )
            vfs.DeleteFile( partPath, true );
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreDownloadManager::OnJobException( const CORE::CString& jobId )
{GUCEF_TRACE;

    EndJob( jobId, ASSETDOWNLOAD_STATE_FAILED, ASSETSTORE_STATUS_FAILED, "an exception ended the job" );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreDownloadManager::OnJobTaskEnded( const CORE::CString& jobId )
{GUCEF_TRACE;

    CAssetStoreDownloadJob requeued;
    bool wasRequeued = false;
    {
        MT::CScopeMutex lock( m_lock );
        TJobMap::iterator i = m_jobs.find( jobId );
        if ( i != m_jobs.end() )
        {
            SJobEntry& entry = *(*i).second;
            entry.taskActive = false;

            // a task that ended without an end state (a pool that stopped) leaves its job queued
            if ( !AssetDownloadJobStateIsFinal( entry.job.state ) && ASSETDOWNLOAD_STATE_PAUSED != entry.job.state && ASSETDOWNLOAD_STATE_QUEUED != entry.job.state )
            {
                entry.job.state = ASSETDOWNLOAD_STATE_QUEUED;
                requeued = entry.job;
                wasRequeued = true;
            }
        }
        if ( m_activeTasks > 0 )
            --m_activeTasks;
    }
    if ( wasRequeued )
        NotifyJobEvent( JobStateChangedEvent, requeued );
    Schedule();
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreDownloadManager::PlanJob( SJobRun& run, CORE::CString& message )
{GUCEF_TRACE;

    UInt32 attempt = 0;
    for ( ;; )
    {
        if ( IsJobStopRequested( run.job.jobId ) )
            return StopStatusOf( run.job.jobId );

        CAssetStoreDownloadPlan plan;
        TAssetStoreStatus status = run.provider->PlanDownload( run.job.itemId, run.job.formatId, run.job.fileIds, plan );
        if ( ASSETSTORE_STATUS_OK == status )
        {
            run.plan = plan;
            return ASSETSTORE_STATUS_OK;
        }
        if ( ASSETSTORE_STATUS_SESSION_EXPIRED == status && !run.sessionRenewed )
        {
            run.sessionRenewed = true;
            if ( run.provider->RenewSession() )
                continue;
        }
        if ( IsRetryable( status ) )
        {
            ++attempt;
            UInt32 maxAttempts = 1;
            {
                MT::CScopeMutex lock( m_lock );
                maxAttempts = m_settings.maxAttempts;
            }
            if ( attempt < maxAttempts )
            {
                if ( !SleepUnlessStopped( run.job.jobId, GetBackOffDelayInMs( attempt, 0 ) ) )
                    return StopStatusOf( run.job.jobId );
                continue;
            }
        }
        message = "the store could not plan the download";
        if ( ASSETSTORE_STATUS_SESSION_EXPIRED == status || ASSETSTORE_STATUS_NOT_SIGNED_IN == status )
            message = "sign in again";
        const CORE::CString problem = run.provider->GetPlanProblem( run.job.itemId, run.job.formatId );
        if ( !problem.IsNULLOrEmpty() )
            message += ": " + problem;
        return status;
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreDownloadManager::RunJob( const CORE::CString& jobId )
{GUCEF_TRACE;

    SJobRun run;
    run.planRenewed = false;
    run.planRenewals = 0;
    run.renewedAtFile = 0;
    run.renewedAtBytes = 0;
    run.sessionRenewed = false;
    if ( !GetJob( jobId, run.job ) )
        return;

    run.provider = FindProvider( run.job.providerId );
    if ( run.provider.IsNULL() )
    {
        EndJob( jobId, ASSETDOWNLOAD_STATE_FAILED, ASSETSTORE_STATUS_NOT_FOUND, "the store " + run.job.providerId + " is not available" );
        return;
    }
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().IsWriteable( run.job.destination ) )
    {
        EndJob( jobId, ASSETDOWNLOAD_STATE_FAILED, ASSETSTORE_STATUS_UNAVAILABLE, "the destination " + run.job.destination + " is not writeable" );
        return;
    }

    // the plan
    CORE::CString message;
    TAssetStoreStatus status = PlanJob( run, message );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        if ( ASSETSTORE_STATUS_CANCELLED == status )
            EndJob( jobId, ASSETDOWNLOAD_STATE_CANCELLED, ASSETSTORE_STATUS_CANCELLED, CORE::CString::Empty );
        else
        if ( ASSETSTORE_STATUS_PAUSED == status )
            EndJob( jobId, IsStarted() ? ASSETDOWNLOAD_STATE_PAUSED : ASSETDOWNLOAD_STATE_QUEUED, ASSETSTORE_STATUS_OK, CORE::CString::Empty );
        else
            EndJob( jobId, ASSETDOWNLOAD_STATE_FAILED, status, message );
        return;
    }

    // every name is checked before anything is written; two names that differ only in case would be one file on a share that
    // ignores case (CIFS, Windows), so the second would overwrite the first: such a plan is refused with the names
    CORE::CString::StringMap lowercaseNames;
    for ( size_t t=0; t<run.plan.transfers.size(); ++t )
    {
        CORE::CString reason;
        const CORE::CString& name = run.plan.transfers[ t ].targetName;
        if ( !CAssetStoreSafeNames::IsSafeRelativeName( name, reason ) )
        {
            EndJob( jobId, ASSETDOWNLOAD_STATE_FAILED, ASSETSTORE_STATUS_UNSAFE_NAME, "the store gave the unsafe file name \"" + name + "\": " + reason );
            return;
        }
        const CORE::CString folded = CAssetStoreSafeNames::NormalizeSeparators( name ).Lowercase();
        CORE::CString::StringMap::const_iterator same = lowercaseNames.find( folded );
        if ( same != lowercaseNames.end() && (*same).second != name )
        {
            EndJob( jobId, ASSETDOWNLOAD_STATE_FAILED, ASSETSTORE_STATUS_UNSAFE_NAME, "the store gave the file names \"" + (*same).second + "\" and \"" + name +
                    "\", which differ only in case: a share that ignores case would hold one of them" );
            return;
        }
        lowercaseNames[ folded ] = name;
    }

    // the files of the job follow the plan; files that were complete stay complete
    {
        MT::CScopeMutex lock( m_lock );
        TJobMap::iterator i = m_jobs.find( jobId );
        if ( i == m_jobs.end() )
            return;
        CAssetStoreDownloadJob& job = (*i).second->job;
        TAssetStoreJobFileVector files;
        for ( size_t t=0; t<run.plan.transfers.size(); ++t )
        {
            const CAssetStoreTransfer& transfer = run.plan.transfers[ t ];
            CAssetStoreJobFile file;
            file.fileId = transfer.fileId;
            file.name = CAssetStoreSafeNames::NormalizeSeparators( transfer.targetName );
            file.size = transfer.size;
            file.digest = transfer.digest;
            file.digestAlgorithm = transfer.digestAlgorithm;
            for ( size_t f=0; f<job.files.size(); ++f )
            {
                if ( job.files[ f ].fileId == file.fileId && job.files[ f ].name == file.name && job.files[ f ].done )
                {
                    file.done = true;
                    file.bytesDone = job.files[ f ].bytesDone;
                }
            }
            files.push_back( file );
        }
        job.files = files;
        run.job = job;
    }
    SetJobState( jobId, ASSETDOWNLOAD_STATE_RUNNING );
    SaveQueueIfAutomatic();

    for ( UInt32 f=0; f<(UInt32) run.job.files.size(); ++f )
    {
        if ( run.job.files[ f ].done )
            continue;
        status = RunFile( run, f, message );
        if ( ASSETSTORE_STATUS_OK != status )
        {
            if ( ASSETSTORE_STATUS_CANCELLED == status )
                EndJob( jobId, ASSETDOWNLOAD_STATE_CANCELLED, ASSETSTORE_STATUS_CANCELLED, CORE::CString::Empty );
            else
            if ( ASSETSTORE_STATUS_PAUSED == status )
                EndJob( jobId, IsStarted() ? ASSETDOWNLOAD_STATE_PAUSED : ASSETDOWNLOAD_STATE_QUEUED, ASSETSTORE_STATUS_OK, CORE::CString::Empty );
            else
                EndJob( jobId, ASSETDOWNLOAD_STATE_FAILED, status, message );
            return;
        }
        run.job.files[ f ].done = true;
        SaveQueueIfAutomatic();
    }
    EndJob( jobId, ASSETDOWNLOAD_STATE_DONE, ASSETSTORE_STATUS_OK, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreDownloadManager::RunFile( SJobRun& run, UInt32 fileIndex, CORE::CString& message )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CAssetStoreJobFile& file = run.job.files[ fileIndex ];
    const CORE::CString partPath = GetPartFilePath( run.job, file );
    const CORE::CString parentDir = ParentOfVfsPath( partPath );
    if ( !parentDir.IsNULLOrEmpty() && !vfs.DirExists( parentDir ) )
        vfs.CreateDir( parentDir );

    CAssetStoreDownloadSettings settings = GetSettings();
    UInt32 attempt = 0;
    for ( ;; )
    {
        if ( IsJobStopRequested( run.job.jobId ) )
            return StopStatusOf( run.job.jobId );

        const CAssetStoreTransfer* transferPtr = run.plan.FindTransfer( file.fileId );
        if ( GUCEF_NULL == transferPtr )
        {
            message = "the renewed plan has no transfer for the file " + file.name;
            return ASSETSTORE_STATUS_NOT_FOUND;
        }
        const CAssetStoreTransfer transfer = *transferPtr;

        // an address that expired before it was used
        TAssetStoreStatus status = ASSETSTORE_STATUS_OK;
        if ( transfer.IsExpired( NowUtcMs() ) )
        {
            status = ASSETSTORE_STATUS_URL_EXPIRED;
        }
        else
        {
            UInt64 resumeOffset = 0;
            if ( vfs.FileExists( partPath ) )
            {
                resumeOffset = vfs.GetFileSize( partPath );
                if ( transfer.size > 0 && resumeOffset > transfer.size )
                {
                    vfs.DeleteFile( partPath, true );
                    resumeOffset = 0;
                }
            }
            CAssetStoreTransferContext context( *this, run.job.jobId, fileIndex, partPath, resumeOffset, settings.maxBytesPerSecond );
            OnTransferProgress( run.job.jobId, fileIndex, resumeOffset );

            if ( transfer.size > 0 && resumeOffset == transfer.size )
            {
                status = ASSETSTORE_STATUS_OK;        // complete already, only the checks are left
            }
            else
            {
                CIAssetStoreTransferSource* source = run.provider->GetTransferSource();
                if ( GUCEF_NULL != source )
                    status = source->RunTransfer( transfer, context );
                else
                    status = RunHttpTransfer( run, transfer, context, message );
            }

            if ( IsRetryable( status ) )
            {
                ++attempt;
                {
                    MT::CScopeMutex lock( m_lock );
                    TJobMap::iterator i = m_jobs.find( run.job.jobId );
                    if ( i != m_jobs.end() )
                        ++(*i).second->job.attempts;
                }
                if ( attempt >= settings.maxAttempts )
                {
                    message = "gave up after " + CORE::ToString( attempt ) + " attempts: " + message;
                    return status;
                }
                const UInt32 delay = GetBackOffDelayInMs( attempt, context.GetRetryAfterInMs() );
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreDownloadManager: Job " << run.job.jobId << " retries " << file.name << " in " << delay << " ms (" << AssetStoreStatusToString( status ) << ")" );
                if ( !SleepUnlessStopped( run.job.jobId, delay ) )
                    return StopStatusOf( run.job.jobId );
                continue;
            }
        }

        switch ( status )
        {
            case ASSETSTORE_STATUS_OK:
            {
                return VerifyFile( run, fileIndex, transfer, message );
            }
            case ASSETSTORE_STATUS_URL_EXPIRED:
            {
                // the addresses of a plan live for a while: a long download (the chunks of a large build) outlives them and renews
                // them again as often as it made progress since the last renewal; an address that expires again without progress fails
                const UInt64 bytesNow = vfs.FileExists( partPath ) ? vfs.GetFileSize( partPath ) : 0;
                if ( run.planRenewed && ( ( fileIndex == run.renewedAtFile && bytesNow <= run.renewedAtBytes ) || run.planRenewals >= MaxPlanRenewals ) )
                {
                    message = run.planRenewals >= MaxPlanRenewals ? "the download addresses expired " + CORE::ToString( run.planRenewals ) + " times"
                                                                  : CORE::CString( "the download address expired again after the plan was renewed, without progress since" );
                    return status;
                }
                run.planRenewed = true;
                ++run.planRenewals;
                run.renewedAtFile = fileIndex;
                run.renewedAtBytes = bytesNow;
                const TAssetStoreStatus renewStatus = run.provider->RenewPlan( run.plan );
                if ( ASSETSTORE_STATUS_OK != renewStatus )
                {
                    message = "the plan could not be renewed after the download address expired";
                    return renewStatus;
                }
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreDownloadManager: Job " << run.job.jobId << " renewed its plan after an address expired (renewal " << run.planRenewals << ", at " << file.name << ", " << bytesNow << " bytes of it)" );
                continue;
            }
            case ASSETSTORE_STATUS_SESSION_EXPIRED:
            {
                if ( !run.sessionRenewed )
                {
                    run.sessionRenewed = true;
                    if ( run.provider->RenewSession() )
                    {
                        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreDownloadManager: Job " << run.job.jobId << " renewed the session of store " << run.job.providerId );
                        continue;
                    }
                }
                message = "sign in again";
                return status;
            }
            case ASSETSTORE_STATUS_CANCELLED:
            {
                return StopStatusOf( run.job.jobId );
            }
            default:
            {
                if ( message.IsNULLOrEmpty() )
                    message = "the transfer of " + file.name + " failed";
                return status;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreDownloadManager::RunHttpTransfer( SJobRun& run, const CAssetStoreTransfer& transfer, CAssetStoreTransferContext& context, CORE::CString& message )
{GUCEF_TRACE;

    CAssetStoreDownloadSettings settings = GetSettings();

    WEB::CHttpRequestData request;
    if ( !request.SetUrl( transfer.url ) )
    {
        message = "the store gave an address that cannot be used";
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    }
    request.requestType = "GET";
    if ( !run.provider->AuthorizeTransferRequest( transfer, request ) )
    {
        message = "the store could not authorize the download";
        return ASSETSTORE_STATUS_NOT_SIGNED_IN;
    }

    WEB::CHttpResponseData response;
    WEB::CHttpClientTransferInfo info;
    CAssetStoreTransferSink* sink = GUCEF_NEW CAssetStoreTransferSink( context, response );
    CORE::IOAccessPtr sinkPtr( sink );

    WEB::CHttpClientRequestOptions options;
    options.responseBodySink = sinkPtr;
    options.connectTimeoutInMs = settings.connectTimeoutInMs;
    options.followRedirects = true;
    options.maxRedirects = 10;
    if ( context.GetResumeOffset() > 0 )
    {
        options.hasRange = true;
        options.rangeStart = context.GetResumeOffset();
        options.rangeLength = -1;
    }

    WEB::CHTTPClient client;
    client.Execute( request, options, response, &info );
    sink->Close();
    options.responseBodySink.Unlink();

    if ( sink->WasStopped() )
        return ASSETSTORE_STATUS_CANCELLED;
    if ( sink->HadWriteError() )
    {
        message = "the file " + context.GetPartFilePath() + " could not be written";
        return ASSETSTORE_STATUS_IO_ERROR;
    }
    if ( !info.success )
    {
        message = "no response: " + CORE::CString( WEB::HttpTransferErrorClassToString( info.errorClass ) );
        return ASSETSTORE_STATUS_NETWORK_ERROR;
    }

    const int code = response.statusCode;
    if ( 200 == code || 206 == code )
    {
        // a 200 or 206 with an empty body still leaves a file behind
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        if ( !vfs.FileExists( context.GetPartFilePath() ) )
        {
            VFS::TBasicVfsResourcePtr empty = vfs.GetFile( context.GetPartFilePath(), "wb", true );
            if ( empty.IsNULL() )
            {
                message = "the file " + context.GetPartFilePath() + " could not be written";
                return ASSETSTORE_STATUS_IO_ERROR;
            }
        }
        return ASSETSTORE_STATUS_OK;
    }
    if ( 416 == code )
    {
        // the range is not there: the part is not what the store has, it starts again
        VFS::CVfsGlobal::Instance()->GetVfs().DeleteFile( context.GetPartFilePath(), true );
        message = "the store refused to resume the download";
        return ASSETSTORE_STATUS_SERVER_ERROR;
    }
    if ( 401 == code )
        return ASSETSTORE_STATUS_SESSION_EXPIRED;
    if ( run.provider->IsExpiredUrlResponse( transfer, response ) )
        return ASSETSTORE_STATUS_URL_EXPIRED;
    if ( 429 == code )
    {
        context.SetRetryAfterInMs( ParseRetryAfterInMs( response ) );
        message = "the store asked to slow down (HTTP 429)";
        return ASSETSTORE_STATUS_RATE_LIMITED;
    }
    if ( code >= 500 )
    {
        context.SetRetryAfterInMs( ParseRetryAfterInMs( response ) );
        message = "the store answered HTTP " + CORE::ToString( code );
        return ASSETSTORE_STATUS_SERVER_ERROR;
    }
    message = "the store answered HTTP " + CORE::ToString( code );
    return 404 == code ? ASSETSTORE_STATUS_NOT_FOUND : ASSETSTORE_STATUS_HTTP_ERROR;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreDownloadManager::VerifyFile( SJobRun& run, UInt32 fileIndex, const CAssetStoreTransfer& transfer, CORE::CString& message )
{GUCEF_TRACE;

    SetJobState( run.job.jobId, ASSETDOWNLOAD_STATE_VERIFYING );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CAssetStoreJobFile& file = run.job.files[ fileIndex ];
    const CORE::CString partPath = GetPartFilePath( run.job, file );
    const CORE::CString finalPath = run.job.GetFilePath( file );

    CORE::CString algorithm = transfer.digestAlgorithm;
    const bool hasDigest = !transfer.digest.IsNULLOrEmpty() && CAssetDigest::IsSupportedAlgorithm( algorithm );
    if ( !hasDigest )
        algorithm = CAssetDigest::Sha256;
    CORE::CString digest;
    UInt64 size = 0;
    if ( !CAssetDigest::ComputeVfsFileDigest( partPath, algorithm, digest, &size ) )
    {
        message = "the downloaded file " + partPath + " cannot be read";
        return ASSETSTORE_STATUS_IO_ERROR;
    }
    if ( transfer.size > 0 && size != transfer.size )
    {
        vfs.DeleteFile( partPath, true );
        message = "the downloaded file " + file.name + " has " + CORE::ToString( size ) + " bytes instead of " + CORE::ToString( transfer.size );
        return ASSETSTORE_STATUS_SIZE_MISMATCH;
    }
    if ( hasDigest && !CAssetDigest::AreEqual( digest, transfer.digest ) )
    {
        vfs.DeleteFile( partPath, true );
        message = "the downloaded file " + file.name + " does not have the " + algorithm + " digest the store gave";
        return ASSETSTORE_STATUS_DIGEST_MISMATCH;
    }
    if ( vfs.FileExists( finalPath ) )
        vfs.DeleteFile( finalPath, true );
    if ( !vfs.MoveFile( partPath, finalPath, true ) )
    {
        message = "the downloaded file could not be renamed to " + finalPath;
        return ASSETSTORE_STATUS_IO_ERROR;
    }
    {
        MT::CScopeMutex lock( m_lock );
        TJobMap::iterator i = m_jobs.find( run.job.jobId );
        if ( i != m_jobs.end() && fileIndex < (*i).second->job.files.size() )
        {
            CAssetStoreJobFile& jobFile = (*i).second->job.files[ fileIndex ];
            jobFile.done = true;
            jobFile.bytesDone = size;
            if ( jobFile.digest.IsNULLOrEmpty() )
            {
                jobFile.digest = digest;
                jobFile.digestAlgorithm = algorithm;
            }
        }
    }
    SetJobState( run.job.jobId, ASSETDOWNLOAD_STATE_RUNNING );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreDownloadManager::SaveQueueIfAutomatic( void )
{GUCEF_TRACE;

    bool autoSave = false;
    {
        MT::CScopeMutex lock( m_lock );
        autoSave = m_settings.autoSaveQueue && !m_settings.queueFilePath.IsNULLOrEmpty();
    }
    if ( autoSave )
        SaveQueue();
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreDownloadManager::SaveQueue( void )
{GUCEF_TRACE;

    CORE::CString path;
    TAssetStoreDownloadJobVector jobs;
    {
        MT::CScopeMutex lock( m_lock );
        path = m_settings.queueFilePath;
    }
    if ( path.IsNULLOrEmpty() )
        return true;
    GetJobs( jobs );

    CORE::CDataNode root( "downloadQueue", GUCEF_DATATYPE_OBJECT );
    root.SetAttribute( "version", (UInt32) 1 );
    CORE::CDataNode* jobsNode = root.AddChild( "jobs", GUCEF_DATATYPE_ARRAY );
    CORE::CDataNodeSerializableSettings serializerSettings;
    for ( size_t j=0; j<jobs.size(); ++j )
    {
        CORE::CDataNode* jobNode = jobsNode->AddChild( "job", GUCEF_DATATYPE_OBJECT );
        jobs[ j ].Serialize( *jobNode, serializerSettings );
    }

    MT::CScopeMutex saveLock( m_saveLock );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString parentDir = ParentOfVfsPath( path );
    if ( !parentDir.IsNULLOrEmpty() && !vfs.DirExists( parentDir ) )
        vfs.CreateDir( parentDir );
    if ( !vfs.StoreAsFile( path, root, "json", true ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreDownloadManager: The queue could not be saved to " << path );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreDownloadManager::LoadQueue( void )
{GUCEF_TRACE;

    CORE::CString path;
    {
        MT::CScopeMutex lock( m_lock );
        path = m_settings.queueFilePath;
    }
    if ( path.IsNULLOrEmpty() )
        return true;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.FileExists( path ) )
        return true;

    CORE::CDataNode root;
    if ( !vfs.LoadFile( root, path, "json" ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreDownloadManager: The queue file " << path << " cannot be read" );
        return false;
    }
    const CORE::CDataNode* jobsNode = root.Find( "jobs" );
    if ( GUCEF_NULL == jobsNode )
        return true;

    CORE::CDataNodeSerializableSettings serializerSettings;
    UInt32 added = 0;
    CORE::CDataNode::const_iterator i = jobsNode->ConstBegin();
    while ( i != jobsNode->ConstEnd() )
    {
        CAssetStoreDownloadJob job;
        if ( job.Deserialize( *(*i), serializerSettings ) )
        {
            if ( ASSETDOWNLOAD_STATE_PLANNING == job.state || ASSETDOWNLOAD_STATE_RUNNING == job.state || ASSETDOWNLOAD_STATE_VERIFYING == job.state )
                job.state = ASSETDOWNLOAD_STATE_QUEUED;
            MT::CScopeMutex lock( m_lock );
            if ( m_jobs.end() == m_jobs.find( job.jobId ) )
            {
                SJobEntry* entry = GUCEF_NEW SJobEntry();
                entry->job = job;
                entry->taskActive = false;
                entry->cancelRequested = false;
                entry->pauseRequested = false;
                entry->lastProgressTicks = 0;
                entry->lastProgressBytes = 0;
                entry->bytesPerSecond = 0;
                m_jobs[ job.jobId ] = entry;
                if ( job.sequence >= m_nextSequence )
                    m_nextSequence = job.sequence + 1;
                ++added;
            }
        }
        ++i;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreDownloadManager: Took " << added << " job(s) back from the queue file " << path );
    Schedule();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreDownloadManager::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    CORE::CDataNode* node = config.FindOrAddChild( "downloads" );
    if ( GUCEF_NULL == node )
        return false;
    CORE::CDataNodeSerializableSettings serializerSettings;
    return GetSettings().Serialize( *node, serializerSettings );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreDownloadManager::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    const CORE::CDataNode* node = config.Find( "downloads" );
    if ( GUCEF_NULL == node )
        node = &config;
    CAssetStoreDownloadSettings settings;
    CORE::CDataNodeSerializableSettings serializerSettings;
    if ( !settings.Deserialize( *node, serializerSettings ) )
        return false;
    SetSettings( settings );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreDownloadManager::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStoreDownloadManager";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
