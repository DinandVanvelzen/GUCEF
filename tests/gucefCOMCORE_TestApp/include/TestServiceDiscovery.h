/*
 *  gucefCOMCORE_TestApp: GUCEF test application for the communication module
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

#ifndef GUCEF_COMCORE_TESTSERVICEDISCOVERY_H
#define GUCEF_COMCORE_TESTSERVICEDISCOVERY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The generic service discovery of gucefCOMCORE without a network: announcements, their validation and config, the
 *  answer limiter with an injected clock, and the service advertiser and finder over discovery clients that hand each
 *  other what they advertise in memory (sync and async find, merge across clients, filter, TTL expiry, withdrawal),
 *  plus the move of a UDP socket to another pulse generator
 */
void PerformServiceDiscoveryTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TESTSERVICEDISCOVERY_H ? */
