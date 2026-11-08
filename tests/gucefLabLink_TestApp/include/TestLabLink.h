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

#ifndef GUCEF_LABLINK_TESTAPP_TESTLABLINK_H
#define GUCEF_LABLINK_TESTAPP_TESTLABLINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests of gucefLabLink: the cluster and machine roles in one process over loopback, with throwaway state
 *  directories and stand-ins for join-command.sh, join.sh and claude (shell scripts that record what they get).
 *  Skipped without codecspluginMONOCYPHER.
 */
void
PerformLabLinkTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_TESTAPP_TESTLABLINK_H ? */
