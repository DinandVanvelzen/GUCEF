/*
 *  gucefDRN_TestApp: GUCEF test application for the DRN module
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

#ifndef GUCEF_DRN_TESTAPP_TESTDRNRPCTRANSPORT_H
#define GUCEF_DRN_TESTAPP_TESTDRNRPCTRANSPORT_H

/**
 *  The "drn" RPC transport (suite "rpc"): the RPC message class, the transport and listener, principals, protection
 *  classes, resume across member links, frame sizes, threading, and the RPC endpoint behaviour over drn
 */
void PerformDRNRpcTransportTests( void );

/** Latency and throughput of the RPC transport (suite "rpcperf", part of "rpc") */
void PerformDRNRpcTransportPerformanceTests( void );

/** The RPC transport cases that matter for thread sanitizer runs (suite "rpcthreads") */
void PerformDRNRpcTransportThreadingTests( void );

#endif /* GUCEF_DRN_TESTAPP_TESTDRNRPCTRANSPORT_H ? */
