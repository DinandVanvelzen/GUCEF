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

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSDRN_H
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSDRN_H

/** RU-T10 (two members in one process over drn) and RU-T12 (two processes) of gucefDRNUTILS_Design.md 15.3 */
void PerformDrnUtilsDrnTests( const char* exePath );

/** The child process of RU-T12: hosts a scheduler over drn until SIGTERM; returns the exit code */
int RunDrnUtilsChildHost( const char* portText );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSDRN_H ? */
