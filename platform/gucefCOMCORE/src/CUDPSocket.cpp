/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_CORE_DVSTRUTILS_H
#include "dvstrutils.h"
#define GUCEF_CORE_DVSTRUTILS_H
#endif /* GUCEF_CORE_DVSTRUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_COMCORE_SOCKETUTILS_H
#include "socketutils.h"
#define GUCEF_COMCORE_SOCKETUTILS_H
#endif /* GUCEF_COMCORE_SOCKETUTILS_H ? */

#include "CUDPSocket.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CUDPSocket::UDPSocketErrorEvent = "GUCEF::COMCORE::CUDPSocket::UDPSocketErrorEvent";
const CORE::CEvent CUDPSocket::UDPSocketClosingEvent = "GUCEF::COMCORE::CUDPSocket::UDPSocketClosingEvent";
const CORE::CEvent CUDPSocket::UDPSocketClosedEvent = "GUCEF::COMCORE::CUDPSocket::UDPSocketClosedEvent";
const CORE::CEvent CUDPSocket::UDPSocketOpenedEvent = "GUCEF::COMCORE::CUDPSocket::UDPSocketOpenedEvent";
const CORE::CEvent CUDPSocket::UDPPacketsRecievedEvent = "GUCEF::COMCORE::CUDPSocket::UDPPacketsRecievedEvent";

#define GUCEF_PULSE_UPDATE_INTERVAL                         25     // We use 25 as the magic number to allow for some real yielding
#define GUCEF_DEFAULT_UDP_RECEIVE_PACKET_BUFFER_SIZE        9216   // Set a default buffer size large enough to handle Jumbo frames
#define GUCEF_DEFAULT_UDP_OS_LEVEL_RECEIVE_BUFFER_SIZE      65535  // Set the default OS level buffer a little more generous then historical defaults

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Forward declarations of types used
 */
