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

#ifndef GUCEF_WEB_CWEBSOCKETFRAMECODEC_H
#define GUCEF_WEB_CWEBSOCKETFRAMECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

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
 *  Stateless encoding and decoding of single RFC 6455 web socket frames.
 *  Most users want CWebSocketMessageParser (message level, with fragmentation and validation)
 *  or the CWebSocketServer / CWebSocketClient classes instead of this low level codec.
 */
class GUCEF_WEB_PUBLIC_CPP CWebSocketFrameCodec
{
    public:

    enum EOpcode
    {
        OPCODE_CONTINUATION = 0x0 ,
        OPCODE_TEXT         = 0x1 ,
        OPCODE_BINARY       = 0x2 ,
        OPCODE_CLOSE        = 0x8 ,
        OPCODE_PING         = 0x9 ,
        OPCODE_PONG         = 0xA
    };

    struct SFrame
    {
        bool fin;
        UInt8 opcode;
        bool wasMasked;                 /**< whether the frame was masked on the wire, the payload below is always unmasked */
        CORE::CDynamicBuffer payload;

        SFrame( void ) : fin( true ), opcode( OPCODE_TEXT ), wasMasked( false ), payload() {}
    };

    enum EDecodeResult
    {
        DECODE_OK              ,    /**< a complete frame was decoded */
        DECODE_NEED_MORE_DATA  ,    /**< the data does not (yet) hold a complete frame */
        DECODE_PROTOCOL_ERROR  ,    /**< the data violates the protocol: reserved bits/opcodes, bad control frame */
        DECODE_TOO_BIG              /**< the frame payload is larger than the allowed maximum */
    };

    /**
     *  Appends an encoded frame to the output buffer
     *  @param maskFrame    clients MUST mask frames they send, servers MUST NOT
     */
    static bool EncodeFrame( bool fin                       ,
                             UInt8 opcode                   ,
                             const void* payload            ,
                             UInt32 payloadSize             ,
                             bool maskFrame                 ,
                             CORE::CDynamicBuffer& output   );

    /**
     *  Appends an encoded close frame to the output buffer
     *  @param closeCode    1005 means no status code is sent
     */
    static bool EncodeCloseFrame( UInt16 closeCode                ,
                                  const CORE::CString& reason     ,
                                  bool maskFrame                  ,
                                  CORE::CDynamicBuffer& output    );

    /**
     *  Attempts to decode a single frame from the start of the given data
     *  @param bytesConsumed    set to the size of the frame if DECODE_OK, otherwise 0
     */
    static EDecodeResult DecodeFrame( const void* data         ,
                                      UInt64 dataSize          ,
                                      UInt64 maxPayloadSize    ,
                                      SFrame& frame            ,
                                      UInt64& bytesConsumed    );

    private:

    CWebSocketFrameCodec( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBSOCKETFRAMECODEC_H ? */
