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

#ifndef GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PCHANNEL_H
#include "drnchannelpluginEOSP2P_CEosP2PChannel.h"
#define GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PCHANNEL_H
#endif /* GUCEF_DRNPLUGIN_EOSP2P_CEOSP2PCHANNEL_H ? */

#include "drnchannelpluginEOSP2P_CEosP2PChannelListener.h"

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

CEosP2PChannelListener::CEosP2PChannelListener( void )
    : DRN::CIDRNChannelListener()
    , m_platform( GUCEF_NULL )
    , m_socket( GUCEF_NULL )
    , m_previousPolicy( PLUGINGLUE::EOS::CEosP2PSocket::ACCEPT_NONE )
    , m_changedPolicy( false )
    , m_tickDemand( 0 )
    , m_info()
    , m_listenEndpoint()
    , m_pulseGenerator()
    , m_channels()
    , m_listening( false )
    , m_events()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosP2PChannelListener::~CEosP2PChannelListener()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    SignalUpcomingObserverDestruction();
    StopListening();

    // The channels that are still around lose their socket
    TEntryVector channels;
    channels.swap( m_channels );
    for ( size_t i=0; i<channels.size(); ++i )
        channels[ i ].channel->OnListenerGone();

    Release();
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannelListener::Release( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_platform )
        return;

    UnsubscribeAllFromObserver();
    m_events.Clear();
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

