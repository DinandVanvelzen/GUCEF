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

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CDATABASETRANSACTION_H
#include "gucefCOMCORE_CDatabaseTransaction.h"
#define GUCEF_COMCORE_CDATABASETRANSACTION_H
#endif /* GUCEF_COMCORE_CDATABASETRANSACTION_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDatabaseClientFake.h"
#include "TestDatabaseClientTransactional.h"
#include "TestDatabaseTransaction.h"

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

/** A client in the shared pointer type of the factory */
CIDatabaseClientPtr
OwnClient( CIDatabaseClient* client )
{
    return CIDatabaseClientPtr( CORE::CTSharedPtr< CIDatabaseClient, MT::CMutex >( client ) );
}

/*-------------------------------------------------------------------------*/

CDatabaseClientSettings
TransactionTestSettings( const CORE::CString& backendName )
{
    CDatabaseClientSettings settings;
    settings.backendName = backendName;
    settings.database = "transactions";
    return settings;
}

/*-------------------------------------------------------------------------*/

CORE::CDataTableDefinition
TransactionTestDefinition( const char* name )
{
    CORE::CDataTableDefinition definition;
    definition.name = name;
    definition.schema.AddColumn( CORE::CDataTableColumnDef( "id", CORE::DATATABLE_TYPE_INT64, false ) );
    definition.schema.AddColumn( CORE::CDataTableColumnDef( "label", CORE::DATATABLE_TYPE_UTF8, true ) );
    CORE::CString::StringVector key;
    key.push_back( "id" );
    definition.schema.SetPrimaryKey( key );
    return definition;
}

/*-------------------------------------------------------------------------*/

/** A batch of the rows first .. first + count - 1, labelled with their id */
CORE::CDataTableBatchPtr
TransactionTestRows( CORE::CIDataTable& table, Int64 first, UInt32 count )
{
    CORE::CDataTableBatchBuilder builder( table.GetSchema(), CORE::CCoreGlobal::Instance()->GetDataTableBufferPool() );
    for ( UInt32 i=0; i<count; ++i )
    {
        CORE::CVariant row[ 2 ];
        row[ 0 ] = CORE::CVariant( (Int64) ( first + i ) );
        row[ 1 ] = CORE::CVariant( CORE::CString( "row " ) + CORE::ToString( first + i ) );
        if ( !builder.AppendRow( row, 2 ) )
            return CORE::CDataTableBatchPtr();
    }
    CORE::CDataTableBatchPtr batch;
    CORE::CString error;
    if ( CORE::DATATABLE_STATUS_OK != builder.Build( batch, error ) )
        return CORE::CDataTableBatchPtr();
    return batch;
}

/*-------------------------------------------------------------------------*/

bool
WriteTransactionTestRows( CORE::CIDataTable& table, Int64 first, UInt32 count )
{
    CORE::CDataTableBatchPtr rows = TransactionTestRows( table, first, count );
    if ( rows.IsNULL() )
        return false;
    CORE::CDataTableWriteResult result;
    return CORE::DATATABLE_STATUS_OK == table.Write( *rows, CORE::DATATABLE_WRITE_INSERT, result );
}

/*-------------------------------------------------------------------------*/

/** The rows of the table, GUCEF_UINT64MAX when the scan failed */
UInt64
CountTransactionTestRows( CORE::CIDataTable& table )
{
    CORE::CDataTableScanPtr scan;
    if ( CORE::DATATABLE_STATUS_OK != table.Scan( CORE::CDataTableQuery(), scan ) )
        return GUCEF_UINT64MAX;
    UInt64 rows = 0;
    while ( true )
    {
        CORE::CDataTableBatchPtr batch;
        const CORE::TDataTableStatus status = scan->NextBatch( batch );
        if ( CORE::DATATABLE_STATUS_END == status )
            return rows;
        if ( CORE::DATATABLE_STATUS_OK != status )
            return GUCEF_UINT64MAX;
        rows += batch->GetSelectedRowCount();
    }
}

/*-------------------------------------------------------------------------*/

bool
TransactionFutureFinished( const CORE::CFutureResult& future )
{
    CORE::CTaskPtr task = future.GetResult( 20000 );
    return !task.IsNULL() && task->IsTaskInEndState();
}

/*-------------------------------------------------------------------------*/

