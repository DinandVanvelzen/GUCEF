/*
 *  dbpluginPOSTGRESQL: gucefCOMCORE database backend for PostgreSQL
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CSHA1_H
#include "gucefCORE_CSha1.h"
#define GUCEF_CORE_CSHA1_H
#endif /* GUCEF_CORE_CSHA1_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLVALUES_H
#include "dbpluginPOSTGRESQL_CPostgreSqlValues.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLVALUES_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLVALUES_H ? */

#include "dbpluginPOSTGRESQL_CPostgreSqlSql.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DBPLUGIN {
namespace POSTGRESQL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CPostgreSqlSql::CatalogueTableName = "_gucef_tables";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** "$n" */
CORE::CString
Parameter( UInt32 number )
{
    return "$" + CORE::ToString( number );
}

/*-------------------------------------------------------------------------*/

bool
IsFloatColumn( const CORE::CDataTableColumnDef& column )
{
    return CORE::DATATABLE_TYPE_FLOAT32 == column.type || CORE::DATATABLE_TYPE_FLOAT64 == column.type;
}

/*-------------------------------------------------------------------------*/

bool
IsNaNLiteral( const CORE::CDataTableColumnDef& column, const CORE::CVariant& literal )
{
    if ( !IsFloatColumn( column ) )
        return false;
    const Float64 value = literal.AsFloat64();
    return value != value;
}

/*-------------------------------------------------------------------------*/

/** The column as an operand of a comparison: text in bytewise order */
CORE::CString
ComparedColumn( const CORE::CDataTableColumnDef& column )
{
    CORE::CString text = CPostgreSqlSql::QuoteIdentifier( column.name );
    if ( CORE::DATATABLE_TYPE_UTF8 == column.type )
        text += " COLLATE \"C\"";
    return text;
}

/*-------------------------------------------------------------------------*/

/** Adds the literal and gives its expression; false when it cannot be sent */
bool
AddLiteral( const CORE::CDataTableColumnDef& column, const CORE::CVariant& literal, CPostgreSqlParameters& parameters, CORE::CString& expression )
{
    const UInt32 number = CPostgreSqlValues::AddLiteralParameter( column, literal, parameters );
    if ( 0 == number )
        return false;
    expression = CPostgreSqlSql::MakeParameterExpression( column, number );
    return true;
}

/*-------------------------------------------------------------------------*/

/** "1" followed by the zeros of the scale: 10 to the power of the scale */
CORE::CString
PowerOfTen( UInt8 scale )
{
    CORE::CString text = "1";
    for ( UInt8 i=0; i<scale; ++i )
        text += "0";
    return text;
}

/*-------------------------------------------------------------------------*/

