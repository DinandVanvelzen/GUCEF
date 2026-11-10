/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
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

#ifndef GUCEF_WEB_TESTAPP_TESTHTTPSERVERBODY_H
#define GUCEF_WEB_TESTAPP_TESTHTTPSERVERBODY_H

/**
 *  Tests how CHTTPServer takes requests in: one response per request, bodies and headers that arrive over several
 *  reads, pipelined requests, the limits and timeouts of a request that is coming in, the request takeover with
 *  CHttpServerExchange, streamed responses and SSE framing, and the connection state across disconnects
 */
void
PerformHttpServerBodyTests( void );

#endif /* GUCEF_WEB_TESTAPP_TESTHTTPSERVERBODY_H ? */
