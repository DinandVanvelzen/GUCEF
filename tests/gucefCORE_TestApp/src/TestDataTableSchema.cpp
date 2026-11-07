/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDATATABLEDEFINITION_H
#include "gucefCORE_CDataTableDefinition.h"
#define GUCEF_CORE_CDATATABLEDEFINITION_H
#endif /* GUCEF_CORE_CDATATABLEDEFINITION_H ? */

#ifndef GUCEF_CORE_CDATATABLEQUERY_H
#include "gucefCORE_CDataTableQuery.h"
#define GUCEF_CORE_CDATATABLEQUERY_H
#endif /* GUCEF_CORE_CDATATABLEQUERY_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDataTableSupport.h"
#include "TestDataTableSchema.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------*/

static Int32
DaysOf( Int32 year, UInt32 month, UInt32 day )
{
    Int32 days = 0;
    CDataTableValues::DaysFromCivil( year, month, day, days );
    return days;
}

/*-------------------------------------------------------------------------*/

static CString::StringVector
Names( const char* a, const char* b = GUCEF_NULL, const char* c = GUCEF_NULL )
{
    CString::StringVector names;
    names.push_back( a );
    if ( GUCEF_NULL != b ) names.push_back( b );
    if ( GUCEF_NULL != c ) names.push_back( c );
    return names;
}

/*-------------------------------------------------------------------------*/

