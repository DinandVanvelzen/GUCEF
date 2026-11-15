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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOSPATIALVOICE_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOSPATIALVOICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Spatial audio in the engine, the voice path of a voice with an emitter: the mono working signal, the spatializer and its
 *  registry, the gains of distance, cone and occlusion, the low pass, Doppler through the resampler, the sends, the listener
 *  frame and the 2D mode heard in the output, surround output with the centre and LFE options, continuity of a moving
 *  emitter, the velocity derived from positions, and a game thread updating 200 emitters and the listener while rendering.
 */
void
PerformAudioSpatialVoiceTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOSPATIALVOICE_H ? */
