/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#ifndef GUCEF_VFS_TESTAPP_TESTREMOTEARCHIVECONFORMANCE_H
#define GUCEF_VFS_TESTAPP_TESTREMOTEARCHIVECONFORMANCE_H

#include "TestRemoteEnvironment.h"

/** Every case of the remote archive suite, against the arrangement that the factory makes (a remote place with the archive mounted on it) */
void
PerformRemoteArchiveCasesOn( TRemoteEnvFactory factory, const char* suiteName );

/** The cases against the memory place of this test application, then the archive conformance suite on the same archive type */
void
PerformRemoteArchiveConformanceTests( void );

#endif /* GUCEF_VFS_TESTAPP_TESTREMOTEARCHIVECONFORMANCE_H ? */
