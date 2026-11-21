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

/**
 *  First byte of a packet whose body is clear and authenticated by the link's AEAD with an empty plaintext (the tag
 *  covers header and body), one per key phase; and of a packet of PUBLIC lane 3 messages without any protection,
 *  which has a sequence space of its own and never acknowledges anything
 */
static const UInt8 DRN_PROTOCOL_ID_MAC_0            = 0xD6;
static const UInt8 DRN_PROTOCOL_ID_MAC_1            = 0xD7;
static const UInt8 DRN_PROTOCOL_ID_PUBLIC_PLAIN     = 0xD8;

static const UInt8 DRN_PROTOCOL_VERSION_MAJOR       = 2;
static const UInt8 DRN_PROTOCOL_VERSION_MINOR       = 3;   /**< 1: the RPC message class (DRN_MSGCLASS_RPC); 2: resource transfers (DRN_MSGCLASS_RESOURCE); 3: namespace operations of the resource class (DRN_RES_NS_*) */

/** The first minor of protocol 2 whose nodes take DRN_MSGCLASS_RPC */
static const UInt8 DRN_PROTOCOL_MINOR_RPC           = 1;

/** The first minor of protocol 2 whose nodes take DRN_MSGCLASS_RESOURCE */
static const UInt8 DRN_PROTOCOL_MINOR_RESOURCE      = 2;

/** The first minor of protocol 2 whose nodes take the namespace opcodes (DRN_RES_NS_*) of DRN_MSGCLASS_RESOURCE */
static const UInt8 DRN_PROTOCOL_MINOR_NAMESPACE     = 3;

/** Feature bits announced in HELLO, none are defined for protocol 2.0 */
static const UInt32 DRN_FEATURE_NONE                = 0;

/** What packet protection adds to a packet: the AEAD tag. Kept out of the datagram budget of every link */
static const UInt32 DRN_PACKET_PROTECTION_OVERHEAD  = 16;

/** Size of the policy hash in HELLO and CHALLENGE, of x25519 keys and of the transcript hash */
static const UInt32 DRN_HANDSHAKE_HASH_SIZE         = 32;
static const UInt32 DRN_HANDSHAKE_KEY_SIZE          = 32;

/** Number of packet sequences remembered for detecting duplicates */
static const UInt32 DRN_RECEIVE_WINDOW_SIZE         = 1024;

/**
 *  The window of a reliable lane on a datagram channel, a power of 2: a sender sends a message of the lane only while
 *  its lane sequence is less than this beyond the oldest message of the lane not acknowledged yet, and a receiver
 *  takes (holds back on lane 0, remembers on lane 1) every lane sequence less than this beyond the oldest one it has
 *  not received yet. A packet with a message beyond that is not taken and not acknowledged.
 */
static const UInt32 DRN_RELIABLE_LANE_WINDOW        = DRN_RECEIVE_WINDOW_SIZE;

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

/**
 *  How a packet is protected on a protected link, from strongest to weakest. A packet carries messages of one kind
 *  only; a message received has the kind of its packet (of the weakest of its fragments)
 */
enum TDRNPacketKind
{
    DRN_PACKETKIND_AEAD     = 0,    /**< encrypted and authenticated (0xD4 / 0xD5) */
    DRN_PACKETKIND_MAC      = 1,    /**< clear, authenticated (0xD6 / 0xD7) */
    DRN_PACKETKIND_PLAIN    = 2,    /**< clear, not authenticated, lane 3 only (0xD8) */

    DRN_PACKETKIND_COUNT    = 3
};

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
    DRN_MSGCLASS_RPC        = 8,    /**< frames of the GUCEF RPC protocol (CDRNRpcSessionComponent), protocol 2.1 */
    DRN_MSGCLASS_RESOURCE   = 9,    /**< resource transfers (CDRNResourceSessionComponent), protocol 2.2 */

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

/**
 *  RPC messages (DRN_MSGCLASS_RPC, lane 0, direct links only). The opcode names the RPC channel between the two
 *  members: ( channel << 1 ) | DRN_RPC_OPCODE_FROM_ACCEPTOR, the channel number given by the member that opened it
 *  (from 1). The payload is one RPC frame, unchanged; an empty payload closes the channel. Opcode DRN_RPC_OPCODE_OPEN
 *  opens a channel: { varint channel }, sent by the member that opens it before its first frame; the receiver's
 *  listener accepts it or closes it.
 */
static const UInt32 DRN_RPC_OPCODE_OPEN             = 0x00;   /**< { varint channel }: the sender opened the channel */
static const UInt32 DRN_RPC_OPCODE_FROM_ACCEPTOR    = 0x01;   /**< sent by the member that did not open the channel */

inline UInt32 DRNRpcOpcode( UInt32 channel, bool fromAcceptor ) { return ( channel << 1 ) | ( fromAcceptor ? DRN_RPC_OPCODE_FROM_ACCEPTOR : 0 ); }
inline UInt32 DRNRpcChannel( UInt32 opcode ) { return opcode >> 1; }
inline bool DRNRpcIsFromAcceptor( UInt32 opcode ) { return 0 != ( opcode & DRN_RPC_OPCODE_FROM_ACCEPTOR ); }

