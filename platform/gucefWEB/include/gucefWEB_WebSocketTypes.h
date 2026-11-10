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

#ifndef GUCEF_WEB_WEBSOCKETTYPES_H
#define GUCEF_WEB_WEBSOCKETTYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#include <map>

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The number of content bytes of a string as put on the wire: CString::ByteSize() includes the null terminator
 *  and CString::Length() counts code points which for non ASCII text is less than the number of bytes
 */
inline UInt32
GetWebSocketPayloadSize( const CORE::CString& str )
{
    UInt32 byteSize = str.ByteSize();
    return byteSize > 0 ? byteSize - 1 : 0;
}

/** Close status codes of RFC 6455 section 7.4 */
enum EWebSocketCloseCode
{
    WEBSOCKET_CLOSE_NORMAL             = 1000 ,
    WEBSOCKET_CLOSE_GOING_AWAY         = 1001 ,
    WEBSOCKET_CLOSE_PROTOCOL_ERROR     = 1002 ,
    WEBSOCKET_CLOSE_UNSUPPORTED_DATA   = 1003 ,
    WEBSOCKET_CLOSE_NO_STATUS          = 1005 ,     /**< never sent on the wire, indicates the peer sent no code */
    WEBSOCKET_CLOSE_ABNORMAL           = 1006 ,     /**< never sent on the wire, the connection dropped without a close handshake */
    WEBSOCKET_CLOSE_INVALID_PAYLOAD    = 1007 ,
    WEBSOCKET_CLOSE_POLICY_VIOLATION   = 1008 ,
    WEBSOCKET_CLOSE_MESSAGE_TOO_BIG    = 1009 ,
    WEBSOCKET_CLOSE_INTERNAL_ERROR     = 1011
};

/** Details of a connected web socket peer */
struct SWebSocketPeerInfo
{
    UInt32 connectionId;                                    /**< server side id of the connection, always 0 for a client */
    CORE::CString requestUri;                               /**< the resource the client connected to, including params */
    CORE::CString subProtocol;                              /**< the negotiated sub protocol, empty if none */
    std::map< CORE::CString, CORE::CString > headers;       /**< the headers of the upgrade request, names in lower case */

    SWebSocketPeerInfo( void ) : connectionId( 0 ), requestUri(), subProtocol(), headers() {}
};

struct SWebSocketTextMessage
{
    UInt32 connectionId;
    CORE::CString text;                                     /**< valid UTF-8 */

    SWebSocketTextMessage( void ) : connectionId( 0 ), text() {}
};

struct SWebSocketBinaryMessage
{
    UInt32 connectionId;
    CORE::CDynamicBuffer data;

    SWebSocketBinaryMessage( void ) : connectionId( 0 ), data() {}
};

struct SWebSocketCloseInfo
{
    UInt32 connectionId;
    UInt16 closeCode;                                       /**< see EWebSocketCloseCode, 1006 if the connection dropped */
    CORE::CString closeReason;
    bool closedByPeer;                                      /**< false if we initiated the close */

    SWebSocketCloseInfo( void ) : connectionId( 0 ), closeCode( WEBSOCKET_CLOSE_ABNORMAL ), closeReason(), closedByPeer( true ) {}
};

typedef CORE::CTCloneableObj< SWebSocketPeerInfo >      TWebSocketConnectedEventData;
typedef CORE::CTCloneableObj< SWebSocketTextMessage >   TWebSocketTextMessageEventData;
typedef CORE::CTCloneableObj< SWebSocketBinaryMessage > TWebSocketBinaryMessageEventData;
typedef CORE::CTCloneableObj< SWebSocketCloseInfo >     TWebSocketClosedEventData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_WEBSOCKETTYPES_H ? */
