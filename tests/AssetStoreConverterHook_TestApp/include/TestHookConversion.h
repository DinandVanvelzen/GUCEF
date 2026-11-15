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

#ifndef TESTHOOKCONVERSION_H
#define TESTHOOKCONVERSION_H

/** The settings, the licence record, the manifest, the cancel and the versions of the hook, without a store */
void PerformHookUnitTests( void );

/** The primary store with the converter hook on the pipeline of AssetConverterLib, on the vfstables backend */
void PerformHookPrimaryStoreVfsTablesTests( void );

/** The same on PostgreSQL, when GUCEF_TEST_POSTGRESQL names a test database; skipped with a notice otherwise */
void PerformHookPrimaryStorePostgreSqlTests( void );

#endif /* TESTHOOKCONVERSION_H ? */
