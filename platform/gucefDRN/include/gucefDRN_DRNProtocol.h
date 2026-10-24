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

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#define GUCEF_DRN_DRNPROTOCOL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** First byte of every DRN packet whose body is plain */
static const UInt8 DRN_PROTOCOL_ID                  = 0xD1;

/** First byte of a packet whose body is encrypted (CDRNPacketProtection), one per key phase */
static const UInt8 DRN_PROTOCOL_ID_PROTECTED_0      = 0xD4;
static const UInt8 DRN_PROTOCOL_ID_PROTECTED_1      = 0xD5;

static const UInt8 DRN_PROTOCOL_VERSION_MAJOR       = 2;
static const UInt8 DRN_PROTOCOL_VERSION_MINOR       = 0;

/** Feature bits announced in HELLO, none are defined for protocol 2.0 */
static const UInt32 DRN_FEATURE_NONE                = 0;

/** What packet protection adds to a packet: the AEAD tag. Kept out of the datagram budget of every link */
static const UInt32 DRN_PACKET_PROTECTION_OVERHEAD  = 16;

/** Size of the policy hash in HELLO and CHALLENGE, of x25519 keys and of the transcript hash */
static const UInt32 DRN_HANDSHAKE_HASH_SIZE         = 32;
static const UInt32 DRN_HANDSHAKE_KEY_SIZE          = 32;

/** Number of packet sequences remembered for detecting duplicates */
static const UInt32 DRN_RECEIVE_WINDOW_SIZE         = 1024;

/** Default budget of a packet on a datagram channel */
static const UInt32 DRN_DEFAULT_DATAGRAM_SIZE       = 1200;

/** Upper bound of the size of an encoded packet header */
static const UInt32 DRN_MAX_PACKET_HEADER_SIZE      = 1 + 5 + 10 + 5 + 4 + 10 + 10;

/** Upper bound of the size of an encoded message header (class, opcode, lane, lane sequence, length) */
static const UInt32 DRN_MAX_MESSAGE_HEADER_SIZE     = 1 + 5 + 1 + 5 + 5;

/** Largest packet a stream channel accepts */
static const UInt32 DRN_MAX_STREAM_PACKET_SIZE      = 16 * 1024 * 1024;

/** Size of the nonce of a CHALLENGE */
static const UInt32 DRN_CHALLENGE_NONCE_SIZE        = 32;

/**
 *  UDP stateless cookies: datagrams of the "udp" channel that are not DRN packets.
 *  HELLO_RETRY   := u8 0xD2, u8 version, u32 window, 16 bytes cookie            (listener to an unknown address)
 *  COOKIE_ECHO   := u8 0xD3, u8 version, u32 window, 16 bytes cookie, packet    (the connecting side, until answered)
 *  cookie = the first 16 bytes of HMAC-SHA256( listener secret, IPv4 address, port, window )
 */
static const UInt8 DRN_UDP_HELLO_RETRY              = 0xD2;
static const UInt8 DRN_UDP_COOKIE_ECHO              = 0xD3;
static const UInt8 DRN_UDP_COOKIE_VERSION           = 1;
static const UInt32 DRN_UDP_COOKIE_MAC_SIZE         = 16;
static const UInt32 DRN_UDP_COOKIE_SIZE             = 1 + 1 + 4 + DRN_UDP_COOKIE_MAC_SIZE;
static const UInt32 DRN_UDP_COOKIE_WINDOW_MS        = 10000;

/** Lane 0 messages a link holds back for later before it is operational */
static const UInt32 DRN_PREAUTH_HOLD_SLOTS          = 8;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Delivery lanes of a link */
enum TDRNLane
{
    DRN_LANE_RELIABLE_ORDERED       = 0,    /**< every message, in order */
    DRN_LANE_RELIABLE_UNORDERED     = 1,    /**< every message, any order */
    DRN_LANE_UNRELIABLE_SEQUENCED   = 2,    /**< newest only, older ones are dropped */
    DRN_LANE_UNRELIABLE             = 3,    /**< as it comes */

    DRN_LANE_COUNT
};

inline bool DRNIsReliableLane( UInt8 lane ) { return lane <= DRN_LANE_RELIABLE_UNORDERED; }

/*-------------------------------------------------------------------------*/