CORE::TTaskStatus
TransactionFutureStatus( const CORE::CFutureResult& future )
{
    CORE::CTaskPtr task = future.GetResult();
    return task.IsNULL() ? CORE::TASKSTATUS_UNDEFINED : task->GetTaskStatus();
}

/*-------------------------------------------------------------------------*/

CORE::CString
TransactionFutureInfo( const CORE::CFutureResult& future )
{
    CORE::CTaskPtr task = future.GetResult();
    return task.IsNULL() ? CORE::CString() : task->GetTaskStatusExtraInfo();
}

/*-------------------------------------------------------------------------*/

/** Writes rows to a table on a thread of its own, as another part of the application would */
class CTransactionTestWriterThread : public MT::CActiveObject
{
    public:

    CORE::CIDataTable* table;
    Int64 first;
    UInt32 count;
    bool written;

    CTransactionTestWriterThread( void ) : MT::CActiveObject(), table( GUCEF_NULL ), first( 0 ), count( 0 ), written( false ) {}

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return this;
    }

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        return true;
    }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        written = WriteTransactionTestRows( *table, first, count );
        return true;
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDatabaseTransactionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DatabaseTransaction" );

    GUCEF_TESTFW_TESTCASE( "DatabaseTransaction: a backend without transactions reports UNSUPPORTED, sync and async, and a scope does not begin" )
    try
    {
        CIDatabaseClientPtr client = OwnClient( GUCEF_NEW CTestFakeDatabaseClient() );
        ASSERT_TRUE( client->Open( TransactionTestSettings( CTestFakeDatabaseClient::BackendName ) ) );
        ASSERT_TRUE( 0 == ( client->GetCapabilities() & GUCEF_DATABASE_CAPABILITY_TRANSACTIONS ) );
        ASSERT_FALSE( client->IsInTransaction() );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_UNSUPPORTED == client->BeginTransaction() );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_UNSUPPORTED == client->CommitTransaction() );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_UNSUPPORTED == client->RollbackTransaction() );
        ASSERT_FALSE( client->IsInTransaction() );

        CORE::CFutureResult begun = client->BeginTransactionAsync();
        CORE::CFutureResult committed = client->CommitTransactionAsync();
        CORE::CFutureResult rolledBack = client->RollbackTransactionAsync();
        ASSERT_TRUE( TransactionFutureFinished( begun ) && TransactionFutureFinished( committed ) && TransactionFutureFinished( rolledBack ) );
        ASSERT_TRUE( CORE::TASKSTATUS_TASKDATA_INVALID == TransactionFutureStatus( begun ) );
        ASSERT_TRUE( CORE::TASKSTATUS_TASKDATA_INVALID == TransactionFutureStatus( committed ) );
        ASSERT_TRUE( CORE::TASKSTATUS_TASKDATA_INVALID == TransactionFutureStatus( rolledBack ) );
        ASSERT_TRUE( 0 == TransactionFutureInfo( begun ).HasSubstr( "UNSUPPORTED" ) );

        {
            CDatabaseTransaction transaction( client );
            ASSERT_FALSE( transaction.IsActive() );
            ASSERT_TRUE( CORE::DATATABLE_STATUS_UNSUPPORTED == transaction.GetBeginStatus() );
            ASSERT_TRUE( CORE::DATATABLE_STATUS_UNSUPPORTED == transaction.Commit() );
            ASSERT_TRUE( CORE::DATATABLE_STATUS_UNSUPPORTED == transaction.Rollback() );
        }
        {
            // the end of a scope that did not begin calls nothing
            CDatabaseTransaction transaction( *client );
            ASSERT_FALSE( transaction.IsActive() );
        }
        ASSERT_TRUE( client->IsOpen() );
        client->Close();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DatabaseTransaction: a committed scope keeps the writes, the end of an uncommitted scope undoes them and the tables it made" )
    try
    {
        CTestTransactionalDatabaseClient* backend = GUCEF_NEW CTestTransactionalDatabaseClient();
        CIDatabaseClientPtr client = OwnClient( backend );
        ASSERT_TRUE( client->Open( TransactionTestSettings( CTestTransactionalDatabaseClient::BackendName ) ) );
        ASSERT_TRUE( 0 != ( client->GetCapabilities() & GUCEF_DATABASE_CAPABILITY_TRANSACTIONS ) );

        CORE::CIDataTablePtr packs;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->CreateTable( TransactionTestDefinition( "packs" ), packs ) );
        ASSERT_TRUE( WriteTransactionTestRows( *packs, 1, 3 ) );

        {
            CDatabaseTransaction transaction( client );
            ASSERT_TRUE( transaction.IsActive() && CORE::DATATABLE_STATUS_OK == transaction.GetBeginStatus() );
            ASSERT_TRUE( client->IsInTransaction() );
            ASSERT_TRUE( WriteTransactionTestRows( *packs, 10, 5 ) );
            ASSERT_TRUE( 8 == CountTransactionTestRows( *packs ) );
            ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == transaction.Commit() );
            ASSERT_FALSE( transaction.IsActive() );
            ASSERT_FALSE( client->IsInTransaction() );
            // a second commit of the same scope is refused
            ASSERT_TRUE( CORE::DATATABLE_STATUS_INVALID_ARGUMENT == transaction.Commit() );
        }
        ASSERT_TRUE( 8 == CountTransactionTestRows( *packs ) );
        ASSERT_TRUE( 1 == backend->GetCommitCount() && 0 == backend->GetRollbackCount() );

        {
            CDatabaseTransaction transaction( client );
            ASSERT_TRUE( transaction.IsActive() );
            ASSERT_TRUE( WriteTransactionTestRows( *packs, 100, 2 ) );
            CORE::CDataTableWriteResult deleted;
            CORE::CDataTableQuery firstRow;
            firstRow.WhereEquals( "id", CORE::CVariant( (Int64) 1 ) );
            ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == packs->DeleteRows( firstRow, deleted ) );
            CORE::CIDataTablePtr assets;
            ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->CreateTable( TransactionTestDefinition( "assets" ), assets ) );
            ASSERT_TRUE( WriteTransactionTestRows( *assets, 1, 4 ) );
            ASSERT_TRUE( 9 == CountTransactionTestRows( *packs ) );
        }
        ASSERT_FALSE( client->IsInTransaction() );
        ASSERT_TRUE( 1 == backend->GetRollbackCount() );
        ASSERT_TRUE( 8 == CountTransactionTestRows( *packs ) );
        CORE::CString::StringSet names;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->ListTables( names ) );
        ASSERT_TRUE( 1 == names.size() && names.count( "packs" ) );
        client->Close();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DatabaseTransaction: writes of other threads and async writes inside the scope belong to the transaction" )
    try
    {
        CTestTransactionalDatabaseClient* backend = GUCEF_NEW CTestTransactionalDatabaseClient();
        CIDatabaseClientPtr client = OwnClient( backend );
        ASSERT_TRUE( client->Open( TransactionTestSettings( CTestTransactionalDatabaseClient::BackendName ) ) );
        CORE::CIDataTablePtr table;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->CreateTable( TransactionTestDefinition( "shared" ), table ) );

        {
            CDatabaseTransaction transaction( client );
            ASSERT_TRUE( transaction.IsActive() );

            CTransactionTestWriterThread writer;
            writer.table = table.GetPointerAlways();
            writer.first = 1;
            writer.count = 20;
            ASSERT_TRUE( writer.Activate() );
            writer.WaitForThreadToFinish( 20000 );
            ASSERT_TRUE( writer.written );

            CORE::CFutureResult asyncWrite = table->WriteAsync( TransactionTestRows( *table, 100, 10 ), CORE::DATATABLE_WRITE_INSERT );
            ASSERT_TRUE( TransactionFutureFinished( asyncWrite ) );
            ASSERT_TRUE( CORE::TASKSTATUS_FINISHED == TransactionFutureStatus( asyncWrite ) );
            ASSERT_TRUE( 30 == CountTransactionTestRows( *table ) );

            ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == transaction.Rollback() );
            ASSERT_FALSE( transaction.IsActive() );
            ASSERT_TRUE( CORE::DATATABLE_STATUS_INVALID_ARGUMENT == transaction.Rollback() );
        }
        ASSERT_TRUE( 0 == CountTransactionTestRows( *table ) );
        ASSERT_TRUE( 1 == backend->GetRollbackCount() );
        client->Close();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DatabaseTransaction: one transaction per client: a second scope does not begin and does not end the first" )
    try
    {
        CTestTransactionalDatabaseClient* backend = GUCEF_NEW CTestTransactionalDatabaseClient();
        CIDatabaseClientPtr client = OwnClient( backend );
        ASSERT_TRUE( client->Open( TransactionTestSettings( CTestTransactionalDatabaseClient::BackendName ) ) );
        CORE::CIDataTablePtr table;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->CreateTable( TransactionTestDefinition( "nested" ), table ) );

        ASSERT_TRUE( CORE::DATATABLE_STATUS_INVALID_ARGUMENT == client->CommitTransaction() );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_INVALID_ARGUMENT == client->RollbackTransaction() );
        ASSERT_FALSE( client->GetLastErrorMessage().IsNULLOrEmpty() );

        {
            CDatabaseTransaction outer( client );
            ASSERT_TRUE( outer.IsActive() );
            ASSERT_TRUE( WriteTransactionTestRows( *table, 1, 2 ) );
            {
                CDatabaseTransaction inner( client );
                ASSERT_FALSE( inner.IsActive() );
                ASSERT_TRUE( CORE::DATATABLE_STATUS_INVALID_ARGUMENT == inner.GetBeginStatus() );
                ASSERT_TRUE( CORE::DATATABLE_STATUS_INVALID_ARGUMENT == inner.Commit() );
            }
            ASSERT_TRUE( client->IsInTransaction() && outer.IsActive() );
            ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == outer.Commit() );
        }
        ASSERT_TRUE( 2 == CountTransactionTestRows( *table ) );
        ASSERT_TRUE( 1 == backend->GetCommitCount() && 0 == backend->GetRollbackCount() );
        client->Close();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DatabaseTransaction: the async transaction calls run on a pool thread, Close() rolls back an open transaction" )
    try
    {
        CTestTransactionalDatabaseClient* backend = GUCEF_NEW CTestTransactionalDatabaseClient();
        CIDatabaseClientPtr client = OwnClient( backend );
        ASSERT_TRUE( client->Open( TransactionTestSettings( CTestTransactionalDatabaseClient::BackendName ) ) );
        CORE::CIDataTablePtr table;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->CreateTable( TransactionTestDefinition( "async" ), table ) );
        const TThreadId thisThread = MT::GetCurrentThreadID();

        CORE::CFutureResult begun = client->BeginTransactionAsync();
        ASSERT_TRUE( TransactionFutureFinished( begun ) && CORE::TASKSTATUS_FINISHED == TransactionFutureStatus( begun ) );
        ASSERT_TRUE( 0 != backend->GetThreadOfLastTransactionCall() && thisThread != backend->GetThreadOfLastTransactionCall() );
        ASSERT_TRUE( client->IsInTransaction() );
        ASSERT_TRUE( WriteTransactionTestRows( *table, 1, 5 ) );
        CORE::CFutureResult committed = client->CommitTransactionAsync();
        ASSERT_TRUE( TransactionFutureFinished( committed ) && CORE::TASKSTATUS_FINISHED == TransactionFutureStatus( committed ) );
        ASSERT_FALSE( client->IsInTransaction() );
        ASSERT_TRUE( 5 == CountTransactionTestRows( *table ) );

        // a failed async call ends its future with the status
        CORE::CFutureResult nothing = client->RollbackTransactionAsync();
        ASSERT_TRUE( TransactionFutureFinished( nothing ) && CORE::TASKSTATUS_TASKDATA_INVALID == TransactionFutureStatus( nothing ) );
        ASSERT_TRUE( 0 == TransactionFutureInfo( nothing ).HasSubstr( "INVALID_ARGUMENT" ) );

        begun = client->BeginTransactionAsync();
        ASSERT_TRUE( TransactionFutureFinished( begun ) && CORE::TASKSTATUS_FINISHED == TransactionFutureStatus( begun ) );
        ASSERT_TRUE( WriteTransactionTestRows( *table, 10, 5 ) );
        CORE::CFutureResult rolledBack = client->RollbackTransactionAsync();
        ASSERT_TRUE( TransactionFutureFinished( rolledBack ) && CORE::TASKSTATUS_FINISHED == TransactionFutureStatus( rolledBack ) );
        ASSERT_TRUE( 5 == CountTransactionTestRows( *table ) );

        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->BeginTransaction() );
        ASSERT_TRUE( WriteTransactionTestRows( *table, 20, 5 ) );
        ASSERT_TRUE( 10 == CountTransactionTestRows( *table ) );
        client->Close();
        ASSERT_FALSE( client->IsInTransaction() );
        ASSERT_TRUE( 5 == CountTransactionTestRows( *table ) );
        ASSERT_TRUE( 2 == backend->GetRollbackCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
