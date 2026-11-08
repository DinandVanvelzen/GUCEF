/*
 *  gucefMT_TestApp: GUCEF test application for the MT module
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

#ifndef GUCEF_MT_TESTAPP_TESTLOCKBENCHMARK_H
#define GUCEF_MT_TESTAPP_TESTLOCKBENCHMARK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Measures the cost of a lock and unlock pair of the mutex (CMutex and the C level mutex, also re-entered) and the
 *  read/write lock (reader and writer), uncontended on one thread and contended by several threads.
 *  Report only, nothing is asserted: the numbers are written to the log. Switched off by default, run with
 *  GUCEF_TEST_MT_LOCK_BENCH=1, preferably in an optimised build.
 */
void
PerformLockBenchmarks( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MT_TESTAPP_TESTLOCKBENCHMARK_H ? */
