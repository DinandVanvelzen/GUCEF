/*
 *  gucefDRNUTILS_TestApp: GUCEF test application
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

#ifndef GUCEF_TESTDRNUTILSLANDISCOVERY_H
#define GUCEF_TESTDRNUTILSLANDISCOVERY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The remote service host and its client on the local network: LAN discovery on by default for both, a host that
 *  announces itself with the default settings and one with port, instance id and key fingerprint, found by a service
 *  finder, updated with product properties and withdrawn when it drains, and a paired client without an endpoint that
 *  finds its service (not a spoofer with another key) and connects
 */
void PerformDrnUtilsLanDiscoveryTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_TESTDRNUTILSLANDISCOVERY_H ? */
