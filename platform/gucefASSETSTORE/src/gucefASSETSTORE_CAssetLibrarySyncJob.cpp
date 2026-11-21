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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */
#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */
#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */
#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#include "gucefASSETSTORE_CAssetStoreDownloadManager.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOBEVENTDATA_H
#include "gucefASSETSTORE_CAssetStoreDownloadJobEventData.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOBEVENTDATA_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOBEVENTDATA_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETINGESTEVENTDATA_H
#include "gucefASSETSTORE_CAssetIngestEventData.h"
#define GUCEF_ASSETSTORE_CASSETINGESTEVENTDATA_H
#endif /* GUCEF_ASSETSTORE_CASSETINGESTEVENTDATA_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETDOWNLOADREQUEST_H
#include "gucefASSETSTORE_CAssetDownloadRequest.h"
#define GUCEF_ASSETSTORE_CASSETDOWNLOADREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETDOWNLOADREQUEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#include "gucefASSETSTORE_CAssetPipelineRequest.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSERVICERESULT_H
#include "gucefASSETSTORE_CAssetServiceResult.h"
#define GUCEF_ASSETSTORE_CASSETSERVICERESULT_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICERESULT_H ? */
#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#include "gucefASSETSTORE_CPrimaryAssetStore.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H ? */
#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#include "gucefASSETSTORE_CLocalAssetStoreService.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H ? */

#include "gucefASSETSTORE_CAssetLibrarySyncJob.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Brings a call of another thread to the thread of the pulse generator of the job: Wake() mails an event there
 */
class CAssetLibrarySyncWakeup : public CORE::CTSGNotifier
{
    public:

    static const CORE::CEvent WakeEvent;

    explicit CAssetLibrarySyncWakeup( CORE::PulseGeneratorPtr pulseGenerator )
        : CORE::CTSGNotifier( pulseGenerator )
    {GUCEF_TRACE;

        WakeEvent.Initialize();
    }

    virtual ~CAssetLibrarySyncWakeup() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        SignalUpcomingDestruction();
    }

    void Wake( void )
    {GUCEF_TRACE;

        NotifyObserversFromThread( WakeEvent );
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetLibrarySyncWakeup";
        return classTypeName;
    }
};

const CORE::CEvent CAssetLibrarySyncWakeup::WakeEvent = "GUCEF::ASSETSTORE::CAssetLibrarySyncWakeup::WakeEvent";

/*-------------------------------------------------------------------------*/

