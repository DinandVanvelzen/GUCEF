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

#ifndef GUCEF_COMCORE_TESTAPP_TESTSOCKETREADINESS_H
#define GUCEF_COMCORE_TESTAPP_TESTSOCKETREADINESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Suite socket_readiness: CSocketReadinessWaiter and the sockets that use it on dedicated tasks. One shot arming
 *  (no wake storm), a closed socket's registration while the waiter waits, shutdown, wake latency of UDP and TCP
 *  sockets against periodic servicing, and the waiter's wait returns while idle
 */
void PerformSocketReadinessTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TESTAPP_TESTSOCKETREADINESS_H ? */
