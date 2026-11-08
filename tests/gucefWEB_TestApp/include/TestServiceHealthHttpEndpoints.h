/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
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

#ifndef GUCEF_WEB_TESTAPP_TESTSERVICEHEALTHHTTPENDPOINTS_H
#define GUCEF_WEB_TESTAPP_TESTSERVICEHEALTHHTTPENDPOINTS_H

/**
 *  Tests WEB::CServiceHealthHttpEndpoints over a real CHTTPServer on the loopback interface: status codes and
 *  bodies of /health/live, /health/ready, /health/started and /info before and after the start, HEAD, refused
 *  methods, a check that throws, a check that blocks the thread of the registry (fails within the staleness
 *  window while the server keeps answering), redaction of the details, configured paths and a path that is taken
 */
void
PerformServiceHealthHttpEndpointsTests( void );

#endif /* GUCEF_WEB_TESTAPP_TESTSERVICEHEALTHHTTPENDPOINTS_H ? */
