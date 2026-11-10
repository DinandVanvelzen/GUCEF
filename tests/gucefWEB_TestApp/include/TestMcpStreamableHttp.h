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

#ifndef GUCEF_WEB_TESTAPP_TESTMCPSTREAMABLEHTTP_H
#define GUCEF_WEB_TESTAPP_TESTMCPSTREAMABLEHTTP_H

/**
 *  Tests the Streamable HTTP transport of MCP: conversations of a client with an MCP server over HTTP (JSON replies,
 *  SSE replies with progress, subscriptions, cancellation by closing the stream, the legacy era), the checks before
 *  the body (Host, Origin, method, authentication, content type and length, Accept), the header checks of modern
 *  requests and the mapping of errors to HTTP statuses
 */
void
PerformMcpStreamableHttpTests( void );

#endif /* GUCEF_WEB_TESTAPP_TESTMCPSTREAMABLEHTTP_H ? */
