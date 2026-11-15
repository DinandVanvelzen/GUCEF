/*
 *  gucefMATH_TestApp: GUCEF test application for the Math module
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

#ifndef GUCEF_MATH_TESTAPP_TESTBATCHDISPATCHER_H
#define GUCEF_MATH_TESTAPP_TESTBATCHDISPATCHER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The batch dispatch table: tiers, caps, call flags, determinism, fallback after a failing provider, alignment,
 *  owner-based unregistration, external kernels and concurrent registration while calls run
 */
void
PerformBatchDispatcherTests( void );

/*-------------------------------------------------------------------------*/

/** The first command line argument with which a test starts the test application as the child of the concurrent first batch call */
#define GUCEF_MATH_TESTAPP_CONCURRENT_FIRST_CALL_ARG    "--math-concurrent-first-batch-call"

/**
 *  The child process of that test: 16 threads make the first batch call of the process (which creates CMathGlobal)
 *  together. Returns 0 when every thread got the right result; a crash ends the process with a signal.
 */
int
RunConcurrentFirstBatchCall( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_TESTAPP_TESTBATCHDISPATCHER_H ? */
