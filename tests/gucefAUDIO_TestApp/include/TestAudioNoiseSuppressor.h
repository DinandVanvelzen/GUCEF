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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIONOISESUPPRESSOR_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIONOISESUPPRESSOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The noise suppressor effect (section 4.11 of the intercom design): parameters, registry and configuration, the
 *  latency it reports against the delay it has, the improvement of the signal to noise ratio of speech in white and pink
 *  noise, the distortion of clean speech, the steadiness of the gains on noise alone (no musical noise), transients and
 *  the tracking of a change of the noise, block size invariance, channels, silence, denormals and not-a-number input,
 *  zero cost when it has nothing to do, bit exact bypass in its slot and real time safety.
 */
void
PerformAudioNoiseSuppressorTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIONOISESUPPRESSOR_H ? */
