/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COMCORE_CDATABASECLIENTFACTORY_H
#include "gucefCOMCORE_CDatabaseClientFactory.h"
#define GUCEF_COMCORE_CDATABASECLIENTFACTORY_H
#endif /* GUCEF_COMCORE_CDATABASECLIENTFACTORY_H ? */

#ifndef GUCEF_COMCORE_CDATABASETABLERESULT_H
#include "gucefCOMCORE_CDatabaseTableResult.h"
#define GUCEF_COMCORE_CDATABASETABLERESULT_H
#endif /* GUCEF_COMCORE_CDATABASETABLERESULT_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDatabaseClientFake.h"

#include "TestDatabaseClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::COMCORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef CORE::CTFactory< CIDatabaseClient, CTestFakeDatabaseClient, MT::CMutex > TTestFakeDatabaseClientFactory;
TTestFakeDatabaseClientFactory g_testFakeDatabaseClientFactory;

/*-------------------------------------------------------------------------*/

/** Counts the events of a database client and keeps the last event data; one handler per event */
class CDatabaseEventRecorder : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CDatabaseEventRecorder > TEventCallback;

    UInt32 opened;
    UInt32 closed;
    UInt32 errors;
    UInt32 created;
    UInt32 dropped;
    CORE::CString lastError;
    CORE::CString lastTable;

    CDatabaseEventRecorder( void )
        : CORE::CObserver()
        , opened( 0 ), closed( 0 ), errors( 0 ), created( 0 ), dropped( 0 )
        , lastError(), lastTable()
    {GUCEF_TRACE;

    }

    virtual ~CDatabaseEventRecorder()
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
    }

    void Attach( CORE::CNotifier* client )
    {GUCEF_TRACE;

        TEventCallback openedCallback( this, &CDatabaseEventRecorder::OnOpened );
        SubscribeTo( client, CIDatabaseClient::DatabaseOpenedEvent, openedCallback );
        TEventCallback closedCallback( this, &CDatabaseEventRecorder::OnClosed );
        SubscribeTo( client, CIDatabaseClient::DatabaseClosedEvent, closedCallback );
        TEventCallback errorCallback( this, &CDatabaseEventRecorder::OnError );
        SubscribeTo( client, CIDatabaseClient::DatabaseErrorEvent, errorCallback );
        TEventCallback createdCallback( this, &CDatabaseEventRecorder::OnCreated );
        SubscribeTo( client, CIDatabaseClient::TableCreatedEvent, createdCallback );
        TEventCallback droppedCallback( this, &CDatabaseEventRecorder::OnDropped );
        SubscribeTo( client, CIDatabaseClient::TableDroppedEvent, droppedCallback );
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "CDatabaseEventRecorder";
        return classTypeName;
    }

    private:

    void OnOpened( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {GUCEF_TRACE;

        ++opened;
    }

    void OnClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {GUCEF_TRACE;

        ++closed;
    }

    void OnError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {GUCEF_TRACE;

        ++errors;
        if ( GUCEF_NULL != eventData )
            lastError = static_cast< CIDatabaseClient::TDatabaseErrorEventData* >( eventData )->GetData();
    }

    void OnCreated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {GUCEF_TRACE;

        ++created;
        if ( GUCEF_NULL != eventData )
            lastTable = static_cast< CIDatabaseClient::TTableEventData* >( eventData )->GetData();
    }

    void OnDropped( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {GUCEF_TRACE;

        ++dropped;
        if ( GUCEF_NULL != eventData )
            lastTable = static_cast< CIDatabaseClient::TTableEventData* >( eventData )->GetData();
    }
};

/*-------------------------------------------------------------------------*/

CDatabaseClientSettings
FakeSettings( const char* database = "testdb" )
{
    CDatabaseClientSettings settings;
    settings.backendName = CTestFakeDatabaseClient::BackendName;
    settings.database = database;
    return settings;
}

/*-------------------------------------------------------------------------*/

CORE::CDataTableDefinition
Definition( const char* name )
{
    CORE::CDataTableDefinition definition;
    definition.name = name;
    definition.codecType = "csv";
    definition.schema.AddColumn( CORE::CDataTableColumnDef( "id", CORE::DATATABLE_TYPE_INT64, false ) );
    definition.schema.AddColumn( CORE::CDataTableColumnDef( "label", CORE::DATATABLE_TYPE_UTF8, true ) );
    return definition;
}

/*-------------------------------------------------------------------------*/

/** Waits for the future to end; false after a timeout */
bool
Finished( const CORE::CFutureResult& future )
{
    CORE::CTaskPtr task = future.GetResult( 20000 );
    return !task.IsNULL() && task->IsTaskInEndState();
}

/*-------------------------------------------------------------------------*/

CORE::CString
ExtraInfoOf( const CORE::CFutureResult& future )
{
    CORE::CTaskPtr task = future.GetResult();
    return task.IsNULL() ? CORE::CString() : task->GetTaskStatusExtraInfo();
}

/*-------------------------------------------------------------------------*/

CORE::TTaskStatus
StatusOf( const CORE::CFutureResult& future )
{
    CORE::CTaskPtr task = future.GetResult();
    return task.IsNULL() ? CORE::TTaskStatus::TASKSTATUS_UNDEFINED : task->GetTaskStatus();
}

/*-------------------------------------------------------------------------*/

CTestFakeDatabaseClient*
AsFake( CIDatabaseClientPtr& client )
{
    return static_cast< CTestFakeDatabaseClient* >( client.GetPointerAlways() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDatabaseClientTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefCOMCORE.DatabaseClient" );

    CDatabaseClientFactory& factory = CComCoreGlobal::Instance()->GetDatabaseClientFactory();

    GUCEF_TESTFW_TESTCASE( "DatabaseClientFactory: a backend registers by name and creates clients by name, unknown names give none" )
    try
    {
        ASSERT_FALSE( factory.IsConstructible( "fake" ) );
        ASSERT_TRUE( factory.Create( "fake" ).IsNULL() );

        factory.RegisterConcreteFactory( "fake", &g_testFakeDatabaseClientFactory );
        ASSERT_TRUE( factory.IsConstructible( "fake" ) );
        ASSERT_FALSE( factory.IsConstructible( "Fake" ) );
        ASSERT_FALSE( factory.IsConstructible( "vfstables" ) );

        CIDatabaseClientPtr first = factory.Create( "fake" );
        CIDatabaseClientPtr second = factory.Create( "fake" );
        ASSERT_FALSE( first.IsNULL() );
        ASSERT_FALSE( second.IsNULL() );
        ASSERT_TRUE( first.GetPointerAlways() != second.GetPointerAlways() );
        ASSERT_TRUE( first->GetBackendName() == "fake" );
        ASSERT_FALSE( first->IsOpen() );
        ASSERT_TRUE( factory.Create( "vfstables" ).IsNULL() );
        ASSERT_TRUE( factory.Create( "" ).IsNULL() );

        factory.UnregisterConcreteFactory( "fake" );
        ASSERT_FALSE( factory.IsConstructible( "fake" ) );
        ASSERT_TRUE( factory.Create( "fake" ).IsNULL() );
        ASSERT_FALSE( first.IsNULL() );     // a client made earlier lives on
        ASSERT_TRUE( first->GetBackendName() == "fake" );

        factory.RegisterConcreteFactory( "fake", &g_testFakeDatabaseClientFactory );
        ASSERT_FALSE( factory.Create( "fake" ).IsNULL() );
        factory.UnregisterConcreteFactory( "fake" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DatabaseClientFactory: the global holds one factory that is the same on every access" )
    try
    {
        ASSERT_TRUE( &CComCoreGlobal::Instance()->GetDatabaseClientFactory() == &factory );
        ASSERT_TRUE( &CComCoreGlobal::Instance()->GetDatabaseClientFactory() != GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DatabaseClientSettings: defaults" )
    try
    {
        CDatabaseClientSettings settings;
        ASSERT_TRUE( settings.backendName.IsNULLOrEmpty() );
        ASSERT_TRUE( settings.database.IsNULLOrEmpty() );
        ASSERT_FALSE( settings.readOnly );
        ASSERT_TRUE( settings.threadPoolName == "DataTables" );
        ASSERT_TRUE( 512ULL * 1024 * 1024 == settings.bufferPoolMaxBytes );
        ASSERT_TRUE( 0 == settings.backendOptions.GetKeyCount() );
        ASSERT_TRUE( settings.pulseGenerator.IsNULL() );
        ASSERT_TRUE( settings.threadPool.IsNULL() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DatabaseClientSettings: serialization round trip keeps every serialized member" )
    try
    {
        CDatabaseClientSettings settings;
        settings.backendName = "vfstables";
        settings.database = "marketdata";
        settings.readOnly = true;
        settings.threadPoolName = "MarketPool";
        settings.bufferPoolMaxBytes = 123456789;
        settings.backendOptions.Set( "footerCacheBytes", CORE::CVariant( CORE::CString( "1048576" ) ) );
        settings.backendOptions.Set( "mode", CORE::CVariant( CORE::CString( "strict" ) ) );

        CORE::CDataNode node;
        ASSERT_TRUE( settings.Serialize( node, CORE::CDataNodeSerializableSettings() ) );

        CDatabaseClientSettings loaded;
        ASSERT_TRUE( loaded.Deserialize( node, CORE::CDataNodeSerializableSettings() ) );
        ASSERT_TRUE( loaded == settings );
        ASSERT_TRUE( loaded.backendName == "vfstables" );
        ASSERT_TRUE( loaded.database == "marketdata" );
        ASSERT_TRUE( loaded.readOnly );
        ASSERT_TRUE( loaded.threadPoolName == "MarketPool" );
        ASSERT_TRUE( 123456789 == loaded.bufferPoolMaxBytes );
        ASSERT_TRUE( 2 == loaded.backendOptions.GetKeyCount() );
        ASSERT_TRUE( loaded.backendOptions.GetValueAlways( "footerCacheBytes" ).AsString() == "1048576" );
        ASSERT_TRUE( loaded.backendOptions.GetValueAlways( "mode" ).AsString() == "strict" );

        // the clone is a copy
        CORE::CICloneable* clone = settings.Clone();
        ASSERT_TRUE( GUCEF_NULL != clone );
        ASSERT_TRUE( *static_cast< CDatabaseClientSettings* >( clone ) == settings );
        GUCEF_DELETE clone;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DatabaseClientSettings: a node without settings leaves them as they were, absent members keep their defaults" )
    try
    {
        CDatabaseClientSettings settings = FakeSettings();
        settings.readOnly = true;

        CORE::CDataNode empty;
        ASSERT_FALSE( settings.Deserialize( empty, CORE::CDataNodeSerializableSettings() ) );
        ASSERT_TRUE( settings.backendName == "fake" );
        ASSERT_TRUE( settings.readOnly );

        CORE::CDataNode partial;
        partial.SetAttribute( "backendName", CORE::CString( "fake" ) );
        partial.SetAttribute( "database", CORE::CString( "other" ) );
        CDatabaseClientSettings loaded;
        ASSERT_TRUE( loaded.Deserialize( partial, CORE::CDataNodeSerializableSettings() ) );
        ASSERT_TRUE( loaded.database == "other" );
        ASSERT_FALSE( loaded.readOnly );
        ASSERT_TRUE( loaded.threadPoolName == "DataTables" );
        ASSERT_TRUE( CDatabaseClientSettings::DefaultBufferPoolMaxBytes == loaded.bufferPoolMaxBytes );

        // what the code gave stays: it is not serialized
        CDatabaseClientSettings withPool = FakeSettings();
        withPool.threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "SettingsRoundTripPool" );
        ASSERT_FALSE( withPool.threadPool.IsNULL() );
        ASSERT_TRUE( withPool.Deserialize( partial, CORE::CDataNodeSerializableSettings() ) );
        ASSERT_FALSE( withPool.threadPool.IsNULL() );
        ASSERT_TRUE( withPool.database == "other" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DatabaseClientSettings: Validate names what is missing, equality ignores the pointers" )
    try
    {
        CORE::CString error;
        CDatabaseClientSettings settings;
        ASSERT_FALSE( settings.Validate( error ) );
        ASSERT_TRUE( error.HasSubstr( "backend" ) >= 0 );

        settings.backendName = "fake";
        ASSERT_FALSE( settings.Validate( error ) );
        ASSERT_TRUE( error.HasSubstr( "database" ) >= 0 );

        settings.database = "x";
        ASSERT_TRUE( settings.Validate( error ) );

        settings.bufferPoolMaxBytes = 0;
        ASSERT_FALSE( settings.Validate( error ) );
        settings.bufferPoolMaxBytes = 1;
        settings.threadPoolName.Clear();
        ASSERT_FALSE( settings.Validate( error ) );

        CDatabaseClientSettings a = FakeSettings();
        CDatabaseClientSettings b = FakeSettings();
        ASSERT_TRUE( a == b );
        b.threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( "SettingsRoundTripPool" );
        ASSERT_TRUE( a == b );
        b.backendOptions.Set( "k", CORE::CVariant( CORE::CString( "v" ) ) );
        ASSERT_TRUE( a != b );
        a.backendOptions.Set( "k", CORE::CVariant( CORE::CString( "w" ) ) );
        ASSERT_TRUE( a != b );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    factory.RegisterConcreteFactory( "fake", &g_testFakeDatabaseClientFactory );

    GUCEF_TESTFW_TESTCASE( "DatabaseClient: the settings of the client are saved and loaded as configuration" )
    try
    {
        CIDatabaseClientPtr client = factory.Create( "fake" );
        ASSERT_FALSE( client.IsNULL() );
        CDatabaseClientSettings settings = FakeSettings( "configured" );
        settings.readOnly = true;
        ASSERT_TRUE( client->Open( settings ) );
        ASSERT_TRUE( client->GetSettings() == settings );

        CORE::CDataNode config;
        ASSERT_TRUE( client->SaveConfig( config ) );

        CIDatabaseClientPtr other = factory.Create( "fake" );
        ASSERT_TRUE( other->LoadConfig( config ) );
        ASSERT_TRUE( other->GetSettings() == settings );
        ASSERT_FALSE( other->IsOpen() );        // loading the configuration does not open

        CORE::CDataNode nothing;
        ASSERT_FALSE( other->LoadConfig( nothing ) );
        ASSERT_TRUE( other->GetSettings() == settings );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DatabaseClient sync: the calls run on the calling thread and do their work" )
    try
    {
        CIDatabaseClientPtr client = factory.Create( "fake" );
        CTestFakeDatabaseClient* fake = AsFake( client );
        const TThreadId thisThread = MT::GetCurrentThreadID();

        ASSERT_TRUE( client->Open( FakeSettings() ) );
        ASSERT_TRUE( client->IsOpen() );
        ASSERT_TRUE( thisThread == fake->GetThreadOfLastCall( 0 ) );
        ASSERT_TRUE( 0 != ( client->GetCapabilities() & GUCEF_DATABASE_CAPABILITY_WRITE ) );
        ASSERT_TRUE( 0 != ( client->GetCapabilities() & GUCEF_DATABASE_CAPABILITY_ASYNC ) );
        ASSERT_TRUE( 0 == ( client->GetCapabilities() & GUCEF_DATABASE_CAPABILITY_SCHEMA_EVOLUTION ) );

        CORE::CString::StringSet names;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->ListTables( names ) );
        ASSERT_TRUE( names.empty() );
        ASSERT_TRUE( thisThread == fake->GetThreadOfLastCall( 1 ) );

        CORE::CIDataTablePtr table;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->CreateTable( Definition( "prices" ), table ) );
        ASSERT_FALSE( table.IsNULL() );
        ASSERT_TRUE( table->GetName() == "prices" );
        ASSERT_TRUE( 2 == table->GetSchema()->GetColumnCount() );
        ASSERT_TRUE( thisThread == fake->GetThreadOfLastCall( 3 ) );

        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->CreateTable( Definition( "bars" ), table ) );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->ListTables( names ) );
        ASSERT_TRUE( 2 == names.size() && names.count( "prices" ) && names.count( "bars" ) );

        CORE::CIDataTablePtr opened;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->OpenTable( "bars", opened ) );
        ASSERT_FALSE( opened.IsNULL() );
        ASSERT_TRUE( opened->GetName() == "bars" );
        ASSERT_FALSE( opened->IsReadOnly() );
        ASSERT_TRUE( thisThread == fake->GetThreadOfLastCall( 2 ) );

        CORE::CDataTableDefinition definition;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->GetTableDefinition( "prices", definition ) );
        ASSERT_TRUE( definition == Definition( "prices" ) );

        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->DropTable( "prices", true ) );
        ASSERT_TRUE( thisThread == fake->GetThreadOfLastCall( 4 ) );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->ListTables( names ) );
        ASSERT_TRUE( 1 == names.size() && names.count( "bars" ) );

        client->Close();
        ASSERT_FALSE( client->IsOpen() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DatabaseClient sync: failures return their status and keep the message for the calling thread" )
    try
    {
        CIDatabaseClientPtr client = factory.Create( "fake" );
        CORE::CString::StringSet names;
        CORE::CIDataTablePtr table;

        // before Open
        ASSERT_TRUE( CORE::DATATABLE_STATUS_FAILED == client->ListTables( names ) );
        ASSERT_TRUE( client->GetLastErrorMessage().HasSubstr( "not open" ) >= 0 );

        // Open with settings that are not valid, for another backend, and a database that cannot be opened
        CDatabaseClientSettings invalid;
        ASSERT_FALSE( client->Open( invalid ) );
        ASSERT_FALSE( client->IsOpen() );
        ASSERT_FALSE( client->GetLastErrorMessage().IsNULLOrEmpty() );
        CDatabaseClientSettings foreign = FakeSettings();
        foreign.backendName = "other";
        ASSERT_FALSE( client->Open( foreign ) );
        ASSERT_TRUE( client->GetLastErrorMessage().HasSubstr( "other" ) >= 0 );
        ASSERT_FALSE( client->Open( FakeSettings( "unavailable" ) ) );
        ASSERT_TRUE( client->GetLastErrorMessage().HasSubstr( "unavailable" ) >= 0 );

        ASSERT_TRUE( client->Open( FakeSettings() ) );
        ASSERT_TRUE( client->GetLastErrorMessage().IsNULLOrEmpty() );       // a successful call clears it

        ASSERT_TRUE( CORE::DATATABLE_STATUS_NOT_FOUND == client->OpenTable( "missing", table ) );
        ASSERT_TRUE( table.IsNULL() );
        ASSERT_TRUE( client->GetLastErrorMessage().HasSubstr( "missing" ) >= 0 );
        CORE::CDataTableDefinition definition;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_NOT_FOUND == client->GetTableDefinition( "missing", definition ) );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_NOT_FOUND == client->DropTable( "missing", false ) );

        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->CreateTable( Definition( "t" ), table ) );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_ALREADY_EXISTS == client->CreateTable( Definition( "t" ), table ) );
        CORE::CDataTableDefinition bad = Definition( "bad name" );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_INVALID_ARGUMENT == client->CreateTable( bad, table ) );
        CORE::CDataTableDefinition noColumns = Definition( "empty" );
        noColumns.schema.Clear();
        ASSERT_TRUE( CORE::DATATABLE_STATUS_INVALID_ARGUMENT == client->CreateTable( noColumns, table ) );

        // read only
        CIDatabaseClientPtr reader = factory.Create( "fake" );
        CDatabaseClientSettings readOnly = FakeSettings();
        readOnly.readOnly = true;
        ASSERT_TRUE( reader->Open( readOnly ) );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_READ_ONLY == reader->CreateTable( Definition( "ro" ), table ) );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_READ_ONLY == reader->DropTable( "t", false ) );

        // a failed call does not leave the client unusable
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->OpenTable( "t", table ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DatabaseClient sync: the events of the calls reach an observer on the calling thread" )
    try
    {
        CIDatabaseClientPtr client = factory.Create( "fake" );
        CDatabaseEventRecorder recorder;
        recorder.Attach( client.GetPointerAlways() );
        CORE::CIDataTablePtr table;

        ASSERT_TRUE( client->Open( FakeSettings() ) );
        ASSERT_TRUE( 1 == recorder.opened );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->CreateTable( Definition( "events" ), table ) );
        ASSERT_TRUE( 1 == recorder.created );
        ASSERT_TRUE( recorder.lastTable == "events" );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_ALREADY_EXISTS == client->CreateTable( Definition( "events" ), table ) );
        ASSERT_TRUE( 1 == recorder.errors );
        ASSERT_TRUE( recorder.lastError.HasSubstr( "events" ) >= 0 );
        ASSERT_TRUE( 1 == recorder.created );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->DropTable( "events", false ) );
        ASSERT_TRUE( 1 == recorder.dropped );
        ASSERT_TRUE( recorder.lastTable == "events" );
        client->Close();
        ASSERT_TRUE( 1 == recorder.closed );
        client->Close();
        ASSERT_TRUE( 1 == recorder.closed );     // closing a closed client says nothing
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DatabaseClient async: every call runs on a pool thread and completes with its result" )
    try
    {
        CIDatabaseClientPtr client = factory.Create( "fake" );
        CTestFakeDatabaseClient* fake = AsFake( client );
        const TThreadId thisThread = MT::GetCurrentThreadID();

        CORE::CFutureResult opened = client->OpenAsync( FakeSettings() );
        ASSERT_TRUE( Finished( opened ) );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_FINISHED == StatusOf( opened ) );
        ASSERT_TRUE( client->IsOpen() );
        ASSERT_TRUE( 0 != fake->GetThreadOfLastCall( 0 ) && thisThread != fake->GetThreadOfLastCall( 0 ) );

        CORE::CFutureResult created = client->CreateTableAsync( Definition( "async_table" ) );
        ASSERT_TRUE( Finished( created ) );
        CORE::CIDataTablePtr table;
        ASSERT_TRUE( CDatabaseTableResult::TryGetFromFuture( created, table ) );
        ASSERT_FALSE( table.IsNULL() );
        ASSERT_TRUE( table->GetName() == "async_table" );
        ASSERT_TRUE( thisThread != fake->GetThreadOfLastCall( 3 ) );

        CORE::CFutureResult list = client->ListTablesAsync();
        ASSERT_TRUE( Finished( list ) );
        CORE::CString::StringSet names;
        ASSERT_TRUE( CDatabaseTableNamesResult::TryGetFromFuture( list, names ) );
        ASSERT_TRUE( 1 == names.size() && names.count( "async_table" ) );
        ASSERT_TRUE( thisThread != fake->GetThreadOfLastCall( 1 ) );

        CORE::CFutureResult openedTable = client->OpenTableAsync( "async_table" );
        ASSERT_TRUE( Finished( openedTable ) );
        CORE::CIDataTablePtr opened2;
        ASSERT_TRUE( CDatabaseTableResult::TryGetFromFuture( openedTable, opened2 ) );
        ASSERT_TRUE( opened2->GetName() == "async_table" );
        ASSERT_TRUE( thisThread != fake->GetThreadOfLastCall( 2 ) );

        // the results do not cross: a table future holds no names and the other way round
        ASSERT_FALSE( CDatabaseTableNamesResult::TryGetFromFuture( openedTable, names ) );
        ASSERT_FALSE( CDatabaseTableResult::TryGetFromFuture( list, table ) );

        CORE::CFutureResult dropped = client->DropTableAsync( "async_table", true );
        ASSERT_TRUE( Finished( dropped ) );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_FINISHED == StatusOf( dropped ) );
        ASSERT_TRUE( thisThread != fake->GetThreadOfLastCall( 4 ) );

        // the sync call sees what the async calls did
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->ListTables( names ) );
        ASSERT_TRUE( names.empty() );
        ASSERT_TRUE( client->GetSettings().database == "testdb" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DatabaseClient async: a failed call ends the future in an error with the status and the message of the pool thread" )
    try
    {
        CIDatabaseClientPtr client = factory.Create( "fake" );
        ASSERT_TRUE( client->Open( FakeSettings() ) );

        CORE::CFutureResult missing = client->OpenTableAsync( "nothing" );
        ASSERT_TRUE( Finished( missing ) );
        ASSERT_TRUE( CORE::TaskStatusIsAnError( StatusOf( missing ) ) );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_TASKDATA_INVALID == StatusOf( missing ) );
        ASSERT_TRUE( ExtraInfoOf( missing ).StartsWith( "NOT_FOUND: " ) );
        ASSERT_TRUE( ExtraInfoOf( missing ).HasSubstr( "nothing" ) >= 0 );
        CORE::CIDataTablePtr table;
        ASSERT_FALSE( CDatabaseTableResult::TryGetFromFuture( missing, table ) );
        ASSERT_TRUE( table.IsNULL() );

        // the message belongs to the thread that made the call: this thread made no failing call
        ASSERT_TRUE( client->GetLastErrorMessage().HasSubstr( "nothing" ) < 0 );

        CORE::CFutureResult first = client->CreateTableAsync( Definition( "dup" ) );
        ASSERT_TRUE( Finished( first ) );
        CORE::CFutureResult second = client->CreateTableAsync( Definition( "dup" ) );
        ASSERT_TRUE( Finished( second ) );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_TASKDATA_INVALID == StatusOf( second ) );
        ASSERT_TRUE( ExtraInfoOf( second ).StartsWith( "ALREADY_EXISTS: " ) );

        CORE::CFutureResult dropMissing = client->DropTableAsync( "nothing", false );
        ASSERT_TRUE( Finished( dropMissing ) );
        ASSERT_TRUE( ExtraInfoOf( dropMissing ).StartsWith( "NOT_FOUND: " ) );

        CORE::CFutureResult badOpen = factory.Create( "fake" )->OpenAsync( FakeSettings( "unavailable" ) );
        ASSERT_TRUE( Finished( badOpen ) );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_EXCEPTION_DURING_RUN == StatusOf( badOpen ) );
        ASSERT_TRUE( ExtraInfoOf( badOpen ).StartsWith( "FAILED: " ) );
        ASSERT_TRUE( ExtraInfoOf( badOpen ).HasSubstr( "unavailable" ) >= 0 );

        CDatabaseClientSettings readOnly = FakeSettings();
        readOnly.readOnly = true;
        CIDatabaseClientPtr reader = factory.Create( "fake" );
        ASSERT_TRUE( Finished( reader->OpenAsync( readOnly ) ) );
        CORE::CFutureResult refused = reader->CreateTableAsync( Definition( "ro" ) );
        ASSERT_TRUE( Finished( refused ) );
        ASSERT_TRUE( ExtraInfoOf( refused ).StartsWith( "READ_ONLY: " ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DatabaseClient async: the thread pool of the settings is used, the name of the pool when none is given" )
    try
    {
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        ASSERT_TRUE( taskManager.GetThreadPool( "DbTestNamedPool" ).IsNULL() );

        CIDatabaseClientPtr client = factory.Create( "fake" );
        CDatabaseClientSettings settings = FakeSettings();
        settings.threadPoolName = "DbTestNamedPool";
        ASSERT_TRUE( Finished( client->OpenAsync( settings ) ) );
        ASSERT_FALSE( taskManager.GetThreadPool( "DbTestNamedPool" ).IsNULL() );

        CIDatabaseClientPtr direct = factory.Create( "fake" );
        CDatabaseClientSettings withPool = FakeSettings();
        withPool.threadPool = taskManager.GetOrCreateThreadPool( "DbTestGivenPool" );
        withPool.threadPoolName = "DbTestNeverMade";
        ASSERT_TRUE( Finished( direct->OpenAsync( withPool ) ) );
        ASSERT_TRUE( Finished( direct->ListTablesAsync() ) );
        ASSERT_TRUE( taskManager.GetThreadPool( "DbTestNeverMade" ).IsNULL() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DatabaseClient async: many calls in flight all complete and the pending count returns to zero" )
    try
    {
        CIDatabaseClientPtr client = factory.Create( "fake" );
        CTestFakeDatabaseClient* fake = AsFake( client );
        ASSERT_TRUE( client->Open( FakeSettings() ) );

        const UInt32 count = 120;
        CORE::CFutureResult::TFutureResultVector futures;
        for ( UInt32 i = 0; i < count; ++i )
            futures.push_back( client->CreateTableAsync( Definition( ( CORE::CString( "table_" ) + CORE::UInt32ToString( i ) ).C_String() ) ) );

        UInt32 succeeded = 0;
        for ( UInt32 i = 0; i < count; ++i )
        {
            ASSERT_TRUE( Finished( futures[ i ] ) );
            CORE::CIDataTablePtr table;
            if ( CDatabaseTableResult::TryGetFromFuture( futures[ i ], table ) && !table.IsNULL() )
                ++succeeded;
        }
        ASSERT_TRUE( count == succeeded );

        // the pending count is zero once all futures ended and the last task has let go
        UInt32 waited = 0;
        while ( static_cast< CDatabaseClientBase* >( fake )->GetPendingAsyncOperationCount() > 0 && waited < 5000 )
        {
            MT::ThreadDelay( 1 );
            ++waited;
        }
        ASSERT_TRUE( 0 == static_cast< CDatabaseClientBase* >( fake )->GetPendingAsyncOperationCount() );

        CORE::CString::StringSet names;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->ListTables( names ) );
        ASSERT_TRUE( count == names.size() );

        // mixed reads and writes at once
        CORE::CFutureResult::TFutureResultVector mixed;
        for ( UInt32 i = 0; i < count; ++i )
        {
            const CORE::CString name = CORE::CString( "table_" ) + CORE::UInt32ToString( i );
            mixed.push_back( 0 == i % 3 ? client->DropTableAsync( name, false ) : client->OpenTableAsync( name ) );
        }
        UInt32 ended = 0;
        for ( UInt32 i = 0; i < count; ++i )
            if ( Finished( mixed[ i ] ) )
                ++ended;
        ASSERT_TRUE( count == ended );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DatabaseClient async: a client that is destroyed with calls pending waits for them" )
    try
    {
        CORE::CFutureResult::TFutureResultVector futures;
        UInt32 callsBefore = 0;
        {
            CIDatabaseClientPtr client = factory.Create( "fake" );
            CTestFakeDatabaseClient* fake = AsFake( client );
            ASSERT_TRUE( client->Open( FakeSettings() ) );
            fake->delayInMs = 10;
            for ( UInt32 i = 0; i < 10; ++i )
                futures.push_back( client->ListTablesAsync() );
            callsBefore = fake->GetCallCount();
            // the client goes out of scope here with calls queued and running
        }
        UInt32 finished = 0;
        for ( UInt32 i = 0; i < futures.size(); ++i )
            if ( Finished( futures[ i ] ) && CORE::TTaskStatus::TASKSTATUS_FINISHED == StatusOf( futures[ i ] ) )
                ++finished;
        ASSERT_TRUE( futures.size() == finished );
        ASSERT_TRUE( callsBefore >= 1 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    factory.UnregisterConcreteFactory( "fake" );
}

/*-------------------------------------------------------------------------*/
