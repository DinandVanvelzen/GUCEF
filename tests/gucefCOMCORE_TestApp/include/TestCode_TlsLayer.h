/*
 *  gucefCOMCORE_TestApp: GUCEF test application for the COMCORE module
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

#ifndef GUCEF_COMCORE_TESTAPP_TESTCODE_TLSLAYER_H
#define GUCEF_COMCORE_TESTAPP_TESTCODE_TLSLAYER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests of the TLS layer without any TLS library: the settings, the provider registry and the connection
 *  state machine (events, handshake timer, upgrade, close ordering, observers that delete the connection)
 *  against a fake provider and a fake peer on a real loopback TCP connection.
 */
void
PerformTlsLayerTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TESTAPP_TESTCODE_TLSLAYER_H ? */
