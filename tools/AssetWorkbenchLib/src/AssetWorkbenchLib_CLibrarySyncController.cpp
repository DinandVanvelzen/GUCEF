/*
 *  AssetWorkbenchLib: views and controllers of the asset workbench
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLAN_H
#include "gucefASSETSTORE_CAssetLibrarySyncPlan.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLAN_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLAN_H ? */

#include "AssetWorkbenchLib_CLibrarySyncController.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CLibrarySyncController::StoresChangedEvent = "GUCEF::ASSETWORKBENCH::CLibrarySyncController::StoresChangedEvent";
const CORE::CEvent CLibrarySyncController::StoreChangedEvent = "GUCEF::ASSETWORKBENCH::CLibrarySyncController::StoreChangedEvent";
const CORE::CEvent CLibrarySyncController::PlanReadyEvent = "GUCEF::ASSETWORKBENCH::CLibrarySyncController::PlanReadyEvent";
const CORE::CEvent CLibrarySyncController::JobStartedEvent = "GUCEF::ASSETWORKBENCH::CLibrarySyncController::JobStartedEvent";
const CORE::CEvent CLibrarySyncController::NotificationEvent = "GUCEF::ASSETWORKBENCH::CLibrarySyncController::NotificationEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CLibrarySyncController::RegisterEvents( void )
{GUCEF_TRACE;

    StoresChangedEvent.Initialize();
    StoreChangedEvent.Initialize();
    PlanReadyEvent.Initialize();
    JobStartedEvent.Initialize();
    NotificationEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CLibrarySyncController::SStoreState::SStoreState( void )
    : storeId()
    , displayName()
    , signedIn( false )
    , planJobId()
    , planning( false )
    , planned( false )
    , listed( 0 )
    , missing( 0 )
    , stale( 0 )
    , current( 0 )
    , unavailable( 0 )
    , queuedDownloads( 0 )
    , estimatedBytes( 0 )
    , freeBytes( 0 )
    , storageVerdict()
    , runJobId()
    , running( false )
    , runState( ASSETSTORE::ASSETJOB_STATE_QUEUED )
    , itemsDone( 0 )
    , itemsTotal( 0 )
    , failed( 0 )
    , retryAvailable( false )
    , fraction( 0.0f )
    , progressText()
    , lastResult()
    , problem()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLibrarySyncController::SPending::SPending( void )
    : future( CORE::TASKSTATUS_UNDEFINED )
    , storeId()
    , kind( PENDING_STORES )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLibrarySyncController::CLibrarySyncController( ASSETSTORE::CIAssetStoreService& service )
    : CORE::CObservingNotifier()
    , m_service( service )
    , m_stores()
    , m_pending()
    , m_notifications()
    , m_problem()
    , m_commandCounter( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    TEventCallback changed( this, &CLibrarySyncController::OnJobChanged );
    SubscribeTo( &m_service, ASSETSTORE::CIAssetStoreService::JobChangedEvent, changed );
    TEventCallback storeChanged( this, &CLibrarySyncController::OnStoreStateChanged );
    SubscribeTo( &m_service, ASSETSTORE::CIAssetStoreService::StoreStateChangedEvent, storeChanged );
}

/*-------------------------------------------------------------------------*/

CLibrarySyncController::~CLibrarySyncController()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLibrarySyncController::NextCommandId( void )
{GUCEF_TRACE;

    return "sync-" + CORE::UInt64ToString( MT::PrecisionTickCount() ) + "-" + CORE::UInt32ToString( ++m_commandCounter );
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncController::StoreChanged( const CORE::CString& storeId )
{GUCEF_TRACE;

    CORE::TCloneableString data( storeId );
    NotifyObservers( StoreChangedEvent, &data );
}

/*-------------------------------------------------------------------------*/

CLibrarySyncController::SStoreState*
CLibrarySyncController::FindStoreState( const CORE::CString& storeId )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_stores.size(); ++i )
        if ( m_stores[ i ].storeId == storeId )
            return &m_stores[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CLibrarySyncController::SStoreState&
CLibrarySyncController::GetOrAddStoreState( const CORE::CString& storeId )
{GUCEF_TRACE;

    SStoreState* state = FindStoreState( storeId );
    if ( GUCEF_NULL != state )
        return *state;
    SStoreState added;
    added.storeId = storeId;
    added.displayName = storeId;
    m_stores.push_back( added );
    return m_stores.back();
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncController::RefreshStores( void )
{GUCEF_TRACE;

    m_problem.Clear();
    if ( !m_service.IsInProcess() )
    {
        SPending stores;
        stores.kind = PENDING_STORES;
        stores.future = m_service.ListStoresAsync();
        SPending jobs;
        jobs.kind = PENDING_JOBS;
        jobs.future = m_service.ListJobsAsync( CORE::CString::Empty, ASSETSTORE::CIAssetStoreService::MaxPageEntries );
        if ( !stores.future.HasAFuture() )
        {
            m_problem = "The stores of the service cannot be asked for";
            return false;
        }
        m_pending.push_back( stores );
        if ( jobs.future.HasAFuture() )
            m_pending.push_back( jobs );
        return true;
    }
    ASSETSTORE::CAssetStoreInfoPage stores;
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service.ListStores( stores ) )
    {
        m_problem = "The stores of the service cannot be read";
        return false;
    }
    TakeStores( stores );
    ASSETSTORE::CAssetJobPage jobs;
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK == m_service.ListJobs( CORE::CString::Empty, ASSETSTORE::CIAssetStoreService::MaxPageEntries, jobs ) )
        TakeJobs( jobs );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncController::TakeStores( const ASSETSTORE::CAssetStoreInfoPage& page )
{GUCEF_TRACE;

    // the stores keep their state; a store that is gone goes, unless a job of it is followed
    TStoreStateVector stores;
    for ( size_t i=0; i<page.entries.size(); ++i )
    {
        const ASSETSTORE::CAssetStoreInfo& info = page.entries[ i ];
        SStoreState* known = FindStoreState( info.storeId );
        SStoreState state = GUCEF_NULL != known ? *known : SStoreState();
        state.storeId = info.storeId;
        state.displayName = info.displayName.IsNULLOrEmpty() ? info.storeId : info.displayName;
        state.signedIn = info.signedIn;
        stores.push_back( state );
    }
    for ( size_t i=0; i<m_stores.size(); ++i )
    {
        bool listed = false;
        for ( size_t s=0; s<stores.size() && !listed; ++s )
            listed = stores[ s ].storeId == m_stores[ i ].storeId;
        if ( !listed && ( m_stores[ i ].running || m_stores[ i ].planning ) )
            stores.push_back( m_stores[ i ] );
    }
    m_stores = stores;
    NotifyObservers( StoresChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncController::TakeJobs( const ASSETSTORE::CAssetJobPage& page )
{GUCEF_TRACE;

    // the syncs that run on the service are followed, also those this client did not start
    for ( size_t i=0; i<page.entries.size(); ++i )
    {
        const ASSETSTORE::CAssetJobEntry& job = page.entries[ i ];
        if ( "sync" != job.subKind || job.providerId.IsNULLOrEmpty() || job.IsEnded() )
            continue;
        SStoreState& state = GetOrAddStoreState( job.providerId );
        const bool plan = job.scope.HasSubstr( ", plan" ) > 0;
        if ( plan )
        {
            state.planJobId = job.jobId;
            state.planning = true;
        }
        else
            state.runJobId = job.jobId;
        TakeJob( job );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CLibrarySyncController::GetStoreCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_stores.size();
}

/*-------------------------------------------------------------------------*/

const CLibrarySyncController::SStoreState*
CLibrarySyncController::GetStore( UInt32 index ) const
{GUCEF_TRACE;

    return index < (UInt32) m_stores.size() ? &m_stores[ index ] : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CLibrarySyncController::SStoreState*
CLibrarySyncController::FindStore( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    return const_cast< CLibrarySyncController* >( this )->FindStoreState( storeId );
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncController::StartJob( const CORE::CString& storeId, const ASSETSTORE::CAssetLibrarySyncSpec& spec )
{GUCEF_TRACE;

    m_problem.Clear();
    SStoreState& state = GetOrAddStoreState( storeId );
    state.problem.Clear();
    CORE::CString reason;
    if ( !spec.IsValid( reason ) )
    {
        m_problem = reason;
        state.problem = reason;
        return false;
    }
    ASSETSTORE::CAssetPipelineRequest request;
    spec.ToRequest( request );
    const bool plan = spec.IsPlanOnly();
    if ( plan )
    {
        state.planning = true;
        state.planned = false;
    }
    else
    {
        state.running = true;
        state.progressText = "starting";
        state.itemsDone = 0;
        state.itemsTotal = 0;
        state.fraction = 0.0f;
    }
    const CORE::CString commandId = NextCommandId();
    if ( !m_service.IsInProcess() )
    {
        SPending pending;
        pending.kind = plan ? PENDING_START_PLAN : PENDING_START_RUN;
        pending.storeId = storeId;
        pending.future = m_service.StartPipelineJobAsync( commandId, request );
        if ( !pending.future.HasAFuture() )
        {
            m_problem = "The sync of " + storeId + " cannot be asked for";
            state.problem = m_problem;
            state.planning = state.planning && !plan;
            state.running = state.running && plan;
            return false;
        }
        m_pending.push_back( pending );
        StoreChanged( storeId );
        return true;
    }
    ASSETSTORE::CAssetServiceResult result;
    const ASSETSTORE::TAssetStoreStatus status = m_service.StartPipelineJob( commandId, request, result );
    TakeStarted( storeId, plan, status, result );
    return ASSETSTORE::ASSETSTORE_STATUS_OK == status || ASSETSTORE::ASSETSTORE_STATUS_ALREADY_EXISTS == status;
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncController::TakeStarted( const CORE::CString& storeId, bool plan, ASSETSTORE::TAssetStoreStatus status, const ASSETSTORE::CAssetServiceResult& result )
{GUCEF_TRACE;

    SStoreState& state = GetOrAddStoreState( storeId );
    if ( ASSETSTORE::ASSETSTORE_STATUS_ALREADY_EXISTS == status && result.id.IsNULLOrEmpty() )
    {
        // a sync of the store runs already and the answer did not carry its id (a remote failure answer): the job list names it
        state.running = true;
        state.planning = false;
        state.progressText = "a sync of the store runs already";
        if ( m_service.IsInProcess() )
        {
            ASSETSTORE::CAssetJobPage jobs;
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK == m_service.ListJobs( CORE::CString::Empty, ASSETSTORE::CIAssetStoreService::MaxPageEntries, jobs ) )
                TakeJobs( jobs );
        }
        else
        {
            SPending pending;
            pending.kind = PENDING_JOBS;
            pending.future = m_service.ListJobsAsync( CORE::CString::Empty, ASSETSTORE::CIAssetStoreService::MaxPageEntries );
            if ( pending.future.HasAFuture() )
                m_pending.push_back( pending );
        }
        StoreChanged( storeId );
        return;
    }
    if ( ( ASSETSTORE::ASSETSTORE_STATUS_OK != status && ASSETSTORE::ASSETSTORE_STATUS_ALREADY_EXISTS != status ) || result.id.IsNULLOrEmpty() )
    {
        // refused: the reason is the row's
        m_problem = result.message.IsNULLOrEmpty() ? CORE::CString( ASSETSTORE::AssetStoreStatusToString( status ) ) : result.message;
        state.problem = m_problem;
        if ( plan )
            state.planning = false;
        else
        {
            state.running = false;
            state.progressText.Clear();
        }
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LibrarySync: The " << ( plan ? "plan" : "sync" ) << " of store " << storeId << " was refused: " << m_problem );
        StoreChanged( storeId );
        return;
    }
    if ( ASSETSTORE::ASSETSTORE_STATUS_ALREADY_EXISTS == status )
    {
        // a sync of the store runs already: that one is followed
        state.runJobId = result.id;
        state.running = true;
        state.planning = false;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LibrarySync: A sync of store " << storeId << " runs already, it is followed: " << result.id );
    }
    else
    if ( plan )
        state.planJobId = result.id;
    else
        state.runJobId = result.id;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LibrarySync: The " << ( plan ? "plan" : "sync" ) << " of store " << storeId << " is job " << result.id );
    CORE::TCloneableString data( result.id );
    NotifyObservers( JobStartedEvent, &data );
    StoreChanged( storeId );

    // a job that ended before its id was known here (a service in this process can be that quick)
    ASSETSTORE::CAssetJobEntry job;
    if ( m_service.IsInProcess() && ASSETSTORE::ASSETSTORE_STATUS_OK == m_service.GetJob( result.id, job ) )
        TakeJob( job );
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncController::RequestPlan( const CORE::CString& storeId )
{GUCEF_TRACE;

    ASSETSTORE::CAssetLibrarySyncSpec spec;
    spec.storeId = storeId;
    spec.mode = ASSETSTORE::CAssetLibrarySyncSpec::ModePlan;
    spec.useStoreProfile = true;
    return StartJob( storeId, spec );
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncController::StartSync( const CORE::CString& storeId )
{GUCEF_TRACE;

    ASSETSTORE::CAssetLibrarySyncSpec spec;
    spec.storeId = storeId;
    spec.mode = ASSETSTORE::CAssetLibrarySyncSpec::ModeRun;
    spec.useStoreProfile = true;
    return StartJob( storeId, spec );
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncController::DownloadItems( const CORE::CString& storeId, const CORE::CString::StringVector& itemIds )
{GUCEF_TRACE;

    if ( itemIds.empty() )
    {
        m_problem = "No item is selected";
        return false;
    }
    ASSETSTORE::CAssetLibrarySyncSpec spec;
    spec.storeId = storeId;
    spec.mode = ASSETSTORE::CAssetLibrarySyncSpec::ModeRun;
    spec.useStoreProfile = true;
    spec.itemIds = itemIds;
    // a download that a person asks for item by item: the free space reserve of the profile does not apply, an item that
    // does not fit on the library volume at all is refused
    spec.manual = true;
    return StartJob( storeId, spec );
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncController::Control( const CORE::CString& jobId, ASSETSTORE::TAssetJobAction action )
{GUCEF_TRACE;

    m_problem.Clear();
    if ( jobId.IsNULLOrEmpty() )
    {
        m_problem = "There is no sync to " + CORE::CString( ASSETSTORE::AssetJobActionToString( action ) );
        return false;
    }
    if ( !m_service.IsInProcess() )
    {
        SPending pending;
        pending.kind = PENDING_CONTROL;
        pending.future = m_service.ControlJobAsync( NextCommandId(), jobId, action );
        if ( !pending.future.HasAFuture() )
            return false;
        m_pending.push_back( pending );
        return true;
    }
    ASSETSTORE::CAssetServiceResult result;
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service.ControlJob( NextCommandId(), jobId, action, result ) )
    {
        m_problem = result.message.IsNULLOrEmpty() ? "The sync " + jobId + " cannot be told to " + ASSETSTORE::AssetJobActionToString( action ) : result.message;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncController::Cancel( const CORE::CString& storeId )
{GUCEF_TRACE;

    const SStoreState* state = FindStore( storeId );
    if ( GUCEF_NULL == state )
        return false;
    const CORE::CString jobId = state->running ? state->runJobId : ( state->planning ? state->planJobId : CORE::CString::Empty );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LibrarySync: cancel of " << jobId << " of store " << storeId );
    return Control( jobId, ASSETSTORE::ASSETJOB_ACTION_CANCEL );
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncController::RetryFailed( const CORE::CString& storeId )
{GUCEF_TRACE;

    const SStoreState* state = FindStore( storeId );
    if ( GUCEF_NULL == state || state->running )
    {
        m_problem = "The sync of " + storeId + " has not ended";
        return false;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LibrarySync: retry of the failed items of " << state->runJobId << " of store " << storeId );
    if ( !Control( state->runJobId, ASSETSTORE::ASSETJOB_ACTION_RETRY ) )
        return false;
    SStoreState& changed = GetOrAddStoreState( storeId );
    changed.running = true;
    changed.progressText = "starting";
    StoreChanged( storeId );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncController::OnJobChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const ASSETSTORE::CAssetJobEntry* job = CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetJobEntry >( eventData );
    if ( GUCEF_NULL == job || "sync" != job->subKind )
        return;
    TakeJob( *job );
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncController::TakeJob( const ASSETSTORE::CAssetJobEntry& job )
{GUCEF_TRACE;

    const CORE::CString storeId = job.providerId;
    SStoreState* state = FindStoreState( storeId );
    if ( GUCEF_NULL == state )
        return;
    const bool plan = job.jobId == state->planJobId;
    const bool run = job.jobId == state->runJobId;
    if ( !plan && !run )
    {
        // a run of the store that another client or a retry started: followed from now on
        if ( job.scope.HasSubstr( ", plan" ) > 0 || job.IsEnded() )
            return;
        state->runJobId = job.jobId;
    }
    if ( plan )
    {
        state->progressText = job.IsEnded() ? CORE::CString::Empty : job.stateText;
        if ( job.IsEnded() && state->planning )
        {
            state->planning = false;
            if ( ASSETSTORE::ASSETJOB_STATE_DONE == job.state )
                AskResult( storeId, job.jobId, true );
            else
            {
                state->problem = job.message.IsNULLOrEmpty() ? "The plan ended " + CORE::CString( ASSETSTORE::AssetJobStateToString( job.state ) ) : job.message;
                CORE::TCloneableString data( storeId );
                NotifyObservers( PlanReadyEvent, &data );
            }
        }
        StoreChanged( storeId );
        return;
    }
    state->runState = job.state;
    state->itemsDone = job.itemsDone;
    state->itemsTotal = job.itemsTotal;
    state->retryAvailable = job.IsActionAvailable( ASSETSTORE::ASSETJOB_ACTION_RETRY );
    state->fraction = job.fraction >= 0.0f ? job.fraction : ( job.totalBytes > 0 ? (Float32) job.doneBytes / (Float32) job.totalBytes : 0.0f );
    state->progressText = job.stateText.IsNULLOrEmpty() ? CORE::CString( ASSETSTORE::AssetJobStateToString( job.state ) ) : job.stateText;
    if ( !job.IsEnded() )
    {
        state->running = true;
        StoreChanged( storeId );
        return;
    }
    if ( state->running )
    {
        state->running = false;
        state->lastResult = CORE::CString( ASSETSTORE::AssetJobStateToString( job.state ) ) + ( job.resultText.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + job.resultText ) +
                            ( job.message.IsNULLOrEmpty() ? CORE::CString::Empty : " (" + job.message + ")" );
        const CORE::CString notice = "Library sync of " + state->displayName + " " + state->lastResult;
        m_notifications.push_back( notice );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "LibrarySync: " << notice );
        // the plan counts were those before the run; the next plan says what is left
        state->planned = false;
        CORE::TCloneableString data( notice );
        NotifyObservers( NotificationEvent, &data );
        // the counts of the run (the failed downloads) come with its result
        AskResult( storeId, job.jobId, false );
        state = FindStoreState( storeId );
        if ( GUCEF_NULL == state )
            return;
    }
    StoreChanged( storeId );
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncController::OnStoreStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const ASSETSTORE::CAssetStoreInfo* info = CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetStoreInfo >( eventData );
    if ( GUCEF_NULL == info )
        return;
    SStoreState* state = FindStoreState( info->storeId );
    if ( GUCEF_NULL == state || state->signedIn == info->signedIn )
        return;
    state->signedIn = info->signedIn;
    StoreChanged( info->storeId );
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncController::AskResult( const CORE::CString& storeId, const CORE::CString& jobId, bool plan )
{GUCEF_TRACE;

    if ( !m_service.IsInProcess() )
    {
        SPending pending;
        pending.kind = plan ? PENDING_RESULT_PLAN : PENDING_RESULT_RUN;
        pending.storeId = storeId;
        pending.future = m_service.GetJobResultAsync( jobId, "plan", CORE::CString::Empty, 0 );
        if ( pending.future.HasAFuture() )
            m_pending.push_back( pending );
        return;
    }
    ASSETSTORE::CAssetServiceDocument result;
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK == m_service.GetJobResult( jobId, "plan", CORE::CString::Empty, 0, result ) )
        TakeResult( storeId, plan, result );
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncController::TakeResult( const CORE::CString& storeId, bool plan, const ASSETSTORE::CAssetServiceDocument& result )
{GUCEF_TRACE;

    SStoreState& state = GetOrAddStoreState( storeId );
    if ( !plan )
    {
        // the result of a run: its failed downloads
        const CORE::CDataNode* run = result.document.FindChild( "run" );
        if ( GUCEF_NULL != run )
            state.failed = run->GetAttributeValueOrChildValueByName( "failed" ).AsUInt32( 0, true );
        StoreChanged( storeId );
        return;
    }
    const CORE::CDataNode* planNode = result.document.FindChild( "plan" );
    if ( GUCEF_NULL != planNode )
    {
        ASSETSTORE::CAssetLibrarySyncPlan counts;
        CORE::CDataNodeSerializableSettings settings;
        counts.Deserialize( *planNode, settings );
        state.listed = counts.listed;
        state.missing = counts.GetCount( "missing" );
        state.stale = counts.GetCount( "stale" );
        state.current = counts.GetCount( "current" );
        state.unavailable = counts.GetCount( "unavailable" );
        state.queuedDownloads = counts.queuedDownloads;
        state.estimatedBytes = counts.estimatedBytes;
        state.freeBytes = counts.storageFreeBytes;
        state.storageVerdict = counts.storageVerdict;
        state.planned = true;
    }
    else
        state.problem = result.message;
    CORE::TCloneableString data( storeId );
    NotifyObservers( PlanReadyEvent, &data );
    StoreChanged( storeId );
}

/*-------------------------------------------------------------------------*/

void
CLibrarySyncController::Update( void )
{GUCEF_TRACE;

    // the answers that are there, in the order they were asked; the others wait for the next frame
    TPendingVector waiting;
    TPendingVector ready;
    for ( size_t i=0; i<m_pending.size(); ++i )
    {
        if ( m_pending[ i ].future.IsReady() || m_pending[ i ].future.HasNoFuture() )
            ready.push_back( m_pending[ i ] );
        else
            waiting.push_back( m_pending[ i ] );
    }
    m_pending = waiting;
    for ( size_t i=0; i<ready.size(); ++i )
    {
        const SPending& pending = ready[ i ];
        const ASSETSTORE::TAssetStoreStatus status = ASSETSTORE::CIAssetStoreService::GetStatusOfFuture( pending.future );
        switch ( pending.kind )
        {
            case PENDING_STORES:
            {
                ASSETSTORE::CAssetStoreInfoPage page;
                if ( ASSETSTORE::CIAssetStoreService::TryGetFutureResult( pending.future, page ) )
                    TakeStores( page );
                else
                    m_problem = "The stores of the service could not be read: " + ASSETSTORE::CIAssetStoreService::GetMessageOfFuture( pending.future );
                break;
            }
            case PENDING_JOBS:
            {
                ASSETSTORE::CAssetJobPage page;
                if ( ASSETSTORE::CIAssetStoreService::TryGetFutureResult( pending.future, page ) )
                    TakeJobs( page );
                break;
            }
            case PENDING_START_PLAN:
            case PENDING_START_RUN:
            {
                ASSETSTORE::CAssetServiceResult result;
                if ( !ASSETSTORE::CIAssetStoreService::TryGetFutureResult( pending.future, result ) )
                    result.message = ASSETSTORE::CIAssetStoreService::GetMessageOfFuture( pending.future );
                TakeStarted( pending.storeId, PENDING_START_PLAN == pending.kind, status, result );
                break;
            }
            case PENDING_RESULT_PLAN:
            case PENDING_RESULT_RUN:
            {
                ASSETSTORE::CAssetServiceDocument result;
                if ( ASSETSTORE::CIAssetStoreService::TryGetFutureResult( pending.future, result ) )
                    TakeResult( pending.storeId, PENDING_RESULT_PLAN == pending.kind, result );
                break;
            }
            case PENDING_CONTROL:
            default:
            {
                if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status )
                    m_problem = ASSETSTORE::CIAssetStoreService::GetMessageOfFuture( pending.future );
                break;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CLibrarySyncController::IsWaiting( void ) const
{GUCEF_TRACE;

    return !m_pending.empty();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLibrarySyncController::GetProblem( void ) const
{GUCEF_TRACE;

    return m_problem;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringVector&
CLibrarySyncController::GetNotifications( void ) const
{GUCEF_TRACE;

    return m_notifications;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLibrarySyncController::FormatBytes( UInt64 bytes )
{GUCEF_TRACE;

    static const char* const units[] = { "bytes", "KiB", "MiB", "GiB", "TiB" };
    if ( bytes < 1024 )
        return CORE::UInt64ToString( bytes ) + " bytes";
    Float64 value = (Float64) bytes;
    UInt32 unit = 0;
    while ( value >= 1024.0 && unit < 4 )
    {
        value /= 1024.0;
        ++unit;
    }
    // one decimal, rounded
    const UInt64 tenths = (UInt64) ( value * 10.0 + 0.5 );
    return CORE::UInt64ToString( tenths / 10 ) + "." + CORE::UInt64ToString( tenths % 10 ) + " " + units[ unit ];
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLibrarySyncController::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CLibrarySyncController";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
