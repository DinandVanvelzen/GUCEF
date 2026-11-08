/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#include "gucefAUDIO_CRtpVoicePacketCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRtpVoicePacketCodec::CRtpVoicePacketCodec( void )
    : m_redPayloadType( 113 )
{

}

/*-------------------------------------------------------------------------*/

UInt8
CRtpVoicePacketCodec::GetRedPayloadType( void ) const
{
    return m_redPayloadType;
}

/*-------------------------------------------------------------------------*/

void
CRtpVoicePacketCodec::SetRedPayloadType( UInt8 payloadType )
{
    m_redPayloadType = payloadType;
}

/*-------------------------------------------------------------------------*/

Int32
CRtpVoicePacketCodec::Write( const CVoiceFrame& frame, UInt8* packet, UInt32 capacity ) const
{
    if ( 0 == frame.GetPayloadSize() || frame.GetPayloadType() > 127 )
        return -1;

    const bool hasRedundancy = frame.HasRedundantBlock();
    if ( hasRedundancy )
    {
        if ( m_redPayloadType > 127 || frame.GetRedundantPayloadType() > 127 ||
             frame.GetRedundantSize() > CVoiceFrame::MaxRedundantSize || frame.GetRedundantTimestampOffset() > CVoiceFrame::MaxRedundantTimestampOffset )
            return -1;
    }

    const UInt32 redHeaderSize = hasRedundancy ? 5 : 0;
    const UInt32 total = CRtpHeader::FixedHeaderSize + redHeaderSize + ( hasRedundancy ? frame.GetRedundantSize() : 0 ) + frame.GetPayloadSize();
    if ( capacity < total )
        return -1;

    CRtpHeader header;
    header.SetMarker( frame.GetMarker() );
    header.SetPayloadType( hasRedundancy ? m_redPayloadType : frame.GetPayloadType() );
    header.SetSequenceNumber( frame.GetSequenceNumber() );
    header.SetTimestamp( frame.GetTimestamp() );
    header.SetSsrc( frame.GetStreamId() );
    if ( header.Write( packet, capacity ) < 0 )
        return -1;

    UInt8* at = packet + CRtpHeader::FixedHeaderSize;
    if ( hasRedundancy )
    {
        const UInt32 offset = frame.GetRedundantTimestampOffset();
        const UInt32 length = frame.GetRedundantSize();
        at[ 0 ] = (UInt8) ( 0x80 | frame.GetRedundantPayloadType() );
        at[ 1 ] = (UInt8) ( offset >> 6 );
        at[ 2 ] = (UInt8) ( ( ( offset & 0x3F ) << 2 ) | ( length >> 8 ) );
        at[ 3 ] = (UInt8) ( length & 0xFF );
        at[ 4 ] = frame.GetPayloadType();                       // F bit clear: the primary block
        at += 5;
        memcpy( at, frame.GetRedundantPayload(), length );
        at += length;
    }
    memcpy( at, frame.GetPayload(), frame.GetPayloadSize() );
    return (Int32) total;
}

/*-------------------------------------------------------------------------*/

bool
CRtpVoicePacketCodec::Read( const UInt8* packet, UInt32 size, CVoiceFrame& frame ) const
{
    CRtpHeader header;
    if ( !header.Read( packet, size ) )
        return false;

    const UInt8* data = packet + header.GetPayloadOffset();
    const UInt32 length = header.GetPayloadSize();

    CVoiceFrame parsed;
    parsed.SetStreamId( header.GetSsrc() );
    parsed.SetSequenceNumber( header.GetSequenceNumber() );
    parsed.SetTimestamp( header.GetTimestamp() );
    parsed.SetMarker( header.GetMarker() );

    if ( header.GetPayloadType() != m_redPayloadType )
    {
        if ( 0 == length || !parsed.SetPayload( data, length ) )
            return false;
        parsed.SetPayloadType( header.GetPayloadType() );
        frame = parsed;
        return true;
    }

    // RFC 2198: the headers of the redundant blocks (F bit set, 4 bytes) and then the header of the primary block (F bit clear, 1 byte)
    UInt32 blockOffset[ MaxRedundantBlocks ];
    UInt32 blockLength[ MaxRedundantBlocks ];
    UInt8 blockType[ MaxRedundantBlocks ];
    UInt32 blockTimestamp[ MaxRedundantBlocks ];
    UInt32 blockCount = 0;
    UInt32 at = 0;
    UInt8 primaryType = 0;
    for ( ;; )
    {
        if ( at >= length )
            return false;
        if ( 0 == ( data[ at ] & 0x80 ) )
        {
            primaryType = data[ at ] & 0x7F;
            ++at;
            break;
        }
        if ( at + 4 > length || blockCount >= MaxRedundantBlocks )
            return false;
        blockType[ blockCount ] = data[ at ] & 0x7F;
        blockTimestamp[ blockCount ] = ( (UInt32) data[ at + 1 ] << 6 ) | ( (UInt32) data[ at + 2 ] >> 2 );
        blockLength[ blockCount ] = ( ( (UInt32) data[ at + 2 ] & 0x03 ) << 8 ) | (UInt32) data[ at + 3 ];
        ++blockCount;
        at += 4;
    }

    // the data of the redundant blocks follows in the order of their headers, the primary data is the rest
    for ( UInt32 b=0; b<blockCount; ++b )
    {
        if ( blockLength[ b ] > length - at )
            return false;
        blockOffset[ b ] = at;
        at += blockLength[ b ];
    }
    const UInt32 primaryLength = length - at;
    if ( 0 == primaryLength || !parsed.SetPayload( data + at, primaryLength ) )
        return false;
    parsed.SetPayloadType( primaryType );

    if ( blockCount > 0 )
    {
        // the block nearest to the primary one is the last; an empty one carries nothing
        const UInt32 last = blockCount - 1;
        if ( blockLength[ last ] > 0 && !parsed.SetRedundantBlock( (UInt16) blockTimestamp[ last ], blockType[ last ], data + blockOffset[ last ], blockLength[ last ] ) )
            return false;
    }

    frame = parsed;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
