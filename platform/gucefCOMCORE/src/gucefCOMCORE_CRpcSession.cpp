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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMCORE_CRPCFRAMECODEC_H
#include "gucefCOMCORE_CRpcFrameCodec.h"
#define GUCEF_COMCORE_CRPCFRAMECODEC_H
#endif /* GUCEF_COMCORE_CRPCFRAMECODEC_H ? */

#include "gucefCOMCORE_CRpcSession.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRpcSession::CISink::~CISink()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcSession::SSettings::SSettings( void )
    : reliableOrdered( true )
    , ackDelayMs( 50 )
    , ackEveryFrames( 32 )
    , receiveWindowBytes( 4 * 1024 * 1024 )
    , retentionMaxFrames( 8192 )
    , retentionMaxBytes( 16 * 1024 * 1024 )
    , sendQueueMaxBytes( 16 * 1024 * 1024 )
    , suspendedOverflow( RPC_SUSPENDEDOVERFLOW_REFUSE_NEW )
    , initialRtoMs( 1000 )
    , minRtoMs( 200 )
    , maxRtoMs( 60000 )
    , dupThreshold( 3 )
    , maxBurstFrames( 16 )
    , maxReorderFrames( 4096 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRpcSession::SSettings::ApplyProfile( const CRpcTransportProfile& profile )
{GUCEF_TRACE;

    ackDelayMs = profile.ackDelayMs;
    ackEveryFrames = profile.ackEveryFrames;
    initialRtoMs = profile.initialRtoMs;
    minRtoMs = profile.minRtoMs;
    maxRtoMs = profile.maxRtoMs;
    dupThreshold = profile.dupThreshold;
    maxBurstFrames = profile.maxBurstFrames;
}

/*-------------------------------------------------------------------------*/

CRpcSession::SCounters::SCounters( void )
    : framesSent( 0 )
    , framesReceived( 0 )
    , bytesSent( 0 )
    , bytesReceived( 0 )
    , retransmissions( 0 )
    , resumeRetransmissions( 0 )
    , duplicateFrames( 0 )
    , gaps( 0 )
    , standaloneAcks( 0 )
    , droppedFrames( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcSession::CRpcSession( CISink& sink )
    : m_sink( &sink )
    , m_settings()
    , m_transportUp( false )
    , m_nextSeq( 1 )
    , m_deliveredSeq( 0 )
    , m_peerAckedSeq( 0 )
    , m_lastAckSent( 0 )
    , m_peerWindow( 64 * 1024 )
    , m_retained()
    , m_retainedHead( 0 )
    , m_retainedBytes( 0 )
    , m_queuedBytes( 0 )
    , m_inFlightBytes( 0 )
    , m_freeFrames()
    , m_reorder()
    , m_reorderBytes( 0 )
    , m_unackedDeliveries( 0 )
    , m_unackedBytes( 0 )
    , m_firstUnackedDeliveryMs( 0 )
    , m_ackNow( false )
    , m_srttMs( 0 )
    , m_rttvarMs( 0 )
    , m_rtoMs( 1000 )
    , m_hasRtt( false )
    , m_highestSacked( 0 )
    , m_inFlush( false )
    , m_scanDepth( 0 )
    , m_flushFromSeq( 0 )
    , m_counters()
    , m_scratch()
    , m_controlScratch()
    , m_controlDepth( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcSession::~CRpcSession()
{GUCEF_TRACE;

    Reset();
    for ( UInt32 i=0; i<m_freeFrames.size(); ++i )
        GUCEF_DELETE m_freeFrames[ i ];
    m_freeFrames.clear();
}

/*-------------------------------------------------------------------------*/

void
CRpcSession::Configure( const SSettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
    m_rtoMs = settings.initialRtoMs;
}

/*-------------------------------------------------------------------------*/

const CRpcSession::SSettings&
CRpcSession::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CRpcSession::SetPeerReceiveWindow( UInt32 bytes )
{GUCEF_TRACE;

    m_peerWindow = bytes;
}

/*-------------------------------------------------------------------------*/

CRpcSession::SOutFrame*
CRpcSession::AllocFrame( void )
{GUCEF_TRACE;

    SOutFrame* frame = GUCEF_NULL;
    if ( !m_freeFrames.empty() )
    {
        frame = m_freeFrames.back();
        m_freeFrames.pop_back();
    }
    else
        frame = GUCEF_NEW SOutFrame();
    frame->seq = 0;
    frame->type = 0;
    frame->flags = 0;
    frame->protection = RPC_PROTECTION_CONFIDENTIAL;
    frame->body.SetDataSize( 0 );
    frame->sent = false;
    frame->needsResend = false;
    frame->sacked = false;
    frame->fastRetransmitted = false;
    frame->sendCount = 0;
    frame->wireSize = 0;
    frame->firstSendMs = 0;
    frame->lastSendMs = 0;
    return frame;
}

/*-------------------------------------------------------------------------*/

void
CRpcSession::FreeFrame( SOutFrame* frame )
{GUCEF_TRACE;

    // large buffers are not kept, so a burst of big frames does not pin memory
    if ( frame->body.GetBufferSize() > 256 * 1024 || m_freeFrames.size() >= 1024 )
    {
        GUCEF_DELETE frame;
        return;
    }
    m_freeFrames.push_back( frame );
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcSession::RetainedCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_retained.size() - m_retainedHead;
}

/*-------------------------------------------------------------------------*/

CRpcSession::SOutFrame*
CRpcSession::RetainedAt( UInt32 index ) const
{GUCEF_TRACE;

    return m_retained[ m_retainedHead + index ];
}

/*-------------------------------------------------------------------------*/

void
CRpcSession::PopFront( void )
{GUCEF_TRACE;

    SOutFrame* frame = m_retained[ m_retainedHead ];
    m_retained[ m_retainedHead ] = GUCEF_NULL;
    ++m_retainedHead;
    UInt32 size = frame->sent ? frame->wireSize : frame->body.GetDataSize();
    m_retainedBytes -= size;
    if ( frame->sent )
        m_inFlightBytes -= frame->wireSize;
    else
        m_queuedBytes -= frame->body.GetDataSize();
    FreeFrame( frame );

    // compact now and then instead of shifting on every pop
    if ( m_retainedHead == m_retained.size() )
    {
        m_retained.clear();
        m_retainedHead = 0;
    }
    else if ( m_retainedHead > 64 && m_retainedHead * 2 > m_retained.size() )
    {
        m_retained.erase( m_retained.begin(), m_retained.begin() + m_retainedHead );
        m_retainedHead = 0;
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcSession::ShouldPiggyBackAck( void ) const
{GUCEF_TRACE;

    return m_deliveredSeq > 0;
}

/*-------------------------------------------------------------------------*/

void
CRpcSession::SetTransportUp( bool up, UInt64 nowMs )
{GUCEF_TRACE;

    m_transportUp = up;
    if ( up )
        Flush( nowMs );
}

/*-------------------------------------------------------------------------*/

bool
CRpcSession::IsTransportUp( void ) const
{GUCEF_TRACE;

    return m_transportUp;
}

/*-------------------------------------------------------------------------*/

void
CRpcSession::EnforceSuspendedBounds( void )
{GUCEF_TRACE;

    // DROP_OLDEST: the oldest unacknowledged frames go; the resume reports the gap (8.4)
    while ( RetainedCount() > 0 &&
            ( ( 0 != m_settings.retentionMaxFrames && RetainedCount() > m_settings.retentionMaxFrames ) ||
              ( 0 != m_settings.retentionMaxBytes && m_retainedBytes > m_settings.retentionMaxBytes ) ) )
    {
        PopFront();
        ++m_counters.droppedFrames;
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcSession::QueueFrame( UInt8 type, UInt8 flags, const void* body, UInt32 bodySize, TRpcProtectionClass protection,
                         UInt64 nowMs, UInt64& seq, CRpcStatus& refusal )
{GUCEF_TRACE;

    if ( 0 != m_settings.sendQueueMaxBytes && m_queuedBytes + bodySize > m_settings.sendQueueMaxBytes && m_queuedBytes > 0 )
    {
        refusal.Set( RPC_STATUS_RESOURCE_EXHAUSTED, "the send queue is full" );
        return false;
    }
    if ( !m_transportUp && RPC_SUSPENDEDOVERFLOW_REFUSE_NEW == m_settings.suspendedOverflow )
    {
        if ( ( 0 != m_settings.retentionMaxFrames && RetainedCount() + 1 > m_settings.retentionMaxFrames ) ||
             ( 0 != m_settings.retentionMaxBytes && m_retainedBytes + bodySize > m_settings.retentionMaxBytes ) )
        {
            refusal.Set( RPC_STATUS_UNAVAILABLE, "the session is suspended and its retention is full" );
            return false;
        }
    }

    SOutFrame* frame = AllocFrame();
    frame->seq = m_nextSeq++;
    frame->type = type;
    frame->flags = (UInt8) ( flags & ~GUCEF_RPC_FRAMEFLAG_ACK );
    frame->protection = protection;
    if ( bodySize > 0 )
        frame->body.CopyFrom( bodySize, body );
    m_retained.push_back( frame );
    m_retainedBytes += bodySize;
    m_queuedBytes += bodySize;
    seq = frame->seq;

    if ( !m_transportUp && RPC_SUSPENDEDOVERFLOW_DROP_OLDEST == m_settings.suspendedOverflow )
        EnforceSuspendedBounds();

    Flush( nowMs );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcSession::Emit( SOutFrame& frame, UInt64 nowMs )
{GUCEF_TRACE;

    // the header is written when the frame leaves, with the current ack; after that the bytes never change
    m_scratch.SetDataSize( 0 );
    CORE::CBinaryWireWriter writer( m_scratch );
    bool withAck = ShouldPiggyBackAck();
    CRpcFrameCodec::WriteHeader( writer, frame.type, frame.flags, frame.seq, withAck, m_deliveredSeq );
    writer.WriteBytes( frame.body.GetConstBufferPtr(), frame.body.GetDataSize() );

    // the frame's state is final before the transport sees it: a transport that delivers at once may bring the
    // peer's ACK back inside OnSessionTransmit, which releases the frame (the scan depth defers that release)
    UInt32 bodySize = frame.body.GetDataSize();
    frame.body.SetDataSize( 0 );
    frame.body.CopyFrom( m_scratch.GetDataSize(), m_scratch.GetConstBufferPtr() );
    frame.wireSize = m_scratch.GetDataSize();
    frame.sent = true;
    frame.sendCount = 1;
    frame.firstSendMs = nowMs;
    frame.lastSendMs = nowMs;
    m_queuedBytes -= bodySize;
    m_retainedBytes = m_retainedBytes - bodySize + frame.wireSize;
    m_inFlightBytes += frame.wireSize;
    UInt64 lastAckSent = m_lastAckSent;
    UInt32 unackedDeliveries = m_unackedDeliveries;
    UInt32 unackedBytes = m_unackedBytes;
    if ( withAck )
    {
        m_lastAckSent = m_deliveredSeq;
        m_unackedDeliveries = 0;
        m_unackedBytes = 0;
    }

    ++m_scanDepth;
    bool sent = m_sink->OnSessionTransmit( static_cast< const UInt8* >( frame.body.GetConstBufferPtr() ), frame.wireSize, frame.protection );
    --m_scanDepth;
    if ( !sent )
    {
        // not sent: back to queued (the peer cannot have acknowledged it)
        m_scratch.SetDataSize( 0 );
        m_scratch.CopyFrom( bodySize, static_cast< const UInt8* >( frame.body.GetConstBufferPtr() ) + ( frame.wireSize - bodySize ) );
        frame.body.SetDataSize( 0 );
        frame.body.CopyFrom( bodySize, m_scratch.GetConstBufferPtr() );
        m_inFlightBytes -= frame.wireSize;
        m_retainedBytes = m_retainedBytes - frame.wireSize + bodySize;
        m_queuedBytes += bodySize;
        frame.sent = false;
        frame.sendCount = 0;
        frame.wireSize = 0;
        if ( withAck )
        {
            m_lastAckSent = lastAckSent;
            m_unackedDeliveries = unackedDeliveries;
            m_unackedBytes = unackedBytes;
        }
        return false;
    }
    ++m_counters.framesSent;
    m_counters.bytesSent += frame.wireSize;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcSession::Resend( SOutFrame& frame, UInt64 nowMs )
{GUCEF_TRACE;

    UInt64 lastSendMs = frame.lastSendMs;
    ++frame.sendCount;
    frame.lastSendMs = nowMs;
    UInt32 wireSize = frame.wireSize;
    ++m_scanDepth;
    bool sent = m_sink->OnSessionTransmit( static_cast< const UInt8* >( frame.body.GetConstBufferPtr() ), frame.wireSize, frame.protection );
    --m_scanDepth;
    if ( !sent )
    {
        --frame.sendCount;
        frame.lastSendMs = lastSendMs;
        return false;
    }
    ++m_counters.framesSent;
    m_counters.bytesSent += wireSize;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcSession::Flush( UInt64 nowMs )
{GUCEF_TRACE;

    if ( !m_transportUp || m_inFlush )
        return;
    m_inFlush = true;
    ++m_scanDepth;

    // the frames before m_flushFromSeq went out already: a session with thousands of frames in flight does not look
    // at all of them for every frame it sends. Retained frames have consecutive seq numbers; should that not hold at
    // the expected place, everything is looked at
    bool progress = false;
    UInt32 count = RetainedCount();
    UInt32 first = 0;
    if ( count > 0 && m_flushFromSeq > RetainedAt( 0 )->seq )
    {
        UInt64 offset = m_flushFromSeq - RetainedAt( 0 )->seq;
        first = offset < count ? (UInt32) offset : count;
        if ( first < count && RetainedAt( first )->seq != m_flushFromSeq )
            first = 0;
    }
    for ( UInt32 i=first; i<count && m_transportUp; ++i )
    {
        SOutFrame* frame = RetainedAt( i );
        if ( frame->sent && !frame->needsResend )
        {
            m_flushFromSeq = frame->seq + 1;
            continue;
        }
        UInt32 size = frame->sent ? frame->wireSize : frame->body.GetDataSize() + 24;

        // the window keeps unacknowledged bytes within what the peer accepts; one frame always goes
        if ( m_inFlightBytes > 0 && 0 != m_peerWindow && m_inFlightBytes + size > m_peerWindow && !frame->needsResend )
            break;
        if ( frame->needsResend )
        {
            if ( !Resend( *frame, nowMs ) )
                break;
            frame->needsResend = false;
            ++m_counters.resumeRetransmissions;
        }
        else if ( !Emit( *frame, nowMs ) )
            break;
        m_flushFromSeq = frame->seq + 1;
        progress = true;
        count = RetainedCount();
    }
    --m_scanDepth;
    m_inFlush = false;
    ReleaseAcknowledged( nowMs );
    if ( progress && 0 == m_queuedBytes )
        m_sink->OnSessionWritable();
}

/*-------------------------------------------------------------------------*/

bool
CRpcSession::SendControlFrame( UInt8 type, UInt8 flags, const void* body, UInt32 bodySize )
{GUCEF_TRACE;

    if ( !m_transportUp )
        return false;

    // a buffer of the session, so a control frame allocates nothing once it is large enough (15); a transport that
    // delivers inside the send may bring a frame whose handling sends again: that nested send takes a buffer of its own
    CORE::CDynamicBuffer nestedFrame;
    CORE::CDynamicBuffer& frame = 0 == m_controlDepth ? m_controlScratch : nestedFrame;
    frame.SetDataSize( 0 );
    CORE::CBinaryWireWriter writer( frame );
    bool withAck = ShouldPiggyBackAck() && GUCEF_RPC_FRAME_HELLO != type && GUCEF_RPC_FRAME_HELLO_ACK != type;
    CRpcFrameCodec::WriteHeader( writer, type, flags, 0, withAck, m_deliveredSeq );
    writer.WriteBytes( body, bodySize );
    ++m_controlDepth;
    bool sent = m_sink->OnSessionTransmit( static_cast< const UInt8* >( frame.GetConstBufferPtr() ), frame.GetDataSize(), RPC_PROTECTION_CONFIDENTIAL );
    --m_controlDepth;
    if ( !sent )
        return false;
    if ( withAck )
    {
        m_lastAckSent = m_deliveredSeq;
        m_unackedDeliveries = 0;
        m_unackedBytes = 0;
    }
    ++m_counters.framesSent;
    m_counters.bytesSent += frame.GetDataSize();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcSession::SendAck( UInt64 nowMs )
{GUCEF_TRACE;

    if ( !m_transportUp )
        return;

    // ACK always carries the ACK flag, also with nothing delivered yet (ackSeq 0); then the receive window and the SACK
    // ranges of what is buffered beyond the in-order point (unreliable transports). Written into a buffer of the session
    // as SendControlFrame() does, so an ACK allocates nothing
    CORE::CDynamicBuffer nestedFrame;
    CORE::CDynamicBuffer& frame = 0 == m_controlDepth ? m_controlScratch : nestedFrame;
    frame.SetDataSize( 0 );
    CORE::CBinaryWireWriter writer( frame );
    CRpcFrameCodec::WriteHeader( writer, GUCEF_RPC_FRAME_ACK, 0, 0, true, m_deliveredSeq );
    writer.WriteVarUInt32( m_settings.receiveWindowBytes );
    UInt32 rangeCount = 0;
    UInt64 ranges[ 32 ];
    UInt64 previousEnd = m_deliveredSeq;
    TReorderMap::const_iterator i = m_reorder.begin();
    while ( i != m_reorder.end() && rangeCount < 16 )
    {
        UInt64 start = (*i).first;
        UInt64 end = start;
        ++i;
        while ( i != m_reorder.end() && (*i).first == end + 1 )
        {
            end = (*i).first;
            ++i;
        }
        ranges[ rangeCount * 2 ] = start - previousEnd - 1;
        ranges[ rangeCount * 2 + 1 ] = end - start + 1;
        previousEnd = end;
        ++rangeCount;
    }
    writer.WriteVarUInt32( rangeCount );
    for ( UInt32 r=0; r<rangeCount * 2; ++r )
        writer.WriteVarUInt64( ranges[ r ] );

    ++m_controlDepth;
    bool sent = m_sink->OnSessionTransmit( static_cast< const UInt8* >( frame.GetConstBufferPtr() ), frame.GetDataSize(), RPC_PROTECTION_CONFIDENTIAL );
    --m_controlDepth;
    if ( sent )
    {
        m_lastAckSent = m_deliveredSeq;
        m_unackedDeliveries = 0;
        m_unackedBytes = 0;
        m_ackNow = false;
        ++m_counters.standaloneAcks;
        ++m_counters.framesSent;
        m_counters.bytesSent += frame.GetDataSize();
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcSession::UpdateRtt( UInt64 sampleMs )
{GUCEF_TRACE;

    Float64 sample = (Float64) sampleMs;
    if ( !m_hasRtt )
    {
        m_srttMs = sample;
        m_rttvarMs = sample / 2;
        m_hasRtt = true;
    }
    else
    {
        Float64 delta = m_srttMs - sample;
        if ( delta < 0 )
            delta = -delta;
        m_rttvarMs = 0.75 * m_rttvarMs + 0.25 * delta;
        m_srttMs = 0.875 * m_srttMs + 0.125 * sample;
    }
    Float64 rto = m_srttMs + 4 * m_rttvarMs;
    if ( rto < m_settings.minRtoMs )
        rto = m_settings.minRtoMs;
    if ( 0 != m_settings.maxRtoMs && rto > m_settings.maxRtoMs )
        rto = m_settings.maxRtoMs;
    m_rtoMs = (UInt32) rto;
}

/*-------------------------------------------------------------------------*/

void
CRpcSession::ProcessAck( UInt64 ackSeq, UInt64 nowMs )
{GUCEF_TRACE;

    if ( ackSeq <= m_peerAckedSeq )
        return;
    m_peerAckedSeq = ackSeq;
    ReleaseAcknowledged( nowMs );
}

/*-------------------------------------------------------------------------*/

void
CRpcSession::ReleaseAcknowledged( UInt64 nowMs )
{GUCEF_TRACE;

    // while a scan over the retained frames runs (or a frame is being handed to the transport) they stay put
    if ( m_scanDepth > 0 )
        return;

    UInt64 ackSeq = m_peerAckedSeq;
    UInt64 rttSample = 0;
    bool hasSample = false;
    bool released = false;
    while ( RetainedCount() > 0 && RetainedAt( 0 )->seq <= ackSeq )
    {
        SOutFrame* frame = RetainedAt( 0 );
        if ( frame->sent && 1 == frame->sendCount )
        {
            // Karn: only frames sent once give a sample
            rttSample = nowMs - frame->firstSendMs;
            hasSample = true;
        }
        PopFront();
        released = true;
    }
    if ( hasSample )
        UpdateRtt( rttSample );
    if ( released && !m_hasRtt )
        m_rtoMs = m_settings.initialRtoMs;
    if ( released )
        Flush( nowMs );
}

/*-------------------------------------------------------------------------*/

void
CRpcSession::ProcessAckFrame( const UInt8* body, UInt32 size, UInt64 frameAckSeq, UInt64 nowMs )
{GUCEF_TRACE;

    CORE::CBinaryWireReader reader( body, size );
    UInt32 window = 0;
    UInt32 rangeCount = 0;
    if ( !reader.ReadVarUInt32( window ) || !reader.ReadVarUInt32( rangeCount ) || rangeCount > 64 )
    {
        m_sink->OnSessionProtocolViolation( CRpcStatus( RPC_STATUS_INTERNAL, "malformed ACK frame" ) );
        return;
    }
    m_peerWindow = window;

    // the ranges count from the cumulative ack of this ACK frame, which may be older than the newest one seen (an
    // ACK that was delayed or reordered on an unreliable transport)
    UInt64 previousEnd = frameAckSeq;
    for ( UInt32 r=0; r<rangeCount; ++r )
    {
        UInt64 gap = 0;
        UInt64 length = 0;
        if ( !reader.ReadVarUInt64( gap ) || !reader.ReadVarUInt64( length ) || 0 == length )
        {
            m_sink->OnSessionProtocolViolation( CRpcStatus( RPC_STATUS_INTERNAL, "malformed SACK range" ) );
            return;
        }
        UInt64 start = previousEnd + gap + 1;
        UInt64 end = start + length - 1;
        if ( end >= m_nextSeq )
        {
            m_sink->OnSessionProtocolViolation( CRpcStatus( RPC_STATUS_INTERNAL, "SACK of frames never sent" ) );
            return;
        }
        previousEnd = end;

        // a reliable ordered transport loses nothing while it is up: a SACK range carries no information there
        if ( m_settings.reliableOrdered )
            continue;

        // retained frames have consecutive seqs from the oldest one: a range is indexed, not searched; the ranges
        // ascend and do not overlap, so all of them together touch every retained frame at most once
        UInt32 count = RetainedCount();
        if ( count > 0 )
        {
            UInt64 oldest = RetainedAt( 0 )->seq;
            UInt64 seq = start > oldest ? start : oldest;
            for ( ; seq <= end && seq - oldest < count; ++seq )
                RetainedAt( (UInt32) ( seq - oldest ) )->sacked = true;
        }
        if ( end > m_highestSacked )
            m_highestSacked = end;
    }

    // fast retransmit: a frame with dupThreshold later frames SACKed was most likely lost
    if ( !m_settings.reliableOrdered && rangeCount > 0 )
    {
        ++m_scanDepth;
        UInt32 sackedAbove = 0;

        // only frames below the highest SACKed one can have later frames SACKed
        Int32 top = (Int32) RetainedCount() - 1;
        if ( top >= 0 )
        {
            UInt64 oldest = RetainedAt( 0 )->seq;
            if ( m_highestSacked < oldest )
                top = -1;
            else if ( m_highestSacked - oldest < (UInt64) top )
                top = (Int32) ( m_highestSacked - oldest );
        }
        for ( Int32 i=top; i>=0; --i )
        {
            SOutFrame* frame = RetainedAt( (UInt32) i );
            if ( frame->sacked )
            {
                ++sackedAbove;
                continue;
            }
            if ( frame->sent && !frame->fastRetransmitted && frame->seq < m_highestSacked && sackedAbove >= m_settings.dupThreshold )
            {
                if ( Resend( *frame, nowMs ) )
                {
                    frame->fastRetransmitted = true;
                    ++m_counters.retransmissions;
                }
            }
        }
        --m_scanDepth;
    }
    Flush( nowMs );
}

/*-------------------------------------------------------------------------*/

void
CRpcSession::NoteDelivery( UInt32 size, UInt64 nowMs )
{GUCEF_TRACE;

    if ( 0 == m_unackedDeliveries )
        m_firstUnackedDeliveryMs = nowMs;
    ++m_unackedDeliveries;
    m_unackedBytes += size;
    if ( ( 0 != m_settings.ackEveryFrames && m_unackedDeliveries >= m_settings.ackEveryFrames ) ||
         m_unackedBytes >= m_settings.receiveWindowBytes / 4 )
        m_ackNow = true;
}

/*-------------------------------------------------------------------------*/

void
CRpcSession::Deliver( const UInt8* frame, UInt32 size, const SRpcFrameHeader& header, TRpcProtectionClass protection, UInt64 nowMs )
{GUCEF_TRACE;

    m_deliveredSeq = header.seq;
    NoteDelivery( size, nowMs );
    m_sink->OnSessionDeliver( frame, size, header, protection );
}

/*-------------------------------------------------------------------------*/

void
CRpcSession::OnFrameReceived( const UInt8* frame, UInt32 size, TRpcProtectionClass protection, UInt64 nowMs )
{GUCEF_TRACE;

    SRpcFrameHeader header;
    if ( !CRpcFrameCodec::ParseHeader( frame, size, header ) )
    {
        m_sink->OnSessionProtocolViolation( CRpcStatus( RPC_STATUS_INTERNAL, "malformed frame header or reserved flag bits set" ) );
        return;
    }
    ++m_counters.framesReceived;
    m_counters.bytesReceived += size;

    if ( header.hasAck )
    {
        if ( header.ackSeq >= m_nextSeq )
        {
            m_sink->OnSessionProtocolViolation( CRpcStatus( RPC_STATUS_INTERNAL, "acknowledgement of frames never sent" ) );
            return;
        }
        ProcessAck( header.ackSeq, nowMs );
    }

    if ( header.IsControl() )
    {
        if ( GUCEF_RPC_FRAME_ACK == header.type )
        {
            if ( !header.hasAck )
            {
                m_sink->OnSessionProtocolViolation( CRpcStatus( RPC_STATUS_INTERNAL, "ACK frame without the ACK flag" ) );
                return;
            }
            ProcessAckFrame( frame + header.bodyOffset, size - header.bodyOffset, header.ackSeq, nowMs );
            return;
        }
        m_sink->OnSessionDeliver( frame, size, header, protection );
        return;
    }

    if ( header.seq <= m_deliveredSeq || m_reorder.find( header.seq ) != m_reorder.end() )
    {
        // already delivered: the peer sends again what it thinks is lost, an ACK stops that
        ++m_counters.duplicateFrames;
        m_ackNow = true;
        return;
    }

    if ( header.seq == m_deliveredSeq + 1 )
    {
        Deliver( frame, size, header, protection, nowMs );

        // frames that were waiting for this one follow in order
        while ( !m_reorder.empty() )
        {
            TReorderMap::iterator next = m_reorder.begin();
            if ( (*next).first != m_deliveredSeq + 1 )
                break;
            SInFrame* waiting = (*next).second;
            m_reorder.erase( next );
            m_reorderBytes -= waiting->frame.GetDataSize();
            SRpcFrameHeader waitingHeader;
            CRpcFrameCodec::ParseHeader( static_cast< const UInt8* >( waiting->frame.GetConstBufferPtr() ), waiting->frame.GetDataSize(), waitingHeader );
            Deliver( static_cast< const UInt8* >( waiting->frame.GetConstBufferPtr() ), waiting->frame.GetDataSize(), waitingHeader, waiting->protection, nowMs );
            GUCEF_DELETE waiting;
        }
        if ( !m_reorder.empty() )
            m_ackNow = true;
        return;
    }

    // a gap
    ++m_counters.gaps;
    if ( m_settings.reliableOrdered )
    {
        m_sink->OnSessionGap( m_deliveredSeq + 1, header.seq - 1 );
        return;
    }
    if ( m_reorder.size() < m_settings.maxReorderFrames && m_reorderBytes + size <= m_settings.receiveWindowBytes )
    {
        SInFrame* waiting = GUCEF_NEW SInFrame();
        waiting->frame.CopyFrom( size, frame );
        waiting->protection = protection;
        m_reorder[ header.seq ] = waiting;
        m_reorderBytes += size;
    }
    // beyond the buffer the frame is dropped: the sender sends it again
    m_ackNow = true;
}

/*-------------------------------------------------------------------------*/

void
CRpcSession::OnTick( UInt64 nowMs )
{GUCEF_TRACE;

    if ( !m_transportUp )
        return;

    // retransmission on unreliable transports
    if ( !m_settings.reliableOrdered )
    {
        UInt32 burst = 0;
        bool expired = false;
        ++m_scanDepth;
        for ( UInt32 i=0; i<RetainedCount() && burst < m_settings.maxBurstFrames; ++i )
        {
            SOutFrame* frame = RetainedAt( i );
            if ( !frame->sent || frame->sacked )
                continue;
            if ( nowMs - frame->lastSendMs >= m_rtoMs )
            {
                if ( !Resend( *frame, nowMs ) )
                    break;
                ++m_counters.retransmissions;
                ++burst;
                expired = true;
            }
        }
        --m_scanDepth;
        if ( expired )
        {
            // back off per expiry
            UInt32 doubled = m_rtoMs * 2;
            m_rtoMs = ( 0 != m_settings.maxRtoMs && doubled > m_settings.maxRtoMs ) ? m_settings.maxRtoMs : doubled;
        }
    }

    Flush( nowMs );

    if ( m_deliveredSeq > m_lastAckSent || m_ackNow )
    {
        if ( m_ackNow || nowMs - m_firstUnackedDeliveryMs >= m_settings.ackDelayMs )
            SendAck( nowMs );
    }
}

/*-------------------------------------------------------------------------*/

UInt64
CRpcSession::GetNextSeq( void ) const
{GUCEF_TRACE;

    return m_nextSeq;
}

/*-------------------------------------------------------------------------*/

UInt64
CRpcSession::GetDeliveredSeq( void ) const
{GUCEF_TRACE;

    return m_deliveredSeq;
}

/*-------------------------------------------------------------------------*/

UInt64
CRpcSession::GetOldestRetainedSeq( void ) const
{GUCEF_TRACE;

    return RetainedCount() > 0 ? RetainedAt( 0 )->seq : m_nextSeq;
}

/*-------------------------------------------------------------------------*/

UInt64
CRpcSession::GetPeerAckedSeq( void ) const
{GUCEF_TRACE;

    return m_peerAckedSeq;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcSession::GetRetainedFrameCount( void ) const
{GUCEF_TRACE;

    return RetainedCount();
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcSession::GetRetainedBytes( void ) const
{GUCEF_TRACE;

    return m_retainedBytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcSession::GetQueuedBytes( void ) const
{GUCEF_TRACE;

    return m_queuedBytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcSession::GetInFlightBytes( void ) const
{GUCEF_TRACE;

    return m_inFlightBytes;
}

/*-------------------------------------------------------------------------*/

bool
CRpcSession::HasUnackedDeliveries( void ) const
{GUCEF_TRACE;

    return m_deliveredSeq > m_lastAckSent || m_ackNow;
}

/*-------------------------------------------------------------------------*/

Float64
CRpcSession::GetSmoothedRttMs( void ) const
{GUCEF_TRACE;

    return m_srttMs;
}

/*-------------------------------------------------------------------------*/

const CRpcSession::SCounters&
CRpcSession::GetCounters( void ) const
{GUCEF_TRACE;

    return m_counters;
}

/*-------------------------------------------------------------------------*/

bool
CRpcSession::PrepareResume( UInt64 peerLastReceivedSeq, UInt64 nowMs )
{GUCEF_TRACE;

    if ( peerLastReceivedSeq >= m_nextSeq )
        return false;
    ProcessAck( peerLastReceivedSeq, nowMs );

    // everything still held that was sent goes again, unchanged and in order, before anything new
    m_flushFromSeq = 0;
    for ( UInt32 i=0; i<RetainedCount(); ++i )
    {
        SOutFrame* frame = RetainedAt( i );
        if ( frame->sent )
            frame->needsResend = true;
        frame->sacked = false;
        frame->fastRetransmitted = false;
    }
    m_lastAckSent = 0;
    m_ackNow = m_deliveredSeq > 0;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcSession::Reset( void )
{GUCEF_TRACE;

    while ( RetainedCount() > 0 )
        PopFront();
    m_retained.clear();
    m_retainedHead = 0;
    TReorderMap::iterator i = m_reorder.begin();
    for ( ; i != m_reorder.end(); ++i )
        GUCEF_DELETE (*i).second;
    m_reorder.clear();
    m_reorderBytes = 0;
    m_nextSeq = 1;
    m_flushFromSeq = 0;
    m_deliveredSeq = 0;
    m_peerAckedSeq = 0;
    m_lastAckSent = 0;
    m_retainedBytes = 0;
    m_queuedBytes = 0;
    m_inFlightBytes = 0;
    m_unackedDeliveries = 0;
    m_unackedBytes = 0;
    m_ackNow = false;
    m_highestSacked = 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
