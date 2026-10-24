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

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_DRN_CDRNUDPCHANNELLISTENER_H
#include "gucefDRN_CDRNUdpChannelListener.h"
#define GUCEF_DRN_CDRNUDPCHANNELLISTENER_H
#endif /* GUCEF_DRN_CDRNUDPCHANNELLISTENER_H ? */

#include "gucefDRN_CDRNUdpChannel.h"

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

CDRNUdpChannel::CDRNUdpChannel( void )
    : CIDRNChannel()
    , m_socket( GUCEF_NULL )
    , m_listener( GUCEF_NULL )
    , m_remoteAddress()
    , m_pulseGenerator()
    , m_capabilities()
    , m_remote()
    , m_rxView()
    , m_rxEventData( &m_rxView )
    , m_connected( false )
    , m_answered( false )
    , m_haveCookie( false )
    , m_lastUnanswered()
    , m_cookieScratch()
{GUCEF_TRACE;

    memset( m_cookie, 0, sizeof( m_cookie ) );
    m_capabilities.isReliable = false;
    m_capabilities.isDatagram = true;
    m_capabilities.maxPacketSize = DRN_DEFAULT_DATAGRAM_SIZE;
    m_capabilities.isBrowserCompatible = false;
    m_capabilities.canListen = true;
}

/*-------------------------------------------------------------------------*/

CDRNUdpChannel::CDRNUdpChannel( CDRNUdpChannelListener* listener              ,
                                const COMCORE::CIPv4Address& remote          ,
                                CORE::PulseGeneratorPtr pulseGenerator       ,
                                UInt32 maxPacketSize                         )
    : CIDRNChannel()
    , m_socket( GUCEF_NULL )
    , m_listener( listener )
    , m_remoteAddress( remote )
    , m_pulseGenerator( pulseGenerator )
    , m_capabilities()
    , m_remote( CDRNEndpoint::ChannelTypeUdp, remote.AddressAndPortAsString() )
    , m_rxView()
    , m_rxEventData( &m_rxView )
    , m_connected( true )
    , m_answered( true )
    , m_haveCookie( false )
    , m_lastUnanswered()
    , m_cookieScratch()
{GUCEF_TRACE;

    memset( m_cookie, 0, sizeof( m_cookie ) );
    m_capabilities.isReliable = false;
    m_capabilities.isDatagram = true;
    m_capabilities.maxPacketSize = maxPacketSize;
    m_capabilities.isBrowserCompatible = false;
    m_capabilities.canListen = true;
}

/*-------------------------------------------------------------------------*/

