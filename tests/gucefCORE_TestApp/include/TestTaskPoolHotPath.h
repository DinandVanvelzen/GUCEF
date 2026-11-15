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

#ifndef GUCEF_CORE_TESTAPP_TESTTASKPOOLHOTPATH_H
#define GUCEF_CORE_TESTAPP_TESTTASKPOOLHOTPATH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Performance guard of the thread pool hot path (design TM-T8): throughput of empty tasks carried out on the calling
 *  thread and on worker threads, allocations per task, and the cost of the per pool counters and task time stamps on
 *  their own. The figures are written to the log and to TaskPoolHotPath_Benchmark.txt in the working directory, so
 *  runs of two builds can be compared. Cost guard: a figure more than 1.5 times the recorded first figure fails.
 */
void
PerformTaskPoolHotPathTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTTASKPOOLHOTPATH_H ? */
