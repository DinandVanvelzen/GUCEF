/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

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

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestJobSchedulerSupport.h"
#include "TestJobRunProgress.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace CORETEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Steps of the progress task that may be taken: the task waits before step n until it is at least n */
MT::CTAtomic< UInt32 > g_rpReleased( 0 );
/** How many progress reports of the last task were accepted and refused */
MT::CTAtomic< UInt32 > g_rpAccepted( 0 );
MT::CTAtomic< UInt32 > g_rpRefused( 0 );
MT::CTAtomic< UInt32 > g_rpDocumentAccepted( 0 );

/** The task data of the progress task: how many steps and the result document it names */
class CRpTestTaskData : public CORE::CIDataNodeSerializableTaskData
{
    public:

    static const CORE::CString TaskType;

    UInt32 steps;
    bool gated;                         /**< waits for g_rpReleased before every step after the first */
    CORE::CString document;

    CRpTestTaskData( void ) : CORE::CIDataNodeSerializableTaskData(), steps( 3 ), gated( false ), document() {}
    CRpTestTaskData( const CRpTestTaskData& src ) : CORE::CIDataNodeSerializableTaskData( src ), steps( src.steps ), gated( src.gated ), document( src.document ) {}
    virtual ~CRpTestTaskData() GUCEF_VIRTUAL_OVERRIDE {}

    virtual const CORE::CString& GetTaskTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }
    virtual const CORE::CString& GetThreadPoolName( void ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::CString::Empty; }
    virtual bool GetOnlyUseExistingThreadPool( void ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool GetTaskCanBeQueued( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual UInt32 GetTaskId( void ) const GUCEF_VIRTUAL_OVERRIDE { return 0; }
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CRpTestTaskData( *this ); }
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { static const CORE::CString name = "CORETEST::CRpTestTaskData"; return name; }
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return CloneableTypeName(); }
    static const char* CloneableTypeName( void ) { return "CORETEST::CRpTestTaskData"; }

    virtual bool Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE
    {
        domRootNode.SetAttribute( "steps", steps );
        domRootNode.SetAttribute( "gated", gated );
        domRootNode.SetAttribute( "document", document );
        return true;
    }

    virtual bool Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CVariant stepsVar = domRootNode.GetAttributeValueOrChildValueByName( "steps" );
        if ( stepsVar.IsNULLOrEmpty() )
            return false;
        steps = stepsVar.AsUInt32( 3, true );
        gated = domRootNode.GetAttributeValueOrChildValueByName( "gated" ).AsBool( false, true );
        document = domRootNode.GetAttributeValueOrChildValueByName( "document" ).AsString( CORE::CString::Empty, true );
        return true;
    }
};

const CORE::CString CRpTestTaskData::TaskType = "CORETEST::RunProgressTask";

/*-------------------------------------------------------------------------*/

/** Reports one step per cycle: "step <n>" with n of the steps; the result document at the end */
class CRpTestTask : public CORE::CTaskConsumer
{
    public:

    CRpTestTask( void ) : CORE::CTaskConsumer(), m_step( 0 ) {}
    virtual ~CRpTestTask() GUCEF_VIRTUAL_OVERRIDE {}

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return CRpTestTaskData::TaskType; }

    /** A pool may hand the same consumer the next task: every task starts at its first step */
    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_step = 0;
        return CORE::CTaskConsumer::OnTaskStart( task );
    }

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        const CRpTestTaskData* data = task.IsNULL() ? GUCEF_NULL : MT::CICloneable::CloneableAs< CRpTestTaskData >( task->GetTaskData() );
        UInt32 steps = GUCEF_NULL != data ? data->steps : 3;
        bool gated = GUCEF_NULL != data && data->gated;
        // asked to cancel: not done (false), so that the delegator ends the task CANCELLED and not FINISHED
        const UInt32 next = m_step + 1;
        if ( gated && next > 1 )
        {
            for ( UInt32 waited=0; waited<10000 && g_rpReleased.Load() < next; ++waited )
            {
                if ( task->IsCancellationRequested() )
                    return false;
                MT::ThreadDelay( 1 );
            }
        }
        m_step = next;
        if ( ReportRunProgress( m_step, steps, "step " + CORE::ToString( m_step ) ) )
            g_rpAccepted.FetchAdd( 1 );
        else
            g_rpRefused.FetchAdd( 1 );
        if ( m_step < steps )
            return false;
        if ( GUCEF_NULL != data && !data->document.IsNULLOrEmpty() && ReportRunResultDocument( data->document ) )
            g_rpDocumentAccepted.FetchAdd( 1 );
        return true;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
    }

    private:

    UInt32 m_step;
};

typedef CORE::CTFactory< CORE::CTaskConsumer, CRpTestTask, MT::CMutex > TRpTaskFactory;
typedef CORE::CTFactory< CORE::CIDataNodeSerializableTaskData, CRpTestTaskData, MT::CMutex > TRpTaskDataFactory;
TRpTaskFactory g_rpTaskFactory;
TRpTaskDataFactory g_rpTaskDataFactory;

