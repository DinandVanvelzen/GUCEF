/*
 *  gucefCOMCORE_TestApp: Test application for the gucefCOMCORE module
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

#ifndef GUCEF_COMCORE_TESTAPP_TESTCODE_TLSSERVER_H
#define GUCEF_COMCORE_TESTAPP_TESTCODE_TLSSERVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests of the server side of TLS: sessions of the OpenSSL provider against its client sessions in memory, and the
 *  TCP server socket with TLS settings against the TLS client connection, with throwaway certificates (skipped without
 *  the openssl tool or without a usable OpenSSL library).
 */
void
PerformTlsServerTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TESTAPP_TESTCODE_TLSSERVER_H ? */
