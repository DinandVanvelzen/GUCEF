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

#ifndef GUCEF_COMCORE_TESTAPP_TESTRPCENDPOINT_H
#define GUCEF_COMCORE_TESTAPP_TESTRPCENDPOINT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Suite rpc_endpoint: RPC over the loopback transport: handshake, contract fetch and the disk cache, binding (minor
 *  interop, major refused), no names on the wire after binding, all four call kinds, ACCEPTED + RESULT with string and
 *  UInt64 request ids, QueryStatus / CancelRequest (durable ids from a second session), deadlines, cancellation,
 *  peer-to-peer calls, reflection, access policy, handler execution modes and blocking calls
 */
void PerformRpcEndpointTests( void );

/** Started by a test as a process of its own, which nothing in it used gucefCOMCORE before the RPC test it runs */
#define GUCEF_COMCORE_TESTAPP_RPC_FRESH_PROCESS_ARG     "--rpc-thread-pool-handler-in-fresh-process"

/**
 *  What the test app does when started with GUCEF_COMCORE_TESTAPP_RPC_FRESH_PROCESS_ARG <result file>: two endpoints
 *  over loopback, a handler in the RPC_EXECUTION_THREAD_POOL mode, one call. Writes the outcome into the file: 0 when
 *  the handler ran on a pool thread and the call came back OK.
 */
int RunRpcThreadPoolHandlerInFreshProcess( const char* resultFile );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TESTAPP_TESTRPCENDPOINT_H ? */
