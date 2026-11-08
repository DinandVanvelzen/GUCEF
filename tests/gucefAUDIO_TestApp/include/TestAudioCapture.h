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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOCAPTURE_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOCAPTURE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Capture in the devices: the capture loop of the push device (blocks in order on the device thread, overruns counted
 *  before the block after the gap, a lost backend, the direction branch), real-time scheduling of the device thread
 *  from the device option, and the capture source of the null device (bit for bit, format, channel and rate
 *  conversion, silence after the end, a live source that has nothing for a while). The capture path of the engine: capture
 *  parameters and their configuration, signal through conversion, channel selection and resampling, effect stages and their
 *  order, mute ramp, meter, consumers with futures, overruns, a lost or missing capture device, threading, shutdown,
 *  ResolveReferences() of effects, capture effects outside bus layouts, and the real time rules (RealtimeSanitizer build).
 */
void
PerformAudioCaptureTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOCAPTURE_H ? */
