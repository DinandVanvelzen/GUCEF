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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOSPATIAL_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOSPATIAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Spatial audio, the parts below the voice path: the distance models and their configuration, the cone, the occlusion mapping, the
 *  Doppler ratio, the panner (stereo law, ring panning, speaker table, widening), the listener frame and the 2D mode, the state of the
 *  listener, emitters and world settings with their publication to the audio thread, the configuration of the world settings and the engine
 *  that publishes them, and a stress run of writers against a reading audio thread.
 */
void
PerformAudioSpatialTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOSPATIAL_H ? */
