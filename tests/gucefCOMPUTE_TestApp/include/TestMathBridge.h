/*
 *  gucefCOMPUTE_TestApp: GUCEF test application for the Compute module
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

#ifndef GUCEF_COMPUTE_TESTAPP_TESTMATHBRIDGE_H
#define GUCEF_COMPUTE_TESTAPP_TESTMATHBRIDGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The MATH bridge suite (COMPUTE design section 9): the plugin computebridgeMATH loaded through the plugin control.
 *  Every bridged kernel through the dispatch table at the forced MULTICORE tier against the scalar provider (counts,
 *  misaligned, in place, strided, 1, 2 and N workers), the call flags, providers that fail, the threshold, settings,
 *  the CPU kernels against the scalar path, the ACCELERATOR providers on a test device (strides, in place, device
 *  loss) and the unload of the plugin while other threads call the batch functions.
 */
void
PerformMathBridgeTests( void );

/**
 *  Opt in with GUCEF_TEST_COMPUTE_BENCH=1: per bridged kernel and tier the time over batch sizes of 2^8 to 2^24, the
 *  crossover where the multi-core tier beats SIMD, and the fork-join latency. Reports only: no test fails on a time
 */
void
PerformMathBridgeBenchmarks( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_TESTAPP_TESTMATHBRIDGE_H ? */
