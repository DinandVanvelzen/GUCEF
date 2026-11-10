/*
 *  gucefCOM_TestApp: GUCEF test application for the COM module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_COM_TESTAPP_TESTJSONRPC_H
#define GUCEF_COM_TESTAPP_TESTJSONRPC_H

/**
 *  Tests the JSON-RPC endpoint:
 *  - the examples of the JSON-RPC 2.0 specification, fed to an endpoint through a capturing transport
 *  - calls between two endpoints over an in-memory transport: ids, errors, timeouts, deferred answers, batches
 *  - CallAndWait(), which only waits on the thread that drives the pulse generator of the endpoint
 *  - the lenient dialect and extra envelope fields as used by the Chrome DevTools Protocol
 */
void
PerformJsonRpcTests( void );

#endif /* GUCEF_COM_TESTAPP_TESTJSONRPC_H ? */