/** The daily bars table of the design document */
static CDataTableDefinition
MakeBarsDefinition( void )
{
    CDataTableDefinition definition;
    definition.name = "bars_1d";
    definition.codecType = "parquet";
    definition.schema.AddColumn( CDataTableColumnDef( "mic", DATATABLE_TYPE_UTF8, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "symbol", DATATABLE_TYPE_UTF8, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "date", DATATABLE_TYPE_DATE, false ) );
    definition.schema.AddColumn( CDataTableColumnDef::Decimal( "open", 18, 6 ) );
    definition.schema.AddColumn( CDataTableColumnDef::Decimal( "high", 18, 6 ) );
    definition.schema.AddColumn( CDataTableColumnDef::Decimal( "low", 18, 6 ) );
    definition.schema.AddColumn( CDataTableColumnDef::Decimal( "close", 18, 6 ) );
    CDataTableColumnDef volume( "volume", DATATABLE_TYPE_INT64 );
    volume.description = "shares traded";
    volume.encodingHint = DATATABLE_ENCODING_PLAIN;
    definition.schema.AddColumn( volume );
    definition.schema.SetPrimaryKey( Names( "mic", "symbol", "date" ) );
    definition.schema.SetSortKey( Names( "date" ) );
    definition.partitioning.AddField( "mic", "mic", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
    definition.partitioning.AddField( "symbol", "symbol", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
    definition.partitioning.AddField( "year", "date", DATATABLE_PARTITION_TRANSFORM_YEAR );
    definition.partitioning.SetPathTemplate( "{mic}/{symbol}/bars_1d/{year}.parquet" );
    definition.storage.rowGroupRowCount = 1048576;
    definition.storage.pageRowCount = 16384;
    definition.storage.keyValueMetaData.Set( "origin", CVariant( "test" ) );
    definition.backendOptions.Set( "footerCacheBytes", CVariant( "67108864" ) );
    return definition;
}

/*-------------------------------------------------------------------------*/

void
PerformDataTableSchemaTests( void )
{
    GUCEF_LOG( LOGLEVEL_NORMAL, "COMMENCING data table schema and definition TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "DataTableSchema" );

    GUCEF_TESTFW_TESTCASE( "Test: type names, widths and storage of the logical types and the text forms of the enums" )
        try
        {
            const char* names[] = { "bool", "int8", "int16", "int32", "int64", "uint8", "uint16", "uint32", "uint64", "float32", "float64",
                                    "decimal", "date", "timestamp", "utf8", "binary" };
            for ( UInt32 i=0; i<sizeof( names ) / sizeof( names[ 0 ] ); ++i )
            {
                TDataTableLogicalType type = DATATABLE_TYPE_UNSUPPORTED;
                ASSERT_TRUE( DataTableLogicalTypeFromString( names[ i ], type ) );
                ASSERT_TRUE( 0 == strcmp( names[ i ], DataTableLogicalTypeToString( type ) ) );
            }
            TDataTableLogicalType type = DATATABLE_TYPE_BOOL;
            ASSERT_FALSE( DataTableLogicalTypeFromString( "INT8", type ) || DataTableLogicalTypeFromString( "int128", type ) );
            ASSERT_TRUE( 1 == DataTableLogicalTypeByteWidth( DATATABLE_TYPE_UINT8 ) && 2 == DataTableLogicalTypeByteWidth( DATATABLE_TYPE_INT16 ) );
            ASSERT_TRUE( 4 == DataTableLogicalTypeByteWidth( DATATABLE_TYPE_DATE ) && 8 == DataTableLogicalTypeByteWidth( DATATABLE_TYPE_DECIMAL ) );
            ASSERT_TRUE( 0 == DataTableLogicalTypeByteWidth( DATATABLE_TYPE_BOOL ) && 0 == DataTableLogicalTypeByteWidth( DATATABLE_TYPE_UTF8 ) );
            ASSERT_TRUE( DATATABLE_STORAGE_BITS == DataTableLogicalTypeStorage( DATATABLE_TYPE_BOOL ) );
            ASSERT_TRUE( DATATABLE_STORAGE_FIXED == DataTableLogicalTypeStorage( DATATABLE_TYPE_TIMESTAMP ) );
            ASSERT_TRUE( DATATABLE_STORAGE_VARIABLE == DataTableLogicalTypeStorage( DATATABLE_TYPE_BINARY ) );
            ASSERT_TRUE( DATATABLE_STORAGE_NONE == DataTableLogicalTypeStorage( DATATABLE_TYPE_UNSUPPORTED ) );
            ASSERT_TRUE( DataTableLogicalTypeIsInteger( DATATABLE_TYPE_UINT64 ) && !DataTableLogicalTypeIsInteger( DATATABLE_TYPE_DECIMAL ) );
            ASSERT_TRUE( DataTableLogicalTypeIsUnsigned( DATATABLE_TYPE_UINT8 ) && !DataTableLogicalTypeIsUnsigned( DATATABLE_TYPE_INT64 ) );

            ASSERT_TRUE( 0 == strcmp( "TYPE_MISMATCH", DataTableStatusToString( DATATABLE_STATUS_TYPE_MISMATCH ) ) );
            ASSERT_TRUE( 15 == DATATABLE_STATUS_TYPE_MISMATCH && 23 == DATATABLE_STATUS_LIMIT_EXCEEDED && 3 == DATATABLE_STATUS_PRUNED );
            ASSERT_TRUE( DataTableStatusIsError( DATATABLE_STATUS_FAILED ) && !DataTableStatusIsError( DATATABLE_STATUS_PRUNED ) && !DataTableStatusIsError( DATATABLE_STATUS_END ) );

            TDataTableTimeUnit unit = DATATABLE_TIMEUNIT_MILLISECONDS;
            ASSERT_TRUE( DataTableTimeUnitFromString( "ns", unit ) && DATATABLE_TIMEUNIT_NANOSECONDS == unit );
            ASSERT_TRUE( 0 == strcmp( "us", DataTableTimeUnitToString( DATATABLE_TIMEUNIT_MICROSECONDS ) ) );
            TDataTablePartitionTransform transform = DATATABLE_PARTITION_TRANSFORM_IDENTITY;
            ASSERT_TRUE( DataTablePartitionTransformFromString( "month", transform ) && DATATABLE_PARTITION_TRANSFORM_MONTH == transform );
            ASSERT_FALSE( DataTablePartitionTransformFromString( "week", transform ) );
            TDataTableWriteMode mode = DATATABLE_WRITE_INSERT;
            ASSERT_TRUE( DataTableWriteModeFromString( "replacePartitions", mode ) && DATATABLE_WRITE_REPLACE_PARTITIONS == mode );
            TDataTableFilterMode filter = DATATABLE_FILTER_EXACT;
            ASSERT_TRUE( DataTableFilterModeFromString( "pruneOnly", filter ) && DATATABLE_FILTER_PRUNE_ONLY == filter );
            TDataTablePredicateOp op = DATATABLE_PREDICATE_EQUALS;
            ASSERT_TRUE( DataTablePredicateOpFromString( "isNotNull", op ) && DATATABLE_PREDICATE_IS_NOT_NULL == op );
            ASSERT_TRUE( 0 == strcmp( "greaterOrEqual", DataTablePredicateOpToString( DATATABLE_PREDICATE_GREATER_OR_EQUAL ) ) );
            TDataTableDictionaryMode dictionary = DATATABLE_DICTIONARY_AUTO;
            ASSERT_TRUE( DataTableDictionaryModeFromString( "never", dictionary ) && DATATABLE_DICTIONARY_NEVER == dictionary );
            TDataTableEncodingHint hint = DATATABLE_ENCODING_AUTO;
            ASSERT_TRUE( DataTableEncodingHintFromString( "dictionary", hint ) && DATATABLE_ENCODING_DICTIONARY == hint );
            ASSERT_TRUE( 64 == DataTableBufferPadding && 18 == DataTableMaxDecimalPrecision );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: column definitions: type text round trips, parameters, validity and comparison" )
        try
        {
            const char* texts[] = { "bool", "int8", "int16", "int32", "int64", "uint8", "uint16", "uint32", "uint64", "float32", "float64",
                                    "decimal(18,6)", "decimal(1,0)", "decimal(18,18)", "date", "timestamp(us,utc)", "timestamp(ms)",
                                    "timestamp(ns,utc)", "utf8", "binary" };
            for ( UInt32 i=0; i<sizeof( texts ) / sizeof( texts[ 0 ] ); ++i )
            {
                CDataTableColumnDef column( "c", DATATABLE_TYPE_UNSUPPORTED );
                ASSERT_TRUE( column.SetTypeFromText( texts[ i ] ) );
                ASSERT_TRUE( column.GetTypeText() == texts[ i ] );
                CString error;
                ASSERT_TRUE( column.IsValid( error ) );
            }

            CDataTableColumnDef column( "c", DATATABLE_TYPE_UNSUPPORTED );
            ASSERT_TRUE( column.SetTypeFromText( " decimal( 10 , 2 ) " ) && 10 == column.decimalPrecision && 2 == column.decimalScale );
            ASSERT_TRUE( column.SetTypeFromText( "decimal" ) && 18 == column.decimalPrecision && 0 == column.decimalScale );
            ASSERT_TRUE( column.SetTypeFromText( "decimal(9)" ) && 9 == column.decimalPrecision && 0 == column.decimalScale );
            ASSERT_TRUE( column.SetTypeFromText( "timestamp" ) && DATATABLE_TIMEUNIT_MICROSECONDS == column.timeUnit && column.utcAdjusted );
            ASSERT_TRUE( column.SetTypeFromText( "timestamp( ns )" ) && DATATABLE_TIMEUNIT_NANOSECONDS == column.timeUnit && !column.utcAdjusted );

            const char* invalid[] = { "", "int9", "INT8", "decimal(19,2)", "decimal(5,6)", "decimal(0)", "decimal(18,)", "decimal(a,b)",
                                      "decimal(18,2,1)", "timestamp(xs)", "timestamp(us,local)", "timestamp(us,utc,x)", "utf8(3)", "date(", "decimal(1000,1)" };
            for ( UInt32 i=0; i<sizeof( invalid ) / sizeof( invalid[ 0 ] ); ++i )
            {
                CDataTableColumnDef unchanged( "u", DATATABLE_TYPE_INT32 );
                ASSERT_FALSE( unchanged.SetTypeFromText( invalid[ i ] ) );
                ASSERT_TRUE( DATATABLE_TYPE_INT32 == unchanged.type );
            }

            CDataTableColumnDef a = CDataTableColumnDef::Decimal( "a", 18, 6 );
            CDataTableColumnDef b = CDataTableColumnDef::Decimal( "b", 18, 6, false );
            CDataTableColumnDef c = CDataTableColumnDef::Decimal( "c", 18, 4 );
            ASSERT_TRUE( a.IsSameType( b ) && !a.IsSameType( c ) && a != b );
            ASSERT_TRUE( CDataTableColumnDef::Timestamp( "x", DATATABLE_TIMEUNIT_MILLISECONDS ).IsSameType( CDataTableColumnDef::Timestamp( "y", DATATABLE_TIMEUNIT_MILLISECONDS ) ) );
            ASSERT_FALSE( CDataTableColumnDef::Timestamp( "x", DATATABLE_TIMEUNIT_MILLISECONDS ).IsSameType( CDataTableColumnDef::Timestamp( "x", DATATABLE_TIMEUNIT_MILLISECONDS, false ) ) );
            ASSERT_TRUE( CDataTableColumnDef( "x", DATATABLE_TYPE_UTF8 ).IsSameType( CDataTableColumnDef( "y", DATATABLE_TYPE_UTF8, false ) ) );

            CString error;
            ASSERT_FALSE( CDataTableColumnDef( "", DATATABLE_TYPE_INT32 ).IsValid( error ) );
            ASSERT_FALSE( CDataTableColumnDef( "bad\tname", DATATABLE_TYPE_INT32 ).IsValid( error ) );
            ASSERT_FALSE( CDataTableColumnDef( "x", DATATABLE_TYPE_UNSUPPORTED ).IsValid( error ) );
            ASSERT_FALSE( CDataTableColumnDef::Decimal( "x", 19, 2 ).IsValid( error ) );
            ASSERT_FALSE( CDataTableColumnDef::Decimal( "x", 4, 5 ).IsValid( error ) );
            ASSERT_TRUE( CDataTableColumnDef( "Transactions Total ($)", DATATABLE_TYPE_UTF8 ).IsValid( error ) );
            ASSERT_TRUE( CDataTableColumnDef::IsValidName( "naïve" ) && !CDataTableColumnDef::IsValidName( "" ) );

            // DataNode: defaults are left out, everything round trips
            CDataTableColumnDef full = CDataTableColumnDef::Timestamp( "ts", DATATABLE_TIMEUNIT_NANOSECONDS, false, false );
            full.encodingHint = DATATABLE_ENCODING_DICTIONARY;
            full.description = "trade time";
            CDataNode node;
            ASSERT_TRUE( full.Serialize( node, CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( node.GetAttributeValue( "type" ).AsString() == "timestamp(ns)" );
            ASSERT_TRUE( node.GetAttributeValue( "encoding" ).AsString() == "dictionary" );
            ASSERT_FALSE( node.GetAttributeValue( "nullable" ).AsBool( true ) );
            CDataTableColumnDef fullBack;
            ASSERT_TRUE( fullBack.Deserialize( node, CDataNodeSerializableSettings() ) && fullBack == full );

            CDataNode plainNode;
            CDataTableColumnDef plain( "p", DATATABLE_TYPE_INT64 );
            ASSERT_TRUE( plain.Serialize( plainNode, CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( 2 == plainNode.GetAttCount() );
            CDataTableColumnDef plainBack;
            ASSERT_TRUE( plainBack.Deserialize( plainNode, CDataNodeSerializableSettings() ) && plainBack == plain && plainBack.nullable );

            CDataNode badNode;
            badNode.SetAttribute( "name", "x" );
            badNode.SetAttribute( "type", "decimal(30,2)" );
            CDataTableColumnDef untouched( "keep", DATATABLE_TYPE_BOOL );
            ASSERT_FALSE( untouched.Deserialize( badNode, CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( untouched.name == "keep" && DATATABLE_TYPE_BOOL == untouched.type );
            badNode.SetAttribute( "type", "int32" );
            badNode.SetAttribute( "encoding", "huffman" );
            ASSERT_FALSE( untouched.Deserialize( badNode, CDataNodeSerializableSettings() ) );

            CICloneable* clone = full.Clone();
            ASSERT_TRUE( GUCEF_NULL != clone && *static_cast< CDataTableColumnDef* >( clone ) == full );
            GUCEF_DELETE clone;
            ASSERT_TRUE( full.GetClassTypeName() == "GUCEF::CORE::CDataTableColumnDef" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: schemas: columns, keys, validation and reading files of other schemas" )
        try
        {
            CDataTableSchema schema;
            CString error;
            ASSERT_FALSE( schema.Validate( error ) );                                           // no columns
            ASSERT_TRUE( schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) ) );
            ASSERT_TRUE( schema.AddColumn( CDataTableColumnDef( "Name", DATATABLE_TYPE_UTF8 ) ) );
            ASSERT_FALSE( schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT32 ) ) );    // duplicate
            ASSERT_FALSE( schema.AddColumn( CDataTableColumnDef( "", DATATABLE_TYPE_INT32 ) ) );      // invalid name
            ASSERT_FALSE( schema.AddColumn( CDataTableColumnDef::Decimal( "x", 25, 2 ) ) );           // invalid type
            ASSERT_TRUE( 2 == schema.GetColumnCount() && 1 == schema.FindColumn( "Name" ) && -1 == schema.FindColumn( "name" ) );
            ASSERT_TRUE( schema.GetColumn( 1 ).name == "Name" && 2 == schema.GetColumns().size() );
            ASSERT_TRUE( schema.Validate( error ) );
            ASSERT_TRUE( 1 == schema.GetSchemaVersion() );

            ASSERT_FALSE( schema.SetPrimaryKey( Names( "id", "missing" ) ) );
            ASSERT_FALSE( schema.SetPrimaryKey( Names( "id", "id" ) ) );
            ASSERT_TRUE( schema.GetPrimaryKey().empty() );
            ASSERT_TRUE( schema.SetPrimaryKey( Names( "id" ) ) && 1 == schema.GetPrimaryKey().size() && 0 == schema.GetPrimaryKey()[ 0 ] );
            ASSERT_TRUE( schema.SetSortKey( Names( "Name", "id" ) ) && 2 == schema.GetSortKey().size() && 1 == schema.GetSortKey()[ 0 ] );
            ASSERT_TRUE( schema.Validate( error ) );
            ASSERT_TRUE( schema.SetPrimaryKey( Names( "id", "Name" ) ) );
            ASSERT_FALSE( schema.Validate( error ) );                                           // a nullable primary key column
            ASSERT_TRUE( error.HasSubstr( "Name" ) >= 0 );
            ASSERT_TRUE( schema.SetPrimaryKey( CString::StringVector() ) && schema.GetPrimaryKey().empty() );

            // DataNode round trip with keys and version
            schema.SetPrimaryKey( Names( "id" ) );
            schema.SetSchemaVersion( 7 );
            CDataNode node;
            ASSERT_TRUE( schema.Serialize( node, CDataNodeSerializableSettings() ) );
            CDataTableSchema back;
            ASSERT_TRUE( back.Deserialize( node, CDataNodeSerializableSettings() ) && back == schema && 7 == back.GetSchemaVersion() );
            CDataNode noColumns;
            ASSERT_FALSE( back.Deserialize( noColumns, CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( back == schema );
            back.Clear();
            ASSERT_TRUE( 0 == back.GetColumnCount() && 1 == back.GetSchemaVersion() && back != schema );

            // shared immutable copies
            CDataTableSchemaPtr shared = CreateDataTableSchemaPtr( schema );
            CDataTableSchemaPtr copy = shared;
            ASSERT_TRUE( !shared.IsNULL() && copy.GetPointerAlways() == shared.GetPointerAlways() && *shared == schema );
            schema.AddColumn( CDataTableColumnDef( "later", DATATABLE_TYPE_BOOL ) );
            ASSERT_TRUE( 2 == shared->GetColumnCount() );

            // reading files of another schema
            CDataTableSchema table;
            table.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
            table.AddColumn( CDataTableColumnDef( "price", DATATABLE_TYPE_FLOAT64 ) );
            table.AddColumn( CDataTableColumnDef::Decimal( "amount", 18, 4 ) );
            table.AddColumn( CDataTableColumnDef( "added", DATATABLE_TYPE_UTF8 ) );
            CDataTableSchema file;
            file.AddColumn( CDataTableColumnDef( "price", DATATABLE_TYPE_FLOAT32 ) );
            file.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT32, false ) );
            file.AddColumn( CDataTableColumnDef::Decimal( "amount", 16, 2 ) );
            file.AddColumn( CDataTableColumnDef( "extra", DATATABLE_TYPE_BINARY ) );
            ASSERT_TRUE( table.IsReadableAs( file, error ) );

            CDataTableSchema nullableId( file );
            nullableId = CDataTableSchema();
            nullableId.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, true ) );
            ASSERT_FALSE( table.IsReadableAs( nullableId, error ) );                            // may hold nulls the table does not allow
            CDataTableSchema lacksId;
            lacksId.AddColumn( CDataTableColumnDef( "price", DATATABLE_TYPE_FLOAT64 ) );
            ASSERT_FALSE( table.IsReadableAs( lacksId, error ) );                               // the table column is not nullable
            CDataTableSchema narrowing;
            narrowing.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
            narrowing.AddColumn( CDataTableColumnDef::Decimal( "amount", 18, 6 ) );           // a higher scale cannot be rescaled exactly
            ASSERT_FALSE( table.IsReadableAs( narrowing, error ) );
            CDataTableSchema tooWide;
            tooWide.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
            tooWide.AddColumn( CDataTableColumnDef::Decimal( "amount", 18, 2 ) );             // 16 integer digits do not fit 14
            ASSERT_FALSE( table.IsReadableAs( tooWide, error ) );
            CDataTableSchema otherType;
            otherType.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
            otherType.AddColumn( CDataTableColumnDef( "added", DATATABLE_TYPE_INT32 ) );
            ASSERT_FALSE( table.IsReadableAs( otherType, error ) );
            ASSERT_TRUE( error.HasSubstr( "added" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: a file schema lists the columns a codec cannot decode as UNSUPPORTED, a table schema refuses them" )
        try
        {
            CDataTableSchema fileSchema;
            CString error;
            ASSERT_TRUE( fileSchema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) ) );
            ASSERT_TRUE( fileSchema.AddColumn( CDataTableColumnDef( "nested.list.element", DATATABLE_TYPE_UNSUPPORTED ) ) );
            ASSERT_FALSE( fileSchema.AddColumn( CDataTableColumnDef( "", DATATABLE_TYPE_UNSUPPORTED ) ) );        // invalid name
            ASSERT_FALSE( fileSchema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_UNSUPPORTED ) ) );      // duplicate
            ASSERT_TRUE( 2 == fileSchema.GetColumnCount() && DATATABLE_TYPE_UNSUPPORTED == fileSchema.GetColumn( 1 ).type );
            ASSERT_FALSE( fileSchema.Validate( error ) );
            ASSERT_TRUE( error.HasSubstr( "nested.list.element" ) >= 0 );

            // the column can never be projected, the others stay readable
            CDataTableQuery all;
            CDataTableResolvedQuery resolved;
            ASSERT_TRUE( all.Resolve( fileSchema, resolved, error ) );
            ASSERT_TRUE( 1 == resolved.projection.size() && 0 == resolved.projection[ 0 ] );
            CDataTableQuery unsupported;
            unsupported.Select( "nested.list.element" );
            ASSERT_FALSE( unsupported.Resolve( fileSchema, resolved, error ) );
            ASSERT_TRUE( DATATABLE_STATUS_UNSUPPORTED == resolved.status );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: partition fields, path templates and their validation against the schema" )
        try
        {
            CDataTablePartitionSpec spec;
            ASSERT_FALSE( spec.IsPartitioned() );
            ASSERT_TRUE( spec.AddField( "mic", "mic", DATATABLE_PARTITION_TRANSFORM_IDENTITY ) );
            ASSERT_FALSE( spec.AddField( "mic", "other", DATATABLE_PARTITION_TRANSFORM_IDENTITY ) );   // duplicate field
            ASSERT_FALSE( spec.AddField( "bad-name", "x", DATATABLE_PARTITION_TRANSFORM_IDENTITY ) );
            ASSERT_FALSE( spec.AddField( "", "x", DATATABLE_PARTITION_TRANSFORM_IDENTITY ) );
            ASSERT_FALSE( spec.AddField( "x", "", DATATABLE_PARTITION_TRANSFORM_IDENTITY ) );
            ASSERT_TRUE( spec.IsPartitioned() && GUCEF_NULL != spec.FindField( "mic" ) && GUCEF_NULL == spec.FindField( "year" ) );

            const char* invalid[] = { "", "/abs/{mic}", "a//b", "a/./b", "a/../b", "{mic}{x}", "{mic", "mic}", "{}", "a\\b", "{a-b}", "a/", "x/{mic}{year}.parquet" };
            for ( UInt32 i=0; i<sizeof( invalid ) / sizeof( invalid[ 0 ] ); ++i )
                ASSERT_FALSE( spec.SetPathTemplate( invalid[ i ] ) );
            ASSERT_TRUE( spec.GetPathTemplate().IsNULLOrEmpty() );
            ASSERT_TRUE( spec.SetPathTemplate( "{mic}/{symbol}/bars_1d/{year}.parquet" ) && 4 == spec.GetSegmentCount() );
            ASSERT_TRUE( spec.SetPathTemplate( "data_{mic}.csv" ) && 1 == spec.GetSegmentCount() );

            CDataTableSchema schema;
            schema.AddColumn( CDataTableColumnDef( "mic", DATATABLE_TYPE_UTF8, false ) );
            schema.AddColumn( CDataTableColumnDef( "symbol", DATATABLE_TYPE_UTF8, false ) );
            schema.AddColumn( CDataTableColumnDef( "date", DATATABLE_TYPE_DATE, false ) );
            schema.AddColumn( CDataTableColumnDef( "price", DATATABLE_TYPE_FLOAT64, false ) );
            schema.AddColumn( CDataTableColumnDef( "note", DATATABLE_TYPE_UTF8 ) );
            schema.AddColumn( CDataTableColumnDef::Timestamp( "ts", DATATABLE_TIMEUNIT_MICROSECONDS, true, false ) );

            CDataTablePartitionSpec good;
            good.AddField( "mic", "mic", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
            good.AddField( "symbol", "symbol", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
            good.AddField( "year", "date", DATATABLE_PARTITION_TRANSFORM_YEAR );
            good.SetPathTemplate( "{mic}/{symbol}/bars_1d/{year}.parquet" );
            CString error;
            ASSERT_TRUE( good.Validate( schema, error ) );
            CDataTablePartitionSpec none;
            ASSERT_TRUE( none.Validate( schema, error ) );

            CDataTablePartitionSpec noTemplate;
            noTemplate.AddField( "mic", "mic", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
            ASSERT_FALSE( noTemplate.Validate( schema, error ) );
            CDataTablePartitionSpec noFields;
            noFields.SetPathTemplate( "fixed/path.parquet" );
            ASSERT_FALSE( noFields.Validate( schema, error ) );
            CDataTablePartitionSpec unknownToken( good );
            unknownToken.SetPathTemplate( "{mic}/{symbol}/{year}/{day}.parquet" );
            ASSERT_FALSE( unknownToken.Validate( schema, error ) );
            CDataTablePartitionSpec missingField( good );
            missingField.SetPathTemplate( "{mic}/{year}.parquet" );
            ASSERT_FALSE( missingField.Validate( schema, error ) );
            CDataTablePartitionSpec twice( good );
            twice.SetPathTemplate( "{mic}/{symbol}/{year}/{mic}.parquet" );
            ASSERT_FALSE( twice.Validate( schema, error ) );
            CDataTablePartitionSpec nullableSource;
            nullableSource.AddField( "note", "note", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
            nullableSource.SetPathTemplate( "{note}.parquet" );
            ASSERT_FALSE( nullableSource.Validate( schema, error ) );
            CDataTablePartitionSpec floatIdentity;
            floatIdentity.AddField( "p", "price", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
            floatIdentity.SetPathTemplate( "{p}.parquet" );
            ASSERT_FALSE( floatIdentity.Validate( schema, error ) );
            CDataTablePartitionSpec yearOfText;
            yearOfText.AddField( "y", "mic", DATATABLE_PARTITION_TRANSFORM_YEAR );
            yearOfText.SetPathTemplate( "{y}.parquet" );
            ASSERT_FALSE( yearOfText.Validate( schema, error ) );
            CDataTablePartitionSpec dayOfTimestamp;
            dayOfTimestamp.AddField( "d", "ts", DATATABLE_PARTITION_TRANSFORM_DAY );
            dayOfTimestamp.SetPathTemplate( "{d}.parquet" );
            ASSERT_TRUE( dayOfTimestamp.Validate( schema, error ) );
            CDataTablePartitionSpec missingColumn;
            missingColumn.AddField( "x", "absent", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
            missingColumn.SetPathTemplate( "{x}.parquet" );
            ASSERT_FALSE( missingColumn.Validate( schema, error ) );

            // the primary key must hold every source column
            CDataTableSchema keyed( schema );
            keyed.SetPrimaryKey( Names( "mic", "date" ) );
            ASSERT_FALSE( good.Validate( keyed, error ) );
            ASSERT_TRUE( error.HasSubstr( "symbol" ) >= 0 );
            keyed.SetPrimaryKey( Names( "mic", "symbol", "date" ) );
            ASSERT_TRUE( good.Validate( keyed, error ) );

            // DataNode round trip
            CDataNode node;
            ASSERT_TRUE( good.Serialize( node, CDataNodeSerializableSettings() ) );
            CDataTablePartitionSpec back;
            ASSERT_TRUE( back.Deserialize( node, CDataNodeSerializableSettings() ) && back == good );
            ASSERT_TRUE( 4 == back.GetSegmentCount() && 3 == back.GetFields().size() );
            CDataNode badTransform;
            CDataNode* fields = badTransform.AddChild( "fields", GUCEF_DATATYPE_ARRAY );
            CDataNode* field = fields->AddChild( "field", GUCEF_DATATYPE_OBJECT );
            field->SetAttribute( "name", "w" );
            field->SetAttribute( "source", "date" );
            field->SetAttribute( "transform", "week" );
            ASSERT_FALSE( back.Deserialize( badTransform, CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( back == good );
            spec.Clear();
            ASSERT_FALSE( spec.IsPartitioned() || !spec.GetPathTemplate().IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: path segment escaping: case, Windows device names, dots and other bytes" )
        try
        {
            ASSERT_TRUE( CDataTablePartitionSpec::EscapeSegmentValue( "KO" ) == "KO" );
            ASSERT_TRUE( CDataTablePartitionSpec::EscapeSegmentValue( "BRK.B" ) == "BRK.B" );
            ASSERT_TRUE( CDataTablePartitionSpec::EscapeSegmentValue( "XNYS-2_0" ) == "XNYS-2_0" );
            ASSERT_TRUE( CDataTablePartitionSpec::EscapeSegmentValue( "brk.b" ) == "%62%72%6B.%62" );
            ASSERT_TRUE( CDataTablePartitionSpec::EscapeSegmentValue( ".X" ) == "%2EX" );
            ASSERT_TRUE( CDataTablePartitionSpec::EscapeSegmentValue( "X." ) == "X%2E" );
            ASSERT_TRUE( CDataTablePartitionSpec::EscapeSegmentValue( "." ) == "%2E" );
            ASSERT_TRUE( CDataTablePartitionSpec::EscapeSegmentValue( "AUX" ) == "%41UX" );
            ASSERT_TRUE( CDataTablePartitionSpec::EscapeSegmentValue( "CON" ) == "%43ON" );
            ASSERT_TRUE( CDataTablePartitionSpec::EscapeSegmentValue( "NUL.B" ) == "%4EUL.B" );
            ASSERT_TRUE( CDataTablePartitionSpec::EscapeSegmentValue( "COM1" ) == "%43OM1" );
            ASSERT_TRUE( CDataTablePartitionSpec::EscapeSegmentValue( "LPT9" ) == "%4CPT9" );
            ASSERT_TRUE( CDataTablePartitionSpec::EscapeSegmentValue( "COM0" ) == "COM0" );
            ASSERT_TRUE( CDataTablePartitionSpec::EscapeSegmentValue( "CONX" ) == "CONX" );
            ASSERT_TRUE( CDataTablePartitionSpec::EscapeSegmentValue( "PRNT" ) == "PRNT" );
            ASSERT_TRUE( CDataTablePartitionSpec::EscapeSegmentValue( "aux" ) == "%61%75%78" );
            ASSERT_TRUE( CDataTablePartitionSpec::EscapeSegmentValue( "A B/C" ) == "A%20B%2FC" );
            ASSERT_TRUE( CDataTablePartitionSpec::EscapeSegmentValue( "100%" ) == "100%25" );
            ASSERT_TRUE( CDataTablePartitionSpec::EscapeSegmentValue( "\xC3\xA9" ) == "%C3%A9" );

            const char* values[] = { "KO", "brk.b", ".X", "X.", "AUX", "aux", "COM1.TXT", "A B/C", "100%", "\xC3\xA9t\xC3\xA9", "2026-11-03" };
            for ( UInt32 i=0; i<sizeof( values ) / sizeof( values[ 0 ] ); ++i )
            {
                CString escaped = CDataTablePartitionSpec::EscapeSegmentValue( values[ i ] );
                CString back;
                ASSERT_TRUE( CDataTablePartitionSpec::UnescapeSegmentValue( escaped, back ) && back == values[ i ] );
            }

            CString value;
            ASSERT_FALSE( CDataTablePartitionSpec::UnescapeSegmentValue( "%4", value ) );           // truncated escape
            ASSERT_FALSE( CDataTablePartitionSpec::UnescapeSegmentValue( "A%ZZ", value ) );         // not hex
            ASSERT_FALSE( CDataTablePartitionSpec::UnescapeSegmentValue( "%41B", value ) );         // 'A' needs no escape
            ASSERT_FALSE( CDataTablePartitionSpec::UnescapeSegmentValue( "ab", value ) );           // lower case must be escaped
            ASSERT_FALSE( CDataTablePartitionSpec::UnescapeSegmentValue( "AUX", value ) );          // a device name must be escaped
            ASSERT_FALSE( CDataTablePartitionSpec::UnescapeSegmentValue( "", value ) );
            ASSERT_TRUE( CDataTablePartitionSpec::UnescapeSegmentValue( "%62", value ) && value == "b" );
            ASSERT_TRUE( CDataTablePartitionSpec::UnescapeSegmentValue( "%41UX", value ) && value == "AUX" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: a template whose separator between two fields can occur inside an escaped value is refused, the others take every value" )
        try
        {
            CDataTableSchema schema;
            schema.AddColumn( CDataTableColumnDef( "sym", DATATABLE_TYPE_UTF8, false ) );
            schema.AddColumn( CDataTableColumnDef( "cls", DATATABLE_TYPE_UTF8, false ) );
            schema.AddColumn( CDataTableColumnDef( "date", DATATABLE_TYPE_DATE, false ) );
            schema.AddColumn( CDataTableColumnDef( "n", DATATABLE_TYPE_INT32, false ) );

            // '-', '.', '_', digits, upper case letters and '%' occur in escaped values: "BF-B", "BRK.B", "%41UX", "2026-11-03", "-5"
            const char* ambiguous[] = { "{sym}-{cls}.parquet", "{sym}.{cls}.parquet", "{sym}_{cls}.parquet", "x/{sym}X{cls}/a.parquet",
                                        "{sym}%{cls}.parquet", "{date}-{n}.csv", "{n}9{sym}", "{sym}-_.{cls}" };
            for ( UInt32 i=0; i<sizeof( ambiguous ) / sizeof( ambiguous[ 0 ] ); ++i )
            {
                CDataTablePartitionSpec spec;
                spec.AddField( "sym", "sym", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
                spec.AddField( "cls", "cls", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
                spec.AddField( "date", "date", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
                spec.AddField( "n", "n", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
                const CString pathTemplate( ambiguous[ i ] );
                CDataTablePartitionSpec used;
                for ( UInt32 f=0; f<(UInt32) spec.GetFields().size(); ++f )
                {
                    const CDataTablePartitionSpec::SField& field = spec.GetFields()[ f ];
                    if ( pathTemplate.HasSubstr( "{" + field.name + "}", true ) >= 0 )
                        used.AddField( field.name, field.sourceColumn, field.transform );
                }
                ASSERT_TRUE( used.SetPathTemplate( pathTemplate ) );
                CString error;
                ASSERT_FALSE( used.Validate( schema, error ) );
                ASSERT_TRUE( error.HasSubstr( "separator", true ) >= 0 );
            }

            // a separator that holds a byte no escaped value has: the value ends where the separator starts
            const char* separators[] = { "~", "_s", "-x-", "@" };
            const char* symbols[] = { "BF-B", "BRK.B", "KO", "A~B", "x_s", "-x-", "%41", "CON", "9" };
            const char* classes[] = { "B", "-", "e.f", "_s", "~", "A-x-B" };
            for ( UInt32 s=0; s<sizeof( separators ) / sizeof( separators[ 0 ] ); ++s )
            {
                CDataTablePartitionSpec spec;
                spec.AddField( "sym", "sym", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
                spec.AddField( "cls", "cls", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
                ASSERT_TRUE( spec.SetPathTemplate( CString( "{sym}" ) + separators[ s ] + "{cls}.parquet" ) );
                CString error;
                ASSERT_TRUE( spec.Validate( schema, error ) );
                for ( UInt32 a=0; a<sizeof( symbols ) / sizeof( symbols[ 0 ] ); ++a )
                {
                    for ( UInt32 b=0; b<sizeof( classes ) / sizeof( classes[ 0 ] ); ++b )
                    {
                        CValueList keys;
                        keys.SetAllowMultipleValues( false );
                        keys.Set( "sym", CVariant( symbols[ a ] ) );
                        keys.Set( "cls", CVariant( classes[ b ] ) );
                        CString path;
                        CValueList parsed;
                        parsed.SetAllowMultipleValues( false );
                        ASSERT_TRUE( spec.BuildPath( keys, path ) && spec.ParseSegment( 0, path, parsed ) );
                        ASSERT_TRUE( parsed.GetValueAlways( "sym" ).AsString() == symbols[ a ] && parsed.GetValueAlways( "cls" ).AsString() == classes[ b ] );
                    }
                }
            }

            // the intraday template of the tests: "_s" holds a lower case letter, which escaped values never have
            CDataTablePartitionSpec intraday;
            intraday.AddField( "date", "date", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
            intraday.AddField( "n", "n", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
            ASSERT_TRUE( intraday.SetPathTemplate( "day_{date}_s{n}.parquet" ) );
            CString error;
            ASSERT_TRUE( intraday.Validate( schema, error ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: partition paths are built from key values and parsed back level by level" )
        try
        {
            CDataTablePartitionSpec spec = MakeBarsDefinition().partitioning;
            CValueList keys;
            keys.SetAllowMultipleValues( false );
            keys.Set( "mic", CVariant( "XNYS" ) );
            keys.Set( "symbol", CVariant( "BRK.B" ) );
            keys.Set( "year", CDataTableValues::Date( DaysOf( 2019, 7, 4 ) ) );
            CString path;
            ASSERT_TRUE( spec.BuildPath( keys, path ) && path == "XNYS/BRK.B/bars_1d/2019.parquet" );

            // any date or timestamp in the period gives the same partition, a timestamp before 1970 included
            keys.Set( "year", CDataTableValues::Timestamp( -1, DATATABLE_TIMEUNIT_MICROSECONDS ) );
            ASSERT_TRUE( spec.BuildPath( keys, path ) && path == "XNYS/BRK.B/bars_1d/1969.parquet" );
            keys.Set( "year", CDataTableValues::Date( DaysOf( 999, 12, 31 ) ) );
            ASSERT_TRUE( spec.BuildPath( keys, path ) && path == "XNYS/BRK.B/bars_1d/0999.parquet" );
            keys.Set( "symbol", CVariant( "aux" ) );
            keys.Set( "year", CDataTableValues::Date( DaysOf( 2026, 1, 1 ) ) );
            ASSERT_TRUE( spec.BuildPath( keys, path ) && path == "XNYS/%61%75%78/bars_1d/2026.parquet" );

            CValueList incomplete;
            incomplete.Set( "mic", CVariant( "XNYS" ) );
            ASSERT_FALSE( spec.BuildPath( incomplete, path ) );
            CValueList wrongType;
            wrongType.SetAllowMultipleValues( false );
            wrongType.Set( "mic", CVariant( "XNYS" ) );
            wrongType.Set( "symbol", CVariant( "KO" ) );
            wrongType.Set( "year", CVariant( "2019" ) );
            ASSERT_FALSE( spec.BuildPath( wrongType, path ) );
            wrongType.Set( "year", CDataTableValues::Date( 0 ) );
            wrongType.Set( "symbol", CVariant( "" ) );
            ASSERT_FALSE( spec.BuildPath( wrongType, path ) );

            CValueList parsed;
            ASSERT_TRUE( spec.ParseSegment( 0, "XNYS", parsed ) && parsed.GetValue( "mic" ).AsString() == "XNYS" );
            ASSERT_TRUE( spec.ParseSegment( 1, "BRK.B", parsed ) && parsed.GetValue( "symbol" ).AsString() == "BRK.B" );
            ASSERT_TRUE( spec.ParseSegment( 1, "%61%75%78", parsed ) && parsed.GetValue( "symbol" ).AsString() == "aux" );
            ASSERT_TRUE( spec.ParseSegment( 2, "bars_1d", parsed ) );
            ASSERT_FALSE( spec.ParseSegment( 2, "bars_1m", parsed ) );
            ASSERT_TRUE( spec.ParseSegment( 3, "2019.parquet", parsed ) );
            ASSERT_TRUE( GUCEF_DATATYPE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == parsed.GetValue( "year" ).GetTypeId() );
            ASSERT_TRUE( DaysOf( 2019, 1, 1 ) == parsed.GetValue( "year" ).AsInt32() );
            ASSERT_FALSE( spec.ParseSegment( 3, "2019.parquet.tmp-host1-7", parsed ) );         // temporary files
            ASSERT_FALSE( spec.ParseSegment( 3, "x2019.parquet", parsed ) );
            ASSERT_FALSE( spec.ParseSegment( 3, ".parquet", parsed ) );
            ASSERT_FALSE( spec.ParseSegment( 3, "19.parquet", parsed ) );
            ASSERT_FALSE( spec.ParseSegment( 0, "xnys", parsed ) );                             // not the escaped form
            ASSERT_FALSE( spec.ParseSegment( 0, "XN\xC3\xA9", parsed ) );
            ASSERT_FALSE( spec.ParseSegment( 4, "anything", parsed ) );

            // the parsed key values build the same path again
            CValueList all;
            ASSERT_TRUE( spec.ParseSegment( 0, "XNYS", all ) && spec.ParseSegment( 1, "KO", all ) && spec.ParseSegment( 3, "1962.parquet", all ) );
            ASSERT_TRUE( spec.BuildPath( all, path ) && path == "XNYS/KO/bars_1d/1962.parquet" );

            // intraday: the trading date as a DAY field, months, an integer identity and literal text around tokens
            CDataTablePartitionSpec intraday;
            intraday.AddField( "mic", "mic", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
            intraday.AddField( "year", "tradingDate", DATATABLE_PARTITION_TRANSFORM_YEAR );
            intraday.AddField( "tradingDate", "tradingDate", DATATABLE_PARTITION_TRANSFORM_DAY );
            intraday.AddField( "month", "ts", DATATABLE_PARTITION_TRANSFORM_MONTH );
            intraday.AddField( "shard", "shard", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
            ASSERT_TRUE( intraday.SetPathTemplate( "{mic}/bars_1m/{year}/{month}/day_{tradingDate}_s{shard}.parquet" ) );
            CValueList intradayKeys;
            intradayKeys.Set( "mic", CVariant( "XASX" ) );
            intradayKeys.Set( "year", CDataTableValues::Date( DaysOf( 2026, 11, 3 ) ) );
            intradayKeys.Set( "tradingDate", CDataTableValues::Date( DaysOf( 2026, 11, 3 ) ) );
            Int64 ticks = 0;
            CDataTableValues::ParseTimestamp( "2026-11-02T23:00:00Z", 20, DATATABLE_TIMEUNIT_MICROSECONDS, ticks );
            intradayKeys.Set( "month", CDataTableValues::Timestamp( ticks, DATATABLE_TIMEUNIT_MICROSECONDS ) );
            intradayKeys.Set( "shard", CVariant( (Int32) -3 ) );
            ASSERT_TRUE( intraday.BuildPath( intradayKeys, path ) );
            ASSERT_TRUE( path == "XASX/bars_1m/2026/2026-11/day_2026-11-03_s-3.parquet" );
            CValueList intradayParsed;
            ASSERT_TRUE( intraday.ParseSegment( 3, "2026-11", intradayParsed ) && DaysOf( 2026, 11, 1 ) == intradayParsed.GetValue( "month" ).AsInt32() );
            ASSERT_TRUE( intraday.ParseSegment( 4, "day_2026-11-03_s-3.parquet", intradayParsed ) );
            ASSERT_TRUE( DaysOf( 2026, 11, 3 ) == intradayParsed.GetValue( "tradingDate" ).AsInt32() );
            ASSERT_TRUE( intradayParsed.GetValue( "shard" ).AsString() == "-3" );
            ASSERT_FALSE( intraday.ParseSegment( 4, "day_2026-11-31_s1.parquet", intradayParsed ) );    // no such day
            ASSERT_FALSE( intraday.ParseSegment( 3, "2026-13", intradayParsed ) );

            // a date identity field and an unsigned integer identity field
            CDataTablePartitionSpec byDate;
            byDate.AddField( "d", "date", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
            byDate.AddField( "n", "n", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
            byDate.SetPathTemplate( "{d}/{n}.csv" );
            CValueList dateKeys;
            dateKeys.Set( "d", CDataTableValues::Date( DaysOf( 1999, 12, 31 ) ) );
            dateKeys.Set( "n", CVariant( (UInt64) 18446744073709551615ULL ) );
            ASSERT_TRUE( byDate.BuildPath( dateKeys, path ) && path == "1999-12-31/18446744073709551615.csv" );

            // single segments, for a backend that walks the template level by level, need only the keys of their own fields
            CString segment;
            CValueList micOnly;
            micOnly.Set( "mic", CVariant( "XASX" ) );
            ASSERT_TRUE( intraday.BuildSegment( 0, micOnly, segment ) && segment == "XASX" );
            ASSERT_TRUE( intraday.BuildSegment( 1, CValueList(), segment ) && segment == "bars_1m" );
            ASSERT_FALSE( intraday.BuildSegment( 2, micOnly, segment ) );
            ASSERT_FALSE( intraday.BuildSegment( 5, intradayKeys, segment ) );
            ASSERT_TRUE( intraday.BuildSegment( 4, intradayKeys, segment ) && segment == "day_2026-11-03_s-3.parquet" );
            CString::StringVector segmentFields;
            intraday.GetSegmentFields( 4, segmentFields );
            ASSERT_TRUE( 2 == segmentFields.size() && "tradingDate" == segmentFields[ 0 ] && "shard" == segmentFields[ 1 ] );
            intraday.GetSegmentFields( 1, segmentFields );
            ASSERT_TRUE( segmentFields.empty() );
            intraday.GetSegmentFields( 9, segmentFields );
            ASSERT_TRUE( segmentFields.empty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: writer options: defaults, validation and DataNode round trip" )
        try
        {
            CDataTableWriterOptions options;
            ASSERT_TRUE( options.compression == "none" && CDataTableWriterOptions::DefaultCompressionLevel == options.compressionLevel );
            ASSERT_TRUE( 1048576 == options.rowGroupRowCount && 512ULL * 1024 * 1024 == options.rowGroupByteLimit );
            ASSERT_TRUE( 16384 == options.pageRowCount && 1024 * 1024 == options.pageByteLimit && 1024 * 1024 == options.dictionaryByteLimit );
            ASSERT_TRUE( DATATABLE_DICTIONARY_AUTO == options.dictionary && options.writeStatistics && options.writePageStatistics );
            ASSERT_TRUE( 64 == options.statisticsMaxValueBytes && 0 == options.keyValueMetaData.GetKeyCount() );
            CString error;
            ASSERT_TRUE( options.Validate( error ) );

            CDataTableWriterOptions changed;
            changed.compression = "gzip";
            changed.compressionLevel = 6;
            changed.rowGroupRowCount = 1000;
            changed.rowGroupByteLimit = 5000000000ULL;
            changed.pageRowCount = 100;
            changed.pageByteLimit = 4096;
            changed.dictionary = DATATABLE_DICTIONARY_NEVER;
            changed.dictionaryByteLimit = 2048;
            changed.writeStatistics = false;
            changed.writePageStatistics = false;
            changed.statisticsMaxValueBytes = 16;
            changed.keyValueMetaData.Set( "creator", CVariant( "unit test" ) );
            changed.codecOptions.Set( "level", CVariant( "fast" ) );
            ASSERT_TRUE( changed.Validate( error ) && changed != options );

            CDataNode node;
            ASSERT_TRUE( changed.Serialize( node, CDataNodeSerializableSettings() ) );
            CDataTableWriterOptions back;
            ASSERT_TRUE( back.Deserialize( node, CDataNodeSerializableSettings() ) && back == changed );

            CDataNode empty;
            CDataTableWriterOptions fromEmpty;
            fromEmpty.compression = "zstd";
            ASSERT_TRUE( fromEmpty.Deserialize( empty, CDataNodeSerializableSettings() ) && fromEmpty == options );

            CDataNode bad;
            bad.SetAttribute( "dictionary", "sometimes" );
            ASSERT_FALSE( back.Deserialize( bad, CDataNodeSerializableSettings() ) );

            CDataTableWriterOptions invalid;
            invalid.pageRowCount = 0;
            ASSERT_FALSE( invalid.Validate( error ) );
            invalid = options;
            invalid.pageRowCount = invalid.rowGroupRowCount + 1;
            ASSERT_FALSE( invalid.Validate( error ) );
            invalid = options;
            invalid.compression.Clear();
            ASSERT_FALSE( invalid.Validate( error ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: table definitions: validation and DataNode round trip" )
        try
        {
            CDataTableDefinition definition = MakeBarsDefinition();
            CString error;
            ASSERT_TRUE( definition.Validate( error ) );

            CDataNode node;
            ASSERT_TRUE( definition.Serialize( node, CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( node.GetAttributeValue( "name" ).AsString() == "bars_1d" && node.GetAttributeValue( "codec" ).AsString() == "parquet" );
            ASSERT_TRUE( 1 == node.GetAttributeValue( "schemaVersion" ).AsUInt32() );
            const CDataNode* columns = node.FindChild( "columns" );
            ASSERT_TRUE( GUCEF_NULL != columns && 8 == columns->GetNrOfChildNodes() );
            CString::StringVector primaryKey;
            ASSERT_TRUE( node.GetValuesOfChildByName( "primaryKey", primaryKey ) && 3 == primaryKey.size() && primaryKey[ 2 ] == "date" );
            ASSERT_TRUE( GUCEF_NULL != node.FindChild( "partitioning" ) && GUCEF_NULL != node.FindChild( "storage" ) && GUCEF_NULL != node.FindChild( "backendOptions" ) );

            CDataTableDefinition back;
            ASSERT_TRUE( back.Deserialize( node, CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( back == definition );
            ASSERT_TRUE( back.Validate( error ) );

            CICloneable* clone = definition.Clone();
            ASSERT_TRUE( GUCEF_NULL != clone && *static_cast< CDataTableDefinition* >( clone ) == definition );
            GUCEF_DELETE clone;

            CDataTableSchemaPtr schema = definition.CreateSchemaPtr();
            ASSERT_TRUE( !schema.IsNULL() && *schema == definition.schema );

            // an unpartitioned table writes no partitioning
            CDataTableDefinition plain;
            plain.name = "fx.rates-2";
            plain.schema.AddColumn( CDataTableColumnDef( "rate", DATATABLE_TYPE_FLOAT64 ) );
            ASSERT_TRUE( plain.Validate( error ) );
            CDataNode plainNode;
            ASSERT_TRUE( plain.Serialize( plainNode, CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( GUCEF_NULL == plainNode.FindChild( "partitioning" ) && GUCEF_NULL == plainNode.FindChild( "backendOptions" ) );
            CDataTableDefinition plainBack;
            ASSERT_TRUE( plainBack.Deserialize( plainNode, CDataNodeSerializableSettings() ) && plainBack == plain );

            // invalid definitions
            ASSERT_FALSE( CDataTableDefinition::IsValidTableName( ".hidden" ) || CDataTableDefinition::IsValidTableName( "a/b" ) || CDataTableDefinition::IsValidTableName( "" ) );
            ASSERT_TRUE( CDataTableDefinition::IsValidTableName( "bars_1d" ) );

            // names that are devices on Windows, alone or with an extension, in any case, and names Windows would cut
            // ("x." and "x" are one file there): refused on every platform, so data moves between platforms
            const char* reserved[] = { "CON", "con", "Con.json", "PRN", "aux", "AUX.parquet", "NUL", "nul.x.y", "COM1", "com9", "COM0", "LPT1",
                                       "lpt9.txt", "LPT0", "x.", "bars_1d.", "a..", "." };
            for ( UInt32 i=0; i<sizeof( reserved ) / sizeof( reserved[ 0 ] ); ++i )
                ASSERT_FALSE( CDataTableDefinition::IsValidTableName( reserved[ i ] ) );
            const char* allowed[] = { "CONSOLE", "console_log", "auxiliary", "NULL", "nuls", "COM10", "COM", "COMA", "LPT", "LPT10", "prn2",
                                      "con_x", "x.y", "a-b.c", "CON-1" };
            for ( UInt32 i=0; i<sizeof( allowed ) / sizeof( allowed[ 0 ] ); ++i )
                ASSERT_TRUE( CDataTableDefinition::IsValidTableName( allowed[ i ] ) );
            CDataTableDefinition deviceName = definition;
            deviceName.name = "aux";
            ASSERT_FALSE( deviceName.Validate( error ) );

            CDataTableDefinition badName = definition;
            badName.name = "bars 1d";
            ASSERT_FALSE( badName.Validate( error ) );
            CDataTableDefinition badCodec = definition;
            badCodec.codecType.Clear();
            ASSERT_FALSE( badCodec.Validate( error ) );
            CDataTableDefinition badPartitioning = definition;
            badPartitioning.partitioning.SetPathTemplate( "{mic}/{year}.parquet" );
            ASSERT_FALSE( badPartitioning.Validate( error ) );
            ASSERT_TRUE( error.HasSubstr( "bars_1d" ) >= 0 );
            CDataTableDefinition badStorage = definition;
            badStorage.storage.rowGroupRowCount = 0;
            ASSERT_FALSE( badStorage.Validate( error ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: table definitions round trip through JSON, and the JSON form of the design reads" )
        try
        {
            CDStoreCodecRegistry::TDStoreCodecPtr codec;
            ASSERT_TRUE( CORETEST::GetDataTableTestCodec( "json", "dstorepluginJSONPARSER", codec ) );
            if ( !codec.IsNULL() )
            {
                CDataTableDefinition definition = MakeBarsDefinition();
                definition.storage.rowGroupByteLimit = 5000000000ULL;
                CDataNode tree;
                ASSERT_TRUE( definition.Serialize( tree, CDataNodeSerializableSettings() ) );
                CDynamicBuffer output;
                {
                    CDynamicBufferAccess outputAccess( &output, false );
                    ASSERT_TRUE( codec->StoreDataTree( &tree, &outputAccess ) );
                }
                CString text = CORETEST::DataTableTestBufferText( output );
                ASSERT_TRUE( text.HasSubstr( "\"decimal(18,6)\"" ) >= 0 && text.HasSubstr( "{mic}/{symbol}/bars_1d/{year}.parquet" ) >= 0 );

                CDynamicBufferAccess rereadAccess( output );
                CDataNode reread;
                ASSERT_TRUE( codec->BuildDataTree( &reread, &rereadAccess ) );
                const CDataNode* root = reread.FindChild( "DataTableDefinition" );
                if ( GUCEF_NULL == root )
                    root = &reread;
                CDataTableDefinition back;
                ASSERT_TRUE( back.Deserialize( *root, CDataNodeSerializableSettings() ) );
                CString error;
                ASSERT_TRUE( back.Validate( error ) );
                ASSERT_TRUE( back.name == definition.name && back.schema == definition.schema && back.partitioning == definition.partitioning );
                ASSERT_TRUE( back.storage.rowGroupByteLimit == definition.storage.rowGroupByteLimit && back.storage.pageRowCount == definition.storage.pageRowCount );
                ASSERT_TRUE( back.storage.keyValueMetaData.GetValueAlways( "origin" ).AsString() == "test" );
                ASSERT_TRUE( back.backendOptions.GetValueAlways( "footerCacheBytes" ).AsString() == "67108864" );

                const char* designForm =
                    "{ \"name\": \"bars_1d\", \"schemaVersion\": 1, \"codec\": \"parquet\","
                    "  \"columns\": [ { \"name\": \"mic\", \"type\": \"utf8\", \"nullable\": false },"
                    "                 { \"name\": \"symbol\", \"type\": \"utf8\", \"nullable\": false },"
                    "                 { \"name\": \"date\", \"type\": \"date\", \"nullable\": false },"
                    "                 { \"name\": \"open\", \"type\": \"decimal(18,6)\" },"
                    "                 { \"name\": \"high\", \"type\": \"decimal(18,6)\" },"
                    "                 { \"name\": \"low\", \"type\": \"decimal(18,6)\" },"
                    "                 { \"name\": \"close\", \"type\": \"decimal(18,6)\" },"
                    "                 { \"name\": \"volume\", \"type\": \"int64\" } ],"
                    "  \"primaryKey\": [ \"mic\", \"symbol\", \"date\" ],"
                    "  \"sortKey\": [ \"date\" ],"
                    "  \"partitioning\": { \"fields\": [ { \"name\": \"mic\", \"source\": \"mic\", \"transform\": \"identity\" },"
                    "                                    { \"name\": \"symbol\", \"source\": \"symbol\", \"transform\": \"identity\" },"
                    "                                    { \"name\": \"year\", \"source\": \"date\", \"transform\": \"year\" } ],"
                    "                      \"pathTemplate\": \"{mic}/{symbol}/bars_1d/{year}.parquet\" },"
                    "  \"storage\": { \"compression\": \"none\", \"rowGroupRowCount\": 1048576, \"pageRowCount\": 16384 } }";
                CDynamicBuffer input;
                input.CopyFrom( (UInt32) strlen( designForm ), designForm );
                CDynamicBufferAccess inputAccess( input );
                CDataNode designTree;
                ASSERT_TRUE( codec->BuildDataTree( &designTree, &inputAccess ) );
                CDataTableDefinition fromDesign;
                ASSERT_TRUE( fromDesign.Deserialize( designTree, CDataNodeSerializableSettings() ) );
                ASSERT_TRUE( fromDesign.Validate( error ) );
                ASSERT_TRUE( 8 == fromDesign.schema.GetColumnCount() && 3 == fromDesign.schema.GetPrimaryKey().size() );
                ASSERT_FALSE( fromDesign.schema.GetColumn( 0 ).nullable );
                ASSERT_TRUE( fromDesign.schema.GetColumn( 3 ).decimalScale == 6 && fromDesign.schema.GetColumn( 7 ).nullable );
                ASSERT_TRUE( fromDesign.partitioning == definition.partitioning );
                ASSERT_TRUE( 16384 == fromDesign.storage.pageRowCount && 512ULL * 1024 * 1024 == fromDesign.storage.rowGroupByteLimit );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
