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

#include "gucefDRN_CDRNLinkReliability.h"

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

/** Upper bound of the size of the FRAGMENT header in front of the bytes of a fragment */
static const UInt32 FragmentHeaderMaxSize = 5 + 5 + 5 + 5 + 5 + 1 + 5;

static const UInt32 SentRingSize = DRN_RECEIVE_WINDOW_SIZE;
static const UInt32 HeldRingSize = DRN_RELIABLE_LANE_WINDOW;
static const UInt32 InitialPendingRingSize = 256;

const UInt32 CDRNLinkReliability::MaxPlainSequenceLead;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNSequenceWindow::CDRNSequenceWindow( void )
    : m_any( false )
    , m_highest( 0 )
{GUCEF_TRACE;
    Reset();
}

/*-------------------------------------------------------------------------*/

void
CDRNSequenceWindow::Reset( void )
{GUCEF_TRACE;
    m_any = false;
    m_highest = 0;
    memset( m_bits, 0, sizeof( m_bits ) );
}

/*-------------------------------------------------------------------------*/

bool
CDRNSequenceWindow::IsSet( UInt32 sequence ) const
{GUCEF_TRACE;
    UInt32 index = sequence & ( WindowSize - 1 );
    return 0 != ( m_bits[ index >> 5 ] & ( 1u << ( index & 31 ) ) );
}

/*-------------------------------------------------------------------------*/

void
CDRNSequenceWindow::Set( UInt32 sequence )
{GUCEF_TRACE;
    UInt32 index = sequence & ( WindowSize - 1 );
    m_bits[ index >> 5 ] |= ( 1u << ( index & 31 ) );
}

/*-------------------------------------------------------------------------*/

void
CDRNSequenceWindow::Unset( UInt32 sequence )
{GUCEF_TRACE;
    UInt32 index = sequence & ( WindowSize - 1 );
    m_bits[ index >> 5 ] &= ~( 1u << ( index & 31 ) );
}

/*-------------------------------------------------------------------------*/

CDRNSequenceWindow::TAcceptResult
CDRNSequenceWindow::Accept( UInt32 sequence, UInt32& gap )
{GUCEF_TRACE;
    gap = 0;
    if ( !m_any )
    {
        m_any = true;
        m_highest = sequence;
        memset( m_bits, 0, sizeof( m_bits ) );
        Set( sequence );
        return SEQUENCE_NEWEST;
    }

    if ( DRNIsNewerSequence( sequence, m_highest ) )
    {
        UInt32 distance = sequence - m_highest;
        if ( distance >= WindowSize )
        {
            memset( m_bits, 0, sizeof( m_bits ) );
        }
        else
        {
            for ( UInt32 s = m_highest + 1; s != sequence; ++s )
                Unset( s );
        }
        Set( sequence );
        gap = distance - 1;
        m_highest = sequence;
        return SEQUENCE_NEWEST;
    }

    UInt32 back = m_highest - sequence;
    if ( back >= WindowSize )
        return SEQUENCE_TOO_OLD;
    if ( IsSet( sequence ) )
        return SEQUENCE_DUPLICATE;
    Set( sequence );
    return SEQUENCE_LATE;
}

/*-------------------------------------------------------------------------*/

