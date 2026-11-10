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

#ifndef GUCEF_WEB_CWEBSOCKETMESSAGEPARSER_H
#define GUCEF_WEB_CWEBSOCKETMESSAGEPARSER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string>
#include <vector>

#ifndef GUCEF_WEB_CWEBSOCKETFRAMECODEC_H
#include "gucefWEB_CWebSocketFrameCodec.h"
#define GUCEF_WEB_CWEBSOCKETFRAMECODEC_H
#endif /* GUCEF_WEB_CWEBSOCKETFRAMECODEC_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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
 *  Turns the byte stream of one web socket connection into messages.
 *  Handles frames split over multiple network reads, multiple frames in one read, fragmented messages
 *  (including control frames interleaved between fragments) and validates what the RFC requires:
 *  masking direction, UTF-8 in text messages and close reasons, valid close codes and a maximum message size.
 *  Once an error is reported the parser stays in the failed state until Reset().
 */
class GUCEF_WEB_PUBLIC_CPP CWebSocketMessageParser
{
    public:

    enum EMessageType
    {
        MESSAGE_TEXT   ,
        MESSAGE_BINARY ,
        MESSAGE_PING   ,
        MESSAGE_PONG   ,
        MESSAGE_CLOSE
    };

    struct SMessage
    {
        EMessageType type;
        CORE::CDynamicBuffer payload;       /**< for text the UTF-8 bytes, for close the raw payload */
        UInt16 closeCode;                   /**< close messages only, 1005 if the peer sent no code */
        CORE::CString closeReason;          /**< close messages only */

        SMessage( void ) : type( MESSAGE_BINARY ), payload(), closeCode( 1005 ), closeReason() {}
    };

    typedef std::vector< SMessage > TMessageVector;

    enum EParseResult
    {
        PARSE_OK               ,
        PARSE_PROTOCOL_ERROR   ,    /**< close with 1002 */
        PARSE_INVALID_PAYLOAD  ,    /**< invalid UTF-8, close with 1007 */
        PARSE_MESSAGE_TOO_BIG       /**< close with 1009 */
    };

    /**
     *  @param isServerSide     a server expects masked frames from its clients, a client expects unmasked frames from the server
     *  @param maxMessageSize   maximum size of a (reassembled) message in bytes
     */
    CWebSocketMessageParser( bool isServerSide, UInt32 maxMessageSize = 16 * 1024 * 1024 );

    /**
     *  Processes newly received bytes, appending every message that completed to 'messages'
     */
    EParseResult Feed( const void* data, UInt32 size, TMessageVector& messages );

    /**
     *  The close code to send to the peer after Feed() reported an error
     */
    UInt16 GetErrorCloseCode( void ) const;

    void Reset( void );

    /**
     *  Changes the maximum message size for the frames that are decoded from now on, including a frame of which
     *  only a part was received so far. A message being assembled that already is larger fails with the next frame.
     */
    void SetMaxMessageSize( UInt32 maxMessageSize );

    UInt32 GetMaxMessageSize( void ) const;

    /**
     *  Strict UTF-8 validation: rejects overlong forms, surrogates and code points beyond U+10FFFF
     */
    static bool IsValidUtf8( const void* data, UInt32 size );

    /**
     *  Is the given code allowed in a close frame
     */
    static bool IsValidCloseCode( UInt16 code );

    private:

    EParseResult SetError( EParseResult result );

    EParseResult ProcessFrame( CWebSocketFrameCodec::SFrame& frame, TMessageVector& messages );

    private:

    bool m_isServerSide;
    UInt32 m_maxMessageSize;
    std::string m_input;                    /**< received bytes not yet turned into frames */
    std::string m_fragments;                /**< payload of the fragmented message being assembled */
    bool m_inFragmentedMessage;
    CWebSocketFrameCodec::EOpcode m_fragmentedType;
    EParseResult m_failure;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBSOCKETMESSAGEPARSER_H ? */
