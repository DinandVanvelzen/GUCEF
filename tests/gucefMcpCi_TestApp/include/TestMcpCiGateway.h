/*
 *  gucefMcpCi_TestApp: tests of the gucefMcpCi MCP server
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

#ifndef GUCEF_MCPCI_TESTAPP_TESTMCPCIGATEWAY_H
#define GUCEF_MCPCI_TESTAPP_TESTMCPCIGATEWAY_H

/**
 *  Tests of the modes gateway and proxy: a gucefMcpCi gateway as a child process on 127.0.0.1 (plain HTTP, and HTTPS
 *  with test certificates) in front of the fake Forgejo, and gucefMcpCi proxies as MCP servers over stdio that talk
 *  to it; the raw gateway API for the limits and refusals; a front that loses chunks or answers garbage. Nothing goes
 *  over the network. Tests that need git or the openssl tool skip without them.
 */
void PerformMcpCiGatewayTests( void );

#endif /* GUCEF_MCPCI_TESTAPP_TESTMCPCIGATEWAY_H ? */
