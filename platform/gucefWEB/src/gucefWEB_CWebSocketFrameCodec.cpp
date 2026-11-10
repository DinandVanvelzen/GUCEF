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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_WEB_CWEBSOCKETFRAMECODEC_H
#include "gucefWEB_CWebSocketFrameCodec.h"
#define GUCEF_WEB_CWEBSOCKETFRAMECODEC_H
#endif /* GUCEF_WEB_CWEBSOCKETFRAMECODEC_H ? */

#ifndef GUCEF_WEB_WEBSOCKETTYPES_H
#include "gucefWEB_WebSocketTypes.h"
#define GUCEF_WEB_WEBSOCKETTYPES_H
#endif /* GUCEF_WEB_WEBSOCKETTYPES_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CWebSocketFrameCodec::EncodeFrame( bool fin                     ,
                                   UInt8 opcode                 ,
                                   const void* payload          ,
                                   UInt32 payloadSize           ,
                                   bool maskFrame               ,
                                   CORE::CDynamicBuffer& output )
{GUCEF_TRACE;

    if ( GUCEF_NULL == payload && 0 != payloadSize )
        return false;

    // Control frames have to be small and can not be fragmented
    bool isControl = ( opcode & 0x8 ) != 0;
    if ( isControl && ( payloadSize > 125 || !fin ) )
        return false;

    UInt8 header[ 14 ];
    UInt32 headerSize = 0;
    header[ headerSize++ ] = (UInt8) ( ( fin ? 0x80 : 0x00 ) | ( opcode & 0x0F ) );

    UInt8 maskBit = maskFrame ? 0x80 : 0x00;
    if ( payloadSize <= 125 )
    {
        header[ headerSize++ ] = (UInt8) ( maskBit | payloadSize );
    }
    else
    if ( payloadSize <= 0xFFFF )
    {
        header[ headerSize++ ] = (UInt8) ( maskBit | 126 );
        header[ headerSize++ ] = (UInt8) ( payloadSize >> 8 );
        header[ headerSize++ ] = (UInt8) ( payloadSize & 0xFF );
    }
    else
    {
        header[ headerSize++ ] = (UInt8) ( maskBit | 127 );
        for ( Int32 shift = 56; shift >= 0; shift -= 8 )
            header[ headerSize++ ] = (UInt8) ( ( (UInt64) payloadSize >> shift ) & 0xFF );
    }

    UInt8 maskKey[ 4 ] = { 0, 0, 0, 0 };
    if ( maskFrame )
    {
        if ( !CORE::CRandom::GetRandomBytes( maskKey, sizeof( maskKey ) ) )
            return false;
        memcpy( header + headerSize, maskKey, sizeof( maskKey ) );
        headerSize += sizeof( maskKey );
    }

    output.Append( header, headerSize );
    if ( 0 == payloadSize )
        return true;

    if ( !maskFrame )
    {
        output.Append( payload, payloadSize );
        return true;
    }

    // Mask in chunks to avoid needing a second full copy of the payload
    const UInt8* src = static_cast< const UInt8* >( payload );
    UInt8 chunk[ 4096 ];
    UInt32 offset = 0;
    while ( offset < payloadSize )
    {
        UInt32 chunkSize = payloadSize - offset;
        if ( chunkSize > sizeof( chunk ) )
            chunkSize = sizeof( chunk );
        for ( UInt32 i=0; i<chunkSize; ++i )
            chunk[ i ] = src[ offset + i ] ^ maskKey[ ( offset + i ) & 3 ];
        output.Append( chunk, chunkSize );
        offset += chunkSize;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebSocketFrameCodec::EncodeCloseFrame( UInt16 closeCode             ,
                                        const CORE::CString& reason  ,
                                        bool maskFrame               ,
                                        CORE::CDynamicBuffer& output )
{GUCEF_TRACE;

    if ( 1005 == closeCode )
        return EncodeFrame( true, OPCODE_CLOSE, GUCEF_NULL, 0, maskFrame, output );

    // 2 bytes of status code leaves 123 for the reason. Note that Length() counts code points not bytes
    const char* reasonBytes = reason.C_String();
    UInt32 reasonSize = GetWebSocketPayloadSize( reason );
    if ( reasonSize > 123 )
    {
        // never cut a multi byte UTF-8 sequence in half as the reason has to be valid UTF-8
        reasonSize = 123;
        while ( reasonSize > 0 && ( (UInt8) reasonBytes[ reasonSize ] & 0xC0 ) == 0x80 )
            --reasonSize;
    }

    UInt8 payload[ 125 ];
    payload[ 0 ] = (UInt8) ( closeCode >> 8 );
    payload[ 1 ] = (UInt8) ( closeCode & 0xFF );
    if ( reasonSize > 0 )
        memcpy( payload + 2, reasonBytes, reasonSize );
    return EncodeFrame( true, OPCODE_CLOSE, payload, 2 + reasonSize, maskFrame, output );
}

/*-------------------------------------------------------------------------*/

CWebSocketFrameCodec::EDecodeResult
CWebSocketFrameCodec::DecodeFrame( const void* data         ,
                                   UInt64 dataSize          ,
                                   UInt64 maxPayloadSize    ,
                                   SFrame& frame            ,
                                   UInt64& bytesConsumed    )
{GUCEF_TRACE;

    bytesConsumed = 0;
    if ( dataSize < 2 )
        return DECODE_NEED_MORE_DATA;

    const UInt8* bytes = static_cast< const UInt8* >( data );
    bool fin = ( bytes[ 0 ] & 0x80 ) != 0;
    UInt8 reservedBits = bytes[ 0 ] & 0x70;
    UInt8 opcode = bytes[ 0 ] & 0x0F;
    bool masked = ( bytes[ 1 ] & 0x80 ) != 0;
    UInt64 payloadSize = bytes[ 1 ] & 0x7F;

    // We never negotiate extensions so reserved bits must be zero
    if ( 0 != reservedBits )
        return DECODE_PROTOCOL_ERROR;

    // Only the opcodes defined by the RFC are valid
    if ( !( opcode <= OPCODE_BINARY || ( opcode >= OPCODE_CLOSE && opcode <= OPCODE_PONG ) ) )
        return DECODE_PROTOCOL_ERROR;

    UInt64 headerSize = 2;
    if ( 126 == payloadSize )
    {
        if ( dataSize < 4 )
            return DECODE_NEED_MORE_DATA;
        payloadSize = ( (UInt64) bytes[ 2 ] << 8 ) | bytes[ 3 ];
        headerSize = 4;
    }
    else
    if ( 127 == payloadSize )
    {
        if ( dataSize < 10 )
            return DECODE_NEED_MORE_DATA;
        payloadSize = 0;
        for ( UInt32 i=0; i<8; ++i )
            payloadSize = ( payloadSize << 8 ) | bytes[ 2 + i ];
        if ( payloadSize & ( (UInt64) 1 << 63 ) )
            return DECODE_PROTOCOL_ERROR;   // the most significant bit must be 0
        headerSize = 10;
    }

    // Control frames: no fragmentation and at most 125 bytes
    if ( ( opcode & 0x8 ) != 0 && ( !fin || payloadSize > 125 ) )
        return DECODE_PROTOCOL_ERROR;

    if ( payloadSize > maxPayloadSize || payloadSize > (UInt64) GUCEF_UINT32MAX )
        return DECODE_TOO_BIG;

    UInt64 maskSize = masked ? 4 : 0;
    UInt64 totalSize = headerSize + maskSize + payloadSize;
    if ( dataSize < totalSize )
        return DECODE_NEED_MORE_DATA;

    frame.fin = fin;
    frame.opcode = opcode;
    frame.wasMasked = masked;
    frame.payload.Clear( true );
    if ( payloadSize > 0 )
    {
        frame.payload.SetDataSize( (UInt32) payloadSize );
        UInt8* dest = static_cast< UInt8* >( frame.payload.GetBufferPtr() );
        const UInt8* src = bytes + headerSize + maskSize;
        if ( masked )
        {
            const UInt8* maskKey = bytes + headerSize;
            for ( UInt64 i=0; i<payloadSize; ++i )
                dest[ i ] = src[ i ] ^ maskKey[ i & 3 ];
        }
        else
        {
            memcpy( dest, src, (size_t) payloadSize );
        }
    }
    else
    {
        frame.payload.SetDataSize( 0 );
    }

    bytesConsumed = totalSize;
    return DECODE_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
