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

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_DRN_CDRNSECURITYPOLICY_H
#include "gucefDRN_CDRNSecurityPolicy.h"
#define GUCEF_DRN_CDRNSECURITYPOLICY_H
#endif /* GUCEF_DRN_CDRNSECURITYPOLICY_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_DRN_CDRNUDPCHANNEL_H
#include "gucefDRN_CDRNUdpChannel.h"
#define GUCEF_DRN_CDRNUDPCHANNEL_H
#endif /* GUCEF_DRN_CDRNUDPCHANNEL_H ? */

#include "gucefDRN_CDRNUdpChannelListener.h"

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

CDRNUdpChannelListener::CDRNUdpChannelListener( void )
    : CIDRNChannelListener()
    , m_socket( GUCEF_NULL )
    , m_pulseGenerator()
    , m_listenEndpoint()
    , m_maxPacketSize( DRN_DEFAULT_DATAGRAM_SIZE )
    , m_channels()
    , m_lastSource()
    , m_lastChannel( GUCEF_NULL )
    , m_statelessCookies( true )
    , m_cookieRetries( 0 )
    , m_invalidCookies( 0 )
{GUCEF_TRACE;

    memset( m_cookieSecret, 0, sizeof( m_cookieSecret ) );
}

/*-------------------------------------------------------------------------*/

CDRNUdpChannelListener::~CDRNUdpChannelListener()
{GUCEF_TRACE;

    SignalUpcomingObserverDestruction();
    StopListening();

    // the channels that are still around lose their socket
    TChannelMap channels;
    channels.swap( m_channels );
    m_lastChannel = GUCEF_NULL;
    TChannelMap::iterator i = channels.begin();
    while ( i != channels.end() )
    {
        (*i).second->OnListenerGone();
        ++i;
    }

    GUCEF_DELETE m_socket;
    m_socket = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDRNUdpChannelListener::SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    m_pulseGenerator = pulseGenerator;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CDRNUdpChannelListener::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

bool
CDRNUdpChannelListener::Listen( const CDRNEndpoint& local )
{GUCEF_TRACE;

    StopListening();

    UInt16 port = local.GetPort();
    if ( 0 == port )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNUdpChannelListener: The listen endpoint needs a port: " << local.ToString() );
        return false;
    }

    if ( m_pulseGenerator.IsNULL() )
        m_pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();

    if ( GUCEF_NULL == m_socket )
    {
        m_socket = GUCEF_NEW COMCORE::CUDPSocket( m_pulseGenerator, false );
        SubscribeTo( m_socket );
    }

    m_maxPacketSize = local.GetParams().GetValueAlways( "maxPacketSize", CORE::CVariant( DRN_DEFAULT_DATAGRAM_SIZE ) ).AsUInt32( DRN_DEFAULT_DATAGRAM_SIZE );
    m_listenEndpoint = local;

    // a fresh cookie secret per Listen(): without randomness the cookies would be guessable, so no listening
    if ( m_statelessCookies && !CORE::CRandom::GetRandomBytes( m_cookieSecret, sizeof( m_cookieSecret ) ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "DRNUdpChannelListener: No randomness for the cookie secret, not listening on " << local.ToString() );
        return false;
    }

    CORE::CString host = local.GetHost();
    bool opened = host.IsNULLOrEmpty() ? m_socket->Open( port ) : m_socket->Open( COMCORE::CIPv4Address( host, port ) );
    if ( !opened )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNUdpChannelListener: Failed to listen on " << local.ToString() );
        return false;
    }
    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNUdpChannelListener: Listening on " << local.ToString() );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNUdpChannelListener::StopListening( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_socket && m_socket->IsActive() )
        m_socket->Close();
}

/*-------------------------------------------------------------------------*/

bool
CDRNUdpChannelListener::IsListening( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_socket && m_socket->IsActive();
}

/*-------------------------------------------------------------------------*/

CDRNEndpoint
CDRNUdpChannelListener::GetListenEndpoint( void ) const
{GUCEF_TRACE;

    return m_listenEndpoint;
}

/*-------------------------------------------------------------------------*/

void
CDRNUdpChannelListener::ApplySecurityPolicy( const CDRNSecurityPolicy& policy )
{GUCEF_TRACE;

    m_statelessCookies = policy.statelessCookies;
}

/*-------------------------------------------------------------------------*/

void
CDRNUdpChannelListener::SetStatelessCookies( bool enabled )
{GUCEF_TRACE;

    m_statelessCookies = enabled;
}

