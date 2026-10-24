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
static const UInt32 HeldRingSize = DRN_RECEIVE_WINDOW_SIZE;
static const UInt32 InitialPendingRingSize = 256;

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
    , got()
    , data()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNLinkReliability::CDRNLinkReliability( void )
    : m_transportIsReliable( true )
    , m_maxPacketSize( 0 )
    , m_nextSequence( 1 )
    , m_nextFragmentGroup( 1 )
    , m_streamQueue()
    , m_streamQueueEnd( DRN_MAX_PACKET_HEADER_SIZE )
    , m_unreliableQueue()
    , m_unreliableSizes()
    , m_unreliableReadIndex( 0 )
    , m_unreliableReadOffset( 0 )
    , m_pending()
    , m_pendingHead( 0 )
    , m_pendingNext( 0 )
    , m_needsSendCount( 0 )
    , m_sent()
    , m_packetIds()
    , m_datagram()
    , m_scratch()
    , m_receiveWindow()
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
    , m_lane1Window()
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
    m_unreliableQueue.SetDataSize( 0 );
    m_unreliableSizes.clear();
    m_unreliableReadIndex = 0;
    m_unreliableReadOffset = 0;

    m_pending.clear();
    if ( !m_transportIsReliable )
        m_pending.resize( InitialPendingRingSize );
    m_pendingHead = 0;
    m_pendingNext = 0;
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
    m_lane1Window.Reset();
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
        to.encoded.Swap( from.encoded );
    }
    m_pending.swap( grown );
}

/*-------------------------------------------------------------------------*/