namespace {

const UInt32 PollIntervalMs = 20;
const UInt32 AdmissionIntervalMs = 1000;
const UInt32 MaxPageRetries = 3;
const UInt32 MaxKeptFailures = 256;
const UInt32 SignInCheckIntervalMs = 30000;

Int64
NowMs( void )
{
    return (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

}

/*-------------------------------------------------------------------------*/

const CORE::CEvent CAssetLibrarySyncJob::SyncJobChangedEvent = "GUCEF::ASSETSTORE::CAssetLibrarySyncJob::SyncJobChangedEvent";

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::RegisterEvents( void )
{GUCEF_TRACE;

    SyncJobChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncJob::SInFlight::SInFlight( void )
    : work()
    , downloaded( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncJob::SRunCounts::SRunCounts( void )
    : queued( 0 )
    , downloaded( 0 )
    , ingested( 0 )
    , revalidated( 0 )
    , failed( 0 )
    , cancelled( 0 )
    , skippedInFlight( 0 )
    , peakInFlight( 0 )
    , doneBytes( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncJob::CAssetLibrarySyncJob( CLocalAssetStoreService& service, const CORE::CString& jobId, const CAssetLibrarySyncSpec& spec )
    : CORE::CTSGNotifier( service.GetPulseGenerator() )
    , m_service( service )
    , m_jobId( jobId )
    , m_spec( spec )
    , m_provider()
    , m_store( GUCEF_NULL )
    , m_manager( GUCEF_NULL )
    , m_wakeup( GUCEF_NULL )
    , m_timer( GUCEF_NULL )
    , m_cancelRequested( 0 )
    , m_pauseRequest( 0 )
    , m_paused( false )
    , m_subscribed( false )
    , m_restoredRun( false )
    , m_lastCheckpoint()
    , m_lastQueuedTitle()
    , m_phase( PHASE_CREATED )
    , m_wait( WAIT_NONE )
    , m_future( CORE::TASKSTATUS_UNDEFINED )
    , m_cursor()
    , m_pageRetries( 0 )
    , m_retryAtMs( 0 )
    , m_enrichment()
    , m_nextEnrichment( 0 )
    , m_planner()
    , m_admission()
    , m_worklist()
    , m_next( 0 )
    , m_inFlight()
    , m_dataLock()
    , m_state( ASSETJOB_STATE_QUEUED )
    , m_status( ASSETSTORE_STATUS_OK )
    , m_stateText( "starting" )
    , m_message()
    , m_indexJobId()
    , m_plan()
    , m_counts()
    , m_failures()
    , m_storageFreeBytes( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    m_wakeup = GUCEF_NEW CAssetLibrarySyncWakeup( GetPulseGenerator() );
    TEventCallback wakeCallback( this, &CAssetLibrarySyncJob::OnWake );
    SubscribeTo( m_wakeup, CAssetLibrarySyncWakeup::WakeEvent, wakeCallback );
    m_timer = GUCEF_NEW CORE::CTimer( GetPulseGenerator(), PollIntervalMs );
    TEventCallback timerCallback( this, &CAssetLibrarySyncJob::OnTimer );
    SubscribeTo( m_timer, CORE::CTimer::TimerUpdateEvent, timerCallback );
}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncJob::~CAssetLibrarySyncJob()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    UnsubscribeAllFromObserver();
    if ( m_future.HasAFuture() && !m_future.IsReady() )
        m_future.RequestCancellation();
    GUCEF_DELETE m_timer;
    m_timer = GUCEF_NULL;
    GUCEF_DELETE m_wakeup;
    m_wakeup = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetLibrarySyncJob::GetJobId( void ) const
{GUCEF_TRACE;

    return m_jobId;
}

/*-------------------------------------------------------------------------*/

const CAssetLibrarySyncSpec&
CAssetLibrarySyncJob::GetSpec( void ) const
{GUCEF_TRACE;

    return m_spec;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetLibrarySyncJob::Start( CORE::CString& problem )
{GUCEF_TRACE;

    problem.Clear();
    if ( !m_spec.IsValid( problem ) )
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    m_store = m_service.GetPrimaryStore();
    m_manager = m_service.GetDownloadManager();
    if ( GUCEF_NULL == m_store || GUCEF_NULL == m_manager || !m_service.IsCatalogueAvailable() )
    {
        problem = "The library sync needs the primary store and the download manager";
        return ASSETSTORE_STATUS_UNAVAILABLE;
    }
    if ( !m_service.GetStoreProvider( m_spec.storeId, m_provider ) )
    {
        problem = "There is no store " + m_spec.storeId;
        return ASSETSTORE_STATUS_NOT_FOUND;
    }
    {
        MT::CScopeMutex lock( m_dataLock );
        m_state = ASSETJOB_STATE_RUNNING;
        m_stateText = "starting";
    }
    // kept from the start: a restart before the first page still finds the job
    Checkpoint( true );
    m_wakeup->Wake();
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::RequestCancel( void )
{GUCEF_TRACE;

    m_cancelRequested.Store( 1 );
    {
        MT::CScopeMutex lock( m_dataLock );
        if ( !AssetJobStateIsFinal( m_state ) )
            m_stateText = "cancelling";
    }
    if ( GUCEF_NULL != m_wakeup )
        m_wakeup->Wake();
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::RequestPause( void )
{GUCEF_TRACE;

    m_pauseRequest.Store( 1 );
    if ( GUCEF_NULL != m_wakeup )
        m_wakeup->Wake();
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::RequestResume( void )
{GUCEF_TRACE;

    m_pauseRequest.Store( 2 );
    if ( GUCEF_NULL != m_wakeup )
        m_wakeup->Wake();
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::ApplyPauseRequest( void )
{GUCEF_TRACE;

    const UInt32 request = m_pauseRequest.Exchange( 0 );
    if ( 1 == request && !m_paused )
    {
        m_paused = true;
        {
            MT::CScopeMutex lock( m_dataLock );
            m_state = ASSETJOB_STATE_PAUSED;
            m_stateText = "paused: " + DescribeRun();
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncJob: " << m_jobId << " is paused: nothing more is queued until it is resumed" );
        Checkpoint( true );
        Changed();
    }
    else
    if ( 2 == request && m_paused )
    {
        m_paused = false;
        {
            MT::CScopeMutex lock( m_dataLock );
            m_state = ASSETJOB_STATE_RUNNING;
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncJob: " << m_jobId << " is resumed" );
        Checkpoint( true );
        if ( PHASE_RUNNING == m_phase )
            Fill();
        else
            Changed();
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::OnWake( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( PHASE_ENDED == m_phase )
        return;
    if ( 0 != m_cancelRequested.Load() )
    {
        Cancel();
        return;
    }
    if ( PHASE_CREATED == m_phase )
        Begin();
    ApplyPauseRequest();
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::Begin( void )
{GUCEF_TRACE;

    if ( m_restoredRun )
    {
        // a run that a restart interrupted, with its work list (Restore() configured the admission): the downloads it had queued are
        // looked at again, then it goes on queueing
        Subscribe();
        m_phase = PHASE_RUNNING;
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncJob: " << m_jobId << " goes on with the sync of store " << m_spec.storeId << " after a restart: "
                          << ( m_worklist.size() - m_next ) << " work item(s) to queue, " << m_inFlight.size() << " download(s) of the run to follow" );
        ReconcileInFlight();
        AdoptQueuedDownloads();
        Fill();
        return;
    }

    m_planner.Reset( m_spec );
    m_planner.SetVocabulary( &m_store->GetVocabulary() );
    ConfigureAdmission();
    Subscribe();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncJob: " << m_jobId << " starts the " << ( m_spec.IsPlanOnly() ? "plan" : "sync" ) << " of the library of store " << m_spec.storeId );
    m_phase = PHASE_LISTING;
    m_cursor.Clear();
    m_pageRetries = 0;
    Checkpoint( true );
    RequestPage();
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::ConfigureAdmission( void )
{GUCEF_TRACE;

    m_admission.Configure( m_spec.profile, m_manager->GetSettings().maxConcurrentJobsPerProvider );
    m_admission.SetManual( m_spec.manual );
    // the space that counts is that of the volume of the library location the packs end up on, never that of a scratch staging area
    m_admission.SetVolumePath( ResolveVolumePath() );
    // the caps count what earlier runs of the store queued too: the ledger of the store is the service's
    m_admission.SetLedger( m_service.GetSyncLedger( m_spec.storeId ) );
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::Subscribe( void )
{GUCEF_TRACE;

    if ( m_subscribed )
        return;
    m_subscribed = true;
    TEventCallback downloadCallback( this, &CAssetLibrarySyncJob::OnDownloadJobEvent );
    SubscribeTo( m_manager, CAssetStoreDownloadManager::JobFinishedEvent, downloadCallback );
    SubscribeTo( m_manager, CAssetStoreDownloadManager::JobFailedEvent, downloadCallback );
    SubscribeTo( m_manager, CAssetStoreDownloadManager::JobStateChangedEvent, downloadCallback );
    SubscribeTo( m_manager, CAssetStoreDownloadManager::JobRemovedEvent, downloadCallback );
    TEventCallback ingestCallback( this, &CAssetLibrarySyncJob::OnIngestEvent );
    SubscribeTo( m_store, CPrimaryAssetStore::IngestFinishedEvent, ingestCallback );
    SubscribeTo( m_store, CPrimaryAssetStore::IngestFailedEvent, ingestCallback );
    TEventCallback storeCallback( this, &CAssetLibrarySyncJob::OnStoreStateChanged );
    SubscribeTo( &m_service, CIAssetStoreService::StoreStateChangedEvent, storeCallback );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetLibrarySyncJob::ResolveVolumePath( void ) const
{GUCEF_TRACE;

    // the location the downloads of the store go to: that of its policy, else the default location
    const CPrimaryAssetStoreSettings settings = m_store->GetSettings();
    CORE::CString locationId = settings.GetPolicy( m_spec.storeId ).locationId;
    if ( locationId.IsNULLOrEmpty() )
        locationId = settings.defaultLocationId;
    for ( size_t l=0; l<settings.locations.size(); ++l )
        if ( settings.locations[ l ].locationId == locationId && !settings.locations[ l ].fileSystemPath.IsNULLOrEmpty() )
            return CORE::RelativePath( settings.locations[ l ].fileSystemPath );

    // a location of a VFS root: the folder of a small file written there tells where it is on the file system
    const CORE::CString root = m_store->GetLocationRoot( locationId );
    if ( root.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString probe = root + "/staging/.librarysync-volume";
    if ( !vfs.FileExists( probe ) )
    {
        CORE::CDynamicBuffer content;
        content.Append( "volume probe", 12 );
        vfs.StoreAsFile( probe, content, 0, true );
    }
    CORE::CString actual;
    if ( !vfs.GetActualFilePath( probe, actual ) )
        return CORE::CString::Empty;
    return CORE::StripFilename( actual );
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::SetWait( EWait wait, UInt32 intervalMs )
{GUCEF_TRACE;

    m_wait = wait;
    if ( GUCEF_NULL == m_timer )
        return;
    if ( WAIT_NONE == wait )
    {
        m_timer->SetEnabled( false );
        return;
    }
    m_timer->SetInterval( intervalMs );
    m_timer->SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::SetStateText( const CORE::CString& text )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_stateText = text;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::Changed( void )
{GUCEF_TRACE;

    CORE::TCloneableString jobId( m_jobId );
    NotifyObservers( SyncJobChangedEvent, &jobId );
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::RequestPage( void )
{GUCEF_TRACE;

    CAssetStoreQuery query;
    query.cursor = m_cursor;
    query.pageSize = m_spec.pageSize;
    m_future = m_provider->QueryLibraryAsync( query );
    SetStateText( "listing: page " + CORE::ToString( m_planner.GetPageCount() + 1 ) + ", " + CORE::ToString( m_planner.GetListedCount() ) + " item(s) so far" );
    SetWait( WAIT_PAGE, PollIntervalMs );
    Changed();
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::OnTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( PHASE_ENDED == m_phase )
    {
        SetWait( WAIT_NONE, 0 );
        return;
    }
    if ( 0 != m_cancelRequested.Load() )
    {
        Cancel();
        return;
    }
    switch ( m_wait )
    {
        case WAIT_PAGE:
        {
            if ( m_future.IsReady() || m_future.HasNoFuture() )
                OnPageReady();
            break;
        }
        case WAIT_PAGE_RETRY:
        {
            if ( NowMs() >= m_retryAtMs )
                RequestPage();
            break;
        }
        case WAIT_ITEM:
        {
            if ( m_future.IsReady() || m_future.HasNoFuture() )
            {
                CAssetStoreItem item;
                if ( CIAssetStoreService::TryGetFutureResult( m_future, item ) )
                    m_planner.ApplyEnrichment( item );
                else
                {
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncJob: " << m_jobId << " could not get the details of item " << m_enrichment[ m_nextEnrichment - 1 ] << ": " << AssetStoreStatusToString( CIAssetStoreProvider::GetStatusOfFuture( m_future ) ) );
                }
                RequestNextEnrichment();
            }
            break;
        }
        case WAIT_ADMISSION:
        {
            Fill();
            break;
        }
        case WAIT_SIGN_IN:
        {
            // the event of the store says when it is signed in; this slow look is for a store that does not tell
            if ( !m_provider.IsNULL() && m_provider->IsSignedIn() )
            {
                {
                    MT::CScopeMutex lock( m_dataLock );
                    m_status = ASSETSTORE_STATUS_OK;
                }
                RequestPage();
            }
            break;
        }
        default:
        {
            SetWait( WAIT_NONE, 0 );
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::OnPageReady( void )
{GUCEF_TRACE;

    CAssetStoreLibraryPage page;
    if ( CIAssetStoreService::TryGetFutureResult( m_future, page ) )
    {
        m_pageRetries = 0;
        m_planner.AddListingPage( page );
        if ( page.nextCursor.IsNULLOrEmpty() || page.nextCursor == m_cursor )
        {
            m_planner.SetListingComplete( !page.nextCursor.IsNULLOrEmpty() ? false : true );
            BeginPlanning();
            return;
        }
        m_cursor = page.nextCursor;
        RequestPage();
        return;
    }

    // a page that failed is asked again after a wait; after the retries the listing is partial
    const TAssetStoreStatus status = CIAssetStoreProvider::GetStatusOfFuture( m_future );
    if ( ASSETSTORE_STATUS_NOT_SIGNED_IN == status || ASSETSTORE_STATUS_SESSION_EXPIRED == status )
    {
        // the run waits for the sign in of the store (a person's step) and takes the listing up again at the page it was at
        {
            MT::CScopeMutex lock( m_dataLock );
            m_status = ASSETSTORE_STATUS_OWNER_ACTION_NEEDED;
            m_stateText = "waiting for sign in";
            m_message = "The store " + m_spec.storeId + " needs a sign in before its library can be synced; the sync goes on after the sign in";
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncJob: " << m_jobId << " waits for the sign in of store " << m_spec.storeId );
        SetWait( WAIT_SIGN_IN, SignInCheckIntervalMs );
        Checkpoint( true );
        Changed();
        return;
    }
    if ( m_pageRetries < MaxPageRetries )
    {
        ++m_pageRetries;
        m_retryAtMs = NowMs() + ( 1000 << m_pageRetries );
        SetStateText( "listing: page " + CORE::ToString( m_planner.GetPageCount() + 1 ) + " failed (" + CORE::CString( AssetStoreStatusToString( status ) ) + "), asked again" );
        SetWait( WAIT_PAGE_RETRY, 100 );
        Changed();
        return;
    }
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncJob: " << m_jobId << " could not read page " << ( m_planner.GetPageCount() + 1 ) << " of the library of store " << m_spec.storeId << ": " << AssetStoreStatusToString( status ) << "; the listing is partial" );
    if ( 0 == m_planner.GetListedCount() )
    {
        End( ASSETJOB_STATE_FAILED, status, "The library of store " + m_spec.storeId + " could not be listed: " + CIAssetStoreService::GetMessageOfFuture( m_future ) );
        return;
    }
    m_planner.SetListingComplete( false );
    BeginPlanning();
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::BeginPlanning( void )
{GUCEF_TRACE;

    SetWait( WAIT_NONE, 0 );
    const TAssetStoreStatus status = m_planner.LoadStoreState( *m_store );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        End( ASSETJOB_STATE_FAILED, status, "The packs of store " + m_spec.storeId + " could not be read from the catalogue" );
        return;
    }
    TAssetStoreDownloadJobVector jobs;
    m_manager->GetJobs( jobs );
    m_planner.SetDownloadJobs( jobs );
    m_planner.GetEnrichmentCandidates( m_enrichment );
    m_nextEnrichment = 0;
    if ( m_enrichment.empty() )
    {
        BuildPlan();
        return;
    }
    m_phase = PHASE_ENRICHING;
    RequestNextEnrichment();
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::RequestNextEnrichment( void )
{GUCEF_TRACE;

    if ( m_nextEnrichment >= m_enrichment.size() )
    {
        SetWait( WAIT_NONE, 0 );
        BuildPlan();
        return;
    }
    m_future = m_provider->GetItemAsync( m_enrichment[ m_nextEnrichment ] );
    ++m_nextEnrichment;
    SetStateText( "details of the items: " + CORE::ToString( (UInt32) m_nextEnrichment ) + " of " + CORE::ToString( (UInt32) m_enrichment.size() ) );
    SetWait( WAIT_ITEM, PollIntervalMs );
    Changed();
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::BuildPlan( void )
{GUCEF_TRACE;

    const Int64 now = NowMs();
    CAssetLibrarySyncPlan plan;
    m_planner.BuildPlan( now, plan, m_worklist );
    m_next = 0;

    // the storage: what the volume of the location has against what the plan needs
    UInt64 freeBytes = 0;
    UInt64 totalBytes = 0;
    if ( CAssetLibrarySyncAdmission::ReadVolume( m_admission.GetVolumePath(), freeBytes, totalBytes ) )
    {
        plan.storageFreeBytes = freeBytes;
        plan.storageTotalBytes = totalBytes;
        plan.storageReserveBytes = m_admission.GetReserveBytes( totalBytes );
        plan.storageVerdict = CAssetLibrarySyncAdmission::FitsOnDisk( freeBytes, 0, plan.estimatedBytes, m_spec.profile.diskFactor, plan.storageReserveBytes ) ? "fits" : "does-not-fit";
    }
    const TAssetStoreStatus observed = m_planner.WriteObservations( m_store->GetSyncStore(), now, !m_spec.IsPlanOnly() );
    if ( ASSETSTORE_STATUS_OK != observed && ASSETSTORE_STATUS_UNSUPPORTED != observed )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncJob: " << m_jobId << " could not write the observations of the items: " << AssetStoreStatusToString( observed ) );
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncJob: " << m_jobId << " planned store " << m_spec.storeId << ": " << plan.Describe() << "; " << plan.queuedDownloads << " download(s) of about " << plan.estimatedBytes << " bytes" );
    {
        MT::CScopeMutex lock( m_dataLock );
        m_plan = plan;
        m_storageFreeBytes = freeBytes;
    }
    if ( m_spec.IsPlanOnly() )
    {
        End( ASSETJOB_STATE_DONE, ASSETSTORE_STATUS_OK, CORE::CString::Empty );
        return;
    }
    m_phase = PHASE_RUNNING;
    if ( !WriteWorklist() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AssetLibrarySyncJob: " << m_jobId << " keeps no work list file: a restart plans again" );
    }
    // the downloads a run of this job queued before a restart are followed, not queued again
    AdoptQueuedDownloads();
    Checkpoint( true );
    Fill();
}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncAdmission::SLoad
CAssetLibrarySyncJob::GetLoad( void ) const
{GUCEF_TRACE;

    CAssetLibrarySyncAdmission::SLoad load;
    for ( TInFlightMap::const_iterator i = m_inFlight.begin(); i != m_inFlight.end(); ++i )
    {
        ++load.inFlightItems;
        load.inFlightBytes += (*i).second.work.estimatedBytes;
        if ( (*i).second.downloaded )
        {
            ++load.pendingIngests;
            load.stagedBytes += (*i).second.work.estimatedBytes;
        }
    }
    return load;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::Fill( void )
{GUCEF_TRACE;

    if ( PHASE_RUNNING != m_phase )
        return;
    if ( WAIT_ADMISSION == m_wait )
        SetWait( WAIT_NONE, 0 );
    // the decision is looked at again at the dispatch: a download of the item in that format that is in flight now (queued by a
    // person or another process meanwhile) is not queued twice
    CORE::CString::StringSet othersInFlight;
    {
        TAssetStoreDownloadJobVector jobs;
        m_manager->GetJobs( jobs );
        for ( size_t j=0; j<jobs.size(); ++j )
        {
            const CAssetStoreDownloadJob& job = jobs[ j ];
            if ( job.forIngest && job.providerId == m_spec.storeId && ASSETDOWNLOAD_STATE_DONE != job.state && ASSETDOWNLOAD_STATE_FAILED != job.state && ASSETDOWNLOAD_STATE_CANCELLED != job.state )
                othersInFlight.insert( job.itemId + "\n" + job.formatId );
        }
    }
    CORE::CString::StringSet ownInFlight;
    for ( TInFlightMap::const_iterator i = m_inFlight.begin(); i != m_inFlight.end(); ++i )
        ownInFlight.insert( (*i).second.work.itemId + "\n" + (*i).second.work.formatId );
    CORE::CString waitReason;
    while ( !m_paused && m_next < m_worklist.size() && 0 == m_cancelRequested.Load() )
    {
        const CAssetLibrarySyncPlanner::SWorkItem& work = m_worklist[ m_next ];
        if ( ownInFlight.end() != ownInFlight.find( work.itemId + "\n" + work.formatId ) )
        {
            // queued by this run before a restart and followed already
            ++m_next;
            continue;
        }
        if ( othersInFlight.end() != othersInFlight.find( work.itemId + "\n" + work.formatId ) )
        {
            MT::CScopeMutex lock( m_dataLock );
            ++m_counts.skippedInFlight;
            ++m_next;
            continue;
        }

        CORE::CString reason;
        bool waiting = false;
        bool refused = false;
        if ( !m_admission.Admit( work.estimatedBytes, GetLoad(), NowMs(), reason, waiting, refused ) )
        {
            if ( refused )
            {
                // a manual download of an item that does not fit: it fails with the reason, the others go on
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncJob: " << m_jobId << " leaves out " << work.itemId << " (" << work.formatId << "): " << reason );
                ++m_next;
                RecordAttempt( work, ASSETSTORE_STATUS_IO_ERROR, reason, false );
                continue;
            }
            if ( waiting )
            {
                waitReason = reason;
                SetWait( WAIT_ADMISSION, AdmissionIntervalMs );
            }
            break;
        }

        CAssetDownloadRequest request;
        request.providerId = m_spec.storeId;
        request.itemId = work.itemId;
        request.itemTitle = work.title;
        request.formatId = work.formatId;
        request.forIngest = true;
        request.expectedRevision = work.expectedRevision;
        request.expectedBytes = work.estimatedBytes;
        request.syncJobId = m_jobId;
        CAssetServiceResult queued;
        const TAssetStoreStatus status = m_service.EnqueueDownload( CORE::CString::Empty, request, queued );
        ++m_next;
        if ( ASSETSTORE_STATUS_OK != status )
        {
            RecordAttempt( work, status, queued.message, false );
            continue;
        }
        SInFlight entry;
        entry.work = work;
        m_inFlight[ queued.id ] = entry;
        m_lastQueuedTitle = work.title.IsNULLOrEmpty() ? work.itemId : work.title;
        m_admission.NoteQueued( NowMs(), work.estimatedBytes );
        MT::CScopeMutex lock( m_dataLock );
        ++m_counts.queued;
        if ( m_inFlight.size() > m_counts.peakInFlight )
            m_counts.peakInFlight = (UInt32) m_inFlight.size();
    }

    if ( m_next >= m_worklist.size() && m_inFlight.empty() )
    {
        Finish();
        return;
    }
    if ( m_paused )
        SetStateText( "paused: " + DescribeRun() );
    else
        SetStateText( waitReason.IsNULLOrEmpty() ? DescribeRun() : waitReason );
    Checkpoint( false );
    Changed();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetLibrarySyncJob::DescribeRun( void ) const
{GUCEF_TRACE;

    CORE::CString text = CORE::ToString( (UInt32) m_next ) + " of " + CORE::ToString( (UInt32) m_worklist.size() ) + " queued, " + CORE::ToString( (UInt32) m_inFlight.size() ) + " in flight";
    if ( !m_inFlight.empty() && !m_lastQueuedTitle.IsNULLOrEmpty() )
        text += "; last queued: " + m_lastQueuedTitle;
    return text;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::RecordAttempt( const CAssetLibrarySyncPlanner::SWorkItem& work, TAssetStoreStatus status, const CORE::CString& message, bool succeeded )
{GUCEF_TRACE;

    CAssetSyncMarkStore& syncStore = m_store->GetSyncStore();
    if ( !succeeded )
    {
        MT::CScopeMutex lock( m_dataLock );
        ++m_counts.failed;
        if ( m_failures.size() < MaxKeptFailures )
        {
            SFailure failure;
            failure.itemId = work.itemId;
            failure.formatId = work.formatId;
            failure.status = AssetStoreStatusToString( status );
            failure.message = message;
            m_failures.push_back( failure );
        }
    }
    if ( !syncStore.IsAvailable() )
        return;
    CAssetSyncItemRecord record;
    if ( ASSETSTORE_STATUS_OK != syncStore.GetSyncItem( m_spec.storeId, work.itemId, record ) )
    {
        record.sourceId = m_spec.storeId;
        record.itemId = work.itemId;
        record.listedRevision = work.expectedRevision;
    }
    if ( succeeded )
        CAssetLibrarySyncPlanner::NoteSucceededAttempt( record, status, NowMs() );
    else
        CAssetLibrarySyncPlanner::NoteFailedAttempt( record, status, message, NowMs(), m_spec.profile );
    TAssetSyncItemRecordVector records( 1, record );
    syncStore.WriteSyncItems( records );
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::ItemEnded( const CORE::CString& downloadJobId, TAssetStoreStatus status, const CORE::CString& message, bool succeeded )
{GUCEF_TRACE;

    TInFlightMap::iterator i = m_inFlight.find( downloadJobId );
    if ( i == m_inFlight.end() )
        return;
    const CAssetLibrarySyncPlanner::SWorkItem work = (*i).second.work;
    m_inFlight.erase( i );
    RecordAttempt( work, status, message, succeeded );
    if ( succeeded )
    {
        // the download is in the store now: it leaves the queue of the manager, which keeps the queue (and its file) as small as the window
        m_admission.NoteSuccess();
        m_manager->Remove( downloadJobId );
    }
    Fill();
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::OnDownloadJobEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CAssetStoreDownloadJobEventData* data = CORE::CICloneable::CloneableAs< CAssetStoreDownloadJobEventData >( eventData );
    if ( GUCEF_NULL == data || PHASE_RUNNING != m_phase )
        return;
    TInFlightMap::iterator i = m_inFlight.find( data->job.jobId );
    if ( i == m_inFlight.end() )
        return;
    if ( CAssetStoreDownloadManager::JobFinishedEvent == eventId )
    {
        if ( !(*i).second.downloaded )
        {
            (*i).second.downloaded = true;
            MT::CScopeMutex lock( m_dataLock );
            ++m_counts.downloaded;
            m_counts.doneBytes += (*i).second.work.estimatedBytes;
        }
        Changed();
        return;
    }
    if ( CAssetStoreDownloadManager::JobFailedEvent == eventId || ASSETDOWNLOAD_STATE_FAILED == data->job.state )
    {
        if ( ASSETSTORE_STATUS_RATE_LIMITED == data->job.status )
            m_admission.NoteRateLimited();
        ItemEnded( data->job.jobId, data->job.status, data->job.message, false );
        return;
    }
    if ( ( ASSETDOWNLOAD_STATE_CANCELLED == data->job.state || CAssetStoreDownloadManager::JobRemovedEvent == eventId ) && !(*i).second.downloaded )
    {
        // cancelled or removed by someone else: not a failure of the item, a later run plans it again
        m_inFlight.erase( i );
        {
            MT::CScopeMutex lock( m_dataLock );
            ++m_counts.cancelled;
        }
        Fill();
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::OnStoreStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CAssetStoreInfo* info = CORE::CICloneable::CloneableAs< CAssetStoreInfo >( eventData );
    if ( GUCEF_NULL == info || WAIT_SIGN_IN != m_wait || PHASE_ENDED == m_phase || info->storeId.Lowercase() != m_spec.storeId || !info->signedIn )
        return;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncJob: " << m_jobId << ": store " << m_spec.storeId << " is signed in, the listing goes on" );
    {
        MT::CScopeMutex lock( m_dataLock );
        m_status = ASSETSTORE_STATUS_OK;
        m_message.Clear();
    }
    RequestPage();
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::OnIngestEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CAssetIngestEventData* data = static_cast< const CAssetIngestEventData* >( eventData );
    if ( GUCEF_NULL == data || data->downloadJobId.IsNULLOrEmpty() || PHASE_RUNNING != m_phase )
        return;
    if ( m_inFlight.end() == m_inFlight.find( data->downloadJobId ) )
        return;
    const TAssetStoreStatus status = AssetStoreStatusFromString( data->status );
    if ( CPrimaryAssetStore::IngestFinishedEvent == eventId && ( ASSETSTORE_STATUS_OK == status || ASSETSTORE_STATUS_DUPLICATE == status ) )
    {
        {
            MT::CScopeMutex lock( m_dataLock );
            if ( ASSETSTORE_STATUS_OK == status )
                ++m_counts.ingested;
            else
                ++m_counts.revalidated;
        }
        ItemEnded( data->downloadJobId, status, data->message, true );
        return;
    }
    ItemEnded( data->downloadJobId, ASSETSTORE_STATUS_OK == status ? ASSETSTORE_STATUS_FAILED : status, data->message, false );
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::Finish( void )
{GUCEF_TRACE;

    SRunCounts counts = GetRunCounts();
    if ( m_spec.profile.indexAfter && counts.ingested > 0 )
    {
        // the packs that the ingest did not index get their index
        CAssetPipelineRequest index;
        index.operation = CAssetPipelineOperations::IndexPack;
        index.arguments.SetAttribute( "missing", true );
        CAssetServiceResult started;
        if ( ASSETSTORE_STATUS_OK == m_service.StartPipelineJob( CORE::CString::Empty, index, started ) )
        {
            MT::CScopeMutex lock( m_dataLock );
            m_indexJobId = started.id;
        }
    }
    End( ASSETJOB_STATE_DONE, ASSETSTORE_STATUS_OK, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::Cancel( void )
{GUCEF_TRACE;

    if ( PHASE_ENDED == m_phase )
        return;
    // the downloads of the run that have not finished go; what is downloaded is ingested and kept
    UInt32 cancelled = 0;
    for ( TInFlightMap::iterator i = m_inFlight.begin(); i != m_inFlight.end(); ++i )
    {
        if ( (*i).second.downloaded )
            continue;
        if ( ASSETSTORE_STATUS_OK == m_manager->Cancel( (*i).first ) )
            ++cancelled;
    }
    m_inFlight.clear();
    if ( m_future.HasAFuture() && !m_future.IsReady() )
        m_future.RequestCancellation();
    {
        MT::CScopeMutex lock( m_dataLock );
        m_counts.cancelled += cancelled;
    }
    End( ASSETJOB_STATE_CANCELLED, ASSETSTORE_STATUS_CANCELLED, "Cancelled; " + CORE::ToString( cancelled ) + " download(s) that had not finished were cancelled" );
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::End( TAssetJobState state, TAssetStoreStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    m_phase = PHASE_ENDED;
    SetWait( WAIT_NONE, 0 );
    if ( GUCEF_NULL != m_manager )
        UnsubscribeFrom( m_manager );
    if ( GUCEF_NULL != m_store )
        UnsubscribeFrom( m_store );
    UnsubscribeFrom( &m_service );
    m_subscribed = false;
    // the run is over: the service forgets its checkpoint and its work list file, and keeps the ledger of the caps of the store
    m_service.NoteSyncEnded( m_jobId, m_spec.storeId, m_admission.GetLedger() );
    {
        MT::CScopeMutex lock( m_dataLock );
        m_state = state;
        m_status = status;
        m_message = message;
        m_stateText = ASSETJOB_STATE_DONE == state && m_counts.failed > 0 ? CORE::CString( "partly failed" ) : CORE::CString::Empty;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncJob: " << m_jobId << " ended " << AssetJobStateToString( state ) << ": " << GetResultText() << ( message.IsNULLOrEmpty() ? CORE::CString::Empty : " (" + message + ")" ) );
    Changed();
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetLibrarySyncJob::Restore( const CORE::CDataNode& checkpoint, CORE::CString& problem )
{GUCEF_TRACE;

    problem.Clear();
    if ( !m_spec.IsValid( problem ) )
        return ASSETSTORE_STATUS_INVALID_ARGUMENT;
    m_store = m_service.GetPrimaryStore();
    m_manager = m_service.GetDownloadManager();
    // the service is opening: its catalogue is open, the service is not open yet
    if ( GUCEF_NULL == m_store || GUCEF_NULL == m_manager || !m_store->IsOpen() )
    {
        problem = "The library sync needs the primary store and the download manager";
        return ASSETSTORE_STATUS_UNAVAILABLE;
    }
    if ( !m_service.GetStoreProvider( m_spec.storeId, m_provider ) )
    {
        problem = "There is no store " + m_spec.storeId;
        return ASSETSTORE_STATUS_NOT_FOUND;
    }

    // what the run did before the restart counts on
    const CORE::CDataNode* counts = checkpoint.FindChild( "counts" );
    if ( GUCEF_NULL != counts )
    {
        m_counts.queued = counts->GetAttributeValueOrChildValueByName( "queued" ).AsUInt32( 0, true );
        m_counts.downloaded = counts->GetAttributeValueOrChildValueByName( "downloaded" ).AsUInt32( 0, true );
        m_counts.ingested = counts->GetAttributeValueOrChildValueByName( "ingested" ).AsUInt32( 0, true );
        m_counts.revalidated = counts->GetAttributeValueOrChildValueByName( "revalidated" ).AsUInt32( 0, true );
        m_counts.failed = counts->GetAttributeValueOrChildValueByName( "failed" ).AsUInt32( 0, true );
        m_counts.cancelled = counts->GetAttributeValueOrChildValueByName( "cancelled" ).AsUInt32( 0, true );
        m_counts.skippedInFlight = counts->GetAttributeValueOrChildValueByName( "skippedInFlight" ).AsUInt32( 0, true );
        m_counts.peakInFlight = counts->GetAttributeValueOrChildValueByName( "peakInFlight" ).AsUInt32( 0, true );
        m_counts.doneBytes = counts->GetAttributeValueOrChildValueByName( "doneBytes" ).AsUInt64( 0, true );
    }
    m_paused = checkpoint.GetAttributeValueOrChildValueByName( "paused" ).AsBool( false, true );

    // a run that had planned goes on from its work list, when the file of it can be read
    const CORE::CString phase = checkpoint.GetAttributeValueOrChildValueByName( "phase" ).AsString();
    const CORE::CString worklistPath = m_service.GetSyncWorklistPath( m_jobId );
    if ( "running" == phase && !worklistPath.IsNULLOrEmpty() && ReadWorklist( worklistPath ) )
    {
        m_next = checkpoint.GetAttributeValueOrChildValueByName( "next" ).AsUInt32( 0, true );
        if ( m_next > m_worklist.size() )
            m_next = m_worklist.size();
        const CORE::CDataNode* inFlight = checkpoint.FindChild( "inFlight" );
        if ( GUCEF_NULL != inFlight )
        {
            for ( CORE::CDataNode::const_iterator i = inFlight->ConstBegin(); i != inFlight->ConstEnd(); ++i )
            {
                const CORE::CString downloadJobId = (*i)->GetAttributeValueOrChildValueByName( "downloadJobId" ).AsString();
                if ( downloadJobId.IsNULLOrEmpty() )
                    continue;
                SInFlight entry;
                DeserializeWorkItem( *(*i), entry.work );
                entry.downloaded = (*i)->GetAttributeValueOrChildValueByName( "downloaded" ).AsBool( false, true );
                m_inFlight[ downloadJobId ] = entry;
            }
        }
        // the phase stays "created" until Begin() on the thread of the pulse generator has looked at the downloads again
        m_restoredRun = true;
        m_planner.SetVocabulary( &m_store->GetVocabulary() );
        ConfigureAdmission();
    }
    else
    if ( "running" == phase )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncJob: " << m_jobId << " has no work list file after the restart: it plans again; the downloads it queued are in flight to the plan" );
    }

    // the events of the downloads and the ingests are followed from now on: those of downloads that finish while the service starts are not missed
    Subscribe();
    {
        MT::CScopeMutex lock( m_dataLock );
        m_state = m_paused ? ASSETJOB_STATE_PAUSED : ASSETJOB_STATE_RUNNING;
        m_stateText = "resuming after a restart";
    }
    m_wakeup->Wake();
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncJob::HasCurrentPack( const CAssetLibrarySyncPlanner::SWorkItem& work ) const
{GUCEF_TRACE;

    TAssetPackSourceRecordVector facts;
    if ( ASSETSTORE_STATUS_OK != m_store->GetSyncStore().GetPackSourcesOfItem( m_spec.storeId, work.itemId, facts ) )
        return false;
    for ( size_t f=0; f<facts.size(); ++f )
    {
        if ( facts[ f ].IsCurrent() && facts[ f ].revision == work.expectedRevision &&
             ( facts[ f ].sourceFormatId == work.formatId || facts[ f ].sourceFormatId.IsNULLOrEmpty() ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::ReconcileInFlight( void )
{GUCEF_TRACE;

    // what became of the downloads the run had queued while the service was down: the queue of the manager and the store facts tell
    CORE::CString::StringVector finished;
    CORE::CString::StringVector requeue;
    CORE::CString::StringVector failed;
    for ( TInFlightMap::iterator i = m_inFlight.begin(); i != m_inFlight.end(); ++i )
    {
        CAssetStoreDownloadJob job;
        const bool known = m_manager->GetJob( (*i).first, job );
        if ( HasCurrentPack( (*i).second.work ) )
            finished.push_back( (*i).first );
        else
        if ( !known || ASSETDOWNLOAD_STATE_CANCELLED == job.state )
            requeue.push_back( (*i).first );
        else
        if ( ASSETDOWNLOAD_STATE_FAILED == job.state )
            failed.push_back( (*i).first );
        else
        if ( ASSETDOWNLOAD_STATE_DONE == job.state )
            (*i).second.downloaded = true;
    }
    for ( size_t f=0; f<finished.size(); ++f )
    {
        // ingested before the restart; the event was not seen
        {
            MT::CScopeMutex lock( m_dataLock );
            ++m_counts.ingested;
        }
        TInFlightMap::iterator i = m_inFlight.find( finished[ f ] );
        const CAssetLibrarySyncPlanner::SWorkItem work = (*i).second.work;
        m_inFlight.erase( i );
        RecordAttempt( work, ASSETSTORE_STATUS_OK, CORE::CString::Empty, true );
        m_manager->Remove( finished[ f ] );
    }
    for ( size_t r=0; r<requeue.size(); ++r )
    {
        // gone from the queue without a pack: the work item is queued again (the dispatch looks at it again first)
        TInFlightMap::iterator i = m_inFlight.find( requeue[ r ] );
        m_worklist.push_back( (*i).second.work );
        m_inFlight.erase( i );
    }
    for ( size_t f=0; f<failed.size(); ++f )
    {
        CAssetStoreDownloadJob job;
        m_manager->GetJob( failed[ f ], job );
        TInFlightMap::iterator i = m_inFlight.find( failed[ f ] );
        const CAssetLibrarySyncPlanner::SWorkItem work = (*i).second.work;
        m_inFlight.erase( i );
        RecordAttempt( work, job.status, job.message, false );
    }
    if ( !finished.empty() || !requeue.empty() || !failed.empty() )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncJob: " << m_jobId << ": " << finished.size() << " download(s) of the run were ingested before the restart, "
                          << requeue.size() << " are queued again, " << failed.size() << " failed" );
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::AdoptQueuedDownloads( void )
{GUCEF_TRACE;

    // downloads this job queued that it does not follow: queued just before a restart that came before the checkpoint
    TAssetStoreDownloadJobVector jobs;
    m_manager->GetJobs( jobs );
    UInt32 adopted = 0;
    for ( size_t j=0; j<jobs.size(); ++j )
    {
        const CAssetStoreDownloadJob& job = jobs[ j ];
        if ( job.syncJobId != m_jobId || m_inFlight.end() != m_inFlight.find( job.jobId ) ||
             ASSETDOWNLOAD_STATE_FAILED == job.state || ASSETDOWNLOAD_STATE_CANCELLED == job.state )
            continue;
        SInFlight entry;
        entry.work.itemId = job.itemId;
        entry.work.title = job.itemTitle;
        entry.work.formatId = job.formatId;
        entry.work.expectedRevision = job.expectedRevision;
        for ( size_t w=0; w<m_worklist.size(); ++w )
            if ( m_worklist[ w ].itemId == job.itemId && m_worklist[ w ].formatId == job.formatId )
                entry.work = m_worklist[ w ];
        entry.downloaded = ASSETDOWNLOAD_STATE_DONE == job.state;
        if ( entry.downloaded && HasCurrentPack( entry.work ) )
        {
            RecordAttempt( entry.work, ASSETSTORE_STATUS_OK, CORE::CString::Empty, true );
            m_manager->Remove( job.jobId );
            MT::CScopeMutex lock( m_dataLock );
            ++m_counts.ingested;
            continue;
        }
        m_inFlight[ job.jobId ] = entry;
        ++adopted;
    }
    if ( adopted > 0 )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncJob: " << m_jobId << " follows " << adopted << " download(s) it had queued before a restart" );
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::SerializeWorkItem( const CAssetLibrarySyncPlanner::SWorkItem& work, CORE::CDataNode& node )
{GUCEF_TRACE;

    node.SetAttribute( "itemId", work.itemId );
    node.SetAttribute( "title", work.title );
    node.SetAttribute( "formatId", work.formatId );
    node.SetAttribute( "expectedRevision", work.expectedRevision );
    node.SetAttribute( "estimatedBytes", work.estimatedBytes );
    node.SetAttribute( "stale", work.stale );
    node.SetAttribute( "changedAtMs", work.changedAtMs );
    node.SetAttribute( "acquiredAtMs", work.acquiredAtMs );
    node.SetAttribute( "listIndex", work.listIndex );
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::DeserializeWorkItem( const CORE::CDataNode& node, CAssetLibrarySyncPlanner::SWorkItem& work )
{GUCEF_TRACE;

    work.itemId = node.GetAttributeValueOrChildValueByName( "itemId" ).AsString();
    work.title = node.GetAttributeValueOrChildValueByName( "title" ).AsString();
    work.formatId = node.GetAttributeValueOrChildValueByName( "formatId" ).AsString();
    work.expectedRevision = node.GetAttributeValueOrChildValueByName( "expectedRevision" ).AsString();
    work.estimatedBytes = node.GetAttributeValueOrChildValueByName( "estimatedBytes" ).AsUInt64( 0, true );
    work.stale = node.GetAttributeValueOrChildValueByName( "stale" ).AsBool( false, true );
    work.changedAtMs = node.GetAttributeValueOrChildValueByName( "changedAtMs" ).AsInt64( 0, true );
    work.acquiredAtMs = node.GetAttributeValueOrChildValueByName( "acquiredAtMs" ).AsInt64( 0, true );
    work.listIndex = node.GetAttributeValueOrChildValueByName( "listIndex" ).AsUInt32( 0, true );
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncJob::WriteWorklist( void ) const
{GUCEF_TRACE;

    const CORE::CString path = m_service.GetSyncWorklistPath( m_jobId );
    if ( path.IsNULLOrEmpty() )
        return false;
    CORE::CDataNodeSerializableSettings settings;
    CORE::CDataNode root( "syncWorklist", GUCEF_DATATYPE_OBJECT );
    root.SetAttribute( "version", (UInt32) 1 );
    root.SetAttribute( "jobId", m_jobId );
    {
        MT::CScopeMutex lock( m_dataLock );
        CORE::CDataNode* planNode = root.AddChild( "plan", GUCEF_DATATYPE_OBJECT );
        m_plan.Serialize( *planNode, settings );
        CORE::CDataNode* lines = root.AddChild( "lines", GUCEF_DATATYPE_ARRAY );
        for ( size_t i=0; i<m_plan.items.size(); ++i )
            m_plan.items[ i ].Serialize( *lines->AddChild( "item", GUCEF_DATATYPE_OBJECT ), settings );
    }
    CORE::CDataNode* work = root.AddChild( "work", GUCEF_DATATYPE_ARRAY );
    for ( size_t w=0; w<m_worklist.size(); ++w )
        SerializeWorkItem( m_worklist[ w ], *work->AddChild( "item", GUCEF_DATATYPE_OBJECT ) );
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().StoreAsFile( path, root, "json", true ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncJob: " << m_jobId << " could not write its work list to " << path << ": a restart plans again" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncJob::ReadWorklist( const CORE::CString& path )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.FileExists( path ) )
        return false;
    CORE::CDataNode root;
    if ( !vfs.LoadFile( root, path, "json" ) || m_jobId != root.GetAttributeValueOrChildValueByName( "jobId" ).AsString() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetLibrarySyncJob: " << m_jobId << " cannot use the work list file " << path << ": it plans again" );
        return false;
    }
    CORE::CDataNodeSerializableSettings settings;
    CAssetLibrarySyncPlan plan;
    const CORE::CDataNode* planNode = root.FindChild( "plan" );
    if ( GUCEF_NULL != planNode )
        plan.Deserialize( *planNode, settings );
    const CORE::CDataNode* lines = root.FindChild( "lines" );
    if ( GUCEF_NULL != lines )
    {
        for ( CORE::CDataNode::const_iterator i = lines->ConstBegin(); i != lines->ConstEnd(); ++i )
        {
            CAssetLibrarySyncItem line;
            if ( line.Deserialize( *(*i), settings ) )
                plan.items.push_back( line );
        }
    }
    m_worklist.clear();
    const CORE::CDataNode* work = root.FindChild( "work" );
    if ( GUCEF_NULL != work )
    {
        for ( CORE::CDataNode::const_iterator i = work->ConstBegin(); i != work->ConstEnd(); ++i )
        {
            CAssetLibrarySyncPlanner::SWorkItem item;
            DeserializeWorkItem( *(*i), item );
            if ( !item.itemId.IsNULLOrEmpty() )
                m_worklist.push_back( item );
        }
    }
    MT::CScopeMutex lock( m_dataLock );
    m_plan = plan;
    m_storageFreeBytes = plan.storageFreeBytes;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::Checkpoint( bool force )
{GUCEF_TRACE;

    // what a restart needs to go on: the spec, the phase, the place in the work list, the downloads in flight and the counts
    CORE::CDataNode node( "syncJob", GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "jobId", m_jobId );
    node.SetAttribute( "phase", PHASE_RUNNING == m_phase ? CORE::CString( "running" ) : CORE::CString( "listing" ) );
    node.SetAttribute( "paused", m_paused );
    node.SetAttribute( "next", (UInt32) m_next );
    CORE::CDataNodeSerializableSettings settings;
    m_spec.Serialize( *node.AddChild( "spec", GUCEF_DATATYPE_OBJECT ), settings );
    CORE::CString signature = node.GetAttributeValueOrChildValueByName( "phase" ).AsString() + ( m_paused ? "|p|" : "|r|" ) + CORE::ToString( (UInt32) m_next );
    CORE::CDataNode* inFlight = node.AddChild( "inFlight", GUCEF_DATATYPE_ARRAY );
    for ( TInFlightMap::const_iterator i = m_inFlight.begin(); i != m_inFlight.end(); ++i )
    {
        CORE::CDataNode* entry = inFlight->AddChild( "download", GUCEF_DATATYPE_OBJECT );
        entry->SetAttribute( "downloadJobId", (*i).first );
        entry->SetAttribute( "downloaded", (*i).second.downloaded );
        SerializeWorkItem( (*i).second.work, *entry );
        signature += "|" + (*i).first + ( (*i).second.downloaded ? "+" : "-" );
    }
    {
        MT::CScopeMutex lock( m_dataLock );
        CORE::CDataNode* counts = node.AddChild( "counts", GUCEF_DATATYPE_OBJECT );
        counts->SetAttribute( "queued", m_counts.queued );
        counts->SetAttribute( "downloaded", m_counts.downloaded );
        counts->SetAttribute( "ingested", m_counts.ingested );
        counts->SetAttribute( "revalidated", m_counts.revalidated );
        counts->SetAttribute( "failed", m_counts.failed );
        counts->SetAttribute( "cancelled", m_counts.cancelled );
        counts->SetAttribute( "skippedInFlight", m_counts.skippedInFlight );
        counts->SetAttribute( "peakInFlight", m_counts.peakInFlight );
        counts->SetAttribute( "doneBytes", m_counts.doneBytes );
        signature += "|" + CORE::ToString( m_counts.queued ) + "/" + CORE::ToString( m_counts.ingested ) + "/" + CORE::ToString( m_counts.failed );
    }
    if ( !force && signature == m_lastCheckpoint )
        return;
    m_lastCheckpoint = signature;
    m_service.NoteSyncCheckpoint( m_jobId, m_spec.storeId, node, m_admission.GetLedger() );
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncJob::IsEnded( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return AssetJobStateIsFinal( m_state );
}

/*-------------------------------------------------------------------------*/

TAssetJobState
CAssetLibrarySyncJob::GetState( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return m_state;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetLibrarySyncJob::GetStatus( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return m_status;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetLibrarySyncJob::GetFailedCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return m_counts.failed;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetLibrarySyncJob::GetMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return m_message;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetLibrarySyncJob::GetResultText( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return GetResultTextLocked();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetLibrarySyncJob::GetResultTextLocked( void ) const
{GUCEF_TRACE;

    CORE::CString text = m_plan.Describe();
    if ( ASSETJOB_STATE_DONE != m_state && ASSETJOB_STATE_CANCELLED != m_state )
        return text;
    if ( CAssetLibrarySyncSpec::ModeRun == m_spec.mode )
        text += "; " + CORE::ToString( m_counts.ingested ) + " ingested, " + CORE::ToString( m_counts.revalidated ) + " revalidated, " + CORE::ToString( m_counts.failed ) + " failed";
    return text;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::GetPlan( CAssetLibrarySyncPlan& plan ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    plan = m_plan;
}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncJob::SRunCounts
CAssetLibrarySyncJob::GetRunCounts( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return m_counts;
}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncJob::FillEntry( CAssetJobEntry& entry ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    entry.subKind = "sync";
    entry.providerId = m_spec.storeId;
    entry.scope = m_spec.storeId + ( m_spec.IsPlanOnly() ? CORE::CString( ", plan" ) : CORE::CString::Empty );
    entry.state = m_state;
    entry.status = m_status;
    entry.stateText = m_stateText;
    entry.message = m_message;
    entry.itemsTotal = m_plan.queuedDownloads;
    entry.itemsDone = m_counts.ingested + m_counts.revalidated + m_counts.failed + m_counts.cancelled + m_counts.skippedInFlight;
    entry.totalBytes = m_plan.estimatedBytes;
    entry.doneBytes = m_counts.doneBytes;
    entry.fraction = ASSETJOB_STATE_DONE == m_state ? 1.0f : ( entry.itemsTotal > 0 ? (Float32) entry.itemsDone / (Float32) entry.itemsTotal : 0.0f );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetLibrarySyncJob::GetResult( const CORE::CString& section, const CORE::CString& pageToken, UInt32 maxEntries, CAssetServiceDocument& result ) const
{GUCEF_TRACE;

    CORE::CDataNodeSerializableSettings settings;
    MT::CScopeMutex lock( m_dataLock );
    result.status = m_status;
    result.message = m_message;
    result.document.Clear();
    result.document.SetName( "result" );
    result.document.SetNodeType( GUCEF_DATATYPE_OBJECT );
    if ( "items" == section )
    {
        const UInt32 first = CORE::StringToUInt32( pageToken, 0 );
        const UInt32 count = CIAssetStoreService::ClampPageEntries( maxEntries );
        CORE::CDataNode* items = result.document.AddChild( "items", GUCEF_DATATYPE_ARRAY );
        UInt32 i = first;
        for ( ; i<(UInt32) m_plan.items.size() && i<first+count && GUCEF_NULL != items; ++i )
        {
            if ( "not-selected" == m_plan.items[ i ].reason )
                continue;
            CORE::CDataNode* line = items->AddChild( "item", GUCEF_DATATYPE_OBJECT );
            if ( GUCEF_NULL != line )
                m_plan.items[ i ].Serialize( *line, settings );
        }
        result.document.SetAttribute( "total", (UInt32) m_plan.items.size() );
        if ( i < (UInt32) m_plan.items.size() )
            result.nextPageToken = CORE::ToString( i );
        return ASSETSTORE_STATUS_OK;
    }
    if ( !section.IsNULLOrEmpty() && "plan" != section && "summary" != section )
        return ASSETSTORE_STATUS_NOT_FOUND;
    CORE::CDataNode* planNode = result.document.AddChild( "plan", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL != planNode )
        m_plan.Serialize( *planNode, settings );
    CORE::CDataNode* run = result.document.AddChild( "run", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL != run )
    {
        run->SetAttribute( "queued", m_counts.queued );
        run->SetAttribute( "downloaded", m_counts.downloaded );
        run->SetAttribute( "ingested", m_counts.ingested );
        run->SetAttribute( "revalidated", m_counts.revalidated );
        run->SetAttribute( "failed", m_counts.failed );
        run->SetAttribute( "cancelled", m_counts.cancelled );
        run->SetAttribute( "skippedInFlight", m_counts.skippedInFlight );
        run->SetAttribute( "peakInFlight", m_counts.peakInFlight );
        run->SetAttribute( "window", m_admission.GetConfiguredWindow() );
        run->SetAttribute( "doneBytes", m_counts.doneBytes );
        run->SetAttribute( "indexJobId", m_indexJobId );
    }
    CORE::CDataNode* failures = result.document.AddChild( "failures", GUCEF_DATATYPE_ARRAY );
    for ( size_t f=0; f<m_failures.size() && GUCEF_NULL != failures; ++f )
    {
        CORE::CDataNode* failure = failures->AddChild( "failure", GUCEF_DATATYPE_OBJECT );
        if ( GUCEF_NULL == failure )
            continue;
        failure->SetAttribute( "itemId", m_failures[ f ].itemId );
        failure->SetAttribute( "formatId", m_failures[ f ].formatId );
        failure->SetAttribute( "status", m_failures[ f ].status );
        failure->SetAttribute( "message", m_failures[ f ].message );
    }
    CORE::CDataNode* counts = result.document.AddChild( "counts", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL != counts )
        counts->SetAttribute( "failed", m_counts.failed );
    result.document.SetAttribute( "summary", GetResultTextLocked() );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetLibrarySyncJob::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetLibrarySyncJob";
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