void
CEosP2PChannelListener::SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    m_pulseGenerator = pulseGenerator;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CEosP2PChannelListener::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PChannelListener::Listen( const DRN::CDRNEndpoint& local )
{GUCEF_TRACE;

    if ( m_listening || GUCEF_NULL != m_platform )
        return false;

    CEosP2PEndpointInfo info;
    CORE::CString reason;
    if ( !info.Parse( local, true, reason ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "EosP2PChannelListener: Cannot listen on " << local.ToString() << ": " << reason );
        return false;
    }

    // The glue ticks the platform on the pulse generator of the application, and the SDK is used on that thread only:
    // the pulse generator the listener runs on has to be pulsed by that one (the node's, without a thread of its own)
    CORE::PulseGeneratorPtr appPulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    if ( m_pulseGenerator.IsNULL() )
        m_pulseGenerator = appPulseGenerator;
    CORE::PulseGeneratorPtr drivingPulseGenerator = GetDrivingPulseGenerator().IsNULL() ? m_pulseGenerator : GetDrivingPulseGenerator();
    if ( drivingPulseGenerator.GetPointerAlways() != appPulseGenerator.GetPointerAlways() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "EosP2PChannelListener: Cannot listen on " << local.ToString() << ": the platform of Epic Online " <<
                         "Services is ticked on the pulse generator of the application, and the listener would run on another thread (a node with a thread of its own)" );
        return false;
    }

    // The hold on the platform is the lease until all has worked: every way out before that lets the platform go
    PLUGINGLUE::EOS::CEosPlatformLease platformLease( info.localUser, reason );
    PLUGINGLUE::EOS::CEosPlatform* platform = platformLease.Get();
    if ( GUCEF_NULL == platform )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "EosP2PChannelListener: Cannot listen on " << local.ToString() << ": " << reason );
        return false;
    }
    PLUGINGLUE::EOS::CEosP2PRouter* router = platform->GetP2PRouter();
    if ( GUCEF_NULL == router )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "EosP2PChannelListener: The platform has no P2P interface" );
        return false;
    }
    if ( info.hasRelayControl )
        router->SetRelayControl( info.relayControl );

    PLUGINGLUE::EOS::CEosP2PSocket* socket = router->AcquireSocket( info.socketName );
    if ( GUCEF_NULL == socket )
        return false;

    // Who may connect: everybody when the endpoint says so, else the allow list. A policy that somebody else set
    // (the lobby that owns the allow list) stays
    m_previousPolicy = socket->GetAcceptPolicy();
    m_changedPolicy = false;
    if ( info.acceptAny )
    {
        socket->SetAcceptPolicy( PLUGINGLUE::EOS::CEosP2PSocket::ACCEPT_ANY );
        m_changedPolicy = true;
    }
    else
    if ( PLUGINGLUE::EOS::CEosP2PSocket::ACCEPT_NONE == m_previousPolicy )
    {
        socket->SetAcceptPolicy( PLUGINGLUE::EOS::CEosP2PSocket::ACCEPT_LIST );
        m_changedPolicy = true;
    }
    for ( CORE::CString::StringSet::const_iterator i = info.allowedRemotes.begin(); i != info.allowedRemotes.end(); ++i )
        socket->AllowRemote( (*i) );
    socket->SetMaxConnections( info.maxConnections );

    m_info = info;
    m_platform = platformLease.Keep();
    m_socket = socket;
    m_listenEndpoint = DRN::CDRNEndpoint( CEosP2PEndpointInfo::ChannelType, router->GetLocalUser() + "/" + info.socketName );

    // the socket tells on the thread that ticks the platform: its events wait for the pulse the listener runs on
    TEventCallback packetCallback( this, &CEosP2PChannelListener::OnSocketPacketReceived );
    TEventCallback closedCallback( this, &CEosP2PChannelListener::OnSocketConnectionClosed );
    TEventCallback establishedCallback( this, &CEosP2PChannelListener::OnSocketConnectionEstablished );
    TEventCallback pulseCallback( this, &CEosP2PChannelListener::OnPulse );
    SubscribeTo( m_socket, PLUGINGLUE::EOS::CEosP2PSocket::PacketReceivedEvent, packetCallback );
    SubscribeTo( m_socket, PLUGINGLUE::EOS::CEosP2PSocket::ConnectionClosedEvent, closedCallback );
    SubscribeTo( m_socket, PLUGINGLUE::EOS::CEosP2PSocket::ConnectionEstablishedEvent, establishedCallback );
    SubscribeTo( m_pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, pulseCallback );
    m_tickDemand = m_platform->AddTickDemand( info.tickIntervalInMs );
    m_listening = true;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EosP2PChannelListener: Listening on the EOS P2P socket " << info.socketName );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannelListener::StopListening( void )
{GUCEF_TRACE;

    if ( !m_listening )
        return;
    m_listening = false;

    // Nobody new gets in; the channels that are open stay
    if ( GUCEF_NULL != m_socket && m_changedPolicy )
        m_socket->SetAcceptPolicy( m_previousPolicy );
    m_changedPolicy = false;
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PChannelListener::IsListening( void ) const
{GUCEF_TRACE;

    return m_listening;
}

/*-------------------------------------------------------------------------*/

DRN::CDRNEndpoint
CEosP2PChannelListener::GetListenEndpoint( void ) const
{GUCEF_TRACE;

    return m_listenEndpoint;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEosP2PChannelListener::GetChannelType( void ) const
{GUCEF_TRACE;

    return CEosP2PEndpointInfo::ChannelType;
}

/*-------------------------------------------------------------------------*/

PLUGINGLUE::EOS::CEosP2PSocket*
CEosP2PChannelListener::GetSocket( void ) const
{GUCEF_TRACE;

    return m_socket;
}

/*-------------------------------------------------------------------------*/

UInt32
CEosP2PChannelListener::GetChannelCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_channels.size();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEosP2PChannelListener::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRNPLUGIN::EOSP2P::CEosP2PChannelListener";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

Int32
CEosP2PChannelListener::FindChannel( const char* remoteUser ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_channels.size(); ++i )
        if ( 0 == strcmp( m_channels[ i ].remoteUser, remoteUser ) )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannelListener::OnChannelClosed( CEosP2PChannel* channel )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_channels.size(); ++i )
    {
        if ( m_channels[ i ].channel == channel )
        {
            m_channels.erase( m_channels.begin() + i );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PChannelListener::AcceptChannel( const PLUGINGLUE::EOS::CEosP2PPacketEventData& packet )
{GUCEF_TRACE;

    CEosP2PChannel* newChannel = GUCEF_NEW CEosP2PChannel( this, m_socket, m_info, CORE::CString( packet.remoteUser ) );
    DRN::CIDRNChannelPtr channelPtr( newChannel );
    newChannel->SetPulseGenerator( m_pulseGenerator );
    newChannel->SetDrivingPulseGenerator( GetDrivingPulseGenerator() );

    SEntry entry;
    strncpy( entry.remoteUser, packet.remoteUser, sizeof( entry.remoteUser ) - 1 );
    entry.remoteUser[ sizeof( entry.remoteUser ) - 1 ] = 0;
    entry.channel = newChannel;
    m_channels.push_back( entry );

    DRN::CDRNChannelEventData channelData( channelPtr );
    if ( !NotifyObservers( ChannelAcceptedEvent, &channelData ) )
        return false;

    // The first packet goes to whoever took the channel, if nobody did it is closed when the pointer goes
    if ( newChannel->IsConnected() )
        newChannel->OnPacket( packet );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosP2PChannelListener::OnPacket( const PLUGINGLUE::EOS::CEosP2PPacketEventData& packet )
{GUCEF_TRACE;

    Int32 index = FindChannel( packet.remoteUser );
    if ( index >= 0 )
    {
        CEosP2PChannel* channel = m_channels[ index ].channel;
        if ( channel->IsConnected() )
        {
            channel->OnPacket( packet );
            return true;
        }
        // A channel that ended, the peer comes again: it gets a new one
        m_channels.erase( m_channels.begin() + index );
    }
    if ( !m_listening )
        return true;
    return AcceptChannel( packet );
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannelListener::OnConnectionEstablished( const PLUGINGLUE::EOS::CEosP2PConnectionEventData& data )
{GUCEF_TRACE;

    Int32 index = FindChannel( data.remoteUser );
    if ( index >= 0 )
        m_channels[ index ].channel->OnConnectionEstablished( data );
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannelListener::OnConnectionClosed( const PLUGINGLUE::EOS::CEosP2PConnectionEventData& data )
{GUCEF_TRACE;

    Int32 index = FindChannel( data.remoteUser );
    if ( index >= 0 )
    {
        CEosP2PChannel* channel = m_channels[ index ].channel;
        m_channels.erase( m_channels.begin() + index );
        channel->m_listener = GUCEF_NULL;
        channel->OnConnectionClosed( data );
    }
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannelListener::RequestPulse( void )
{GUCEF_TRACE;

    // on the thread that ticks the platform, without the node lock: never a pulse on this thread, a request to the
    // driver of the pulse generator (for a node's: the application's pulse generator)
    if ( !m_pulseGenerator.IsNULL() )
        m_pulseGenerator->RequestImmediatePulseThroughDriver();
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannelListener::OnSocketPacketReceived( CORE::CNotifier* notifier    ,
                                                const CORE::CEvent& eventId  ,
                                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the thread that ticks the platform: the packet waits for the pulse the listener runs on
    if ( GUCEF_NULL != eventData && m_events.AddPacket( *static_cast< const PLUGINGLUE::EOS::CEosP2PPacketEventData* >( eventData ) ) )
        RequestPulse();
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannelListener::OnSocketConnectionEstablished( CORE::CNotifier* notifier    ,
                                                       const CORE::CEvent& eventId  ,
                                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;

    const PLUGINGLUE::EOS::CEosP2PConnectionEventData* data = static_cast< const PLUGINGLUE::EOS::CEosP2PConnectionEventData* >( eventData );
    if ( m_events.AddConnectionEvent( CEosP2PSocketEventQueue::EVENTTYPE_CONNECTION_ESTABLISHED, *data ) )
        RequestPulse();
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannelListener::OnSocketConnectionClosed( CORE::CNotifier* notifier    ,
                                                  const CORE::CEvent& eventId  ,
                                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;

    const PLUGINGLUE::EOS::CEosP2PConnectionEventData* data = static_cast< const PLUGINGLUE::EOS::CEosP2PConnectionEventData* >( eventData );
    if ( m_events.AddConnectionEvent( CEosP2PSocketEventQueue::EVENTTYPE_CONNECTION_CLOSED, *data ) )
        RequestPulse();
}

/*-------------------------------------------------------------------------*/

void
CEosP2PChannelListener::OnPulse( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // a pulse of the pulse generator the listener runs on, for a node's under the node lock: the events of the socket
    // in the order the platform gave them. Nothing is touched once an observer let go of the listener
    UInt32 count = 0;
    const CEosP2PSocketEventQueue::TQueuedEventVector& events = m_events.TakeAll( count );
    for ( UInt32 i=0; i<count; ++i )
    {
        const CEosP2PSocketEventQueue::CQueuedEvent& queued = *events[ i ];
        switch ( queued.type )
        {
            case CEosP2PSocketEventQueue::EVENTTYPE_PACKET :
            {
                if ( !OnPacket( queued.packet ) )
                    return;
                break;
            }
            case CEosP2PSocketEventQueue::EVENTTYPE_CONNECTION_ESTABLISHED :
            {
                OnConnectionEstablished( queued.connection );
                break;
            }
            case CEosP2PSocketEventQueue::EVENTTYPE_CONNECTION_CLOSED :
            {
                OnConnectionClosed( queued.connection );
                break;
            }
        }
    }
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