/** Message classes */
enum TDRNMessageClass
{
    DRN_MSGCLASS_LINK       = 1,
    DRN_MSGCLASS_SESSION    = 2,
    DRN_MSGCLASS_REPL       = 3,
    DRN_MSGCLASS_STREAM     = 4,
    DRN_MSGCLASS_BULK       = 5,
    DRN_MSGCLASS_RELAY      = 6,
    DRN_MSGCLASS_APP        = 7,

    DRN_MSGCLASS_COUNT
};

/*-------------------------------------------------------------------------*/

enum TDRNLinkOpcode
{
    DRN_LINK_HELLO          = 1,    /**< { u8 major, u8 minor, varint features, string nodeId, string service, varint linkEpoch, bytes policyHash, bytes ephemeralKey, varint count, string cipher* } */
    DRN_LINK_CHALLENGE      = 2,    /**< { bytes nonce, string nodeId, u8 minor, string authMethod, bytes policyHash, bytes ephemeralKey, bytes staticKey, string cipher } */
    DRN_LINK_AUTH           = 3,    /**< { string method, bytes proof }, encrypted when the handshake agreed on a cipher */
    DRN_LINK_WELCOME        = 4,    /**< { varint linkId, varint linkEpoch }, encrypted when the handshake agreed on a cipher */
    DRN_LINK_REFUSE         = 5,    /**< { varint reason, string text } */
    DRN_LINK_CLOSE          = 6,    /**< { varint reason, string text } */
    DRN_LINK_ECHO           = 7,    /**< empty, keeps the header (acks, times) flowing */
    DRN_LINK_FRAGMENT       = 8     /**< { varint group, varint index, varint count, varint totalSize, u8 class, varint opcode, bytes } */
};

/*-------------------------------------------------------------------------*/

enum TDRNSessionOpcode
{
    DRN_SESSION_JOIN            = 1,    /**< { string session, varint peerId (0 = new member), varint flags, string nodeId, string endpoints } */
    DRN_SESSION_JOINED          = 2,    /**< { varint result, string reason, varint peerId, varint flags, u8 topology, varint sequencerPeerId, varint sessionEpoch } */
    DRN_SESSION_LEAVE           = 3,    /**< { varint peerId, varint reason } */
    DRN_SESSION_MEMBER_ADDED    = 4,    /**< { varint peerId, string nodeId, string endpoints } */
    DRN_SESSION_MEMBER_REMOVED  = 5,    /**< { varint peerId, varint reason } */
    DRN_SESSION_MEMBER_LIST     = 6,    /**< { varint count, ( varint peerId, string nodeId, string endpoints )* } */
    DRN_SESSION_CONNECT_TO      = 7     /**< { varint peerId, string endpoints } */
};

/*-------------------------------------------------------------------------*/

enum TDRNReplOpcode
{
    DRN_REPL_ITEM_ANNOUNCE      = 1,
    DRN_REPL_ITEM_REMOVED       = 2,
    DRN_REPL_SUBSCRIBE          = 3,
    DRN_REPL_UNSUBSCRIBE        = 4,
    DRN_REPL_SNAPSHOT_BEGIN     = 5,
    DRN_REPL_SNAPSHOT_END       = 6,
    DRN_REPL_KEY_DEFINE         = 7,
    DRN_REPL_GRID_DELTA         = 8,
    DRN_REPL_GRID_ACK           = 9,
    DRN_REPL_GRID_PROPOSE       = 10,
    DRN_REPL_PROPOSE_RESULT     = 11,
    DRN_REPL_VOTE_START         = 12,
    DRN_REPL_VOTE_CAST          = 13,
    DRN_REPL_VOTE_END           = 14
};

/*-------------------------------------------------------------------------*/

enum TDRNStreamOpcode
{
    DRN_STREAM_ENTRIES          = 1,
    DRN_STREAM_CREDIT           = 2
};

/*-------------------------------------------------------------------------*/

enum TDRNBulkOpcode
{
    DRN_BULK_OFFER              = 1,
    DRN_BULK_ACCEPT             = 2,
    DRN_BULK_CHUNK              = 3,
    DRN_BULK_DONE               = 4,
    DRN_BULK_CANCEL             = 5
};

/*-------------------------------------------------------------------------*/

