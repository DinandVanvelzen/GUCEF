/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
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

#ifndef GUCEF_AUTH_TESTAPP_TESTLINKSTORE_H
#define GUCEF_AUTH_TESTAPP_TESTLINKSTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformLinkStoreTests( void );

/*-------------------------------------------------------------------------*/

/**
 *  Whether this run of the test application is the second process of the link store tests: the environment
 *  variable GUCEF_AUTH_TEST_HELPER is "linkstore-writer"
 */
bool
IsLinkStoreHelperRun( void );

/*-------------------------------------------------------------------------*/

/**
 *  The second process of the link store tests: links and unlinks accounts in the link file GUCEF_AUTH_TEST_STORE_PATH
 *  with one instance, at the same time as the test process does. Exit code 0 when every change was accepted and every
 *  account it linked resolves to its profile afterwards
 */
int
RunLinkStoreHelper( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_TESTAPP_TESTLINKSTORE_H ? */
