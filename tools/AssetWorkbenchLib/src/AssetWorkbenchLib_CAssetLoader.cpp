/*
 *  AssetWorkbenchLib: opening an asset without blocking the interface
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

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_GUI_CACTIVITYSCOPE_H
#include "gucefGUI_CActivityScope.h"
#define GUCEF_GUI_CACTIVITYSCOPE_H
#endif /* GUCEF_GUI_CACTIVITYSCOPE_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETLOADTASKCONSUMER_H
#include "AssetWorkbenchLib_CAssetLoadTaskConsumer.h"
#define ASSETWORKBENCHLIB_CASSETLOADTASKCONSUMER_H
#endif /* ASSETWORKBENCHLIB_CASSETLOADTASKCONSUMER_H ? */

#include "AssetWorkbenchLib_CAssetLoader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {


const CORE::CEvent CAssetLoader::LoadFinishedEvent = "GUCEF::ASSETWORKBENCH::CAssetLoader::LoadFinishedEvent";
const CORE::CString CAssetLoader::DefaultThreadPoolName = "AssetWorkbenchLoading";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The task consumer factory is registered while any loader exists */
UInt32 g_factoryUsers = 0;
TAssetLoadTaskConsumerFactory* g_factory = GUCEF_NULL;

void
AcquireFactory( void )
{GUCEF_TRACE;

    if ( 0 == g_factoryUsers++ )
    {
        g_factory = GUCEF_NEW TAssetLoadTaskConsumerFactory();
        CORE::CCoreGlobal::Instance()->GetTaskManager().RegisterTaskConsumerFactory( CAssetLoadTaskConsumer::TaskType, g_factory );
    }
}

void
ReleaseFactory( void )
{GUCEF_TRACE;

    if ( 0 == --g_factoryUsers )
    {
        CORE::CCoreGlobal::Instance()->GetTaskManager().UnregisterTaskConsumerFactory( CAssetLoadTaskConsumer::TaskType );
        GUCEF_DELETE g_factory;
        g_factory = GUCEF_NULL;
    }
}

UInt64
NowInMs( void )
{GUCEF_TRACE;

    const UInt64 resolution = MT::PrecisionTimerResolution();
    return 0 == resolution ? 0 : MT::PrecisionTickCount() * 1000 / resolution;
}