/*-------------------------------------------------------------------------*/

/**
 *  Resource transfer messages (DRN_MSGCLASS_RESOURCE, direct links only). R is the requesting member, S the serving one.
 *  Handles are chosen by R, unique per link; read ids per handle. Credit is counted in chunks of DRN_RESOURCE_CHUNK_SIZE.
 */
enum TDRNResourceOpcode
{
    DRN_RES_OPEN                = 1,    /**< R to S, lane 0: { varint handle, string provider, string name, varint flags, string expectedRevision } */
    DRN_RES_READ                = 2,    /**< R to S, lane 0: { varint handle, varint readId, varint64 firstChunk, varint64 chunkCount, varint credit } */
    DRN_RES_CREDIT              = 3,    /**< R to S, lane 0: { varint handle, varint readId, varint chunks } */
    DRN_RES_CANCEL              = 4,    /**< R to S, lane 0: { varint handle, varint readId } */
    DRN_RES_CLOSE               = 5,    /**< R to S, lane 0: { varint handle } */
    DRN_RES_INFO                = 6,    /**< S to R, lane 0: { varint handle, varint status, string message [, varint64 size, varint64 modifiedUtcMs, string revision, varint chunkSize, u8 digestFlags, bytes32 sha256?, bytes32 treeRoot?] } */
    DRN_RES_DATA                = 7,    /**< S to R, lane 1: { varint handle, varint readId, varint64 chunk, u8 pathLength, bytes32 path*, bytes data } */
    DRN_RES_READ_END            = 8     /**< S to R, lane 0: { varint handle, varint readId, varint status, string message }: a read that failed */
};

/**
 *  Namespace messages of DRN_MSGCLASS_RESOURCE (protocol 2.3): browsing what a provider shares (shares, stat, list)
 *  without opening anything. R is the requesting member, S the serving one; every message is on lane 0 and every
 *  request carries a varint requestId chosen by R, unique per member and run, which the answer repeats.
 *
 *  The numbers 16 to 63 belong to the namespace operations: 16 to 23 requests that read, 24 to 31 their answers, 32 to 47
 *  requests that change (reserved, a serving side answers them READ_ONLY), 48 to 63 for later (watches, versions).
 */
enum TDRNResourceNamespaceOpcode
{
    DRN_RES_NS_HELLO            = 16,   /**< R to S: { varint requestId, varint maxVersion, varint clientCaps } */
    DRN_RES_NS_SHARES           = 17,   /**< R to S: { varint requestId } */
    DRN_RES_NS_STAT             = 18,   /**< R to S: { varint requestId, string share, varint count, ( string path, string ifRevisionNot )* count } */
    DRN_RES_NS_LIST             = 19,   /**< R to S: { varint requestId, string share, string dir, bytes cursor, varint maxEntries, string ifRevisionNot } */
    DRN_RES_NS_ANSWER           = 24,   /**< S to R: { varint requestId, varint kind (the opcode answered), varint status, string message, body of the kind (status OK, for LIST also UNCHANGED) } */

    DRN_RES_NS_MUTATION_FIRST   = 32,   /**< first opcode of the range reserved for requests that change a share (create, write, delete, rename, make directory) */
    DRN_RES_NS_MUTATION_LAST    = 47,
    DRN_RES_NS_RESERVED_FIRST   = 48,   /**< reserved: watch, unwatch, changes, list versions */
    DRN_RES_NS_RESERVED_LAST    = 63
};

/** The version of the namespace protocol this build speaks, negotiated in NS_HELLO (the lower of both sides) */
static const UInt32 DRN_NS_VERSION                  = 1;

/**
 *  Capabilities of a share (CIDRNResourceProvider::GetNamespaceCapabilities(), per share in the SHARES answer) and of a
 *  serving side (NS_HELLO answer). A requester asks only for what a share advertises.
 */
static const UInt32 DRN_NS_CAP_READ                 = 0x0001;   /**< resources can be opened by name (OPEN) */
static const UInt32 DRN_NS_CAP_LIST                 = 0x0002;
static const UInt32 DRN_NS_CAP_STAT                 = 0x0004;
static const UInt32 DRN_NS_CAP_WATCH                = 0x0008;   /**< reserved: changes are pushed */
static const UInt32 DRN_NS_CAP_VERSIONS             = 0x0010;   /**< reserved: older revisions are kept and can be read */
static const UInt32 DRN_NS_CAP_WRITE                = 0x0100;   /**< reserved */
static const UInt32 DRN_NS_CAP_CREATE               = 0x0200;   /**< reserved */
static const UInt32 DRN_NS_CAP_DELETE               = 0x0400;   /**< reserved */
static const UInt32 DRN_NS_CAP_RENAME               = 0x0800;   /**< reserved */
static const UInt32 DRN_NS_CAP_MKDIR                = 0x1000;   /**< reserved */

