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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHJOBFILTER_H
#include "AssetWorkbenchLib_CWorkbenchJobFilter.h"
#define ASSETWORKBENCHLIB_CWORKBENCHJOBFILTER_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHJOBFILTER_H ? */

#ifndef ASSETWORKBENCHLIB_CBULKCONVERSIONCONTROLLER_H
#include "AssetWorkbenchLib_CBulkConversionController.h"
#define ASSETWORKBENCHLIB_CBULKCONVERSIONCONTROLLER_H
#endif /* ASSETWORKBENCHLIB_CBULKCONVERSIONCONTROLLER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CBulkConversionController::PlanReadyEvent = "GUCEF::ASSETWORKBENCH::CBulkConversionController::PlanReadyEvent";
const CORE::CEvent CBulkConversionController::JobStartedEvent = "GUCEF::ASSETWORKBENCH::CBulkConversionController::JobStartedEvent";
const CORE::CEvent CBulkConversionController::NotificationEvent = "GUCEF::ASSETWORKBENCH::CBulkConversionController::NotificationEvent";

/*-------------------------------------------------------------------------*/

void
CBulkConversionController::RegisterEvents( void )
{GUCEF_TRACE;

    PlanReadyEvent.Initialize();
    JobStartedEvent.Initialize();
    NotificationEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CBulkConversionController::SPackScope::SPackScope( void )
    : packId()
    , assets()
    , refused( false )
    , workCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CBulkConversionController::CBulkConversionController( ASSETSTORE::CIAssetStoreService& service )
    : CORE::CObservingNotifier()
    , m_service( service )
    , m_settings()
    , m_scopes()
    , m_plan()
    , m_packPlans()
    , m_planFutures()
    , m_resolveFuture( CORE::TASKSTATUS_UNDEFINED )
    , m_stage( STAGE_IDLE )
    , m_jobIds()
    , m_notified()
    , m_notifications()
    , m_startFutures()
    , m_problem()
    , m_commandCounter( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    TEventCallback changed( this, &CBulkConversionController::OnJobChanged );
    SubscribeTo( &m_service, ASSETSTORE::CIAssetStoreService::JobChangedEvent, changed );
}

/*-------------------------------------------------------------------------*/

CBulkConversionController::~CBulkConversionController()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionController::RequestPlan( const CBulkConversionSettings& settings )
{GUCEF_TRACE;

    m_plan.Clear();
    m_packPlans.clear();
    m_planFutures.clear();
    m_scopes.clear();
    m_problem.Clear();
    m_stage = STAGE_IDLE;
    if ( !settings.IsValid( m_problem ) )
        return false;
    m_settings = settings;
    ResolveScope();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionController::ResolveScope( void )
{GUCEF_TRACE;

    if ( CBulkConversionSettings::ScopeSavedSearch != m_settings.scopeKind )
    {
        SPackScope scope;
        scope.packId = m_settings.packId;
        m_settings.GetScopeGlobs( scope.assets );
        m_scopes.push_back( scope );
        BeginPlanning();
        return;
    }
    if ( !m_service.IsInProcess() )
    {
        m_stage = STAGE_RESOLVING;
        m_resolveFuture = m_service.RunSavedSearchAsync( m_settings.savedSearchId, 0 );
        if ( !m_resolveFuture.HasAFuture() )
        {
            m_problem = "The saved search cannot be asked for";
            m_stage = STAGE_IDLE;
            NotifyObservers( PlanReadyEvent );
        }
        return;
    }
    ASSETSTORE::CAssetSearchResultPage page;
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service.RunSavedSearch( m_settings.savedSearchId, 0, page ) )
    {
        m_problem = "The saved search " + m_settings.savedSearchId + " cannot be run";
        NotifyObservers( PlanReadyEvent );
        return;
    }
    for ( size_t h=0; h<page.hits.size(); ++h )
    {
        bool known = false;
        for ( size_t s=0; s<m_scopes.size() && !known; ++s )
            known = m_scopes[ s ].packId == page.hits[ h ].packId;
        if ( !known && !page.hits[ h ].packId.IsNULLOrEmpty() )
        {
            SPackScope scope;
            scope.packId = page.hits[ h ].packId;
            m_scopes.push_back( scope );
        }
    }
    BeginPlanning();
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionController::BeginPlanning( void )
{GUCEF_TRACE;

    if ( m_scopes.empty() )
    {
        m_problem = "The scope names no pack";
        FinishPlanning();
        return;
    }
    m_stage = STAGE_PLANNING;
    m_packPlans.resize( m_scopes.size() );
    m_planFutures.resize( m_scopes.size(), CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED ) );
    for ( size_t i=0; i<m_scopes.size(); ++i )
    {
        ASSETSTORE::CAssetPipelineRequest request;
        if ( !m_settings.MakeRequest( m_scopes[ i ].packId, m_scopes[ i ].assets, true, request ) )
            continue;       // the asset filter leaves nothing of the scope in this pack: no work
        if ( m_service.IsInProcess() )
        {
            ASSETSTORE::CAssetServiceDocument answer;
            m_service.RunPipelineQuery( request, answer );
            TakePlan( i, answer );
        }
        else
            m_planFutures[ i ] = m_service.RunPipelineQueryAsync( request );
    }
    if ( m_service.IsInProcess() )
        FinishPlanning();
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionController::TakePlan( size_t index, const ASSETSTORE::CAssetServiceDocument& answer )
{GUCEF_TRACE;

    CBulkConversionPlan& plan = m_packPlans[ index ];
    if ( !plan.Parse( answer ) && !answer.message.IsNULLOrEmpty() )
        m_problem = answer.message;
    m_scopes[ index ].refused = !plan.conversionAllowed;
    m_scopes[ index ].workCount = plan.valid && plan.conversionAllowed && plan.lockHolder.IsNULLOrEmpty() ? plan.GetWorkCount() : 0;
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionController::FinishPlanning( void )
{GUCEF_TRACE;

    m_plan.Clear();
    bool anyAllowed = false;
    for ( size_t i=0; i<m_packPlans.size(); ++i )
    {
        const CBulkConversionPlan& plan = m_packPlans[ i ];
        if ( !plan.valid && plan.conversionAllowed )
            continue;
        m_plan.valid = m_plan.valid || plan.valid;
        if ( plan.conversionAllowed )
            anyAllowed = true;
        else
            m_plan.refusalReason += ( m_plan.refusalReason.IsNULLOrEmpty() ? "" : "; " ) + m_scopes[ i ].packId + ": " + plan.refusalReason;
        m_plan.importCount += plan.importCount;
        m_plan.cachedCount += plan.cachedCount;
        m_plan.cookCount += plan.cookCount;
        m_plan.refusedCount += plan.refusedCount;
        m_plan.failedCount += plan.failedCount;
        m_plan.validateCount += plan.validateCount;
        m_plan.noIntermediateCount += plan.noIntermediateCount;
        m_plan.filteredOutCount += plan.filteredOutCount;
        m_plan.lastRunAssetTimeMs += plan.lastRunAssetTimeMs;
        m_plan.totalItems += plan.totalItems;
        if ( m_plan.lockHolder.IsNULLOrEmpty() )
            m_plan.lockHolder = plan.lockHolder;
        for ( size_t r=0; r<plan.restrictions.size(); ++r )
            m_plan.restrictions.push_back( plan.restrictions[ r ] );
        for ( size_t p=0; p<plan.problems.size(); ++p )
            m_plan.problems.push_back( plan.problems[ p ] );
        if ( m_plan.cookTargets.empty() )
            m_plan.cookTargets = plan.cookTargets;
        for ( size_t t=0; t<plan.items.size() && m_plan.items.size() < 500; ++t )
            m_plan.items.push_back( plan.items[ t ] );
        if ( m_plan.packId.IsNULLOrEmpty() )
            m_plan.packId = plan.packId;
    }
    m_plan.conversionAllowed = anyAllowed || m_packPlans.empty();
    m_stage = STAGE_IDLE;
    NotifyObservers( PlanReadyEvent );
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionController::IsPlanning( void ) const
{GUCEF_TRACE;

    return STAGE_IDLE != m_stage;
}

/*-------------------------------------------------------------------------*/

const CBulkConversionPlan&
CBulkConversionController::GetPlan( void ) const
{GUCEF_TRACE;

    return m_plan;
}

/*-------------------------------------------------------------------------*/

const CBulkConversionController::TPackScopeVector&
CBulkConversionController::GetPackScopes( void ) const
{GUCEF_TRACE;

    return m_scopes;
}

/*-------------------------------------------------------------------------*/

const CBulkConversionSettings&
CBulkConversionController::GetPlannedSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionController::CanStart( void ) const
{GUCEF_TRACE;

    if ( IsPlanning() || !m_plan.valid )
        return false;
    for ( size_t i=0; i<m_scopes.size(); ++i )
        if ( !m_scopes[ i ].refused && m_scopes[ i ].workCount > 0 )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CBulkConversionController::StartOne( const SPackScope& scope )
{GUCEF_TRACE;

    ASSETSTORE::CAssetPipelineRequest request;
    if ( !m_settings.MakeRequest( scope.packId, scope.assets, false, request ) )
        return 0;
    const CORE::CString commandId = "bulk-" + CORE::UInt64ToString( MT::PrecisionTickCount() ) + "-" + CORE::UInt32ToString( ++m_commandCounter );
    if ( !m_service.IsInProcess() )
    {
        CORE::CFutureResult future = m_service.StartPipelineJobAsync( commandId, request );
        if ( !future.HasAFuture() )
            return 0;
        m_startFutures.push_back( future );
        return 1;
    }
    ASSETSTORE::CAssetServiceResult result;
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service.StartPipelineJob( commandId, request, result ) )
    {
        m_problem = scope.packId + ": " + result.message;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "BulkConversion: The job for " << scope.packId << " was refused: " << result.message );
        return 0;
    }
    m_jobIds.push_back( result.id );
    CORE::TCloneableString data( result.id );
    NotifyObservers( JobStartedEvent, &data );
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32
CBulkConversionController::Start( void )
{GUCEF_TRACE;

    m_problem.Clear();
    if ( !CanStart() )
    {
        m_problem = m_plan.valid ? ( m_plan.conversionAllowed ? CORE::CString( "The plan has no work" ) : "Refused: " + m_plan.refusalReason ) : CORE::CString( "Make the plan first" );
        return 0;
    }
    UInt32 started = 0;
    for ( size_t i=0; i<m_scopes.size(); ++i )
    {
        if ( m_scopes[ i ].refused )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "BulkConversion: The pack " << m_scopes[ i ].packId << " is skipped, the licence does not allow the conversion" );
            continue;
        }
        if ( 0 == m_scopes[ i ].workCount )
            continue;
        started += StartOne( m_scopes[ i ] );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "BulkConversion: " << m_settings.Describe() << ": " << started << " job(s) started" );
    return started;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringVector&
CBulkConversionController::GetJobIds( void ) const
{GUCEF_TRACE;

    return m_jobIds;
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionController::IsTracked( const CORE::CString& jobId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_jobIds.size(); ++i )
        if ( m_jobIds[ i ] == jobId )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionController::TrackJob( const CORE::CString& jobId )
{GUCEF_TRACE;

    if ( !jobId.IsNULLOrEmpty() && !IsTracked( jobId ) )
        m_jobIds.push_back( jobId );
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionController::SendControl( const CORE::CString& jobId, ASSETSTORE::TAssetJobAction action, const CORE::CString& what )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "BulkConversion: " << what << " of " << jobId );
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionController::ResumeJob( const CORE::CString& jobId )
{GUCEF_TRACE;

    SendControl( jobId, ASSETSTORE::ASSETJOB_ACTION_RESUME, "resume" );
    if ( !m_service.IsInProcess() )
    {
        CORE::CFutureResult future = m_service.ControlJobAsync( CORE::CString::Empty, jobId, ASSETSTORE::ASSETJOB_ACTION_RESUME );
        if ( future.HasAFuture() )
            m_startFutures.push_back( future );
        return future.HasAFuture();
    }
    ASSETSTORE::CAssetServiceResult result;
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service.ControlJob( CORE::CString::Empty, jobId, ASSETSTORE::ASSETJOB_ACTION_RESUME, result ) )
        return false;
    TrackJob( result.id );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionController::RetryFailedOfJob( const CORE::CString& jobId )
{GUCEF_TRACE;

    SendControl( jobId, ASSETSTORE::ASSETJOB_ACTION_RETRY, "retry of the failed assets" );
    if ( !m_service.IsInProcess() )
    {
        CORE::CFutureResult future = m_service.ControlJobAsync( CORE::CString::Empty, jobId, ASSETSTORE::ASSETJOB_ACTION_RETRY );
        if ( future.HasAFuture() )
            m_startFutures.push_back( future );
        return future.HasAFuture();
    }
    ASSETSTORE::CAssetServiceResult result;
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service.ControlJob( CORE::CString::Empty, jobId, ASSETSTORE::ASSETJOB_ACTION_RETRY, result ) )
        return false;
    TrackJob( result.id );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBulkConversionController::CancelJob( const CORE::CString& jobId )
{GUCEF_TRACE;

    if ( !m_service.IsInProcess() )
        return m_service.ControlJobAsync( CORE::CString::Empty, jobId, ASSETSTORE::ASSETJOB_ACTION_CANCEL ).HasAFuture();
    ASSETSTORE::CAssetServiceResult result;
    return ASSETSTORE::ASSETSTORE_STATUS_OK == m_service.ControlJob( CORE::CString::Empty, jobId, ASSETSTORE::ASSETJOB_ACTION_CANCEL, result );
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionController::Update( void )
{GUCEF_TRACE;

    // the saved search of a remote service
    if ( STAGE_RESOLVING == m_stage && m_resolveFuture.IsReady() )
    {
        ASSETSTORE::CAssetSearchResultPage page;
        if ( ASSETSTORE::CIAssetStoreService::TryGetFutureResult( m_resolveFuture, page ) )
        {
            for ( size_t h=0; h<page.hits.size(); ++h )
            {
                bool known = false;
                for ( size_t s=0; s<m_scopes.size() && !known; ++s )
                    known = m_scopes[ s ].packId == page.hits[ h ].packId;
                if ( !known && !page.hits[ h ].packId.IsNULLOrEmpty() )
                {
                    SPackScope scope;
                    scope.packId = page.hits[ h ].packId;
                    m_scopes.push_back( scope );
                }
            }
            m_resolveFuture = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
            BeginPlanning();
        }
        else
        {
            m_problem = "The saved search cannot be read";
            m_resolveFuture = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
            m_stage = STAGE_IDLE;
            NotifyObservers( PlanReadyEvent );
        }
    }
    if ( STAGE_PLANNING == m_stage && !m_service.IsInProcess() )
    {
        bool allDone = true;
        for ( size_t i=0; i<m_planFutures.size(); ++i )
        {
            if ( !m_planFutures[ i ].HasAFuture() )
                continue;
            if ( !m_planFutures[ i ].IsReady() )
            {
                allDone = false;
                continue;
            }
            ASSETSTORE::CAssetServiceDocument answer;
            if ( ASSETSTORE::CIAssetStoreService::TryGetFutureResult( m_planFutures[ i ], answer ) )
                TakePlan( i, answer );
            else
            {
                answer.status = ASSETSTORE::ASSETSTORE_STATUS_FAILED;
                answer.message = ASSETSTORE::CIAssetStoreService::GetMessageOfFuture( m_planFutures[ i ] );
                TakePlan( i, answer );
            }
            m_planFutures[ i ] = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
        }
        if ( allDone )
            FinishPlanning();
    }
    // the ids of the jobs a remote service started for us
    for ( size_t i=0; i<m_startFutures.size(); )
    {
        if ( !m_startFutures[ i ].IsReady() )
        {
            ++i;
            continue;
        }
        ASSETSTORE::CAssetServiceResult result;
        if ( ASSETSTORE::CIAssetStoreService::TryGetFutureResult( m_startFutures[ i ], result ) && !result.id.IsNULLOrEmpty() )
        {
            TrackJob( result.id );
            CORE::TCloneableString data( result.id );
            NotifyObservers( JobStartedEvent, &data );
        }
        m_startFutures.erase( m_startFutures.begin() + i );
    }
}

/*-------------------------------------------------------------------------*/

void
CBulkConversionController::OnJobChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const ASSETSTORE::CAssetJobEntry* job = CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetJobEntry >( eventData );
    if ( GUCEF_NULL == job || !job->IsEnded() || !IsTracked( job->jobId ) )
        return;
    for ( size_t i=0; i<m_notified.size(); ++i )
        if ( m_notified[ i ] == job->jobId )
            return;
    m_notified.push_back( job->jobId );
    const CORE::CString text = CWorkbenchJobFilter::DisplayKindOf( *job ) + " of " + job->scope + " " + ASSETSTORE::AssetJobStateToString( job->state ) +
                               ( job->resultText.IsNULLOrEmpty() ? CORE::CString::Empty : ": " + job->resultText );
    m_notifications.push_back( text );
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "BulkConversion: " << text );
    CORE::TCloneableString data( text );
    NotifyObservers( NotificationEvent, &data );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CBulkConversionController::GetProblem( void ) const
{GUCEF_TRACE;

    return m_problem;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringVector&
CBulkConversionController::GetNotifications( void ) const
{GUCEF_TRACE;

    return m_notifications;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CBulkConversionController::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CBulkConversionController";
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
