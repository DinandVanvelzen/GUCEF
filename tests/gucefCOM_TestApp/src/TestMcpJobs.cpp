/*
 *  gucefCOM_TestApp: GUCEF test application for the COM module
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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_MCP_RIG_H
#include "gucef_test_mcp_rig.h"
#define GUCEF_TEST_MCP_RIG_H
#endif /* GUCEF_TEST_MCP_RIG_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H
#include "gucefCORE_CIDataNodeSerializableTaskData.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CJSONSCHEMAVALIDATOR_H
#include "gucefCORE_CJsonSchemaValidator.h"
#define GUCEF_CORE_CJSONSCHEMAVALIDATOR_H
#endif /* GUCEF_CORE_CJSONSCHEMAVALIDATOR_H ? */

#ifndef GUCEF_COM_CMCPJOBBRIDGE_H
#include "gucefCOM_CMcpJobBridge.h"
#define GUCEF_COM_CMCPJOBBRIDGE_H
#endif /* GUCEF_COM_CMCPJOBBRIDGE_H ? */

#ifndef GUCEF_COM_CMCPSCHEMABUILDER_H
#include "gucefCOM_CMcpSchemaBuilder.h"
#define GUCEF_COM_CMCPSCHEMABUILDER_H
#endif /* GUCEF_COM_CMCPSCHEMABUILDER_H ? */

#include "TestMcpSupport.h"
#include "TestMcpJobs.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The steps a gated test job may take: before step n it waits until this is at least n */
MT::CTAtomic< UInt32 > g_mjReleased( 0 );

/** The task data of the job of the tests: how many steps, whether they are gated, the result document it names */
class CMjTaskData : public CORE::CIDataNodeSerializableTaskData
{
    public:

    static const CORE::CString TaskType;

    UInt32 steps;
    bool gated;
    CORE::CString document;

    CMjTaskData( void ) : CORE::CIDataNodeSerializableTaskData(), steps( 3 ), gated( false ), document() {}
    CMjTaskData( const CMjTaskData& src ) : CORE::CIDataNodeSerializableTaskData( src ), steps( src.steps ), gated( src.gated ), document( src.document ) {}
    virtual ~CMjTaskData() GUCEF_VIRTUAL_OVERRIDE {}

    virtual const CORE::CString& GetTaskTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return TaskType; }
    virtual const CORE::CString& GetThreadPoolName( void ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::CString::Empty; }
    virtual bool GetOnlyUseExistingThreadPool( void ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool GetTaskCanBeQueued( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual UInt32 GetTaskId( void ) const GUCEF_VIRTUAL_OVERRIDE { return 0; }
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CMjTaskData( *this ); }
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { static const CORE::CString name = "COMTEST::CMjTaskData"; return name; }
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return CloneableTypeName(); }
    static const char* CloneableTypeName( void ) { return "COMTEST::CMjTaskData"; }

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

const CORE::CString CMjTaskData::TaskType = "COMTEST::McpJobTask";

/*-------------------------------------------------------------------------*/

/** One step per cycle, with its progress; stops when asked to cancel */
class CMjTaskConsumer : public CORE::CTaskConsumer
{
    public:

    CMjTaskConsumer( void ) : CORE::CTaskConsumer(), m_step( 0 ) {}
    virtual ~CMjTaskConsumer() GUCEF_VIRTUAL_OVERRIDE {}

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return CMjTaskData::TaskType; }

    /** A pool may hand the same consumer the next task: every task starts at its first step */
    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_step = 0;
        return CORE::CTaskConsumer::OnTaskStart( task );
    }

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        const CMjTaskData* data = task.IsNULL() ? GUCEF_NULL : MT::CICloneable::CloneableAs< CMjTaskData >( task->GetTaskData() );
        UInt32 steps = GUCEF_NULL != data ? data->steps : 3;
        bool gated = GUCEF_NULL != data && data->gated;

        // asked to cancel: not done. Returning true would tell the delegator the work is finished, which ends the task
        // FINISHED (the run SUCCEEDED); returning false lets the delegator, which the cancellation also asks to stop the
        // task, end it CANCELLED
        const UInt32 next = m_step + 1;
        for ( UInt32 waited=0; gated && waited<20000 && g_mjReleased.Load() < next; ++waited )
        {
            if ( task->IsCancellationRequested() )
                return false;
            MT::ThreadDelay( 1 );
        }
        if ( task->IsCancellationRequested() )
            return false;
        m_step = next;
        MT::ThreadDelay( 2 );
        ReportRunProgress( m_step, steps, "step " + CORE::ToString( m_step ) );
        if ( m_step < steps )
            return false;
        if ( GUCEF_NULL != data && !data->document.IsNULLOrEmpty() )
            ReportRunResultDocument( data->document );
        return true;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
    }

    private:

    UInt32 m_step;
};

typedef CORE::CTFactory< CORE::CTaskConsumer, CMjTaskConsumer, MT::CMutex > TMjTaskConsumerFactory;
typedef CORE::CTFactory< CORE::CIDataNodeSerializableTaskData, CMjTaskData, MT::CMutex > TMjTaskDataFactory;
TMjTaskConsumerFactory g_mjTaskConsumerFactory;
TMjTaskDataFactory g_mjTaskDataFactory;

/*-------------------------------------------------------------------------*/

/** A UTC clock the test sets */
class CMjUtcClock : public CORE::CIUtcClock
{
    public:

    Int64 now;

    CMjUtcClock( void ) : CORE::CIUtcClock(), now( 1798761600000 ) {}
    virtual Int64 GetUtcTimeInMs( void ) const GUCEF_VIRTUAL_OVERRIDE { return now; }
};

/*-------------------------------------------------------------------------*/

CORE::CJobDefinition
MakeMjDefinition( UInt32 steps, bool gated, const CORE::CString& document = CORE::CString::Empty )
{
    CORE::CJobDefinition definition;
    definition.SetTaskTypeName( CMjTaskData::TaskType );
    CORE::CDataNode payload;
    payload.SetNodeType( GUCEF_DATATYPE_OBJECT );
    payload.SetAttribute( "steps", steps );
    payload.SetAttribute( "gated", gated );
    payload.SetAttribute( "document", document );
    definition.SetPayload( payload );
    return definition;
}

/*-------------------------------------------------------------------------*/

/** The tools of the tests: start a test job, ask for its status, cancel it. Marked read-only for the default policy */
class CMjTool : public COM::CIMcpTool
{
    public:

    enum EAction { START, STATUS, CANCEL };

    CMjTool( COM::CMcpJobBridge& bridge, EAction action ) : COM::CIMcpTool(), m_bridge( bridge ), m_action( action ) {}

    virtual bool GetDescriptor( COM::CMcpToolDescriptor& descriptor ) const GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CDataNode input = COM::CMcpSchemaBuilder::Object();
        COM::CMcpSchemaBuilder::AddProperty( input, "waitMs", COM::CMcpSchemaBuilder::Integer( 0, 60000 ) );
        switch ( m_action )
        {
            case START:
            {
                descriptor.name = "test_start_job";
                COM::CMcpSchemaBuilder::AddProperty( input, "steps", COM::CMcpSchemaBuilder::Integer( 1, 1000 ), true );
                COM::CMcpSchemaBuilder::AddProperty( input, "gated", COM::CMcpSchemaBuilder::Boolean() );
                COM::CMcpSchemaBuilder::AddProperty( input, "document", COM::CMcpSchemaBuilder::String( 0, 256 ) );
                COM::CMcpSchemaBuilder::AddProperty( input, "requestKey", COM::CMcpSchemaBuilder::String( 1, 64 ) );
                break;
            }
            case STATUS:
            {
                descriptor.name = "test_job_status";
                COM::CMcpSchemaBuilder::AddProperty( input, "jobId", COM::CMcpSchemaBuilder::String( 1, 64 ), true );
                break;
            }
            case CANCEL:
            default:
            {
                descriptor.name = "test_job_cancel";
                COM::CMcpSchemaBuilder::AddProperty( input, "jobId", COM::CMcpSchemaBuilder::String( 1, 64 ), true );
                break;
            }
        }
        descriptor.description = "A job tool of the tests";
        descriptor.effect = COM::MCP_TOOLEFFECT_READ_ONLY;
        descriptor.inputSchema = input;
        return true;
    }

    virtual COM::EMcpExecutionMode GetExecutionMode( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return COM::MCP_EXECUTION_CALLING_THREAD;
    }

    virtual COM::EMcpExecuteResult Execute( COM::CMcpRequestContext& context      ,
                                            const CORE::CDataNode& arguments      ,
                                            COM::CMcpToolResult& result           ,
                                            COM::CMcpInputRequired& inputRequired ) GUCEF_VIRTUAL_OVERRIDE
    {
        UInt32 waitMs = arguments.GetAttributeValueOrChildValueByName( "waitMs" ).AsUInt32( m_bridge.GetSettings().defaultWaitMs, true );
        CORE::CString jobId = arguments.GetAttributeValueOrChildValueByName( "jobId" ).AsString( CORE::CString::Empty, true );
        if ( START == m_action )
        {
            UInt32 steps = arguments.GetAttributeValueOrChildValueByName( "steps" ).AsUInt32( 1, true );
            bool gated = arguments.GetAttributeValueOrChildValueByName( "gated" ).AsBool( false, true );
            CORE::CString document = arguments.GetAttributeValueOrChildValueByName( "document" ).AsString( CORE::CString::Empty, true );
            CORE::CString requestKey = arguments.GetAttributeValueOrChildValueByName( "requestKey" ).AsString( CORE::CString::Empty, true );
            CORE::CString digest = CORE::ToString( steps ) + "|" + CORE::ToString( gated ) + "|" + document;
            COM::CMcpJobInfo job;
            CORE::CString error;
            switch ( m_bridge.StartJob( context.GetPrincipal(), "test_job", MakeMjDefinition( steps, gated, document ), requestKey, digest, job, error ) )
            {
                case COM::CMcpJobBridge::JOBSTART_STARTED:
                case COM::CMcpJobBridge::JOBSTART_REPEATED:
                {
                    jobId = job.jobId;
                    break;
                }
                case COM::CMcpJobBridge::JOBSTART_KEY_CONFLICT:
                {
                    COM::CMcpJobBridge::BuildErrorResult( "request-key-conflict", error, false, result );
                    return COM::MCP_EXECUTE_COMPLETE;
                }
                case COM::CMcpJobBridge::JOBSTART_BUSY:
                {
                    COM::CMcpJobBridge::BuildErrorResult( "busy", error, true, result );
                    return COM::MCP_EXECUTE_COMPLETE;
                }
                default:
                {
                    COM::CMcpJobBridge::BuildErrorResult( "refused", error, false, result );
                    return COM::MCP_EXECUTE_COMPLETE;
                }
            }
        }
        else
        if ( CANCEL == m_action )
        {
            COM::CMcpJobInfo job;
            CORE::CString error;
            if ( COM::CMcpJobBridge::JOB_FOUND != m_bridge.CancelJob( context.GetPrincipal(), jobId, job, error ) )
            {
                COM::CMcpJobBridge::BuildErrorResult( "job-unknown", "no such job", false, result );
                return COM::MCP_EXECUTE_COMPLETE;
            }
            waitMs = 0;
        }
        return m_bridge.Wait( context, jobId, waitMs, result );
    }

    private:

    COM::CMcpJobBridge& m_bridge;
    EAction m_action;
};

/*-------------------------------------------------------------------------*/

/** A scheduler of the tests with the job task type submittable, the bridge on it, and the tools in a rig */
class CMjFixture
{
    public:

    TESTFW::CTestMcpRig rig;
    CORE::JobSchedulerPtr scheduler;
    COM::CMcpJobBridge* bridge;
    bool ok;

    CMjFixture( const char* hostName, const CORE::CString::StringVector& scopes = CORE::CString::StringVector() )
        : rig( "local:tester", scopes )
        , scheduler()
        , bridge( GUCEF_NULL )
        , ok( false )
    {
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        CORE::CJobSchedulerSettings settings;
        settings.SetHostName( hostName );
        settings.SetDispatcherPoolName( "test.mcpjobs.dispatch" );
        settings.SetRunPoolName( CORE::CString( "test.mcpjobs.run." ) + hostName );
        settings.SetMainThreadPoolName( CORE::CString( "test.mcpjobs.main." ) + hostName );
        settings.SetStopTimeoutMs( 5000 );
        settings.SetProgressEventMinIntervalMs( 0 );
        scheduler = taskManager.GetOrCreateJobScheduler( settings, rig.GetPulseGenerator() );
        if ( scheduler.IsNULL() )
            return;
        CORE::CString error;
        scheduler->RegisterSubmittableTaskType( CMjTaskData::TaskType );
        if ( !scheduler->Start( error ) )
            return;
        bridge = GUCEF_NEW COM::CMcpJobBridge( scheduler );
        COM::CMcpJobBridgeSettings bridgeSettings;
        bridgeSettings.pollIntervalMs = 10;
        bridge->SetSettings( bridgeSettings );
        COM::CIMcpToolPtr start( GUCEF_NEW CMjTool( *bridge, CMjTool::START ) );
        COM::CIMcpToolPtr status( GUCEF_NEW CMjTool( *bridge, CMjTool::STATUS ) );
        COM::CIMcpToolPtr cancel( GUCEF_NEW CMjTool( *bridge, CMjTool::CANCEL ) );
        ok = rig.GetServer().GetToolRegistry().Register( start, error ) &&
             rig.GetServer().GetToolRegistry().Register( status, error ) &&
             rig.GetServer().GetToolRegistry().Register( cancel, error );
    }

