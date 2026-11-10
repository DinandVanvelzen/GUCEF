/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_TESTAPP_TESTBUSYWAITPULSEGENERATORDRIVER_H
#define GUCEF_CORE_TESTAPP_TESTBUSYWAITPULSEGENERATORDRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests CBusyWaitPulseGeneratorDriver as the driver of a pulse generator whose Run() loop is on a thread of its own:
 *  other threads ask for periodic pulses, pulse intervals, immediate pulses and the stop of periodic pulses while the
 *  loop pulses, and the last stop ends the loop. Run under the thread sanitizer it shows whether the driver keeps the
 *  state it shares between those threads safe.
 *  A pulse request through the driver (CPulseGenerator::RequestImmediatePulseThroughDriver()) from another thread
 *  never pulses on that thread, also while the loop does not run, and the loop pulses for it when it runs; the driver
 *  is called without the data lock of the pulse generator, and removing a driver waits for a request inside it.
 */
void
PerformBusyWaitPulseGeneratorDriverTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTBUSYWAITPULSEGENERATORDRIVER_H ? */