/*-------------------------------------------------------------------------*/

bool
CDRNUdpChannelListener::IsUsingStatelessCookies( void ) const
{GUCEF_TRACE;

    return m_statelessCookies;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNUdpChannelListener::GetCookieRetryCount( void ) const
{GUCEF_TRACE;

    return m_cookieRetries;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNUdpChannelListener::GetInvalidCookieCount( void ) const
{GUCEF_TRACE;

    return m_invalidCookies;
}

/*-------------------------------------------------------------------------*/

void
CDRNUdpChannelListener::MakeCookieMac( const COMCORE::CIPv4Address& source, UInt32 window, UInt8* mac ) const
{GUCEF_TRACE;

    UInt32 address = source.GetAddress();
    UInt16 port = source.GetPort();
    UInt8 input[ 4 + 2 + 4 ];
    input[ 0 ] = (UInt8) ( address >> 24 );
    input[ 1 ] = (UInt8) ( address >> 16 );
    input[ 2 ] = (UInt8) ( address >> 8 );
    input[ 3 ] = (UInt8) address;
    input[ 4 ] = (UInt8) ( port >> 8 );
    input[ 5 ] = (UInt8) port;
    input[ 6 ] = (UInt8) ( window >> 24 );
    input[ 7 ] = (UInt8) ( window >> 16 );
    input[ 8 ] = (UInt8) ( window >> 8 );
    input[ 9 ] = (UInt8) window;
    UInt8 full[ CORE::CSha256::DigestSize ];
    CORE::CHmacSha256::Mac( m_cookieSecret, sizeof( m_cookieSecret ), input, sizeof( input ), full );
    memcpy( mac, full, DRN_UDP_COOKIE_MAC_SIZE );
}

/*-------------------------------------------------------------------------*/

bool
CDRNUdpChannelListener::IsCookieValid( const COMCORE::CIPv4Address& source, const UInt8* cookie, UInt32 size ) const
{GUCEF_TRACE;

    if ( size < DRN_UDP_COOKIE_SIZE || DRN_UDP_COOKIE_ECHO != cookie[ 0 ] || DRN_UDP_COOKIE_VERSION != cookie[ 1 ] )
        return false;

    UInt32 window = ( (UInt32) cookie[ 2 ] << 24 ) | ( (UInt32) cookie[ 3 ] << 16 ) | ( (UInt32) cookie[ 4 ] << 8 ) | (UInt32) cookie[ 5 ];
    UInt32 now = (UInt32) ( CDRNClock::NowInMilliseconds() / DRN_UDP_COOKIE_WINDOW_MS );
    if ( window != now && window + 1 != now )
        return false;

    UInt8 mac[ DRN_UDP_COOKIE_MAC_SIZE ];
    MakeCookieMac( source, window, mac );
    return CORE::CSha256::ConstantTimeEquals( mac, cookie + 6, DRN_UDP_COOKIE_MAC_SIZE );
}

/*-------------------------------------------------------------------------*/

void
CDRNUdpChannelListener::SendHelloRetry( const COMCORE::CIPv4Address& source )
{GUCEF_TRACE;

    UInt32 window = (UInt32) ( CDRNClock::NowInMilliseconds() / DRN_UDP_COOKIE_WINDOW_MS );
    UInt8 retry[ DRN_UDP_COOKIE_SIZE ];
    retry[ 0 ] = DRN_UDP_HELLO_RETRY;
    retry[ 1 ] = DRN_UDP_COOKIE_VERSION;
    retry[ 2 ] = (UInt8) ( window >> 24 );
    retry[ 3 ] = (UInt8) ( window >> 16 );
    retry[ 4 ] = (UInt8) ( window >> 8 );
    retry[ 5 ] = (UInt8) window;
    MakeCookieMac( source, window, retry + 6 );
    ++m_cookieRetries;
    SendTo( source, retry, sizeof( retry ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNUdpChannelListener::GetChannelType( void ) const
{GUCEF_TRACE;

    return CDRNEndpoint::ChannelTypeUdp;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNUdpChannelListener::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNUdpChannelListener";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNUdpChannelListener::GetChannelCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_channels.size();
}

/*-------------------------------------------------------------------------*/

COMCORE::CUDPSocket*
CDRNUdpChannelListener::GetSocket( void ) const
{GUCEF_TRACE;

    return m_socket;
}

/*-------------------------------------------------------------------------*/

bool
CDRNUdpChannelListener::SendTo( const COMCORE::CIPv4Address& remote, const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_socket || !m_socket->IsActive() )
        return false;
    return m_socket->SendPacketTo( remote, data, (UInt16) size ) >= 0;
}

/*-------------------------------------------------------------------------*/

void
CDRNUdpChannelListener::OnChannelClosed( CDRNUdpChannel* channel )
{GUCEF_TRACE;

    if ( m_lastChannel == channel )
        m_lastChannel = GUCEF_NULL;
    TChannelMap::iterator i = m_channels.find( channel->m_remoteAddress );
    if ( i != m_channels.end() && (*i).second == channel )
        m_channels.erase( i );
}

/*-------------------------------------------------------------------------*/

void
CDRNUdpChannelListener::OnDatagram( const COMCORE::CIPv4Address& source, const CORE::CDynamicBuffer& data )
{GUCEF_TRACE;

    CDRNUdpChannel* channel = GUCEF_NULL;
    if ( GUCEF_NULL != m_lastChannel && source == m_lastSource )
    {
        channel = m_lastChannel;
    }
    else
    {
        TChannelMap::iterator i = m_channels.find( source );
        if ( i != m_channels.end() )
            channel = (*i).second;
    }

    const UInt8* bytes = static_cast< const UInt8* >( data.GetConstBufferPtr() );
    UInt32 size = data.GetDataSize();
    if ( 0 == size || GUCEF_NULL == bytes )
        return;

    if ( GUCEF_NULL != channel )
    {
        m_lastSource = source;
        m_lastChannel = channel;

        // the connecting side keeps the cookie in front until it hears from us
        if ( DRN_UDP_COOKIE_ECHO == bytes[ 0 ] )
        {
            if ( size > DRN_UDP_COOKIE_SIZE )
                channel->OnPacket( bytes + DRN_UDP_COOKIE_SIZE, size - DRN_UDP_COOKIE_SIZE );
            return;
        }
        channel->OnPacket( bytes, size );
        return;
    }

    if ( !m_statelessCookies )
    {
        // a new peer, only for something that looks like a DRN packet
        if ( DRN_PROTOCOL_ID == bytes[ 0 ] )
            AcceptChannel( source, bytes, size );
        return;
    }

    // stateless: a DRN packet of an unknown address gets a cookie, no more than it sent
    if ( DRN_PROTOCOL_ID == bytes[ 0 ] )
    {
        if ( size >= DRN_UDP_COOKIE_SIZE )
            SendHelloRetry( source );
        return;
    }

    // only the address that got the cookie can send it back
    if ( DRN_UDP_COOKIE_ECHO == bytes[ 0 ] )
    {
        if ( size <= DRN_UDP_COOKIE_SIZE || DRN_PROTOCOL_ID != bytes[ DRN_UDP_COOKIE_SIZE ] || !IsCookieValid( source, bytes, size ) )
        {
            ++m_invalidCookies;
            return;
        }
        AcceptChannel( source, bytes + DRN_UDP_COOKIE_SIZE, size - DRN_UDP_COOKIE_SIZE );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNUdpChannelListener::AcceptChannel( const COMCORE::CIPv4Address& source, const UInt8* packet, UInt32 packetSize )
{GUCEF_TRACE;

    CDRNUdpChannel* newChannel = GUCEF_NEW CDRNUdpChannel( this, source, m_pulseGenerator, m_maxPacketSize );
    CIDRNChannelPtr channelPtr( newChannel );
    m_channels[ source ] = newChannel;
    m_lastSource = source;
    m_lastChannel = newChannel;

    CDRNChannelEventData channelData( channelPtr );
    if ( !NotifyObservers( ChannelAcceptedEvent, &channelData ) )
        return;

    // the first datagram goes to whoever took the channel, if nobody did it is closed when the pointer goes
    if ( newChannel->IsConnected() )
        newChannel->OnPacket( packet, packetSize );
}

/*-------------------------------------------------------------------------*/

void
CDRNUdpChannelListener::OnNotify( CORE::CNotifier* notifier                 ,
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
            if ( entry.dataBuffer.IsNULL() )
                continue;
            OnDatagram( entry.sourceAddress, entry.dataBuffer.GetData() );
        }
    }
    else
    if ( COMCORE::CUDPSocket::UDPSocketErrorEvent == eventid )
    {
        CDRNChannelErrorEventData errorData( "udp socket error" );
        NotifyObservers( ListenerErrorEvent, &errorData );
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
