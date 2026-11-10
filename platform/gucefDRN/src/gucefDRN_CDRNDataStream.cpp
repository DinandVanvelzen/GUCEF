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

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

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
const CORE::CEvent CDRNDataStream::GapEvent = "GUCEF::DRN::CDRNDataStream::GapEvent";

/** Bit 0 of the flags that follow the priority in the announcement: a PUBLIC stream */
static const UInt8 StreamAnnounceFlagPublic = 0x01;

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::RegisterEvents( void )
{GUCEF_TRACE;

    EntriesAvailableEvent.Initialize();
    GapEvent.Initialize();
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
    , m_firstSeq( 0 != settings.firstSeq ? settings.firstSeq : 1 )
    , m_nextSeq( 0 != settings.firstSeq ? settings.firstSeq : 1 )
    , m_epoch( 0 )
    , m_subscribers()
    , m_writeStream()
    , m_entriesSent( 0 )
    , m_queue()
    , m_queueSeqs()
    , m_queueHead( 0 )
    , m_queueCount( 0 )
    , m_anyReceived( false )
    , m_nextExpected( 0 )
    , m_snapshotEndSeq( 0 )
    , m_snapshotKnown( false )
    , m_snapshotJustCompleted( false )
    , m_awaitingSnapshotBegin( false )
    , m_lastCreditSent( 0 )
    , m_creditSent( false )
    , m_arrived( false )
    , m_entriesReceived( 0 )
    , m_requestedFromSeq( 0 )
    , m_resumeFromSeq( 0 )
    , m_expectedEpoch( 0 )
    , m_optionsApplied( false )
    , m_gapCount( 0 )
    , m_missedEntryCount( 0 )
    , m_pendingGaps()
{GUCEF_TRACE;

    RegisterEvents();
    m_name = settings.name;
    if ( m_settings.backlogLimit < 1 )
        m_settings.backlogLimit = 1;
    if ( 0 == m_settings.firstSeq )
        m_settings.firstSeq = 1;
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

bool
CDRNDataStream::InitPublisher( void )
{GUCEF_TRACE;

    if ( 0 != m_settings.epoch )
    {
        m_epoch = m_settings.epoch;
        return true;
    }
    // a random epoch, so that a subscriber resuming across a restart of the publisher notices the new numbering
    UInt64 epoch = 0;
    while ( 0 == epoch )
    {
        if ( !CDRNCrypto::RandomBytes( &epoch, sizeof( epoch ) ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNDataStream(" << m_name << "): No randomness for the stream epoch" );
            return false;
        }
    }
    m_epoch = epoch;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::ApplySubscribeOptions( const CDRNSubscribeOptions& options )
{GUCEF_TRACE;

    if ( m_optionsApplied || m_isOrigin )
        return;
    m_optionsApplied = true;
    if ( 0 != options.fromSeq && !m_anyReceived )
    {
        m_resumeFromSeq = options.fromSeq;
        m_expectedEpoch = options.expectedEpoch;
    }
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

UInt64
CDRNDataStream::GetSendFrom( const CSubscriber& subscriber ) const
{GUCEF_TRACE;
    // a ring that restarted above what a subscriber was sent (the receive of a serving replica jumped) has
    // nothing below m_firstSeq: the subscriber continues there
    return subscriber.nextSendSeq < m_firstSeq ? m_firstSeq : subscriber.nextSendSeq;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::TrimRing( void )
{GUCEF_TRACE;
    // keep what a subscriber still has to get and the retained history
    UInt64 keepFrom = m_nextSeq - m_firstSeq > m_settings.retainedHistory ? m_nextSeq - m_settings.retainedHistory : m_firstSeq;
    for ( UInt32 i=0; i<m_subscribers.size(); ++i )
    {
        const CSubscriber& subscriber = *m_subscribers[ i ];
        UInt64 from = GetSendFrom( subscriber );
        if ( subscriber.started && from < keepFrom )
            keepFrom = from;
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
    CORE::CDynamicBuffer*& slot = m_ring[ (UInt32) ( m_nextSeq % ringSize ) ];
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
    UInt64 backlog = 0;
    for ( UInt32 i=0; i<m_subscribers.size(); ++i )
    {
        const CSubscriber& subscriber = *m_subscribers[ i ];
        UInt64 from = subscriber.started ? GetSendFrom( subscriber ) : m_nextSeq;
        if ( m_nextSeq - from > backlog )
            backlog = m_nextSeq - from;
    }
    return backlog > GUCEF_UINT32MAX ? GUCEF_UINT32MAX : (UInt32) backlog;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::CanAppend( void ) const
{GUCEF_TRACE;

    return GetBacklog() < m_settings.backlogLimit;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::AppendEntry( const void* data, UInt32 dataSize, UInt64& seq )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( !m_isOrigin || m_removed )
        return false;
    if ( !CanAppend() )
        return false;
    UInt64 entrySeq = m_nextSeq;
    if ( !PushToRing( data, dataSize ) )
        return false;
    TrimRing();
    seq = entrySeq;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::AppendEntry( const void* data, UInt32 dataSize )
{GUCEF_TRACE;

    UInt64 seq = 0;
    return AppendEntry( data, dataSize, seq );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::AppendEntry( const CORE::CVariantStream& segment, UInt64& seq )
{GUCEF_TRACE;

    return AppendEntry( segment.GetBuffer().GetConstBufferPtr(), segment.GetDataSize(), seq );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::AppendEntry( const CORE::CVariantStream& segment )
{GUCEF_TRACE;

    UInt64 seq = 0;
    return AppendEntry( segment.GetBuffer().GetConstBufferPtr(), segment.GetDataSize(), seq );
}

/*-------------------------------------------------------------------------*/

CORE::CVariantStream&
CDRNDataStream::GetWriteStream( void )
{GUCEF_TRACE;

    return m_writeStream;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::CommitEntry( UInt64& seq )
{GUCEF_TRACE;

    if ( !AppendEntry( m_writeStream, seq ) )
        return false;
    m_writeStream.Clear();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::CommitEntry( void )
{GUCEF_TRACE;

    UInt64 seq = 0;
    return CommitEntry( seq );
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNDataStream::GetFirstRetainedSeq( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    return m_isOrigin || m_ringStarted ? m_firstSeq : 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNDataStream::GetNextSeq( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    return m_isOrigin || m_ringStarted ? m_nextSeq : 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNDataStream::GetStreamEpoch( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    return m_epoch;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::PushToQueue( const UInt8* data, UInt32 dataSize, UInt64 seq )
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
        TSeqVector grownSeqs( newSize, 0 );
        UInt32 oldSize = (UInt32) m_queue.size();
        for ( UInt32 i=0; i<oldSize; ++i )
        {
            grown[ i ] = m_queue[ ( m_queueHead + i ) % oldSize ];
            grownSeqs[ i ] = m_queueSeqs[ ( m_queueHead + i ) % oldSize ];
        }
        m_queue.swap( grown );
        m_queueSeqs.swap( grownSeqs );
        m_queueHead = 0;
    }
    UInt32 index = ( m_queueHead + m_queueCount ) % (UInt32) m_queue.size();
    CORE::CDynamicBuffer*& slot = m_queue[ index ];
    if ( GUCEF_NULL == slot )
        slot = GUCEF_NEW CORE::CDynamicBuffer();
    slot->SetDataSize( 0 );
    slot->CopyFrom( dataSize, data );
    m_queueSeqs[ index ] = seq;
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
CDRNDataStream::ReadNextEntry( CORE::CVariantStream& segment, UInt64& seq )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( 0 == m_queueCount )
        return false;
    ToVariantStream( *m_queue[ m_queueHead ], segment );
    seq = m_queueSeqs[ m_queueHead ];
    m_queueHead = ( m_queueHead + 1 ) % (UInt32) m_queue.size();
    --m_queueCount;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::ReadNextEntry( CORE::CVariantStream& segment )
{GUCEF_TRACE;

    UInt64 seq = 0;
    return ReadNextEntry( segment, seq );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::ReadNextEntry( CORE::CDynamicBuffer& entry, UInt64& seq )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( 0 == m_queueCount )
        return false;
    const CORE::CDynamicBuffer& queued = *m_queue[ m_queueHead ];
    entry.SetDataSize( 0 );
    entry.CopyFrom( queued.GetDataSize(), queued.GetConstBufferPtr() );
    seq = m_queueSeqs[ m_queueHead ];
    m_queueHead = ( m_queueHead + 1 ) % (UInt32) m_queue.size();
    --m_queueCount;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::ReadNextEntry( CORE::CDynamicBuffer& entry )
{GUCEF_TRACE;

    UInt64 seq = 0;
    return ReadNextEntry( entry, seq );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::PeekEntry( UInt32 index, CORE::CVariantStream& segment, UInt64& seq ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( index >= m_queueCount )
        return false;
    UInt32 queueIndex = ( m_queueHead + index ) % (UInt32) m_queue.size();
    ToVariantStream( *m_queue[ queueIndex ], segment );
    seq = m_queueSeqs[ queueIndex ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::PeekEntry( UInt32 index, CORE::CVariantStream& segment ) const
{GUCEF_TRACE;

    UInt64 seq = 0;
    return PeekEntry( index, segment, seq );
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
CDRNDataStream::GetNextExpectedSeq( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    return m_anyReceived ? m_nextExpected : 0;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::SetResumeFromSeq( UInt64 seq )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( m_isOrigin || m_removed )
        return;
    m_optionsApplied = true;
    m_resumeFromSeq = seq;
    m_expectedEpoch = 0;
    if ( m_subscribeSent )
    {
        // what is queued belongs to the old position; the replica asks again from the new one
        m_queueHead = 0;
        m_queueCount = 0;
        m_pendingGaps.clear();
        m_anyReceived = false;
        GetReplicator().Resubscribe( *this );
    }
    else
    {
        m_anyReceived = false;
    }
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNDataStream::GetGapCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    return m_gapCount;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNDataStream::GetMissedEntryCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    return m_missedEntryCount;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::ReportGap( UInt64 fromSeq, UInt64 toSeq, TDRNStreamGapReason reason )
{GUCEF_TRACE;

    ++m_gapCount;
    if ( 0 != toSeq && toSeq >= fromSeq )
        m_missedEntryCount += toSeq - fromSeq + 1;
    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNDataStream(" << m_name << "): Entries " << fromSeq << " to " << toSeq << " will not arrive (" << CDRNStreamGapInfo::ReasonToString( reason ) << ")" );

    // the event is for the application (raised at the end of the tick while it subscribed); a replica that only
    // serves others counts
    m_pendingGaps.push_back( CDRNStreamGapInfo( fromSeq, toSeq, reason ) );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::IsOtherEpoch( UInt64 epoch ) const
{GUCEF_TRACE;

    return 0 != m_expectedEpoch && 0 != epoch && epoch != m_expectedEpoch;
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
    writer.WriteUInt8( DRN_PROTECTION_PUBLIC == m_protectionClass ? StreamAnnounceFlagPublic : 0 );
    writer.WriteVarUInt64( m_epoch );
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

    // trailing fields of later versions; a publisher that sends none has flags 0 and no epoch
    UInt8 flags = 0;
    UInt64 epoch = 0;
    if ( reader.ReadUInt8( flags ) && reader.ReadVarUInt64( epoch ) )
        m_epoch = epoch;
    SetProtectionClass( 0 != ( flags & StreamAnnounceFlagPublic ) ? DRN_PROTECTION_PUBLIC : DRN_PROTECTION_CONFIDENTIAL );
    m_settings.protection = m_protectionClass;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNDataStream::GetCreditSeq( void ) const
{GUCEF_TRACE;
    // the receiver holds the sender back by what the application has not read and what the subscribers here
    // have not been sent yet
    bool limited = false;
    UInt64 credit = 0;
    if ( m_appSubscribed )
    {
        credit = ( m_nextExpected - m_queueCount ) + m_settings.backlogLimit;
        limited = true;
    }
    for ( UInt32 i=0; i<m_subscribers.size(); ++i )
    {
        const CSubscriber& subscriber = *m_subscribers[ i ];
        UInt64 from = subscriber.started ? GetSendFrom( subscriber ) : m_nextExpected;
        UInt64 limit = from + m_settings.backlogLimit;
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

    UInt64 fromSeq = 0;
    if ( !m_anyReceived && 0 != m_resumeFromSeq )
    {
        // the application resumes: it knows the epoch from the announcement already
        if ( IsOtherEpoch( m_epoch ) )
        {
            ReportGap( m_resumeFromSeq, 0, DRN_STREAMGAP_EPOCH_CHANGED );
            m_resumeFromSeq = 0;
            m_expectedEpoch = 0;
        }
        else
        {
            fromSeq = m_resumeFromSeq;
        }
    }
    else
    if ( m_anyReceived )
    {
        fromSeq = m_nextExpected;
    }
    UInt64 window = m_anyReceived ? GetCreditSeq() - m_nextExpected : m_settings.backlogLimit;
    m_requestedFromSeq = fromSeq;
    writer.WriteVarUInt64( fromSeq );
    writer.WriteVarUInt64( window );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::IsExpectedPublicData( UInt8 lane, UInt8 msgClass, UInt32 opcode, const CDRNWireReader& /* body */ ) const
{GUCEF_TRACE;

    // snapshot boundaries and entries go on lane 0 only
    if ( DRN_LANE_RELIABLE_ORDERED != lane )
        return false;
    if ( DRN_MSGCLASS_REPL == msgClass )
        return DRN_REPL_SNAPSHOT_BEGIN == opcode || DRN_REPL_SNAPSHOT_END == opcode;
    return DRN_MSGCLASS_STREAM == msgClass && DRN_STREAM_ENTRIES == opcode;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataStream::OnResubscribe( void )
{GUCEF_TRACE;

    CDRNDataItem::OnResubscribe();
    m_snapshotKnown = false;
    m_snapshotJustCompleted = false;
    m_creditSent = false;
    m_awaitingSnapshotBegin = true;
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

    UInt64 fromSeq = 0;
    UInt64 window = m_settings.backlogLimit;
    params.ReadVarUInt64( fromSeq );
    params.ReadVarUInt64( window );

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

    UInt64 kept = m_nextSeq - m_firstSeq;
    UInt64 start = 0;
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
    writer.WriteVarUInt64( start );
    writer.WriteVarUInt64( m_epoch );
    SendMessage( subscriber.peerId, DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_REPL, DRN_REPL_SNAPSHOT_BEGIN );
    CDRNWireWriter& endWriter = BeginMessage();
    endWriter.WriteVarUInt64( m_nextSeq );
    SendMessage( subscriber.peerId, DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_REPL, DRN_REPL_SNAPSHOT_END );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStream::SendEntries( CSubscriber& subscriber )
{GUCEF_TRACE;
    if ( subscriber.nextSendSeq < m_firstSeq )
    {
        // the ring restarted above what this subscriber was sent (the receive of this serving replica jumped):
        // the ring slots below m_firstSeq hold no current entry. The subscriber continues at m_firstSeq and sees
        // the jump; its window is not used up by entries it never gets
        UInt64 skipped = m_firstSeq - subscriber.nextSendSeq;
        subscriber.nextSendSeq = m_firstSeq;
        subscriber.creditSeq += skipped;
    }

    UInt64 limit = subscriber.creditSeq < m_nextSeq ? subscriber.creditSeq : m_nextSeq;
    if ( subscriber.nextSendSeq >= limit )
        return true;

    UInt32 ringSize = GetRingSize();
    UInt32 messageBudget = GetReplicator().GetMessageBudget( subscriber.peerId );
    UInt32 budget = GetReplicator().GetLinkBudget( subscriber.peerId );
    while ( subscriber.nextSendSeq < limit && budget > 0 )
    {
        UInt64 seq = subscriber.nextSendSeq;
        CDRNWireWriter& writer = BeginMessage();
        writer.WriteVarUInt64( seq );
        UInt32 countOffset = writer.GetOffset();
        writer.WriteUInt16( 0 );
        UInt32 count = 0;
        while ( seq < limit && count < 0xFFFF )
        {
            const CORE::CDynamicBuffer& entry = *m_ring[ (UInt32) ( seq % ringSize ) ];
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
        {
            // a serving replica answers once its own snapshot arrived, so that the start it gives a subscriber
            // is that of its upstream's current state (and a gap there is reported as such)
            if ( !m_isOrigin && !m_snapshotComplete )
                continue;
            StartSubscriber( subscriber );
        }
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
    UInt64 seq = 0;
    UInt16 count = 0;
    if ( m_awaitingSnapshotBegin || !reader.ReadVarUInt64( seq ) || !reader.ReadUInt16( count ) )
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
                ReportGap( m_nextExpected, seq - 1, DRN_STREAMGAP_TRIMMED );
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
            PushToQueue( data, dataSize, seq );
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
CDRNDataStream::OnSnapshotBegin( CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt64 start = 0;
    if ( !reader.ReadVarUInt64( start ) )
        return;
    UInt64 epoch = 0;
    reader.ReadVarUInt64( epoch );      // absent from publishers that announce no epoch
    m_awaitingSnapshotBegin = false;

    if ( 0 != epoch )
    {
        if ( IsOtherEpoch( epoch ) )
            ReportGap( m_requestedFromSeq, 0, DRN_STREAMGAP_EPOCH_CHANGED );
        else
        if ( m_requestedFromSeq >= 1 && start > m_requestedFromSeq )
            ReportGap( m_requestedFromSeq, start - 1, DRN_STREAMGAP_BEYOND_RETENTION );
        else
        if ( m_anyReceived && start > m_nextExpected )
            ReportGap( m_nextExpected, start - 1, DRN_STREAMGAP_BEYOND_RETENTION );
        m_epoch = epoch;
    }
    else
    {
        if ( m_requestedFromSeq >= 1 && start > m_requestedFromSeq )
            ReportGap( m_requestedFromSeq, start - 1, DRN_STREAMGAP_BEYOND_RETENTION );
        else
        if ( m_anyReceived && start > m_nextExpected )
            ReportGap( m_nextExpected, start - 1, DRN_STREAMGAP_BEYOND_RETENTION );
    }
    m_requestedFromSeq = 0;
    m_resumeFromSeq = 0;
    m_expectedEpoch = 0;

    if ( !m_anyReceived || start > m_nextExpected )
    {
        m_nextExpected = start;
        m_firstSeq = m_nextSeq = start;
        m_anyReceived = true;
    }
    m_ringStarted = true;
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
            UInt64 creditSeq = 0;
            CSubscriber* subscriber = FindSubscriber( fromPeerId );
            if ( GUCEF_NULL != subscriber && subscriber->started && reader.ReadVarUInt64( creditSeq ) && creditSeq > subscriber->creditSeq )
                subscriber->creditSeq = creditSeq;
        }
        return;
    }

    if ( !fromUpstream )
        return;
    if ( DRN_REPL_SNAPSHOT_BEGIN == opcode )
    {
        OnSnapshotBegin( reader );
    }
    else
    if ( DRN_REPL_SNAPSHOT_END == opcode )
    {
        UInt64 seq = 0;
        if ( !reader.ReadVarUInt64( seq ) )
            return;
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
    if ( !m_pendingGaps.empty() )
    {
        // before the entries of the same tick: the event says a gap happened, the entry sequences say where
        for ( UInt32 i=0; i<m_pendingGaps.size() && m_appSubscribed; ++i )
        {
            CDRNStreamGapEventData eventData( m_pendingGaps[ i ] );
            Emit( GapEvent, &eventData );
        }
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNDataStream(" << m_name << "): " << m_pendingGaps.size() << " gaps at the end of the tick, application subscribed: " << m_appSubscribed );
        m_pendingGaps.clear();
    }
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
        UInt64 creditSeq = GetCreditSeq();
        if ( !m_creditSent || creditSeq > m_lastCreditSent )
        {
            m_creditSent = true;
            m_lastCreditSent = creditSeq;
            CDRNWireWriter& writer = BeginMessage();
            writer.WriteVarUInt64( creditSeq );
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
        for ( UInt64 seq=m_firstSeq; seq<m_nextSeq; ++seq )
        {
            const CORE::CDynamicBuffer* entry = m_ring[ (UInt32) ( seq % ringSize ) ];
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
            UInt32 queueIndex = ( m_queueHead + i ) % (UInt32) m_queue.size();
            const CORE::CDynamicBuffer* entry = m_queue[ queueIndex ];
            CORE::CVariant value;
            value.Set( entry->GetConstBufferPtr(), entry->GetDataSize(), GUCEF_DATATYPE_BINARY_BLOB );
            CORE::CDataNode* entryNode = entriesNode->AddChildWithValue( "entry", value );
            entryNode->SetAttribute( "seq", m_queueSeqs[ queueIndex ] );
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
