/*
 *  drnchannelpluginEOSP2P: DRN channel plugin that moves DRN packets over the Epic Online Services P2P interface
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMLEASE_H
#include "pluginglueEOS_CEosPlatformLease.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMLEASE_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMLEASE_H ? */

#ifndef GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PCHANNELLISTENER_H
#include "drnchannelpluginEOSP2P_CEosP2PChannelListener.h"
#define GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PCHANNELLISTENER_H
#endif /* GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PCHANNELLISTENER_H ? */

#include "drnchannelpluginEOSP2P_CEosP2PChannel.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNPLUGIN {
namespace EOSP2P {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEosP2PChannel::CEosP2PChannel( void )
    : DRN::CIDRNChannel()
    , m_listener( GUCEF_NULL )
    , m_platform( GUCEF_NULL )
    , m_socket( GUCEF_NULL )
    , m_tickDemand( 0 )
    , m_info()
    , m_remoteUser()
    , m_pulseGenerator()
    , m_capabilities()
    , m_remote()
    , m_rxView()
    , m_rxEventData( &m_rxView )
    , m_connected( false )
    , m_established( false )
    , m_deadlineInMs( 0 )
    , m_networkType( PLUGINGLUE::EOS::EOSNETWORK_NONE )
    , m_sent( 0 )
    , m_received( 0 )
{GUCEF_TRACE;

    m_capabilities.isReliable = false;
    m_capabilities.isDatagram = true;
    m_capabilities.maxPacketSize = PLUGINGLUE::EOS::EosP2PMaxPacketSize;
    m_capabilities.isBrowserCompatible = false;
    m_capabilities.canListen = true;
}

/*-------------------------------------------------------------------------*/

CEosP2PChannel::CEosP2PChannel( CEosP2PChannelListener* listener       ,
                                PLUGINGLUE::EOS::CEosP2PSocket* socket ,
                                const CEosP2PEndpointInfo& info        ,
                                const CORE::CString& remoteUser        )
    : DRN::CIDRNChannel()
    , m_listener( listener )
    , m_platform( GUCEF_NULL )
    , m_socket( socket )
    , m_tickDemand( 0 )
    , m_info( info )
    , m_remoteUser( remoteUser )
    , m_pulseGenerator()
    , m_capabilities()
    , m_remote( CEosP2PEndpointInfo::MakeEndpoint( remoteUser, socket->GetSocketName() ) )
    , m_rxView()
    , m_rxEventData( &m_rxView )
    , m_connected( true )
    , m_established( true )
    , m_deadlineInMs( 0 )
    , m_networkType( PLUGINGLUE::EOS::EOSNETWORK_NONE )
    , m_sent( 0 )
    , m_received( 0 )
{GUCEF_TRACE;

    m_capabilities.isReliable = false;
    m_capabilities.isDatagram = true;
    m_capabilities.maxPacketSize = info.maxPacketSize;
    m_capabilities.isBrowserCompatible = false;
    m_capabilities.canListen = true;
}

/*-------------------------------------------------------------------------*/

CEosP2PChannel::~CEosP2PChannel()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    SignalUpcomingObserverDestruction();
    Close();
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannel::SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    m_pulseGenerator = pulseGenerator;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CEosP2PChannel::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PChannel::Attach( PLUGINGLUE::EOS::CEosPlatform* platform, PLUGINGLUE::EOS::CEosP2PSocket* socket )
{GUCEF_TRACE;

    m_platform = platform;
    m_socket = socket;
    TEventCallback packetCallback( this, &CEosP2PChannel::OnSocketPacketReceived );
    TEventCallback establishedCallback( this, &CEosP2PChannel::OnSocketConnectionEstablished );
    TEventCallback closedCallback( this, &CEosP2PChannel::OnSocketConnectionClosed );
    TEventCallback tickCallback( this, &CEosP2PChannel::OnPlatformTick );
    SubscribeTo( m_socket, PLUGINGLUE::EOS::CEosP2PSocket::PacketReceivedEvent, packetCallback );
    SubscribeTo( m_socket, PLUGINGLUE::EOS::CEosP2PSocket::ConnectionEstablishedEvent, establishedCallback );
    SubscribeTo( m_socket, PLUGINGLUE::EOS::CEosP2PSocket::ConnectionClosedEvent, closedCallback );
    SubscribeTo( m_platform, PLUGINGLUE::EOS::CEosPlatform::TickEvent, tickCallback );

    // The platform is ticked at the pace of the packets for as long as the channel is open
    m_tickDemand = m_platform->AddTickDemand( m_info.tickIntervalInMs );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannel::Detach( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_platform )
        return;

    UnsubscribeAllFromObserver();
    if ( 0 != m_tickDemand )
        m_platform->RemoveTickDemand( m_tickDemand );
    m_tickDemand = 0;
    PLUGINGLUE::EOS::CEosP2PRouter* router = m_platform->GetP2PRouter();
    if ( GUCEF_NULL != router && GUCEF_NULL != m_socket )
        router->ReleaseSocket( m_socket );
    m_socket = GUCEF_NULL;
    PLUGINGLUE::EOS::CEosGlue::Instance()->ReleasePlatform( m_platform );
    m_platform = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PChannel::Connect( const DRN::CDRNEndpoint& remote )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_listener || m_connected || GUCEF_NULL != m_platform )
        return false;

