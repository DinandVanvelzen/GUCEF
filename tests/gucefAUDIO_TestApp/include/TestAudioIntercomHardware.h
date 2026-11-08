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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOINTERCOMHARDWARE_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOINTERCOMHARDWARE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Opt-in hardware and listening tests of the intercom audio on a unit (ALSA, Linux), items 1 to 5 of section 6.3 of
 *  gucefAUDIO_Intercom_Design.md. They are skipped unless GUCEF_TEST_AUDIO_INTERCOM=1, and nothing opens a sound
 *  device before then. Every test case logs what it is about to do ("INTERCOM HARDWARE TEST:") before it opens a
 *  device. Results go to the log; no test fails on a time, a count or a judgement, only when it could not be set up.
 *
 *  Environment variables
 *    GUCEF_TEST_AUDIO_INTERCOM=1                 enables the tests at all
 *    GUCEF_TEST_AUDIO_INTERCOM_ITEMS             the items to run, a comma separated list of 1, 2, 3, 4, 5 or "all"
 *                                                (none when unset: nothing is opened or played)
 *    GUCEF_TEST_AUDIO_DEVICE                     the ALSA playback device, for example "hw:CARD=Device,DEV=0"
 *                                                ("null" is the ALSA null device: no sound, the run is safe anywhere)
 *    GUCEF_TEST_AUDIO_CAPTURE_DEVICE             the ALSA capture device ("null" records silence)
 *    GUCEF_TEST_AUDIO_INTERCOM_MINUTES           item 1: minutes per combination (default 1, fractions allowed; the design asks for 10)
 *    GUCEF_TEST_AUDIO_INTERCOM_LOAD              item 1: number of CPU load threads (default 0; use the number of cores)
 *    GUCEF_TEST_AUDIO_INTERCOM_RT_PRIORITY       the real-time priority of the "with real-time scheduling" runs (default 70)
 *    GUCEF_TEST_AUDIO_INTERCOM_SECONDS           items 2 to 5: seconds of each measurement or listening stage (default 10)
 *    GUCEF_TEST_AUDIO_INTERCOM_LEVEL_DB          peak level of clicks and level of the noise in dBFS (default -20)
 *    GUCEF_TEST_AUDIO_INTERCOM_CABLE=1           item 3: a cable runs from the line out to the line in, so the end to
 *                                                end latency stage may play clicks (without the variable only the listening
 *                                                stages run, which need headphones)
 *
 *  Item 1: capture and playback at 240-frame periods with 2 and 3 periods, with and without real-time scheduling;
 *          silence is played, the capture is discarded.
 *  Item 2: round trip latency of a click from the playback to the capture (cable or acoustic path).
 *  Item 3: capture, sender, simulated link, receiver and playback on one unit: listening stages with headphones, and
 *          with the cable the end to end latency.
 *  Item 4: acoustic coupling of the unit's speaker to its microphone and the half-duplex controller's behaviour.
 *  Item 5: the echo canceller on the unit's speaker and microphone (16 kHz processing): convergence, ERLE against the
 *          canceller switched off, bulk delay, drift and the double talk detector while the operator speaks.
 */
void
PerformAudioIntercomHardwareTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOINTERCOMHARDWARE_H ? */
