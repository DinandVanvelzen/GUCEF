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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICELINK_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICELINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the intercom end to end (section 6.2 of the design): two or more units, each an engine on the null driver,
 *  connected by simulated links (CTestSimulatedVoiceLink) in simulated time: latency on wired and Wi-Fi models with PCM
 *  and Opus, loss with forward error correction, redundancy and concealment, reordering and duplicates, clock drift,
 *  talkspurts of the voice activity detector, push-to-talk with chimes, stream restarts, two streams mixed on a bus,
 *  a long run, threaded engines (TSan) and the audio threads under RealtimeSanitizer.
 */
void
PerformAudioVoiceLinkTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICELINK_H ? */