/** Flags of a share in the SHARES answer */
static const UInt32 DRN_NS_SHAREFLAG_READ_ONLY      = 0x01;     /**< nothing can be changed through the share (always set in this version) */
static const UInt32 DRN_NS_SHAREFLAG_WEAK_REVISION  = 0x02;     /**< the share has no modification times: a revision is "<size>-0", a change of content at the same size goes unnoticed */

/** What a namespace entry is */
enum TDRNNamespaceEntryKind
{
    DRN_NS_KIND_FILE        = 1,
    DRN_NS_KIND_DIRECTORY   = 2
};

/** Flags of an entry in a STAT or LIST answer */
static const UInt32 DRN_NS_ENTRYFLAG_SHA256         = 0x01;     /**< the provider knows the SHA-256 of the file: it follows the revision */
static const UInt32 DRN_NS_ENTRYFLAG_READ_ONLY      = 0x02;     /**< the entry itself is marked read-only */
static const UInt32 DRN_NS_ENTRYFLAG_HIDDEN         = 0x04;     /**< the file system marks the entry hidden */
static const UInt32 DRN_NS_ENTRYFLAG_WEAK_REVISION  = 0x08;     /**< the revision of the entry holds no modification time */
static const UInt32 DRN_NS_ENTRYFLAG_SUB_ARCHIVE    = 0x10;     /**< reserved: a file that is also a directory (an archive the serving side walks into) */

/** Flags of OPEN */
static const UInt32 DRN_RES_OPEN_FLAG_TREE      = 0x01;     /**< the requester wants the hash tree root in INFO and an audit path with every chunk */

/** Digest flags of INFO */
static const UInt8 DRN_RES_DIGEST_SHA256        = 0x01;     /**< the SHA-256 of the whole resource follows */
static const UInt8 DRN_RES_DIGEST_TREE          = 0x02;     /**< the root of the hash tree follows, chunks carry audit paths */

/** The unit of transfer, verification and resume bookkeeping */
static const UInt32 DRN_RESOURCE_CHUNK_SIZE     = 64 * 1024;

/** Statuses of resource transfers, on the wire as a varint. Values are never reused */
enum TDRNResourceStatus
{
    DRN_RESSTATUS_OK                = 0,
    DRN_RESSTATUS_NOT_FOUND         = 1,    /**< no such resource, or one the provider does not expose */
    DRN_RESSTATUS_FORBIDDEN         = 2,    /**< the member may not read from the provider */
    DRN_RESSTATUS_INVALID_NAME      = 3,    /**< the name is not a safe relative name */
    DRN_RESSTATUS_NO_SUCH_PROVIDER  = 4,
    DRN_RESSTATUS_BUSY              = 5,    /**< a limit of the serving side, try again later */
    DRN_RESSTATUS_CHANGED           = 6,    /**< the resource is not the revision that was expected or opened */
    DRN_RESSTATUS_OUT_OF_RANGE      = 7,
    DRN_RESSTATUS_IO_ERROR          = 8,    /**< the serving side could not read */
    DRN_RESSTATUS_DIGEST_MISMATCH   = 9,    /**< what arrived does not match its digest */
    DRN_RESSTATUS_CANCELLED         = 10,
    DRN_RESSTATUS_LINK_LOST         = 11,   /**< the member left, the link or the session closed */
    DRN_RESSTATUS_UNSUPPORTED       = 12,   /**< the member's node does not take resource transfers (protocol below 2.2) */
    DRN_RESSTATUS_INSECURE_LINK     = 13,   /**< the link has no packet protection and the settings require it */
    DRN_RESSTATUS_TIMEOUT           = 14,
    DRN_RESSTATUS_LOCAL_IO_ERROR    = 15,   /**< this side could not write or read its target */
    DRN_RESSTATUS_PROTOCOL_ERROR    = 16,
    DRN_RESSTATUS_TOO_LARGE         = 17,   /**< the resource does not fit the target */
    DRN_RESSTATUS_CLOSED            = 18,   /**< the handle or read is not open (any more) */
    DRN_RESSTATUS_NO_TREE           = 19,   /**< verification by hash tree was required, the serving side has none */

    // namespace operations (protocol 2.3)
    DRN_RESSTATUS_UNCHANGED         = 20,   /**< the revision the requester holds (ifRevisionNot) is the current one: no entries follow */
    DRN_RESSTATUS_CURSOR_EXPIRED    = 21,   /**< the listing cursor is unknown, expired or not the next one: list again from the start */
    DRN_RESSTATUS_NOT_A_DIRECTORY   = 22,   /**< a listing of a file */

    // reserved for writes, nothing produces them yet except READ_ONLY (a request that changes a share)
    DRN_RESSTATUS_READ_ONLY         = 23,   /**< the share cannot be changed */
    DRN_RESSTATUS_CONFLICT          = 24,   /**< reserved: a change against a revision that moved on */
    DRN_RESSTATUS_ALREADY_EXISTS    = 25,   /**< reserved */
    DRN_RESSTATUS_NOT_EMPTY         = 26,   /**< reserved */
    DRN_RESSTATUS_NO_SPACE          = 27,   /**< reserved */

    DRN_RESSTATUS_COUNT
};

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
