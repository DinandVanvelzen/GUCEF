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

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H
#include "dbpluginPOSTGRESQL_CPostgreSqlSql.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLVALUES_H
#include "dbpluginPOSTGRESQL_CPostgreSqlValues.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLVALUES_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLVALUES_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H
#include "dbpluginPOSTGRESQL_CPostgreSqlSettings.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H
#include "dbpluginPOSTGRESQL_CPostgreSqlConnection.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPostgreSqlUnit.h"

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Compares, logging both texts when they differ */
bool
SameText( const CString& actual, const CString& expected )
{
    if ( actual == expected )
        return true;
    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PostgreSqlUnit: expected [" << expected << "] but got [" << actual << "]" );
    return false;
}

/*-------------------------------------------------------------------------*/

/** An asset table: id key, a nullable name, an unsigned size */
CDataTableDefinition
MakeAssetsDefinition( void )
{
    CDataTableDefinition definition;
    definition.name = "assets";
    definition.schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "name", DATATABLE_TYPE_UTF8, true ) );
    definition.schema.AddColumn( CDataTableColumnDef( "size", DATATABLE_TYPE_UINT32, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "f", DATATABLE_TYPE_FLOAT64, true ) );
    CString::StringVector key;
    key.push_back( "id" );
    definition.schema.SetPrimaryKey( key );
    CString::StringVector sortKey;
    sortKey.push_back( "id" );
    definition.schema.SetSortKey( sortKey );
    return definition;
}

/*-------------------------------------------------------------------------*/

bool
ResolveQuery( const CDataTableDefinition& definition, const CDataTableQuery& query, CDataTableResolvedQuery& resolved )
{
    CString error;
    if ( query.Resolve( definition.schema, resolved, error ) )
        return true;
    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "PostgreSqlUnit: the query does not resolve: " << error );
    return false;
}

/*-------------------------------------------------------------------------*/

/** The WHERE text of the query on the assets table, with the parameters */
CString
WhereOf( const CDataTableQuery& query, CPostgreSqlParameters& parameters )
{
    const CDataTableDefinition definition = MakeAssetsDefinition();
    CDataTableResolvedQuery resolved;
    if ( !ResolveQuery( definition, query, resolved ) )
        return "UNRESOLVED";
    CString where;
    CString error;
    if ( DATATABLE_STATUS_OK != CPostgreSqlSql::MakeWhere( resolved, definition.schema, parameters, where, error ) )
        return "FAILED: " + error;
    return where;
}

/*-------------------------------------------------------------------------*/