/** The data of the task of a request that ended, when it is ready */
const CAssetLoadTaskData*
GetOutcomeData( const CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( future.HasNoFuture() || !future.IsReady() )
        return GUCEF_NULL;
    CORE::CTaskPtr task = future.GetResult();
    return task.IsNULL() ? GUCEF_NULL : static_cast< const CAssetLoadTaskData* >( task->GetTaskData() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CAssetLoader::RegisterEvents( void )
{GUCEF_TRACE;

    LoadFinishedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAssetLoader::SRequest::SRequest( void )
    : id( 0 )
    , job()
    , future( CORE::TASKSTATUS_UNDEFINED )
    , activityId( 0 )
    , jobNumber( 0 )
    , state( STATE_PREPARING )
    , stepCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLoader::CAssetLoader( GUI::CActivityService* activity )
    : CORE::CObservingNotifier()
    , m_activity( activity )
    , m_registry( GUCEF_NULL )
    , m_defaultKind( "open" )
    , m_nextKind()
    , m_nextSubKind()
    , m_nextScope()
    , m_threadPoolName()
    , m_current()
    , m_hasCurrent( false )
    , m_ended()
    , m_nextRequestId( 1 )
    , m_lastFinished( 0 )
    , m_lastOutcome( OUTCOME_NONE )
    , m_lastProblem()
    , m_lastStepCount( 0 )
    , m_droppedCount( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    AcquireFactory();
    if ( GUCEF_NULL != m_activity )
    {
        TEventCallback cancelCallback( this, &CAssetLoader::OnCancelRequested );
        SubscribeTo( m_activity, GUI::CActivityService::CancelRequestedEvent, cancelCallback );
    }
}

/*-------------------------------------------------------------------------*/

CAssetLoader::~CAssetLoader()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();

    // the tasks that were started are given time to see the cancellation: they use the job and the activity service
    Abandon();
    for ( size_t i=0; i<m_ended.size(); ++i )
    {
        try
        {
            if ( m_ended[ i ].future.HasAFuture() )
            {
                m_ended[ i ].future.RequestCancellation();
                m_ended[ i ].future.Await( 30000 );
            }
        }
        catch ( ... )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetLoader: A preparation did not stop in time" );
        }
        m_ended[ i ].job->Discard();
    }
    m_ended.clear();
    ReleaseFactory();
}

/*-------------------------------------------------------------------------*/

void
CAssetLoader::SetJobRegistry( CWorkbenchJobRegistry* registry, const CORE::CString& defaultKind )
{GUCEF_TRACE;

    m_registry = registry;
    m_defaultKind = defaultKind;
}

/*-------------------------------------------------------------------------*/

void
CAssetLoader::DescribeNext( const CORE::CString& kind, const CORE::CString& subKind, const CORE::CString& scope )
{GUCEF_TRACE;

    m_nextKind = kind;
    m_nextSubKind = subKind;
    m_nextScope = scope;
}

/*-------------------------------------------------------------------------*/

void
CAssetLoader::SetThreadPoolName( const CORE::CString& name )
{GUCEF_TRACE;

    m_threadPoolName = name;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CAssetLoader::Begin( const TAssetLoadJobPtr& job, const CORE::CString& label, UInt32 expectedDurationInMs )
{GUCEF_TRACE;

    if ( m_hasCurrent )
        Finish( OUTCOME_CANCELLED, "Another asset was opened" );

    m_current = SRequest();
    m_current.id = m_nextRequestId++;
    if ( 0 == m_nextRequestId )
        m_nextRequestId = 1;
    m_current.job = job;
    m_hasCurrent = true;
    if ( GUCEF_NULL != m_activity )
        m_current.activityId = m_activity->Begin( label, expectedDurationInMs, job->ReportsProgress(), true );
    if ( GUCEF_NULL != m_registry )
    {
        m_current.jobNumber = m_registry->Begin( m_nextKind.IsNULLOrEmpty() ? m_defaultKind : m_nextKind, m_nextSubKind, label, m_nextScope.IsNULLOrEmpty() ? label : m_nextScope, true, expectedDurationInMs );
        m_registry->LinkActivity( m_current.jobNumber, m_current.activityId );
    }
    m_nextKind.Clear();
    m_nextSubKind.Clear();
    m_nextScope.Clear();

    try
    {
        CAssetLoadTaskData taskData;
        taskData.job = job;
        taskData.activity = m_activity;
        taskData.activityId = m_current.activityId;
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        const bool ownPool = m_threadPoolName.IsNULLOrEmpty();
        CORE::ThreadPoolPtr threadPool = taskManager.GetOrCreateThreadPool( ownPool ? DefaultThreadPoolName : m_threadPoolName );
        if ( !threadPool.IsNULL() )
        {
            // one request at a time: a preparation that was ended (and cannot be stopped inside a decoder) is over before the next begins
            if ( ownPool )
                threadPool->SetDesiredMaxTotalNrOfThreads( 1 );
            m_current.future = threadPool->QueueTask( CAssetLoadTaskConsumer::TaskType, &taskData );
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetLoader: Failed to queue the preparation of " << label << ": " << e.what() );
    }
    return m_current.future;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetLoader::GetCurrentRequest( void ) const
{GUCEF_TRACE;

    return m_hasCurrent ? m_current.id : 0;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLoader::IsBusy( void ) const
{GUCEF_TRACE;

    return m_hasCurrent;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetLoader::GetCurrentActivity( void ) const
{GUCEF_TRACE;

    return m_hasCurrent ? m_current.activityId : 0;
}

/*-------------------------------------------------------------------------*/

void
CAssetLoader::Abandon( void )
{GUCEF_TRACE;

    if ( !m_hasCurrent )
        return;
    if ( STATE_PREPARING == m_current.state && m_current.future.HasAFuture() && !m_current.future.IsReady() )
    {
        // the task may still run: the job is discarded when it has stopped
        m_current.future.RequestCancellation();
        m_ended.push_back( m_current );
    }
    else
    {
        m_current.job->Discard();
    }
    if ( GUCEF_NULL != m_activity && 0 != m_current.activityId )
        m_activity->End( m_current.activityId );
    m_hasCurrent = false;
    m_current = SRequest();
}

/*-------------------------------------------------------------------------*/

void
CAssetLoader::Finish( EOutcome outcome, const CORE::CString& problem )
{GUCEF_TRACE;

    if ( !m_hasCurrent )
        return;
    if ( GUCEF_NULL != m_registry && 0 != m_current.jobNumber )
    {
        m_registry->End( m_current.jobNumber, OUTCOME_SUCCEEDED == outcome ? ASSETSTORE::ASSETJOB_STATE_DONE : ( OUTCOME_CANCELLED == outcome ? ASSETSTORE::ASSETJOB_STATE_CANCELLED : ASSETSTORE::ASSETJOB_STATE_FAILED ),
                         OUTCOME_SUCCEEDED == outcome ? CORE::CString( "done" ) : problem );
        m_current.jobNumber = 0;
    }
    const UInt32 id = m_current.id;
    const UInt32 steps = m_current.stepCount;
    if ( OUTCOME_SUCCEEDED == outcome )
    {
        // its result is in the views: the job has nothing left to let go of
        if ( GUCEF_NULL != m_activity && 0 != m_current.activityId )
            m_activity->End( m_current.activityId );
        m_hasCurrent = false;
        m_current = SRequest();
    }
    else
        Abandon();
    m_lastFinished = id;
    m_lastOutcome = outcome;
    m_lastProblem = problem;
    m_lastStepCount = steps;
    CORE::TCloneableUInt32 eventData( id );
    NotifyObservers( LoadFinishedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CAssetLoader::Cancel( void )
{GUCEF_TRACE;

    Finish( OUTCOME_CANCELLED, "Cancelled" );
}

/*-------------------------------------------------------------------------*/

void
CAssetLoader::Update( UInt32 sliceBudgetInMs )
{GUCEF_TRACE;

    // the results of requests that were ended meanwhile are dropped, now that their tasks stopped
    for ( size_t i=m_ended.size(); i>0; --i )
    {
        SRequest& request = m_ended[ i-1 ];
        if ( request.future.HasNoFuture() || request.future.IsReady() )
        {
            request.job->Discard();
            ++m_droppedCount;
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AssetLoader: The result of request " << request.id << " was dropped, it was ended before it arrived" );
            m_ended.erase( m_ended.begin() + ( i-1 ) );
        }
    }

    if ( !m_hasCurrent )
        return;

    if ( STATE_PREPARING == m_current.state )
    {
        if ( m_current.future.HasNoFuture() )
        {
            Finish( OUTCOME_FAILED, "The preparation could not be started" );
            return;
        }
        if ( !m_current.future.IsReady() )
            return;
        const CAssetLoadTaskData* data = GetOutcomeData( m_current.future );
        if ( GUCEF_NULL == data || !data->prepared )
        {
            CORE::CString problem = GUCEF_NULL != data ? data->problem : CORE::CString( "The preparation ended without a result" );
            if ( GUCEF_NULL != data && data->cancelled )
                Finish( OUTCOME_CANCELLED, "Cancelled" );
            else
            {
                if ( problem.IsNULLOrEmpty() )
                    problem = m_current.job->GetProblem();
                Finish( OUTCOME_FAILED, problem.IsNULLOrEmpty() ? CORE::CString( "The asset cannot be loaded" ) : problem );
            }
            return;
        }
        m_current.state = STATE_STEPPING;
    }

    // the pieces of the renderer: at least one per frame, more while the budget lasts
    const UInt64 start = NowInMs();
    for ( ;; )
    {
        ++m_current.stepCount;
        const CIAssetLoadJob::EStepResult result = m_current.job->Step();
        if ( CIAssetLoadJob::STEP_DONE == result )
        {
            Finish( OUTCOME_SUCCEEDED, CORE::CString::Empty );
            return;
        }
        if ( CIAssetLoadJob::STEP_FAILED == result )
        {
            const CORE::CString problem = m_current.job->GetProblem();
            Finish( OUTCOME_FAILED, problem.IsNULLOrEmpty() ? CORE::CString( "The asset cannot be shown" ) : problem );
            return;
        }
        if ( NowInMs() - start >= sliceBudgetInMs )
            return;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetLoader::Run( const TAssetLoadJobPtr& jobToRun, const CORE::CString& label )
{GUCEF_TRACE;

    TAssetLoadJobPtr job( jobToRun );
    m_lastStepCount = 0;
    m_lastProblem.Clear();
    GUI::CActivityScope scope( m_activity, label, 0, job->ReportsProgress(), false );
    CAssetLoadControl control( m_activity, scope.GetId(), GUCEF_NULL );
    bool ok = false;
    try
    {
        ok = job->Prepare( control );
    }
    catch ( const std::exception& e )
    {
        m_lastProblem = CORE::CString( "The asset could not be prepared: " ) + e.what();
    }
    if ( !ok )
    {
        if ( m_lastProblem.IsNULLOrEmpty() )
            m_lastProblem = job->GetProblem();
        job->Discard();
        return false;
    }
    for ( ;; )
    {
        ++m_lastStepCount;
        const CIAssetLoadJob::EStepResult result = job->Step();
        if ( CIAssetLoadJob::STEP_DONE == result )
            return true;
        if ( CIAssetLoadJob::STEP_FAILED == result )
        {
            m_lastProblem = job->GetProblem();
            job->Discard();
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetLoader::GetLastFinishedRequest( void ) const
{GUCEF_TRACE;

    return m_lastFinished;
}

/*-------------------------------------------------------------------------*/

CAssetLoader::EOutcome
CAssetLoader::GetLastOutcome( void ) const
{GUCEF_TRACE;

    return m_lastOutcome;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetLoader::GetLastProblem( void ) const
{GUCEF_TRACE;

    return m_lastProblem;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetLoader::GetDroppedCount( void ) const
{GUCEF_TRACE;

    return m_droppedCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetLoader::GetLastStepCount( void ) const
{GUCEF_TRACE;

    return m_lastStepCount;
}

/*-------------------------------------------------------------------------*/

void
CAssetLoader::OnCancelRequested( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableUInt32* id = static_cast< const CORE::TCloneableUInt32* >( eventData );
    if ( GUCEF_NULL != id && m_hasCurrent && 0 != m_current.activityId && id->GetData() == m_current.activityId )
        Cancel();
}

/*-------------------------------------------------------------------------*/

void
CAssetLoader::OnNotify( CORE::CNotifier* notifier    ,
                        const CORE::CEvent& eventId  ,
                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CORE::CNotifier::DestructionEvent == eventId && GUCEF_NULL != notifier && notifier == m_activity )
    {
        // the tasks keep a pointer to the service: they are told to stop, and the request goes on without feedback
        m_activity = GUCEF_NULL;
        if ( m_hasCurrent )
        {
            m_current.activityId = 0;
            Abandon();
        }
        return;
    }
    CORE::CObservingNotifier::OnNotify( notifier, eventId, eventData );
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
