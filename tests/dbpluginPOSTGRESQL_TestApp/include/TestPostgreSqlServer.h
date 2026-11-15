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

#ifndef DBPLUGIN_POSTGRESQL_TESTAPP_TESTPOSTGRESQLSERVER_H
#define DBPLUGIN_POSTGRESQL_TESTAPP_TESTPOSTGRESQLSERVER_H

/**
 *  Tests against a PostgreSQL server, run only when GUCEF_TEST_POSTGRESQL holds a connection string (skipped with a
 *  notice otherwise). Each run works in a schema of its own, which it drops at the end: tables and their definitions,
 *  index hints as indexes, values of every type through a write and a scan, random writes, deletes and queries
 *  against a CMemoryDataTable, upserts, conflicts, the write modes, partitions, transactions, async calls and read
 *  only clients
 */
void
PerformPostgreSqlServerTests( void );

#endif /* DBPLUGIN_POSTGRESQL_TESTAPP_TESTPOSTGRESQLSERVER_H ? */
