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

#ifndef GUCEF_AUDIO_CRTPVOICEPACKETCODEC_H
#define GUCEF_AUDIO_CRTPVOICEPACKETCODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CVOICEFRAME_H
#include "gucefAUDIO_CVoiceFrame.h"
#define GUCEF_AUDIO_CVOICEFRAME_H
#endif /* GUCEF_AUDIO_CVOICEFRAME_H ? */

#ifndef GUCEF_AUDIO_CRTPHEADER_H
#include "gucefAUDIO_CRtpHeader.h"
#define GUCEF_AUDIO_CRTPHEADER_H
#endif /* GUCEF_AUDIO_CRTPHEADER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Turns a CVoiceFrame into an RTP packet (RFC 3550) and back. Pure code on byte buffers: no sockets.
 *
 *  Without a redundant block the RTP payload is the encoded frame (RFC 7587 for Opus: one Opus packet per RTP packet;
 *  RFC 3551 for L16) and the payload type of the RTP header is the payload type of the frame.
 *
 *  With a redundant block the payload type of the RTP header is the RED payload type of the codec (default 113) and
 *  the payload is RFC 2198: a header of 4 bytes for the redundant block (F bit set, block payload type, 14 bit
 *  timestamp offset, 10 bit block length), a header of 1 byte for the primary block (F bit clear, payload type),
 *  the data of the redundant block and the data of the primary block. Read() understands packets with more than
 *  one redundant block and returns the one nearest to the primary block (the sequence number one lower); the
 *  older blocks are checked and dropped.
 *
 *  Malformed input is refused: Read() returns false and does not touch its output.
 */
class GUCEF_AUDIO_PUBLIC_CPP CRtpVoicePacketCodec
{
    public:

    /** The largest packet Write() makes: header, RED headers, a redundant block of the largest size and a primary of the largest size */
    static const UInt32 MaxPacketSize = CRtpHeader::FixedHeaderSize + 4 + 1 + CVoiceFrame::MaxRedundantSize + CVoiceFrame::MaxPayloadSize;

    /** The most redundant blocks Read() accepts in one packet */
    static const UInt32 MaxRedundantBlocks = 4;

    CRtpVoicePacketCodec( void );

    /** The payload type of packets that carry redundancy (RFC 2198), 0 to 127, default 113 */
    UInt8 GetRedPayloadType( void ) const;
    void SetRedPayloadType( UInt8 payloadType );

    /**
     *  Writes the frame as a packet. Returns the bytes written, or -1 when the capacity is too small or the frame
     *  cannot be sent (payload type above 127, the redundant block does not fit the fields of RFC 2198, no payload)
     */
    Int32 Write( const CVoiceFrame& frame, UInt8* packet, UInt32 capacity ) const;

    /**
     *  Reads a packet of 'size' bytes into 'frame'. The capture time is cleared. False when the packet is not a valid
     *  voice packet, in which case 'frame' is untouched
     */
    bool Read( const UInt8* packet, UInt32 size, CVoiceFrame& frame ) const;

    private:

    UInt8 m_redPayloadType;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CRTPVOICEPACKETCODEC_H ? */
