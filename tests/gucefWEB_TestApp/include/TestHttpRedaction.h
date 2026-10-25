/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_WEB_TESTAPP_TESTHTTPREDACTION_H
#define GUCEF_WEB_TESTAPP_TESTHTTPREDACTION_H

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

/**
 *  Tests that the log output of the web socket client and of the server side response header holds
 *  no secrets of the URLs and headers that they handle
 */
void
PerformHttpRedactionTests( void );

/**
 *  Tests with the log output captured that a secret in the URL or in a header of a request which
 *  fails never ends up in the log, whichever way the request is made: Execute(), ExecuteAsync(),
 *  the URI resource accessor and a redirect to a URL which is not valid.
 *
 *  @param client the HTTP client to use, set up to use the backend that is tested
 *  @param backendName the name of the backend, for the names of the test cases
 *  @param closedPort a loopback port which nothing listens on
 *  @param scriptedServerPort a loopback port with the scripted test server which has the path /redirect/invalid
 */
void
PerformHttpRedactionBackendTests( GUCEF::WEB::CHTTPClient& client         ,
                                  const GUCEF::CORE::CString& backendName ,
                                  GUCEF::UInt16 closedPort                ,
                                  GUCEF::UInt16 scriptedServerPort        );

#endif /* GUCEF_WEB_TESTAPP_TESTHTTPREDACTION_H ? */
