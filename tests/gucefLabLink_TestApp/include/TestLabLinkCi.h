/*
 *  gucefLabLink_TestApp: tests of gucefLabLink
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

#ifndef GUCEF_LABLINK_TESTAPP_TESTLABLINKCI_H
#define GUCEF_LABLINK_TESTAPP_TESTLABLINKCI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests of the CI configuration (gucef.lab 1.2, CiConfig): the contract, the registry of the hub, the gateway tokens
 *  and their hash file, the answer of the hub for approval and lab secret tokens, the files of a machine with their
 *  access, a hub of the previous minor, and the commands --list and --revoke of the program. Skipped without
 *  codecspluginMONOCYPHER or dstorepluginJSONPARSER where they are needed.
 */
void
PerformLabLinkCiTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_TESTAPP_TESTLABLINKCI_H ? */
