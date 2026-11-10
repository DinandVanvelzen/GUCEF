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

#ifndef GUCEF_WEB_TESTAPP_TESTWEBSOCKETCODEC_H
#define GUCEF_WEB_TESTAPP_TESTWEBSOCKETCODEC_H

/**
 *  Tests the web socket (RFC 6455) building blocks that need no networking:
 *  - the opening handshake key computation (needs the sha1-raw codec plugin)
 *  - frame encoding and decoding using the examples of RFC 6455 section 5.7
 *  - message parsing: fragmentation, interleaved control frames, masking direction, UTF-8 and size validation
 */
void
PerformWebSocketCodecTests( void );

#endif /* GUCEF_WEB_TESTAPP_TESTWEBSOCKETCODEC_H ? */
