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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICERECEIVER_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICERECEIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the voice receiver and its playout source: configuration, bit-exact PCM end to end through the engine at a
 *  constant delay, loss with concealment and forward error correction, statistics, the drift resampler and its switch,
 *  stream restarts and other streams, receivers added and removed while the engine runs, threads (TSan) and the audio
 *  thread under RealtimeSanitizer.
 */
void
PerformAudioVoiceReceiverTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICERECEIVER_H ? */
