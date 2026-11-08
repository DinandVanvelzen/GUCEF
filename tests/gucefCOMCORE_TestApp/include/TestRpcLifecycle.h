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

#ifndef GUCEF_COMCORE_TESTAPP_TESTRPCLIFECYCLE_H
#define GUCEF_COMCORE_TESTAPP_TESTRPCLIFECYCLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Suite rpc_lifecycle: how transports, listeners, hosts and endpoints come and go (gucefCOMCORE RPC design 10.2,
 *  10.5, 10.9): an endpoint connecting over a transport that brings its own thread, accepted transports the host lets
 *  go at the next pulse of their own thread, subscriptions made on one thread while the protocol thread raises
 *  events, test decorators passing calls on, and server calls nobody observes staying unlinked from their event
 *  thread. Teardown with thousands of calls outstanding is a case of the
 *  conformance suite (TestRpcConformance.h), so that it runs over every transport
 */
void PerformRpcLifecycleTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TESTAPP_TESTRPCLIFECYCLE_H ? */
