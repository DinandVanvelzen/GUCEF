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

#ifndef GUCEF_COMCORE_RPCTYPES_H
#include "gucefCOMCORE_RpcTypes.h"
#define GUCEF_COMCORE_RPCTYPES_H
#endif /* GUCEF_COMCORE_RPCTYPES_H ? */

#ifndef GUCEF_DRN_CDRNWIREREADER_H
#include "gucefDRN_CDRNWireReader.h"
#define GUCEF_DRN_CDRNWIREREADER_H
#endif /* GUCEF_DRN_CDRNWIREREADER_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNRPCTRANSPORT_H
#include "gucefDRN_CDRNRpcTransport.h"
#define GUCEF_DRN_CDRNRPCTRANSPORT_H
#endif /* GUCEF_DRN_CDRNRPCTRANSPORT_H ? */

#ifndef GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H
#include "gucefDRN_CDRNRpcTransportListener.h"
#define GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H
#endif /* GUCEF_DRN_CDRNRPCTRANSPORTLISTENER_H ? */

#include "gucefDRN_CDRNRpcSessionComponent.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The RPC component of each session that has one; the lock is a leaf, taken under the node lock */
typedef GUCEF::map< const CDRNSession*, CDRNRpcSessionComponent* >   TRpcComponentMap;
static MT::CMutex g_rpcComponentLock;
static TRpcComponentMap g_rpcComponents;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNRpcSessionComponent*
CDRNRpcSessionComponent::Obtain( CDRNSession& session )
{GUCEF_TRACE;

    MT::CScopeMutex nodeLock( session.GetNode().GetLock() );
    {
        MT::CScopeMutex lock( g_rpcComponentLock );
        TRpcComponentMap::iterator i = g_rpcComponents.find( &session );
        if ( i != g_rpcComponents.end() )
            return (*i).second;
    }

    CDRNRpcSessionComponent* component = GUCEF_NEW CDRNRpcSessionComponent( session );
    if ( !session.RegisterComponent( component ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNRpcSessionComponent: Session \"" << session.GetName() << "\" has another component for the RPC message class" );
        GUCEF_DELETE component;
        return GUCEF_NULL;
    }
    MT::CScopeMutex lock( g_rpcComponentLock );
    g_rpcComponents[ &session ] = component;
    return component;
}

/*-------------------------------------------------------------------------*/

CDRNRpcSessionComponent::CDRNRpcSessionComponent( CDRNSession& session )
    : CIDRNSessionComponent()
    , m_session( &session )
    , m_node( &session.GetNode() )
    , m_channels()
    , m_nextChannel()
    , m_listeners()
    , m_deliveryDepth( 0 )
    , m_linksToFlush()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNRpcSessionComponent::~CDRNRpcSessionComponent()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNSession&
CDRNRpcSessionComponent::GetSession( void ) const
{GUCEF_TRACE;

    return *m_session;
}

/*-------------------------------------------------------------------------*/

