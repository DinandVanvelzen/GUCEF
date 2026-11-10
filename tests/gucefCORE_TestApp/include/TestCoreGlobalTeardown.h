/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_TESTAPP_TESTCOREGLOBALTEARDOWN_H
#define GUCEF_CORE_TESTAPP_TESTCOREGLOBALTEARDOWN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Starts the test app as the application that loads and unloads a plugin and exits, see RunCoreGlobalTeardown() */
#define GUCEF_CORE_TESTAPP_CORE_GLOBAL_TEARDOWN_ARG    "--core-global-teardown"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the test app does when started with GUCEF_CORE_TESTAPP_CORE_GLOBAL_TEARDOWN_ARG: it makes the global objects
 *  of gucefCORE, loads a plugin, unloads it and exits. Once the global objects are destroyed at exit it looks at the
 *  logging global and ends the process with an exit code that tells what it found.
 *  @return what main returns, which the exit code of that look replaces
 */
int
RunCoreGlobalTeardown( void );

/*-------------------------------------------------------------------------*/

/**
 *  Starts the test app as such an application: the logging global is made once and is gone after the teardown, it is
 *  not made again by something that logs during the teardown
 */
void
PerformCoreGlobalTeardownTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTCOREGLOBALTEARDOWN_H ? */