/*-------------------------------------------------------------------------*/

void
RegisterRpTaskType( void )
{
    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    taskManager.RegisterTaskConsumerFactory( CRpTestTaskData::TaskType, &g_rpTaskFactory );
    taskManager.RegisterTaskDataFactory( CRpTestTaskData::TaskType, &g_rpTaskDataFactory );
}

void
UnregisterRpTaskType( void )
{
    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    taskManager.UnregisterTaskConsumerFactory( CRpTestTaskData::TaskType );
    taskManager.UnregisterTaskDataFactory( CRpTestTaskData::TaskType );
}

void
ResetRpCounters( void )
{
    g_rpReleased.Store( 0 );
    g_rpAccepted.Store( 0 );
    g_rpRefused.Store( 0 );
    g_rpDocumentAccepted.Store( 0 );
}

CORE::CJobDefinition
MakeRpJob( const CORE::CString& id, UInt32 steps, bool gated, const CORE::CString& document )
{
    CORE::CJobDefinition job;
    job.SetId( id );
    job.SetTaskTypeName( CRpTestTaskData::TaskType );
    job.SetTrigger( CORE::CJobTrigger::Manual() );
    CORE::CDataNode payload;
    payload.SetNodeType( GUCEF_DATATYPE_OBJECT );
    payload.SetAttribute( "steps", steps );
    payload.SetAttribute( "gated", gated );
    payload.SetAttribute( "document", document );
    job.SetPayload( payload );
    return job;
}

UInt64
RunRpNow( CORE::CJobScheduler& scheduler, const CORE::CString& jobId )
{
    CORE::SJobControlCommand command;
    command.action = CORE::JOBCONTROLACTION_RUN_NOW;
    command.targetKind = CORE::JOBCONTROLTARGET_JOB;
    command.targetId = jobId;
    command.requestedBy = "tester";
    CORE::SJobControlResult result;
    if ( CORE::JOBCONTROLSTATUS_OK != scheduler.Control( command, result ) )
        return 0;
    return result.runId;
}

/** Waits (bounded) until the run's record has at least 'done' progress; the record as it is then */
bool
WaitForProgress( CORE::CJobScheduler& scheduler, UInt64 runId, UInt64 done, CORE::SJobRunRecord& run )
{
    for ( UInt32 waited=0; waited<10000; ++waited )
    {
        if ( scheduler.GetRun( runId, run ) && run.progressDone >= done )
            return true;
        MT::ThreadDelay( 1 );
    }
    return false;
}

