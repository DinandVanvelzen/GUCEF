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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIODSP_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIODSP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The DSP building blocks: the state variable filter against the analytic response of the Audio EQ Cookbook, the biquad design, the delay line (whole and fractional reads, wrap around), the sliding minimum against brute force and the time constants of the envelope follower.
 */
void
PerformAudioDspTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIODSP_H ? */
