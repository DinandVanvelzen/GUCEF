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

#ifndef GUCEF_WEB_TESTAPP_TESTHTTPLOGREDACTION_H
#define GUCEF_WEB_TESTAPP_TESTHTTPLOGREDACTION_H

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

/**
 *  Tests the helper that the HTTP stack logs its URLs, headers and texts through: the default rules for
 *  URLs (user info of a proxy URL, query, fragment, identifiers in the path), for the sensitive headers
 *  (Authorization, Proxy-Authorization, Cookie, Set-Cookie, X-Api-Key), for redirect targets and error texts,
 *  the secrets that an application registers and the opt-out for developers
 */
void
PerformHttpLogRedactionTests( void );

/**
 *  Tests with the log output captured that a secret which an application registered, and a path that an
 *  application asked to see in full, are logged as asked when a request fails
 *
 *  @param client the HTTP client to use, set up to use the backend that is tested
 *  @param backendName the name of the backend, for the names of the test cases
 *  @param closedPort a loopback port which nothing listens on
 */
void
PerformHttpLogRedactionBackendTests( GUCEF::WEB::CHTTPClient& client         ,
                                     const GUCEF::CORE::CString& backendName ,
                                     GUCEF::UInt16 closedPort                );

#endif /* GUCEF_WEB_TESTAPP_TESTHTTPLOGREDACTION_H ? */
