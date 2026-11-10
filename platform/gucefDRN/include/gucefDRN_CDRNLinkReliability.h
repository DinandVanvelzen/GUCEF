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

#ifndef GUCEF_DRN_CDRNLINKRELIABILITY_H
#define GUCEF_DRN_CDRNLINKRELIABILITY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CDRNPACKETCODEC_H
#include "gucefDRN_CDRNPacketCodec.h"
#define GUCEF_DRN_CDRNPACKETCODEC_H
#endif /* GUCEF_DRN_CDRNPACKETCODEC_H ? */

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
 *  Remembers which of the last 1024 sequence numbers were seen, wrap-safe
 */
class GUCEF_DRN_EXPORT_CPP CDRNSequenceWindow
{
    public:

    static const UInt32 WindowSize = DRN_RECEIVE_WINDOW_SIZE;

    enum TAcceptResult
    {
        SEQUENCE_NEWEST     ,   /**< newer than all before */
        SEQUENCE_LATE       ,   /**< older than the newest but not seen before (reordered) */
        SEQUENCE_DUPLICATE  ,   /**< seen before */
        SEQUENCE_TOO_OLD        /**< older than the window */
    };

    CDRNSequenceWindow( void );

    void Reset( void );

    /** @param gap set to the number of sequences skipped when the result is SEQUENCE_NEWEST */
    TAcceptResult Accept( UInt32 sequence, UInt32& gap );

    bool HasAny( void ) const;
    UInt32 GetHighest( void ) const;

    /** bit n = sequence highest-1-n was seen */
    UInt32 GetAckBits( void ) const;

    private:

    bool IsSet( UInt32 sequence ) const;
    void Set( UInt32 sequence );
    void Unset( UInt32 sequence );

    bool m_any;
    UInt32 m_highest;
    UInt32 m_bits[ WindowSize / 32 ];
};

/*-------------------------------------------------------------------------*/

/**
 *  Counters of a link
 */
class GUCEF_DRN_EXPORT_CPP CDRNLinkStats
{
    public:

    UInt64 packetsSent;
    UInt64 packetsReceived;
    UInt64 bytesSent;
    UInt64 bytesReceived;
    UInt64 messagesSent;
    UInt64 messagesReceived;
    UInt64 packetsLost;             /**< gaps in the sequences received from the peer */
    UInt64 duplicatePackets;        /**< received again, dropped */
    UInt64 tooOldPackets;           /**< older than the receive window, dropped */
    UInt64 staleMessages;           /**< lane 2 messages dropped because a newer packet was already delivered */
    UInt64 duplicateMessages;       /**< reliable messages received again, dropped */
    UInt64 packetsAcked;            /**< of our packets, acknowledged by the peer */
    UInt64 messagesResent;          /**< reliable messages sent again */
    UInt64 fragmentedMessages;      /**< reliable messages that were split into fragments */
    UInt64 preAuthRefusedPackets;   /**< received before the link was operational with more than the handshake, dropped */
    UInt64 rateLimitedPackets;      /**< received while the peer was over a receive limit, dropped or deferred */
    UInt64 windowRefusedPackets;    /**< received with a reliable message beyond what its lane holds, dropped unacknowledged */
    UInt64 plainTooFarPackets;      /**< PLAIN packets whose sequence lies too far ahead of the newest one, dropped */

