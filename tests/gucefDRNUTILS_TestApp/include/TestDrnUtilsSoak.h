/*
 *  gucefDRNUTILS_TestApp: GUCEF test application for gucefDRNUTILS
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

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSSOAK_H
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSSOAK_H

/**
 *  RU-T14 soak (opt-in duration GUCEF_TEST_SOAK_MINUTES, a short default run otherwise): viewer churn and command load
 *  with bounded memory, threads and file descriptors; and the adapters over the real CJobScheduler and task manager
 */
void PerformDrnUtilsSoakTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSSOAK_H ? */
