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

#include "gucefDRN_CDRNReplicator.h"

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

/** Messages are kept this small on stream channels too, so that one item does not hold up the others */
static const UInt32 MaxMessageBudget = 16 * 1024;

static const UInt32 NoBudgetLimit = 0xFFFFFFFF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt8
CDRNReplicator::CStreamComponent::GetMessageClass( void ) const
{GUCEF_TRACE;

    return DRN_MSGCLASS_STREAM;
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::CStreamComponent::OnSessionMessage( CDRNSession& /* session */, CDRNPeerLink& /* link */, UInt32 fromPeerId, const CDRNLinkMessage& message )
{GUCEF_TRACE;

    m_replicator->HandleMessage( fromPeerId, message );
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::CStreamComponent::OnSessionTick( CDRNSession& /* session */, UInt64 /* nodeNow */ )
{GUCEF_TRACE;

    // the replicator ticks the streams with the grids
}

/*-------------------------------------------------------------------------*/

CDRNReplicator::CDRNReplicator( CDRNSession& session )
    : CIDRNSessionComponent()
    , m_session( &session )
    , m_streamComponent()
    , m_items()
    , m_itemList()
    , m_retired()
    , m_dueItems()
    , m_appSubscriptions()
    , m_linkBudgets()
    , m_nextItemId( 1 )
    , m_tickSessionTime( 0 )
    , m_txBuffer()
{GUCEF_TRACE;

    m_streamComponent.m_replicator = this;
}

/*-------------------------------------------------------------------------*/

CDRNReplicator::~CDRNReplicator()
{GUCEF_TRACE;

    RetireAll();
    for ( UInt32 i=0; i<m_retired.size(); ++i )
        GUCEF_DELETE m_retired[ i ];
    m_retired.clear();
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::RetireAll( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_itemList.size(); ++i )
    {
        CDRNDataItem* item = m_itemList[ i ];
        if ( !item->m_removed )
            item->OnRemoved();
        item->m_subscribeSent = false;
        m_retired.push_back( item );
    }
    m_itemList.clear();
    m_items.clear();
    m_dueItems.clear();
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::Reset( void )
{GUCEF_TRACE;

    RetireAll();
    m_appSubscriptions.clear();
    m_nextItemId = 1;
}

/*-------------------------------------------------------------------------*/

CDRNSession&
CDRNReplicator::GetSession( void ) const
{GUCEF_TRACE;

    return *m_session;
}

/*-------------------------------------------------------------------------*/

CIDRNSessionComponent*
CDRNReplicator::GetStreamComponent( void )
{GUCEF_TRACE;

    return &m_streamComponent;
}

/*-------------------------------------------------------------------------*/

UInt8
CDRNReplicator::GetMessageClass( void ) const
{GUCEF_TRACE;

    return DRN_MSGCLASS_REPL;
}

/*-------------------------------------------------------------------------*/

CDRNPeerLink*
CDRNReplicator::GetLinkToPeer( UInt32 peerId ) const
{GUCEF_TRACE;
    CDRNSession::CMember* member = m_session->GetMember( peerId );
    if ( GUCEF_NULL == member || member->link.IsNULL() )
        return GUCEF_NULL;
    return member->link.GetPointerAlways();
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNReplicator::GetLocalPeerId( void ) const
{GUCEF_TRACE;
    return m_session->m_localPeerId;
}

/*-------------------------------------------------------------------------*/

bool
CDRNReplicator::IsSequencer( void ) const
{GUCEF_TRACE;
    return CDRNSession::SESSIONSTATE_HOSTING == m_session->m_state;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNReplicator::GetTickSessionTime( void ) const
{GUCEF_TRACE;
    return m_tickSessionTime;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNReplicator::GetMessageBudget( UInt32 peerId ) const
{GUCEF_TRACE;
    CDRNPeerLink* link = GetLinkToPeer( peerId );
    if ( GUCEF_NULL == link )
        return 1024;
    UInt32 budget = link->GetMaxUnfragmentedPayload();
    if ( budget > MaxMessageBudget )
        budget = MaxMessageBudget;
    if ( budget < 256 )
        budget = 256;
    return budget;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNReplicator::GetLinkBudget( UInt32 peerId ) const
{GUCEF_TRACE;
    if ( peerId < m_linkBudgets.size() )
        return m_linkBudgets[ peerId ];
    return NoBudgetLimit;
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::ConsumeLinkBudget( UInt32 peerId, UInt32 bytes )
{GUCEF_TRACE;
    if ( peerId >= m_linkBudgets.size() || NoBudgetLimit == m_linkBudgets[ peerId ] )
        return;
    UInt32& budget = m_linkBudgets[ peerId ];
    budget = bytes >= budget ? 0 : budget - bytes;
}

/*-------------------------------------------------------------------------*/

CDRNDataItem*
CDRNReplicator::GetItem( UInt32 ownerPeerId, UInt32 itemId ) const
{GUCEF_TRACE;
    if ( ownerPeerId >= m_items.size() )
        return GUCEF_NULL;
    const TItemVector& ownerItems = m_items[ ownerPeerId ];
    if ( itemId >= ownerItems.size() )
        return GUCEF_NULL;
    return ownerItems[ itemId ];
}

/*-------------------------------------------------------------------------*/

bool
CDRNReplicator::AddItem( CDRNDataItem* item )
{GUCEF_TRACE;

    UInt32 owner = item->m_ownerPeerId;
    UInt32 itemId = item->m_itemId;
    if ( 0 == owner || 0 == itemId || owner > 0xFFFFF || itemId > 0xFFFFF )
        return false;
    if ( owner >= m_items.size() )
        m_items.resize( owner + 1 );
    TItemVector& ownerItems = m_items[ owner ];
    if ( itemId >= ownerItems.size() )
        ownerItems.resize( itemId + 1, GUCEF_NULL );
    if ( GUCEF_NULL != ownerItems[ itemId ] )
        return false;
    ownerItems[ itemId ] = item;
    m_itemList.push_back( item );
    m_dueItems.reserve( m_itemList.size() );
    return true;
}

/*-------------------------------------------------------------------------*/

CDRNDataItem*
CDRNReplicator::FindItem( const CORE::CString& itemName, UInt32 ownerPeerId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    for ( UInt32 i=0; i<m_itemList.size(); ++i )
    {
        CDRNDataItem* item = m_itemList[ i ];
        if ( !item->m_removed && item->m_name == itemName && ( 0 == ownerPeerId || item->m_ownerPeerId == ownerPeerId ) )
            return item;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CDRNDataGrid*
CDRNReplicator::FindGrid( const CORE::CString& itemName, UInt32 ownerPeerId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    for ( UInt32 i=0; i<m_itemList.size(); ++i )
    {
        CDRNDataItem* item = m_itemList[ i ];
        if ( !item->m_removed && CDRNDataItem::ITEMKIND_GRID == item->m_kind && item->m_name == itemName &&
             ( 0 == ownerPeerId || item->m_ownerPeerId == ownerPeerId ) )
            return static_cast< CDRNDataGrid* >( item );
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CDRNDataStream*
CDRNReplicator::FindStream( const CORE::CString& itemName, UInt32 ownerPeerId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    for ( UInt32 i=0; i<m_itemList.size(); ++i )
    {
        CDRNDataItem* item = m_itemList[ i ];
        if ( !item->m_removed && CDRNDataItem::ITEMKIND_STREAM == item->m_kind && item->m_name == itemName &&
             ( 0 == ownerPeerId || item->m_ownerPeerId == ownerPeerId ) )
            return static_cast< CDRNDataStream* >( item );
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::GetItems( TDRNItemInfoVector& items ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    items.clear();
    for ( UInt32 i=0; i<m_itemList.size(); ++i )
    {
        const CDRNDataItem* item = m_itemList[ i ];
        if ( item->m_removed )
            continue;
        CDRNItemInfo info;
        info.ownerPeerId = item->m_ownerPeerId;
        info.itemId = item->m_itemId;
        info.name = item->m_name;
        info.kind = item->m_kind;
        items.push_back( info );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::EmitItemEvent( const CORE::CEvent& eventid, const CDRNDataItem& item )
{GUCEF_TRACE;

    CDRNItemInfoEventData eventData;
    CDRNItemInfo& info = eventData.GetData();
    info.ownerPeerId = item.m_ownerPeerId;
    info.itemId = item.m_itemId;
    info.name = item.m_name;
    info.kind = item.m_kind;
    m_session->Emit( eventid, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::SendItemReference( UInt32 toPeerId, UInt32 opcode, const CDRNDataItem& item )
{GUCEF_TRACE;

    CDRNPeerLink* link = GetLinkToPeer( toPeerId );
    if ( GUCEF_NULL == link )
        return;
    m_txBuffer.SetDataSize( 0 );
    CDRNWireWriter writer( m_txBuffer );
    writer.WriteVarUInt32( item.m_ownerPeerId );
    writer.WriteVarUInt32( item.m_itemId );
    link->Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_REPL, opcode, m_txBuffer.GetConstBufferPtr(), writer.GetOffset() );
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::SendAnnounce( UInt32 toPeerId, const CDRNDataItem& item )
{GUCEF_TRACE;

    CDRNPeerLink* link = GetLinkToPeer( toPeerId );
    if ( GUCEF_NULL == link )
        return;
    m_txBuffer.SetDataSize( 0 );
    CDRNWireWriter writer( m_txBuffer );
    writer.WriteVarUInt32( item.m_ownerPeerId );
    writer.WriteVarUInt32( item.m_itemId );
    writer.WriteString( item.m_name );
    writer.WriteUInt8( (UInt8) item.m_kind );
    item.WriteAnnounceSettings( writer );
    link->Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_REPL, DRN_REPL_ITEM_ANNOUNCE, m_txBuffer.GetConstBufferPtr(), writer.GetOffset() );
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::AnnounceToOthers( const CDRNDataItem& item, UInt32 exceptPeerId )
{GUCEF_TRACE;

    const CDRNSession::TMemberVector& members = m_session->m_members;
    for ( UInt32 i=0; i<members.size(); ++i )
    {
        const CDRNSession::CMember* member = members[ i ];
        if ( GUCEF_NULL == member || member->isLocal || member->link.IsNULL() )
            continue;
        if ( member->peerId == exceptPeerId || member->peerId == item.m_ownerPeerId )
            continue;
        SendAnnounce( member->peerId, item );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::SendItemRemoved( UInt32 toPeerId, UInt32 ownerPeerId, UInt32 itemId )
{GUCEF_TRACE;

    CDRNPeerLink* link = GetLinkToPeer( toPeerId );
    if ( GUCEF_NULL == link )
        return;
    UInt8 payload[ 16 ];
    UInt32 size = CDRNWireWriter::EncodeVarUInt64( ownerPeerId, payload );
    size += CDRNWireWriter::EncodeVarUInt64( itemId, payload + size );
    link->Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_REPL, DRN_REPL_ITEM_REMOVED, payload, size );
}

/*-------------------------------------------------------------------------*/

CDRNDataGrid*
CDRNReplicator::CreateGrid( const CDRNDataGridSettings& settings )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );

    if ( !m_session->IsActive() || 0 == m_session->m_localPeerId )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNReplicator: Grid \"" << settings.name << "\" needs an active session" );
        return GUCEF_NULL;
    }
    if ( settings.name.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNReplicator: A grid needs a name" );
        return GUCEF_NULL;
    }
    if ( DRN_AUTHORITY_OWNER != settings.authority && !IsSequencer() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNReplicator: Grid \"" << settings.name << "\" has " << CDRNDataGridSettings::AuthorityToString( settings.authority ) << " authority, only the sequencer creates such grids" );
        return GUCEF_NULL;
    }
    UInt32 localPeerId = m_session->m_localPeerId;
    if ( GUCEF_NULL != FindItem( settings.name, localPeerId ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNReplicator: This member already publishes an item named \"" << settings.name << "\"" );
        return GUCEF_NULL;
    }

    CDRNDataGrid* grid = GUCEF_NEW CDRNDataGrid( *this, settings );
    grid->m_ownerPeerId = localPeerId;
    grid->m_itemId = m_nextItemId++;
    grid->m_isOrigin = true;
    grid->m_snapshotComplete = true;
    if ( !AddItem( grid ) )
    {
        GUCEF_DELETE grid;
        return GUCEF_NULL;
    }

    if ( IsSequencer() )
        AnnounceToOthers( *grid, 0 );
    else
        SendAnnounce( m_session->m_sequencerPeerId, *grid );

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNReplicator: Publishing grid \"" << settings.name << "\" as item " << localPeerId << "." << grid->m_itemId );
    return grid;
}

/*-------------------------------------------------------------------------*/

CDRNDataStream*
CDRNReplicator::CreateStream( const CDRNDataStreamSettings& settings )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );

    if ( !m_session->IsActive() || 0 == m_session->m_localPeerId )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNReplicator: Stream \"" << settings.name << "\" needs an active session" );
        return GUCEF_NULL;
    }
    if ( settings.name.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNReplicator: A stream needs a name" );
        return GUCEF_NULL;
    }
    UInt32 localPeerId = m_session->m_localPeerId;
    if ( GUCEF_NULL != FindItem( settings.name, localPeerId ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNReplicator: This member already publishes an item named \"" << settings.name << "\"" );
        return GUCEF_NULL;
    }

    CDRNDataStream* stream = GUCEF_NEW CDRNDataStream( *this, settings );
    stream->m_ownerPeerId = localPeerId;
    stream->m_itemId = m_nextItemId++;
    stream->m_isOrigin = true;
    stream->m_snapshotComplete = true;
    stream->m_ringStarted = true;
    if ( !AddItem( stream ) )
    {
        GUCEF_DELETE stream;
        return GUCEF_NULL;
    }

    if ( IsSequencer() )
        AnnounceToOthers( *stream, 0 );
    else
        SendAnnounce( m_session->m_sequencerPeerId, *stream );

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNReplicator: Publishing stream \"" << settings.name << "\" as item " << localPeerId << "." << stream->m_itemId );
    return stream;
}

/*-------------------------------------------------------------------------*/

bool
CDRNReplicator::RemoveItem( CDRNDataItem* item )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    if ( GUCEF_NULL == item || !item->m_isOrigin || item->m_removed || GetItem( item->m_ownerPeerId, item->m_itemId ) != item )
        return false;

    item->OnRemoved();
    if ( IsSequencer() )
    {
        const CDRNSession::TMemberVector& members = m_session->m_members;
        for ( UInt32 i=0; i<members.size(); ++i )
        {
            const CDRNSession::CMember* member = members[ i ];
            if ( GUCEF_NULL != member && !member->isLocal && !member->link.IsNULL() )
                SendItemRemoved( member->peerId, item->m_ownerPeerId, item->m_itemId );
        }
    }
    else
    {
        SendItemRemoved( m_session->m_sequencerPeerId, item->m_ownerPeerId, item->m_itemId );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNReplicator::MatchesAppSubscription( const CDRNDataItem& item ) const
{GUCEF_TRACE;
    for ( UInt32 i=0; i<m_appSubscriptions.size(); ++i )
    {
        const CAppSubscription& subscription = m_appSubscriptions[ i ];
        if ( subscription.name == item.m_name && ( 0 == subscription.ownerPeerId || subscription.ownerPeerId == item.m_ownerPeerId ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNReplicator::Subscribe( const CORE::CString& itemName, UInt32 ownerPeerId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    if ( itemName.IsNULLOrEmpty() )
        return false;

    bool known = false;
    for ( UInt32 i=0; i<m_appSubscriptions.size(); ++i )
        if ( m_appSubscriptions[ i ].name == itemName && m_appSubscriptions[ i ].ownerPeerId == ownerPeerId )
            known = true;
    if ( !known )
    {
        CAppSubscription subscription;
        subscription.name = itemName;
        subscription.ownerPeerId = ownerPeerId;
        m_appSubscriptions.push_back( subscription );
    }

    for ( UInt32 i=0; i<m_itemList.size(); ++i )
    {
        CDRNDataItem* item = m_itemList[ i ];
        if ( item->m_isOrigin || item->m_removed || item->m_name != itemName || ( 0 != ownerPeerId && item->m_ownerPeerId != ownerPeerId ) )
            continue;
        item->m_appSubscribed = true;
        UpdateUpstream( *item );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNReplicator::Unsubscribe( const CORE::CString& itemName, UInt32 ownerPeerId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    bool found = false;
    for ( UInt32 i=0; i<m_appSubscriptions.size(); )
    {
        if ( m_appSubscriptions[ i ].name == itemName && m_appSubscriptions[ i ].ownerPeerId == ownerPeerId )
        {
            m_appSubscriptions.erase( m_appSubscriptions.begin() + i );
            found = true;
        }
        else
        {
            ++i;
        }
    }

    for ( UInt32 i=0; i<m_itemList.size(); ++i )
    {
        CDRNDataItem* item = m_itemList[ i ];
        if ( item->m_isOrigin || !item->m_appSubscribed || item->m_name != itemName || ( 0 != ownerPeerId && item->m_ownerPeerId != ownerPeerId ) )
            continue;
        if ( MatchesAppSubscription( *item ) )
            continue;
        item->m_appSubscribed = false;
        UpdateUpstream( *item );
        found = true;
    }
    return found;
}

/*-------------------------------------------------------------------------*/

bool
CDRNReplicator::Subscribe( CDRNDataItem& item )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    if ( item.m_isOrigin || item.m_removed )
        return false;
    item.m_appSubscribed = true;
    UpdateUpstream( item );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNReplicator::Unsubscribe( CDRNDataItem& item )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    if ( item.m_isOrigin || !item.m_appSubscribed )
        return false;
    item.m_appSubscribed = false;
    UpdateUpstream( item );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNReplicator::ChooseUpstream( UInt32 ownerPeerId ) const
{GUCEF_TRACE;
    if ( ownerPeerId == m_session->m_localPeerId )
        return 0;
    if ( IsSequencer() )
        return GUCEF_NULL != GetLinkToPeer( ownerPeerId ) ? ownerPeerId : 0;
    if ( DRN_TOPOLOGY_PEER_TO_PEER == m_session->m_topology && GUCEF_NULL != GetLinkToPeer( ownerPeerId ) )
        return ownerPeerId;
    UInt32 sequencerPeerId = m_session->m_sequencerPeerId;
    return GUCEF_NULL != GetLinkToPeer( sequencerPeerId ) ? sequencerPeerId : 0;
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::SendSubscribe( CDRNDataItem& item, UInt32 flags )
{GUCEF_TRACE;

    CDRNPeerLink* link = GetLinkToPeer( item.m_upstreamPeerId );
    if ( GUCEF_NULL == link )
    {
        item.m_subscribeSent = false;
        return;
    }
    item.OnResubscribe();
    m_txBuffer.SetDataSize( 0 );
    CDRNWireWriter writer( m_txBuffer );
    writer.WriteVarUInt32( item.m_ownerPeerId );
    writer.WriteVarUInt32( item.m_itemId );
    writer.WriteVarUInt32( flags );
    item.WriteSubscribeParams( writer );
    link->Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_REPL, DRN_REPL_SUBSCRIBE, m_txBuffer.GetConstBufferPtr(), writer.GetOffset() );
    item.m_subscribeSent = true;
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::UpdateUpstream( CDRNDataItem& item )
{GUCEF_TRACE;

    if ( item.m_isOrigin || item.m_removed )
        return;

    bool wanted = item.m_appSubscribed || item.m_subscriberCount > 0;
    if ( wanted && !item.m_subscribeSent )
    {
        UInt32 upstream = ChooseUpstream( item.m_ownerPeerId );
        if ( 0 == upstream )
            return;     // tried again every tick
        item.m_upstreamPeerId = upstream;
        SendSubscribe( item, 0 );
    }
    else
    if ( !wanted && item.m_subscribeSent )
    {
        SendItemReference( item.m_upstreamPeerId, DRN_REPL_UNSUBSCRIBE, item );
        item.m_subscribeSent = false;
        item.OnResubscribe();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::Resubscribe( CDRNDataItem& item )
{GUCEF_TRACE;

    if ( item.m_isOrigin || item.m_removed )
        return;
    if ( !item.m_subscribeSent )
    {
        UpdateUpstream( item );
        return;
    }
    SendSubscribe( item, DRN_SUBSCRIBE_FLAG_RESYNC );
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::MarkRemoved( CDRNDataItem& item )
{GUCEF_TRACE;

    if ( item.m_removed )
        return;
    item.OnRemoved();
    item.m_subscribeSent = false;
    item.Emit( CDRNDataItem::ItemRemovedEvent );
    EmitItemEvent( CDRNSession::ItemRemovedEvent, item );
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::OnSessionMessage( CDRNSession& /* session */, CDRNPeerLink& /* link */, UInt32 fromPeerId, const CDRNLinkMessage& message )
{GUCEF_TRACE;

    HandleMessage( fromPeerId, message );
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::HandleMessage( UInt32 fromPeerId, const CDRNLinkMessage& message )
{GUCEF_TRACE;
    if ( 0 == fromPeerId )
        return;
    CDRNWireReader reader( message.payload );
    UInt32 ownerPeerId = 0;
    UInt32 itemId = 0;
    if ( !reader.ReadVarUInt32( ownerPeerId ) || !reader.ReadVarUInt32( itemId ) )
        return;

    if ( DRN_MSGCLASS_REPL == message.msgClass )
    {
        if ( DRN_REPL_ITEM_ANNOUNCE == message.opcode )
        {
            HandleAnnounce( fromPeerId, ownerPeerId, itemId, reader );
            return;
        }
        if ( DRN_REPL_ITEM_REMOVED == message.opcode )
        {
            HandleItemRemoved( fromPeerId, ownerPeerId, itemId );
            return;
        }
    }

    CDRNDataItem* item = GetItem( ownerPeerId, itemId );
    if ( GUCEF_NULL == item || item->m_removed )
    {
        if ( DRN_MSGCLASS_REPL == message.msgClass && DRN_REPL_SUBSCRIBE == message.opcode && ownerPeerId == m_session->m_localPeerId )
            SendItemRemoved( fromPeerId, ownerPeerId, itemId );
        return;
    }

    if ( DRN_MSGCLASS_REPL == message.msgClass )
    {
        if ( DRN_REPL_SUBSCRIBE == message.opcode )
        {
            HandleSubscribe( fromPeerId, item, reader );
            return;
        }
        if ( DRN_REPL_UNSUBSCRIBE == message.opcode )
        {
            HandleUnsubscribe( fromPeerId, item );
            return;
        }
    }
    item->OnItemMessage( fromPeerId, message.msgClass, message.opcode, reader );
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::HandleAnnounce( UInt32 fromPeerId, UInt32 ownerPeerId, UInt32 itemId, CDRNWireReader& reader )
{GUCEF_TRACE;

    CORE::CString name;
    UInt8 kind = 0;
    if ( !reader.ReadString( name ) || !reader.ReadUInt8( kind ) )
        return;
    if ( ownerPeerId == m_session->m_localPeerId || 0 == ownerPeerId || 0 == itemId )
        return;
    // the owner announces its items, the sequencer passes announcements on
    if ( fromPeerId != ownerPeerId && fromPeerId != m_session->m_sequencerPeerId )
        return;
    if ( GUCEF_NULL != GetItem( ownerPeerId, itemId ) )
        return;

    CDRNDataItem* item = GUCEF_NULL;
    if ( CDRNDataItem::ITEMKIND_GRID == kind )
    {
        CDRNDataGridSettings settings;
        settings.name = name;
        item = GUCEF_NEW CDRNDataGrid( *this, settings );
    }
    else
    if ( CDRNDataItem::ITEMKIND_STREAM == kind )
    {
        CDRNDataStreamSettings settings;
        settings.name = name;
        item = GUCEF_NEW CDRNDataStream( *this, settings );
    }
    else
    {
        return;
    }
    item->m_ownerPeerId = ownerPeerId;
    item->m_itemId = itemId;
    item->m_isOrigin = false;
    item->ReadAnnounceSettings( reader );
    if ( !AddItem( item ) )
    {
        GUCEF_DELETE item;
        return;
    }

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNReplicator: Item \"" << name << "\" " << ownerPeerId << "." << itemId << " announced" );
    if ( IsSequencer() )
        AnnounceToOthers( *item, fromPeerId );

    EmitItemEvent( CDRNSession::ItemAnnouncedEvent, *item );

    if ( MatchesAppSubscription( *item ) )
    {
        item->m_appSubscribed = true;
        UpdateUpstream( *item );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::HandleItemRemoved( UInt32 fromPeerId, UInt32 ownerPeerId, UInt32 itemId )
{GUCEF_TRACE;

    CDRNDataItem* item = GetItem( ownerPeerId, itemId );
    if ( GUCEF_NULL == item || item->m_isOrigin )
        return;
    if ( fromPeerId != ownerPeerId && fromPeerId != m_session->m_sequencerPeerId && fromPeerId != item->m_upstreamPeerId )
        return;

    if ( IsSequencer() && !item->m_removed )
    {
        const CDRNSession::TMemberVector& members = m_session->m_members;
        for ( UInt32 i=0; i<members.size(); ++i )
        {
            const CDRNSession::CMember* member = members[ i ];
            if ( GUCEF_NULL != member && !member->isLocal && !member->link.IsNULL() && member->peerId != ownerPeerId && member->peerId != fromPeerId )
                SendItemRemoved( member->peerId, ownerPeerId, itemId );
        }
    }
    MarkRemoved( *item );
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::HandleSubscribe( UInt32 fromPeerId, CDRNDataItem* item, CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 flags = 0;
    if ( !reader.ReadVarUInt32( flags ) )
        return;

    // the publisher serves its subscribers, the sequencer serves those that reach the publisher through it
    if ( !item->m_isOrigin && !IsSequencer() )
        return;
    if ( !m_session->GetNode().GetAccessPolicy().MaySubscribeAs( fromPeerId, m_session->GetMemberAuthResult( fromPeerId ), item->m_name ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRNReplicator: Member " << fromPeerId << " may not subscribe to \"" << item->m_name << "\"" );
        return;
    }
    item->OnSubscriberAdded( fromPeerId, flags, reader );
    UpdateUpstream( *item );
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::HandleUnsubscribe( UInt32 fromPeerId, CDRNDataItem* item )
{GUCEF_TRACE;

    item->OnSubscriberRemoved( fromPeerId );
    UpdateUpstream( *item );
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::RemoveSubscriberEverywhere( UInt32 peerId )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_itemList.size(); ++i )
    {
        CDRNDataItem* item = m_itemList[ i ];
        UInt32 before = item->m_subscriberCount;
        item->OnSubscriberRemoved( peerId );
        if ( before != item->m_subscriberCount )
            UpdateUpstream( *item );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::OnSessionTick( CDRNSession& session, UInt64 nodeNow )
{GUCEF_TRACE;
    m_tickSessionTime = session.GetSessionTime();

    // the byte budget of this tick per link
    UInt32 limit = session.GetSettings().maxBytesPerTickPerLink;
    if ( 0 == limit )
        limit = NoBudgetLimit;
    UInt32 memberSlots = (UInt32) session.m_members.size();
    if ( m_linkBudgets.size() < memberSlots )
        m_linkBudgets.resize( memberSlots, limit );
    for ( UInt32 i=0; i<m_linkBudgets.size(); ++i )
        m_linkBudgets[ i ] = limit;

    // replicas whose upstream went away subscribe again, through the sequencer if need be
    m_dueItems.clear();
    for ( UInt32 i=0; i<m_itemList.size(); ++i )
    {
        CDRNDataItem* item = m_itemList[ i ];
        if ( item->m_removed )
            continue;
        if ( !item->m_isOrigin )
        {
            if ( item->m_subscribeSent && GUCEF_NULL == GetLinkToPeer( item->m_upstreamPeerId ) )
            {
                item->m_subscribeSent = false;
                item->OnResubscribe();
            }
            if ( !item->m_subscribeSent && ( item->m_appSubscribed || item->m_subscriberCount > 0 ) )
                UpdateUpstream( *item );
        }
        if ( item->IsDue( nodeNow ) )
            m_dueItems.push_back( item );
    }

    // the most starved item goes first
    for ( UInt32 i=1; i<m_dueItems.size(); ++i )
    {
        CDRNDataItem* item = m_dueItems[ i ];
        Float32 key = item->m_priorityAccumulator + item->GetPriority();
        UInt32 n = i;
        while ( n > 0 && m_dueItems[ n-1 ]->m_priorityAccumulator + m_dueItems[ n-1 ]->GetPriority() < key )
        {
            m_dueItems[ n ] = m_dueItems[ n-1 ];
            --n;
        }
        m_dueItems[ n ] = item;
    }
    for ( UInt32 i=0; i<m_dueItems.size(); ++i )
    {
        CDRNDataItem* item = m_dueItems[ i ];
        if ( item->OnSendTick( nodeNow ) )
            item->m_priorityAccumulator = 0;
        else
            item->m_priorityAccumulator += item->GetPriority();
    }

    // hand what arrived to the application, acknowledge, grant credit
    for ( UInt32 i=0; i<m_itemList.size(); ++i )
    {
        CDRNDataItem* item = m_itemList[ i ];
        if ( !item->m_removed )
            item->OnEndOfTick( nodeNow );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::OnMemberJoined( CDRNSession& /* session */, UInt32 peerId )
{GUCEF_TRACE;

    if ( !IsSequencer() )
        return;
    // the new member learns about every item there is
    for ( UInt32 i=0; i<m_itemList.size(); ++i )
    {
        const CDRNDataItem* item = m_itemList[ i ];
        if ( !item->m_removed && item->m_ownerPeerId != peerId )
            SendAnnounce( peerId, *item );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::OnMemberLeft( CDRNSession& /* session */, UInt32 peerId )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_itemList.size(); ++i )
    {
        CDRNDataItem* item = m_itemList[ i ];
        if ( item->m_ownerPeerId == peerId )
            MarkRemoved( *item );
    }
    RemoveSubscriberEverywhere( peerId );
    for ( UInt32 i=0; i<m_itemList.size(); ++i )
    {
        CDRNDataItem* item = m_itemList[ i ];
        if ( !item->m_isOrigin && !item->m_removed && item->m_subscribeSent && item->m_upstreamPeerId == peerId )
        {
            item->m_subscribeSent = false;
            item->OnResubscribe();
            UpdateUpstream( *item );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNReplicator::OnSessionClosed( CDRNSession& /* session */ )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_itemList.size(); ++i )
    {
        CDRNDataItem* item = m_itemList[ i ];
        if ( !item->m_removed )
            item->OnRemoved();
        item->m_subscribeSent = false;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
