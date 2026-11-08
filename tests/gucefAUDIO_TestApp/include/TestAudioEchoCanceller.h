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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOECHOCANCELLER_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOECHOCANCELLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The echo stage (section 4.13 of the intercom design): the render tap ring, the decimator of the reference, the delay
 *  estimator, the settings, and the acoustic echo canceller in simulation: echo return loss enhancement for echo paths of
 *  5 to 150 ms and -6 to -30 dB and reverberant rooms, convergence time, double talk, a change of the echo path, clock
 *  drift of 100 and 300 ppm, background noise, a long run, the residual echo suppressor, bypass and the idle gate at no
 *  cost, block size invariance, the engine with the render tap (futures, threads, wake-ups), the hand-over of the
 *  half-duplex controller and the intercom call between two units; RealtimeSanitizer and ThreadSanitizer runs.
 */
void
PerformAudioEchoCancellerTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOECHOCANCELLER_H ? */
