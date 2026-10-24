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

#ifndef GUCEF_DRN_CDRNRELAYSERVICE_H
#include "gucefDRN_CDRNRelayService.h"
#define GUCEF_DRN_CDRNRELAYSERVICE_H
#endif /* GUCEF_DRN_CDRNRELAYSERVICE_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#include "gucefDRN_CDRNRelayChannel.h"

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

/** Room for the FORWARD header in a message on the link to the relay */
static const UInt32 ForwardHeaderRoom = 8;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Splits the path of a relay endpoint, /realm/peerId */
static bool
ParseRelayPath( const CORE::CString& path, CORE::CString& realm, UInt32& peerId )
{GUCEF_TRACE;

    if ( path.Length() < 2 || '/' != path[ 0 ] )
        return false;
    CORE::CString rest = path.SubstrFromRange( 1, path.Length() );
    Int32 slash = rest.HasChar( '/', false );
    if ( slash <= 0 )
        return false;
    realm = rest.SubstrToIndex( (UInt32) slash, true );
    peerId = CORE::StringToUInt32( rest.SubstrFromRange( (UInt32) slash + 1, rest.Length() ), 0 );
    return 0 != peerId;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNRelayChannel::CDRNRelayChannel( CDRNRelayClient& client )
    : CIDRNChannel()
    , m_client( &client )
    , m_binding( GUCEF_NULL )
    , m_capabilities()
    , m_pulseGenerator()
    , m_remote()
    , m_remotePeerId( 0 )
    , m_connected( false )
    , m_announceConnected( false )
    , m_announceDisconnected( false )
    , m_rxView()
    , m_rxEventData( &m_rxView )
{GUCEF_TRACE;

    m_capabilities.isReliable = false;
    m_capabilities.isDatagram = true;
    m_capabilities.maxPacketSize = DRN_DEFAULT_DATAGRAM_SIZE;
    m_capabilities.isBrowserCompatible = false;
    m_capabilities.canListen = false;
}

/*-------------------------------------------------------------------------*/

CDRNRelayChannel::~CDRNRelayChannel()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_client )
        m_client->Detach( *this );
}

/*-------------------------------------------------------------------------*/