enum TDRNRelayOpcode
{
    DRN_RELAY_BIND              = 1,
    DRN_RELAY_FORWARD           = 2,
    DRN_RELAY_UNBIND            = 3
};

/*-------------------------------------------------------------------------*/

enum TDRNAppOpcode
{
    DRN_APP_MESSAGE             = 1     /**< { varint fromPeer, varint toPeer, u8 flags, bytes } */
};

/** Flags of an APP message */
static const UInt8 DRN_APP_FLAG_FORWARD = 0x01;   /**< the receiving sequencer passes the message on to its addressee(s) */
static const UInt8 DRN_APP_FLAG_TO_ALL  = 0x02;   /**< sent to all members, routed to this one member through the sequencer */

/*-------------------------------------------------------------------------*/

/** Why a handshake was refused or a link was closed, as carried by REFUSE and CLOSE */
enum TDRNLinkReason
{
    DRN_LINKREASON_NONE                 = 0,
    DRN_LINKREASON_CLOSED_BY_APP        = 1,    /**< the application closed the link */
    DRN_LINKREASON_PROTOCOL_VERSION     = 2,    /**< a different major protocol version */
    DRN_LINKREASON_SERVICE_MISMATCH     = 3,    /**< a different service name */
    DRN_LINKREASON_AUTH_METHOD          = 4,    /**< a different authentication method */
    DRN_LINKREASON_AUTH_FAILED          = 5,    /**< the proof was not accepted */
    DRN_LINKREASON_HANDSHAKE_TIMEOUT    = 6,    /**< a handshake step did not complete in time */
    DRN_LINKREASON_LINK_TIMEOUT         = 7,    /**< nothing was received for too long */
    DRN_LINKREASON_CHANNEL_LOST         = 8,    /**< the channel was disconnected */
    DRN_LINKREASON_CHANNEL_ERROR        = 9,    /**< the channel reported an error */
    DRN_LINKREASON_PROTOCOL_ERROR       = 10,   /**< a malformed or unexpected packet */
    DRN_LINKREASON_REMOTE_CLOSED        = 11,   /**< the peer sent CLOSE */
    DRN_LINKREASON_REFUSED_BY_PEER      = 12,   /**< the peer sent REFUSE */
    DRN_LINKREASON_SHUTDOWN             = 13,   /**< the node is shutting down */
    DRN_LINKREASON_CONNECT_FAILED       = 14,   /**< the channel could not connect */
    DRN_LINKREASON_LIMIT_EXCEEDED       = 15,   /**< the peer went over a limit of the security policy */
    DRN_LINKREASON_BANNED               = 16,   /**< the peer is on the ban list */
    DRN_LINKREASON_POLICY               = 17,   /**< the security policy does not allow it */
    DRN_LINKREASON_LOCAL_ERROR          = 18,   /**< this side cannot continue safely, for example without randomness */
    DRN_LINKREASON_KEY_MISMATCH         = 19    /**< the peer's key is not the one pinned, or it cannot prove its key */
};

/*-------------------------------------------------------------------------*/

/** Session topologies */
enum TDRNTopology
{
    DRN_TOPOLOGY_CLIENT_SERVER  = 0,
    DRN_TOPOLOGY_PEER_TO_PEER   = 1
};

/** Result codes of JOINED */
enum TDRNJoinResult
{
    DRN_JOIN_OK                 = 0,
    DRN_JOIN_NO_SUCH_SESSION    = 1,
    DRN_JOIN_SESSION_FULL       = 2,
    DRN_JOIN_DENIED             = 3,
    DRN_JOIN_NOT_HOSTING        = 4
};

/** Flags of JOIN / JOINED */
static const UInt32 DRN_JOIN_FLAG_MESH = 0x01;    /**< peer to peer: a member links to another member */

/** Reasons a member left */
enum TDRNLeaveReason
{
    DRN_LEAVE_LEFT              = 0,    /**< the member left */
    DRN_LEAVE_LINK_LOST         = 1,    /**< the link to the member went down */
    DRN_LEAVE_SESSION_CLOSED    = 2     /**< the host / sequencer ended the session */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Wrap-safe comparison of 32 bit sequence numbers: true if a is newer than b */
inline bool DRNIsNewerSequence( UInt32 a, UInt32 b ) { return (Int32)( a - b ) > 0; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */
