/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_TESTAPP_TESTDATATABLESCANBENCHMARK_H
#define GUCEF_CORE_TESTAPP_TESTDATATABLESCANBENCHMARK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The scan benchmark, run only when GUCEF_TEST_BENCHMARK is set: rows of a timestamp, a float64 and a dictionary
 *  encoded text column in a memory table, scanned through typed spans, through CVariant cells and async, with the
 *  rows per second and the heap allocations logged. Gates the zero allocation steady state and zero pool misses.
 *  GUCEF_TEST_BENCHMARK_ROWS sets the row count (default 10,000,000; 1,000,000 in debug builds).
 */
void
PerformDataTableScanBenchmarkTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTDATATABLESCANBENCHMARK_H ? */
