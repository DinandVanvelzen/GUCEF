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
                       UInt32 size2              );

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

    /** Marks reliable messages that have not been acknowledged within rto for sending again */
    void CheckRetransmits( UInt64 now, UInt64 rto );

    /** Reliable messages sent but not yet acknowledged */
    UInt32 GetUnackedCount( void ) const;

    enum TPacketResult
    {
        PACKET_ACCEPTED         ,
        PACKET_DUPLICATE        ,
        PACKET_TOO_OLD          ,
        PACKET_MALFORMED        ,
        PACKET_REFUSED          ,   /**< restricted: the packet carries more than the handshake, nothing of it was taken */
        PACKET_LIMIT_EXCEEDED       /**< a message or the fragments went over a receive limit, see GetLimitViolation() */
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
     *  Takes a received packet apart and hands its messages to the sink in delivery order
     *  @param lossDelay how long a packet that is not acknowledged while a later one is, counts as lost
     */
    TPacketResult ProcessPacket( const UInt8* data            ,
                                 UInt32 size                  ,
                                 UInt64 now                   ,
                                 UInt64 lossDelay             ,
                                 CIDRNLinkMessageSink& sink   ,
                                 CDRNPacketHeader& header     ,
                                 UInt32& lostBefore           );

    const CDRNLinkStats& GetStats( void ) const;

    UInt32 GetNextSequence( void ) const;

    bool IsTransportReliable( void ) const;

    UInt32 GetMaxPacketSize( void ) const;

    /** The largest payload of a message that goes out in one packet, without fragments */
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
        UInt32 received;
        GUCEF::vector< UInt8 > got;
        CORE::CDynamicBuffer data;

        CFragmentAssembly( void );
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
    CPendingMessage& AddPending( void );
    void GrowPending( void );
    bool QueueReliableDatagram( UInt8 lane, UInt8 msgClass, UInt32 opcode, const void* part1, UInt32 size1, const void* part2, UInt32 size2 );
    void FinishPacket( CDRNWireWriter& writer, UInt32 bodyStart, UInt64 now, const UInt8*& packet, UInt32& packetSize );
    void AckSentPacket( UInt32 sequence );
    void AckPending( UInt32 id );
    void MarkForResend( UInt32 id );
    void ProcessAcks( const CDRNPacketHeader& header, UInt64 now, UInt64 lossDelay );
    bool Deliver( const CDRNMessageView& message, CIDRNLinkMessageSink& sink );
    bool DeliverLaneOrdered( const CDRNMessageView& message, CIDRNLinkMessageSink& sink );
    bool HandleFragment( const CDRNMessageView& message, CIDRNLinkMessageSink& sink );
    void ClearFragments( void );
    bool IsAllowedBeforeOperational( const CDRNWireReader& reader ) const;
    void ResizeHeldRing( UInt32 newSize );

    bool m_transportIsReliable;
    UInt32 m_maxPacketSize;

    // sending
    UInt32 m_nextSequence;
    UInt32 m_nextLaneSequence[ 2 ];
    UInt32 m_nextFragmentGroup;
    CORE::CDynamicBuffer m_streamQueue;         /**< stream channels: everything queued, after room for the header */
    UInt32 m_streamQueueEnd;
    CORE::CDynamicBuffer m_unreliableQueue;     /**< datagram channels: encoded unreliable messages */
    GUCEF::vector< UInt32 > m_unreliableSizes;
    UInt32 m_unreliableReadIndex;
    UInt32 m_unreliableReadOffset;
    TPendingVector m_pending;                   /**< ring of reliable messages kept until acked, indexed by id */
    UInt32 m_pendingHead;                       /**< id of the oldest message kept */
    UInt32 m_pendingNext;                       /**< id of the next message */
    UInt32 m_needsSendCount;
    TSentVector m_sent;                         /**< ring of the packets sent, indexed by sequence */
    GUCEF::vector< UInt32 > m_packetIds;
    CORE::CDynamicBuffer m_datagram;
    CORE::CDynamicBuffer m_scratch;

    // receiving
    CDRNSequenceWindow m_receiveWindow;
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
    CDRNSequenceWindow m_lane1Window;
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
