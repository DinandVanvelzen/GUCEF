/*
 *  gucefDRN_TestApp: GUCEF test application for the DRN module
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

#include <exception>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_CTABLECODECREGISTRY_H
#include "gucefCORE_CTableCodecRegistry.h"
#define GUCEF_CORE_CTABLECODECREGISTRY_H
#endif /* GUCEF_CORE_CTABLECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CITABLEWRITER_H
#include "gucefCORE_CITableWriter.h"
#define GUCEF_CORE_CITABLEWRITER_H
#endif /* GUCEF_CORE_CITABLEWRITER_H ? */

#ifndef GUCEF_COMCORE_CDATABASETABLERESULT_H
#include "gucefCOMCORE_CDatabaseTableResult.h"
#define GUCEF_COMCORE_CDATABASETABLERESULT_H
#endif /* GUCEF_COMCORE_CDATABASETABLERESULT_H ? */

#include "TestDRNTestUtils.h"
#include "TestDRNGroup.h"
#include "TestDRNTableScanThread.h"
#include "TestDRNDataTables.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace DRNTEST;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define REQUIRE_TRUE(t)  do { if ( !(t) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #t, __FILE__, __LINE__ ); throw std::exception(); } } while ( 0 )

typedef GUCEF::vector< CORE::CVariant >     TRow;
typedef GUCEF::vector< TRow >               TRowVector;

/*-------------------------------------------------------------------------*/

/** Grid settings with declared types */
static DRN::CDRNDataGridSettings
TypedGrid( const CORE::CString& name, DRN::TDRNAuthority authority, CORE::TDataTableLogicalType keyType, CORE::TDataTableLogicalType valueType )
{
    DRN::CDRNDataGridSettings settings;
    settings.name = name;
    settings.authority = authority;
    settings.sendRateHz = 0;
    settings.keyType = keyType;
    settings.valueType = valueType;
    return settings;
}

/*-------------------------------------------------------------------------*/

/** Table settings for a replica, which does not learn the declared types */
static DRN::CDRNDataGridTableSettings
TypedTable( CORE::TDataTableLogicalType keyType, CORE::TDataTableLogicalType valueType )
{
    DRN::CDRNDataGridTableSettings settings;
    settings.keyType = keyType;
    settings.valueType = valueType;
    return settings;
}

/*-------------------------------------------------------------------------*/

/** The rows a scan delivers, every projected column of a row as a variant */
static CORE::TDataTableStatus
ReadRows( CORE::CIDataTable& table, const CORE::CDataTableQuery& query, TRowVector& rows, CORE::CDataTableScanStats* stats = GUCEF_NULL )
{
    rows.clear();
    CORE::CDataTableScanPtr scan;
    CORE::TDataTableStatus status = table.Scan( query, scan );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    CORE::CDataTableBatchPtr batch;
    while ( CORE::DATATABLE_STATUS_OK == ( status = scan->NextBatch( batch ) ) )
    {
        const UInt32 selected = batch->GetSelectedRowCount();
        const UInt32* selection = batch->GetSelection();
        for ( UInt32 i=0; i<selected; ++i )
        {
            TRow row;
            for ( UInt32 c=0; c<batch->GetColumnCount(); ++c )
            {
                CORE::CVariant cell;
                batch->GetValue( GUCEF_NULL == selection ? i : selection[ i ], c, cell, false );
                row.push_back( cell );
            }
            rows.push_back( row );
        }
    }
    if ( GUCEF_NULL != stats )
        scan->GetStats( *stats );
    return CORE::DATATABLE_STATUS_END == status ? CORE::DATATABLE_STATUS_OK : status;
}

/*-------------------------------------------------------------------------*/

static UInt32
CountRows( CORE::CIDataTable& table, const CORE::CDataTableQuery& query = CORE::CDataTableQuery() )
{
    TRowVector rows;
    return CORE::DATATABLE_STATUS_OK == ReadRows( table, query, rows ) ? (UInt32) rows.size() : 0xFFFFFFFF;
}

/*-------------------------------------------------------------------------*/

/** Rows of the key and value columns of a table */
static CORE::CDataTableBatchPtr
MakeRows( CORE::CIDataTable& table, const TRow& keys, const TRow& values )
{
    CORE::CDataTableSchema schema;
    schema.AddColumn( table.GetSchema()->GetColumn( 0 ) );
    schema.AddColumn( table.GetSchema()->GetColumn( 1 ) );
    CORE::CDataTableBatchBuilder builder( CORE::CreateDataTableSchemaPtr( schema ), CORE::CCoreGlobal::Instance()->GetDataTableBufferPool() );
    for ( UInt32 i=0; i<(UInt32) keys.size(); ++i )
    {
        builder.GetColumn( 0 ).Append( keys[ i ] );
        builder.GetColumn( 1 ).Append( values[ i ] );
    }
    CORE::CDataTableBatchPtr batch;
    CORE::CString error;
    builder.Build( batch, error );
    return batch;
}

/*-------------------------------------------------------------------------*/

/** Waits for the future while pumping the pulse generator that delivers the events of the nodes */
static CORE::TTaskStatus
AwaitPumped( const CORE::CFutureResult& future, CORE::CTaskPtr& task, UInt32 timeoutMs = 10000 )
{
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    const UInt64 start = DRN::CDRNClock::NowInMicroseconds();
    while ( !future.IsReady() )
    {
        if ( DRN::CDRNClock::NowInMicroseconds() - start > (UInt64) timeoutMs * 1000 )
            return CORE::TTaskStatus::TASKSTATUS_UNDEFINED;
        pulseGenerator->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    task = future.GetResult( 0 );
    return task.IsNULL() ? CORE::TTaskStatus::TASKSTATUS_UNDEFINED : task->GetTaskStatus();
}

/*-------------------------------------------------------------------------*/

/** The write result of a finished future */
static const CORE::CDataTableWriteResult*
GetWriteResult( const CORE::CTaskPtr& task )
{
    return task.IsNULL() ? GUCEF_NULL : static_cast< const CORE::CDataTableWriteResult* >( task->GetTaskData() );
}

/*-------------------------------------------------------------------------*/

/** The rows of a table written as CSV with the CSV table codec, by code that knows nothing of the table it reads */
static bool
ExportCsv( CORE::CIDataTable& table, const CORE::CDataTableQuery& query, CORE::CString& csv )
{
    csv.Clear();
    CORE::CDataTableScanPtr scan;
    if ( CORE::DATATABLE_STATUS_OK != table.Scan( query, scan ) )
        return false;
    CORE::CTableCodecRegistry::TTableCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetTableCodecRegistry().TryGetCsvCodec( codec ) )
        return false;

    CORE::CDynamicBuffer buffer( 1, true );
    buffer.SetDataSize( 0 );
    CORE::CDynamicBufferAccess* access = GUCEF_NEW CORE::CDynamicBufferAccess( &buffer, false );
    CORE::IOAccessPtr destination = access->CreateSharedPtr();

    CORE::CITableWriterPtr writer;
    CORE::CDataTableBatchPtr batch;
    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    while ( CORE::DATATABLE_STATUS_OK == ( status = scan->NextBatch( batch ) ) )
    {
        if ( writer.IsNULL() )
        {
            // the columns the batches have, as the schema of the file
            CORE::CDataTableSchema schema;
            for ( UInt32 c=0; c<batch->GetColumnCount(); ++c )
                schema.AddColumn( batch->GetSchema()->GetColumn( batch->GetSchemaColumnIndex( c ) ) );
            CORE::CString error;
            if ( CORE::DATATABLE_STATUS_OK != codec->CreateWriter( destination, CORE::CreateDataTableSchemaPtr( schema ), CORE::CDataTableWriterOptions(), writer, error ) )
                return false;
        }
        if ( CORE::DATATABLE_STATUS_OK != writer->WriteBatch( *batch ) )
            return false;
    }
    if ( CORE::DATATABLE_STATUS_END != status || writer.IsNULL() )
        return false;
    CORE::CDataTableFileSummary summary;
    if ( CORE::DATATABLE_STATUS_OK != writer->Finish( summary ) )
        return false;
    if ( buffer.GetDataSize() > 0 )
        csv.Append( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

/** Refuses the writes of one key, with a reason */
class CForbiddenKeyValidator : public DRN::CIDRNWriteValidator
{
    public:
    virtual bool ValidateWrite( UInt32 /* proposerPeerId */, const DRN::CDRNDataGrid& /* grid */, const CORE::CVariant& key, const CORE::CVariant& /* value */, CORE::CString& refusalReason ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( key.AsString() == "forbidden" )
        {
            refusalReason = "not today";
            return false;
        }
        return true;
    }
};

/*-------------------------------------------------------------------------*/

/** Counts the rows changed events of a table */
class CRowsChangedRecorder : public CORE::CObserver
{
    public:

    UInt32 events;
    UInt32 keys;
    GUCEF::vector< UInt32 > keysPerEvent;
    TRow lastKeys;

    CRowsChangedRecorder( void ) : CORE::CObserver(), events( 0 ), keys( 0 ), keysPerEvent(), lastKeys() {}
    virtual ~CRowsChangedRecorder() { SignalUpcomingObserverDestruction(); }

    void Watch( CORE::CIDataTable& table )
    {
        TEventCallback callback( this, &CRowsChangedRecorder::OnRowsChanged );
        SubscribeTo( &table, CORE::CIDataTable::RowsChangedEvent, callback );
    }

    private:

    typedef CORE::CTEventHandlerFunctor< CRowsChangedRecorder > TEventCallback;

    void OnRowsChanged( CORE::CNotifier* /* notifier */, const CORE::CEvent& /* eventid */, CORE::CICloneable* eventData )
    {
        const CORE::TDataTableRowsChangedEventData* data = static_cast< CORE::TDataTableRowsChangedEventData* >( eventData );
        ++events;
        keys += (UInt32) data->GetData().size();
        keysPerEvent.push_back( (UInt32) data->GetData().size() );
        lastKeys = data->GetData();
    }
};

/*-------------------------------------------------------------------------*/

/** A replica of the grid: subscribed and complete, NULL when it does not get there */
static DRN::CDRNDataGrid*
Subscribed( DRN::CDRNSession& session, const CORE::CString& name, UInt32 owner, const CORE::CVariant& key, const CORE::CVariant& value )
{
    session.Subscribe( name );
    CGridCondition subscribed;
    subscribed.Expect( session, name, owner, key, value );
    return PumpUntil( subscribed, 10000 ) ? session.FindGrid( name, owner ) : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

class CTableCondition : public CTestCondition
{
    public:

    CTableCondition( CORE::CIDataTable& table, UInt32 rows ) : m_table( table ), m_rows( rows ) {}

    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return CountRows( m_table ) == m_rows; }

    private:

    CORE::CIDataTable& m_table;
    UInt32 m_rows;
};

/*-------------------------------------------------------------------------*/

class CClosedCondition : public CTestCondition
{
    public:
    explicit CClosedCondition( DRN::CDRNDataGridTable& table ) : m_table( table ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return m_table.IsClosed(); }
    private:
    DRN::CDRNDataGridTable& m_table;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDRNDataTableTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefDRN.DataTables" );

    GUCEF_TESTFW_TESTCASE( "Tables: A typed grid has typed columns, a grid without declared types has text columns and is read only, declared types round trip through the settings" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "dt-schema", "mem" );
        REQUIRE_TRUE( group.Start( 1 ) );

        DRN::CDRNDataGrid* typed = group[ 0 ].CreateGrid( TypedGrid( "typed", DRN::DRN_AUTHORITY_OWNER, CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_FLOAT64 ) );
        DRN::CDRNDataGridSettings plain;
        plain.name = "plain";
        DRN::CDRNDataGrid* untyped = group[ 0 ].CreateGrid( plain );
        REQUIRE_TRUE( GUCEF_NULL != typed && GUCEF_NULL != untyped );
        typed->SetValue( CORE::CVariant( (UInt32) 1 ), CORE::CVariant( (Float64) 2.5 ) );
        untyped->SetValue( "hp", CORE::CVariant( (Int32) 100 ) );

        DRN::CDRNDataGridTable typedTable( group[ 0 ], *typed );
        const CORE::CDataTableSchemaPtr schema = typedTable.GetSchema();
        ASSERT_TRUE( "typed" == typedTable.GetName() && "typed" == typedTable.GetDefinition().name );
        ASSERT_TRUE( 5 == schema->GetColumnCount() );
        ASSERT_TRUE( "key" == schema->GetColumn( 0 ).name && CORE::DATATABLE_TYPE_UINT32 == schema->GetColumn( 0 ).type && !schema->GetColumn( 0 ).nullable );
        ASSERT_TRUE( "value" == schema->GetColumn( 1 ).name && CORE::DATATABLE_TYPE_FLOAT64 == schema->GetColumn( 1 ).type && schema->GetColumn( 1 ).nullable );
        ASSERT_TRUE( "revision" == schema->GetColumn( 2 ).name && CORE::DATATABLE_TYPE_UINT32 == schema->GetColumn( 2 ).type );
        ASSERT_TRUE( "lastWriter" == schema->GetColumn( 3 ).name && CORE::DATATABLE_TYPE_UINT32 == schema->GetColumn( 3 ).type );
        ASSERT_TRUE( "lastChange" == schema->GetColumn( 4 ).name && CORE::DATATABLE_TYPE_UINT64 == schema->GetColumn( 4 ).type );
        ASSERT_TRUE( 1 == schema->GetPrimaryKey().size() && 0 == schema->GetPrimaryKey()[ 0 ] );
        ASSERT_TRUE( typedTable.IsTyped() && !typedTable.IsReadOnly() && !typedTable.IsClosed() );
        ASSERT_TRUE( "OWNER" == typedTable.GetDefinition().backendOptions.GetValueAlways( "authority" ).AsString() );

        DRN::CDRNDataGridTable untypedTable( group[ 0 ], *untyped );
        ASSERT_TRUE( CORE::DATATABLE_TYPE_UTF8 == untypedTable.GetSchema()->GetColumn( 0 ).type && CORE::DATATABLE_TYPE_UTF8 == untypedTable.GetSchema()->GetColumn( 1 ).type );
        ASSERT_TRUE( !untypedTable.IsTyped() && untypedTable.IsReadOnly() );
        TRowVector rows;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == ReadRows( untypedTable, CORE::CDataTableQuery(), rows ) && 1 == rows.size() );
        ASSERT_TRUE( "hp" == rows[ 0 ][ 0 ].AsString() && "100" == rows[ 0 ][ 1 ].AsString() );

        // only one of the types declared is as good as none
        DRN::CDRNDataGridTable halfTable( group[ 0 ], *untyped, TypedTable( CORE::DATATABLE_TYPE_UTF8, CORE::DATATABLE_TYPE_UNSUPPORTED ) );
        ASSERT_TRUE( !halfTable.IsTyped() && halfTable.IsReadOnly() );

        // removed keys are rows when the table says so
        DRN::CDRNDataGridTableSettings withRemoved;
        withRemoved.includeRemoved = true;
        DRN::CDRNDataGridTable removedTable( group[ 0 ], *typed, withRemoved );
        ASSERT_TRUE( 6 == removedTable.GetSchema()->GetColumnCount() && "removed" == removedTable.GetSchema()->GetColumn( 5 ).name && CORE::DATATABLE_TYPE_BOOL == removedTable.GetSchema()->GetColumn( 5 ).type );

        // declared types are part of the settings of the grid, and only when they are declared
        CORE::CDataNode node;
        DRN::CDRNDataGridSettings declared = TypedGrid( "declared", DRN::DRN_AUTHORITY_HOST, CORE::DATATABLE_TYPE_INT64, CORE::DATATABLE_TYPE_UTF8 );
        ASSERT_TRUE( declared.SaveConfig( node ) );
        DRN::CDRNDataGridSettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( node ) );
        ASSERT_TRUE( CORE::DATATABLE_TYPE_INT64 == loaded.keyType && CORE::DATATABLE_TYPE_UTF8 == loaded.valueType );
        CORE::CDataNode plainNode;
        ASSERT_TRUE( plain.SaveConfig( plainNode ) );
        ASSERT_TRUE( plainNode.GetAttributeValueOrChildValueByName( "keyType" ).IsNULLOrEmpty() && plainNode.GetAttributeValueOrChildValueByName( "valueType" ).IsNULLOrEmpty() );
        DRN::CDRNDataGridSettings loadedPlain;
        ASSERT_TRUE( loadedPlain.LoadConfig( plainNode ) );
        ASSERT_TRUE( CORE::DATATABLE_TYPE_UNSUPPORTED == loadedPlain.keyType && CORE::DATATABLE_TYPE_UNSUPPORTED == loadedPlain.valueType );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Tables: A value or key that does not fit the declared type is shown as null or hidden, and counted" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "dt-mismatch", "mem" );
        REQUIRE_TRUE( group.Start( 1 ) );
        DRN::CDRNDataGrid* grid = group[ 0 ].CreateGrid( TypedGrid( "mixed", DRN::DRN_AUTHORITY_OWNER, CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_FLOAT64 ) );
        REQUIRE_TRUE( GUCEF_NULL != grid );
        grid->SetValue( CORE::CVariant( (UInt32) 1 ), CORE::CVariant( (Float64) 1.5 ) );
        grid->SetValue( CORE::CVariant( (UInt32) 2 ), CORE::CVariant( "not a number" ) );
        grid->SetValue( CORE::CVariant( "stray" ), CORE::CVariant( (Float64) 9 ) );
        grid->SetValue( CORE::CVariant( (UInt32) 3 ), CORE::CVariant( (Float64) 3.5 ) );

        DRN::CDRNDataGridTable table( group[ 0 ], *grid );
        TRowVector rows;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == ReadRows( table, CORE::CDataTableQuery(), rows ) );
        ASSERT_TRUE( 3 == rows.size() );
        UInt32 nulls = 0;
        for ( UInt32 i=0; i<rows.size(); ++i )
        {
            if ( 2 == rows[ i ][ 0 ].AsUInt32() )
            {
                ASSERT_TRUE( rows[ i ][ 1 ].IsNULL() );
                ++nulls;
            }
        }
        ASSERT_TRUE( 1 == nulls );
        ASSERT_TRUE( 1 == table.GetMismatchedValueCount() && 1 == table.GetMismatchedKeyCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Tables: Scans of a publisher and of a replica: all rows, projection, key equals by lookup, key in, range, limit" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "dt-scan", "mem" );
        REQUIRE_TRUE( group.Start( 2 ) );
        DRN::CDRNDataGrid* grid = group[ 0 ].CreateGrid( TypedGrid( "scores", DRN::DRN_AUTHORITY_OWNER, CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_FLOAT64 ) );
        REQUIRE_TRUE( GUCEF_NULL != grid );
        const UInt32 count = 20000;
        for ( UInt32 i=0; i<count; ++i )
            grid->SetValue( CORE::CVariant( i ), CORE::CVariant( (Float64) ( i * 1.5 ) ) );
        grid->SetValue( CORE::CVariant( (UInt32) 10 ), CORE::CVariant( (Float64) 15.25 ) );

        DRN::CDRNDataGrid* replicaGrid = Subscribed( group[ 1 ], "scores", 1, CORE::CVariant( count - 1 ), CORE::CVariant( (Float64) ( ( count - 1 ) * 1.5 ) ) );
        REQUIRE_TRUE( GUCEF_NULL != replicaGrid );

        DRN::CDRNDataGridTable publisher( group[ 0 ], *grid );
        DRN::CDRNDataGridTable replica( group[ 1 ], *replicaGrid, TypedTable( CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_FLOAT64 ) );
        CORE::CIDataTable* tables[] = { &publisher, &replica };
        for ( UInt32 t=0; t<2; ++t )
        {
            CORE::CIDataTable& table = *tables[ t ];
            TRowVector rows;
            CORE::CDataTableScanStats stats;

            // everything, with the bookkeeping columns
            ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == ReadRows( table, CORE::CDataTableQuery(), rows, &stats ) );
            ASSERT_TRUE( count == rows.size() && count == stats.rowsScanned );
            bool found = false;
            for ( UInt32 i=0; i<rows.size() && !found; ++i )
            {
                if ( 10 == rows[ i ][ 0 ].AsUInt32() )
                {
                    found = true;
                    ASSERT_TRUE( 5 == rows[ i ].size() && 15.25 == rows[ i ][ 1 ].AsFloat64() && 2 == rows[ i ][ 2 ].AsUInt32() && 1 == rows[ i ][ 3 ].AsUInt32() && rows[ i ][ 4 ].AsUInt64() > 0 );
                }
            }
            ASSERT_TRUE( found );

            // the projection
            ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == ReadRows( table, CORE::CDataTableQuery().Select( "value" ), rows ) );
            ASSERT_TRUE( count == rows.size() && 1 == rows[ 0 ].size() );

            // one key: found by lookup, the scan sees one row and not all of them
            ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == ReadRows( table, CORE::CDataTableQuery().WhereEquals( "key", CORE::CVariant( (UInt32) 777 ) ), rows, &stats ) );
            ASSERT_TRUE( 1 == rows.size() && 777 == rows[ 0 ][ 0 ].AsUInt32() && 1165.5 == rows[ 0 ][ 1 ].AsFloat64() );
            ASSERT_TRUE( stats.rowsScanned <= 1 );

            // a key that is not there
            ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == ReadRows( table, CORE::CDataTableQuery().WhereEquals( "key", CORE::CVariant( (UInt32) 4000000 ) ), rows ) && rows.empty() );

            // keys in a set, one of them is not there
            CORE::CVariant::VariantVector wanted;
            wanted.push_back( CORE::CVariant( (UInt32) 3 ) );
            wanted.push_back( CORE::CVariant( (UInt32) 5 ) );
            wanted.push_back( CORE::CVariant( (UInt32) 19999 ) );
            wanted.push_back( CORE::CVariant( (UInt32) 4000000 ) );
            ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == ReadRows( table, CORE::CDataTableQuery().WhereIn( "key", wanted ), rows, &stats ) );
            ASSERT_TRUE( 3 == rows.size() && stats.rowsScanned <= 3 );

            // a range goes over the copy of the grid
            ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == ReadRows( table, CORE::CDataTableQuery().WhereRange( "key", CORE::CVariant( (UInt32) 100 ), true, CORE::CVariant( (UInt32) 109 ), true ), rows ) );
            ASSERT_TRUE( 10 == rows.size() );
            ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == ReadRows( table, CORE::CDataTableQuery().WhereRange( "value", CORE::CVariant( (Float64) 1.5 ), true, CORE::CVariant( (Float64) 6.0 ), true ), rows ) );
            ASSERT_TRUE( 4 == rows.size() );

            // a limit
            ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == ReadRows( table, CORE::CDataTableQuery().SetLimit( 5 ), rows ) );
            ASSERT_TRUE( 5 == rows.size() );

            // an unknown column is refused with its status
            CORE::CDataTableScanPtr scan;
            ASSERT_TRUE( CORE::DATATABLE_STATUS_NOT_FOUND == table.Scan( CORE::CDataTableQuery().Select( "nosuchcolumn" ), scan ) );
            ASSERT_FALSE( table.GetLastErrorMessage().IsNULLOrEmpty() );
        }

        // the async scan: the copy is taken at the call, the work runs on the pool
        CORE::CDataTableScanPtr asyncScan;
        CORE::CFutureResult scanned = publisher.ScanAsync( CORE::CDataTableQuery(), asyncScan );
        UInt32 asyncRows = 0;
        CORE::CDataTableBatchPtr batch;
        while ( CORE::DATATABLE_STATUS_OK == asyncScan->NextBatch( batch ) )
            asyncRows += batch->GetSelectedRowCount();
        ASSERT_TRUE( count == asyncRows );
        CORE::CTaskPtr task;
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_FINISHED == AwaitPumped( scanned, task ) );

        // a scan that outlives the table it came from
        CORE::CDataTableScanPtr orphan;
        {
            DRN::CDRNDataGridTable shortLived( group[ 0 ], *grid );
            ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == shortLived.Scan( CORE::CDataTableQuery(), orphan ) );
        }
        asyncRows = 0;
        while ( CORE::DATATABLE_STATUS_OK == orphan->NextBatch( batch ) )
            asyncRows += batch->GetSelectedRowCount();
        ASSERT_TRUE( count == asyncRows );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Tables: A scan of a replica shows the state of one update tick while deltas arrive" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "dt-tick", "mem" );
        REQUIRE_TRUE( group.Start( 2 ) );
        const UInt32 keyCount = 8;
        DRN::CDRNDataGrid* grid = group[ 0 ].CreateGrid( TypedGrid( "atomic", DRN::DRN_AUTHORITY_OWNER, CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_UINT32 ) );
        REQUIRE_TRUE( GUCEF_NULL != grid );
        for ( UInt32 k=0; k<keyCount; ++k )
            grid->SetValue( CORE::CVariant( k ), CORE::CVariant( (UInt32) 0 ) );
        DRN::CDRNDataGrid* replicaGrid = Subscribed( group[ 1 ], "atomic", 1, CORE::CVariant( keyCount - 1 ), CORE::CVariant( (UInt32) 0 ) );
        REQUIRE_TRUE( GUCEF_NULL != replicaGrid );

        // one chunk: every scan is one state; chunks of three keys: a scan that ended in the tick it started in is one state
        DRN::CDRNDataGridTableSettings whole = TypedTable( CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_UINT32 );
        DRN::CDRNDataGridTableSettings chunked = whole;
        chunked.scanChunkKeyCount = 3;
        DRN::CDRNDataGridTable wholeTable( group[ 1 ], *replicaGrid, whole );
        DRN::CDRNDataGridTable chunkedTable( group[ 1 ], *replicaGrid, chunked );
        UInt32 scansInOneTick = 0;
        UInt32 distinct = 0;
        UInt32 lastSeen = 0;
        for ( UInt32 round=1; round<=150; ++round )
        {
            for ( UInt32 k=0; k<keyCount; ++k )
                grid->SetValue( CORE::CVariant( k ), CORE::CVariant( round ) );
            PumpFor( 1 );

            CORE::CIDataTable* tables[] = { &wholeTable, &chunkedTable };
            for ( UInt32 t=0; t<2; ++t )
            {
                TRowVector rows;
                ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == ReadRows( *tables[ t ], CORE::CDataTableQuery().Select( "key" ).Select( "value" ), rows ) );
                ASSERT_TRUE( keyCount == rows.size() );
                bool same = true;
                for ( UInt32 i=1; i<rows.size(); ++i )
                    same = same && rows[ i ][ 1 ].AsUInt32() == rows[ 0 ][ 1 ].AsUInt32();
                UInt32 firstTick = 0;
                UInt32 lastTick = 0;
                DRN::CDRNDataGridTable* inspected = 0 == t ? &wholeTable : &chunkedTable;
                ASSERT_TRUE( inspected->GetLastScanTicks( firstTick, lastTick ) );
                if ( 0 == t || firstTick == lastTick )
                {
                    ASSERT_TRUE( same );
                    ++scansInOneTick;
                }
                if ( 0 == t && rows[ 0 ][ 1 ].AsUInt32() != lastSeen )
                {
                    lastSeen = rows[ 0 ][ 1 ].AsUInt32();
                    ++distinct;
                }
            }
        }
        // the checks were not trivial: scans saw the state change, and almost all were within one tick
        ASSERT_TRUE( distinct >= 20 );
        ASSERT_TRUE( scansInOneTick >= 200 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Tables: Rows are written and deleted on the publisher, INSERT conflicts, REPLACE_PARTITIONS is unsupported, a wrong type and a read only table are refused" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "dt-write", "mem" );
        REQUIRE_TRUE( group.Start( 1 ) );
        DRN::CDRNDataGrid* grid = group[ 0 ].CreateGrid( TypedGrid( "store", DRN::DRN_AUTHORITY_OWNER, CORE::DATATABLE_TYPE_UTF8, CORE::DATATABLE_TYPE_INT32 ) );
        REQUIRE_TRUE( GUCEF_NULL != grid );
        DRN::CDRNDataGridTable table( group[ 0 ], *grid );

        TRow keys;
        TRow values;
        keys.push_back( CORE::CVariant( "gold" ) );
        values.push_back( CORE::CVariant( (Int32) 10 ) );
        keys.push_back( CORE::CVariant( "wood" ) );
        values.push_back( CORE::CVariant( (Int32) 20 ) );
        CORE::CDataTableWriteResult result;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == table.Write( *MakeRows( table, keys, values ), CORE::DATATABLE_WRITE_UPSERT, result ) );
        ASSERT_TRUE( 2 == result.partitions.size() && CORE::DATATABLE_STATUS_OK == result.GetOverallStatus() && 2 == result.GetTotalRowsWritten() );
        CORE::CVariant value;
        ASSERT_TRUE( grid->GetValue( "gold", value ) && 10 == value.AsInt32() && grid->GetValue( "wood", value ) && 20 == value.AsInt32() );
        ASSERT_TRUE( 2 == CountRows( table ) );

        // upsert replaces
        keys.clear();
        values.clear();
        keys.push_back( CORE::CVariant( "gold" ) );
        values.push_back( CORE::CVariant( (Int32) 11 ) );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == table.Write( *MakeRows( table, keys, values ), CORE::DATATABLE_WRITE_UPSERT, result ) );
        ASSERT_TRUE( grid->GetValue( "gold", value ) && 11 == value.AsInt32() && 2 == grid->GetRevision( grid->GetKeyIndex( "gold" ) ) );

        // insert of a key that is there fails that row and no other
        keys.push_back( CORE::CVariant( "iron" ) );
        values.push_back( CORE::CVariant( (Int32) 5 ) );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_CONFLICT == table.Write( *MakeRows( table, keys, values ), CORE::DATATABLE_WRITE_INSERT, result ) );
        ASSERT_TRUE( 2 == result.partitions.size() && CORE::DATATABLE_STATUS_CONFLICT == result.partitions[ 0 ].status && CORE::DATATABLE_STATUS_OK == result.partitions[ 1 ].status );
        ASSERT_TRUE( "key=gold" == result.partitions[ 0 ].path && result.partitions[ 0 ].message.HasSubstr( "exists" ) >= 0 );
        ASSERT_FALSE( table.GetLastErrorMessage().IsNULLOrEmpty() );
        ASSERT_TRUE( grid->GetValue( "gold", value ) && 11 == value.AsInt32() && grid->GetValue( "iron", value ) && 5 == value.AsInt32() );

        // no partitions to replace, none to delete or compact
        ASSERT_TRUE( CORE::DATATABLE_STATUS_UNSUPPORTED == table.Write( *MakeRows( table, keys, values ), CORE::DATATABLE_WRITE_REPLACE_PARTITIONS, result ) );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_UNSUPPORTED == table.DeletePartitions( CORE::CDataTableQuery(), result ) );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_UNSUPPORTED == table.Compact( CORE::CDataTableQuery(), true, result ) );
        CORE::CDataTablePartitionInfo::TPartitionInfoVector partitions;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == table.ListPartitions( CORE::CDataTableQuery(), true, partitions ) && partitions.empty() );

        // rows of another type than the columns
        CORE::CDataTableSchema wrong;
        wrong.AddColumn( CORE::CDataTableColumnDef( "key", CORE::DATATABLE_TYPE_UTF8, false ) );
        wrong.AddColumn( CORE::CDataTableColumnDef( "value", CORE::DATATABLE_TYPE_FLOAT64 ) );
        CORE::CDataTableBatchBuilder wrongBuilder( CORE::CreateDataTableSchemaPtr( wrong ), CORE::CCoreGlobal::Instance()->GetDataTableBufferPool() );
        wrongBuilder.GetColumn( 0 ).Append( CORE::CVariant( "silver" ) );
        wrongBuilder.GetColumn( 1 ).Append( CORE::CVariant( (Float64) 2.5 ) );
        CORE::CDataTableBatchPtr wrongRows;
        CORE::CString error;
        REQUIRE_TRUE( CORE::DATATABLE_STATUS_OK == wrongBuilder.Build( wrongRows, error ) );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_TYPE_MISMATCH == table.Write( *wrongRows, CORE::DATATABLE_WRITE_UPSERT, result ) );
        ASSERT_FALSE( grid->HasKey( "silver" ) );

        // delete by predicate
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == table.DeleteRows( CORE::CDataTableQuery().WhereEquals( "key", CORE::CVariant( "wood" ) ), result ) );
        ASSERT_TRUE( 1 == result.partitions.size() && 1 == result.partitions[ 0 ].rowsDeleted );
        ASSERT_FALSE( grid->HasKey( "wood" ) );
        ASSERT_TRUE( 2 == CountRows( table ) );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == table.DeleteRows( CORE::CDataTableQuery().WhereRange( "value", CORE::CVariant( (Int32) 0 ), true, CORE::CVariant( (Int32) 100 ), true ), result ) );
        ASSERT_TRUE( 2 == result.partitions.size() && 0 == grid->GetKeyCount() && 0 == CountRows( table ) );

        // the async forms on the publisher run on the pool
        keys.clear();
        values.clear();
        keys.push_back( CORE::CVariant( "pool" ) );
        values.push_back( CORE::CVariant( (Int32) 1 ) );
        CORE::CTaskPtr task;
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_FINISHED == AwaitPumped( table.WriteAsync( MakeRows( table, keys, values ), CORE::DATATABLE_WRITE_UPSERT ), task ) );
        ASSERT_TRUE( GUCEF_NULL != GetWriteResult( task ) && 1 == GetWriteResult( task )->GetTotalRowsWritten() && grid->HasKey( "pool" ) );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED == AwaitPumped( table.WriteAsync( MakeRows( table, keys, values ), CORE::DATATABLE_WRITE_INSERT ), task ) );
        ASSERT_TRUE( task->GetTaskStatusExtraInfo().HasSubstr( "CONFLICT" ) >= 0 );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_FINISHED == AwaitPumped( table.DeleteRowsAsync( CORE::CDataTableQuery() ), task ) );
        ASSERT_FALSE( grid->HasKey( "pool" ) );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED == AwaitPumped( table.DeletePartitionsAsync( CORE::CDataTableQuery() ), task ) );

        // a table opened read only refuses changes whatever the grid allows
        DRN::CDRNDataGridTableSettings readOnly;
        readOnly.readOnly = true;
        DRN::CDRNDataGridTable readOnlyTable( group[ 0 ], *grid, readOnly );
        ASSERT_TRUE( readOnlyTable.IsReadOnly() );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_READ_ONLY == readOnlyTable.Write( *MakeRows( readOnlyTable, keys, values ), CORE::DATATABLE_WRITE_UPSERT, result ) );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_READ_ONLY == readOnlyTable.DeleteRows( CORE::CDataTableQuery(), result ) );
        ASSERT_FALSE( grid->HasKey( "pool" ) );

        // and so does a table of a grid without declared types
        DRN::CDRNDataGridSettings plain;
        plain.name = "plainstore";
        DRN::CDRNDataGrid* plainGrid = group[ 0 ].CreateGrid( plain );
        REQUIRE_TRUE( GUCEF_NULL != plainGrid );
        DRN::CDRNDataGridTable plainTable( group[ 0 ], *plainGrid );
        CORE::CDataTableSchema texts;
        texts.AddColumn( CORE::CDataTableColumnDef( "key", CORE::DATATABLE_TYPE_UTF8, false ) );
        texts.AddColumn( CORE::CDataTableColumnDef( "value", CORE::DATATABLE_TYPE_UTF8 ) );
        CORE::CDataTableBatchBuilder textBuilder( CORE::CreateDataTableSchemaPtr( texts ), CORE::CCoreGlobal::Instance()->GetDataTableBufferPool() );
        textBuilder.GetColumn( 0 ).Append( CORE::CVariant( "a" ) );
        textBuilder.GetColumn( 1 ).Append( CORE::CVariant( "b" ) );
        CORE::CDataTableBatchPtr textRows;
        REQUIRE_TRUE( CORE::DATATABLE_STATUS_OK == textBuilder.Build( textRows, error ) );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_READ_ONLY == plainTable.Write( *textRows, CORE::DATATABLE_WRITE_UPSERT, result ) );
        ASSERT_FALSE( plainGrid->HasKey( "a" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Tables: Writes on a replica of a HOST grid are proposals: sent, accepted, refused by the validator with the reason" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "dt-host", "mem" );
        REQUIRE_TRUE( group.Start( 2 ) );
        DRN::CDRNDataGrid* host = group[ 0 ].CreateGrid( TypedGrid( "rules", DRN::DRN_AUTHORITY_HOST, CORE::DATATABLE_TYPE_UTF8, CORE::DATATABLE_TYPE_INT32 ) );
        REQUIRE_TRUE( GUCEF_NULL != host );
        CForbiddenKeyValidator validator;
        host->SetWriteValidator( &validator );
        host->SetValue( "gravity", CORE::CVariant( (Int32) 10 ) );
        DRN::CDRNDataGrid* replicaGrid = Subscribed( group[ 1 ], "rules", 1, "gravity", CORE::CVariant( (Int32) 10 ) );
        REQUIRE_TRUE( GUCEF_NULL != replicaGrid );

        DRN::CDRNDataGridTable replica( group[ 1 ], *replicaGrid, TypedTable( CORE::DATATABLE_TYPE_UTF8, CORE::DATATABLE_TYPE_INT32 ) );
        ASSERT_FALSE( replica.IsReadOnly() );
        ASSERT_TRUE( 0 == replica.GetGridSubscriptionCount() );

        // sent: the result says so, the answer is not waited for
        TRow keys;
        TRow values;
        keys.push_back( CORE::CVariant( "speed" ) );
        values.push_back( CORE::CVariant( (Int32) 5 ) );
        CORE::CDataTableWriteResult result;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == replica.Write( *MakeRows( replica, keys, values ), CORE::DATATABLE_WRITE_UPSERT, result ) );
        ASSERT_TRUE( 1 == result.partitions.size() && "proposed" == result.partitions[ 0 ].message );
        CGridCondition applied;
        applied.Expect( group[ 0 ], "rules", 1, "speed", CORE::CVariant( (Int32) 5 ) );
        applied.Expect( group[ 1 ], "rules", 1, "speed", CORE::CVariant( (Int32) 5 ) );
        ASSERT_TRUE( PumpUntil( applied, 10000 ) );
        ASSERT_TRUE( 2 == host->GetLastWriter( host->GetKeyIndex( "speed" ) ) );

        // answered: both rows of one write, one accepted and one refused with the reason of the validator
        keys.clear();
        values.clear();
        keys.push_back( CORE::CVariant( "forbidden" ) );
        values.push_back( CORE::CVariant( (Int32) 1 ) );
        keys.push_back( CORE::CVariant( "jump" ) );
        values.push_back( CORE::CVariant( (Int32) 3 ) );
        CORE::CFutureResult written = replica.WriteAsync( MakeRows( replica, keys, values ), CORE::DATATABLE_WRITE_UPSERT );
        ASSERT_TRUE( 0 != replica.GetGridSubscriptionCount() );
        CORE::CTaskPtr task;
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_FINISHED == AwaitPumped( written, task ) );
        const CORE::CDataTableWriteResult* answers = GetWriteResult( task );
        REQUIRE_TRUE( GUCEF_NULL != answers && 2 == answers->partitions.size() );
        ASSERT_TRUE( "key=forbidden" == answers->partitions[ 0 ].path && CORE::DATATABLE_STATUS_OK != answers->partitions[ 0 ].status && "not today" == answers->partitions[ 0 ].message );
        ASSERT_TRUE( "key=jump" == answers->partitions[ 1 ].path && CORE::DATATABLE_STATUS_OK == answers->partitions[ 1 ].status && "accepted" == answers->partitions[ 1 ].message );
        ASSERT_FALSE( host->HasKey( "forbidden" ) );
        CGridCondition jumped;
        jumped.Expect( group[ 0 ], "rules", 1, "jump", CORE::CVariant( (Int32) 3 ) );
        ASSERT_TRUE( PumpUntil( jumped, 10000 ) );
        ASSERT_TRUE( 0 == replica.GetGridSubscriptionCount() );

        // a delete is a proposal too
        CORE::CFutureResult removed = replica.DeleteRowsAsync( CORE::CDataTableQuery().WhereEquals( "key", CORE::CVariant( "speed" ) ) );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_FINISHED == AwaitPumped( removed, task ) );
        ASSERT_TRUE( GUCEF_NULL != GetWriteResult( task ) && 1 == GetWriteResult( task )->partitions.size() && CORE::DATATABLE_STATUS_OK == GetWriteResult( task )->partitions[ 0 ].status );
        CGridCondition gone;
        gone.ExpectMissing( group[ 0 ], "rules", 1, "speed" );
        ASSERT_TRUE( PumpUntil( gone, 10000 ) );

        // an INSERT of a key that exists is refused by the table, before anything is sent
        keys.clear();
        values.clear();
        keys.push_back( CORE::CVariant( "gravity" ) );
        values.push_back( CORE::CVariant( (Int32) 99 ) );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_CONFLICT == replica.Write( *MakeRows( replica, keys, values ), CORE::DATATABLE_WRITE_INSERT, result ) );
        PumpFor( 50 );
        CORE::CVariant gravity;
        ASSERT_TRUE( host->GetValue( "gravity", gravity ) && 10 == gravity.AsInt32() );

        // two writes at once are answered one after the other
        keys.clear();
        values.clear();
        keys.push_back( CORE::CVariant( "a1" ) );
        values.push_back( CORE::CVariant( (Int32) 1 ) );
        TRow otherKeys;
        TRow otherValues;
        otherKeys.push_back( CORE::CVariant( "forbidden" ) );
        otherValues.push_back( CORE::CVariant( (Int32) 2 ) );
        CORE::CFutureResult first = replica.WriteAsync( MakeRows( replica, keys, values ), CORE::DATATABLE_WRITE_UPSERT );
        CORE::CFutureResult second = replica.WriteAsync( MakeRows( replica, otherKeys, otherValues ), CORE::DATATABLE_WRITE_UPSERT );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_FINISHED == AwaitPumped( first, task ) );
        ASSERT_TRUE( GUCEF_NULL != GetWriteResult( task ) && CORE::DATATABLE_STATUS_OK == GetWriteResult( task )->partitions[ 0 ].status );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_FINISHED == AwaitPumped( second, task ) );
        ASSERT_TRUE( GUCEF_NULL != GetWriteResult( task ) && CORE::DATATABLE_STATUS_OK != GetWriteResult( task )->partitions[ 0 ].status );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Tables: A replica of an OWNER grid is read only, the owner writes; a voted grid answers a write when the vote ended, yes and no" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "dt-owner", "mem" );
        REQUIRE_TRUE( group.Start( 4 ) );

        DRN::CDRNDataGrid* mine = group[ 1 ].CreateGrid( TypedGrid( "mine", DRN::DRN_AUTHORITY_OWNER, CORE::DATATABLE_TYPE_UTF8, CORE::DATATABLE_TYPE_INT32 ) );
        REQUIRE_TRUE( GUCEF_NULL != mine );
        mine->SetValue( "gold", CORE::CVariant( (Int32) 3 ) );
        DRN::CDRNDataGrid* theirs = Subscribed( group[ 2 ], "mine", 2, "gold", CORE::CVariant( (Int32) 3 ) );
        REQUIRE_TRUE( GUCEF_NULL != theirs );
        DRN::CDRNDataGridTable theirTable( group[ 2 ], *theirs, TypedTable( CORE::DATATABLE_TYPE_UTF8, CORE::DATATABLE_TYPE_INT32 ) );
        DRN::CDRNDataGridTable ownTable( group[ 1 ], *mine );
        ASSERT_TRUE( theirTable.IsReadOnly() && !ownTable.IsReadOnly() );
        TRow keys;
        TRow values;
        keys.push_back( CORE::CVariant( "gold" ) );
        values.push_back( CORE::CVariant( (Int32) 1000 ) );
        CORE::CDataTableWriteResult result;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_READ_ONLY == theirTable.Write( *MakeRows( theirTable, keys, values ), CORE::DATATABLE_WRITE_UPSERT, result ) );
        CORE::CTaskPtr task;
        ASSERT_TRUE( theirTable.WriteAsync( MakeRows( theirTable, keys, values ), CORE::DATATABLE_WRITE_UPSERT ).HasNoFuture() );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == ownTable.Write( *MakeRows( ownTable, keys, values ), CORE::DATATABLE_WRITE_UPSERT, result ) );
        CGridCondition rich;
        rich.Expect( group[ 2 ], "mine", 2, "gold", CORE::CVariant( (Int32) 1000 ) );
        ASSERT_TRUE( PumpUntil( rich, 10000 ) );

        // a voted grid of the sequencer, the others vote
        DRN::CDRNDataGridSettings ballotSettings = TypedGrid( "ballot", DRN::DRN_AUTHORITY_PROPOSAL_VOTE, CORE::DATATABLE_TYPE_UTF8, CORE::DATATABLE_TYPE_UTF8 );
        ballotSettings.votePolicy.rule = DRN::CDRNVotePolicy::VOTERULE_MAJORITY;
        ballotSettings.votePolicy.timeoutMs = 1000;
        DRN::CDRNDataGrid* ballot = group[ 0 ].CreateGrid( ballotSettings );
        REQUIRE_TRUE( GUCEF_NULL != ballot );
        ballot->SetValue( "map", CORE::CVariant( "office" ) );
        CItemRecorder recorders[ 4 ];
        DRN::CDRNDataGrid* ballots[ 4 ] = { ballot, GUCEF_NULL, GUCEF_NULL, GUCEF_NULL };
        for ( UInt32 i=1; i<4; ++i )
        {
            ballots[ i ] = Subscribed( group[ i ], "ballot", 1, "map", CORE::CVariant( "office" ) );
            REQUIRE_TRUE( GUCEF_NULL != ballots[ i ] );
            recorders[ i ].SubscribeTo( ballots[ i ] );
        }
        DRN::CDRNDataGridTable proposer( group[ 1 ], *ballots[ 1 ], TypedTable( CORE::DATATABLE_TYPE_UTF8, CORE::DATATABLE_TYPE_UTF8 ) );

        keys.clear();
        values.clear();
        keys.push_back( CORE::CVariant( "map" ) );
        values.push_back( CORE::CVariant( "dust" ) );
        recorders[ 2 ].autoVote = CItemRecorder::VOTE_YES;
        recorders[ 3 ].autoVote = CItemRecorder::VOTE_YES;
        CORE::CFutureResult yes = proposer.WriteAsync( MakeRows( proposer, keys, values ), CORE::DATATABLE_WRITE_UPSERT );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_FINISHED == AwaitPumped( yes, task ) );
        ASSERT_TRUE( GUCEF_NULL != GetWriteResult( task ) && CORE::DATATABLE_STATUS_OK == GetWriteResult( task )->partitions[ 0 ].status );
        CORE::CVariant map;
        ASSERT_TRUE( ballot->GetValue( "map", map ) && "dust" == map.AsString() );

        values[ 0 ] = CORE::CVariant( "nuke" );
        recorders[ 2 ].autoVote = CItemRecorder::VOTE_NO;
        recorders[ 3 ].autoVote = CItemRecorder::VOTE_NO;
        CORE::CFutureResult no = proposer.WriteAsync( MakeRows( proposer, keys, values ), CORE::DATATABLE_WRITE_UPSERT );
        ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_FINISHED == AwaitPumped( no, task ) );
        REQUIRE_TRUE( GUCEF_NULL != GetWriteResult( task ) );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK != GetWriteResult( task )->partitions[ 0 ].status && GetWriteResult( task )->partitions[ 0 ].message.HasSubstr( "vote failed" ) >= 0 );
        PumpFor( 50 );
        ASSERT_TRUE( ballot->GetValue( "map", map ) && "dust" == map.AsString() );

        // the sequencer's own table writes the voted grid at once
        DRN::CDRNDataGridTable sequencerTable( group[ 0 ], *ballot );
        values[ 0 ] = CORE::CVariant( "train" );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == sequencerTable.Write( *MakeRows( sequencerTable, keys, values ), CORE::DATATABLE_WRITE_UPSERT, result ) );
        ASSERT_TRUE( ballot->GetValue( "map", map ) && "train" == map.AsString() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Tables: RowsChangedEvent comes per packet with the changed keys, and the grid has no observer of the table while nobody listens" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "dt-events", "mem" );
        REQUIRE_TRUE( group.Start( 2 ) );
        DRN::CDRNDataGrid* grid = group[ 0 ].CreateGrid( TypedGrid( "events", DRN::DRN_AUTHORITY_OWNER, CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_UINT32 ) );
        REQUIRE_TRUE( GUCEF_NULL != grid );
        grid->SetValue( CORE::CVariant( (UInt32) 0 ), CORE::CVariant( (UInt32) 0 ) );
        DRN::CDRNDataGrid* replicaGrid = Subscribed( group[ 1 ], "events", 1, CORE::CVariant( (UInt32) 0 ), CORE::CVariant( (UInt32) 0 ) );
        REQUIRE_TRUE( GUCEF_NULL != replicaGrid );
        replicaGrid->SetNotifyMode( DRN::DRN_NOTIFY_PER_PACKET );
        DRN::CDRNDataGridTable table( group[ 1 ], *replicaGrid, TypedTable( CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_UINT32 ) );

        // nobody listens: the grid has nothing of the table
        ASSERT_TRUE( 0 == replicaGrid->GetSubscriptionCountForObserver( &table.AsObserver() ) );
        ASSERT_TRUE( 0 == table.GetGridSubscriptionCount() );
        for ( UInt32 k=1; k<=5; ++k )
            grid->SetValue( CORE::CVariant( k ), CORE::CVariant( k * 10 ) );
        PumpFor( 100 );
        ASSERT_TRUE( 0 == replicaGrid->GetSubscriptionCountForObserver( &table.AsObserver() ) );
        ASSERT_TRUE( 6 == CountRows( table ) );

        // one observer: the grid has the table; five keys changed in one tick make one event
        CRowsChangedRecorder recorder;
        recorder.Watch( table );
        ASSERT_TRUE( 0 != replicaGrid->GetSubscriptionCountForObserver( &table.AsObserver() ) );
        for ( UInt32 k=1; k<=5; ++k )
            grid->SetValue( CORE::CVariant( k ), CORE::CVariant( k * 100 ) );
        ASSERT_TRUE( PumpUntil( recorder.events, 1 ) );
        PumpFor( 100 );
        ASSERT_TRUE( 1 == recorder.events && 5 == recorder.keys && 5 == recorder.lastKeys.size() );
        UInt32 sum = 0;
        for ( UInt32 i=0; i<recorder.lastKeys.size(); ++i )
            sum += recorder.lastKeys[ i ].AsUInt32();
        ASSERT_TRUE( 15 == sum );

        // a removal is a change too
        grid->RemoveKey( CORE::CVariant( (UInt32) 3 ) );
        ASSERT_TRUE( PumpUntil( recorder.events, 2 ) );
        ASSERT_TRUE( 1 == recorder.lastKeys.size() && 3 == recorder.lastKeys[ 0 ].AsUInt32() );

        // nobody listens again: the grid is free of the table
        recorder.UnsubscribeFrom( table );
        ASSERT_TRUE( 0 == replicaGrid->GetSubscriptionCountForObserver( &table.AsObserver() ) );
        grid->SetValue( CORE::CVariant( (UInt32) 1 ), CORE::CVariant( (UInt32) 1 ) );
        PumpFor( 100 );
        ASSERT_TRUE( 2 == recorder.events );

        // an observer that goes without unsubscribing
        {
            CRowsChangedRecorder fleeting;
            fleeting.Watch( table );
            ASSERT_TRUE( 0 != replicaGrid->GetSubscriptionCountForObserver( &table.AsObserver() ) );
        }
        ASSERT_TRUE( 0 == replicaGrid->GetSubscriptionCountForObserver( &table.AsObserver() ) );

        // an observer of everything counts, an observer of other events does not
        CRowsChangedRecorder everything;
        everything.SubscribeTo( &table );
        ASSERT_TRUE( 0 != replicaGrid->GetSubscriptionCountForObserver( &table.AsObserver() ) );
        everything.UnsubscribeFrom( table );
        ASSERT_TRUE( 0 == replicaGrid->GetSubscriptionCountForObserver( &table.AsObserver() ) );
        CRowsChangedRecorder partitions;
        partitions.SubscribeTo( &table, CORE::CIDataTable::PartitionWrittenEvent );
        ASSERT_TRUE( 0 == replicaGrid->GetSubscriptionCountForObserver( &table.AsObserver() ) );

        // a grid that tells each change (the default) is followed too
        DRN::CDRNDataGrid* perChange = group[ 0 ].CreateGrid( TypedGrid( "perchange", DRN::DRN_AUTHORITY_OWNER, CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_UINT32 ) );
        REQUIRE_TRUE( GUCEF_NULL != perChange );
        perChange->SetValue( CORE::CVariant( (UInt32) 0 ), CORE::CVariant( (UInt32) 0 ) );
        DRN::CDRNDataGrid* perChangeReplica = Subscribed( group[ 1 ], "perchange", 1, CORE::CVariant( (UInt32) 0 ), CORE::CVariant( (UInt32) 0 ) );
        REQUIRE_TRUE( GUCEF_NULL != perChangeReplica );
        DRN::CDRNDataGridTable perChangeTable( group[ 1 ], *perChangeReplica, TypedTable( CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_UINT32 ) );
        CRowsChangedRecorder changes;
        changes.Watch( perChangeTable );
        perChange->SetValue( CORE::CVariant( (UInt32) 1 ), CORE::CVariant( (UInt32) 1 ) );
        perChange->SetValue( CORE::CVariant( (UInt32) 2 ), CORE::CVariant( (UInt32) 2 ) );
        ASSERT_TRUE( PumpUntil( changes.events, 2 ) );
        ASSERT_TRUE( 2 == changes.keys );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Tables: A table ends when the item is removed and when the session ends; its async writes end with it" )
    try
    {
        // the item is removed
        {
            CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "dt-removed", "mem" );
            REQUIRE_TRUE( group.Start( 2 ) );
            DRN::CDRNDataGrid* grid = group[ 1 ].CreateGrid( TypedGrid( "short", DRN::DRN_AUTHORITY_OWNER, CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_UINT32 ) );
            REQUIRE_TRUE( GUCEF_NULL != grid );
            grid->SetValue( CORE::CVariant( (UInt32) 1 ), CORE::CVariant( (UInt32) 1 ) );
            DRN::CDRNDataGrid* replicaGrid = Subscribed( group[ 0 ], "short", 2, CORE::CVariant( (UInt32) 1 ), CORE::CVariant( (UInt32) 1 ) );
            REQUIRE_TRUE( GUCEF_NULL != replicaGrid );
            DRN::CDRNDataGridTable replica( group[ 0 ], *replicaGrid, TypedTable( CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_UINT32 ) );
            DRN::CDRNDataGridTable origin( group[ 1 ], *grid );
            ASSERT_TRUE( !replica.IsClosed() && !origin.IsClosed() && 1 == CountRows( replica ) );

            ASSERT_TRUE( group[ 1 ].GetReplicator().RemoveItem( grid ) );
            CClosedCondition replicaClosed( replica );
            ASSERT_TRUE( PumpUntil( replicaClosed, 10000 ) );
            ASSERT_TRUE( origin.IsClosed() );
            CORE::CDataTableScanPtr scan;
            ASSERT_TRUE( CORE::DATATABLE_STATUS_NOT_FOUND == replica.Scan( CORE::CDataTableQuery(), scan ) );
            ASSERT_TRUE( CORE::DATATABLE_STATUS_NOT_FOUND == origin.Scan( CORE::CDataTableQuery(), scan ) );
            ASSERT_FALSE( replica.GetLastErrorMessage().IsNULLOrEmpty() );
            ASSERT_TRUE( replica.IsReadOnly() );
            CORE::CDataTableWriteResult result;
            TRow keys;
            TRow values;
            keys.push_back( CORE::CVariant( (UInt32) 5 ) );
            values.push_back( CORE::CVariant( (UInt32) 5 ) );
            ASSERT_TRUE( CORE::DATATABLE_STATUS_NOT_FOUND == origin.Write( *MakeRows( origin, keys, values ), CORE::DATATABLE_WRITE_UPSERT, result ) );
            ASSERT_TRUE( CORE::DATATABLE_STATUS_NOT_FOUND == origin.DeleteRows( CORE::CDataTableQuery(), result ) );
        }

        // the session ends while the table is open, and while a write waits for the publisher
        {
            CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "dt-ended", "mem" );
            REQUIRE_TRUE( group.Start( 2 ) );
            DRN::CDRNDataGrid* host = group[ 0 ].CreateGrid( TypedGrid( "ended", DRN::DRN_AUTHORITY_HOST, CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_UINT32 ) );
            REQUIRE_TRUE( GUCEF_NULL != host );
            host->SetValue( CORE::CVariant( (UInt32) 1 ), CORE::CVariant( (UInt32) 1 ) );
            DRN::CDRNDataGrid* replicaGrid = Subscribed( group[ 1 ], "ended", 1, CORE::CVariant( (UInt32) 1 ), CORE::CVariant( (UInt32) 1 ) );
            REQUIRE_TRUE( GUCEF_NULL != replicaGrid );
            DRN::CDRNDataGridTable replica( group[ 1 ], *replicaGrid, TypedTable( CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_UINT32 ) );
            ASSERT_TRUE( 1 == CountRows( replica ) );

            TRow keys;
            TRow values;
            keys.push_back( CORE::CVariant( (UInt32) 2 ) );
            values.push_back( CORE::CVariant( (UInt32) 2 ) );
            CORE::CFutureResult pending = replica.WriteAsync( MakeRows( replica, keys, values ), CORE::DATATABLE_WRITE_UPSERT );
            ASSERT_FALSE( pending.IsReady() );

            // the nodes go; the tables outlive them, and answer that their grid is gone
            group.Stop();
            ASSERT_TRUE( replica.IsClosed() );
            CORE::CDataTableScanPtr scan;
            ASSERT_TRUE( CORE::DATATABLE_STATUS_NOT_FOUND == replica.Scan( CORE::CDataTableQuery(), scan ) );
            CORE::CTaskPtr task;
            ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED == AwaitPumped( pending, task, 2000 ) );
            ASSERT_TRUE( task->GetTaskStatusExtraInfo().HasSubstr( "NOT_FOUND" ) >= 0 );
            ASSERT_TRUE( replica.WriteAsync( MakeRows( replica, keys, values ), CORE::DATATABLE_WRITE_UPSERT ).HasNoFuture() );
        }

        // a write that is not answered ends with the timeout
        {
            CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "dt-timeout", "mem" );
            REQUIRE_TRUE( group.Start( 3 ) );
            DRN::CDRNDataGridSettings silentSettings = TypedGrid( "silent", DRN::DRN_AUTHORITY_PROPOSAL_VOTE, CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_UINT32 );
            silentSettings.votePolicy.rule = DRN::CDRNVotePolicy::VOTERULE_MAJORITY;
            silentSettings.votePolicy.timeoutMs = 60000;
            DRN::CDRNDataGrid* host = group[ 0 ].CreateGrid( silentSettings );
            REQUIRE_TRUE( GUCEF_NULL != host );
            host->SetValue( CORE::CVariant( (UInt32) 1 ), CORE::CVariant( (UInt32) 1 ) );
            DRN::CDRNDataGrid* proposerGrid = Subscribed( group[ 1 ], "silent", 1, CORE::CVariant( (UInt32) 1 ), CORE::CVariant( (UInt32) 1 ) );
            DRN::CDRNDataGrid* voterGrid = Subscribed( group[ 2 ], "silent", 1, CORE::CVariant( (UInt32) 1 ), CORE::CVariant( (UInt32) 1 ) );
            REQUIRE_TRUE( GUCEF_NULL != proposerGrid && GUCEF_NULL != voterGrid );

            // the voter never votes
            DRN::CDRNDataGridTableSettings impatient = TypedTable( CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_UINT32 );
            impatient.writeTimeoutMs = 300;
            DRN::CDRNDataGridTable proposer( group[ 1 ], *proposerGrid, impatient );
            TRow keys;
            TRow values;
            keys.push_back( CORE::CVariant( (UInt32) 2 ) );
            values.push_back( CORE::CVariant( (UInt32) 2 ) );
            const UInt64 start = DRN::CDRNClock::NowInMicroseconds();
            CORE::CTaskPtr task;
            ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED == AwaitPumped( proposer.WriteAsync( MakeRows( proposer, keys, values ), CORE::DATATABLE_WRITE_UPSERT ), task, 5000 ) );
            ASSERT_TRUE( task->GetTaskStatusExtraInfo().HasSubstr( "did not answer" ) >= 0 );
            ASSERT_TRUE( DRN::CDRNClock::NowInMicroseconds() - start >= 250000 );
            ASSERT_TRUE( 0 == proposer.GetGridSubscriptionCount() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Tables: Scans run against writes and against incoming deltas, and a replica scan within one tick is never torn" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "dt-concurrent", "mem" );
        REQUIRE_TRUE( group.Start( 2 ) );
        const UInt32 keyCount = 8;
        DRN::CDRNDataGrid* grid = group[ 0 ].CreateGrid( TypedGrid( "conc", DRN::DRN_AUTHORITY_OWNER, CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_UINT32 ) );
        REQUIRE_TRUE( GUCEF_NULL != grid );
        for ( UInt32 k=0; k<keyCount; ++k )
            grid->SetValue( CORE::CVariant( k ), CORE::CVariant( (UInt32) 0 ) );
        DRN::CDRNDataGrid* replicaGrid = Subscribed( group[ 1 ], "conc", 1, CORE::CVariant( keyCount - 1 ), CORE::CVariant( (UInt32) 0 ) );
        REQUIRE_TRUE( GUCEF_NULL != replicaGrid );

        DRN::CDRNDataGridTableSettings chunked = TypedTable( CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_UINT32 );
        chunked.scanChunkKeyCount = 3;
        DRN::CDRNDataGridTable publisherTable( group[ 0 ], *grid );
        DRN::CDRNDataGridTable replicaTable( group[ 1 ], *replicaGrid, TypedTable( CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_UINT32 ) );
        DRN::CDRNDataGridTable chunkedTable( group[ 1 ], *replicaGrid, chunked );
        CRowsChangedRecorder recorder;
        recorder.Watch( replicaTable );

        CTableScanThread replicaScanner( true );
        replicaScanner.AddTable( &replicaTable );
        replicaScanner.AddTable( &chunkedTable );
        CTableScanThread publisherScanner( false );
        publisherScanner.AddTable( &publisherTable );
        REQUIRE_TRUE( replicaScanner.Start() && publisherScanner.Start() );

        UInt32 round = 0;
        const UInt64 start = DRN::CDRNClock::NowInMicroseconds();
        while ( round < 300 || DRN::CDRNClock::NowInMicroseconds() - start < 1500000 )
        {
            ++round;
            if ( 0 == round % 7 )
            {
                // every few rounds the keys are written through the table, as a tool would
                TRow keys;
                TRow values;
                for ( UInt32 k=0; k<keyCount; ++k )
                {
                    keys.push_back( CORE::CVariant( k ) );
                    values.push_back( CORE::CVariant( round ) );
                }
                CORE::CDataTableWriteResult result;
                ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == publisherTable.Write( *MakeRows( publisherTable, keys, values ), CORE::DATATABLE_WRITE_UPSERT, result ) );
            }
            else
            {
                for ( UInt32 k=0; k<keyCount; ++k )
                    grid->SetValue( CORE::CVariant( k ), CORE::CVariant( round ) );
            }
            PumpFor( 1 );
            if ( round >= 3000 )
                break;
        }
        replicaScanner.Stop();
        publisherScanner.Stop();

        ASSERT_TRUE( 0 == replicaScanner.failures.Load() && 0 == publisherScanner.failures.Load() );
        ASSERT_TRUE( replicaScanner.scans.Load() > 10 && publisherScanner.scans.Load() > 10 );
        ASSERT_TRUE( 0 == replicaScanner.torn.Load() );
        ASSERT_TRUE( replicaScanner.scansWithinOneTick.Load() > 0 );
        ASSERT_TRUE( recorder.events > 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: the drn backend is registered, finds the session by name, lists, opens, creates and drops grid tables" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "dt-db", "mem" );
        REQUIRE_TRUE( group.Start( 1 ) );
        DRN::CDRNDataGrid* alpha = group[ 0 ].CreateGrid( TypedGrid( "alpha", DRN::DRN_AUTHORITY_OWNER, CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_FLOAT64 ) );
        DRN::CDRNDataGridSettings betaSettings;
        betaSettings.name = "beta";
        DRN::CDRNDataGrid* beta = group[ 0 ].CreateGrid( betaSettings );
        REQUIRE_TRUE( GUCEF_NULL != alpha && GUCEF_NULL != beta );
        for ( UInt32 k=1; k<=3; ++k )
            alpha->SetValue( CORE::CVariant( k ), CORE::CVariant( (Float64) ( k * 1.5 ) ) );
        beta->SetValue( "hp", CORE::CVariant( (Int32) 100 ) );

        COMCORE::CDatabaseClientFactory& factory = COMCORE::CComCoreGlobal::Instance()->GetDatabaseClientFactory();
        ASSERT_TRUE( factory.IsConstructible( "drn" ) );
        COMCORE::CIDatabaseClientPtr client = factory.Create( "drn" );
        REQUIRE_TRUE( !client.IsNULL() );
        ASSERT_TRUE( "drn" == client->GetBackendName() );

        COMCORE::CDatabaseClientSettings settings;
        settings.backendName = "drn";
        settings.database = "dt-db";
        ASSERT_FALSE( client->Open( settings ) );
        ASSERT_TRUE( client->GetLastErrorMessage().HasSubstr( "No session named" ) >= 0 );
        ASSERT_FALSE( client->IsOpen() );
        CORE::CString::StringSet names;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_FAILED == client->ListTables( names ) );

        // the application makes the session known by name
        DRN::CDRNSessionRegistry& registry = DRN::CDRNGlobal::Instance()->GetSessionRegistry();
        ASSERT_TRUE( registry.Register( group[ 0 ] ) && !registry.Register( group[ 0 ] ) );
        ASSERT_TRUE( &group[ 0 ] == registry.Find( "dt-db" ) && GUCEF_NULL == registry.Find( "nosuchsession" ) );
        settings.backendOptions.Set( "alpha.keyType", "uint32" );
        settings.backendOptions.Set( "alpha.valueType", "float64" );
        ASSERT_TRUE( client->Open( settings ) );
        ASSERT_TRUE( client->IsOpen() );
        ASSERT_TRUE( 0 != ( GUCEF_DATABASE_CAPABILITY_WRITE & client->GetCapabilities() ) && 0 != ( GUCEF_DATABASE_CAPABILITY_ASYNC & client->GetCapabilities() ) );
        ASSERT_TRUE( 0 == ( GUCEF_DATABASE_CAPABILITY_PARTITIONED_TABLES & client->GetCapabilities() ) );

        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->ListTables( names ) );
        ASSERT_TRUE( names.find( "alpha" ) != names.end() && names.find( "beta" ) != names.end() );
        CORE::CDataTableDefinition definition;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->GetTableDefinition( "alpha", definition ) );
        ASSERT_TRUE( "alpha" == definition.name && CORE::DATATABLE_TYPE_FLOAT64 == definition.schema.GetColumn( 1 ).type );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_NOT_FOUND == client->GetTableDefinition( "nosuch", definition ) );

        CORE::CIDataTablePtr table;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->OpenTable( "alpha", table ) );
        REQUIRE_TRUE( !table.IsNULL() );
        ASSERT_TRUE( 3 == CountRows( *table ) && !table->IsReadOnly() );
        CORE::CIDataTablePtr betaTable;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->OpenTable( "beta", betaTable ) && betaTable->IsReadOnly() );
        CORE::CIDataTablePtr missing;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_NOT_FOUND == client->OpenTable( "nosuch", missing ) && missing.IsNULL() );

        // a table made through the client is a grid of this member, written through the table
        CORE::CDataTableDefinition gamma;
        gamma.name = "gamma";
        gamma.schema.AddColumn( CORE::CDataTableColumnDef( "key", CORE::DATATABLE_TYPE_INT32, false ) );
        gamma.schema.AddColumn( CORE::CDataTableColumnDef( "value", CORE::DATATABLE_TYPE_UTF8 ) );
        gamma.backendOptions.Set( "authority", "OWNER" );
        gamma.backendOptions.Set( "sendRateHz", "0" );
        CORE::CIDataTablePtr gammaTable;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->CreateTable( gamma, gammaTable ) );
        REQUIRE_TRUE( !gammaTable.IsNULL() );
        ASSERT_TRUE( !gammaTable->IsReadOnly() );
        DRN::CDRNDataGrid* gammaGrid = group[ 0 ].FindGrid( "gamma" );
        REQUIRE_TRUE( GUCEF_NULL != gammaGrid );
        ASSERT_TRUE( DRN::DRN_AUTHORITY_OWNER == gammaGrid->GetSettings().authority && CORE::DATATABLE_TYPE_INT32 == gammaGrid->GetSettings().keyType );
        TRow keys;
        TRow values;
        keys.push_back( CORE::CVariant( (Int32) 7 ) );
        values.push_back( CORE::CVariant( "seven" ) );
        CORE::CDataTableWriteResult result;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == gammaTable->Write( *MakeRows( *gammaTable, keys, values ), CORE::DATATABLE_WRITE_UPSERT, result ) );
        CORE::CVariant seven;
        ASSERT_TRUE( gammaGrid->GetValue( CORE::CVariant( (Int32) 7 ), seven ) && "seven" == seven.AsString() );
        CORE::CIDataTablePtr again;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_ALREADY_EXISTS == client->CreateTable( gamma, again ) );
        CORE::CDataTableDefinition invalid;
        invalid.name = "delta";
        invalid.schema.AddColumn( CORE::CDataTableColumnDef( "id", CORE::DATATABLE_TYPE_INT32, false ) );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_INVALID_ARGUMENT == client->CreateTable( invalid, again ) );

        // only the publisher withdraws a grid
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->DropTable( "gamma", true ) );
        ASSERT_TRUE( gammaGrid->IsRemoved() && static_cast< DRN::CDRNDataGridTable* >( gammaTable.GetPointerAlways() )->IsClosed() );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_NOT_FOUND == client->DropTable( "gamma", true ) );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_NOT_FOUND == client->DropTable( "nosuch", true ) );

        // read only
        COMCORE::CIDatabaseClientPtr readOnlyClient = factory.Create( "drn" );
        COMCORE::CDatabaseClientSettings readOnlySettings = settings;
        readOnlySettings.readOnly = true;
        ASSERT_TRUE( readOnlyClient->Open( readOnlySettings ) );
        ASSERT_TRUE( 0 == ( GUCEF_DATABASE_CAPABILITY_WRITE & readOnlyClient->GetCapabilities() ) );
        CORE::CIDataTablePtr readOnlyTable;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == readOnlyClient->OpenTable( "alpha", readOnlyTable ) && readOnlyTable->IsReadOnly() );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_READ_ONLY == readOnlyClient->CreateTable( gamma, again ) );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_READ_ONLY == readOnlyClient->DropTable( "alpha", true ) );

        // a client that holds the session needs no registry
        DRN::CDRNDatabaseClient attached;
        ASSERT_TRUE( attached.Attach( group[ 0 ] ) );
        COMCORE::CDatabaseClientSettings attachedSettings;
        attachedSettings.backendName = "drn";
        ASSERT_TRUE( attached.Open( attachedSettings ) && attached.IsOpen() );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == attached.ListTables( names ) && names.find( "alpha" ) != names.end() );
        attached.Close();
        ASSERT_FALSE( attached.IsOpen() );

        client->Close();
        ASSERT_FALSE( client->IsOpen() );
        ASSERT_TRUE( CORE::DATATABLE_STATUS_FAILED == client->OpenTable( "alpha", missing ) );

        // the registry forgets a session that is gone, and a client with a gone session is not open
        {
            CGroup shortLived( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "dt-db-short", "mem" );
            REQUIRE_TRUE( shortLived.Start( 1 ) );
            ASSERT_TRUE( registry.Register( shortLived[ 0 ] ) && &shortLived[ 0 ] == registry.Find( "dt-db-short" ) );
            COMCORE::CDatabaseClientSettings shortSettings;
            shortSettings.backendName = "drn";
            shortSettings.database = "dt-db-short";
            ASSERT_TRUE( readOnlyClient->Open( shortSettings ) && readOnlyClient->IsOpen() );
            shortLived.Stop();
            ASSERT_TRUE( GUCEF_NULL == registry.Find( "dt-db-short" ) );
            ASSERT_FALSE( readOnlyClient->IsOpen() );
        }
        registry.Unregister( group[ 0 ] );
        ASSERT_TRUE( GUCEF_NULL == registry.Find( "dt-db" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: a grid that is announced and not subscribed is subscribed on open, and the table is there when its snapshot is" )
    try
    {
        // on the pulse of this thread: OpenTableAsync, the pool waits for the snapshot while this thread drives the nodes
        {
            CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "dt-open", "mem" );
            REQUIRE_TRUE( group.Start( 2 ) );
            DRN::CDRNDataGrid* remote = group[ 1 ].CreateGrid( TypedGrid( "remote", DRN::DRN_AUTHORITY_OWNER, CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_FLOAT64 ) );
            REQUIRE_TRUE( GUCEF_NULL != remote );
            for ( UInt32 k=1; k<=3; ++k )
                remote->SetValue( CORE::CVariant( k ), CORE::CVariant( (Float64) k ) );

            DRN::CDRNDatabaseClient client;
            ASSERT_TRUE( client.Attach( group[ 0 ] ) );
            COMCORE::CDatabaseClientSettings settings;
            settings.backendName = "drn";
            settings.database = "dt-open";
            settings.backendOptions.Set( "remote.keyType", "uint32" );
            settings.backendOptions.Set( "remote.valueType", "float64" );
            REQUIRE_TRUE( client.Open( settings ) );

            // announced to the member, not subscribed by it
            CORE::CString::StringSet names;
            bool announced = false;
            const UInt64 start = DRN::CDRNClock::NowInMicroseconds();
            while ( !announced && DRN::CDRNClock::NowInMicroseconds() - start < 10000000 )
            {
                PumpFor( 5 );
                announced = CORE::DATATABLE_STATUS_OK == client.ListTables( names ) && names.find( "remote" ) != names.end();
            }
            REQUIRE_TRUE( announced );
            DRN::CDRNDataGrid* replicaGrid = group[ 0 ].FindGrid( "remote" );
            REQUIRE_TRUE( GUCEF_NULL != replicaGrid && !replicaGrid->IsSnapshotComplete() );

            CORE::CFutureResult opened = client.OpenTableAsync( "remote" );
            CORE::CTaskPtr task;
            ASSERT_TRUE( CORE::TTaskStatus::TASKSTATUS_FINISHED == AwaitPumped( opened, task ) );
            CORE::CIDataTablePtr table;
            REQUIRE_TRUE( COMCORE::CDatabaseTableResult::TryGetFromFuture( opened, table ) );
            ASSERT_TRUE( replicaGrid->IsSnapshotComplete() );
            ASSERT_TRUE( 3 == CountRows( *table ) );

            // the sync call on the thread that drives the node cannot see the snapshot arrive: it says so
            CORE::CIDataTablePtr stuck;
            DRN::CDRNDataGrid* other = group[ 1 ].CreateGrid( TypedGrid( "other", DRN::DRN_AUTHORITY_OWNER, CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_UINT32 ) );
            REQUIRE_TRUE( GUCEF_NULL != other );
            other->SetValue( CORE::CVariant( (UInt32) 1 ), CORE::CVariant( (UInt32) 1 ) );
            announced = false;
            const UInt64 startOther = DRN::CDRNClock::NowInMicroseconds();
            while ( !announced && DRN::CDRNClock::NowInMicroseconds() - startOther < 10000000 )
            {
                PumpFor( 5 );
                announced = CORE::DATATABLE_STATUS_OK == client.ListTables( names ) && names.find( "other" ) != names.end();
            }
            REQUIRE_TRUE( announced );
            COMCORE::CDatabaseClientSettings quick = settings;
            quick.backendOptions.Set( "openTimeoutMs", "200" );
            REQUIRE_TRUE( client.Open( quick ) );
            ASSERT_TRUE( CORE::DATATABLE_STATUS_FAILED == client.OpenTable( "other", stuck ) );
            ASSERT_TRUE( client.GetLastErrorMessage().HasSubstr( "OpenTableAsync" ) >= 0 );
        }

        // nodes on threads of their own: the sync call waits on the calling thread, the node thread brings the snapshot
        {
            CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "dt-open-threads", "mem" );
            group.nodeSettings.ownThread = true;
            REQUIRE_TRUE( group.Start( 2 ) );
            DRN::CDRNDataGrid* remote = group[ 1 ].CreateGrid( TypedGrid( "remote", DRN::DRN_AUTHORITY_OWNER, CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_FLOAT64 ) );
            REQUIRE_TRUE( GUCEF_NULL != remote );
            for ( UInt32 k=1; k<=3; ++k )
                remote->SetValue( CORE::CVariant( k ), CORE::CVariant( (Float64) k ) );

            DRN::CDRNDatabaseClient client;
            ASSERT_TRUE( client.Attach( group[ 0 ] ) );
            COMCORE::CDatabaseClientSettings settings;
            settings.backendName = "drn";
            settings.database = "dt-open-threads";
            settings.backendOptions.Set( "remote.keyType", "uint32" );
            settings.backendOptions.Set( "remote.valueType", "float64" );
            REQUIRE_TRUE( client.Open( settings ) );

            CORE::CIDataTablePtr table;
            CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_NOT_FOUND;
            const UInt64 start = DRN::CDRNClock::NowInMicroseconds();
            while ( CORE::DATATABLE_STATUS_NOT_FOUND == status && DRN::CDRNClock::NowInMicroseconds() - start < 10000000 )
            {
                status = client.OpenTable( "remote", table );
                if ( CORE::DATATABLE_STATUS_NOT_FOUND == status )
                    MT::ThreadDelay( 5 );
            }
            ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == status );
            REQUIRE_TRUE( !table.IsNULL() );
            ASSERT_TRUE( 3 == CountRows( *table ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: a generic consumer exports a grid table as CSV exactly like a table of another backend" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "dt-csv", "mem" );
        REQUIRE_TRUE( group.Start( 2 ) );
        DRN::CDRNDataGrid* grid = group[ 0 ].CreateGrid( TypedGrid( "export", DRN::DRN_AUTHORITY_OWNER, CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_FLOAT64 ) );
        REQUIRE_TRUE( GUCEF_NULL != grid );
        for ( UInt32 k=1; k<=3; ++k )
            grid->SetValue( CORE::CVariant( k ), CORE::CVariant( (Float64) ( k * 1.5 ) ) );
        DRN::CDRNDataGrid* replicaGrid = Subscribed( group[ 1 ], "export", 1, CORE::CVariant( (UInt32) 3 ), CORE::CVariant( (Float64) 4.5 ) );
        REQUIRE_TRUE( GUCEF_NULL != replicaGrid );

        // the same rows in a table that is not a grid
        CORE::CDataTableDefinition reference;
        reference.name = "reference";
        reference.schema.AddColumn( CORE::CDataTableColumnDef( "key", CORE::DATATABLE_TYPE_UINT32, false ) );
        reference.schema.AddColumn( CORE::CDataTableColumnDef( "value", CORE::DATATABLE_TYPE_FLOAT64 ) );
        reference.schema.AddColumn( CORE::CDataTableColumnDef( "revision", CORE::DATATABLE_TYPE_UINT32 ) );
        CORE::CMemoryDataTable memoryTable( reference );
        REQUIRE_TRUE( memoryTable.IsValid() );
        CORE::CDataTableBatchBuilder builder( memoryTable.GetSchema(), CORE::CCoreGlobal::Instance()->GetDataTableBufferPool() );
        for ( UInt32 k=1; k<=3; ++k )
        {
            builder.GetColumn( 0 ).Append( CORE::CVariant( k ) );
            builder.GetColumn( 1 ).Append( CORE::CVariant( (Float64) ( k * 1.5 ) ) );
            builder.GetColumn( 2 ).Append( CORE::CVariant( (UInt32) 1 ) );
        }
        CORE::CDataTableBatchPtr referenceRows;
        CORE::CString error;
        REQUIRE_TRUE( CORE::DATATABLE_STATUS_OK == builder.Build( referenceRows, error ) );
        CORE::CDataTableWriteResult result;
        REQUIRE_TRUE( CORE::DATATABLE_STATUS_OK == memoryTable.Write( *referenceRows, CORE::DATATABLE_WRITE_INSERT, result ) );

        DRN::CDRNDataGridTable publisher( group[ 0 ], *grid );
        DRN::CDRNDataGridTable replica( group[ 1 ], *replicaGrid, TypedTable( CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_FLOAT64 ) );
        CORE::CDataTableQuery query;
        query.Select( "key" ).Select( "value" ).Select( "revision" );
        CORE::CString expected;
        CORE::CString fromPublisher;
        CORE::CString fromReplica;
        REQUIRE_TRUE( ExportCsv( memoryTable, query, expected ) );
        REQUIRE_TRUE( ExportCsv( publisher, query, fromPublisher ) );
        REQUIRE_TRUE( ExportCsv( replica, query, fromReplica ) );
        ASSERT_TRUE( expected.HasSubstr( "key,value,revision" ) == 0 && expected.HasSubstr( "3,4.5,1" ) >= 0 );
        ASSERT_TRUE( expected == fromPublisher );
        ASSERT_TRUE( expected == fromReplica );
        if ( expected != fromPublisher )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CSV of the reference table:\n" << expected << "CSV of the grid table:\n" << fromPublisher );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Tables: Removed keys are hidden unless the table includes them, and a table can serve scans from a copy of the grid that is a few milliseconds old" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "dt-removed-keys", "mem" );
        REQUIRE_TRUE( group.Start( 2 ) );
        DRN::CDRNDataGrid* grid = group[ 0 ].CreateGrid( TypedGrid( "items", DRN::DRN_AUTHORITY_OWNER, CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_UINT32 ) );
        REQUIRE_TRUE( GUCEF_NULL != grid );
        for ( UInt32 k=1; k<=3; ++k )
            grid->SetValue( CORE::CVariant( k ), CORE::CVariant( k * 10 ) );
        grid->RemoveKey( CORE::CVariant( (UInt32) 2 ) );
        DRN::CDRNDataGrid* replicaGrid = Subscribed( group[ 1 ], "items", 1, CORE::CVariant( (UInt32) 3 ), CORE::CVariant( (UInt32) 30 ) );
        REQUIRE_TRUE( GUCEF_NULL != replicaGrid );

        DRN::CDRNDataGridTableSettings including = TypedTable( CORE::DATATABLE_TYPE_UINT32, CORE::DATATABLE_TYPE_UINT32 );
        including.includeRemoved = true;
        DRN::CDRNDataGridTable hiding( group[ 0 ], *grid );
        DRN::CDRNDataGridTable showing( group[ 0 ], *grid, including );
        DRN::CDRNDataGridTable replicaShowing( group[ 1 ], *replicaGrid, including );
        ASSERT_TRUE( 2 == CountRows( hiding ) );
        CORE::CIDataTable* showingTables[] = { &showing, &replicaShowing };
        for ( UInt32 t=0; t<2; ++t )
        {
            TRowVector rows;
            ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == ReadRows( *showingTables[ t ], CORE::CDataTableQuery(), rows ) && 3 == rows.size() );
            UInt32 removedRows = 0;
            for ( UInt32 i=0; i<rows.size(); ++i )
            {
                ASSERT_TRUE( 6 == rows[ i ].size() );
                if ( rows[ i ][ 5 ].AsBool() )
                {
                    ++removedRows;
                    ASSERT_TRUE( 2 == rows[ i ][ 0 ].AsUInt32() && rows[ i ][ 1 ].IsNULL() && 2 == rows[ i ][ 2 ].AsUInt32() );
                }
            }
            ASSERT_TRUE( 1 == removedRows );
            ASSERT_TRUE( 1 == CountRows( *showingTables[ t ], CORE::CDataTableQuery().WhereEquals( "removed", CORE::CVariant( true ) ) ) );
        }

        // a copy of the grid that may be a few milliseconds old: the next scan does not take another one until it is dropped
        DRN::CDRNDataGridTableSettings cached;
        cached.snapshotCacheMs = 60000;
        DRN::CDRNDataGridTable cachedTable( group[ 0 ], *grid, cached );
        ASSERT_TRUE( 2 == CountRows( cachedTable ) && 1 == cachedTable.GetSnapshotCount() );
        grid->SetValue( CORE::CVariant( (UInt32) 9 ), CORE::CVariant( (UInt32) 90 ) );
        ASSERT_TRUE( 2 == CountRows( cachedTable ) && 1 == cachedTable.GetSnapshotCount() );
        ASSERT_TRUE( 3 == CountRows( hiding ) );
        cachedTable.Refresh();
        ASSERT_TRUE( 3 == CountRows( cachedTable ) && 2 == cachedTable.GetSnapshotCount() );
        // a lookup of a key is not served from the copy: it is exact
        grid->SetValue( CORE::CVariant( (UInt32) 10 ), CORE::CVariant( (UInt32) 100 ) );
        ASSERT_TRUE( 1 == CountRows( cachedTable, CORE::CDataTableQuery().WhereEquals( "key", CORE::CVariant( (UInt32) 10 ) ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
