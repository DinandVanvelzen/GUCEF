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

#ifndef GUCEF_MCPCI_TESTAPP_TESTMCPCIDEPLOY_H
#define GUCEF_MCPCI_TESTAPP_TESTMCPCIDEPLOY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      FUNCTIONS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The deployments of the server: images of runs, the guards of ci_deploy, the rollout and its failures, rollback,
 *  release status, the audit, submits of published commits, the scopes of gateway tokens, and proxy to gateway; against
 *  a fake Forgejo, a fake registry and a fake kubectl script in temporary directories
 */
void PerformMcpCiDeployTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_TESTAPP_TESTMCPCIDEPLOY_H ? */