    CDRNLinkStats( void );
    void Clear( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  Receives the messages of a packet in delivery order
 */
class GUCEF_DRN_EXPORT_CPP CIDRNLinkMessageSink
{
    public:

    virtual ~CIDRNLinkMessageSink();

    /** @return false to stop processing the rest of the packet */
    virtual bool OnLinkMessage( const CDRNMessageView& message ) = 0;
};

/*-------------------------------------------------------------------------*/

/**
 *  The packet level of a link: sequences, acks, the lanes, and on channels whose transport is not reliable
 *  the reliability of the reliable lanes (keeping messages until acked, resending, ordering, fragmentation).
 *
 *  Messages are queued, BuildPacket() turns the queue into packets (within the datagram budget on datagram
 *  channels, everything in one packet on stream channels), ProcessPacket() takes a received packet apart.
 *  After warming up neither allocates: message storage, packet buffers and the hold-back slots are reused.
 */
class GUCEF_DRN_EXPORT_CPP CDRNLinkReliability
{
    public:

    CDRNLinkReliability( void );

    ~CDRNLinkReliability();

    /**
     *  @param transportIsReliable the channel never loses or reorders
     *  @param maxPacketSize the datagram budget, 0 for stream channels
     *  @param firstSequence the sequence of the first packet sent, never 0
     */
    void Reset( bool transportIsReliable, UInt32 maxPacketSize, UInt32 firstSequence );

    /** Queues a message, its payload being part1 followed by part2 (either may be empty) */
    bool QueueMessage( UInt8 lane                ,
                       UInt8 msgClass            ,
                       UInt32 opcode             ,
                       const void* part1         ,
                       UInt32 size1              ,
                       const void* part2         ,
                       UInt32 size2              ,
                       UInt8 packetKind = DRN_PACKETKIND_AEAD );

    bool HasQueuedOutput( void ) const;

    /** True if received packets carried reliable messages and no packet went out since to acknowledge them */
    bool IsAckOwed( void ) const;

    UInt64 GetAckOwedSince( void ) const;

    /**
     *  Builds the next packet out of the queue
     *  @param force build a packet even if there is nothing to carry (header only: acks, times)
     *  @return false if there was nothing to send, otherwise packet and packetSize describe it until the next call
     */
    bool BuildPacket( UInt64 now, bool force, const UInt8*& packet, UInt32& packetSize );

    /**
     *  As BuildPacket(), a packet of messages of one packet kind (a packet never mixes kinds): AEAD first, then MAC,
     *  then PLAIN; on stream channels one packet per run of equal kind, in the order of the stream. A PLAIN packet
     *  starts with DRN_PROTOCOL_ID_PUBLIC_PLAIN and has a sequence of its own; the others start with DRN_PROTOCOL_ID.
     */
    bool BuildPacket( UInt64 now, bool force, const UInt8*& packet, UInt32& packetSize, UInt8& packetKind );

    /** Marks reliable messages that have not been acknowledged within rto for sending again */
    void CheckRetransmits( UInt64 now, UInt64 rto );

    /** Reliable messages queued and not yet acknowledged: sent, or waiting for the lane window (DRN_RELIABLE_LANE_WINDOW) */
    UInt32 GetUnackedCount( void ) const;

    enum TPacketResult
    {
        PACKET_ACCEPTED         ,
        PACKET_DUPLICATE        ,
        PACKET_TOO_OLD          ,
        PACKET_MALFORMED        ,
        PACKET_REFUSED          ,   /**< restricted: the packet carries more than the handshake, nothing of it was taken */
        PACKET_LIMIT_EXCEEDED   ,   /**< a message or the fragments went over a receive limit, see GetLimitViolation() */
        PACKET_WINDOW_FULL      ,   /**< a reliable message lies beyond what its lane holds: nothing of it was taken or acknowledged */
        PACKET_TOO_FAR_AHEAD        /**< PLAIN: the sequence lies more than MaxPlainSequenceLead ahead of the newest one, not taken */
    };

    enum TLimitViolation
    {
        LIMIT_NONE              ,
        LIMIT_MESSAGE_SIZE      ,
        LIMIT_FRAGMENT_GROUPS   ,
        LIMIT_FRAGMENT_BYTES
    };

    /**
     *  Before the link is operational the receiving side is restricted: a packet is only taken if its messages are
     *  link messages of the handshake (no fragments, nothing on lanes 1 and 2) and lane 0 holds back at most
     *  DRN_PREAUTH_HOLD_SLOTS messages; anything else refuses the whole packet without keeping any of it, so that
     *  the peer sends it again once the link is operational. Messages after a WELCOME are taken.
     */
    void SetPreAuthRestricted( bool restricted );

    bool IsPreAuthRestricted( void ) const;

    /** Receive limits, 0 = no limit */
    void SetReceiveLimits( UInt32 maxMessageSize, UInt32 maxFragmentGroups, UInt32 maxFragmentBytes );

    /** What the last PACKET_LIMIT_EXCEEDED went over */
    TLimitViolation GetLimitViolation( void ) const;

    /** Messages being joined from fragments */
    UInt32 GetOpenFragmentGroups( void ) const;

    /** The total size of the messages being joined from fragments */
    UInt32 GetOpenFragmentBytes( void ) const;

    /** Counts a packet the link did not hand over because the peer was over a receive limit */
    void NoteRateLimitedPacket( void );

    /**
     *  Takes a received packet apart and hands its messages to the sink in delivery order.
     *  A packet is acknowledged only when all of it was taken: on a datagram channel a packet with a reliable message
     *  beyond the window of its lane (DRN_RELIABLE_LANE_WINDOW) is refused whole (PACKET_WINDOW_FULL), as is a
     *  malformed one, before any of it is kept, so that the sender sends its messages again.
     *  @param lossDelay how long a packet that is not acknowledged while a later one is, counts as lost
     */
    TPacketResult ProcessPacket( const UInt8* data            ,
                                 UInt32 size                  ,
                                 UInt64 now                   ,
                                 UInt64 lossDelay             ,
                                 CIDRNLinkMessageSink& sink   ,
                                 CDRNPacketHeader& header     ,
                                 UInt32& lostBefore           ,
                                 UInt8 packetKind = DRN_PACKETKIND_AEAD );

    /** How far ahead of the newest PLAIN sequence a PLAIN packet may be (the first one: of 0) */
    static const UInt32 MaxPlainSequenceLead = 65536;

    /**
     *  A PLAIN packet (DRN_PROTOCOL_ID_PUBLIC_PLAIN): its own sequence space with a window that only drops duplicates;
     *  its ack and echo fields are not read, it acknowledges and measures nothing. Only lane 3 messages are taken,
     *  every other message is refused and counted (refused).
     *  A PLAIN sequence has no integrity: a sequence more than MaxPlainSequenceLead ahead of the newest one is not
     *  taken (PACKET_TOO_FAR_AHEAD), and one behind the window is delivered without the duplicate check, so that forged
     *  jumps of the window never shut out the genuine PLAIN packets.
     */
    TPacketResult ProcessPlainPacket( const UInt8* data, UInt32 size, CIDRNLinkMessageSink& sink, UInt32& refused );

    const CDRNLinkStats& GetStats( void ) const;

    UInt32 GetNextSequence( void ) const;

    bool IsTransportReliable( void ) const;

    UInt32 GetMaxPacketSize( void ) const;

    /**
     *  The largest payload of a message that goes out in one packet, without fragments: on a datagram channel what
     *  fits the datagram, on a stream channel the stream limit (stream packet size and the policy's maxMessageSize)
     */
    UInt32 GetMaxUnfragmentedPayload( void ) const;

    private:

    class CPendingMessage
    {
        public:
        UInt32 id;
        bool inUse;
        bool acked;
        bool needsSend;
        UInt64 lastSendTime;
        UInt32 sendCount;
        UInt8 packetKind;
        UInt8 lane;
        UInt32 laneSequence;
        CORE::CDynamicBuffer encoded;

        CPendingMessage( void );
    };

    class CSentPacket
    {
        public:
        UInt32 sequence;
        bool valid;
        bool acked;
        bool lossHandled;
        UInt64 sendTime;
        GUCEF::vector< UInt32 > pendingIds;

        CSentPacket( void );
    };

    class CHeldMessage
    {
        public:
        bool used;
        UInt32 laneSequence;
        UInt8 msgClass;
        UInt32 opcode;
        UInt8 packetKind;
        CORE::CDynamicBuffer payload;

        CHeldMessage( void );
    };

    class CFragmentAssembly
    {
        public:
        UInt8 msgClass;
        UInt32 opcode;
        UInt8 lane;
        UInt32 totalSize;
        UInt32 count;
        UInt8 packetKind;           /**< the weakest of the fragments so far */
        UInt32 received;
        GUCEF::vector< UInt8 > got;
        CORE::CDynamicBuffer data;

        CFragmentAssembly( void );
    };

    /** Encoded unreliable messages of one packet kind, datagram channels */
    class CUnreliableQueue
    {
        public:
        CORE::CDynamicBuffer queue;
        GUCEF::vector< UInt32 > sizes;
        UInt32 readIndex;
        UInt32 readOffset;
        CUnreliableQueue( void );
        bool HasMessages( void ) const;
        void Clear( void );
    };

    /** The end of a run of messages of one kind in the stream queue */
    class CStreamRun
    {
        public:
        UInt32 end;
        UInt8 packetKind;
    };

    typedef GUCEF::vector< CPendingMessage > TPendingVector;
    typedef GUCEF::vector< CSentPacket > TSentVector;
    typedef GUCEF::vector< CHeldMessage > THeldVector;
    typedef GUCEF::map< UInt32, CFragmentAssembly* > TFragmentMap;

    CDRNLinkReliability( const CDRNLinkReliability& src );
    CDRNLinkReliability& operator=( const CDRNLinkReliability& src );

    UInt32 GetBodyBudget( void ) const;
    void EncodeMessage( CDRNWireWriter& writer, UInt8 lane, UInt8 msgClass, UInt32 opcode, UInt32 laneSequence,
                        const void* part1, UInt32 size1, const void* part2, UInt32 size2 );
    CPendingMessage& AddPending( UInt8 packetKind, UInt8 lane, UInt32 laneSequence );
    UInt32 GetLaneFloor( UInt8 lane );
    void ReleaseWithinLaneWindows( void );
    UInt8 ChooseDatagramKind( void ) const;
    void GrowPending( void );
    bool QueueReliableDatagram( UInt8 lane, UInt8 msgClass, UInt32 opcode, const void* part1, UInt32 size1, const void* part2, UInt32 size2, UInt8 packetKind );
    void FinishPacket( CDRNWireWriter& writer, UInt32 bodyStart, UInt64 now, const UInt8*& packet, UInt32& packetSize, UInt8 packetKind );
    void AckSentPacket( UInt32 sequence );
    void AckPending( UInt32 id );
    void MarkForResend( UInt32 id );
    void ProcessAcks( const CDRNPacketHeader& header, UInt64 now, UInt64 lossDelay );
    bool Deliver( const CDRNMessageView& message, CIDRNLinkMessageSink& sink );
    bool DeliverLaneOrdered( const CDRNMessageView& message, CIDRNLinkMessageSink& sink );
    bool DeliverLaneUnordered( const CDRNMessageView& message, CIDRNLinkMessageSink& sink );
    bool HandleFragment( const CDRNMessageView& message, CIDRNLinkMessageSink& sink );
    void ClearFragments( void );
    TPacketResult CheckBeforeTaking( const CDRNWireReader& reader ) const;
    void ResizeHeldRing( UInt32 newSize );

    bool m_transportIsReliable;
    UInt32 m_maxPacketSize;

    // sending
    UInt32 m_nextSequence;
    UInt32 m_nextLaneSequence[ 2 ];
    UInt32 m_nextFragmentGroup;
    CORE::CDynamicBuffer m_streamQueue;         /**< stream channels: everything queued, after room for the header */
    UInt32 m_streamQueueEnd;
    UInt32 m_streamBodyStart;                   /**< stream channels: where the part not sent yet starts */
    GUCEF::vector< CStreamRun > m_streamRuns;   /**< stream channels: the runs of equal kind, only while a non AEAD message is queued */
    UInt32 m_streamRunsHead;
    CUnreliableQueue m_unreliable[ DRN_PACKETKIND_COUNT ];  /**< datagram channels: encoded unreliable messages per packet kind */
    UInt32 m_needsSendMacCount;                 /**< of m_needsSendCount, the MAC messages */
    UInt32 m_nextPlainSequence;                 /**< sequence of the next PLAIN packet */
    TPendingVector m_pending;                   /**< ring of reliable messages kept until acked, indexed by id */
    UInt32 m_pendingHead;                       /**< id of the oldest message kept */
    UInt32 m_pendingNext;                       /**< id of the next message */
    UInt32 m_releaseNext;                       /**< id of the first message still waiting for its lane window, every one before it may be sent */
    UInt32 m_laneFloorId[ 2 ];                  /**< per reliable lane: no message of the lane before this id is unacknowledged */
    UInt32 m_needsSendCount;
    TSentVector m_sent;                         /**< ring of the packets sent, indexed by sequence */
    GUCEF::vector< UInt32 > m_packetIds;
    CORE::CDynamicBuffer m_datagram;
    CORE::CDynamicBuffer m_scratch;

    // receiving
    CDRNSequenceWindow m_receiveWindow;
    CDRNSequenceWindow m_plainReceiveWindow;    /**< PLAIN packets, duplicates only */
    UInt64 m_highestReceivedSendTime;
    UInt64 m_highestReceivedLocalTime;
    bool m_ackOwed;
    UInt64 m_ackOwedSince;
    bool m_lane2Any;
    UInt32 m_lane2Newest;
    UInt32 m_lane0Expected;
    THeldVector m_held;
    UInt32 m_heldCount;
    UInt32 m_heldRingSize;
    UInt32 m_lane1Base;                         /**< lane 1: the oldest lane sequence not received yet */
    UInt32 m_lane1Received[ DRN_RELIABLE_LANE_WINDOW / 32 ];   /**< lane 1: received lane sequences from m_lane1Base on */
    TFragmentMap m_fragments;
    UInt32 m_fragmentBytes;
    UInt32 m_currentPacketSequence;

    // receive limits
    bool m_preAuthRestricted;
    UInt32 m_maxMessageSize;
    UInt32 m_maxFragmentGroups;
    UInt32 m_maxFragmentBytes;
    TLimitViolation m_limitViolation;

    CDRNLinkStats m_stats;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNLINKRELIABILITY_H ? */
