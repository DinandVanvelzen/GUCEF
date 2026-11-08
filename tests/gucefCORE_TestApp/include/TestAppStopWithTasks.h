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

#ifndef GUCEF_CORE_TESTAPP_TESTAPPSTOPWITHTASKS_H
#define GUCEF_CORE_TESTAPP_TESTAPPSTOPWITHTASKS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Starts the test app as the application that stops while a task of it calls into the application */
#define GUCEF_CORE_TESTAPP_APP_STOP_WITH_TASK_ARG    "--app-stop-with-app-using-task"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the test app does when started with GUCEF_CORE_TESTAPP_APP_STOP_WITH_TASK_ARG: it starts a dedicated task
 *  that calls into the application (CGUCEFApplication::GetApplicationDir()) on every cycle, then stops the application
 *  and exits. The application has to stop within seconds, with the task ended, and the process has to exit with the
 *  task threads gone before the global objects they use are destroyed.
 *  @return 0 when all of that holds, else a number that tells what did not
 */
int
RunAppStopWithAppUsingTask( void );

/*-------------------------------------------------------------------------*/

/**
 *  Starts the test app as such an application and looks at its exit code and how long it took
 */
void
PerformAppStopWithTasksTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTAPPSTOPWITHTASKS_H ? */
