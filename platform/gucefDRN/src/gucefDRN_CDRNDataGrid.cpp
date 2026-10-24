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

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#ifndef GUCEF_DRN_CIDRNACCESSPOLICY_H
#include "gucefDRN_CIDRNAccessPolicy.h"
#define GUCEF_DRN_CIDRNACCESSPOLICY_H
#endif /* GUCEF_DRN_CIDRNACCESSPOLICY_H ? */

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

#include "gucefDRN_CDRNDataGrid.h"

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

/** Flags of GRID_DELTA */
static const UInt8 DeltaFlagReliable    = 0x01;    /**< on lane 0, carries the sequence of the previous reliable delta */
static const UInt8 DeltaFlagSnapshot    = 0x02;    /**< part of a snapshot */
static const UInt8 DeltaFlagRevisions   = 0x04;    /**< every entry carries its revision and writer */
static const UInt8 DeltaFlagNoAck       = 0x08;    /**< the publisher does not track acknowledgements */

/** Flags of a KEY_DEFINE entry */
static const UInt8 KeyFlagReliable      = 0x01;

/** Flags of the grid settings in ITEM_ANNOUNCE */
static const UInt8 AnnounceFlagReliable         = 0x01;
static const UInt8 AnnounceFlagDeltaAgainstAck  = 0x02;
static const UInt8 AnnounceFlagSnapshot         = 0x04;

/** Deltas a replica keeps while its snapshot is on the way */
static const UInt32 MaxBufferedDeltas   = 64;

