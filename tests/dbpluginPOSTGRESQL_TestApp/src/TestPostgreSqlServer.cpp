/*
 *  dbpluginPOSTGRESQL_TestApp: tests of the postgresql database backend
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

#include <string.h>
#include <limits>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CMEMORYDATATABLE_H
#include "gucefCORE_CMemoryDataTable.h"
#define GUCEF_CORE_CMEMORYDATATABLE_H
#endif /* GUCEF_CORE_CMEMORYDATATABLE_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CDATABASETABLERESULT_H
#include "gucefCOMCORE_CDatabaseTableResult.h"
#define GUCEF_COMCORE_CDATABASETABLERESULT_H
#endif /* GUCEF_COMCORE_CDATABASETABLERESULT_H ? */

#ifndef GUCEF_COMCORE_CDATABASETRANSACTION_H
#include "gucefCOMCORE_CDatabaseTransaction.h"
#define GUCEF_COMCORE_CDATABASETRANSACTION_H
#endif /* GUCEF_COMCORE_CDATABASETRANSACTION_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H
#include "dbpluginPOSTGRESQL_CPostgreSqlClient.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H
#include "dbpluginPOSTGRESQL_CPostgreSqlSql.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDataTableSupport.h"
#include "TestDataTableScanSupport.h"
#include "TestPostgreSqlSupport.h"
#include "TestPostgreSqlServer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace GUCEF::DBPLUGIN::POSTGRESQL;
using namespace POSTGRESQLTEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef GUCEF::vector< CVariant > TRow;
typedef GUCEF::vector< TRow > TRowVector;

const char* const SkipReason = "GUCEF_TEST_POSTGRESQL is not set: no PostgreSQL test server";

/*-------------------------------------------------------------------------*/

/** A full batch of the rows, one value per schema column */
CDataTableBatchPtr
MakeRows( const CDataTableSchemaPtr& schema, const TRowVector& rows )
{
    CDataTableBatchBuilder builder( schema, CCoreGlobal::Instance()->GetDataTableBufferPool() );
    for ( UInt32 r=0; r<(UInt32) rows.size(); ++r )
    {
        if ( !builder.AppendRow( &rows[ r ][ 0 ], (UInt32) rows[ r ].size() ) )
            return CDataTableBatchPtr();
    }
    CDataTableBatchPtr batch;
    CString error;
    if ( DATATABLE_STATUS_OK != builder.Build( batch, error ) )
        return CDataTableBatchPtr();
    return batch;
}

/*-------------------------------------------------------------------------*/

/** A row of a key and a value */
TRow
KeyValue( Int32 key, const char* value )
{
    TRow row;
    row.push_back( CVariant( (Int64) key ) );
    row.push_back( GUCEF_NULL == value ? CVariant() : CVariant( CUtf8String( value ) ) );
    return row;
}

/*-------------------------------------------------------------------------*/

