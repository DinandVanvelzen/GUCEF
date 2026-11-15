/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef TESTASSETTOOLSMCP_H
#define TESTASSETTOOLSMCP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      FUNCTIONS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The asset tool set over the MCP test rig on the fake service: every tool, the schemas, the policy, the safety rules */
void PerformAssetToolsMcpFakeTests( void );

/** The asset tool set over the MCP test rig on CLocalAssetStoreService with a loopback store and a test converter */
void PerformAssetToolsMcpLocalTests( void );

/*-------------------------------------------------------------------------*/

#endif /* TESTASSETTOOLSMCP_H ? */
