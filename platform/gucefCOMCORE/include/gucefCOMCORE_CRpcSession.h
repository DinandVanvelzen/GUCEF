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

#ifndef GUCEF_COMCORE_CRPCSESSION_H
#define GUCEF_COMCORE_CRPCSESSION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_COMCORE_RPCPROTOCOL_H
#include "gucefCOMCORE_RpcProtocol.h"
#define GUCEF_COMCORE_RPCPROTOCOL_H
#endif /* GUCEF_COMCORE_RPCPROTOCOL_H ? */

#ifndef GUCEF_COMCORE_CRPCSTATUS_H
#include "gucefCOMCORE_CRpcStatus.h"
#define GUCEF_COMCORE_CRPCSTATUS_H
#endif /* GUCEF_COMCORE_CRPCSTATUS_H ? */

#ifndef GUCEF_COMCORE_CRPCTRANSPORTPROFILE_H
#include "gucefCOMCORE_CRpcTransportProfile.h"
#define GUCEF_COMCORE_CRPCTRANSPORTPROFILE_H
#endif /* GUCEF_COMCORE_CRPCTRANSPORTPROFILE_H ? */

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

/**
 *  The reliability layer of an RPC session (gucefCOMCORE RPC design 7, 8.3): per direction UInt64 sequence numbers,
 *  piggy-backed and standalone acknowledgements, the retention buffer of unacknowledged frames, receive ordering with
 *  duplicate drop and gap detection, the session window, and on unreliable transports SACK, retransmission timers
 *  (RFC 6298) and fast retransmit.
 *
 *  It owns no thread and takes no lock: the endpoint drives it under its own lock on the protocol thread (frames,
 *  ticks) and from callers (sends). Frames go out and come in through the sink. Frame objects are recycled, so steady
 *  traffic does not allocate frame records.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcSession
{
    public:

    /** Implemented by the endpoint */
    class GUCEF_COMCORE_EXPORT_CPP CISink
    {
        public:
        virtual ~CISink();

        /** One whole frame to the transport; false when the transport did not take it */
        virtual bool OnSessionTransmit( const UInt8* frame, UInt32 size, TRpcProtectionClass protection ) = 0;

        /** A frame to process: control frames at once, session frames in seq order exactly once */
        virtual void OnSessionDeliver( const UInt8* frame, UInt32 size, const SRpcFrameHeader& header, TRpcProtectionClass arrivedWith ) = 0;

        /** A protocol violation: the endpoint ends the session with ABORT */
        virtual void OnSessionProtocolViolation( const CRpcStatus& status ) = 0;

        /** A gap on a reliable ordered transport: the endpoint closes the transport so the resume repairs it */
        virtual void OnSessionGap( UInt64 from, UInt64 to ) = 0;

        /** Queued frames went out: the window or the transport accepts again */
        virtual void OnSessionWritable( void ) = 0;
    };

    struct GUCEF_COMCORE_EXPORT_CPP SSettings
    {
        bool reliableOrdered;               /**< the transport neither loses nor reorders while up */
        UInt32 ackDelayMs;
        UInt32 ackEveryFrames;
        UInt32 receiveWindowBytes;          /**< what this side accepts unacknowledged */
        UInt32 retentionMaxFrames;          /**< 0 = no maximum */
        UInt32 retentionMaxBytes;           /**< 0 = no maximum */
        UInt32 sendQueueMaxBytes;           /**< frames not yet sent; 0 = no maximum */
        TRpcSuspendedOverflow suspendedOverflow;
        UInt32 initialRtoMs;
        UInt32 minRtoMs;
        UInt32 maxRtoMs;
        UInt32 dupThreshold;
        UInt32 maxBurstFrames;
        UInt32 maxReorderFrames;            /**< unreliable transports: frames buffered ahead of a gap */

        SSettings( void );
        void ApplyProfile( const CRpcTransportProfile& profile );
    };

    struct GUCEF_COMCORE_EXPORT_CPP SCounters
    {
        UInt64 framesSent;
        UInt64 framesReceived;
        UInt64 bytesSent;
        UInt64 bytesReceived;
        UInt64 retransmissions;
        UInt64 resumeRetransmissions;
        UInt64 duplicateFrames;
        UInt64 gaps;
        UInt64 standaloneAcks;
        UInt64 droppedFrames;               /**< DROP_OLDEST while suspended */

        SCounters( void );
    };

    explicit CRpcSession( CISink& sink );
    ~CRpcSession();

    void Configure( const SSettings& settings );
    const SSettings& GetSettings( void ) const;

    void SetPeerReceiveWindow( UInt32 bytes );

    /** Up: queued and retransmittable frames go out. Down: frames stay in retention (SUSPENDED) */
    void SetTransportUp( bool up, UInt64 nowMs );
    bool IsTransportUp( void ) const;

    /**
     *  Queues a session frame: its seq is assigned now (the next of this side), it is sent when the window and the
     *  transport allow, and it is retained until acknowledged. False with a status when it is refused (send queue
     *  limit, retention full while suspended with REFUSE_NEW).
     */
    bool QueueFrame( UInt8 type, UInt8 flags, const void* body, UInt32 bodySize, TRpcProtectionClass protection,
                     UInt64 nowMs, UInt64& seq, CRpcStatus& refusal );

    /** A control frame (seq 0): sent at once, not retained, carries the current ack */
    bool SendControlFrame( UInt8 type, UInt8 flags, const void* body, UInt32 bodySize );

    /** Sends an ACK frame now (window and SACK ranges) */
    void SendAck( UInt64 nowMs );

    void OnFrameReceived( const UInt8* frame, UInt32 size, TRpcProtectionClass protection, UInt64 nowMs );

    /** Timers: delayed acks, retransmissions on unreliable transports */
    void OnTick( UInt64 nowMs );

    UInt64 GetNextSeq( void ) const;                /**< the seq the next session frame gets */
    UInt64 GetDeliveredSeq( void ) const;           /**< highest seq delivered in order */
    UInt64 GetOldestRetainedSeq( void ) const;      /**< lowest seq still held; GetNextSeq() when none */
    UInt64 GetPeerAckedSeq( void ) const;
    UInt32 GetRetainedFrameCount( void ) const;
    UInt32 GetRetainedBytes( void ) const;
    UInt32 GetQueuedBytes( void ) const;            /**< not yet sent */
    UInt32 GetInFlightBytes( void ) const;
    bool HasUnackedDeliveries( void ) const;
    Float64 GetSmoothedRttMs( void ) const;
    const SCounters& GetCounters( void ) const;

    /**
     *  Resume (8.3): frames the peer received (seq <= peerLastReceivedSeq) are released, the others are sent again
     *  unchanged, in order, then numbering continues. False when the peer claims frames this side never sent.
     */
    bool PrepareResume( UInt64 peerLastReceivedSeq, UInt64 nowMs );

    /** Starts over for a new session: numbering from 1, retention and reorder buffers emptied */
    void Reset( void );

    private:

    struct SOutFrame
    {
        UInt64 seq;
        UInt8 type;
        UInt8 flags;
        TRpcProtectionClass protection;
        CORE::CDynamicBuffer body;          /**< the body until sent, then the whole frame as sent */
        bool sent;
        bool needsResend;
        bool sacked;
        bool fastRetransmitted;
        UInt32 sendCount;
        UInt32 wireSize;
        UInt64 firstSendMs;
        UInt64 lastSendMs;
    };
    typedef GUCEF::vector< SOutFrame* > TOutFrameVector;

    struct SInFrame
    {
        CORE::CDynamicBuffer frame;
        TRpcProtectionClass protection;
    };
    typedef GUCEF::map< UInt64, SInFrame* > TReorderMap;

    SOutFrame* AllocFrame( void );
    void FreeFrame( SOutFrame* frame );
    UInt32 RetainedCount( void ) const;
    SOutFrame* RetainedAt( UInt32 index ) const;
    void PopFront( void );
    void Flush( UInt64 nowMs );
    bool Emit( SOutFrame& frame, UInt64 nowMs );
    bool Resend( SOutFrame& frame, UInt64 nowMs );
    void ProcessAck( UInt64 ackSeq, UInt64 nowMs );
    void ReleaseAcknowledged( UInt64 nowMs );
    void ProcessAckFrame( const UInt8* body, UInt32 size, UInt64 frameAckSeq, UInt64 nowMs );
    void Deliver( const UInt8* frame, UInt32 size, const SRpcFrameHeader& header, TRpcProtectionClass protection, UInt64 nowMs );
    void NoteDelivery( UInt32 size, UInt64 nowMs );
    void UpdateRtt( UInt64 sampleMs );
    void EnforceSuspendedBounds( void );
    bool ShouldPiggyBackAck( void ) const;

    CRpcSession( const CRpcSession& src );                  /**< not implemented */
    CRpcSession& operator=( const CRpcSession& src );       /**< not implemented */

    CISink* m_sink;
    SSettings m_settings;
    bool m_transportUp;
    UInt64 m_nextSeq;
    UInt64 m_deliveredSeq;
    UInt64 m_peerAckedSeq;
    UInt64 m_lastAckSent;
    UInt32 m_peerWindow;
    TOutFrameVector m_retained;
    UInt32 m_retainedHead;
    UInt32 m_retainedBytes;
    UInt32 m_queuedBytes;
    UInt32 m_inFlightBytes;
    TOutFrameVector m_freeFrames;
    TReorderMap m_reorder;
    UInt32 m_reorderBytes;
    UInt32 m_unackedDeliveries;
    UInt32 m_unackedBytes;
    UInt64 m_firstUnackedDeliveryMs;
    bool m_ackNow;
    Float64 m_srttMs;
    Float64 m_rttvarMs;
    UInt32 m_rtoMs;
    bool m_hasRtt;
    UInt64 m_highestSacked;
    UInt64 m_flushFromSeq;          /**< every retained frame below this seq is sent and needs no resend: Flush() starts here */
    bool m_inFlush;
    UInt32 m_scanDepth;                 /**< scans of the retained frames running: acknowledged frames are released after */
    SCounters m_counters;
    CORE::CDynamicBuffer m_scratch;
    CORE::CDynamicBuffer m_controlScratch;      /**< control frames and ACKs are written here (SendControlFrame(), SendAck()) */
    UInt32 m_controlDepth;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCSESSION_H ? */
