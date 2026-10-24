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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETCLIENT_H
#include "gucefWEB_CWebSocketClient.h"
#define GUCEF_WEB_CWEBSOCKETCLIENT_H
#endif /* GUCEF_WEB_CWEBSOCKETCLIENT_H ? */

#ifndef GUCEF_DRN_CDRNWEBSOCKETCHANNELLISTENER_H
#include "gucefDRN_CDRNWebSocketChannelListener.h"
#define GUCEF_DRN_CDRNWEBSOCKETCHANNELLISTENER_H
#endif /* GUCEF_DRN_CDRNWEBSOCKETCHANNELLISTENER_H ? */

#include "gucefDRN_CDRNWebSocketChannel.h"

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

static void
InitWebSocketCapabilities( CDRNChannelCapabilities& capabilities )
{GUCEF_TRACE;
    capabilities.isReliable = true;
    capabilities.isDatagram = false;
    capabilities.maxPacketSize = 0;
    capabilities.isBrowserCompatible = true;
    capabilities.canListen = true;
}

/*-------------------------------------------------------------------------*/

CDRNWebSocketChannel::CDRNWebSocketChannel( void )
    : CIDRNChannel()
    , m_client( GUCEF_NULL )
    , m_listener( GUCEF_NULL )
    , m_connection()
    , m_pulseGenerator()
    , m_capabilities()
    , m_remote()
    , m_rxEventData()
    , m_connected( false )
    , m_closing( false )
    , m_maxReceivePacketSize( DRN_MAX_STREAM_PACKET_SIZE )
{GUCEF_TRACE;

    InitWebSocketCapabilities( m_capabilities );
}

/*-------------------------------------------------------------------------*/

CDRNWebSocketChannel::CDRNWebSocketChannel( CDRNWebSocketChannelListener* listener          ,
                                            const WEB::TWebSocketConnectionPtr& connection  ,
                                            CORE::PulseGeneratorPtr pulseGenerator          )
    : CIDRNChannel()
    , m_client( GUCEF_NULL )
    , m_listener( listener )
    , m_connection( connection )
    , m_pulseGenerator( pulseGenerator )
    , m_capabilities()
    , m_remote()
    , m_rxEventData()
    , m_connected( true )
    , m_closing( false )
    , m_maxReceivePacketSize( DRN_MAX_STREAM_PACKET_SIZE )
{GUCEF_TRACE;

    InitWebSocketCapabilities( m_capabilities );
    m_remote.SetChannelType( CDRNEndpoint::ChannelTypeWebSocket );
    if ( !m_connection.IsNULL() )
    {
        m_remote.SetAddress( "peer" + CORE::UInt32ToString( m_connection->GetId() ) + m_connection->GetPeerInfo().requestUri );
        m_connection->SetUserData( this );
    }
}

/*-------------------------------------------------------------------------*/

CDRNWebSocketChannel::~CDRNWebSocketChannel()
{GUCEF_TRACE;

    SignalUpcomingObserverDestruction();
    Close();
    if ( GUCEF_NULL != m_listener )
    {
        m_listener->OnChannelGone( this );
        m_listener = GUCEF_NULL;
    }
    GUCEF_DELETE m_client;
    m_client = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDRNWebSocketChannel::SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    m_pulseGenerator = pulseGenerator;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CDRNWebSocketChannel::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

bool
CDRNWebSocketChannel::Connect( const CDRNEndpoint& remote )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_listener || !m_connection.IsNULL() )
        return false;

    if ( m_pulseGenerator.IsNULL() )
        m_pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();

    if ( GUCEF_NULL == m_client )
    {
        m_client = GUCEF_NEW WEB::CWebSocketClient( m_pulseGenerator );
        m_client->SetMaxMessageSize( m_maxReceivePacketSize );
        SubscribeTo( m_client );
    }

    m_remote = remote;
    m_connected = false;
    m_closing = false;
    CORE::CString url = "ws://" + remote.GetAddress();
    return m_client->Connect( url );
}

/*-------------------------------------------------------------------------*/

