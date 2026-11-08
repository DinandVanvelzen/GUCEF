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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICESENDER_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICESENDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the voice sender: configuration, frames and RTP fields with wrap, overruns, the outbound queue, redundancy within
 *  the packet size, push to talk, voice activated mode with pre-roll and hangover, mute, encoder errors, bit rate, codec
 *  changes, the sender as a capture consumer of the engine, threads (TSan) and the capture side under RealtimeSanitizer.
 */
void
PerformAudioVoiceSenderTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOVOICESENDER_H ? */
