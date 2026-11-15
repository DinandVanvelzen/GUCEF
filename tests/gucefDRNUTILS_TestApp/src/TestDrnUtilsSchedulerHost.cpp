/*
 *  gucefDRNUTILS_TestApp: GUCEF test application for gucefDRNUTILS
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTCOMPATIBILITY_H
#include "gucefCOMCORE_CRpcContractCompatibility.h"
#define GUCEF_COMCORE_CRPCCONTRACTCOMPATIBILITY_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTCOMPATIBILITY_H ? */

#ifndef GUCEF_COMCORE_CRPCROLEACCESSPOLICY_H
#include "gucefCOMCORE_CRpcRoleAccessPolicy.h"
#define GUCEF_COMCORE_CRPCROLEACCESSPOLICY_H
#endif /* GUCEF_COMCORE_CRPCROLEACCESSPOLICY_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#include "gucefCOMCORE_CRpcContractRegistry.h"
#define GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H ? */

#ifndef GUCEF_DRNUTILS_H
#include "gucefDRNUTILS.h"
#define GUCEF_DRNUTILS_H
#endif /* GUCEF_DRNUTILS_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERHOST_H
#include "gucefDRNUTILS_CSchedulerHost.h"
#define GUCEF_DRNUTILS_CSCHEDULERHOST_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERHOST_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERHOST_H
#include "gucefDRNUTILS_CTaskManagerHost.h"
#define GUCEF_DRNUTILS_CTASKMANAGERHOST_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERHOST_H ? */

#ifndef GUCEF_DRNUTILS_CDRNUTILSACCESSRULES_H
#include "gucefDRNUTILS_CDrnUtilsAccessRules.h"
#define GUCEF_DRNUTILS_CDRNUTILSACCESSRULES_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSACCESSRULES_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERCLIENT_H
#include "gucefDRNUTILS_CSchedulerClient.h"
#define GUCEF_DRNUTILS_CSCHEDULERCLIENT_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERCLIENT_H ? */

#ifndef GUCEF_COMCORE_TESTAPP_TESTRPCSUPPORT_H
#include "TestRpcSupport.h"
#define GUCEF_COMCORE_TESTAPP_TESTRPCSUPPORT_H
#endif /* GUCEF_COMCORE_TESTAPP_TESTRPCSUPPORT_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H
#include "TestDrnUtilsRpcWorld.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKESCHEDULERSOURCE_H
#include "TestDrnUtilsFakeSchedulerSource.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKESCHEDULERSOURCE_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKESCHEDULERSOURCE_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKETASKMANAGERSOURCE_H
#include "TestDrnUtilsFakeTaskManagerSource.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKETASKMANAGERSOURCE_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKETASKMANAGERSOURCE_H ? */

#include "TestDrnUtilsSchedulerHost.h"

using namespace GUCEF;
using namespace DRNUTILSTEST;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define REQUIRE_TRUE(t)  do { if ( !(t) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #t, __FILE__, __LINE__ ); throw std::exception(); } } while ( 0 )

namespace {

const char* P_GETINFO = "gucef.drnutils.scheduler.Scheduler/GetInfo";
const char* P_LISTJOBS = "gucef.drnutils.scheduler.Scheduler/ListJobs";
const char* P_GETJOB = "gucef.drnutils.scheduler.Scheduler/GetJob";
const char* P_LISTLANES = "gucef.drnutils.scheduler.Scheduler/ListLanes";
const char* P_LISTRUNS = "gucef.drnutils.scheduler.Scheduler/ListRuns";
const char* P_WATCH = "gucef.drnutils.scheduler.Scheduler/Watch";
const char* P_CONTROL = "gucef.drnutils.scheduler.Scheduler/Control";
const char* P_SUBMIT = "gucef.drnutils.scheduler.Scheduler/SubmitJob";

/*-------------------------------------------------------------------------*/

/** Records audit records; may call back into the host (the records come outside every lock of the host) */
class CRecordingAuditSink : public DRNUTILS::CIDrnUtilsAuditSink
{
    public:
    GUCEF::vector< DRNUTILS::SDrnUtilsAuditRecord > records;
    DRNUTILS::CSchedulerHost* callBack;
    MT::CMutex lock;
    CRecordingAuditSink( void ) : records(), callBack( GUCEF_NULL ), lock() {}
    virtual void OnAuditRecord( const DRNUTILS::SDrnUtilsAuditRecord& record ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL != callBack )
        {
            DRNUTILS::SSchedulerHostInfo info;
            callBack->GetHostInfo( COMCORE::CRpcPrincipal( "local", "local:audit", true ), info );
        }
        MT::CScopeMutex scope( lock );
        records.push_back( record );
    }
    UInt32 Count( void ) { MT::CScopeMutex scope( lock ); return (UInt32) records.size(); }
    DRNUTILS::SDrnUtilsAuditRecord Last( void ) { MT::CScopeMutex scope( lock ); return records.back(); }
};

/*-------------------------------------------------------------------------*/

/** A scheduler host over a fake source, exposed on the service set of a loopback world */
class CSchedulerWorld
{
    public:
    CDrnUtilsRpcWorld rpc;
    CFakeSchedulerSource source;
    DRNUTILS::CSchedulerHostSettings settings;
    DRNUTILS::CSchedulerHost* host;
    CRecordingAuditSink audit;

    CSchedulerWorld( void )
        : rpc(), source( CORE::CCoreGlobal::Instance()->GetPulseGenerator() ), settings(), host( GUCEF_NULL ), audit()
    {
        settings.SetExposure( DRNUTILS::DRNUTILS_EXPOSURE_CONTROL );
        CORE::TJobControlActionSet allowed;
        allowed.insert( CORE::JOBCONTROLACTION_RUN_NOW );
        allowed.insert( CORE::JOBCONTROLACTION_CANCEL_RUN );
        allowed.insert( CORE::JOBCONTROLACTION_ENABLE );
        allowed.insert( CORE::JOBCONTROLACTION_DISABLE );
        allowed.insert( CORE::JOBCONTROLACTION_SET_PRIORITY );
        settings.SetAllowedActions( allowed );
        CORE::CString::StringSet custom;
        custom.insert( "watchlist-add" );
        custom.insert( "rebuild" );
        settings.SetAllowedCustomActions( custom );
        CORE::CString::StringSet submittable;
        submittable.insert( "test.task" );
        settings.SetSubmittableTaskTypes( submittable );
        source.submittable.insert( "test.task" );
        source.submittable.insert( "other.task" );
        source.AddLane( "lane1" );
        source.AddLane( "lane2" );
    }

    ~CSchedulerWorld()
    {
        rpc.Disconnect();
        GUCEF_DELETE host;
    }

    bool Start( bool expose = true )
    {
        host = GUCEF_NEW DRNUTILS::CSchedulerHost( source, settings, rpc.pulseGenerator );
        host->SetAuditSink( &audit );
        CORE::CString error;
        if ( expose && !host->Expose( rpc.services, error ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SchedulerWorld: " << error );
            return false;
        }
        return true;
    }

    bool Connect( const char* roles, const CORE::CString& transport = "loopback", const CORE::CString& subject = "drn:alice" )
    {
        return rpc.Connect( CDrnUtilsRpcWorld::Principal( subject, roles, transport ), false, true );
    }

    COMCORE::CRpcCallResult Control( const CORE::CString& commandId, CORE::TJobControlAction action, CORE::TJobControlTargetKind kind,
                                     const CORE::CString& target, bool waitForRun = false )
    {
        COMCORE::CRpcMessage request = rpc.NewRequest( P_CONTROL );
        if ( !commandId.IsNULLOrEmpty() )
            request.SetString( 1, commandId );
        request.SetEnum( 2, (Int32) action );
        request.SetEnum( 3, (Int32) kind );
        request.SetString( 4, target );
        request.SetBool( 13, waitForRun );
        return rpc.Call( P_CONTROL, request );
    }
};

/*-------------------------------------------------------------------------*/

COMCORE::CRpcPrincipal
LocalPrincipal( const char* roles, const CORE::CString& transport = "local" )
{
    return CDrnUtilsRpcWorld::Principal( "local:admin", roles, transport );
}

/*-------------------------------------------------------------------------*/

bool
LoadJson( const CORE::CString& path, CORE::CDataNode& tree )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) || codec.IsNULL() )
        return false;
    CORE::CFileAccess file( path, "rb" );
    if ( !file.IsValid() )
        return false;
    tree.Clear();
    return codec->BuildDataTree( &tree, &file );
}

/*-------------------------------------------------------------------------*/

CORE::CString
Sha256HexOf( const CORE::CString& text )
{
    UInt8 digest[ 32 ];
    CORE::CSha256::Hash( text.C_String(), text.ByteSize() - 1, digest );
    return RpcTestHex( digest, 32 );
}

/*-------------------------------------------------------------------------*/

UInt32
CountKind( const GUCEF::vector< COMCORE::CRpcMessage >& messages, Int32 kind )
{
    UInt32 count = 0;
    for ( UInt32 i=0; i<messages.size(); ++i )
        if ( messages[ i ].GetEnum( 1 ) == kind )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

/** Controls the host from a pool thread */
class CControlTask : public CORE::CTaskConsumer
{
    public:
    DRNUTILS::CSchedulerHost* host;
    UInt32 index;
    MT::CTAtomic< UInt32 > ok;
    MT::CTAtomic< UInt32 > done;        /**< the work is done: read by the test thread (CTaskConsumer::IsActive() is not for other threads) */
    CControlTask( DRNUTILS::CSchedulerHost* h, UInt32 i ) : CORE::CTaskConsumer(), host( h ), index( i ), ok( 0 ), done( 0 ) {}
    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return "drnutils.test.control"; }
    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 c=0; c<200; ++c )
        {
            CORE::SJobControlCommand cmd;
            cmd.action = 0 == c % 2 ? CORE::JOBCONTROLACTION_ENABLE : CORE::JOBCONTROLACTION_DISABLE;
            cmd.targetKind = CORE::JOBCONTROLTARGET_JOB;
            cmd.targetId = "j1";
            CORE::SJobControlResult res;
            bool rep = false;
            COMCORE::CRpcPrincipal p( "local", "local:t" + CORE::ToString( index ), true );
            p.roles.insert( "scheduler.control" );
            if ( DRNUTILS::DRNUTILS_STATUS_OK == host->Control( p, "t" + CORE::ToString( index ) + "-" + CORE::ToString( c ), cmd, res, rep ) )
                ok.Store( ok.Load() + 1 );
            DRNUTILS::SSchedulerHostInfo info;
            host->GetHostInfo( p, info );
        }
        done.Store( 1 );
        return true;
    }
    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE {}
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

void
PerformDrnUtilsSchedulerHostTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnUtilsSchedulerHost" );

    GUCEF_TESTFW_TESTCASE( "RU-T1 contracts: the built contracts equal the JSON sources (canonical hash), are compatible with themselves and are registered" )
    try
    {
        COMCORE::CRpcContractPtr scheduler = DRNUTILS::CSchedulerContracts::GetContract();
        COMCORE::CRpcContractPtr tasks = DRNUTILS::CTaskManagerContracts::GetContract();
        REQUIRE_TRUE( !scheduler.IsNULL() && !tasks.IsNULL() );
        COMCORE::CRpcCompatibilityReport report;
        ASSERT_TRUE( COMCORE::CRpcContractCompatibility::RESULT_COMPATIBLE == COMCORE::CRpcContractCompatibility::Check( *scheduler, *scheduler, report ) );
        ASSERT_TRUE( COMCORE::CRpcContractCompatibility::RESULT_COMPATIBLE == COMCORE::CRpcContractCompatibility::Check( *tasks, *tasks, report ) );
        ASSERT_TRUE( 0 == report.GetBreakingCount() );
        COMCORE::CRpcContractRegistry& registry = COMCORE::CComCoreGlobal::Instance()->GetRpcContractRegistry();
        ASSERT_TRUE( !registry.FindByHash( scheduler->GetHash() ).IsNULL() );
        ASSERT_TRUE( !registry.FindByHash( tasks->GetHash() ).IsNULL() );

        GUCEF_TESTFW_REQUIRE_ENV( rpcDirEnv, "GUCEF_DRNUTILS_RPC_DIR" );
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        const char* files[ 2 ] = { "gucef.drnutils.scheduler.json", "gucef.drnutils.tasks.json" };
        COMCORE::CRpcContractPtr built[ 2 ] = { scheduler, tasks };
        for ( UInt32 i=0; i<2; ++i )
        {
            CORE::CDataNode tree;
            REQUIRE_TRUE( LoadJson( CORE::CombinePath( rpcDirEnv.c_str(), files[ i ] ), tree ) );
            CORE::CString error;
            COMCORE::CRpcContractPtr authored = COMCORE::CRpcContract::LoadFromDataNode( tree, error );
            REQUIRE_TRUE( !authored.IsNULL() );
            if ( !( authored->GetHash() == built[ i ]->GetHash() ) )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RU-T1: " << files[ i ] << " hash " << authored->GetHash().ToHexString() << " built " << built[ i ]->GetHash().ToHexString() );
            ASSERT_TRUE( authored->GetHash() == built[ i ]->GetHash() );
        }
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RU-T4 nothing is exposed by default: default settings expose nothing, allow nothing, and a host with them registers no service and publishes no table" )
    try
    {
        DRNUTILS::CSchedulerHostSettings defaults;
        ASSERT_TRUE( DRNUTILS::DRNUTILS_EXPOSURE_NONE == defaults.GetExposure() );
        ASSERT_TRUE( defaults.GetAllowedActions().empty() && defaults.GetAllowedCustomActions().empty() && defaults.GetSubmittableTaskTypes().empty() );
        ASSERT_FALSE( defaults.GetAllowInsecureControl() );
        DRNUTILS::CTaskManagerHostSettings taskDefaults;
        ASSERT_TRUE( DRNUTILS::DRNUTILS_EXPOSURE_NONE == taskDefaults.GetExposure() );
        ASSERT_TRUE( taskDefaults.GetControllablePools().empty() && taskDefaults.GetCancellableTaskTypes().empty() );

        CDrnUtilsRpcWorld rpc;
        CFakeSchedulerSource source( rpc.pulseGenerator );
        source.AddJob( "j1" );
        DRNUTILS::CSchedulerHost host( source, defaults, rpc.pulseGenerator );
        CORE::CString error;
        ASSERT_FALSE( host.Expose( rpc.services, error ) );
        ASSERT_FALSE( host.IsExposed() );
        ASSERT_TRUE( 0 == rpc.services->GetServiceCount() );
        ASSERT_FALSE( host.PublishTables( "no-session", error ) );
        CFakeTaskManagerSource tasks;
        DRNUTILS::CTaskManagerHost taskHost( tasks, taskDefaults, rpc.pulseGenerator );
        ASSERT_FALSE( taskHost.Expose( rpc.services, error ) );
        ASSERT_TRUE( 0 == rpc.services->GetServiceCount() );

        // the local control path refuses everything with default settings too
        CORE::SJobControlCommand command;
        command.action = CORE::JOBCONTROLACTION_RUN_NOW;
        command.targetKind = CORE::JOBCONTROLTARGET_JOB;
        command.targetId = "j1";
        CORE::SJobControlResult result;
        bool replayed = false;
        ASSERT_TRUE( DRNUTILS::DRNUTILS_STATUS_PERMISSION_DENIED == host.Control( LocalPrincipal( "scheduler.control" ), "c1", command, result, replayed ) );
        ASSERT_TRUE( 0 == source.GetApplyCalls() );

        // a client without the service: the contract is not offered, nothing binds
        ASSERT_FALSE( rpc.Connect( CDrnUtilsRpcWorld::Principal( "drn:alice", "scheduler.view" ), false, true ) );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RU-T2 every read method over loopback: info, lists with paging, details, revisions; hidden jobs never listed, sensitive notes redacted" )
    try
    {
        CSchedulerWorld w;
        for ( UInt32 i=0; i<25; ++i )
            w.source.AddJob( "job" + CORE::ToString( 100 + i ), 0 == i % 2 ? "lane1" : "lane2" );
        w.source.AddJob( "secret", "lane1", CORE::JOBFLAG_HIDDEN );
        w.source.AddJob( "sensitive", "lane1", CORE::JOBFLAG_SENSITIVE );
        UInt64 hiddenRun = w.source.StartRun( "secret", "x" );
        UInt64 sensitiveRun = w.source.StartRun( "sensitive", "x" );
        for ( UInt32 r=0; r<7; ++r )
        {
            UInt64 run = w.source.StartRun( "job100", "drn:bob" );
            w.source.EndRun( run, CORE::JOBRUNSTATE_SUCCEEDED, false );
        }
        REQUIRE_TRUE( w.Start() );
        REQUIRE_TRUE( w.Connect( "scheduler.view" ) );

        COMCORE::CRpcCallResult info = w.rpc.Call( P_GETINFO, w.rpc.NewRequest( P_GETINFO ) );
        REQUIRE_TRUE( info.status.IsOk() );
        ASSERT_TRUE( "node1" == info.response.GetString( 1 ) );
        ASSERT_TRUE( 1001 == info.response.GetUInt64( 2 ) );
        ASSERT_TRUE( info.response.GetUInt64( 3 ) == w.source.revision );
        ASSERT_TRUE( CORE::CString( CORE::JobSchedulerHostStateToString( CORE::JOBSCHEDULERSTATE_RUNNING ) ) == info.response.GetString( 5 ) );
        ASSERT_TRUE( 0 == info.response.GetCount( 6 ) );     // a viewer gets no actions
        ASSERT_TRUE( 0 == info.response.GetCount( 8 ) );     // and no submittable task types

        // paging: 26 visible jobs, 10 per page
        CORE::CString::StringSet seen;
        CORE::CString token;
        UInt32 pages = 0;
        do
        {
            COMCORE::CRpcMessage request = w.rpc.NewRequest( P_LISTJOBS );
            request.SetUInt32( 3, 10 );
            if ( !token.IsNULLOrEmpty() )
                request.SetString( 4, token );
            COMCORE::CRpcCallResult page = w.rpc.Call( P_LISTJOBS, request );
            REQUIRE_TRUE( page.status.IsOk() );
            ASSERT_TRUE( page.response.GetUInt64( 3 ) == w.source.revision );
            for ( UInt32 j=0; j<page.response.GetCount( 1 ); ++j )
                seen.insert( page.response.GetMessageAt( 1, j )->GetString( 1 ) );
            token = page.response.GetString( 2 );
            ++pages;
        }
        while ( !token.IsNULLOrEmpty() && pages < 10 );
        ASSERT_TRUE( 3 == pages );
        ASSERT_TRUE( 26 == seen.size() );
        ASSERT_TRUE( seen.find( "secret" ) == seen.end() );

        COMCORE::CRpcMessage byLane = w.rpc.NewRequest( P_LISTJOBS );
        byLane.SetString( 2, "lane2" );
        COMCORE::CRpcCallResult lane2 = w.rpc.Call( P_LISTJOBS, byLane );
        ASSERT_TRUE( lane2.status.IsOk() && 12 == lane2.response.GetCount( 1 ) );

        COMCORE::CRpcCallResult lanes = w.rpc.Call( P_LISTLANES, w.rpc.NewRequest( P_LISTLANES ) );
        ASSERT_TRUE( lanes.status.IsOk() && 2 == lanes.response.GetCount( 1 ) );
        ASSERT_TRUE( 4 == lanes.response.GetMessageAt( 1, 0 )->GetCount( 6 ) );

        COMCORE::CRpcMessage detailRequest = w.rpc.NewRequest( P_GETJOB );
        detailRequest.SetString( 1, "job100" );
        detailRequest.SetUInt32( 2, 5 );
        detailRequest.SetBool( 3, true );
        COMCORE::CRpcCallResult detail = w.rpc.Call( P_GETJOB, detailRequest );
        REQUIRE_TRUE( detail.status.IsOk() );
        ASSERT_TRUE( "job100" == detail.response.GetMessage( 1 )->GetString( 1 ) );
        ASSERT_TRUE( 5 == detail.response.GetCount( 2 ) );
        ASSERT_TRUE( "the description of job100" == detail.response.GetString( 4 ) );
        CORE::CDataNode sourceStatus;
        ASSERT_TRUE( detail.response.GetDataNode( 3, sourceStatus ) && "3" == sourceStatus.GetAttributeValue( "pending" ).AsString() );

        COMCORE::CRpcMessage hiddenRequest = w.rpc.NewRequest( P_GETJOB );
        hiddenRequest.SetString( 1, "secret" );
        ASSERT_TRUE( COMCORE::RPC_STATUS_NOT_FOUND == w.rpc.Call( P_GETJOB, hiddenRequest ).status.code );

        // runs: newest first, active only unless includeEnded, paging by run id
        COMCORE::CRpcCallResult active = w.rpc.Call( P_LISTRUNS, w.rpc.NewRequest( P_LISTRUNS ) );
        REQUIRE_TRUE( active.status.IsOk() );
        ASSERT_TRUE( 1 == active.response.GetCount( 1 ) );   // the sensitive job's run; the hidden job's run is never listed
        const COMCORE::CRpcMessage* sensitive = active.response.GetMessageAt( 1, 0 );
        ASSERT_TRUE( sensitiveRun == sensitive->GetUInt64( 1 ) );
        ASSERT_TRUE( "[redacted]" == sensitive->GetString( 12 ) );
        COMCORE::CRpcMessage allRuns = w.rpc.NewRequest( P_LISTRUNS );
        allRuns.SetBool( 5, true );
        allRuns.SetUInt32( 3, 4 );
        COMCORE::CRpcCallResult firstPage = w.rpc.Call( P_LISTRUNS, allRuns );
        REQUIRE_TRUE( firstPage.status.IsOk() && 4 == firstPage.response.GetCount( 1 ) );
        ASSERT_TRUE( firstPage.response.GetMessageAt( 1, 0 )->GetUInt64( 1 ) > firstPage.response.GetMessageAt( 1, 1 )->GetUInt64( 1 ) );
        allRuns.SetString( 4, firstPage.response.GetString( 2 ) );
        COMCORE::CRpcCallResult secondPage = w.rpc.Call( P_LISTRUNS, allRuns );
        REQUIRE_TRUE( secondPage.status.IsOk() && 4 == secondPage.response.GetCount( 1 ) );
        ASSERT_TRUE( secondPage.response.GetMessageAt( 1, 0 )->GetUInt64( 1 ) < firstPage.response.GetMessageAt( 1, 3 )->GetUInt64( 1 ) );
        for ( UInt32 p=0; p<2; ++p )
        {
            const COMCORE::CRpcCallResult& page = 0 == p ? firstPage : secondPage;
            for ( UInt32 r=0; r<page.response.GetCount( 1 ); ++r )
                ASSERT_TRUE( hiddenRun != page.response.GetMessageAt( 1, r )->GetUInt64( 1 ) );
        }
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RU-T4 per-method roles: without scheduler.view every read method is PERMISSION_DENIED; actions shown are filtered per viewer" )
    try
    {
        CSchedulerWorld w;
        CORE::SJobStatus& limited = w.source.AddJob( "limited" );
        limited.remoteActions.clear();
        limited.remoteActions.insert( CORE::JOBCONTROLACTION_RUN_NOW );
        w.source.AddJob( "full" );
        REQUIRE_TRUE( w.Start() );
        REQUIRE_TRUE( w.Connect( "somebody" ) );
        const char* paths[ 6 ] = { P_GETINFO, P_LISTJOBS, P_GETJOB, P_LISTLANES, P_LISTRUNS, P_WATCH };
        for ( UInt32 i=0; i<5; ++i )
            ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == w.rpc.Call( paths[ i ], w.rpc.NewRequest( paths[ i ] ) ).status.code );
        COMCORE::CIRpcStreamPtr watch = w.rpc.OpenStream( P_WATCH, w.rpc.NewRequest( P_WATCH ) );
        REQUIRE_TRUE( !watch.IsNULL() );
        CStreamRecorder recorder;
        recorder.Watch( watch );
        class CEnded : public CCondition { public: CStreamRecorder& r; CEnded( CStreamRecorder& rec ) : r( rec ) {} virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return r.ended; } };
        CEnded ended( recorder );
        ASSERT_TRUE( PumpUntil( ended, 3000 ) );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == recorder.status.code );

        // a controller sees the allowed actions; per job only the job's remote actions
        REQUIRE_TRUE( w.Connect( "scheduler.view,scheduler.control,scheduler.submit" ) );
        COMCORE::CRpcCallResult info = w.rpc.Call( P_GETINFO, w.rpc.NewRequest( P_GETINFO ) );
        REQUIRE_TRUE( info.status.IsOk() );
        ASSERT_TRUE( info.response.GetCount( 6 ) >= 5 );
        ASSERT_TRUE( 1 == info.response.GetCount( 8 ) );     // test.task: allowed by host and source; other.task only by the source
        COMCORE::CRpcCallResult jobs = w.rpc.Call( P_LISTJOBS, w.rpc.NewRequest( P_LISTJOBS ) );
        REQUIRE_TRUE( jobs.status.IsOk() && 2 == jobs.response.GetCount( 1 ) );
        const COMCORE::CRpcMessage* first = jobs.response.GetMessageAt( 1, 0 );
        const COMCORE::CRpcMessage* second = jobs.response.GetMessageAt( 1, 1 );
        const COMCORE::CRpcMessage* limitedRow = "limited" == first->GetString( 1 ) ? first : second;
        const COMCORE::CRpcMessage* fullRow = "limited" == first->GetString( 1 ) ? second : first;
        ASSERT_TRUE( 1 == limitedRow->GetCount( 19 ) );
        ASSERT_TRUE( CORE::JOBCONTROLACTION_RUN_NOW == limitedRow->GetMessageAt( 19, 0 )->GetEnum( 1 ) );
        ASSERT_TRUE( 5 == fullRow->GetCount( 19 ) );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RU-T4 a viewer cannot control; control needs a secure transport; per-action role refinement; VIEW exposure refuses control" )
    try
    {
        CSchedulerWorld w;
        w.source.AddJob( "j1" );
        CORE::CString::StringSet high;
        high.insert( "admin.high" );
        DRNUTILS::CDrnUtilsRoleMap roles;
        roles.SetControlRolesForAction( CORE::JOBCONTROLACTION_RUN_NOW, high );
        roles.SetControlRolesForAction( CORE::JOBCONTROLACTION_SET_PRIORITY, high );
        w.settings.SetRoleMap( roles );
        REQUIRE_TRUE( w.Start() );

        REQUIRE_TRUE( w.Connect( "scheduler.view" ) );
        COMCORE::CRpcCallResult refused = w.Control( "c1", CORE::JOBCONTROLACTION_RUN_NOW, CORE::JOBCONTROLTARGET_JOB, "j1" );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == refused.status.code );
        ASSERT_TRUE( 0 == w.source.GetApplyCalls() );
        REQUIRE_TRUE( w.audit.Count() > 0 );
        ASSERT_TRUE( "PERMISSION_DENIED" == w.audit.Last().outcome );

        // not loopback, not protected: refused even with the role
        REQUIRE_TRUE( w.Connect( "scheduler.view,scheduler.control", "tcp" ) );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == w.Control( "c2", CORE::JOBCONTROLACTION_ENABLE, CORE::JOBCONTROLTARGET_JOB, "j1" ).status.code );
        ASSERT_TRUE( 0 == w.source.GetApplyCalls() );
        // a drn principal of a protected link may
        COMCORE::CRpcPrincipal drn = CDrnUtilsRpcWorld::Principal( "drn:alice", "scheduler.view,scheduler.control", "drn" );
        drn.attributes.Set( "drn.protected", CORE::CVariant( true ) );
        REQUIRE_TRUE( w.rpc.Connect( drn, false, true ) );
        ASSERT_TRUE( w.Control( "c3", CORE::JOBCONTROLACTION_ENABLE, CORE::JOBCONTROLTARGET_JOB, "j1" ).status.IsOk() );
        ASSERT_TRUE( 1 == w.source.GetApplyCalls() );
        // an unauthenticated principal may not
        COMCORE::CRpcPrincipal anonymous = CDrnUtilsRpcWorld::Principal( "loopback:anon", "scheduler.view,scheduler.control" );
        anonymous.isAuthenticated = false;
        REQUIRE_TRUE( w.rpc.Connect( anonymous, false, true ) );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == w.Control( "c4", CORE::JOBCONTROLACTION_ENABLE, CORE::JOBCONTROLTARGET_JOB, "j1" ).status.code );

        // refinement: plain RUN_NOW is fine with the control role, at priority 0 it needs admin.high
        REQUIRE_TRUE( w.Connect( "scheduler.view,scheduler.control" ) );
        ASSERT_TRUE( w.Control( "c5", CORE::JOBCONTROLACTION_RUN_NOW, CORE::JOBCONTROLTARGET_JOB, "j1" ).status.IsOk() );
        COMCORE::CRpcMessage urgent = w.rpc.NewRequest( P_CONTROL );
        urgent.SetString( 1, "c6" );
        urgent.SetEnum( 2, CORE::JOBCONTROLACTION_RUN_NOW );
        urgent.SetEnum( 3, CORE::JOBCONTROLTARGET_JOB );
        urgent.SetString( 4, "j1" );
        urgent.SetUInt32( 8, 0 );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == w.rpc.Call( P_CONTROL, urgent ).status.code );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == w.Control( "c7", CORE::JOBCONTROLACTION_SET_PRIORITY, CORE::JOBCONTROLTARGET_JOB, "j1" ).status.code );
        REQUIRE_TRUE( w.Connect( "scheduler.view,scheduler.control,admin.high" ) );
        urgent.SetString( 1, "c8" );
        ASSERT_TRUE( w.rpc.Call( P_CONTROL, urgent ).status.IsOk() );
        // an action not allowed on the host
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == w.Control( "c9", CORE::JOBCONTROLACTION_SET_LANE_MODE, CORE::JOBCONTROLTARGET_LANE, "lane1" ).status.code );
        // pause and resume are not in 1.0: the reserved numbers are refused
        ASSERT_TRUE( COMCORE::RPC_STATUS_UNIMPLEMENTED == w.Control( "c10", (CORE::TJobControlAction) 1, CORE::JOBCONTROLTARGET_JOB, "j1" ).status.code );

        // AllowInsecureControl (tests only) lets a tcp principal control
        CSchedulerWorld insecure;
        insecure.source.AddJob( "j1" );
        insecure.settings.SetAllowInsecureControl( true );
        REQUIRE_TRUE( insecure.Start() );
        REQUIRE_TRUE( insecure.Connect( "scheduler.view,scheduler.control", "tcp" ) );
        ASSERT_TRUE( insecure.Control( "c1", CORE::JOBCONTROLACTION_ENABLE, CORE::JOBCONTROLTARGET_JOB, "j1" ).status.IsOk() );

        // exposure VIEW: the read methods work, control and submission are refused
        CSchedulerWorld viewOnly;
        viewOnly.source.AddJob( "j1" );
        viewOnly.settings.SetExposure( DRNUTILS::DRNUTILS_EXPOSURE_VIEW );
        REQUIRE_TRUE( viewOnly.Start() );
        REQUIRE_TRUE( viewOnly.Connect( "scheduler.view,scheduler.control,scheduler.submit" ) );
        ASSERT_TRUE( viewOnly.rpc.Call( P_GETINFO, viewOnly.rpc.NewRequest( P_GETINFO ) ).status.IsOk() );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == viewOnly.Control( "c1", CORE::JOBCONTROLACTION_ENABLE, CORE::JOBCONTROLTARGET_JOB, "j1" ).status.code );
        ASSERT_TRUE( 0 == viewOnly.source.GetApplyCalls() );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RU-T5 idempotency: a replayed commandId returns the stored outcome and is not applied twice; per subject; metadata key; bounds and expiry" )
    try
    {
        CSchedulerWorld w;
        w.source.AddJob( "j1" );
        w.settings.SetMaxCommandsPerSubject( 2 );
        w.settings.SetIdempotencyTtlMs( 400 );
        REQUIRE_TRUE( w.Start() );
        REQUIRE_TRUE( w.Connect( "scheduler.view,scheduler.control" ) );
        COMCORE::CRpcCallResult first = w.Control( "same", CORE::JOBCONTROLACTION_RUN_NOW, CORE::JOBCONTROLTARGET_JOB, "j1" );
        REQUIRE_TRUE( first.status.IsOk() );
        ASSERT_FALSE( first.response.GetBool( 1 ) );
        UInt64 runId = first.response.GetUInt64( 3 );
        ASSERT_TRUE( 0 != runId );
        COMCORE::CRpcCallResult again = w.Control( "same", CORE::JOBCONTROLACTION_RUN_NOW, CORE::JOBCONTROLTARGET_JOB, "j1" );
        REQUIRE_TRUE( again.status.IsOk() );
        ASSERT_TRUE( again.response.GetBool( 1 ) );
        ASSERT_TRUE( runId == again.response.GetUInt64( 3 ) );
        ASSERT_TRUE( 1 == w.source.GetApplyCalls() );
        ASSERT_TRUE( w.audit.Last().replayed );

        // the empty id is refused, the metadata key is taken in its place, both differing is refused
        ASSERT_TRUE( COMCORE::RPC_STATUS_INVALID_ARGUMENT == w.Control( "", CORE::JOBCONTROLACTION_ENABLE, CORE::JOBCONTROLTARGET_JOB, "j1" ).status.code );
        COMCORE::CRpcCallOptions options;
        options.metadata.Set( "idempotency-key", CORE::CVariant( "meta-1" ) );
        COMCORE::CRpcMessage request = w.rpc.NewRequest( P_CONTROL );
        request.SetEnum( 2, CORE::JOBCONTROLACTION_ENABLE );
        request.SetEnum( 3, CORE::JOBCONTROLTARGET_JOB );
        request.SetString( 4, "j1" );
        ASSERT_TRUE( w.rpc.Call( P_CONTROL, request, options ).status.IsOk() );
        ASSERT_TRUE( w.rpc.Call( P_CONTROL, request, options ).response.GetBool( 1 ) );
        request.SetString( 1, "other" );
        ASSERT_TRUE( COMCORE::RPC_STATUS_INVALID_ARGUMENT == w.rpc.Call( P_CONTROL, request, options ).status.code );

        // another subject with the same id is another command
        REQUIRE_TRUE( w.Connect( "scheduler.view,scheduler.control", "loopback", "drn:bob" ) );
        UInt32 before = w.source.GetApplyCalls();
        COMCORE::CRpcCallResult bob = w.Control( "same", CORE::JOBCONTROLACTION_RUN_NOW, CORE::JOBCONTROLTARGET_JOB, "j1" );
        ASSERT_TRUE( bob.status.IsOk() && !bob.response.GetBool( 1 ) );
        ASSERT_TRUE( before + 1 == w.source.GetApplyCalls() );

        // at most 2 per subject: the oldest goes
        w.Control( "b2", CORE::JOBCONTROLACTION_ENABLE, CORE::JOBCONTROLTARGET_JOB, "j1" );
        w.Control( "b3", CORE::JOBCONTROLACTION_ENABLE, CORE::JOBCONTROLTARGET_JOB, "j1" );
        before = w.source.GetApplyCalls();
        ASSERT_FALSE( w.Control( "same", CORE::JOBCONTROLACTION_RUN_NOW, CORE::JOBCONTROLTARGET_JOB, "j1" ).response.GetBool( 1 ) );
        ASSERT_TRUE( before + 1 == w.source.GetApplyCalls() );
        ASSERT_TRUE( w.Control( "b3", CORE::JOBCONTROLACTION_ENABLE, CORE::JOBCONTROLTARGET_JOB, "j1" ).response.GetBool( 1 ) );

        // expiry
        PumpFor( 500 );
        before = w.source.GetApplyCalls();
        ASSERT_FALSE( w.Control( "b3", CORE::JOBCONTROLACTION_ENABLE, CORE::JOBCONTROLTARGET_JOB, "j1" ).response.GetBool( 1 ) );
        ASSERT_TRUE( before + 1 == w.source.GetApplyCalls() );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RU-T6 audit: one record per applied, refused and replayed command, the arguments as SHA-256 digest only; a sink that calls the host does not deadlock" )
    try
    {
        CSchedulerWorld w;
        w.source.AddJob( "j1" );
        DRNUTILS::SSchedulerActionInfo rebuild;
        rebuild.action = CORE::JOBCONTROLACTION_CUSTOM;
        rebuild.targetKind = CORE::JOBCONTROLTARGET_HOST;
        rebuild.customName = "rebuild";
        w.source.declaredCustom.push_back( rebuild );
        REQUIRE_TRUE( w.Start() );
        w.audit.callBack = w.host;
        REQUIRE_TRUE( w.Connect( "scheduler.view,scheduler.control" ) );
        UInt32 before = w.audit.Count();
        COMCORE::CRpcMessage request = w.rpc.NewRequest( P_CONTROL );
        request.SetString( 1, "cmd-audit-1" );
        request.SetEnum( 2, CORE::JOBCONTROLACTION_CUSTOM );
        request.SetEnum( 3, CORE::JOBCONTROLTARGET_HOST );
        request.SetString( 4, "node1" );
        request.SetString( 5, "rebuild" );
        request.SetMapEntry( 6, CORE::CVariant( "symbol" ), CORE::CVariant( "PRIVATE-HOLDING-XYZ" ) );
        request.SetMapEntry( 6, CORE::CVariant( "account" ), CORE::CVariant( "ACCT-0042" ) );
        request.SetString( 10, "operator reason" );
        COMCORE::CRpcCallResult applied = w.rpc.Call( P_CONTROL, request );
        REQUIRE_TRUE( applied.status.IsOk() );
        ASSERT_TRUE( "custom applied" == applied.response.GetString( 4 ) || 1 == applied.response.GetCount( 4 ) );
        REQUIRE_TRUE( before + 1 == w.audit.Count() );
        DRNUTILS::SDrnUtilsAuditRecord record = w.audit.Last();
        ASSERT_TRUE( "drn:alice" == record.subject && "loopback" == record.transport && "CUSTOM" == record.action && "rebuild" == record.customName );
        ASSERT_TRUE( "gucef.drnutils.scheduler" == record.contract && "cmd-audit-1" == record.commandId && "OK" == record.outcome );
        ASSERT_TRUE( 32 == record.sessionId.Length() );
        ASSERT_TRUE( Sha256HexOf( "account=ACCT-0042\nsymbol=PRIVATE-HOLDING-XYZ\n" ) == record.argumentsDigest );
        CORE::CString all = record.subject + record.transport + record.sessionId + record.contract + record.action + record.targetKind + record.targetId +
                            record.customName + record.taskTypeName + record.argumentsDigest + record.outcome + record.reason + record.commandId;
        ASSERT_TRUE( 0 > all.HasSubstr( "PRIVATE-HOLDING-XYZ" ) && 0 > all.HasSubstr( "ACCT-0042" ) );
        // replayed and refused commands are audited too
        w.rpc.Call( P_CONTROL, request );
        REQUIRE_TRUE( before + 2 == w.audit.Count() );
        ASSERT_TRUE( w.audit.Last().replayed );
        request.SetString( 1, "cmd-audit-2" );
        request.SetString( 5, "not-declared-and-not-allowed" );
        ASSERT_FALSE( w.rpc.Call( P_CONTROL, request ).status.IsOk() );
        REQUIRE_TRUE( before + 3 == w.audit.Count() );
        ASSERT_TRUE( "PERMISSION_DENIED" == w.audit.Last().outcome && !w.audit.Last().reason.IsNULLOrEmpty() );

        // the local path from another thread: the sink's call into the host comes outside every lock of the host
        CORE::SJobControlCommand command;
        command.action = CORE::JOBCONTROLACTION_ENABLE;
        command.targetKind = CORE::JOBCONTROLTARGET_JOB;
        command.targetId = "j1";
        CORE::SJobControlResult result;
        bool replayed = false;
        ASSERT_TRUE( DRNUTILS::DRNUTILS_STATUS_OK == w.host->Control( LocalPrincipal( "scheduler.control" ), "local-1", command, result, replayed ) );
        ASSERT_TRUE( before + 4 == w.audit.Count() && "local" == w.audit.Last().transport );

        // the default sink logs and never fails
        w.host->SetAuditSink( GUCEF_NULL );
        ASSERT_TRUE( DRNUTILS::DRNUTILS_STATUS_OK == w.host->Control( LocalPrincipal( "scheduler.control" ), "local-2", command, result, replayed ) );
        w.host->SetAuditSink( &w.audit );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RU-T7 rate limit per subject: beyond the maximum per minute RESOURCE_EXHAUSTED (audited), other subjects are not affected" )
    try
    {
        CSchedulerWorld w;
        w.source.AddJob( "j1" );
        w.settings.SetMaxControlCommandsPerMinutePerSubject( 3 );
        REQUIRE_TRUE( w.Start() );
        REQUIRE_TRUE( w.Connect( "scheduler.view,scheduler.control" ) );
        for ( UInt32 i=0; i<3; ++i )
            ASSERT_TRUE( w.Control( "r" + CORE::ToString( i ), CORE::JOBCONTROLACTION_ENABLE, CORE::JOBCONTROLTARGET_JOB, "j1" ).status.IsOk() );
        ASSERT_TRUE( COMCORE::RPC_STATUS_RESOURCE_EXHAUSTED == w.Control( "r3", CORE::JOBCONTROLACTION_ENABLE, CORE::JOBCONTROLTARGET_JOB, "j1" ).status.code );
        ASSERT_TRUE( "RESOURCE_EXHAUSTED" == w.audit.Last().outcome );
        ASSERT_TRUE( 3 == w.source.GetApplyCalls() );
        // a replay is not a new command: it is answered within the limit
        ASSERT_TRUE( w.Control( "r1", CORE::JOBCONTROLACTION_ENABLE, CORE::JOBCONTROLTARGET_JOB, "j1" ).response.GetBool( 1 ) );
        REQUIRE_TRUE( w.Connect( "scheduler.view,scheduler.control", "loopback", "drn:bob" ) );
        ASSERT_TRUE( w.Control( "r0", CORE::JOBCONTROLACTION_ENABLE, CORE::JOBCONTROLTARGET_JOB, "j1" ).status.IsOk() );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RU-T8 RUN_NOW with waitForRun: ACCEPTED with the run id, then RESULT with the final run; a cancel of that run ends the call" )
    try
    {
        CSchedulerWorld w;
        w.source.AddJob( "j1" );
        REQUIRE_TRUE( w.Start() );
        REQUIRE_TRUE( w.Connect( "scheduler.view,scheduler.control" ) );
        COMCORE::CRpcMessage request = w.rpc.NewRequest( P_CONTROL );
        request.SetString( 1, "wait-1" );
        request.SetEnum( 2, CORE::JOBCONTROLACTION_RUN_NOW );
        request.SetEnum( 3, CORE::JOBCONTROLTARGET_JOB );
        request.SetString( 4, "j1" );
        request.SetBool( 13, true );
        CORE::CFutureResult future = w.rpc.client->CallAsync( P_CONTROL, request );
        PumpFor( 200 );
        ASSERT_FALSE( future.IsReady() );
        REQUIRE_TRUE( 1 == w.source.runs.size() );
        UInt64 runId = w.source.runs[ 0 ].runId;
        w.source.EndRun( runId, CORE::JOBRUNSTATE_SUCCEEDED );
        class CReady : public CCondition { public: CORE::CFutureResult& f; CReady( CORE::CFutureResult& fr ) : f( fr ) {} virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return f.IsReady(); } };
        CReady ready( future );
        REQUIRE_TRUE( PumpUntil( ready, 3000 ) );
        COMCORE::CRpcCallResult result;
        REQUIRE_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( future, result ) );
        ASSERT_TRUE( result.status.IsOk() );
        ASSERT_TRUE( result.wasAccepted );
        ASSERT_TRUE( runId == result.requestId.GetUInt64() );
        REQUIRE_TRUE( GUCEF_NULL != result.response.GetMessage( 5 ) );
        ASSERT_TRUE( CORE::JOBRUNSTATE_SUCCEEDED == result.response.GetMessage( 5 )->GetEnum( 5 ) );

        // a cancel of the awaited run ends the call with the cancelled run
        request.SetString( 1, "wait-2" );
        CORE::CFutureResult second = w.rpc.client->CallAsync( P_CONTROL, request );
        PumpFor( 200 );
        REQUIRE_TRUE( 2 == w.source.runs.size() );
        UInt64 secondRun = w.source.runs[ 1 ].runId;
        ASSERT_TRUE( w.Control( "cancel-1", CORE::JOBCONTROLACTION_CANCEL_RUN, CORE::JOBCONTROLTARGET_RUN, CORE::ToString( secondRun ) ).status.IsOk() );
        w.source.RaiseChanged();
        CReady secondReady( second );
        REQUIRE_TRUE( PumpUntil( secondReady, 3000 ) );
        REQUIRE_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( second, result ) );
        ASSERT_TRUE( result.status.IsOk() && result.wasAccepted );
        ASSERT_TRUE( CORE::JOBRUNSTATE_CANCELLED == result.response.GetMessage( 5 )->GetEnum( 5 ) );

        // a source that raises its change while Apply() runs (the run ended at once) answers at once too
        w.source.raiseOnApply = true;
        request.SetString( 1, "wait-3" );
        request.SetBool( 13, false );
        ASSERT_TRUE( w.rpc.Call( P_CONTROL, request ).status.IsOk() );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "SubmitJob: allowlisted task types only (host and source), payload bound, role scheduler.submit, audited with the payload digest; REMOVE_JOB of submitted jobs only" )
    try
    {
        CSchedulerWorld w;
        w.source.AddJob( "coded" );
        w.settings.SetMaxSubmitPayloadBytes( 512 );
        REQUIRE_TRUE( w.Start() );
        REQUIRE_TRUE( w.Connect( "scheduler.view,scheduler.submit" ) );
        COMCORE::CRpcMessage request = w.rpc.NewRequest( P_SUBMIT );
        request.SetString( 1, "submit-1" );
        request.SetString( 2, "fetch-aapl" );
        request.SetString( 3, "Fetch" );
        request.SetString( 4, "test.task" );
        CORE::CDataNode payload( "payload", GUCEF_DATATYPE_OBJECT );
        payload.SetAttribute( "symbol", "CONFIDENTIAL-SYMBOL" );
        request.SetDataNode( 5, payload );
        COMCORE::CRpcMessage* trigger = request.GetMutableMessage( 6 );
        trigger->SetEnum( 1, CORE::JOBTRIGGER_SCHEDULE );
        trigger->SetString( 2, "0 2 * * *" );
        COMCORE::CRpcCallResult ok = w.rpc.Call( P_SUBMIT, request );
        if ( !ok.status.IsOk() )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SubmitJob: " << ok.status.ToString() );
        REQUIRE_TRUE( ok.status.IsOk() );
        REQUIRE_TRUE( 1 == w.source.submitted.size() );
        ASSERT_TRUE( "drn:alice" == w.source.submitted[ 0 ].submittedBy );
        ASSERT_TRUE( "test.task" == w.source.submitted[ 0 ].definition.GetTaskTypeName() );
        ASSERT_TRUE( CORE::JOBTRIGGER_SCHEDULE == w.source.submitted[ 0 ].definition.GetTrigger().GetKind() );
        DRNUTILS::SDrnUtilsAuditRecord record = w.audit.Last();
        ASSERT_TRUE( "SUBMIT_JOB" == record.action && "test.task" == record.taskTypeName && 64 == record.argumentsDigest.Length() );
        CORE::CString canonical;
        UInt32 bytes = 0;
        ASSERT_TRUE( record.argumentsDigest == DRNUTILS::CDrnUtilsDigest::DataNodeDigest( payload, bytes ) );
        ASSERT_TRUE( 0 > record.reason.HasSubstr( "CONFIDENTIAL" ) && 0 > record.argumentsDigest.HasSubstr( "CONFIDENTIAL" ) );

        // not allowlisted by the host (the source would take it), unknown, oversized
        request.SetString( 1, "submit-2" );
        request.SetString( 4, "other.task" );
        ASSERT_TRUE( COMCORE::RPC_STATUS_FAILED_PRECONDITION == w.rpc.Call( P_SUBMIT, request ).status.code );
        request.SetString( 1, "submit-3" );
        request.SetString( 4, "unknown.task" );
        ASSERT_TRUE( COMCORE::RPC_STATUS_FAILED_PRECONDITION == w.rpc.Call( P_SUBMIT, request ).status.code );
        request.SetString( 1, "submit-4" );
        request.SetString( 4, "test.task" );
        CORE::CString big;
        for ( UInt32 i=0; i<100; ++i )
            big += "0123456789";
        payload.SetAttribute( "blob", big );
        request.SetDataNode( 5, payload );
        ASSERT_TRUE( COMCORE::RPC_STATUS_RESOURCE_EXHAUSTED == w.rpc.Call( P_SUBMIT, request ).status.code );
        ASSERT_TRUE( 1 == w.source.submitted.size() );
        // a bad cron expression
        payload.DelAttribute( "blob" );
        request.SetDataNode( 5, payload );
        request.SetString( 1, "submit-5" );
        request.GetMutableMessage( 6 )->SetString( 2, "not a cron" );
        ASSERT_TRUE( COMCORE::RPC_STATUS_INVALID_ARGUMENT == w.rpc.Call( P_SUBMIT, request ).status.code );

        // the submit role only: no control
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == w.Control( "c1", CORE::JOBCONTROLACTION_ENABLE, CORE::JOBCONTROLTARGET_JOB, "coded" ).status.code );
        // REMOVE_JOB: the submitted job yes, the job defined in code no
        w.source.RaiseChanged();
        PumpFor( 20 );
        ASSERT_TRUE( COMCORE::RPC_STATUS_FAILED_PRECONDITION == w.Control( "rm-1", CORE::JOBCONTROLACTION_REMOVE_JOB, CORE::JOBCONTROLTARGET_JOB, "coded" ).status.code );
        ASSERT_TRUE( w.Control( "rm-2", CORE::JOBCONTROLACTION_REMOVE_JOB, CORE::JOBCONTROLTARGET_JOB, "fetch-aapl" ).status.IsOk() );
        ASSERT_TRUE( GUCEF_NULL == w.source.FindJob( "fetch-aapl" ) );

        // without the submit role
        REQUIRE_TRUE( w.Connect( "scheduler.view,scheduler.control" ) );
        request.SetString( 1, "submit-6" );
        request.GetMutableMessage( 6 )->SetString( 2, "0 3 * * *" );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == w.rpc.Call( P_SUBMIT, request ).status.code );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Custom actions: declared by the source or the service handler and allowed on the host; undeclared ones UNIMPLEMENTED; per job only its remote custom actions" )
    try
    {
        CSchedulerWorld w;
        CORE::SJobStatus& job = w.source.AddJob( "market-data" );
        job.remoteCustomActions.insert( "rebuild" );
        DRNUTILS::SSchedulerActionInfo rebuild;
        rebuild.action = CORE::JOBCONTROLACTION_CUSTOM;
        rebuild.targetKind = CORE::JOBCONTROLTARGET_JOB;
        rebuild.customName = "rebuild";
        w.source.declaredCustom.push_back( rebuild );
        CFakeCustomActionHandler handler;
        REQUIRE_TRUE( w.Start() );
        w.host->SetCustomActionHandler( &handler );
        REQUIRE_TRUE( w.Connect( "scheduler.view,scheduler.control" ) );
        COMCORE::CRpcCallResult info = w.rpc.Call( P_GETINFO, w.rpc.NewRequest( P_GETINFO ) );
        UInt32 customs = 0;
        for ( UInt32 a=0; a<info.response.GetCount( 6 ); ++a )
            if ( CORE::JOBCONTROLACTION_CUSTOM == info.response.GetMessageAt( 6, a )->GetEnum( 1 ) )
                ++customs;
        ASSERT_TRUE( 2 == customs );

        COMCORE::CRpcMessage request = w.rpc.NewRequest( P_CONTROL );
        request.SetString( 1, "x1" );
        request.SetEnum( 2, CORE::JOBCONTROLACTION_CUSTOM );
        request.SetEnum( 3, CORE::JOBCONTROLTARGET_HOST );
        request.SetString( 4, "node1" );
        request.SetString( 5, "watchlist-add" );
        request.SetMapEntry( 6, CORE::CVariant( "symbol" ), CORE::CVariant( "IBM" ) );
        COMCORE::CRpcCallResult byHandler = w.rpc.Call( P_CONTROL, request );
        REQUIRE_TRUE( byHandler.status.IsOk() );
        ASSERT_TRUE( 1 == handler.calls && "IBM" == handler.lastArguments[ "symbol" ] );
        ASSERT_TRUE( 0 == w.source.GetApplyCalls() );
        CORE::CDataNode detail;
        ASSERT_TRUE( byHandler.response.GetDataNode( 6, detail ) && "1" == detail.GetAttributeValue( "added" ).AsString() );

        request.SetString( 1, "x2" );
        request.SetEnum( 3, CORE::JOBCONTROLTARGET_JOB );
        request.SetString( 4, "market-data" );
        request.SetString( 5, "rebuild" );
        ASSERT_TRUE( w.rpc.Call( P_CONTROL, request ).status.IsOk() );
        ASSERT_TRUE( 1 == w.source.GetApplyCalls() );

        CORE::CString::StringSet allowed = w.settings.GetAllowedCustomActions();
        request.SetString( 1, "x3" );
        request.SetString( 5, "unknown" );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == w.rpc.Call( P_CONTROL, request ).status.code );
        // allowed on the host but declared by nobody
        CSchedulerWorld w2;
        w2.source.AddJob( "j1" );
        REQUIRE_TRUE( w2.Start() );
        REQUIRE_TRUE( w2.Connect( "scheduler.view,scheduler.control" ) );
        COMCORE::CRpcMessage undeclared = w2.rpc.NewRequest( P_CONTROL );
        undeclared.SetString( 1, "y1" );
        undeclared.SetEnum( 2, CORE::JOBCONTROLACTION_CUSTOM );
        undeclared.SetEnum( 3, CORE::JOBCONTROLTARGET_HOST );
        undeclared.SetString( 5, "rebuild" );
        ASSERT_TRUE( COMCORE::RPC_STATUS_UNIMPLEMENTED == w2.rpc.Call( P_CONTROL, undeclared ).status.code );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RU-T3 Watch: RESET, snapshot and SNAPSHOT_END, then live changes in revision order; removals; resume from the host revision on a new session; host restart gives RESET" )
    try
    {
        CSchedulerWorld w;
        w.source.AddJob( "a" );
        w.source.AddJob( "b" );
        REQUIRE_TRUE( w.Start() );
        REQUIRE_TRUE( w.Connect( "scheduler.view" ) );
        COMCORE::CRpcMessage request = w.rpc.NewRequest( P_WATCH );
        request.SetBool( 3, true );
        COMCORE::CIRpcStreamPtr stream = w.rpc.OpenStream( P_WATCH, request );
        REQUIRE_TRUE( !stream.IsNULL() );
        CStreamRecorder recorder;
        recorder.Watch( stream );
        CCounterAtLeast snapshot( recorder.count, 6 );
        REQUIRE_TRUE( PumpUntil( snapshot, 3000 ) );
        // RESET, 2 jobs, 2 lanes, SNAPSHOT_END
        ASSERT_TRUE( 4 == recorder.messages[ 0 ].GetEnum( 1 ) );
        ASSERT_TRUE( 5 == recorder.messages[ 5 ].GetEnum( 1 ) );
        ASSERT_TRUE( 2 == CountKind( recorder.messages, 1 ) && 2 == CountKind( recorder.messages, 2 ) );
        UInt64 cursor = recorder.messages[ 5 ].GetUInt64( 2 );
        UInt64 instance = recorder.messages[ 5 ].GetUInt64( 6 );
        ASSERT_TRUE( cursor == w.source.revision && 1001 == instance );

        w.source.Touch( "b" );
        w.source.Touch( "a" );
        UInt64 run = w.source.StartRun( "a", "drn:alice" );
        w.source.RaiseChanged();
        w.source.RemoveJob( "b" );
        CCounterAtLeast live( recorder.count, 6 + 4 );
        REQUIRE_TRUE( PumpUntil( live, 3000 ) );
        PumpFor( 50 );
        UInt64 last = cursor;
        bool removedB = false;
        bool sawRun = false;
        for ( UInt32 i=6; i<recorder.messages.size(); ++i )
        {
            const COMCORE::CRpcMessage& change = recorder.messages[ i ];
            ASSERT_TRUE( change.GetUInt64( 2 ) >= last );
            last = change.GetUInt64( 2 );
            if ( "b" == change.GetString( 7 ) )
                removedB = true;
            if ( 3 == change.GetEnum( 1 ) && GUCEF_NULL != change.GetMessage( 5 ) && run == change.GetMessage( 5 )->GetUInt64( 1 ) )
                sawRun = true;
        }
        ASSERT_TRUE( removedB && sawRun );
        cursor = last;

        // a new session: resume after the cursor gives only what changed since
        stream->Cancel( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_CANCELLED, "test" ) );
        w.rpc.Disconnect();
        PumpFor( 50 );
        w.source.Touch( "a" );
        REQUIRE_TRUE( w.Connect( "scheduler.view" ) );
        request = w.rpc.NewRequest( P_WATCH );
        request.SetUInt64( 1, instance );
        request.SetUInt64( 2, cursor );
        stream = w.rpc.OpenStream( P_WATCH, request );
        CStreamRecorder resumed;
        resumed.Watch( stream );
        CCounterAtLeast one( resumed.count, 1 );
        REQUIRE_TRUE( PumpUntil( one, 3000 ) );
        PumpFor( 100 );
        ASSERT_TRUE( 0 == CountKind( resumed.messages, 4 ) );
        ASSERT_TRUE( 1 == resumed.messages.size() && "a" == resumed.messages[ 0 ].GetMessage( 3 )->GetString( 1 ) );

        // the scheduler restarts: RESET and a new snapshot on the open stream
        w.source.Restart();
        CCounterAtLeast reset( resumed.count, 2 );
        REQUIRE_TRUE( PumpUntil( reset, 3000 ) );
        PumpFor( 50 );
        ASSERT_TRUE( 4 == resumed.messages[ 1 ].GetEnum( 1 ) && 1002 == resumed.messages[ 1 ].GetUInt64( 6 ) );
        // a cursor of the old instance gives RESET as well
        request.SetUInt64( 1, instance );
        COMCORE::CIRpcStreamPtr stale = w.rpc.OpenStream( P_WATCH, request );
        CStreamRecorder staleRecorder;
        staleRecorder.Watch( stale );
        CCounterAtLeast staleOne( staleRecorder.count, 1 );
        REQUIRE_TRUE( PumpUntil( staleOne, 3000 ) );
        ASSERT_TRUE( 4 == staleRecorder.messages[ 0 ].GetEnum( 1 ) );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RU-T3 a viewer without credit for 10 000 changes holds at most the rows pending, and the scheduler is not slowed; stream limits per session" )
    try
    {
        CSchedulerWorld w;
        for ( UInt32 i=0; i<20; ++i )
            w.source.AddJob( "job" + CORE::ToString( i ) );
        w.settings.SetMaxWatchStreamsPerSession( 2 );
        REQUIRE_TRUE( w.Start() );
        REQUIRE_TRUE( w.Connect( "scheduler.view" ) );
        COMCORE::CIRpcStreamPtr stream = w.rpc.OpenStream( P_WATCH, w.rpc.NewRequest( P_WATCH ) );
        REQUIRE_TRUE( !stream.IsNULL() );
        stream->SetManualCredit( true );
        PumpFor( 100 );
        UInt64 start = MT::PrecisionTickCount();
        for ( UInt32 c=0; c<10000; ++c )
        {
            w.source.Touch( "job" + CORE::ToString( c % 20 ), false );
            if ( 0 == c % 100 )
            {
                w.source.RaiseChanged();
                w.rpc.pulseGenerator->ForceDirectPulse();
            }
        }
        w.source.RaiseChanged();
        Float64 elapsedMs = (Float64) ( MT::PrecisionTickCount() - start ) * 1000.0 / (Float64) MT::PrecisionTimerResolution();
        PumpFor( 100 );
        // drain: what is delivered after the credit is the initial snapshot plus at most the latest of each row
        COMCORE::CRpcMessage message;
        COMCORE::CRpcStatus status;
        UInt32 received = 0;
        stream->GrantCredit( 100000 );
        while ( stream->ReadNext( message, status, 300 ) )
            ++received;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RU-T3 credit: 10000 changes took " << elapsedMs << " ms, the viewer received " << received << " messages" );
        ASSERT_TRUE( received <= 1 + 20 + 2 + 1 + 20 + 1 );
        ASSERT_TRUE( elapsedMs < 10000.0 );

        COMCORE::CIRpcStreamPtr second = w.rpc.OpenStream( P_WATCH, w.rpc.NewRequest( P_WATCH ) );
        COMCORE::CIRpcStreamPtr third = w.rpc.OpenStream( P_WATCH, w.rpc.NewRequest( P_WATCH ) );
        CStreamRecorder thirdRecorder;
        thirdRecorder.Watch( third );
        class CEnded : public CCondition { public: CStreamRecorder& r; CEnded( CStreamRecorder& rec ) : r( rec ) {} virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return r.ended; } };
        CEnded ended( thirdRecorder );
        ASSERT_TRUE( PumpUntil( ended, 3000 ) );
        ASSERT_TRUE( COMCORE::RPC_STATUS_RESOURCE_EXHAUSTED == thirdRecorder.status.code );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RU-T3 journal overflow and pending overflow give RESET and a snapshot (bounded memory for any viewer)" )
    try
    {
        CSchedulerWorld w;
        for ( UInt32 i=0; i<10; ++i )
            w.source.AddJob( "job" + CORE::ToString( i ) );
        w.settings.SetJournalSize( 4 );
        w.settings.SetMaxPendingPerStream( 3 );
        REQUIRE_TRUE( w.Start() );
        REQUIRE_TRUE( w.Connect( "scheduler.view" ) );
        UInt64 cursor = w.source.revision;
        for ( UInt32 i=0; i<6; ++i )
            w.source.Touch( "job" + CORE::ToString( i ) );
        COMCORE::CRpcMessage request = w.rpc.NewRequest( P_WATCH );
        request.SetUInt64( 1, 1001 );
        request.SetUInt64( 2, cursor );
        COMCORE::CIRpcStreamPtr stream = w.rpc.OpenStream( P_WATCH, request );
        CStreamRecorder recorder;
        recorder.Watch( stream );
        CCounterAtLeast first( recorder.count, 1 );
        REQUIRE_TRUE( PumpUntil( first, 3000 ) );
        ASSERT_TRUE( 4 == recorder.messages[ 0 ].GetEnum( 1 ) );     // the journal no longer covers the cursor
        PumpFor( 50 );

        // more pending changes than allowed while the stream has no credit: RESET instead of a long queue
        stream->SetManualCredit( true );
        UInt64 before = recorder.count;
        for ( UInt32 i=0; i<10; ++i )
            w.source.Touch( "job" + CORE::ToString( i ), false );
        w.source.RaiseChanged();
        stream->GrantCredit( 1000 );
        PumpFor( 200 );
        bool sawReset = false;
        for ( UInt32 i=(UInt32)before; i<recorder.messages.size(); ++i )
            if ( 4 == recorder.messages[ i ].GetEnum( 1 ) )
                sawReset = true;
        ASSERT_TRUE( sawReset );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Lock rules 9.1: no lock of the host is held while the source applies a command or while the audit sink runs (probes from other threads), under control from 6 threads" )
    try
    {
        CSchedulerWorld w;
        w.source.AddJob( "j1" );
        REQUIRE_TRUE( w.Start( false ) );
        w.audit.callBack = w.host;
        w.source.probeHost = w.host;
        CORE::SJobControlCommand command;
        command.action = CORE::JOBCONTROLACTION_ENABLE;
        command.targetKind = CORE::JOBCONTROLTARGET_JOB;
        command.targetId = "j1";
        CORE::SJobControlResult result;
        bool replayed = false;
        ASSERT_TRUE( DRNUTILS::DRNUTILS_STATUS_OK == w.host->Control( LocalPrincipal( "scheduler.control" ), "probe-1", command, result, replayed ) );
        ASSERT_TRUE( 1 == w.source.probeOk && 0 == w.source.probeFailed );
        w.source.probeHost = GUCEF_NULL;

        // control from several threads while the test thread delivers changes (and the host refreshes its model)
        w.settings.SetMaxControlCommandsPerMinutePerSubject( 0 );
        GUCEF_DELETE w.host;
        w.host = GUCEF_NULL;
        w.settings.SetMaxControlCommandsPerMinutePerSubject( 0 );
        REQUIRE_TRUE( w.Start( false ) );
        w.audit.callBack = w.host;
        CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "drnutils.test.control", true );
        GUCEF::vector< CControlTask* > tasks;
        GUCEF::vector< CORE::CTaskConsumerPtr > owners;
        for ( UInt32 t=0; t<6; ++t )
        {
            CControlTask* task = GUCEF_NEW CControlTask( w.host, t );
            CORE::CTSharedPtr< CControlTask, MT::CMutex > owner( task );
            CORE::CTaskConsumerPtr consumer;
            consumer = owner;
            owners.push_back( consumer );
            tasks.push_back( task );
            REQUIRE_TRUE( pool->StartTaskWithConsumer( consumer ).HasAFuture() );
        }
        UInt64 start = MT::PrecisionTickCount();
        UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
        bool allDone = false;
        while ( !allDone && ( MT::PrecisionTickCount() - start ) / ticksPerMs < 60000 )
        {
            w.source.Touch( "j1" );
            PumpFor( 2 );
            allDone = true;
            for ( UInt32 t=0; t<tasks.size(); ++t )
                if ( 0 == tasks[ t ]->done.Load() )
                    allDone = false;
        }
        for ( UInt32 t=0; t<tasks.size(); ++t )
            tasks[ t ]->WaitForTaskToFinish( 10000 );
        ASSERT_TRUE( allDone );
        UInt32 total = 0;
        for ( UInt32 t=0; t<tasks.size(); ++t )
            total += tasks[ t ]->ok.Load();
        ASSERT_TRUE( 1200 == total );
        ASSERT_TRUE( 1200 <= w.audit.Count() );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Access rules: CDrnUtilsAccessRules gives CRpcRoleAccessPolicy the method rules of the role map (the investment mapping: view -> viewer, control and submit -> admin)" )
    try
    {
        DRNUTILS::CDrnUtilsRoleMap roles;
        CORE::CString::StringSet viewer;
        viewer.insert( "viewer" );
        CORE::CString::StringSet admin;
        admin.insert( "admin" );
        roles.SetRoles( DRNUTILS::CDrnUtilsRoleMap::SchedulerViewRole, viewer );
        roles.SetRoles( DRNUTILS::CDrnUtilsRoleMap::TasksViewRole, viewer );
        roles.SetRoles( DRNUTILS::CDrnUtilsRoleMap::SchedulerControlRole, admin );
        roles.SetRoles( DRNUTILS::CDrnUtilsRoleMap::SchedulerSubmitRole, admin );
        roles.SetRoles( DRNUTILS::CDrnUtilsRoleMap::TasksControlRole, admin );
        COMCORE::CRpcRoleAccessPolicy policy;
        DRNUTILS::CDrnUtilsAccessRules::AddTo( policy, roles );
        COMCORE::CRpcContractPtr scheduler = DRNUTILS::CSchedulerContracts::GetContract();
        COMCORE::CRpcContractPtr tasks = DRNUTILS::CTaskManagerContracts::GetContract();
        COMCORE::CRpcPrincipal v = CDrnUtilsRpcWorld::Principal( "drn:v", "viewer" );
        COMCORE::CRpcPrincipal a = CDrnUtilsRpcWorld::Principal( "drn:a", "admin" );
        COMCORE::CRpcPrincipal n = CDrnUtilsRpcWorld::Principal( "drn:n", "nobody" );
        const COMCORE::CRpcServiceDescriptor* service = scheduler->FindService( "Scheduler" );
        const COMCORE::CRpcServiceDescriptor* taskService = tasks->FindService( "TaskManager" );
        REQUIRE_TRUE( GUCEF_NULL != service && GUCEF_NULL != taskService );
        COMCORE::CRpcMetadata metadata;
        CORE::CString reason;
        ASSERT_TRUE( COMCORE::RPC_ACCESS_ALLOW == policy.AuthorizeCall( v, *scheduler, *service, *service->FindMethodByName( "ListJobs" ), metadata, reason ) );
        ASSERT_TRUE( COMCORE::RPC_ACCESS_DENY == policy.AuthorizeCall( v, *scheduler, *service, *service->FindMethodByName( "Control" ), metadata, reason ) );
        ASSERT_TRUE( COMCORE::RPC_ACCESS_DENY == policy.AuthorizeCall( v, *scheduler, *service, *service->FindMethodByName( "SubmitJob" ), metadata, reason ) );
        ASSERT_TRUE( COMCORE::RPC_ACCESS_ALLOW == policy.AuthorizeCall( a, *scheduler, *service, *service->FindMethodByName( "Control" ), metadata, reason ) );
        ASSERT_TRUE( COMCORE::RPC_ACCESS_ALLOW == policy.AuthorizeCall( a, *scheduler, *service, *service->FindMethodByName( "SubmitJob" ), metadata, reason ) );
        ASSERT_TRUE( COMCORE::RPC_ACCESS_DENY == policy.AuthorizeCall( n, *scheduler, *service, *service->FindMethodByName( "GetInfo" ), metadata, reason ) );
        ASSERT_TRUE( COMCORE::RPC_ACCESS_ALLOW == policy.AuthorizeCall( v, *tasks, *taskService, *taskService->FindMethodByName( "ListScheduledJobs" ), metadata, reason ) );
        ASSERT_TRUE( COMCORE::RPC_ACCESS_DENY == policy.AuthorizeCall( v, *tasks, *taskService, *taskService->FindMethodByName( "Control" ), metadata, reason ) );
        ASSERT_TRUE( COMCORE::RPC_ACCESS_ALLOW == policy.AuthorizeCall( a, *tasks, *taskService, *taskService->FindMethodByName( "Control" ), metadata, reason ) );

        // the policy on the endpoint refuses before the host is asked (defense in depth: the host checks again)
        CSchedulerWorld w;
        w.source.AddJob( "j1" );
        w.settings.SetRoleMap( roles );
        REQUIRE_TRUE( w.Start() );
        COMCORE::CRpcRoleAccessPolicy* serverPolicy = GUCEF_NEW COMCORE::CRpcRoleAccessPolicy();
        DRNUTILS::CDrnUtilsAccessRules::AddTo( *serverPolicy, roles );
        COMCORE::CIRpcAccessPolicyPtr policyPtr( serverPolicy );
        REQUIRE_TRUE( w.Connect( "viewer" ) );
        w.rpc.server->SetAccessPolicy( policyPtr );
        ASSERT_TRUE( w.rpc.Call( P_LISTJOBS, w.rpc.NewRequest( P_LISTJOBS ) ).status.IsOk() );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == w.Control( "c1", CORE::JOBCONTROLACTION_ENABLE, CORE::JOBCONTROLTARGET_JOB, "j1" ).status.code );
        ASSERT_TRUE( 0 == w.audit.Count() );   // refused by the policy: the host never saw it
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Settings: SaveConfig and LoadConfig round trip every setting, the role map and the action names" )
    try
    {
        DRNUTILS::CSchedulerHostSettings settings;
        settings.SetExposure( DRNUTILS::DRNUTILS_EXPOSURE_CONTROL );
        CORE::TJobControlActionSet allowed;
        allowed.insert( CORE::JOBCONTROLACTION_RUN_NOW );
        allowed.insert( CORE::JOBCONTROLACTION_CANCEL_RUN );
        settings.SetAllowedActions( allowed );
        CORE::CString::StringSet set;
        set.insert( "a" );
        settings.SetAllowedCustomActions( set );
        settings.SetSubmittableTaskTypes( set );
        settings.SetTablePrefix( "svc" );
        settings.SetMaxCommands( 77 );
        settings.SetAllowInsecureControl( true );
        DRNUTILS::CDrnUtilsRoleMap roles;
        CORE::CString::StringSet admin;
        admin.insert( "admin" );
        roles.SetRoles( DRNUTILS::CDrnUtilsRoleMap::SchedulerControlRole, admin );
        roles.SetControlRolesForAction( CORE::JOBCONTROLACTION_RUN_NOW, admin );
        settings.SetRoleMap( roles );
        CORE::CDataNode config;
        REQUIRE_TRUE( settings.SaveConfig( config ) );
        DRNUTILS::CSchedulerHostSettings loaded;
        REQUIRE_TRUE( loaded.LoadConfig( config ) );
        ASSERT_TRUE( DRNUTILS::DRNUTILS_EXPOSURE_CONTROL == loaded.GetExposure() );
        ASSERT_TRUE( loaded.GetAllowedActions() == allowed );
        ASSERT_TRUE( loaded.GetAllowedCustomActions() == set && loaded.GetSubmittableTaskTypes() == set );
        ASSERT_TRUE( "svc" == loaded.GetTablePrefix() && 77 == loaded.GetMaxCommands() && loaded.GetAllowInsecureControl() );
        ASSERT_TRUE( loaded.GetRoleMap().GetRoles( DRNUTILS::CDrnUtilsRoleMap::SchedulerControlRole ) == admin );
        ASSERT_TRUE( loaded.GetRoleMap().GetControlRolesForAction( CORE::JOBCONTROLACTION_RUN_NOW ) == admin );

        DRNUTILS::CTaskManagerHostSettings taskSettings;
        taskSettings.SetExposure( DRNUTILS::DRNUTILS_EXPOSURE_VIEW );
        taskSettings.SetControllablePools( set );
        taskSettings.SetSampleIntervalMs( 10 );
        ASSERT_TRUE( 1000 == taskSettings.GetSampleIntervalMs() );
        taskSettings.SetMaxAllowedThreads( -1 );
        CORE::CDataNode taskConfig;
        REQUIRE_TRUE( taskSettings.SaveConfig( taskConfig ) );
        DRNUTILS::CTaskManagerHostSettings taskLoaded;
        REQUIRE_TRUE( taskLoaded.LoadConfig( taskConfig ) );
        ASSERT_TRUE( DRNUTILS::DRNUTILS_EXPOSURE_VIEW == taskLoaded.GetExposure() && taskLoaded.GetControllablePools() == set && -1 == taskLoaded.GetMaxAllowedThreads() );

        // the floor of the sample intervals: 1000 ms by default, lower only when set (tests), never 0
        DRNUTILS::CTaskManagerHostSettings fastSettings;
        ASSERT_TRUE( 1000 == fastSettings.GetMinSampleIntervalMs() );
        fastSettings.SetMinSampleIntervalMs( 50 );
        fastSettings.SetSampleIntervalMs( 10 );
        ASSERT_TRUE( 50 == fastSettings.GetSampleIntervalMs() );
        CORE::CDataNode fastConfig;
        REQUIRE_TRUE( fastSettings.SaveConfig( fastConfig ) );
        DRNUTILS::CTaskManagerHostSettings fastLoaded;
        REQUIRE_TRUE( fastLoaded.LoadConfig( fastConfig ) );
        ASSERT_TRUE( 50 == fastLoaded.GetMinSampleIntervalMs() && 50 == fastLoaded.GetSampleIntervalMs() );
        fastLoaded.SetMinSampleIntervalMs( 0 );
        ASSERT_TRUE( 1 == fastLoaded.GetMinSampleIntervalMs() );
        fastLoaded.SetMinSampleIntervalMs( 2000 );
        ASSERT_TRUE( 2000 == fastLoaded.GetSampleIntervalMs() );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Client facade: CSchedulerClient issues RPC calls with fresh command ids, never blocks, raises InfoReceived and CommandCompleted (with the RESULT of waitForRun)" )
    try
    {
        CSchedulerWorld w;
        w.source.AddJob( "j1" );
        REQUIRE_TRUE( w.Start() );
        COMCORE::CRpcPrincipal principal = CDrnUtilsRpcWorld::Principal( "drn:alice", "scheduler.view,scheduler.control,scheduler.submit" );
        REQUIRE_TRUE( w.rpc.Connect( principal, false, true ) );
        DRNUTILS::CSchedulerClient client( w.rpc.pulseGenerator );
        class CClientRecorder : public CORE::CObserver
        {
            public:
            UInt64 infos;
            UInt64 commands;
            UInt64 details;
            GUCEF::vector< DRNUTILS::SDrnUtilsCommandOutcome > outcomes;
            GUCEF::vector< DRNUTILS::SSchedulerJobDetailAnswer > detailAnswers;
            DRNUTILS::SSchedulerHostInfo lastInfo;
            CClientRecorder( void ) : infos( 0 ), commands( 0 ), details( 0 ), outcomes(), detailAnswers(), lastInfo() {}
            virtual ~CClientRecorder() { SignalUpcomingObserverDestruction(); }
            virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
            {
                if ( DRNUTILS::CSchedulerClient::JobDetailReceivedEvent == eventid )
                {
                    ++details;
                    detailAnswers.push_back( static_cast< DRNUTILS::CSchedulerClient::TJobDetailReceivedEventData* >( eventdata )->GetData() );
                }
                if ( DRNUTILS::CSchedulerClient::InfoReceivedEvent == eventid )
                {
                    ++infos;
                    lastInfo = static_cast< DRNUTILS::CSchedulerClient::TInfoReceivedEventData* >( eventdata )->GetData();
                }
                if ( DRNUTILS::CSchedulerClient::CommandCompletedEvent == eventid )
                {
                    ++commands;
                    outcomes.push_back( static_cast< DRNUTILS::CSchedulerClient::TCommandCompletedEventData* >( eventdata )->GetData() );
                }
            }
        };
        CClientRecorder recorder;
        client.Subscribe( &recorder );
        ASSERT_TRUE( client.GetLastInfo().hostName.IsNULLOrEmpty() && client.GetLastInfo().allowedActions.empty() );
        client.SetEndpoint( w.rpc.client );
        ASSERT_TRUE( client.IsConnected() );
        // C-3: valid right after SetEndpoint (nothing asked yet: the empty info)
        ASSERT_TRUE( client.GetLastInfo().hostName.IsNULLOrEmpty() && 0 == client.GetLastInfo().revision );
        CORE::CString a = DRNUTILS::CSchedulerClient::NewCommandId();
        CORE::CString b = DRNUTILS::CSchedulerClient::NewCommandId();
        ASSERT_TRUE( 32 == a.Length() && a != b );
        REQUIRE_TRUE( client.RequestInfo() );
        CCounterAtLeast info( recorder.infos, 1 );
        REQUIRE_TRUE( PumpUntil( info, 3000 ) );
        ASSERT_TRUE( "node1" == client.GetLastInfo().hostName && !client.GetLastInfo().allowedActions.empty() );
        ASSERT_TRUE( 1 == client.GetLastInfo().submittableTaskTypes.size() );
        // C-3: InfoReceivedEvent carries the whole host info
        const DRNUTILS::SSchedulerHostInfo& got = client.GetLastInfo();
        ASSERT_TRUE( recorder.lastInfo.hostName == got.hostName && recorder.lastInfo.hostInstanceId == got.hostInstanceId && 0 != got.hostInstanceId );
        ASSERT_TRUE( recorder.lastInfo.revision == got.revision && recorder.lastInfo.state == got.state && !got.state.IsNULLOrEmpty() );
        ASSERT_TRUE( recorder.lastInfo.allowedActions.size() == got.allowedActions.size() && recorder.lastInfo.tableNames == got.tableNames );
        ASSERT_TRUE( recorder.lastInfo.submittableTaskTypes == got.submittableTaskTypes && recorder.lastInfo.maxSubmitPayloadBytes == got.maxSubmitPayloadBytes && 0 != got.maxSubmitPayloadBytes );
        ASSERT_TRUE( 0 != recorder.lastInfo.serverTimeUtcMs );

        UInt64 start = MT::PrecisionTickCount();
        CORE::CString commandId = client.RunNow( "j1", "a test", true );
        Float64 sendMs = (Float64) ( MT::PrecisionTickCount() - start ) * 1000.0 / (Float64) MT::PrecisionTimerResolution();
        ASSERT_TRUE( !commandId.IsNULLOrEmpty() );
        ASSERT_TRUE( sendMs < 100.0 );
        PumpFor( 100 );
        ASSERT_TRUE( 0 == recorder.commands );
        REQUIRE_TRUE( 1 == w.source.runs.size() );
        ASSERT_TRUE( "a test" == w.source.applied.back().reason );
        w.source.EndRun( w.source.runs[ 0 ].runId, CORE::JOBRUNSTATE_SUCCEEDED );
        CCounterAtLeast done( recorder.commands, 1 );
        REQUIRE_TRUE( PumpUntil( done, 3000 ) );
        ASSERT_TRUE( commandId == recorder.outcomes[ 0 ].commandId && DRNUTILS::DRNUTILS_STATUS_OK == recorder.outcomes[ 0 ].status );
        ASSERT_TRUE( recorder.outcomes[ 0 ].hasRun && CORE::JOBRUNSTATE_SUCCEEDED == recorder.outcomes[ 0 ].run.state );

        // a refused command completes with its status
        client.SetJobEnabled( "nope", false, "x" );
        CCounterAtLeast refused( recorder.commands, 2 );
        REQUIRE_TRUE( PumpUntil( refused, 3000 ) );
        ASSERT_TRUE( DRNUTILS::DRNUTILS_STATUS_NOT_FOUND == recorder.outcomes[ 1 ].status );

        // a job as data
        CORE::SJobSubmission submission;
        submission.definition.SetId( "submitted-1" );
        submission.definition.SetTaskTypeName( "test.task" );
        CORE::CJobTrigger trigger;
        trigger.SetKind( CORE::JOBTRIGGER_INTERVAL );
        trigger.SetPeriodMs( 60000 );
        submission.definition.SetTrigger( trigger );
        ASSERT_FALSE( client.SubmitJob( submission ).IsNULLOrEmpty() );
        CCounterAtLeast submitted( recorder.commands, 3 );
        REQUIRE_TRUE( PumpUntil( submitted, 3000 ) );
        ASSERT_TRUE( DRNUTILS::DRNUTILS_STATUS_OK == recorder.outcomes[ 2 ].status );
        ASSERT_TRUE( 60000 == w.source.submitted.back().definition.GetTrigger().GetPeriodMs() );

        // C-3: the id Control() returns is the one CommandCompletedEvent carries
        CORE::SJobControlCommand disable;
        disable.action = CORE::JOBCONTROLACTION_DISABLE;
        disable.targetKind = CORE::JOBCONTROLTARGET_JOB;
        disable.targetId = "j1";
        disable.reason = "direct";
        CORE::CString disableId = client.Control( disable );
        ASSERT_FALSE( disableId.IsNULLOrEmpty() );
        CCounterAtLeast disabled( recorder.commands, 4 );
        REQUIRE_TRUE( PumpUntil( disabled, 3000 ) );
        ASSERT_TRUE( disableId == recorder.outcomes[ 3 ].commandId && DRNUTILS::DRNUTILS_STATUS_OK == recorder.outcomes[ 3 ].status );

        // C-2: the detail of a job (GetJob), matched by its request id; a refusal arrives as well
        CORE::CString detailId = client.RequestJobDetail( "j1", 5, true );
        ASSERT_FALSE( detailId.IsNULLOrEmpty() );
        CORE::CString missingId = client.RequestJobDetail( "nope", 5, false );
        ASSERT_FALSE( missingId.IsNULLOrEmpty() );
        ASSERT_TRUE( client.RequestJobDetail( "" ).IsNULLOrEmpty() );
        CCounterAtLeast detailed( recorder.details, 2 );
        REQUIRE_TRUE( PumpUntil( detailed, 3000 ) );
        PumpFor( 50 );
        REQUIRE_TRUE( 2 == recorder.detailAnswers.size() );
        const DRNUTILS::SSchedulerJobDetailAnswer* found = GUCEF_NULL;
        const DRNUTILS::SSchedulerJobDetailAnswer* missing = GUCEF_NULL;
        for ( UInt32 i=0; i<recorder.detailAnswers.size(); ++i )
        {
            if ( recorder.detailAnswers[ i ].requestId == detailId )
                found = &recorder.detailAnswers[ i ];
            if ( recorder.detailAnswers[ i ].requestId == missingId )
                missing = &recorder.detailAnswers[ i ];
        }
        REQUIRE_TRUE( GUCEF_NULL != found && GUCEF_NULL != missing );
        ASSERT_TRUE( DRNUTILS::DRNUTILS_STATUS_OK == found->status && "j1" == found->jobId && "j1" == found->detail.job.jobId );
        ASSERT_TRUE( "the description of j1" == found->detail.job.description );
        ASSERT_TRUE( 1 == found->detail.recentRuns.size() && CORE::JOBRUNSTATE_SUCCEEDED == found->detail.recentRuns[ 0 ].state );
        ASSERT_TRUE( "3" == found->detail.sourceStatus.GetAttributeValueOrChildValueByName( "pending" ).AsString() );
        ASSERT_FALSE( found->allowedActions.empty() );
        ASSERT_TRUE( DRNUTILS::DRNUTILS_STATUS_NOT_FOUND == missing->status && "nope" == missing->jobId );

        // without an endpoint nothing is sent
        client.SetEndpoint( COMCORE::CRpcEndpointPtr() );
        ASSERT_TRUE( client.RunNow( "j1", "x" ).IsNULLOrEmpty() );
        ASSERT_FALSE( client.RequestInfo() );
        ASSERT_TRUE( client.RequestJobDetail( "j1" ).IsNULLOrEmpty() );
        client.Unsubscribe( &recorder );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
