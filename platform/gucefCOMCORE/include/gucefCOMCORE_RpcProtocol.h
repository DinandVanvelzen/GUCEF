/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_RPCPROTOCOL_H
#define GUCEF_COMCORE_RPCPROTOCOL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COMCORE_RPCTYPES_H
#include "gucefCOMCORE_RpcTypes.h"
#define GUCEF_COMCORE_RPCTYPES_H
#endif /* GUCEF_COMCORE_RPCTYPES_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTHASH_H
#include "gucefCOMCORE_CRpcContractHash.h"
#define GUCEF_COMCORE_CRPCCONTRACTHASH_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTHASH_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Constants and plain data of the RPC wire protocol (gucefCOMCORE RPC design 6). Protocol 1.0.
 */

#define GUCEF_RPC_PROTOCOL_MAJOR                1
#define GUCEF_RPC_PROTOCOL_MINOR                0

/* frame types (6.3) */
#define GUCEF_RPC_FRAME_HELLO                   0x01
#define GUCEF_RPC_FRAME_HELLO_ACK               0x02
#define GUCEF_RPC_FRAME_ACK                     0x03
#define GUCEF_RPC_FRAME_PING                    0x04
#define GUCEF_RPC_FRAME_PONG                    0x05
#define GUCEF_RPC_FRAME_STREAM_CREDIT           0x06
#define GUCEF_RPC_FRAME_ABORT                   0x07
#define GUCEF_RPC_FRAME_GET_CONTRACT            0x10
#define GUCEF_RPC_FRAME_CONTRACT                0x11
#define GUCEF_RPC_FRAME_BIND                    0x12
#define GUCEF_RPC_FRAME_BIND_ACK                0x13
#define GUCEF_RPC_FRAME_REQUEST                 0x20
#define GUCEF_RPC_FRAME_RESPONSE                0x21
#define GUCEF_RPC_FRAME_ACCEPTED                0x22
#define GUCEF_RPC_FRAME_RESULT                  0x23
#define GUCEF_RPC_FRAME_STREAM_MSG              0x24
#define GUCEF_RPC_FRAME_HALF_CLOSE              0x25
#define GUCEF_RPC_FRAME_CANCEL                  0x26
#define GUCEF_RPC_FRAME_STATUS_QUERY            0x27
#define GUCEF_RPC_FRAME_STATUS                  0x28
#define GUCEF_RPC_FRAME_CONTINUATION            0x29
#define GUCEF_RPC_FRAME_ERROR                   0x2A
#define GUCEF_RPC_FRAME_GOODBYE                 0x2B

/* frame flags (6.2) */
#define GUCEF_RPC_FRAMEFLAG_ACK                 0x01
#define GUCEF_RPC_FRAMEFLAG_RESPONDER           0x02
#define GUCEF_RPC_FRAMEFLAG_MORE                0x04
#define GUCEF_RPC_FRAMEFLAG_NO_MESSAGE          0x08
#define GUCEF_RPC_FRAMEFLAG_RESERVED_MASK       0xF0

/* REQUEST call flags (6.5.1) */
#define GUCEF_RPC_CALLFLAG_RESUME_FROM          0x01

/* ACCEPTED flags (6.3) */
#define GUCEF_RPC_ACCEPTFLAG_DURABLE            0x01

/* HELLO modes and HELLO_ACK results (6.4) */
#define GUCEF_RPC_HELLO_NEW                     0
#define GUCEF_RPC_HELLO_RESUME                  1
#define GUCEF_RPC_HELLOACK_NEW                  0
#define GUCEF_RPC_HELLOACK_RESUMED              1
#define GUCEF_RPC_HELLOACK_RESUME_GAP           2
#define GUCEF_RPC_HELLOACK_SESSION_UNKNOWN      3
#define GUCEF_RPC_HELLOACK_REFUSED              4

#define GUCEF_RPC_SESSIONID_SIZE                16
#define GUCEF_RPC_RESUMETOKEN_SIZE              32
#define GUCEF_RPC_NONCE_SIZE                    16
#define GUCEF_RPC_PROOF_SIZE                    32

/* the bind index of gucef.rpc, bound implicitly in every session (6.4.3) */
#define GUCEF_RPC_BINDINDEX_RPC                 0

/* first interned metadata key index (6.9) */
#define GUCEF_RPC_METADATA_FIRST_INTERNED       64

/*-------------------------------------------------------------------------*/

/** The limits a side announces in HELLO / HELLO_ACK (6.4.1); 0 = no maximum where that is allowed */
struct GUCEF_COMCORE_EXPORT_CPP SRpcLimits
{
    UInt32 maxFrameSize;
    UInt32 maxMessageSize;
    UInt32 receiveWindowBytes;
    UInt32 initialStreamCredit;
    UInt32 maxConcurrentCalls;
    UInt32 maxBindings;
    UInt32 resumeGraceMs;

    SRpcLimits( void );
};

/** A contract a side exports (HELLO offers) */
struct GUCEF_COMCORE_EXPORT_CPP SRpcContractOffer
{
    CORE::CString name;
    UInt32 major;
    UInt32 minor;
    CRpcContractHash hash;

    SRpcContractOffer( void );
};
typedef GUCEF::vector< SRpcContractOffer > TRpcContractOfferVector;

/** A contract a side intends to call (HELLO wants) */
struct GUCEF_COMCORE_EXPORT_CPP SRpcContractWant
{
    CORE::CString name;
    UInt32 major;
    UInt32 minMinor;

    SRpcContractWant( void );
};
typedef GUCEF::vector< SRpcContractWant > TRpcContractWantVector;

/** A parsed frame header (6.2) */
struct GUCEF_COMCORE_EXPORT_CPP SRpcFrameHeader
{
    UInt8 type;
    UInt8 flags;
    UInt64 seq;
    UInt64 ackSeq;
    bool hasAck;
    UInt32 bodyOffset;      /**< where the body starts in the frame */

    SRpcFrameHeader( void );

    bool IsControl( void ) const { return 0 == seq; }
    bool HasFlag( UInt8 flag ) const { return 0 != ( flags & flag ); }
};

/** True for the control frame types (6.3), which are never sequenced */
GUCEF_COMCORE_EXPORT_CPP bool RpcFrameTypeIsControl( UInt8 frameType );

/** "REQUEST", "ACK", ... for logs */
GUCEF_COMCORE_EXPORT_CPP const char* RpcFrameTypeToString( UInt8 frameType );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_RPCPROTOCOL_H ? */
