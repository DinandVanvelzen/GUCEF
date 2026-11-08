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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIONOISEGATE_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIONOISEGATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The noise gate effect: static open and closed levels, threshold and hysteresis, no chatter near the threshold, attack,
 *  hold and release against the settings, the voice detector mode on speech and on noise, noise reduction in pauses,
 *  click free gain changes, block size invariance, denormals, saving and loading, the registry and bus layouts, real time
 *  safety.
 */
void
PerformAudioNoiseGateTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIONOISEGATE_H ? */