    ~CMjFixture()
    {
        g_mjReleased.Store( 100000 );
        rig.GetServer().GetToolRegistry().UnregisterAll();
        GUCEF_DELETE bridge;
        bridge = GUCEF_NULL;
        if ( !scheduler.IsNULL() )
        {
            scheduler->Stop();
            CORE::CCoreGlobal::Instance()->GetTaskManager().UnregisterJobScheduler( scheduler->GetName() );
            scheduler.Unlink();
        }
    }

    /** Waits (pumping the rig) until the job is in the state */
    bool WaitForState( const COM::CMcpPrincipal& principal, const CORE::CString& jobId, const CORE::CString& state, UInt32 timeoutMs = 10000 )
    {
        for ( UInt32 waited=0; waited<timeoutMs; ++waited )
        {
            COM::CMcpJobInfo job;
            if ( COM::CMcpJobBridge::JOB_FOUND == bridge->GetJob( principal, jobId, job ) && job.state == state )
                return true;
            rig.Pump();
        }
        return false;
    }
};

COM::CMcpPrincipal
Tester( void )
{
    return COM::CMcpPrincipal( "test", "local:tester", true );
}

CORE::CString
Attr( const CORE::CDataNode* node, const char* name )
{
    if ( GUCEF_NULL == node )
        return "<none>";
    const CORE::CDataNode::TKeyValuePair* attribute = node->GetAttribute( name );
    return GUCEF_NULL != attribute ? (*attribute).second.AsString() : CORE::CString( "<none>" );
}

CORE::CString
JobIdOf( const CORE::CDataNode& response )
{
    return Attr( TESTFW::CTestMcpRig::GetStructuredContent( response ), "jobId" );
}

CORE::CString
StateOf( const CORE::CDataNode& response )
{
    return Attr( TESTFW::CTestMcpRig::GetStructuredContent( response ), "state" );
}

