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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNREPLICATOR_H
#include "gucefDRN_CDRNReplicator.h"
#define GUCEF_DRN_CDRNREPLICATOR_H
#endif /* GUCEF_DRN_CDRNREPLICATOR_H ? */

#include "gucefDRN_CDRNDataStream.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CDRNDataStream::EntriesAvailableEvent = "GUCEF::DRN::CDRNDataStream::EntriesAvailableEvent";

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::RegisterEvents( void )
{GUCEF_TRACE;

    EntriesAvailableEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CDRNDataStream::CSubscriber::CSubscriber( void )
    : peerId( 0 )
    , started( false )
    , fromSeq( 0 )
    , nextSendSeq( 0 )
    , creditSeq( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNDataStream::CDRNDataStream( CDRNReplicator& replicator, const CDRNDataStreamSettings& settings )
    : CDRNDataItem( replicator, ITEMKIND_STREAM )
    , m_settings( settings )
    , m_ring()
    , m_ringStarted( false )
    , m_firstSeq( 1 )
    , m_nextSeq( 1 )
    , m_subscribers()
    , m_writeStream()
    , m_entriesSent( 0 )
    , m_queue()
    , m_queueHead( 0 )
    , m_queueCount( 0 )
    , m_anyReceived( false )
    , m_nextExpected( 0 )
    , m_snapshotEndSeq( 0 )
    , m_snapshotKnown( false )
    , m_snapshotJustCompleted( false )
    , m_lastCreditSent( 0 )
    , m_creditSent( false )
    , m_arrived( false )
    , m_entriesReceived( 0 )
{GUCEF_TRACE;

    RegisterEvents();
    m_name = settings.name;
    if ( m_settings.backlogLimit < 1 )
        m_settings.backlogLimit = 1;
}

/*-------------------------------------------------------------------------*/

CDRNDataStream::~CDRNDataStream()
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_subscribers.size(); ++i )
        GUCEF_DELETE m_subscribers[ i ];
    m_subscribers.clear();
    for ( UInt32 i=0; i<m_ring.size(); ++i )
        GUCEF_DELETE m_ring[ i ];
    m_ring.clear();
    for ( UInt32 i=0; i<m_queue.size(); ++i )
        GUCEF_DELETE m_queue[ i ];
    m_queue.clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNDataStream::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNDataStream";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const CDRNDataStreamSettings&
CDRNDataStream::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::SetNotifyMode( TDRNNotifyMode mode )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    m_settings.notify = mode;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDataStream::GetRingSize( void ) const
{GUCEF_TRACE;
    return m_settings.backlogLimit + m_settings.retainedHistory + 1;
}

/*-------------------------------------------------------------------------*/

CDRNDataStream::CSubscriber*
CDRNDataStream::FindSubscriber( UInt32 peerId ) const
{GUCEF_TRACE;
    for ( UInt32 i=0; i<m_subscribers.size(); ++i )
        if ( m_subscribers[ i ]->peerId == peerId )
            return m_subscribers[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::TrimRing( void )
{GUCEF_TRACE;
    // keep what a subscriber still has to get and the retained history
    UInt32 keepFrom = m_nextSeq - m_firstSeq > m_settings.retainedHistory ? m_nextSeq - m_settings.retainedHistory : m_firstSeq;
    for ( UInt32 i=0; i<m_subscribers.size(); ++i )
    {
        const CSubscriber& subscriber = *m_subscribers[ i ];
        if ( subscriber.started && subscriber.nextSendSeq < keepFrom )
            keepFrom = subscriber.nextSendSeq;
    }
    if ( keepFrom > m_firstSeq )
        m_firstSeq = keepFrom;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::PushToRing( const void* data, UInt32 dataSize )
{GUCEF_TRACE;
    UInt32 ringSize = GetRingSize();
    if ( m_ring.size() < ringSize )
        m_ring.resize( ringSize, GUCEF_NULL );

    if ( m_nextSeq - m_firstSeq >= ringSize )
    {
        TrimRing();
        if ( m_nextSeq - m_firstSeq >= ringSize )
            return false;
    }
    CORE::CDynamicBuffer*& slot = m_ring[ m_nextSeq % ringSize ];
    if ( GUCEF_NULL == slot )
        slot = GUCEF_NEW CORE::CDynamicBuffer();
    slot->SetDataSize( 0 );
    slot->CopyFrom( dataSize, data );
    ++m_nextSeq;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDataStream::GetBacklog( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    UInt32 backlog = 0;
    for ( UInt32 i=0; i<m_subscribers.size(); ++i )
    {
        const CSubscriber& subscriber = *m_subscribers[ i ];
        UInt32 from = subscriber.started ? subscriber.nextSendSeq : m_nextSeq;
        if ( m_nextSeq - from > backlog )
            backlog = m_nextSeq - from;
    }
    return backlog;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::CanAppend( void ) const
{GUCEF_TRACE;

    return GetBacklog() < m_settings.backlogLimit;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::AppendEntry( const void* data, UInt32 dataSize )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( !m_isOrigin || m_removed )
        return false;
    if ( !CanAppend() )
        return false;
    if ( !PushToRing( data, dataSize ) )
        return false;
    TrimRing();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::AppendEntry( const CORE::CVariantStream& segment )
{GUCEF_TRACE;

    return AppendEntry( segment.GetBuffer().GetConstBufferPtr(), segment.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

CORE::CVariantStream&
CDRNDataStream::GetWriteStream( void )
{GUCEF_TRACE;

    return m_writeStream;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::CommitEntry( void )
{GUCEF_TRACE;

    if ( !AppendEntry( m_writeStream ) )
        return false;
    m_writeStream.Clear();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::PushToQueue( const UInt8* data, UInt32 dataSize )
{GUCEF_TRACE;
    if ( m_queueCount >= m_queue.size() )
    {
        // grows once to the backlog limit, more only when credit was granted elsewhere
        UInt32 newSize = (UInt32) m_queue.size() * 2;
        if ( newSize < m_settings.backlogLimit )
            newSize = m_settings.backlogLimit;
        if ( newSize < 4 )
            newSize = 4;
        TBufferVector grown( newSize, GUCEF_NULL );
        UInt32 oldSize = (UInt32) m_queue.size();
        for ( UInt32 i=0; i<oldSize; ++i )
            grown[ i ] = m_queue[ ( m_queueHead + i ) % oldSize ];
        m_queue.swap( grown );
        m_queueHead = 0;
    }
    CORE::CDynamicBuffer*& slot = m_queue[ ( m_queueHead + m_queueCount ) % m_queue.size() ];
    if ( GUCEF_NULL == slot )
        slot = GUCEF_NEW CORE::CDynamicBuffer();
    slot->SetDataSize( 0 );
    slot->CopyFrom( dataSize, data );
    ++m_queueCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDataStream::GetEntryCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    return m_queueCount;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::ToVariantStream( const CORE::CDynamicBuffer& entry, CORE::CVariantStream& segment )
{GUCEF_TRACE;
    segment.Clear();
    segment.GetBuffer().CopyFrom( entry.GetDataSize(), entry.GetConstBufferPtr() );
    segment.ResetReadPosition();
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::ReadNextEntry( CORE::CVariantStream& segment )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( 0 == m_queueCount )
        return false;
    ToVariantStream( *m_queue[ m_queueHead ], segment );
    m_queueHead = ( m_queueHead + 1 ) % (UInt32) m_queue.size();
    --m_queueCount;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::ReadNextEntry( CORE::CDynamicBuffer& entry )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( 0 == m_queueCount )
        return false;
    const CORE::CDynamicBuffer& queued = *m_queue[ m_queueHead ];
    entry.SetDataSize( 0 );
    entry.CopyFrom( queued.GetDataSize(), queued.GetConstBufferPtr() );
    m_queueHead = ( m_queueHead + 1 ) % (UInt32) m_queue.size();
    --m_queueCount;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::PeekEntry( UInt32 index, CORE::CVariantStream& segment ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( index >= m_queueCount )
        return false;
    ToVariantStream( *m_queue[ ( m_queueHead + index ) % m_queue.size() ], segment );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDataStream::DiscardEntries( UInt32 count )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( count > m_queueCount )
        count = m_queueCount;
    if ( count > 0 )
    {
        m_queueHead = ( m_queueHead + count ) % (UInt32) m_queue.size();
        m_queueCount -= count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNDataStream::GetEntriesSent( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    return m_entriesSent;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNDataStream::GetEntriesReceived( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    return m_entriesReceived;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::WriteAnnounceSettings( CDRNWireWriter& writer ) const
{GUCEF_TRACE;

    writer.WriteVarUInt32( m_settings.backlogLimit );
    writer.WriteVarUInt32( m_settings.retainedHistory );
    writer.WriteFloat32( m_settings.priority );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::ReadAnnounceSettings( CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 backlogLimit = 0;
    UInt32 retainedHistory = 0;
    Float32 priority = 1;
    if ( !reader.ReadVarUInt32( backlogLimit ) || !reader.ReadVarUInt32( retainedHistory ) || !reader.ReadFloat32( priority ) )
        return;
    m_settings.name = m_name;
    m_settings.backlogLimit = backlogLimit < 1 ? 1 : ( backlogLimit > 65536 ? 65536 : backlogLimit );
    m_settings.retainedHistory = retainedHistory > 65536 ? 65536 : retainedHistory;
    m_settings.priority = priority;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDataStream::GetCreditSeq( void ) const
{GUCEF_TRACE;
    // the receiver holds the sender back by what the application has not read and what the subscribers here
    // have not been sent yet
    bool limited = false;
    UInt32 credit = 0;
    if ( m_appSubscribed )
    {
        credit = ( m_nextExpected - m_queueCount ) + m_settings.backlogLimit;
        limited = true;
    }
    for ( UInt32 i=0; i<m_subscribers.size(); ++i )
    {
        const CSubscriber& subscriber = *m_subscribers[ i ];
        UInt32 from = subscriber.started ? subscriber.nextSendSeq : m_nextExpected;
        UInt32 limit = from + m_settings.backlogLimit;
        if ( !limited || limit < credit )
            credit = limit;
        limited = true;
    }
    if ( !limited )
        credit = m_nextExpected + m_settings.backlogLimit;
    return credit;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::WriteSubscribeParams( CDRNWireWriter& writer )
{GUCEF_TRACE;

    UInt32 fromSeq = m_anyReceived ? m_nextExpected : 0;
    UInt32 window = m_anyReceived ? GetCreditSeq() - m_nextExpected : m_settings.backlogLimit;
    writer.WriteVarUInt32( fromSeq );
    writer.WriteVarUInt32( window );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::OnResubscribe( void )
{GUCEF_TRACE;

    CDRNDataItem::OnResubscribe();
    m_snapshotKnown = false;
    m_snapshotJustCompleted = false;
    m_creditSent = false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::IsDue( UInt64 /* nodeNow */ ) const
{GUCEF_TRACE;
    if ( !m_ringStarted && !m_isOrigin )
        return false;
    for ( UInt32 i=0; i<m_subscribers.size(); ++i )
    {
        const CSubscriber& subscriber = *m_subscribers[ i ];
        if ( !subscriber.started )
            return true;
        if ( subscriber.nextSendSeq < m_nextSeq && subscriber.nextSendSeq < subscriber.creditSeq )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

Float32
CDRNDataStream::GetPriority( void ) const
{GUCEF_TRACE;
    return m_settings.priority;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::OnSubscriberAdded( UInt32 peerId, UInt32 /* flags */, CDRNWireReader& params )
{GUCEF_TRACE;

    UInt32 fromSeq = 0;
    UInt32 window = m_settings.backlogLimit;
    params.ReadVarUInt32( fromSeq );
    params.ReadVarUInt32( window );

    CSubscriber* subscriber = FindSubscriber( peerId );
    if ( GUCEF_NULL == subscriber )
    {
        subscriber = GUCEF_NEW CSubscriber();
        subscriber->peerId = peerId;
        m_subscribers.push_back( subscriber );
    }
    subscriber->started = false;
    subscriber->fromSeq = fromSeq;
    subscriber->creditSeq = window;     // a window until the subscriber starts
    m_subscriberCount = (UInt32) m_subscribers.size();
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::OnSubscriberRemoved( UInt32 peerId )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_subscribers.size(); ++i )
    {
        if ( m_subscribers[ i ]->peerId == peerId )
        {
            GUCEF_DELETE m_subscribers[ i ];
            m_subscribers.erase( m_subscribers.begin() + i );
            break;
        }
    }
    m_subscriberCount = (UInt32) m_subscribers.size();
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::StartSubscriber( CSubscriber& subscriber )
{GUCEF_TRACE;

    UInt32 kept = m_nextSeq - m_firstSeq;
    UInt32 start = 0;
    if ( 0 == subscriber.fromSeq )
        start = m_nextSeq - ( kept < m_settings.retainedHistory ? kept : m_settings.retainedHistory );
    else
    if ( subscriber.fromSeq < m_firstSeq )
        start = m_firstSeq;
    else
    if ( subscriber.fromSeq > m_nextSeq )
        start = m_nextSeq;
    else
        start = subscriber.fromSeq;

    subscriber.nextSendSeq = start;
    subscriber.creditSeq = start + subscriber.creditSeq;
    subscriber.started = true;

    CDRNWireWriter& writer = BeginMessage();
    writer.WriteVarUInt32( start );
    SendMessage( subscriber.peerId, DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_REPL, DRN_REPL_SNAPSHOT_BEGIN );
    CDRNWireWriter& endWriter = BeginMessage();
    endWriter.WriteVarUInt32( m_nextSeq );
    SendMessage( subscriber.peerId, DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_REPL, DRN_REPL_SNAPSHOT_END );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::SendEntries( CSubscriber& subscriber )
{GUCEF_TRACE;
    UInt32 limit = subscriber.creditSeq < m_nextSeq ? subscriber.creditSeq : m_nextSeq;
    if ( subscriber.nextSendSeq >= limit )
        return true;

    UInt32 ringSize = GetRingSize();
    UInt32 messageBudget = GetReplicator().GetMessageBudget( subscriber.peerId );
    UInt32 budget = GetReplicator().GetLinkBudget( subscriber.peerId );
    while ( subscriber.nextSendSeq < limit && budget > 0 )
    {
        UInt32 seq = subscriber.nextSendSeq;
        CDRNWireWriter& writer = BeginMessage();
        writer.WriteVarUInt32( seq );
        UInt32 countOffset = writer.GetOffset();
        writer.WriteUInt16( 0 );
        UInt32 count = 0;
        while ( seq < limit && count < 0xFFFF )
        {
            const CORE::CDynamicBuffer& entry = *m_ring[ seq % ringSize ];
            UInt32 before = writer.GetOffset();
            writer.WriteSizedBytes( entry.GetConstBufferPtr(), entry.GetDataSize() );
            if ( count > 0 && ( writer.GetOffset() > messageBudget || writer.GetOffset() > budget ) )
            {
                writer.SetOffset( before );
                break;
            }
            ++count;
            ++seq;
        }
        UInt8* countPtr = static_cast< UInt8* >( m_txBuffer.GetBufferPtr() ) + countOffset;
        countPtr[ 0 ] = (UInt8) ( count >> 8 );
        countPtr[ 1 ] = (UInt8) count;

        UInt32 bytes = writer.GetOffset();
        SendMessage( subscriber.peerId, DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_STREAM, DRN_STREAM_ENTRIES );
        GetReplicator().ConsumeLinkBudget( subscriber.peerId, bytes );
        budget = bytes >= budget ? 0 : budget - bytes;
        m_entriesSent += count;
        subscriber.nextSendSeq = seq;
    }
    return subscriber.nextSendSeq >= limit;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::OnSendTick( UInt64 /* nodeNow */ )
{GUCEF_TRACE;
    if ( !m_ringStarted && !m_isOrigin )
        return true;
    if ( m_isOrigin )
        m_ringStarted = true;

    bool allSent = true;
    for ( UInt32 i=0; i<m_subscribers.size(); ++i )
    {
        CSubscriber& subscriber = *m_subscribers[ i ];
        if ( !subscriber.started )
            StartSubscriber( subscriber );
        if ( !SendEntries( subscriber ) )
            allSent = false;
    }
    TrimRing();
    return allSent;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::OnEntries( CDRNWireReader& reader )
{GUCEF_TRACE;
    UInt32 seq = 0;
    UInt16 count = 0;
    if ( !reader.ReadVarUInt32( seq ) || !reader.ReadUInt16( count ) )
        return;

    for ( UInt32 n=0; n<count; ++n, ++seq )
    {
        const UInt8* data = GUCEF_NULL;
        UInt32 dataSize = 0;
        if ( !reader.ReadSizedBytes( data, dataSize ) )
            return;
        if ( m_anyReceived && seq < m_nextExpected )
            continue;   // had it
        if ( !m_anyReceived || seq > m_nextExpected )
        {
            if ( m_anyReceived )
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNDataStream(" << m_name << "): Entries " << m_nextExpected << " to " << seq - 1 << " are no longer kept by the publisher" );
            m_nextExpected = seq;
            m_firstSeq = m_nextSeq = seq;
            m_anyReceived = true;
        }
        if ( !m_ringStarted )
        {
            m_firstSeq = m_nextSeq = seq;
            m_ringStarted = true;
        }

        if ( !PushToRing( data, dataSize ) )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNDataStream(" << m_name << "): No room for an entry for the subscribers" );
        if ( m_appSubscribed )
            PushToQueue( data, dataSize );
        ++m_nextExpected;
        ++m_entriesReceived;
        m_arrived = true;
    }

    if ( m_snapshotKnown && !m_snapshotComplete && m_nextExpected >= m_snapshotEndSeq )
    {
        m_snapshotComplete = true;
        m_snapshotJustCompleted = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::OnItemMessage( UInt32 fromPeerId, UInt8 msgClass, UInt32 opcode, CDRNWireReader& reader )
{GUCEF_TRACE;
    bool fromUpstream = !m_isOrigin && fromPeerId == m_upstreamPeerId && m_subscribeSent;
    if ( DRN_MSGCLASS_STREAM == msgClass )
    {
        if ( DRN_STREAM_ENTRIES == opcode )
        {
            if ( fromUpstream )
                OnEntries( reader );
        }
        else
        if ( DRN_STREAM_CREDIT == opcode )
        {
            UInt32 creditSeq = 0;
            CSubscriber* subscriber = FindSubscriber( fromPeerId );
            if ( GUCEF_NULL != subscriber && subscriber->started && reader.ReadVarUInt32( creditSeq ) && DRNIsNewerSequence( creditSeq, subscriber->creditSeq ) )
                subscriber->creditSeq = creditSeq;
        }
        return;
    }

    if ( !fromUpstream )
        return;
    UInt32 seq = 0;
    if ( !reader.ReadVarUInt32( seq ) )
        return;
    if ( DRN_REPL_SNAPSHOT_BEGIN == opcode )
    {
        if ( !m_anyReceived || seq > m_nextExpected )
        {
            m_nextExpected = seq;
            m_firstSeq = m_nextSeq = seq;
            m_anyReceived = true;
        }
        m_ringStarted = true;
    }
    else
    if ( DRN_REPL_SNAPSHOT_END == opcode )
    {
        m_snapshotEndSeq = seq;
        m_snapshotKnown = true;
        if ( !m_snapshotComplete && m_nextExpected >= m_snapshotEndSeq )
        {
            m_snapshotComplete = true;
            m_snapshotJustCompleted = true;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::OnEndOfTick( UInt64 /* nodeNow */ )
{GUCEF_TRACE;
    if ( m_arrived )
    {
        m_arrived = false;
        if ( DRN_NOTIFY_POLL != m_settings.notify && m_appSubscribed && m_queueCount > 0 )
        {
            CORE::TCloneableUInt32 eventData( m_queueCount );
            Emit( EntriesAvailableEvent, &eventData );
        }
    }
    if ( m_snapshotJustCompleted )
    {
        m_snapshotJustCompleted = false;
        Emit( SnapshotCompleteEvent );
    }

    if ( !m_isOrigin && m_subscribeSent && m_anyReceived )
    {
        UInt32 creditSeq = GetCreditSeq();
        if ( !m_creditSent || DRNIsNewerSequence( creditSeq, m_lastCreditSent ) )
        {
            m_creditSent = true;
            m_lastCreditSent = creditSeq;
            CDRNWireWriter& writer = BeginMessage();
            writer.WriteVarUInt32( creditSeq );
            SendMessage( m_upstreamPeerId, DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_STREAM, DRN_STREAM_CREDIT );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& /* settings */ ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );

    domRootNode.SetName( "DRNDataStream" );
    domRootNode.SetAttribute( "name", m_name );
    domRootNode.SetAttribute( "ownerPeerId", m_ownerPeerId );
    domRootNode.SetAttribute( "itemId", m_itemId );
    domRootNode.SetAttribute( "isOrigin", m_isOrigin );

    CORE::CDataNode* settingsNode = domRootNode.AddChild( "settings" );
    m_settings.SaveConfig( *settingsNode );

    CORE::CDataNode* entriesNode = domRootNode.AddChild( "entries" );
    if ( m_isOrigin )
    {
        UInt32 ringSize = GetRingSize();
        for ( UInt32 seq=m_firstSeq; seq<m_nextSeq; ++seq )
        {
            const CORE::CDynamicBuffer* entry = m_ring[ seq % ringSize ];
            CORE::CVariant value;
            value.Set( entry->GetConstBufferPtr(), entry->GetDataSize(), GUCEF_DATATYPE_BINARY_BLOB );
            CORE::CDataNode* entryNode = entriesNode->AddChildWithValue( "entry", value );
            entryNode->SetAttribute( "seq", seq );
        }
    }
    else
    {
        for ( UInt32 i=0; i<m_queueCount; ++i )
        {
            const CORE::CDynamicBuffer* entry = m_queue[ ( m_queueHead + i ) % m_queue.size() ];
            CORE::CVariant value;
            value.Set( entry->GetConstBufferPtr(), entry->GetDataSize(), GUCEF_DATATYPE_BINARY_BLOB );
            entriesNode->AddChildWithValue( "entry", value );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& /* settings */ )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( !m_isOrigin )
        return false;
    const CORE::CDataNode* entriesNode = domRootNode.FindChild( "entries" );
    if ( GUCEF_NULL == entriesNode )
        return false;
    bool allAppended = true;
    CORE::CDataNode::const_iterator i = entriesNode->ConstBegin();
    while ( i != entriesNode->ConstEnd() )
    {
        const CORE::CVariant& value = (*i)->GetValue();
        ++i;
        if ( !AppendEntry( value.AsVoidPtr(), value.ByteSize( false ) ) )
            allAppended = false;
    }
    return allAppended;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
