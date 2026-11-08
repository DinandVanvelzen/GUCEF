/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_TESTAPP_TESTSERVICEHEALTHREGISTRY_H
#define GUCEF_CORE_TESTAPP_TESTSERVICEHEALTHREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests CORE::CServiceHealthRegistry: the three probes before and after the start, check states and their
 *  times, a check that throws, the check interval, staleness when the pulse stops, registration rules, the info
 *  and an unregistration while the checks run on another thread or from within a check on the evaluating thread
 */
void
PerformServiceHealthRegistryTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTSERVICEHEALTHREGISTRY_H ? */