void
CDRNWebSocketChannel::Close( void )
{GUCEF_TRACE;

    m_closing = true;
    m_connected = false;
    if ( GUCEF_NULL != m_client && WEB::CWebSocketClient::STATE_DISCONNECTED != m_client->GetState() )
        m_client->Disconnect();
    if ( !m_connection.IsNULL() )
    {
        m_connection->SetUserData( GUCEF_NULL );
        if ( m_connection->IsOpen() )
            m_connection->Close();
        m_connection.Unlink();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNWebSocketChannel::OnRemoteClosed( void )
{GUCEF_TRACE;

    bool wasConnected = m_connected;
    m_connected = false;
    if ( !m_connection.IsNULL() )
    {
        m_connection->SetUserData( GUCEF_NULL );
        m_connection.Unlink();
    }
    if ( wasConnected && !m_closing )
        NotifyObservers( ChannelDisconnectedEvent );
}

/*-------------------------------------------------------------------------*/

void
CDRNWebSocketChannel::OnListenerGone( void )
{GUCEF_TRACE;

    m_listener = GUCEF_NULL;
    OnRemoteClosed();
}

/*-------------------------------------------------------------------------*/

bool
CDRNWebSocketChannel::IsConnected( void ) const
{GUCEF_TRACE;

    return m_connected;
}

/*-------------------------------------------------------------------------*/

bool
CDRNWebSocketChannel::SendPacket( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( !m_connected || size > DRN_MAX_STREAM_PACKET_SIZE )
        return false;
    if ( GUCEF_NULL != m_client )
        return m_client->SendBinary( data, size );
    if ( !m_connection.IsNULL() )
        return m_connection->SendBinary( data, size );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNWebSocketChannel::OnPacket( const CORE::CDynamicBuffer& data )
{GUCEF_TRACE;

    if ( !m_connected || m_closing )
        return true;
    m_rxEventData.LinkTo( &data );
    return NotifyObservers( PacketReceivedEvent, &m_rxEventData );
}

/*-------------------------------------------------------------------------*/

const CDRNChannelCapabilities&
CDRNWebSocketChannel::GetCapabilities( void ) const
{GUCEF_TRACE;

    return m_capabilities;
}

/*-------------------------------------------------------------------------*/

CDRNEndpoint
CDRNWebSocketChannel::GetRemoteEndpoint( void ) const
{GUCEF_TRACE;

    return m_remote;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNWebSocketChannel::GetChannelType( void ) const
{GUCEF_TRACE;

    return CDRNEndpoint::ChannelTypeWebSocket;
}

/*-------------------------------------------------------------------------*/

void
CDRNWebSocketChannel::SetMaxReceivePacketSize( UInt32 maxPacketSize )
{GUCEF_TRACE;

    m_maxReceivePacketSize = maxPacketSize < DRN_MAX_STREAM_PACKET_SIZE ? maxPacketSize : DRN_MAX_STREAM_PACKET_SIZE;
    if ( !m_connection.IsNULL() )
        m_connection->SetMaxMessageSize( m_maxReceivePacketSize );
    if ( GUCEF_NULL != m_client )
        m_client->SetMaxMessageSize( m_maxReceivePacketSize );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNWebSocketChannel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNWebSocketChannel";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CDRNWebSocketChannel::OnNotify( CORE::CNotifier* notifier                 ,
                                const CORE::CEvent& eventid               ,
                                CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client || notifier != m_client )
        return;

    if ( WEB::CWebSocketClient::BinaryMessageEvent == eventid )
    {
        if ( GUCEF_NULL != eventdata )
            OnPacket( static_cast< WEB::TWebSocketBinaryMessageEventData* >( eventdata )->GetData().data );
    }
    else
    if ( WEB::CWebSocketClient::ConnectedEvent == eventid )
    {
        if ( !m_closing )
        {
            m_connected = true;
            NotifyObservers( ChannelConnectedEvent );
        }
    }
    else
    if ( WEB::CWebSocketClient::ClosedEvent == eventid )
    {
        bool wasConnected = m_connected;
        m_connected = false;
        if ( !m_closing )
        {
            if ( wasConnected )
            {
                NotifyObservers( ChannelDisconnectedEvent );
            }
            else
            {
                CDRNChannelErrorEventData errorData( "connection failed" );
                NotifyObservers( ChannelErrorEvent, &errorData );
            }
        }
    }
    else
    if ( WEB::CWebSocketClient::ErrorEvent == eventid )
    {
        if ( !m_closing && GUCEF_NULL != eventdata )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "DRNWebSocketChannel: " << static_cast< CORE::TCloneableString* >( eventdata )->GetData() );
        }
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
