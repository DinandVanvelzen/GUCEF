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

#ifndef GUCEF_AUDIO_CRTPHEADER_H
#define GUCEF_AUDIO_CRTPHEADER_H

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
 *  The fixed header of an RTP packet, RFC 3550 section 5.1: version 2, padding, extension, CSRC count, marker,
 *  payload type, sequence number, timestamp and SSRC, in network byte order.
 *
 *  Write() writes the plain header of 12 bytes: no padding, no extension, no CSRC list. Read() accepts all of
 *  these in a packet and skips them: it counts the CSRC entries, notes the extension and the padding and tells where
 *  the payload is and how long it is. Both validate the version, the lengths and the ranges and fail without
 *  changing anything.
 */
class GUCEF_AUDIO_PUBLIC_CPP CRtpHeader
{
    public:

    static const UInt32 FixedHeaderSize = 12;
    static const UInt8 RtpVersion = 2;

    CRtpHeader( void );

    bool GetMarker( void ) const;
    void SetMarker( bool marker );

    /** 0 to 127 */
    UInt8 GetPayloadType( void ) const;
    void SetPayloadType( UInt8 payloadType );

    UInt16 GetSequenceNumber( void ) const;
    void SetSequenceNumber( UInt16 sequenceNumber );

    UInt32 GetTimestamp( void ) const;
    void SetTimestamp( UInt32 timestamp );

    /** The synchronisation source */
    UInt32 GetSsrc( void ) const;
    void SetSsrc( UInt32 ssrc );

    /**
     *  Writes the 12 bytes. Returns the bytes written, or -1 when the capacity is too small or the payload type
     *  is above 127
     */
    Int32 Write( UInt8* buffer, UInt32 capacity ) const;

    /**
     *  Reads the header of a whole packet of 'size' bytes. False, and the object unchanged, when the packet is
     *  shorter than its header says (CSRC list, extension), the version is not 2, or the padding count does not fit.
     *  On success the fields are set, as are the values below
     */
    bool Read( const UInt8* packet, UInt32 size );

    /** After Read(): the P bit was set */
    bool HasPadding( void ) const;

    /** After Read(): the X bit was set (the extension is skipped) */
    bool HasExtension( void ) const;

    /** After Read(): the number of CSRC entries that were skipped */
    UInt32 GetCsrcCount( void ) const;

    /** After Read(): the offset of the payload in the packet: the fixed header, the CSRC list and the extension */
    UInt32 GetPayloadOffset( void ) const;

    /** After Read(): the bytes of the payload, without the padding */
    UInt32 GetPayloadSize( void ) const;

    /** After Read(): the bytes of padding at the end of the packet, the count byte included */
    UInt32 GetPaddingSize( void ) const;

    private:

    UInt32 m_timestamp;
    UInt32 m_ssrc;
    UInt32 m_payloadOffset;
    UInt32 m_payloadSize;
    UInt32 m_paddingSize;
    UInt16 m_sequenceNumber;
    UInt8 m_payloadType;
    UInt8 m_csrcCount;
    bool m_marker;
    bool m_hasPadding;
    bool m_hasExtension;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CRTPHEADER_H ? */