/** The condition of one resolved predicate; empty when it cannot be made */
CORE::TDataTableStatus
MakeCondition( const CORE::CDataTableResolvedPredicate& predicate, const CORE::CDataTableColumnDef& column, CPostgreSqlParameters& parameters,
               CORE::CString& condition, CORE::CString& error )
{
    const CORE::CString quoted = CPostgreSqlSql::QuoteIdentifier( column.name );
    if ( CORE::DATATABLE_PREDICATE_IS_NULL == predicate.op )
    {
        condition = quoted + " IS NULL";
        return CORE::DATATABLE_STATUS_OK;
    }
    if ( CORE::DATATABLE_PREDICATE_IS_NOT_NULL == predicate.op || predicate.matchesAllNonNull )
    {
        condition = quoted + " IS NOT NULL";
        return CORE::DATATABLE_STATUS_OK;
    }
    if ( predicate.matchesNothing )
    {
        condition = "FALSE";
        return CORE::DATATABLE_STATUS_OK;
    }

    const CORE::CString compared = ComparedColumn( column );
    CORE::CString expression;
    switch ( predicate.op )
    {
        case CORE::DATATABLE_PREDICATE_EQUALS:
        case CORE::DATATABLE_PREDICATE_NOT_EQUALS:
        {
            if ( IsNaNLiteral( column, predicate.low ) )
            {
                // a comparison with NaN is false
                condition = "FALSE";
                return CORE::DATATABLE_STATUS_OK;
            }
            if ( !AddLiteral( column, predicate.low, parameters, expression ) )
                break;
            condition = compared + ( CORE::DATATABLE_PREDICATE_EQUALS == predicate.op ? " = " : " <> " ) + expression;
            break;
        }
        case CORE::DATATABLE_PREDICATE_IN:
        {
            CORE::CString list;
            UInt32 sent = 0;
            bool failed = false;
            for ( UInt32 i=0; i<(UInt32) predicate.values.size(); ++i )
            {
                // a NaN in the set matches nothing
                if ( IsNaNLiteral( column, predicate.values[ i ] ) )
                    continue;
                if ( !AddLiteral( column, predicate.values[ i ], parameters, expression ) )
                {
                    failed = true;
                    break;
                }
                if ( sent > 0 )
                    list += ", ";
                list += expression;
                ++sent;
            }
            if ( failed )
                break;
            if ( 0 == sent )
            {
                condition = "FALSE";
                return CORE::DATATABLE_STATUS_OK;
            }
            condition = compared + " IN (" + list + ")";
            break;
        }
        default:
        {
            // the comparisons and RANGE: the bounds set by the resolution
            if ( ( predicate.hasLow && IsNaNLiteral( column, predicate.low ) ) || ( predicate.hasHigh && IsNaNLiteral( column, predicate.high ) ) )
            {
                condition = "FALSE";
                return CORE::DATATABLE_STATUS_OK;
            }
            CORE::CString parts;
            if ( predicate.hasLow )
            {
                if ( !AddLiteral( column, predicate.low, parameters, expression ) )
                    break;
                parts = compared + ( predicate.lowInclusive ? " >= " : " > " ) + expression;
            }
            if ( predicate.hasHigh )
            {
                if ( !AddLiteral( column, predicate.high, parameters, expression ) )
                {
                    parts.Clear();
                    break;
                }
                if ( !parts.IsNULLOrEmpty() )
                    parts += " AND ";
                parts += compared + ( predicate.highInclusive ? " <= " : " < " ) + expression;
            }
            if ( !predicate.hasLow && !predicate.hasHigh )
                parts = quoted + " IS NOT NULL";
            condition = parts;
            break;
        }
    }

    if ( condition.IsNULLOrEmpty() )
    {
        if ( parameters.GetCount() >= CPostgreSqlParameters::MaxCount )
        {
            error = "The predicates need more than " + CORE::ToString( CPostgreSqlParameters::MaxCount ) + " parameters";
            return CORE::DATATABLE_STATUS_LIMIT_EXCEEDED;
        }
        error = "A literal of the predicate on column \"" + column.name + "\" cannot be sent to PostgreSQL (text with a NUL byte)";
        return CORE::DATATABLE_STATUS_UNSUPPORTED;
    }

    // PostgreSQL orders NaN above every number and has NaN = NaN: a NaN cell matches no comparison
    if ( IsFloatColumn( column ) )
        condition = "(" + condition + " AND " + quoted + " <> 'NaN'::" + ( CORE::DATATABLE_TYPE_FLOAT32 == column.type ? "real" : "double precision" ) + ")";
    return CORE::DATATABLE_STATUS_OK;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CPostgreSqlSql::QuoteIdentifier( const CORE::CString& name )
{GUCEF_TRACE;

    return "\"" + name.ReplaceSubstr( "\"", "\"\"" ) + "\"";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPostgreSqlSql::QualifiedName( const CORE::CString& schema, const CORE::CString& name )
{GUCEF_TRACE;

    return QuoteIdentifier( schema ) + "." + QuoteIdentifier( name );
}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlSql::IsUsableIdentifier( const CORE::CString& name )
{GUCEF_TRACE;

    if ( name.IsNULLOrEmpty() )
        return false;
    const CORE::CUtf8String utf8 = CORE::ToUtf8String( name );
    const UInt32 length = utf8.ByteSize() - 1;
    return length > 0 && length <= MaxIdentifierBytes && GUCEF_NULL == memchr( utf8.C_String(), 0, length );
}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlSql::GetColumnType( const CORE::CDataTableColumnDef& column, CORE::CString& sqlType )
{GUCEF_TRACE;

    switch ( column.type )
    {
        case CORE::DATATABLE_TYPE_BOOL: sqlType = "boolean"; return true;
        case CORE::DATATABLE_TYPE_INT8: sqlType = "smallint"; return true;
        case CORE::DATATABLE_TYPE_INT16: sqlType = "smallint"; return true;
        case CORE::DATATABLE_TYPE_INT32: sqlType = "integer"; return true;
        case CORE::DATATABLE_TYPE_INT64: sqlType = "bigint"; return true;
        case CORE::DATATABLE_TYPE_UINT8: sqlType = "smallint"; return true;
        case CORE::DATATABLE_TYPE_UINT16: sqlType = "integer"; return true;
        case CORE::DATATABLE_TYPE_UINT32: sqlType = "bigint"; return true;
        case CORE::DATATABLE_TYPE_UINT64: sqlType = "numeric(20,0)"; return true;
        case CORE::DATATABLE_TYPE_FLOAT32: sqlType = "real"; return true;
        case CORE::DATATABLE_TYPE_FLOAT64: sqlType = "double precision"; return true;
        case CORE::DATATABLE_TYPE_DECIMAL:
            sqlType = "numeric(" + CORE::ToString( (UInt32) column.decimalPrecision ) + "," + CORE::ToString( (UInt32) column.decimalScale ) + ")";
            return true;
        case CORE::DATATABLE_TYPE_DATE: sqlType = "date"; return true;
        case CORE::DATATABLE_TYPE_TIMESTAMP:
        {
            if ( CORE::DATATABLE_TIMEUNIT_NANOSECONDS == column.timeUnit )
            {
                sqlType = "bigint";
                return true;
            }
            sqlType = CORE::DATATABLE_TIMEUNIT_MILLISECONDS == column.timeUnit ? "timestamp(3)" : "timestamp(6)";
            sqlType += column.utcAdjusted ? " with time zone" : " without time zone";
            return true;
        }
        case CORE::DATATABLE_TYPE_UTF8: sqlType = "text COLLATE \"C\""; return true;
        case CORE::DATATABLE_TYPE_BINARY: sqlType = "bytea"; return true;
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPostgreSqlSql::GetColumnCheck( const CORE::CDataTableColumnDef& column )
{GUCEF_TRACE;

    const CORE::CString quoted = QuoteIdentifier( column.name );
    switch ( column.type )
    {
        case CORE::DATATABLE_TYPE_INT8: return "CHECK (" + quoted + " BETWEEN -128 AND 127)";
        case CORE::DATATABLE_TYPE_UINT8: return "CHECK (" + quoted + " BETWEEN 0 AND 255)";
        case CORE::DATATABLE_TYPE_UINT16: return "CHECK (" + quoted + " BETWEEN 0 AND 65535)";
        case CORE::DATATABLE_TYPE_UINT32: return "CHECK (" + quoted + " BETWEEN 0 AND 4294967295)";
        case CORE::DATATABLE_TYPE_UINT64: return "CHECK (" + quoted + " BETWEEN 0 AND 18446744073709551615)";
        default: return CORE::CString::Empty;
    }
}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlSql::MakeCreateTable( const CORE::CDataTableDefinition& definition, const CORE::CString& schema, CORE::CString& sql, CORE::CString& error )
{GUCEF_TRACE;

    if ( !IsUsableIdentifier( definition.name ) )
    {
        error = "The table name \"" + definition.name + "\" is longer than the " + CORE::ToString( MaxIdentifierBytes ) + " bytes of a PostgreSQL name";
        return false;
    }
    const CORE::CDataTableSchema::TColumnDefVector& columns = definition.schema.GetColumns();
    sql = "CREATE TABLE IF NOT EXISTS " + QualifiedName( schema, definition.name ) + " (";
    for ( UInt32 i=0; i<(UInt32) columns.size(); ++i )
    {
        const CORE::CDataTableColumnDef& column = columns[ i ];
        CORE::CString sqlType;
        if ( !GetColumnType( column, sqlType ) )
        {
            error = "Column \"" + column.name + "\" has a type PostgreSQL tables cannot hold: " + column.GetTypeText();
            return false;
        }
        if ( !IsUsableIdentifier( column.name ) )
        {
            error = "The column name \"" + column.name + "\" is longer than the " + CORE::ToString( MaxIdentifierBytes ) + " bytes of a PostgreSQL name";
            return false;
        }
        if ( i > 0 )
            sql += ", ";
        sql += QuoteIdentifier( column.name ) + " " + sqlType;
        if ( !column.nullable )
            sql += " NOT NULL";
        const CORE::CString check = GetColumnCheck( column );
        if ( !check.IsNULLOrEmpty() )
            sql += " " + check;
    }
    const CORE::CDataTableSchema::TColumnIndexVector& key = definition.schema.GetPrimaryKey();
    if ( !key.empty() )
    {
        sql += ", PRIMARY KEY (";
        for ( UInt32 i=0; i<(UInt32) key.size(); ++i )
        {
            if ( i > 0 )
                sql += ", ";
            sql += QuoteIdentifier( columns[ key[ i ] ].name );
        }
        sql += ")";
    }
    sql += ")";
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPostgreSqlSql::MakeIndexName( const CORE::CString& tableName, const CORE::CString& indexName )
{GUCEF_TRACE;

    const CORE::CString full = tableName + "_" + indexName;
    const CORE::CUtf8String utf8 = CORE::ToUtf8String( full );
    const UInt32 length = utf8.ByteSize() - 1;
    if ( length <= MaxIdentifierBytes )
        return full;

    // cut, keeping whole UTF-8 characters, and made unique again with the start of the SHA-1 of the whole name
    const CORE::CString digest = CORE::CSha1::HashHex( utf8.C_String(), length );
    UInt32 keep = MaxIdentifierBytes - 9;
    const char* bytes = utf8.C_String();
    while ( keep > 0 && 0x80 == ( (UInt8) bytes[ keep ] & 0xC0 ) )
        --keep;
    return CORE::CString( bytes, keep ) + "_" + digest.SubstrToIndex( 8, true );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPostgreSqlSql::MakeCreateIndex( const CORE::CDataTableDefinition& definition, const CORE::CDataTableIndexDef& index, const CORE::CString& schema )
{GUCEF_TRACE;

    CORE::CString sql = index.unique ? "CREATE UNIQUE INDEX IF NOT EXISTS " : "CREATE INDEX IF NOT EXISTS ";
    sql += QuoteIdentifier( MakeIndexName( definition.name, index.name ) ) + " ON " + QualifiedName( schema, definition.name ) + " (";
    for ( UInt32 i=0; i<(UInt32) index.columns.size(); ++i )
    {
        if ( i > 0 )
            sql += ", ";
        sql += QuoteIdentifier( index.columns[ i ] );
    }
    sql += ")";
    return sql;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPostgreSqlSql::MakeParameterExpression( const CORE::CDataTableColumnDef& column, UInt32 parameterNumber )
{GUCEF_TRACE;

    const CORE::CString parameter = Parameter( parameterNumber );
    switch ( column.type )
    {
        case CORE::DATATABLE_TYPE_BOOL: return parameter + "::boolean";
        case CORE::DATATABLE_TYPE_INT8:
        case CORE::DATATABLE_TYPE_INT16:
        case CORE::DATATABLE_TYPE_UINT8: return parameter + "::smallint";
        case CORE::DATATABLE_TYPE_INT32:
        case CORE::DATATABLE_TYPE_UINT16: return parameter + "::integer";
        case CORE::DATATABLE_TYPE_INT64:
        case CORE::DATATABLE_TYPE_UINT32: return parameter + "::bigint";
        case CORE::DATATABLE_TYPE_UINT64:
        case CORE::DATATABLE_TYPE_DECIMAL: return parameter + "::numeric";
        case CORE::DATATABLE_TYPE_FLOAT32: return parameter + "::real";
        case CORE::DATATABLE_TYPE_FLOAT64: return parameter + "::double precision";
        case CORE::DATATABLE_TYPE_DATE: return "(DATE '1970-01-01' + " + parameter + "::integer)";
        case CORE::DATATABLE_TYPE_TIMESTAMP:
        {
            if ( CORE::DATATABLE_TIMEUNIT_NANOSECONDS == column.timeUnit )
                return parameter + "::bigint";
            // whole seconds and the rest, each exact as a double factor of an interval
            const bool millis = CORE::DATATABLE_TIMEUNIT_MILLISECONDS == column.timeUnit;
            const CORE::CString perSecond = millis ? "1000" : "1000000";
            const CORE::CString unit = millis ? "INTERVAL '1 millisecond'" : "INTERVAL '1 microsecond'";
            const CORE::CString epoch = column.utcAdjusted ? "TIMESTAMPTZ '1970-01-01 00:00:00+00'" : "TIMESTAMP '1970-01-01 00:00:00'";
            return "(" + epoch + " + (" + parameter + "::bigint / " + perSecond + ") * INTERVAL '1 second' + (" +
                   parameter + "::bigint % " + perSecond + ") * " + unit + ")";
        }
        case CORE::DATATABLE_TYPE_UTF8: return parameter + "::text";
        case CORE::DATATABLE_TYPE_BINARY: return parameter + "::bytea";
        default: return parameter;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPostgreSqlSql::MakeReadExpression( const CORE::CDataTableColumnDef& column )
{GUCEF_TRACE;

    const CORE::CString quoted = QuoteIdentifier( column.name );
    switch ( column.type )
    {
        case CORE::DATATABLE_TYPE_DECIMAL:
        {
            if ( 0 == column.decimalScale )
                return quoted;
            return "(" + quoted + " * " + PowerOfTen( column.decimalScale ) + "::numeric)::bigint";
        }
        case CORE::DATATABLE_TYPE_DATE:
            return "(" + quoted + " - DATE '1970-01-01')";
        case CORE::DATATABLE_TYPE_TIMESTAMP:
        {
            if ( CORE::DATATABLE_TIMEUNIT_NANOSECONDS == column.timeUnit )
                return quoted;
            const CORE::CString micros = "(EXTRACT(EPOCH FROM date_trunc('second', " + quoted + "))::bigint * 1000000 + EXTRACT(MICROSECONDS FROM " +
                                         quoted + ")::bigint % 1000000)";
            if ( CORE::DATATABLE_TIMEUNIT_MILLISECONDS == column.timeUnit )
                return "(" + micros + " / 1000)";
            return micros;
        }
        case CORE::DATATABLE_TYPE_BINARY:
            return "encode(" + quoted + ", 'hex')";
        default:
            return quoted;
    }
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlSql::MakeWhere( const CORE::CDataTableResolvedQuery& query, const CORE::CDataTableSchema& schema,
                           CPostgreSqlParameters& parameters, CORE::CString& where, CORE::CString& error )
{GUCEF_TRACE;

    where.Clear();
    if ( query.matchesNothing )
    {
        where = "FALSE";
        return CORE::DATATABLE_STATUS_OK;
    }
    for ( UInt32 i=0; i<(UInt32) query.predicates.size(); ++i )
    {
        const CORE::CDataTableResolvedPredicate& predicate = query.predicates[ i ];
        if ( predicate.columnIndex >= schema.GetColumnCount() )
        {
            error = "A predicate names column " + CORE::ToString( predicate.columnIndex ) + ", which the schema does not have";
            return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
        }
        CORE::CString condition;
        const CORE::TDataTableStatus status = MakeCondition( predicate, schema.GetColumn( predicate.columnIndex ), parameters, condition, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        if ( "FALSE" == condition )
        {
            where = "FALSE";
            return CORE::DATATABLE_STATUS_OK;
        }
        if ( !where.IsNULLOrEmpty() )
            where += " AND ";
        where += condition;
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlSql::MakeSelect( const CORE::CDataTableDefinition& definition, const CORE::CString& schema,
                            const CORE::CDataTableSchema::TColumnIndexVector& columns, const CORE::CDataTableResolvedQuery& query,
                            CPostgreSqlParameters& parameters, CORE::CString& sql, CORE::CString& error )
{GUCEF_TRACE;

    const CORE::CDataTableSchema& tableSchema = definition.schema;
    sql = "SELECT ";
    if ( columns.empty() )
        sql += "1";
    for ( UInt32 i=0; i<(UInt32) columns.size(); ++i )
    {
        if ( columns[ i ] >= tableSchema.GetColumnCount() )
        {
            error = "The projection names column " + CORE::ToString( columns[ i ] ) + ", which the schema does not have";
            return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
        }
        if ( i > 0 )
            sql += ", ";
        sql += MakeReadExpression( tableSchema.GetColumn( columns[ i ] ) );
    }
    sql += " FROM " + QualifiedName( schema, definition.name );

    CORE::CString where;
    const CORE::TDataTableStatus status = MakeWhere( query, tableSchema, parameters, where, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    if ( !where.IsNULLOrEmpty() )
        sql += " WHERE " + where;

    const CORE::CDataTableSchema::TColumnIndexVector& sortKey = tableSchema.GetSortKey();
    for ( UInt32 i=0; i<(UInt32) sortKey.size(); ++i )
        sql += ( 0 == i ? " ORDER BY " : ", " ) + QuoteIdentifier( tableSchema.GetColumn( sortKey[ i ] ).name ) + " ASC NULLS LAST";

    if ( CORE::CDataTableQuery::NoLimit != query.limit )
    {
        const CORE::CString limitText = CORE::ToString( query.limit );
        const UInt32 number = parameters.AddText( limitText );
        if ( 0 == number )
        {
            error = "The query needs more than " + CORE::ToString( CPostgreSqlParameters::MaxCount ) + " parameters";
            return CORE::DATATABLE_STATUS_LIMIT_EXCEEDED;
        }
        sql += " LIMIT " + Parameter( number ) + "::bigint";
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlSql::MakeCount( const CORE::CDataTableDefinition& definition, const CORE::CString& schema, const CORE::CDataTableResolvedQuery& query,
                           CPostgreSqlParameters& parameters, CORE::CString& sql, CORE::CString& error )
{GUCEF_TRACE;

    sql = "SELECT count(*) FROM " + QualifiedName( schema, definition.name );
    CORE::CString where;
    const CORE::TDataTableStatus status = MakeWhere( query, definition.schema, parameters, where, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    if ( !where.IsNULLOrEmpty() )
        sql += " WHERE " + where;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlSql::MakeDelete( const CORE::CDataTableDefinition& definition, const CORE::CString& schema, const CORE::CDataTableResolvedQuery& query,
                            CPostgreSqlParameters& parameters, CORE::CString& sql, CORE::CString& error )
{GUCEF_TRACE;

    sql = "DELETE FROM " + QualifiedName( schema, definition.name );
    CORE::CString where;
    const CORE::TDataTableStatus status = MakeWhere( query, definition.schema, parameters, where, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    if ( !where.IsNULLOrEmpty() )
        sql += " WHERE " + where;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPostgreSqlSql::MakeInsertPrefix( const CORE::CDataTableDefinition& definition, const CORE::CString& schema )
{GUCEF_TRACE;

    CORE::CString sql = "INSERT INTO " + QualifiedName( schema, definition.name ) + " (";
    const CORE::CDataTableSchema::TColumnDefVector& columns = definition.schema.GetColumns();
    for ( UInt32 i=0; i<(UInt32) columns.size(); ++i )
    {
        if ( i > 0 )
            sql += ", ";
        sql += QuoteIdentifier( columns[ i ].name );
    }
    sql += ") VALUES ";
    return sql;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPostgreSqlSql::MakeInsertRow( const CORE::CDataTableDefinition& definition, UInt32 firstParameterNumber )
{GUCEF_TRACE;

    CORE::CString row = "(";
    const CORE::CDataTableSchema::TColumnDefVector& columns = definition.schema.GetColumns();
    for ( UInt32 i=0; i<(UInt32) columns.size(); ++i )
    {
        if ( i > 0 )
            row += ", ";
        row += MakeParameterExpression( columns[ i ], firstParameterNumber + i );
    }
    row += ")";
    return row;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPostgreSqlSql::MakeUpsertSuffix( const CORE::CDataTableDefinition& definition )
{GUCEF_TRACE;

    const CORE::CDataTableSchema::TColumnIndexVector& key = definition.schema.GetPrimaryKey();
    if ( key.empty() )
        return CORE::CString::Empty;
    const CORE::CDataTableSchema::TColumnDefVector& columns = definition.schema.GetColumns();
    CORE::CString sql = " ON CONFLICT (";
    for ( UInt32 i=0; i<(UInt32) key.size(); ++i )
    {
        if ( i > 0 )
            sql += ", ";
        sql += QuoteIdentifier( columns[ key[ i ] ].name );
    }
    sql += ") ";
    CORE::CString assignments;
    for ( UInt32 c=0; c<(UInt32) columns.size(); ++c )
    {
        bool inKey = false;
        for ( UInt32 k=0; k<(UInt32) key.size() && !inKey; ++k )
            inKey = key[ k ] == c;
        if ( inKey )
            continue;
        if ( !assignments.IsNULLOrEmpty() )
            assignments += ", ";
        const CORE::CString quoted = QuoteIdentifier( columns[ c ].name );
        assignments += quoted + " = EXCLUDED." + quoted;
    }
    if ( assignments.IsNULLOrEmpty() )
        sql += "DO NOTHING";
    else
        sql += "DO UPDATE SET " + assignments;
    // xmax of a row an upsert updated is the id of the transaction, of a row it added 0
    sql += " RETURNING (xmax::text <> '0')";
    return sql;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace POSTGRESQL */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
