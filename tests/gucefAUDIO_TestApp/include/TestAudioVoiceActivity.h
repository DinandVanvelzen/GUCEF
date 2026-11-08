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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICEACTIVITY_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICEACTIVITY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The voice test signals (determinism, levels, signal to noise ratios), the settings of the voice activity detection and
 *  their configuration (also as part of the engine parameters), the detector (detection of speech-like bursts in noise
 *  at several signal to noise ratios with measured hit and false alarm rates, steady noise, hum and clicks ignored,
 *  noise floors that change, hangover and hysteresis timing, no chatter, onset latency, identical decisions for any
 *  block size and with gaps in the stream, robustness to silence and bad samples) and the detector in the capture path of
 *  the engine (null device capture source, device threads, the real time rules in a RealtimeSanitizer build).
 */
void
PerformAudioVoiceActivityTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICEACTIVITY_H ? */
