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

#ifndef GUCEF_CORE_TESTAPP_TESTSERVICEHOSTING_H
#define GUCEF_CORE_TESTAPP_TESTSERVICEHOSTING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  First argument with which the stop signal tests start the test app itself. The second argument says when the
 *  application gets its SIGTERM:
 *  "default"   before anything: GUCEF_OSMAIN_BEGIN keeps the default action, the process ends by the signal
 *  "before"    after GUCEF_OSMAIN_DEFER_STOP_SIGNALS and before GUCEF_OSMAIN_SIGNAL_HANDLER(), as during the
 *              configuration load of a service: the signal is kept, handed to the handler and main() returns at once
 *  "after"     from a timer while CGUCEFApplication::main() runs its loop: the loop ends at the next pulse
 *  The application ends with exit code 0 when it behaved as it should.
 */
#define GUCEF_CORE_TESTAPP_STOP_SIGNAL_ARG     "--stop-signal-application"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests what a service process needs from gucefCORE and gucef_osmain.h: the recorded behaviour of the argument
 *  parsing the tools used to copy, CORE::ParseApplicationArguments(), CORE::GetProcessArguments(), the exact
 *  --console match of the Windows entry point, the minimalLogLevel shape of ProcessMetrics, and stop signals
 *  before and after the start (in a child process)
 */
void
PerformServiceHostingTests( void );

/*-------------------------------------------------------------------------*/

/**
 *  What the test app does when started with GUCEF_CORE_TESTAPP_STOP_SIGNAL_ARG, after the entry point installed
 *  (or deferred) its handler: runs the application. 'signalBeforeRun' says the SIGTERM came before.
 *  Returns the exit code
 */
int
RunStopSignalApplication( bool signalBeforeRun );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTSERVICEHOSTING_H ? */
