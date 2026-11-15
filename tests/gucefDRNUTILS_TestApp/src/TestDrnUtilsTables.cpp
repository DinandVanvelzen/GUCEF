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

#include <algorithm>

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CTABLECODECREGISTRY_H
#include "gucefCORE_CTableCodecRegistry.h"
#define GUCEF_CORE_CTABLECODECREGISTRY_H
#endif /* GUCEF_CORE_CTABLECODECREGISTRY_H ? */

#ifndef GUCEF_DRN_CDRNPUBLISHEDTABLE_H
#include "gucefDRN_CDRNPublishedTable.h"
#define GUCEF_DRN_CDRNPUBLISHEDTABLE_H
#endif /* GUCEF_DRN_CDRNPUBLISHEDTABLE_H ? */

#ifndef GUCEF_DRN_CDRNTABLEPUBLISHER_H
#include "gucefDRN_CDRNTablePublisher.h"
#define GUCEF_DRN_CDRNTABLEPUBLISHER_H
#endif /* GUCEF_DRN_CDRNTABLEPUBLISHER_H ? */

#ifndef GUCEF_DRN_CDRNDATAGRID_H
#include "gucefDRN_CDRNDataGrid.h"
#define GUCEF_DRN_CDRNDATAGRID_H
#endif /* GUCEF_DRN_CDRNDATAGRID_H ? */

#ifndef GUCEF_DRN_CIDRNACCESSPOLICY_H
#include "gucefDRN_CIDRNAccessPolicy.h"
#define GUCEF_DRN_CIDRNACCESSPOLICY_H
#endif /* GUCEF_DRN_CIDRNACCESSPOLICY_H ? */

#ifndef GUCEF_DRN_CIDRNAUTHENTICATOR_H
#include "gucefDRN_CIDRNAuthenticator.h"
#define GUCEF_DRN_CIDRNAUTHENTICATOR_H
#endif /* GUCEF_DRN_CIDRNAUTHENTICATOR_H ? */

#ifndef GUCEF_DRNUTILS_H
#include "gucefDRNUTILS.h"
#define GUCEF_DRNUTILS_H
#endif /* GUCEF_DRNUTILS_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H
#include "TestDrnUtilsRpcWorld.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSDRNGROUP_H
#include "TestDrnUtilsDrnGroup.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSDRNGROUP_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSDRNGROUP_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKESCHEDULERSOURCE_H
#include "TestDrnUtilsFakeSchedulerSource.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKESCHEDULERSOURCE_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKESCHEDULERSOURCE_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKETASKMANAGERSOURCE_H
#include "TestDrnUtilsFakeTaskManagerSource.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKETASKMANAGERSOURCE_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSFAKETASKMANAGERSOURCE_H ? */

#include "TestDrnUtilsTables.h"

using namespace GUCEF;
using namespace DRNUTILSTEST;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define REQUIRE_TRUE(t)  do { if ( !(t) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #t, __FILE__, __LINE__ ); throw std::exception(); } } while ( 0 )

namespace {

/** The whole table written by the CSV table codec */
bool
ExportCsv( CORE::CIDataTable& table, CORE::CString& text )
{
    CORE::CTableCodecRegistry::TTableCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetTableCodecRegistry().TryGetCsvCodec( codec ) || codec.IsNULL() )
        return false;
    CORE::CDynamicBuffer csv;
    CORE::CDynamicBufferAccess* access = GUCEF_NEW CORE::CDynamicBufferAccess( &csv, false );
    CORE::IOAccessPtr destination = access->CreateSharedPtr();
    CORE::CITableWriterPtr writer;
    CORE::CString error;
    if ( CORE::DATATABLE_STATUS_OK != codec->CreateWriter( destination, table.GetSchema(), CORE::CDataTableWriterOptions(), writer, error ) )
        return false;
    CORE::CDataTableScanPtr scan;
    CORE::CDataTableQuery query;
    if ( CORE::DATATABLE_STATUS_OK != table.Scan( query, scan ) || scan.IsNULL() )
        return false;
    CORE::CDataTableBatchPtr batch;
    while ( CORE::DATATABLE_STATUS_OK == scan->NextBatch( batch ) && !batch.IsNULL() )
    {
        if ( CORE::DATATABLE_STATUS_OK != writer->WriteBatch( *batch ) )
            return false;
    }
    CORE::CDataTableFileSummary summary;
    if ( CORE::DATATABLE_STATUS_OK != writer->Finish( summary ) )
        return false;
    text = CORE::CString( (const char*) csv.GetConstBufferPtr(), csv.GetDataSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The lines of a CSV text in order: the row order of two tables may differ */
CORE::CString
SortedLines( const CORE::CString& text )
{
    CORE::CString::StringVector lines = text.ParseElements( '\n', false );
    std::sort( lines.begin(), lines.end() );
    CORE::CString sorted;
    for ( UInt32 i=0; i<lines.size(); ++i )
        sorted += lines[ i ] + "\n";
    return sorted;
}

/*-------------------------------------------------------------------------*/

/** What a published grid holds as seen by a member: the rows (keys not starting with '#') and all value bytes */
UInt32
GridRows( DRN::CDRNSession& session, const CORE::CString& gridName, CORE::CString* allBytes = GUCEF_NULL )
{
    MT::CScopeMutex lock( session.GetNode().GetLock() );
    DRN::CDRNDataGrid* grid = session.FindGrid( gridName );
    if ( GUCEF_NULL == grid )
        return 0;
    UInt32 rows = 0;
    UInt32 count = grid->GetKeyCount();
    for ( UInt32 i=0; i<count; ++i )
    {
        CORE::CVariant key = grid->GetKeyByIndex( i );
        CORE::CVariant value;
        if ( !grid->GetValueByIndex( i, value ) || !value.IsInitialized() )
            continue;
        if ( !DRN::CDRNTablePublisher::IsReservedKey( key ) )
            ++rows;
        if ( GUCEF_NULL != allBytes )
        {
            CORE::CDynamicBuffer bytes = value.AsBuffer();
            for ( UInt32 b=0; b<bytes.GetDataSize(); ++b )
            {
                char c = ( (const char*) bytes.GetConstBufferPtr() )[ b ];
                *allBytes += ( c >= 32 && c < 127 ) ? c : '.';
            }
            *allBytes += key.AsString() + "\n";
        }
    }
    return rows;
}

/*-------------------------------------------------------------------------*/

UInt32
CountRows( CORE::CIDataTable& table )
{
    CORE::CDataTableScanPtr scan;
    CORE::CDataTableQuery query;
    CORE::TDataTableStatus status = table.Scan( query, scan );
    if ( CORE::DATATABLE_STATUS_OK != status || scan.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CountRows: scan of " << table.GetName() << " gave " << (Int32) status << ": " << table.GetLastErrorMessage() );
        return 0;
    }
    UInt32 rows = 0;
    CORE::CDataTableBatchPtr batch;
    while ( CORE::DATATABLE_STATUS_OK == ( status = scan->NextBatch( batch ) ) && !batch.IsNULL() )
        rows += batch->GetRowCount();
    if ( CORE::DATATABLE_STATUS_END != status )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CountRows: batches of " << table.GetName() << " ended with " << (Int32) status << ": " << table.GetLastErrorMessage() );
    return rows;
}

/*-------------------------------------------------------------------------*/

CORE::CIDataTable&
Table( const CORE::CIDataTablePtr& table )
{
    return *const_cast< CORE::CIDataTable* >( table.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

DRN::CDRNPublishedTable*
Published( const CORE::CIDataTablePtr& table )
{
    return static_cast< DRN::CDRNPublishedTable* >( &Table( table ) );
}

/*-------------------------------------------------------------------------*/

class CPageAtLeast : public CCondition
{
    public:
    DRN::CDRNPublishedTable* t;
    UInt64 revision;
    CPageAtLeast( DRN::CDRNPublishedTable* table, UInt64 r ) : t( table ), revision( r ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return t->IsValid() && t->GetPageRevision() >= revision && t->IsConsistent(); }
};

/*-------------------------------------------------------------------------*/

class CGridAnnounced : public CCondition
{
    public:
    DRN::CDRNSession& s;
    CORE::CString name;
    CGridAnnounced( DRN::CDRNSession& session, const CORE::CString& grid ) : s( session ), name( grid ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex lock( s.GetNode().GetLock() );
        return GUCEF_NULL != s.FindGrid( name );
    }
};

/*-------------------------------------------------------------------------*/

/** A service's policy for its session: the DRNUTILS grids by the grid rules, the rest as the default */
class CGridRulePolicy : public DRN::CDRNDefaultAccessPolicy
{
    public:
    DRNUTILS::CDrnUtilsGridAccessRules rules;
    UInt32 refusals;
    CGridRulePolicy( void ) : DRN::CDRNDefaultAccessPolicy(), rules(), refusals( 0 ) {}
    virtual bool MaySubscribeAs( UInt32 peerId, const DRN::CDRNAuthResult& auth, const CORE::CString& itemName ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !rules.IsGoverned( itemName ) )
            return DRN::CDRNDefaultAccessPolicy::MaySubscribeAs( peerId, auth, itemName );
        CORE::CString::StringSet roles;
        CORE::CString::StringVector list = auth.role.ParseElements( ',', false );
        for ( UInt32 i=0; i<list.size(); ++i )
            roles.insert( list[ i ].Trim( true ).Trim( false ) );
        bool may = rules.MaySubscribe( roles, itemName );
        if ( !may )
            ++refusals;
        return may;
    }
    virtual bool MayWriteAs( UInt32 peerId, const DRN::CDRNAuthResult& auth, const CORE::CString& itemName, UInt32 keyIndex ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( rules.IsGoverned( itemName ) )
            return false;
        return DRN::CDRNDefaultAccessPolicy::MayWriteAs( peerId, auth, itemName, keyIndex );
    }
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

void
PerformDrnUtilsTablesTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnUtilsTables" );

    GUCEF_TESTFW_TESTCASE( "RU-T11 the jobs, lanes and runs grids arrive typed on a replica; hidden jobs and private item keys are never published; the replica equals the local snapshot table (CSV)" )
    try
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        TThreadId previousDriver = pulseGenerator->GetPulseDriverThreadId();
        pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
        {
            CDrnUtilsGroup group( "drnutils-tables" );
            REQUIRE_TRUE( group.Start( 2 ) );
            CFakeSchedulerSource source( pulseGenerator );
            source.AddLane( "lane1" );
            source.AddJob( "visible-1" );
            source.AddJob( "visible-2" );
            source.AddJob( "hidden", "lane1", CORE::JOBFLAG_HIDDEN );
            source.AddJob( "sensitive", "lane1", CORE::JOBFLAG_SENSITIVE );
            CORE::CString longTitle;
            for ( UInt32 i=0; i<2000; ++i )
                longTitle += "0123456789";
            source.AddJob( "long" ).title = longTitle;
            source.StartRun( "visible-1", "drn:bob", "PRIVATE-ITEM-KEY" );
            source.StartRun( "sensitive", "drn:bob" );
            source.StartRun( "hidden", "drn:bob" );
            UInt64 done = source.StartRun( "visible-2", "drn:bob" );
            source.EndRun( done, CORE::JOBRUNSTATE_SUCCEEDED, false );

            DRNUTILS::CSchedulerHostSettings settings;
            settings.SetExposure( DRNUTILS::DRNUTILS_EXPOSURE_VIEW );
            settings.SetTablePrefix( "svc" );
            DRNUTILS::CSchedulerHost* host = GUCEF_NEW DRNUTILS::CSchedulerHost( source, settings, pulseGenerator );
            CORE::CString error;
            ASSERT_TRUE( host->PublishTables( "drnutils-tables", error ) );
            CORE::CString::StringMap names;
            host->GetTableNames( names );
            ASSERT_TRUE( "svc/jobs" == names[ "jobs" ] && "svc/lanes" == names[ "lanes" ] && "svc/runs" == names[ "runs" ] );

            CGridAnnounced announced( group[ 1 ], "svc/runs" );
            REQUIRE_TRUE( PumpUntil( announced, 10000 ) );
            REQUIRE_TRUE( group.Register( 1 ) );
            DRNUTILS::CSchedulerTableSet tables;
            REQUIRE_TRUE( DRNUTILS::CDrnUtilsTableBinder::OpenSchedulerTables( "drnutils-tables", "svc", tables, error ) );
            REQUIRE_TRUE( tables.IsComplete() );
            CPageAtLeast jobsArrived( Published( tables.GetJobs() ), 1 );
            CPageAtLeast runsArrived( Published( tables.GetRuns() ), 1 );
            CPageAtLeast lanesArrived( Published( tables.GetLanes() ), 1 );
            REQUIRE_TRUE( PumpUntil( jobsArrived, 10000 ) && PumpUntil( runsArrived, 10000 ) && PumpUntil( lanesArrived, 10000 ) );

            // the rows on the replica, as the grid holds them
            CORE::CString jobBytes, runBytes;
            ASSERT_TRUE( 4 == GridRows( group[ 1 ], "svc/jobs", &jobBytes ) );
            ASSERT_TRUE( 1 == GridRows( group[ 1 ], "svc/lanes" ) );
            ASSERT_TRUE( 3 == GridRows( group[ 1 ], "svc/runs", &runBytes ) );
            ASSERT_TRUE( 0 > jobBytes.HasSubstr( "hidden" ) );
            ASSERT_TRUE( 0 > runBytes.HasSubstr( "PRIVATE-ITEM-KEY" ) );
            ASSERT_TRUE( 0 <= runBytes.HasSubstr( "[redacted]" ) );
            ASSERT_TRUE( 0 > jobBytes.HasSubstr( longTitle.SubstrToIndex( 300 ) ) );   // cut to its bound, the row was not refused
            // typed by the published schema
            REQUIRE_TRUE( Published( tables.GetJobs() )->IsValid() );
            ASSERT_TRUE( "jobId" == Table( tables.GetJobs() ).GetSchema()->GetColumn( 0 ).name );
            ASSERT_TRUE( CORE::DATATABLE_TYPE_TIMESTAMP == Table( tables.GetJobs() ).GetSchema()->GetColumn( DRNUTILS::CJobsTableSchema::COLUMN_NEXT_DUE_UTC ).type );

            // the local binding without DRN: the same rows, redacted the same way
            DRNUTILS::CSchedulerTableSet local;
            REQUIRE_TRUE( DRNUTILS::CDrnUtilsTableBinder::OpenLocalSchedulerTables( source, pulseGenerator, local, error ) );
            CORE::CString localJobs, localRuns;
            REQUIRE_TRUE( ExportCsv( Table( local.GetJobs() ), localJobs ) && ExportCsv( Table( local.GetRuns() ), localRuns ) );
            ASSERT_TRUE( 4 == CountRows( Table( local.GetJobs() ) ) && 3 == CountRows( Table( local.GetRuns() ) ) );
            ASSERT_TRUE( 0 > localRuns.HasSubstr( "PRIVATE-ITEM-KEY" ) && 0 <= localRuns.HasSubstr( "[redacted]" ) );

            // the replica equals the local snapshot table (B-5: a grid named with '/' is scanned through the replica)
            CORE::CDataTableScanPtr scan;
            CORE::TDataTableStatus scanStatus = Table( tables.GetJobs() ).Scan( CORE::CDataTableQuery(), scan );
            if ( CORE::DATATABLE_STATUS_OK != scanStatus )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RU-T11: the replica of svc/jobs cannot be scanned: " << Table( tables.GetJobs() ).GetLastErrorMessage() );
            ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == scanStatus );
            ASSERT_TRUE( "svc/jobs" == Table( tables.GetJobs() ).GetName() );
            CORE::CString jobsCsv, runsCsv, lanesCsv, localLanes;
            REQUIRE_TRUE( ExportCsv( Table( tables.GetJobs() ), jobsCsv ) && ExportCsv( Table( tables.GetRuns() ), runsCsv ) );
            REQUIRE_TRUE( ExportCsv( Table( tables.GetLanes() ), lanesCsv ) && ExportCsv( Table( local.GetLanes() ), localLanes ) );
            ASSERT_TRUE( SortedLines( localJobs ) == SortedLines( jobsCsv ) );
            ASSERT_TRUE( SortedLines( localRuns ) == SortedLines( runsCsv ) );
            ASSERT_TRUE( SortedLines( localLanes ) == SortedLines( lanesCsv ) );
            ASSERT_TRUE( 4 == CountRows( Table( tables.GetJobs() ) ) && 3 == CountRows( Table( tables.GetRuns() ) ) && 1 == CountRows( Table( tables.GetLanes() ) ) );

            // an unchanged model publishes nothing; changes within one publish interval go out as one page
            UInt64 page = Published( tables.GetJobs() )->GetPageRevision();
            PumpFor( 2500 );
            ASSERT_TRUE( page == Published( tables.GetJobs() )->GetPageRevision() );
            for ( UInt32 t=0; t<5; ++t )
                source.Touch( "visible-1" );
            CPageAtLeast next( Published( tables.GetJobs() ), page + 1 );
            REQUIRE_TRUE( PumpUntil( next, 5000 ) );
            PumpFor( 1500 );
            ASSERT_TRUE( page + 1 == Published( tables.GetJobs() )->GetPageRevision() );

            REQUIRE_TRUE( group.Register( 0 ) );
            tables.Clear();
            local.Clear();
            GUCEF_DELETE host;
        }
        pulseGenerator->SetPulseDriverThreadId( previousDriver );
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

    GUCEF_TESTFW_TESTCASE( "Local binding: CSchedulerSnapshotTable costs nothing while unobserved, follows the source once observed and raises RowsChangedEvent with the changed keys; it is read only" )
    try
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        TThreadId previousDriver = pulseGenerator->GetPulseDriverThreadId();
        pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
        {
            CFakeSchedulerSource source( pulseGenerator );
            source.AddJob( "a" );
            source.AddJob( "b" );
            DRNUTILS::CSchedulerTableSet local;
            CORE::CString error;
            REQUIRE_TRUE( DRNUTILS::CDrnUtilsTableBinder::OpenLocalSchedulerTables( source, pulseGenerator, local, error ) );
            UInt32 calls = source.snapshotCalls;
            for ( UInt32 t=0; t<10; ++t )
                source.Touch( "a" );
            PumpFor( 20 );
            ASSERT_TRUE( calls == source.snapshotCalls );     // nobody observes, nobody scans: no copy taken
            ASSERT_TRUE( 2 == CountRows( Table( local.GetJobs() ) ) );

            class CRowsRecorder : public CORE::CObserver
            {
                public:
                GUCEF::vector< CORE::CVariant > keys;
                UInt64 events;
                CRowsRecorder( void ) : keys(), events( 0 ) {}
                virtual ~CRowsRecorder() { SignalUpcomingObserverDestruction(); }
                virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
                {
                    if ( CORE::CIDataTable::RowsChangedEvent == eventid && GUCEF_NULL != eventdata )
                    {
                        ++events;
                        const GUCEF::vector< CORE::CVariant >& changed = static_cast< CORE::TDataTableRowsChangedEventData* >( eventdata )->GetData();
                        keys.insert( keys.end(), changed.begin(), changed.end() );
                    }
                }
            };
            CRowsRecorder recorder;
            Table( local.GetJobs() ).Subscribe( &recorder );
            source.Touch( "b" );
            CCounterAtLeast changed( recorder.events, 1 );
            REQUIRE_TRUE( PumpUntil( changed, 2000 ) );
            ASSERT_TRUE( 1 == recorder.keys.size() && "b" == recorder.keys[ 0 ].AsString() );
            source.RemoveJob( "a" );
            CCounterAtLeast removed( recorder.events, 2 );
            REQUIRE_TRUE( PumpUntil( removed, 2000 ) );
            ASSERT_TRUE( 1 == CountRows( Table( local.GetJobs() ) ) );
            CORE::CDataTableWriteResult result;
            CORE::CDataTableQuery all;
            ASSERT_TRUE( Table( local.GetJobs() ).IsReadOnly() );
            ASSERT_TRUE( CORE::DATATABLE_STATUS_READ_ONLY == Table( local.GetJobs() ).DeleteRows( all, result ) );
            Table( local.GetJobs() ).Unsubscribe( &recorder );
            local.Clear();
        }
        pulseGenerator->SetPulseDriverThreadId( previousDriver );
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

    GUCEF_TESTFW_TESTCASE( "RU-T11 task tables: pools, threads, tasks and scheduled jobs are published while a viewer watches and emptied after the viewing grace" )
    try
    {
        CDrnUtilsRpcWorld rpc;
        {
            CDrnUtilsGroup group( "drnutils-tasks" );
            REQUIRE_TRUE( group.Start( 2 ) );
            CFakeTaskManagerSource source;
            source.AddPool( "work", 2 );
            source.AddTask( 11, "work", "t", 501 );
            CORE::CScheduledJobInfo job;
            job.SetSchedulerName( "node1" );
            job.SetJobId( "j" );
            source.scheduledJobs.push_back( job );
            DRNUTILS::CTaskManagerHostSettings settings;
            settings.SetExposure( DRNUTILS::DRNUTILS_EXPOSURE_VIEW );
            settings.SetTablePrefix( "svc" );
            settings.SetViewingGraceMs( 300 );
            DRNUTILS::CTaskManagerHost* host = GUCEF_NEW DRNUTILS::CTaskManagerHost( source, settings, rpc.pulseGenerator );
            CORE::CString error;
            REQUIRE_TRUE( host->Expose( rpc.services, error ) );
            REQUIRE_TRUE( host->PublishTables( "drnutils-tasks", error ) );
            CGridAnnounced announced( group[ 1 ], "svc/scheduledjobs" );
            REQUIRE_TRUE( PumpUntil( announced, 10000 ) );
            REQUIRE_TRUE( group.Register( 1 ) );
            DRNUTILS::CTaskManagerTableSet tables;
            REQUIRE_TRUE( DRNUTILS::CDrnUtilsTableBinder::OpenTaskManagerTables( "drnutils-tasks", "svc", tables, error ) );
            PumpFor( 1500 );
            ASSERT_TRUE( 0 == source.GetCounterCalls() );
            REQUIRE_TRUE( rpc.Connect( CDrnUtilsRpcWorld::Principal( "drn:alice", "tasks.view" ), true, false ) );
            COMCORE::CRpcMessage request = rpc.NewRequest( "gucef.drnutils.tasks.TaskManager/Watch" );
            request.SetUInt32( 1, 1000 );
            COMCORE::CIRpcStreamPtr stream = rpc.OpenStream( "gucef.drnutils.tasks.TaskManager/Watch", request );
            REQUIRE_TRUE( !stream.IsNULL() );
            CPageAtLeast tasksArrived( Published( tables.GetTasks() ), 1 );
            CPageAtLeast poolsArrived( Published( tables.GetPools() ), 1 );
            REQUIRE_TRUE( PumpUntil( tasksArrived, 8000 ) && PumpUntil( poolsArrived, 8000 ) );
            PumpFor( 300 );
            ASSERT_TRUE( 1 == GridRows( group[ 1 ], "svc/pools" ) && 1 == GridRows( group[ 1 ], "svc/tasks" ) );
            ASSERT_TRUE( 1 == GridRows( group[ 1 ], "svc/threads" ) && 1 == GridRows( group[ 1 ], "svc/scheduledjobs" ) );
            stream->Cancel( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_CANCELLED, "closed" ) );
            PumpFor( 1500 );
            ASSERT_FALSE( host->IsInViewingMode() );
            ASSERT_TRUE( 0 == GridRows( group[ 1 ], "svc/tasks" ) && 0 == GridRows( group[ 1 ], "svc/pools" ) );
            rpc.Disconnect();
            REQUIRE_TRUE( group.Register( 0 ) );
            tables.Clear();
            GUCEF_DELETE host;
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

    GUCEF_TESTFW_TESTCASE( "RU-T13 grid access rules: patterns and roles decide subscriptions, writes are always refused; a member without the view role cannot subscribe to the grids" )
    try
    {
        DRNUTILS::CDrnUtilsRoleMap roles;
        CORE::CString::StringSet viewer;
        viewer.insert( "viewer" );
        roles.SetRoles( DRNUTILS::CDrnUtilsRoleMap::SchedulerViewRole, viewer );
        DRNUTILS::CDrnUtilsGridAccessRules rules;
        rules.SetRoleMap( roles );
        rules.AddGridPattern( "svc/*", DRNUTILS::CDrnUtilsRoleMap::SchedulerViewRole );
        CORE::CString::StringSet none;
        CORE::CString::StringSet asViewer;
        asViewer.insert( "viewer" );
        ASSERT_TRUE( rules.IsGoverned( "svc/jobs" ) && rules.IsGoverned( "svc/" ) && !rules.IsGoverned( "other/jobs" ) && !rules.IsGoverned( "svc" ) );
        ASSERT_TRUE( rules.MaySubscribe( asViewer, "svc/jobs" ) );
        ASSERT_FALSE( rules.MaySubscribe( none, "svc/jobs" ) );
        ASSERT_FALSE( rules.MaySubscribe( asViewer, "other/jobs" ) );
        ASSERT_FALSE( rules.MayWrite( asViewer, "svc/jobs" ) );
        CGridRulePolicy policy;
        policy.rules = rules;
        DRN::CDRNAuthResult auth;
        auth.role = "viewer, editor";
        ASSERT_TRUE( policy.MaySubscribeAs( 2, auth, "svc/runs" ) );
        ASSERT_FALSE( policy.MayWriteAs( 2, auth, "svc/runs", 0 ) );
        auth.role = "editor";
        ASSERT_FALSE( policy.MaySubscribeAs( 2, auth, "svc/runs" ) );

        // in a session: the host's node takes the policy, an anonymous member (no role) gets none of the grids
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        TThreadId previousDriver = pulseGenerator->GetPulseDriverThreadId();
        pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
        {
            CDrnUtilsGroup group( "drnutils-rules" );
            REQUIRE_TRUE( group.AddNode() );
            group.nodes[ 0 ]->SetAccessPolicy( &policy );
            DRN::CDRNSessionSettings hostSettings;
            hostSettings.sessionName = "drnutils-rules";
            REQUIRE_TRUE( group.sessions[ 0 ]->Host( hostSettings ) );
            REQUIRE_TRUE( group.AddNode() );
            DRN::CDRNSessionSettings joinSettings;
            joinSettings.sessionName = "drnutils-rules";
            REQUIRE_TRUE( group.sessions[ 1 ]->Join( DRN::CDRNEndpointList( group.endpoints[ 0 ] ), joinSettings ) );
            PumpFor( 1500 );
            REQUIRE_TRUE( group.Register( 0 ) );
            CFakeSchedulerSource source( pulseGenerator );
            source.AddJob( "j" );
            DRNUTILS::CSchedulerHostSettings settings;
            settings.SetExposure( DRNUTILS::DRNUTILS_EXPOSURE_VIEW );
            settings.SetTablePrefix( "svc" );
            DRNUTILS::CSchedulerHost* host = GUCEF_NEW DRNUTILS::CSchedulerHost( source, settings, pulseGenerator );
            CORE::CString error;
            REQUIRE_TRUE( host->PublishTables( "drnutils-rules", error ) );
            CGridAnnounced announced( group[ 1 ], "svc/jobs" );
            bool seen = PumpUntil( announced, 3000 );
            if ( seen )
            {
                MT::CScopeMutex lock( group.nodes[ 1 ]->GetLock() );
                group.sessions[ 1 ]->Subscribe( "svc/jobs" );
            }
            PumpFor( 1500 );
            DRN::CDRNDataGrid* replica = GUCEF_NULL;
            {
                MT::CScopeMutex lock( group.nodes[ 1 ]->GetLock() );
                replica = group.sessions[ 1 ]->FindGrid( "svc/jobs" );
            }
            UInt32 keys = 0;
            if ( GUCEF_NULL != replica )
            {
                MT::CScopeMutex lock( group.nodes[ 1 ]->GetLock() );
                keys = replica->GetKeyCount();
            }
            ASSERT_TRUE( 0 == keys );
            ASSERT_TRUE( policy.refusals > 0 || !seen );
            GUCEF_DELETE host;
            group.nodes[ 0 ]->SetAccessPolicy( GUCEF_NULL );
        }
        pulseGenerator->SetPulseDriverThreadId( previousDriver );
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