struct CUDPSocket::SUDPSockData
{
    SOCKET sockid;
    struct sockaddr_in localaddress;
    struct timeval timeout;         /* timeout for blocking operations */

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    std::vector< struct mmsghdr > msgs;
    std::vector< struct iovec > iovecs;
    #endif
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CUDPSocket::CUDPSocket( CORE::CPulseGenerator& pulseGenerator ,
                        bool blocking                         )
    : CSocket()
    , m_autoReopenOnError( false )
    , _blocking( blocking )
    , _data( NULL )
    , m_hostAddress()
    , m_underlyingReceiveBuffer()
    , m_receiveBuffers()
    , m_udpPacketsReceivedEventData()
    , m_maxRecievedDataBufferSize( GUCEF_DEFAULT_UDP_RECEIVE_PACKET_BUFFER_SIZE )
    , m_pulseGenerator( &pulseGenerator )
    , m_maxUpdatesPerCycle( 100 )
    , m_allowMulticastLoopback( false )
    , m_multicastTTL( 8 )
    , m_allowBroadcast( false )
    , m_osLevelSocketReceiveBufferSize( GUCEF_DEFAULT_UDP_OS_LEVEL_RECEIVE_BUFFER_SIZE )
    , m_bytesReceived( 0 )
    , m_bytesTransmitted( 0 )
    , m_nrOfDataReceivedEvents( 0 )
    , m_nrOfDataSentEvents( 0 )
{GUCEF_TRACE;

    RegisterEvents();

    _data = new TUDPSockData;
    memset( _data, 0, sizeof( TUDPSockData ) );

    SetNrOfReceiveBuffers( 1, GUCEF_DEFAULT_UDP_RECEIVE_PACKET_BUFFER_SIZE );

    TEventCallback callback( this, &CUDPSocket::OnPulse );
    SubscribeTo( m_pulseGenerator                  ,
                 CORE::CPulseGenerator::PulseEvent ,
                 callback                          );
    TEventCallback callback2( this, &CUDPSocket::OnPulseGeneratorDestruction );
    SubscribeTo( m_pulseGenerator                        ,
                 CORE::CPulseGenerator::DestructionEvent ,
                 callback2                               );
}

/*-------------------------------------------------------------------------*/

CUDPSocket::CUDPSocket( bool blocking )
    : CSocket()
    , m_autoReopenOnError( false )
    , _blocking( blocking )
    , _data( NULL )
    , m_hostAddress()
    , m_underlyingReceiveBuffer()
    , m_receiveBuffers()
    , m_udpPacketsReceivedEventData()
    , m_maxRecievedDataBufferSize( GUCEF_DEFAULT_UDP_RECEIVE_PACKET_BUFFER_SIZE )
    , m_pulseGenerator( &CORE::CCoreGlobal::Instance()->GetPulseGenerator() )
    , m_maxUpdatesPerCycle( 100 )
    , m_allowMulticastLoopback( false )
    , m_multicastTTL( 8 )
    , m_allowBroadcast( false )
    , m_osLevelSocketReceiveBufferSize( GUCEF_DEFAULT_UDP_OS_LEVEL_RECEIVE_BUFFER_SIZE )
    , m_bytesReceived( 0 )
    , m_bytesTransmitted( 0 )
    , m_nrOfDataReceivedEvents( 0 )
    , m_nrOfDataSentEvents( 0 )
{GUCEF_TRACE;

    RegisterEvents();

    _data = new TUDPSockData;
    memset( _data, 0, sizeof( TUDPSockData ) );

    SetNrOfReceiveBuffers( 1, GUCEF_DEFAULT_UDP_RECEIVE_PACKET_BUFFER_SIZE );

    TEventCallback callback( this, &CUDPSocket::OnPulse );
    SubscribeTo( m_pulseGenerator                  ,
                 CORE::CPulseGenerator::PulseEvent ,
                 callback                          );
    TEventCallback callback2( this, &CUDPSocket::OnPulseGeneratorDestruction );
    SubscribeTo( m_pulseGenerator                        ,
                 CORE::CPulseGenerator::DestructionEvent ,
                 callback2                               );
}

/*-------------------------------------------------------------------------*/

CUDPSocket::~CUDPSocket()
{GUCEF_TRACE;

    Close( true );

    delete _data;
    _data = NULL;
}

/*-------------------------------------------------------------------------*/

void
CUDPSocket::RegisterEvents( void )
{GUCEF_TRACE;

    UDPSocketErrorEvent.Initialize();
    UDPSocketClosingEvent.Initialize();
    UDPSocketClosedEvent.Initialize();
    UDPSocketOpenedEvent.Initialize();
    UDPPacketsRecievedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::SetNrOfReceiveBuffers( UInt32 nrOfReceiveBuffers ,
                                   UInt32 newBufferSize      )
{GUCEF_TRACE;

    // Having multiple receive buffers is not supported for blocking sockets at the moment
    if ( _blocking && nrOfReceiveBuffers > 1 )
        return false;

    if ( 0 == nrOfReceiveBuffers )
    {
        nrOfReceiveBuffers = 1;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "):SetNrOfReceiveBuffers: Attempt to set nrOfReceiveBuffers to 0, this is not allowed. Forcing to 1" );
    }

    if ( !m_underlyingReceiveBuffer.SetBufferSize( nrOfReceiveBuffers * newBufferSize ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "):SetNrOfReceiveBuffers: Failed to allocate enough memory for " + CORE::UInt32ToString( nrOfReceiveBuffers ) +
            " receive buffers with each buffer having a size of " + CORE::UInt32ToString( newBufferSize ) + " bytes."
            " Total required bytes for underlying buffer in bytes = " + CORE::UInt32ToString( nrOfReceiveBuffers * newBufferSize ) );
        return false;
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "):SetNrOfReceiveBuffers: Successfully allocated enough memory for " + CORE::UInt32ToString( nrOfReceiveBuffers ) +
        " receive buffers with each buffer having a size of " + CORE::UInt32ToString( newBufferSize ) + " bytes."
        " Total required bytes for underlying buffer in bytes = " + CORE::UInt32ToString( nrOfReceiveBuffers * newBufferSize ) );

    m_maxRecievedDataBufferSize = newBufferSize;

    TUdpPacketsRecievedEventData& infoStruct = m_udpPacketsReceivedEventData.GetData();
    infoStruct.packets.resize( nrOfReceiveBuffers );
    infoStruct.packetsReceived = 0;

    m_receiveBuffers.resize( nrOfReceiveBuffers );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    _data->msgs.resize( nrOfReceiveBuffers );
    _data->iovecs.resize( nrOfReceiveBuffers );
    #endif

    for ( UInt32 i=0; i<m_receiveBuffers.size(); ++i )
    {
        CORE::CDynamicBuffer& linkedBuffer = m_receiveBuffers[ i ];
        linkedBuffer.LinkTo( m_underlyingReceiveBuffer.GetBufferPtr( i * newBufferSize ), newBufferSize );
        infoStruct.packets[ i ].dataBuffer.LinkTo( &linkedBuffer );

        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
        memset( &_data->iovecs[ i ], 0, sizeof( _data->iovecs[ i ] ) );
        _data->iovecs[ i ].iov_base         = m_underlyingReceiveBuffer.GetBufferPtr( i * newBufferSize );
        _data->iovecs[ i ].iov_len          = newBufferSize;
        memset( &_data->msgs[ i ], 0, sizeof( _data->msgs[ i ] ) );
        _data->msgs[ i ].msg_hdr.msg_iov    = &_data->iovecs[ i ];
        _data->msgs[ i ].msg_hdr.msg_iovlen = 1;
        #endif

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "):SetNrOfReceiveBuffers: Linked logical receive buffer " +
                CORE::UInt32ToString( i+1 ) + "/" + CORE::UInt32ToString( nrOfReceiveBuffers ) );
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::SetNrOfReceiveBuffers( const UInt32 nrOfReceiveBuffers )
{GUCEF_TRACE;

    return SetNrOfReceiveBuffers( nrOfReceiveBuffers, GetRecievedDataBufferSize() );
}

/*-------------------------------------------------------------------------*/

UInt32
CUDPSocket::GetNrOfReceiveBuffers( void ) const
{GUCEF_TRACE;

    return (UInt32) m_receiveBuffers.size();
}

/*-------------------------------------------------------------------------*/

Int32
CUDPSocket::SendPacketTo( const CIPAddress& dest ,
                          const void* data       ,
                          UInt16 datasize        )
{GUCEF_TRACE;

    if ( !_data->sockid )
    {
        Open();
    }
    return const_cast< const CUDPSocket* >( this )->SendPacketTo( dest     ,
                                                                  data     ,
                                                                  datasize );
}

/*-------------------------------------------------------------------------*/

Int32
CUDPSocket::SendPacketTo( const CIPAddress& dest ,
                          const void* data       ,
                          UInt16 datasize        ) const
{GUCEF_TRACE;

    struct sockaddr_in remote;
    memset( &remote, 0, sizeof( remote ) );
    UInt32 destAddr = dest.GetAddress();
    if ( 0 == destAddr )
        destAddr = CIPAddress::LoopbackIP.GetAddress();
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    remote.sin_addr.S_un.S_addr = destAddr;
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    remote.sin_addr.s_addr = destAddr;
    #endif
    remote.sin_port = dest.GetPort();
    remote.sin_family = AF_INET;

    int socketError = 0;
    Int32 returnValue = dvsocket_sendto( _data->sockid             ,
                                         (const char*)data         ,
                                         datasize                  ,
                                         0                         ,
                                         (struct sockaddr*)&remote ,
                                         sizeof(remote)            ,
                                         &socketError              );

    if ( returnValue >= 0 )
    {
        m_bytesTransmitted += returnValue;
        ++m_nrOfDataSentEvents;

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "):SendPacketTo: Sent " + CORE::UInt16ToString( datasize ) +
            " bytes to " + dest.AddressAndPortAsString() + " from " + m_hostAddress.AddressAndPortAsString() );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "):SendPacketTo: Failed to send " + CORE::UInt16ToString( datasize ) +
            " bytes to " + dest.AddressAndPortAsString()+ " from " + m_hostAddress.AddressAndPortAsString() + ". ErrorCode: " + CORE::Int32ToString( socketError ) );

        TSocketErrorEventData eData( socketError );
        NotifyObservers( UDPSocketErrorEvent, &eData );
    }
    return returnValue;
}

/*-------------------------------------------------------------------------*/