CDRNLinkReliability::CPendingMessage&
CDRNLinkReliability::AddPending( void )
{GUCEF_TRACE;
    if ( m_pendingNext - m_pendingHead >= (UInt32) m_pending.size() )
        GrowPending();

    UInt32 id = m_pendingNext++;
    CPendingMessage& entry = m_pending[ id & ( m_pending.size() - 1 ) ];
    entry.id = id;
    entry.inUse = true;
    entry.acked = false;
    entry.needsSend = true;
    entry.lastSendTime = 0;
    entry.sendCount = 0;
    entry.encoded.SetDataSize( 0 );
    ++m_needsSendCount;
    return entry;
}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkReliability::QueueReliableDatagram( UInt8 lane            ,
                                            UInt8 msgClass        ,
                                            UInt32 opcode         ,
                                            const void* part1     ,
                                            UInt32 size1          ,
                                            const void* part2     ,
                                            UInt32 size2          )
{GUCEF_TRACE;
    UInt32 budget = GetBodyBudget();
    UInt32 payloadSize = size1 + size2;
    UInt32 encodedSize = CDRNPacketCodec::GetMessageSize( opcode, lane, m_nextLaneSequence[ lane ], payloadSize );
    if ( encodedSize <= budget )
    {
        CPendingMessage& entry = AddPending();
        CDRNWireWriter writer( entry.encoded );
        EncodeMessage( writer, lane, msgClass, opcode, m_nextLaneSequence[ lane ]++, part1, size1, part2, size2 );
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

        CPendingMessage& entry = AddPending();
        CDRNWireWriter writer( entry.encoded );
        EncodeMessage( writer, lane, DRN_MSGCLASS_LINK, DRN_LINK_FRAGMENT, m_nextLaneSequence[ lane ]++, header, headerSize, payload + offset, bytes );
    }
    ++m_stats.fragmentedMessages;
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
                                   UInt32 size2              )
{GUCEF_TRACE;
    if ( lane >= DRN_LANE_COUNT )
        return false;
    if ( 0 == size1 )
        part1 = GUCEF_NULL;
    if ( 0 == size2 )
        part2 = GUCEF_NULL;

    ++m_stats.messagesSent;

    if ( m_transportIsReliable )
    {
        if ( m_streamQueue.GetDataSize() < m_streamQueueEnd )
            m_streamQueue.SetDataSize( m_streamQueueEnd );
        CDRNWireWriter writer( m_streamQueue, m_streamQueueEnd );
        UInt32 laneSequence = DRNIsReliableLane( lane ) ? m_nextLaneSequence[ lane ]++ : 0;
        EncodeMessage( writer, lane, msgClass, opcode, laneSequence, part1, size1, part2, size2 );
        m_streamQueueEnd = writer.GetOffset();
        return true;
    }

    if ( DRNIsReliableLane( lane ) )
        return QueueReliableDatagram( lane, msgClass, opcode, part1, size1, part2, size2 );

    // unreliable lanes on a datagram channel: never fragmented, the sender keeps them within the budget
    UInt32 encodedSize = CDRNPacketCodec::GetMessageSize( opcode, lane, 0, size1 + size2 );
    if ( encodedSize > GetBodyBudget() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNLinkReliability: An unreliable message of " << ( size1 + size2 ) << " bytes does not fit in a datagram of " << m_maxPacketSize << " bytes" );
        --m_stats.messagesSent;
        return false;
    }
    CDRNWireWriter writer( m_unreliableQueue, m_unreliableQueue.GetDataSize() );
    EncodeMessage( writer, lane, msgClass, opcode, 0, part1, size1, part2, size2 );
    m_unreliableSizes.push_back( encodedSize );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNLinkReliability::HasQueuedOutput( void ) const
{GUCEF_TRACE;
    if ( m_transportIsReliable )
        return m_streamQueueEnd > DRN_MAX_PACKET_HEADER_SIZE;
    return m_needsSendCount > 0 || m_unreliableReadIndex < m_unreliableSizes.size();
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
CDRNLinkReliability::FinishPacket( CDRNWireWriter& writer, UInt32 bodyStart, UInt64 now, const UInt8*& packet, UInt32& packetSize )
{GUCEF_TRACE;
    CDRNPacketHeader header;
    header.sequence = m_nextSequence;
    header.sendTime = now;
    if ( m_receiveWindow.HasAny() )
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
        p[ headerSize++ ] = DRN_PROTOCOL_ID;
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

bool
CDRNLinkReliability::BuildPacket( UInt64 now, bool force, const UInt8*& packet, UInt32& packetSize )
{GUCEF_TRACE;
    if ( m_transportIsReliable )
    {
        bool hasBody = m_streamQueueEnd > DRN_MAX_PACKET_HEADER_SIZE;
        if ( !hasBody && !force )
            return false;
        if ( m_streamQueue.GetDataSize() < m_streamQueueEnd )
            m_streamQueue.SetDataSize( m_streamQueueEnd );
        CDRNWireWriter writer( m_streamQueue, m_streamQueueEnd );
        FinishPacket( writer, DRN_MAX_PACKET_HEADER_SIZE, now, packet, packetSize );
        m_streamQueueEnd = DRN_MAX_PACKET_HEADER_SIZE;
        return true;
    }

    UInt32 budget = GetBodyBudget();
    if ( m_datagram.GetDataSize() < DRN_MAX_PACKET_HEADER_SIZE )
        m_datagram.SetDataSize( DRN_MAX_PACKET_HEADER_SIZE );
    CDRNWireWriter writer( m_datagram, DRN_MAX_PACKET_HEADER_SIZE );
    UInt32 bodySize = 0;
    bool full = false;

    // reliable messages that are due go first
    if ( m_needsSendCount > 0 )
    {
        UInt32 mask = (UInt32) m_pending.size() - 1;
        for ( UInt32 id = m_pendingHead; id != m_pendingNext; ++id )
        {
            CPendingMessage& entry = m_pending[ id & mask ];
            if ( !entry.inUse || entry.acked || !entry.needsSend )
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
            if ( entry.sendCount > 0 )
                ++m_stats.messagesResent;
            ++entry.sendCount;
            entry.lastSendTime = now;
            m_packetIds.push_back( id );
            if ( 0 == m_needsSendCount )
                break;
        }
    }

    // then the unreliable ones while there is room
    if ( !full )
    {
        while ( m_unreliableReadIndex < m_unreliableSizes.size() )
        {
            UInt32 size = m_unreliableSizes[ m_unreliableReadIndex ];
            if ( bodySize + size > budget )
                break;
            writer.WriteBytes( m_unreliableQueue.GetConstBufferPtr( m_unreliableReadOffset ), size );
            bodySize += size;
            m_unreliableReadOffset += size;
            ++m_unreliableReadIndex;
        }
        if ( m_unreliableReadIndex >= m_unreliableSizes.size() )
        {
            m_unreliableSizes.clear();
            m_unreliableReadIndex = 0;
            m_unreliableReadOffset = 0;
            m_unreliableQueue.SetDataSize( 0 );
        }
    }

    if ( 0 == bodySize && !force && !m_ackOwed )
        return false;

    FinishPacket( writer, DRN_MAX_PACKET_HEADER_SIZE, now, packet, packetSize );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNLinkReliability::CheckRetransmits( UInt64 now, UInt64 rto )
{GUCEF_TRACE;
    if ( m_transportIsReliable || m_pendingHead == m_pendingNext )
        return;

    UInt32 mask = (UInt32) m_pending.size() - 1;
    for ( UInt32 id = m_pendingHead; id != m_pendingNext; ++id )
    {
        CPendingMessage& entry = m_pending[ id & mask ];
        if ( entry.inUse && !entry.acked && !entry.needsSend && entry.sendCount > 0 && now - entry.lastSendTime >= rto )
        {
            entry.needsSend = true;
            ++m_needsSendCount;
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
            return true;    // beyond what we hold back, the sender sends it again
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

CDRNLinkReliability::TPacketResult
CDRNLinkReliability::ProcessPacket( const UInt8* data            ,
                                    UInt32 size                  ,
                                    UInt64 now                   ,
                                    UInt64 lossDelay             ,
                                    CIDRNLinkMessageSink& sink   ,
                                    CDRNPacketHeader& header     ,
                                    UInt32& lostBefore           )
{GUCEF_TRACE;
    lostBefore = 0;
    m_limitViolation = LIMIT_NONE;
    CDRNWireReader reader( data, size );
    if ( !CDRNPacketCodec::ReadHeader( reader, header ) )
        return PACKET_MALFORMED;

    // before the link is operational nothing but the handshake is taken, and of a packet with more nothing is
    if ( m_preAuthRestricted && !IsAllowedBeforeOperational( reader ) )
    {
        ++m_stats.preAuthRefusedPackets;
        return PACKET_REFUSED;
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
                {
                    proceed = Deliver( message, sink );
                }
                else
                {
                    UInt32 laneGap = 0;
                    CDRNSequenceWindow::TAcceptResult laneAccept = m_lane1Window.Accept( message.laneSequence, laneGap );
                    if ( CDRNSequenceWindow::SEQUENCE_NEWEST == laneAccept || CDRNSequenceWindow::SEQUENCE_LATE == laneAccept )
                        proceed = Deliver( message, sink );
                    else
                        ++m_stats.duplicateMessages;
                }
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

bool
CDRNLinkReliability::IsAllowedBeforeOperational( const CDRNWireReader& packetReader ) const
{GUCEF_TRACE;

    CDRNWireReader reader( packetReader );
    UInt32 expected = m_lane0Expected;
    while ( reader.GetRemaining() > 0 )
    {
        CDRNMessageView message;
        if ( !CDRNPacketCodec::ReadMessage( reader, message ) )
            return true;    // malformed: left to the normal processing

        bool inOrder = m_transportIsReliable || ( DRN_LANE_RELIABLE_ORDERED == message.lane && message.laneSequence == expected );
        if ( DRN_LANE_RELIABLE_ORDERED == message.lane && message.laneSequence == expected )
            ++expected;

        if ( DRN_MSGCLASS_LINK != message.msgClass || DRN_LINK_FRAGMENT == message.opcode )
            return false;
        if ( DRN_LANE_RELIABLE_UNORDERED == message.lane || DRN_LANE_UNRELIABLE_SEQUENCED == message.lane )
            return false;
        if ( DRN_LANE_RELIABLE_ORDERED == message.lane && !m_transportIsReliable &&
             DRNIsNewerSequence( message.laneSequence, m_lane0Expected ) && message.laneSequence - m_lane0Expected >= m_heldRingSize )
            return false;

        // the link is operational once it takes a WELCOME, what follows it is for the operational link
        if ( DRN_LINK_WELCOME == message.opcode && DRN_LANE_RELIABLE_ORDERED == message.lane && inOrder )
            return true;
    }
    return true;
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
