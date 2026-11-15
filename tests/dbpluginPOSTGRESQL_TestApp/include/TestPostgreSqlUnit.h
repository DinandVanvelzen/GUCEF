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

#ifndef DBPLUGIN_POSTGRESQL_TESTAPP_TESTPOSTGRESQLUNIT_H
#define DBPLUGIN_POSTGRESQL_TESTAPP_TESTPOSTGRESQLUNIT_H

/**
 *  Tests of the backend that need no server: the type mapping, identifier quoting, the SQL of tables, indexes,
 *  inserts, upserts, selects and deletes with their parameters, the translation of predicates, the text forms of the
 *  values both ways, the parameters, the settings (with the password kept out of descriptions and errors) and the
 *  registration of the plugin
 */
void
PerformPostgreSqlUnitTests( void );

#endif /* DBPLUGIN_POSTGRESQL_TESTAPP_TESTPOSTGRESQLUNIT_H ? */
