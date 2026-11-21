/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_WEB_TESTAPP_TESTHTTPRESPONSESIZES_H
#define GUCEF_WEB_TESTAPP_TESTHTTPRESPONSESIZES_H

/**
 *  Tests that a response of CHTTPServer arrives whole and byte exact at its clients whatever the size of the response
 *  is: sizes of header plus body around the multiples of 1024 bytes, with and without a long header, on keep-alive
 *  and on closing connections, read by the native HTTP client, the TCP client of gucefCOMCORE and a plain socket
 */
void
PerformHttpResponseSizeTests( void );

#endif /* GUCEF_WEB_TESTAPP_TESTHTTPRESPONSESIZES_H ? */