CDRNUdpChannel::~CDRNUdpChannel()
{GUCEF_TRACE;

    SignalUpcomingObserverDestruction();
    Close();
    GUCEF_DELETE m_socket;
    m_socket = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDRNUdpChannel::SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    m_pulseGenerator = pulseGenerator;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CDRNUdpChannel::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

bool
CDRNUdpChannel::Connect( const CDRNEndpoint& remote )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_listener )
        return false;

    UInt16 port = remote.GetPort();
    CORE::CString host = remote.GetHost();
    if ( 0 == port || host.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNUdpChannel: Cannot connect to an endpoint without host and port: " << remote.ToString() );
        return false;
    }

    m_remoteAddress = COMCORE::CIPv4Address( host, port );
    if ( 0 == m_remoteAddress.GetAddress() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNUdpChannel: Cannot resolve " << host );
        return false;
    }
    m_capabilities.maxPacketSize = remote.GetParams().GetValueAlways( "maxPacketSize", CORE::CVariant( DRN_DEFAULT_DATAGRAM_SIZE ) ).AsUInt32( DRN_DEFAULT_DATAGRAM_SIZE );

    if ( m_pulseGenerator.IsNULL() )
        m_pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();

    if ( GUCEF_NULL == m_socket )
    {
        m_socket = GUCEF_NEW COMCORE::CUDPSocket( m_pulseGenerator, false );
        SubscribeTo( m_socket );
    }
    if ( !m_socket->IsActive() && !m_socket->Open( (UInt16) 0 ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNUdpChannel: Failed to open a socket for " << remote.ToString() );
        return false;
    }

    m_remote = remote;
    m_connected = true;
    m_answered = false;
    m_haveCookie = false;
    m_lastUnanswered.SetDataSize( 0 );
    NotifyObservers( ChannelConnectedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNUdpChannel::Close( void )
{GUCEF_TRACE;

    m_connected = false;
    if ( GUCEF_NULL != m_socket && m_socket->IsActive() )
        m_socket->Close();
    if ( GUCEF_NULL != m_listener )
    {
        CDRNUdpChannelListener* listener = m_listener;
        m_listener = GUCEF_NULL;
        listener->OnChannelClosed( this );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNUdpChannel::OnListenerGone( void )
{GUCEF_TRACE;

    m_listener = GUCEF_NULL;
    if ( m_connected )
    {
        m_connected = false;
        NotifyObservers( ChannelDisconnectedEvent );
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNUdpChannel::IsConnected( void ) const
{GUCEF_TRACE;

    return m_connected;
}

/*-------------------------------------------------------------------------*/

bool
CDRNUdpChannel::SendPacket( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( !m_connected || size > 65507 - DRN_UDP_COOKIE_SIZE )
        return false;

    if ( GUCEF_NULL != m_listener )
        return m_listener->SendTo( m_remoteAddress, data, size );
    if ( GUCEF_NULL == m_socket )
        return false;

    if ( !m_answered )
    {
        // kept to send again with the cookie should the listener ask for one
        m_lastUnanswered.SetDataSize( 0 );
        m_lastUnanswered.CopyFrom( 0, size, data );
        if ( m_haveCookie )
        {
            m_cookieScratch.SetDataSize( 0 );
            m_cookieScratch.CopyFrom( 0, DRN_UDP_COOKIE_SIZE, m_cookie );
            m_cookieScratch.CopyFrom( DRN_UDP_COOKIE_SIZE, size, data );
            return SendRaw( m_cookieScratch.GetConstBufferPtr(), m_cookieScratch.GetDataSize() );
        }
    }
    return SendRaw( data, size );
}

/*-------------------------------------------------------------------------*/

bool
CDRNUdpChannel::SendRaw( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_socket )
        return false;
    return m_socket->SendPacketTo( m_remoteAddress, data, (UInt16) size ) >= 0;
}

/*-------------------------------------------------------------------------*/

void
CDRNUdpChannel::OnHelloRetry( const UInt8* data, UInt32 size )
{GUCEF_TRACE;

    if ( m_answered || DRN_UDP_COOKIE_SIZE != size || DRN_UDP_COOKIE_VERSION != data[ 1 ] )
        return;

    memcpy( m_cookie, data, DRN_UDP_COOKIE_SIZE );
    m_cookie[ 0 ] = DRN_UDP_COOKIE_ECHO;
    m_haveCookie = true;
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNUdpChannel: " << m_remote.ToString() << " asked for its cookie" );

    if ( m_lastUnanswered.GetDataSize() > 0 )
    {
        m_cookieScratch.SetDataSize( 0 );
        m_cookieScratch.CopyFrom( 0, DRN_UDP_COOKIE_SIZE, m_cookie );
        m_cookieScratch.CopyFrom( DRN_UDP_COOKIE_SIZE, m_lastUnanswered.GetDataSize(), m_lastUnanswered.GetConstBufferPtr() );
        SendRaw( m_cookieScratch.GetConstBufferPtr(), m_cookieScratch.GetDataSize() );
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNUdpChannel::OnPacket( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( !m_connected )
        return true;
    m_rxView.LinkTo( data, size );
    return NotifyObservers( PacketReceivedEvent, &m_rxEventData );
}

/*-------------------------------------------------------------------------*/

const CDRNChannelCapabilities&
CDRNUdpChannel::GetCapabilities( void ) const
{GUCEF_TRACE;

    return m_capabilities;
}

/*-------------------------------------------------------------------------*/

CDRNEndpoint
CDRNUdpChannel::GetRemoteEndpoint( void ) const
{GUCEF_TRACE;

    return m_remote;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNUdpChannel::GetChannelType( void ) const
{GUCEF_TRACE;

    return CDRNEndpoint::ChannelTypeUdp;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNUdpChannel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNUdpChannel";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CDRNUdpChannel::OnNotify( CORE::CNotifier* notifier                 ,
                          const CORE::CEvent& eventid               ,
                          CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_socket || notifier != m_socket )
        return;

    if ( COMCORE::CUDPSocket::UDPPacketsRecievedEvent == eventid && GUCEF_NULL != eventdata )
    {
        const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& packets = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( eventdata )->GetData();
        for ( UInt32 i=0; i<packets.packetsReceived && i<packets.packets.size(); ++i )
        {
            const COMCORE::CUDPSocket::TPacketEntry& entry = packets.packets[ i ];
            if ( entry.sourceAddress != m_remoteAddress || entry.dataBuffer.IsNULL() )
                continue;
            const CORE::CDynamicBuffer& data = entry.dataBuffer.GetData();
            const UInt8* bytes = static_cast< const UInt8* >( data.GetConstBufferPtr() );
            if ( data.GetDataSize() > 0 && DRN_UDP_HELLO_RETRY == bytes[ 0 ] )
            {
                OnHelloRetry( bytes, data.GetDataSize() );
                continue;
            }
            if ( !m_answered && data.GetDataSize() > 0 && ( DRN_PROTOCOL_ID == bytes[ 0 ] || DRN_PROTOCOL_ID_PROTECTED_0 == bytes[ 0 ] || DRN_PROTOCOL_ID_PROTECTED_1 == bytes[ 0 ] ) )
            {
                m_answered = true;
                m_lastUnanswered.Clear();
            }
            if ( !OnPacket( data.GetConstBufferPtr(), data.GetDataSize() ) )
                return;
            if ( !m_connected )
                return;
        }
    }
    else
    if ( COMCORE::CUDPSocket::UDPSocketErrorEvent == eventid )
    {
        if ( m_connected )
        {
            CDRNChannelErrorEventData errorData( "udp socket error" );
            NotifyObservers( ChannelErrorEvent, &errorData );
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
