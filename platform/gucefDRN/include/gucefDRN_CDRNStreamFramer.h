/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNSTREAMFRAMER_H
#define GUCEF_DRN_CDRNSTREAMFRAMER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#include "gucefDRN_DRNProtocol.h"
#define GUCEF_DRN_DRNPROTOCOL_H
#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Frames DRN packets on a byte stream: [0x47 0x44][varint length][packet]
 *
 *  Receiving: hand every chunk read from the stream to BeginChunk(), take the complete packets with NextPacket()
 *  and finish with EndChunk(). Packets that are complete within a chunk are handed out without copying; only the
 *  bytes of a packet that is split over chunks are kept in an internal buffer which is reused.
 */
class GUCEF_DRN_EXPORT_CPP CDRNStreamFramer
{
    public:

    static const UInt8 FrameMagic0 = 0x47;
    static const UInt8 FrameMagic1 = 0x44;
    static const UInt32 MaxFrameHeaderSize = 2 + 5;

    enum TFrameResult
    {
        FRAME_NEED_MORE_DATA ,
        FRAME_PACKET         ,
        FRAME_ERROR                 /**< bad magic or a packet over the size limit: the stream is unusable */
    };

    explicit CDRNStreamFramer( UInt32 maxPacketSize = DRN_MAX_STREAM_PACKET_SIZE );

    /** Writes the frame header for a packet of the given size, dest must hold MaxFrameHeaderSize bytes */
    static UInt32 WriteFrameHeader( UInt32 packetSize, UInt8* dest );

    void BeginChunk( const void* data, UInt32 size );

    /** The packet view stays valid until the next call */
    TFrameResult NextPacket( const UInt8*& packet, UInt32& packetSize );

    void EndChunk( void );

    void Reset( void );

    /** The largest packet taken from now on, a larger one is a FRAME_ERROR */
    void SetMaxPacketSize( UInt32 maxPacketSize );

    UInt32 GetMaxPacketSize( void ) const;

    /** Bytes held back waiting for the rest of a packet */
    UInt32 GetPendingByteCount( void ) const;

    private:

    UInt32 m_maxPacketSize;
    CORE::CDynamicBuffer m_pending;     /**< bytes of an incomplete frame */
    const UInt8* m_src;
    UInt32 m_srcSize;
    UInt32 m_srcPos;
    bool m_parsingPending;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNSTREAMFRAMER_H ? */