    CEosP2PEndpointInfo info;
    CORE::CString reason;
    if ( !info.Parse( remote, false, reason ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "EosP2PChannel: Cannot connect to " << remote.ToString() << ": " << reason );
        return false;
    }

    // The platform is ticked from the pulse generator of the application: that is where the channel has to run
    if ( !m_pulseGenerator.IsNULL() && m_pulseGenerator.GetPointerAlways() != CORE::CCoreGlobal::Instance()->GetPulseGenerator().GetPointerAlways() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "EosP2PChannel: The channel runs on the pulse generator of the application, a node on a thread of its own cannot use it" );
        return false;
    }

    // The hold on the platform is the lease until all has worked: every way out before that lets the platform go
    PLUGINGLUE::EOS::CEosPlatformLease platformLease( info.localUser, reason );
    PLUGINGLUE::EOS::CEosPlatform* platform = platformLease.Get();
    if ( GUCEF_NULL == platform )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "EosP2PChannel: Cannot connect to " << remote.ToString() << ": " << reason );
        return false;
    }
    PLUGINGLUE::EOS::CEosP2PRouter* router = platform->GetP2PRouter();
    if ( GUCEF_NULL == router )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "EosP2PChannel: The platform has no P2P interface" );
        return false;
    }
    if ( router->GetLocalUser() == info.remoteUser )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "EosP2PChannel: A user cannot connect to itself" );
        return false;
    }
    if ( info.hasRelayControl )
        router->SetRelayControl( info.relayControl );

    PLUGINGLUE::EOS::CEosP2PSocket* socket = router->AcquireSocket( info.socketName );
    if ( GUCEF_NULL == socket )
        return false;

    m_info = info;
    m_remoteUser = info.remoteUser;
    m_remote = remote;
    m_capabilities.maxPacketSize = info.maxPacketSize;
    Attach( platformLease.Keep(), socket );

    m_connected = true;
    m_established = false;
    m_deadlineInMs = 0 == info.connectTimeoutInMs ? 0 : PLUGINGLUE::EOS::CEosPlatform::GetMonotonicTimeInMs() + info.connectTimeoutInMs;
    NotifyObservers( ChannelConnectedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannel::Close( void )
{GUCEF_TRACE;

    bool wasConnected = m_connected;
    m_connected = false;
    if ( GUCEF_NULL != m_socket && wasConnected )
        m_socket->CloseConnection( m_remoteUser );
    Detach();
    if ( GUCEF_NULL != m_listener )
    {
        CEosP2PChannelListener* listener = m_listener;
        m_listener = GUCEF_NULL;
        m_socket = GUCEF_NULL;
        listener->OnChannelClosed( this );
    }
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannel::OnListenerGone( void )
{GUCEF_TRACE;

    m_listener = GUCEF_NULL;
    m_socket = GUCEF_NULL;
    if ( m_connected )
    {
        m_connected = false;
        NotifyObservers( ChannelDisconnectedEvent );
    }
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PChannel::IsConnected( void ) const
{GUCEF_TRACE;

    return m_connected;
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PChannel::IsEstablished( void ) const
{GUCEF_TRACE;

    return m_established;
}

/*-------------------------------------------------------------------------*/

PLUGINGLUE::EOS::EEosNetworkType
CEosP2PChannel::GetNetworkType( void ) const
{GUCEF_TRACE;

    return m_networkType;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEosP2PChannel::GetRemoteUser( void ) const
{GUCEF_TRACE;

    return m_remoteUser;
}

/*-------------------------------------------------------------------------*/

UInt32
CEosP2PChannel::GetSentPacketCount( void ) const
{GUCEF_TRACE;

    return m_sent;
}

/*-------------------------------------------------------------------------*/

UInt32
CEosP2PChannel::GetReceivedPacketCount( void ) const
{GUCEF_TRACE;

    return m_received;
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PChannel::SendPacket( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( !m_connected || GUCEF_NULL == m_socket || size > m_capabilities.maxPacketSize )
        return false;
    if ( !m_socket->SendPacket( m_remoteUser, data, size, m_info.reliability ) )
        return false;
    ++m_sent;
    return true;
}

/*-------------------------------------------------------------------------*/

const DRN::CDRNChannelCapabilities&
CEosP2PChannel::GetCapabilities( void ) const
{GUCEF_TRACE;

    return m_capabilities;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNEndpoint
CEosP2PChannel::GetRemoteEndpoint( void ) const
{GUCEF_TRACE;

    return m_remote;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEosP2PChannel::GetChannelType( void ) const
{GUCEF_TRACE;

    return CEosP2PEndpointInfo::ChannelType;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEosP2PChannel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRNPLUGIN::EOSP2P::CEosP2PChannel";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PChannel::IsOurs( const char* remoteUser ) const
{GUCEF_TRACE;

    return 0 == strcmp( remoteUser, m_remoteUser.C_String() );
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PChannel::OnPacket( const PLUGINGLUE::EOS::CEosP2PPacketEventData& packet )
{GUCEF_TRACE;

    if ( !m_connected )
        return true;

    m_established = true;
    ++m_received;
    m_rxView.LinkTo( packet.data, packet.size );
    return NotifyObservers( PacketReceivedEvent, &m_rxEventData );
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannel::OnConnectionEstablished( const PLUGINGLUE::EOS::CEosP2PConnectionEventData& data )
{GUCEF_TRACE;

    m_established = true;
    m_networkType = data.networkType;
    const CORE::CString route = PLUGINGLUE::EOS::EOSNETWORK_RELAYED == data.networkType ? "through the relay" : "directly";
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "EosP2PChannel: The connection to the peer is established, " << route );
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannel::OnConnectionClosed( const PLUGINGLUE::EOS::CEosP2PConnectionEventData& data )
{GUCEF_TRACE;

    if ( !m_connected )
        return;

    switch ( data.closeReason )
    {
        case PLUGINGLUE::EOS::EOSCLOSE_TIMED_OUT :
        case PLUGINGLUE::EOS::EOSCLOSE_CONNECTION_FAILED :
        case PLUGINGLUE::EOS::EOSCLOSE_NEGOTIATION_FAILED :
        case PLUGINGLUE::EOS::EOSCLOSE_UNEXPECTED_ERROR :
        case PLUGINGLUE::EOS::EOSCLOSE_TOO_MANY_CONNECTIONS :
        case PLUGINGLUE::EOS::EOSCLOSE_INVALID_MESSAGE :
        case PLUGINGLUE::EOS::EOSCLOSE_INVALID_DATA :
        {
            CORE::CString error = "The EOS P2P connection to the peer was closed by the SDK";
            End( &error );
            break;
        }
        default :
            End( GUCEF_NULL );
            break;
    }
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannel::OnTick( void )
{GUCEF_TRACE;

    if ( m_connected && !m_established && 0 != m_deadlineInMs && PLUGINGLUE::EOS::CEosPlatform::GetMonotonicTimeInMs() >= m_deadlineInMs )
    {
        CORE::CString error = "The EOS P2P connection to the peer was not established in time";
        End( &error );
    }
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannel::End( const CORE::CString* error )
{GUCEF_TRACE;

    if ( !m_connected )
        return;
    m_connected = false;

    // Nothing is touched after the events: whoever hears them may let go of the channel
    if ( GUCEF_NULL != m_socket )
        m_socket->CloseConnection( m_remoteUser );
    if ( GUCEF_NULL != error )
    {
        DRN::CDRNChannelErrorEventData errorData( *error );
        if ( !NotifyObservers( ChannelErrorEvent, &errorData ) )
            return;
    }
    NotifyObservers( ChannelDisconnectedEvent );
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannel::OnSocketPacketReceived( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( notifier != m_socket || GUCEF_NULL == eventData )
        return;

    const PLUGINGLUE::EOS::CEosP2PPacketEventData* packet = static_cast< const PLUGINGLUE::EOS::CEosP2PPacketEventData* >( eventData );
    if ( IsOurs( packet->remoteUser ) )
        OnPacket( *packet );
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannel::OnSocketConnectionEstablished( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventId  ,
                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( notifier != m_socket || GUCEF_NULL == eventData )
        return;

    const PLUGINGLUE::EOS::CEosP2PConnectionEventData* data = static_cast< const PLUGINGLUE::EOS::CEosP2PConnectionEventData* >( eventData );
    if ( IsOurs( data->remoteUser ) )
        OnConnectionEstablished( *data );
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannel::OnSocketConnectionClosed( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( notifier != m_socket || GUCEF_NULL == eventData )
        return;

    const PLUGINGLUE::EOS::CEosP2PConnectionEventData* data = static_cast< const PLUGINGLUE::EOS::CEosP2PConnectionEventData* >( eventData );
    if ( IsOurs( data->remoteUser ) )
        OnConnectionClosed( *data );
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannel::OnPlatformTick( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    OnTick();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOSP2P */
}; /* namespace DRNPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
