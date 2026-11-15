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

#ifndef DBPLUGIN_POSTGRESQL_TESTAPP_TESTPOSTGRESQLSUPPORT_H
#define DBPLUGIN_POSTGRESQL_TESTAPP_TESTPOSTGRESQLSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CIDATATABLE_H
#include "gucefCORE_CIDataTable.h"
#define GUCEF_CORE_CIDATATABLE_H
#endif /* GUCEF_CORE_CIDATATABLE_H ? */

#ifndef GUCEF_COMCORE_CIDATABASECLIENT_H
#include "gucefCOMCORE_CIDatabaseClient.h"
#define GUCEF_COMCORE_CIDATABASECLIENT_H
#endif /* GUCEF_COMCORE_CIDATABASECLIENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace POSTGRESQLTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Loads the JSON DStore plugin and registers the linked backend */
void SetUpTestEnvironment( void );

/**
 *  The connection string of the test server from GUCEF_TEST_POSTGRESQL, false when it is not set. The text can hold a
 *  password: it is never logged
 */
bool GetTestServerConnectionString( GUCEF::CORE::CString& connectionString );

/** A name for a schema of this run of the tests: "gucef_test_" with the process id and a random part */
GUCEF::CORE::CString MakeTestSchemaName( void );

/** Settings of the test server in the schema; the database is the one the connection string names */
GUCEF::COMCORE::CDatabaseClientSettings MakeServerSettings( const GUCEF::CORE::CString& connectionString, const GUCEF::CORE::CString& schema, bool readOnly );

/** A definition with an "id" key column and a column of every type, see CORETEST::MakeAllTypesSchema */
GUCEF::CORE::CDataTableDefinition MakeAllTypesDefinition( const GUCEF::CORE::CString& name );

/** The rows of a scan of the table with the query as text, sorted; the status that ended the scan */
GUCEF::CORE::TDataTableStatus ScanRows( GUCEF::CORE::CIDataTable& table, const GUCEF::CORE::CDataTableQuery& query,
                                        GUCEF::CORE::CString::StringVector& rows, bool async = false );

/** Waits for a future, false when it did not end in the time */
bool WaitForFuture( const GUCEF::CORE::CFutureResult& future, GUCEF::UInt32 timeoutInMs = 30000 );

/** The status of an ended future */
GUCEF::CORE::TTaskStatus StatusOfFuture( const GUCEF::CORE::CFutureResult& future );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace POSTGRESQLTEST */

/*-------------------------------------------------------------------------*/

#endif /* DBPLUGIN_POSTGRESQL_TESTAPP_TESTPOSTGRESQLSUPPORT_H ? */