CString::StringVector
Names( const char* a, const char* b = GUCEF_NULL )
{
    CString::StringVector names;
    names.push_back( a );
    if ( GUCEF_NULL != b )
        names.push_back( b );
    return names;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPostgreSqlUnitTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "PostgreSqlUnit" );

    GUCEF_TESTFW_TESTCASE( "Unit: every data table type maps to a PostgreSQL type, ranges the type lacks become CHECK constraints" )
        try
        {
            CString type;
            ASSERT_TRUE( CPostgreSqlSql::GetColumnType( CDataTableColumnDef( "c", DATATABLE_TYPE_BOOL ), type ) && SameText( type, "boolean" ) );
            ASSERT_TRUE( CPostgreSqlSql::GetColumnType( CDataTableColumnDef( "c", DATATABLE_TYPE_INT8 ), type ) && SameText( type, "smallint" ) );
            ASSERT_TRUE( CPostgreSqlSql::GetColumnType( CDataTableColumnDef( "c", DATATABLE_TYPE_INT16 ), type ) && SameText( type, "smallint" ) );
            ASSERT_TRUE( CPostgreSqlSql::GetColumnType( CDataTableColumnDef( "c", DATATABLE_TYPE_INT32 ), type ) && SameText( type, "integer" ) );
            ASSERT_TRUE( CPostgreSqlSql::GetColumnType( CDataTableColumnDef( "c", DATATABLE_TYPE_INT64 ), type ) && SameText( type, "bigint" ) );
            ASSERT_TRUE( CPostgreSqlSql::GetColumnType( CDataTableColumnDef( "c", DATATABLE_TYPE_UINT8 ), type ) && SameText( type, "smallint" ) );
            ASSERT_TRUE( CPostgreSqlSql::GetColumnType( CDataTableColumnDef( "c", DATATABLE_TYPE_UINT16 ), type ) && SameText( type, "integer" ) );
            ASSERT_TRUE( CPostgreSqlSql::GetColumnType( CDataTableColumnDef( "c", DATATABLE_TYPE_UINT32 ), type ) && SameText( type, "bigint" ) );
            ASSERT_TRUE( CPostgreSqlSql::GetColumnType( CDataTableColumnDef( "c", DATATABLE_TYPE_UINT64 ), type ) && SameText( type, "numeric(20,0)" ) );
            ASSERT_TRUE( CPostgreSqlSql::GetColumnType( CDataTableColumnDef( "c", DATATABLE_TYPE_FLOAT32 ), type ) && SameText( type, "real" ) );
            ASSERT_TRUE( CPostgreSqlSql::GetColumnType( CDataTableColumnDef( "c", DATATABLE_TYPE_FLOAT64 ), type ) && SameText( type, "double precision" ) );
            ASSERT_TRUE( CPostgreSqlSql::GetColumnType( CDataTableColumnDef::Decimal( "c", 10, 3 ), type ) && SameText( type, "numeric(10,3)" ) );
            ASSERT_TRUE( CPostgreSqlSql::GetColumnType( CDataTableColumnDef( "c", DATATABLE_TYPE_DATE ), type ) && SameText( type, "date" ) );
            ASSERT_TRUE( CPostgreSqlSql::GetColumnType( CDataTableColumnDef::Timestamp( "c", DATATABLE_TIMEUNIT_MICROSECONDS, true ), type ) && SameText( type, "timestamp(6) with time zone" ) );
            ASSERT_TRUE( CPostgreSqlSql::GetColumnType( CDataTableColumnDef::Timestamp( "c", DATATABLE_TIMEUNIT_MILLISECONDS, false ), type ) && SameText( type, "timestamp(3) without time zone" ) );
            ASSERT_TRUE( CPostgreSqlSql::GetColumnType( CDataTableColumnDef::Timestamp( "c", DATATABLE_TIMEUNIT_NANOSECONDS, true ), type ) && SameText( type, "bigint" ) );
            ASSERT_TRUE( CPostgreSqlSql::GetColumnType( CDataTableColumnDef( "c", DATATABLE_TYPE_UTF8 ), type ) && SameText( type, "text COLLATE \"C\"" ) );
            ASSERT_TRUE( CPostgreSqlSql::GetColumnType( CDataTableColumnDef( "c", DATATABLE_TYPE_BINARY ), type ) && SameText( type, "bytea" ) );
            ASSERT_FALSE( CPostgreSqlSql::GetColumnType( CDataTableColumnDef( "c", DATATABLE_TYPE_UNSUPPORTED ), type ) );

            ASSERT_TRUE( SameText( CPostgreSqlSql::GetColumnCheck( CDataTableColumnDef( "c", DATATABLE_TYPE_INT8 ) ), "CHECK (\"c\" BETWEEN -128 AND 127)" ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::GetColumnCheck( CDataTableColumnDef( "c", DATATABLE_TYPE_UINT8 ) ), "CHECK (\"c\" BETWEEN 0 AND 255)" ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::GetColumnCheck( CDataTableColumnDef( "c", DATATABLE_TYPE_UINT64 ) ), "CHECK (\"c\" BETWEEN 0 AND 18446744073709551615)" ) );
            ASSERT_TRUE( CPostgreSqlSql::GetColumnCheck( CDataTableColumnDef( "c", DATATABLE_TYPE_INT64 ) ).IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Unit: identifiers are quoted with quotes doubled, longer than 63 bytes they are refused" )
        try
        {
            ASSERT_TRUE( SameText( CPostgreSqlSql::QuoteIdentifier( "plain" ), "\"plain\"" ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::QuoteIdentifier( "a\"b\"\"c" ), "\"a\"\"b\"\"\"\"c\"" ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::QuoteIdentifier( "x\"; DROP TABLE y; --" ), "\"x\"\"; DROP TABLE y; --\"" ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::QualifiedName( "my schema", "t.1-a" ), "\"my schema\".\"t.1-a\"" ) );
            const CString longest( "a23456789012345678901234567890123456789012345678901234567890123" );
            ASSERT_TRUE( 63 == longest.Length() && CPostgreSqlSql::IsUsableIdentifier( longest ) );
            ASSERT_FALSE( CPostgreSqlSql::IsUsableIdentifier( longest + "4" ) );
            ASSERT_FALSE( CPostgreSqlSql::IsUsableIdentifier( CString::Empty ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Unit: CREATE TABLE and CREATE INDEX of a definition, index names cut to 63 bytes with a SHA-1 suffix" )
        try
        {
            CDataTableDefinition definition = MakeAssetsDefinition();
            CString sql;
            CString error;
            ASSERT_TRUE( CPostgreSqlSql::MakeCreateTable( definition, "s", sql, error ) );
            ASSERT_TRUE( SameText( sql, "CREATE TABLE IF NOT EXISTS \"s\".\"assets\" (\"id\" bigint NOT NULL, \"name\" text COLLATE \"C\", "
                                        "\"size\" bigint NOT NULL CHECK (\"size\" BETWEEN 0 AND 4294967295), \"f\" double precision, PRIMARY KEY (\"id\"))" ) );

            CDataTableIndexDef index( "by_name", Names( "name", "size" ), true );
            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeCreateIndex( definition, index, "s" ),
                                   "CREATE UNIQUE INDEX IF NOT EXISTS \"assets_by_name\" ON \"s\".\"assets\" (\"name\", \"size\")" ) );
            index.unique = false;
            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeCreateIndex( definition, index, "s" ),
                                   "CREATE INDEX IF NOT EXISTS \"assets_by_name\" ON \"s\".\"assets\" (\"name\", \"size\")" ) );

            const CString table( "t234567890123456789012345678901234567890" );
            const CString cut1 = CPostgreSqlSql::MakeIndexName( table, "i234567890123456789012345678901234567890" );
            const CString cut2 = CPostgreSqlSql::MakeIndexName( table, "i234567890123456789012345678901234567891" );
            ASSERT_TRUE( 63 == cut1.Length() && 63 == cut2.Length() && cut1 != cut2 );
            ASSERT_TRUE( cut1 == CPostgreSqlSql::MakeIndexName( table, "i234567890123456789012345678901234567890" ) );
            ASSERT_TRUE( '_' == cut1[ 54 ] && cut1.SubstrToIndex( 54, true ) == ( table + "_i234567890123" ) );

            // a column type PostgreSQL cannot hold and a name that is too long are refused
            CDataTableDefinition unsupported = definition;
            unsupported.schema.AddColumn( CDataTableColumnDef( "u", DATATABLE_TYPE_UNSUPPORTED ) );
            ASSERT_FALSE( CPostgreSqlSql::MakeCreateTable( unsupported, "s", sql, error ) );
            CDataTableDefinition longName = definition;
            longName.name = "a234567890123456789012345678901234567890123456789012345678901234";
            ASSERT_FALSE( CPostgreSqlSql::MakeCreateTable( longName, "s", sql, error ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Unit: parameters are cast or converted to the column type, reads give the native form" )
        try
        {
            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeParameterExpression( CDataTableColumnDef( "c", DATATABLE_TYPE_INT32 ), 1 ), "$1::integer" ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeParameterExpression( CDataTableColumnDef( "c", DATATABLE_TYPE_UINT64 ), 2 ), "$2::numeric" ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeParameterExpression( CDataTableColumnDef( "c", DATATABLE_TYPE_DATE ), 3 ), "(DATE '1970-01-01' + $3::integer)" ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeParameterExpression( CDataTableColumnDef::Timestamp( "c", DATATABLE_TIMEUNIT_MICROSECONDS, true ), 4 ),
                                   "(TIMESTAMPTZ '1970-01-01 00:00:00+00' + ($4::bigint / 1000000) * INTERVAL '1 second' + ($4::bigint % 1000000) * INTERVAL '1 microsecond')" ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeParameterExpression( CDataTableColumnDef::Timestamp( "c", DATATABLE_TIMEUNIT_MILLISECONDS, false ), 5 ),
                                   "(TIMESTAMP '1970-01-01 00:00:00' + ($5::bigint / 1000) * INTERVAL '1 second' + ($5::bigint % 1000) * INTERVAL '1 millisecond')" ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeParameterExpression( CDataTableColumnDef::Timestamp( "c", DATATABLE_TIMEUNIT_NANOSECONDS, true ), 6 ), "$6::bigint" ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeParameterExpression( CDataTableColumnDef( "c", DATATABLE_TYPE_BINARY ), 7 ), "$7::bytea" ) );

            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeReadExpression( CDataTableColumnDef::Decimal( "dec", 18, 4 ) ), "(\"dec\" * 10000::numeric)::bigint" ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeReadExpression( CDataTableColumnDef::Decimal( "dec", 9, 0 ) ), "\"dec\"" ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeReadExpression( CDataTableColumnDef( "d", DATATABLE_TYPE_DATE ) ), "(\"d\" - DATE '1970-01-01')" ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeReadExpression( CDataTableColumnDef::Timestamp( "t", DATATABLE_TIMEUNIT_MICROSECONDS, true ) ),
                                   "(EXTRACT(EPOCH FROM date_trunc('second', \"t\"))::bigint * 1000000 + EXTRACT(MICROSECONDS FROM \"t\")::bigint % 1000000)" ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeReadExpression( CDataTableColumnDef::Timestamp( "t", DATATABLE_TIMEUNIT_MILLISECONDS, true ) ),
                                   "((EXTRACT(EPOCH FROM date_trunc('second', \"t\"))::bigint * 1000000 + EXTRACT(MICROSECONDS FROM \"t\")::bigint % 1000000) / 1000)" ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeReadExpression( CDataTableColumnDef( "b", DATATABLE_TYPE_BINARY ) ), "encode(\"b\", 'hex')" ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeReadExpression( CDataTableColumnDef( "s", DATATABLE_TYPE_UTF8 ) ), "\"s\"" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Unit: predicates become a parameterised WHERE clause, values never become part of the statement" )
        try
        {
            const CString hostile( "O'Brien\"; DROP TABLE assets; --" );
            CDataTableQuery query;
            query.WhereEquals( "name", CVariant( hostile ) );
            query.WhereRange( "size", CVariant( (Int64) 10 ), true, CVariant( (Int64) 20 ), false );
            CVariant::VariantVector set;
            set.push_back( CVariant( (Int64) 3 ) );
            set.push_back( CVariant( (Int64) 1 ) );
            set.push_back( CVariant( (Int64) 2 ) );
            set.push_back( CVariant( (Int64) 3 ) );
            query.WhereIn( "id", set );
            CPostgreSqlParameters parameters;
            const CString where = WhereOf( query, parameters );
            ASSERT_TRUE( SameText( where, "\"name\" COLLATE \"C\" = $1::text AND \"size\" >= $2::bigint AND \"size\" < $3::bigint AND \"id\" IN ($4::bigint, $5::bigint, $6::bigint)" ) );
            ASSERT_TRUE( 6 == parameters.GetCount() );
            ASSERT_TRUE( SameText( parameters.GetDisplayText( 1 ), hostile ) );
            ASSERT_TRUE( SameText( parameters.GetDisplayText( 2 ), "10" ) && SameText( parameters.GetDisplayText( 3 ), "20" ) );
            ASSERT_TRUE( SameText( parameters.GetDisplayText( 4 ), "1" ) && SameText( parameters.GetDisplayText( 5 ), "2" ) && SameText( parameters.GetDisplayText( 6 ), "3" ) );
            ASSERT_TRUE( where.HasSubstr( "Brien" ) < 0 && where.HasSubstr( "DROP" ) < 0 );

            // the nulls and the comparisons of a float column, which keep NaN cells out
            CDataTableQuery nulls;
            nulls.WhereNull( "name", true ).WhereNull( "f", false );
            parameters.Clear();
            ASSERT_TRUE( SameText( WhereOf( nulls, parameters ), "\"name\" IS NULL AND \"f\" IS NOT NULL" ) );
            CDataTableQuery floats;
            floats.Where( CDataTablePredicate( "f", DATATABLE_PREDICATE_GREATER, CVariant( (Float64) 1.5 ) ) );
            parameters.Clear();
            ASSERT_TRUE( SameText( WhereOf( floats, parameters ), "(\"f\" > $1::double precision AND \"f\" <> 'NaN'::double precision)" ) );
            ASSERT_TRUE( SameText( parameters.GetDisplayText( 1 ), "1.5" ) );
            CDataTableQuery notEqual;
            notEqual.Where( CDataTablePredicate( "f", DATATABLE_PREDICATE_NOT_EQUALS, CVariant( (Float64) -2.25 ) ) );
            parameters.Clear();
            ASSERT_TRUE( SameText( WhereOf( notEqual, parameters ), "(\"f\" <> $1::double precision AND \"f\" <> 'NaN'::double precision)" ) );

            // a comparison with NaN, a set of NaN only and a literal the column cannot hold match nothing
            CDataTableQuery nan;
            nan.WhereEquals( "f", CVariant( std::numeric_limits< Float64 >::quiet_NaN() ) );
            parameters.Clear();
            ASSERT_TRUE( SameText( WhereOf( nan, parameters ), "FALSE" ) );
            CVariant::VariantVector onlyNaN;
            onlyNaN.push_back( CVariant( std::numeric_limits< Float64 >::quiet_NaN() ) );
            CDataTableQuery nanSet;
            nanSet.WhereIn( "f", onlyNaN );
            parameters.Clear();
            ASSERT_TRUE( SameText( WhereOf( nanSet, parameters ), "FALSE" ) && 0 == parameters.GetCount() );
            CDataTableQuery negative;
            negative.WhereEquals( "size", CVariant( (Int64) -1 ) );
            parameters.Clear();
            ASSERT_TRUE( SameText( WhereOf( negative, parameters ), "FALSE" ) && 0 == parameters.GetCount() );
            CDataTableQuery none;
            parameters.Clear();
            ASSERT_TRUE( WhereOf( none, parameters ).IsNULLOrEmpty() );

            // text with a NUL byte cannot be sent
            CDataTableQuery nul;
            nul.WhereEquals( "name", CVariant( CString( "a\0b", 3 ) ) );
            const CDataTableDefinition definition = MakeAssetsDefinition();
            CDataTableResolvedQuery resolved;
            ASSERT_TRUE( ResolveQuery( definition, nul, resolved ) );
            CString text;
            CString error;
            parameters.Clear();
            ASSERT_TRUE( DATATABLE_STATUS_UNSUPPORTED == CPostgreSqlSql::MakeWhere( resolved, definition.schema, parameters, text, error ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Unit: SELECT, count, DELETE, INSERT and the upsert end of a table" )
        try
        {
            const CDataTableDefinition definition = MakeAssetsDefinition();
            CDataTableQuery query;
            query.WhereEquals( "size", CVariant( (Int64) 7 ) ).SetLimit( 25 );
            CDataTableResolvedQuery resolved;
            ASSERT_TRUE( ResolveQuery( definition, query, resolved ) );
            CDataTableSchema::TColumnIndexVector columns;
            columns.push_back( 1 );
            columns.push_back( 0 );
            CPostgreSqlParameters parameters;
            CString sql;
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_OK == CPostgreSqlSql::MakeSelect( definition, "s", columns, resolved, parameters, sql, error ) );
            ASSERT_TRUE( SameText( sql, "SELECT \"name\", \"id\" FROM \"s\".\"assets\" WHERE \"size\" = $1::bigint ORDER BY \"id\" ASC NULLS LAST LIMIT $2::bigint" ) );
            ASSERT_TRUE( 2 == parameters.GetCount() && SameText( parameters.GetDisplayText( 2 ), "25" ) );

            parameters.Clear();
            ASSERT_TRUE( DATATABLE_STATUS_OK == CPostgreSqlSql::MakeCount( definition, "s", resolved, parameters, sql, error ) );
            ASSERT_TRUE( SameText( sql, "SELECT count(*) FROM \"s\".\"assets\" WHERE \"size\" = $1::bigint" ) );
            parameters.Clear();
            ASSERT_TRUE( DATATABLE_STATUS_OK == CPostgreSqlSql::MakeDelete( definition, "s", resolved, parameters, sql, error ) );
            ASSERT_TRUE( SameText( sql, "DELETE FROM \"s\".\"assets\" WHERE \"size\" = $1::bigint" ) );

            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeInsertPrefix( definition, "s" ), "INSERT INTO \"s\".\"assets\" (\"id\", \"name\", \"size\", \"f\") VALUES " ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeInsertRow( definition, 5 ), "($5::bigint, $6::text, $7::bigint, $8::double precision)" ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeUpsertSuffix( definition ),
                                   " ON CONFLICT (\"id\") DO UPDATE SET \"name\" = EXCLUDED.\"name\", \"size\" = EXCLUDED.\"size\", \"f\" = EXCLUDED.\"f\" RETURNING (xmax::text <> '0')" ) );

            CDataTableDefinition keyOnly;
            keyOnly.name = "tags";
            keyOnly.schema.AddColumn( CDataTableColumnDef( "tag", DATATABLE_TYPE_UTF8, false ) );
            keyOnly.schema.SetPrimaryKey( Names( "tag" ) );
            ASSERT_TRUE( SameText( CPostgreSqlSql::MakeUpsertSuffix( keyOnly ), " ON CONFLICT (\"tag\") DO NOTHING RETURNING (xmax::text <> '0')" ) );
            CDataTableDefinition noKey = keyOnly;
            noKey.schema.SetPrimaryKey( CString::StringVector() );
            ASSERT_TRUE( CPostgreSqlSql::MakeUpsertSuffix( noKey ).IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Unit: the text forms of values: decimals, floats, whole numbers and hexadecimal bytes" )
        try
        {
            ASSERT_TRUE( SameText( CPostgreSqlValues::MantissaToDecimalText( 12345, 2 ), "123.45" ) );
            ASSERT_TRUE( SameText( CPostgreSqlValues::MantissaToDecimalText( -5, 3 ), "-0.005" ) );
            ASSERT_TRUE( SameText( CPostgreSqlValues::MantissaToDecimalText( 0, 0 ), "0" ) );
            ASSERT_TRUE( SameText( CPostgreSqlValues::MantissaToDecimalText( 0, 2 ), "0.00" ) );
            ASSERT_TRUE( SameText( CPostgreSqlValues::MantissaToDecimalText( 7, 0 ), "7" ) );
            ASSERT_TRUE( SameText( CPostgreSqlValues::MantissaToDecimalText( -123, 1 ), "-12.3" ) );
            ASSERT_TRUE( SameText( CPostgreSqlValues::MantissaToDecimalText( GUCEF_INT64MIN, 18 ), "-9.223372036854775808" ) );
            ASSERT_TRUE( SameText( CPostgreSqlValues::MantissaToDecimalText( GUCEF_INT64MAX, 0 ), "9223372036854775807" ) );

            ASSERT_TRUE( SameText( CPostgreSqlValues::Float64ToText( std::numeric_limits< Float64 >::quiet_NaN() ), "NaN" ) );
            ASSERT_TRUE( SameText( CPostgreSqlValues::Float64ToText( std::numeric_limits< Float64 >::infinity() ), "Infinity" ) );
            ASSERT_TRUE( SameText( CPostgreSqlValues::Float64ToText( -std::numeric_limits< Float64 >::infinity() ), "-Infinity" ) );
            ASSERT_TRUE( SameText( CPostgreSqlValues::Float64ToText( 0.1 ), "0.1" ) );
            Float64 parsed = 0.0;
            ASSERT_TRUE( CPostgreSqlValues::ParseFloat64( "0.1", 3, parsed ) && 0.1 == parsed );
            ASSERT_TRUE( CPostgreSqlValues::ParseFloat64( "NaN", 3, parsed ) && parsed != parsed );
            ASSERT_TRUE( CPostgreSqlValues::ParseFloat64( "-Infinity", 9, parsed ) && parsed < 0 && parsed * 0.5 == parsed );
            ASSERT_TRUE( CPostgreSqlValues::ParseFloat64( "1e-300", 6, parsed ) && 1e-300 == parsed );
            ASSERT_FALSE( CPostgreSqlValues::ParseFloat64( "inf", 3, parsed ) );

            Int64 signedValue = 0;
            ASSERT_TRUE( CPostgreSqlValues::ParseInt64( "-9223372036854775808", 20, signedValue ) && GUCEF_INT64MIN == signedValue );
            ASSERT_TRUE( CPostgreSqlValues::ParseInt64( "9223372036854775807", 19, signedValue ) && GUCEF_INT64MAX == signedValue );
            ASSERT_FALSE( CPostgreSqlValues::ParseInt64( "9223372036854775808", 19, signedValue ) );
            ASSERT_FALSE( CPostgreSqlValues::ParseInt64( "-", 1, signedValue ) || CPostgreSqlValues::ParseInt64( "1a", 2, signedValue ) || CPostgreSqlValues::ParseInt64( "", 0, signedValue ) );
            UInt64 unsignedValue = 0;
            ASSERT_TRUE( CPostgreSqlValues::ParseUInt64( "18446744073709551615", 20, unsignedValue ) && GUCEF_UINT64MAX == unsignedValue );
            ASSERT_FALSE( CPostgreSqlValues::ParseUInt64( "18446744073709551616", 20, unsignedValue ) || CPostgreSqlValues::ParseUInt64( "-1", 2, unsignedValue ) );

            GUCEF::vector< UInt8 > bytes;
            ASSERT_TRUE( CPostgreSqlValues::DecodeHex( "00ff7F", 6, bytes ) && 3 == bytes.size() && 0 == bytes[ 0 ] && 255 == bytes[ 1 ] && 127 == bytes[ 2 ] );
            ASSERT_TRUE( CPostgreSqlValues::DecodeHex( "", 0, bytes ) && bytes.empty() );
            ASSERT_FALSE( CPostgreSqlValues::DecodeHex( "abc", 3, bytes ) || CPostgreSqlValues::DecodeHex( "zz", 2, bytes ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Unit: cells become parameters and texts of the server become cells, for every type" )
        try
        {
            CDataTableSchema schema;
            schema.AddColumn( CDataTableColumnDef( "b", DATATABLE_TYPE_BOOL ) );
            schema.AddColumn( CDataTableColumnDef( "i8", DATATABLE_TYPE_INT8 ) );
            schema.AddColumn( CDataTableColumnDef( "u64", DATATABLE_TYPE_UINT64 ) );
            schema.AddColumn( CDataTableColumnDef( "f32", DATATABLE_TYPE_FLOAT32 ) );
            schema.AddColumn( CDataTableColumnDef::Decimal( "dec", 18, 4 ) );
            schema.AddColumn( CDataTableColumnDef( "d", DATATABLE_TYPE_DATE ) );
            schema.AddColumn( CDataTableColumnDef::Timestamp( "ts", DATATABLE_TIMEUNIT_MICROSECONDS, true ) );
            schema.AddColumn( CDataTableColumnDef( "s", DATATABLE_TYPE_UTF8 ) );
            schema.AddColumn( CDataTableColumnDef( "bin", DATATABLE_TYPE_BINARY ) );
            const CDataTableSchemaPtr schemaPtr = CreateDataTableSchemaPtr( schema );

            // the server's texts into a batch
            CDataTableBatchBuilder builder( schemaPtr, CCoreGlobal::Instance()->GetDataTableBufferPool() );
            const char* texts[] = { "t", "-128", "18446744073709551615", "0.1", "-123456", "-1", "-1500000", "\xC3\xA9t\xC3\xA9", "00ff10" };
            for ( UInt32 c=0; c<schema.GetColumnCount(); ++c )
                ASSERT_TRUE( DATATABLE_STATUS_OK == CPostgreSqlValues::AppendResultValue( schema.GetColumn( c ), texts[ c ], (UInt32) strlen( texts[ c ] ), false, builder.GetColumn( c ) ) );
            for ( UInt32 c=0; c<schema.GetColumnCount(); ++c )
                ASSERT_TRUE( DATATABLE_STATUS_OK == CPostgreSqlValues::AppendResultValue( schema.GetColumn( c ), "", 0, true, builder.GetColumn( c ) ) );
            CDataTableBatchPtr batch;
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( batch, error ) && 2 == batch->GetRowCount() );
            ASSERT_TRUE( 0 != ( batch->GetColumn( 0 ).GetBoolBits()[ 0 ] & 1 ) );
            ASSERT_TRUE( -128 == batch->GetColumn( 1 ).GetInt8Data()[ 0 ] );
            ASSERT_TRUE( GUCEF_UINT64MAX == batch->GetColumn( 2 ).GetUInt64Data()[ 0 ] );
            ASSERT_TRUE( 0.1f == batch->GetColumn( 3 ).GetFloat32Data()[ 0 ] );
            ASSERT_TRUE( -123456 == batch->GetColumn( 4 ).GetInt64Data()[ 0 ] );
            ASSERT_TRUE( -1 == batch->GetColumn( 5 ).GetInt32Data()[ 0 ] );
            ASSERT_TRUE( -1500000 == batch->GetColumn( 6 ).GetInt64Data()[ 0 ] );
            UInt32 length = 0;
            const char* text = batch->GetColumn( 7 ).GetVariableValue( 0, length );
            ASSERT_TRUE( 5 == length && 0 == memcmp( text, "\xC3\xA9t\xC3\xA9", 5 ) );
            text = batch->GetColumn( 8 ).GetVariableValue( 0, length );
            ASSERT_TRUE( 3 == length && 0 == (UInt8) text[ 0 ] && 0xFF == (UInt8) text[ 1 ] && 0x10 == (UInt8) text[ 2 ] );
            for ( UInt32 c=0; c<schema.GetColumnCount(); ++c )
                ASSERT_FALSE( batch->GetColumn( c ).IsValid( 1 ) );

            // and back out as parameters
            CPostgreSqlParameters parameters;
            for ( UInt32 c=0; c<schema.GetColumnCount(); ++c )
                ASSERT_TRUE( 0 != CPostgreSqlValues::AddCellParameter( schema.GetColumn( c ), batch->GetColumn( c ), 0, parameters ) );
            ASSERT_TRUE( SameText( parameters.GetDisplayText( 1 ), "true" ) );
            ASSERT_TRUE( SameText( parameters.GetDisplayText( 2 ), "-128" ) );
            ASSERT_TRUE( SameText( parameters.GetDisplayText( 3 ), "18446744073709551615" ) );
            ASSERT_TRUE( SameText( parameters.GetDisplayText( 5 ), "-12.3456" ) );
            ASSERT_TRUE( SameText( parameters.GetDisplayText( 6 ), "-1" ) );
            ASSERT_TRUE( SameText( parameters.GetDisplayText( 7 ), "-1500000" ) );
            ASSERT_TRUE( parameters.IsBinary( 9 ) && SameText( parameters.GetDisplayText( 9 ), "00ff10" ) );
            Float64 back = 0.0;
            const CString f32Text = parameters.GetDisplayText( 4 );
            ASSERT_TRUE( CPostgreSqlValues::ParseFloat64( f32Text.C_String(), f32Text.Length(), back ) && 0.1f == (Float32) back );
            for ( UInt32 c=0; c<schema.GetColumnCount(); ++c )
                ASSERT_TRUE( 0 != CPostgreSqlValues::AddCellParameter( schema.GetColumn( c ), batch->GetColumn( c ), 1, parameters ) );
            ASSERT_TRUE( parameters.IsNull( 10 ) && parameters.IsNull( 18 ) );

            // texts that do not fit the column
            CDataTableBatchBuilder failing( schemaPtr, CCoreGlobal::Instance()->GetDataTableBufferPool() );
            ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == CPostgreSqlValues::AppendResultValue( schema.GetColumn( 0 ), "x", 1, false, failing.GetColumn( 0 ) ) );
            ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == CPostgreSqlValues::AppendResultValue( schema.GetColumn( 1 ), "300", 3, false, failing.GetColumn( 1 ) ) );
            ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == CPostgreSqlValues::AppendResultValue( schema.GetColumn( 5 ), "9999999999", 10, false, failing.GetColumn( 5 ) ) );
            ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == CPostgreSqlValues::AppendResultValue( schema.GetColumn( 8 ), "0g", 2, false, failing.GetColumn( 8 ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Unit: parameters keep text, binary and nulls apart, refuse NUL bytes in text and stop at 65535" )
        try
        {
            CPostgreSqlParameters parameters;
            ASSERT_TRUE( 1 == parameters.AddText( "abc" ) );
            ASSERT_TRUE( 2 == parameters.AddBinary( "\0\1", 2 ) );
            ASSERT_TRUE( 3 == parameters.AddNull() );
            ASSERT_TRUE( 0 == parameters.AddText( "a\0b", 3 ) );
            ASSERT_TRUE( 3 == parameters.GetCount() );
            ASSERT_TRUE( 0 == strcmp( parameters.GetValues()[ 0 ], "abc" ) && 3 == parameters.GetLengths()[ 0 ] && 0 == parameters.GetFormats()[ 0 ] );
            ASSERT_TRUE( 0 == parameters.GetValues()[ 1 ][ 0 ] && 1 == parameters.GetValues()[ 1 ][ 1 ] && 2 == parameters.GetLengths()[ 1 ] && 1 == parameters.GetFormats()[ 1 ] );
            ASSERT_TRUE( GUCEF_NULL == parameters.GetValues()[ 2 ] && parameters.IsNull( 3 ) );
            parameters.Clear();
            UInt32 last = 0;
            for ( UInt32 i=0; i<CPostgreSqlParameters::MaxCount; ++i )
                last = parameters.AddNull();
            ASSERT_TRUE( 65535 == last && 0 == parameters.AddText( "x" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Unit: settings are read from the backend options, refused when they cannot be used, the password is never described" )
        try
        {
            COMCORE::CDatabaseClientSettings settings;
            settings.backendName = "postgresql";
            settings.database = "assets";
            settings.backendOptions.Set( "host", CVariant( "db.example" ) );
            settings.backendOptions.Set( "port", CVariant( "6543" ) );
            settings.backendOptions.Set( "user", CVariant( "reader" ) );
            settings.backendOptions.Set( "password", CVariant( "s3cret-value" ) );
            settings.backendOptions.Set( "sslMode", CVariant( "verify-full" ) );
            settings.backendOptions.Set( "schema", CVariant( "catalogue" ) );
            settings.backendOptions.Set( "connectTimeout", CVariant( "7" ) );
            settings.backendOptions.Set( "maxRowsPerStatement", CVariant( "250" ) );
            settings.backendOptions.Set( "connectionString", CVariant( "host=other password=secretinconn" ) );
            CPostgreSqlSettings loaded;
            CString error;
            ASSERT_TRUE( loaded.LoadFrom( settings, error ) );
            ASSERT_TRUE( loaded.host == "db.example" && 6543 == loaded.port && loaded.user == "reader" && loaded.password == "s3cret-value" );
            ASSERT_TRUE( loaded.sslMode == "verify-full" && loaded.schema == "catalogue" && 7 == loaded.connectTimeoutInSecs && 250 == loaded.maxRowsPerStatement );
            ASSERT_TRUE( loaded.applicationName == CPostgreSqlSettings::DefaultApplicationName && !loaded.createSchema && !loaded.readOnly );

            const CString description = loaded.Describe();
            ASSERT_TRUE( description.HasSubstr( "s3cret-value" ) < 0 && description.HasSubstr( "secretinconn" ) < 0 );
            ASSERT_TRUE( description.HasSubstr( "db.example" ) >= 0 && description.HasSubstr( "assets" ) >= 0 );

            // the connection string first, expanded by libpq, the options after it override it
            CString::StringVector keywords;
            CString::StringVector values;
            loaded.GetConnectionParameters( keywords, values );
            ASSERT_TRUE( keywords.size() == values.size() && keywords.size() >= 8 );
            ASSERT_TRUE( keywords[ 0 ] == "dbname" && values[ 0 ] == "host=other password=secretinconn" );
            ASSERT_TRUE( keywords[ 1 ] == "host" && values[ 1 ] == "db.example" && keywords[ 2 ] == "port" && values[ 2 ] == "6543" );
            ASSERT_TRUE( keywords[ 3 ] == "dbname" && values[ 3 ] == "assets" );
            bool hasEncoding = false;
            for ( UInt32 i=0; i<(UInt32) keywords.size(); ++i )
                hasEncoding = hasEncoding || ( keywords[ i ] == "client_encoding" && values[ i ] == "UTF8" );
            ASSERT_TRUE( hasEncoding );

            // read only sessions
            settings.readOnly = true;
            ASSERT_TRUE( loaded.LoadFrom( settings, error ) && loaded.readOnly );
            loaded.GetConnectionParameters( keywords, values );
            ASSERT_TRUE( keywords.back() == "options" && values.back() == "-c default_transaction_read_only=on" );

            // values that cannot be used leave the settings as they were
            const char* badKeys[] = { "port", "port", "sslMode", "connectTimeout", "maxRowsPerStatement", "maxRowsPerStatement", "schema", "connectionString" };
            const char* badValues[] = { "0", "65536", "sometimes", "-1", "0", "10001", "s234567890123456789012345678901234567890123456789012345678901234", "host='unterminated" };
            for ( UInt32 i=0; i<sizeof( badKeys ) / sizeof( badKeys[ 0 ] ); ++i )
            {
                COMCORE::CDatabaseClientSettings bad = settings;
                bad.backendOptions.Delete( badKeys[ i ] );
                bad.backendOptions.Set( badKeys[ i ], CVariant( badValues[ i ] ) );
                CPostgreSqlSettings before = loaded;
                ASSERT_FALSE( loaded.LoadFrom( bad, error ) );
                ASSERT_TRUE( loaded.port == before.port && loaded.schema == before.schema );
                ASSERT_TRUE( error.HasSubstr( "s3cret-value" ) < 0 );
            }
            COMCORE::CDatabaseClientSettings noDatabase = settings;
            noDatabase.database.Clear();
            ASSERT_FALSE( loaded.LoadFrom( noDatabase, error ) );

            CValueList parsed;
            ASSERT_TRUE( CPostgreSqlSettings::ParseConnectionString( "host=h port=5 dbname=d user=u", parsed, error ) );
            ASSERT_TRUE( parsed.GetValueAlways( CString( "dbname" ) ).AsString() == "d" && parsed.GetValueAlways( CString( "port" ) ).AsString() == "5" );
            ASSERT_TRUE( CPostgreSqlSettings::ParseConnectionString( "postgresql://u@h:5/d2?sslmode=disable", parsed, error ) );
            ASSERT_TRUE( parsed.GetValueAlways( CString( "dbname" ) ).AsString() == "d2" && parsed.GetValueAlways( CString( "sslmode" ) ).AsString() == "disable" );
            ASSERT_FALSE( CPostgreSqlSettings::ParseConnectionString( "host='unterminated", parsed, error ) );
            ASSERT_TRUE( CPostgreSqlSettings::IsValidSslMode( "prefer" ) && !CPostgreSqlSettings::IsValidSslMode( "Prefer" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Unit: SQLSTATEs map to data table statuses" )
        try
        {
            ASSERT_TRUE( DATATABLE_STATUS_CONFLICT == CPostgreSqlConnection::StatusOfSqlState( "23505" ) );
            ASSERT_TRUE( DATATABLE_STATUS_CONFLICT == CPostgreSqlConnection::StatusOfSqlState( "40001" ) );
            ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == CPostgreSqlConnection::StatusOfSqlState( "23514" ) );
            ASSERT_TRUE( DATATABLE_STATUS_TYPE_MISMATCH == CPostgreSqlConnection::StatusOfSqlState( "22003" ) );
            ASSERT_TRUE( DATATABLE_STATUS_NOT_FOUND == CPostgreSqlConnection::StatusOfSqlState( "42P01" ) );
            ASSERT_TRUE( DATATABLE_STATUS_ALREADY_EXISTS == CPostgreSqlConnection::StatusOfSqlState( "42P07" ) );
            ASSERT_TRUE( DATATABLE_STATUS_READ_ONLY == CPostgreSqlConnection::StatusOfSqlState( "25006" ) );
            ASSERT_TRUE( DATATABLE_STATUS_IO_ERROR == CPostgreSqlConnection::StatusOfSqlState( "08006" ) );
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == CPostgreSqlConnection::StatusOfSqlState( "42601" ) );
            ASSERT_TRUE( DATATABLE_STATUS_FAILED == CPostgreSqlConnection::StatusOfSqlState( "XX000" ) );
            ASSERT_TRUE( DATATABLE_STATUS_FAILED == CPostgreSqlConnection::StatusOfSqlState( GUCEF_NULL ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Unit: the plugin registers \"postgresql\"; a client refuses bad settings and keeps the password out of the error of a failed connection" )
        try
        {
            COMCORE::CDatabaseClientFactory& factory = COMCORE::CComCoreGlobal::Instance()->GetDatabaseClientFactory();
            ASSERT_TRUE( factory.IsConstructible( "postgresql" ) );
            COMCORE::CIDatabaseClientPtr client = factory.Create( "postgresql" );
            ASSERT_FALSE( client.IsNULL() );
            ASSERT_TRUE( client->GetBackendName() == "postgresql" );
            const UInt32 capabilities = client->GetCapabilities();
            ASSERT_TRUE( 0 != ( capabilities & GUCEF_DATABASE_CAPABILITY_TRANSACTIONS ) && 0 != ( capabilities & GUCEF_DATABASE_CAPABILITY_INDEXES ) );
            ASSERT_TRUE( 0 != ( capabilities & GUCEF_DATABASE_CAPABILITY_PREDICATE_PUSHDOWN ) && 0 != ( capabilities & GUCEF_DATABASE_CAPABILITY_ASYNC ) );
            ASSERT_TRUE( 0 == ( capabilities & GUCEF_DATABASE_CAPABILITY_PARTITIONED_TABLES ) && 0 == ( capabilities & GUCEF_DATABASE_CAPABILITY_SCHEMA_EVOLUTION ) );
            ASSERT_FALSE( client->IsOpen() );

            COMCORE::CDatabaseClientSettings settings;
            settings.backendName = "vfstables";
            settings.database = "assets";
            ASSERT_FALSE( client->Open( settings ) );
            settings.backendName = "postgresql";
            settings.backendOptions.Set( "port", CVariant( "not a port" ) );
            ASSERT_FALSE( client->Open( settings ) );

            // nothing listens on port 1 of the local host: the connection is refused at once
            settings.backendOptions.Delete( "port" );
            settings.backendOptions.Set( "host", CVariant( "127.0.0.1" ) );
            settings.backendOptions.Set( "port", CVariant( "1" ) );
            settings.backendOptions.Set( "password", CVariant( "s3cret-value" ) );
            settings.backendOptions.Set( "connectTimeout", CVariant( "2" ) );
            ASSERT_FALSE( client->Open( settings ) );
            ASSERT_FALSE( client->IsOpen() );
            const CString error = client->GetLastErrorMessage();
            ASSERT_TRUE( !error.IsNULLOrEmpty() && error.HasSubstr( "s3cret-value" ) < 0 );

            CString::StringSet names;
            ASSERT_TRUE( DATATABLE_STATUS_OK != client->ListTables( names ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK != client->BeginTransaction() );
            ASSERT_FALSE( client->IsInTransaction() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