bool
CDRNSequenceWindow::HasAny( void ) const
{GUCEF_TRACE;
    return m_any;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNSequenceWindow::GetHighest( void ) const
{GUCEF_TRACE;
    return m_highest;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNSequenceWindow::GetAckBits( void ) const
{GUCEF_TRACE;
    if ( !m_any )
        return 0;
    UInt32 bits = 0;
    for ( UInt32 n=0; n<32; ++n )
    {
        if ( IsSet( m_highest - 1 - n ) )
            bits |= ( 1u << n );
    }
    return bits;
}

/*-------------------------------------------------------------------------*/

CDRNLinkStats::CDRNLinkStats( void )
{GUCEF_TRACE;
    Clear();
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkStats::Clear( void )
{GUCEF_TRACE;
    packetsSent = 0;
    packetsReceived = 0;
    bytesSent = 0;
    bytesReceived = 0;
    messagesSent = 0;
    messagesReceived = 0;
    packetsLost = 0;
    duplicatePackets = 0;
    tooOldPackets = 0;
    staleMessages = 0;
    duplicateMessages = 0;
    packetsAcked = 0;
    messagesResent = 0;
    fragmentedMessages = 0;
    preAuthRefusedPackets = 0;
    rateLimitedPackets = 0;
    windowRefusedPackets = 0;
    plainTooFarPackets = 0;
}

/*-------------------------------------------------------------------------*/

CIDRNLinkMessageSink::~CIDRNLinkMessageSink()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNLinkReliability::CPendingMessage::CPendingMessage( void )
    : id( 0 )
    , inUse( false )
    , acked( false )
    , needsSend( false )
    , lastSendTime( 0 )
    , sendCount( 0 )
    , packetKind( DRN_PACKETKIND_AEAD )
    , lane( 0 )
    , laneSequence( 0 )
    , encoded()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNLinkReliability::CSentPacket::CSentPacket( void )
    : sequence( 0 )
    , valid( false )
    , acked( false )
    , lossHandled( false )
    , sendTime( 0 )
    , pendingIds()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNLinkReliability::CHeldMessage::CHeldMessage( void )
    : used( false )
    , laneSequence( 0 )
    , msgClass( 0 )
    , opcode( 0 )
    , packetKind( DRN_PACKETKIND_AEAD )
    , payload()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNLinkReliability::CFragmentAssembly::CFragmentAssembly( void )
    : msgClass( 0 )
    , opcode( 0 )
    , lane( 0 )
    , totalSize( 0 )
    , count( 0 )
    , received( 0 )
    , packetKind( DRN_PACKETKIND_AEAD )
    , got()
    , data()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNLinkReliability::CUnreliableQueue::CUnreliableQueue( void )
    : queue()
    , sizes()
    , readIndex( 0 )
    , readOffset( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkReliability::CUnreliableQueue::HasMessages( void ) const
{GUCEF_TRACE;
    return readIndex < sizes.size();
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkReliability::CUnreliableQueue::Clear( void )
{GUCEF_TRACE;
    queue.SetDataSize( 0 );
    sizes.clear();
    readIndex = 0;
    readOffset = 0;
}

/*-------------------------------------------------------------------------*/

CDRNLinkReliability::CDRNLinkReliability( void )
    : m_transportIsReliable( true )
    , m_maxPacketSize( 0 )
    , m_nextSequence( 1 )
    , m_nextFragmentGroup( 1 )
    , m_streamQueue()
    , m_streamQueueEnd( DRN_MAX_PACKET_HEADER_SIZE )
    , m_streamBodyStart( DRN_MAX_PACKET_HEADER_SIZE )
    , m_streamRuns()
    , m_streamRunsHead( 0 )
    , m_needsSendMacCount( 0 )
    , m_nextPlainSequence( 1 )
    , m_pending()
    , m_pendingHead( 0 )
    , m_pendingNext( 0 )
    , m_releaseNext( 0 )
    , m_needsSendCount( 0 )
    , m_sent()
    , m_packetIds()
    , m_datagram()
    , m_scratch()
    , m_receiveWindow()
    , m_plainReceiveWindow()
    , m_highestReceivedSendTime( 0 )
    , m_highestReceivedLocalTime( 0 )
    , m_ackOwed( false )
    , m_ackOwedSince( 0 )
    , m_lane2Any( false )
    , m_lane2Newest( 0 )
    , m_lane0Expected( 0 )
    , m_held()
    , m_heldCount( 0 )
    , m_heldRingSize( 0 )
    , m_lane1Base( 0 )
    , m_fragments()
    , m_fragmentBytes( 0 )
    , m_currentPacketSequence( 0 )
    , m_preAuthRestricted( false )
    , m_maxMessageSize( 0 )
    , m_maxFragmentGroups( 0 )
    , m_maxFragmentBytes( 0 )
    , m_limitViolation( LIMIT_NONE )
    , m_stats()
{GUCEF_TRACE;

    m_nextLaneSequence[ 0 ] = 0;
    m_nextLaneSequence[ 1 ] = 0;
    m_laneFloorId[ 0 ] = 0;
    m_laneFloorId[ 1 ] = 0;
    memset( m_lane1Received, 0, sizeof( m_lane1Received ) );
    Reset( true, 0, 1 );
}

/*-------------------------------------------------------------------------*/

CDRNLinkReliability::~CDRNLinkReliability()
{GUCEF_TRACE;

    ClearFragments();
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkReliability::ClearFragments( void )
{GUCEF_TRACE;
    TFragmentMap::iterator i = m_fragments.begin();
    while ( i != m_fragments.end() )
    {
        GUCEF_DELETE (*i).second;
        ++i;
    }
    m_fragments.clear();
    m_fragmentBytes = 0;
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkReliability::Reset( bool transportIsReliable, UInt32 maxPacketSize, UInt32 firstSequence )
{GUCEF_TRACE;

    m_transportIsReliable = transportIsReliable;
    m_maxPacketSize = maxPacketSize;
    m_nextSequence = 0 != firstSequence ? firstSequence : 1;
    m_nextLaneSequence[ 0 ] = 0;
    m_nextLaneSequence[ 1 ] = 0;
    m_nextFragmentGroup = 1;

    m_streamQueue.SetDataSize( 0 );
    m_streamQueueEnd = DRN_MAX_PACKET_HEADER_SIZE;
    m_streamBodyStart = DRN_MAX_PACKET_HEADER_SIZE;
    m_streamRuns.clear();
    m_streamRunsHead = 0;
    for ( UInt32 k=0; k<DRN_PACKETKIND_COUNT; ++k )
        m_unreliable[ k ].Clear();
    m_needsSendMacCount = 0;
    m_nextPlainSequence = 1;
    m_plainReceiveWindow.Reset();

    m_pending.clear();
    if ( !m_transportIsReliable )
        m_pending.resize( InitialPendingRingSize );
    m_pendingHead = 0;
    m_pendingNext = 0;
    m_releaseNext = 0;
    m_laneFloorId[ 0 ] = 0;
    m_laneFloorId[ 1 ] = 0;
    m_needsSendCount = 0;

    m_sent.clear();
    m_sent.resize( SentRingSize );
    m_packetIds.clear();

    m_receiveWindow.Reset();
    m_highestReceivedSendTime = 0;
    m_highestReceivedLocalTime = 0;
    m_ackOwed = false;
    m_ackOwedSince = 0;
    m_lane2Any = false;
    m_lane2Newest = 0;
    m_lane0Expected = 0;
    m_held.clear();
    m_heldRingSize = 0;
    if ( !m_transportIsReliable )
    {
        m_held.resize( HeldRingSize );
        m_heldRingSize = HeldRingSize;
    }
    m_heldCount = 0;
    m_lane1Base = 0;
    memset( m_lane1Received, 0, sizeof( m_lane1Received ) );
    ClearFragments();
    m_currentPacketSequence = 0;
    m_preAuthRestricted = false;
    m_limitViolation = LIMIT_NONE;
    m_stats.Clear();
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkReliability::ResizeHeldRing( UInt32 newSize )
{GUCEF_TRACE;

    if ( m_transportIsReliable || newSize == m_heldRingSize )
        return;

    // the messages held back move to their slot in the ring of the new size
    THeldVector resized( newSize );
    for ( UInt32 i=0; i<m_held.size(); ++i )
    {
        CHeldMessage& from = m_held[ i ];
        if ( !from.used )
            continue;
        CHeldMessage& to = resized[ from.laneSequence & ( newSize - 1 ) ];
        to.used = true;
        to.laneSequence = from.laneSequence;
        to.msgClass = from.msgClass;
        to.opcode = from.opcode;
        to.packetKind = from.packetKind;
        to.payload.Swap( from.payload );
    }
    m_held.swap( resized );
    m_heldRingSize = newSize;
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkReliability::SetPreAuthRestricted( bool restricted )
{GUCEF_TRACE;

    m_preAuthRestricted = restricted;
    ResizeHeldRing( restricted ? DRN_PREAUTH_HOLD_SLOTS : HeldRingSize );
}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkReliability::IsPreAuthRestricted( void ) const
{GUCEF_TRACE;
    return m_preAuthRestricted;
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkReliability::SetReceiveLimits( UInt32 maxMessageSize, UInt32 maxFragmentGroups, UInt32 maxFragmentBytes )
{GUCEF_TRACE;

    m_maxMessageSize = maxMessageSize;
    m_maxFragmentGroups = maxFragmentGroups;
    m_maxFragmentBytes = maxFragmentBytes;
}

/*-------------------------------------------------------------------------*/

CDRNLinkReliability::TLimitViolation
CDRNLinkReliability::GetLimitViolation( void ) const
{GUCEF_TRACE;
    return m_limitViolation;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNLinkReliability::GetOpenFragmentGroups( void ) const
{GUCEF_TRACE;
    return (UInt32) m_fragments.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNLinkReliability::GetOpenFragmentBytes( void ) const
{GUCEF_TRACE;
    return m_fragmentBytes;
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkReliability::NoteRateLimitedPacket( void )
{GUCEF_TRACE;
    ++m_stats.rateLimitedPackets;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNLinkReliability::GetBodyBudget( void ) const
{GUCEF_TRACE;
    if ( m_maxPacketSize <= DRN_MAX_PACKET_HEADER_SIZE + DRN_MAX_MESSAGE_HEADER_SIZE + FragmentHeaderMaxSize + 16 )
        return 512;
    return m_maxPacketSize - DRN_MAX_PACKET_HEADER_SIZE;
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkReliability::EncodeMessage( CDRNWireWriter& writer   ,
                                    UInt8 lane               ,
                                    UInt8 msgClass           ,
                                    UInt32 opcode            ,
                                    UInt32 laneSequence      ,
                                    const void* part1        ,
                                    UInt32 size1             ,
                                    const void* part2        ,
                                    UInt32 size2             )
{GUCEF_TRACE;
    writer.WriteUInt8( msgClass );
    writer.WriteVarUInt32( opcode );
    writer.WriteUInt8( lane );
    if ( DRNIsReliableLane( lane ) )
        writer.WriteVarUInt32( laneSequence );
    writer.WriteVarUInt32( size1 + size2 );
    if ( size1 > 0 )
        writer.WriteBytes( part1, size1 );
    if ( size2 > 0 )
        writer.WriteBytes( part2, size2 );
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkReliability::GrowPending( void )
{GUCEF_TRACE;

    UInt32 oldSize = (UInt32) m_pending.size();
    UInt32 newSize = oldSize > 0 ? oldSize * 2 : InitialPendingRingSize;
    TPendingVector grown( newSize );
    for ( UInt32 id = m_pendingHead; id != m_pendingNext; ++id )
    {
        CPendingMessage& from = m_pending[ id & ( oldSize - 1 ) ];
        CPendingMessage& to = grown[ id & ( newSize - 1 ) ];
        to.id = from.id;
        to.inUse = from.inUse;
        to.acked = from.acked;
        to.needsSend = from.needsSend;
        to.lastSendTime = from.lastSendTime;
        to.sendCount = from.sendCount;
        to.packetKind = from.packetKind;
        to.lane = from.lane;
        to.laneSequence = from.laneSequence;
        to.encoded.Swap( from.encoded );
    }
    m_pending.swap( grown );
}

/*-------------------------------------------------------------------------*/

CDRNLinkReliability::CPendingMessage&
CDRNLinkReliability::AddPending( UInt8 packetKind, UInt8 lane, UInt32 laneSequence )
{GUCEF_TRACE;
    if ( m_pendingNext - m_pendingHead >= (UInt32) m_pending.size() )
        GrowPending();

    // it waits for its lane window, ReleaseWithinLaneWindows() makes it due
    UInt32 id = m_pendingNext++;
    CPendingMessage& entry = m_pending[ id & ( m_pending.size() - 1 ) ];
    entry.id = id;
    entry.inUse = true;
    entry.acked = false;
    entry.needsSend = false;
    entry.lastSendTime = 0;
    entry.sendCount = 0;
    entry.packetKind = packetKind;
    entry.lane = lane;
    entry.laneSequence = laneSequence;
    entry.encoded.SetDataSize( 0 );
    return entry;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNLinkReliability::GetLaneFloor( UInt8 lane )
{GUCEF_TRACE;

    // the lane sequence of the oldest message of the lane that is not acknowledged, the next lane sequence when there
    // is none. The search goes on from where it ended before: a message once acknowledged stays so, so it visits
    // every message once
    UInt32 mask = (UInt32) m_pending.size() - 1;
    UInt32 id = m_laneFloorId[ lane ];
    if ( (UInt32) ( id - m_pendingHead ) > (UInt32) ( m_pendingNext - m_pendingHead ) )
        id = m_pendingHead;
    while ( id != m_pendingNext )
    {
        const CPendingMessage& entry = m_pending[ id & mask ];
        if ( !entry.acked && entry.lane == lane )
            break;
        ++id;
    }
    m_laneFloorId[ lane ] = id;
    return id != m_pendingNext ? m_pending[ id & mask ].laneSequence : m_nextLaneSequence[ lane ];
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkReliability::ReleaseWithinLaneWindows( void )
{GUCEF_TRACE;

    // a message may go out once its lane sequence is within the window of its lane from the oldest message of the lane
    // not acknowledged: the receiver takes every lane sequence of that window, so nothing it acknowledges is dropped.
    // In the order of the queue: a message of one lane that waits holds back the later ones of the other lane, the
    // oldest one of every lane is always due, so the window always moves on
    UInt32 mask = (UInt32) m_pending.size() - 1;
    while ( m_releaseNext != m_pendingNext )
    {
        CPendingMessage& entry = m_pending[ m_releaseNext & mask ];
        if ( (UInt32) ( entry.laneSequence - GetLaneFloor( entry.lane ) ) >= DRN_RELIABLE_LANE_WINDOW )
            break;
        entry.needsSend = true;
        ++m_needsSendCount;
        if ( DRN_PACKETKIND_MAC == entry.packetKind )
            ++m_needsSendMacCount;
        ++m_releaseNext;
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkReliability::QueueReliableDatagram( UInt8 lane            ,
                                            UInt8 msgClass        ,
                                            UInt32 opcode         ,
                                            const void* part1     ,
                                            UInt32 size1          ,
                                            const void* part2     ,
                                            UInt32 size2          ,
                                            UInt8 packetKind      )
{GUCEF_TRACE;
    UInt32 budget = GetBodyBudget();
    UInt32 payloadSize = size1 + size2;
    UInt32 encodedSize = CDRNPacketCodec::GetMessageSize( opcode, lane, m_nextLaneSequence[ lane ], payloadSize );
    if ( encodedSize <= budget )
    {
        CPendingMessage& entry = AddPending( packetKind, lane, m_nextLaneSequence[ lane ] );
        CDRNWireWriter writer( entry.encoded );
        EncodeMessage( writer, lane, msgClass, opcode, m_nextLaneSequence[ lane ]++, part1, size1, part2, size2 );
        ReleaseWithinLaneWindows();
        return true;
    }

    // too large for a datagram: FRAGMENT messages on the same lane, joined on arrival
    const UInt8* payload = static_cast< const UInt8* >( part1 );
    if ( size2 > 0 )
    {
        m_scratch.SetDataSize( 0 );
        CDRNWireWriter scratchWriter( m_scratch );
        scratchWriter.WriteBytes( part1, size1 );
        scratchWriter.WriteBytes( part2, size2 );
        payload = static_cast< const UInt8* >( m_scratch.GetConstBufferPtr() );
    }

    UInt32 chunkSize = budget - DRN_MAX_MESSAGE_HEADER_SIZE - FragmentHeaderMaxSize;
    UInt32 count = ( payloadSize + chunkSize - 1 ) / chunkSize;
    UInt32 group = m_nextFragmentGroup++;
    UInt8 header[ FragmentHeaderMaxSize + 8 ];
    for ( UInt32 i=0; i<count; ++i )
    {
        UInt32 offset = i * chunkSize;
        UInt32 bytes = payloadSize - offset < chunkSize ? payloadSize - offset : chunkSize;

        UInt32 headerSize = 0;
        headerSize += CDRNWireWriter::EncodeVarUInt64( group, header + headerSize );
        headerSize += CDRNWireWriter::EncodeVarUInt64( i, header + headerSize );
        headerSize += CDRNWireWriter::EncodeVarUInt64( count, header + headerSize );
        headerSize += CDRNWireWriter::EncodeVarUInt64( payloadSize, header + headerSize );
        headerSize += CDRNWireWriter::EncodeVarUInt64( offset, header + headerSize );
        header[ headerSize++ ] = msgClass;
        headerSize += CDRNWireWriter::EncodeVarUInt64( opcode, header + headerSize );

        CPendingMessage& entry = AddPending( packetKind, lane, m_nextLaneSequence[ lane ] );
        CDRNWireWriter writer( entry.encoded );
        EncodeMessage( writer, lane, DRN_MSGCLASS_LINK, DRN_LINK_FRAGMENT, m_nextLaneSequence[ lane ]++, header, headerSize, payload + offset, bytes );
    }
    ++m_stats.fragmentedMessages;
    ReleaseWithinLaneWindows();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkReliability::QueueMessage( UInt8 lane                ,
                                   UInt8 msgClass            ,
                                   UInt32 opcode             ,
                                   const void* part1         ,
                                   UInt32 size1              ,
                                   const void* part2         ,
                                   UInt32 size2              ,
                                   UInt8 packetKind          )
{GUCEF_TRACE;
    if ( lane >= DRN_LANE_COUNT )
        return false;
    if ( 0 == size1 )
        part1 = GUCEF_NULL;
    if ( 0 == size2 )
        part2 = GUCEF_NULL;

    ++m_stats.messagesSent;

    // PLAIN carries lane 3 only: anything else of a public item goes in MAC packets
    if ( DRN_PACKETKIND_PLAIN == packetKind && DRN_LANE_UNRELIABLE != lane )
        packetKind = DRN_PACKETKIND_MAC;
    if ( packetKind >= DRN_PACKETKIND_COUNT )
        packetKind = DRN_PACKETKIND_AEAD;

    if ( m_transportIsReliable )
    {
        if ( m_streamQueue.GetDataSize() < m_streamQueueEnd )
            m_streamQueue.SetDataSize( m_streamQueueEnd );
        UInt32 before = m_streamQueueEnd;
        CDRNWireWriter writer( m_streamQueue, m_streamQueueEnd );
        UInt32 laneSequence = DRNIsReliableLane( lane ) ? m_nextLaneSequence[ lane ]++ : 0;
        EncodeMessage( writer, lane, msgClass, opcode, laneSequence, part1, size1, part2, size2 );
        m_streamQueueEnd = writer.GetOffset();

        // runs of equal kind are only kept while something else than AEAD is queued: one packet per run, in order
        if ( DRN_PACKETKIND_AEAD != packetKind || m_streamRunsHead < m_streamRuns.size() )
        {
            if ( m_streamRunsHead >= m_streamRuns.size() && before > m_streamBodyStart )
            {
                CStreamRun aeadRun;
                aeadRun.end = before;
                aeadRun.packetKind = DRN_PACKETKIND_AEAD;
                m_streamRuns.push_back( aeadRun );
            }
            if ( m_streamRunsHead < m_streamRuns.size() && m_streamRuns.back().packetKind == packetKind )
            {
                m_streamRuns.back().end = m_streamQueueEnd;
            }
            else
            {
                CStreamRun run;
                run.end = m_streamQueueEnd;
                run.packetKind = packetKind;
                m_streamRuns.push_back( run );
            }
        }
        return true;
    }

    if ( DRNIsReliableLane( lane ) )
        return QueueReliableDatagram( lane, msgClass, opcode, part1, size1, part2, size2, packetKind );

    // unreliable lanes on a datagram channel: never fragmented, the sender keeps them within the budget
    UInt32 encodedSize = CDRNPacketCodec::GetMessageSize( opcode, lane, 0, size1 + size2 );
    if ( encodedSize > GetBodyBudget() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNLinkReliability: An unreliable message of " << ( size1 + size2 ) << " bytes does not fit in a datagram of " << m_maxPacketSize << " bytes" );
        --m_stats.messagesSent;
        return false;
    }
    CUnreliableQueue& unreliable = m_unreliable[ packetKind ];
    CDRNWireWriter writer( unreliable.queue, unreliable.queue.GetDataSize() );
    EncodeMessage( writer, lane, msgClass, opcode, 0, part1, size1, part2, size2 );
    unreliable.sizes.push_back( encodedSize );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkReliability::HasQueuedOutput( void ) const
{GUCEF_TRACE;
    if ( m_transportIsReliable )
        return m_streamQueueEnd > m_streamBodyStart;
    return m_needsSendCount > 0 || m_unreliable[ DRN_PACKETKIND_AEAD ].HasMessages() || m_unreliable[ DRN_PACKETKIND_MAC ].HasMessages() ||
           m_unreliable[ DRN_PACKETKIND_PLAIN ].HasMessages();
}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkReliability::IsAckOwed( void ) const
{GUCEF_TRACE;
    return m_ackOwed;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNLinkReliability::GetAckOwedSince( void ) const
{GUCEF_TRACE;
    return m_ackOwedSince;
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkReliability::FinishPacket( CDRNWireWriter& writer, UInt32 bodyStart, UInt64 now, const UInt8*& packet, UInt32& packetSize, UInt8 packetKind )
{GUCEF_TRACE;
    bool plain = DRN_PACKETKIND_PLAIN == packetKind;
    CDRNPacketHeader header;
    header.sequence = plain ? m_nextPlainSequence : m_nextSequence;
    header.sendTime = now;

    // a PLAIN packet can be forged by anyone on the path: it acknowledges and echoes nothing
    if ( !plain && m_receiveWindow.HasAny() )
    {
        header.ackSequence = m_receiveWindow.GetHighest();
        header.ackBits = m_receiveWindow.GetAckBits();
        header.echoTime = m_highestReceivedSendTime;
        header.echoHold = now >= m_highestReceivedLocalTime ? now - m_highestReceivedLocalTime : 0;
    }

    // the header goes right in front of the body, in the room left for it
    CORE::CDynamicBuffer& buffer = writer.GetBuffer();
    UInt32 bodyEnd = writer.GetOffset();
    UInt8 headerBytes[ DRN_MAX_PACKET_HEADER_SIZE ];
    UInt32 headerSize = 0;
    {
        UInt8* p = headerBytes;
        p[ headerSize++ ] = plain ? DRN_PROTOCOL_ID_PUBLIC_PLAIN : DRN_PROTOCOL_ID;
        headerSize += CDRNWireWriter::EncodeVarUInt64( header.sequence, p + headerSize );
        headerSize += CDRNWireWriter::EncodeVarUInt64( header.sendTime, p + headerSize );
        headerSize += CDRNWireWriter::EncodeVarUInt64( header.ackSequence, p + headerSize );
        p[ headerSize++ ] = (UInt8) ( header.ackBits >> 24 );
        p[ headerSize++ ] = (UInt8) ( header.ackBits >> 16 );
        p[ headerSize++ ] = (UInt8) ( header.ackBits >> 8 );
        p[ headerSize++ ] = (UInt8) header.ackBits;
        headerSize += CDRNWireWriter::EncodeVarUInt64( header.echoTime, p + headerSize );
        headerSize += CDRNWireWriter::EncodeVarUInt64( header.echoHold, p + headerSize );
    }
    UInt32 packetStart = bodyStart - headerSize;
    buffer.CopyFrom( packetStart, headerSize, headerBytes );
    packet = static_cast< const UInt8* >( buffer.GetConstBufferPtr( packetStart ) );
    packetSize = bodyEnd - packetStart;

    if ( plain )
    {
        ++m_nextPlainSequence;
        if ( 0 == m_nextPlainSequence )
            m_nextPlainSequence = 1;
        ++m_stats.packetsSent;
        m_stats.bytesSent += packetSize;
        return;
    }

    CSentPacket& record = m_sent[ header.sequence & ( SentRingSize - 1 ) ];
    record.sequence = header.sequence;
    record.valid = true;
    record.acked = false;
    record.lossHandled = false;
    record.sendTime = now;
    record.pendingIds.swap( m_packetIds );
    m_packetIds.clear();

    ++m_nextSequence;
    if ( 0 == m_nextSequence )
        m_nextSequence = 1;     // 0 means "none" as an ack sequence

    m_ackOwed = false;
    ++m_stats.packetsSent;
    m_stats.bytesSent += packetSize;
}

/*-------------------------------------------------------------------------*/

UInt8
CDRNLinkReliability::ChooseDatagramKind( void ) const
{GUCEF_TRACE;
    // control and confidential first, then MAC, then PLAIN
    if ( m_needsSendCount > m_needsSendMacCount || m_unreliable[ DRN_PACKETKIND_AEAD ].HasMessages() )
        return DRN_PACKETKIND_AEAD;
    if ( m_needsSendMacCount > 0 || m_unreliable[ DRN_PACKETKIND_MAC ].HasMessages() )
        return DRN_PACKETKIND_MAC;
    if ( m_unreliable[ DRN_PACKETKIND_PLAIN ].HasMessages() )
        return DRN_PACKETKIND_PLAIN;
    return DRN_PACKETKIND_AEAD;
}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkReliability::BuildPacket( UInt64 now, bool force, const UInt8*& packet, UInt32& packetSize )
{GUCEF_TRACE;
    UInt8 packetKind = DRN_PACKETKIND_AEAD;
    return BuildPacket( now, force, packet, packetSize, packetKind );
}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkReliability::BuildPacket( UInt64 now, bool force, const UInt8*& packet, UInt32& packetSize, UInt8& packetKind )
{GUCEF_TRACE;
    packetKind = DRN_PACKETKIND_AEAD;
    if ( m_transportIsReliable )
    {
        bool hasBody = m_streamQueueEnd > m_streamBodyStart;
        if ( !hasBody && !force )
            return false;
        if ( m_streamQueue.GetDataSize() < m_streamQueueEnd )
            m_streamQueue.SetDataSize( m_streamQueueEnd );

        if ( m_streamRunsHead < m_streamRuns.size() )
        {
            // one packet per run of equal kind, in queue order: the order of the stream is the order of delivery.
            // The header of a later run goes in front of it, over the end of the run before, which went out already
            const CStreamRun run = m_streamRuns[ m_streamRunsHead++ ];
            packetKind = run.packetKind;
            CDRNWireWriter writer( m_streamQueue, run.end );
            FinishPacket( writer, m_streamBodyStart, now, packet, packetSize, packetKind );
            m_streamBodyStart = run.end;
            if ( m_streamRunsHead >= m_streamRuns.size() )
            {
                m_streamRuns.clear();
                m_streamRunsHead = 0;
                if ( m_streamBodyStart >= m_streamQueueEnd )
                {
                    m_streamQueueEnd = DRN_MAX_PACKET_HEADER_SIZE;
                    m_streamBodyStart = DRN_MAX_PACKET_HEADER_SIZE;
                }
            }
            return true;
        }

        CDRNWireWriter writer( m_streamQueue, m_streamQueueEnd );
        FinishPacket( writer, m_streamBodyStart, now, packet, packetSize, DRN_PACKETKIND_AEAD );
        m_streamQueueEnd = DRN_MAX_PACKET_HEADER_SIZE;
        m_streamBodyStart = DRN_MAX_PACKET_HEADER_SIZE;
        return true;
    }

    // a datagram never mixes kinds; while everything is AEAD this is the one kind there is
    packetKind = ChooseDatagramKind();
    UInt32 budget = GetBodyBudget();
    if ( m_datagram.GetDataSize() < DRN_MAX_PACKET_HEADER_SIZE )
        m_datagram.SetDataSize( DRN_MAX_PACKET_HEADER_SIZE );
    CDRNWireWriter writer( m_datagram, DRN_MAX_PACKET_HEADER_SIZE );
    UInt32 bodySize = 0;
    bool full = false;

    // reliable messages that are due go first
    UInt32 dueOfKind = DRN_PACKETKIND_AEAD == packetKind ? m_needsSendCount - m_needsSendMacCount : ( DRN_PACKETKIND_MAC == packetKind ? m_needsSendMacCount : 0 );
    if ( dueOfKind > 0 )
    {
        UInt32 mask = (UInt32) m_pending.size() - 1;
        for ( UInt32 id = m_pendingHead; id != m_pendingNext; ++id )
        {
            CPendingMessage& entry = m_pending[ id & mask ];
            if ( !entry.inUse || entry.acked || !entry.needsSend || entry.packetKind != packetKind )
                continue;
            UInt32 size = entry.encoded.GetDataSize();
            if ( bodySize + size > budget )
            {
                full = true;
                break;
            }
            writer.WriteBytes( entry.encoded.GetConstBufferPtr(), size );
            bodySize += size;
            entry.needsSend = false;
            --m_needsSendCount;
            if ( DRN_PACKETKIND_MAC == entry.packetKind )
                --m_needsSendMacCount;
            --dueOfKind;
            if ( entry.sendCount > 0 )
                ++m_stats.messagesResent;
            ++entry.sendCount;
            entry.lastSendTime = now;
            m_packetIds.push_back( id );
            if ( 0 == dueOfKind )
                break;
        }
    }

    // then the unreliable ones while there is room
    if ( !full )
    {
        CUnreliableQueue& unreliable = m_unreliable[ packetKind ];
        while ( unreliable.readIndex < unreliable.sizes.size() )
        {
            UInt32 size = unreliable.sizes[ unreliable.readIndex ];
            if ( bodySize + size > budget )
                break;
            writer.WriteBytes( unreliable.queue.GetConstBufferPtr( unreliable.readOffset ), size );
            bodySize += size;
            unreliable.readOffset += size;
            ++unreliable.readIndex;
        }
        if ( unreliable.readIndex >= unreliable.sizes.size() )
            unreliable.Clear();
    }

    if ( 0 == bodySize && !force && !m_ackOwed )
        return false;
    if ( 0 == bodySize && DRN_PACKETKIND_PLAIN == packetKind )
        packetKind = DRN_PACKETKIND_AEAD;      // an ack only packet is never PLAIN

    FinishPacket( writer, DRN_MAX_PACKET_HEADER_SIZE, now, packet, packetSize, packetKind );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkReliability::CheckRetransmits( UInt64 now, UInt64 rto )
{GUCEF_TRACE;
    if ( m_transportIsReliable || m_pendingHead == m_pendingNext )
        return;

    // only the messages within their lane window were ever sent
    UInt32 mask = (UInt32) m_pending.size() - 1;
    for ( UInt32 id = m_pendingHead; id != m_releaseNext; ++id )
    {
        CPendingMessage& entry = m_pending[ id & mask ];
        if ( entry.inUse && !entry.acked && !entry.needsSend && entry.sendCount > 0 && now - entry.lastSendTime >= rto )
        {
            entry.needsSend = true;
            ++m_needsSendCount;
            if ( DRN_PACKETKIND_MAC == entry.packetKind )
                ++m_needsSendMacCount;
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNLinkReliability::GetUnackedCount( void ) const
{GUCEF_TRACE;
    return m_pendingNext - m_pendingHead;
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkReliability::AckPending( UInt32 id )
{GUCEF_TRACE;
    if ( (UInt32) ( id - m_pendingHead ) >= (UInt32) ( m_pendingNext - m_pendingHead ) )
        return;

    UInt32 mask = (UInt32) m_pending.size() - 1;
    CPendingMessage& entry = m_pending[ id & mask ];
    if ( !entry.inUse || entry.id != id || entry.acked )
        return;
    entry.acked = true;
    if ( entry.needsSend )
    {
        entry.needsSend = false;
        --m_needsSendCount;
        if ( DRN_PACKETKIND_MAC == entry.packetKind )
            --m_needsSendMacCount;
    }

    // let go of the acknowledged messages at the front
    while ( m_pendingHead != m_pendingNext )
    {
        CPendingMessage& front = m_pending[ m_pendingHead & mask ];
        if ( !front.inUse || !front.acked )
            break;
        front.inUse = false;
        front.encoded.SetDataSize( 0 );
        ++m_pendingHead;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkReliability::MarkForResend( UInt32 id )
{GUCEF_TRACE;
    if ( (UInt32) ( id - m_pendingHead ) >= (UInt32) ( m_pendingNext - m_pendingHead ) )
        return;

    CPendingMessage& entry = m_pending[ id & ( m_pending.size() - 1 ) ];
    if ( entry.inUse && entry.id == id && !entry.acked && !entry.needsSend )
    {
        entry.needsSend = true;
        ++m_needsSendCount;
        if ( DRN_PACKETKIND_MAC == entry.packetKind )
            ++m_needsSendMacCount;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkReliability::AckSentPacket( UInt32 sequence )
{GUCEF_TRACE;
    CSentPacket& record = m_sent[ sequence & ( SentRingSize - 1 ) ];
    if ( !record.valid || record.sequence != sequence || record.acked )
        return;
    record.acked = true;
    ++m_stats.packetsAcked;
    for ( UInt32 i=0; i<record.pendingIds.size(); ++i )
        AckPending( record.pendingIds[ i ] );
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkReliability::ProcessAcks( const CDRNPacketHeader& header, UInt64 now, UInt64 lossDelay )
{GUCEF_TRACE;
    if ( 0 == header.ackSequence )
        return;

    AckSentPacket( header.ackSequence );
    for ( UInt32 n=0; n<32; ++n )
    {
        if ( 0 != ( header.ackBits & ( 1u << n ) ) )
            AckSentPacket( header.ackSequence - 1 - n );
    }

    if ( m_transportIsReliable )
        return;

    // a packet older than an acknowledged one that is still not acknowledged after the loss delay is lost:
    // its reliable messages go out again with the next packet
    for ( UInt32 n=0; n<32; ++n )
    {
        if ( 0 != ( header.ackBits & ( 1u << n ) ) )
            continue;
        UInt32 sequence = header.ackSequence - 1 - n;
        CSentPacket& record = m_sent[ sequence & ( SentRingSize - 1 ) ];
        if ( !record.valid || record.sequence != sequence || record.acked || record.lossHandled )
            continue;
        if ( now - record.sendTime < lossDelay )
            continue;
        record.lossHandled = true;
        for ( UInt32 i=0; i<record.pendingIds.size(); ++i )
            MarkForResend( record.pendingIds[ i ] );
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkReliability::HandleFragment( const CDRNMessageView& message, CIDRNLinkMessageSink& sink )
{GUCEF_TRACE;
    CDRNWireReader reader( message.payload, message.payloadSize );
    UInt32 group = 0, index = 0, count = 0, totalSize = 0, offset = 0, opcode = 0;
    UInt8 msgClass = 0;
    if ( !reader.ReadVarUInt32( group ) || !reader.ReadVarUInt32( index ) || !reader.ReadVarUInt32( count ) ||
         !reader.ReadVarUInt32( totalSize ) || !reader.ReadVarUInt32( offset ) || !reader.ReadUInt8( msgClass ) ||
         !reader.ReadVarUInt32( opcode ) )
        return true;

    UInt32 bytes = reader.GetRemaining();
    if ( 0 == count || index >= count || totalSize > DRN_MAX_STREAM_PACKET_SIZE || offset + bytes > totalSize )
        return true;
    if ( 0 != m_maxMessageSize && totalSize > m_maxMessageSize )
    {
        m_limitViolation = LIMIT_MESSAGE_SIZE;
        return false;
    }

    CFragmentAssembly* assembly = GUCEF_NULL;
    TFragmentMap::iterator i = m_fragments.find( group );
    if ( i == m_fragments.end() )
    {
        // a new group: within the number of groups and the bytes a peer may have us hold
        if ( 0 != m_maxFragmentGroups && m_fragments.size() >= m_maxFragmentGroups )
        {
            m_limitViolation = LIMIT_FRAGMENT_GROUPS;
            return false;
        }
        if ( 0 != m_maxFragmentBytes && (UInt64) m_fragmentBytes + totalSize > (UInt64) m_maxFragmentBytes )
        {
            m_limitViolation = LIMIT_FRAGMENT_BYTES;
            return false;
        }
        m_fragmentBytes += totalSize;
        assembly = GUCEF_NEW CFragmentAssembly();
        assembly->msgClass = msgClass;
        assembly->opcode = opcode;
        assembly->lane = message.lane;
        assembly->totalSize = totalSize;
        assembly->packetKind = message.packetKind;
        assembly->count = count;
        assembly->got.resize( count, 0 );
        assembly->data.SetBufferSize( totalSize, false );
        assembly->data.SetDataSize( totalSize );
        m_fragments[ group ] = assembly;
    }
    else
    {
        assembly = (*i).second;
        if ( assembly->count != count || assembly->totalSize != totalSize )
            return true;
    }

    if ( 0 != assembly->got[ index ] )
        return true;
    assembly->got[ index ] = 1;
    if ( message.packetKind > assembly->packetKind )
        assembly->packetKind = message.packetKind;     // a message joined from fragments has the weakest kind of them
    ++assembly->received;
    if ( bytes > 0 )
        assembly->data.CopyFrom( offset, bytes, reader.GetCurrentPtr() );

    if ( assembly->received < assembly->count )
        return true;

    m_fragments.erase( group );
    m_fragmentBytes -= assembly->totalSize;
    CDRNMessageView whole;
    whole.msgClass = assembly->msgClass;
    whole.opcode = assembly->opcode;
    whole.lane = assembly->lane;
    whole.packetKind = assembly->packetKind;
    whole.laneSequence = message.laneSequence;
    whole.payload = static_cast< const UInt8* >( assembly->data.GetConstBufferPtr() );
    whole.payloadSize = assembly->totalSize;
    ++m_stats.messagesReceived;
    bool proceed = sink.OnLinkMessage( whole );
    GUCEF_DELETE assembly;
    return proceed;
}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkReliability::Deliver( const CDRNMessageView& message, CIDRNLinkMessageSink& sink )
{GUCEF_TRACE;
    if ( DRN_MSGCLASS_LINK == message.msgClass && DRN_LINK_FRAGMENT == message.opcode )
        return HandleFragment( message, sink );
    ++m_stats.messagesReceived;
    return sink.OnLinkMessage( message );
}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkReliability::DeliverLaneOrdered( const CDRNMessageView& message, CIDRNLinkMessageSink& sink )
{GUCEF_TRACE;
    UInt32 laneSequence = message.laneSequence;
    if ( laneSequence == m_lane0Expected )
    {
        ++m_lane0Expected;
        if ( !Deliver( message, sink ) )
            return false;

        // hand out what was held back waiting for this one
        while ( m_heldCount > 0 )
        {
            CHeldMessage& held = m_held[ m_lane0Expected & ( m_heldRingSize - 1 ) ];
            if ( !held.used || held.laneSequence != m_lane0Expected )
                break;
            held.used = false;
            --m_heldCount;
            ++m_lane0Expected;

            CDRNMessageView view;
            view.msgClass = held.msgClass;
            view.opcode = held.opcode;
            view.packetKind = held.packetKind;
            view.lane = DRN_LANE_RELIABLE_ORDERED;
            view.laneSequence = held.laneSequence;
            view.payload = static_cast< const UInt8* >( held.payload.GetConstBufferPtr() );
            view.payloadSize = held.payload.GetDataSize();
            if ( !Deliver( view, sink ) )
                return false;
        }
        return true;
    }

    if ( DRNIsNewerSequence( laneSequence, m_lane0Expected ) )
    {
        if ( laneSequence - m_lane0Expected >= m_heldRingSize )
            return true;    // not reached: CheckBeforeTaking() refused the packet, unacknowledged, so it comes again
        CHeldMessage& held = m_held[ laneSequence & ( m_heldRingSize - 1 ) ];
        if ( held.used )
        {
            ++m_stats.duplicateMessages;
            return true;
        }
        held.used = true;
        held.laneSequence = laneSequence;
        held.msgClass = message.msgClass;
        held.opcode = message.opcode;
        held.packetKind = message.packetKind;
        held.payload.SetDataSize( 0 );
        if ( message.payloadSize > 0 )
            held.payload.CopyFrom( 0, message.payloadSize, message.payload );
        ++m_heldCount;
        return true;
    }

    ++m_stats.duplicateMessages;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkReliability::DeliverLaneUnordered( const CDRNMessageView& message, CIDRNLinkMessageSink& sink )
{GUCEF_TRACE;

    // every lane sequence from the oldest one not received on, within the lane window, is remembered: an older one
    // was received, a newer one is not taken (CheckBeforeTaking())
    UInt32 laneSequence = message.laneSequence;
    if ( DRNIsNewerSequence( m_lane1Base, laneSequence ) || laneSequence - m_lane1Base >= DRN_RELIABLE_LANE_WINDOW )
    {
        ++m_stats.duplicateMessages;
        return true;
    }
    UInt32 index = laneSequence & ( DRN_RELIABLE_LANE_WINDOW - 1 );
    UInt32 bit = 1u << ( index & 31 );
    if ( 0 != ( m_lane1Received[ index >> 5 ] & bit ) )
    {
        ++m_stats.duplicateMessages;
        return true;
    }
    m_lane1Received[ index >> 5 ] |= bit;

    // the oldest one not received moves on over what arrived, freeing the slots for the window's far end
    for ( ;; )
    {
        UInt32 baseIndex = m_lane1Base & ( DRN_RELIABLE_LANE_WINDOW - 1 );
        UInt32 baseBit = 1u << ( baseIndex & 31 );
        if ( 0 == ( m_lane1Received[ baseIndex >> 5 ] & baseBit ) )
            break;
        m_lane1Received[ baseIndex >> 5 ] &= ~baseBit;
        ++m_lane1Base;
    }
    return Deliver( message, sink );
}

/*-------------------------------------------------------------------------*/

CDRNLinkReliability::TPacketResult
CDRNLinkReliability::ProcessPacket( const UInt8* data            ,
                                    UInt32 size                  ,
                                    UInt64 now                   ,
                                    UInt64 lossDelay             ,
                                    CIDRNLinkMessageSink& sink   ,
                                    CDRNPacketHeader& header     ,
                                    UInt32& lostBefore           ,
                                    UInt8 packetKind             )
{GUCEF_TRACE;
    lostBefore = 0;
    m_limitViolation = LIMIT_NONE;
    CDRNWireReader reader( data, size );
    if ( !CDRNPacketCodec::ReadHeader( reader, header ) )
        return PACKET_MALFORMED;

    // a packet is acknowledged only when all of it is taken: before the link is operational nothing but the handshake
    // is, and on a datagram channel no reliable message beyond the window of its lane. Of a packet with more nothing is
    // taken, so the sender sends it again
    if ( m_preAuthRestricted || !m_transportIsReliable )
    {
        TPacketResult check = CheckBeforeTaking( reader );
        if ( PACKET_ACCEPTED != check )
        {
            if ( PACKET_REFUSED == check )
                ++m_stats.preAuthRefusedPackets;
            else
            if ( PACKET_WINDOW_FULL == check )
                ++m_stats.windowRefusedPackets;
            return check;
        }
    }

    UInt32 gap = 0;
    CDRNSequenceWindow::TAcceptResult accept = m_receiveWindow.Accept( header.sequence, gap );
    if ( CDRNSequenceWindow::SEQUENCE_DUPLICATE == accept )
    {
        ++m_stats.duplicatePackets;
        return PACKET_DUPLICATE;
    }
    if ( CDRNSequenceWindow::SEQUENCE_TOO_OLD == accept )
    {
        ++m_stats.tooOldPackets;
        return PACKET_TOO_OLD;
    }
    if ( CDRNSequenceWindow::SEQUENCE_NEWEST == accept )
    {
        m_highestReceivedSendTime = header.sendTime;
        m_highestReceivedLocalTime = now;
        lostBefore = gap;
        m_stats.packetsLost += gap;
    }
    ++m_stats.packetsReceived;
    m_stats.bytesReceived += size;

    ProcessAcks( header, now, lossDelay );
    if ( m_releaseNext != m_pendingNext )
        ReleaseWithinLaneWindows();

    m_currentPacketSequence = header.sequence;
    while ( reader.GetRemaining() > 0 )
    {
        CDRNMessageView message;
        if ( !CDRNPacketCodec::ReadMessage( reader, message ) )
            return PACKET_MALFORMED;
        if ( 0 != m_maxMessageSize && message.payloadSize > m_maxMessageSize )
        {
            m_limitViolation = LIMIT_MESSAGE_SIZE;
            return PACKET_LIMIT_EXCEEDED;
        }
        message.packetKind = packetKind;

        if ( DRNIsReliableLane( message.lane ) && !m_transportIsReliable && !m_ackOwed )
        {
            m_ackOwed = true;
            m_ackOwedSince = now;
        }

        bool proceed = true;
        switch ( message.lane )
        {
            case DRN_LANE_UNRELIABLE:
            {
                proceed = Deliver( message, sink );
                break;
            }
            case DRN_LANE_UNRELIABLE_SEQUENCED:
            {
                // newest only: nothing from a packet older than the newest one that delivered on this lane
                if ( m_lane2Any && DRNIsNewerSequence( m_lane2Newest, header.sequence ) )
                {
                    ++m_stats.staleMessages;
                    break;
                }
                m_lane2Any = true;
                m_lane2Newest = header.sequence;
                proceed = Deliver( message, sink );
                break;
            }
            case DRN_LANE_RELIABLE_ORDERED:
            {
                if ( m_transportIsReliable )
                    proceed = Deliver( message, sink );
                else
                    proceed = DeliverLaneOrdered( message, sink );
                break;
            }
            case DRN_LANE_RELIABLE_UNORDERED:
            {
                if ( m_transportIsReliable )
                    proceed = Deliver( message, sink );
                else
                    proceed = DeliverLaneUnordered( message, sink );
                break;
            }
            default:
                break;
        }
        if ( !proceed )
        {
            if ( LIMIT_NONE != m_limitViolation )
                return PACKET_LIMIT_EXCEEDED;
            break;
        }
    }
    return PACKET_ACCEPTED;
}

/*-------------------------------------------------------------------------*/

CDRNLinkReliability::TPacketResult
CDRNLinkReliability::ProcessPlainPacket( const UInt8* data, UInt32 size, CIDRNLinkMessageSink& sink, UInt32& refused )
{GUCEF_TRACE;
    refused = 0;
    m_limitViolation = LIMIT_NONE;
    if ( m_preAuthRestricted )
    {
        ++m_stats.preAuthRefusedPackets;
        return PACKET_REFUSED;
    }

    // the header of a PLAIN packet: its sequence counts in the plain window only; acks and echo are never read
    CDRNWireReader reader( data, size );
    UInt8 protocol = 0;
    UInt32 sequence = 0;
    UInt64 value = 0;
    UInt32 ackSequence = 0;
    UInt32 ackBits = 0;
    if ( !reader.ReadUInt8( protocol ) || DRN_PROTOCOL_ID_PUBLIC_PLAIN != protocol || !reader.ReadVarUInt32( sequence ) ||
         !reader.ReadVarUInt64( value ) || !reader.ReadVarUInt32( ackSequence ) || !reader.ReadUInt32( ackBits ) ||
         !reader.ReadVarUInt64( value ) || !reader.ReadVarUInt64( value ) )
        return PACKET_MALFORMED;

    // anyone can write a PLAIN sequence: one far ahead would carry the window away from the genuine packets (the
    // sender's sequence starts at 1); one behind the window is not judged, it is delivered
    bool tooFar = false;
    if ( m_plainReceiveWindow.HasAny() )
    {
        UInt32 newest = m_plainReceiveWindow.GetHighest();
        tooFar = DRNIsNewerSequence( sequence, newest ) && sequence - newest > MaxPlainSequenceLead;
    }
    else
    {
        tooFar = 0 == sequence || sequence > MaxPlainSequenceLead;
    }
    if ( tooFar )
    {
        ++m_stats.plainTooFarPackets;
        return PACKET_TOO_FAR_AHEAD;
    }
    UInt32 gap = 0;
    CDRNSequenceWindow::TAcceptResult accept = m_plainReceiveWindow.Accept( sequence, gap );
    if ( CDRNSequenceWindow::SEQUENCE_DUPLICATE == accept )
    {
        ++m_stats.duplicatePackets;
        return PACKET_DUPLICATE;
    }
    ++m_stats.packetsReceived;
    m_stats.bytesReceived += size;

    while ( reader.GetRemaining() > 0 )
    {
        CDRNMessageView message;
        if ( !CDRNPacketCodec::ReadMessage( reader, message ) )
            return PACKET_MALFORMED;
        if ( 0 != m_maxMessageSize && message.payloadSize > m_maxMessageSize )
        {
            m_limitViolation = LIMIT_MESSAGE_SIZE;
            return PACKET_LIMIT_EXCEEDED;
        }
        message.packetKind = DRN_PACKETKIND_PLAIN;

        // lane 3 only: a PLAIN message on another lane could suppress or reorder genuine reliable ones
        if ( DRN_LANE_UNRELIABLE != message.lane || DRN_MSGCLASS_LINK == message.msgClass )
        {
            ++refused;
            continue;
        }
        if ( !Deliver( message, sink ) )
            break;
    }
    return PACKET_ACCEPTED;
}

/*-------------------------------------------------------------------------*/

CDRNLinkReliability::TPacketResult
CDRNLinkReliability::CheckBeforeTaking( const CDRNWireReader& packetReader ) const
{GUCEF_TRACE;

    CDRNWireReader reader( packetReader );
    bool restricted = m_preAuthRestricted;
    UInt32 expected = m_lane0Expected;
    while ( reader.GetRemaining() > 0 )
    {
        CDRNMessageView message;
        if ( !CDRNPacketCodec::ReadMessage( reader, message ) )
            return PACKET_MALFORMED;

        bool inOrder = m_transportIsReliable || ( DRN_LANE_RELIABLE_ORDERED == message.lane && message.laneSequence == expected );
        if ( DRN_LANE_RELIABLE_ORDERED == message.lane && message.laneSequence == expected )
            ++expected;

        if ( restricted )
        {
            // the handshake: link messages without fragments on lane 0 or 3, within the slots held back before the
            // link is operational
            if ( DRN_MSGCLASS_LINK != message.msgClass || DRN_LINK_FRAGMENT == message.opcode )
                return PACKET_REFUSED;
            if ( DRN_LANE_RELIABLE_UNORDERED == message.lane || DRN_LANE_UNRELIABLE_SEQUENCED == message.lane )
                return PACKET_REFUSED;
            if ( DRN_LANE_RELIABLE_ORDERED == message.lane && !m_transportIsReliable &&
                 DRNIsNewerSequence( message.laneSequence, m_lane0Expected ) && message.laneSequence - m_lane0Expected >= m_heldRingSize )
                return PACKET_REFUSED;

            // the link is operational once it takes a WELCOME, what follows it is for the operational link
            if ( DRN_LINK_WELCOME == message.opcode && DRN_LANE_RELIABLE_ORDERED == message.lane && inOrder )
                restricted = false;
            continue;
        }

        if ( m_transportIsReliable )
            continue;

        // the lane windows of the operational link (the lane 0 ring has that size from the WELCOME on)
        if ( DRN_LANE_RELIABLE_ORDERED == message.lane &&
             DRNIsNewerSequence( message.laneSequence, m_lane0Expected ) && message.laneSequence - m_lane0Expected >= HeldRingSize )
            return PACKET_WINDOW_FULL;
        if ( DRN_LANE_RELIABLE_UNORDERED == message.lane &&
             !DRNIsNewerSequence( m_lane1Base, message.laneSequence ) && message.laneSequence - m_lane1Base >= DRN_RELIABLE_LANE_WINDOW )
            return PACKET_WINDOW_FULL;
    }
    return PACKET_ACCEPTED;
}

/*-------------------------------------------------------------------------*/

const CDRNLinkStats&
CDRNLinkReliability::GetStats( void ) const
{GUCEF_TRACE;
    return m_stats;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNLinkReliability::GetNextSequence( void ) const
{GUCEF_TRACE;
    return m_nextSequence;
}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkReliability::IsTransportReliable( void ) const
{GUCEF_TRACE;
    return m_transportIsReliable;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNLinkReliability::GetMaxPacketSize( void ) const
{GUCEF_TRACE;
    return m_maxPacketSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNLinkReliability::GetMaxUnfragmentedPayload( void ) const
{GUCEF_TRACE;
    if ( m_transportIsReliable )
    {
        // a stream channel carries every message whole, in a packet of the stream: the stream packet limit and the
        // policy's largest message bound a message, no datagram does. Senders that want small messages (the
        // replicator's message budget) cap this themselves
        UInt32 streamLimit = DRN_MAX_STREAM_PACKET_SIZE - DRN_MAX_PACKET_HEADER_SIZE - DRN_MAX_MESSAGE_HEADER_SIZE;
        if ( 0 != m_maxMessageSize && m_maxMessageSize < streamLimit )
            streamLimit = m_maxMessageSize;
        return streamLimit;
    }
    UInt32 budget = GetBodyBudget();
    return budget > DRN_MAX_MESSAGE_HEADER_SIZE ? budget - DRN_MAX_MESSAGE_HEADER_SIZE : 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