/** Shortest time after which an unacknowledged delta counts as lost */
static const UInt64 MinResendTimeout    = 100000;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CDRNDataGrid::GridKeyChangedEvent = "GUCEF::DRN::CDRNDataGrid::GridKeyChangedEvent";
const CORE::CEvent CDRNDataGrid::GridUpdatedEvent = "GUCEF::DRN::CDRNDataGrid::GridUpdatedEvent";
const CORE::CEvent CDRNDataGrid::ProposeResultEvent = "GUCEF::DRN::CDRNDataGrid::ProposeResultEvent";
const CORE::CEvent CDRNDataGrid::VoteStartedEvent = "GUCEF::DRN::CDRNDataGrid::VoteStartedEvent";
const CORE::CEvent CDRNDataGrid::VoteEndedEvent = "GUCEF::DRN::CDRNDataGrid::VoteEndedEvent";

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::RegisterEvents( void )
{GUCEF_TRACE;

    GridKeyChangedEvent.Initialize();
    GridUpdatedEvent.Initialize();
    ProposeResultEvent.Initialize();
    VoteStartedEvent.Initialize();
    VoteEndedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CDRNProposeResult::CDRNProposeResult( void )
    : proposalId( 0 )
    , accepted( false )
    , voteOutcome( CDRNVotePolicy::VOTEOUTCOME_PENDING )
    , reason()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNVoteInfo::CDRNVoteInfo( void )
    : voteId( 0 )
    , proposerPeerId( 0 )
    , key()
    , value()
    , isRemoval( false )
    , timeoutMs( 0 )
    , outcome( CDRNVotePolicy::VOTEOUTCOME_PENDING )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNDataGrid::CKeySlot::CKeySlot( void )
    : key()
    , value()
    , frontValue()
    , quantisation()
    , revision( 0 )
    , changeTick( 0 )
    , lastWriter( 0 )
    , changeSendTime( 0 )
    , rxSeq( 0 )
    , frontRevision( 0 )
    , frontWriter( 0 )
    , frontSendTime( 0 )
    , updateTick( 0 )
    , used( false )
    , removed( false )
    , frontRemoved( false )
    , reliableKey( false )
    , rxSeqValid( false )
    , frontPending( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNDataGrid::CKeyState::CKeyState( void )
    : sentChange( 0 )
    , sentSeq( 0 )
    , ackedChange( 0 )
    , defined( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNDataGrid::CSubscriber::CSubscriber( void )
    : peerId( 0 )
    , needsSnapshot( true )
    , nextSeq( 1 )
    , lastReliableSeq( 0 )
    , hasUnacked( false )
    , unackedSince( 0 )
    , keys()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNDataGrid::CVote::CVote( void )
    : voteId( 0 )
    , proposerPeerId( 0 )
    , proposalId( 0 )
    , key()
    , value()
    , isRemoval( false )
    , deadline( 0 )
    , voters()
    , votes()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNDataGrid::CDRNDataGrid( CDRNReplicator& replicator, const CDRNDataGridSettings& settings )
    : CDRNDataItem( replicator, ITEMKIND_GRID )
    , m_settings( settings )
    , m_validator( GUCEF_NULL )
    , m_defaultValidator()
    , m_slots()
    , m_keyIndex()
    , m_keyCount( 0 )
    , m_subscribers()
    , m_writeTick( 1 )
    , m_writeTickUsed( false )
    , m_nextSendTime( 0 )
    , m_sendPending( false )
    , m_unreliableIndices()
    , m_reliableIndices()
    , m_defineIndices()
    , m_bytesSent( 0 )
    , m_deltasSent( 0 )
    , m_txValue()
    , m_ackWindow()
    , m_ackDue( false )
    , m_startSeq( 1 )
    , m_highestSeqSeen( 0 )
    , m_anySeqSeen( false )
    , m_lastReliableSeq( 0 )
    , m_inSnapshot( false )
    , m_snapshotJustCompleted( false )
    , m_snapshotSeq( 0 )
    , m_bufferedDeltas()
    , m_bufferedCount( 0 )
    , m_deltasReceived( 0 )
    , m_resyncCount( 0 )
    , m_frontPending()
    , m_updateTick( 0 )
    , m_updatedEventData()
    , m_rxScratch()
    , m_rxKey()
    , m_nextProposalId( 0 )
    , m_nextVoteId( 0 )
    , m_votes()
{GUCEF_TRACE;

    RegisterEvents();
    m_name = settings.name;
    EnsureSlotCount( settings.capacity > 0 ? settings.capacity : 1 );
}

/*-------------------------------------------------------------------------*/

CDRNDataGrid::~CDRNDataGrid()
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_subscribers.size(); ++i )
        GUCEF_DELETE m_subscribers[ i ];
    m_subscribers.clear();
    for ( UInt32 i=0; i<m_bufferedDeltas.size(); ++i )
        GUCEF_DELETE m_bufferedDeltas[ i ];
    m_bufferedDeltas.clear();
    for ( UInt32 i=0; i<m_votes.size(); ++i )
        GUCEF_DELETE m_votes[ i ];
    m_votes.clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNDataGrid::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNDataGrid";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const CDRNDataGridSettings&
CDRNDataGrid::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::SetNotifyMode( TDRNNotifyMode mode )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    m_settings.notify = mode;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::SetWriteValidator( CIDRNWriteValidator* validator )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    m_validator = validator;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::EnsureSlotCount( UInt32 count )
{GUCEF_TRACE;

    if ( count <= m_slots.size() )
        return;

    // grows outside the hot path: at least doubles
    UInt32 newSize = (UInt32) m_slots.size() * 2;
    if ( newSize < count )
        newSize = count;
    if ( newSize < m_settings.capacity )
        newSize = m_settings.capacity;
    m_slots.resize( newSize );
    for ( UInt32 i=0; i<m_subscribers.size(); ++i )
        m_subscribers[ i ]->keys.resize( newSize );
    m_frontPending.reserve( newSize );
    m_updatedEventData.GetData().reserve( newSize );
    m_unreliableIndices.reserve( newSize );
    m_reliableIndices.reserve( newSize );
    m_defineIndices.reserve( newSize );
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDataGrid::AddKey( const CORE::CVariant& key, UInt32 index, bool reliableKey, const CDRNQuantisation& quantisation )
{GUCEF_TRACE;

    if ( InvalidKeyIndex == index )
    {
        TKeyIndexMap::const_iterator i = m_keyIndex.find( key );
        if ( i != m_keyIndex.end() )
            return (*i).second;
        index = m_keyCount;
    }

    EnsureSlotCount( index + 1 );
    CKeySlot& slot = m_slots[ index ];
    if ( slot.used && !CDRNVariantCodec::IsSame( slot.key, key ) )
        m_keyIndex.erase( slot.key );
    slot.key = key;
    slot.used = true;
    slot.reliableKey = reliableKey;
    slot.quantisation = quantisation;
    m_keyIndex[ key ] = index;
    if ( index >= m_keyCount )
        m_keyCount = index + 1;
    return index;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::StampChange( CKeySlot& slot )
{GUCEF_TRACE;
    slot.changeTick = m_writeTick;
    m_writeTickUsed = true;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::MarkFrontPending( UInt32 index )
{GUCEF_TRACE;
    CKeySlot& slot = m_slots[ index ];
    if ( !slot.frontPending )
    {
        slot.frontPending = true;
        m_frontPending.push_back( index );
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGrid::ApplyWrite( UInt32 index, const CORE::CVariant& value, bool remove, UInt32 writerPeerId )
{GUCEF_TRACE;
    CKeySlot& slot = m_slots[ index ];
    if ( remove )
    {
        if ( slot.removed || 0 == slot.changeTick )
        {
            slot.removed = true;
            return true;
        }
        slot.removed = true;
        slot.value.Clear();
    }
    else
    {
        if ( slot.quantisation.IsFull() )
        {
            if ( !slot.removed && 0 != slot.changeTick && CDRNVariantCodec::IsSame( slot.value, value ) )
                return true;
            slot.value = value;
        }
        else
        {
            m_txValue = value;
            CDRNVariantCodec::Quantise( m_txValue, slot.quantisation );
            if ( !slot.removed && 0 != slot.changeTick && CDRNVariantCodec::IsSame( slot.value, m_txValue ) )
                return true;
            slot.value = m_txValue;
        }
        slot.removed = false;
    }

    ++slot.revision;
    slot.lastWriter = writerPeerId;
    slot.changeSendTime = GetReplicator().GetTickSessionTime();
    StampChange( slot );
    MarkFrontPending( index );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGrid::SetValue( const CORE::CVariant& key, const CORE::CVariant& value )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( m_removed )
        return false;
    if ( !m_isOrigin )
        return Propose( key, value, false );

    UInt32 index = AddKey( key, InvalidKeyIndex, m_settings.IsReliableKey( key ), m_settings.GetQuantisation( key ) );
    return ApplyWrite( index, value, false, GetReplicator().GetLocalPeerId() );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGrid::SetValueByIndex( UInt32 index, const CORE::CVariant& value )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( index >= m_keyCount || !m_slots[ index ].used || m_removed )
        return false;
    if ( !m_isOrigin )
        return Propose( m_slots[ index ].key, value, false );
    return ApplyWrite( index, value, false, GetReplicator().GetLocalPeerId() );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGrid::RemoveKey( const CORE::CVariant& key )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( m_removed )
        return false;
    if ( !m_isOrigin )
        return Propose( key, CORE::CVariant::Empty, true );

    TKeyIndexMap::const_iterator i = m_keyIndex.find( key );
    if ( i == m_keyIndex.end() )
        return false;
    return ApplyWrite( (*i).second, CORE::CVariant::Empty, true, GetReplicator().GetLocalPeerId() );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGrid::GetValue( const CORE::CVariant& key, CORE::CVariant& value ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    TKeyIndexMap::const_iterator i = m_keyIndex.find( key );
    if ( i == m_keyIndex.end() )
        return false;
    return GetValueByIndex( (*i).second, value );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGrid::GetValueByIndex( UInt32 index, CORE::CVariant& value ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( index >= m_keyCount )
        return false;
    const CKeySlot& slot = m_slots[ index ];
    if ( !slot.used )
        return false;
    if ( m_isOrigin )
    {
        if ( slot.removed || 0 == slot.changeTick )
            return false;
        value = slot.value;
        return true;
    }
    if ( slot.frontRemoved || 0 == slot.updateTick )
        return false;
    value = slot.frontValue;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGrid::HasKey( const CORE::CVariant& key ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    TKeyIndexMap::const_iterator i = m_keyIndex.find( key );
    if ( i == m_keyIndex.end() )
        return false;
    const CKeySlot& slot = m_slots[ (*i).second ];
    if ( m_isOrigin )
        return !slot.removed && 0 != slot.changeTick;
    return !slot.frontRemoved && 0 != slot.updateTick;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDataGrid::GetKeyIndex( const CORE::CVariant& key ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    TKeyIndexMap::const_iterator i = m_keyIndex.find( key );
    if ( i == m_keyIndex.end() )
        return InvalidKeyIndex;
    return (*i).second;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDataGrid::GetOrAddKeyIndex( const CORE::CVariant& key )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( !m_isOrigin )
    {
        TKeyIndexMap::const_iterator i = m_keyIndex.find( key );
        if ( i == m_keyIndex.end() )
            return InvalidKeyIndex;
        return (*i).second;
    }
    return AddKey( key, InvalidKeyIndex, m_settings.IsReliableKey( key ), m_settings.GetQuantisation( key ) );
}

/*-------------------------------------------------------------------------*/

CORE::CVariant
CDRNDataGrid::GetKeyByIndex( UInt32 index ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( index >= m_keyCount || !m_slots[ index ].used )
        return CORE::CVariant::Empty;
    return m_slots[ index ].key;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDataGrid::GetKeyCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    UInt32 count = 0;
    for ( UInt32 i=0; i<m_keyCount; ++i )
    {
        const CKeySlot& slot = m_slots[ i ];
        if ( !slot.used )
            continue;
        if ( m_isOrigin ? ( !slot.removed && 0 != slot.changeTick ) : ( !slot.frontRemoved && 0 != slot.updateTick ) )
            ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::GetKeys( GUCEF::vector< CORE::CVariant >& keys ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    keys.clear();
    for ( UInt32 i=0; i<m_keyCount; ++i )
    {
        const CKeySlot& slot = m_slots[ i ];
        if ( !slot.used )
            continue;
        if ( m_isOrigin ? ( !slot.removed && 0 != slot.changeTick ) : ( !slot.frontRemoved && 0 != slot.updateTick ) )
            keys.push_back( slot.key );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDataGrid::GetRevision( UInt32 index ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( index >= m_keyCount )
        return 0;
    return m_isOrigin ? m_slots[ index ].revision : m_slots[ index ].frontRevision;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDataGrid::GetLastChangeTick( UInt32 index ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( index >= m_keyCount )
        return 0;
    return m_slots[ index ].updateTick;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNDataGrid::GetLastChangeSendTime( UInt32 index ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( index >= m_keyCount )
        return 0;
    return m_isOrigin ? m_slots[ index ].changeSendTime : m_slots[ index ].frontSendTime;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDataGrid::GetLastWriter( UInt32 index ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( index >= m_keyCount )
        return 0;
    return m_isOrigin ? m_slots[ index ].lastWriter : m_slots[ index ].frontWriter;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDataGrid::GetUpdateTick( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    return m_updateTick;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::GetChangedSince( UInt32 updateTick, TDRNKeyIndexVector& indices ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    indices.clear();
    for ( UInt32 i=0; i<m_keyCount; ++i )
    {
        const CKeySlot& slot = m_slots[ i ];
        if ( slot.used && 0 != slot.updateTick && DRNIsNewerSequence( slot.updateTick, updateTick ) )
            indices.push_back( i );
    }
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNDataGrid::GetBytesSent( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    return m_bytesSent;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNDataGrid::GetDeltasSent( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    return m_deltasSent;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNDataGrid::GetDeltasReceived( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    return m_deltasReceived;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDataGrid::GetResyncCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    return m_resyncCount;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::RequestResync( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( m_isOrigin || m_removed || !m_subscribeSent )
        return;
    ++m_resyncCount;
    GetReplicator().Resubscribe( *this );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::ClearBufferedDeltas( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_bufferedCount; ++i )
        m_bufferedDeltas[ i ]->SetDataSize( 0 );
    m_bufferedCount = 0;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::OnResubscribe( void )
{GUCEF_TRACE;

    CDRNDataItem::OnResubscribe();
    m_inSnapshot = false;
    m_snapshotJustCompleted = false;
    m_ackDue = false;
    m_ackWindow.Reset();
    ClearBufferedDeltas();
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::WriteSubscribeParams( CDRNWireWriter& writer )
{GUCEF_TRACE;

    // the publisher numbers its deltas from here on: older ones still on the way are recognised as stale
    if ( m_anySeqSeen )
        m_startSeq = m_highestSeqSeen + 0x10000;
    else
        m_startSeq = (UInt32) ( CDRNClock::NowInMicroseconds() & 0x3FFFFFFF ) + 1;
    m_anySeqSeen = true;
    m_highestSeqSeen = m_startSeq;
    m_snapshotSeq = m_startSeq;
    writer.WriteUInt32( m_startSeq );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::OnRemoved( void )
{GUCEF_TRACE;

    CDRNDataItem::OnRemoved();
    while ( !m_votes.empty() )
        EvaluateVote( 0, false, true );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::WriteAnnounceSettings( CDRNWireWriter& writer ) const
{GUCEF_TRACE;

    UInt8 flags = 0;
    if ( m_settings.reliable )
        flags |= AnnounceFlagReliable;
    if ( m_settings.deltaAgainstAck )
        flags |= AnnounceFlagDeltaAgainstAck;
    if ( m_settings.snapshotOnSubscribe )
        flags |= AnnounceFlagSnapshot;
    writer.WriteUInt8( (UInt8) m_settings.authority );
    writer.WriteUInt8( flags );
    writer.WriteVarUInt32( m_settings.sendRateHz );
    writer.WriteVarUInt32( m_settings.maxBytesPerTick );
    writer.WriteVarUInt32( m_settings.capacity );
    writer.WriteFloat32( m_settings.priority );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::ReadAnnounceSettings( CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt8 authority = 0;
    UInt8 flags = 0;
    UInt32 sendRateHz = m_settings.sendRateHz;
    UInt32 maxBytesPerTick = m_settings.maxBytesPerTick;
    UInt32 capacity = m_settings.capacity;
    Float32 priority = m_settings.priority;
    if ( !reader.ReadUInt8( authority ) || !reader.ReadUInt8( flags ) || !reader.ReadVarUInt32( sendRateHz ) ||
         !reader.ReadVarUInt32( maxBytesPerTick ) || !reader.ReadVarUInt32( capacity ) || !reader.ReadFloat32( priority ) )
        return;

    m_settings.name = m_name;
    m_settings.authority = authority <= DRN_AUTHORITY_PROPOSAL_VOTE ? (TDRNAuthority) authority : DRN_AUTHORITY_HOST;
    m_settings.reliable = 0 != ( flags & AnnounceFlagReliable );
    m_settings.deltaAgainstAck = 0 != ( flags & AnnounceFlagDeltaAgainstAck );
    m_settings.snapshotOnSubscribe = 0 != ( flags & AnnounceFlagSnapshot );
    m_settings.sendRateHz = sendRateHz;
    m_settings.maxBytesPerTick = maxBytesPerTick;
    m_settings.capacity = capacity > 65536 ? 65536 : capacity;
    m_settings.priority = priority;
    EnsureSlotCount( m_settings.capacity > 0 ? m_settings.capacity : 1 );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGrid::IsDue( UInt64 nodeNow ) const
{GUCEF_TRACE;
    if ( m_subscribers.empty() )
        return m_writeTickUsed;
    return m_sendPending || nodeNow >= m_nextSendTime;
}

/*-------------------------------------------------------------------------*/

Float32
CDRNDataGrid::GetPriority( void ) const
{GUCEF_TRACE;
    return m_settings.priority;
}

/*-------------------------------------------------------------------------*/

CDRNDataGrid::CSubscriber*
CDRNDataGrid::FindSubscriber( UInt32 peerId ) const
{GUCEF_TRACE;
    for ( UInt32 i=0; i<m_subscribers.size(); ++i )
        if ( m_subscribers[ i ]->peerId == peerId )
            return m_subscribers[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::OnSubscriberAdded( UInt32 peerId, UInt32 /* flags */, CDRNWireReader& params )
{GUCEF_TRACE;

    UInt32 startSeq = 1;
    if ( !params.ReadUInt32( startSeq ) )
        startSeq = 1;

    CSubscriber* subscriber = FindSubscriber( peerId );
    if ( GUCEF_NULL == subscriber )
    {
        subscriber = GUCEF_NEW CSubscriber();
        subscriber->peerId = peerId;
        m_subscribers.push_back( subscriber );
    }
    subscriber->keys.assign( m_slots.size(), CKeyState() );
    subscriber->nextSeq = startSeq;
    subscriber->lastReliableSeq = startSeq;
    subscriber->needsSnapshot = true;
    subscriber->hasUnacked = false;
    m_subscriberCount = (UInt32) m_subscribers.size();
    m_sendPending = true;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::OnSubscriberRemoved( UInt32 peerId )
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

    // the member no longer votes
    for ( UInt32 v=0; v<m_votes.size(); )
    {
        CVote* vote = m_votes[ v ];
        for ( UInt32 n=0; n<vote->voters.size(); ++n )
        {
            if ( vote->voters[ n ] == peerId )
            {
                vote->voters.erase( vote->voters.begin() + n );
                vote->votes.erase( vote->votes.begin() + n );
                break;
            }
        }
        if ( !EvaluateVote( v, false, false ) )
            ++v;
    }
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNDataGrid::GetResendTimeout( UInt32 peerId ) const
{GUCEF_TRACE;
    CDRNPeerLink* link = GetReplicator().GetLinkToPeer( peerId );
    if ( GUCEF_NULL == link )
        return MinResendTimeout;
    CDRNLatencyInfo info;
    link->GetLatencyInfo( info );
    UInt64 timeout = 2 * (UInt64) info.smoothedRtt + 4 * (UInt64) info.rttVariance;
    if ( m_settings.sendRateHz > 0 )
        timeout += 2000000 / m_settings.sendRateHz;
    return timeout < MinResendTimeout ? MinResendTimeout : timeout;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGrid::OnSendTick( UInt64 nodeNow )
{GUCEF_TRACE;
    bool allSent = true;
    if ( !m_isOrigin && !m_snapshotComplete )
    {
        // a replica serves its subscribers once it has the whole state itself
        m_sendPending = !m_subscribers.empty();
    }
    else
    if ( !m_subscribers.empty() && ( m_sendPending || nodeNow >= m_nextSendTime ) )
    {
        if ( m_settings.sendRateHz > 0 )
        {
            UInt64 interval = 1000000 / m_settings.sendRateHz;
            m_nextSendTime += interval;
            if ( m_nextSendTime <= nodeNow )
                m_nextSendTime = nodeNow + interval;
        }
        m_sendPending = false;

        UInt64 sessionTime = GetReplicator().GetTickSessionTime();
        for ( UInt32 i=0; i<m_subscribers.size(); ++i )
        {
            CSubscriber& subscriber = *m_subscribers[ i ];
            if ( subscriber.needsSnapshot )
                SendSnapshot( subscriber, nodeNow, sessionTime );
            else
            if ( !SendDeltas( subscriber, nodeNow, sessionTime ) )
                allSent = false;
        }
        m_sendPending = !allSent;
    }

    // every change after this belongs to the next write tick
    if ( m_writeTickUsed )
    {
        ++m_writeTick;
        if ( 0 == m_writeTick )
            m_writeTick = 1;
        m_writeTickUsed = false;
    }
    return allSent;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::SendBoundary( UInt32 peerId, UInt32 opcode, UInt32 seq )
{GUCEF_TRACE;
    CDRNWireWriter& writer = BeginMessage();
    writer.WriteVarUInt32( seq );
    SendMessage( peerId, DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_REPL, opcode );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::SendSnapshot( CSubscriber& subscriber, UInt64 nodeNow, UInt64 sessionTime )
{GUCEF_TRACE;

    UInt32 seq = subscriber.nextSeq++;
    SendBoundary( subscriber.peerId, DRN_REPL_SNAPSHOT_BEGIN, seq );

    m_defineIndices.clear();
    m_reliableIndices.clear();
    for ( UInt32 i=0; i<m_keyCount; ++i )
    {
        const CKeySlot& slot = m_slots[ i ];
        if ( !slot.used )
            continue;
        m_defineIndices.push_back( i );
        if ( 0 != slot.changeTick )
            m_reliableIndices.push_back( i );
    }
    SendKeyDefines( subscriber, m_defineIndices );

    if ( m_settings.snapshotOnSubscribe )
    {
        UInt32 budget = 0xFFFFFFFF;
        SendDeltaMessages( subscriber, m_reliableIndices, true, true, seq, nodeNow, sessionTime, budget );
    }
    else
    {
        // only what changes from now on
        for ( UInt32 i=0; i<m_reliableIndices.size(); ++i )
        {
            UInt32 index = m_reliableIndices[ i ];
            CKeyState& state = subscriber.keys[ index ];
            state.sentChange = state.ackedChange = m_slots[ index ].changeTick;
        }
    }

    SendBoundary( subscriber.peerId, DRN_REPL_SNAPSHOT_END, seq );
    subscriber.lastReliableSeq = seq;
    subscriber.needsSnapshot = false;
    subscriber.hasUnacked = false;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::SendKeyDefines( CSubscriber& subscriber, const TDRNKeyIndexVector& indices )
{GUCEF_TRACE;

    UInt32 messageBudget = GetReplicator().GetMessageBudget( subscriber.peerId );
    UInt32 pos = 0;
    while ( pos < indices.size() )
    {
        CDRNWireWriter& writer = BeginMessage();
        UInt32 countOffset = writer.GetOffset();
        writer.WriteUInt16( 0 );
        UInt32 count = 0;
        while ( pos < indices.size() && count < 0xFFFF )
        {
            UInt32 index = indices[ pos ];
            const CKeySlot& slot = m_slots[ index ];
            UInt32 before = writer.GetOffset();
            writer.WriteVarUInt32( index );
            writer.WriteUInt8( slot.reliableKey ? KeyFlagReliable : 0 );
            writer.WriteVarInt32( slot.quantisation.floatPrecision );
            writer.WriteVarUInt32( slot.quantisation.intBits );
            CDRNVariantCodec::Write( writer, slot.key );
            if ( count > 0 && writer.GetOffset() > messageBudget )
            {
                writer.SetOffset( before );
                break;
            }
            subscriber.keys[ index ].defined = true;
            ++count;
            ++pos;
        }
        UInt8* countPtr = static_cast< UInt8* >( m_txBuffer.GetBufferPtr() ) + countOffset;
        countPtr[ 0 ] = (UInt8) ( count >> 8 );
        countPtr[ 1 ] = (UInt8) count;
        SendMessage( subscriber.peerId, DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_REPL, DRN_REPL_KEY_DEFINE );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDataGrid::SendDeltaMessages( CSubscriber& subscriber        ,
                                 const TDRNKeyIndexVector& indices ,
                                 bool reliable                  ,
                                 bool snapshot                  ,
                                 UInt32 snapshotSeq             ,
                                 UInt64 nodeNow                 ,
                                 UInt64 sessionTime             ,
                                 UInt32& budget                 )
{GUCEF_TRACE;
    UInt32 messageBudget = GetReplicator().GetMessageBudget( subscriber.peerId );
    UInt8 lane = reliable ? (UInt8) DRN_LANE_RELIABLE_ORDERED : (UInt8) DRN_LANE_UNRELIABLE;
    bool noAck = !reliable && !m_settings.deltaAgainstAck;
    UInt8 flags = 0;
    if ( reliable )
        flags |= DeltaFlagReliable | DeltaFlagRevisions;
    if ( snapshot )
        flags |= DeltaFlagSnapshot;
    if ( noAck )
        flags |= DeltaFlagNoAck;
    bool withRevisions = 0 != ( flags & DeltaFlagRevisions );

    UInt32 total = (UInt32) indices.size();
    UInt32 pos = 0;
    while ( pos < total && budget > 0 )
    {
        UInt32 seq = snapshot ? snapshotSeq : subscriber.nextSeq++;
        CDRNWireWriter& writer = BeginMessage();
        writer.WriteVarUInt32( seq );
        writer.WriteUInt8( flags );
        if ( reliable && !snapshot )
            writer.WriteVarUInt32( subscriber.lastReliableSeq );
        writer.WriteVarUInt64( sessionTime );
        UInt32 countOffset = writer.GetOffset();
        writer.WriteUInt16( 0 );

        UInt32 count = 0;
        bool budgetReached = false;
        while ( pos < total && count < 0xFFFF )
        {
            UInt32 index = indices[ pos ];
            const CKeySlot& slot = m_slots[ index ];
            UInt32 before = writer.GetOffset();
            writer.WriteVarUInt32( index );
            if ( withRevisions )
            {
                writer.WriteVarUInt32( slot.revision );
                writer.WriteVarUInt32( slot.lastWriter );
            }
            if ( slot.removed )
                CDRNVariantCodec::WriteRemoved( writer );
            else
                CDRNVariantCodec::Write( writer, slot.value, slot.quantisation.IsFull() ? GUCEF_NULL : &slot.quantisation );

            UInt32 size = writer.GetOffset();
            if ( count > 0 )
            {
                if ( size > messageBudget )
                {
                    writer.SetOffset( before );
                    break;
                }
                if ( size > budget )
                {
                    writer.SetOffset( before );
                    budgetReached = true;
                    break;
                }
            }

            CKeyState& state = subscriber.keys[ index ];
            state.sentChange = slot.changeTick;
            state.sentSeq = seq;
            if ( reliable || noAck )
                state.ackedChange = slot.changeTick;
            ++count;
            ++pos;
        }

        UInt8* countPtr = static_cast< UInt8* >( m_txBuffer.GetBufferPtr() ) + countOffset;
        countPtr[ 0 ] = (UInt8) ( count >> 8 );
        countPtr[ 1 ] = (UInt8) count;

        UInt32 bytes = writer.GetOffset();
        SendMessage( subscriber.peerId, lane, DRN_MSGCLASS_REPL, DRN_REPL_GRID_DELTA );
        budget = bytes >= budget ? 0 : budget - bytes;
        GetReplicator().ConsumeLinkBudget( subscriber.peerId, bytes );
        m_bytesSent += bytes;
        ++m_deltasSent;

        if ( reliable && !snapshot )
            subscriber.lastReliableSeq = seq;
        if ( !reliable && !noAck && !subscriber.hasUnacked )
        {
            subscriber.hasUnacked = true;
            subscriber.unackedSince = nodeNow;
        }
        if ( budgetReached )
            break;
    }
    return pos;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGrid::SendDeltas( CSubscriber& subscriber, UInt64 nodeNow, UInt64 sessionTime )
{GUCEF_TRACE;
    // deltas that were neither acknowledged nor reported lost in time count as lost
    if ( subscriber.hasUnacked && nodeNow - subscriber.unackedSince > GetResendTimeout( subscriber.peerId ) )
    {
        for ( UInt32 i=0; i<m_keyCount; ++i )
        {
            CKeyState& state = subscriber.keys[ i ];
            if ( state.sentChange != state.ackedChange )
                state.sentChange = state.ackedChange;
        }
        subscriber.hasUnacked = false;
    }

    m_defineIndices.clear();
    m_reliableIndices.clear();
    m_unreliableIndices.clear();
    for ( UInt32 i=0; i<m_keyCount; ++i )
    {
        const CKeySlot& slot = m_slots[ i ];
        if ( 0 == slot.changeTick )
            continue;
        const CKeyState& state = subscriber.keys[ i ];
        if ( state.sentChange == slot.changeTick || state.ackedChange == slot.changeTick )
            continue;
        if ( !state.defined )
            m_defineIndices.push_back( i );
        if ( m_settings.reliable || slot.reliableKey )
            m_reliableIndices.push_back( i );
        else
            m_unreliableIndices.push_back( i );
    }
    if ( m_reliableIndices.empty() && m_unreliableIndices.empty() )
        return true;

    if ( !m_defineIndices.empty() )
        SendKeyDefines( subscriber, m_defineIndices );

    UInt32 budget = GetReplicator().GetLinkBudget( subscriber.peerId );
    if ( m_settings.maxBytesPerTick > 0 && m_settings.maxBytesPerTick < budget )
        budget = m_settings.maxBytesPerTick;

    UInt32 sent = 0;
    if ( !m_reliableIndices.empty() )
        sent += SendDeltaMessages( subscriber, m_reliableIndices, true, false, 0, nodeNow, sessionTime, budget );
    if ( !m_unreliableIndices.empty() && budget > 0 )
        sent += SendDeltaMessages( subscriber, m_unreliableIndices, false, false, 0, nodeNow, sessionTime, budget );
    return sent == m_reliableIndices.size() + m_unreliableIndices.size();
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::OnAck( CSubscriber& subscriber, UInt32 ackSeq, UInt32 ackBits, UInt64 nodeNow )
{GUCEF_TRACE;
    bool inFlight = false;
    bool lost = false;
    for ( UInt32 i=0; i<m_keyCount; ++i )
    {
        CKeyState& state = subscriber.keys[ i ];
        if ( state.sentChange == state.ackedChange )
            continue;
        UInt32 distance = ackSeq - state.sentSeq;
        if ( 0 == distance || ( distance <= 32 && 0 != ( ( ackBits >> ( distance - 1 ) ) & 1 ) ) )
        {
            state.ackedChange = state.sentChange;
        }
        else
        if ( (Int32) distance > 0 )
        {
            // a newer delta arrived and this one did not: send the key again
            state.sentChange = state.ackedChange;
            lost = true;
        }
        else
        {
            inFlight = true;
        }
    }
    subscriber.hasUnacked = inFlight;
    if ( inFlight )
        subscriber.unackedSince = nodeNow;
    if ( lost )
        m_sendPending = true;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::OnItemMessage( UInt32 fromPeerId, UInt8 /* msgClass */, UInt32 opcode, CDRNWireReader& reader )
{GUCEF_TRACE;
    // what the publisher sends comes from upstream only
    bool fromUpstream = !m_isOrigin && fromPeerId == m_upstreamPeerId && m_subscribeSent;
    switch ( opcode )
    {
        case DRN_REPL_GRID_DELTA:
        {
            if ( fromUpstream )
                OnDelta( reader, false );
            break;
        }
        case DRN_REPL_GRID_ACK:
        {
            UInt32 ackSeq = 0;
            UInt32 ackBits = 0;
            CSubscriber* subscriber = FindSubscriber( fromPeerId );
            if ( GUCEF_NULL != subscriber && reader.ReadVarUInt32( ackSeq ) && reader.ReadUInt32( ackBits ) )
                OnAck( *subscriber, ackSeq, ackBits, CDRNClock::NowInMicroseconds() );
            break;
        }
        case DRN_REPL_KEY_DEFINE:
        {
            if ( fromUpstream )
                OnKeyDefine( reader );
            break;
        }
        case DRN_REPL_SNAPSHOT_BEGIN:
        case DRN_REPL_SNAPSHOT_END:
        {
            UInt32 seq = 0;
            if ( fromUpstream && reader.ReadVarUInt32( seq ) )
            {
                if ( DRN_REPL_SNAPSHOT_BEGIN == opcode )
                    OnSnapshotBegin( seq );
                else
                    OnSnapshotEnd( seq );
            }
            break;
        }
        case DRN_REPL_GRID_PROPOSE:     OnPropose( fromPeerId, reader ); break;
        case DRN_REPL_PROPOSE_RESULT:   if ( fromPeerId == m_ownerPeerId || fromUpstream ) OnProposeResult( reader ); break;
        case DRN_REPL_VOTE_START:       if ( fromPeerId == m_ownerPeerId ) OnVoteStart( reader ); break;
        case DRN_REPL_VOTE_CAST:        OnVoteCast( fromPeerId, reader ); break;
        case DRN_REPL_VOTE_END:         if ( fromPeerId == m_ownerPeerId ) OnVoteEnd( reader ); break;
        default: break;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::OnKeyDefine( CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt16 count = 0;
    if ( !reader.ReadUInt16( count ) )
        return;
    for ( UInt32 n=0; n<count; ++n )
    {
        UInt32 index = 0;
        UInt8 flags = 0;
        Int32 precision = -1;
        UInt32 intBits = 0;
        if ( !reader.ReadVarUInt32( index ) || !reader.ReadUInt8( flags ) || !reader.ReadVarInt32( precision ) ||
             !reader.ReadVarUInt32( intBits ) || !CDRNVariantCodec::Read( reader, m_rxKey ) )
            return;
        if ( index >= 0x1000000 )
            return;
        AddKey( m_rxKey, index, 0 != ( flags & KeyFlagReliable ), CDRNQuantisation( precision, intBits ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::OnSnapshotBegin( UInt32 seq )
{GUCEF_TRACE;

    // the snapshot of this subscription carries the sequence the subscription asked to start at
    if ( seq != m_startSeq || m_snapshotComplete )
        return;
    m_inSnapshot = true;
    m_snapshotSeq = seq;
    m_lastReliableSeq = seq;
    m_ackWindow.Reset();
    m_ackDue = false;
    for ( UInt32 i=0; i<m_keyCount; ++i )
        m_slots[ i ].rxSeqValid = false;
    if ( DRNIsNewerSequence( seq, m_highestSeqSeen ) )
        m_highestSeqSeen = seq;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::OnSnapshotEnd( UInt32 seq )
{GUCEF_TRACE;

    if ( !m_inSnapshot || seq != m_snapshotSeq )
        return;
    m_inSnapshot = false;
    m_snapshotComplete = true;
    m_snapshotJustCompleted = true;

    // what came in during the snapshot and is newer than it
    for ( UInt32 i=0; i<m_bufferedCount; ++i )
    {
        CORE::CDynamicBuffer* buffered = m_bufferedDeltas[ i ];
        CDRNWireReader reader( buffered->GetConstBufferPtr(), buffered->GetDataSize() );
        OnDelta( reader, true );
    }
    ClearBufferedDeltas();
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::OnDelta( CDRNWireReader& reader, bool replay )
{GUCEF_TRACE;
    const UInt8* deltaStart = reader.GetCurrentPtr();
    UInt32 deltaSize = reader.GetRemaining();

    UInt32 seq = 0;
    UInt8 flags = 0;
    UInt32 baseSeq = 0;
    UInt64 sendTime = 0;
    UInt16 count = 0;
    if ( !reader.ReadVarUInt32( seq ) || !reader.ReadUInt8( flags ) )
        return;
    bool reliable = 0 != ( flags & DeltaFlagReliable );
    bool snapshot = 0 != ( flags & DeltaFlagSnapshot );
    if ( reliable && !snapshot && !reader.ReadVarUInt32( baseSeq ) )
        return;
    if ( !reader.ReadVarUInt64( sendTime ) || !reader.ReadUInt16( count ) )
        return;
    if ( !replay )
        ++m_deltasReceived;

    if ( snapshot )
    {
        if ( !m_inSnapshot || seq != m_snapshotSeq )
            return;
    }
    else
    {
        if ( !m_snapshotComplete )
        {
            // the snapshot is on the way: keep what is newer than where the subscription starts
            if ( DRNIsNewerSequence( m_startSeq, seq ) || m_bufferedCount >= MaxBufferedDeltas )
                return;
            if ( m_bufferedCount >= m_bufferedDeltas.size() )
                m_bufferedDeltas.push_back( GUCEF_NEW CORE::CDynamicBuffer() );
            m_bufferedDeltas[ m_bufferedCount++ ]->CopyFrom( deltaSize, deltaStart );
            return;
        }
        if ( !DRNIsNewerSequence( seq, m_snapshotSeq ) )
            return;     // from before the snapshot
        if ( reliable )
        {
            if ( baseSeq != m_lastReliableSeq )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRNDataGrid(" << m_name << "): A reliable delta is missing, asking for the whole state again" );
                ++m_resyncCount;
                GetReplicator().Resubscribe( *this );
                return;
            }
            m_lastReliableSeq = seq;
        }
    }
    if ( DRNIsNewerSequence( seq, m_highestSeqSeen ) )
        m_highestSeqSeen = seq;

    bool withRevisions = 0 != ( flags & DeltaFlagRevisions );
    for ( UInt32 n=0; n<count; ++n )
    {
        UInt32 index = 0;
        UInt32 revision = 0;
        UInt32 writer = 0;
        if ( !reader.ReadVarUInt32( index ) )
            return;
        if ( withRevisions && ( !reader.ReadVarUInt32( revision ) || !reader.ReadVarUInt32( writer ) ) )
            return;
        if ( index >= m_keyCount || !m_slots[ index ].used )
            return;     // KEY_DEFINE has not arrived yet: not acknowledged, the publisher sends it again

        CKeySlot& slot = m_slots[ index ];
        bool fresh = snapshot || !slot.rxSeqValid || DRNIsNewerSequence( seq, slot.rxSeq );
        bool removed = false;
        if ( !CDRNVariantCodec::Read( reader, fresh ? slot.value : m_rxScratch, &removed ) )
            return;
        if ( !fresh )
            continue;

        slot.rxSeq = seq;
        slot.rxSeqValid = true;
        slot.removed = removed;
        if ( withRevisions )
        {
            slot.revision = revision;
            slot.lastWriter = writer;
        }
        else
        {
            ++slot.revision;
            slot.lastWriter = m_ownerPeerId;
        }
        slot.changeSendTime = sendTime;
        StampChange( slot );
        MarkFrontPending( index );
    }

    if ( !reliable && 0 == ( flags & DeltaFlagNoAck ) )
    {
        UInt32 gap = 0;
        m_ackWindow.Accept( seq, gap );
        m_ackDue = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::OnEndOfTick( UInt64 nodeNow )
{GUCEF_TRACE;
    if ( !m_frontPending.empty() )
    {
        ++m_updateTick;
        if ( 0 == m_updateTick )
            m_updateTick = 1;

        // the changed indices move to the event data, so that writes from event handlers start a new list
        TDRNKeyIndexVector& changed = m_updatedEventData.GetData();
        changed.clear();
        changed.swap( m_frontPending );
        for ( UInt32 i=0; i<changed.size(); ++i )
        {
            CKeySlot& slot = m_slots[ changed[ i ] ];
            slot.frontPending = false;
            slot.updateTick = m_updateTick;
            if ( !m_isOrigin )
            {
                slot.frontValue = slot.value;
                slot.frontRemoved = slot.removed;
                slot.frontRevision = slot.revision;
                slot.frontWriter = slot.lastWriter;
                slot.frontSendTime = slot.changeSendTime;
            }
        }

        if ( DRN_NOTIFY_PER_PACKET == m_settings.notify )
        {
            Emit( GridUpdatedEvent, &m_updatedEventData );
        }
        else
        if ( DRN_NOTIFY_PER_CHANGE == m_settings.notify )
        {
            for ( UInt32 i=0; i<changed.size(); ++i )
            {
                CORE::TCloneableUInt32 eventData( changed[ i ] );
                Emit( GridKeyChangedEvent, &eventData );
            }
        }
    }

    if ( m_snapshotJustCompleted )
    {
        m_snapshotJustCompleted = false;
        Emit( SnapshotCompleteEvent );
    }

    if ( m_ackDue && !m_isOrigin && m_ackWindow.HasAny() )
    {
        m_ackDue = false;
        CDRNWireWriter& writer = BeginMessage();
        writer.WriteVarUInt32( m_ackWindow.GetHighest() );
        writer.WriteUInt32( m_ackWindow.GetAckBits() );
        SendMessage( m_upstreamPeerId, DRN_LANE_UNRELIABLE, DRN_MSGCLASS_REPL, DRN_REPL_GRID_ACK );
    }

    if ( m_isOrigin )
    {
        for ( UInt32 v=0; v<m_votes.size(); )
        {
            if ( nodeNow >= m_votes[ v ]->deadline && EvaluateVote( v, true, false ) )
                continue;
            ++v;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGrid::Propose( const CORE::CVariant& key, const CORE::CVariant& value, bool remove )
{GUCEF_TRACE;

    if ( DRN_AUTHORITY_OWNER == m_settings.authority )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNDataGrid(" << m_name << "): Only the owner writes this grid" );
        return false;
    }
    CDRNWireWriter& writer = BeginMessage();
    writer.WriteVarUInt32( ++m_nextProposalId );
    CDRNVariantCodec::Write( writer, key );
    if ( remove )
        CDRNVariantCodec::WriteRemoved( writer );
    else
        CDRNVariantCodec::Write( writer, value );
    return SendMessage( m_ownerPeerId, DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_REPL, DRN_REPL_GRID_PROPOSE );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::SendProposeResult( UInt32 proposerPeerId, UInt32 proposalId, bool accepted, UInt32 voteOutcome, const CORE::CString& reason )
{GUCEF_TRACE;

    CDRNWireWriter& writer = BeginMessage();
    writer.WriteVarUInt32( proposalId );
    writer.WriteUInt8( accepted ? 1 : 0 );
    writer.WriteVarUInt32( voteOutcome );
    writer.WriteString( reason );
    SendMessage( proposerPeerId, DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_REPL, DRN_REPL_PROPOSE_RESULT );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::OnPropose( UInt32 fromPeerId, CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 proposalId = 0;
    bool remove = false;
    if ( !reader.ReadVarUInt32( proposalId ) || !CDRNVariantCodec::Read( reader, m_rxKey ) || !CDRNVariantCodec::Read( reader, m_rxScratch, &remove ) )
        return;

    if ( !m_isOrigin || m_removed )
    {
        SendProposeResult( fromPeerId, proposalId, false, CDRNVotePolicy::VOTEOUTCOME_PENDING, "not the publisher of the grid" );
        return;
    }
    if ( DRN_AUTHORITY_OWNER == m_settings.authority )
    {
        SendProposeResult( fromPeerId, proposalId, false, CDRNVotePolicy::VOTEOUTCOME_PENDING, "only the owner writes the grid" );
        return;
    }

    TKeyIndexMap::const_iterator i = m_keyIndex.find( m_rxKey );
    UInt32 index = i == m_keyIndex.end() ? InvalidKeyIndex : (*i).second;
    if ( !GetSession().GetNode().GetAccessPolicy().MayWriteAs( fromPeerId, GetSession().GetMemberAuthResult( fromPeerId ), m_name, index ) )
    {
        SendProposeResult( fromPeerId, proposalId, false, CDRNVotePolicy::VOTEOUTCOME_PENDING, "not allowed to write" );
        return;
    }
    CORE::CString reason;
    CIDRNWriteValidator* validator = GUCEF_NULL != m_validator ? m_validator : &m_defaultValidator;
    if ( !validator->ValidateWrite( fromPeerId, *this, m_rxKey, m_rxScratch, reason ) )
    {
        SendProposeResult( fromPeerId, proposalId, false, CDRNVotePolicy::VOTEOUTCOME_PENDING, reason.IsNULLOrEmpty() ? CORE::CString( "refused" ) : reason );
        return;
    }

    if ( DRN_AUTHORITY_PROPOSAL_VOTE == m_settings.authority )
    {
        StartVote( fromPeerId, proposalId, m_rxKey, m_rxScratch, remove );
        return;
    }

    if ( InvalidKeyIndex == index )
    {
        if ( remove )
        {
            SendProposeResult( fromPeerId, proposalId, false, CDRNVotePolicy::VOTEOUTCOME_PENDING, "no such key" );
            return;
        }
        index = AddKey( m_rxKey, InvalidKeyIndex, m_settings.IsReliableKey( m_rxKey ), m_settings.GetQuantisation( m_rxKey ) );
    }
    ApplyWrite( index, m_rxScratch, remove, fromPeerId );
    SendProposeResult( fromPeerId, proposalId, true, CDRNVotePolicy::VOTEOUTCOME_PENDING, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::OnProposeResult( CDRNWireReader& reader )
{GUCEF_TRACE;

    CDRNProposeResultEventData eventData;
    CDRNProposeResult& result = eventData.GetData();
    UInt8 accepted = 0;
    if ( !reader.ReadVarUInt32( result.proposalId ) || !reader.ReadUInt8( accepted ) || !reader.ReadVarUInt32( result.voteOutcome ) || !reader.ReadString( result.reason ) )
        return;
    result.accepted = 0 != accepted;
    Emit( ProposeResultEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::StartVote( UInt32 proposerPeerId, UInt32 proposalId, const CORE::CVariant& key, const CORE::CVariant& value, bool remove )
{GUCEF_TRACE;

    const CDRNVotePolicy& policy = m_settings.votePolicy;
    CVote* vote = GUCEF_NEW CVote();
    vote->voteId = ++m_nextVoteId;
    vote->proposerPeerId = proposerPeerId;
    vote->proposalId = proposalId;
    vote->key = key;
    vote->value = value;
    vote->isRemoval = remove;
    vote->deadline = CDRNClock::NowInMicroseconds() + (UInt64) policy.timeoutMs * 1000;

    // the members that have the grid vote, the sequencer does not
    UInt32 localPeerId = GetReplicator().GetLocalPeerId();
    for ( UInt32 i=0; i<m_subscribers.size(); ++i )
    {
        UInt32 peerId = m_subscribers[ i ]->peerId;
        if ( peerId == localPeerId )
            continue;
        if ( peerId == proposerPeerId )
        {
            if ( !policy.proposerVoteCounts )
                continue;
            vote->voters.push_back( peerId );
            vote->votes.push_back( 1 );
            continue;
        }
        vote->voters.push_back( peerId );
        vote->votes.push_back( 0 );
    }
    m_votes.push_back( vote );

    for ( UInt32 i=0; i<vote->voters.size(); ++i )
    {
        if ( 0 != vote->votes[ i ] )
            continue;
        CDRNWireWriter& writer = BeginMessage();
        writer.WriteVarUInt32( vote->voteId );
        writer.WriteVarUInt32( proposerPeerId );
        CDRNVariantCodec::Write( writer, key );
        if ( remove )
            CDRNVariantCodec::WriteRemoved( writer );
        else
            CDRNVariantCodec::Write( writer, value );
        writer.WriteVarUInt32( policy.timeoutMs );
        SendMessage( vote->voters[ i ], DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_REPL, DRN_REPL_VOTE_START );
    }

    CDRNVoteEventData eventData;
    CDRNVoteInfo& info = eventData.GetData();
    info.voteId = vote->voteId;
    info.proposerPeerId = proposerPeerId;
    info.key = key;
    info.value = value;
    info.isRemoval = remove;
    info.timeoutMs = policy.timeoutMs;
    Emit( VoteStartedEvent, &eventData );

    EvaluateVote( (UInt32) m_votes.size() - 1, false, false );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGrid::EvaluateVote( UInt32 voteSlot, bool timedOut, bool cancel )
{GUCEF_TRACE;

    if ( voteSlot >= m_votes.size() )
        return false;
    CVote* vote = m_votes[ voteSlot ];
    UInt32 yes = 0;
    UInt32 no = 0;
    for ( UInt32 i=0; i<vote->votes.size(); ++i )
    {
        if ( 1 == vote->votes[ i ] )
            ++yes;
        else
        if ( 2 == vote->votes[ i ] )
            ++no;
    }
    CDRNVotePolicy::TVoteOutcome outcome = cancel ? CDRNVotePolicy::VOTEOUTCOME_CANCELLED
                                                  : m_settings.votePolicy.Evaluate( (UInt32) vote->voters.size(), yes, no, timedOut );
    if ( CDRNVotePolicy::VOTEOUTCOME_PENDING == outcome )
        return false;

    m_votes.erase( m_votes.begin() + voteSlot );
    bool passed = CDRNVotePolicy::VOTEOUTCOME_PASSED == outcome;
    if ( passed && !m_removed )
    {
        UInt32 index = AddKey( vote->key, InvalidKeyIndex, m_settings.IsReliableKey( vote->key ), m_settings.GetQuantisation( vote->key ) );
        ApplyWrite( index, vote->value, vote->isRemoval, vote->proposerPeerId );
    }

    for ( UInt32 i=0; i<vote->voters.size(); ++i )
    {
        CDRNWireWriter& writer = BeginMessage();
        writer.WriteVarUInt32( vote->voteId );
        writer.WriteVarUInt32( (UInt32) outcome );
        SendMessage( vote->voters[ i ], DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_REPL, DRN_REPL_VOTE_END );
    }

    CORE::CString reason;
    switch ( outcome )
    {
        case CDRNVotePolicy::VOTEOUTCOME_FAILED:    reason = "the vote failed"; break;
        case CDRNVotePolicy::VOTEOUTCOME_CANCELLED: reason = "the vote was cancelled"; break;
        case CDRNVotePolicy::VOTEOUTCOME_TIMEOUT:   reason = "the vote timed out"; break;
        default: break;
    }
    SendProposeResult( vote->proposerPeerId, vote->proposalId, passed, (UInt32) outcome, reason );

    CDRNVoteEventData eventData;
    CDRNVoteInfo& info = eventData.GetData();
    info.voteId = vote->voteId;
    info.proposerPeerId = vote->proposerPeerId;
    info.key = vote->key;
    info.value = vote->value;
    info.isRemoval = vote->isRemoval;
    info.outcome = (UInt32) outcome;
    GUCEF_DELETE vote;
    Emit( VoteEndedEvent, &eventData );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::OnVoteStart( CDRNWireReader& reader )
{GUCEF_TRACE;

    CDRNVoteEventData eventData;
    CDRNVoteInfo& info = eventData.GetData();
    if ( !reader.ReadVarUInt32( info.voteId ) || !reader.ReadVarUInt32( info.proposerPeerId ) ||
         !CDRNVariantCodec::Read( reader, info.key ) || !CDRNVariantCodec::Read( reader, info.value, &info.isRemoval ) ||
         !reader.ReadVarUInt32( info.timeoutMs ) )
        return;
    Emit( VoteStartedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::OnVoteEnd( CDRNWireReader& reader )
{GUCEF_TRACE;

    CDRNVoteEventData eventData;
    CDRNVoteInfo& info = eventData.GetData();
    if ( !reader.ReadVarUInt32( info.voteId ) || !reader.ReadVarUInt32( info.outcome ) )
        return;
    Emit( VoteEndedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CDRNDataGrid::OnVoteCast( UInt32 fromPeerId, CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 voteId = 0;
    UInt8 yes = 0;
    if ( !m_isOrigin || !reader.ReadVarUInt32( voteId ) || !reader.ReadUInt8( yes ) )
        return;
    for ( UInt32 v=0; v<m_votes.size(); ++v )
    {
        CVote* vote = m_votes[ v ];
        if ( vote->voteId != voteId )
            continue;
        for ( UInt32 n=0; n<vote->voters.size(); ++n )
        {
            if ( vote->voters[ n ] == fromPeerId && 0 == vote->votes[ n ] )
            {
                vote->votes[ n ] = 0 != yes ? 1 : 2;
                EvaluateVote( v, false, false );
                return;
            }
        }
        return;
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGrid::CastVote( UInt32 voteId, bool yes )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( m_isOrigin || m_removed )
        return false;
    CDRNWireWriter& writer = BeginMessage();
    writer.WriteVarUInt32( voteId );
    writer.WriteUInt8( yes ? 1 : 0 );
    return SendMessage( m_ownerPeerId, DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_REPL, DRN_REPL_VOTE_CAST );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGrid::Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& /* settings */ ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );

    domRootNode.SetName( "DRNDataGrid" );
    domRootNode.SetAttribute( "name", m_name );
    domRootNode.SetAttribute( "ownerPeerId", m_ownerPeerId );
    domRootNode.SetAttribute( "itemId", m_itemId );
    domRootNode.SetAttribute( "isOrigin", m_isOrigin );
    domRootNode.SetAttribute( "updateTick", m_updateTick );

    CORE::CDataNode* settingsNode = domRootNode.AddChild( "settings" );
    m_settings.SaveConfig( *settingsNode );

    CORE::CDataNode* keysNode = domRootNode.AddChild( "keys" );
    for ( UInt32 i=0; i<m_keyCount; ++i )
    {
        const CKeySlot& slot = m_slots[ i ];
        if ( !slot.used )
            continue;
        bool removed = m_isOrigin ? slot.removed : slot.frontRemoved;
        bool hasValue = m_isOrigin ? 0 != slot.changeTick : 0 != slot.updateTick;
        CORE::CDataNode* keyNode = keysNode->AddChild( "entry" );
        keyNode->SetAttribute( "index", i );
        keyNode->SetAttribute( "revision", m_isOrigin ? slot.revision : slot.frontRevision );
        keyNode->SetAttribute( "lastWriter", m_isOrigin ? slot.lastWriter : slot.frontWriter );
        keyNode->SetAttribute( "removed", removed || !hasValue );
        keyNode->AddChildWithValue( "key", slot.key );
        if ( !removed && hasValue )
            keyNode->AddChildWithValue( "value", m_isOrigin ? slot.value : slot.frontValue );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataGrid::Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& /* settings */ )
{GUCEF_TRACE;

    MT::CScopeMutex lock( GetSession().GetNode().GetLock() );
    if ( !m_isOrigin )
        return false;

    const CORE::CDataNode* keysNode = domRootNode.FindChild( "keys" );
    if ( GUCEF_NULL == keysNode )
        return false;

    UInt32 localPeerId = GetReplicator().GetLocalPeerId();
    CORE::CDataNode::const_iterator i = keysNode->ConstBegin();
    while ( i != keysNode->ConstEnd() )
    {
        const CORE::CDataNode* keyNode = (*i);
        ++i;
        const CORE::CDataNode* keyValueNode = keyNode->FindChild( "key" );
        if ( GUCEF_NULL == keyValueNode )
            continue;
        const CORE::CVariant& key = keyValueNode->GetValue();
        const CORE::CDataNode* valueNode = keyNode->FindChild( "value" );
        bool removed = keyNode->GetAttributeValue( "removed" ).AsBool( false, true ) || GUCEF_NULL == valueNode;
        UInt32 index = AddKey( key, InvalidKeyIndex, m_settings.IsReliableKey( key ), m_settings.GetQuantisation( key ) );
        ApplyWrite( index, removed ? CORE::CVariant::Empty : valueNode->GetValue(), removed, localPeerId );
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