/** Waits (bounded) until the run ended */
bool
WaitForRunEnd( CORE::CJobScheduler& scheduler, UInt64 runId, CORE::SJobRunRecord& run )
{
    for ( UInt32 waited=0; waited<10000; ++waited )
    {
        if ( scheduler.GetRun( runId, run ) && run.endedUtcMs > 0 )
            return true;
        MT::ThreadDelay( 1 );
    }
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformJobRunProgressTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "JobRunProgress" );
    RegisterRpTaskType();

    GUCEF_TESTFW_TESTCASE( "JobRunProgress: a run reports its progress into its record while it goes; the change event is coalesced; the result document and the last note stay" )
    try
    {
        ResetRpCounters();
        CORE::CJobSchedulerSettings settings = MakeJsSettings( "rp-coalesced" );
        settings.SetProgressEventMinIntervalMs( 600000 );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( settings, GUCEF_NULL );
        ASSERT_TRUE( !scheduler.IsNULL() );
        if ( !scheduler.IsNULL() )
        {
            CORE::CString error;
            ASSERT_TRUE( scheduler->AddJob( MakeRpJob( "progress", 4, true, "results/run.json" ), error ) );
            ASSERT_TRUE( scheduler->Start( error ) );
            UInt64 runId = RunRpNow( *scheduler, "progress" );
            ASSERT_TRUE( 0 != runId );

            CORE::SJobRunRecord run;
            ASSERT_TRUE( WaitForProgress( *scheduler, runId, 1, run ) );
            ASSERT_TRUE( 1 == run.progressDone && 4 == run.progressTotal && "step 1" == run.note && 0 == run.endedUtcMs );
            const UInt64 firstRevision = run.revision;

            // within the interval: the record has the latest values, without a new revision (no change event)
            for ( UInt32 step=2; step<=3; ++step )
            {
                g_rpReleased.Store( step );
                ASSERT_TRUE( WaitForProgress( *scheduler, runId, step, run ) );
                ASSERT_TRUE( step == run.progressDone && "step " + CORE::ToString( step ) == run.note );
                ASSERT_TRUE( firstRevision == run.revision );
            }

            g_rpReleased.Store( 4 );
            ASSERT_TRUE( WaitForRunEnd( *scheduler, runId, run ) );
            ASSERT_TRUE( CORE::JOBRUNSTATE_SUCCEEDED == run.state );
            ASSERT_TRUE( 4 == run.progressDone && 4 == run.progressTotal );
            ASSERT_TRUE( "step 4" == run.note );             // the task ended without status extra info: the last note stays
            ASSERT_TRUE( "results/run.json" == run.resultDocument );
            ASSERT_TRUE( run.revision > firstRevision );
            ASSERT_TRUE( 4 == g_rpAccepted.Load() && 0 == g_rpRefused.Load() && 1 == g_rpDocumentAccepted.Load() );
            DropJsScheduler( scheduler );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JobRunProgress: without a minimum interval every report is a change of the run" )
    try
    {
        ResetRpCounters();
        CORE::CJobSchedulerSettings settings = MakeJsSettings( "rp-every" );
        settings.SetProgressEventMinIntervalMs( 0 );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( settings, GUCEF_NULL );
        ASSERT_TRUE( !scheduler.IsNULL() );
        if ( !scheduler.IsNULL() )
        {
            CORE::CString error;
            ASSERT_TRUE( scheduler->AddJob( MakeRpJob( "every", 3, true, CORE::CString::Empty ), error ) );
            ASSERT_TRUE( scheduler->Start( error ) );
            UInt64 runId = RunRpNow( *scheduler, "every" );
            CORE::SJobRunRecord run;
            ASSERT_TRUE( WaitForProgress( *scheduler, runId, 1, run ) );
            UInt64 revision = run.revision;
            g_rpReleased.Store( 2 );
            ASSERT_TRUE( WaitForProgress( *scheduler, runId, 2, run ) );
            ASSERT_TRUE( run.revision > revision );
            g_rpReleased.Store( 3 );
            ASSERT_TRUE( WaitForRunEnd( *scheduler, runId, run ) );
            ASSERT_TRUE( CORE::JOBRUNSTATE_SUCCEEDED == run.state && 3 == run.progressDone && run.resultDocument.IsNULLOrEmpty() );
            DropJsScheduler( scheduler );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JobRunProgress: a task that carries out no scheduled run cannot report run progress" )
    try
    {
        ResetRpCounters();
        CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "test.rp.plain", true );
        ASSERT_TRUE( !pool.IsNULL() );
        if ( !pool.IsNULL() )
        {
            CRpTestTaskData data;
            data.steps = 2;
            data.document = "nowhere.json";
            CORE::CFutureResult future = pool->QueueTask( CRpTestTaskData::TaskType, &data );
            ASSERT_TRUE( future.HasAFuture() );
            CORE::CTaskPtr task = future.GetResult( 10000 );
            ASSERT_TRUE( !task.IsNULL() );
            ASSERT_TRUE( 0 == g_rpAccepted.Load() && 2 == g_rpRefused.Load() && 0 == g_rpDocumentAccepted.Load() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "JobRunProgress: the progress and the result document are kept with the persisted run history" )
    try
    {
        ResetRpCounters();
        CMemoryJobStateStore store;
        CORE::CJobSchedulerSettings settings = MakeJsSettings( "rp-persist" );
        settings.SetPersistRunHistory( 10 );
        CORE::JobSchedulerPtr scheduler = MakeJsScheduler( settings, GUCEF_NULL );
        ASSERT_TRUE( !scheduler.IsNULL() );
        if ( !scheduler.IsNULL() )
        {
            scheduler->SetStateStore( &store );
            CORE::CString error;
            ASSERT_TRUE( scheduler->AddJob( MakeRpJob( "persisted", 2, false, "vfs://results/persisted.json" ), error ) );
            ASSERT_TRUE( scheduler->Start( error ) );
            UInt64 runId = RunRpNow( *scheduler, "persisted" );
            CORE::SJobRunRecord run;
            ASSERT_TRUE( WaitForRunEnd( *scheduler, runId, run ) );
            scheduler->Stop();
            CORE::CDataNode state;
            ASSERT_TRUE( store.GetState( state ) );
            bool found = false;
            const CORE::CDataNode* runs = state.FindChild( "runs" );
            if ( GUCEF_NULL != runs )
            {
                for ( CORE::CDataNode::const_iterator r = runs->ConstBegin(); r != runs->ConstEnd(); ++r )
                {
                    if ( "vfs://results/persisted.json" == (*r)->GetAttributeValueOrChildValueByName( "resultDocument" ).AsString() &&
                         2 == (*r)->GetAttributeValueOrChildValueByName( "progressDone" ).AsUInt64() &&
                         2 == (*r)->GetAttributeValueOrChildValueByName( "progressTotal" ).AsUInt64() )
                        found = true;
                }
            }
            ASSERT_TRUE( found );
            DropJsScheduler( scheduler );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    UnregisterRpTaskType();
}

/*-------------------------------------------------------------------------*/