Int32
CUDPSocket::SendPacketTo( const CORE::CString& dnsname ,
                          UInt16 port                  ,
                          const void* data             ,
                          UInt16 datasize              )
{GUCEF_TRACE;

    CIPAddress dest;
    if ( ConvertToIPAddress( dnsname, port, dest ) )
    {
        return SendPacketTo( dest, data, datasize );
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::IsIncomingDataQueued( void )
{GUCEF_TRACE;

    return Update( false );
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::Update( bool performRead )
{GUCEF_TRACE;

    if ( !_blocking && _data->sockid > 0 )
    {
        fd_set readfds;      /* Setup the read variable for the select function */
        fd_set exceptfds;    /* Setup the except variable for the select function */
        int errorCode = 0;

        TUdpPacketsRecievedEventData& eventData = m_udpPacketsReceivedEventData.GetData();
        UInt32 maxPackets = 1;
        if ( performRead )
        {
            maxPackets = (UInt32) m_receiveBuffers.size();
        }

        eventData.packetsReceived = 0;
        UInt32 i=0;
        for ( ; i<maxPackets; ++i )
        {
            FD_ZERO( &readfds );
            FD_ZERO( &exceptfds );
            FD_SET( _data->sockid, &readfds );
            FD_SET( _data->sockid, &exceptfds );

            if ( dvsocket_select( (int)_data->sockid+1 ,
                                  &readfds             ,
                                  NULL                 , // We don't care about socket writes here
                                  &exceptfds           ,
                                  &_data->timeout      ,
                                  &errorCode           ) != SOCKET_ERROR )
            {
                // something happened on the socket

                if ( FD_ISSET( _data->sockid, &exceptfds ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CUDPSocket(" + CORE::PointerToString( this ) + "): Socket error occured: " +
                            CORE::Int32ToString( errorCode ) );

                    int closeError;
                    dvsocket_closesocket( _data->sockid, &closeError );
                    _data->sockid = 0;
                    break;
                }
                else
                if ( FD_ISSET( _data->sockid, &readfds ) )
                {
                    // data can be read from the socket
                    // This is why we loop, to batch multiple reads per cycle
                    if ( performRead )
                    {
                        UInt32 packetsRead = 0;
                        if ( !PerformRead( i, packetsRead ) || 0 == packetsRead )
                            break;

                        // In some cases we can read multiple packets in a single read operation
                        // not all operating systems support this. For those that do packetsRead can be > 1
                        i += ( packetsRead - 1 );

                        eventData.packetsReceived += packetsRead;
                    }
                }
                else
                {
                    // Nothing more to do on this socket
                    break;
                }
            }
            else
            {
                // select call failed
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "UDPSocket(" + CORE::PointerToString( this ) + "): Socket error occured (select call failed) on socket " +
                    m_hostAddress.AddressAndPortAsString() + ". Error code: " + CORE::Int32ToString( errorCode ) );

                int closeError;
                dvsocket_closesocket( _data->sockid, &closeError );
                _data->sockid = 0;
                break;
            }
        }

        if ( eventData.packetsReceived > 0 )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "):Update: Received " + CORE::UInt32ToString( eventData.packetsReceived ) + " packets in one cycle" );

            // Send an event notifying people that the data was recieved
            if ( !NotifyObservers( UDPPacketsRecievedEvent, &m_udpPacketsReceivedEventData ) ) return false;

            // Sanity check to enure nobody messed with our structure
            // The linking between these is for performance and should not be messed with
            assert( eventData.packets.size() == m_receiveBuffers.size() );
        }

        if ( 0 != errorCode )
        {
            TSocketErrorEventData eData( errorCode );
            if ( !NotifyObservers( UDPSocketErrorEvent, &eData ) ) return false;

            if ( m_autoReopenOnError )
                Open();
        }

        if ( i >= maxPackets )
        {
            // We reached out max bulk operation size per notification
            // However there is more work to do
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "):Update: Max operations per notification cycle reached for the current nr of read buffers" );
            return true;
        }
        else
        {
            // no more work, thus false
            return false;
        }
    }

    // Not applicable thus no work to do
    return false;
}

/*-------------------------------------------------------------------------*/

void
CUDPSocket::SetMaxUpdatesPerCycle( UInt32 maxUpdates )
{GUCEF_TRACE;

    m_maxUpdatesPerCycle = maxUpdates;
}

/*-------------------------------------------------------------------------*/


UInt32
CUDPSocket::GetMaxUpdatesPerCycle( void ) const
{GUCEF_TRACE;

    return m_maxUpdatesPerCycle;
}

/*-------------------------------------------------------------------------*/

