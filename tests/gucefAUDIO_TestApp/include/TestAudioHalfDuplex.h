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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOHALFDUPLEX_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOHALFDUPLEX_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the half-duplex controller of the capture path (section 4.13 of the design): its settings, the state machine
 *  against scripted near and far signals, timing, chatter, click-free attenuation, the coupling estimate, the echo
 *  scenario of section 6.2 between two units, the engine integration and the capture thread under RealtimeSanitizer.
 */
void
PerformAudioHalfDuplexTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOHALFDUPLEX_H ? */
