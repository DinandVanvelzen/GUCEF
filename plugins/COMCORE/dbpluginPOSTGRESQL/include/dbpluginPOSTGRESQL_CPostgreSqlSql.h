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

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLEDEFINITION_H
#include "gucefCORE_CDataTableDefinition.h"
#define GUCEF_CORE_CDATATABLEDEFINITION_H
#endif /* GUCEF_CORE_CDATATABLEDEFINITION_H ? */

#ifndef GUCEF_CORE_CDATATABLEQUERY_H
#include "gucefCORE_CDataTableQuery.h"
#define GUCEF_CORE_CDATATABLEQUERY_H
#endif /* GUCEF_CORE_CDATATABLEQUERY_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLPARAMETERS_H
#include "dbpluginPOSTGRESQL_CPostgreSqlParameters.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLPARAMETERS_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLPARAMETERS_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The SQL of the postgresql backend. Names are always quoted identifiers and values always parameters ($1, $2, ...)
 *  with their text in CPostgreSqlParameters: no value is ever part of the text of a statement.
 *
 *  Column types:
 *
 *      bool            boolean
 *      int8            smallint, CHECK -128 .. 127             uint8     smallint, CHECK 0 .. 255
 *      int16           smallint                                uint16    integer, CHECK 0 .. 65535
 *      int32           integer                                 uint32    bigint, CHECK 0 .. 4294967295
 *      int64           bigint                                  uint64    numeric(20,0), CHECK 0 .. 18446744073709551615
 *      float32         real                                    float64   double precision
 *      decimal(p,s)    numeric(p,s)
 *      date            date
 *      timestamp       timestamp(3) or timestamp(6) with time zone (UTC adjusted) or without (local), for ms and us;
 *                      bigint (the ticks) for ns, which PostgreSQL timestamps cannot hold
 *      utf8            text COLLATE "C": bytewise order, as the data tables define it
 *      binary          bytea
 *
 *  Predicates follow the data tables exactly: text compares bytewise (COLLATE "C"), a comparison with NaN or of a NaN
 *  cell is false (PostgreSQL orders NaN above every number: those rows are excluded), a null never matches a
 *  comparison or an IN. Index hints become CREATE INDEX, on the columns in order, UNIQUE when the hint says so, named
 *  "<table>_<index>" (cut to 63 bytes with a SHA-1 suffix when longer, since index names are unique per schema).
 *
 *  Reentrant: static members only.
 */
class DBPLUGIN_POSTGRESQL_PUBLIC_CPP CPostgreSqlSql
{
    public:

    /** The table of the definitions of the tables the backend made, in the schema of the database */
    static const CORE::CString CatalogueTableName;

    /** The longest name PostgreSQL keeps (NAMEDATALEN - 1) */
    static const UInt32 MaxIdentifierBytes = 63;

    /** "name" with every '"' doubled */
    static CORE::CString QuoteIdentifier( const CORE::CString& name );

    /** "schema"."name" */
    static CORE::CString QualifiedName( const CORE::CString& schema, const CORE::CString& name );

    /** Not empty, at most MaxIdentifierBytes bytes of UTF-8, no NUL */
    static bool IsUsableIdentifier( const CORE::CString& name );

    /** The type of the column, false for UNSUPPORTED */
    static bool GetColumnType( const CORE::CDataTableColumnDef& column, CORE::CString& sqlType );

    /** The CHECK constraint of the range of the column ("CHECK (...)"), empty for types without one */
    static CORE::CString GetColumnCheck( const CORE::CDataTableColumnDef& column );

    /** CREATE TABLE IF NOT EXISTS of the definition: its columns, NOT NULL, the CHECK constraints and the primary key */
    static bool MakeCreateTable( const CORE::CDataTableDefinition& definition, const CORE::CString& schema, CORE::CString& sql, CORE::CString& error );

    /** The name of the PostgreSQL index of an index hint of the table */
    static CORE::CString MakeIndexName( const CORE::CString& tableName, const CORE::CString& indexName );

    /** CREATE [UNIQUE] INDEX IF NOT EXISTS of an index hint */
    static CORE::CString MakeCreateIndex( const CORE::CDataTableDefinition& definition, const CORE::CDataTableIndexDef& index, const CORE::CString& schema );

    /** The value of parameter n as a value of the column: "$n::bigint", "(DATE '1970-01-01' + $n::integer)", ... */
    static CORE::CString MakeParameterExpression( const CORE::CDataTableColumnDef& column, UInt32 parameterNumber );

    /** What a SELECT reads of the column, in the text CPostgreSqlValues::AppendResultValue() takes */
    static CORE::CString MakeReadExpression( const CORE::CDataTableColumnDef& column );

    /**
     *  The conditions of the predicates of a resolved query, AND'ed, with their literals added as parameters: empty
     *  for none, "FALSE" when the query matches nothing. UNSUPPORTED for a literal that cannot be sent (text with a NUL
     *  byte), LIMIT_EXCEEDED beyond the parameters of one statement.
     */
    static CORE::TDataTableStatus MakeWhere( const CORE::CDataTableResolvedQuery& query, const CORE::CDataTableSchema& schema,
                                             CPostgreSqlParameters& parameters, CORE::CString& where, CORE::CString& error );

    /** SELECT of the read expressions of the columns (schema indices, in order) with the conditions, ordered by the sort key, with the limit of the query */
    static CORE::TDataTableStatus MakeSelect( const CORE::CDataTableDefinition& definition, const CORE::CString& schema,
                                              const CORE::CDataTableSchema::TColumnIndexVector& columns, const CORE::CDataTableResolvedQuery& query,
                                              CPostgreSqlParameters& parameters, CORE::CString& sql, CORE::CString& error );

    /** SELECT count(*) of the rows that match the conditions */
    static CORE::TDataTableStatus MakeCount( const CORE::CDataTableDefinition& definition, const CORE::CString& schema, const CORE::CDataTableResolvedQuery& query,
                                             CPostgreSqlParameters& parameters, CORE::CString& sql, CORE::CString& error );

    /** DELETE of the rows that match the conditions */
    static CORE::TDataTableStatus MakeDelete( const CORE::CDataTableDefinition& definition, const CORE::CString& schema, const CORE::CDataTableResolvedQuery& query,
                                              CPostgreSqlParameters& parameters, CORE::CString& sql, CORE::CString& error );

    /** "INSERT INTO "schema"."table" ("a", "b") VALUES " */
    static CORE::CString MakeInsertPrefix( const CORE::CDataTableDefinition& definition, const CORE::CString& schema );

    /** "($1::bigint, $2::text)": a row of parameters from the first number, one per column */
    static CORE::CString MakeInsertRow( const CORE::CDataTableDefinition& definition, UInt32 firstParameterNumber );

    /**
     *  The end of an upsert: ON CONFLICT on the primary key DO UPDATE the other columns (DO NOTHING when every column is
     *  in the key), RETURNING whether each row replaced one ("t") or was added ("f"). Empty without a primary key
     */
    static CORE::CString MakeUpsertSuffix( const CORE::CDataTableDefinition& definition );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace POSTGRESQL */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H ? */
