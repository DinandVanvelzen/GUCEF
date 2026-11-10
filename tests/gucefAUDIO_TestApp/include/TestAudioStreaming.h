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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOSTREAMING_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOSTREAMING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the compressed codec plugins and the .au plugin through CAudioStream with its read ahead task and the real
 *  engine: seeks while the task runs, loop restarts by voice and by stream, a slow source, several streams of one
 *  plugin at once, stream teardown while decoding and streams that survive a device reopen.
 */
void
PerformAudioStreamingTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOSTREAMING_H ? */
