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

#ifndef GUCEF_DRN_CDRNPACKETCODEC_H
#define GUCEF_DRN_CDRNPACKETCODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#include "gucefDRN_DRNProtocol.h"
#define GUCEF_DRN_DRNPROTOCOL_H
#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */

#ifndef GUCEF_DRN_CDRNWIREWRITER_H
#include "gucefDRN_CDRNWireWriter.h"
#define GUCEF_DRN_CDRNWIREWRITER_H
#endif /* GUCEF_DRN_CDRNWIREWRITER_H ? */

#ifndef GUCEF_DRN_CDRNWIREREADER_H
#include "gucefDRN_CDRNWireReader.h"
#define GUCEF_DRN_CDRNWIREREADER_H
#endif /* GUCEF_DRN_CDRNWIREREADER_H ? */

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
 *  The header every DRN packet starts with (after the protocol byte)
 */
class GUCEF_DRN_EXPORT_CPP CDRNPacketHeader
{
    public:

    UInt32 sequence;        /**< per link, per direction, wraps at 2^32 */
    UInt64 sendTime;        /**< microseconds of the sender's link clock */
    UInt32 ackSequence;     /**< highest sequence received from the peer, 0 = none */
    UInt32 ackBits;         /**< bit n = sequence ackSequence-1-n was received */
    UInt64 echoTime;        /**< the sendTime of the packet with ackSequence as the peer sent it */
    UInt64 echoHold;        /**< microseconds between receiving that packet and sending this one */

    CDRNPacketHeader( void );
    void Clear( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  A view on one decoded message of a packet. The payload points into the packet buffer.
 */
class GUCEF_DRN_EXPORT_CPP CDRNMessageView
{
    public:

    UInt8 msgClass;
    UInt32 opcode;
    UInt8 lane;
    UInt32 laneSequence;    /**< only meaningful on the reliable lanes */
    const UInt8* payload;
    UInt32 payloadSize;

    CDRNMessageView( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  Encodes and decodes packet headers and messages
 *
 *  packet  := header message*
 *  header  := u8 protocol, varint sequence, varint sendTime, varint ackSequence, u32 ackBits, varint echoTime, varint echoHold
 *  message := u8 class, varint opcode, u8 lane, [varint laneSequence if the lane is reliable], varint length, payload
 */
class GUCEF_DRN_EXPORT_CPP CDRNPacketCodec
{
    public:

    static void WriteHeader( CDRNWireWriter& writer, const CDRNPacketHeader& header );

    /** Reads and checks the protocol byte and the header fields */
    static bool ReadHeader( CDRNWireReader& reader, CDRNPacketHeader& header );

    static void WriteMessage( CDRNWireWriter& writer   ,
                              UInt8 msgClass           ,
                              UInt32 opcode            ,
                              UInt8 lane               ,
                              UInt32 laneSequence      ,
                              const void* payload      ,
                              UInt32 payloadSize       );

    static bool ReadMessage( CDRNWireReader& reader, CDRNMessageView& message );

    /** The exact encoded size of a message */
    static UInt32 GetMessageSize( UInt32 opcode, UInt8 lane, UInt32 laneSequence, UInt32 payloadSize );

    private:

    CDRNPacketCodec( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNPACKETCODEC_H ? */