void
CUDPSocket::OnPulse( CORE::CNotifier* notifier                 ,
                     const CORE::CEvent& eventid               ,
                     CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( m_maxUpdatesPerCycle > 0 )
    {
        UInt32 cycleCount = 0;
        while( ( cycleCount < m_maxUpdatesPerCycle ) && Update( true ) )
        {
            ++cycleCount;
        }
        if ( cycleCount >= m_maxUpdatesPerCycle )
        {
            // This is a busy socket, don't yield to the scheduler
            m_pulseGenerator->RequestImmediatePulse();
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "):OnPulse: Max update cycles per pulse reached, will request an immediate pulse (CPU burst mode)" );
        }
    }
    else
    {
        while( Update( true ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CUDPSocket::OnPulseGeneratorDestruction( CORE::CNotifier* notifier                 ,
                                         const CORE::CEvent& eventid               ,
                                         CORE::CICloneable* eventdata /* = NULL */ )

{GUCEF_TRACE;

    if ( notifier == m_pulseGenerator )
    {
        m_pulseGenerator = NULL;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CUDPSocket::GetRecievedDataBufferSize( void ) const
{GUCEF_TRACE;

    return m_maxRecievedDataBufferSize;
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::SetRecievedDataBufferSize( const UInt32 newBufferSize )
{GUCEF_TRACE;

    return SetNrOfReceiveBuffers( GetNrOfReceiveBuffers(), newBufferSize );
}

/*-------------------------------------------------------------------------*/

void
CUDPSocket::SetAllowBroadcast( bool allowBroadcast )
{GUCEF_TRACE;

    m_allowBroadcast = allowBroadcast;
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::GetAllowBroadcast( void ) const
{GUCEF_TRACE;

    return m_allowBroadcast;
}

/*-------------------------------------------------------------------------*/

void
CUDPSocket::SetAllowMulticastLoopback( bool allowLoopback )
{GUCEF_TRACE;

    m_allowMulticastLoopback = allowLoopback;
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::GetAllowMulticastLoopback( void ) const
{GUCEF_TRACE;

    return m_allowMulticastLoopback;
}

/*-------------------------------------------------------------------------*/

void
CUDPSocket::SetMulticastTTL( Int32 ttl )
{GUCEF_TRACE;

    m_multicastTTL = ttl;
}

/*-------------------------------------------------------------------------*/

Int32
CUDPSocket::GetMulticastTTL( void ) const
{GUCEF_TRACE;

    return m_multicastTTL;
}

/*-------------------------------------------------------------------------*/

UInt32
CUDPSocket::GetNrOfDataReceivedEvents( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        UInt32 nrOfDataReceivedEvents = m_nrOfDataReceivedEvents;
        m_nrOfDataReceivedEvents = 0;
        return nrOfDataReceivedEvents;
    }
    else
        return m_nrOfDataReceivedEvents;
}

/*-------------------------------------------------------------------------*/

UInt32
CUDPSocket::GetNrOfDataSentEvents( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        UInt32 nrOfDataSentEvents = m_nrOfDataSentEvents;
        m_nrOfDataSentEvents = 0;
        return nrOfDataSentEvents;
    }
    else
        return m_nrOfDataSentEvents;
}

/*-------------------------------------------------------------------------*/

UInt32
CUDPSocket::GetBytesReceived( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        UInt32 bytesReceived = m_bytesReceived;
        m_bytesReceived = 0;
        return bytesReceived;
    }
    else
        return m_bytesReceived;
}

/*-------------------------------------------------------------------------*/

UInt32
CUDPSocket::GetBytesTransmitted( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        UInt32 bytesTransmitted = m_bytesTransmitted;
        m_bytesTransmitted = 0;
        return bytesTransmitted;
    }
    else
        return m_bytesTransmitted;
}

/*-------------------------------------------------------------------------*/

Int32
CUDPSocket::Recieve( void )
{GUCEF_TRACE;

    return Recieve( nullptr, 0 );
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::PerformRead( void )
{GUCEF_TRACE;

    TUdpPacketsRecievedEventData& eventData = m_udpPacketsReceivedEventData.GetData();
    eventData.packetsReceived = 0;

    UInt32 packetsRead = 0;
    return PerformRead( 0, packetsRead );
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::PerformRead( UInt32 readIndex, UInt32& packetsRead )
{GUCEF_TRACE;

    TUdpPacketsRecievedEventData& eventData = m_udpPacketsReceivedEventData.GetData();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    UInt32 maxPackets = (UInt32) m_receiveBuffers.size() - readIndex;

    struct timespec timeout;
    memset( &timeout, 0, sizeof( timeout ) );
    int retval = recvmmsg( _data->sockid, &_data->msgs[ readIndex ], maxPackets, 0, &timeout );
    if ( retval < 0 )
    {
        int sockError = errno;
        if ( EWOULDBLOCK == sockError )
        {
            // Not an error
            packetsRead = 0;
            return false;
        }
        if ( ENETRESET == sockError )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "):PerformRead: Discovered the socket was closed gracefully on socket " +
                m_hostAddress.AddressAndPortAsString() );

            packetsRead = 0;
            Close( false );
            return false;
        }

        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "UDPSocket(" + CORE::PointerToString( this ) + "):PerformRead: Socket error occured (recvmmsg call failed) on socket " +
            m_hostAddress.AddressAndPortAsString() + ". Error code: " + CORE::Int32ToString( errno ) );

        packetsRead = 0;
        return false;
    }
    packetsRead = (UInt32) retval;

    UInt32 maxIndex = readIndex + packetsRead;
    for ( UInt32 i=readIndex; i<maxIndex; ++i )
    {
        // Set the actual usefull bytes of data in the buffer to match the number of retrieved bytes
        // We achieve this by linking. The data is retained in the underlying buffer.
        // We are merely setting the demarkation of the logical buffer into the underlying buffer
        CORE::CDynamicBuffer& bufferLink = m_receiveBuffers[ i ];
        bufferLink.LinkTo( m_underlyingReceiveBuffer.GetBufferPtr( i * m_maxRecievedDataBufferSize ), (UInt32) _data->msgs[ i ].msg_len );

        // Fill in the remote source address from where the data originated
        CIPAddress& sourceAddress = eventData.packets[ i ].sourceAddress;

        // The msg_name field of struct msghdr does not necessarily have to point to a struct sockaddr_in:
        // it points to a generic socket address; the exact structure depends on the socket family: if it's an AF_UNIX socket,
        // it points to a struct sockaddr_un, if it's AF_INET, it points to struct sockaddr_in, and if it's AF_INET6 it points to struct sockaddr_in6.
        // All of them share the sa_family field, which is also the first field, so you can cast msg_name to struct sockaddr *, read the sa_family field,
        // and decide where to go from there.
        //
        // This specific UDP class only allows creation of one type of port so we can cast accordingly
        struct sockaddr_in* remote = (struct sockaddr_in*) _data->msgs[ i ].msg_hdr.msg_name;
        if ( GUCEF_NULL != remote && sizeof( struct sockaddr_in ) == _data->msgs[ i ].msg_hdr.msg_namelen )
        {
            sourceAddress.SetPort( remote->sin_port );
            sourceAddress.SetAddress( remote->sin_addr.s_addr );
        }

        // Update our metrics
        m_bytesReceived += _data->msgs[ i ].msg_len;
        ++m_nrOfDataReceivedEvents;
    }

    return true;

    #else

    int sockError = 0;
    struct sockaddr_in remote;
    socklen_t structsize( sizeof( remote ) );

    // Carry out the receive which copies from the OS buffer into our application memory's buffer
    int retval = dvsocket_recvfrom( _data->sockid                          ,
                                    m_underlyingReceiveBuffer.GetBufferPtr( readIndex * m_maxRecievedDataBufferSize ) ,
                                    m_maxRecievedDataBufferSize            ,
                                    0                                      ,
                                    (struct sockaddr*)&remote              ,
                                    &structsize                            ,
                                    &sockError                             );
    if ( retval < 0 )
    {
        // recvfrom call failed
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "UDPSocket(" + CORE::PointerToString( this ) + "):PerformRead: Socket error occured (recvfrom call failed) on socket " +
            m_hostAddress.AddressAndPortAsString() + ". Error code: " + CORE::Int32ToString( sockError ) );

        packetsRead = 0;
        return false;
    }
    if ( retval == 0 )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "):PerformRead: Discovered the socket was closed gracefully on socket " +
            m_hostAddress.AddressAndPortAsString() );

        // The read is telling us that the socket was closed
        packetsRead = 0;
        Close( false );
        return false;
    }

    // Set the actual usefull bytes of data in the buffer to match the number of retrieved bytes
    // We achieve this by linking. The data is retained in the underlying buffer.
    // We are merely setting the demarkation of the logical buffer into the underlying buffer
    CORE::CDynamicBuffer& bufferLink = m_receiveBuffers[ readIndex ];
    bufferLink.LinkTo( m_underlyingReceiveBuffer.GetBufferPtr( readIndex * m_maxRecievedDataBufferSize ), (UInt32) retval );

    // Fill in the remote source address from where the data originated
    CIPAddress& sourceAddress = eventData.packets[ readIndex ].sourceAddress;
    sourceAddress.SetPort( remote.sin_port );
    sourceAddress.SetAddress( remote.sin_addr.s_addr );

    packetsRead = 1;

    // Update our metrics
    m_bytesReceived += retval;
    ++m_nrOfDataReceivedEvents;

    return true;

    #endif
}

