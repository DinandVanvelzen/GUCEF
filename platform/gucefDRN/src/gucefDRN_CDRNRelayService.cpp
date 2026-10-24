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

#ifndef GUCEF_DRN_CDRNWIREWRITER_H
#include "gucefDRN_CDRNWireWriter.h"
#define GUCEF_DRN_CDRNWIREWRITER_H
#endif /* GUCEF_DRN_CDRNWIREWRITER_H ? */

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#include "gucefDRN_CDRNRelayService.h"

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

/**
 *  The peer ids of a realm: those a node asks for (its session peer id, for instance a host binding as 1) are at most
 *  MaxFixedRelayPeerId, those the relay hands out to a node that asks for none are above it: a node binding without
 *  an id never takes the id another node of the session binds with later
 */
static const UInt32 MaxFixedRelayPeerId = 0x7FFF;
static const UInt32 FirstAutomaticRelayPeerId = MaxFixedRelayPeerId + 1;
static const UInt32 MaxRelayPeerId = 0xFFFF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNRelayService::CBinding::CBinding( void )
    : realm( GUCEF_NULL )
    , peerId( 0 )
    , link( GUCEF_NULL )
    , linkPtr()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNRelayService::CRealm::CRealm( void )
    : name()
    , bindings()
    , automaticBindings()
    , bindingCount( 0 )
    , nextPeerId( FirstAutomaticRelayPeerId )
    , budget( 0 )
    , lastRefill( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNRelayService::CDRNRelayService( CDRNNode& node )
    : m_node( &node )
    , m_realms()
    , m_bindingsByLinkId()
    , m_bindingCount( 0 )
    , m_forwardedBytes( 0 )
    , m_droppedBytes( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNRelayService::~CDRNRelayService()
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_bindingsByLinkId.size(); ++i )
        GUCEF_DELETE m_bindingsByLinkId[ i ];
    m_bindingsByLinkId.clear();
    TRealmMap::iterator i = m_realms.begin();
    while ( i != m_realms.end() )
    {
        GUCEF_DELETE (*i).second;
        ++i;
    }
    m_realms.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNRelayService::GetBindingCount( void ) const
{GUCEF_TRACE;

    return m_bindingCount;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNRelayService::GetForwardedBytes( void ) const
{GUCEF_TRACE;

    return m_forwardedBytes;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNRelayService::GetDroppedBytes( void ) const
{GUCEF_TRACE;

    return m_droppedBytes;
}

/*-------------------------------------------------------------------------*/

CDRNRelayService::CBinding*
CDRNRelayService::GetBinding( const CDRNPeerLink* link ) const
{GUCEF_TRACE;
    UInt32 linkId = link->GetLinkId();
    if ( linkId >= m_bindingsByLinkId.size() )
        return GUCEF_NULL;
    CBinding* binding = m_bindingsByLinkId[ linkId ];
    return ( GUCEF_NULL != binding && binding->link == link ) ? binding : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayService::SendBindReply( CDRNPeerLink* link, UInt32 result, UInt32 peerId, const CORE::CString& realm )
{GUCEF_TRACE;

    CORE::CDynamicBuffer reply;
    CDRNWireWriter writer( reply );
    writer.WriteVarUInt32( result );
    writer.WriteVarUInt32( peerId );
    writer.WriteString( realm );
    link->Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_RELAY, DRN_RELAY_BIND, reply.GetConstBufferPtr(), writer.GetOffset() );
}

/*-------------------------------------------------------------------------*/

bool
CDRNRelayService::OnLinkMessage( CDRNPeerLink* link, const CDRNLinkMessage& message )
{GUCEF_TRACE;
    if ( DRN_MSGCLASS_RELAY != message.msgClass )
        return false;
    CDRNWireReader reader( message.payload );
    switch ( message.opcode )
    {
        case DRN_RELAY_BIND:    OnBind( link, reader ); return true;
        case DRN_RELAY_FORWARD: OnForward( link, message, reader ); return true;
        case DRN_RELAY_UNBIND:  Unbind( link ); return true;
        default: return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayService::OnBind( CDRNPeerLink* link, CDRNWireReader& reader )
{GUCEF_TRACE;

    CORE::CString realmName;
    UInt32 requestedId = 0;
    CORE::CString token;
    if ( !reader.ReadString( realmName ) || !reader.ReadVarUInt32( requestedId ) || !reader.ReadString( token ) )
        return;

    const CORE::CString& expectedToken = m_node->GetSettings().relayToken;
    bool tokenOk = expectedToken.IsNULLOrEmpty() ||
                   CDRNCrypto::ConstantTimeEqualsHashed( token.C_String(), token.ByteSize(), expectedToken.C_String(), expectedToken.ByteSize() );
    if ( link->IsInitiator() || !tokenOk || requestedId > MaxFixedRelayPeerId )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRNRelayService: Refused a binding in realm \"" << realmName << "\"" );
        SendBindReply( link, DRN_RELAY_BIND_REFUSED, 0, realmName );
        return;
    }

    // a link binds once, binding again replaces it
    Unbind( link );

    CRealm* realm = GUCEF_NULL;
    TRealmMap::iterator r = m_realms.find( realmName );
    if ( r == m_realms.end() )
    {
        realm = GUCEF_NEW CRealm();
        realm->name = realmName;
        realm->lastRefill = CDRNClock::NowInMicroseconds();
        realm->budget = m_node->GetSettings().relayMaxBytesPerSecondPerRealm;
        m_realms[ realmName ] = realm;
    }
    else
    {
        realm = (*r).second;
    }

    UInt32 peerId = requestedId;
    if ( 0 == peerId )
    {
        // the next free id of the automatic range
        for ( UInt32 n=FirstAutomaticRelayPeerId; n<=MaxRelayPeerId && 0 == peerId; ++n )
        {
            UInt32 candidate = realm->nextPeerId;
            realm->nextPeerId = candidate >= MaxRelayPeerId ? FirstAutomaticRelayPeerId : candidate + 1;
            if ( GUCEF_NULL == FindBinding( *realm, candidate ) )
                peerId = candidate;
        }
    }
    else
    if ( GUCEF_NULL != FindBinding( *realm, peerId ) )
    {
        // A binding whose link is gone without a close, a peer whose process ended for instance, would hold the
        // id until its link times out: a new bind takes the id over once that link is stale
        CBinding* existing = FindBinding( *realm, peerId );
        if ( !IsBindingStale( *existing ) )
        {
            SendBindReply( link, DRN_RELAY_BIND_ID_TAKEN, 0, realmName );
            return;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRNRelayService: Peer id " << peerId << " in realm \"" << realmName << "\" is taken over by " <<
            link->GetRemoteEndpoint().ToString() << ", the link of its binding is gone" );
        CDRNPeerLinkPtr staleLink = existing->linkPtr;
        Unbind( existing->link );
        if ( !staleLink.IsNULL() && !staleLink->IsClosed() )
            staleLink->Close( DRN_LINKREASON_LINK_TIMEOUT, "the binding was taken over" );
    }
    if ( 0 == peerId )
    {
        SendBindReply( link, DRN_RELAY_BIND_REFUSED, 0, realmName );
        return;
    }

    CBinding* binding = GUCEF_NEW CBinding();
    binding->realm = realm;
    binding->peerId = peerId;
    binding->link = link;
    binding->linkPtr = link->GetSharedPtr();
    SetBinding( *realm, peerId, binding );
    ++realm->bindingCount;
    UInt32 linkId = link->GetLinkId();
    if ( linkId >= m_bindingsByLinkId.size() )
        m_bindingsByLinkId.resize( linkId + 1, GUCEF_NULL );
    m_bindingsByLinkId[ linkId ] = binding;
    ++m_bindingCount;

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNRelayService: Bound " << link->GetRemoteEndpoint().ToString() << " as " << peerId << " in realm \"" << realmName << "\"" );
    SendBindReply( link, DRN_RELAY_BIND_OK, peerId, realmName );
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayService::OnForward( CDRNPeerLink* link, const CDRNLinkMessage& message, CDRNWireReader& reader )
{GUCEF_TRACE;
    CBinding* from = GetBinding( link );
    UInt32 toPeerId = 0;
    if ( GUCEF_NULL == from || !reader.ReadVarUInt32( toPeerId ) )
        return;
    const UInt8* packet = reader.GetCurrentPtr();
    UInt32 packetSize = reader.GetRemaining();

    CRealm* realm = from->realm;
    CBinding* to = FindBinding( *realm, toPeerId );
    if ( GUCEF_NULL == to || to == from )
    {
        m_droppedBytes += packetSize;
        return;
    }
    UInt32 limit = m_node->GetSettings().relayMaxBytesPerSecondPerRealm;
    if ( 0 != limit )
    {
        if ( realm->budget < (Int64) packetSize )
        {
            m_droppedBytes += packetSize;
            return;
        }
        realm->budget -= packetSize;
    }

    UInt32 headerSize = CDRNWireWriter::EncodeVarUInt64( from->peerId, m_header );
    if ( to->link->SendGather( message.lane, DRN_MSGCLASS_RELAY, DRN_RELAY_FORWARD, m_header, headerSize, packet, packetSize ) )
        m_forwardedBytes += packetSize;
    else
        m_droppedBytes += packetSize;
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayService::Unbind( CDRNPeerLink* link )
{GUCEF_TRACE;

    CBinding* binding = GetBinding( link );
    if ( GUCEF_NULL == binding )
        return;
    m_bindingsByLinkId[ link->GetLinkId() ] = GUCEF_NULL;
    --m_bindingCount;
    CRealm* realm = binding->realm;
    if ( FindBinding( *realm, binding->peerId ) == binding )
    {
        SetBinding( *realm, binding->peerId, GUCEF_NULL );
        --realm->bindingCount;
    }
    GUCEF_DELETE binding;
}

/*-------------------------------------------------------------------------*/

CDRNRelayService::CBinding*
CDRNRelayService::FindBinding( const CRealm& realm, UInt32 peerId )
{GUCEF_TRACE;

    if ( peerId >= FirstAutomaticRelayPeerId )
    {
        UInt32 index = peerId - FirstAutomaticRelayPeerId;
        return index < realm.automaticBindings.size() ? realm.automaticBindings[ index ] : GUCEF_NULL;
    }
    return peerId < realm.bindings.size() ? realm.bindings[ peerId ] : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayService::SetBinding( CRealm& realm, UInt32 peerId, CBinding* binding )
{GUCEF_TRACE;

    TBindingVector& bindings = peerId >= FirstAutomaticRelayPeerId ? realm.automaticBindings : realm.bindings;
    UInt32 index = peerId >= FirstAutomaticRelayPeerId ? peerId - FirstAutomaticRelayPeerId : peerId;
    if ( index >= bindings.size() )
    {
        if ( GUCEF_NULL == binding )
            return;
        bindings.resize( index + 1, GUCEF_NULL );
    }
    bindings[ index ] = binding;
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayService::OnLinkClosed( CDRNPeerLink* link )
{GUCEF_TRACE;

    Unbind( link );
}

/*-------------------------------------------------------------------------*/

bool
CDRNRelayService::IsBindingStale( const CBinding& binding ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == binding.link || binding.link->IsClosed() || !binding.link->IsOperational() )
        return true;

    // An alive peer answers the echoes of the relay, four echo intervals without a word and at least a second
    UInt64 staleAfter = (UInt64) m_node->GetSettings().echoIntervalMs * 4000;
    if ( staleAfter < 1000000 )
        staleAfter = 1000000;
    UInt64 now = CDRNClock::NowInMicroseconds();
    UInt64 lastReceived = binding.link->GetLastReceiveTime();
    return now > lastReceived && now - lastReceived > staleAfter;
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayService::Tick( UInt64 nodeNow )
{GUCEF_TRACE;
    UInt32 limit = m_node->GetSettings().relayMaxBytesPerSecondPerRealm;
    if ( 0 == limit )
        return;
    TRealmMap::iterator i = m_realms.begin();
    while ( i != m_realms.end() )
    {
        CRealm* realm = (*i).second;
        UInt64 elapsed = nodeNow > realm->lastRefill ? nodeNow - realm->lastRefill : 0;
        Int64 refill = (Int64) ( (UInt64) limit * elapsed / 1000000 );
        if ( refill > 0 )
        {
            realm->budget += refill;
            if ( realm->budget > (Int64) limit )
                realm->budget = limit;
            realm->lastRefill = nodeNow;
        }
        ++i;
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