CDRNNode&
CDRNRpcSessionComponent::GetNode( void ) const
{GUCEF_TRACE;

    return *m_node;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNRpcSessionComponent::ChannelKey( UInt32 peerId, UInt32 channel, bool openedLocally )
{GUCEF_TRACE;

    return ( ( (UInt64) peerId ) << 32 ) | ( ( (UInt64) ( channel & 0x7FFFFFFF ) ) << 1 ) | ( openedLocally ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

UInt8
CDRNRpcSessionComponent::GetMessageClass( void ) const
{GUCEF_TRACE;

    return DRN_MSGCLASS_RPC;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNRpcSessionComponent::OpenChannel( UInt32 peerId, CDRNRpcTransport* transport )
{GUCEF_TRACE;

    UInt32& next = m_nextChannel[ peerId ];
    ++next;
    if ( 0 == next || next > 0x7FFFFFFF )
        next = 1;
    m_channels[ ChannelKey( peerId, next, true ) ] = transport;
    return next;
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcSessionComponent::AddAcceptedChannel( UInt32 peerId, UInt32 channel, CDRNRpcTransport* transport )
{GUCEF_TRACE;

    m_channels[ ChannelKey( peerId, channel, false ) ] = transport;
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcSessionComponent::RemoveTransport( CDRNRpcTransport* transport )
{GUCEF_TRACE;

    TChannelMap::iterator i = m_channels.begin();
    while ( i != m_channels.end() )
    {
        if ( (*i).second == transport )
        {
            m_channels.erase( i );
            return;
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcSessionComponent::AddListener( CDRNRpcTransportListener* listener )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_listeners.size(); ++i )
        if ( m_listeners[ i ] == listener )
            return;
    m_listeners.push_back( listener );
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcSessionComponent::RemoveListener( CDRNRpcTransportListener* listener )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_listeners.size(); ++i )
    {
        if ( m_listeners[ i ] == listener )
        {
            m_listeners.erase( m_listeners.begin() + i );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNRpcSessionComponent::GetAcceptedChannelCount( UInt32 peerId ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    TChannelMap::const_iterator i = m_channels.begin();
    while ( i != m_channels.end() )
    {
        UInt64 key = (*i).first;
        if ( (UInt32) ( key >> 32 ) == peerId && 0 == ( key & 1 ) )
            ++count;
        ++i;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcSessionComponent::IsDelivering( void ) const
{GUCEF_TRACE;

    return m_deliveryDepth > 0;
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcSessionComponent::FlushAfterDelivery( CDRNPeerLink& link )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_linksToFlush.size(); ++i )
        if ( m_linksToFlush[ i ] == &link )
            return;
    m_linksToFlush.push_back( &link );
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcSessionComponent::SendClose( CDRNPeerLink& link, UInt32 channel, bool fromAcceptor )
{GUCEF_TRACE;

    static const UInt8 noPayload = 0;
    link.Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_RPC, DRNRpcOpcode( channel, fromAcceptor ), &noPayload, 0 );
    FlushAfterDelivery( link );
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcSessionComponent::OnSessionMessage( CDRNSession& /* session */, CDRNPeerLink& link, UInt32 fromPeerId, const CDRNLinkMessage& message )
{GUCEF_TRACE;

    ++m_deliveryDepth;
    if ( DRN_RPC_OPCODE_OPEN == message.opcode )
    {
        // the member opens a channel: a listener takes it now, its first frame follows
        CDRNWireReader reader( message.payload );
        UInt32 opened = 0;
        if ( reader.ReadVarUInt32( opened ) && 0 != opened && opened <= 0x7FFFFFFF &&
             m_channels.find( ChannelKey( fromPeerId, opened, false ) ) == m_channels.end() )
            Accept( link, fromPeerId, opened );
        EndDelivery();
        return;
    }
    UInt32 channel = DRNRpcChannel( message.opcode );
    if ( 0 == channel )
    {
        EndDelivery();
        return;
    }

    // the bit says who opened the channel: a frame of the acceptor belongs to a channel this side opened
    bool openedLocally = DRNRpcIsFromAcceptor( message.opcode );
    bool hasFrame = message.payload.GetDataSize() > 0;

    TChannelMap::iterator i = m_channels.find( ChannelKey( fromPeerId, channel, openedLocally ) );
    if ( i != m_channels.end() )
    {
        CDRNRpcTransport* transport = (*i).second;
        if ( !hasFrame )
            m_channels.erase( i );
        transport->OnFrame( link, message );
    }
    else
    if ( hasFrame )
    {
        // a channel that is not open here (closed already, or refused): the peer hears so
        SendClose( link, channel, !openedLocally );
    }
    EndDelivery();
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcSessionComponent::EndDelivery( void )
{GUCEF_TRACE;

    --m_deliveryDepth;

    // what the transports sent while frames were delivered leaves now, in one packet where it fits
    if ( 0 == m_deliveryDepth && !m_linksToFlush.empty() )
    {
        TLinkVector links;
        links.swap( m_linksToFlush );
        for ( UInt32 n=0; n<links.size(); ++n )
            links[ n ]->Flush();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcSessionComponent::Accept( CDRNPeerLink& link, UInt32 fromPeerId, UInt32 channel )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_listeners.size(); ++i )
    {
        if ( m_listeners[ i ]->OfferChannel( *this, link.GetSharedPtr(), fromPeerId, channel ) )
            return;
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNRpcSessionComponent: Member " << fromPeerId << " opened RPC channel " << channel <<
                     " of session \"" << m_session->GetName() << "\" and nothing takes it, closing it" );
    SendClose( link, channel, true );
}

/*-------------------------------------------------------------------------*/

bool
CDRNRpcSessionComponent::MayAcceptPacketKind( const CDRNLinkMessage& message ) const
{GUCEF_TRACE;

    // the link takes MAC packets only where the policy allows public items; the RPC session judges the frame's method
    return DRN_PACKETKIND_MAC == message.packetKind;
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcSessionComponent::OnSessionTick( CDRNSession& /* session */, UInt64 /* nodeNow */ )
{GUCEF_TRACE;

    // a direct link between two members may go down while both stay in the session (peer to peer)
    TChannelMap::iterator i = m_channels.begin();
    while ( i != m_channels.end() )
    {
        CDRNRpcTransport* transport = (*i).second;
        if ( transport->m_link.IsNULL() || !transport->m_link->IsClosed() )
        {
            ++i;
            continue;
        }
        TDRNLinkReason reason = transport->m_link->GetCloseReason();
        Int32 code = ( DRN_LINKREASON_BANNED == reason || DRN_LINKREASON_POLICY == reason || DRN_LINKREASON_KEY_MISMATCH == reason ) ?
                        COMCORE::RPC_STATUS_PERMISSION_DENIED : COMCORE::RPC_STATUS_UNAVAILABLE;
        m_channels.erase( i );
        transport->OnLinkGone( code, "the link with the member closed" );
        i = m_channels.begin();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcSessionComponent::DisconnectAll( UInt32 peerId, bool allPeers, Int32 statusCode, const char* why )
{GUCEF_TRACE;

    // one at a time, the map looked at again after each: what a DisconnectedEvent sets off may let go of other
    // transports, which then leave the map themselves
    TChannelMap::iterator i = m_channels.begin();
    while ( i != m_channels.end() )
    {
        if ( allPeers || (UInt32) ( (*i).first >> 32 ) == peerId )
        {
            CDRNRpcTransport* transport = (*i).second;
            m_channels.erase( i );
            transport->OnLinkGone( statusCode, why );
            i = m_channels.begin();
        }
        else
            ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcSessionComponent::OnMemberLeft( CDRNSession& /* session */, UInt32 peerId )
{GUCEF_TRACE;

    DisconnectAll( peerId, false, COMCORE::RPC_STATUS_UNAVAILABLE, "the member left the session" );
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcSessionComponent::OnSessionClosed( CDRNSession& /* session */ )
{GUCEF_TRACE;

    DisconnectAll( 0, true, COMCORE::RPC_STATUS_UNAVAILABLE, "the session closed" );
    m_nextChannel.clear();
}

/*-------------------------------------------------------------------------*/

void
CDRNRpcSessionComponent::OnSessionDestroyed( CDRNSession& session )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( g_rpcComponentLock );
        g_rpcComponents.erase( &session );
    }

    DisconnectAll( 0, true, COMCORE::RPC_STATUS_UNAVAILABLE, "the session is gone" );
    TListenerVector listeners;
    listeners.swap( m_listeners );
    for ( UInt32 i=0; i<listeners.size(); ++i )
        listeners[ i ]->OnComponentGone();

    // the session forgot this component before it called: nothing refers to it any more
    GUCEF_DELETE this;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
