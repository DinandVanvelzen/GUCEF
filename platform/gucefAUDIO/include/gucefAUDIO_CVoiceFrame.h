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

#ifndef GUCEF_AUDIO_CVOICEFRAME_H
#define GUCEF_AUDIO_CVOICEFRAME_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

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
 *  One encoded voice frame with the fields that travel with it in an RTP packet (RFC 3550): the stream identity,
 *  the sequence number, the timestamp, the marker and the payload type; and, optionally, the previous frame as an
 *  RFC 2198 redundant block. A value class: copyable, no heap memory (the payloads are inside the object, about
 *  2.9 KB, copying copies only the bytes in use).
 *
 *  The bytes of a payload beyond its size are not initialised.
 */
class GUCEF_AUDIO_PUBLIC_CPP CVoiceFrame
{
    public:

    /** The largest payload: keeps a frame with IP, UDP and RTP headers inside an Ethernet MTU of 1500 bytes */
    static const UInt32 MaxPayloadSize = 1440;

    /** A redundant block is described by 14 bits of timestamp offset and 10 bits of length (RFC 2198) */
    static const UInt32 MaxRedundantTimestampOffset = 16383;
    static const UInt32 MaxRedundantSize = 1023;

    CVoiceFrame( void );

    CVoiceFrame( const CVoiceFrame& src );

    CVoiceFrame& operator=( const CVoiceFrame& src );

    ~CVoiceFrame();

    /** RTP SSRC: chosen by the network layer */
    UInt32 GetStreamId( void ) const;
    void SetStreamId( UInt32 streamId );

    /** RTP sequence number: plus one per sent frame */
    UInt16 GetSequenceNumber( void ) const;
    void SetSequenceNumber( UInt16 sequenceNumber );

    /** RTP timestamp: the first sample of the frame in the timestamp rate of the codec */
    UInt32 GetTimestamp( void ) const;
    void SetTimestamp( UInt32 timestamp );

    /** True for the first frame of a talkspurt */
    bool GetMarker( void ) const;
    void SetMarker( bool marker );

    /** RTP payload type of the payload, 0 to 127 */
    UInt8 GetPayloadType( void ) const;
    void SetPayloadType( UInt8 payloadType );

    /** The encoded frame */
    const UInt8* GetPayload( void ) const;
    UInt32 GetPayloadSize( void ) const;

    /** Sets the encoded frame. False, and nothing changed, when it is larger than MaxPayloadSize */
    bool SetPayload( const UInt8* data, UInt32 size );

    /** The buffer of MaxPayloadSize bytes to encode into, followed by SetPayloadSize() */
    UInt8* GetPayloadBuffer( void );
    bool SetPayloadSize( UInt32 size );

    /** True when the frame carries a redundant block */
    bool HasRedundantBlock( void ) const;

    /** The timestamp distance in timestamp units from this frame back to the redundant frame (0 to MaxRedundantTimestampOffset) */
    UInt16 GetRedundantTimestampOffset( void ) const;
    UInt8 GetRedundantPayloadType( void ) const;
    const UInt8* GetRedundantPayload( void ) const;
    UInt32 GetRedundantSize( void ) const;

    /**
     *  Sets the redundant block: the previous frame as it was encoded. False, and nothing changed, when the block is
     *  larger than MaxRedundantSize, empty, or the offset or the payload type are out of range
     */
    bool SetRedundantBlock( UInt16 timestampOffset, UInt8 payloadType, const UInt8* data, UInt32 size );

    void ClearRedundantBlock( void );

    /** Sender side only, not on the wire: the tick count of the capture time of the frame, for latency statistics */
    UInt64 GetCaptureTickCount( void ) const;
    void SetCaptureTickCount( UInt64 ticks );

    /** Clears everything, the redundant block and the capture time included */
    void Clear( void );

    private:

    UInt32 m_streamId;
    UInt32 m_timestamp;
    UInt64 m_captureTickCount;
    UInt32 m_payloadSize;
    UInt32 m_redundantSize;
    UInt16 m_sequenceNumber;
    UInt16 m_redundantTimestampOffset;
    UInt8 m_payloadType;
    UInt8 m_redundantPayloadType;
    bool m_marker;
    UInt8 m_payload[ MaxPayloadSize ];
    UInt8 m_redundantPayload[ MaxPayloadSize ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CVOICEFRAME_H ? */
