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

#ifndef GUCEF_COMCORE_TESTAPP_TESTRPCPERF_H
#define GUCEF_COMCORE_TESTAPP_TESTRPCPERF_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Suite rpc_perf: unary round trip latency (median, p99) and pipelined throughput over loopback, the protocol on the
 *  test thread and on tasks of its own. The figures go to the log; the targets of design 15 are for release builds
 */
void PerformRpcPerfTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TESTAPP_TESTRPCPERF_H ? */
