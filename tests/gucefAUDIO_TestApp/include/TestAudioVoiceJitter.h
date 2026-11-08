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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICEJITTER_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICEJITTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the voice jitter buffer and the playout controller, driven directly in simulated time without an engine:
 *  sequence and timestamp extension with wrap, duplicates, reordering, late frames, stream restarts and redundancy;
 *  playout of constant delay (bit-exact), jitter, loss, talkspurts and clock drift with the measured delay, margin
 *  and ratio; long runs; the playout path under RealtimeSanitizer.
 */
void
PerformAudioVoiceJitterTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICEJITTER_H ? */
