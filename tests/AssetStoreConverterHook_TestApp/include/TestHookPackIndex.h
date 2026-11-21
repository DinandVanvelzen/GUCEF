/*
 *  AssetStoreConverterHook_TestApp: Test application for the converter hook of the primary asset store (AssetStoreConverterHook)
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

#ifndef TESTHOOKPACKINDEX_H
#define TESTHOOKPACKINDEX_H

/** The pack index through the converter hook on the pipeline of AssetConverterLib (vfstables): the document the converter wrote is loaded by the store and read by both, indexing a pack without one, derivation, licence flags, the reindex from the documents and the group filter */
void PerformHookPackIndexVfsTablesTests( void );

/** The same on PostgreSQL, when GUCEF_TEST_POSTGRESQL names a test database; skipped with a notice otherwise */
void PerformHookPackIndexPostgreSqlTests( void );

#endif /* TESTHOOKPACKINDEX_H ? */