/*-------------------------------------------------------------------------*/

Int32
CUDPSocket::Recieve( CIPAddress& src ,
                     void* destbuf   ,
                     UInt16 bufsize  )
{GUCEF_TRACE;

    // When using this API enforce the historical default for now
    // This is for blocking sockets only
    SetNrOfReceiveBuffers( 1 );

    if ( !m_receiveBuffers.empty() )
    {
        if ( PerformRead() )
        {
            TPacketEntry& entry = m_udpPacketsReceivedEventData.GetData().packets.front();
            src = entry.sourceAddress;
            return (Int32) entry.dataBuffer.GetData().CopyTo( 0, bufsize, destbuf );
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

Int32
CUDPSocket::Recieve( void* destbuf  ,
                     UInt16 bufsize )
{GUCEF_TRACE;

    CIPAddress src;
    return Recieve( src     ,
                    destbuf ,
                    bufsize );
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::Open( void )
{GUCEF_TRACE;

    return Open( m_hostAddress );
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::Open( UInt16 port )
{GUCEF_TRACE;

    CIPAddress address( INADDR_ANY, htons( port ) );
    return Open( address );
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::Open( const CORE::CString& localaddr ,
                  UInt16 port                    )
{GUCEF_TRACE;

    CHostAddress address( localaddr, port );
    bool success = Open( address );
    m_hostAddress = address;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::Open( const CIPAddress& localaddr )
{GUCEF_TRACE;

    Close( false );

    m_hostAddress = localaddr;

    //create a UDP socket
    int errorCode = 0;
    if ( ( _data->sockid = dvsocket_socket( AF_INET     ,
                                            SOCK_DGRAM  ,
                                            IPPROTO_UDP ,
                                            &errorCode  ) ) == INVALID_SOCKET )
    {
        TSocketErrorEventData eData( errorCode );
        if ( !NotifyObservers( UDPSocketErrorEvent, &eData ) ) return false;

        if ( m_autoReopenOnError )
            m_pulseGenerator->RequestPeriodicPulses( this, GUCEF_PULSE_UPDATE_INTERVAL );

        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed to open socket at " +
            m_hostAddress.AddressAndPortAsString() + ". Error code: " + CORE::Int32ToString( errorCode ) );
        return false;
    }

    // Set the desired blocking mode
    if ( !SetBlockingMode( _data->sockid, _blocking ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed to set blocking mode \"" +
            CORE::BoolToString( _blocking ) + "\" on socket" );

        if ( m_autoReopenOnError )
            m_pulseGenerator->RequestPeriodicPulses( this, GUCEF_PULSE_UPDATE_INTERVAL );

        return false;
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Successfully set blocking mode \"" +
        CORE::BoolToString( _blocking ) + "\" on socket" );

    int allowAddressReuse = 1;
    if ( 0 > dvsocket_setsockopt( _data->sockid, SOL_SOCKET, SO_REUSEADDR, (const char*) &allowAddressReuse, sizeof(int), &errorCode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed to set address reuse mode \"" + CORE::BoolToString( allowAddressReuse != 0 )
            + "\" on socket. Error code: " + CORE::UInt32ToString( errorCode ) );
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Successfully set address reuse mode \"" + CORE::BoolToString( allowAddressReuse != 0 ) + "\" on socket" );

    #ifdef SO_REUSEPORT
    int allowPortReuse = 1;
    if ( 0 > dvsocket_setsockopt( _data->sockid, SOL_SOCKET, SO_REUSEPORT, (const char*) &allowPortReuse, sizeof(int), &errorCode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed to set port reuse mode \"" + CORE::BoolToString( allowPortReuse != 0 )
            + "\" on socket. Error code: " + CORE::UInt32ToString( errorCode ) );
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Successfully set port reuse mode \"" + CORE::BoolToString( allowPortReuse != 0 ) + "\" on socket" );
    #endif

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    if ( !_blocking )
    {
        // Important: SO_RCVTIMEO must be set when using non-blocking or recvmmsg will block until data is received regardless
        // of the overall socket blocking mode
        struct timeval receiveTimeout;
        memset( &receiveTimeout, 0, sizeof( receiveTimeout ) );
        if ( 0 > dvsocket_setsockopt( _data->sockid, SOL_SOCKET, SO_RCVTIMEO, (const char*) &receiveTimeout, sizeof(receiveTimeout), &errorCode ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed to set receive timeout \"0\" on socket"
                + ". Error code: " + CORE::UInt32ToString( errorCode ) );
        }
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Successfully set receive timeout \"0\" on socket" );
    }
    #endif

    SetOsLevelSocketReceiveBufferSize( m_osLevelSocketReceiveBufferSize );

    Int32 loopch = m_allowMulticastLoopback ? (Int32)1 : (Int32)0;
    if ( 0 > dvsocket_setsockopt( _data->sockid, IPPROTO_IP, IP_MULTICAST_LOOP, (const char*) &loopch, sizeof(loopch), &errorCode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed to set multicast loopback (dis)allowed mode \"" + CORE::BoolToString( loopch != 0 )
            + "\" on socket. Error code: " + CORE::UInt32ToString( errorCode ) );
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Successfully set multicast loopback mode \"" + CORE::BoolToString( loopch != 0 ) + "\" on socket" );

    if ( 0 > dvsocket_setsockopt( _data->sockid, IPPROTO_IP, IP_MULTICAST_TTL, (const char*) &m_multicastTTL, sizeof(m_multicastTTL), &errorCode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed to set multicast TTL to " + CORE::Int32ToString( m_multicastTTL )
            + "\" on socket. Error code: " + CORE::UInt32ToString( errorCode ) );
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Successfully set multicast TTL to \"" + CORE::Int32ToString( m_multicastTTL ) + "\" on socket" );

    // This option is needed on the socket in order to be able to transmit broadcast messages
    // If not set the socket will not be able to transmit broadcast messages in the local network subnet.
    Int32 broadcastOption = m_allowBroadcast ? 1 : 0;
    if ( 0 > dvsocket_setsockopt( _data->sockid, SOL_SOCKET, SO_BROADCAST, (const char*) &broadcastOption, sizeof(broadcastOption), &errorCode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed to set broadcast option to " + CORE::BoolToString( m_allowBroadcast )
            + "\" on socket. Error code: " + CORE::UInt32ToString( errorCode ) );
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Successfully set broadcast option to \"" + CORE::BoolToString( m_allowBroadcast ) + "\" on socket" );

    _data->localaddress.sin_family = AF_INET;
    _data->localaddress.sin_port = m_hostAddress.GetPort();
    _data->localaddress.sin_addr.s_addr = m_hostAddress.GetAddress();

    SetDefaultMulticastInterface( m_hostAddress );

    if ( dvsocket_bind( _data->sockid                            ,
                        (struct sockaddr *) &_data->localaddress ,
                        sizeof(struct sockaddr_in)               ,
                        &errorCode                               ) == 0 )
    {
        // Did we ask the O/S to assign a port?
        if ( 0 == m_hostAddress.GetPortInHostByteOrder() )
        {
            struct sockaddr_in sin;
            socklen_t len = sizeof(sin);
            if ( 0 == getsockname( _data->sockid, (struct sockaddr*) &sin, &len ) )
                m_hostAddress.SetPort( sin.sin_port );
        }

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Successfully bound and opened socket at " + m_hostAddress.AddressAndPortAsString()
            + " aka " + CORE::UInt32ToString( m_hostAddress.GetAddress() ) + ":" + CORE::UInt16ToString( m_hostAddress.GetPort() ) + " in network format" );
        if ( !NotifyObservers( UDPSocketOpenedEvent ) ) return true;

        // We will now be requiring periodic updates to poll for data
        m_pulseGenerator->RequestPeriodicPulses( this, GUCEF_PULSE_UPDATE_INTERVAL );

        return true;
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed to bind to " + m_hostAddress.AddressAndPortAsString() + ". Error code: " + CORE::Int32ToString( errorCode ) );
        TSocketErrorEventData eData( errorCode );
        if ( !NotifyObservers( UDPSocketErrorEvent, &eData ) ) return false;

        if ( m_autoReopenOnError )
            m_pulseGenerator->RequestPeriodicPulses( this, GUCEF_PULSE_UPDATE_INTERVAL );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::SetOsLevelSocketReceiveBufferSize( UInt32 osLevelSocketReceiveBufferSize )
{GUCEF_TRACE;

    m_osLevelSocketReceiveBufferSize = osLevelSocketReceiveBufferSize;

    if ( GUCEF_NULL != _data && 0 != _data->sockid )
    {
        int errorCode = 0;
        int socketReceiveBufferSize = (int) m_osLevelSocketReceiveBufferSize;

        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

        int actualSocketReceiveBufferSize = 0; int actualParamSize = sizeof(int);
        if ( 0 > dvsocket_setsockopt_and_validate( _data->sockid, SOL_SOCKET, SO_RCVBUF, (const char*) &socketReceiveBufferSize, sizeof(int), (char*) &actualSocketReceiveBufferSize, &actualParamSize, &errorCode ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed to set OS level socket receive buffer size to \"" + CORE::Int32ToString( socketReceiveBufferSize )
                + "\" on socket. Error code: " + CORE::UInt32ToString( errorCode ) + ". Actual current value is: " + CORE::Int32ToString( actualSocketReceiveBufferSize )
                + ". Will try to force which requires elevated rights" );

            int newActualSocketReceiveBufferSize = 0; actualParamSize = sizeof(int);
            if ( 0 > dvsocket_setsockopt_and_validate( _data->sockid, SOL_SOCKET, SO_RCVBUFFORCE, (const char*) &socketReceiveBufferSize, sizeof(int), (char*) &actualSocketReceiveBufferSize, &actualParamSize, &errorCode ) )
            {
                if ( 0 == newActualSocketReceiveBufferSize )
                    newActualSocketReceiveBufferSize = actualSocketReceiveBufferSize;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed to force set OS level socket receive buffer size to \"" + CORE::Int32ToString( socketReceiveBufferSize )
                    + "\" on socket. Error code: " + CORE::UInt32ToString( errorCode ) + ". Actual current value is: " + CORE::Int32ToString( actualSocketReceiveBufferSize ) );

                return false;
            }
        }
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Successfully set OS level socket receive buffer size to " + CORE::Int32ToString( socketReceiveBufferSize ) );

        #else

        int actualSocketReceiveBufferSize = 0; int actualParamSize = sizeof(int);
        if ( 0 > dvsocket_setsockopt_and_validate( _data->sockid, SOL_SOCKET, SO_RCVBUF, (const char*) &socketReceiveBufferSize, sizeof(int), (char*) &actualSocketReceiveBufferSize, &actualParamSize, &errorCode ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed to set OS level socket receive buffer size to \"" + CORE::Int32ToString( socketReceiveBufferSize )
                + "\" on socket. Error code: " + CORE::UInt32ToString( errorCode ) + ". Actual current value is: " + CORE::Int32ToString( actualSocketReceiveBufferSize ) );

            return false;
        }
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Successfully set OS level socket receive buffer size to " + CORE::Int32ToString( socketReceiveBufferSize ) );

        #endif
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CUDPSocket::GetOsLevelSocketReceiveBufferSize( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != _data && 0 != _data->sockid )
    {
        int errorCode = 0;
        int actualSocketReceiveBufferSize = 0; int actualParamSize = sizeof(int);
        if ( 0 > dvsocket_getsockopt( _data->sockid, SOL_SOCKET, SO_RCVBUF, (char*) &actualSocketReceiveBufferSize, &actualParamSize, &errorCode ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "):GetOsLevelSocketReceiveBufferSize: Failed to obtain OS level socket receive buffer size. "
                ". Error code: " + CORE::UInt32ToString( errorCode ) );

            return m_osLevelSocketReceiveBufferSize;
        }

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "):GetOsLevelSocketReceiveBufferSize: OS level receive buffer size is: " +
                CORE::Int32ToString( actualSocketReceiveBufferSize ) );
        return (UInt32) actualSocketReceiveBufferSize;
    }

    return m_osLevelSocketReceiveBufferSize;
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::SetDefaultMulticastInterface( const CIPAddress& multicastInterface )
{GUCEF_TRACE;

    struct in_addr interface_addr;
    memset( &interface_addr, 0, sizeof( interface_addr ) );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    interface_addr.S_un.S_addr = multicastInterface.GetAddress();
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    interface_addr.s_addr = multicastInterface.GetAddress();
    #endif

    int errorCode = 0;
    if ( 0 > dvsocket_setsockopt( _data->sockid, IPPROTO_IP, IP_MULTICAST_IF, (char*) &interface_addr, sizeof(interface_addr), &errorCode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed to set default multicast interface " + multicastInterface.AddressAsString()
            + ". Error code: " + CORE::UInt32ToString( errorCode ) )
        return false;
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Successfully set the default multicast interface to " +  multicastInterface.AddressAsString() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::Join( const CIPAddress& multicastGroup ,
                  const CIPAddress& srcAddr        )
{GUCEF_TRACE;

    struct ip_mreq_source imr;
    memset( &imr, 0, sizeof( imr ) );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    imr.imr_multiaddr.S_un.S_addr = multicastGroup.GetAddress();
    imr.imr_sourceaddr.S_un.S_addr = srcAddr.GetAddress();
    imr.imr_interface.S_un.S_addr = m_hostAddress.GetAddress();
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    imr.imr_multiaddr.s_addr = multicastGroup.GetAddress();
    imr.imr_sourceaddr.s_addr = srcAddr.GetAddress();
    imr.imr_interface.s_addr = m_hostAddress.GetAddress();
    #endif

    int errorCode = 0;
    if ( 0 > dvsocket_setsockopt( _data->sockid, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, (char*) &imr, sizeof(imr), &errorCode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed to join multicast group " + multicastGroup.AddressAsString()
            + ", using interface " + m_hostAddress.AddressAsString() + ", for data coming from " + srcAddr.AddressAsString()
            + ". Error code: " + CORE::UInt32ToString( errorCode ) )
        return false;
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Joined multicast group " + multicastGroup.AddressAsString()
        + ", using interface " + m_hostAddress.AddressAsString() + ", for data coming from " + srcAddr.AddressAsString() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::Join( const CIPAddress& multicastGroup )
{GUCEF_TRACE;

    struct ip_mreq_source imr;
    memset( &imr, 0, sizeof( imr ) );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    imr.imr_multiaddr.S_un.S_addr = multicastGroup.GetAddress();
    imr.imr_interface.S_un.S_addr = m_hostAddress.GetAddress();
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    imr.imr_multiaddr.s_addr = multicastGroup.GetAddress();
    imr.imr_interface.s_addr = m_hostAddress.GetAddress();
    #endif

    int errorCode = 0;
    if ( 0 > dvsocket_setsockopt( _data->sockid, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &imr, sizeof(imr), &errorCode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed to join multicast group " + multicastGroup.AddressAsString()
            + ", using interface " + m_hostAddress.AddressAsString()
            + " aka " + CORE::UInt32ToString( m_hostAddress.GetAddress() ) + ":" + CORE::UInt16ToString( m_hostAddress.GetPort() ) + " in network format"
            + ", for all data on the multicast group except explicit blocks"
            + ". Error code: " + CORE::UInt32ToString( errorCode ) )
        return false;
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Joined multicast group " + multicastGroup.AddressAsString()
        + ", using interface " + m_hostAddress.AddressAsString() + ", for all data on the multicast group except explicit blocks" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::Leave( const CIPAddress& multicastGroup ,
                   const CIPAddress& srcAddr        )
{GUCEF_TRACE;

    struct ip_mreq_source imr;
    memset( &imr, 0, sizeof( imr ) );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    imr.imr_multiaddr.S_un.S_addr = multicastGroup.GetAddress();
    imr.imr_sourceaddr.S_un.S_addr = srcAddr.GetAddress();
    imr.imr_interface.S_un.S_addr = m_hostAddress.GetAddress();
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    imr.imr_multiaddr.s_addr = multicastGroup.GetAddress();
    imr.imr_sourceaddr.s_addr = srcAddr.GetAddress();
    imr.imr_interface.s_addr = m_hostAddress.GetAddress();
    #endif

    int errorCode = 0;
    if ( 0 > dvsocket_setsockopt( _data->sockid, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP, (char*) &imr, sizeof(imr), &errorCode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed to leave multicast group " + multicastGroup.AddressAsString()
            + ", using interface " + m_hostAddress.AddressAsString() + ", for data coming data from " + srcAddr.AddressAsString()
            + ". Error code: " + CORE::UInt32ToString( errorCode ) );
        return false;
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Left multicast group " + multicastGroup.AddressAsString()
        + ", using interface " + m_hostAddress.AddressAsString() + ", for data coming from " + srcAddr.AddressAsString() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::Leave( const CIPAddress& multicastGroup )
{GUCEF_TRACE;

    struct ip_mreq_source imr;
    memset( &imr, 0, sizeof( imr ) );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    imr.imr_multiaddr.S_un.S_addr = multicastGroup.GetAddress();
    imr.imr_interface.S_un.S_addr = m_hostAddress.GetAddress();
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    imr.imr_multiaddr.s_addr = multicastGroup.GetAddress();
    imr.imr_interface.s_addr = m_hostAddress.GetAddress();
    #endif

    int errorCode = 0;
    if ( 0 > dvsocket_setsockopt( _data->sockid, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*) &imr, sizeof(imr), &errorCode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed to leave multicast group " + multicastGroup.AddressAsString()
            + ", using interface " + m_hostAddress.AddressAsString() + ", for all data on the multicast group"
            + ". Error code: " + CORE::UInt32ToString( errorCode ) );
        return false;
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Left multicast group " + multicastGroup.AddressAsString()
        + ", using interface " + m_hostAddress.AddressAsString() + ", for all data on the multicast group" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::Block( const CIPAddress& multicastGroup ,
                   const CIPAddress& srcAddr        )
{GUCEF_TRACE;

    struct ip_mreq_source imr;
    memset( &imr, 0, sizeof( imr ) );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    imr.imr_multiaddr.S_un.S_addr = multicastGroup.GetAddress();
    imr.imr_sourceaddr.S_un.S_addr = srcAddr.GetAddress();
    imr.imr_interface.S_un.S_addr = m_hostAddress.GetAddress();
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    imr.imr_multiaddr.s_addr = multicastGroup.GetAddress();
    imr.imr_sourceaddr.s_addr = srcAddr.GetAddress();
    imr.imr_interface.s_addr = m_hostAddress.GetAddress();
    #endif

    int errorCode = 0;
    if ( 0 > dvsocket_setsockopt( _data->sockid, IPPROTO_IP, IP_BLOCK_SOURCE, (char*) &imr, sizeof(imr), &errorCode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed to block data for multicast group " + multicastGroup.AddressAsString()
            + ", using interface " + m_hostAddress.AddressAsString() + ", for data coming data from " + srcAddr.AddressAsString()
            + ". Error code: " + CORE::UInt32ToString( errorCode ) );
        return false;
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Blocking data for multicast group " + multicastGroup.AddressAsString()
        + ", using interface " + m_hostAddress.AddressAsString() + ", for data coming from " + srcAddr.AddressAsString() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::Unblock( const CIPAddress& multicastGroup ,
                     const CIPAddress& srcAddr        )
{GUCEF_TRACE;

    struct ip_mreq_source imr;
    memset( &imr, 0, sizeof( imr ) );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    imr.imr_multiaddr.S_un.S_addr = multicastGroup.GetAddress();
    imr.imr_sourceaddr.S_un.S_addr = srcAddr.GetAddress();
    imr.imr_interface.S_un.S_addr = m_hostAddress.GetAddress();
    #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    imr.imr_multiaddr.s_addr = multicastGroup.GetAddress();
    imr.imr_sourceaddr.s_addr = srcAddr.GetAddress();
    imr.imr_interface.s_addr = m_hostAddress.GetAddress();
    #endif

    int errorCode = 0;
    if ( 0 > dvsocket_setsockopt( _data->sockid, IPPROTO_IP, IP_UNBLOCK_SOURCE, (char*) &imr, sizeof(imr), &errorCode ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed to unblock data for multicast group " + multicastGroup.AddressAsString()
            + ", using interface " + m_hostAddress.AddressAsString() + ", for data coming data from " + srcAddr.AddressAsString()
            + ". Error code: " + CORE::UInt32ToString( errorCode ) );
        return false;
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Unblocking data for multicast group " + multicastGroup.AddressAsString()
        + ", using interface " + m_hostAddress.AddressAsString() + ", for data coming from " + srcAddr.AddressAsString() );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CUDPSocket::Close( bool shutdownOnly )
{GUCEF_TRACE;

    if ( IsActive() )
    {
        NotifyObservers( UDPSocketClosingEvent );

        if ( !shutdownOnly )
        {
            // A socket close will trigger a shutdown sequence if one hasnt occured yet and will also free up the related
            // system resources
            int errorCode = 0;
            if ( 0 == dvsocket_closesocket( _data->sockid, &errorCode ) )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Closed socket at " + m_hostAddress.AddressAndPortAsString() );

                // We now no longer require periodic updates to poll for data
                if ( !m_autoReopenOnError )
                    m_pulseGenerator->RequestStopOfPeriodicUpdates( this );

                NotifyObservers( UDPSocketClosedEvent );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): Failed close socket at " + m_hostAddress.AddressAndPortAsString() );

                TSocketErrorEventData eData( errorCode );
                NotifyObservers( UDPSocketErrorEvent, &eData );
            }
        }
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "UDPSocket(" + CORE::PointerToString( this ) + "): shutting down socket at " + m_hostAddress.AddressAndPortAsString() );
            shutdown( _data->sockid, 1 );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::IsBlocking( void ) const
{GUCEF_TRACE;

    return _blocking;
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::IsActive( void ) const
{GUCEF_TRACE;

    return _data->sockid > 0;
}

/*-------------------------------------------------------------------------*/

UInt16
CUDPSocket::GetPort( void ) const
{GUCEF_TRACE;

    return m_hostAddress.GetPortInHostByteOrder();
}

/*-------------------------------------------------------------------------*/

void
CUDPSocket::SetAutoReOpenOnError( bool autoReOpen )
{GUCEF_TRACE;

    m_autoReopenOnError = autoReOpen;
}

/*-------------------------------------------------------------------------*/

bool
CUDPSocket::GetAutoReOpenOnError( void ) const
{GUCEF_TRACE;

    return m_autoReopenOnError;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
