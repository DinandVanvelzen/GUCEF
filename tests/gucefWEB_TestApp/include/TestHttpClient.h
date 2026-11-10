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

#ifndef GUCEF_WEB_TESTAPP_TESTHTTPCLIENT_H
#define GUCEF_WEB_TESTAPP_TESTHTTPCLIENT_H

/**
 *  Tests the HTTP client: URL handling of CHttpRequestData, the cookie jar, the native backend via Execute() and
 *  ExecuteAsync() against a scripted server (responses split over packets, chunked, redirects, cookies, ranges,
 *  streaming, timeouts, cancellation) and the classic event based API
 */
void
PerformHttpClientTests( void );

#endif /* GUCEF_WEB_TESTAPP_TESTHTTPCLIENT_H ? */
