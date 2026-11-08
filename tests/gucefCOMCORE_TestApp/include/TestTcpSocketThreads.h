/*
 *  gucefCOMCORE_TestApp: GUCEF test application for the COMCORE module
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

#ifndef GUCEF_COMCORE_TESTAPP_TESTTCPSOCKETTHREADS_H
#define GUCEF_COMCORE_TESTAPP_TESTTCPSOCKETTHREADS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Suite TcpSocketThreads: TCP sockets whose pulse generator is pulsed on a thread of its own while another thread
 *  uses them. A client socket is connected, asked whether it is active and closed from the test thread (run under the
 *  thread sanitizer: no data race on its state). Connections of a server socket are closed from the test thread
 *  while a third thread sends to all clients of that server: both threads keep going, a thread that makes no progress
 *  for seconds is a deadlock, which the test reports instead of hanging.
 */
void PerformTcpSocketThreadsTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TESTAPP_TESTTCPSOCKETTHREADS_H ? */
