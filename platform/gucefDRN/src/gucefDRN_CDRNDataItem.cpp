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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRN_CDRNREPLICATOR_H
#include "gucefDRN_CDRNReplicator.h"
#define GUCEF_DRN_CDRNREPLICATOR_H
#endif /* GUCEF_DRN_CDRNREPLICATOR_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#include "gucefDRN_CDRNDataItem.h"

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

const CORE::CEvent CDRNDataItem::SnapshotCompleteEvent = "GUCEF::DRN::CDRNDataItem::SnapshotCompleteEvent";
const CORE::CEvent CDRNDataItem::ItemRemovedEvent = "GUCEF::DRN::CDRNDataItem::ItemRemovedEvent";

/*-------------------------------------------------------------------------*/

void
CDRNDataItem::RegisterEvents( void )
{GUCEF_TRACE;

    SnapshotCompleteEvent.Initialize();
    ItemRemovedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CDRNItemInfo::CDRNItemInfo( void )
    : ownerPeerId( 0 )
    , itemId( 0 )
    , name()
    , kind( CDRNDataItem::ITEMKIND_GRID )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNDataItem::CDRNDataItem( CDRNReplicator& replicator, TItemKind kind )
    : CDRNApplicationNotifier( replicator.GetSession().GetNode() )
    , CORE::CIDataNodeSerializable()
    , m_replicator( &replicator )
    , m_kind( kind )
    , m_name()
    , m_ownerPeerId( 0 )
    , m_itemId( 0 )
    , m_isOrigin( false )
    , m_isRelay( false )
    , m_appSubscribed( false )
    , m_subscribeSent( false )
    , m_upstreamPeerId( 0 )
    , m_snapshotComplete( false )
    , m_removed( false )
    , m_priorityAccumulator( 0 )
    , m_subscriberCount( 0 )
    , m_protectionClass( DRN_PROTECTION_CONFIDENTIAL )
    , m_txBuffer()
    , m_txWriter( m_txBuffer )
{GUCEF_TRACE;

    RegisterEvents();
    for ( UInt32 i=0; i<DRN_LANE_COUNT; ++i )
        m_laneKinds[ i ] = DRN_PACKETKIND_AEAD;
}

/*-------------------------------------------------------------------------*/

TDRNProtectionClass
CDRNDataItem::GetProtectionClass( void ) const
{GUCEF_TRACE;

    return m_protectionClass;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataItem::SetProtectionClass( TDRNProtectionClass protectionClass )
{GUCEF_TRACE;

    // the links read the node's policy; the items do the same, so that both agree on every member
    const CDRNSecurityPolicy& policy = m_replicator->GetSession().GetNode().GetSettings().securityPolicy;
    if ( DRN_PROTECTION_PUBLIC == protectionClass && !policy.allowPublicItems )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNDataItem(" << m_name << "): Declared public, but the security policy does not allow public items: it is sealed" );
        protectionClass = DRN_PROTECTION_CONFIDENTIAL;
    }
    m_protectionClass = protectionClass;
    for ( UInt32 i=0; i<DRN_LANE_COUNT; ++i )
        m_laneKinds[ i ] = (UInt8) policy.ResolvePacketKind( protectionClass, (UInt8) i );
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataItem::IsPublicDataMessage( UInt8 msgClass, UInt32 opcode )
{GUCEF_TRACE;

    if ( DRN_MSGCLASS_STREAM == msgClass )
        return DRN_STREAM_ENTRIES == opcode;
    if ( DRN_MSGCLASS_REPL == msgClass )
        return DRN_REPL_SNAPSHOT_BEGIN == opcode || DRN_REPL_SNAPSHOT_END == opcode || DRN_REPL_KEY_DEFINE == opcode || DRN_REPL_GRID_DELTA == opcode;
    return false;
}

/*-------------------------------------------------------------------------*/

CDRNDataItem::~CDRNDataItem()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNDataItem::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

CDRNDataItem::TItemKind
CDRNDataItem::GetKind( void ) const
{GUCEF_TRACE;

    return m_kind;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDataItem::GetOwnerPeerId( void ) const
{GUCEF_TRACE;

    return m_ownerPeerId;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDataItem::GetItemId( void ) const
{GUCEF_TRACE;

    return m_itemId;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataItem::IsOrigin( void ) const
{GUCEF_TRACE;

    return m_isOrigin;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataItem::IsSnapshotComplete( void ) const
{GUCEF_TRACE;

    return m_isOrigin || m_snapshotComplete;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataItem::IsRemoved( void ) const
{GUCEF_TRACE;

    return m_removed;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDataItem::GetSubscriberCount( void ) const
{GUCEF_TRACE;

    return m_subscriberCount;
}

/*-------------------------------------------------------------------------*/

CDRNSession&
CDRNDataItem::GetSession( void ) const
{GUCEF_TRACE;

    return m_replicator->GetSession();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CDRNDataItem::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNDataItem::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNDataItem";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CDRNReplicator&
CDRNDataItem::GetReplicator( void ) const
{GUCEF_TRACE;

    return *m_replicator;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataItem::WriteSubscribeParams( CDRNWireWriter& /* writer */ )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDRNDataItem::OnResubscribe( void )
{GUCEF_TRACE;

    m_snapshotComplete = false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataItem::IsExpectedPublicData( UInt8 /* lane */, UInt8 /* msgClass */, UInt32 /* opcode */, const CDRNWireReader& /* body */ ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataItem::OnRemoved( void )
{GUCEF_TRACE;

    m_removed = true;
}

/*-------------------------------------------------------------------------*/

void
CDRNDataItem::Emit( const CORE::CEvent& eventid, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    EmitToApplication( eventid, eventData );
}

/*-------------------------------------------------------------------------*/

CDRNWireWriter&
CDRNDataItem::BeginMessage( void )
{GUCEF_TRACE;
    m_txWriter.SetOffset( 0 );
    m_txBuffer.SetDataSize( 0 );
    m_txWriter.WriteVarUInt32( m_ownerPeerId );
    m_txWriter.WriteVarUInt32( m_itemId );
    return m_txWriter;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataItem::SendMessage( UInt32 peerId, UInt8 lane, UInt8 msgClass, UInt32 opcode )
{GUCEF_TRACE;
    CDRNPeerLink* link = m_replicator->GetLinkToPeer( peerId );
    if ( GUCEF_NULL == link )
        return false;
    UInt8 packetKind = DRN_PROTECTION_PUBLIC == m_protectionClass && lane < DRN_LANE_COUNT && IsPublicDataMessage( msgClass, opcode ) ? m_laneKinds[ lane ] : (UInt8) DRN_PACKETKIND_AEAD;
    return link->Send( lane, msgClass, opcode, m_txBuffer.GetConstBufferPtr(), m_txWriter.GetOffset(), packetKind );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
