/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#define GUCEF_WEB_CWEBSOCKETHANDSHAKE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Helpers for the opening handshake of RFC 6455 which is shared by the web socket server and client
 *
 *  The SHA-1 needed for the handshake is not implemented in the platform: it is obtained from the
 *  "sha1-raw" codec in the cryptographic hash codec registry which is provided by a codec plugin
 *  (codecspluginSTBRUMMEHASH). If that plugin is not loaded the functions fail with a logged error.
 */
class GUCEF_WEB_PUBLIC_CPP CWebSocketHandshake
{
    public:

    /** The web socket protocol version we implement (RFC 6455) */
    static const CORE::CString ProtocolVersion;

    /** Is everything needed for the handshake available, ie is the SHA-1 codec loaded */
    static bool IsAvailable( void );

    /**
     *  Creates a new random Sec-WebSocket-Key value: the base64 of 16 random bytes
     */
    static bool GenerateClientKey( CORE::CString& clientKey );

    /**
     *  Computes the Sec-WebSocket-Accept value a server has to return for the given client key:
     *  base64( SHA-1( clientKey + GUID ) ) where the SHA-1 is the raw 20 byte digest
     */
    static bool ComputeAcceptKey( const CORE::CString& clientKey ,
                                  CORE::CString& acceptKey       );

    private:

    CWebSocketHandshake( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBSOCKETHANDSHAKE_H ? */
