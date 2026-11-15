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

#ifndef GUCEF_CORE_TESTAPP_TESTLOGMANAGERLOGGERCHANGES_H
#define GUCEF_CORE_TESTAPP_TESTLOGMANAGERLOGGERCHANGES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The argument which starts the test app as the process of a logger change case, see RunLoggerChangeCase().
 *  The argument after it names the case: "race", "stop", "reentrant" or "bench".
 */
#define GUCEF_CORE_TESTAPP_LOGGER_CHANGE_CASE_ARG    "--logger-change-case"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the test app does when started with GUCEF_CORE_TESTAPP_LOGGER_CHANGE_CASE_ARG, in a process of its own: the
 *  log manager of that process has only the loggers of the case.
 *
 *  "race":      the logging thread delivers while threads log heavily and other threads add and remove loggers
 *  "stop":      the logging thread is switched off and on again while threads log and loggers are added and removed
 *  "reentrant": loggers add and remove loggers from within their log calls, on the logging thread and without it
 *  "bench":     the cost measurements of PerformLogManagerLoggerChangeTests(), appended to
 *               gucefCORE_TestApp_LoggerChange_bench.txt in the working directory
 *
 *  Every message of the logging threads must reach a logger that stays registered exactly once and in the order of its
 *  thread, a logger must never be called by two threads at once, and a logger must not be called once its removal
 *  returned. The exit code is 0 when all of that holds, else a combination of the LoggerChangeFailure flags of
 *  TestLogManagerLoggerChanges.cpp.
 */
int
RunLoggerChangeCase( const char* caseName );

/*-------------------------------------------------------------------------*/

/**
 *  Runs the cases of RunLoggerChangeCase() as child processes, and measures the cost per log message of the delivery
 *  to the loggers (CMultiLogger) and of the logging thread (CLoggingTask)
 */
void
PerformLogManagerLoggerChangeTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTLOGMANAGERLOGGERCHANGES_H ? */
