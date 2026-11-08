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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOAUTOMATICGAIN_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOAUTOMATICGAIN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The automatic gain effect: convergence to the target for quiet and loud speech and its time, steady state error, no
 *  gain change in pauses, the gain limits and the ceiling, level steps, block size invariance, saving and loading, the
 *  registry and bus layouts, real time safety, and the combined capture chain (high pass, gate, automatic gain, voice
 *  detector) on a null capture device.
 */
void
PerformAudioAutomaticGainTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOAUTOMATICGAIN_H ? */