bool
CDRNRelayChannel::Connect( const CDRNEndpoint& remote )
{GUCEF_TRACE;

    CORE::CString realm;
    UInt32 peerId = 0;
    if ( GUCEF_NULL == m_client || remote.GetChannelType() != CDRNEndpoint::ChannelTypeRelay ||
         0 == remote.GetPort() || !ParseRelayPath( remote.GetPath(), realm, peerId ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNRelayChannel: Not a relay endpoint: " << remote.ToString() );
        return false;
    }
    CORE::CString via = remote.GetParams().GetValueAlways( "via", CORE::CVariant( CDRNEndpoint::ChannelTypeUdp ) ).AsString( CDRNEndpoint::ChannelTypeUdp, true );
    CORE::CString relayPolicy = remote.GetParams().GetValueAlways( "relayPolicy" ).AsString( CORE::CString::Empty, true );
    CORE::CString relayKey = remote.GetParams().GetValueAlways( "relayKey" ).AsString( CORE::CString::Empty, true );

    m_remote = remote;
    m_remotePeerId = peerId;
    CDRNRelayBinding* binding = m_client->FindBinding( remote.GetHost(), remote.GetPort(), via, realm );
    if ( GUCEF_NULL == binding || CDRNRelayBinding::BINDINGSTATE_FAILED == binding->state )
        binding = m_client->AddBinding( remote.GetHost(), remote.GetPort(), via, realm, 0, m_client->m_node->GetSettings().relayToken, false, relayPolicy, relayKey );
    if ( GUCEF_NULL == binding )
        return false;
    m_client->AttachChannel( *binding, *this );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayChannel::Close( void )
{GUCEF_TRACE;

    m_connected = false;
    m_announceConnected = false;
    if ( GUCEF_NULL != m_client )
        m_client->Detach( *this );
}

/*-------------------------------------------------------------------------*/

bool
CDRNRelayChannel::IsConnected( void ) const
{GUCEF_TRACE;

    return m_connected;
}

/*-------------------------------------------------------------------------*/

bool
CDRNRelayChannel::SendPacket( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( !m_connected || GUCEF_NULL == m_binding || GUCEF_NULL == m_client || size > m_capabilities.maxPacketSize )
        return false;
    return m_client->Forward( *m_binding, m_remotePeerId, data, size );
}

/*-------------------------------------------------------------------------*/

const CDRNChannelCapabilities&
CDRNRelayChannel::GetCapabilities( void ) const
{GUCEF_TRACE;

    return m_capabilities;
}

/*-------------------------------------------------------------------------*/

CDRNEndpoint
CDRNRelayChannel::GetRemoteEndpoint( void ) const
{GUCEF_TRACE;

    return m_remote;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNRelayChannel::GetChannelType( void ) const
{GUCEF_TRACE;

    return CDRNEndpoint::ChannelTypeRelay;
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayChannel::SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    m_pulseGenerator = pulseGenerator;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CDRNRelayChannel::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNRelayChannel::GetRemotePeerId( void ) const
{GUCEF_TRACE;

    return m_remotePeerId;
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayChannel::OnBindingReady( UInt32 maxPacketSize )
{GUCEF_TRACE;

    m_capabilities.maxPacketSize = maxPacketSize;
    if ( !m_connected )
    {
        m_connected = true;
        m_announceConnected = true;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayChannel::OnBindingGone( void )
{GUCEF_TRACE;

    m_binding = GUCEF_NULL;
    m_announceConnected = false;
    m_announceDisconnected = true;
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayChannel::OnPacket( const UInt8* data, UInt32 size )
{GUCEF_TRACE;
    if ( !m_connected )
        return;
    m_rxView.LinkTo( data, size );
    NotifyObservers( PacketReceivedEvent, &m_rxEventData );
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayChannel::Tick( void )
{GUCEF_TRACE;
    if ( m_announceConnected )
    {
        m_announceConnected = false;
        if ( !NotifyObservers( ChannelConnectedEvent ) )
            return;
    }
    if ( m_announceDisconnected )
    {
        m_announceDisconnected = false;
        m_connected = false;
        NotifyObservers( ChannelDisconnectedEvent );
    }
}

/*-------------------------------------------------------------------------*/

CDRNRelayBinding::CDRNRelayBinding( void )
    : relayHost()
    , relayPort( 0 )
    , via()
    , realm()
    , token()
    , relayPolicy()
    , relayKey()
    , requestedPeerId( 0 )
    , boundPeerId( 0 )
    , advertise( false )
    , state( BINDINGSTATE_CONNECTING )
    , failure()
    , connector()
    , link()
    , maxPacketSize( DRN_DEFAULT_DATAGRAM_SIZE )
    , m_channelsByPeerId()
    , m_waiting()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNEndpoint
CDRNRelayBinding::GetEndpoint( void ) const
{GUCEF_TRACE;

    CDRNEndpoint endpoint( "relay://" + relayHost + ":" + CORE::UInt32ToString( relayPort ) + "/" + realm + "/" +
                           CORE::UInt32ToString( boundPeerId ) + "?via=" + via );
    if ( !relayPolicy.IsNULLOrEmpty() )
        endpoint.GetParams().Set( "relayPolicy", CORE::CVariant( relayPolicy ) );
    if ( !relayKey.IsNULLOrEmpty() )
        endpoint.GetParams().Set( "relayKey", CORE::CVariant( relayKey ) );
    return endpoint;
}

/*-------------------------------------------------------------------------*/

CDRNRelayClient::CDRNRelayClient( CDRNNode& node )
    : m_node( &node )
    , m_bindings()
    , m_channels()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNRelayClient::~CDRNRelayClient()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayClient::Shutdown( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_channels.size(); ++i )
    {
        m_channels[ i ]->m_binding = GUCEF_NULL;
        m_channels[ i ]->m_client = GUCEF_NULL;
        m_channels[ i ]->m_connected = false;
    }
    m_channels.clear();
    for ( UInt32 i=0; i<m_bindings.size(); ++i )
    {
        CDRNRelayBinding* binding = m_bindings[ i ];
        if ( !binding->connector.IsNULL() )
            binding->connector->Cancel();
        GUCEF_DELETE binding;
    }
    m_bindings.clear();
}

/*-------------------------------------------------------------------------*/

CDRNRelayBinding*
CDRNRelayClient::FindBinding( const CORE::CString& host, UInt16 port, const CORE::CString& via, const CORE::CString& realm ) const
{GUCEF_TRACE;
    for ( UInt32 i=0; i<m_bindings.size(); ++i )
    {
        CDRNRelayBinding* binding = m_bindings[ i ];
        if ( binding->relayPort == port && binding->relayHost == host && binding->via == via && binding->realm == realm &&
             CDRNRelayBinding::BINDINGSTATE_FAILED != binding->state )
            return binding;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CDRNRelayBinding*
CDRNRelayClient::FindBindingByLink( const CDRNPeerLink* link ) const
{GUCEF_TRACE;
    for ( UInt32 i=0; i<m_bindings.size(); ++i )
    {
        CDRNRelayBinding* binding = m_bindings[ i ];
        if ( !binding->link.IsNULL() && binding->link.GetPointerAlways() == link )
            return binding;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CDRNRelayBinding*
CDRNRelayClient::AddBinding( const CORE::CString& host, UInt16 port, const CORE::CString& via, const CORE::CString& realm, UInt32 requestedPeerId,
                             const CORE::CString& token, bool advertise, const CORE::CString& relayPolicy, const CORE::CString& relayKey )
{GUCEF_TRACE;

    CDRNRelayBinding* binding = GUCEF_NEW CDRNRelayBinding();
    binding->relayHost = host;
    binding->relayPort = port;
    binding->via = via;
    binding->realm = realm;
    binding->token = token;
    binding->requestedPeerId = requestedPeerId;
    binding->relayPolicy = relayPolicy;
    binding->relayKey = relayKey;
    binding->advertise = advertise;
    binding->state = CDRNRelayBinding::BINDINGSTATE_CONNECTING;
    m_bindings.push_back( binding );

    // the link to the relay runs under the relay's policy when it is known, the links over it under the game's
    CDRNEndpoint relay( via, host + ":" + CORE::UInt32ToString( port ) );
    if ( !relayKey.IsNULLOrEmpty() )
        relay.GetParams().Set( "key", CORE::CVariant( relayKey ) );
    UInt8 policyHash[ DRN_HANDSHAKE_HASH_SIZE ];
    bool havePolicy = !relayPolicy.IsNULLOrEmpty() && CDRNCrypto::FromHex( relayPolicy, policyHash, DRN_HANDSHAKE_HASH_SIZE );
    if ( !relayPolicy.IsNULLOrEmpty() && !havePolicy )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNRelayClient: The relay policy of " << relay.ToString() << " is not a policy hash, the node's own is used" );
    binding->connector = m_node->ConnectToService( CDRNEndpointList( relay ), havePolicy ? policyHash : GUCEF_NULL );
    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNRelayClient: Binding at " << relay.ToString() << " in realm \"" << realm << "\"" );
    return binding;
}

/*-------------------------------------------------------------------------*/

bool
CDRNRelayClient::Bind( const CDRNEndpoint& relayService, const CORE::CString& realm, UInt32 requestedPeerId, const CORE::CString& token )
{GUCEF_TRACE;

    if ( 0 == relayService.GetPort() || realm.IsNULLOrEmpty() || relayService.GetChannelType() == CDRNEndpoint::ChannelTypeRelay )
        return false;
    CDRNRelayBinding* binding = FindBinding( relayService.GetHost(), relayService.GetPort(), relayService.GetChannelType(), realm );
    if ( GUCEF_NULL != binding )
    {
        binding->advertise = true;
        return true;
    }
    CORE::CString relayPolicy = relayService.GetParams().GetValueAlways( "policy" ).AsString( CORE::CString::Empty, true );
    CORE::CString relayKey = relayService.GetParams().GetValueAlways( "key" ).AsString( CORE::CString::Empty, true );
    return GUCEF_NULL != AddBinding( relayService.GetHost(), relayService.GetPort(), relayService.GetChannelType(), realm, requestedPeerId, token, true, relayPolicy, relayKey );
}

/*-------------------------------------------------------------------------*/

bool
CDRNRelayClient::IsBound( const CDRNEndpoint& relayService, const CORE::CString& realm ) const
{GUCEF_TRACE;

    CDRNRelayBinding* binding = FindBinding( relayService.GetHost(), relayService.GetPort(), relayService.GetChannelType(), realm );
    return GUCEF_NULL != binding && CDRNRelayBinding::BINDINGSTATE_BOUND == binding->state;
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayClient::GetBoundEndpoints( CDRNEndpointList& endpoints ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_bindings.size(); ++i )
    {
        const CDRNRelayBinding* binding = m_bindings[ i ];
        if ( binding->advertise && CDRNRelayBinding::BINDINGSTATE_BOUND == binding->state )
            endpoints.Add( binding->GetEndpoint() );
    }
}

/*-------------------------------------------------------------------------*/

CIDRNChannelPtr
CDRNRelayClient::CreateChannel( const CDRNEndpoint& remote )
{GUCEF_TRACE;

    if ( remote.GetChannelType() != CDRNEndpoint::ChannelTypeRelay )
        return CIDRNChannelPtr();
    return CIDRNChannelPtr( GUCEF_NEW CDRNRelayChannel( *this ) );
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayClient::AttachChannel( CDRNRelayBinding& binding, CDRNRelayChannel& channel )
{GUCEF_TRACE;

    channel.m_binding = &binding;
    m_channels.push_back( &channel );
    if ( CDRNRelayBinding::BINDINGSTATE_BOUND == binding.state )
    {
        UInt32 peerId = channel.m_remotePeerId;
        if ( peerId >= binding.m_channelsByPeerId.size() )
            binding.m_channelsByPeerId.resize( peerId + 1, GUCEF_NULL );
        binding.m_channelsByPeerId[ peerId ] = &channel;
        channel.OnBindingReady( binding.maxPacketSize );
    }
    else
    {
        binding.m_waiting.push_back( &channel );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayClient::Detach( CDRNRelayChannel& channel )
{GUCEF_TRACE;

    CDRNRelayBinding* binding = channel.m_binding;
    channel.m_binding = GUCEF_NULL;
    if ( GUCEF_NULL != binding )
    {
        UInt32 peerId = channel.m_remotePeerId;
        if ( peerId < binding->m_channelsByPeerId.size() && binding->m_channelsByPeerId[ peerId ] == &channel )
            binding->m_channelsByPeerId[ peerId ] = GUCEF_NULL;
        for ( UInt32 i=0; i<binding->m_waiting.size(); ++i )
        {
            if ( binding->m_waiting[ i ] == &channel )
            {
                binding->m_waiting.erase( binding->m_waiting.begin() + i );
                break;
            }
        }
    }
    for ( UInt32 i=0; i<m_channels.size(); ++i )
    {
        if ( m_channels[ i ] == &channel )
        {
            m_channels.erase( m_channels.begin() + i );
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNRelayClient::Forward( CDRNRelayBinding& binding, UInt32 toPeerId, const void* data, UInt32 size )
{GUCEF_TRACE;
    if ( CDRNRelayBinding::BINDINGSTATE_BOUND != binding.state || binding.link.IsNULL() )
        return false;
    UInt32 headerSize = CDRNWireWriter::EncodeVarUInt64( toPeerId, binding.m_header );
    return binding.link->SendGather( DRN_LANE_UNRELIABLE, DRN_MSGCLASS_RELAY, DRN_RELAY_FORWARD, binding.m_header, headerSize, data, size );
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayClient::SendBind( CDRNRelayBinding& binding )
{GUCEF_TRACE;

    CORE::CDynamicBuffer request;
    CDRNWireWriter writer( request );
    writer.WriteString( binding.realm );
    writer.WriteVarUInt32( binding.requestedPeerId );
    writer.WriteString( binding.token );
    binding.link->Send( DRN_LANE_RELIABLE_ORDERED, DRN_MSGCLASS_RELAY, DRN_RELAY_BIND, request.GetConstBufferPtr(), writer.GetOffset() );
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayClient::FailBinding( CDRNRelayBinding& binding, const CORE::CString& reason )
{GUCEF_TRACE;

    if ( CDRNRelayBinding::BINDINGSTATE_FAILED == binding.state )
        return;
    binding.state = CDRNRelayBinding::BINDINGSTATE_FAILED;
    binding.failure = reason;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRNRelayClient: The binding at " << binding.relayHost << ":" << binding.relayPort << " in realm \"" << binding.realm << "\" failed: " << reason );

    for ( UInt32 i=0; i<binding.m_waiting.size(); ++i )
        binding.m_waiting[ i ]->OnBindingGone();
    binding.m_waiting.clear();
    for ( UInt32 i=0; i<binding.m_channelsByPeerId.size(); ++i )
        if ( GUCEF_NULL != binding.m_channelsByPeerId[ i ] )
            binding.m_channelsByPeerId[ i ]->OnBindingGone();
    binding.m_channelsByPeerId.clear();

    if ( !binding.connector.IsNULL() )
    {
        binding.connector->Cancel();
        binding.connector.Unlink();
    }
    if ( !binding.link.IsNULL() )
    {
        if ( !binding.link->IsClosed() )
            binding.link->Close( DRN_LINKREASON_CLOSED_BY_APP, "the relay binding ended" );
        binding.link.Unlink();
    }
    if ( binding.advertise )
        m_node->OnRelayBindingChanged( binding.GetEndpoint(), false, reason );
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayClient::OnBindReply( CDRNRelayBinding& binding, CDRNWireReader& reader )
{GUCEF_TRACE;

    UInt32 result = 0;
    UInt32 peerId = 0;
    CORE::CString realm;
    if ( CDRNRelayBinding::BINDINGSTATE_BINDING != binding.state || !reader.ReadVarUInt32( result ) || !reader.ReadVarUInt32( peerId ) || !reader.ReadString( realm ) )
        return;
    if ( DRN_RELAY_BIND_OK != result || 0 == peerId )
    {
        FailBinding( binding, DRN_RELAY_BIND_ID_TAKEN == result ? "the peer id is taken" :
                              DRN_RELAY_BIND_NO_SERVICE == result ? "the node does not relay" : "the relay refused" );
        return;
    }

    binding.boundPeerId = peerId;
    binding.state = CDRNRelayBinding::BINDINGSTATE_BOUND;
    UInt32 linkBudget = binding.link->GetMaxUnfragmentedPayload();
    binding.maxPacketSize = linkBudget > ForwardHeaderRoom ? linkBudget - ForwardHeaderRoom : 0;
    if ( binding.maxPacketSize > DRN_DEFAULT_DATAGRAM_SIZE )
        binding.maxPacketSize = DRN_DEFAULT_DATAGRAM_SIZE;
    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNRelayClient: Bound as " << binding.GetEndpoint().ToString() );

    TChannelVector waiting;
    waiting.swap( binding.m_waiting );
    for ( UInt32 i=0; i<waiting.size(); ++i )
    {
        CDRNRelayChannel* channel = waiting[ i ];
        UInt32 remotePeerId = channel->m_remotePeerId;
        if ( remotePeerId >= binding.m_channelsByPeerId.size() )
            binding.m_channelsByPeerId.resize( remotePeerId + 1, GUCEF_NULL );
        binding.m_channelsByPeerId[ remotePeerId ] = channel;
        channel->OnBindingReady( binding.maxPacketSize );
    }
    if ( binding.advertise )
        m_node->OnRelayBindingChanged( binding.GetEndpoint(), true, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayClient::OnForward( CDRNRelayBinding& binding, CDRNWireReader& reader )
{GUCEF_TRACE;
    UInt32 fromPeerId = 0;
    if ( CDRNRelayBinding::BINDINGSTATE_BOUND != binding.state || !reader.ReadVarUInt32( fromPeerId ) || 0 == fromPeerId )
        return;
    const UInt8* packet = reader.GetCurrentPtr();
    UInt32 packetSize = reader.GetRemaining();

    CDRNRelayChannel* channel = fromPeerId < binding.m_channelsByPeerId.size() ? binding.m_channelsByPeerId[ fromPeerId ] : GUCEF_NULL;
    if ( GUCEF_NULL == channel )
    {
        // a peer that reaches us through the relay: a new connection, accepted like one from a listener
        CDRNRelayChannel* accepted = GUCEF_NEW CDRNRelayChannel( *this );
        CIDRNChannelPtr acceptedPtr( accepted );
        accepted->m_remote = CDRNEndpoint( "relay://" + binding.relayHost + ":" + CORE::UInt32ToString( binding.relayPort ) + "/" + binding.realm + "/" +
                                           CORE::UInt32ToString( fromPeerId ) + "?via=" + binding.via );
        accepted->m_remotePeerId = fromPeerId;
        accepted->m_binding = &binding;
        accepted->m_connected = true;
        accepted->m_capabilities.maxPacketSize = binding.maxPacketSize;
        accepted->SetPulseGenerator( m_node->GetPulseGenerator() );
        m_channels.push_back( accepted );
        if ( fromPeerId >= binding.m_channelsByPeerId.size() )
            binding.m_channelsByPeerId.resize( fromPeerId + 1, GUCEF_NULL );
        binding.m_channelsByPeerId[ fromPeerId ] = accepted;
        m_node->OnChannelAccepted( acceptedPtr );
        accepted->OnPacket( packet, packetSize );
        return;
    }
    channel->OnPacket( packet, packetSize );
}

/*-------------------------------------------------------------------------*/

bool
CDRNRelayClient::OnLinkMessage( CDRNPeerLink* link, const CDRNLinkMessage& message )
{GUCEF_TRACE;
    if ( DRN_MSGCLASS_RELAY != message.msgClass )
        return false;
    CDRNRelayBinding* binding = FindBindingByLink( link );
    if ( GUCEF_NULL == binding )
        return false;
    CDRNWireReader reader( message.payload );
    if ( DRN_RELAY_FORWARD == message.opcode )
        OnForward( *binding, reader );
    else
    if ( DRN_RELAY_BIND == message.opcode )
        OnBindReply( *binding, reader );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayClient::OnLinkClosed( CDRNPeerLink* link )
{GUCEF_TRACE;

    CDRNRelayBinding* binding = FindBindingByLink( link );
    if ( GUCEF_NULL != binding )
        FailBinding( *binding, "the link to the relay closed" );
}

/*-------------------------------------------------------------------------*/

void
CDRNRelayClient::Tick( UInt64 /* nodeNow */ )
{GUCEF_TRACE;
    for ( UInt32 i=0; i<m_bindings.size(); ++i )
    {
        CDRNRelayBinding& binding = *m_bindings[ i ];
        if ( CDRNRelayBinding::BINDINGSTATE_CONNECTING == binding.state && !binding.connector.IsNULL() && binding.connector->IsDone() )
        {
            CDRNConnectorPtr connector = binding.connector;
            binding.connector.Unlink();
            if ( !connector->HasSucceeded() )
            {
                FailBinding( binding, "the relay could not be reached" );
                continue;
            }
            binding.link = connector->GetLink();
            binding.state = CDRNRelayBinding::BINDINGSTATE_BINDING;
            SendBind( binding );
        }
    }

    // bindings that failed and nobody uses go
    for ( UInt32 i=0; i<m_bindings.size(); )
    {
        CDRNRelayBinding* binding = m_bindings[ i ];
        bool unused = binding->m_waiting.empty();
        for ( UInt32 n=0; n<binding->m_channelsByPeerId.size() && unused; ++n )
            unused = GUCEF_NULL == binding->m_channelsByPeerId[ n ];
        bool used = false;
        for ( UInt32 n=0; n<m_channels.size() && !used; ++n )
            used = m_channels[ n ]->m_binding == binding;
        if ( CDRNRelayBinding::BINDINGSTATE_FAILED == binding->state && unused && !used )
        {
            m_bindings.erase( m_bindings.begin() + i );
            GUCEF_DELETE binding;
            continue;
        }
        ++i;
    }

    for ( UInt32 i=0; i<m_channels.size(); ++i )
        m_channels[ i ]->Tick();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