CORE::CString
ErrorCodeOf( const CORE::CDataNode& response )
{
    const CORE::CDataNode* content = TESTFW::CTestMcpRig::GetStructuredContent( response );
    return Attr( GUCEF_NULL != content ? content->FindChild( "error" ) : GUCEF_NULL, "code" );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMcpJobBridgeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "McpJobBridge" );

    if ( !McpTestJsonAvailable() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MCP JOB BRIDGE TESTS skipped: the json codec is not available" );
        return;
    }

    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    taskManager.RegisterTaskConsumerFactory( CMjTaskData::TaskType, &g_mjTaskConsumerFactory );
    taskManager.RegisterTaskDataFactory( CMjTaskData::TaskType, &g_mjTaskDataFactory );

    GUCEF_TESTFW_TESTCASE( "McpJobBridge: a job that ends within waitMs answers with its final state, with progress notifications on the way" )
    try
    {
        g_mjReleased.Store( 100000 );
        CMjFixture fixture( "mcpjobs-direct" );
        ASSERT_TRUE( fixture.ok );
        if ( fixture.ok )
        {
            CORE::CDataNode response;
            ASSERT_TRUE( fixture.rig.CallTool( "1", "test_start_job", "{\"steps\":5,\"waitMs\":20000,\"document\":\"results/direct.json\"}", response, 20000, "tok-1" ) );
            ASSERT_FALSE( TESTFW::CTestMcpRig::IsToolError( response ) );
            ASSERT_TRUE( "completed" == StateOf( response ) );
            ASSERT_TRUE( 22 == JobIdOf( response ).Length() );
            const CORE::CDataNode* content = TESTFW::CTestMcpRig::GetStructuredContent( response );
            ASSERT_TRUE( GUCEF_NULL != content && "5" == Attr( content->FindChild( "progress" ), "done" ) && "5" == Attr( content->FindChild( "progress" ), "total" ) );

            // the job object validates against its schema
            CORE::CJsonSchema schema;
            CORE::CString error;
            ASSERT_TRUE( schema.Compile( COM::CMcpJobInfo::GetSchema(), error ) );
            CORE::CJsonSchemaValidator::TErrorVector errors;
            ASSERT_TRUE( GUCEF_NULL != content && CORE::CJsonSchemaValidator::Validate( schema, *content, errors ) );

            // progress came as notifications, increasing, with the token of the request
            TESTFW::CTestMcpChannel::TMessageVector progress;
            fixture.rig.GetNotifications( "notifications/progress", progress );
            ASSERT_TRUE( !progress.empty() );
            Float64 last = 0.0;
            bool increasing = true;
            for ( size_t i=0; i<progress.size(); ++i )
            {
                const CORE::CDataNode* params = progress[ i ].FindChild( "params" );
                Float64 value = GUCEF_NULL != params ? params->GetAttributeValueOrChildValueByName( "progress" ).AsFloat64( 0.0 ) : 0.0;
                if ( value <= last || "tok-1" != Attr( params, "progressToken" ) )
                    increasing = false;
                last = value;
            }
            ASSERT_TRUE( increasing );

            // the result document reached the run record behind the handle
            COM::CMcpJobInfo job;
            ASSERT_TRUE( COM::CMcpJobBridge::JOB_FOUND == fixture.bridge->GetJob( Tester(), JobIdOf( response ), job ) );
            ASSERT_TRUE( "results/direct.json" == job.run.resultDocument && job.expiresUtcMs > 0 );
            ASSERT_TRUE( 0 == fixture.bridge->GetWaitingCount() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "McpJobBridge: a job longer than waitMs answers with the working job; its status with a wait gives the final state; a call made in process waits too" )
    try
    {
        g_mjReleased.Store( 0 );
        CMjFixture fixture( "mcpjobs-long" );
        ASSERT_TRUE( fixture.ok );
        if ( fixture.ok )
        {
            CORE::CDataNode response;
            ASSERT_TRUE( fixture.rig.CallTool( "1", "test_start_job", "{\"steps\":3,\"gated\":true,\"waitMs\":100}", response, 20000 ) );
            CORE::CString jobId = JobIdOf( response );
            ASSERT_TRUE( "working" == StateOf( response ) || "queued" == StateOf( response ) );
            ASSERT_TRUE( !TESTFW::CTestMcpRig::IsToolError( response ) );

            // waitMs 0: the job as it is, at once
            CORE::CDataNode now;
            ASSERT_TRUE( fixture.rig.CallTool( "2", "test_job_status", "{\"jobId\":\"" + jobId + "\",\"waitMs\":0}", now, 5000 ) );
            ASSERT_TRUE( jobId == JobIdOf( now ) && !COM::CMcpJobInfo::IsFinalState( StateOf( now ) ) );

            // a wait that ends with the job
            fixture.rig.Send( TESTFW::CTestMcpRig::MakeRequest( "3", "tools/call", "\"name\":\"test_job_status\",\"arguments\":{\"jobId\":\"" + jobId + "\",\"waitMs\":20000}" ) );
            fixture.rig.Pump( 20 );
            ASSERT_TRUE( 1 == fixture.bridge->GetWaitingCount() );
            g_mjReleased.Store( 3 );
            CORE::CDataNode final;
            ASSERT_TRUE( fixture.rig.WaitForResponse( "3", final, 20000 ) );
            ASSERT_TRUE( "completed" == StateOf( final ) );

            // in process: CallTool() drives the wait on a pulse generator of the calling thread
            g_mjReleased.Store( 0 );
            CORE::CDataNode startArgs( GUCEF_DATATYPE_OBJECT );
            startArgs.SetAttribute( "steps", (UInt32) 2 );
            startArgs.SetAttribute( "gated", true );
            startArgs.SetAttribute( "waitMs", (UInt32) 0 );
            COM::CMcpToolResult started;
            CORE::CString error;
            ASSERT_TRUE( fixture.rig.GetServer().CallTool( "test_start_job", startArgs, Tester(), started, error ) );
            CORE::CString inProcessJob = Attr( &started.structuredContent, "jobId" );
            g_mjReleased.Store( 2 );
            CORE::CDataNode statusArgs( GUCEF_DATATYPE_OBJECT );
            statusArgs.SetAttribute( "jobId", inProcessJob );
            statusArgs.SetAttribute( "waitMs", (UInt32) 20000 );
            COM::CMcpToolResult waited;
            ASSERT_TRUE( fixture.rig.GetServer().CallTool( "test_job_status", statusArgs, Tester(), waited, error ) );
            ASSERT_TRUE( "completed" == Attr( &waited.structuredContent, "state" ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "McpJobBridge: cancelling the call detaches it and the job goes on; the cancel tool stops a job" )
    try
    {
        g_mjReleased.Store( 0 );
        CMjFixture fixture( "mcpjobs-cancel" );
        ASSERT_TRUE( fixture.ok );
        if ( fixture.ok )
        {
            CORE::CDataNode response;
            ASSERT_TRUE( fixture.rig.CallTool( "1", "test_start_job", "{\"steps\":2,\"gated\":true,\"waitMs\":0}", response, 5000 ) );
            CORE::CString jobId = JobIdOf( response );
            fixture.rig.Send( TESTFW::CTestMcpRig::MakeRequest( "2", "tools/call", "\"name\":\"test_job_status\",\"arguments\":{\"jobId\":\"" + jobId + "\",\"waitMs\":30000}" ) );
            fixture.rig.Pump( 20 );
            ASSERT_TRUE( 1 == fixture.bridge->GetWaitingCount() );
            fixture.rig.Cancel( "2" );
            for ( UInt32 i=0; i<2000 && fixture.bridge->GetWaitingCount() > 0; ++i )
                fixture.rig.Pump();
            ASSERT_TRUE( 0 == fixture.bridge->GetWaitingCount() );
            COM::CMcpJobInfo job;
            ASSERT_TRUE( COM::CMcpJobBridge::JOB_FOUND == fixture.bridge->GetJob( Tester(), jobId, job ) && !job.IsFinal() );
            g_mjReleased.Store( 2 );
            ASSERT_TRUE( fixture.WaitForState( Tester(), jobId, "completed" ) );

            // the cancel tool
            g_mjReleased.Store( 0 );
            ASSERT_TRUE( fixture.rig.CallTool( "3", "test_start_job", "{\"steps\":4,\"gated\":true,\"waitMs\":0}", response, 5000 ) );
            CORE::CString cancelled = JobIdOf( response );

            // the task runs (held at its first step) when the cancel comes: the task stops cooperatively and the run
            // ends CANCELLED, not FINISHED
            ASSERT_TRUE( fixture.WaitForState( Tester(), cancelled, "working" ) );
            CORE::CDataNode cancelResponse;
            ASSERT_TRUE( fixture.rig.CallTool( "4", "test_job_cancel", "{\"jobId\":\"" + cancelled + "\"}", cancelResponse, 5000 ) );
            ASSERT_FALSE( TESTFW::CTestMcpRig::IsToolError( cancelResponse ) );
            ASSERT_TRUE( fixture.WaitForState( Tester(), cancelled, "cancelled" ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "McpJobBridge: a handle is the starting principal's: another gets job-unknown, an operator sees it" )
    try
    {
        g_mjReleased.Store( 100000 );
        CMjFixture fixture( "mcpjobs-binding" );
        ASSERT_TRUE( fixture.ok );
        if ( fixture.ok )
        {
            CORE::CDataNode response;
            ASSERT_TRUE( fixture.rig.CallTool( "1", "test_start_job", "{\"steps\":1,\"waitMs\":20000}", response, 20000 ) );
            CORE::CString jobId = JobIdOf( response );
            COM::CMcpJobInfo job;
            COM::CMcpPrincipal other( "test", "local:other", true );
            ASSERT_TRUE( COM::CMcpJobBridge::JOB_UNKNOWN == fixture.bridge->GetJob( other, jobId, job ) );
            CORE::CString error;
            ASSERT_TRUE( COM::CMcpJobBridge::JOB_UNKNOWN == fixture.bridge->CancelJob( other, jobId, job, error ) );
            COM::TMcpJobInfoVector seen;
            fixture.bridge->ListJobs( other, seen );
            ASSERT_TRUE( seen.empty() );
            COM::CMcpPrincipal operatorPrincipal( "test", "local:operator", true );
            operatorPrincipal.scopes.push_back( "jobs:all" );
            ASSERT_TRUE( COM::CMcpJobBridge::JOB_FOUND == fixture.bridge->GetJob( operatorPrincipal, jobId, job ) );
            fixture.bridge->ListJobs( operatorPrincipal, seen );
            ASSERT_TRUE( 1 == seen.size() );
            fixture.bridge->ListJobs( Tester(), seen, "test_job", "completed" );
            ASSERT_TRUE( 1 == seen.size() && jobId == seen[ 0 ].jobId );
            fixture.bridge->ListJobs( Tester(), seen, "other_kind" );
            ASSERT_TRUE( seen.empty() );

            // an unknown handle through the tool: a tool execution error with the code
            CORE::CDataNode unknown;
            ASSERT_TRUE( fixture.rig.CallTool( "2", "test_job_status", "{\"jobId\":\"AAAAAAAAAAAAAAAAAAAAAA\",\"waitMs\":0}", unknown, 5000 ) );
            ASSERT_TRUE( TESTFW::CTestMcpRig::IsToolError( unknown ) && "job-unknown" == ErrorCodeOf( unknown ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "McpJobBridge: a request key repeated with the same arguments gives the first job, with other arguments a conflict; too many jobs going is busy" )
    try
    {
        g_mjReleased.Store( 0 );
        CMjFixture fixture( "mcpjobs-keys" );
        ASSERT_TRUE( fixture.ok );
        if ( fixture.ok )
        {
            COM::CMcpJobBridgeSettings settings = fixture.bridge->GetSettings();
            settings.maxActivePerPrincipal = 2;
            ASSERT_TRUE( fixture.bridge->SetSettings( settings ) );

            CORE::CDataNode first;
            ASSERT_TRUE( fixture.rig.CallTool( "1", "test_start_job", "{\"steps\":2,\"gated\":true,\"waitMs\":0,\"requestKey\":\"k-1\"}", first, 5000 ) );
            CORE::CDataNode repeat;
            ASSERT_TRUE( fixture.rig.CallTool( "2", "test_start_job", "{\"steps\":2,\"gated\":true,\"waitMs\":0,\"requestKey\":\"k-1\"}", repeat, 5000 ) );
            ASSERT_TRUE( JobIdOf( first ) == JobIdOf( repeat ) && 1 == fixture.bridge->GetJobCount() );
            CORE::CDataNode conflict;
            ASSERT_TRUE( fixture.rig.CallTool( "3", "test_start_job", "{\"steps\":3,\"gated\":true,\"waitMs\":0,\"requestKey\":\"k-1\"}", conflict, 5000 ) );
            ASSERT_TRUE( TESTFW::CTestMcpRig::IsToolError( conflict ) && "request-key-conflict" == ErrorCodeOf( conflict ) );

            // a request key of another principal is another key
            COM::CMcpJobInfo otherJob;
            CORE::CString error;
            ASSERT_TRUE( COM::CMcpJobBridge::JOBSTART_STARTED == fixture.bridge->StartJob( COM::CMcpPrincipal( "test", "local:other", true ), "test_job",
                                                                                          MakeMjDefinition( 2, true ), "k-1", "2|1|", otherJob, error ) );
            ASSERT_TRUE( otherJob.jobId != JobIdOf( first ) );

            CORE::CDataNode second;
            ASSERT_TRUE( fixture.rig.CallTool( "4", "test_start_job", "{\"steps\":2,\"gated\":true,\"waitMs\":0}", second, 5000 ) );
            ASSERT_FALSE( TESTFW::CTestMcpRig::IsToolError( second ) );
            CORE::CDataNode busy;
            ASSERT_TRUE( fixture.rig.CallTool( "5", "test_start_job", "{\"steps\":2,\"gated\":true,\"waitMs\":0}", busy, 5000 ) );
            ASSERT_TRUE( TESTFW::CTestMcpRig::IsToolError( busy ) && "busy" == ErrorCodeOf( busy ) );

            g_mjReleased.Store( 100000 );
            ASSERT_TRUE( fixture.WaitForState( Tester(), JobIdOf( first ), "completed" ) );
            ASSERT_TRUE( fixture.WaitForState( Tester(), JobIdOf( second ), "completed" ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "McpJobBridge: a finished job's handle expires after resultTtlMs; handles survive a restart, a run of an earlier process is interrupted" )
    try
    {
        g_mjReleased.Store( 100000 );
        CMjFixture fixture( "mcpjobs-expiry" );
        ASSERT_TRUE( fixture.ok );
        if ( fixture.ok )
        {
            CMjUtcClock clock;
            fixture.bridge->SetClocks( &clock, GUCEF_NULL );
            COM::CMcpJobBridgeSettings settings = fixture.bridge->GetSettings();
            settings.resultTtlMs = 60000;
            ASSERT_TRUE( fixture.bridge->SetSettings( settings ) );

            CORE::CDataNode response;
            ASSERT_TRUE( fixture.rig.CallTool( "1", "test_start_job", "{\"steps\":1,\"waitMs\":20000}", response, 20000 ) );
            CORE::CString jobId = JobIdOf( response );
            ASSERT_TRUE( "completed" == StateOf( response ) );

            // the state, and a made-up job of an earlier process that did not end
            CORE::CDataNode state;
            ASSERT_TRUE( fixture.bridge->SaveState( state ) );
            CORE::CDataNode* jobs = state.FindChild( "jobs" );
            ASSERT_TRUE( GUCEF_NULL != jobs && 1 == McpTestChildCount( jobs ) );
            if ( GUCEF_NULL != jobs )
            {
                CORE::CDataNode* earlier = jobs->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                earlier->SetAttribute( "jobId", "BBBBBBBBBBBBBBBBBBBBBB" );
                earlier->SetAttribute( "kind", "test_job" );
                earlier->SetAttribute( "subject", "local:tester" );
                earlier->SetAttribute( "state", "working" );
                earlier->SetAttribute( "createdUtcMs", clock.now );
                earlier->SetAttribute( "hostInstanceId", fixture.scheduler->GetHostInstanceId() + 1 );
                earlier->SetAttribute( "runId", (UInt64) 7 );
            }

            CORE::CString path = CORE::RelativePath( "$CURWORKDIR$" );
            CORE::AppendToPath( path, "gucefCOM_TestApp_McpJobBridgeState.json" );
            CORE::DeleteFile( path );
            {
                COM::CMcpJobBridge restarted( fixture.scheduler );
                restarted.SetClocks( &clock, GUCEF_NULL );
                restarted.SetSettings( settings );
                ASSERT_TRUE( restarted.LoadState( state ) );
                COM::CMcpJobInfo job;
                ASSERT_TRUE( COM::CMcpJobBridge::JOB_FOUND == restarted.GetJob( Tester(), jobId, job ) && "completed" == job.state );
                ASSERT_TRUE( COM::CMcpJobBridge::JOB_FOUND == restarted.GetJob( Tester(), "BBBBBBBBBBBBBBBBBBBBBB", job ) && "interrupted" == job.state );
                CORE::CString error;
                ASSERT_TRUE( restarted.SaveStateToFile( path, error ) );
            }
            {
                COM::CMcpJobBridge reloaded( fixture.scheduler );
                reloaded.SetClocks( &clock, GUCEF_NULL );
                reloaded.SetSettings( settings );
                CORE::CString error;
                ASSERT_TRUE( reloaded.LoadStateFromFile( path, error ) );
                COM::CMcpJobInfo job;
                ASSERT_TRUE( COM::CMcpJobBridge::JOB_FOUND == reloaded.GetJob( Tester(), jobId, job ) && "completed" == job.state );
                ASSERT_TRUE( COM::CMcpJobBridge::JOB_FOUND == reloaded.GetJob( Tester(), "BBBBBBBBBBBBBBBBBBBBBB", job ) && "interrupted" == job.state );
            }
            CORE::DeleteFile( path );

            // expiry: within the time to live found, after it expired, for its principal only
            COM::CMcpJobInfo job;
            clock.now += 59000;
            ASSERT_TRUE( COM::CMcpJobBridge::JOB_FOUND == fixture.bridge->GetJob( Tester(), jobId, job ) );
            clock.now += 2000;
            ASSERT_TRUE( COM::CMcpJobBridge::JOB_EXPIRED == fixture.bridge->GetJob( Tester(), jobId, job ) );
            ASSERT_TRUE( COM::CMcpJobBridge::JOB_UNKNOWN == fixture.bridge->GetJob( COM::CMcpPrincipal( "test", "local:other", true ), jobId, job ) );
            CORE::CDataNode expired;
            ASSERT_TRUE( fixture.rig.CallTool( "2", "test_job_status", "{\"jobId\":\"" + jobId + "\",\"waitMs\":0}", expired, 5000 ) );
            ASSERT_TRUE( TESTFW::CTestMcpRig::IsToolError( expired ) && "job-expired" == ErrorCodeOf( expired ) );
            fixture.bridge->SetClocks( GUCEF_NULL, GUCEF_NULL );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    taskManager.UnregisterTaskConsumerFactory( CMjTaskData::TaskType );
    taskManager.UnregisterTaskDataFactory( CMjTaskData::TaskType );
}

/*-------------------------------------------------------------------------*/
