/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
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

#ifndef GUCEF_AUTH_TESTAPP_TESTOAUTH2TLS_H
#define GUCEF_AUTH_TESTAPP_TESTOAUTH2TLS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The login engine over https through the native HTTP client backend (the TLS layer of gucefCOMCORE): the fake provider of
 *  the tests behind a TLS terminator, the check of an OpenID 2.0 assertion as a POST over TLS, and the refusal of a certificate
 *  that is not trusted. Without the plugin httpclientpluginCURL: the plugin goes for the duration of this group.
 */
void
PerformOAuth2TlsTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_TESTAPP_TESTOAUTH2TLS_H ? */
