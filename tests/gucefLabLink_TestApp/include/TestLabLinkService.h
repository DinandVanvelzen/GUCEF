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

#ifndef GUCEF_LABLINK_TESTAPP_TESTLABLINKSERVICE_H
#define GUCEF_LABLINK_TESTAPP_TESTLABLINKSERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests of the service form of gucefLabLink: the configuration file, the arguments the option parser gets, the
 *  defaults of each platform, the start status of a Windows service (the platform-neutral logic of gucef_osmain.h),
 *  and the program run with a configuration file (when gucefLabLink is next to the test application).
 */
void
PerformLabLinkServiceTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_TESTAPP_TESTLABLINKSERVICE_H ? */
