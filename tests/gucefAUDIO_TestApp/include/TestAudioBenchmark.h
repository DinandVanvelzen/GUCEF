/*
 *  gucefAUDIO_TestApp: GUCEF test application for the AUDIO module
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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOBENCHMARK_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOBENCHMARK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Report only measurements of the performance budget: decoding per block of 500 ms for each codec, mixing at
 *  48 kHz with 256 frames per block, each built-in effect and the master limiter per block, and ApplyBusLayout.
 *  Opt in with the environment variable GUCEF_TEST_AUDIO_BENCH=1, skipped otherwise. The numbers go to the log; the
 *  tests fail only when something could not be set up, never on a time.
 */
void
PerformAudioBenchmarkTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOBENCHMARK_H ? */