CDataTableDefinition
MakeKeyValueDefinition( const CString& name, bool withKey )
{
    CDataTableDefinition definition;
    definition.name = name;
    definition.schema.AddColumn( CDataTableColumnDef( "k", DATATABLE_TYPE_INT32, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "v", DATATABLE_TYPE_UTF8, true ) );
    if ( withKey )
    {
        CString::StringVector key;
        key.push_back( "k" );
        definition.schema.SetPrimaryKey( key );
    }
    return definition;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
WriteRows( CIDataTable& table, const TRowVector& rows, TDataTableWriteMode mode, CDataTableWriteResult* result = GUCEF_NULL )
{
    CDataTableBatchPtr batch = MakeRows( table.GetSchema(), rows );
    if ( batch.IsNULL() )
        return DATATABLE_STATUS_TYPE_MISMATCH;
    CDataTableWriteResult local;
    return table.Write( *batch, mode, GUCEF_NULL != result ? *result : local );
}

/*-------------------------------------------------------------------------*/

UInt32
CountRows( CIDataTable& table, const CDataTableQuery& query = CDataTableQuery() )
{
    CString::StringVector rows;
    if ( DATATABLE_STATUS_OK != ScanRows( table, query, rows ) )
        return GUCEF_UINT32MAX;
    return (UInt32) rows.size();
}

/*-------------------------------------------------------------------------*/

/** Runs a statement on the connection of a client; the rows in the result */
TDataTableStatus
RunSql( COMCORE::CIDatabaseClientPtr& client, const CString& sql, const CString::StringVector& texts, CPostgreSqlResult& result )
{
    CPostgreSqlConnectionPtr connection = static_cast< CPostgreSqlClient* >( client.GetPointerAlways() )->GetConnection();
    if ( connection.IsNULL() )
        return DATATABLE_STATUS_FAILED;
    CPostgreSqlParameters parameters;
    for ( UInt32 i=0; i<(UInt32) texts.size(); ++i )
        parameters.AddText( texts[ i ] );
    CString error;
    MT::CScopeMutex lock( connection->GetLock(), GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    const TDataTableStatus status = connection->Execute( sql, parameters, result, error );
    if ( DATATABLE_STATUS_OK != status )
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PostgreSqlServer: statement failed: " << error );
    return status;
}

/*-------------------------------------------------------------------------*/

/** Whether a relation of the name exists in the schema */
bool
RelationExists( COMCORE::CIDatabaseClientPtr& client, const CString& schema, const CString& name )
{
    CString::StringVector texts;
    texts.push_back( CPostgreSqlSql::QualifiedName( schema, name ) );
    CPostgreSqlResult result;
    return DATATABLE_STATUS_OK == RunSql( client, "SELECT to_regclass($1::text) IS NOT NULL", texts, result ) &&
           1 == result.GetRowCount() && 't' == result.GetValue( 0, 0 )[ 0 ];
}

/*-------------------------------------------------------------------------*/

/** A random predicate on a random column of the schema, with literals of the column */
CDataTablePredicate
RandomPredicate( CORETEST::CTestRandom& random, const CDataTableSchema& schema )
{
    const CDataTableColumnDef& column = schema.GetColumn( random.Below( schema.GetColumnCount() ) );
    switch ( random.Below( 10 ) )
    {
        case 0: return CDataTablePredicate( column.name, DATATABLE_PREDICATE_EQUALS, CORETEST::RandomCellValue( random, column, 0 ) );
        case 1: return CDataTablePredicate( column.name, DATATABLE_PREDICATE_NOT_EQUALS, CORETEST::RandomCellValue( random, column, 0 ) );
        case 2: return CDataTablePredicate( column.name, DATATABLE_PREDICATE_LESS, CORETEST::RandomCellValue( random, column, 0 ) );
        case 3: return CDataTablePredicate( column.name, DATATABLE_PREDICATE_LESS_OR_EQUAL, CORETEST::RandomCellValue( random, column, 0 ) );
        case 4: return CDataTablePredicate( column.name, DATATABLE_PREDICATE_GREATER, CORETEST::RandomCellValue( random, column, 0 ) );
        case 5: return CDataTablePredicate( column.name, DATATABLE_PREDICATE_GREATER_OR_EQUAL, CORETEST::RandomCellValue( random, column, 0 ) );
        case 6:
        {
            const CVariant low = random.Chance( 20 ) ? CVariant::Empty : CORETEST::RandomCellValue( random, column, 0 );
            const CVariant high = random.Chance( 20 ) ? CVariant::Empty : CORETEST::RandomCellValue( random, column, 0 );
            return CDataTablePredicate( column.name, low, random.Chance( 50 ), high, random.Chance( 50 ) );
        }
        case 7:
        {
            CVariant::VariantVector values;
            const UInt32 count = random.Below( 4 );
            for ( UInt32 i=0; i<count; ++i )
                values.push_back( CORETEST::RandomCellValue( random, column, 0 ) );
            return CDataTablePredicate( column.name, values );
        }
        case 8: return CDataTablePredicate( column.name, DATATABLE_PREDICATE_IS_NULL );
        default: return CDataTablePredicate( column.name, DATATABLE_PREDICATE_IS_NOT_NULL );
    }
}

/*-------------------------------------------------------------------------*/

/** Rows of every type with the extreme values the columns hold */
TRowVector
MakeExtremeRows( const CDataTableSchema& schema )
{
    TRowVector rows;
    for ( UInt32 variant=0; variant<3; ++variant )
    {
        TRow row;
        for ( UInt32 c=0; c<schema.GetColumnCount(); ++c )
        {
            const CDataTableColumnDef& column = schema.GetColumn( c );
            const bool low = 0 == variant;
            if ( "id" == column.name )
            {
                row.push_back( CVariant( (Int64) ( 1000000 + variant ) ) );
                continue;
            }
            if ( 2 == variant )
            {
                row.push_back( CVariant() );
                continue;
            }
            switch ( column.type )
            {
                case DATATABLE_TYPE_BOOL: row.push_back( CVariant( low ) ); break;
                case DATATABLE_TYPE_INT8: row.push_back( CVariant( (Int64) ( low ? -128 : 127 ) ) ); break;
                case DATATABLE_TYPE_INT16: row.push_back( CVariant( (Int64) ( low ? -32768 : 32767 ) ) ); break;
                case DATATABLE_TYPE_INT32: row.push_back( CVariant( (Int64) ( low ? GUCEF_INT32MIN : GUCEF_INT32MAX ) ) ); break;
                case DATATABLE_TYPE_INT64: row.push_back( CVariant( (Int64) ( low ? GUCEF_INT64MIN : GUCEF_INT64MAX ) ) ); break;
                case DATATABLE_TYPE_UINT8: row.push_back( CVariant( (UInt64) ( low ? 0 : 255 ) ) ); break;
                case DATATABLE_TYPE_UINT16: row.push_back( CVariant( (UInt64) ( low ? 0 : 65535 ) ) ); break;
                case DATATABLE_TYPE_UINT32: row.push_back( CVariant( (UInt64) ( low ? 0 : 4294967295U ) ) ); break;
                case DATATABLE_TYPE_UINT64: row.push_back( CVariant( (UInt64) ( low ? 0 : GUCEF_UINT64MAX ) ) ); break;
                case DATATABLE_TYPE_FLOAT32: row.push_back( CVariant( low ? std::numeric_limits< Float32 >::denorm_min() : std::numeric_limits< Float32 >::infinity() ) ); break;
                case DATATABLE_TYPE_FLOAT64: row.push_back( CVariant( low ? -std::numeric_limits< Float64 >::infinity() : std::numeric_limits< Float64 >::max() ) ); break;
                case DATATABLE_TYPE_DECIMAL:
                {
                    CDecimal value;
                    CDecimal::FromUnscaled( low ? -999999999999999999LL : 999999999999999999LL, column.decimalScale, value );
                    row.push_back( CDataTableValues::Decimal( value ) );
                    break;
                }
                case DATATABLE_TYPE_DATE: row.push_back( CDataTableValues::Date( low ? -719162 : 2932896 ) ); break;
                case DATATABLE_TYPE_TIMESTAMP:
                {
                    Int64 ticks = 0;
                    if ( DATATABLE_TIMEUNIT_NANOSECONDS == column.timeUnit )
                        ticks = low ? GUCEF_INT64MIN : GUCEF_INT64MAX;
                    else
                    if ( DATATABLE_TIMEUNIT_MILLISECONDS == column.timeUnit )
                        ticks = low ? -62135596800000LL - 1 : 253402300799999LL;
                    else
                        ticks = low ? -62135596800000000LL + 1 : 253402300799999999LL;
                    row.push_back( CDataTableValues::Timestamp( ticks, column.timeUnit ) );
                    break;
                }
                case DATATABLE_TYPE_UTF8: row.push_back( CVariant( CUtf8String( low ? "" : "'; DROP TABLE x; -- \"quoted\" \xE2\x82\xAC \xF0\x9F\x98\x80" ) ) ); break;
                case DATATABLE_TYPE_BINARY:
                {
                    static const UInt8 bytes[] = { 0x00, 0xFF, 0x00, 0x27, 0x5C, 0x00 };
                    CVariant value;
                    value.Set( bytes, low ? 0 : (UInt32) sizeof( bytes ), GUCEF_DATATYPE_BINARY_BLOB, false );
                    row.push_back( value );
                    break;
                }
                default: row.push_back( CVariant() ); break;
            }
        }
        rows.push_back( row );
    }
    return rows;
}

/*-------------------------------------------------------------------------*/

/** The server, the schema of this run and the client of the tests, set up once */
struct SServerFixture
{
    bool available;
    bool opened;
    CString connectionString;
    CString schema;
    COMCORE::CIDatabaseClientPtr client;

    SServerFixture( void ) : available( false ), opened( false ), connectionString(), schema(), client() {}
};

/*-------------------------------------------------------------------------*/

COMCORE::CIDatabaseClientPtr
OpenClient( SServerFixture& fixture, bool readOnly )
{
    COMCORE::CIDatabaseClientPtr client = COMCORE::CComCoreGlobal::Instance()->GetDatabaseClientFactory().Create( "postgresql" );
    if ( client.IsNULL() || !client->Open( MakeServerSettings( fixture.connectionString, fixture.schema, readOnly ) ) )
    {
        if ( !client.IsNULL() )
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PostgreSqlServer: a client does not open: " << client->GetLastErrorMessage() );
        return COMCORE::CIDatabaseClientPtr();
    }
    return client;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPostgreSqlServerTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "PostgreSqlServer" );

    SServerFixture fixture;
    fixture.available = GetTestServerConnectionString( fixture.connectionString );
    if ( !fixture.available )
    {
        GUCEF_LOG( LOGLEVEL_NORMAL, "PostgreSqlServer: the tests against a server are skipped: the environment variable GUCEF_TEST_POSTGRESQL is not set" );
    }
    else
    {
        fixture.schema = MakeTestSchemaName();
        fixture.client = OpenClient( fixture, false );
        fixture.opened = !fixture.client.IsNULL();
        GUCEF_LOG( LOGLEVEL_NORMAL, "PostgreSqlServer: testing in schema \"" << fixture.schema << "\"" );
    }

    GUCEF_TESTFW_TESTCASE( "Server: a client opens, makes its schema and catalogue and lists no tables" )
        if ( !fixture.available )
            GUCEF_TESTFW_SKIP( SkipReason );
        try
        {
            ASSERT_TRUE( fixture.opened );
            COMCORE::CIDatabaseClientPtr& client = fixture.client;
            ASSERT_TRUE( client->IsOpen() );
            const UInt32 capabilities = client->GetCapabilities();
            ASSERT_TRUE( 0 != ( capabilities & GUCEF_DATABASE_CAPABILITY_WRITE ) && 0 != ( capabilities & GUCEF_DATABASE_CAPABILITY_TRANSACTIONS ) );
            CString::StringSet names;
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->ListTables( names ) && names.empty() );
            ASSERT_TRUE( RelationExists( client, fixture.schema, CPostgreSqlSql::CatalogueTableName ) );
            CDataTableDefinition definition;
            ASSERT_TRUE( DATATABLE_STATUS_NOT_FOUND == client->GetTableDefinition( "nothing", definition ) );
            CIDataTablePtr table;
            ASSERT_TRUE( DATATABLE_STATUS_NOT_FOUND == client->OpenTable( "nothing", table ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Server: tables are made with their index hints as indexes, described, opened by another client and dropped with or without their data" )
        if ( !fixture.available )
            GUCEF_TESTFW_SKIP( SkipReason );
        try
        {
            ASSERT_TRUE( fixture.opened );
            COMCORE::CIDatabaseClientPtr& client = fixture.client;
            CDataTableDefinition definition = MakeAllTypesDefinition( "all_types" );
            CString::StringVector byText;
            byText.push_back( "s" );
            CString::StringVector byNumbers;
            byNumbers.push_back( "i32" );
            byNumbers.push_back( "id" );
            definition.indexes.push_back( CDataTableIndexDef( "by_s", byText ) );
            definition.indexes.push_back( CDataTableIndexDef( "uniq_i32_id", byNumbers, true ) );
            CIDataTablePtr table;
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->CreateTable( definition, table ) );
            ASSERT_TRUE( !table.IsNULL() && table->GetDefinition() == definition );

            CDataTableDefinition described;
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->GetTableDefinition( "all_types", described ) && described == definition );
            CString::StringSet names;
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->ListTables( names ) && 1 == names.size() && 1 == names.count( "all_types" ) );

            CString::StringVector texts;
            texts.push_back( fixture.schema );
            texts.push_back( "all_types" );
            CPostgreSqlResult indexes;
            ASSERT_TRUE( DATATABLE_STATUS_OK == RunSql( client, "SELECT indexname, indexdef FROM pg_indexes WHERE schemaname = $1::text AND tablename = $2::text ORDER BY indexname", texts, indexes ) );
            ASSERT_TRUE( 3 == indexes.GetRowCount() );
            ASSERT_TRUE( 0 == strcmp( indexes.GetValue( 0, 0 ), "all_types_by_s" ) && GUCEF_NULL == strstr( indexes.GetValue( 0, 1 ), "UNIQUE" ) );
            ASSERT_TRUE( 0 == strcmp( indexes.GetValue( 2, 0 ), "all_types_uniq_i32_id" ) && GUCEF_NULL != strstr( indexes.GetValue( 2, 1 ), "UNIQUE" ) );

            // another client reads the definition
            COMCORE::CIDatabaseClientPtr other = OpenClient( fixture, true );
            ASSERT_FALSE( other.IsNULL() );
            CIDataTablePtr opened;
            ASSERT_TRUE( DATATABLE_STATUS_OK == other->OpenTable( "all_types", opened ) && opened->GetDefinition() == definition && opened->IsReadOnly() );
            opened.Unlink();
            other->Close();

            // what cannot be made
            CIDataTablePtr refused;
            ASSERT_TRUE( DATATABLE_STATUS_ALREADY_EXISTS == client->CreateTable( definition, refused ) );
            CDataTableDefinition partitioned = MakeKeyValueDefinition( "partitioned", true );
            partitioned.partitioning.AddField( "k", "k", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
            partitioned.partitioning.SetPathTemplate( "{k}.parquet" );
            ASSERT_TRUE( DATATABLE_STATUS_UNSUPPORTED == client->CreateTable( partitioned, refused ) );
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == client->CreateTable( MakeKeyValueDefinition( CPostgreSqlSql::CatalogueTableName, true ), refused ) );
            CDataTableDefinition badHint = MakeKeyValueDefinition( "bad_hint", true );
            CString::StringVector missing;
            missing.push_back( "missing" );
            badHint.indexes.push_back( CDataTableIndexDef( "by_missing", missing ) );
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == client->CreateTable( badHint, refused ) );
            ASSERT_FALSE( RelationExists( client, fixture.schema, "bad_hint" ) );

            // dropped without its data the table stays, a table of the same definition takes it over
            CIDataTablePtr kv;
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->CreateTable( MakeKeyValueDefinition( "kept", true ), kv ) );
            TRowVector rows;
            rows.push_back( KeyValue( 1, "a" ) );
            rows.push_back( KeyValue( 2, "b" ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *kv, rows, DATATABLE_WRITE_INSERT ) );
            kv.Unlink();
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->DropTable( "kept", false ) );
            ASSERT_TRUE( DATATABLE_STATUS_NOT_FOUND == client->OpenTable( "kept", kv ) );
            ASSERT_TRUE( RelationExists( client, fixture.schema, "kept" ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->CreateTable( MakeKeyValueDefinition( "kept", true ), kv ) );
            ASSERT_TRUE( 2 == CountRows( *kv ) );
            kv.Unlink();
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->DropTable( "kept", true ) );
            ASSERT_FALSE( RelationExists( client, fixture.schema, "kept" ) );
            ASSERT_TRUE( DATATABLE_STATUS_NOT_FOUND == client->DropTable( "kept", true ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Server: values of every type, extremes, NaN, infinities, -0, nulls, text and bytes come back as they were written" )
        if ( !fixture.available )
            GUCEF_TESTFW_SKIP( SkipReason );
        try
        {
            ASSERT_TRUE( fixture.opened );
            COMCORE::CIDatabaseClientPtr& client = fixture.client;
            CIDataTablePtr table;
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->OpenTable( "all_types", table ) );
            const CDataTableDefinition definition = table->GetDefinition();
            CMemoryDataTable reference( definition );

            CORETEST::CTestRandom random( 20261118 );
            CDataTableBatchPtr randomRows = CORETEST::MakeRandomBatch( table->GetSchema(), CCoreGlobal::Instance()->GetDataTableBufferPool(), random, 300, 1, 15 );
            ASSERT_FALSE( randomRows.IsNULL() );
            CDataTableWriteResult result;
            CDataTableWriteResult memoryResult;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->Write( *randomRows, DATATABLE_WRITE_UPSERT, result ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == reference.Write( *randomRows, DATATABLE_WRITE_UPSERT, memoryResult ) );
            CString::StringVector pgRows;

            const TRowVector extremes = MakeExtremeRows( *table->GetSchema() );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, extremes, DATATABLE_WRITE_INSERT ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( reference, extremes, DATATABLE_WRITE_INSERT ) );

            CString::StringVector memoryRows;
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, CDataTableQuery(), pgRows ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( reference, CDataTableQuery(), memoryRows ) );
            ASSERT_TRUE( !pgRows.empty() && pgRows == memoryRows );
            if ( pgRows != memoryRows )
            {
                for ( UInt32 i=0; i<(UInt32) pgRows.size() && i<(UInt32) memoryRows.size(); ++i )
                {
                    if ( pgRows[ i ] != memoryRows[ i ] )
                    {
                        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PostgreSqlServer: first difference, server [" << pgRows[ i ] << "] memory [" << memoryRows[ i ] << "]" );
                        break;
                    }
                }
            }

            // the extreme rows by key, through an async scan as well
            CDataTableQuery byKey;
            byKey.WhereRange( "id", CVariant( (Int64) 1000000 ), true, CVariant( (Int64) 1000002 ), true );
            CString::StringVector asyncRows;
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, byKey, asyncRows, true ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( reference, byKey, memoryRows ) );
            ASSERT_TRUE( 3 == asyncRows.size() && asyncRows == memoryRows );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Server: after the same random writes and deletes, random queries give the rows of a CMemoryDataTable" )
        if ( !fixture.available )
            GUCEF_TESTFW_SKIP( SkipReason );
        try
        {
            ASSERT_TRUE( fixture.opened );
            COMCORE::CIDatabaseClientPtr& client = fixture.client;
            const CDataTableDefinition definition = MakeAllTypesDefinition( "random_rows" );
            CIDataTablePtr table;
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->CreateTable( definition, table ) );
            CMemoryDataTable reference( definition );
            const CDataTableSchemaPtr schema = table->GetSchema();

            CORETEST::CTestRandom random( 1118 );
            UInt32 comparisons = 0;
            UInt32 mismatches = 0;
            UInt32 rowsCompared = 0;
            Int64 nextId = 1;
            for ( UInt32 round=0; round<12; ++round )
            {
                // new keys by insert, or new and existing keys (some repeated) by upsert
                const bool upsert = round > 0 && random.Chance( 50 );
                const UInt32 count = 1 + random.Below( 40 );
                CDataTableBatchPtr rows = CORETEST::MakeRandomBatch( schema, CCoreGlobal::Instance()->GetDataTableBufferPool(), random, count, upsert ? nextId - 20 : nextId, 15 );
                nextId += count;
                CDataTableWriteResult pgResult;
                CDataTableWriteResult memoryResult;
                const TDataTableStatus pgStatus = table->Write( *rows, upsert ? DATATABLE_WRITE_UPSERT : DATATABLE_WRITE_INSERT, pgResult );
                const TDataTableStatus memoryStatus = reference.Write( *rows, upsert ? DATATABLE_WRITE_UPSERT : DATATABLE_WRITE_INSERT, memoryResult );
                ASSERT_TRUE( DATATABLE_STATUS_OK == pgStatus && pgStatus == memoryStatus );
                if ( upsert )
                    ASSERT_TRUE( pgResult.partitions[ 0 ].rowsReplaced == memoryResult.partitions[ 0 ].rowsReplaced );

                if ( random.Chance( 30 ) )
                {
                    CDataTableQuery remove;
                    remove.Where( RandomPredicate( random, *schema ) );
                    CDataTableWriteResult pgDeleted;
                    CDataTableWriteResult memoryDeleted;
                    ASSERT_TRUE( DATATABLE_STATUS_OK == table->DeleteRows( remove, pgDeleted ) );
                    ASSERT_TRUE( DATATABLE_STATUS_OK == reference.DeleteRows( remove, memoryDeleted ) );
                }

                for ( UInt32 q=0; q<6; ++q )
                {
                    CDataTableQuery query;
                    const UInt32 predicates = random.Below( 3 );
                    for ( UInt32 p=0; p<predicates; ++p )
                        query.Where( RandomPredicate( random, *schema ) );
                    if ( random.Chance( 30 ) )
                    {
                        query.Select( "id" );
                        query.Select( schema->GetColumn( 1 + random.Below( schema->GetColumnCount() - 1 ) ).name );
                    }
                    CString::StringVector pgRows;
                    CString::StringVector memoryRows;
                    const TDataTableStatus pgScan = ScanRows( *table, query, pgRows, random.Chance( 30 ) );
                    const TDataTableStatus memoryScan = ScanRows( reference, query, memoryRows );
                    ++comparisons;
                    if ( pgScan != memoryScan || pgRows != memoryRows )
                    {
                        ++mismatches;
                        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PostgreSqlServer: query " << comparisons << " differs: " << pgRows.size() << " rows (" <<
                            DataTableStatusToString( pgScan ) << ") against " << memoryRows.size() << " (" << DataTableStatusToString( memoryScan ) << ")" );
                    }
                    rowsCompared += (UInt32) memoryRows.size();
                }
            }
            CString::StringVector pgAll;
            CString::StringVector memoryAll;
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, CDataTableQuery(), pgAll ) && DATATABLE_STATUS_OK == ScanRows( reference, CDataTableQuery(), memoryAll ) );
            ASSERT_TRUE( pgAll == memoryAll && !pgAll.empty() );
            ASSERT_TRUE( 72 == comparisons && 0 == mismatches && rowsCompared > 0 );
            GUCEF_LOG( LOGLEVEL_NORMAL, "PostgreSqlServer: " << comparisons << " random queries compared, " << rowsCompared << " rows" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Server: upserts replace by key with the last row of a key winning, an insert of a known key changes nothing, replace partitions replaces all rows" )
        if ( !fixture.available )
            GUCEF_TESTFW_SKIP( SkipReason );
        try
        {
            ASSERT_TRUE( fixture.opened );
            COMCORE::CIDatabaseClientPtr& client = fixture.client;
            CIDataTablePtr table;
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->CreateTable( MakeKeyValueDefinition( "kv", true ), table ) );

            TRowVector rows;
            rows.push_back( KeyValue( 1, "a" ) );
            rows.push_back( KeyValue( 2, "b" ) );
            rows.push_back( KeyValue( 3, "c" ) );
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, rows, DATATABLE_WRITE_INSERT, &result ) );
            ASSERT_TRUE( 1 == result.partitions.size() && 3 == result.partitions[ 0 ].rowsWritten );

            // a known key fails the whole write
            rows.clear();
            rows.push_back( KeyValue( 4, "d" ) );
            rows.push_back( KeyValue( 3, "c2" ) );
            ASSERT_TRUE( DATATABLE_STATUS_CONFLICT == WriteRows( *table, rows, DATATABLE_WRITE_INSERT, &result ) );
            ASSERT_TRUE( DATATABLE_STATUS_CONFLICT == result.GetOverallStatus() && !table->GetLastErrorMessage().IsNULLOrEmpty() );
            ASSERT_TRUE( 3 == CountRows( *table ) );
            rows.clear();
            rows.push_back( KeyValue( 7, "x" ) );
            rows.push_back( KeyValue( 7, "y" ) );
            ASSERT_TRUE( DATATABLE_STATUS_CONFLICT == WriteRows( *table, rows, DATATABLE_WRITE_INSERT ) );

            rows.clear();
            rows.push_back( KeyValue( 2, "b2" ) );
            rows.push_back( KeyValue( 5, "e" ) );
            rows.push_back( KeyValue( 5, "e2" ) );
            rows.push_back( KeyValue( 1, GUCEF_NULL ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, rows, DATATABLE_WRITE_UPSERT, &result ) );
            ASSERT_TRUE( 3 == result.partitions[ 0 ].rowsWritten && 2 == result.partitions[ 0 ].rowsReplaced );
            CString::StringVector scanned;
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, CDataTableQuery(), scanned ) );
            ASSERT_TRUE( 4 == scanned.size() );
            ASSERT_TRUE( scanned[ 0 ] == "1|<null>|" && scanned[ 1 ] == "2|b2|" && scanned[ 2 ] == "3|c|" && scanned[ 3 ] == "5|e2|" );

            rows.clear();
            rows.push_back( KeyValue( 9, "i" ) );
            rows.push_back( KeyValue( 10, "j" ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, rows, DATATABLE_WRITE_REPLACE_PARTITIONS, &result ) );
            ASSERT_TRUE( 4 == result.partitions[ 0 ].rowsDeleted && 2 == result.partitions[ 0 ].rowsWritten );
            ASSERT_TRUE( 2 == CountRows( *table ) );

            // without a primary key there is nothing to upsert on
            CIDataTablePtr keyless;
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->CreateTable( MakeKeyValueDefinition( "keyless", false ), keyless ) );
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == WriteRows( *keyless, rows, DATATABLE_WRITE_UPSERT ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *keyless, rows, DATATABLE_WRITE_INSERT ) && DATATABLE_STATUS_OK == WriteRows( *keyless, rows, DATATABLE_WRITE_INSERT ) );
            ASSERT_TRUE( 4 == CountRows( *keyless ) );

            // a table of key columns only: an upsert of a known key changes nothing and counts as a replacement
            CDataTableDefinition tags;
            tags.name = "tags";
            tags.schema.AddColumn( CDataTableColumnDef( "tag", DATATABLE_TYPE_UTF8, false ) );
            CString::StringVector tagKey;
            tagKey.push_back( "tag" );
            tags.schema.SetPrimaryKey( tagKey );
            CIDataTablePtr tagTable;
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->CreateTable( tags, tagTable ) );
            TRowVector tagRows( 2, TRow( 1 ) );
            tagRows[ 0 ][ 0 ] = CVariant( CUtf8String( "red" ) );
            tagRows[ 1 ][ 0 ] = CVariant( CUtf8String( "blue" ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *tagTable, tagRows, DATATABLE_WRITE_UPSERT, &result ) && 0 == result.partitions[ 0 ].rowsReplaced );
            tagRows[ 1 ][ 0 ] = CVariant( CUtf8String( "green" ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *tagTable, tagRows, DATATABLE_WRITE_UPSERT, &result ) && 1 == result.partitions[ 0 ].rowsReplaced );
            ASSERT_TRUE( 3 == CountRows( *tagTable ) );

            // a unique index hint holds
            CDataTableDefinition unique = MakeKeyValueDefinition( "unique_v", true );
            CString::StringVector byValue;
            byValue.push_back( "v" );
            unique.indexes.push_back( CDataTableIndexDef( "by_v", byValue, true ) );
            CIDataTablePtr uniqueTable;
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->CreateTable( unique, uniqueTable ) );
            rows.clear();
            rows.push_back( KeyValue( 1, "same" ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *uniqueTable, rows, DATATABLE_WRITE_INSERT ) );
            rows[ 0 ] = KeyValue( 2, "same" );
            ASSERT_TRUE( DATATABLE_STATUS_CONFLICT == WriteRows( *uniqueTable, rows, DATATABLE_WRITE_INSERT ) );
            ASSERT_TRUE( 1 == CountRows( *uniqueTable ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Server: DeleteRows deletes the matching rows, DeletePartitions all of them, ListPartitions lists the one partition while it holds rows" )
        if ( !fixture.available )
            GUCEF_TESTFW_SKIP( SkipReason );
        try
        {
            ASSERT_TRUE( fixture.opened );
            COMCORE::CIDatabaseClientPtr& client = fixture.client;
            CIDataTablePtr table;
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->CreateTable( MakeKeyValueDefinition( "deletes", true ), table ) );
            TRowVector rows;
            for ( Int32 k=1; k<=10; ++k )
                rows.push_back( KeyValue( k, 0 == k % 2 ? "even" : "odd" ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, rows, DATATABLE_WRITE_INSERT ) );

            CDataTablePartitionInfo::TPartitionInfoVector partitions;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->ListPartitions( CDataTableQuery(), true, partitions ) );
            ASSERT_TRUE( 1 == partitions.size() && partitions[ 0 ].hasFileStats && 10 == partitions[ 0 ].rowCount && partitions[ 0 ].fileSize > 0 );
            CDataTableQuery none;
            none.WhereEquals( "v", CVariant( CUtf8String( "neither" ) ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->ListPartitions( none, false, partitions ) && partitions.empty() );

            CDataTableQuery even;
            even.WhereEquals( "v", CVariant( CUtf8String( "even" ) ) ).Where( CDataTablePredicate( "k", DATATABLE_PREDICATE_GREATER, CVariant( (Int64) 4 ) ) );
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->DeleteRows( even, result ) );
            ASSERT_TRUE( 1 == result.partitions.size() && 3 == result.partitions[ 0 ].rowsDeleted );
            ASSERT_TRUE( 7 == CountRows( *table ) );

            CDataTableQuery filter;
            filter.WhereEquals( "k", CVariant( (Int64) 1 ) );
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == table->DeletePartitions( filter, result ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->DeletePartitions( CDataTableQuery(), result ) && 7 == result.partitions[ 0 ].rowsDeleted );
            ASSERT_TRUE( 0 == CountRows( *table ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->ListPartitions( CDataTableQuery(), true, partitions ) && partitions.empty() );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->Compact( CDataTableQuery(), true, result ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Server: a transaction scope keeps its writes and tables only with a commit, other clients see nothing before it, a failed write does not end it" )
        if ( !fixture.available )
            GUCEF_TESTFW_SKIP( SkipReason );
        try
        {
            ASSERT_TRUE( fixture.opened );
            COMCORE::CIDatabaseClientPtr& client = fixture.client;
            COMCORE::CIDatabaseClientPtr other = OpenClient( fixture, false );
            ASSERT_FALSE( other.IsNULL() );
            CIDataTablePtr table;
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->CreateTable( MakeKeyValueDefinition( "accounts", true ), table ) );
            CIDataTablePtr otherTable;
            ASSERT_TRUE( DATATABLE_STATUS_OK == other->OpenTable( "accounts", otherTable ) );
            TRowVector rows;
            rows.push_back( KeyValue( 1, "a" ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, rows, DATATABLE_WRITE_INSERT ) );

            // without a commit: the rows and the table made in the scope are gone
            {
                COMCORE::CDatabaseTransaction transaction( client );
                ASSERT_TRUE( transaction.IsActive() && client->IsInTransaction() );
                rows[ 0 ] = KeyValue( 2, "b" );
                ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, rows, DATATABLE_WRITE_INSERT ) );
                CIDataTablePtr scratch;
                ASSERT_TRUE( DATATABLE_STATUS_OK == client->CreateTable( MakeKeyValueDefinition( "scratch", true ), scratch ) );
                ASSERT_TRUE( 2 == CountRows( *table ) );
                ASSERT_TRUE( 1 == CountRows( *otherTable ) );
                CString::StringSet otherNames;
                ASSERT_TRUE( DATATABLE_STATUS_OK == other->ListTables( otherNames ) && 0 == otherNames.count( "scratch" ) );
            }
            ASSERT_FALSE( client->IsInTransaction() );
            ASSERT_TRUE( 1 == CountRows( *table ) );
            CIDataTablePtr gone;
            ASSERT_TRUE( DATATABLE_STATUS_NOT_FOUND == client->OpenTable( "scratch", gone ) );
            ASSERT_FALSE( RelationExists( client, fixture.schema, "scratch" ) );

            // with a commit, a failed write inside undoes only itself
            {
                COMCORE::CDatabaseTransaction transaction( client );
                ASSERT_TRUE( transaction.IsActive() );
                rows[ 0 ] = KeyValue( 3, "c" );
                ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, rows, DATATABLE_WRITE_INSERT ) );
                rows[ 0 ] = KeyValue( 1, "again" );
                ASSERT_TRUE( DATATABLE_STATUS_CONFLICT == WriteRows( *table, rows, DATATABLE_WRITE_INSERT ) );
                rows[ 0 ] = KeyValue( 4, "d" );
                ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, rows, DATATABLE_WRITE_INSERT ) );
                ASSERT_TRUE( 1 == CountRows( *otherTable ) );
                ASSERT_TRUE( DATATABLE_STATUS_OK == transaction.Commit() );
            }
            ASSERT_TRUE( 3 == CountRows( *table ) && 3 == CountRows( *otherTable ) );

            // one transaction per client, the calls without one are refused
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == client->CommitTransaction() );
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == client->RollbackTransaction() );
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->BeginTransaction() );
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == client->BeginTransaction() );
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->RollbackTransaction() );

            // async: on a pool thread, the writes of this thread in between belong to it
            CFutureResult begun = client->BeginTransactionAsync();
            ASSERT_TRUE( WaitForFuture( begun ) && TASKSTATUS_FINISHED == StatusOfFuture( begun ) );
            rows[ 0 ] = KeyValue( 5, "e" );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, rows, DATATABLE_WRITE_INSERT ) );
            CFutureResult rolledBack = client->RollbackTransactionAsync();
            ASSERT_TRUE( WaitForFuture( rolledBack ) && TASKSTATUS_FINISHED == StatusOfFuture( rolledBack ) );
            ASSERT_TRUE( 3 == CountRows( *table ) );

            // a transaction open at close is rolled back
            ASSERT_TRUE( DATATABLE_STATUS_OK == other->BeginTransaction() );
            rows[ 0 ] = KeyValue( 6, "f" );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *otherTable, rows, DATATABLE_WRITE_INSERT ) );
            otherTable.Unlink();
            other->Close();
            ASSERT_TRUE( 3 == CountRows( *table ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Server: the async calls of the client and its tables complete their futures with their results" )
        if ( !fixture.available )
            GUCEF_TESTFW_SKIP( SkipReason );
        try
        {
            ASSERT_TRUE( fixture.opened );
            COMCORE::CIDatabaseClientPtr& client = fixture.client;
            CFutureResult created = client->CreateTableAsync( MakeKeyValueDefinition( "async_kv", true ) );
            ASSERT_TRUE( WaitForFuture( created ) );
            CIDataTablePtr table;
            ASSERT_TRUE( COMCORE::CDatabaseTableResult::TryGetFromFuture( created, table ) && !table.IsNULL() );

            TRowVector rows;
            for ( Int32 k=1; k<=5; ++k )
                rows.push_back( KeyValue( k, "v" ) );
            CFutureResult written = table->WriteAsync( MakeRows( table->GetSchema(), rows ), DATATABLE_WRITE_INSERT );
            ASSERT_TRUE( WaitForFuture( written ) && TASKSTATUS_FINISHED == StatusOfFuture( written ) );
            CString::StringVector scanned;
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, CDataTableQuery(), scanned, true ) && 5 == scanned.size() );

            CDataTableQuery some;
            some.Where( CDataTablePredicate( "k", DATATABLE_PREDICATE_LESS_OR_EQUAL, CVariant( (Int64) 2 ) ) );
            CFutureResult deleted = table->DeleteRowsAsync( some );
            ASSERT_TRUE( WaitForFuture( deleted ) && TASKSTATUS_FINISHED == StatusOfFuture( deleted ) );
            CFutureResult listed = table->ListPartitionsAsync( CDataTableQuery(), true );
            ASSERT_TRUE( WaitForFuture( listed ) && TASKSTATUS_FINISHED == StatusOfFuture( listed ) );
            ASSERT_TRUE( 3 == CountRows( *table ) );

            CFutureResult names = client->ListTablesAsync();
            ASSERT_TRUE( WaitForFuture( names ) );
            CString::StringSet tableNames;
            ASSERT_TRUE( COMCORE::CDatabaseTableNamesResult::TryGetFromFuture( names, tableNames ) && 1 == tableNames.count( "async_kv" ) );

            CFutureResult opened = client->OpenTableAsync( "missing" );
            ASSERT_TRUE( WaitForFuture( opened ) && TASKSTATUS_TASKDATA_INVALID == StatusOfFuture( opened ) );

            table.Unlink();
            CFutureResult dropped = client->DropTableAsync( "async_kv", true );
            ASSERT_TRUE( WaitForFuture( dropped ) && TASKSTATUS_FINISHED == StatusOfFuture( dropped ) );
            ASSERT_FALSE( RelationExists( client, fixture.schema, "async_kv" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Server: a read only client reads the tables and refuses every change" )
        if ( !fixture.available )
            GUCEF_TESTFW_SKIP( SkipReason );
        try
        {
            ASSERT_TRUE( fixture.opened );
            COMCORE::CIDatabaseClientPtr reader = OpenClient( fixture, true );
            ASSERT_FALSE( reader.IsNULL() );
            ASSERT_TRUE( 0 == ( reader->GetCapabilities() & GUCEF_DATABASE_CAPABILITY_WRITE ) );
            CIDataTablePtr table;
            ASSERT_TRUE( DATATABLE_STATUS_OK == reader->OpenTable( "accounts", table ) && table->IsReadOnly() );
            ASSERT_TRUE( 3 == CountRows( *table ) );
            TRowVector rows;
            rows.push_back( KeyValue( 99, "z" ) );
            ASSERT_TRUE( DATATABLE_STATUS_READ_ONLY == WriteRows( *table, rows, DATATABLE_WRITE_INSERT ) );
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_READ_ONLY == table->DeleteRows( CDataTableQuery(), result ) );
            CIDataTablePtr refused;
            ASSERT_TRUE( DATATABLE_STATUS_READ_ONLY == reader->CreateTable( MakeKeyValueDefinition( "nope", true ), refused ) );
            ASSERT_TRUE( DATATABLE_STATUS_READ_ONLY == reader->DropTable( "accounts", true ) );
            // a read only transaction is allowed
            ASSERT_TRUE( DATATABLE_STATUS_OK == reader->BeginTransaction() && DATATABLE_STATUS_OK == reader->CommitTransaction() );
            table.Unlink();
            reader->Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Server: column names and text with quotes and semicolons are stored and found as they are" )
        if ( !fixture.available )
            GUCEF_TESTFW_SKIP( SkipReason );
        try
        {
            ASSERT_TRUE( fixture.opened );
            COMCORE::CIDatabaseClientPtr& client = fixture.client;
            CDataTableDefinition definition;
            definition.name = "odd.names-1";
            definition.schema.AddColumn( CDataTableColumnDef( "we\"ird; DROP TABLE x; --", DATATABLE_TYPE_INT32, false ) );
            definition.schema.AddColumn( CDataTableColumnDef( "it's", DATATABLE_TYPE_UTF8, true ) );
            CString::StringVector key;
            key.push_back( "we\"ird; DROP TABLE x; --" );
            definition.schema.SetPrimaryKey( key );
            CString::StringVector byText;
            byText.push_back( "it's" );
            definition.indexes.push_back( CDataTableIndexDef( "by_text", byText ) );
            CIDataTablePtr table;
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->CreateTable( definition, table ) );
            TRowVector rows;
            rows.push_back( KeyValue( 1, "'); DELETE FROM accounts; --" ) );
            rows.push_back( KeyValue( 2, "$1 \\ \" '" ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, rows, DATATABLE_WRITE_INSERT ) );
            CDataTableQuery query;
            query.WhereEquals( "it's", CVariant( CUtf8String( "'); DELETE FROM accounts; --" ) ) );
            CString::StringVector scanned;
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, query, scanned ) && 1 == scanned.size() );
            ASSERT_TRUE( scanned[ 0 ] == "1|'); DELETE FROM accounts; --|" );
            CIDataTablePtr accounts;
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->OpenTable( "accounts", accounts ) && 3 == CountRows( *accounts ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    // the schema of this run goes, with every table in it
    if ( fixture.opened )
    {
        const CString schema = fixture.schema;
        CPostgreSqlResult dropped;
        if ( DATATABLE_STATUS_OK != RunSql( fixture.client, "DROP SCHEMA " + CPostgreSqlSql::QuoteIdentifier( schema ) + " CASCADE", CString::StringVector(), dropped ) )
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PostgreSqlServer: the schema \"" << schema << "\" of the tests could not be dropped" );
        fixture.client->Close();
        fixture.client.Unlink();
    }
}

/*-------------------------------------------------------------------------*/
